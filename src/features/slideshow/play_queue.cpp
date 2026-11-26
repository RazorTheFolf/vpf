//
// Created by Razor on 15/10/2026.
//

#include "play_queue.h"

#include <unordered_set>

void PlayQueue::sync(const std::vector<std::string>& ids)
{
    const std::unordered_set<std::string> current(ids.begin(), ids.end());
    const std::unordered_set<std::string> known(order.begin(), order.end());

    // Drop removed photos
    std::erase_if(upcoming, [&current](const std::string& id) { return !current.contains(id); });
    changes.removed = 0;
    for (const auto& id : order)
        if (!current.contains(id))
            changes.removed++;

    // New photos go first so they show next
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
    if (upcoming.empty())  // Lap done, start over
        upcoming.assign(order.begin(), order.end());

    if (upcoming.empty())
        return {};

    std::string id = std::move(upcoming.front());
    upcoming.pop_front();
    return id;
}
