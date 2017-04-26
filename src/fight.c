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
bool check_stalk				( CHAR_DATA* );
void do_shortbowfire			( CHAR_DATA*,CHAR_DATA* );
void do_bowfire					( CHAR_DATA*,char* );
void do_longbowfire				( CHAR_DATA*,char* );
void set_fighting				( CHAR_DATA*,CHAR_DATA* );
bool check_evade				( CHAR_DATA*,CHAR_DATA* );
bool check_guard				( CHAR_DATA*,CHAR_DATA* );
bool check_damagereduction		( CHAR_DATA*,CHAR_DATA*,int );
bool check_preparationattack	( CHAR_DATA*,CHAR_DATA*,int,int );
int  crunch_lead_stance			( CHAR_DATA*,CHAR_DATA*,int );
void final_strike				( CHAR_DATA*,CHAR_DATA* );
void check_parting_shot				( CHAR_DATA*,CHAR_DATA* );
void check_paired_attack(CHAR_DATA*,CHAR_DATA*,int);
void brawler_faller (CHAR_DATA*,CHAR_DATA*);


bool skill_damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,bool show){
	int n;

	if(victim->position == POS_DEAD)
		return false;

/*	if (ch->in_room != victim->in_room){
		ch->send("You can't reach them!\n\r");
		return false;
	}*/

	if(dam > 1){	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SBse({R%d{d) ",dam);
		if(!IS_NPC(ch) && IS_DRUNK(ch)){//Punish/Bonus for drunkeness
			if(number_percent() <= get_skill(ch,gsn_drunkfighting) * .5){
				dam *= 1.25;
				check_improve(ch,gsn_drunkfighting,true,5);
			}
			else{
				dam *= .75;
				check_improve(ch,gsn_drunkfighting,false,2);
			}
		}if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SDnk({R%d{d) ",dam);

		if(victim->bashwait > 0 || victim->position <= POS_SLEEPING)//Bonuses and detriments for sleepy fighting
			dam *= 1.4;
		else if(victim->position < POS_FIGHTING)
			dam *= 1.2;
		if(ch->bashwait > 0)
			dam *= .85;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SBSH%d ",dam);

		//if(!IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] < 10 && ch->pcdata->condition[COND_HUNGER] > -1)//punish the ch and victim for being hungry/thirsty
		//	dam *= .6;
		//if(!IS_NPC(victim) && victim->pcdata->condition[COND_HUNGER] < 10 && victim->pcdata->condition[COND_HUNGER] > -1)
		//	dam *= 1.4;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SHNG%d ",dam);

		if (show && check_absorb(ch,victim,dam)) //Absorb stuff
			dam /= 2;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SAbs({R%d{d)",dam);

		//RAGE = POWER
		dam += check_rage(ch,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SRge({R%d{d) ",dam);

		if (check_damagereduction(ch,victim,dam_type))
			dam *= .6;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SDrd({R%d{d) ",dam);

		if (check_fluidmotion(victim))
			dam *= .8;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SFld({R%d{d) ",dam);

		if(check_leadership(ch))//The figher's boss can boost damage
			dam *= 1.25;
		if(check_leadership(victim))//The victim's boss can reduce damage
			dam *= .75;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SLDR%d ",dam);

		if(ch->isaff(AF_DEFENDER))
			dam /= 2;
		if(ch->isaff(AF_IMMATERIAL_BURST) && number_percent() < 50)//nash fix this
			dam *= 1.5;
		if(victim->isaff(AF_DEFENDER))
			dam *= 0.75;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SDef({r%d{d)\n\r",dam);

		dam = check_enhanced(ch,victim,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SEnh({r%d{d) ",dam);

		dam = crunch_durability(victim,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SDUR%d\n\r",dam);


		int ac = 0;
		if(dam_type == DAM_PIERCE)
			ac = GET_AC(victim,AC_PIERCE);
		else if(dam_type == DAM_BASH)
			ac = GET_AC(victim,AC_BASH);
		else if(dam_type == DAM_SLASH)
			ac = GET_AC(victim,AC_SLASH);
		else
			ac = GET_AC(victim,AC_EXOTIC);
		ac -= 200;
		if(ac > 0)
			ac = 0;
		if(ac < -1000)
			ac = -1000;
		ac = UMAX(1,ac * -.05);
		dam -= dam * ac / 100;
		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SAC(%d %d)\n\r",dam,ac);
	}
//	dam *= 2;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SBUF%d\n\r",dam);
	return damage(ch,victim,dam,dt,dam_type,show);
}

void check_rebound(CHAR_DATA *victim, CHAR_DATA *ch){
	int chance = get_skill(victim,gsn_rebound) *.25;
	if (chance < 1 || IS_NPC(victim))
		return;
	chance += get_curr_stat(victim,STAT_END)/2;
	chance = calcReflex(victim,ch,chance);

	if (number_percent() < chance){
		act("You bounce off the ground, slamming into $n's stomach to regain your footing!",ch,NULL,victim,TO_VICT);
		act("$N bounces off the ground, slamming into $n's stomach to regain $S footing!",ch,NULL,victim,TO_NOTVICT);
		act("$N bounces off the ground, slamming into your stomach to regain $S footing!",ch,NULL,victim,TO_CHAR);
		skill_damage(victim,ch,get_curr_stat(victim,STAT_STR) * 3,gsn_rebound,DAM_BASH,true);
		damage(victim,victim,get_curr_stat(victim,STAT_STR) * 2 - get_curr_stat(victim,STAT_END),gsn_rebound,DAM_BASH,true);
		victim->bashwait=0;
		victim->daze += 2;
		check_improve(victim,gsn_rebound,true,3);
	}
	else
		check_improve(victim,gsn_rebound,false,4);
}

void disarm(CHAR_DATA *ch,CHAR_DATA *victim){
    OBJ_DATA *obj;

    if ((obj = get_eq_char(victim,WEAR_WIELD)) == NULL)
		return;

    if (IS_OBJ_STAT(obj,ITM_NOREMOVE)){
		act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
		act("$n tries to disarm you, but your weapon won't budge!",ch,NULL,victim,TO_VICT);
		act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
		return;
    }

    act("$n DISARMS you and sends your weapon flying!",ch,NULL,victim,TO_VICT);
    act("You disarm $N!",ch,NULL,victim,TO_CHAR);
    act("$n disarms $N!",ch,NULL,victim,TO_NOTVICT);

    obj_from_char(obj);
    if (IS_OBJ_STAT(obj,ITM_NODROP) || IS_OBJ_STAT(obj,ITM_INVENTORY))
		obj_to_char(obj,victim);
    else{
		obj_to_room(obj,victim->in_room);
		if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
			get_obj(victim,obj,NULL,true);
    }
    return;
}

void do_bash(CHAR_DATA *ch,char *argument){
    char arg[MIL];
    CHAR_DATA *victim;
    int chance = get_skill(ch,gsn_bash)*.75,cAC;
	bool dMiss = false;

    one_argument(argument,arg);
 
    if (chance < 1 || (IS_NPC(ch) && !ch->isoff(OF_BASH))){
		send_to_char("Bashing? What's that?\n\r",ch);
		return;
    }

    if (!(victim = grab_char(ch,arg,true)))
		return;

    if (MOUNTED(ch)){
        send_to_char("You can't bash while riding!\n\r",ch);
        return;
    }

    if (victim->position < POS_FIGHTING || victim->bashwait > 0){
		act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
		return;
    }
	if(ch->bashwait > 0){
		ch->send("You have to get up first.\n\r");
		return;
	}

	if(!canFight(ch,victim,true) || is_safe(ch,victim))
		return;

    if (ch->isaff(AF_CHARM) && ch->master == victim){
		act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
		return;
    }

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBash({c%d{x)\n\r",chance);
    if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 15;
    else
		chance += (ch->size - victim->size) * 10;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBash({c%d{x)\n\r",chance);
	chance += (get_curr_stat(ch,STAT_STR) - get_curr_stat(victim,STAT_AGI)) * 2;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBash({c%d{x)\n\r",chance);
	cAC = GET_AC(victim,AC_BASH);
	if ( cAC < -800 )
		cAC = -800;
	else if( cAC > 200)
		cAC = 200;
	chance += cAC / 50;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBash({c%d{x)\n\r",chance);
	if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
        chance += 10;
    if (victim->isoff(OF_FAST) || victim->isaff(AF_HASTE))
        chance -= 30;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBash({c%d{x)\n\r",chance);
    chance += (ch->level - victim->level) / 10;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBash({c%d{x)\n\r",chance);
    chance = crunch_lead_stance(ch,victim,chance);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBash({c%d{x)\n\r",chance);
    if (roll_chance(ch,chance)){
		if (check_sidestep(ch,victim,NULL) || check_dodge(ch,victim,NULL) || check_feint(ch,victim) || check_mirage(ch,victim) || check_evade(ch,victim))
			dMiss = true;
		else if (check_guard(ch,victim))
			victim = victim->guardby;

		if (dMiss){
			WAIT_STATE(ch,PULSE_VIOLENCE * 1.5);
			ch->bashwait = 6;
			skill_damage(ch,victim,0,gsn_bash,DAM_BASH,false);
			if(!do_downstrike(victim,ch))
				if(!do_trample(ch))
					do_facekick(victim,ch);
			brawler_faller(victim,ch);
			return;
		}

		act("$n sends you sprawling with a powerful bash!",ch,NULL,victim,TO_VICT);
		act("You slam into $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
		act("$n sends $N sprawling with a powerful bash.",ch,NULL,victim,TO_NOTVICT);

		victim->position = POS_RESTING;
		DAZE_STATE(victim,PULSE_VIOLENCE*24);
		if (check_mithril_bash(ch,victim)){
			WAIT_STATE(victim,PULSE_VIOLENCE*20);
			WAIT_STATE(ch,PULSE_VIOLENCE*20);
			victim->bashwait = 24;
			skill_damage(ch,victim,number_range(10,10+2*ch->size + ch->level/10),gsn_bash,DAM_BASH,true);
		}
		else{
			WAIT_STATE(victim,PULSE_VIOLENCE * 10);
			WAIT_STATE(ch,PULSE_VIOLENCE * 16);
			victim->bashwait = 16;
			skill_damage(ch,victim,number_range(2,2 + 2 * ch->size + ch->level/20),gsn_bash,DAM_BASH,true);
		}
		check_improve(ch,gsn_bash,true,1);
		if(!do_trample(victim))
			do_facekick(ch,victim);
        check_dehorsing(victim);
		check_rebound(victim,ch);
		brawler_faller(ch,victim);
    }
    else{
		damage(ch,victim,0,gsn_bash,DAM_BASH,false);
		act("You fall flat on your face!",ch,NULL,victim,TO_CHAR);
		act("$n falls flat on $s face.",ch,NULL,victim,TO_NOTVICT);
		act("You evade $n's bash, causing $m to fall flat on $s face.",ch,NULL,victim,TO_VICT);
		check_improve(ch,gsn_bash,false,1);
		if(!do_downstrike(victim,ch))
			if(!do_trample(ch))
				do_facekick(victim,ch);
		ch->bashwait = 20;
		ch->position = POS_RESTING;
		brawler_faller(ch,victim);
		WAIT_STATE(ch,skill_table[gsn_bash].beats / 2);
    }
}

void do_shieldbash(CHAR_DATA *ch,char *argument){
    char arg[MIL];
    CHAR_DATA *victim;
    int chance = get_skill(ch,gsn_shieldbash)*.75,cAC,cSTR = get_curr_stat(ch,STAT_STR),vSTR;
	bool dMiss = false;

    one_argument(argument,arg);
	printf_to_char(ch,"sb %d\n\r",chance); 
    if (chance < 1 || IS_NPC(ch)){
		send_to_char("What's that?\n\r",ch);
		return;
    }
 
    if (!(victim = grab_char(ch,arg,true)))
		return;

    if (victim->position < POS_FIGHTING || victim->bashwait > 0){
		act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
		return;
    }
	vSTR = get_curr_stat(victim,STAT_STR);

	if (!get_eq_char(ch,WEAR_SHIELD)){
		send_to_char("You must be wearing a shield to do such a thing!\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gShieldbash({c%d{x)\n\r",chance);
	chance += (cSTR - vSTR) / 2;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gShieldbash({c%d{x)\n\r",chance);
	cAC = GET_AC(victim,AC_BASH);
	if (cAC < -800)
		cAC = -800;
	else if (cAC > 200)
		cAC = 200;
	chance += cAC / 100;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gShieldbash({c%d{x)\n\r",chance);
	if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
        chance += 10;
    if (victim->isoff(OF_FAST) || victim->isaff(AF_HASTE))
        chance -= 10;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gShieldbash({c%d{x)\n\r",chance);
    chance = crunch_lead_stance(ch,victim,chance);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gShieldbash({c%d{x)\n\r",chance);

    if (number_percent() < chance){
		if (check_sidestep(ch,victim,NULL) || check_dodge(ch,victim,NULL) || check_feint(ch,victim) || check_mirage(ch,victim))
			dMiss = true;
		else if (check_guard(ch,victim))
			victim = victim->guardby;

		if (dMiss){
			WAIT_STATE(ch,PULSE_VIOLENCE * 1.5);
			ch->bashwait = 6;
			skill_damage(ch,victim,0,gsn_shieldbash,DAM_BASH,true);
			if(!do_downstrike(victim,ch))
				if(!do_trample(ch))
					do_facekick(victim,ch);
			return;
		}

		act("$n slams $s shield into you, smashing you into the ground!",ch,NULL,victim,TO_VICT);
		act("You slam your shield into $N, and send $M sprawling!",ch,NULL,victim,TO_CHAR);
		act("$n slams $s shield into $N, sending $M sprawling.",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,number_range(50,50 + 3 * cSTR + ch->level/20),gsn_shieldbash, DAM_BASH,true);
		check_improve(ch,gsn_shieldbash,true,1);
		if(!do_trample(victim))
			do_facekick(ch,victim);

		WAIT_STATE(victim,PULSE_VIOLENCE);
		WAIT_STATE(ch,PULSE_VIOLENCE * 2);
		victim->position = POS_RESTING;
		DAZE_STATE(victim,PULSE_VIOLENCE*30);
		victim->bashwait = 26;
        check_dehorsing(victim);
		check_rebound(victim,ch);
    }
    else
    {
		skill_damage(ch,victim,0,gsn_shieldbash,DAM_BASH,true);
		act("You fall flat on your face!",ch,NULL,victim,TO_CHAR);
		act("$n falls flat on $s face.",ch,NULL,victim,TO_NOTVICT);
		act("You evade $n's shieldbash, causing $m to fall flat on $s face.",ch,NULL,victim,TO_VICT);
		check_improve(ch,gsn_shieldbash,false,1);
		if(!do_downstrike(victim,ch))
			if(!do_trample(ch))
				do_facekick(victim,ch);
		ch->bashwait = 24;
		WAIT_STATE(ch,PULSE_VIOLENCE * 1.5);
    }
	return;
}

void do_dirt(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	AFFECT_DATA af;
	CHAR_DATA *victim;
	int chance,deduc,sn,dam=0;
	bool improve = false;

	one_argument(argument,arg);

	if (!(victim = grab_char(ch,arg,true)))
		return;

	if (MOUNTED(ch))sn = gsn_mounteddirt;
	else			sn = gsn_dirt;

	if ((chance = get_skill(ch,sn)) < 1 || (IS_NPC(ch) && !ch->isoff(OF_KICK_DIRT))){
		send_to_char("You get your feet dirty.\n\r",ch);
		return;
	}

	if (victim->isaff(AF_BLIND)){
		act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
		return;
	}

	/* modifiers */
	chance += get_curr_stat(ch,STAT_AGI);
	chance -= 2 * get_curr_stat(victim,STAT_AGI);

	if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
		chance += 10;
	if (victim->isoff(OF_FAST) || victim->isaff(AF_HASTE))
		chance -= 25;

	chance += (ch->level - victim->level) * 2;

	if (chance % 5 == 0)
		chance += 1;

	if(!(deduc = sect_table[ch->in_room->sector_type].dirtkick))
		chance = 0;
	else
		chance += deduc;

	if(is_class(victim,CLASS_IMMORTAL) && ch->in_room->sector_type == SECT_DESERT)
		chance /= 2;
	if(is_class(ch,CLASS_IMMORTAL) && ch->in_room->sector_type == SECT_DESERT)
		chance *= 2;

	if (chance == 0){
		send_to_char("There isn't any dirt to kick.\n\r",ch);
		return;
	}

	if (number_percent() < chance){
		if (sn == gsn_dirt){
			act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
			act("$n kicks dirt in your eyes!",ch,NULL,victim,TO_VICT);
		}
		else{
			act("You spur your horse to kick dirt in $N's eyes!",ch,NULL,victim,TO_CHAR);
			act("$n spurs $s horse to kick dirt in $N's eyes!",ch,NULL,victim,TO_NOTVICT);
			act("$n spurs $s horse to kick dirt in your eyes!",ch,NULL,victim,TO_VICT);
		}
		send_to_char("You can't see a thing!\n\r",victim);
		dam = number_range(2,5);
		improve = true;

		affect_set(victim,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),0,APPLY_HITROLL,-4,AF_BLIND);
	}
	check_improve(ch,sn,improve,2);
	skill_damage(ch,victim,dam,sn,DAM_EARTH,!improve);
	WAIT_STATE(ch,skill_table[sn].beats);
}

bool do_downstrike(CHAR_DATA *ch,CHAR_DATA *victim){
    OBJ_DATA *obj;
	int dam = 0,chance,tempShit;

	tempShit = number_range(ch->level/2, ch->level)/2;

	if(IS_NPC(ch) && !ch->isoff(OF_DOWNSTRIKE) || (chance = get_skill(ch,gsn_downstrike)*.75) < 1 || !(obj = get_eq_char(ch,WEAR_WIELD)))
		return false;

	if (is_class(ch,CLASS_ROGUE) && !is_weapon_small_pierce(obj))
		return false;
	else if(!is_weapon_pierce(obj))
		return false;

	if (obj->value[0] == WEAPON_DIRK)
		chance += 10;

	if (number_percent() >= chance)
		check_improve(ch,gsn_downstrike,false,1);
	else{
		dam = dice(obj->value[1], obj->value[2]);
		if (obj->value[0] == WEAPON_DIRK){
			act("You slam your dirk deep into $N's body!",ch,NULL,victim,TO_CHAR);
			act("$n slams $s dirk deep into $N's body!",ch,NULL,victim,TO_NOTVICT);
			act("$n slams $s dirk deep into YOUR body!",ch,NULL,victim,TO_VICT);
			check_improve(ch,gsn_shortswords,true,8);
		}
		else{
			act("You feel $n's blade sink into your back as you fall to the ground!",ch,NULL,victim,TO_VICT);
			act("You slam your blade into $N's back as $E falls to the ground!",ch,NULL,victim,TO_CHAR);
			act("$n sends $s blade into $N's back as $E hits the ground.",ch,NULL,victim,TO_NOTVICT);
			check_improve(ch,gsn_downstrike,true,1);
		}
	}
	skill_damage(ch,victim,dam,gsn_downstrike,DAM_PIERCE,true);
	if (dam == 0)
		return false;
	else
		return true;
}

void do_trip(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	int chance,dam=0;

	one_argument(argument,arg);

	if ((chance = get_skill(ch,gsn_trip)) == 0 || (IS_NPC(ch) && !ch->isoff(OF_TRIP))){
		send_to_char("Tripping?  What's that?\n\r",ch);
		return;
	}

	if (!(victim = grab_char(ch,arg,true)))
		return;

	if(!canFight(ch,victim,true))
		return;

	if (victim->isaff(AF_FLYING)){
		act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (victim->bashwait > 0 || victim->position < POS_FIGHTING){
		act("$N is already down.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 10;/* bigger = harder to trip */

	chance += get_curr_stat(ch,STAT_AGI);
	chance -= get_curr_stat(victim,STAT_AGI) / 2;

	if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
		chance += 10;
	if (victim->isoff(OF_FAST) || victim->isaff(AF_HASTE))
		chance -= 20;

	chance += (ch->level - victim->level) * 2;

	if (number_percent() < chance){
		act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
		act("You trip $N and $E goes down!",ch,NULL,victim,TO_CHAR);
		act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
		check_improve(ch,gsn_trip,true,1);
		dam = number_range(10, 10 +  10 * victim->size);
		skill_damage(ch,victim,dam,gsn_trip, DAM_BASH,true);
		if (!do_downstrike(ch,victim))
			if(!do_trample(victim))
				do_facekick(ch,victim);
			brawler_faller(ch,victim);
		DAZE_STATE(victim,2 * PULSE_VIOLENCE);
		WAIT_STATE(ch,skill_table[gsn_trip].beats);
		victim->bashwait = 10;
	}
	else{
		skill_damage(ch,victim,dam,gsn_trip, DAM_BASH,true);
		WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
		check_improve(ch,gsn_trip,false,1);
	}
}

void do_kill(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if(!arg[0]){
		send_to_char("Kill who?\n\r",ch);
		return;
	}

	if(ch->fighting){
		ch->send("You are already in combat!\n\r");
		return;
	}

	if(!(victim = get_char_room(ch,NULL,arg))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if (!canFight(ch,victim,true))
		return;

	if (victim == ch){
		send_to_char("You hit yourself.  Ouch!\n\r",ch);
		multi_hit(ch,ch,TYPE_UNDEFINED,false);
		return;
	}

	if (ch->position == POS_FIGHTING || ch->fighting){
		send_to_char("You do the best you can!\n\r",ch);
		return;
	}

	if (check_evade(ch,victim))
		return;

	if (check_guard(ch,victim))
		victim = victim->guardby;

	WAIT_STATE(ch,1 * PULSE_VIOLENCE);
	multi_hit(ch,victim,TYPE_UNDEFINED,false);
}

void do_murde(CHAR_DATA *ch,char *argument){
    send_to_char("If you want to MURDER, spell it out.\n\r",ch);
    return;
}

void do_murder(CHAR_DATA *ch,char *argument){
	char buf[MSL],arg[MIL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (arg[0] == '\0'){
		send_to_char("Murder who?\n\r",ch);
		return;
	}

	if (ch->isaff(AF_CHARM) || (IS_NPC(ch) && ch->isact(AT_PET)))
		return;

	if ((victim = get_char_room(ch,NULL,arg)) == NULL){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if (ch->position == POS_FIGHTING){
		send_to_char("You do the best you can!\n\r",ch);
		return;
	}

	WAIT_STATE(ch,1 * PULSE_VIOLENCE);
	if (IS_NPC(ch))
		sprintf(buf,"Help! I am being attacked by %s!",ch->short_descr);
	else
		sprintf(buf,"Help!  I am being attacked by %s!",ch->name);

	if(check_evade(ch,victim))
		return;

	if (check_guard(ch,victim))
		victim = victim->guardby;
	do_function(victim,&do_yell,buf);
	check_killer(ch,victim);
	multi_hit(ch,victim,TYPE_UNDEFINED,false);
}

void do_flee(CHAR_DATA *ch,char *argument){
	extern char * const dir_name[];
	char arg[MSL];
	ROOM_INDEX_DATA *was_in,*now_in;
	CHAR_DATA *victim;
	EXIT_DATA *pexit;
	int attempt,door;

	one_argument(argument,arg);

	if ((victim = ch->fighting) == NULL){
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		send_to_char("You aren't fighting anyone.\n\r",ch);
		return;
	}

	if(ch->bashwait > 0){
		send_to_char("You have to stand up first.\n\r",ch);
		return;
	}

	if (ch->isaff(AF_SNARED)){
		send_to_char("You're snared to the ground!\n\r",ch);
		return;
	}

	was_in = ch->in_room;

	if(arg[0] && get_skill(ch,gsn_elude) > 0){
		if(!str_prefix(arg,"north"))		door = DIR_NORTH;
		else if(!str_prefix(arg,"south"))	door = DIR_SOUTH;
		else if(!str_prefix(arg,"east"))	door = DIR_EAST;
		else if(!str_prefix(arg,"west"))	door = DIR_WEST;
		else if(!str_prefix(arg,"up"))		door = DIR_UP;
		else if(!str_prefix(arg,"down"))	door = DIR_DOWN;
		else								door = 100;

		if ((pexit = was_in->exit[door]) == 0
		||   pexit->u1.to_room == NULL
		||   IS_SET(pexit->exit_info,EX_CLOSED)
		||   number_range(0,ch->daze) != 0
		||  (IS_NPC(ch) && IS_SET(pexit->u1.to_room->room_flags,ROOM_NO_MOB))){
			send_to_char("You can't flee that way!\n\r",ch);
			return;
		}

		if(door != 100){
			if((attempt = get_skill(ch,gsn_elude)) > 0){
				if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Elude:%d\n\r",attempt);
				if(attempt <= number_percent()){
					if (!check_outmaneuver(ch,ch->fighting)){
						if(check_stalk(ch))
							return;
						check_improve(ch,gsn_elude,true,2);
						act("$n quietly slips away.",ch,NULL,NULL,TO_ROOM);
						act("You successfully elude combat and flee $T!",ch,NULL,dir_name[door],TO_CHAR);
						move_char(ch,door,false,false);
						return;
					}
					else
						act("$n stops dead in $s tracks.",ch,NULL,NULL,TO_ROOM);
				}
				else
					check_improve(ch,gsn_elude,false,1);
			}
		}
	}
	else{
		for (attempt = 0;attempt < 6;attempt++){
			door = number_door();
			if ((pexit = was_in->exit[door]) == 0
			||   pexit->u1.to_room == NULL
			||   IS_SET(pexit->exit_info,EX_CLOSED)
			||   number_range(0,ch->daze) != 0
			|| (IS_NPC(ch) && IS_SET(pexit->u1.to_room->room_flags,ROOM_NO_MOB)))
				continue;

			if (check_outmaneuver(ch,ch->fighting)){
				act("$n stops dead in $s tracks.",ch,NULL,NULL,TO_ROOM);
				break;
			}
			if(check_stalk(ch))
				break;

			move_char(ch,door,false,false);
			if ((now_in = ch->in_room) == was_in)
				continue;

			final_strike(ch,victim);
			check_parting_shot(victim,ch);
			ch->in_room = was_in;
			act("$n has fled $T!",ch,NULL,dir_name[door],TO_ROOM);
			ch->in_room = now_in;

			if (!IS_NPC(ch)){
				send_to_char("You flee from combat!\n\r",ch);
			}

			stop_fighting(ch,true);
			return;
		}
	}

    send_to_char("{YPANIC{x! You couldn't escape!\n\r",ch);
}

void do_rescue(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim,*fch;

	one_argument(argument,arg);
	if (arg[0] == '\0'){
		send_to_char("Rescue who?\n\r",ch);
		return;
	}

	if ((victim = get_char_room(ch,NULL,arg)) == NULL){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if (victim == ch){
		send_to_char("What about fleeing instead?\n\r",ch);
		return;
	}

	if(ch->bashwait > 0){
		send_to_char("You have to stand up first.\n\r",ch);
		return;
	}

	if (!IS_NPC(ch) && IS_NPC(victim)){
		send_to_char("Doesn't need your help!\n\r",ch);
		return;
	}

	if (ch->fighting == victim){
		send_to_char("Too late.\n\r",ch);
		return;
	}

	if ((fch = victim->fighting) == NULL){
		send_to_char("That person is not fighting right now.\n\r",ch);
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_rescue].beats);
	if (number_percent() > get_skill(ch,gsn_rescue)){
		send_to_char("You fail the rescue.\n\r",ch);
		check_improve(ch,gsn_rescue,false,1);
		return;
	}

	act("You rescue $N!",ch,NULL,victim,TO_CHAR);
	act("$n rescues you!",ch,NULL,victim,TO_VICT);
	act("$n rescues $N!",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_rescue,true,1);

	stop_fighting(fch,false);
	stop_fighting(victim,false);

	check_killer(ch,fch);
	set_fighting(ch,fch);
	set_fighting(fch,ch);
	return;
}

void do_cleave(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	int dam;

	one_argument(argument,arg);

	if (!IS_NPC(ch) && get_skill(ch,gsn_cleave) < 1){
		send_to_char("You lack the skill to perform such a skill.\n\r",ch);
		return;
	}

	if (IS_NPC(ch))
		return;

	if (arg[0] == '\0'){
		victim = ch->fighting;
		if (victim == NULL){
			send_to_char("But you aren't fighting anyone!\n\r",ch);
			return;
 		}
	}
	else if ((victim = get_char_room(ch,NULL,arg)) == NULL){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if((wield = get_eq_char(ch,WEAR_WIELD)) == NULL){
		send_to_char("You must have a weapon to cleave with.\n\r",ch);
		return;
	}

	if(wield->item_type != ITEM_WEAPON || !is_weapon_bigslash(wield)){
		send_to_char("You may only cleave with a cleaving weapon.. duh.\n\r",ch);
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_cleave].beats);
	if (get_skill(ch,gsn_cleave) * .85 > number_percent()){
		if (check_duck(ch,victim,wield,false)){
			check_improve(ch,gsn_cleave,false,1);
			act("You swing your blade hard upwards, but $N ducks!",ch,NULL,victim,TO_CHAR);
			act("$n swings $s blade hard upwards, as $N ducks beneath the attack.",ch,NULL,victim,TO_NOTVICT);
			act("$n swings $s blade hard upwards, but you duck!",ch,NULL,victim,TO_VICT);
			dam = 0;
			return;
		}
		else
		{
			check_improve(ch,gsn_cleave,true,1);
			act("You swing your blade hard upwards, cleaving $N.",ch,NULL,victim,TO_CHAR);
			act("$n swings $s blade hard upwards, cleaving $N.",ch,NULL,victim,TO_NOTVICT);
			act("$n swings $s blade hard upwards, cleaving you.",ch,NULL,victim,TO_VICT);
			dam = dice(wield->value[1],wield->value[2])*1.5;
			//stat modifiers for damage go here
			if (check_swordsmanship(ch,false,false)){
				if (check_swordmastery(ch,false,false))
					dam *= 2;
				else
					dam *= 1.5;
			}
		}
		skill_damage(ch,victim,dam,gsn_cleave,DAM_SLASH,true);
		check_paired_attack(ch,victim,dam);
	}
	else
	{
		skill_damage(ch,victim,0,gsn_cleave,DAM_SLASH,true);
		check_improve(ch,gsn_cleave,false,1);
	}
}

void do_disarm(CHAR_DATA *ch,char *argument){
    CHAR_DATA *victim;
	char arg[MSL];
    OBJ_DATA *obj;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;
	bool improve = false;

	one_argument(argument,arg);

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0){
		send_to_char("You don't know how to disarm opponents.\n\r",ch);
		return;
    }

    if (get_eq_char(ch,WEAR_WIELD) == NULL && ((hth = get_skill(ch,gsn_combatives)) == 0 || (IS_NPC(ch) && !ch->isoff(OF_DISARM)))){
		send_to_char("You must wield a weapon to disarm.\n\r",ch);
		return;
    }

    if (!(victim = grab_char(ch,arg,true)) || !canFight(ch,victim,true))
		return;

	if (victim->position <= POS_SLEEPING){
		ch->send("Don't be a dork...\n\r");
		return;
	}

    if ((obj = get_eq_char(victim,WEAR_WIELD)) == NULL){
		ch->send("Your opponent is not wielding a weapon.\n\r");
		return;
    }

    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch,false));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim,false));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim,false));

    /* modifiers */

    if (get_eq_char(ch,WEAR_WIELD) == NULL)
		chance = chance * hth/150;
    else
		chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2;

    chance += get_curr_stat(ch,STAT_AGI);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    chance += (ch->level - victim->level) * 2;
	
	if (IS_SET(obj->extra_flags,ITM_GRIPPED) || IS_SET(obj->extra_flags,ITM_GRASPED))
		chance *= .75;

    if (number_percent() < chance){
		disarm(ch,victim);
		improve = true;
    }
    else{
		if (get_eq_char(ch,WEAR_WIELD) != NULL){
			OBJ_DATA *wield;
			wield = get_eq_char(ch,WEAR_WIELD);
			if (wield->value[0] == WEAPON_WHIP && (ch->in_room == victim->in_room)){
				chance = get_skill(ch,gsn_lashes) * .5;
				chance = (chance * ch->level) / victim->level;
				if (number_percent() <= chance){
					disarm(ch,victim);
					improve = true;
					check_improve(ch,gsn_lashes,true,3);
				}
				else{
					act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
					act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
					act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
				}
			}
			else{
				act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
				act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
				act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
			}
		}
		else{
			act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
			act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
			act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
		}
    }
	check_improve(ch,gsn_disarm,improve,1);
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
    return;
}

void do_surrender( CHAR_DATA *ch, char *argument ){
/*	CHAR_DATA *mob;
	if ((mob = ch->fighting) == NULL){
		send_to_char("But you're not fighting!\n\r",ch);
		return;
	}
	act("You surrender to $N!",ch,NULL,mob,TO_CHAR);
	act("$n surrenders to you!",ch,NULL,mob,TO_VICT);
	act("$n tries to surrender to $N!",ch,NULL,mob,TO_NOTVICT);
	stop_fighting(ch,true);

	if (!IS_NPC(ch) && IS_NPC(mob) && (!HAS_TRIGGER_MOB(mob,TRIG_SURR) || !p_percent_trigger(mob,NULL,NULL,ch,NULL,NULL,TRIG_SURR))){
		act("$N seems to ignore your cowardly act!",ch,NULL,mob,TO_CHAR);
		multi_hit(mob,ch,TYPE_UNDEFINED,false);
	}*/
}

void vorpal_stuff(OBJ_DATA *obj,CHAR_DATA *ch,CHAR_DATA *victim){
	int chance;

	if (!IS_WEAPON_STAT(obj,WPN_VORPAL))
		return;

	chance = number_percent();

	    act("$n is {rburned{x by $p.",victim,obj,NULL,TO_ROOM);
	    act("$p {rsears {xyour flesh.",victim,obj,NULL,TO_CHAR);
}

void check_whip(CHAR_DATA *ch,CHAR_DATA *victim){
	if(get_eq_char(victim,WEAR_WIELD) == NULL)
		return;

	if(number_percent() <= get_skill(ch,gsn_lashes)*.1){
		act("You lash out with your whip, wrapping it around $N's wield!",ch,NULL,victim,TO_CHAR);
		act("$n lashes out with $s whip, wrapping it around your wield!",ch,NULL,victim,TO_VICT);
		act("$n lashes out with $s whip, wrapping it around $N's wield!",ch,NULL,victim,TO_NOTVICT);
    	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
		disarm(ch,victim);
		check_improve(ch,gsn_lashes,true,8);
	}
}

void do_assist(CHAR_DATA *ch,char *argument){
	CHAR_DATA *gch = get_char_room(ch,NULL,argument);

	if(!gch){
		ch->send("They aren't here.\n\r");
		return;
	}
	if(!is_same_group(ch,gch)){
		ch->send("You cannot assist someone not in your group.\n\r");
		return;
	}
	if(!gch->fighting){
		ch->send("They are not fighting anyone.\n\r");
		return;
	}
	do_function(ch,&do_kill,gch->fighting->name);//NASHNEEDSTOMAKETHISMOREFUNCTIONAL
}
