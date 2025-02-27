/*
    This file is part of Helio music sequencer.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "ProjectNode.h"
#include "ProjectMetadata.h"
#include "RescalePreviewTool.h"
#include "PianoRoll.h"
#include "SequencerOperations.h"
#include "NoteComponent.h"
#include "PianoSequence.h"
#include "KeySignaturesSequence.h"
#include "CommandIDs.h"
#include "Config.h"

RescalePreviewTool *RescalePreviewTool::createWithinSelectionAndContext(SafePointer<PianoRoll> roll,
    WeakReference<MidiTrack> keySignatures)
{
    if (roll->getLassoSelection().getNumSelected() > 0)
    {
        Note::Key key;
        Scale::Ptr scale = nullptr;
        const Clip &clip = roll->getLassoSelection().getFirstAs<NoteComponent>()->getClip();
        if (!SequencerOperations::findHarmonicContext(roll->getLassoSelection(), clip, keySignatures, scale, key))
        {
            DBG("Warning: harmonic context could not be detected");
            return new RescalePreviewTool(roll, 0, Scale::makeNaturalMajorScale());
        }

        return new RescalePreviewTool(roll, key, scale);
    }

    return nullptr;
}

RescalePreviewTool::RescalePreviewTool(SafePointer<PianoRoll> roll,
    Note::Key keyContext, Scale::Ptr scaleContext) :
    roll(roll),
    keyContext(keyContext),
    scaleContext(scaleContext)
{
    // this code pretty much duplicates menu from PianoRollSelectionMenu,
    // but adds undos and starts/stops playback of the selected fragment

    MenuPanel::Menu menu;
    menu.add(MenuItem::item(Icons::close, TRANS(I18n::Menu::cancel))->withAction([this]()
    {
        this->undoIfNeeded();
        this->dismissCalloutAsync();
    }));

    const auto scales = App::Config().getScales()->getAll();
    for (int i = 0; i < scales.size(); ++i)
    {
        if (scales.getUnchecked(i)->getBasePeriod() != this->roll->getPeriodSize())
        {
            continue;
        }

        menu.add(MenuItem::item(Icons::arpeggiate,
            scales.getUnchecked(i)->getLocalizedName())->withAction([this, i]()
        {
            if (this->scaleContext == nullptr)
            {
                jassertfalse;
                return;
            }

            auto &transport = this->roll->getTransport();
            const auto scales = App::Config().getScales()->getAll();
            if (!scales[i]->isEquivalentTo(this->lastChosenScale))
            {
                transport.stopPlaybackAndRecording();
                const bool needsCheckpoint = !this->hasMadeChanges;
                this->undoIfNeeded();

                SequencerOperations::rescale(this->roll->getLassoSelection(),
                    this->keyContext, this->scaleContext, scales[i], needsCheckpoint);

                this->lastChosenScale = scales[i];
                this->hasMadeChanges = true;
            }

            if (transport.isPlaying())
            {
                transport.stopPlaybackAndRecording();
            }
            else
            {
                const auto firstBeat = this->roll->getLassoStartBeat();
                const auto lastBeat = this->roll->getLassoEndBeat();
                transport.startPlaybackFragment(firstBeat - 0.001f, lastBeat, true);
            }
        }));
    }

    this->updateContent(menu, MenuPanel::SlideUp);
}

void RescalePreviewTool::handleCommandMessage(int commandId)
{
    if (commandId == CommandIDs::Cancel)
    {
        this->undoIfNeeded();
    }
}

void RescalePreviewTool::dismissCalloutAsync()
{
    if (auto *parent = this->getParentComponent())
    {
        parent->postCommandMessage(CommandIDs::DismissCallout);
    }
}

void RescalePreviewTool::undoIfNeeded()
{
    if (this->hasMadeChanges)
    {
        this->roll->getActiveTrack()->getSequence()->undo();
    }
}

// One-shot rescale menu
QuickRescaleMenu::QuickRescaleMenu(const ProjectNode &project,
    const KeySignatureEvent &event, float endBeat) :
    project(project),
    event(event),
    endBeat(endBeat)
{
    MenuPanel::Menu menu;

    const auto scales = App::Config().getScales()->getAll();
    for (int i = 0; i < scales.size(); ++i)
    {
        if (scales.getUnchecked(i)->getBasePeriod() !=
            this->project.getProjectInfo()->getTemperament()->getPeriodSize())
        {
            continue;
        }

        menu.add(MenuItem::item(Icons::arpeggiate,
            scales.getUnchecked(i)->getLocalizedName())->withAction([this, i]()
        {
            const auto scales = App::Config().getScales()->getAll();
            if (!scales[i]->isEquivalentTo(this->event.getScale()))
            {
                const bool hasMadeChanges = 
                    SequencerOperations::rescale(this->project, this->event.getBeat(), this->endBeat,
                    this->event.getRootKey(), this->event.getScale(), scales[i], true);

                auto *keySequence = static_cast<KeySignaturesSequence *>(this->event.getSequence());
                if (!hasMadeChanges)
                {
                    keySequence->checkpoint();
                }

                keySequence->change(this->event, this->event.withScale(scales[i]), true);

                this->dismissCalloutAsync();
            }
        }));
    }

    this->updateContent(menu, MenuPanel::SlideDown);
}

void QuickRescaleMenu::dismissCalloutAsync()
{
    if (auto *parent = this->getParentComponent())
    {
        parent->postCommandMessage(CommandIDs::DismissCallout);
    }
}
