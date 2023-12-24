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
#include "ProjectMapsScroller.h"
#include "PianoProjectMap.h"
#include "Playhead.h"
#include "Transport.h"
#include "RollBase.h"
#include "HelioTheme.h"
#include "MainLayout.h"

ProjectMapsScroller::ProjectMapsScroller(Transport &transportRef, SafePointer<RollBase> roll) :
    transport(transportRef),
    roll(roll)
{
    this->setOpaque(true);
    this->setPaintingIsUnclipped(true);

    this->playhead = make<Playhead>(*this->roll, this->transport);

    this->helperRectangle = make<HorizontalDragHelper>(*this);
    this->addAndMakeVisible(this->helperRectangle.get());

    this->screenRange = make<ProjectMapsScroller::ScreenRange>(*this);
    this->addAndMakeVisible(this->screenRange.get());
}

ProjectMapsScroller::~ProjectMapsScroller()
{
    this->disconnectPlayhead();
}

void ProjectMapsScroller::disconnectPlayhead()
{
    if (this->playhead->getParentComponent())
    {
        this->playhead->getParentComponent()->removeChildComponent(this->playhead.get());
    }
}

//===----------------------------------------------------------------------===//
// TrackScroller
//===----------------------------------------------------------------------===//

void ProjectMapsScroller::horizontalDragByUser(Component *component, const Rectangle<int> &bounds)
{
    const Rectangle<float> &screenRangeBounds = this->screenRange->getRealBounds();
    this->screenRange->setRealBounds(screenRangeBounds.withX(float(component->getX())));
    this->xMoveByUser();
}

void ProjectMapsScroller::xyMoveByUser()
{
    jassert(this->roll != nullptr);
     
    const auto screenRangeBounds = this->screenRange->getRealBounds();

    const float mw = float(this->getWidth()) - screenRangeBounds.getWidth();
    const float propX = screenRangeBounds.getTopLeft().getX() / mw;
    const float mh = float(this->getHeight()) - screenRangeBounds.getHeight();
    const float propY = screenRangeBounds.getTopLeft().getY() / mh;

    // fixes for header height delta
    const float hh = float(Globals::UI::rollHeaderHeight);
    const float rollHeight = float(this->roll->getHeight());
    const float propY2 = roundf(((rollHeight - hh) * propY) - hh) / rollHeight;
    this->roll->panProportionally(propX, propY2);

    const auto p = this->getIndicatorBounds();
    const auto hp = p.toType<int>();
    this->helperRectangle->setBounds(hp.withTop(1).withBottom(this->getHeight()));
    this->screenRange->setRealBounds(p);

    const auto mapBounds = this->getMapBounds();
    for (auto *map : this->trackMaps)
    {
        map->setBounds(mapBounds);
    }
}

void ProjectMapsScroller::xMoveByUser()
{
    jassert(this->roll != nullptr);

    const auto screenRangeBounds = this->screenRange->getRealBounds();

    const float mw = float(this->getWidth()) - screenRangeBounds.getWidth();
    const float propX = screenRangeBounds.getTopLeft().getX() / mw;
    const float propY = float(this->roll->getViewport().getViewPositionY()) /
        float(this->roll->getHeight() - this->roll->getViewport().getViewHeight());

    this->roll->panProportionally(propX, propY);

    const auto p = this->getIndicatorBounds();
    const auto hp = p.toType<int>();
    this->helperRectangle->setBounds(hp.withTop(1).withBottom(this->getHeight()));
    this->screenRange->setRealBounds(p);

    const auto mapBounds = this->getMapBounds();
    for (auto *map : this->trackMaps)
    {
        map->setBounds(mapBounds);
    }
}

//===----------------------------------------------------------------------===//
// Component
//===----------------------------------------------------------------------===//

void ProjectMapsScroller::resized()
{
    const auto p = this->getIndicatorBounds();
    const auto hp = p.toType<int>();
    this->helperRectangle->setBounds(hp.withTop(1).withBottom(this->getHeight()));
    this->screenRange->setRealBounds(p);
    
    const auto mapBounds = this->getMapBounds();
    for (auto *map : this->trackMaps)
    {
        map->setBounds(mapBounds);
    }
}

void ProjectMapsScroller::paint(Graphics &g)
{
    const auto &theme = HelioTheme::getCurrentTheme();
    g.setFillType({ theme.getSidebarBackground(), {} });
    g.fillRect(this->getLocalBounds());

    g.setColour(this->borderLineDark);
    g.fillRect(0, 0, this->getWidth(), 1);

    g.setColour(this->borderLineLight);
    g.fillRect(0, 1, this->getWidth(), 1);

    if (this->drawingNewScreenRange.hasValue() && !this->drawingNewScreenRange->isEmpty())
    {
        const auto rect = this->getMapBounds().toFloat().getProportion(*this->drawingNewScreenRange);
        g.fillRect(rect);
        g.drawRect(rect);
        g.drawRect(rect);
    }
}

void ProjectMapsScroller::mouseDown(const MouseEvent &event)
{
    if (this->roll->isMultiTouchEvent(event))
    {
        return;
    }

    this->rollViewportPositionAtDragStart = this->roll->getViewport().getViewPosition();

    // this thing feels weird on mobile: panning as the default behaviour is more natural there,
    // while on desktop panning is done via right mouse button everywhere:
    if (!event.source.isTouch() && event.mods.isLeftButtonDown())
    {
        const auto mapBounds = this->getMapBounds();
        this->drawingNewScreenRange = juce::Rectangle<float>{
            (event.position.x - mapBounds.getX()) / float(mapBounds.getWidth()),
            (event.position.y - mapBounds.getY()) / float(mapBounds.getHeight()), 0.f, 0.f };

        this->repaint();
    }
}

void ProjectMapsScroller::mouseDrag(const MouseEvent &event)
{
    if (this->roll->isMultiTouchEvent(event))
    {
        return;
    }

    if (this->stretchedMode())
    {
        if (this->drawingNewScreenRange.hasValue())
        {
            const auto mapBounds = this->getMapBounds();
            const auto r = (event.position.x - mapBounds.getX()) / float(mapBounds.getWidth());
            const auto b = (event.position.y - mapBounds.getY()) / float(mapBounds.getHeight());
            this->drawingNewScreenRange->setRight(r);
            this->drawingNewScreenRange->setBottom(b);
            this->repaint();
        }
        else if (event.source.getIndex() == 0)
        {
            this->setMouseCursor(MouseCursor::DraggingHandCursor);

            const auto mapWidth = this->getMapBounds().getWidth();
            if (mapWidth <= this->getWidth())
            {
                return; // nowhere to move
            }
            
            const auto viewWidth = this->roll->getViewport().getViewWidth();
            const auto dragDistance = float(event.getDistanceFromDragStartX());

            const auto rollWidthProportion =
                float(this->roll->getWidth() - viewWidth) / float(mapWidth - this->getWidth());
            const auto xOffset = jlimit(1, this->roll->getWidth() - viewWidth - 1,
                this->rollViewportPositionAtDragStart.x + int(-dragDistance * rollWidthProportion));

            this->roll->panByOffset(xOffset, this->rollViewportPositionAtDragStart.y);

            this->triggerAsyncUpdate();
        }
    }
    // this feels kinda glitchy, especially on mobile, commenting it out for now:
    //else
    //{
    //    this->setMouseCursor(MouseCursor::DraggingHandCursor);
    //    this->screenRange->setRealBounds(this->screenRange->getRealBounds().withCentre(event.position));
    //    this->xMoveByUser();
    //}
}

void ProjectMapsScroller::mouseUp(const MouseEvent &event)
{
    this->setMouseCursor(MouseCursor::NormalCursor);

    if (this->roll->isMultiTouchEvent(event))
    {
        return;
    }

    if (event.getOffsetFromDragStart().isOrigin())
    {
        App::Layout().broadcastCommandMessage(CommandIDs::ToggleBottomMiniMap);
        return;
    }

    if (this->drawingNewScreenRange.hasValue() && !this->drawingNewScreenRange->isEmpty())
    {
        this->oldAreaBounds = this->getIndicatorBounds();
        this->oldMapBounds = this->getMapBounds().toFloat();
        
        this->roll->zoomAbsolute(*this->drawingNewScreenRange);

        this->drawingNewScreenRange = {};
        this->updateAllBounds();
        this->repaint();

        if (this->animationsEnabled)
        {
            this->startTimerHz(60);
        }
    }
}

void ProjectMapsScroller::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel)
{
    jassert(this->roll != nullptr);
    this->roll->mouseWheelMove(event.getEventRelativeTo(this->roll), wheel);
}

//===----------------------------------------------------------------------===//
// MidiRollListener
//===----------------------------------------------------------------------===//

void ProjectMapsScroller::onMidiRollMoved(RollBase *targetRoll)
{
    if (this->isVisible() && this->roll == targetRoll && !this->isTimerRunning())
    {
        this->triggerAsyncUpdate();
    }
}

void ProjectMapsScroller::onMidiRollResized(RollBase *targetRoll)
{
    if (this->isVisible() && this->roll == targetRoll && !this->isTimerRunning())
    {
        this->triggerAsyncUpdate();
    }
}

// Starts quick and dirty animation from one bounds to another
void ProjectMapsScroller::switchToRoll(SafePointer<RollBase> roll)
{
    this->oldAreaBounds = this->getIndicatorBounds();
    this->oldMapBounds = this->getMapBounds().toFloat();

    this->roll = roll;
    for (auto *map : this->trackMaps)
    {
        map->switchToRoll(roll);
    }

    if (this->animationsEnabled)
    {
        this->startTimerHz(60);
    }
}

//===----------------------------------------------------------------------===//
// Scrolledcomponent
//===----------------------------------------------------------------------===//

void ProjectMapsScroller::ScrolledComponent::switchToRoll(SafePointer<RollBase> roll)
{
    this->roll = roll;
}

bool ProjectMapsScroller::ScrolledComponent::isMultiTouchEvent(const MouseEvent &e) const noexcept
{
    jassert(this->roll != nullptr);
    return this->roll->isMultiTouchEvent(e);
}

//===----------------------------------------------------------------------===//
// Timer
//===----------------------------------------------------------------------===//

static Rectangle<float> lerpMapBounds(const Rectangle<float> &r1,
    const Rectangle<float> &r2, float factor)
{
    const float x1 = r1.getX();
    const float y1 = r1.getY();
    const float x2 = r1.getBottomRight().getX();
    const float y2 = r1.getBottomRight().getY();

    const float dx1 = r2.getX() - x1;
    const float dy1 = r2.getY() - y1;
    const float dx2 = r2.getBottomRight().getX() - x2;
    const float dy2 = r2.getBottomRight().getY() - y2;

    const float lx1 = x1 + dx1 * factor;
    const float ly1 = y1 + dy1 * factor;
    const float lx2 = x2 + dx2 * factor;
    const float ly2 = y2 + dy2 * factor;

    return { lx1, ly1, lx2 - lx1, ly2 - ly1 };
}

static float getMapBoundsDistance(const Rectangle<float> &r1,
    const Rectangle<float> &r2)
{
    return fabs(r1.getX() - r2.getX()) +
        fabs(r1.getY() - r2.getY()) +
        fabs(r1.getWidth() - r2.getWidth()) +
        fabs(r1.getHeight() - r2.getHeight());
}

void ProjectMapsScroller::timerCallback()
{
    const auto mb = this->getMapBounds().toFloat();
    const auto mbLerp = lerpMapBounds(this->oldMapBounds, mb, 0.4f);
    const auto ib = this->getIndicatorBounds();
    const auto ibLerp = lerpMapBounds(this->oldAreaBounds, ib, 0.4f);
    const bool shouldStop = getMapBoundsDistance(this->oldAreaBounds, ib) < 0.5f;
    const auto targetAreaBounds = shouldStop ? ib : ibLerp;
    const auto targetMapBounds = shouldStop ? mb : mbLerp;

    this->oldAreaBounds = targetAreaBounds;
    this->oldMapBounds = targetMapBounds;

    const auto helperBounds = targetAreaBounds.toType<int>();
    this->helperRectangle->setBounds(helperBounds.withTop(1).withBottom(this->getHeight()));
    this->screenRange->setRealBounds(targetAreaBounds);

    for (int i = 0; i < this->trackMaps.size(); ++i)
    {
        this->trackMaps.getUnchecked(i)->setBounds(targetMapBounds.toType<int>());
    }

    if (shouldStop)
    {
        this->stopTimer();
    }
}

//===----------------------------------------------------------------------===//
// AsyncUpdater
//===----------------------------------------------------------------------===//

void ProjectMapsScroller::handleAsyncUpdate()
{
    this->updateAllBounds();
}

void ProjectMapsScroller::updateAllBounds()
{
    const auto p = this->getIndicatorBounds();
    const auto hp = p.toType<int>();
    this->helperRectangle->setBounds(hp.withTop(1).withBottom(this->getHeight()));
    this->screenRange->setRealBounds(p);

    const auto mapBounds = this->getMapBounds();
    for (auto *map : this->trackMaps)
    {
        map->setBounds(mapBounds);
    }

    this->playhead->parentSizeChanged(); // a hack: also update playhead position
}

//===----------------------------------------------------------------------===//
// Private
//===----------------------------------------------------------------------===//

Rectangle<float> ProjectMapsScroller::getIndicatorBounds() const noexcept
{
    jassert(this->roll != nullptr);

    const float viewX = float(this->roll->getViewport().getViewPositionX());
    const float viewWidth = float(jmax(1, this->roll->getViewport().getViewWidth()));
    const float rollWidth = float(this->roll->getWidth());
    const float rollInvisibleArea = rollWidth - viewWidth;
    const float trackWidth = float(this->getWidth());
    const float trackInvisibleArea = float(this->getWidth() - ProjectMapsScroller::screenRangeWidth);
    const float mapWidth = (ProjectMapsScroller::screenRangeWidth * rollWidth) / viewWidth;

    const float zoomFactorY = this->roll->getZoomFactorY();
    const float rollHeaderHeight = float(Globals::UI::rollHeaderHeight);
    const float rollHeight = float(this->roll->getHeight() - rollHeaderHeight);
    const float viewY = float(this->roll->getViewport().getViewPositionY() + rollHeaderHeight);
    const float trackHeight = float(this->getHeight());
    const float trackHeaderHeight = float(rollHeaderHeight * trackHeight / rollHeight);

    const float rY = ceilf(trackHeight * (viewY / rollHeight)) - trackHeaderHeight + 1.f;
    const float rH = trackHeight * zoomFactorY;

    if (mapWidth <= trackWidth || !this->stretchedMode())
    {
        const float rX = (trackWidth * viewX) / rollWidth;
        const float rW = trackWidth * this->roll->getZoomFactorX();
        return { rX, rY, rW, rH };
    }

    const float rX = (trackInvisibleArea * viewX) / jmax(rollInvisibleArea, viewWidth);
    const float rW = ProjectMapsScroller::screenRangeWidth;
    return { rX, rY, rW, rH };
}

Rectangle<int> ProjectMapsScroller::getMapBounds() const noexcept
{
    jassert(this->roll != nullptr);

    const float viewX = float(this->roll->getViewport().getViewPositionX());
    const float viewWidth = float(jmax(1, this->roll->getViewport().getViewWidth()));
    const float rollWidth = float(this->roll->getWidth());
    const float rollInvisibleArea = rollWidth - viewWidth;
    const float trackInvisibleArea = float(this->getWidth() - ProjectMapsScroller::screenRangeWidth);
    const float mapWidth = (ProjectMapsScroller::screenRangeWidth * rollWidth) / viewWidth;

    if (mapWidth <= this->getWidth() || !this->stretchedMode())
    {
        return { 0, 0, this->getWidth(), this->getHeight() };
    }

    const float rX = (trackInvisibleArea * viewX) / jmax(rollInvisibleArea, viewWidth);
    const float dX = (viewX * mapWidth) / rollWidth;
    return { int(rX - dX), 0, int(mapWidth), this->getHeight() };
}

void ProjectMapsScroller::setScrollerMode(ScrollerMode mode)
{
    if (this->scrollerMode == mode)
    {
        return;
    }

    this->scrollerMode = mode;
    const auto isFullMap = mode == ScrollerMode::Map;

    for (auto *map : this->trackMaps)
    {
        if (auto *pianoMap = dynamic_cast<PianoProjectMap *>(map))
        {
            pianoMap->setBrightness(isFullMap ? 1.f : 0.85f);
        }
        else
        {
            map->setVisible(isFullMap);
        }
    }

    this->screenRange->setEnabled(isFullMap);
    this->screenRange->setInterceptsMouseClicks(isFullMap, isFullMap);

    this->updateAllBounds();
}

ProjectMapsScroller::ScrollerMode ProjectMapsScroller::getScrollerMode() const noexcept
{
    return this->scrollerMode;
}

//===----------------------------------------------------------------------===//
// Additional horizontal dragger
//===----------------------------------------------------------------------===//

class HorizontalDragHelperConstrainer final : public ComponentBoundsConstrainer
{
public:

    explicit HorizontalDragHelperConstrainer(ProjectMapsScroller &scrollerRef) :
        scroller(scrollerRef) {}

    void applyBoundsToComponent(Component &component, Rectangle<int> bounds) override
    {
        ComponentBoundsConstrainer::applyBoundsToComponent(component, bounds);
        this->scroller.horizontalDragByUser(&component, bounds);
    }

private:

    ProjectMapsScroller &scroller;
};

ProjectMapsScroller::HorizontalDragHelper::HorizontalDragHelper(ProjectMapsScroller &scrollerRef) :
    scroller(scrollerRef)
{
    this->setPaintingIsUnclipped(true);
    this->setInterceptsMouseClicks(true, false);
    this->setMouseClickGrabsKeyboardFocus(false);
    this->toBack();

    this->setBrightness(HorizontalDragHelper::defaultBrightness);

    this->moveConstrainer = make<HorizontalDragHelperConstrainer>(this->scroller);
    this->moveConstrainer->setMinimumSize(4, 4);
    this->moveConstrainer->setMinimumOnscreenAmounts(0xffffff, 0xffffff, 0xffffff, 0xffffff);
}

void ProjectMapsScroller::HorizontalDragHelper::setBrightness(float brightness)
{
    this->colour = findDefaultColour(ColourIDs::TrackScroller::scrollerFill).withMultipliedAlpha(brightness);
    this->repaint();
}

void ProjectMapsScroller::HorizontalDragHelper::mouseDown(const MouseEvent &e)
{
    this->dragger.startDraggingComponent(this, e);
}

void ProjectMapsScroller::HorizontalDragHelper::mouseDrag(const MouseEvent &e)
{
    this->setMouseCursor(MouseCursor::DraggingHandCursor);
    this->dragger.dragComponent(this, e, this->moveConstrainer.get());
}

void ProjectMapsScroller::HorizontalDragHelper::mouseUp(const MouseEvent &e)
{
    this->setMouseCursor(MouseCursor::NormalCursor);
}

void ProjectMapsScroller::HorizontalDragHelper::paint(Graphics &g)
{
    g.setColour(this->colour);
    g.fillRect(this->getLocalBounds());
    g.fillRect(0.f, 0.f, 1.f, float(this->getHeight()));
    g.fillRect(float(this->getWidth() - 1.f), 0.f, 1.f, float(this->getHeight()));
}
