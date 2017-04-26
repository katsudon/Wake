#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"

/*
 * Local functions.
 */
bool check_shieldmastery	( CHAR_DATA*,CHAR_DATA*,OBJ_DATA* );
void check_defender( CHAR_DATA* );
void check_paired_attack(CHAR_DATA*,CHAR_DATA*,int);
bool check_instinct(CHAR_DATA*);


int calcReflex(CHAR_DATA *ch,CHAR_DATA *otherguy,int chance){
	if (IS_NPC(ch))
		return chance;

	chance += UMAX(get_curr_stat(ch,STAT_AGI)-get_curr_stat(otherguy,STAT_AGI),4)/4;
	chance += UMAX(get_curr_stat(ch,STAT_AGI)-get_curr_stat(otherguy,STAT_AGI),4)/4;
	chance += get_curr_stat(ch,STAT_WIS) / 4;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"reflex(%d,",chance);
	if(ch->isaff(AF_TRANQUILITY))
		chance += (100 - chance) / 3;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d,",chance);
	if(!IS_NPC(ch) && ch->iscomm(CM_MORPH) && ch->morph == MORPH_CANINE)
		chance += chance * 5 * ch->getslvl(gsn_morph_canine) / 100;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d)",chance);
	return UMIN(chance,100);
}

void check_ricochet(CHAR_DATA *victim,CHAR_DATA *ch,OBJ_DATA *wield){
	int chance = get_skill(victim,gsn_ricochet) * .2;
	if (chance < 1 || !wield || (IS_NPC(victim) && !victim->isdef(DF_RICOCHET)))
		return;

	chance += get_curr_stat(victim,STAT_AGI) / 3;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gRicochet({c%d{x)\n\r",chance);
	if (number_percent() < chance){
		act("You bounce your parry, sending your blade at $n!",ch,NULL,victim,TO_VICT);
		act("$N bounces $S parry, sending $S blade at $n!",ch,NULL,victim,TO_NOTVICT);
		act("$N bounces $S parry, sending $S blade at you!",ch,NULL,victim,TO_CHAR);
		damage(victim,ch,get_curr_stat(victim,STAT_AGI) * 3,gsn_ricochet,attack_table[wield->value[3]].damage,true);
		check_improve(victim,gsn_ricochet,true,4);
	}
	else
		check_improve(victim,gsn_ricochet,false,5);
}

bool check_riposte(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt)
{
	int chance,dam_type;
	OBJ_DATA *wield;
	bool parrying = false;

	if ((IS_NPC(victim) && !victim->isdef(DF_RIPOSTE))
	|| (dt == gsn_backstab)
	|| (dt == gsn_charge)
	|| (dt == gsn_assassinate)
	|| (dt != gsn_parry 
	&& ((get_eq_char(victim,WEAR_WIELD) == NULL)
	|| (get_eq_char(ch,WEAR_WIELD) == NULL)
	|| (!IS_AWAKE(victim))
	|| (!can_see(victim,ch))
	|| get_skill(victim,gsn_riposte) < 1)))
		return false;

	wield = get_eq_char(victim,WEAR_WIELD);
	if (wield == NULL)
		return false;

	chance = get_skill(victim,gsn_riposte)*.25;
	chance += (ch->bashwait*10) - (victim->bashwait*10);
	//stats were here
	chance = calcReflex(victim,ch,chance);

	if (dt == gsn_parry)
	{
		parrying = true;
		chance += 10;
	}
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gRiposte({c%d{x)\n\r",chance);
	if ( number_percent() >= chance)
	{
		check_improve(victim,gsn_riposte,false,1);
		return false;
	}

	dt = gsn_riposte;

    if (dt == TYPE_UNDEFINED)
    {
		dt = TYPE_HIT;
		if (wield != NULL && wield->item_type == ITEM_WEAPON)
			dt += wield->value[3];
		else 
            dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
		dam_type = DAM_BASH;

	//stat modifiers for damage go here

	if(parrying == false)
	{
		act("You riposte $n's attack!",ch,NULL,victim,TO_VICT);
		act("$N ripostes your attack!",ch,NULL,victim,TO_CHAR);
		act("$N ripostes $n's attack!",ch,NULL,victim,TO_NOTVICT);
	}
	else
	{
		act("You slip a slip under $n's defenses!",ch,NULL,victim,TO_VICT);
		act("$N slips past your broken defense!",ch,NULL,victim,TO_CHAR);
		act("$N slips an attack in!",ch,NULL,victim,TO_NOTVICT);
	}

    damage(victim,ch,dam,gsn_riposte,dam_type,true);
    check_improve(victim,gsn_riposte,true,1);

    return true;
}

bool check_counter(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,OBJ_DATA *wield,int sn){
	int chance,skill = 0,dam_type;
				if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gCounter{x ");
	if ((IS_NPC(victim) && !victim->isdef(DF_COUNTER))
	||	(dt == gsn_backstab)
	||	(dt == gsn_charge)
	||	(dt == gsn_assassinate)
	||	(!IS_AWAKE(victim))
	||	(!can_see(victim,ch))
	||	(get_skill(victim,gsn_counter) < 1))
		return false;
				if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"a ");
	if ((wield = get_eq_char(ch,WEAR_WIELD))){
		skill = get_skill(victim,get_weapon_sn(ch,false));
		if (skill < 1){
			skill = 0;
				if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"c ");
		}
		else{
			skill *= 2;
				if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"d ");
		}
	}

	chance = get_skill(victim,gsn_counter) / 10;
	chance += victim->getslvl(sn) * 2;
	chance += skill / 10;
	if(victim->bashwait > 0)
		chance /= 2;
	if(ch->bashwait > 0)
		chance *= 1.25;
	//stats were here
	chance = calcReflex(victim,ch,chance);
if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)\n\r",chance);

	if(number_percent() >= chance){
		check_improve(victim,gsn_counter,false,1);
		return false;
	}

	dt = gsn_counter;
    if(dt == TYPE_UNDEFINED){
		dt = TYPE_HIT;
		if (wield && wield->item_type == ITEM_WEAPON)
			dt += wield->value[3];
		else 
            dt += ch->dam_type;
    }
    if(dt < TYPE_HIT)
    	if (wield)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;
    if (dam_type == -1)
		dam_type = DAM_BASH;

    act("You reverse $n's attack and counter with your own!",ch,NULL,victim,TO_VICT);
    act("$N reverses your attack!",ch,NULL,victim,TO_CHAR);
    act("$N counters $n's attack!",ch,NULL,victim,TO_NOTVICT);

    damage(victim,ch,dam/2,gsn_counter,dam_type,true);

	check_paired_attack(victim,ch,dam);
    check_improve(victim,gsn_counter,true,1);

    return true;
}

bool check_parry(CHAR_DATA *ch,CHAR_DATA *victim){
	int chance, dam;
	OBJ_DATA *dual = get_eq_char(victim,WEAR_SECONDARY),*wield = get_eq_char(victim,WEAR_WIELD);

	if (!IS_AWAKE(victim) || (IS_NPC(victim) && !victim->isdef(DF_PARRY)) || get_skill(victim,gsn_parry) < 1)
		return false;

	chance = (get_skill(victim,gsn_parry) *.1) + (ch->bashwait*5) - (victim->bashwait*10) + victim->getslvl(gsn_parry);
	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gParry({c%d{x)",chance);

	if (!wield){
		if (IS_NPC(victim))
			chance /= 2;
		else
			return false;
	}
	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

	if (!can_see(ch,victim))
		chance /= 3;
	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

	if (dual && dual->item_type == ITEM_WEAPON && dual->value[0] == WEAPON_MAINGAUCHE)
		chance *= 1.5;
	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);
	chance = calcReflex(victim,ch,chance);
	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

	if ( number_percent() >= chance){
		check_improve(victim,gsn_parry,false,1);
		return false;
	}
	else{
		if (check_trompement(ch,victim)){
			act("$n deceives you and you fail your parry attempt.",ch,NULL,victim,TO_VICT);
			act("$n deceives $N's attempt to parry.",ch,NULL,victim,TO_NOTVICT);
			act("you deceive $N's attempt to parry.",ch,NULL,victim,TO_CHAR);
			check_improve(victim,gsn_parry,false,1);
			damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
			return false;
		}
		act("You parry $n's attack.",ch,NULL,victim,TO_VICT);
		act("$N parries $n's attack.",ch,NULL,victim,TO_NOTVICT);
		act("$N parries your attack.",ch,NULL,victim,TO_CHAR);
		check_improve(victim,gsn_parry,true,1);

		check_ricochet(victim,ch,wield);

		check_paired_attack(victim,ch,0);
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		return true;
	}
}

bool check_dual_shield(CHAR_DATA *ch,CHAR_DATA *victim){
	OBJ_DATA *shield;
    int chance,dam;

    if (!IS_AWAKE(victim) || (IS_NPC(victim) && !victim->isdef(DF_DUALSHIELD)))
        return false;

    if (!(shield = get_eq_char(victim,WEAR_SECONDARY)) || shield->item_type != ITEM_SHIELD)
        return false;

    chance = get_skill(victim,gsn_shield_block) *.25;
	chance += (ch->bashwait*10) - (victim->bashwait*10);
	chance = calcReflex(victim,ch,chance);

	if (victim->isaff(AF_FORTIFY))
		chance *= 1.5;

	if (get_skill(victim,gsn_dual_shield) > 0)
		chance = chance * get_skill(victim,gsn_dual_shield) / 100;
	else
		return false;

	if (check_shieldmastery(victim,ch,shield))
		chance += get_skill(victim,gsn_shieldmastery) * .2;
    if (number_percent() >= chance + victim->level - ch->level){
		check_improve(victim,gsn_shield_block,false,3);
		check_improve(victim,gsn_dual_shield,false,2);
        return false;
	}
	else{
		act("You block $n's attack with your secondary shield.",ch,NULL,victim,TO_VICT);
		act("$N blocks your attack with $S secondary shield.",ch,NULL,victim,TO_CHAR);
		act("$N blocks $n's attack with $S secondary shield.",ch,NULL,victim,TO_NOTVICT);
		check_improve(victim,gsn_shield_block,true,2);
		check_improve(victim,gsn_dual_shield,true,2);
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		shield_effects(victim,ch,shield);
		return true;
	}
}

bool check_shield_block(CHAR_DATA *ch,CHAR_DATA *victim){
	OBJ_DATA *shield;
    int chance = get_skill(victim,gsn_shield_block) *.25,dam;

    if (!IS_AWAKE(victim) || (IS_NPC(victim) && !victim->isdef(DF_SHIELDBLOCK)))
        return false;

    if (((shield = get_eq_char(victim,WEAR_SHIELD)) == NULL || shield->item_type != ITEM_SHIELD))
        return false;

if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gShieldblock({c%d{x)",chance);
	chance += (ch->bashwait*10) - (victim->bashwait*10);
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);
	chance = calcReflex(victim,ch,chance);
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

	if (victim->isaff(AF_FORTIFY))
		chance *= 1.5;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

	if (check_shieldmastery(victim,ch,shield))
		chance += (get_skill(victim,gsn_shieldmastery) * ch->getslvl(gsn_shieldmastery) / 50);
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"SM({c%d{x)",chance);

	if(victim->isaff(AF_DEFENDER)){
		printf_to_char(victim,"SHIELD%d\n\r",chance);
		chance += 3*victim->getslvl(gsn_defensive_stance);
		printf_to_char(victim,"SHIELD%d\n\r",chance);
	}

if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)\n\r",chance);
    if (number_percent() >= chance){
		check_improve(victim,gsn_shield_block,false,3);
		check_dual_shield(ch,victim);
        return false;
	}
	else{
		act("You block $n's attack with your shield.",ch,NULL,victim,TO_VICT);
		act("$N blocks your attack with $S shield.",ch,NULL,victim,TO_CHAR);
		act("$N blocks $n's attack with $S shield.",ch,NULL,victim,TO_NOTVICT);
		check_improve(victim,gsn_shield_block,true,2);
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		shield_effects(victim,ch,shield);
		return true;
	}
}

bool check_sai_block(CHAR_DATA *ch,CHAR_DATA *victim)
{
    int chance;
	bool isTonfa;
	OBJ_DATA *wield, *dual;

	isTonfa=false;
    if (!IS_AWAKE(victim))
        return false;

	wield = get_eq_char(victim,WEAR_WIELD);
	dual = get_eq_char(victim,WEAR_SECONDARY);

	if (wield == NULL || (wield->value[0] != WEAPON_SAI && wield->value[0] != WEAPON_TONFA))
	{
		if (dual == NULL || (dual->value[0] != WEAPON_SAI && dual->value[0] != WEAPON_TONFA))
			return false;
		else
			wield=get_eq_char(victim,WEAR_SECONDARY);
	}
	if (wield->value[0] == WEAPON_TONFA)
		isTonfa = true;

		if(!isTonfa)
			chance = get_skill(victim,gsn_martial_arms) *.1;
		else
			chance = get_skill(victim,gsn_martial_arms) *.2;
	chance += ch->bashwait*10;
	chance -= victim->bashwait*10;
	chance = calcReflex(victim,ch,chance);

    if (number_percent() >= chance + victim->level - ch->level)
	{
		if(!isTonfa)
			check_improve(victim,gsn_martial_arms,false,1);
		else
			check_improve(victim,gsn_martial_arms,false,1);
        return false;
	}
	else
	{
		if (dice(1,2) == 1)
		{
			act("You flip $p against your forearm, blocking $n's attack!",ch,wield,victim,TO_VICT);
			act("$N flips $p back against $S forearm and blocks your attack with it!",ch,wield,victim,TO_CHAR);
			act("$N whips $p and raises it as a shield, blocking $n's attack!",ch,wield,victim,TO_NOTVICT);
		}
		else
		{
			act("You catch $n's attack and push it away with $p!",ch,wield,victim,TO_VICT);
			act("$N catches your attack with $p and easily pushes it aside!",ch,wield,victim,TO_CHAR);
			act("With incredible agility, $N catches $n's attack with $p pushes it aside!",ch,wield,victim,TO_NOTVICT);
		}
		if(!isTonfa)
			check_improve(victim,gsn_martial_arms,true,1);
		else
			check_improve(victim,gsn_martial_arms,true,1);
        //return false;
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		return true;
	}
}

bool check_dodge(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *wield){
    int chance,sn;

    if (!IS_AWAKE(victim) || (IS_NPC(victim) && !victim->isdef(DF_DODGE)))
		return false;

	if (MOUNTED(victim))
		sn = gsn_mounteddodge;
	else
		sn = gsn_dodge;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gDodge",chance);

	if (chance = get_skill(victim,sn) < 1)
		return false;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

	chance = chance * 0.1;
	chance += 1;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{g({c%d{x)",chance);
	chance += ch->bashwait*5;
	chance -= victim->bashwait*10;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{g({c%d{x)",chance);
	chance += victim->getslvl(sn);
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{g({c%d{x)",chance);

	if (wield && wield->item_type != ITEM_SHIELD && attack_table[wield->value[3]].damage == DAM_SLASH)
		chance /= 4;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{g({c%d{x)",chance);

    if (!can_see(victim,ch))
		chance /= 3;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{g({c%d{x)",chance);

	chance = calcReflex(victim,ch,chance);
	//chance += victim->level - ch->level;
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gDodge({c%d{x)\n\r",chance);
    if (number_percent() >= chance && !check_instinct(victim)){
		check_improve(victim,sn,false,1);
        return false;
	}
	else{
		act("You dodge $n's attack.",ch,NULL,victim,TO_VICT);
		act("$N dodges your attack.",ch,NULL,victim,TO_CHAR);
		act("$N dodges $n's attack.",ch,NULL,victim,TO_NOTVICT);
		check_improve(victim,sn,true,1);
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		if (!check_outmaneuver(victim,ch))
			return false;
		else
			return true;
	}
}

bool check_feint(CHAR_DATA *ch,CHAR_DATA *victim)
{
    int chance;
	OBJ_DATA *obj;

    if (!IS_AWAKE(victim) || MOUNTED(victim) || (IS_NPC(victim) && !victim->isdef(DF_FEINT)) || (obj = get_eq_char(victim,WEAR_WIELD)) == NULL || get_skill(victim,gsn_feint) < 1)
		return false;

	if(!IS_NPC(victim) && is_class(victim,CLASS_IMMORTAL))
		chance = get_skill(victim,gsn_feint) * .35;
	else
		chance = get_skill(victim,gsn_feint) * .25;
	chance += victim->level - ch->level;
	//stats were here
	chance += ch->bashwait*10;
	chance -= victim->bashwait*10;

    if (!can_see(victim,ch))
		chance /= 2;
	chance = calcReflex(victim,ch,chance);
if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gFeint({c%d{x)\n\r",chance);
    if (number_percent() >= chance){
		check_improve(victim,gsn_feint,false,1);
        return false;
	}
	else
	{
		act("You feint $n's attack.",ch,NULL,victim,TO_VICT);
		act("$N feints your attack.",ch,NULL,victim,TO_CHAR);
		act("$N feints $n's attack.",ch,NULL,victim,TO_NOTVICT);
		check_improve(victim,gsn_feint,true,1);
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		return true;
	}
}

bool check_sidestep(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *wield){
    int chance = get_skill(victim,gsn_sidestep) * .3;

    if (!IS_AWAKE(victim) || MOUNTED(victim) || (IS_NPC(victim) && !victim->isdef(DF_SIDESTEP)) || chance < 1)
		return false;

	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gSidestep({c%d{x)",chance);
	chance += ch->bashwait * 10;
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);
	chance -= victim->bashwait * 10;
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

    if (!can_see(victim,ch))
		chance /= 2;
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);

	if (wield != NULL && wield->item_type == ITEM_SHIELD && attack_table[wield->value[3]].damage != DAM_PIERCE)
		chance /= 2;
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)",chance);
	chance = calcReflex(victim,ch,chance);
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"({c%d{x)\n\r",chance);

    if (number_percent() >= chance){
		check_improve(victim,gsn_sidestep,false,1);
        return false;
	}
	else{
		act("You sidestep $n's attack.",ch,NULL,victim,TO_VICT);
		act("$N easily sidesteps your attack.",ch,NULL,victim,TO_CHAR);
		act("$N easily sidesteps $n's attack.",ch,NULL,victim,TO_NOTVICT);
		check_improve(victim,gsn_sidestep,true,1);
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		return true;
	}
}

bool check_evade(CHAR_DATA *ch,CHAR_DATA *victim){
    int chance = get_skill(victim,gsn_evade) * .25;

    if (!IS_AWAKE(victim) || MOUNTED(victim) || chance < 1 || (IS_NPC(victim) && !victim->isdef(DF_EVADE)))
		return false;

	
	if(ch->iscomm(CM_DEBUG)) printf_to_char(ch,"evade: %d",chance);
	chance += ch->bashwait * 10;
	chance -= victim->bashwait * 10;

	if(ch->iscomm(CM_DEBUG)) printf_to_char(ch,"evade: %d",chance);
	if (victim->fighting)
		return false;

    if (!can_see(victim,ch))
		chance /= 2;

	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gEvade({c%d{x)",chance);
	chance = calcReflex(victim,ch,chance);
	chance += (victim->level/5) - (ch->level/5);
	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gEvade({c%d{x)\n\r",chance);
    if (number_percent() >= chance){
		check_improve(victim,gsn_evade,false,1);
        return false;
	}
	else{
		WAIT_STATE(ch,skill_table[gsn_evade].beats);
		act("You evade $n's attack.",ch,NULL,victim,TO_VICT);
		act("$N evades your attack leaving you momentarily disoriented.",ch,NULL,victim,TO_CHAR);
		act("$N evades $n's attack.",ch,NULL,victim,TO_NOTVICT);
		check_improve(victim,gsn_evade,true,1);
		return true;
	}
}

bool check_duck(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *wield,bool message){
    int chance;

    if (!IS_AWAKE(victim) || MOUNTED(victim) || (IS_NPC(victim) && !victim->isdef(DF_DUCK)) || (chance = get_skill(victim,gsn_duck) * .1) < 1)
		return false;

	chance += ch->bashwait * 5;
	chance -= victim->bashwait * 10;
	chance += victim->getslvl(gsn_duck);

    if (!can_see(victim,ch))
		chance /= 3;

	//stats were here

	if (wield != NULL && wield->item_type != ITEM_SHIELD && attack_table[wield->value[3]].damage == DAM_PIERCE)
		chance /= 2;
	chance = calcReflex(victim,ch,chance);
	//chance += victim->level - ch->level;

	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gDuck({c%d{x)\n\r",chance);
    if (number_percent() < chance || check_instinct(victim)){
		if (message){
			act("You duck $n's attack.",ch,NULL,victim,TO_VICT);
			act("$N ducks your attack.",ch,NULL,victim,TO_CHAR);
			act("$N ducks $n's attack.",ch,NULL,victim,TO_NOTVICT);
		}
		check_improve(victim,gsn_duck,true,1);
		damage(victim,ch,0,TYPE_UNDEFINED,DAM_NONE,false);
		return true;
	}
	check_improve(victim,gsn_duck,false,1);
    return false;
}

bool check_guard(CHAR_DATA *ch,CHAR_DATA *victim){
	CHAR_DATA *guard;
	int chance = 0;

	if (!victim->guardby || victim->guardby->in_room != victim->in_room)
		return false;

	guard = victim->guardby;
	if (ch == guard){
		act("You stop guarding $N.",ch,NULL,victim,TO_CHAR);
		act("$n stops guarding you.",ch,NULL,victim,TO_VICT);
		act("$n stops guarding $N.",ch,NULL,victim,TO_NOTVICT);
		victim->guardby == NULL;
		ch->guarding == NULL;
		return false;
	}
	chance = get_skill(guard,gsn_guard) * .75;

	chance = calcReflex(guard,ch,chance);

	if(!IS_NPC(victim) && victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gGuard({c%d{x)\n\r",chance);
	if (number_percent() < chance){
		act("You jump in front of $N and take the hits!",guard,NULL,victim,TO_CHAR);
		act("$n leaps heroicly before you and takes hits meant for you!",guard,NULL,victim,TO_VICT);
		act("$n heroicly leaps before $N and takes the hits for $M!",guard,NULL,victim,TO_NOTVICT);
		check_improve(guard,gsn_guard,true,4);
		check_defender(ch);
		return true;
	}
	else
		check_improve(guard,gsn_guard,false,2);
	return false;
}

bool check_outmaneuver(CHAR_DATA *victim,CHAR_DATA *rider){
	int chance = get_skill(rider,gsn_outmaneuver) * .5;
	if (!victim || !rider || !MOUNTED(rider) || chance < 1)
		return false;

	chance = chance * get_skill(rider,gsn_horsemanship) / 100;
	chance = calcReflex(rider,victim,chance);
	chance += get_curr_stat(rider,STAT_AGI) / 8;
if(!IS_NPC(rider) && rider->iscomm(CM_DEBUG))printf_to_char(rider,"{gOutmaneuver({c%d{x)\n\r",chance);
	if (number_percent() < chance){
		check_improve(rider,gsn_outmaneuver,true,2);
		WAIT_STATE(victim,skill_table[gsn_outmaneuver].beats);
		act("You bring your horse about and cut off $N before $E can move.",rider,NULL,victim,TO_CHAR);
		act("$n brings $s horse about and cuts you off!",rider,NULL,victim,TO_VICT);
		act("$n cuts off $N with $s horse, stopping $N dead in $S tracks.",rider,NULL,victim,TO_NOTVICT);
		return true;
	}
	else
		check_improve(rider,gsn_outmaneuver,false,2);

	return false;
}

bool check_coule(CHAR_DATA *ch, CHAR_DATA *victim) //checks for victim riposteing ch's "miss"
{
	int chance,csn,vsn,dam_type,dam = 0;
	OBJ_DATA *wield, *cwield;

	if ((IS_NPC(victim) && (is_class(victim,CLASS_MYRMIDON) || victim->level < 55)) || get_skill(victim,gsn_coule) < 1)
		return false;

	wield = get_eq_char(victim,WEAR_WIELD);
	cwield = get_eq_char(ch,WEAR_WIELD);

	if (wield == NULL || cwield == NULL)
		return false;

	csn = get_skill(victim,get_weapon_sn(ch,false));
	vsn = get_skill(victim,get_weapon_sn(victim,false));

	dam_type = attack_table[wield->value[3]].damage;

	if(!is_weapon_sharp(wield) || !is_weapon_bigslash(cwield))
		return false;

	if (csn < 1 || vsn < 1)
		return false;

	chance = (((get_skill(victim,gsn_coule)*.2) * csn) / 100);
	chance = (chance * vsn) / 100;
	chance += (ch->bashwait*10) - (victim->bashwait*10);
	chance += 2 * (get_curr_stat(victim,STAT_AGI) - get_curr_stat(ch,STAT_AGI));

	if (!roll_chance(victim,chance)){
		check_improve(victim,gsn_coule,false,1);
		return false;
	}

	//stats were here

	act("You slide your weapon along $n's blade, and coule $s miss!",ch,NULL,victim,TO_VICT);
	act("$N slides $S weapon along your blade, and coules your miss!",ch,NULL,victim,TO_CHAR);
	act("$N slides $S weapon along $n's blade, and coules $s miss!",ch,NULL,victim,TO_NOTVICT);

	dam = dice(wield->value[1],wield->value[2]);
    damage(victim,ch,dam,gsn_coule,dam_type,true);

    check_improve(victim,gsn_coule,true,1);

    return true;
}

bool check_trompement(CHAR_DATA *ch,CHAR_DATA *victim){
	OBJ_DATA *dual;
	int sn = gsn_trompement,skill = get_skill(ch,sn),dam;

	if (!IS_AWAKE(ch) || skill < 1 || (IS_NPC(ch) && !ch->isdef(DF_TROMPEMENT)))
		return false;

	skill = get_skill(ch,gsn_trompement) *.45;
	skill += (ch->bashwait*10) - (victim->bashwait*10);

	if (get_eq_char(ch,WEAR_WIELD) == NULL){
		if (IS_NPC(ch))
			skill /= 2;
		else
			return false;
	}

	if (!can_see(victim,ch))
		skill /= 2;

	skill = calcReflex(victim,ch,skill);
	if (!roll_chance(ch,skill)){
		check_improve(ch,gsn_trompement,false,1);
		return false;
	}
	act("You slip around $N's parry and continue your attack!",ch,NULL,victim,TO_CHAR);
	act("$n slips through $N's parry and continues attacking!",ch,NULL,victim,TO_NOTVICT);
	act("$n slips past your parry and attacks!",ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_trompement,true,1);
	if (get_skill(ch,gsn_riposte)>0 && get_eq_char(ch,WEAR_SECONDARY) != NULL)
	{
		dual = get_eq_char(ch,WEAR_SECONDARY);
		dam = dice(dual->value[1], dual->value[2]);
		check_riposte(ch,victim,dam,gsn_parry);
	}
	return true;
}
