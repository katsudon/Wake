#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "lookup.h"


/* 
 * Local functions.
 */
int melee_hit(CHAR_DATA*,CHAR_DATA*,int,bool);
void shift_combo	( CHAR_DATA*,char);
bool check_evade	( CHAR_DATA*,CHAR_DATA* );
bool check_guard	( CHAR_DATA*,CHAR_DATA* );



int dual_nerf(CHAR_DATA *ch,int num,bool secondary){
	if(!get_eq_char(ch,WEAR_SECONDARY))
		return num;

	if (get_skill(ch,gsn_florentine) < 1){
		if(secondary){
			if(get_skill(ch,gsn_dual_wield) < 1)
				num /= 10;
			else
				num -= (num * (15 - ch->getslvl(gsn_dual_wield)) / 75);
		}
	}
	else{
		if (number_percent() < get_skill(ch,gsn_florentine) /2){
			check_improve(ch,gsn_florentine,true,2);
			num += (num * (ch->getslvl(gsn_florentine)*2) / 100);
		}
		else
			check_improve(ch,gsn_florentine,false,1);
		num -= (num * (10 - ch->getslvl(gsn_florentine))) / 100;
	}

	return num;
}

void do_sword_combo(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char arg[MSL];
	int dam;

	argument = one_argument(argument,arg);

	if (!(victim = grab_char(ch,arg,true)))
		return;
}

void enhanced_combo(CHAR_DATA *ch,CHAR_DATA *victim){
	int dam;
}

void critical_combo(CHAR_DATA *ch,CHAR_DATA *victim){
	int dam;
}

void do_strike(CHAR_DATA *ch,char *argument){
	int chance,hDamage;
	char arg[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	one_argument(argument,arg);

	if (!(victim = grab_char(ch,arg,true)))
		return;

	if(!canFight(ch,victim,true))
		return;

	if((chance = get_skill(ch,gsn_strike)*.85) < 1){
		ch->send("You are not quite good enough to do such a thing!\n\r");
		return;
	}

	if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL){
		send_to_char("You need to wield a weapon to strike an enemy.\n\r",ch);
		return;
	}

	if (is_weapon_pierce(obj)==false){
		send_to_char("You must use a piercing weapon to strike at your foe.\n\r",ch);
		return;
	}

	if (obj->value[0] == WEAPON_RAPIER)
		chance +=5;

	if (obj->value[0] == WEAPON_MAINGAUCHE)
		chance +=3;

	chance += get_curr_stat(ch,STAT_AGI) - get_curr_stat(victim,STAT_AGI);

    if (!can_see(victim,ch))
		chance += 5;
	
    WAIT_STATE(ch,skill_table[gsn_strike].beats * 1.25);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Strike(%d)",chance);
    if (!roll_chance(ch,chance)){
		act("$n lunges at you... and runs right by!",ch,NULL,victim,TO_VICT);
		act("You stab forward at $N... and miss!",ch,NULL,victim,TO_CHAR);
		act("$n strikes out at $N and stumbles right past $M!",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,0,gsn_strike,DAM_PIERCE,true);
		check_improve(ch,gsn_strike,false,1);
	}
	else{
		if (check_sidestep(ch,victim,obj)
		||	check_feint(ch,victim)
		||  check_duck(ch,victim,obj,true)
		||  check_mirage(ch,victim)
		||  check_evade(ch,victim)
		|| (victim->isaff(AF_PHASE) && check_phase(ch,victim))){
			skill_damage(ch,victim,0,gsn_strike,DAM_PIERCE,false);
			check_improve(ch,gsn_strike,false,1);
			return;
		}
		if (check_guard(ch,victim))
			victim = victim->guardby;
		act("$n lunges at you, weapon first!",ch,NULL,victim,TO_VICT);
		act("You stab forward at $N!",ch,NULL,victim,TO_CHAR);
		act("$n strikes out at $N!",ch,NULL,victim,TO_NOTVICT);

		hDamage = dice(obj->value[1],obj->value[2]) * 1.5;

		if (check_critical(ch,victim))
			hDamage *= 1.40;

		if(check_swordsmanship(ch,true,false))
			hDamage *= 1.2;
		else if (check_swordmastery(ch,false,false))
			hDamage *= 1.3;

		skill_damage(ch,victim,hDamage,gsn_strike,DAM_PIERCE,true);
		check_improve(ch,gsn_strike,true,1);
	}
    return;
}

void do_engarde(CHAR_DATA *ch,char *argument){
	int chance = get_skill(ch,gsn_engarde) * .9;

	if (chance < 1 || (!IS_NPC(ch) && ch->level < skill_table[gsn_engarde].skill_level[ch->pclass])){
		send_to_char("You lack the proper training to take a battle stance.\n\r",ch);
		return;
	}

	if (is_affected(ch,gsn_engarde)){
		send_to_char("You're already ready for combat.\n\r",ch);
		return;
	}

	if (ch->isaff(AF_CALM)){
		send_to_char("You're feeling to mellow to prepare for combat.\n\r",ch);
		return;
	}

	if (ch->move < ch->level/2){
		send_to_char("You're too tired to prepare for a fight.\n\r",ch);
		return;
	}

	if (ch->position == POS_FIGHTING){
		send_to_char("The fight's already begun!\n\r",ch);
		return;
	}

	/* damage -- below 50% of hp helps, above hurts */
	//hp_percent = 100 * ch->hit/ch->max_hit;
	//chance += 25 - hp_percent/2;

	if (roll_chance(ch,chance)){
		AFFECT_DATA af;

		WAIT_STATE(ch,PULSE_VIOLENCE);
		ch->move -= ch->level/2;

		send_to_char("You place your feet and ready your wield for combat, En Garde!\n\r",ch);
		act("$n takes a combat stance and shouts out, '{GEn garde!{x'",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,gsn_engarde,true,2);

		affect_set(ch,TO_AFFECTS,gsn_engarde,ch->level,1,number_fuzzy(ch->level / 8),APPLY_HITROLL,UMAX(1,ch->level/5),0);
		affect_set(ch,TO_AFFECTS,gsn_engarde,ch->level,1,number_fuzzy(ch->level / 8),APPLY_DAMROLL,UMAX(1,ch->level/5),0);
		affect_set(ch,TO_AFFECTS,gsn_engarde,ch->level,1,number_fuzzy(ch->level / 8),APPLY_AC,0 - number_range(10,10+5 * (ch->level/8)),0);
	}

	else{
		WAIT_STATE(ch,3 * PULSE_VIOLENCE);
		ch->move -= 25;

		send_to_char("You fumble about and fail to assume a proper battle stance.\n\r",ch);
		check_improve(ch,gsn_engarde,false,2);
	}
}
