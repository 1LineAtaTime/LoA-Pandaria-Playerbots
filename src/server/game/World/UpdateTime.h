/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// #912: AC's world update time statistics (azerothcore-wotlk src/server/game/Time/UpdateTime.h), ported 1:1 so `.server info`
// reports the same "last 500 diffs" mean / median / p95 / p99 / max on both cores (#650 perf tracker).

#ifndef __UPDATETIME_H
#define __UPDATETIME_H

#include "Define.h"
#include "Duration.h"
#include <array>
#include <string>

constexpr auto AVG_DIFF_COUNT = 500;

class TC_GAME_API UpdateTime
{
using DiffTableArray = std::array<uint32, AVG_DIFF_COUNT>;

public:
    uint32 GetAverageUpdateTime() const;
    uint32 GetTimeWeightedAverageUpdateTime() const;
    uint32 GetMaxUpdateTime() const;
    uint32 GetMaxUpdateTimeOfCurrentTable() const;
    uint32 GetLastUpdateTime() const;
    uint32 GetDatasetSize() const;
    uint32 GetPercentile(uint8 p);

    void UpdateWithDiff(uint32 diff);

    void RecordUpdateTimeReset();

protected:
    UpdateTime();

    void SortUpdateTimeDataTable();

private:
    DiffTableArray _updateTimeDataTable;
    uint32 _averageUpdateTime;
    uint32 _totalUpdateTime;
    uint32 _updateTimeTableIndex;
    uint32 _maxUpdateTime;
    uint32 _maxUpdateTimeOfLastTable;
    uint32 _maxUpdateTimeOfCurrentTable;

    DiffTableArray _orderedUpdateTimeDataTable;
    bool _needsReorder;

    uint32 _recordedTime;
};

class TC_GAME_API WorldUpdateTime : public UpdateTime
{
public:
    WorldUpdateTime() : UpdateTime(), _recordUpdateTimeInverval(0), _recordUpdateTimeMin(0), _lastRecordTime(0) { }
    void LoadFromConfig();
    void SetRecordUpdateTimeInterval(Milliseconds t);
    void RecordUpdateTime(uint32 gameTimeMs, uint32 diff, uint32 sessionCount);

private:
    Milliseconds _recordUpdateTimeInverval;
    Milliseconds _recordUpdateTimeMin;
    uint32 _lastRecordTime;
};

TC_GAME_API extern WorldUpdateTime sWorldUpdateTime;

#endif
