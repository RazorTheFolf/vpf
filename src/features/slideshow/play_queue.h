//
// Created by Razor on 15/10/2026.
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

    // Sync with the source's current photos
    void sync(const std::vector<std::string>& ids);

    Changes lastChanges() const { return changes; }

    // "" when empty
    std::string next();

    size_t size() const { return order.size(); }

private:
    std::vector<std::string> order;    // Every photo, in source order
    std::deque<std::string> upcoming;  // Left in this lap
    Changes changes;
};
