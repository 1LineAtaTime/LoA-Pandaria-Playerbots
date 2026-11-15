/* Adapted from ProjectSkyfire/SkyFire_548 release sf_db_27 (f7a9880).
 * GPL-3.0-or-later; original project authors retain attribution.
 * Original encounter42163b72, completion follow-updc61c594.
 * This header is included only by zone_gilneas.cpp.
 */
#ifndef LOA_GILNEAS_PRISON_EVENT_H
#define LOA_GILNEAS_PRISON_EVENT_H
namespace SkyfirePrisonEvent
{
enum BySkinOfHisTeeth
{
    NPC_LORD_DARIUS_CROWLEY_C1     = 35077,
    NPC_SEAN_DEMPSEY               = 35081,
    NPC_WORGEN_ALPHA_C1            = 35170,
    NPC_WORGEN_ALPHA_C2            = 35167,
    NPC_WORGEN_RUNT_C1             = 35188,
    NPC_WORGEN_RUNT_C2             = 35456,

    QUEST_BY_THE_SKIN_OF_HIS_TEETH = 14154,

    SPELL_BSOHT_BUFF               = 66914,
    SPELL_BSOHT_ENRAGE             = 8599,
    SPELL_BSOHT_LEFT_HOOK          = 67825,
    SPELL_BSOHT_DEMORALIZING_SHOUT = 61044,
    SPELL_BSOHT_SNAP_KICK          = 67827,

    EVENT_BSOHT_DURATION           = 118500,
    EVENT_BSOHT_SPAWN_TTL          = 20000,
    EVENT_BSOHT_ENRAGE_CD          = 30000,
};

struct BsohtPoint
{
    float x, y, z;
};

float const BsohtPlatformZ = 52.29f;

BsohtPoint const BsohtNWWaypointLoc1[2] =
{
    { -1630.62f, 1480.55f, 70.40f },
    { -1636.01f, 1475.81f, 64.51f }
};

BsohtPoint const BsohtNWWaypointLoc2[2] =
{
    { -1637.26f, 1488.86f, 69.95f },
    { -1642.45f, 1482.23f, 64.30f }
};

BsohtPoint const BsohtSWWaypointLoc1[2] =
{
    { -1718.31f, 1526.62f, 55.91f },
    { -1717.86f, 1490.77f, 56.61f }
};

BsohtPoint const BsohtSWWaypointLoc2[2] =
{
    { -1718.31f, 1526.62f, 55.91f },
    { -1717.86f, 1487.00f, 57.07f }
};

BsohtPoint const BsohtNWaypointLoc[1] =
{
    { -1593.38f, 1408.02f, 72.64f }
};

uint32 BsohtRand(uint32 min, uint32 max)
{
    return min + (uint32(rand32()) % (max - min + 1));
}

int32 BsohtIRand(int32 min, int32 max)
{
    return min + int32(uint32(rand32()) % uint32(max - min + 1));
}

/*######
## npc_sean_dempsey - controls the worgen wave-spawning for the duration of the event
######*/

class npc_sean_dempsey : public CreatureScript
{
public:
    npc_sean_dempsey() : CreatureScript("npc_sean_dempsey") { }

    struct npc_sean_dempseyAI : public ScriptedAI
    {
        npc_sean_dempseyAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 _spawnTimer;
        uint32 _eventTimer;
        uint32 _waveInterval;
        ObjectGuid _playerGUID;
        bool _eventActive;
        bool _firstWave;

        void Reset() override
        {
            _eventActive = false;
            _firstWave = true;
            _spawnTimer = 0;
            _eventTimer = 0;
            _playerGUID.Clear();
            _waveInterval = BsohtRand(9000, 15000);
        }

        void StartEvent(ObjectGuid playerGUID)
        {
            if (_eventActive)
                return; // Do not reset an encounter already in progress.
            _firstWave = true;
            _spawnTimer = 0;
            _eventActive = true;
            _eventTimer = EVENT_BSOHT_DURATION;
            _playerGUID = playerGUID;
        }

        void SummonNextWave()
        {
            if (_firstWave)
            {
                me->SummonCreature(NPC_WORGEN_ALPHA_C2, -1732.81f, 1526.34f, 55.39f, 0.01f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                me->SummonCreature(NPC_WORGEN_ALPHA_C1, -1618.86f, 1505.68f, 70.24f, 3.91f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                _firstWave = false;
                return;
            }

            switch (BsohtRand(1, 5))
            {
                case 1: // Alphas on the NW and SW roofs
                    me->SummonCreature(NPC_WORGEN_ALPHA_C2, -1732.81f, 1526.34f, 55.39f, 0.01f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                    me->SummonCreature(NPC_WORGEN_ALPHA_C1, -1618.86f, 1505.68f, 70.24f, 3.91f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                    break;
                case 2: // 8 runts on the NW roof
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        me->SummonCreature(NPC_WORGEN_RUNT_C1, -1611.40f, 1498.49f, 69.82f, 3.79f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                        me->SummonCreature(NPC_WORGEN_RUNT_C1, -1618.86f, 1505.68f, 70.24f, 3.91f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                    }
                    break;
                case 3: // 8 runts on the SW roof
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        me->SummonCreature(NPC_WORGEN_RUNT_C2, -1732.81f, 1526.34f, 55.39f, 0.01f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                        me->SummonCreature(NPC_WORGEN_RUNT_C2, -1737.49f, 1526.11f, 55.51f, 0.01f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                    }
                    break;
                case 4: // Alphas on the N and SW roofs
                    me->SummonCreature(NPC_WORGEN_ALPHA_C2, -1732.81f, 1526.34f, 55.39f, 0.01f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                    me->SummonCreature(NPC_WORGEN_ALPHA_C1, -1562.59f, 1409.35f, 71.66f, 3.16f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                    break;
                case 5: // 8 runts split across the NW and SW roofs
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        me->SummonCreature(NPC_WORGEN_RUNT_C2, -1732.81f, 1526.34f, 55.39f, 0.01f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                        me->SummonCreature(NPC_WORGEN_RUNT_C1, -1618.86f, 1505.68f, 70.24f, 3.91f, TempSummonType::TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, Milliseconds(EVENT_BSOHT_SPAWN_TTL));
                    }
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!_eventActive)
                return;

            if (_eventTimer <= diff)
            {
                _eventActive = false;
                if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                    if (player->IsAlive() && player->GetDistance(me) < 100.0f && player->GetQuestStatus(QUEST_BY_THE_SKIN_OF_HIS_TEETH) == QUEST_STATUS_INCOMPLETE)
                        player->AreaExploredOrEventHappens(QUEST_BY_THE_SKIN_OF_HIS_TEETH);
                return;
            }
            _eventTimer -= diff;

            if (_spawnTimer <= diff)
            {
                SummonNextWave();
                _spawnTimer = _waveInterval;
            }
            else
                _spawnTimer -= diff;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sean_dempseyAI(creature);
    }
};

/*######
## npc_lord_darius_crowley_c1 - starts the event on quest accept, fights the worgen
######*/

class npc_lord_darius_crowley_c1 : public CreatureScript
{
public:
    npc_lord_darius_crowley_c1() : CreatureScript("npc_lord_darius_crowley_c1") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_BY_THE_SKIN_OF_HIS_TEETH)
            return true;

        creature->CastSpell(player, SPELL_BSOHT_BUFF, true);

        if (Creature* dempsey = creature->FindNearestCreature(NPC_SEAN_DEMPSEY, 100.0f))
            CAST_AI(npc_sean_dempsey::npc_sean_dempseyAI, dempsey->AI())->StartEvent(player->GetGUID());

        return true;
    }

    struct npc_lord_darius_crowley_c1AI : public ScriptedAI
    {
        npc_lord_darius_crowley_c1AI(Creature* creature) : ScriptedAI(creature) { }

        uint32 _attackTimer;

        void Reset() override
        {
            _attackTimer = BsohtRand(1700, 2400);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (_attackTimer <= diff)
            {
                if (me->IsWithinMeleeRange(me->GetVictim()))
                {
                    switch (BsohtRand(0, 2))
                    {
                        case 0:
                            DoCast(me->GetVictim(), SPELL_BSOHT_LEFT_HOOK, true);
                            break;
                        case 1:
                            DoCast(me->GetVictim(), SPELL_BSOHT_DEMORALIZING_SHOUT, true);
                            break;
                        case 2:
                            DoCast(me->GetVictim(), SPELL_BSOHT_SNAP_KICK, true);
                            break;
                    }
                    _attackTimer = BsohtRand(1700, 2400);
                }
                else
                    me->GetMotionMaster()->MoveChase(me->GetVictim());
            }
            else
            {
                _attackTimer -= diff;
                DoMeleeAttackIfReady();
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lord_darius_crowley_c1AI(creature);
    }
};

/*######
## npc_worgen_runt_c1/c2, npc_worgen_alpha_c1/c2 - run rooftop waypoints to a jump-off
## point, land on the platform below, then fight whoever is closest
######*/

struct BsohtRoofFighterAI : public ScriptedAI
{
    BsohtRoofFighterAI(Creature* creature,
        float spawnX1, float spawnY1, BsohtPoint const* waypoints1, uint8 waypointCount1, float jumpX1, float jumpY1,
        float spawnX2, float spawnY2, BsohtPoint const* waypoints2, uint8 waypointCount2, float jumpX2, float jumpY2,
        float jumpSpeedXY, float jumpSpeedZ, float chaseRange)
        : ScriptedAI(creature),
          _spawnX1(spawnX1), _spawnY1(spawnY1), _waypoints1(waypoints1), _waypointCount1(waypointCount1), _jumpX1(jumpX1), _jumpY1(jumpY1),
          _spawnX2(spawnX2), _spawnY2(spawnY2), _waypoints2(waypoints2), _waypointCount2(waypointCount2), _jumpX2(jumpX2), _jumpY2(jumpY2),
          _jumpSpeedXY(jumpSpeedXY), _jumpSpeedZ(jumpSpeedZ), _chaseRange(chaseRange) { }

    void Reset() override
    {
        _running = _jumping = _fighting = false;
        _atLoc1 = _atLoc2 = false;
        _waypointId = 0;
        _issuedWaypointId = uint8(-1);
        _enrageTimer = 0;
        _willEnrage = BsohtRand(0, 1) != 0;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        if (_fighting)
            return;

        me->GetMotionMaster()->Clear();
        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
        _running = false;
        _jumping = false;
        _fighting = true;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_running && !_jumping && !_fighting)
        {
            if (me->GetPositionX() == _spawnX1 && me->GetPositionY() == _spawnY1)
            {
                _atLoc1 = true;
                _running = true;
            }
            else if (_waypoints2 && me->GetPositionX() == _spawnX2 && me->GetPositionY() == _spawnY2)
            {
                _atLoc2 = true;
                _running = true;
            }
        }

        if (_running)
        {
            if (_waypointId == _issuedWaypointId)
                return;

            if (_atLoc1 && _waypointId < _waypointCount1)
                me->GetMotionMaster()->MovePoint(_waypointId, _waypoints1[_waypointId].x, _waypoints1[_waypointId].y, _waypoints1[_waypointId].z, false);
            else if (_atLoc2 && _waypointId < _waypointCount2)
                me->GetMotionMaster()->MovePoint(_waypointId, _waypoints2[_waypointId].x, _waypoints2[_waypointId].y, _waypoints2[_waypointId].z, false);
            _issuedWaypointId = _waypointId;
            return;
        }

        if (_jumping)
            return;

        if (_fighting)
        {
            if (!me->GetVictim())
            {
                if (Player* player = me->SelectNearestPlayer(_chaseRange))
                    AttackStart(player);
                else if (Creature* crowley = me->FindNearestCreature(NPC_LORD_DARIUS_CROWLEY_C1, _chaseRange))
                    AttackStart(crowley);
            }

            if (!UpdateVictim())
                return;

            if (_enrageTimer <= diff)
            {
                if (_willEnrage && me->GetHealthPct() <= 30.0f)
                {
                    DoCast(me, SPELL_BSOHT_ENRAGE);
                    _enrageTimer = EVENT_BSOHT_ENRAGE_CD;
                }
            }
            else
                _enrageTimer -= diff;

            DoMeleeAttackIfReady();
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == EFFECT_MOTION_TYPE && id == EVENT_JUMP)
        {
            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
            _jumping = false;
            _fighting = true;
            return;
        }

        if (type != POINT_MOTION_TYPE)
            return;

        _waypointId = id + 1;

        uint8 count = _atLoc1 ? _waypointCount1 : _waypointCount2;
        if (_waypointId >= count)
        {
            float jumpX = _atLoc1 ? _jumpX1 : _jumpX2;
            float jumpY = _atLoc1 ? _jumpY1 : _jumpY2;
            me->GetMotionMaster()->MoveJump(jumpX + float(BsohtIRand(-3, 3)), jumpY + float(BsohtIRand(-3, 3)), BsohtPlatformZ, _jumpSpeedXY, _jumpSpeedZ);
            _running = false;
            _jumping = true;
        }
    }

private:
    float _spawnX1, _spawnY1;
    BsohtPoint const* _waypoints1;
    uint8 _waypointCount1;
    float _jumpX1, _jumpY1;
    float _spawnX2, _spawnY2;
    BsohtPoint const* _waypoints2;
    uint8 _waypointCount2;
    float _jumpX2, _jumpY2;
    float _jumpSpeedXY, _jumpSpeedZ, _chaseRange;

    bool _running, _jumping, _fighting;
    bool _atLoc1, _atLoc2;
    uint8 _waypointId;
    uint8 _issuedWaypointId;
    uint32 _enrageTimer;
    bool _willEnrage;
};

class npc_worgen_runt_c1 : public CreatureScript
{
public:
    npc_worgen_runt_c1() : CreatureScript("npc_worgen_runt_c1") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BsohtRoofFighterAI(creature,
            -1611.40f, 1498.49f, BsohtNWWaypointLoc1, 2, -1668.52f, 1439.69f,
            -1618.86f, 1505.68f, BsohtNWWaypointLoc2, 2, -1678.04f, 1450.88f,
            20.0f, 22.0f, 40.0f);
    }
};

class npc_worgen_runt_c2 : public CreatureScript
{
public:
    npc_worgen_runt_c2() : CreatureScript("npc_worgen_runt_c2") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BsohtRoofFighterAI(creature,
            -1732.81f, 1526.34f, BsohtSWWaypointLoc1, 2, -1685.521f, 1458.48f,
            -1737.49f, 1526.11f, BsohtSWWaypointLoc2, 2, -1681.81f, 1445.54f,
            20.0f, 22.0f, 50.0f);
    }
};

class npc_worgen_alpha_c1 : public CreatureScript
{
public:
    npc_worgen_alpha_c1() : CreatureScript("npc_worgen_alpha_c1") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BsohtRoofFighterAI(creature,
            -1618.86f, 1505.68f, BsohtNWWaypointLoc1, 2, -1668.52f, 1439.69f,
            -1562.59f, 1409.35f, BsohtNWaypointLoc, 1, -1660.17f, 1429.55f,
            22.0f, 20.0f, 40.0f);
    }
};

class npc_worgen_alpha_c2 : public CreatureScript
{
public:
    npc_worgen_alpha_c2() : CreatureScript("npc_worgen_alpha_c2") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new BsohtRoofFighterAI(creature,
            -1732.81f, 1526.34f, BsohtSWWaypointLoc1, 2, -1685.52f, 1458.48f,
            0.0f, 0.0f, nullptr, 0, 0.0f, 0.0f,
            20.0f, 22.0f, 40.0f);
    }
};


}
#endif
