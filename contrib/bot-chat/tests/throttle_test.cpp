#include "../../../modules/mod_playerbots/src/Utils/PublicChatThrottle.h"
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <stdexcept>
static void check(bool value) { if (!value) throw std::runtime_error("chat policy regression"); }
int main()
{
    PlayerbotPublicChatThrottle gate;
    int sends = 0;
    auto send = [&] { ++sends; return true; };
    check(!gate.TrySend(100, "general", 1, "a", 30, 300, 1800, [] { return false; }));
    check(gate.TrySend(100, "general", 1, "a", 30, 300, 1800, send));
    check(!gate.TrySend(101, "general", 2, "b", 30, 300, 1800, send));
    check(!gate.TrySend(131, "trade", 1, "b", 30, 300, 1800, send));
    check(!gate.TrySend(131, "trade", 2, "a", 30, 300, 1800, send));
    check(gate.TrySend(130, "general", 2, "b", 30, 300, 1800, send));
    check(gate.TrySend(1900, "general", 1, "a", 30, 300, 1800, send));
    check(sends == 3);
    PlayerbotPublicChatThrottle concurrent;
    std::atomic<int> delivered{0}; std::vector<std::thread> threads;
    for (int i=0; i<32; ++i)
        threads.emplace_back([&,i] {
            concurrent.TrySend(1000,"busy-zone",i,std::to_string(i),30,300,1800,[&] { ++delivered; return true; });
        });
    for (auto& t:threads) t.join();
    check(delivered == 1);
    std::cout << "PASS: channel/bot/repeat budgets, failed send, expiry and 32-thread contention\n";
}
