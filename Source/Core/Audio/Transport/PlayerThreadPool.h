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

#pragma once

class PlayerThreadPool final
{
public:

    PlayerThreadPool(Transport &transport, int poolSize = 5) :
        transport(transport),
        minPoolSize(poolSize)
    {
        for (int i = 0; i < poolSize; ++i)
        {
            this->players.add(new PlayerThread(transport));
        }

        this->currentPlayer = this->findNextFreePlayer();
    }

    ~PlayerThreadPool()
    {
        // Send exit signal to all threads before they are stopped forcefully,
        // so that we don't have to wait for each one separately.
        for (int i = 0; i < this->players.size(); ++i)
        {
            this->players.getUnchecked(i)->signalThreadShouldExit();
        }
    }

    void startPlayback(float startBeat, float rewindBeat, float endBeat, bool loopMode)
    {
        if (this->currentPlayer->isThreadRunning())
        {
            this->currentPlayer->signalThreadShouldExit();
            this->currentPlayer = this->findNextFreePlayer();
        }

        auto playbackContext = this->transport.fillPlaybackContextAt(startBeat);
        playbackContext->endBeat = endBeat;
        playbackContext->rewindBeat = rewindBeat;
        playbackContext->playbackLoopMode = loopMode;

        // let listeners know about the tempo before the playback starts
        this->transport.broadcastCurrentTempoChanged(playbackContext->startBeatTempo);
        this->currentPlayer->startPlayback(playbackContext);
    }

    void stopPlayback()
    {
        if (this->currentPlayer->isThreadRunning())
        {
            // Just signal player to stop:
            // it might be waiting for the next midi event, so it won't stop immediately
            this->currentPlayer->signalThreadShouldExit();
        }
    }

    bool isPlaying() const
    {
        return (this->currentPlayer->isThreadRunning() &&
            !this->currentPlayer->threadShouldExit());
    }

private:

    PlayerThread *findNextFreePlayer()
    {
        this->cleanup();

        for (auto *player : this->players)
        {
            if (!player->isThreadRunning())
            {
                return player;
            }
        }

        DBG("Warning: all playback threads are busy, adding one");
        return this->players.add(new PlayerThread(transport));
    }

    void cleanup()
    {
        // Since all new players are added last,
        // first ones are most likely to be stopped,
        // so simply try to cleanup from the beginning until we meet a busy one:
        while (this->players.size() > this->minPoolSize)
        {
            if (this->players.getFirst() == this->currentPlayer ||
                this->players.getFirst()->isThreadRunning())
            {
                return;
            }

            DBG("Removing a stale playback thread");
            this->players.remove(0);
        }
    }

    Transport &transport;

    const int minPoolSize;

    OwnedArray<PlayerThread> players;
    PlayerThread *currentPlayer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerThreadPool)
};
