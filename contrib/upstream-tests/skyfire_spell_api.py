"""Explicit API translations for SF spell source; no core/header stubs."""
import re
from sql_batch import comma_values

def calls(text,name,fn):
 changes=[]
 for m in re.finditer(r'\b'+re.escape(name)+r'\s*\(',text):
  start=m.end();depth=1;quote=None;i=start
  while depth and i<len(text):
   c=text[i]
   if quote:
    if c=='\\':i+=1
    elif c==quote:quote=None
   elif c in '\"\'':quote=c
   elif c=='(':depth+=1
   elif c==')':depth-=1
   i+=1
  assert depth==0,name
  result=fn(comma_values(text[start:i-1]))
  if result is not None:changes.append((m.start(),i,result))
 for a,b,s in reversed(changes):text=text[:a]+s+text[b:]
 return text

def adapt(mod,s):
 for old,new in {'getPowerType':'GetPowerType','getThreatManager':'GetThreatManager','learnSpell':'LearnSpell','removeSpell':'RemoveSpell','isArena':'IsArena','isRated':'IsRated','isHonorOrXPTarget':'IsHonorOrXPTarget','setPowerType':'SetPowerType','_IsValidAttackTarget':'IsValidAttackTarget','GetComboTarget':'GetComboTargetGUID'}.items():s=s.replace('->'+old+'(', '->'+new+'(')
 s=s.replace('SF_LOG_ERROR','TC_LOG_ERROR').replace('->baseattacktime','->BaseAttackTime')
 for old,new in {'SPELL_FAILED_AURA_BOUNCED':'SPELL_FAILED_STRONGER_SPELL_IN_PROGRESS','SPELL_AURA_MECHANIC_IMMUNITY_MASK':'SPELL_AURA_APPLY_CREATURE_IMMUNITIES','SPELL_AURA_IGNORE_SPELL_COOLDOWN':'SPELL_AURA_ALLOW_CAST_WHILE_IN_COOLDOWN','SPELL_PREVENTION_TYPE_UNK':'(SPELL_PREVENTION_TYPE_SILENCE | SPELL_PREVENTION_TYPE_PACIFY)','MAX_TOTEM_SLOT':'SUMMON_SLOT_MAX_TOTEM'}.items():s=s.replace(old,new)
 s=s.replace('.GetSpell()', '.TriggeredBySpell()');s=re.sub(r'void SetGUID\(uint64 (\w+),',r'void SetGUID(ObjectGuid \1,',s)
 s=s.replace('EnterEvadeMode() override','EnterEvadeMode(EvadeReason /*why*/ = EVADE_REASON_OTHER) override')
 for name,index in [('ObjectAccessor::GetUnit',1),('ObjectAccessor::GetCreature',1),('ObjectAccessor::GetPlayer',1),('ObjectAccessor::FindPlayer',0)]:
  def guid(args):
   args[index]='ObjectGuid('+args[index]+')';return name+'('+', '.join(args)+')'
  s=calls(s,name,guid)
 def aura(args):
  if len(args) in (5,6) and args[0].startswith('SPELL_AURA_'):return 'GetAuraEffect('+', '.join(args[:2]+['flag128('+','.join(args[2:5])+',0)']+args[5:])+')'
 s=calls(s,'GetAuraEffect',aura)
 for name in ('SpellHealingBonusDone','SpellHealingBonusTaken','SpellDamageBonusDone'):
  def bonus(args):
   if len(args) not in (4,5):return None
   index='EFFECT_0'
   if mod=='druid' and args[2]=='healAmount' and len(args)==5:index='aurEff->GetEffIndex()'
   if mod=='priest' and args[2] in ('aurEff->GetAmount()','heal'):index='aurEff->GetEffIndex()'
   if mod=='monk' and name=='SpellDamageBonusDone':index='auraEffect->GetEffIndex()'
   if mod=='warlock' and name=='SpellDamageBonusDone':index='dot->GetEffIndex()'
   args.insert(2,index);return name+'('+', '.join(args)+')'
  s=calls(s,name,bonus)
 s=re.sub(r'(\b\w+|GetCaster\(\)|GetTarget\(\)|GetUnitOwner\(\))->GetPosition\(\s*&(\w+)\s*\)',r'\2 = \1->GetPosition()',s)
 s=re.sub(r'(\b\w+)->GetRandomNearPosition\((\w+),\s*(.*?)\);',r'\2 = \1->GetRandomNearPosition(\3);',s)
 s=re.sub(r'(\b\w+)->GetFirstCollisionPosition\((\w+),\s*([^,]+),\s*([^;]+)\);',r'\2 = \1->GetFirstCollisionPosition(\3, \4);',s)
 def summon(args):
  if len(args)>=4 and 'TEMPSUMMON' in args[2]:args[3]='Milliseconds('+args[3]+')'
  elif len(args)>=7 and 'TEMPSUMMON' in args[5]:args[6]='Milliseconds('+args[6]+')'
  else:return None
  return 'SummonCreature('+', '.join(args)+')'
 s=calls(s,'SummonCreature',summon)
 for old,new in {'spellId':'SpellId','powerType':'PowerType','manaCost':'Cost','ManaCostPercentageFloat':'CostPercentage'}.items():s=s.replace('->'+old,'->'+new)
 if mod=='hunter':
  s=s.replace('new Pet(player, PetType::HUNTER_PET);','new Pet(player, PetType::HUNTER_PET);\n                    pet->SetTemporary();')
  s=s.replace('LoadPetFromDB(player, 0, petNumber, false, -1, true, &spawnPos)','LoadPetFromDB(PET_LOAD_BY_ID, petNumber, &spawnPos)').replace('LoadPetFromDB(player, 0, 0, false, slot, true, &spawnPos)','LoadPetFromDB(PET_LOAD_BY_SLOT, slot, &spawnPos)')
  s=s.replace('currentPet->GetSlot()','player->GetCurrentPetSlot()').replace('PET_SAVE_FIRST_ACTIVE_SLOT','PET_SLOT_ACTIVE_FIRST').replace('PET_SAVE_LAST_ACTIVE_SLOT','PET_SLOT_ACTIVE_LAST')
 if mod=='dk':
  s=s.replace('player->GetRuneCooldown(i) != player->GetRuneBaseCooldown(i)', '!(player->GetLastUsedRuneMask() & (1 << i))')
  s=re.sub(r'(\b\w+)->GetRuneCooldown\(([^()]*)\)',r'(RUNE_READINESS_FULL - \1->GetRune(\2).Readiness)',s)
  s=re.sub(r'(\b\w+)->SetRuneCooldown\((.*?), 0\)',r'\1->ActivateRune(\2)',s)
  s=s.replace('player->AddRuneByAuraEffect(i, RuneType::RUNE_DEATH, aurEff)','player->AddRuneBySpell(i, RuneType::RUNE_DEATH, aurEff->GetId())').replace('player->RemoveRunesByAuraEffect(aurEff)','player->RemoveRunesBySpell(aurEff->GetId())')
 if mod=='rogue':
  s=s.replace('UNIT_FIELD_SHAPESHIFT_FORM','UNIT_FIELD_BYTES_1')
  s=re.sub(r'(\b\w+|GetCaster\(\)|GetTarget\(\))->HasDirtyTricks\(\)',r'SF27HasDirtyTricks(\1)',s)
  s=re.sub(r'(\b\w+|GetCaster\(\)|GetTarget\(\))->IsPoisonOrBleedSpell\(',r'SF27IsPoisonOrBleedSpell(',s)
  s=re.sub(r'(?<!\w)IsPoisonOrBleedSpell\(',r'SF27IsPoisonOrBleedSpell(',s)
  i=s.index('namespace SkyFire27_rogue\n{')+len('namespace SkyFire27_rogue\n{')
  helper='''
inline bool SF27HasDirtyTricks(Unit const* unit)
{
 return unit && (unit->HasAura(108216) || (unit->ToPlayer() && unit->ToPlayer()->HasSpell(108216)));
}
inline bool SF27IsPoisonOrBleedSpell(SpellInfo const* spell)
{
 if (!spell) return false;
 if (spell->Dispel == DISPEL_POISON || (spell->GetAllEffectsMechanicMask() & (1u << MECHANIC_BLEED))) return true;
 switch (spell->Id) { case 703: case 1943: case 2818: case 8680: case 79136: case 89775: case 113780: case 121411: case 122233: return true; default: return false; }
}
'''
  s=s[:i]+helper+s[i:]
 return adapt_more(mod,s)

def adapt_more(mod,s):
 for old,new in {'modifyThreatPercent':'ModifyThreatByPercent','getThreat':'GetThreat','resetAllAggro':'ResetAllThreat'}.items():s=s.replace('.'+old+'(','.'+new+'(')
 for var in set(re.findall(r'SpellModifier\s*[*&\s]+(\w+)',s)):s=s.replace(var+'->SpellId',var+'->spellId')
 s=s.replace('->RefreshTimers()','->RefreshTimers(false)')
 s=calls(s,'IsImmunedToSpell',lambda a:'IsImmunedToSpell('+a[0]+', 0xFFFFFFFFu)' if len(a)==1 else None)
 s=calls(s,'HashMapHolder<Creature>::Find',lambda a:'HashMapHolder<Creature>::Find(ObjectGuid('+a[0]+'))')
 s=calls(s,'Aura::TryCreate',lambda a:'Aura::TryCreate('+', '.join(a[:-1]+['ObjectGuid('+a[-1]+')'])+')' if len(a)==7 else None)
 s=re.sub(r'(\b\w+\(\))->GetPosition\(\s*&(\w+)\s*\)',r'\2 = \1->GetPosition()',s)
 s=re.sub(r'(\b\w+\(\))->GetFirstCollisionPosition\((\w+),\s*(.*?),\s*([^;]+)\);',r'\2 = \1->GetFirstCollisionPosition(\3, \4);',s)
 # Upstream's epoch-second deadline APIs must become relative millisecond durations.
 def cooldown(a):
  if len(a)!=3:return None
  m=re.fullmatch(r'time\(NULL\)\s*\+\s*(.*)',a[2]);assert m,a
  value=m[1]
  if mod=='mage' and value=='cooldown':a[2]=value
  elif re.search(r'/\s*IN_MILLISECONDS$',value):a[2]=re.sub(r'/\s*IN_MILLISECONDS$','',value).strip()
  else:a[2]='('+value+') * IN_MILLISECONDS'
  return 'AddSpellCooldown('+', '.join(a)+')'
 s=calls(s,'AddSpellCooldown',cooldown)
 s=s.replace('->SendSpellCooldown(', '->GetSpellHistory()->SendCooldown(')
 s=s.replace('->GetChargeCooldown(', '->GetSpellHistory()->GetChargeCooldown(')
 if mod=='rogue':
  # Upstream logs every ordinary Blade Flurry hit as ERROR; this is a proc trace.
  s=s.replace('TC_LOG_ERROR("misc", "damage: %u procSpell: %u",','TC_LOG_DEBUG("spells.rogue", "Blade Flurry damage: %u procSpell: %u",')
  s=s.replace('Unit::SF27IsPoisonOrBleedSpell(', 'SF27IsPoisonOrBleedSpell(')
  s=s.replace('caster->SF27IsPoisonOrBleedSpell(', 'SF27IsPoisonOrBleedSpell(').replace('target->SF27IsPoisonOrBleedSpell(', 'SF27IsPoisonOrBleedSpell(')
 if mod=='druid':
  # The event is owned by this mushroom's EventProcessor and dies with it.
  s=s.replace('explicit DelayedWildMushroomInvisEvent(uint64 mushroomGuid) : _mushroomGuid(mushroomGuid) { }','explicit DelayedWildMushroomInvisEvent(Creature* mushroom) : _mushroom(mushroom) { }')
  s=s.replace('Creature* mushroom = HashMapHolder<Creature>::Find(ObjectGuid(_mushroomGuid))','Creature* mushroom = _mushroom').replace('uint64 _mushroomGuid;', 'Creature* _mushroom;').replace('new DelayedWildMushroomInvisEvent(mushroom->GetGUID())','new DelayedWildMushroomInvisEvent(mushroom)')
  s=s.replace('std::min(maxDuration + 2 * IN_MILLISECONDS, maxPossibleDuration)','std::min<int32>(maxDuration + 2 * IN_MILLISECONDS, maxPossibleDuration)').replace('std::min(rip->GetDuration() + 2 * IN_MILLISECONDS, newMaxDuration)','std::min<int32>(rip->GetDuration() + 2 * IN_MILLISECONDS, newMaxDuration)')
  s=s.replace('summon->GetTimer()', 'const_cast<TempSummon*>(summon)->GetTimer()')
 if mod=='paladin':
  s=s.replace('std::max(previousDuration, 5 * IN_MILLISECONDS * holyPower)','std::max<int32>(previousDuration, 5 * IN_MILLISECONDS * holyPower)')
  start=s.index('            Unit::AuraList& auras = GetUnitOwner()->GetSingleCastAuras();');end=s.index('            return NULL;',start)
  s=s[:start]+'''            if (auto const* auras = GetUnitOwner()->GetBoundAurasBySpellId(SPELL_PALADIN_BEACON_OF_LIGHT_MARKER))
                for (Aura* aura : *auras)
                    if (!aura->IsRemoved())
                        return aura;
'''+s[end:]
 if mod=='mage':
  start=s.index('            const SpellCooldowns& cm = caster->GetSpellCooldownMap();');end=s.index('\n        }\n\n        void Register()',start)
  s=s[:start]+'''            for (uint32 spellId : {122u, 120u, 45438u})
                caster->RemoveSpellCooldown(spellId, true);'''+s[end:]
  s=s.replace('SendCooldown(otherSpell, cooldown * IN_MILLISECONDS)','SendCooldown(otherSpell, cooldown)')
 if mod=='warlock':
  s=s.replace('RemovePet(NULL, PET_SAVE_NOT_IN_SLOT)','RemovePet(PET_REMOVE_DISMISS)')
  s=s.replace('LoadPetFromDB(player, 0, petNumber, false)','LoadPetFromDB(PET_LOAD_BY_ID, petNumber)')
  s=s.replace('GetChargeCooldown(darkSouls[i])','GetChargeCooldown(darkSouls[i]).ToMilliseconds()')
  s=s.replace('AddCustomCooldown(darkSouls[i], cooldown)','AddCustomCooldown(darkSouls[i], TimeValue(Milliseconds(cooldown)))')
 if mod=='monk':
  s=re.sub(r'uint64 GetGUID\(int32 /\*id\*/\) const override\s*\{\s*return m_victimGuid;\s*\}', 'ObjectGuid GetGUID(int32 /*id*/) const override\n    {\n        return ObjectGuid(m_victimGuid);\n    }',s)
 s=s.replace('#include "Player.h"','#include "Player.h"\n#include "SpellHistory.h"')
 return adapt_extensions(mod,s)


def adapt_extensions(mod,s):
 s=s.replace('->getStandState(', '->GetStandState(').replace('UNIT_FIELD_FLAGS2','UNIT_FIELD_FLAGS_2')
 s=calls(s,'KilledMonsterCredit',lambda a:'KilledMonsterCredit('+', '.join([a[0],'ObjectGuid::Empty']+a[2:])+')' if len(a)>1 and a[1]=='0' else None)
 s=calls(s,'GetOwnedAura',lambda a:'GetOwnedAura('+', '.join('ObjectGuid::Empty' if i in (1,2) and x=='0' else x for i,x in enumerate(a))+')')
 if mod=='quest':
  s=s.replace('->MonsterWhisper(SAY_1, target, true)', '->Whisper(SAY_1, LANG_UNIVERSAL, target, true)').replace('->MonsterWhisper(SAY_2, target, true)', '->Whisper(SAY_2, LANG_UNIVERSAL, target, true)').replace('->IsEventPhase(1)','->GetPhase() == 1')
  s=re.sub(r'(\b\w+)->GetNearPosition\((\w+),\s*([^,]+),\s*([^;]+)\);',r'\2 = \1->GetNearPosition(\3, \4);',s)
  s=s.replace('ai->GetScript()->SetEventPhase(1);', '''{
            SmartScriptHolder action;
            action.action.type = SMART_ACTION_SET_EVENT_PHASE;
            action.action.setEventPhase.phase = 1;
            ai->GetScript()->ProcessAction(action);
        }''')
  s=s.replace('GetCaster()->GetNearPosition(pos, dist, angle);','pos = GetCaster()->GetNearPosition(dist, angle);')
 if mod=='item':
  s=calls(s,'SummonGameObject',lambda a:'SummonGameObject('+', '.join(a[:5]+['G3D::Quat('+', '.join(a[5:9])+')',a[9]])+')' if len(a)==10 else None)
 if mod=='generic':
  s=s.replace('Player* player = GetCaster()->ToPlayer();\n                if (player->GetLastPetNumber())','Player* player = GetCaster()->ToPlayer();\n                // Local Pet::LoadPetFromDB emits 6962 after attaching its minion.\n                // Ignore that notification; only a missing pet needs resummoning.\n                if (player->GetPet())\n                    return;\n                if (player->GetLastPetNumber())')
  s='#include \"SpellHistory.h\"\n'+s
  s=s.replace('LoadPetFromDB(player, 0, player->GetLastPetNumber(), true)','LoadPetFromDB(PET_LOAD_BY_ID, player->GetLastPetNumber())')
  s=s.replace('RemovePet(NULL, PET_SAVE_NOT_IN_SLOT, true)','RemovePet(PET_REMOVE_DISMISS, PET_REMOVE_FLAG_RETURN_REAGENT)')
  s=s.replace('->AddSpellAndCategoryCooldowns(', '->GetSpellHistory()->StartCooldown(')
 if mod=='mastery':
  i=s.index('namespace SkyFire27_mastery\n{')+len('namespace SkyFire27_mastery\n{')
  s=s[:i]+'''
inline int32 SF27RemainingIgnite(Unit* target, ObjectGuid caster)
{
    if (AuraEffect* effect = target->GetAuraEffect(12654, EFFECT_0, caster))
        if (uint32 ticks = effect->GetTotalTicks())
            return effect->GetAmount() * std::max<int32>(int32(ticks) - int32(effect->GetTickNumber()), 0) / ticks;
    return 0;
}
'''+s[i:]
  s=s.replace('GetRemainingPeriodicAmount(caster->GetGUID(), MASTERY_SPELL_IGNITE, SPELL_AURA_PERIODIC_DAMAGE)', 'SF27RemainingIgnite(target, caster->GetGUID())')
 if mod=='archaeology':
  s=s.replace('uint64 guid = caster->m_ObjectSlot', 'ObjectGuid guid = caster->m_ObjectSlot').replace('m_ObjectSlot[SURVEY_OBJECT_SLOT] = 0','m_ObjectSlot[SURVEY_OBJECT_SLOT] = ObjectGuid::Empty')
  s=s.replace('sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT), entry, map,\n        x, y, z, orientation, 0.0f, 0.0f, 0.0f, 0.0f, 0, GOState::GO_STATE_READY', 'map->GenerateLowGuid<HighGuid::GameObject>(), entry, map, player->GetPhaseMask(),\n        x, y, z, orientation, G3D::Quat(0.0f, 0.0f, std::sin(orientation / 2), std::cos(orientation / 2)), 0, GOState::GO_STATE_READY')
  s=s.replace('SendSurveyCastInfo','SendSurveryCastInfo').replace('DEFAULT_WORLD_OBJECT_SIZE','player->GetObjectSize()')
  s=s.replace('    map->AddToMap(go);\n    return go;', '    if (!map->AddToMap(go))\n    {\n        delete go;\n        return nullptr;\n    }\n    return go;')
  s=s.replace('                    go->SetSpellId(0);\n                }\n            }\n            else', """                    go->SetSpellId(0);
                    // This fork advances discoveries on survey, not in the loot handler.
                    // Advance only after a find was successfully added to the map.
                    digsite->SelectNewArchaeologyFind(false);
                    if (!digsite->IsEmptyDigsite())
                        player->SaveResearchDigsiteToDB(digsite);
                    player->SendSurveryCastInfo(digsite, true);
                    player->UpdateResearchDigsites();
                    return; // UpdateResearchDigsites can invalidate digsite.
                }
                player->SendSurveryCastInfo(digsite, false);
                return;
            }
            else""")
 if mod=='mastery':s=s.replace('target->SF27RemainingIgnite(', 'SF27RemainingIgnite(')

 if mod=='warrior':
  s=s.replace('spell_warr_second_wind_proc_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_DUMMY','spell_warr_second_wind_proc_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY')
  s=s.replace('spell_warr_second_wind_proc_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_DUMMY','spell_warr_second_wind_proc_AuraScript::HandleProc, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY')
  s=s.replace('spell_warr_sudden_death_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_DUMMY','spell_warr_sudden_death_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER')
 if mod=='generic':
  start=s.index('                if (m_scriptSpellId == SPELL_NIGHTMARE_FIGMENT_MIRROR_IMAGE)');end=s.index('\n            }',start)
  s=s[:start]+"""                SpellEffects expected = m_scriptSpellId == SPELL_NIGHTMARE_FIGMENT_MIRROR_IMAGE ? SPELL_EFFECT_DUMMY : SPELL_EFFECT_SCRIPT_EFFECT;
                if (SpellInfo const* info = sSpellMgr->GetSpellInfo(m_scriptSpellId))
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        if (info->Effects[i].Effect == expected)
                            OnEffectHitTarget += SpellEffectFn(spell_gen_clone_SpellScript::HandleScriptEffect, SpellEffIndex(i), expected);"""+s[end:]
 if mod=='quest':
  old="""                OnEffectHit += SpellEffectFn(spell_darkshore_corpse_soothe_SpellScript::RelocateSummonDest, EFFECT_0, SPELL_EFFECT_SUMMON);
                OnEffectHit += SpellEffectFn(spell_darkshore_corpse_soothe_SpellScript::RelocateSummonDest, EFFECT_1, SPELL_EFFECT_SUMMON);
                OnEffectHit += SpellEffectFn(spell_darkshore_corpse_soothe_SpellScript::HandleQuestCredit, EFFECT_0, SPELL_EFFECT_SUMMON);
                OnEffectHit += SpellEffectFn(spell_darkshore_corpse_soothe_SpellScript::HandleQuestCredit, EFFECT_1, SPELL_EFFECT_SUMMON);"""
  new="""                if (SpellInfo const* info = sSpellMgr->GetSpellInfo(m_scriptSpellId))
                    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    {
                        if (info->Effects[i].Effect == SPELL_EFFECT_SUMMON)
                        {
                            OnEffectHit += SpellEffectFn(spell_darkshore_corpse_soothe_SpellScript::RelocateSummonDest, SpellEffIndex(i), SPELL_EFFECT_SUMMON);
                            OnEffectHit += SpellEffectFn(spell_darkshore_corpse_soothe_SpellScript::HandleQuestCredit, SpellEffIndex(i), SPELL_EFFECT_SUMMON);
                        }
                        // The local client carries the final withered-ent credit as DUMMY.
                        if (m_scriptSpellId == SPELL_WITHERED_ENT_DUMMY && info->Effects[i].Effect == SPELL_EFFECT_DUMMY)
                            OnEffectHit += SpellEffectFn(spell_darkshore_corpse_soothe_SpellScript::HandleQuestCredit, SpellEffIndex(i), SPELL_EFFECT_DUMMY);
                    }"""
  assert old in s;s=s.replace(old,new)
 return s
