/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * New context (#173), mirroring AC mod-playerbots Ai/Base/ChatTriggerContext.h. Registers only the
 * chat-command triggers ported in this pass (see ChatActionContext.h for the matching actions).
 */

#ifndef _PLAYERBOT_CHATTRIGGERCONTEXT_H
#define _PLAYERBOT_CHATTRIGGERCONTEXT_H

#include "ChatCommandTrigger.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class ChatTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ChatTriggerContext()
    {
        creators["who"] = &ChatTriggerContext::who;
        creators["talents"] = &ChatTriggerContext::talents;  // AC ChatTriggerContext.h:60
        creators["help"] = &ChatTriggerContext::help;
        creators["co"] = &ChatTriggerContext::co;
        creators["nc"] = &ChatTriggerContext::nc;
        creators["de"] = &ChatTriggerContext::dead;
        creators["rti"] = &ChatTriggerContext::rti;
        creators["follow"] = &ChatTriggerContext::follow;
        creators["stay"] = &ChatTriggerContext::stay;
        creators["move from group"] = &ChatTriggerContext::move_from_group;

        // #177 (group-functionality-parity.md section 3.1a)
        creators["flee"] = &ChatTriggerContext::flee;
        creators["runaway"] = &ChatTriggerContext::runaway;
        creators["grind"] = &ChatTriggerContext::grind;
        creators["tank attack"] = &ChatTriggerContext::tank_attack;
        creators["max dps"] = &ChatTriggerContext::max_dps;
        creators["attack"] = &ChatTriggerContext::attack;
        creators["save mana"] = &ChatTriggerContext::save_mana;
        creators["formation"] = &ChatTriggerContext::formation;
        creators["range"] = &ChatTriggerContext::range;
        creators["stance"] = &ChatTriggerContext::stance;
        creators["position"] = &ChatTriggerContext::position;
        creators["summon"] = &ChatTriggerContext::summon;
        creators["reset botAI"] = &ChatTriggerContext::reset_ai;
        creators["invite"] = &ChatTriggerContext::invite;
        creators["leave"] = &ChatTriggerContext::leave;
        creators["join"] = &ChatTriggerContext::join;
        creators["lfg"] = &ChatTriggerContext::lfg;  // AC ChatTriggerContext.h:98
        creators["give leader"] = &ChatTriggerContext::give_leader;

        // #188 (S6 of epic #163): loot/gear chat commands.
        creators["repair"] = &ChatTriggerContext::repair;
        creators["ll"] = &ChatTriggerContext::ll;
        creators["equip"] = &ChatTriggerContext::equip;
        creators["unequip"] = &ChatTriggerContext::unequip;
        // The chat command stays "roll" as in AC; it maps to the "dice roll" action
        // (ChatCommandHandlerStrategy) because the Monk ability owns the action name "roll".
        creators["roll"] = &ChatTriggerContext::roll;
        creators["equip upgrade"] = &ChatTriggerContext::equip_upgrade;

        // #193 (S7 of epic #163, AC ChatTriggerContext.h:58-59,162): "t"/"nt" both trigger the
        // "trade" action (ChatCommandHandlerStrategy.cpp), and "unlock traded item" is its own
        // trigger/action pair (same name on both sides, like "repair"/"ll"/"equip").
        creators["t"] = &ChatTriggerContext::trade;
        creators["nt"] = &ChatTriggerContext::nontrade;
        creators["unlock traded item"] = &ChatTriggerContext::unlock_traded_item;

        // #194 (S8, AC ChatTriggerContext.h:152-153): "pet"/"pet attack" chat commands. Distinct from
        // the combat ActionContext's own "pet attack" -> PetAttackAction registration (ActionContext.h);
        // AC keeps the same split between its chat and combat action contexts.
        creators["pet"] = &ChatTriggerContext::pet;
        creators["pet attack"] = &ChatTriggerContext::pet_attack;

        // #210 S10 G4 (AC ChatCommandHandlerStrategy.cpp:73): "ready" runs the "ready check" action
        // directly (ChatCommandHandlerStrategy.cpp's InitTriggers wires the trigger -> action name).
        creators["ready"] = &ChatTriggerContext::ready;
        creators["rebuff"] = &ChatTriggerContext::rebuff;  // AC ChatTriggerContext.h:125 (#628)
        creators["disperse"] = &ChatTriggerContext::disperse;  // #246 AC ChatTriggerContext.h:145
        // #379 (AC ChatTriggerContext.h:72-74): the only way a pull is requested (PullRequestAction).
        creators["pull"] = &ChatTriggerContext::pull;
        creators["pull back"] = &ChatTriggerContext::pull_back;
        creators["pull rti"] = &ChatTriggerContext::pull_rti;
        creators["debug"] = &ChatTriggerContext::debug;    // #611 AC ChatTriggerContext.h:126
        creators["cdebug"] = &ChatTriggerContext::cdebug;  // #611 AC ChatTriggerContext.h:127

        // #258 registration (AC ChatTriggerContext.h:55-56,65,83-85,107,120-121). Every one of
        // these AC chat commands is what makes the matching ChatActionContext name reachable.
        // "gbank" deliberately shares AC's gb() factory, which names the trigger "gb" - that is
        // AC's own shape (ChatTriggerContext.h:83-84 -> :209), mirrored verbatim.
        creators["s"] = &ChatTriggerContext::sell;
        creators["b"] = &ChatTriggerContext::buy;
        creators["r"] = &ChatTriggerContext::reward;  // #258 (AC ChatTriggerContext.h; "hire" stays unregistered as in AC :130)
        creators["trainer"] = &ChatTriggerContext::trainer;
        creators["gb"] = &ChatTriggerContext::gb;
        creators["gbank"] = &ChatTriggerContext::gb;
        // AC ChatTriggerContext.h:137-141: guild management chat commands.
        creators["ginvite"] = &ChatTriggerContext::ginvite;
        creators["guild promote"] = &ChatTriggerContext::guild_promote;
        creators["guild demote"] = &ChatTriggerContext::guild_demote;
        creators["guild remove"] = &ChatTriggerContext::guild_remove;
        creators["guild leave"] = &ChatTriggerContext::guild_leave;
        creators["bank"] = &ChatTriggerContext::bank;
        creators["talk"] = &ChatTriggerContext::talk;
        creators["sendmail"] = &ChatTriggerContext::sendmail;
        creators["mail"] = &ChatTriggerContext::mail;
        // AC Ai/Base/ChatTriggerContext.h:156. "focus heal" fires TriggerNode("focus heal") in
        // ChatCommandHandlerStrategy.cpp:56, which was dangling until now - nothing registered it, so the
        // node never fired and the "focus heal targets" chat command was unreachable. The action half
        // already existed (ActionContext.h:216). Factory shape matches AC ChatTriggerContext.h:288 1:1.
        creators["focus heal"] = &ChatTriggerContext::focus_heal;

        // #363 follow-up: AC Ai/Base/ChatTriggerContext.h:113-119. These 7 chat commands are the ONLY
        // entry point AC gives the Druid "cancel <form>" actions - there is no TriggerNode for them
        // anywhere in AC's strategy graph (verified across the whole AC module), so without these the
        // action classes ported in PR #370 are unreachable. Factory shape matches AC :178-184 1:1.
        creators["cancel tree form"] = &ChatTriggerContext::cancel_tree_form;
        creators["cancel travel form"] = &ChatTriggerContext::cancel_travel_form;
        creators["cancel bear form"] = &ChatTriggerContext::cancel_bear_form;
        creators["cancel dire bear form"] = &ChatTriggerContext::cancel_dire_bear_form;
        creators["cancel cat form"] = &ChatTriggerContext::cancel_cat_form;
        creators["cancel moonkin form"] = &ChatTriggerContext::cancel_moonkin_form;
        creators["cancel aquatic form"] = &ChatTriggerContext::cancel_aquatic_form;
        // #536: AC ChatTriggerContext.h:31-32
        creators["rpg status"] = &ChatTriggerContext::rpg_status;
        creators["taxi"] = &ChatTriggerContext::taxi;   // #767 (AC ChatTriggerContext.h:43)
        creators["rpg do quest"] = &ChatTriggerContext::rpg_do_quest;
        // #146: quest chat commands (AC ChatTriggerContext.h:23,35-36). The ChatActionContext half
        // ("quests"/"drop"/"share"/"query quest") was registered by #257 slice H, but no trigger
        // reached it, so the commands were dead.
        creators["quests"] = &ChatTriggerContext::quests;
        creators["drop"] = &ChatTriggerContext::drop;
        creators["share"] = &ChatTriggerContext::share;
        creators["q"] = &ChatTriggerContext::q;
        creators["rtsc"] = &ChatTriggerContext::rtsc;  // #648 (AC ChatTriggerContext.h:142)

        // AC ChatTriggerContext.h:18-19,23-25,30-31,36-39,45-48,50-53,56,58-59,62,70-71,92,97-99,124,127,152,154-155:
        // chat commands whose action already exists on this fork, plus the commands ported together with their actions
        // (stats, rep/reputation, c/items/inventory/inv, go, target, attackers, qi). Trigger names are the ones AC's
        // factories give them, except "e": AC registers its factory under "equip", so its "e" strategy node can never
        // fire. Here "e" and "ue" are real triggers; the full words "equip"/"unequip" stay as well.
        creators["open items"] = &ChatTriggerContext::open_items;
        creators["unlock items"] = &ChatTriggerContext::unlock_items;
        creators["stats"] = &ChatTriggerContext::stats;
        creators["rep"] = &ChatTriggerContext::reputation;
        creators["reputation"] = &ChatTriggerContext::reputation;
        creators["add all loot"] = &ChatTriggerContext::loot_all;
        creators["release"] = &ChatTriggerContext::release;
        creators["u"] = &ChatTriggerContext::use;
        creators["use"] = &ChatTriggerContext::use;
        creators["c"] = &ChatTriggerContext::item_count;
        creators["items"] = &ChatTriggerContext::item_count;
        creators["inventory"] = &ChatTriggerContext::item_count;
        creators["inv"] = &ChatTriggerContext::item_count;
        creators["e"] = &ChatTriggerContext::equip_short;
        creators["ue"] = &ChatTriggerContext::unequip_short;
        creators["spells"] = &ChatTriggerContext::spells;
        creators["accept"] = &ChatTriggerContext::accept;
        creators["home"] = &ChatTriggerContext::home;
        creators["emote"] = &ChatTriggerContext::emote;
        creators["enter vehicle"] = &ChatTriggerContext::enter_vehicle;
        creators["leave vehicle"] = &ChatTriggerContext::leave_vehicle;
        creators["cast"] = &ChatTriggerContext::cast;
        creators["castnc"] = &ChatTriggerContext::castnc;
        creators["revive"] = &ChatTriggerContext::revive;
        creators["warning"] = &ChatTriggerContext::warning;
        creators["go"] = &ChatTriggerContext::go;
        creators["attackers"] = &ChatTriggerContext::attackers;
        creators["target"] = &ChatTriggerContext::target;
        creators["drink"] = &ChatTriggerContext::drink;
        creators["qi"] = &ChatTriggerContext::qi;
        creators["wait for attack time"] = &ChatTriggerContext::wait_for_attack_time;
        // AC ChatTriggerContext.h:67-70.
        creators["maintenance"] = &ChatTriggerContext::maintenance;
        creators["remove glyph"] = &ChatTriggerContext::remove_glyph;
        creators["autogear"] = &ChatTriggerContext::autogear;
        creators["autogear bis"] = &ChatTriggerContext::autogear_bis;
        // AC ChatTriggerContext.h:28,42,75,79,81,129: pvp stats, teleport, chat, destroy, buff, wts. The strategy lists
        // "tell pvp stats" as a supported name (AC :92), so that name gets a trigger as well.
        creators["pvp stats"] = &ChatTriggerContext::pvp_stats;
        creators["tell pvp stats"] = &ChatTriggerContext::tell_pvp_stats;
        creators["teleport"] = &ChatTriggerContext::teleport;
        creators["chat"] = &ChatTriggerContext::chat;
        creators["destroy"] = &ChatTriggerContext::destroy;
        creators["buff"] = &ChatTriggerContext::buff;
        creators["wts"] = &ChatTriggerContext::wts;
    }

private:
    static Trigger* who(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "who"); }
    static Trigger* talents(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "talents"); }
    static Trigger* taxi(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "taxi"); }   // #767 (AC :244)
    static Trigger* rpg_status(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "rpg status"); }  // #536, AC :258
    static Trigger* rpg_do_quest(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "rpg do quest"); }  // #536, AC :259
    static Trigger* quests(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "quests"); }  // #146, AC :251
    static Trigger* drop(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "drop"); }  // #146, AC :249
    static Trigger* share(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "share"); }  // #146, AC :250
    static Trigger* q(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "q"); }  // #146, AC :246
    static Trigger* help(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "help"); }
    static Trigger* co(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "co"); }
    static Trigger* nc(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "nc"); }
    static Trigger* dead(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "de"); }
    static Trigger* rti(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "rti"); }
    static Trigger* follow(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "follow"); }
    static Trigger* stay(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "stay"); }
    static Trigger* move_from_group(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "move from group"); }

    static Trigger* flee(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "flee"); }
    static Trigger* runaway(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "runaway"); }
    static Trigger* grind(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "grind"); }
    static Trigger* tank_attack(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "tank attack"); }
    static Trigger* max_dps(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "max dps"); }
    static Trigger* attack(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "attack"); }
    static Trigger* save_mana(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "save mana"); }
    static Trigger* formation(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "formation"); }
    static Trigger* range(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "range"); }
    static Trigger* stance(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "stance"); }
    static Trigger* position(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "position"); }
    static Trigger* summon(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "summon"); }
    static Trigger* reset_ai(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "reset botAI"); }
    static Trigger* invite(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "invite"); }
    static Trigger* leave(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "leave"); }
    static Trigger* join(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "join"); }
    static Trigger* lfg(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "lfg"); }  // AC ChatTriggerContext.h:197
    static Trigger* give_leader(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "give leader"); }

    static Trigger* repair(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "repair"); }
    static Trigger* ll(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "ll"); }
    static Trigger* equip(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "equip"); }
    static Trigger* unequip(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "unequip"); }
    static Trigger* roll(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "roll"); }
    static Trigger* equip_upgrade(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "equip upgrade"); }

    static Trigger* trade(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "t"); }
    static Trigger* nontrade(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "nt"); }
    static Trigger* unlock_traded_item(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "unlock traded item"); }

    static Trigger* pet(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "pet"); }
    static Trigger* pet_attack(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "pet attack"); }

    static Trigger* ready(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "ready"); }
    static Trigger* rebuff(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "rebuff"); }  // AC :266
    static Trigger* disperse(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "disperse"); }  // #246 AC :277
    static Trigger* pull(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "pull"); }  // #379 AC :219
    static Trigger* pull_back(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "pull back"); }  // #379 AC :220
    static Trigger* pull_rti(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "pull rti"); }  // #379 AC :221
    static Trigger* debug(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "debug"); }    // AC :170
    static Trigger* cdebug(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cdebug"); }  // AC :171

    static Trigger* sell(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "s"); }
    static Trigger* buy(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "b"); }
    static Trigger* reward(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "r"); }
    static Trigger* trainer(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "trainer"); }
    static Trigger* gb(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "gb"); }
    // AC ChatTriggerContext.h:269-273
    static Trigger* ginvite(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "ginvite"); }
    static Trigger* guild_promote(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "guild promote"); }
    static Trigger* guild_demote(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "guild demote"); }
    static Trigger* guild_remove(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "guild remove"); }
    static Trigger* guild_leave(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "guild leave"); }
    static Trigger* bank(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "bank"); }
    static Trigger* talk(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "talk"); }
    static Trigger* sendmail(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "sendmail"); }
    static Trigger* mail(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "mail"); }

    // AC Ai/Base/ChatTriggerContext.h:288, 1:1. See the creators[] entry above.
    static Trigger* focus_heal(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "focus heal"); }

    // #363 follow-up: AC Ai/Base/ChatTriggerContext.h:178-184, 1:1.
    static Trigger* cancel_tree_form(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cancel tree form"); }
    static Trigger* cancel_travel_form(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cancel travel form"); }
    static Trigger* cancel_bear_form(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cancel bear form"); }
    static Trigger* cancel_dire_bear_form(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cancel dire bear form"); }
    static Trigger* cancel_cat_form(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cancel cat form"); }
    static Trigger* cancel_moonkin_form(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cancel moonkin form"); }
    static Trigger* cancel_aquatic_form(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cancel aquatic form"); }
    static Trigger* rtsc(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "rtsc"); }  // #648

    // AC Ai/Base/ChatTriggerContext.h:160-161,172,185-186,193-194,198-199,201-202,213,215-216,231-234,240-241,252,
    // 254,261-262,275,279,287: factories for the commands registered above.
    static Trigger* open_items(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "open items"); }
    static Trigger* unlock_items(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "unlock items"); }
    static Trigger* stats(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "stats"); }
    static Trigger* reputation(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "reputation"); }
    static Trigger* loot_all(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "add all loot"); }
    static Trigger* release(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "release"); }
    static Trigger* use(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "use"); }
    static Trigger* item_count(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "c"); }
    static Trigger* equip_short(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "e"); }
    static Trigger* unequip_short(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "ue"); }
    static Trigger* spells(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "spells"); }
    static Trigger* accept(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "accept"); }
    static Trigger* home(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "home"); }
    static Trigger* emote(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "emote"); }
    static Trigger* enter_vehicle(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "enter vehicle"); }
    static Trigger* leave_vehicle(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "leave vehicle"); }
    static Trigger* cast(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "cast"); }
    static Trigger* castnc(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "castnc"); }
    static Trigger* revive(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "revive"); }
    static Trigger* warning(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "warning"); }
    static Trigger* go(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "go"); }
    static Trigger* attackers(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "attackers"); }
    static Trigger* target(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "target"); }
    static Trigger* drink(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "drink"); }
    static Trigger* qi(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "qi"); }
    static Trigger* wait_for_attack_time(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "wait for attack time"); }
    static Trigger* maintenance(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "maintenance"); }
    static Trigger* remove_glyph(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "remove glyph"); }
    static Trigger* autogear(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "autogear"); }
    static Trigger* autogear_bis(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "autogear bis"); }
    static Trigger* pvp_stats(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "pvp stats"); }
    static Trigger* tell_pvp_stats(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "tell pvp stats"); }
    static Trigger* teleport(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "teleport"); }
    static Trigger* chat(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "chat"); }
    static Trigger* destroy(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "destroy"); }
    static Trigger* buff(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "buff"); }
    static Trigger* wts(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "wts"); }
};

#endif
