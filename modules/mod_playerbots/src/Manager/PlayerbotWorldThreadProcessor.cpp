/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots src/Script/WorldThr/PlayerbotWorldThreadProcessor.cpp (LOG_* {} -> TC_LOG_* %).
 */

#include "PlayerbotWorldThreadProcessor.h"

#include "Log.h"
#include "Timer.h"

#include <algorithm>
#include <exception>
#include <vector>

void PlayerbotWorldThreadProcessor::Update(uint32 diff)
{
    if (!m_enabled)
        return;

    // Accumulate time
    m_timeSinceLastUpdate += diff;

    // Don't process too frequently to reduce overhead
    if (m_timeSinceLastUpdate < m_updateInterval)
        return;

    m_timeSinceLastUpdate = 0;

    // Check queue health (warn if getting full)
    CheckQueueHealth();

    // Process a batch of operations
    ProcessBatch();
}

bool PlayerbotWorldThreadProcessor::QueueOperation(std::unique_ptr<PlayerbotOperation> operation)
{
    if (!operation)
    {
        TC_LOG_ERROR("playerbots", "Attempted to queue null operation");
        return false;
    }

    std::lock_guard<std::mutex> lock(m_queueMutex);

    // Check if queue is full
    if (m_operationQueue.size() >= m_maxQueueSize)
    {
        TC_LOG_ERROR("playerbots", "PlayerbotWorldThreadProcessor queue is full (%u operations). Dropping operation: %s",
                     m_maxQueueSize, operation->GetName().c_str());

        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.totalOperationsSkipped++;
        return false;
    }

    // Queue the operation
    m_operationQueue.push(std::move(operation));

    // Update statistics
    {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.currentQueueSize = static_cast<uint32>(m_operationQueue.size());
        m_stats.maxQueueSize = std::max(m_stats.maxQueueSize, m_stats.currentQueueSize);
    }

    return true;
}

void PlayerbotWorldThreadProcessor::ProcessBatch()
{
    // Extract a batch of operations from the queue
    std::vector<std::unique_ptr<PlayerbotOperation>> batch;
    batch.reserve(m_batchSize);

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);

        // Extract up to batchSize operations
        while (!m_operationQueue.empty() && batch.size() < m_batchSize)
        {
            batch.push_back(std::move(m_operationQueue.front()));
            m_operationQueue.pop();
        }

        // Update current queue size stat
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats.currentQueueSize = static_cast<uint32>(m_operationQueue.size());
    }

    // Execute operations outside of lock to avoid blocking queue
    uint32 totalExecutionTime = 0;
    for (auto& operation : batch)
    {
        if (!operation)
            continue;

        try
        {
            // Check if operation is still valid
            if (!operation->IsValid())
            {
                TC_LOG_DEBUG("playerbots", "Skipping invalid operation: %s", operation->GetName().c_str());

                std::lock_guard<std::mutex> statsLock(m_statsMutex);
                m_stats.totalOperationsSkipped++;
                continue;
            }

            // Time the execution
            uint32 startTime = getMSTime();

            // Execute the operation
            bool success = operation->Execute();

            uint32 executionTime = GetMSTimeDiffToNow(startTime);
            totalExecutionTime += executionTime;

            // Log slow operations
            if (executionTime > 100)
                TC_LOG_WARN("playerbots", "Slow operation: %s took %ums", operation->GetName().c_str(), executionTime);

            // Update statistics
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            if (success)
                m_stats.totalOperationsProcessed++;
            else
            {
                m_stats.totalOperationsFailed++;
                TC_LOG_DEBUG("playerbots", "Operation failed: %s", operation->GetName().c_str());
            }
        }
        catch (std::exception const& e)
        {
            TC_LOG_ERROR("playerbots", "Exception in operation %s: %s", operation->GetName().c_str(), e.what());

            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalOperationsFailed++;
        }
        catch (...)
        {
            TC_LOG_ERROR("playerbots", "Unknown exception in operation %s", operation->GetName().c_str());

            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalOperationsFailed++;
        }
    }

    // Update average execution time
    if (!batch.empty())
    {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        uint32 avgTime = totalExecutionTime / static_cast<uint32>(batch.size());
        // Exponential moving average
        m_stats.averageExecutionTimeMs = (m_stats.averageExecutionTimeMs * 9 + avgTime) / 10;  // 90% old, 10% new
    }
}

void PlayerbotWorldThreadProcessor::CheckQueueHealth()
{
    uint32 queueSize = GetQueueSize();
    uint32 threshold = (m_maxQueueSize * m_queueWarningThreshold) / 100;

    if (queueSize >= threshold)
    {
        TC_LOG_WARN("playerbots",
                    "PlayerbotWorldThreadProcessor queue is %u%% full (%u/%u). "
                    "Consider increasing update frequency or batch size.",
                    (queueSize * 100) / m_maxQueueSize, queueSize, m_maxQueueSize);
    }
}

uint32 PlayerbotWorldThreadProcessor::GetQueueSize() const
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return static_cast<uint32>(m_operationQueue.size());
}

void PlayerbotWorldThreadProcessor::ClearQueue()
{
    std::lock_guard<std::mutex> lock(m_queueMutex);

    // Clear the queue (AC logs the count here; skipped because ClearQueue() also runs during static destruction,
    // after the logger may already be gone)
    while (!m_operationQueue.empty())
        m_operationQueue.pop();

    // Reset queue size stat
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    m_stats.currentQueueSize = 0;
}

PlayerbotWorldThreadProcessor::Statistics PlayerbotWorldThreadProcessor::GetStatistics() const
{
    std::lock_guard<std::mutex> statsLock(m_statsMutex);
    return m_stats;  // Return a copy
}
