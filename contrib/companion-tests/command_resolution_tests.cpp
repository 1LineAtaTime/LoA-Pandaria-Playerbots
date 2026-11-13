#include "VisibleCommandMatches.h"
#include <map>
#include <iostream>
struct Node { int id; bool visible; };
int main()
{
    std::map<std::string, Node> commands{{"bot", {1,true}}, {"botprobe",{2,true}}, {"botscenario",{3,true}}};
    auto lookup = [&](std::string_view token) {
        return Trinity::Impl::ChatCommands::VisibleCommandMatches(commands, token,
            [](Node const& n) { return n.visible; },
            [](std::string_view s, std::string_view p) { return s.substr(0,p.size()) == p; });
    };
    if (lookup("bot").size()!=1 || lookup("bot")[0]->id!=1) return 1;
    if (lookup("bo").size()!=3 || lookup("botp").size()!=1) return 2;
    commands["bot"].visible=false; // Reproduces the deployment with missing permission 1346.
    if (!lookup("bot").empty() || lookup("bo").size()!=2) return 3;
    commands["botprobe"].visible=false;
    if (!lookup("botp").empty() || lookup("bots").size()!=1) return 4;
    if (!lookup("").empty() || !lookup("missing").empty()) return 5;
    std::cout << "Exact, abbreviated and permission-filtered command resolution passed\n";
}
