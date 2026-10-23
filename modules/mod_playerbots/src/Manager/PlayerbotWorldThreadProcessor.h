/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots src/Script/WorldThr/PlayerbotWorldThreadProcessor.h. Bot AI runs in
 * Player::Update on the map-update threads (PlayerbotsPlayerScript::OnAfterUpdate); with MapUpdate.Threads > 1
 * operations that touch objects shared across maps (groups, ...) are queued here and executed on the world thread
 * (PlayerbotsWorldScript::OnUpdate, AC Script/Playerbots.cpp:466-470).
 */

#ifndef PLAYERBOTS_PLAYERBOTWORLDTHREADPROCESSOR_H
#define PLAYERBOTS_PLAYERBOTWORLDTHREADPROCESSOR_H

#include "Log.h"
#include "PlayerbotOperation.h"

#include <memory>
#include <mutex>
#include <queue>

/**
 * @brief Processes thread-unsafe bot operations in the world thread
 *
 * Map threads call QueueOperation(); the world thread calls Update(), which executes a batch of queued
 * operations. Thread-safe queueing, batching, priority/validity checks and statistics as in AC.
 */
class PlayerbotWorldThreadProcessor
{
public:
    static PlayerbotWorldThreadProcessor& instance()
    {
        static PlayerbotWorldThreadProcessor instance;
        return instance;
    }

    /// Update and process queued operations (world thread only, WorldScript::OnUpdate).
    void Update(uint32 diff);

    /// Queue an operation for execution in the world thread. Thread-safe; false if the queue is full.
    bool QueueOperation(std::unique_ptr<PlayerbotOperation> operation);

    /// Current queue size (thread-safe).
    uint32 GetQueueSize() const;

    /// Clear all queued operations (shutdown / emergency).
    void ClearQueue();

    struct Statistics
    {
        uint64 totalOperationsProcessed = 0;
        uint64 totalOperationsFailed = 0;
        uint64 totalOperationsSkipped = 0;
        uint32 currentQueueSize = 0;
        uint32 maxQueueSize = 0;
        uint32 averageExecutionTimeMs = 0;
    };

    Statistics GetStatistics() const;

    /// When disabled, operations are still queued but not processed.
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

private:
    PlayerbotWorldThreadProcessor()
        : m_enabled(true),
          m_maxQueueSize(10000),
          m_batchSize(100),
          m_queueWarningThreshold(80),
          m_timeSinceLastUpdate(0),
          m_updateInterval(50)  // Process at least every 50ms
    {
    }

    ~PlayerbotWorldThreadProcessor() { this->ClearQueue(); }

    PlayerbotWorldThreadProcessor(PlayerbotWorldThreadProcessor const&) = delete;
    PlayerbotWorldThreadProcessor& operator=(PlayerbotWorldThreadProcessor const&) = delete;

    /// Extracts a batch from the queue and executes it (called by Update()).
    void ProcessBatch();

    /// Logs a warning when the queue approaches capacity.
    void CheckQueueHealth();

    // Thread-safe queue
    mutable std::mutex m_queueMutex;
    std::queue<std::unique_ptr<PlayerbotOperation>> m_operationQueue;

    // Configuration
    bool m_enabled;
    uint32 m_maxQueueSize;           // Maximum operations in queue
    uint32 m_batchSize;              // Operations to process per Update()
    uint32 m_queueWarningThreshold;  // Warn when queue reaches this percentage

    // Statistics
    mutable std::mutex m_statsMutex;
    Statistics m_stats;

    // Timing
    uint32 m_timeSinceLastUpdate;
    uint32 m_updateInterval;  // Minimum ms between updates
};

#endif
