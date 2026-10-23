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
    }

private:
    static Trigger* who(PlayerbotAI* botAI) { return new ChatCommandTrigger(botAI, "who"); }
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
};

#endif
