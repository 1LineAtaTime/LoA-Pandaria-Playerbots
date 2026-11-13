/*
* This file is part of Project SkyFire https://www.projectskyfire.org.
* See ../../scripts/Spells/SKYFIRE27-LICENSE.md for Copyright information
*/

#include "GameEventSchedule.h"


#include <limits>

namespace
{
    bool LocalTime(time_t value, tm& result)
    {
#ifdef _WIN32
        return localtime_s(&result, &value) == 0;
#else
        return localtime_r(&value, &result) != nullptr;
#endif
    }

    bool GetFirstWeekdayOfMonth(time_t referenceTime, int monthOffset, int weekday, time_t& result)
    {
        if (weekday < 0 || weekday > 6)
            return false;

        tm localTime;
        if (!LocalTime(referenceTime, localTime))
            return false;

        localTime.tm_mon += monthOffset;
        localTime.tm_mday = 1;
        localTime.tm_hour = 0;
        localTime.tm_min = 0;
        localTime.tm_sec = 0;
        localTime.tm_isdst = -1;

        time_t firstOfMonth = mktime(&localTime);
        if (firstOfMonth == time_t(-1) || !LocalTime(firstOfMonth, localTime))
            return false;

        localTime.tm_mday += (weekday - localTime.tm_wday + 7) % 7;
        localTime.tm_isdst = -1;
        result = mktime(&localTime);
        return result != time_t(-1);
    }

    bool AddLocalMinutes(time_t startTime, std::uint32_t durationMinutes, time_t& result)
    {
        tm localTime;
        if (!LocalTime(startTime, localTime))
            return false;

        localTime.tm_mday += durationMinutes / (24 * 60);
        localTime.tm_min += durationMinutes % (24 * 60);
        localTime.tm_isdst = -1;
        result = mktime(&localTime);
        return result != time_t(-1);
    }

    std::uint32_t GetSecondsUntil(time_t currentTime, time_t transitionTime)
    {
        if (transitionTime <= currentTime)
            return 0;

        double seconds = difftime(transitionTime, currentTime);
        if (seconds >= std::numeric_limits<std::uint32_t>::max())
            return std::numeric_limits<std::uint32_t>::max();

        return std::uint32_t(seconds);
    }
}

bool GameEventSchedule::GetMonthlyFirstWeekdayWindow(time_t currentTime, int weekday, std::uint32_t durationMinutes, EventWindow& window)
{
    if (!durationMinutes || !GetFirstWeekdayOfMonth(currentTime, 0, weekday, window.Start))
        return false;

    if (currentTime < window.Start && !GetFirstWeekdayOfMonth(currentTime, -1, weekday, window.Start))
        return false;

    return AddLocalMinutes(window.Start, durationMinutes, window.End);
}

bool GameEventSchedule::IsMonthlyFirstWeekdayActive(time_t currentTime, int weekday, std::uint32_t durationMinutes)
{
    EventWindow window;
    return GetMonthlyFirstWeekdayWindow(currentTime, weekday, durationMinutes, window)
        && currentTime >= window.Start
        && currentTime < window.End;
}

std::uint32_t GameEventSchedule::GetSecondsUntilMonthlyFirstWeekdayTransition(time_t currentTime, int weekday, std::uint32_t durationMinutes)
{
    time_t currentMonthStart;
    if (!durationMinutes || !GetFirstWeekdayOfMonth(currentTime, 0, weekday, currentMonthStart))
        return 0;

    if (currentTime < currentMonthStart)
        return GetSecondsUntil(currentTime, currentMonthStart);

    time_t currentMonthEnd;
    if (!AddLocalMinutes(currentMonthStart, durationMinutes, currentMonthEnd))
        return 0;

    if (currentTime < currentMonthEnd)
        return GetSecondsUntil(currentTime, currentMonthEnd);

    time_t nextStart;
    if (!GetFirstWeekdayOfMonth(currentTime, 1, weekday, nextStart))
        return 0;

    return GetSecondsUntil(currentTime, nextStart);
}
