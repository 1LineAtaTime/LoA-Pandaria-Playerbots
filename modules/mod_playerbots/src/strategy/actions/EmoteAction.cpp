/*
 * Ported from AC mod-playerbots Ai/Base/Actions/EmoteAction.cpp (issue #194, S8). The previous pass
 * scoped ReceiveEmote() down to a representative subset of AC's reaction table; this pass completes it
 * to AC's full ~150-case table (EmoteAction.cpp:145-618 there). See
 * the 5.4.8 packet layouts, which
 * are unchanged from the previous pass.
 *
 * Deviations from AC, all noted at the call site below:
 *  - The bot's own outgoing text emote now goes through the free helper PlayBotTextEmote() (declared
 *    in EmoteAction.h), which builds a real 5.4.8 CMSG_TEXT_EMOTE packet and calls
 *    bot->GetSession()->HandleTextEmoteOpcode(), matching what AC's EmoteActionBase::Emote() does
 *    (fake an inbound CMSG_TEXT_EMOTE) but with the 5.4.8 bit-packed guid layout instead of AC's plain
 *    3.3.5 one. This replaces the previous pass's "HandleEmoteCommand + Say()" simplification.
 *  - AC's GetNumberOfEmoteVariants() is a hand-rolled table covering only the eight WotLK-era races
 *    (Human/Dwarf/NightElf/Undead/Troll/Gnome/Orc/Tauren) and has no entries for Blood Elf/Draenei/
 *    Worgen/Goblin/Pandaren. Rather than guess variant counts for races AC never had, PlayBotTextEmote()
 *    always sends variant 0 - see its own comment.
 *  - "receive text emote"/"receive emote" (and the "often"->"talk"/"seldom"->"emote" idle-chatter
 *    triggers that share the same AC gate) are wired in EmoteStrategy.cpp behind a local read of
 *    AiPlayerbot.RandomBotEmote (AC default false), not added to PlayerbotAIConfig.h.
 *  - EmoteAction::Execute() only handles the reactive (packet-driven) path. AC's own "seldom"->"emote"
 *    idle-chatter case (no incoming packet - AC EmoteAction.cpp:733-777, picking a random target via
 *    GetTarget() and a random plain animation from its `emotes` map) is NOT ported: Execute() below
 *    still returns false for a non-packet Event, same as the previous pass. AiPlayerbot.RandomBotEmote
 *    defaults to false, so this is inert by default; flagged in the S8 report as not done.
 */

#include "EmoteAction.h"

#include "Event.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Playerbots.h"
#include "BotArc.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "WorldPacket.h"

void PlayBotTextEmote(Player* bot, uint32 textEmoteId, Unit* target)
{
    if (!bot->IsAlive())
        return;

    ObjectGuid targetGuid = target ? target->GetGUID() : ObjectGuid::Empty;

    // 5.4.8 CMSG_TEXT_EMOTE layout (research s8-pet-control-and-class-spells.md Part B.9; core
    // Handlers/ChatHandler.cpp:889-911 read order - text_emote, emoteNum, then the bit-packed guid).
    WorldPacket data(CMSG_TEXT_EMOTE, 4 + 4 + 9);
    data << uint32(textEmoteId);
    // 5.4.8: emoteNum only picks which pre-written race/gender text variant the client displays; the
    // server never validates it (see the file comment - AC's GetNumberOfEmoteVariants() has no data for
    // half of 5.4.8's races). Variant 0 is always a valid, in-bounds index for every emote line.
    data << uint32(0);

    data.WriteBit(targetGuid[6]);
    data.WriteBit(targetGuid[7]);
    data.WriteBit(targetGuid[3]);
    data.WriteBit(targetGuid[2]);
    data.WriteBit(targetGuid[0]);
    data.WriteBit(targetGuid[5]);
    data.WriteBit(targetGuid[1]);
    data.WriteBit(targetGuid[4]);
    data.FlushBits();

    data.WriteByteSeq(targetGuid[0]);
    data.WriteByteSeq(targetGuid[5]);
    data.WriteByteSeq(targetGuid[1]);
    data.WriteByteSeq(targetGuid[4]);
    data.WriteByteSeq(targetGuid[2]);
    data.WriteByteSeq(targetGuid[3]);
    data.WriteByteSeq(targetGuid[7]);
    data.WriteByteSeq(targetGuid[6]);

    bot->GetSession()->HandleTextEmoteOpcode(data);
}

// AC parity, full table (EmoteAction.cpp:145-618). Case labels and the emoteId/textEmote/emoteText/
// emoteYell assignments are transcribed verbatim; every TEXT_EMOTE_*/EMOTE_ONESHOT_* name below was
// checked against this fork's SharedDefines.h and compiles unchanged on 5.4.8 (research Part B.9: the
// TextEmotes/Emotes enums only grew new ids, none of AC's existing ones moved or were removed). AC's own
// commented-out cases (retail-only ids its WotLK enum lacks) are kept as comments for the same reason
// AC has them: 5.4.8 does define most of them now, but adding them would be new behaviour beyond AC's
// spec, not a port of it.
bool EmoteAction::ReceiveEmote(Player* source, uint32 emote, bool verbal)
{
    uint32 emoteId = 0;
    uint32 textEmote = 0;
    std::string emoteText;
    std::string emoteYell;

    switch (emote)
    {
        case TEXT_EMOTE_BONK:
            emoteId = EMOTE_ONESHOT_CRY;
            textEmote = TEXT_EMOTE_CRY;
            break;
        case TEXT_EMOTE_SALUTE:
            emoteId = EMOTE_ONESHOT_SALUTE;
            textEmote = TEXT_EMOTE_SALUTE;
            break;
        case 325:
            if (botAI->GetMaster() == source)
            {
                botAI->ChangeStrategy("-follow,+stay", BOT_STATE_NON_COMBAT);
                botAI->TellMasterNoFacing("Fine.. I'll stay right here..");
            }
            break;
        case TEXT_EMOTE_BECKON:
        case 324:
            if (botAI->GetMaster() == source)
            {
                botAI->ChangeStrategy("+follow", BOT_STATE_NON_COMBAT);
                botAI->TellMasterNoFacing("Wherever you go, I'll follow..");
            }
            break;
        case TEXT_EMOTE_WAVE:
        case TEXT_EMOTE_GREET:
        case TEXT_EMOTE_HAIL:
        case TEXT_EMOTE_HELLO:
        case TEXT_EMOTE_WELCOME:
        case TEXT_EMOTE_INTRODUCE:
            emoteText = "Hey there!";
            emoteId = EMOTE_ONESHOT_WAVE;
            textEmote = TEXT_EMOTE_HELLO;
            break;
        case TEXT_EMOTE_DANCE:
            emoteText = "Shake what your mama gave you!";
            emoteId = EMOTE_ONESHOT_DANCE;
            textEmote = TEXT_EMOTE_DANCE;
            break;
        case TEXT_EMOTE_FLIRT:
        case TEXT_EMOTE_KISS:
        case TEXT_EMOTE_HUG:
        case TEXT_EMOTE_BLUSH:
        case TEXT_EMOTE_SMILE:
        case TEXT_EMOTE_LOVE:
            // case TEXT_EMOTE_HOLDHAND:
            emoteText = "Awwwww...";
            emoteId = EMOTE_ONESHOT_SHY;
            textEmote = TEXT_EMOTE_SHY;
            break;
        case TEXT_EMOTE_FLEX:
            emoteText = "Hercules! Hercules!";
            emoteId = EMOTE_ONESHOT_APPLAUD;
            textEmote = TEXT_EMOTE_APPLAUD;
            break;
        case TEXT_EMOTE_ANGRY:
            // case TEXT_EMOTE_FACEPALM:
        case TEXT_EMOTE_GLARE:
        case TEXT_EMOTE_BLAME:
            // case TEXT_EMOTE_FAIL:
            // case TEXT_EMOTE_REGRET:
            // case TEXT_EMOTE_SCOLD:
            // case TEXT_EMOTE_CROSSARMS:
            emoteText = "Did I do thaaaaat?";
            emoteId = EMOTE_ONESHOT_QUESTION;
            textEmote = TEXT_EMOTE_SHRUG;
            break;
        case TEXT_EMOTE_FART:
        case TEXT_EMOTE_BURP:
        case TEXT_EMOTE_GASP:
        case TEXT_EMOTE_NOSEPICK:
        case TEXT_EMOTE_SNIFF:
        case TEXT_EMOTE_STINK:
            emoteText = "Wasn't me! Just sayin'..";
            emoteId = EMOTE_ONESHOT_POINT;
            textEmote = TEXT_EMOTE_POINT;
            break;
        case TEXT_EMOTE_JOKE:
            emoteId = EMOTE_ONESHOT_LAUGH;
            textEmote = TEXT_EMOTE_LAUGH;
            emoteText = "Oh.. was I not supposed to laugh so soon?";
            break;
        case TEXT_EMOTE_CHICKEN:
            emoteText = "We'll see who's chicken soon enough!";
            emoteId = EMOTE_ONESHOT_RUDE;
            textEmote = TEXT_EMOTE_RUDE;
            break;
        case TEXT_EMOTE_APOLOGIZE:
            emoteId = EMOTE_ONESHOT_POINT;
            textEmote = TEXT_EMOTE_APOLOGIZE;
            emoteText = "You damn right you're sorry!";
            break;
        case TEXT_EMOTE_APPLAUD:
        case TEXT_EMOTE_CLAP:
        case TEXT_EMOTE_CONGRATULATE:
        case TEXT_EMOTE_HAPPY:
            // case TEXT_EMOTE_GOLFCLAP:
            emoteId = EMOTE_ONESHOT_BOW;
            textEmote = TEXT_EMOTE_BOW;
            emoteText = "Thank you.. Thank you.. I'm here all week.";
            break;
        case TEXT_EMOTE_BEG:
        case TEXT_EMOTE_GROVEL:
        case TEXT_EMOTE_PLEAD:
            emoteId = EMOTE_ONESHOT_NO;
            textEmote = TEXT_EMOTE_NO;
            emoteText = "Beg all you want.. I have nothing for you.";
            break;
        case TEXT_EMOTE_BITE:
        case TEXT_EMOTE_POKE:
        case TEXT_EMOTE_SCRATCH:
            // case TEXT_EMOTE_PINCH:
            // case TEXT_EMOTE_PUNCH:
            emoteId = EMOTE_ONESHOT_ROAR;
            textEmote = TEXT_EMOTE_ROAR;
            emoteYell = "OUCH! Dammit, that hurt!";
            break;
        case TEXT_EMOTE_BORED:
            emoteId = EMOTE_ONESHOT_NO;
            textEmote = TEXT_EMOTE_NO;
            emoteText = "My job description doesn't include entertaining you..";
            break;
        case TEXT_EMOTE_BOW:
        case TEXT_EMOTE_CURTSEY:
            emoteId = EMOTE_ONESHOT_BOW;
            textEmote = TEXT_EMOTE_BOW;
            break;
        case TEXT_EMOTE_BRB:
        case TEXT_EMOTE_SIT:
            emoteId = EMOTE_ONESHOT_EAT;
            textEmote = TEXT_EMOTE_EAT;
            emoteText = "Looks like time for an AFK break..";
            break;
        case TEXT_EMOTE_AGREE:
        case TEXT_EMOTE_NOD:
            emoteId = EMOTE_ONESHOT_EXCLAMATION;
            textEmote = TEXT_EMOTE_NOD;
            emoteText = "At least SOMEONE agrees with me!";
            break;
        case TEXT_EMOTE_AMAZE:
        case TEXT_EMOTE_COWER:
        case TEXT_EMOTE_CRINGE:
        case TEXT_EMOTE_EYE:
        case TEXT_EMOTE_KNEEL:
        case TEXT_EMOTE_PEER:
        case TEXT_EMOTE_SURRENDER:
        case TEXT_EMOTE_PRAISE:
        case TEXT_EMOTE_SCARED:
        case TEXT_EMOTE_COMMEND:
            // case TEXT_EMOTE_AWE:
            // case TEXT_EMOTE_JEALOUS:
            // case TEXT_EMOTE_PROUD:
            emoteId = EMOTE_ONESHOT_FLEX;
            textEmote = TEXT_EMOTE_FLEX;
            emoteText = "Yes, Yes. I know I'm amazing..";
            break;
        case TEXT_EMOTE_BLEED:
        case TEXT_EMOTE_MOURN:
        case TEXT_EMOTE_FLOP:
            // case TEXT_EMOTE_FAINT:
            // case TEXT_EMOTE_PULSE:
            emoteId = EMOTE_ONESHOT_KNEEL;
            textEmote = TEXT_EMOTE_KNEEL;
            emoteText = "MEDIC! Stat!";
            break;
        case TEXT_EMOTE_BLINK:
            emoteId = EMOTE_ONESHOT_KICK;
            emoteText = "What? You got something in your eye?";
            break;
        case TEXT_EMOTE_BOUNCE:
        case TEXT_EMOTE_BARK:
            emoteId = EMOTE_ONESHOT_POINT;
            textEmote = TEXT_EMOTE_POINT;
            emoteText = "Who's a good doggy? You're a good doggy!";
            break;
        case TEXT_EMOTE_BYE:
            emoteId = EMOTE_ONESHOT_WAVE;
            textEmote = TEXT_EMOTE_WAVE;
            emoteText = "Umm.... wait! Where are you going?!";
            break;
        case TEXT_EMOTE_CACKLE:
        case TEXT_EMOTE_LAUGH:
        case TEXT_EMOTE_CHUCKLE:
        case TEXT_EMOTE_GIGGLE:
        case TEXT_EMOTE_GUFFAW:
        case TEXT_EMOTE_ROFL:
        case TEXT_EMOTE_SNICKER:
            // case TEXT_EMOTE_SNORT:
            emoteId = EMOTE_ONESHOT_LAUGH;
            textEmote = TEXT_EMOTE_LAUGH;
            emoteText = "Wait... what are we laughing at again?";
            break;
        case TEXT_EMOTE_CONFUSED:
        case TEXT_EMOTE_CURIOUS:
        case TEXT_EMOTE_FIDGET:
        case TEXT_EMOTE_FROWN:
        case TEXT_EMOTE_SHRUG:
        case TEXT_EMOTE_SIGH:
        case TEXT_EMOTE_STARE:
        case TEXT_EMOTE_TAP:
        case TEXT_EMOTE_SURPRISED:
        case TEXT_EMOTE_WHINE:
        case TEXT_EMOTE_BOGGLE:
        case TEXT_EMOTE_LOST:
        case TEXT_EMOTE_PONDER:
        case TEXT_EMOTE_SNUB:
        case TEXT_EMOTE_SERIOUS:
        case TEXT_EMOTE_EYEBROW:
            emoteId = EMOTE_ONESHOT_QUESTION;
            textEmote = TEXT_EMOTE_SHRUG;
            emoteText = "Don't look at  me.. I just work here";
            break;
        case TEXT_EMOTE_COUGH:
        case TEXT_EMOTE_DROOL:
        case TEXT_EMOTE_SPIT:
        case TEXT_EMOTE_LICK:
        case TEXT_EMOTE_BREATH:
            // case TEXT_EMOTE_SNEEZE:
            // case TEXT_EMOTE_SWEAT:
            emoteId = EMOTE_ONESHOT_POINT;
            textEmote = TEXT_EMOTE_POINT;
            emoteText = "Ewww! Keep your nasty germs over there!";
            break;
        case TEXT_EMOTE_CRY:
            emoteId = EMOTE_ONESHOT_CRY;
            textEmote = TEXT_EMOTE_CRY;
            emoteText = "Don't you start crying or it'll make me start crying!";
            break;
        case TEXT_EMOTE_CRACK:
            emoteId = EMOTE_ONESHOT_ROAR;
            textEmote = TEXT_EMOTE_ROAR;
            emoteText = "It's clobbering time!";
            break;
        case TEXT_EMOTE_EAT:
        case TEXT_EMOTE_DRINK:
            emoteId = EMOTE_ONESHOT_EAT;
            textEmote = TEXT_EMOTE_EAT;
            emoteText = "I hope you brought enough for the whole class...";
            break;
        case TEXT_EMOTE_GLOAT:
        case TEXT_EMOTE_MOCK:
        case TEXT_EMOTE_TEASE:
        case TEXT_EMOTE_EMBARRASS:
            emoteId = EMOTE_ONESHOT_CRY;
            textEmote = TEXT_EMOTE_CRY;
            emoteText = "Doesn't mean you need to be an ass about it..";
            break;
        case TEXT_EMOTE_HUNGRY:
            emoteId = EMOTE_ONESHOT_EAT;
            textEmote = TEXT_EMOTE_EAT;
            emoteText = "What? You want some of this?";
            break;
        case TEXT_EMOTE_LAYDOWN:
        case TEXT_EMOTE_TIRED:
        case TEXT_EMOTE_YAWN:
            emoteId = EMOTE_ONESHOT_KNEEL;
            textEmote = TEXT_EMOTE_KNEEL;
            emoteText = "Is it break time already?";
            break;
        case TEXT_EMOTE_MOAN:
        case TEXT_EMOTE_MOON:
        case TEXT_EMOTE_SEXY:
        case TEXT_EMOTE_SHAKE:
        case TEXT_EMOTE_WHISTLE:
        case TEXT_EMOTE_CUDDLE:
        case TEXT_EMOTE_PURR:
        case TEXT_EMOTE_SHIMMY:
        case TEXT_EMOTE_SMIRK:
        case TEXT_EMOTE_WINK:
            // case TEXT_EMOTE_CHARM:
            emoteId = EMOTE_ONESHOT_NO;
            textEmote = TEXT_EMOTE_NO;
            emoteText = "Keep it in your pants, boss..";
            break;
        case TEXT_EMOTE_NO:
        case TEXT_EMOTE_VETO:
        case TEXT_EMOTE_DISAGREE:
        case TEXT_EMOTE_DOUBT:
            emoteId = EMOTE_ONESHOT_QUESTION;
            textEmote = TEXT_EMOTE_SHRUG;
            emoteText = "Aww.... why not?!";
            break;
        case TEXT_EMOTE_PANIC:
            emoteId = EMOTE_ONESHOT_EXCLAMATION;
            textEmote = TEXT_EMOTE_CALM;
            emoteText = "Now is NOT the time to panic!";
            break;
        case TEXT_EMOTE_POINT:
            emoteId = EMOTE_ONESHOT_POINT;
            textEmote = TEXT_EMOTE_POINT;
            emoteText = "What?! I can do that TOO!";
            break;
        case TEXT_EMOTE_RUDE:
        case TEXT_EMOTE_RASP:
            emoteId = EMOTE_ONESHOT_RUDE;
            textEmote = TEXT_EMOTE_RASP;
            emoteText = "Right back at you, bub!";
            break;
        case TEXT_EMOTE_ROAR:
        case TEXT_EMOTE_THREATEN:
        case TEXT_EMOTE_CALM:
        case TEXT_EMOTE_DUCK:
        case TEXT_EMOTE_TAUNT:
        case TEXT_EMOTE_PITY:
        case TEXT_EMOTE_GROWL:
            // case TEXT_EMOTE_TRAIN:
            // case TEXT_EMOTE_INCOMING:
            // case TEXT_EMOTE_CHARGE:
            // case TEXT_EMOTE_FLEE:
            // case TEXT_EMOTE_ATTACKMYTARGET:
        case TEXT_EMOTE_OPENFIRE:
        case TEXT_EMOTE_ENCOURAGE:
        case TEXT_EMOTE_ENEMY:
            // case TEXT_EMOTE_CHALLENGE:
            // case TEXT_EMOTE_REVENGE:
            // case TEXT_EMOTE_SHAKEFIST:
            emoteId = EMOTE_ONESHOT_ROAR;
            textEmote = TEXT_EMOTE_ROAR;
            emoteYell = "RAWR!";
            break;
        case TEXT_EMOTE_TALK:
            emoteId = EMOTE_ONESHOT_TALK;
            textEmote = TEXT_EMOTE_LISTEN;
            break;
        case TEXT_EMOTE_TALKEX:
            emoteId = EMOTE_ONESHOT_YES;
            textEmote = TEXT_EMOTE_AGREE;
            break;
        case TEXT_EMOTE_TALKQ:
        case TEXT_EMOTE_LISTEN:
            emoteId = EMOTE_ONESHOT_TALK;
            textEmote = TEXT_EMOTE_TALKQ;
            emoteText = "Blah Blah Blah Yakety Smackety..";
            break;
        case TEXT_EMOTE_THANK:
            emoteId = EMOTE_ONESHOT_BOW;
            textEmote = TEXT_EMOTE_BOW;
            emoteText = "You are quite welcome!";
            break;
        case TEXT_EMOTE_VICTORY:
        case TEXT_EMOTE_CHEER:
        case TEXT_EMOTE_TOAST:
            // case TEXT_EMOTE_HIGHFIVE:
            // case TEXT_EMOTE_DING:
            emoteId = EMOTE_ONESHOT_CHEER;
            textEmote = TEXT_EMOTE_CHEER;
            emoteText = "Yay!";
            break;
        case TEXT_EMOTE_COLD:
        case TEXT_EMOTE_SHIVER:
        case TEXT_EMOTE_THIRSTY:
            // case TEXT_EMOTE_OOM:
            // case TEXT_EMOTE_HEALME:
            // case TEXT_EMOTE_POUT:
            emoteId = EMOTE_ONESHOT_QUESTION;
            textEmote = TEXT_EMOTE_PUZZLE;
            emoteText = "And what exactly am I supposed to do about that?";
            break;
        case TEXT_EMOTE_COMFORT:
        case TEXT_EMOTE_SOOTHE:
        case TEXT_EMOTE_PAT:
            emoteId = EMOTE_ONESHOT_CRY;
            textEmote = TEXT_EMOTE_CRY;
            emoteText = "Thanks...";
            break;
        case TEXT_EMOTE_INSULT:
            emoteId = EMOTE_ONESHOT_CRY;
            textEmote = TEXT_EMOTE_CRY;
            emoteText = "You hurt my feelings..";
            break;
        case TEXT_EMOTE_JK:
            emoteId = EMOTE_ONESHOT_POINT;
            textEmote = TEXT_EMOTE_POINT;
            emoteText = "You.....";
            break;
        case TEXT_EMOTE_RAISE:
            emoteId = EMOTE_ONESHOT_POINT;
            textEmote = TEXT_EMOTE_POINT;
            emoteText = "Yes.. you.. at the back of the class..";
            break;
        case TEXT_EMOTE_READY:
            emoteId = EMOTE_ONESHOT_SALUTE;
            textEmote = TEXT_EMOTE_SALUTE;
            emoteText = "Ready here, too!";
            break;
        case TEXT_EMOTE_SHOO:
            emoteId = EMOTE_ONESHOT_KICK;
            textEmote = TEXT_EMOTE_SHOO;
            emoteText = "Shoo yourself!";
            break;
        case TEXT_EMOTE_SLAP:
            // case TEXT_EMOTE_SMACK:
            emoteId = EMOTE_ONESHOT_CRY;
            textEmote = TEXT_EMOTE_CRY;
            emoteText = "What did I do to deserve that?";
            break;
        case TEXT_EMOTE_STAND:
            emoteId = EMOTE_ONESHOT_NONE;
            textEmote = TEXT_EMOTE_STAND;
            emoteText = "What? Break time's over? Fine..";
            break;
        case TEXT_EMOTE_TICKLE:
            emoteId = EMOTE_ONESHOT_LAUGH;
            textEmote = TEXT_EMOTE_GIGGLE;
            emoteText = "Hey! Stop that!";
            break;
        case TEXT_EMOTE_VIOLIN:
            emoteId = EMOTE_ONESHOT_TALK;
            textEmote = TEXT_EMOTE_SIGH;
            emoteText = "Har Har.. very funny..";
            break;
            // case TEXT_EMOTE_HELPME:
            //     bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
            //     bot->Yell("Quick! Someone HELP!", LANG_UNIVERSAL);
            //     break;
        case TEXT_EMOTE_GOODLUCK:
            // case TEXT_EMOTE_LUCK:
            emoteId = EMOTE_ONESHOT_TALK;
            textEmote = TEXT_EMOTE_THANK;
            emoteText = "Thanks... I'll need it..";
            break;
        case TEXT_EMOTE_BRANDISH:
            // case TEXT_EMOTE_MERCY:
            emoteId = EMOTE_ONESHOT_BEG;
            textEmote = TEXT_EMOTE_BEG;
            emoteText = "Please don't kill me!";
            break;
        default:
            break;
    }

    if (source && !bot->isMoving() && !BotHasInArcAC(bot, static_cast<float>(M_PI), source, sPlayerbotAIConfig->farDistance))
        sServerFacade->SetFacingTo(bot, source);

    if (verbal)
    {
        if (!emoteText.empty())
            bot->Say(emoteText, bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH);

        if (!emoteYell.empty())
            bot->Yell(emoteYell, bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH);
    }

    if (textEmote)
        PlayBotTextEmote(bot, textEmote, (source && urand(0, 1)) ? source : nullptr);
    else if (emoteId)
        bot->HandleEmoteCommand(emoteId);

    return true;
}

bool EmoteAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    if (p.empty())
        return false;

    Player* source = nullptr;
    uint32 emote = 0;

    if (p.GetOpcode() == SMSG_TEXT_EMOTE)
    {
        // 5.4.8 layout (research s8-pet-control-and-class-spells.md B.9, ChatHandler.cpp:808-858):
        // bit-packed sender (P) and target (T) guids around the two uint32 payload fields, no name
        // string (unlike 3.3.5). "aimed at me by name" becomes "T == bot's guid".
        p.rpos(0);
        ObjectGuid P, T;
        P[1] = p.ReadBit();
        T[7] = p.ReadBit();
        P[6] = p.ReadBit();
        T[5] = p.ReadBit();
        P[3] = p.ReadBit();
        T[6] = p.ReadBit();
        T[2] = p.ReadBit();
        P[7] = p.ReadBit();
        T[0] = p.ReadBit();
        T[1] = p.ReadBit();
        P[4] = p.ReadBit();
        P[2] = p.ReadBit();
        T[3] = p.ReadBit();
        T[4] = p.ReadBit();
        P[0] = p.ReadBit();
        P[5] = p.ReadBit();

        p.ReadByteSeq(T[2]);
        p.ReadByteSeq(T[1]);
        p.ReadByteSeq(P[7]);
        p.ReadByteSeq(P[4]);
        p.ReadByteSeq(T[7]);
        p.ReadByteSeq(P[5]);
        p.ReadByteSeq(P[2]);

        uint32 textEmote = 0;
        p >> textEmote;

        p.ReadByteSeq(P[6]);
        p.ReadByteSeq(T[0]);
        p.ReadByteSeq(P[3]);
        p.ReadByteSeq(P[1]);
        p.ReadByteSeq(T[6]);
        p.ReadByteSeq(P[0]);
        p.ReadByteSeq(T[3]);
        p.ReadByteSeq(T[5]);
        p.ReadByteSeq(T[4]);

        uint32 emoteNum = 0;
        p >> emoteNum;

        source = ObjectAccessor::FindPlayer(P);
        if (source && source->GetGUID() != bot->GetGUID() &&
            ((urand(0, 1) && BotHasInArcAC(bot, static_cast<float>(M_PI), source, 10.0f)) || T == bot->GetGUID()))
            emote = textEmote;
    }
    else if (p.GetOpcode() == SMSG_EMOTE)
    {
        // Unchanged from the previous pass (research B.9): plain uint32 emoteId + uint64 sourceGuid.
        // SMSG_EMOTE only carries an animation id, not a text-emote id, so this fork reacts to it
        // generically (AC's anim -> TextEmotes reverse lookup via sEmotesTextSoundStore is not ported).
        p.rpos(0);
        uint32 emoteId = 0;
        ObjectGuid sourceGuid;
        p >> emoteId >> sourceGuid;

        source = ObjectAccessor::FindPlayer(sourceGuid);
        if (source && source->GetGUID() != bot->GetGUID() && emoteId != EMOTE_ONESHOT_NONE &&
            (source->GetTarget() == bot->GetGUID() ||
             (urand(0, 1) && BotHasInArcAC(bot, static_cast<float>(M_PI), source, 10.0f))))
        {
            // #194 (S8): a real text emote (anim + "Hello!" text bubble) instead of the previous
            // pass's animation-only wave.
            PlayBotTextEmote(bot, TEXT_EMOTE_WAVE, source);
            return true;
        }
        return false;
    }
    else
        return false;

    if (!source || !emote)
        return false;

    // AC parity (EmoteAction.cpp:744): never say/yell the flavor line in a battleground.
    return ReceiveEmote(source, emote, bot->InBattleground() ? false : (urand(0, 1) != 0));
}
