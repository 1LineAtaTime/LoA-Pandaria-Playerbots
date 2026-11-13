/*
* This file is part of Project SkyFire https://www.projectskyfire.org.
* See ../../scripts/Spells/SKYFIRE27-LICENSE.md for Copyright information
*/

#ifndef SKYFIRE_GAME_EVENT_SCHEDULE_H
#define SKYFIRE_GAME_EVENT_SCHEDULE_H

#include <cstdint>

#include <ctime>

namespace GameEventSchedule
{
    struct EventWindow
    {
        time_t Start;
        time_t End;
    };

    bool GetMonthlyFirstWeekdayWindow(time_t currentTime, int weekday, std::uint32_t durationMinutes, EventWindow& window);
    bool IsMonthlyFirstWeekdayActive(time_t currentTime, int weekday, std::uint32_t durationMinutes);
    std::uint32_t GetSecondsUntilMonthlyFirstWeekdayTransition(time_t currentTime, int weekday, std::uint32_t durationMinutes);
}

#endif
