#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"

/*
 * Local functions.
 */
#include "combat.h"

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */

void check_improve_armor(CHAR_DATA *ch){//nash ponder how to make this handle two high armorcounts?
	int index_high = 1,sn;

	for(int i = 0;i < 4;i++){
		for(int j = 0;j < 4;j++){
			if(ch->worn_armor[i] > ch->worn_armor[j] && ch->worn_armor[i] > ch->worn_armor[index_high])
				index_high = i;
		}
	}
	switch(index_high){
		default:
			return;
		case 2:
			if(get_skill(ch,gsn_light_armor) < 1)
				return;
			sn = gsn_light_armor;
			break;
		case 3:
			if(get_skill(ch,gsn_medium_armor) < 1)
				return;
			sn = gsn_medium_armor;
			break;
		case 4:
			if(get_skill(ch,gsn_heavy_armor) < 1)
				return;
			sn = gsn_heavy_armor;
			break;
	}
	check_improve(ch,sn,true,1);
}
void violence_update(void){
    CHAR_DATA *ch,*ch_next,*victim;
	OBJ_DATA *obj, *obj_next;
	bool room_trig = false;

	for (obj = object_list;obj;obj = obj_next){
		obj_next = obj->next_content;
		if(obj->in_room || (obj->carried_by && obj->carried_by->in_room)){// Oprog triggers!
			if(HAS_TRIGGER_OBJ(obj,TRIG_DELAY) && obj->oprog_delay > 0){
				if(--obj->oprog_delay <= 0)
					p_percent_trigger(NULL,obj,NULL,NULL,NULL,NULL,TRIG_DELAY);
			}
			else if (((obj->in_room && !obj->in_room->area->empty) || obj->carried_by ) && HAS_TRIGGER_OBJ(obj,TRIG_RANDOM))
				p_percent_trigger(NULL,obj,NULL,NULL,NULL,NULL,TRIG_RANDOM);
		}
	}

	for (ch = char_list;ch;ch = ch_next){
		ch_next	= ch->next;

		if (!(victim = ch->fighting) || !ch->in_room)
			continue;

		if (IS_AWAKE(ch) && ch->in_room == victim->in_room){
			if(--ch->pcounter <= 0)
				multi_hit(ch,victim,TYPE_UNDEFINED,false);
			if(get_eq_char(ch,WEAR_SECONDARY) && --ch->dcounter <= 0)
				multi_hit(ch,victim,TYPE_UNDEFINED,true);
		}
		else
			stop_fighting(ch,false);

		if ((victim = ch->fighting) == NULL)
			continue;

		if(IS_NPC(ch) && ch->bashwait > 0){
			ch->bashwait--;
			if(ch->bashwait == 0){
				act("$n stands up.",ch,NULL,NULL,TO_ROOM);
				act("You stand up.",ch,NULL,NULL,TO_CHAR);
				ch->position = POS_STANDING;
			}
		}

		// Fun for the whole family!
		check_assist(ch,victim);

		if (IS_NPC(ch)){
			if (HAS_TRIGGER_MOB(ch,TRIG_FIGHT))
				p_percent_trigger(ch,NULL,NULL,victim,NULL,NULL,TRIG_FIGHT);
			if (HAS_TRIGGER_MOB(ch,TRIG_HPCNT))
				p_hprct_trigger(ch,victim);
		}
		for (obj = ch->carrying;obj;obj = obj_next){
			obj_next = obj->next_content;

			if (obj->wear_loc != WEAR_NONE && HAS_TRIGGER_OBJ(obj,TRIG_FIGHT))
				p_percent_trigger(NULL,obj,NULL,victim,NULL,NULL,TRIG_FIGHT);
		}

		if (ch->in_room && HAS_TRIGGER_ROOM(ch->in_room,TRIG_FIGHT) && !room_trig){
			room_trig = true;
			p_percent_trigger(NULL,NULL,ch->in_room,victim,NULL,NULL,TRIG_FIGHT);
		}
	}
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim){
	CHAR_DATA *rch,*rch_next,*vch,*target;
	int number;

	for (rch = ch->in_room->people;rch;rch = rch_next){
		rch_next = rch->next_in_room;

		if (IS_AWAKE(rch) && !rch->fighting && !rch->isaff(AF_CALM)){
			if (!IS_NPC(rch)){
				if(!IS_NPC(ch) && is_same_group(ch,rch) && rch->isplr(PL_AUTOASSIST) && ch->position > POS_SLEEPING && canFight(rch,victim,false)){
					do_function(rch,&do_emote,"charges in to assist!");
					multi_hit(rch,victim,TYPE_UNDEFINED,false);
				}
			}
			else{
				if (rch->isoff(AST_ALL)
				|| (rch->isoff(AST_PLAYERS) && !IS_NPC(ch))
				|| (rch->isoff(AST_RACE) && rch->race == ch->race)
				|| (rch->isoff(AST_ALIGN) && (IS_GOOD(rch) && IS_GOOD(ch) || (IS_EVIL(rch) && IS_EVIL(ch)) || (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))
				|| (rch->isoff(AST_VNUM) && rch->pIndexData == ch->pIndexData)
				|| (rch->group && rch->group == ch->group)){
					if (number_bits(1) == 0)//wtf?
						continue;

					target = NULL;
					number = 0;
					for (vch = ch->in_room->people;vch;vch = vch->next){
						if (can_see(rch,vch) && is_same_group(vch,victim) && number_range(0,number) == 0){
							target = vch;
							number++;
						}
					}

					if (target && !check_amiability(target) && !check_animal_magnetism(target,rch)){
						do_function(rch,&do_emote,"screams and attacks{M!{x");
						multi_hit(rch,target,TYPE_UNDEFINED,false);
					}
				}
				if(rch->master
				&& rch->master->fighting
				&& rch->mount != rch->master//added so horses don't help
				&& canFight(rch,rch->master->fighting,false)
				&& !check_animal_magnetism(rch->master->fighting,rch)){
					do_function(rch,&do_emote,"screams and attacks{Y!{x");
					multi_hit(rch,rch->master->fighting,TYPE_UNDEFINED,false);
				}
			}
		}
	}
}

/*
 * Do one group of attacks.
 */
void multi_hit(CHAR_DATA *ch,CHAR_DATA *victim,int dt,bool secondary){
	OBJ_DATA *weapon;
	int chance,dChance;

	if (!ch->desc){
		ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);
		ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE);
	}

	if (ch->isaff(AF_FORTIFY) || ch->position < POS_RESTING)
		return;

	if (IS_NPC(ch)){
		mob_hit(ch,victim,dt,secondary);
		return;
	}

	if (!secondary){
		ch->pcounter = get_attackspeed(ch,false);
		if ((weapon = get_eq_char(ch,WEAR_WIELD)) || (!weapon && (!(weapon = get_eq_char(ch,WEAR_SHIELD)) || get_skill(ch,gsn_shieldfighting) < 1))){
			one_hit(ch,victim,dt,false,false);// initial hit

			if (ch->fighting != victim)return;

			if (ch->fighting != victim)return;

			if((chance = get_skill(ch,gsn_second_attack)/2) > 0){//SECOND ATTACK START:
				if(victim->bashwait > 0 || victim->position <= POS_SLEEPING)	chance *= 1.75;
				else if(victim->position < POS_STANDING)						chance *= 1.5;

				if(ch->bashwait > 0)											chance *= .5;

				if (ch->isaff(AF_SLOW))											chance /= 2;

				if (number_percent() < chance){
					one_hit(ch,victim,dt,false,false);
					check_improve(ch,gsn_second_attack,true,5);
				}
				else
					check_improve(ch,gsn_second_attack,false,4);
			}

			if((chance = get_skill(ch,gsn_third_attack)/3) > 0){//SECOND ATTACK START:
				if(victim->bashwait > 0 || victim->position <= POS_SLEEPING)	chance *= 1.5;
				else if(victim->position < POS_STANDING)						chance *= 1.25;

				if(ch->bashwait > 0)											chance *= .25;

				if (ch->isaff(AF_SLOW))											chance /= 3;

				if (number_percent() < chance){
					one_hit(ch,victim,dt,false,false);
					check_improve(ch,gsn_third_attack,true,5);
				}
				else
					check_improve(ch,gsn_third_attack,false,4);
			}
			if (ch->fighting != victim)return;
		}
		else
			shield_combat(ch,victim,false);
	}
	else{
		if ((weapon = get_eq_char(ch,WEAR_SECONDARY)) && ((weapon->item_type == ITEM_SHIELD && get_skill(ch,gsn_shieldfighting) > 0) || weapon->item_type == ITEM_WEAPON))
			ch->dcounter = get_attackspeed(ch,true);

		if (weapon)
		{
			if (get_skill(ch,gsn_shieldfighting) < 1 || (weapon->item_type != ITEM_SHIELD && weapon->item_type == ITEM_WEAPON)){
				{//dual wield
					dChance = (get_skill(ch,gsn_dual_wield) *.60) * get_weapon_skill(ch,get_weapon_sn(ch,true))*.75;
					if(victim->bashwait > 0 || victim->position <= POS_SLEEPING)	chance *= 1.75;
					else if(victim->position < POS_STANDING)						chance *= 1.5;
					if(ch->bashwait > 0)											chance *= .5;
					if (ch->isaff(AF_SLOW))											chance /= 2;

					if (number_percent() < dChance){
						one_hit(ch,victim,dt,true,false);
						check_improve(ch,gsn_dual_wield,true,2);
					}
					else
						check_improve(ch,gsn_dual_wield,false,2);
				}

				if (ch->fighting != victim)return;

				if (ch->isaff(AF_HASTE)) one_hit(ch,victim,dt,true,false);

				if (ch->fighting != victim)return;
			}
			else
				shield_combat(ch,victim,true);
		}
	}

	if (ch->isaff(AF_PSWORD)){// Phantom swords
		chance = number_percent();

		if (ch->fighting != victim)
			return;
		if (number_percent() < chance){
			act("$n's phantom sword slashes out at $N!",ch,NULL,victim,TO_NOTVICT);
			act("Your phantom sword slashes out at $N!",ch,NULL,victim,TO_CHAR);
			act("$n's phantom sword slashes out at YOU!",ch,NULL,victim,TO_VICT);
			damage(ch,victim, number_range(ch->level/2,ch->level),find_spell(ch,"phantom sword"),DAM_SLASH,true);
		}
		else{
			act("$n's phantom sword slashes out at $N and misses!",ch,NULL,victim,TO_NOTVICT);
			act("Your phantom sword slashes out at $N and misses!",ch,NULL,victim,TO_CHAR);
			act("$n's phantom sword slashes out at you and misses!",ch,NULL,victim,TO_VICT);
		}
	}
}

/* procedure for all mobile attacks */
void mob_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt,bool secondary)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

	if(!IS_NPC(ch) || !victim)
		return;

	if (ch->isoff(OF_BACKSTAB) && !ch->fighting)
		do_function(ch, &do_backstab, victim->name);

	if (!secondary){

		ch->pcounter = get_attackspeed(ch,false);

		one_hit(ch,victim,dt,false,false);

		if (ch->fighting != victim)return;

		if (ch->isoff(OF_AREA_ATTACK))
			for (vch = ch->in_room->people; vch != NULL; vch = vch_next){
				vch_next = vch->next;
				if ((vch != victim && vch->fighting == ch))
					one_hit(ch,vch,dt,false,false);
			}

		if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_assassinate)return;

		if(ch->isoff(OF_SECONDATTACK)){
			chance = get_skill(ch,gsn_second_attack)/2;
			if (ch->isaff(AF_SLOW))
				chance /= 2;
			if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
				chance *= 2;
			if (number_percent() < chance){
				one_hit(ch,victim,dt, false,false);
				if (ch->fighting != victim)
					return;
			}
		}
		if(ch->isoff(OF_THIRDATTACK)){
			chance = get_skill(ch,gsn_third_attack)/3;
			if (ch->isaff(AF_SLOW))
				chance /= 3;
			if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
				chance *= 1.5;
			if (number_percent() < chance){
				one_hit(ch,victim,dt, false,false);
				if (ch->fighting != victim)
					return;
			}
		}

		if (ch->fighting != victim)return;

		if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
			one_hit(ch,victim,dt, false,false);

		if (ch->fighting != victim)return;
	}
	else{
		if (get_eq_char(ch,WEAR_SECONDARY) && (get_eq_char(ch,WEAR_SECONDARY)->item_type == ITEM_WEAPON || get_skill(ch,gsn_shieldfighting) > 0)){
			ch->dcounter = get_attackspeed(ch,true);
			one_hit(ch,victim,dt,true,false);

			if (ch->fighting != victim)return;

			if (ch->isoff(OF_AREA_ATTACK))
				for (vch = ch->in_room->people; vch != NULL; vch = vch_next){
					vch_next = vch->next;
					if ((vch != victim && vch->fighting == ch))
						one_hit(ch,vch,dt,false,false);
				}

			if (ch->fighting != victim)return;

			if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
				one_hit(ch,victim,dt, false,false);

			if (ch->fighting != victim)return;
		}
	}

    number = number_range(0,2);

    if (number == 1 && ch->isact(AT_MAGE)){
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && ch->isact(AT_CLERIC)){
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    number = number_range(0,7);

    switch(number) 
    {
    case (0):
		if (ch->isoff(OF_BASH) && number_percent() < 60)
			do_function(ch,&do_bash,"");
		break;
    case (1):
		if (ch->isoff(OF_BERSERK) && !ch->isaff(AF_BERSERK) && number_percent() < 60)
			do_function(ch,&do_berserk,"");
		break;
    case (2):
		if (ch->isoff(OF_DISARM) && number_percent() < 60)
			do_function(ch,&do_disarm,"");
		break;
    case (3):
		if (ch->isoff(OF_KICK))
			do_function(ch,&do_kick,"");
		break;
    case (4):
		if (ch->isoff(OF_KICK_DIRT) && number_percent() < 70)
			do_function(ch,&do_dirt,"");
		break;
    case (5):
		if (ch->isoff(OF_TAIL)){
			/* do_function(ch,&do_tail,"") */ ;
		}
		break;
    case (6):
		if (ch->isoff(OF_TRIP) && number_percent() < 60)
			do_function(ch,&do_trip,"");
		break;
    case (7):
		if (ch->isoff(OF_CRUSH) && number_percent() < 50){
			/* do_function(ch,&do_crush,"") */ ;
		}
		break;
    }
}

int apply_drunk(CHAR_DATA *ch,int loN){

	if(IS_NPC(ch) || !IS_DRUNK(ch))
		return loN;

	if (get_skill(ch,gsn_drunkfighting) > 0){
		if(number_percent() <= get_skill(ch,gsn_drunkfighting) * .5){
			loN += number_range(ch->pcdata->condition[COND_DRUNK] - 10,1);
			check_improve(ch,gsn_drunkfighting,true,5);
		}
		else{
			loN -= ch->pcdata->condition[COND_DRUNK] - 10;
			check_improve(ch,gsn_drunkfighting,false,2);
		}
	}
	else
		loN -= ch->pcdata->condition[COND_DRUNK] - 10;

	return loN;
}

int lance_check(CHAR_DATA *ch,OBJ_DATA *wield,int loN,int hiN){
	int lance = get_skill(ch,gsn_lance_fighting);

	if (wield && wield->item_type == ITEM_WEAPON && wield->value[0] == WEAPON_LANCE){
		if (number_percent() > lance){
			check_improve(ch,gsn_lance_fighting,false,1);
			return 0;
		}
		else{
			loN *= .75;
			if (get_skill(ch,gsn_lancemastery) > 0){
				loN = loN * get_skill(ch,gsn_lancemastery) / 80;
				check_improve(ch,gsn_lancemastery,true,1);
			}
			else
				check_improve(ch,gsn_lancemastery,false,1);
			hiN = hiN * lance / 80;
			check_improve(ch,gsn_lance_fighting,true,1);
		}
	}

	return number_range(loN,hiN);
}

int number_crunch_C(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *wield,bool secondary,int skill,int secondsn){
	int loN,hiN;

	loN = classes[get_class(ch)].thac0_00;
	hiN = classes[get_class(ch)].thac0_32;

	if (!secondary){
		loN += (((number_range(GET_HITROLL(ch)/2,GET_HITROLL(ch)) * skill)/100) / 10);
		loN += 5 * (100 - skill) / 100;
	}
	else{
		loN -= (number_range(GET_HITROLL(ch)/2,GET_HITROLL(ch)) * skill/100) * (get_skill(ch,secondsn)/100);
		loN += (5 * (100 - skill) / 100) * ((100 - get_skill(ch,secondsn)) / 100);
	}
	if(get_skill(ch,gsn_combat_proficiency) > 1)
		check_improve(ch,gsn_combat_proficiency,true,1);

	loN = dual_nerf(ch,loN,secondary);
	hiN = dual_nerf(ch,hiN,secondary);

	hiN *= .9;
	loN *= 1.1;

	if(check_stamina(ch))
		loN -= (20 - (ch->getslvl(gsn_stamina) * 2)) * (ch->max_hit - ch->hit) / (ch->max_hit * .75);
	else
		loN -= 20 * (ch->max_hit - ch->hit) / (ch->max_hit * .75);
	hiN += 10 * (victim->max_hit - victim->hit) / (victim->max_hit/2);

	hiN += ((100 - hiN) / 2) * get_curr_stat(ch,STAT_AGI) / STAT_MAX;

	if (!wield || (wield->item_type == ITEM_WEAPON && wield->value[0] == WEAPON_GAUNTLET))
		hiN += ((100 - hiN) / 2) * get_curr_stat(ch,STAT_AGI) / STAT_MAX;

	if(!can_see(ch,victim)){
		if (!check_blind_fighting(ch,victim)){//punish the ch for attacking blind
			loN *= .4;
			hiN *= .5;
		}
		else{
			loN -= (loN/3) / ch->getslvl(gsn_blind_fighting);
			hiN -= (loN/4) / ch->getslvl(gsn_blind_fighting);
		}
	}
	if(ch->bashwait > 0){//punish the ch for attacking while down
		loN *= .75;
		hiN *= .75;
	}
	check_improved_accuracy(ch,loN);

	apply_drunk(ch,loN);

	if(check_leadership(ch))
		hiN = hiN * get_leader(ch)->level / 90;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{dhrC({r%d{d){dhrC({r%d{d)\n\r",loN,hiN);
	return lance_check(ch,wield,loN,hiN);
}

int number_crunch_V(CHAR_DATA *ch,CHAR_DATA *victim,int dam_type){
	int vAC = 0,loN,hiN,lnum = 10,pnum = 10,snum = 10,chance;

	if (dam_type == DAM_BASH || dam_type == DAM_PIERCE || dam_type == DAM_SLASH)
		vAC = GET_AC(victim,dam_type);
	else
		vAC = GET_AC(victim,AC_EXOTIC);

	if(check_leadership(victim))   //Boosts victim for leadership
		vAC -= (get_curr_stat(get_leader(victim),STAT_CHA) * ch->pcdata->skill_level[gsn_leadership]);

	//Do some simple ac ceiling/flooring to prevent super over or super under powered people and mobs
	if (vAC > 200)	vAC = 200;
	vAC -= 201;
	vAC *= -1;
	if (vAC > 1000)
		vAC = 1000;

	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"VAC: %d ",vAC);

	int acount = victim->worn_armor[ARMOR_LIGHT] + (victim->worn_armor[ARMOR_MEDIUM] * 2) +  (victim->worn_armor[ARMOR_HEAVY] * 3);
	int skill[MAX_ARMOR];

	skill[ARMOR_LIGHT] = get_skill(ch,gsn_light_armor);
	skill[ARMOR_MEDIUM] = get_skill(ch,gsn_medium_armor);
	skill[ARMOR_HEAVY] = get_skill(ch,gsn_heavy_armor);
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim," %d ",vAC);

	for(int i = ARMOR_LIGHT;i < MAX_ARMOR;i++){
		if(victim->worn_armor[i] > 0){
			if(!roll_chance(ch,skill[i])){
				vAC -= (vAC / 2) * (victim->worn_armor[i] * (i+1)) / acount;
			}
		}
	}
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim," %d ",vAC);
	/*OBJ_DATA *vwield;
	if((vwield = get_eq_char(victim,WEAR_WIELD)) != NULL && vwield->value[0] == WEAPON_SPEAR){
		if(get_eq_char(ch,WEAR_WIELD) == NULL)
			*victim_ac -= (50 * victim->level)/100;
		else{
			OBJ_DATA *cwield;
			cwield = get_eq_char(ch,WEAR_WIELD);
			if (cwield != NULL && cwield->value[0] != WEAPON_SPEAR)
				*victim_ac -= (25 *victim->level)/100;
		}
	}*/


	//NASHNEEDSTOFINDTHISapply_horsemanship(victim);

	if(victim->bashwait > 0 || victim->position <= POS_SLEEPING) //punish the victim for snoozing on the job
		vAC *= .5;
	else if(victim->position < POS_FIGHTING)
		vAC *= .8;

	if(!IS_NPC(victim) && check_poverty(victim))//Is the victim poor? Cuz their god has some ac to dish out
		pnum = pnum * get_skill(ch,gsn_poverty) / 100;

	snum = 10 * (get_curr_stat(victim,STAT_WIS) + get_curr_stat(victim,STAT_AGI)) / (STAT_MAX * 1.5);
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"pnum(%d)snum(%d)\n\rvAC(%d)\n\r",pnum,snum,vAC);

	vAC = (vAC+snum+pnum) / 10;

	if(victim->worn_armor[ARMOR_LIGHT] > 0 && number_percent() < victim->worn_armor[ARMOR_LIGHT] + 5)
		check_improve(victim,gsn_light_armor,true,1);
	if(victim->worn_armor[ARMOR_MEDIUM] > 0 && number_percent() < victim->worn_armor[ARMOR_MEDIUM] + 5)
		check_improve(victim,gsn_medium_armor,true,1);
	if(victim->worn_armor[ARMOR_HEAVY] > 0 && number_percent() < victim->worn_armor[ARMOR_HEAVY] + 5)
		check_improve(victim,gsn_heavy_armor,true,1);
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"{dacV({r%d{d)\n\r",vAC);
	return number_range(vAC/2,vAC);
}

bool number_crunch(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *wield,int dam_type,bool secondary,int skill,int secondsn){
	int cnum = 0,vnum = 0;

	cnum = number_crunch_C(ch,victim,wield,secondary,skill,secondsn);
	vnum = number_crunch_V(ch,victim,dam_type);

	return cnum > vnum;
}

void shield_combat(CHAR_DATA *ch,CHAR_DATA *victim,bool secondary){
	OBJ_DATA *shield;
	char buf[MSL];
	int dam_type = DAM_PIERCE,victim_ac,thac0,thac0_00,thac0_32,dam,diceroll,sn=gsn_shieldfighting,skill=get_skill(ch,gsn_shieldfighting),wearyCalc,armorP,dt=TYPE_HIT;
	bool result,found=false;

	if (!secondary){
		shield=get_eq_char(ch,WEAR_SHIELD);
	}
	else{
		shield=get_eq_char(ch,WEAR_SECONDARY);
		skill = (skill * get_skill(ch,gsn_dual_shield)) / 100;
	}
	if (shield)
		found = true;

    if (victim->position == POS_DEAD || ch->in_room != victim->in_room || skill < 1 || !canFight(ch,victim,true))
		return;

	if(found){
		number_crunch(ch,victim,shield,dam_type,secondary,skill,gsn_dual_shield);

		diceroll = number_range((thac0 + (thac0 *.1)),(thac0 - (thac0 *.4)));

		//factor armor if not using bs or charge
		armorP = victim_ac/20;

		thac0 += armorP;
		if(!IS_NPC(ch) && IS_NPC(victim) && ch->level < 11)
			diceroll += number_range(((11-ch->level) / 1.5),((11-ch->level)*2));
		if(IS_NPC(ch) && ch->level < 11)
			diceroll += number_range(((11-ch->level)),((11-ch->level)*3));

		if (diceroll < thac0){/* Miss. */
			damage(ch,victim,0,sn,dam_type,true);
			if (!IS_NPC(ch))
				check_improve(ch,sn,false,3);
			check_coule(ch,victim);
			tail_chain();
			return;
		}

		if ( dt >= TYPE_HIT && ch != victim
		&& (check_dodge(ch,victim,shield)
		||  check_sidestep(ch,victim,shield)
		||  check_duck(ch,victim,shield,true)
		||  check_shield_block(ch,victim)
		||  check_mirage(ch,victim)
		|| (victim->isaff(AF_PHASE) && check_phase(ch,victim))))
			return;

		dam = number_range( (5 * skill)/100, 2 * ch->level * skill/100);

	//Stat modifiers to damage go here

		if (check_critical(ch,victim))
			dam += dam/10 * ch->getslvl(gsn_critical);

		dam = check_enhanced(ch,victim,dam);

		if (!IS_AWAKE(victim))
			dam *= 2;
		else if (victim->position < POS_FIGHTING)
			dam *= 1.25;

		if (!check_counter(ch,victim,dam,dt,shield,sn) && !check_riposte(ch,victim,dam,dt) && !check_counterspin(ch,victim,dam,dt,shield,sn)){
			if (dam < 1)
				dam = 1;
			result = damage(ch,victim,dam,sn,dam_type,true);

			if (!IS_NPC(ch) && sn != -1)
				check_improve(ch,sn,true,5);

			if (ch->isaff(AF_DSTRIKE) && ch->fighting == victim){
				act("Your attack strikes $N twice!",ch,NULL,victim,TO_CHAR);
				act("$n's attack strikes you twice!",ch,NULL,victim,TO_VICT);
				act("$n's attack strikes $N twice!",ch,NULL,victim,TO_NOTVICT);
				damage(ch,victim,dam / 2,find_spell(ch,"double strike"),dam_type,true);
			}
		}
		else
			return;

		if (result && shield)
			shield_effects(ch,victim,shield);
	}
	else
		damage(ch,victim,0,sn,DAM_PIERCE,true);
}

bool check_divert(CHAR_DATA *victim,CHAR_DATA *ch,bool isDivert,int dt,bool secondary){
	bool found=false;
	char buf[MSL];
	if (!isDivert && (IS_NPC(victim) && victim->isdef(DF_DIVERT)) && number_percent() < calcReflex(victim,ch,get_skill(victim,gsn_divert)) * .15){
		CHAR_DATA *rch,*vch;
		for (rch = ch->in_room->people;	rch != NULL; rch = rch->next_in_room){
			if(!canPK(rch,ch,false) || ((IS_NPC(rch) || IS_NPC(ch)) && is_same_group(rch,ch)))//protect charmies
				continue;
			if (rch != ch && rch != victim && rch->fighting == victim){
				found = true;
				break;
			}
		}

		if (found){
			for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
				if (vch == victim){
					sprintf(buf,"You divert %s's attack to $N!",can_see(victim,ch) ? IS_NPC(ch) ? ch->short_descr : ch->name : "someone's");
					act(buf,victim,NULL,rch,TO_CHAR);
				}
				else if (vch == rch){
					sprintf(buf,"$N diverts %s's attack to you!",can_see(rch,ch) ? IS_NPC(ch) ? ch->short_descr : ch->name : "someone's");
					act(buf,rch,NULL,victim,TO_CHAR);
				}
				else if (vch == ch){
					sprintf(buf,"$N diverts your attack to %s!",can_see(ch,rch) ? IS_NPC(rch) ? rch->short_descr : rch->name : "someone's");
					act(buf,ch,NULL,victim,TO_CHAR);
				}
				else{
					sprintf(buf,"$n diverts %s's attack to %s!",can_see(vch,ch) ? IS_NPC(ch) ? ch->short_descr : ch->name : "someone's",can_see(vch,rch) ? IS_NPC(rch) ? rch->short_descr : rch->name : "someone's");
					act(buf,rch,NULL,vch,TO_VICT);
				}
			one_hit(ch,rch,dt,secondary,true);
			return true;
		}
	}
	return false;
}
//morphhit

void one_hit(CHAR_DATA *ch,CHAR_DATA *victim,int dt,bool secondary,bool isDivert){
	AFFECT_DATA *af;
    OBJ_DATA *wield;
	char buf[MSL];
    int victim_ac,thac0,thac0_00,thac0_32,dam, diceroll,sn=-1,skill,dam_type,wearyCalc,armorP;
    bool result, found = false;

	if (!victim || !ch || victim->position == POS_DEAD || ch->in_room != victim->in_room || !canFight(ch,victim,false) || check_divert(victim,ch,isDivert,dt,secondary))
		return;

	check_readied(ch);

	wield = secondary ? get_eq_char(ch,WEAR_SECONDARY) : get_eq_char(ch,WEAR_WIELD);

    if (dt == TYPE_UNDEFINED){
		dt = TYPE_HIT;
		if (wield != NULL && wield->item_type == ITEM_WEAPON)
			dt += wield->value[3];
		else
			dt += ch->dam_type;
    }

    if (dt < TYPE_HIT){
    	if (wield != NULL)
			dam_type = attack_table[wield->value[3]].damage;
    	else
			dam_type = attack_table[ch->dam_type].damage;
	}
    else
		dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
		dam_type = DAM_BASH;

	if(wield){
		if(!secondary)
			sn = get_weapon_sn(ch,false);
		else
			sn = get_weapon_sn(ch,true);
		skill = get_weapon_skill(ch,sn);
	}
	else{
		if(ch->morph){
			switch(ch->morph){
				case MORPH_FELINE:	sn = gsn_morph_feline;
				case MORPH_CANINE:	sn = gsn_morph_canine;
				case MORPH_URSA:	sn = gsn_morph_ursa;
				case MORPH_SERPENT:	sn = gsn_morph_serpent;
				case MORPH_AVIAN:	sn = gsn_morph_avian;
				case MORPH_AQUATIC:	sn = gsn_morph_aquatic;
				default:
					sn = gsn_combatives;
			};
			skill = get_skill(ch,sn);
		}
		else{
			skill = get_skill(ch,gsn_combatives);
			sn = gsn_combatives;
		}
	}

	unSneak(ch);
	if (wield && (wield->value[0] == WEAPON_LONGBOW|| wield->value[0] == WEAPON_SHORTBOW)){
		damage(ch,victim,0,dt,dam_type,true);
		check_fluidmotion(victim);
		return;
	}
	else if (!number_crunch(ch,victim,wield,dam_type,secondary,skill,gsn_dual_wield)){/* Miss. */
		damage(ch,victim,0,dt,dam_type,true);
		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Failed: %d %s\n\r",sn,skill_table[sn].name);
		check_improve(ch,sn,false,3);
		check_improve_armor(victim);
		check_coule(ch,victim);
		if(ch->isaff(AF_VERNIER_RED) && (af = affect_find(ch->affected,AF_VERNIER_RED))){
			if(af->slvl > 4 && number_percent() < af->level + (100 - af->level) / 5){
				act("An afterimage of your attack strikes $N!",ch,NULL,victim,TO_CHAR);
				act("An afterimage of $n's attack strikes you!",ch,NULL,victim,TO_VICT);
				act("An afterimage of $n's attack strikes $N!",ch,NULL,victim,TO_NOTVICT);
				damage(ch,victim,dam / 10 * af->slvl,af->type,DAM_HARM,true);
			}
		}
		return;
    }

    if(dt >= TYPE_HIT && ch != victim
	&& (check_dodge(ch,victim,wield) || check_feint(ch,victim) || check_sidestep(ch,victim,wield) || check_duck(ch,victim,wield,true)
	|| check_shield_block(ch,victim) || check_sai_block(ch,victim) || check_mirage(ch,victim)
	|| (victim->isaff(AF_PHASE) && check_phase(ch,victim)) || check_parry(ch,victim))){
		check_fluidmotion(victim);
		return;
	}

    if (IS_NPC(ch) && !wield)
		dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]) + ch->damage[DICE_BONUS];
    else{
		if (wield){
			dam = (dice(wield->value[1],wield->value[2]) * victim->res[RS_WEAPON]) / 100;

			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{dOBse({r%d{d:%d) ",dam,victim->res[RS_WEAPON]);
			dam = (dam * skill)/100;
			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{dOSkl({r%d{d) ",dam);

			if(IS_SET(wield->extra_flags,ITM_GRASPED) || IS_WEAPON_STAT(wield,WPN_TWO_HANDS) && get_skill(ch,gsn_grasp) > 0){
				if(number_percent() < get_skill(ch,gsn_grasp)){
					dam += dam * (.2 * ch->getslvl(gsn_grasp));
					check_improve(ch,gsn_grasp,true,1);
				}
				else
					check_improve(ch,gsn_grasp,false,1);
			}
			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"OGsp({r%d{d) ",dam);

			if (IS_WEAPON_STAT(wield,WPN_SHARP))//Sharp!
				dam = number_range(dam,dam * 1.2);
			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"OShp({r%d{d) ",dam);

			if(victim->isaff(AF_REPULSOR_FIELD) && (af = affect_find(victim->affected,AF_REPULSOR_FIELD))){
				dam -= dam / 20 * af->slvl;
				if(af->slvl > 4){
					act("Your attack is pushed back by $N's repulsor field!",ch,NULL,victim,TO_CHAR);
					act("Your repulsor field pushes back $n's attack!",ch,NULL,victim,TO_VICT);
					act("$n's attack is pushed back by $N's repulsor field!",ch,NULL,victim,TO_NOTVICT);
					damage(victim,ch,dam,af->type,DAM_HARM,true);
				}
			}
		}
		else{
			dam = number_range((get_curr_stat(ch,STAT_STR) * skill / 50),ch->getslvl(sn) * (get_curr_stat(ch,STAT_STR) * skill /75 /2));
		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{yOBsX({r%d{d) ",dam);
		}
    }

	dam = melee_hit(ch,victim,dam,secondary);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"OMHT %d ",dam);
	dam += (dam / 10) * ch->getslvl(sn);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"OSLVL %d %d\n\r",dam,sn);
	if(dt == gsn_raptor)
		dam += (dam / 5) * ch->getslvl(dt);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ORPTR %d\n\r",dam);
	dam += check_furor(ch);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Furor: %d\n\r",dam);

	if(!IS_NPC(ch) && ch->iscomm(CM_MORPH) && ch->morph == MORPH_CANINE)
		dam *= 0.9 + (double)(ch->getslvl(gsn_morph_canine)/10);
	//nash needs to someday add a thing for damage boost for dog packs
	if(!IS_NPC(victim) && victim->iscomm(CM_MORPH) && victim->morph == MORPH_CANINE)
		dam -= 5 * victim->getslvl(gsn_morph_canine);

    if (!check_counter(ch,victim,dam,dt,wield,sn) && !check_riposte(ch,victim,dam,dt) && !check_counterspin(ch,victim,dam,dt,wield,sn)){
			if (dam < 1)
				dam = 1;

			/**///if(!IS_NPC(victim)){sprintf(buf,"BaseDam {Y%d{x\n\r",dam);	send_to_char(buf,victim);}
			/**///if(!IS_NPC(ch)){sprintf(buf,"BaseDam {Y%d{x\n\r",dam);	send_to_char(buf,ch);}
			result = skill_damage(ch,victim,dam,dt,dam_type,true);

			if (victim->in_room == ch->in_room && dt != gsn_return)
				vithe_return(ch,victim,dam);

			check_energy_shield(ch,victim);
			check_fuse(ch,victim,dam);

			if (MOUNTED(victim))
				check_horsemanship(victim);

			if(wield != NULL && wield->value[0] == WEAPON_WHIP)
				check_whip(ch,victim);

			if (!IS_NPC(ch) && sn != -1)
				check_improve(ch,sn,true,5);

			check_sonic_impact(ch,victim,secondary);

			if(ch->fighting == victim){
				if(ch->isaff(AF_DSTRIKE)){
					act("Your attack strikes $N twice!",ch,NULL,victim,TO_CHAR);
					act("$n's attack strikes you twice!",ch,NULL,victim,TO_VICT);
					act("$n's attack strikes $N twice!",ch,NULL,victim,TO_NOTVICT);
					damage(ch,victim,dam / 2,find_spell(ch,"double strike"),dam_type,true);
				}
				else if(ch->isaff(AF_VERNIER_RED) && (af = affect_find(ch->affected,AF_VERNIER_RED))){
					if(number_percent() < af->slvl * 2 + af->level){
						act("An afterimage of your attack strikes $N!",ch,NULL,victim,TO_CHAR);
						act("An afterimage of $n's attack strikes you!",ch,NULL,victim,TO_VICT);
						act("An afterimage of $n's attack strikes $N!",ch,NULL,victim,TO_NOTVICT);
						damage(ch,victim,dam / 10 * af->slvl,af->type,DAM_HARM,true);
					}
				}
			}
	}
	else
		return;

	if ((wield = get_eq_char(ch,WEAR_WIELD)) && wield->value[0] == WEAPON_NUNCHAKU && ch->fighting == victim){
		int chance;
		chance = get_skill(ch,gsn_martial_arms) * .5;
		chance = (chance * ch->level) / victim->level;
		if (number_percent() <= chance){
			act("$n whips $p back at you, smacking you across the face with lightning speed!",ch,wield,victim,TO_VICT);
			act("With lightning speed, $n whips $p back at $N, smacking $M across the face!",ch,wield,victim,TO_NOTVICT);
			act("With lightning speed, you whip $N across the face with $p!",ch,wield,victim,TO_CHAR);
			damage(ch,victim,dam*.75,dt,dam_type,true);
		}
	}

    if(result){
		if(!IS_NPC(ch) && ch->iscomm(CM_MORPH) && ch->morph == MORPH_SERPENT)
			snake_bite(ch,victim);
		if(wield)
			weapon_effects(ch,victim,wield);
	}

	check_deathblow(ch,victim);

    tail_chain();
	
    return;
}

bool check_preparationattack(CHAR_DATA *ch,CHAR_DATA *victim,int dt,int dam){
	int skill = get_skill(victim,gsn_preparationattack) *.05;

	if (dam < 5 || skill < 1 || (IS_NPC(victim) && !victim->isdef(DF_PREPATTACK))) return false;

	if(dt == gsn_preparationattack || dt == gsn_counter || dt == gsn_counter_spin || dt == gsn_riposte || dt == gsn_coule || dt == gsn_trompement || dt == gsn_backfist)
		return false;

	skill = calcReflex(victim,ch,skill);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gPrep({c%d{x)\n\r",skill);
	skill += 10 * victim->level / ch->level;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gPrep({c%d{x)",skill);
	skill += get_curr_stat(victim,STAT_WIS) / 10;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gPrep({c%d{x)",skill);

	if (number_percent() < skill){
		act("You catch $n on $s preparation and attack $m!",ch,NULL,victim,TO_VICT);
		act("$N catches $n when $s guard drops to attack, and counters!",ch,NULL,victim,TO_NOTVICT);
		act("As you prepare to attack $N, $E slips an attack in and disrupts you!",ch,NULL,victim,TO_CHAR);
		damage(victim,ch,dam/2,gsn_preparationattack,DAM_OTHER,true);
		check_improve(victim,gsn_preparationattack,true,1);
		return true;
	}
	else
		check_improve(victim,gsn_preparationattack,false,1);
	return false;
}

int dam_mod(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type){
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DBse({r%d{d)\n\r",dam);
	if(!IS_NPC(ch) && ch->level < 11)// A little goose for newbs, not much, but something
		dam += (11 - ch->level)/3;
	if(IS_NPC(ch) && ch->level < 11)
		dam += (11 - ch->level)/4;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DNwb({R%d{d) ",dam);

	if (dt != gsn_kindred_spirits)
		dam = kindred_spirits(victim,ch,dam);
	if(ch->isaff(AF_GUARDIAN))
		dam *= .9;
	if ( dam > 1 && victim->isaff(AF_SANCTUARY))
		dam /= 2;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DSNC%d ",dam);

	if (!IS_AWAKE(victim))
		dam *= 2;
	else if (victim->position < POS_FIGHTING)
		dam *= 1.25;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DSlp({r%d{x)\n\r",dam);

	if (victim->isaff(AF_BDAMTAKE))
		dam *= 2;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DSHT%d ",dam);

	dam = crunch_resilience(victim,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DRLN%d ",dam);

	//nash probably make this thing handle the elements
	dam = dam_res(ch,victim,dam_type,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DRES%d ",dam);

	if(victim->in_room->israf(RAF_GROVE))
		dam *= .25;
	return dam;
}

int melee_hit(CHAR_DATA *ch,CHAR_DATA *victim,int dam,bool dual){//NASHMAKE SURE THISNERFS DUAL RIGHT, this is for weapon hits
	int n;
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MBSE({r%d{d)\n\r",dam);
	//dam += number_range(1,100) * get_curr_stat(ch,STAT_STR) / STAT_MAX;
	//dam -= number_range(1,50) * get_curr_stat(victim,STAT_END) / STAT_MAX;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DStt({r%d{d)\n\r",dam);

	if (check_lancemastery(ch))
		dam *= 1.5;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MLnc({R%d{d)\n\r",dam);

	dam += 2 * (dice(1,GET_DAMROLL(ch)+1) / ((GET_DAMROLL(ch)+1)/2));if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MDR({R%d{x) ",dam);

	if(check_swordsmanship(ch,false,dual)){
		if (check_swordmastery(ch,false,dual))
			dam *= 1.25;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MSwd({r%d{d) ",dam);
		else
			dam *= 1.15;
	}

	dam = dual_nerf(ch,dam,dual);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MDual({r%d{d) ",dam);

	if (check_critical(ch,victim))
        dam *= 1.30;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MCrt({r%d{d) ",dam);

	dam = heavy_hand(ch,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MHVY({r%d{d) ",dam);

	dam = check_brawler(ch,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MBwl({r%d{d) ",dam);

	if (ch != victim){
		if (check_icebound(ch) == 1)
			dam *= 1.5;
		if (check_icebound(ch) == 3)
			dam *= .25;
	}if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MICE%d ",dam);

	if(victim->isaff(AF_BULWARK))
		dam -= ((dam/3) * victim->getslvl(gsn_bulwark)) / 5;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MBul({R%d{d) ",dam);

	if (ch->isaff(AF_BDAMGIVE))
		dam *= 2;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"MOvr({R%d{d) ",dam);
	return dam;
}

bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,bool show){
	bool immune = false;

	if (victim->position < POS_SLEEPING)
		return false;
	if(ch->position < POS_SLEEPING)
		return false;

	if (ch != victim && dam > 0 && !canPK(ch,victim,true))
		return false;

	if (dt >= TYPE_HIT && dam > 3000){
		if ((IS_IMMORTAL(ch) && dam > 50000))
			dam = 50000;
		if (!IS_IMMORTAL(ch)) 
			dam = 3000;
		if (!IS_IMMORTAL(ch)){
			OBJ_DATA *obj;
			obj = get_eq_char(ch,WEAR_WIELD);
			send_to_char("You really shouldn't cheat.\n\r",ch);
			if (obj != NULL)
	    		extract_obj(obj);
		}
	}
	
	if(dam >= 0){
		if (victim->position <= POS_MORTAL)
			return false;

		if (victim != ch){
			if (!canFight(ch,victim,false))
				return false;

			if (victim->position > POS_STUNNED){
				if (!victim->fighting){
					set_fighting(victim,ch);
					if (IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_KILL))
						p_percent_trigger(victim,NULL,NULL,ch,NULL,NULL,TRIG_KILL);
				}
				if (victim->timer <= 2)
	    			victim->position = POS_FIGHTING;
			}

			if (is_affected(victim,gsn_sap))
				affect_strip(victim,gsn_sap);

			victim->position = POS_FIGHTING;

			if (dam > 0 && ch->isaff(AF_FORTIFY)){
				act("$n takes a more offensive stance.",ch,NULL,NULL,TO_ROOM);
				ch->send("You resume a more offensive stance.\n\r");
				ch->remaff(AF_FORTIFY);
			}
			if (victim->position > POS_STUNNED)
				if (!ch->fighting)
					set_fighting(ch,victim);

			if(victim->master == ch && !victim->isaff(AF_STALK))
				stop_follower(victim);
		}

		if (ch->isaff(AF_INVISIBLE)){
			affect_strip(ch,gsn_invis);
			affect_strip(ch,gsn_mass_invis);
			ch->remaff(AF_INVISIBLE);
			act("$n fades into existence.",ch,NULL,NULL,TO_ROOM);
		}

		if (is_affected(victim,gsn_sleep))
			affect_strip(victim,gsn_sleep);
		if (victim->position == POS_SLEEPING)
			do_function(ch,&do_wake,"");

	dam = dam_mod(ch,victim,dam,dt,dam_type);
	}

	if (show && !(!IS_NPC(ch) && victim->position == POS_MORTAL))
		dam_message(ch,victim,dam,dt,immune);

	if (dam == 0)
		return false;

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	if(!(!IS_NPC(ch) && victim->position == POS_MORTAL)){
		victim->hit -= dam;
		victim->remaff(AF_CALM);//NASHNEEDSTOFIXTHIS so it uses a gsn
		if(IS_NPC(victim))
			mob_remember(victim,ch,MEM_HOSTILE,dam);
	}

	if (victim->hit > victim->max_hit)
		victim->hit = victim->max_hit;

	if ( !IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1 && victim->challenge == NULL)
		victim->hit = 1;

	update_pos(victim,ch);

	switch(victim->position){
		case POS_MORTAL:
			break_spell(ch);
			if(victim->level < 10)
				do_function(victim,&do_help,"death");
			act("$n is mortally wounded, and will die soon, if not aided.",victim,NULL,NULL,TO_ROOM);
			send_to_char("You are mortally wounded, and will die soon, if not aided. Or '{Gdie{x' to give up on life.\n\r",victim);
			victim->hit = -10;
			sprintf(log_buf,"%s is mortally wounded at %s\n\r",victim->name,victim->in_room->name);
			global_message(1,MAX_LEVEL,log_buf,CM_LIFELINE);
			sprintf(log_buf,"%s is mortally wounded at %d",victim->name,victim->in_room->vnum);
			if (IS_NPC(victim))
				wiznet(log_buf,NULL,NULL,WZ_MOBDEATHS,0,0);
			else
				wiznet(log_buf,NULL,NULL,WZ_DEATHS,0,0);
			break;
		case POS_INCAP:
			break_spell(ch);
			act("$n is incapacitated and will slowly die, if not aided.",victim,NULL,NULL,TO_ROOM);
			send_to_char("You are incapacitated and will slowly die, if not aided.\n\r",victim);
			break;
		case POS_STUNNED:
			break_spell(ch);
			act("$n is stunned, but will probably recover.",victim,NULL,NULL,TO_ROOM);
			send_to_char("You are stunned, but will probably recover.\n\r",victim);
			break;
		case POS_DEAD:
			break_spell(ch);
			act("$n is {RDEAD{x!!",victim,0,0,TO_ROOM);
			send_to_char("You have been {RKILLED{x!!\n\r\n\r",victim);
			break;
		default:
			if (dam > victim->max_hit / 4)
				send_to_char("That really did {YHURT{x!\n\r",victim);
			if (dam > 0 && victim->hit < victim->max_hit / 4)
				send_to_char("You sure are {rBLEEDING{x!\n\r",victim);
			break;
	}
	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if (!IS_AWAKE(victim))
		stop_fighting(victim,false);

	/*
	 * Payoff for killing things.
	 */
	if (victim->position == POS_DEAD)
		return victim_die(ch,victim);

	if (victim == ch)
		return true;

	/*
	 * Take care of link dead people.
	 */
	if (!IS_NPC(victim) && !victim->desc){
		if (number_range(0,victim->wait) == 0){
			do_function(victim, &do_recall,"");
			return true;
		}
	}

	/*
	 * Wimp out?
	 */
	if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2){
		if((victim->isact(AT_WIMPY) && number_bits(2)==0 && victim->hit < victim->max_hit / 5) || (victim->isaff(AF_CHARM) && victim->master !=NULL && victim->master->in_room !=victim->in_room))
			do_function(victim,&do_flee,"");
	}

	tail_chain();
	return true;
}
bool miss(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,bool show){
	bool immune = false;

	if (victim->position < POS_SLEEPING)
		return false;
	if(ch->position < POS_SLEEPING)
		return false;

	if (ch != victim && dam > 0 && !canPK(ch,victim,true))
		return false;

	if (dt >= TYPE_HIT && dam > 3000){
		if ((IS_IMMORTAL(ch) && dam > 50000))
			dam = 50000;
		if (!IS_IMMORTAL(ch)) 
			dam = 3000;
		if (!IS_IMMORTAL(ch)){
			OBJ_DATA *obj;
			obj = get_eq_char(ch,WEAR_WIELD);
			send_to_char("You really shouldn't cheat.\n\r",ch);
			if (obj != NULL)
	    		extract_obj(obj);
		}
	}
	
	if(dam >= 0){
		if (victim->position <= POS_MORTAL)
			return false;

		if (victim != ch){
			if (!canFight(ch,victim,false))
				return false;

			if (victim->position > POS_STUNNED){
				if (!victim->fighting){
					set_fighting(victim,ch);
					if (IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_KILL))
						p_percent_trigger(victim,NULL,NULL,ch,NULL,NULL,TRIG_KILL);
				}
				if (victim->timer <= 2)
	    			victim->position = POS_FIGHTING;
			}

			if (is_affected(victim,gsn_sap))
				affect_strip(victim,gsn_sap);

			victim->position = POS_FIGHTING;

			if (dam > 0 && ch->isaff(AF_FORTIFY)){
				act("$n takes a more offensive stance.",ch,NULL,NULL,TO_ROOM);
				ch->send("You resume a more offensive stance.\n\r");
				ch->remaff(AF_FORTIFY);
			}
			if (victim->position > POS_STUNNED)
				if (!ch->fighting)
					set_fighting(ch,victim);

			if(victim->master == ch && !victim->isaff(AF_STALK))
				stop_follower(victim);
		}

		if (ch->isaff(AF_INVISIBLE)){
			affect_strip(ch,gsn_invis);
			affect_strip(ch,gsn_mass_invis);
			ch->remaff(AF_INVISIBLE);
			act("$n fades into existence.",ch,NULL,NULL,TO_ROOM);
		}

		if (is_affected(victim,gsn_sleep))
			affect_strip(victim,gsn_sleep);
		if (victim->position == POS_SLEEPING)
			do_function(ch,&do_wake,"");

	dam = dam_mod(ch,victim,dam,dt,dam_type);
	}

	if (show && !(!IS_NPC(ch) && victim->position == POS_MORTAL))
		dam_message(ch,victim,dam,dt,immune);

	if (dam == 0)
		return false;

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	if(!(!IS_NPC(ch) && victim->position == POS_MORTAL)){
		victim->hit -= dam;
		victim->remaff(AF_CALM);//NASHNEEDSTOFIXTHIS so it uses a gsn
		if(IS_NPC(victim))
			mob_remember(victim,ch,MEM_HOSTILE,dam);
	}

	if (victim->hit > victim->max_hit)
		victim->hit = victim->max_hit;

	if ( !IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1 && victim->challenge == NULL)
		victim->hit = 1;

	update_pos(victim,ch);

	switch(victim->position){
		case POS_MORTAL:
			break_spell(ch);
			if(victim->level < 10)
				do_function(victim,&do_help,"death");
			act("$n is mortally wounded, and will die soon, if not aided.",victim,NULL,NULL,TO_ROOM);
			send_to_char("You are mortally wounded, and will die soon, if not aided. Or '{Gdie{x' to give up on life.\n\r",victim);
			victim->hit = -10;
			sprintf(log_buf,"%s is mortally wounded at %s\n\r",victim->name,victim->in_room->name);
			global_message(1,MAX_LEVEL,log_buf,CM_LIFELINE);
			sprintf(log_buf,"%s is mortally wounded at %d",victim->name,victim->in_room->vnum);
			if (IS_NPC(victim))
				wiznet(log_buf,NULL,NULL,WZ_MOBDEATHS,0,0);
			else
				wiznet(log_buf,NULL,NULL,WZ_DEATHS,0,0);
			break;
		case POS_INCAP:
			break_spell(ch);
			act("$n is incapacitated and will slowly die, if not aided.",victim,NULL,NULL,TO_ROOM);
			send_to_char("You are incapacitated and will slowly die, if not aided.\n\r",victim);
			break;
		case POS_STUNNED:
			break_spell(ch);
			act("$n is stunned, but will probably recover.",victim,NULL,NULL,TO_ROOM);
			send_to_char("You are stunned, but will probably recover.\n\r",victim);
			break;
		case POS_DEAD:
			break_spell(ch);
			act("$n is {RDEAD{x!!",victim,0,0,TO_ROOM);
			send_to_char("You have been {RKILLED{x!!\n\r\n\r",victim);
			break;
		default:
			if (dam > victim->max_hit / 4)
				send_to_char("That really did {YHURT{x!\n\r",victim);
			if (dam > 0 && victim->hit < victim->max_hit / 4)
				send_to_char("You sure are {rBLEEDING{x!\n\r",victim);
			break;
	}
	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if (!IS_AWAKE(victim))
		stop_fighting(victim,false);

	/*
	 * Payoff for killing things.
	 */
	if (victim->position == POS_DEAD)
		return victim_die(ch,victim);

	if (victim == ch)
		return true;

	/*
	 * Take care of link dead people.
	 */
	if (!IS_NPC(victim) && !victim->desc){
		if (number_range(0,victim->wait) == 0){
			do_function(victim, &do_recall,"");
			return true;
		}
	}

	/*
	 * Wimp out?
	 */
	if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2){
		if((victim->isact(AT_WIMPY) && number_bits(2)==0 && victim->hit < victim->max_hit / 5) || (victim->isaff(AF_CHARM) && victim->master !=NULL && victim->master->in_room !=victim->in_room))
			do_function(victim,&do_flee,"");
	}

	tail_chain();
	return true;
}
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MSL];
	if(ch->isplr(PL_ARENA) && victim->isplr(PL_ARENA))
		return;

    while ( victim->isaff(AF_CHARM) && victim->master != NULL )
		victim = victim->master;

    if (IS_NPC(victim)
    ||  victim->isplr(PL_KILLER)
    ||  victim->isplr(PL_THIEF))
		return;

    if (ch->isaff(AF_CHARM))
    {
		if ( ch->master == NULL )
		{
			bugf("Check_killer: %s bad AF_CHARM",	IS_NPC(ch) ? ch->short_descr : ch->name );
			affect_strip( ch, gsn_charm_person );
			ch->remaff(AF_CHARM);
			return;
		}
		return;
    }

    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   ch->isplr(PL_KILLER) 
    ||	 ch->fighting  == victim)
		return;

    sprintf(buf,"$N is attempting to murder %s",victim->name);
    wiznet(buf,ch,NULL,WZ_FLAGS,0,0);
    cql_save_char( ch );
    return;
}

void update_pos(CHAR_DATA *victim,CHAR_DATA *ch){
	if (victim->hit > 0){
		if (victim->position <= POS_STUNNED)
			victim->position = POS_STANDING;
		return;
	}

	if (IS_NPC(victim)){
		if(victim->hit < 1){
			victim->position = POS_DEAD;
			return;
		}
	}
	if(!IS_NPC(victim)){
		if(victim->hit <= -4){
			if (ch && IS_NPC(ch)){
				victim->death_timer = 1;
				victim->position = POS_MORTAL;
				stop_fighting(victim,true);
				get_new_killer(ch,victim);
				if(check_true_belief(ch)){
					ch->position = POS_RESTING;
					ch->hit = ch->max_hit / 10 * ch->getslvl(gsn_true_belief);
				}
			}
			else
				victim->position = POS_DEAD;
		}
		else if (victim->hit <= -3){
			victim->position = POS_INCAP;
			stop_fighting(victim,true);
		}
		else{
			victim->position = POS_STUNNED;
			stop_fighting(victim,true);
		}
	}
}

void set_fighting(CHAR_DATA *ch,CHAR_DATA *victim){
    if (ch->fighting){
		bug("Set_fighting: already fighting",0);
		return;
    }

    unSleep(ch);
	unSleep(victim);

    ch->fighting = victim;
    ch->position = POS_FIGHTING;
	ch->pcounter = get_attackspeed(ch,false) /2;
	ch->dcounter = get_attackspeed(ch,true) /2;
}

void stop_fighting(CHAR_DATA *ch,bool fBoth){
	CHAR_DATA *fch;

	for ( fch = char_list; fch != NULL; fch = fch->next ){
		if ( fch == ch || ( fBoth && fch->fighting == ch ) ){
			fch->fighting	= NULL;
			if (fch->position == POS_FIGHTING)
				fch->position = IS_NPC(fch) ? fch->default_pos : POS_STANDING;
			fch->bashwait = 0;
			//update_pos( fch );
		}
	}
}

void make_corpse(CHAR_DATA *ch){
    char buf[MSL];
    OBJ_DATA *corpse,*obj,*obj_next;
    char *name,*keywords;

	if(!ch->isform(FRM_INSTANT_DECAY)){
		if(IS_NPC(ch)){
			name			= ch->short_descr;
			corpse			= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
			corpse->timer	= number_range( 3, 6 );
			if ( ch->gold > 0 ){
				obj_to_obj( create_money( ch->gold, ch->silver ), corpse );
				ch->gold = 0;
				ch->silver = 0;
			}
			corpse->cost = 0;
		}
		else{
			keywords		= ch->name;
			name			= ch->name;
			corpse			= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
			corpse->timer	= number_range( 25, 40 );
				corpse->owner = NULL;
				if (ch->gold > 1 || ch->silver > 1){
					obj_to_obj(create_money(ch->gold / 2, ch->silver/2), corpse);
					ch->gold -= ch->gold/2;
					ch->silver -= ch->silver/2;
			}
			
			corpse->cost = 0;
			ch->remplr(PL_CANLOOT);

			sprintf( buf, "corpse %scorpse", keywords );
			free_string( corpse->name );
			corpse->name = str_dup( buf );
		}
		corpse->level = ch->level;

		sprintf( buf, corpse->short_descr, name );
		free_string( corpse->short_descr );
		corpse->short_descr = str_dup( buf );

		sprintf( buf, corpse->description, name );
		free_string( corpse->description );
		corpse->description = str_dup( buf );
		for (obj = ch->carrying;obj;obj = obj_next){
			bool floating = false;
			obj_next = obj->next_content;

			if(obj->droprate != 1000 && IS_NPC(ch)){
				if(number_range(1,1000) > obj->droprate){
					extract_obj(obj);
					continue;
				}
			}
			if (obj->wear_loc == WEAR_FLOAT || obj->wear_loc == WEAR_FLOAT_LIGHT)
				floating = true;
			obj_from_char( obj );
			if (obj->item_type == ITEM_POTION)
				obj->timer = number_range(500,1000);
			if (obj->item_type == ITEM_SCROLL)
				obj->timer = number_range(1000,2500);
			if (IS_SET(obj->extra_flags,ITM_ROT_DEATH) && !floating)
			{
				obj->timer = number_range(5,10);
				REMOVE_BIT(obj->extra_flags,ITM_ROT_DEATH);
			}
			REMOVE_BIT(obj->extra_flags,ITM_VIS_DEATH);

			if (IS_SET(obj->extra_flags,ITM_INVENTORY))
				extract_obj( obj );
			else
				if (floating)
				{
					if (IS_OBJ_STAT(obj,ITM_ROT_DEATH)) /* get rid of it! */
					{ 
						if (obj->contains != NULL)
						{
							OBJ_DATA *in, *in_next;

							act("$p evaporates,scattering its contents.", ch,obj,NULL,TO_ROOM);
							for (in = obj->contains; in != NULL; in = in_next){
								in_next = in->next_content;
								obj_from_obj(in);
								obj_to_room(in,ch->in_room);
							}
						}
						else
							act("$p evaporates.",ch,obj,NULL,TO_ROOM);
						extract_obj(obj);
					}
					else{
						act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
						obj_to_room(obj,ch->in_room);
					}
				}
				else
					obj_to_obj( obj, corpse );
		}
		obj_to_room( corpse, ch->in_room );
	}
	else{
		for(obj = ch->carrying;obj;obj = obj_next){
			obj_next = obj->next_content;
			if(obj && obj->droprate < 1000 && IS_NPC(ch)){
				if(number_range(1,1000) > obj->droprate || obj->droprate == 0){
					extract_obj(obj);
					continue;
				}
			}
			obj_from_char(obj);
			obj_to_room(obj,ch->in_room);
		}
	}
}

void death_cry( CHAR_DATA *ch ){
	ROOM_INDEX_DATA *was_in_room;
	char *msg;
	int door,n;
	bool makeobj = false;

	if (ch->isform(FRM_SILENT))
		return;
	msg = "You hear $n's death cry.";

	n = number_bits(4);
	if (n == 0){
			msg  = "$n hits the ground ... DEAD.";
	}
	else if (n == 1){
		if (ch->material == 0)
			msg  = "$n splatters {rblood {xon your armor.";
	}
	else if(part_table[n].keyword && ch->ispart(part_table[n].bit)){
		msg = str_dup(part_table[n].msg);
		makeobj = true;
	}

	act(msg,ch,NULL,NULL,TO_ROOM);

	if (makeobj){
		char buf[MSL];
		OBJ_DATA *obj;

		obj					= create_object(get_obj_index(OBJ_VNUM_GUTS),0);
		obj->timer			= number_range(4,7);
		obj->name			= str_dup(part_table[n].keyword);

		sprintf(buf,part_table[n].short_descr,IS_NPC(ch) ? ch->short_descr : ch->name);
		free_string(obj->short_descr);
		obj->short_descr		= str_dup(buf);

		sprintf(buf,part_table[n].long_descr,IS_NPC(ch) ? ch->short_descr : ch->name);
		free_string(obj->description);
		obj->description	= str_dup(buf);

		if (obj->item_type == ITEM_FOOD){
			if (ch->isform(FRM_POISON))
				obj->value[3] = 1;
			else if (!ch->isform(FRM_EDIBLE))
				obj->item_type = ITEM_TRASH;
		}

		obj_to_room(obj,ch->in_room);
	}

	if ( IS_NPC(ch) )
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	was_in_room = ch->in_room;
	for ( door = 0; door <= 5; door++ ){
		EXIT_DATA *pexit;

		if ( (pexit = was_in_room->exit[door]) != NULL && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room ){
			ch->in_room = pexit->u1.to_room;
			act(msg,ch,NULL,NULL,TO_ROOM);
		}
	}
	ch->in_room = was_in_room;
}

void raw_kill(CHAR_DATA *ch,CHAR_DATA *victim){
	int i;
	OBJ_DATA *tattoo;

	stop_fighting(victim,true);
	if(ch){
		if (IS_NPC(victim) && !IS_NPC(ch)) 
			ch->kills[MK]++;
		if (!IS_NPC(victim) && IS_NPC(ch))
			victim->kills[MD]++;

		if (!IS_NPC(victim) && !IS_NPC(ch) && ch != victim){
			ch->kills[PK]++;
			victim->kills[PD]++;
		}
		stop_fighting(ch,true);
	}

	death_cry(victim);

	tattoo = get_eq_char(victim, WEAR_TATTOO);
	if (tattoo != NULL) 
		obj_from_char(tattoo);

	make_corpse(victim);

	if (IS_NPC(victim)){
		victim->pIndexData->killed++;
		kill_table[URANGE(0,victim->level,MAX_LEVEL-1)].killed++;
		extract_char(victim,true);
		return;
	}

	extract_char(victim,false);

	if (tattoo){
		obj_to_char(tattoo, victim);
		equip_char(victim, tattoo, WEAR_TATTOO);
	}

	while (victim->affected){
		affect_remove(victim,victim->affected);
		victim->massaff(race_table[victim->race].aff);
	}

	for (i = 0; i < 4; i++)
		victim->armor[i]= 100;
	victim->position	= POS_RESTING;
	victim->hit			= UMAX(1,victim->hit);
	victim->setmana(UMAX(1,victim->getmana()));
	victim->move		= UMAX(1,victim->move);
	/*  cql_save_char( victim ); we're stable enough to not need this :) */
}

int gen_total_damage(CHAR_DATA *victim){
	CHAR_DATA *gch;
	int tdam = 0;
	MEM_DATA *remember;
	if(!IS_NPC(victim))
		return 0;
	for(gch = char_list;gch;gch = gch->next){//generate total damage
		if((remember = get_mem_data(victim,gch))){//they did damage to the mob at some time.
			tdam += remember->dam;
		}
	}
	if(tdam < 1){
		log_f("BUG: %s sucks at getting killed.",victim->name);
		return 0;
	}
	return tdam;
}

int gen_group_damage(CHAR_DATA *ch,CHAR_DATA *victim){
	CHAR_DATA *gch;
	int gdam = 0;
	MEM_DATA *remember;
	if(!IS_NPC(victim))
		return 0;
	for(gch = char_list;gch;gch = gch->next){//generate group damage
		if(is_same_group(gch,ch) && (remember = get_mem_data(victim,gch))){
			//sprintf(buf,"%d",remember->dam);do_function(victim,&do_say,buf);
			gdam += remember->dam;
		}
	}
	return gdam;
}

void group_gain(CHAR_DATA *ch,CHAR_DATA *victim){
	char buf[MSL];
	CHAR_DATA *gch,*lch,*vch;
	int xp,members = 0,group_levels = 0,tdam = 0,gdam = 0;
	/*
	* Monsters don't get kill xp's or alignment changes.
	* P-killing doesn't help either.
	* Dying of mortal wounds or poison doesn't give xp to anyone!
	*/
	if(!ch || !victim || victim == ch)
		return;

	tdam = gen_total_damage(victim);

	for(vch = victim->in_room->people;vch;vch = vch->next_in_room){
		if(IS_NPC(vch))
			continue;
		group_levels = 0;
		members = 0;
		for(gch = victim->in_room->people;gch;gch = gch->next_in_room){
			if(is_same_group(gch,vch)){
				members++;
				group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
			}
		}
		if(members == 0){
			bug("Group_gain: members.",members);
			members = 1;
			group_levels = vch->level;
		}

		lch = get_leader(vch);
		gdam = gen_group_damage(vch,victim);

		if(vch->level - lch->level > 10){
			send_to_char("You are too high for this group.\n\r",vch);
			continue;
		}
		if(vch->level - lch->level < -10){
			send_to_char("You are too low for this group.\n\r",vch);
			continue;
		}
		if (vch->iscomm(CM_DEBUG))printf_to_char(vch,"%d %d %d %d %d\n\r",vch->level,victim->level,group_levels,gdam,tdam);
		xp = 2 * xp_compute(vch,victim,group_levels / members,gdam,tdam);//doubled so it doesn't blow ass
		if (vch->iscomm(CM_DEBUG))printf_to_char(vch,"Xp Dbl2 {G%d{x.\n\r", xp );
		xp = UMAX(xp -= xp/2 * (10 * (members - 1)) / 100,xp/2);// might not be enough cut
		if(vch->iscomm(CM_DEBUG))printf_to_char(vch,"Xp Member: {G%d{x.\n\r",xp);
		if(!IS_NPC(vch) && !check_tierlevel(vch))
			xp = 0;
		if(xp != 0){
			printf_to_char(vch,"You receive {Y%d {xexperience points.\n\r",xp);
			gain_exp(vch,xp);
		}
	}
}
/*
* Compute xp for a kill.
* Also adjust alignment of killer.
* Edit this function to change xp computations.
*/
int xp_compute(CHAR_DATA *gch,CHAR_DATA *victim,int total_levels,int gdam,int tdam){
	int xp,base_exp,align,level_range,change,time_per_level;

	level_range = victim->level - gch->level;

	if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"LevelRange {R%d{x. ",level_range);
	if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"VCHLVL {R%d{x. ",victim->level);
	if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"TLVL {R%d{x.\n\r",total_levels);
	if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"DAM {R%d %d{x.\n\r",gdam,tdam);

	/* compute the base exp */
	switch (level_range){
		default : base_exp = 0;   break;
		case -9 : base_exp = 10;   break;
		case -8 : base_exp = 20;   break;
		case -7 : base_exp = 30;   break;
		case -6 : base_exp = 40;   break;
		case -5 : base_exp = 50;  break;
		case -4 : base_exp = 60;  break;
		case -3 : base_exp = 70;  break;
		case -2 : base_exp = 80;  break;
		case -1 : base_exp = 90;  break;
		case  0 : base_exp = 100;  break;
		case  1 : base_exp = 120;  break;
		case  2 : base_exp = 140; break;
		case  3 : base_exp = 160; break;
		case  4 : base_exp = 180; break;
	}
	if (level_range > 4)
		base_exp = 160 + 20 * (level_range - 4);

	/* calculate exp multiplier */
		xp = base_exp;
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp {G%d{x.\n\r", xp );

		if(IS_NPC(victim))
			xp = xp * gdam / tdam;
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp Percent {G%d{x.\n\r", xp );

	/* more exp at the low levels */
	if (gch->level < 10)
		xp = 10 * xp / (gch->level + 4);
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp Nwb {G%d{x.\n\r", xp );

	/* less at high */
//	if (gch->level > 35 )
//		xp = 15 * xp / (gch->level - 25 );
	/* reduce for playing time */
/*	{
		// compute quarter-hours per level
		time_per_level = 4 * (gch->played + (int) (current_time - gch->logon))/3600 / gch->level;
		time_per_level = URANGE(2,time_per_level,12);
		if (gch->level < 15) // make it a curve 
			time_per_level = UMAX(time_per_level,(15 - gch->level));
		xp = xp * time_per_level / 12;
	}*/

	/* randomize the rewards */
	xp = number_range (xp * 3/4, xp * 5/4);
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp Rnd {G%d{x.\n\r", xp );

	xp = (xp * gch->perm_stat[STAT_INT]) / 16;
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp Int {G%d{x.\n\r", xp );

	/* adjust for grouping */
	xp = xp * gch->level / (UMAX(1,total_levels - 1));
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp gchlvl {G%d{x Xp Div {G%d{x.\n\r",gch->level,UMAX(1,total_levels - 1));
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp Grp {G%d{x.\n\r", xp );

	if(double_exp)
		xp *= 2;
		if (gch->iscomm(CM_DEBUG))printf_to_char(gch,"Xp Dbl {G%d{x.\n\r", xp );
	return xp;
}


void dam_message(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,bool immune){
	CHAR_DATA *vch;
    char buf1[256],buf2[256],buf3[256],buf4[50];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
		return;

		 if ( dam <=-500 ) { vs = "{Wrevitalize{x";								vp = "{Crevitalizes{x";								}
	else if ( dam <=-250 ) { vs = "{Crestore{x";								vp = "{Crestores{x";								}
	else if ( dam <=-200 ) { vs = "{cmend{x";									vp = "{cmends{x";									}
	else if ( dam <=-150 ) { vs = "{Gheal{x";									vp = "{Gheals{x";									}
	else if ( dam <=-100 ) { vs = "{gcure{x";									vp = "{gcures{x";									}
	else if ( dam <= -50 ) { vs = "{Maid{x";									vp = "{Maids{x";									}
	else if ( dam <    0 ) { vs = "{msoothe{x";									vp = "{msoothes{x";									}
	else if ( dam ==   0 ) { vs = "{ddo nothing {xto";							vp = "{ddoes nothing {xto";							}
    else if ( dam <=   5 ) { vs = "{yscratch{x";								vp = "{yscratches{x";								}
    else if ( dam <=   8 ) { vs = "{ygraze{x";									vp = "{ygrazes{x";									}
    else if ( dam <=  12 ) { vs = "{chit{x";									vp = "{chits{x";									}
    else if ( dam <=  16 ) { vs = "{cinjure{x";									vp = "{cinjures{x";									}
    else if ( dam <=  23 ) { vs = "{gwound{x";									vp = "{gwounds{x";									}
    else if ( dam <=  28 ) { vs = "{gdecimate{x";								vp = "{gdecimates{x";								}
    else if ( dam <=  34 ) { vs = "{GMUTILATE{x";         						vp = "{GMUTILATES{x";								}
    else if ( dam <=  44 ) { vs = "{GDISEMBOWEL{x";								vp = "{GDISEMBOWELS{x";								}
    else if ( dam <=  54 ) { vs = "{GDISMEMBER{x";								vp = "{GDISMEMBERS{x";								}
    else if ( dam <=  65 ) { vs = "{rMASSACRE{x";								vp = "{rMASSACRES{x";								}
    else if ( dam <=  76 ) { vs = "{rMANGLE{x";									vp = "{rMANGLES{x";									}
    else if ( dam <=  87 ) { vs = "{w*** {gDEMOLISH {w***{x";					vp = "{w*** {gDEMOLISHES {w***{x";					}
    else if ( dam <=  97 ) { vs = "{R*** {wDEVASTATE {R***{x";					vp = "{R*** {wDEVASTATES {R***{x";					}
    else if ( dam <= 109 ) { vs = "{r={w={r= {ROBLITERATE {r={w={r={x";			vp = "{r={w={r= {ROBLITERATES {r={w={r={x";			}
    else if ( dam <= 121 ) { vs = "{r>{w>{r> {RANNIHILATE {r<{w<{r<{x";			vp = "{r>{w>{r> {RANNIHILATES {r<{w<{r<{x";			}
    else if ( dam <= 135 ) { vs = "{y-{w= {YBUTCHER {w={y-{x";					vp = "{y-{w= {YBUTCHERS {w={y-{x";					}
    else if ( dam <= 155 ) { vs = "{c<{w<{c< {wERADICATE {c>{w>{c>{x";			vp = "{c<{w<{c< {wERADICATES {c>{w>{c>{x";			}
    else if ( dam <= 179 ) { vs = "{Y*{y*{Y*{y* {rRAVAGE {y*{Y*{y*{Y*{x";		vp = "{Y*{y*{Y*{y* {rRAVAGES {y*{Y*{y*{Y*{x";		}
    else if ( dam <= 200 ) { vs = "do {RUNSPEAKABLE {xthings to";				vp = "does {RUNSPEAKABLE {xthings to";				}
    else if ( dam <= 250 ) { vs = "{w--{y= {REVISCERATE {y={w--{x";				vp = "{w--{y= {REVISCERATES {y={w--{x";				}
    else if ( dam <= 325 ) { vs = "{w---{y= {rLACERATE {y={w---{x";				vp = "{w---{y= {rLACERATES {y={w---{x";				}
    else if ( dam <= 420 ) { vs = "{R<{r({d--{y= {YSLAUGHTER {y={d--{r){R>{x";	vp = "{R<{r({d--{y= {YSLAUGHTERS {y={d--{r){R>{x";	}
    else if ( dam <= 550 ) { vs = "{r+----== {dSHATTER {r==----+{x";			vp = "{r+----== {dSHATTERS {r==----+{x";			}
    else                   { vs = " {RDestroy {x";							vp = " {RDestroys {x";							}

    punct   = (dam <= 24) ? '.' : '!';

	sprintf(buf1,"BUGGY!!!!!!!!!!!!\n\r");
	if(dam >= 0)
		sprintf(buf4,"  {x[{dDamage {R%d{x]",dam);
	else
		sprintf(buf4,"  {x[{dHeal {R%d{x]",-dam);
    if ( dt == TYPE_HIT ){
		if (ch == victim){
			sprintf(buf1,"$n %s $mself%c",vp,punct);
			sprintf(buf2,"You %s yourself%c",vs,punct);
		}
		else{
			sprintf(buf2,"You %s %s%c%s",vs,capitalize(PERS(victim,ch)),punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
			sprintf(buf3,"%s %s you%c%s",capitalize(PERS(ch,victim)),vp,punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
		}
    }
	if ( dt >= 0 && dt < MAX_SKILL )
		attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
		attack	= attack_table[dt - TYPE_HIT].noun;
	else{
		bug( "Dam_message: bad dt %d.", dt );
		dt  = TYPE_HIT;
		attack  = attack_table[0].name;
	}
    if(dt != TYPE_HIT){
		if (immune){
			if (ch == victim)
				sprintf(buf2,"Luckily, you are immune to that.");
			else{
	    		sprintf(buf2,"%s is unaffected by your %s!%s",capitalize(PERS(victim,ch)),attack,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
	    		sprintf(buf3,"%s's %s is powerless against you.%s",capitalize(PERS(ch,victim)),attack,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
			}
		}
		else{
			if (ch == victim){
				sprintf(buf1,"$n's %s %s $m%c%s",attack,vp,punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
				sprintf(buf2,"Your %s %s you%c%s",attack,vp,punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
			}
			else{
	    		sprintf(buf2,"Your %s %s %s%c%s",  attack, vp,PERS(victim,ch),punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
	    		sprintf(buf3,"%s's %s %s you%c%s",capitalize(PERS(ch,victim)),attack,vp,punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
			}
		}
    }

    if (ch == victim){
		act(buf1,ch,NULL,NULL,TO_ROOM);
		act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else{
		if (ch->in_room == victim->in_room){
			for (vch = victim->in_room->people; vch != NULL; vch = vch->next_in_room)
				if(vch != ch && vch != victim)
					message_stuff(ch,victim,vch,immune,dam,punct,attack,vp,dt);
    		act(buf2,ch,NULL,victim,TO_CHAR);
    		act(buf3,ch,NULL,victim,TO_VICT);
		}
		else{
			if(dt == TYPE_HIT){
    			act(buf2,ch,NULL,victim,TO_CHAR);
    			act(buf3,ch,NULL,victim,TO_VICT);
				for ( vch = victim->in_room->people; vch != NULL; vch = vch->next_in_room){
					if (vch != victim){
						sprintf(buf1,"%s %s %s%c%s\n\r",capitalize(PERS(ch,vch)),vp,can_see(vch,victim) ? IS_NPC(victim) ? capitalize(victim->short_descr) : victim->name : "someone", punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
						send_to_char(buf1,vch);
					}
				}
				for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room){
					if (vch != ch){
						sprintf(buf1,"%s %s %s%c%s\n\r",capitalize(PERS(ch,vch)),vp,can_see(vch,victim) ? IS_NPC(victim) ? capitalize(victim->short_descr) : victim->name : "someone", punct,/*ch->iscomm(CM_DEBUG) ? */buf4/* : ""*/);
						send_to_char(buf1,vch);
					}
				}
			}
			else{
    			act( buf2, ch, NULL, victim, TO_CHAR );
    			act( buf3, ch, NULL, victim, TO_VICT );
				for ( vch = victim->in_room->people; vch != NULL; vch = vch->next_in_room)
					if (vch != victim)
						message_stuff(ch,victim,vch,immune,dam,punct, attack, vp,dt);
				for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
					if (vch != ch)
						message_stuff(ch,victim,vch,immune,dam,punct, attack, vp,dt);
			}
		}
		if(!IS_NPC(ch) && ch->isplr(PL_ARENA) && !IS_NPC(victim) && victim->isplr(PL_ARENA)){
			ROOM_INDEX_DATA *foyerroom = get_room_index(ch->foyervnum);
			ROOM_INDEX_DATA *viewerroom = get_room_index(foyerroom->arenaviewvn);
			for (vch = viewerroom->people; vch != NULL; vch = vch->next_in_room)
				if(vch != ch && vch != victim)
					message_stuff(ch,victim,vch,immune,dam,punct, attack, vp,dt);
		}
    }
}

void message_stuff(CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA *vch, bool immune,int dam,char punct,const char *attack,const char *vp,int dt){
	char buf[MSL],buf2[100];

	if(dam >= 0)
		sprintf(buf2,"{x[{dDamage {R%d{x]",dam);
	else
		sprintf(buf2,"{x[{dHeal {R%d{x]",-dam);
	if (immune){
		if (ch == victim)
			sprintf(buf,"%s is unaffected by %s own %s.\n\r",capitalize(PERS(ch,vch)),sex_table[ch->sex].sword,attack);
		else
			sprintf(buf,"%s's %s doesn't affect %s!  %s\n\r",capitalize(PERS(ch,vch)),attack,PERS(victim,vch),buf2);
	}
	else{
		if (dt != TYPE_HIT){
			if (ch == victim)
				sprintf(buf,"%s's %s %s %s%c\n\r",capitalize(PERS(ch,vch)),attack,vp,sex_table[ch->sex].mword,punct);
			else
				sprintf(buf,"%s's %s %s %s%c  %s\n\r",capitalize(PERS(ch,vch)),attack,vp,PERS(victim,vch),punct,buf2);
		}
		else{
			if (ch == victim)
				sprintf(buf,"%s %s %s%c\n\r",capitalize(PERS(ch,vch)),vp,sex_table[ch->sex].mword,punct);
			else
				sprintf(buf,"%s %s %s%c  %s\n\r",capitalize(PERS(ch,vch)),vp,PERS(victim,vch),punct,buf2);
		}
	}
	send_to_char(buf,vch);
	return;
}

void get_new_killer(CHAR_DATA *ch,CHAR_DATA *oldfight){
	CHAR_DATA *fch;
	for (fch = ch->in_room->people;fch;fch = fch->next_in_room)
		if (fch->fighting == ch && fch != oldfight){
			ch->fighting = fch;
			return;
		}
}

bool victim_die(CHAR_DATA *ch,CHAR_DATA *victim){
	OBJ_DATA *corpse;

	victim->death_timer = 0;

	if(ch){
		sprintf( log_buf, "%s killed by %s at %d", victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name), ch->in_room->vnum );

		if (!IS_NPC(ch) && !IS_NPC(victim)){
			sprintf( log_buf, "%s got fragged by %s at %s [room %d]",(IS_NPC(victim) ? victim->short_descr : victim->name),(IS_NPC(ch) ? ch->short_descr : ch->name),ch->in_room->name, ch->in_room->vnum);
			wiznet(log_buf,NULL,NULL,WZ_PKS,0,0);
		}
		else{
			sprintf(log_buf,"%s got toasted by %s at %s [room %d]",
				(IS_NPC(victim) ? victim->short_descr : victim->name),
				(IS_NPC(ch) ? ch->short_descr : ch->name),
				ch->in_room->name,ch->in_room->vnum);

			if (IS_NPC(victim))
				wiznet(log_buf,NULL,NULL,WZ_MOBDEATHS,0,0);
			else
				wiznet(log_buf,NULL,NULL,WZ_DEATHS,0,0);
		}
		/* dump the flags */
		if (ch != victim && !IS_NPC(ch))
			if (victim->isplr(PL_KILLER))
				victim->remplr(PL_KILLER);
	}
	else{
		sprintf( log_buf, "%s died of mortal wounds at %d", victim->name, victim->in_room->vnum );
		if (IS_NPC(victim))
			wiznet(log_buf,NULL,NULL,WZ_MOBDEATHS,0,0);
		else
			wiznet(log_buf,NULL,NULL,WZ_DEATHS,0,0);
	}

	if(!IS_NPC(victim)){
		if(victim->isplr(PL_ARENA)){
			if(ch)
				end_duel(ch,victim);
			else{
				log_f("%s got shafted in the arena.",victim->name);
				victim->send("You got arena shafted.\n\r");
			}
			return true;
		}

		/* Dying penalty:
		 * 2/3 way back to previous level.*/
		if (victim->exp > exp_per_level(victim))
			gain_exp(victim,(2 * (exp_per_level(victim) * victim->level - victim->exp)/3) + 50);
	}

	group_gain(ch,victim);

	raw_kill(ch,victim);


	/* RT new auto commands */
	if(ch){
		/* Death trigger*/
		if (IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_DEATH)){
			victim->position = POS_STANDING;
			p_percent_trigger(victim,NULL,NULL,ch,NULL,NULL,TRIG_DEATH);
		}

		if (!IS_NPC(ch) && (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL && corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse)){
			OBJ_DATA *coins;

			corpse = get_obj_list(ch,"corpse",ch->in_room->contents);

			if (ch->isplr(PL_AUTOLOOT) && corpse && corpse->contains) /* exists and not empty */
				do_function(ch, &do_get, "all corpse");

			if (ch->isplr(PL_AUTOGOLD) && corpse && corpse->contains && !ch->isplr(PL_AUTOLOOT)){//exists and not empty
				if ((coins = get_obj_list(ch,"gcash",corpse->contains)) != NULL)
					do_function(ch, &do_get, "all.gcash corpse");
			}

			if (ch->isplr(PL_AUTOSAC)){
       			if (ch->isplr(PL_AUTOLOOT) && corpse && corpse->contains)
					return true;  /* leave if corpse has treasure */
				else
					do_function(ch, &do_sacrifice, "corpse");
			}
		}
	}
	return true;
}

void do_die(CHAR_DATA *ch,char *argument){
	if(ch->position != POS_MORTAL){
		ch->send("You are not mortally wounded.\n\r");
		return;
	}
	ch->send("You let go of the last strands of your life and slip painfully into death.\n\r");
	act("$n suddenly seems peaceful as the last of $s life leaves $m.",ch,NULL,NULL,TO_ROOM);
	act("$n is {RDEAD{x!!",ch,0,0,TO_ROOM);
	ch->send("You are {RDEAD{x!\n\r");
	victim_die(NULL,ch);
}
