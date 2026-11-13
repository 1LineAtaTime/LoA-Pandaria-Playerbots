#ifndef PLAYERBOT_PUBLIC_CHAT_THROTTLE_H
#define PLAYERBOT_PUBLIC_CHAT_THROTTLE_H

#include <algorithm>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>

// Shared by all map threads; only successful bot broadcasts consume the budget.
// Callers supply time and jitter so policy tests need neither a server nor sleeps.
class PlayerbotPublicChatThrottle
{
public:
    template<class Send>
    bool TrySend(std::int64_t now, std::string const& channel, std::uint64_t bot,
                 std::string const& phrase, std::uint32_t channelDelay,
                 std::uint32_t botDelay, std::uint32_t repeatDelay, Send send)
    {
        std::lock_guard<std::mutex> guard(lock);
        if (now >= nextPrune)
        {
            Prune(channels, now); Prune(bots, now); Prune(phrases, now);
            nextPrune = now + 60;
        }
        if (Blocked(channels, channel, now) || Blocked(bots, bot, now) || Blocked(phrases, phrase, now))
            return false;
        if (!send())
            return false;
        channels[channel] = now + channelDelay;
        bots[bot] = now + botDelay;
        phrases[phrase] = now + repeatDelay;
        return true;
    }

private:
    template<class K> static bool Blocked(std::map<K, std::int64_t> const& values, K const& key, std::int64_t now)
    {
        auto i = values.find(key);
        return i != values.end() && i->second > now;
    }
    template<class K> static void Prune(std::map<K, std::int64_t>& values, std::int64_t now)
    {
        for (auto i = values.begin(); i != values.end(); )
            if (i->second <= now) i = values.erase(i); else ++i;
    }
    std::mutex lock;
    std::map<std::string, std::int64_t> channels, phrases;
    std::map<std::uint64_t, std::int64_t> bots;
    std::int64_t nextPrune = 0;
};
#endif
