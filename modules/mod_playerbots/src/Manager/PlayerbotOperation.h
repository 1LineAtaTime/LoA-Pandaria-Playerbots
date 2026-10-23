/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots src/Script/WorldThr/PlayerbotOperation.h (MapUpdate.Threads > 1 safety,
 * #650/#231). Placed in Manager/ (an existing include directory) instead of a new Script/WorldThr/ directory.
 */

#ifndef PLAYERBOTS_PLAYERBOTOPERATION_H
#define PLAYERBOTS_PLAYERBOTOPERATION_H

#include "Common.h"
#include "ObjectGuid.h"
#include <memory>
#include <string>

/**
 * @brief Base class for thread-unsafe operations that must be executed in the world thread
 *
 * PlayerbotOperation represents an operation that needs to be deferred from a map thread
 * to the world thread for safe execution. Examples include group modifications, LFG operations,
 * guild operations, etc.
 *
 * Thread Safety:
 * - The constructor and data members must be thread-safe (use copies, not pointers)
 * - Execute() is called in the world thread and can safely perform thread-unsafe operations
 * - Subclasses must not store raw pointers to (core/world thread) game object (use ObjectGuid instead)
 */
class PlayerbotOperation
{
public:
    virtual ~PlayerbotOperation() = default;

    /// Execute this operation in the world thread (PlayerbotWorldThreadProcessor::Update). Returns true on success.
    virtual bool Execute() = 0;

    /// The bot GUID this operation is for (logging only).
    virtual ObjectGuid GetBotGuid() const { return ObjectGuid::Empty; }

    /// Priority (higher = more urgent): 100 critical, 50 high (player-facing), 10 normal, 0 low.
    virtual uint32 GetPriority() const { return 10; }

    /// Human-readable name for logging.
    virtual std::string GetName() const { return "Unknown Operation"; }

    /// Checked before Execute(); false skips the operation (e.g. the bot logged out meanwhile).
    virtual bool IsValid() const { return true; }
};

/**
 * @brief Comparison operator for priority queue (higher priority first)
 */
struct PlayerbotOperationComparator
{
    bool operator()(std::unique_ptr<PlayerbotOperation> const& a, std::unique_ptr<PlayerbotOperation> const& b) const
    {
        return a->GetPriority() < b->GetPriority();  // Lower priority goes to back of queue
    }
};

#endif
