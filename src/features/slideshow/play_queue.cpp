//
// Created by Razor on 15/10/2026.
//

#include "play_queue.h"

#include <unordered_set>

void PlayQueue::sync(const std::vector<std::string>& ids)
{
    // Sets answer "is this id in there?" instantly, instead of scanning a list each time.
    const std::unordered_set<std::string> current(ids.begin(), ids.end());
    const std::unordered_set<std::string> known(order.begin(), order.end());

    // 1. Removed photos leave the queue right away (std::erase_if is C++20).
    std::erase_if(upcoming, [&current](const std::string& id) { return !current.contains(id); });
    changes.removed = 0;
    for (const auto& id : order)
        if (!current.contains(id))
            changes.removed++;

    // 2. New photos go to the FRONT, in the source's order, so they are shown next.
    std::vector<std::string> added;
    for (const auto& id : ids)
        if (!known.contains(id))
            added.push_back(id);
    upcoming.insert(upcoming.begin(), added.begin(), added.end());
    changes.added = added.size();

    order = ids;
}

std::string PlayQueue::next()
{
    if (upcoming.empty())  // lap finished: start a new one
        upcoming.assign(order.begin(), order.end());

    if (upcoming.empty())
        return {};

    std::string id = std::move(upcoming.front());
    upcoming.pop_front();
    return id;
}
