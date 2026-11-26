//
// Created by Razor on 15/10/2026.
//
// Decides which photo comes next, and keeps that order sensible while the
// source changes: removed photos leave the queue at once, new photos are shown next.
//
// Photos are identified by a string (Immich asset id, file name...), never by
// position, so adding or removing one doesn't shift the others.
//

#pragma once
#include <cstddef>
#include <deque>
#include <string>
#include <vector>

class PlayQueue
{
public:
    struct Changes
    {
        size_t added = 0;
        size_t removed = 0;
    };

    // Brings the queue in line with the source's current photos (in display order).
    void sync(const std::vector<std::string>& ids);

    // What the last sync() changed (for logging).
    Changes lastChanges() const { return changes; }

    // Id of the next photo to show, or "" if there are none.
    // When a lap is finished, starts a new one in the source's order.
    std::string next();

    size_t size() const { return order.size(); }

private:
    std::vector<std::string> order;    // every photo, in the source's order
    std::deque<std::string> upcoming;  // photos still to show in this lap (front = next)
    Changes changes;
};
