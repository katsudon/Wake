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
void check_readied		( CHAR_DATA* );
bool is_weapon_pierce	( OBJ_DATA* );
bool check_sidestep		( CHAR_DATA*,CHAR_DATA*,OBJ_DATA* );
bool check_evade		( CHAR_DATA*,CHAR_DATA* );
bool check_guard		( CHAR_DATA*,CHAR_DATA* );
bool check_feint		( CHAR_DATA*,CHAR_DATA* );
void raw_kill			( CHAR_DATA*,CHAR_DATA* );

void do_charge(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	OBJ_DATA *obj;
	CHAR_DATA *victim = NULL;
	ROOM_INDEX_DATA *was_in_room;
	EXIT_DATA *pexit;
	int skill, chance, dam, door, sdoor;

	skill = get_weapon_skill(ch,get_weapon_sn(ch,false));
	chance = get_skill(ch,gsn_charge) * .5;
	sdoor = 0;
	one_argument(argument,arg);

	if (!arg[0]){
		send_to_char("Charge whom?\n\r",ch);
		return;
	}

	if (!MOUNTED(ch))
	{
		send_to_char("You must be on horseback to charge!\n\r", ch);
		return;
	}

	if (ch->fighting){
		send_to_char("They're too close!\n\r",ch);
		return;
	}
	else{
		if (!(victim = get_char_room(ch,NULL,arg))){
			if (!(victim = get_char_around(ch,arg,2,false))){
				send_to_char("You can't find it.\n\r",ch);
				return;
			}
			else{
				if(IS_SET(pexit->exit_info,EX_CLOSED)){
					send_to_char("You can't charge through a door.\n\r",ch);
					return;
				} 
				if(IS_SET(pexit->exit_info,EX_CLIFFTOP)){
					send_to_char("That'd be suicide!.\n\r",ch);
					return;
				} 
			}
		}
	}


	if (victim == ch){
		send_to_char("Your lance can't bend that way..\n\r",ch);
		return;
	}

	if (is_safe(ch,victim))
	  return;

	if (!(obj = get_eq_char(ch,WEAR_WIELD))){
		send_to_char("You can't charge barehanded!\n\r",ch);
		return;
	}

	if ( victim->hit < victim->max_hit / 5){
		act("$N is weak... have some more honor than that!",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (obj->value[0] != WEAPON_LANCE){
		chance -= 10;
		dam = dice(obj->value[1],obj->value[2]) * 2;
	}
	else
		dam = dice(obj->value[1] * 2,obj->value[2] * 2);

	chance += (5 * get_skill(ch,gsn_horsemanship))/100;

	if (check_lancemastery(ch))
		dam *= 1.5;

	WAIT_STATE(ch,skill_table[gsn_charge].beats);
	if (number_percent() < chance && !check_evade(ch,victim)){
		if((ch->in_room) != (victim->in_room)){
			act("You gallop $T, weapon aimed at your target.",ch,NULL,dir_name[sdoor],TO_CHAR);
			act("$n gallops $T, $s weapon forward.",ch,NULL,dir_name[sdoor],TO_ROOM);
			move_char(ch,sdoor,true,false);
			act("$n gallops in, $s weapon aimed at $N!",ch,NULL,victim,TO_ROOM);
		}
		else{
			act("You charge weapon first at $N!",ch,NULL,victim,TO_CHAR);
			act("$n charges full speed at $N, $s weapon first!",ch,NULL,victim,TO_NOTVICT);
			act("$n charges full speed at you, $s weapon first!",ch,NULL,victim,TO_VICT);
		}
		check_improve(ch,gsn_charge,true,1);
		//one_hit(ch,victim,gsn_charge,false);
		skill_damage(ch,victim,dam,gsn_charge,DAM_PIERCE,true);
		skill_damage(ch,victim,dam/6,gsn_charge,DAM_BASH,false);
		chance = number_percent();
		if (chance < 60){
			if (chance < 35){
				if (chance < 10){
					act("$p breaks off inside $N!",ch,obj,victim,TO_CHAR);
					act("$p breaks off inside you!",ch,obj,victim,TO_VICT);
					act("$p breaks off inside $N!",ch,obj,victim,TO_NOTVICT);
					obj_from_char(obj);
					extract_obj(obj);
				}
				else{
					act("$p shatters against $N!",ch,obj,victim,TO_CHAR);
					act("$p shatters against you!",ch,obj,victim,TO_VICT);
					act("$p shatters against $N!",ch,obj,victim,TO_NOTVICT);
					obj_from_char(obj);
					extract_obj(obj);
				}
			}
			else if (!IS_OBJ_STAT(obj,ITM_NODROP)){
				act("You release $p!",ch,obj,NULL,TO_CHAR);
				act("$n releases $p!",ch,obj,NULL,TO_ROOM);
				obj_from_char(obj);
				obj_to_room(obj,ch->in_room);
			}
			else{
				act( "$n waves $s hand around, but $s lance seems stuck!", ch, obj, victim, TO_ROOM );
				act( "You can't drop your lance!", ch, obj, victim, TO_CHAR );
				return;
			}
		}
		check_readied(ch);
	}
	else{
		if((ch->in_room) != (victim->in_room)){
			act("You gallop $T, weapon aimed at your target and miss!.",ch,NULL,dir_name[sdoor],TO_CHAR);
			act("$n gallops $T, $s weapon forward.",ch,NULL,dir_name[sdoor],TO_ROOM);
			move_char(ch,sdoor,true,false);
			act("$n gallops in, $s weapon aimed at $N and misses!",ch,NULL,victim,TO_ROOM);
		}
		else{
			act("You charge weapon first at $N and miss!",ch,NULL,victim,TO_CHAR);
			act("$n charges full speed at $N, $s weapon first and misses!",ch,NULL,victim,TO_NOTVICT);
			act("$n charges full speed at you, $s weapon first and misses!",ch,NULL,victim,TO_VICT);
		}
		check_improve(ch,gsn_charge,false,1);
		skill_damage(ch,victim,0,gsn_charge,DAM_PIERCE,true);
	}
}

void check_readied(CHAR_DATA *ch){
    int chance;
	bool found=false;
    OBJ_DATA *obj,*readywield;

    if (!IS_AWAKE(ch))
		return;

	if (get_eq_char(ch,WEAR_WIELD))
		return;

	if (number_percent() > get_skill(ch,gsn_ready) *.95)
		return;

    for ( obj = ch->carrying; obj && !found && !readywield; obj = obj->next_content )
		if (IS_WEAPON_STAT(obj,WPN_READIED))
			readywield = obj;

	if (found){
		wear_obj(ch,readywield,true,false);
		act("$n grasps $p.",ch,readywield,NULL,TO_ROOM);
		act("You grasp $p.",ch,readywield,NULL,TO_CHAR);
	}
}

void check_dehorsing(CHAR_DATA *victim){
	if (number_percent() < 10)
		return;

    if (MOUNTED(victim)){
		act("You are dehorsed from $N.",victim,NULL,victim->mount,TO_CHAR);
		victim->mounted = false;
		victim->mount->mounted = false;
	}
}

void do_shift(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char arg[MSL];
	int skill = get_skill(ch,gsn_shift) * .75;

	if (skill < 1){
		ch->send("You're too slow to do this.\n\r");
		return;
	}

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("It doesn't work that way.\n\r");
		return;
	}
	else if (!(victim = get_char_room(ch,NULL,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (victim == ch->fighting){
		ch->send("You're already fighting them.\n\r");
		return;
	}

	if (victim->fighting != ch){
		ch->send("They're not fighting you.\n\r");
		return;
	}

	skill = skill * get_curr_stat(ch,STAT_AGI) / STAT_MAX;
	skill -= ((get_curr_stat(victim,STAT_WIS) * get_curr_stat(victim,STAT_AGI)) / STAT_MAX) / 4;

	WAIT_STATE(ch,skill_table[gsn_shift].beats);
	if (number_percent() >= skill){
		ch->send("You failed!\n\r");
		check_improve(ch,gsn_shift,false,2);
	}
	else{
		act("$n shifts to fighting $N.",ch,NULL,victim,TO_NOTVICT);
		act("$n shifts $s attacks to you!",ch,NULL,victim,TO_VICT);
		act("You shift your target to $N.",ch,NULL,victim,TO_CHAR);

		check_improve(ch,gsn_shift,true,1);
		ch->fighting = victim;
		multi_hit(ch,victim,TYPE_UNDEFINED,false);
	}
}

bool do_trample(CHAR_DATA *victim){
	CHAR_DATA *pch,*pch_next,*horse;
	int chance,dam;
	bool found=false;

	if(victim && victim->in_room)
	for(pch=victim->in_room->people;pch;pch=pch_next){
		pch_next = pch->next_in_room;
		if ((horse = MOUNTED(pch))){
			if ((chance = get_skill(pch,gsn_trample)*.25 - ((horse->level - pch->level)/4)) > 0)
			if (saves_skill(horse->level*1.25,victim,DAM_PIERCE))
				chance /= 2;
			if (number_percent() < chance){
				found=true;
				dam = (dice(horse->damage[DICE_NUMBER],horse->damage[DICE_TYPE]) + horse->damage[DICE_BONUS]) *3;
				act("$n's steed tramples you after you hit the ground!",pch,NULL,victim,TO_VICT);
				act("$n's steed tramples $N after $E hits the ground!",pch,NULL,victim,TO_NOTVICT);
				act("Your steed tramples $N after $E hits the ground!",pch,NULL,victim,TO_CHAR);
				skill_damage(horse,victim,dam,gsn_trample,DAM_PIERCE,true);
				check_improve(pch,gsn_trample,true,4);
			}
			else
				check_improve(pch,gsn_trample,false,4);
		}
	}
	return found;
}

void do_pierce(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	char arg[MSL];
	int skill = get_skill(ch,gsn_pierce) * .75,dam = 0;

	one_argument(argument,arg);

	if (skill < 1){
		ch->send("You... wait what were you doing again?\n\r");
		return;
	}

	if (!arg[0]){
		victim = ch->fighting;
		if (!victim){
			send_to_char("But you aren't fighting anyone!\n\r",ch);
			return;
 		}
	}
	else if (!(victim = get_char_room(ch,NULL,arg))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if (!wield)
	{
		ch->send("What... with your hands?\n\r");
		return;
	}

	if (!is_weapon_big_pierce(wield))
	{
		ch->send("You can only use large piercing wields for this skill.\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_pierce].beats);
	if (number_percent() < skill)
	{
		dam = dice(wield->value[1],wield->value[2]);
		dam *= victim->size;
		check_improve(ch,gsn_pierce,true,2);
		act("You slam $p into $N's chest!",ch,wield,victim,TO_CHAR);
		act("$n slams $p into your chest!",ch,wield,victim,TO_VICT);
		act("$n slams $p into $N's chest!",ch,wield,victim,TO_NOTVICT);
	}
	else
	{
		act("You thrust $p at $N but miss!",ch,wield,victim,TO_CHAR);
		act("$n thrusts $p at you and misses!",ch,wield,victim,TO_VICT);
		act("$n thrusts $p at $N and misses!",ch,wield,victim,TO_NOTVICT);
		check_improve(ch,gsn_pierce,false,3);
	}
	skill_damage(ch,victim,dam,gsn_pierce,DAM_PIERCE,true);
}

void check_deathblow(CHAR_DATA *ch,CHAR_DATA *victim){
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	int perc = (victim->hit * 100) / victim->max_hit;

	if (!MOUNTED(ch) || perc > 11 || !wield || wield->item_type != ITEM_WEAPON || IS_IMMORTAL(victim) || !canFight(ch,victim,false))
		return;

	int chance = get_skill(ch,gsn_deathblow) * .25;

	chance = calcReflex(victim,ch,chance);

	if (number_percent() > chance){
		switch(wield->value[0]){
		case WEAPON_LONGSWORD:
		case WEAPON_RAPIER:
		case WEAPON_KATANA:
		case WEAPON_MACHETE:
			act("With a mighty swing of $p, you lob $N's head off!",ch,wield,victim,TO_CHAR);
			act("$n charges at $N, and with a mighty swing of $p, lobs $S head off!",ch,wield,victim,TO_NOTVICT);
			act("The last thing you see is $n charge at you, the glimmering blade of $p as it flies through your neck!",ch,wield,victim,TO_VICT);
			skill_damage(ch,victim,victim->max_hit*2,gsn_deathblow,DAM_OTHER,true);
			raw_kill(ch,victim);
			break;
		case WEAPON_AXE:
		case WEAPON_SCIMITAR:
		case WEAPON_BASTARDSWORD:
		case WEAPON_BATTLEAXE:
			act("Your horse leaps in the air, and you fall, bringing $p down hard, splitting $N's head open!",ch,wield,victim,TO_CHAR);
			act("$n spurs $s mount to leap in the air, and as they fall $e brings $p down, splitting $N's head open!",ch,wield,victim,TO_NOTVICT);
			act("You see $n and $s mount leap in the air, and as they fall $e brings $p down through your skull!",ch,wield,victim,TO_VICT);
			skill_damage(ch,victim,victim->max_hit*2,gsn_deathblow,DAM_OTHER,true);
			raw_kill(ch,victim);
			break;
		case WEAPON_LANCE:
		case WEAPON_POLEARM:
		case WEAPON_SPEAR:
			act("You charge forward, impaling $N through the heart with $p and tossing $M aside.",ch,wield,victim,TO_CHAR);
			act("$n charges forward, impaling $N through the heart with $p and tossing $M aside.",ch,wield,victim,TO_NOTVICT);
			act("$n charges forward, impaling you through the heart with $p and tossing you aside.",ch,wield,victim,TO_VICT);
			skill_damage(ch,victim,victim->max_hit*2,gsn_deathblow,DAM_OTHER,true);
			raw_kill(ch,victim);
			break;
		case WEAPON_MACE:
		case WEAPON_FLAIL:
		case WEAPON_QUARTERSTAFF:
		case WEAPON_HAMMER:
		case WEAPON_WARHAMMER:
		case WEAPON_STAFF:
			act("You swing $p in an arc, bringing it down on $N's skull, shattering $S spine!",ch,wield,victim,TO_CHAR);
			skill_damage(ch,victim,victim->max_hit*2,gsn_deathblow,DAM_OTHER,true);
			raw_kill(ch,victim);
			break;
		case WEAPON_SCYTHE:
			do_function(ch,&do_say,"Die!");
			act("You drag $p into a series of powerful spins, slicing $N to tiny bloody ribbons!",ch,wield,victim,TO_CHAR);
			act("$n drags $p into a series of powerful spins, slicing $N to tiny bloody ribbons!",ch,wield,victim,TO_NOTVICT);
			act("$n spins $p towards you, its blade becoming a blur as it slices through you repeatedly!",ch,wield,victim,TO_VICT);
			skill_damage(ch,victim,100,gsn_cleave,DAM_OTHER,true);
			skill_damage(ch,victim,100,gsn_cleave,DAM_OTHER,true);
			skill_damage(ch,victim,100,gsn_cleave,DAM_OTHER,true);
			skill_damage(ch,victim,100,gsn_cleave,DAM_OTHER,true);
			skill_damage(ch,victim,100,gsn_cleave,DAM_OTHER,true);
			skill_damage(ch,victim,victim->max_hit*2,gsn_deathblow,DAM_OTHER,true);
			raw_kill(ch,victim);
			act("You pull $p up into a silent salute.",ch,wield,NULL,TO_CHAR);
			act("$n pulls $p out of the spin and silently salutes.",ch,wield,NULL,TO_NOTVICT);
			break;
		default:
			break;
		}
		check_improve(ch,gsn_deathblow,true,5);
	}
	else
		check_improve(ch,gsn_deathblow,false,4);
}

void check_outmaneuver(CHAR_DATA *ch){
}

bool valid_swordhorse(CHAR_DATA *ch){
	if(!ch->mounted)
		return false;
	return true;
}

void do_thrust(CHAR_DATA *ch,char *argument){
}

void do_cavalry(CHAR_DATA *ch,char *argument){
}

void do_leverage(CHAR_DATA *ch,char *argument){
}

void do_numbing_force(CHAR_DATA *ch,char *argument){
}

//void do_withdraw(CHAR_DATA *ch,char *argument){
//}

void do_concussive_blow(CHAR_DATA *ch,char *argument){
}

void do_sword_blitz(CHAR_DATA *ch,char *argument){
}

void do_cranial_strike(CHAR_DATA *ch,char *argument){
}

void do_slashing_volley(CHAR_DATA *ch,char *argument){
}

void do_rising_cleave(CHAR_DATA *ch,char *argument){
}

void do_divider(CHAR_DATA *ch,char *argument){
}

void do_lop(CHAR_DATA *ch,char *argument){
}

void do_fissure(CHAR_DATA *ch,char *argument){
}

void do_rift(CHAR_DATA *ch,char *argument){
}

void do_hack(CHAR_DATA *ch,char *argument){
}

void do_breach(CHAR_DATA *ch,char *argument){
}

void do_destrier(CHAR_DATA *ch,char *argument){
}

void do_spiraling_spear(CHAR_DATA *ch,char *argument){
}

void do_spur(CHAR_DATA *ch,char *argument){
}

void do_equestrian(CHAR_DATA *ch,char *argument){
}

void do_courser(CHAR_DATA *ch,char *argument){
}

void do_charger(CHAR_DATA *ch,char *argument){
}

void do_barding(CHAR_DATA *ch,char *argument){
}

void do_hobby(CHAR_DATA *ch,char *argument){
}

void do_errant(CHAR_DATA *ch,char *argument){
}

void do_hotblood(CHAR_DATA *ch,char *argument){
}

void do_coldblood(CHAR_DATA *ch,char *argument){
}
