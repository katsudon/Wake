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
bool check_innerpeace	( CHAR_DATA* );
void shift_combo		( CHAR_DATA*,char );

int MA_mod(CHAR_DATA *ch,int dam,bool punch){
	int ndam = dam;/* NASHNEEDSTOFIXTHIS
	if (dam < 1)
		return dam;
	double mod = get_curr_stat(ch,STAT_INT) / ((75 * ch->level)/100);
		ndam = (dam * mod) + ((get_curr_stat(ch,STAT_STR) + get_curr_stat(ch,STAT_AGI))/2);
	if (punch)
		ndam = ndam * get_skill(ch,gsn_combatives) / 100;*/
	return UMAX(ndam,1);
}

void do_backfist(CHAR_DATA *ch,char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;
	int dam = 0,sn = gsn_backfist;
	bool improve = false;

	one_argument(argument,arg);

	if (!(victim = grab_char(ch,arg,true)) || IS_NPC(ch) || !canFight(ch,victim,true))
		return;

	if (get_skill(ch,sn) < 1){
		ch->send("You better leave that to someone who knows what they're doing\n\r");
		return;
	}

	dam = dice(ch->level/2,3);
	dam = dam * get_skill(ch,sn) / 100;
	MA_mod(ch,dam,true);
	shift_combo(ch,'k');

	check_improve(ch,sn,true,1);

	if (check_innerpeace(ch))
		WAIT_STATE(ch,skill_table[sn].beats/2);
	else
		WAIT_STATE(ch,skill_table[sn].beats);

	act("You throw a high punch, and as $N blocks you slam the back of your other fist into $S gut.",ch,NULL,victim,TO_CHAR);
	act("$n throws a high punch, and as you block $e slams the back of $s other fist into your gut.",ch,NULL,victim,TO_VICT);
	act("$n throws a high punch, and as $N blocks $e slams the back of $s other fist into $N's gut.",ch,NULL,victim,TO_NOTVICT);
	skill_damage(ch,victim,dam,sn,DAM_BASH,true);

	check_martial_arts(ch,victim);
}

void do_punch(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	int dam = 0,skill,sn = 0;
	bool improve = true;

	one_argument(argument,arg);

	if (!(victim = ch->fighting)){
		ch->send("You need to be fighting.\n\r");
		return;
	}
	if (IS_NPC(ch) || !(victim = ch->fighting) || !canFight(ch,victim,true))
		return;

	if (arg[0]){
		if (!str_prefix(arg,"cross")){
			skill = get_skill(ch,(sn = gsn_punchcross));
			dam = dice(get_curr_stat(ch,STAT_AGI) * .75,2);
			shift_combo(ch,'a');
		}
		else if (!str_prefix(arg,"jab")){
			skill = get_skill(ch,(sn = gsn_punchjab));
			dam = dice(3,ch->level/3);
			shift_combo(ch,'b');
		}
		else if (!str_prefix(arg,"tigerfang")){
			skill = get_skill(ch,(sn = gsn_punchtigerfang));
			dam = dice(number_range(ch->level/3,ch->level/2),4);
			shift_combo(ch,'c');
		}
		else if (!str_prefix(arg,"uppercut")){
			skill = get_skill(ch,(sn = gsn_punchuppercut));
			dam = dice(4,get_curr_stat(ch,STAT_STR));
			shift_combo(ch,'d');
		}
		else{
			skill = get_skill(ch,(sn = gsn_punch));
			dam = (number_range(skill * .6,skill) * .5);
			shift_combo(ch,'e');
		}
	}
	else{
		skill = get_skill(ch,(sn = gsn_punch));
		dam = ((number_range(skill * .5,skill) * ch->getslvl(gsn_punch)) / 4);
		shift_combo(ch,'e');
	}

	if (!IS_NPC(ch) && skill < 1){
		ch->send("You lack the skill to perform such a technique.\n\r");
		return;
	}

	MA_mod(ch,dam,true);

	if (number_percent() >= skill){
		dam = 0;
		improve = false;
	}
	check_improve(ch,sn,improve,1);
	if (check_innerpeace(ch))
		WAIT_STATE(ch,skill_table[sn].beats/2);
	else
		WAIT_STATE(ch,skill_table[sn].beats);
	skill_damage(ch,victim,dam,sn,DAM_BASH,true);
	check_martial_arts(ch,victim);
}

void do_kick(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int dam=0,skill=0,sn;
	char arg[MIL];
	bool improve = true;

	one_argument(argument,arg);

	if ((victim = ch->fighting) == NULL){
		send_to_char("You aren't fighting anyone.\n\r",ch);
		return;
	}

	if ((IS_NPC(ch) && !ch->isoff(OF_KICK)) || !canFight(ch,victim,true))
		return;

	if (arg[0]){
		if (!str_prefix(arg,"axe")){
			skill = get_skill(ch,(sn = gsn_kickaxe));
			dam = dice(3,ch->level/3);
			shift_combo(ch,'f');
		}
		else if (!str_prefix(arg,"roundhouse")){
			skill = get_skill(ch,(sn = gsn_kickroundhouse));
			dam = dice(4,get_curr_stat(ch,STAT_AGI));
			shift_combo(ch,'g');
		}
		else if (!str_prefix(arg,"high")){
			skill = get_skill(ch,(sn = gsn_kickhigh));
			dam = dice(5,number_range(ch->level/3,ch->level/2));
			shift_combo(ch,'h');
		}
		else if (!str_prefix(arg,"low")){
			skill = get_skill(ch,(sn = gsn_kicklow));
			dam = dice(4,get_curr_stat(ch,STAT_STR));
			shift_combo(ch,'i');
		}
		else{
			skill = get_skill(ch,(sn = gsn_kick));
			dam = (number_range(skill * .6,skill) * .5);
			shift_combo(ch,'j');
		}
	}
	else{
		skill = get_skill(ch,(sn = gsn_kick));
		dam = (number_range(skill * .6,skill) * .5);
		dam += (ch->level/10) * ch->getslvl(gsn_kick);
			shift_combo(ch,'j');
	}

	if (!IS_NPC(ch) && skill < 1){
		ch->send("You lack the skill to perform such a technique.\n\r");
		return;
	}

	MA_mod(ch,dam,false);

	if (number_percent() >= skill){
		dam = 0;
		improve = false;
	}
	check_improve(ch,sn,improve,1);
	skill_damage(ch,victim,dam,sn,DAM_BASH,true);
	
	if (sn == gsn_kickhigh && dam > 0 && !saves_skill(ch->level,victim,DAM_BASH)){
		victim->send("You see stars!\n\r");
		act("$n looks disoriented.",victim,NULL,NULL,TO_ROOM);
		DAZE_STATE(victim,4*PULSE_VIOLENCE);
	}
	if (check_innerpeace(ch))
		WAIT_STATE(ch,skill_table[sn].beats/2);
	else
		WAIT_STATE(ch,skill_table[sn].beats);
	check_martial_arts(ch,victim);
}

void combo_twin_tiger_fang(CHAR_DATA *ch,CHAR_DATA *victim){
	if (!ch || !victim)
		return;

	ROOM_INDEX_DATA *was_in = victim->in_room,*now_in;
	EXIT_DATA *pexit;
	int dam=dice(ch->level,5),door,tries,skill = (get_skill(ch,gsn_twin_tiger_fang) * get_curr_stat(ch,STAT_AGI)) / STAT_MAX;

	WAIT_STATE(ch,skill_table[gsn_twin_tiger_fang].beats);
	if (number_percent() < skill){
		act("You bring your wrists together, forming the tiger's jaw and thrust into $N's chest.",ch,NULL,victim,TO_CHAR);
		act("$n brings $s wrists together and thrusts $S hands into your chest!",ch,NULL,victim,TO_VICT);
		act("$n forms a tiger's jaw with $s hands and thrusts them into $N's chest.",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,dam,gsn_twin_tiger_fang,DAM_PIERCE,true);
		WAIT_STATE(victim,skill_table[gsn_twin_tiger_fang].beats/2);
		DAZE_STATE(victim,skill_table[gsn_twin_tiger_fang].beats);
		if (!saves_skill(ch->level * 1.5,victim,DAM_BASH)){
			for (tries = 0; tries < 12; tries++ ){
				if ((door = get_random_door(victim->in_room)) == -1)
					break;
				if (IS_NPC(victim) && IS_SET(was_in->exit[door]->u1.to_room->room_flags,ROOM_NO_MOB))
					continue;
				move_char(victim,door,false,false);
				if ((now_in = victim->in_room) == was_in)
					continue;
				victim->in_room = was_in;
				act("You fly $T.",victim,NULL,dir_name[door],TO_CHAR);
				act("$n flies $T.",victim,NULL,dir_name[door],TO_ROOM);
				victim->in_room = now_in;
				stop_fighting(victim,true);
				return;
			}
			act("You slam into a wall.",victim,NULL,NULL,TO_CHAR);
			act("$n slams into a wall.",victim,NULL,NULL,TO_ROOM);
			skill_damage(victim,victim,100,0,DAM_BASH,false);
			victim->bashwait = 15;
		}
		check_improve(ch,gsn_twin_tiger_fang,true,1);
	}
	else{
		check_improve(ch,gsn_twin_tiger_fang,false,2);
		skill_damage(ch,victim,0,gsn_twin_tiger_fang,DAM_BASH,true);
	}
}

void combo_fierce_palm(CHAR_DATA *ch,CHAR_DATA *victim){
	if (!ch || !victim)
		return;

	int dam = number_range(ch->level,ch->level * 2),skill = (get_skill(ch,gsn_fierce_palm) * get_curr_stat(ch,STAT_AGI)) / STAT_MAX;

	if (number_percent() < skill){
		act("You slam your open palm into $N's chest with focused might.",ch,NULL,victim,TO_CHAR);
		act("$n slams $s open palm into your chest and you go flying to the ground!",ch,NULL,victim,TO_VICT);
		act("$n slams $ns open palm into $N's chest and $E flies to the ground.",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,dam,gsn_fierce_palm,DAM_BASH,true);
		victim->bashwait = 20;
		WAIT_STATE(victim,skill_table[gsn_fierce_palm].beats/2);
		check_improve(ch,gsn_fierce_palm,true,1);
	}
	else{
		check_improve(ch,gsn_fierce_palm,false,2);
		skill_damage(ch,victim,0,gsn_fierce_palm,DAM_BASH,true);
	}
	WAIT_STATE(ch,skill_table[gsn_fierce_palm].beats);
}

void combo_lion_combo(CHAR_DATA *ch,CHAR_DATA *victim){
	if (!ch || !victim)
		return;

	int skill = (get_skill(ch,gsn_lion_combo) * get_curr_stat(ch,STAT_AGI)) / STAT_MAX;

	act("You execute a series of precise hits.",ch,NULL,victim,TO_CHAR);
	act("$n steps into a series of powerful hits!",ch,NULL,victim,TO_VICT);
	act("$n steps into a series of graceful hits.",ch,NULL,victim,TO_NOTVICT);
	skill_damage(ch,victim,dice(ch->level/2,6),gsn_backfist,DAM_BASH,true);
	skill_damage(ch,victim,dice(ch->level/2,5),gsn_kickroundhouse,DAM_BASH,true);
	skill_damage(ch,victim,dice(ch->level/2,4),gsn_punchcross,DAM_BASH,true);
	skill_damage(ch,victim,dice(ch->level/2,3),gsn_kickhigh,DAM_BASH,true);
	skill_damage(ch,victim,dice(ch->level/2,2),gsn_kickroundhouse,DAM_BASH,true);
	skill_damage(ch,victim,dice(ch->level/2,1),gsn_kickaxe,DAM_BASH,true);
	check_improve(ch,gsn_fierce_palm,true,1);
	WAIT_STATE(ch,skill_table[gsn_lion_combo].beats);
}

void combo_dragon_fist(CHAR_DATA *ch,CHAR_DATA *victim){
	if (!ch || !victim)
		return;

	int dam = number_range(ch->level*1.5,ch->level*2),skill = (get_skill(ch,gsn_dragon_fist) * get_curr_stat(ch,STAT_AGI)) / STAT_MAX;

	if (number_percent() < skill){
		act("Your fist seems to catch fire as it rockets upwards into $N's jaw.",ch,NULL,victim,TO_CHAR);
		act("$n's fist seems to catch fire as it rockets upwards into your jaw and you see stars!",ch,NULL,victim,TO_VICT);
		act("$n's fist seems to catch fire as it rockets upwards into $N's jaw.",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,dam,gsn_dragon_fist,DAM_BASH,true);
		dam = number_range(1,ch->level/2);
		act("$n is {rburned{x.",victim,NULL,NULL,TO_ROOM);
		act("The punch {rsears {xyour flesh.",victim,NULL,NULL,TO_CHAR);
		skill_damage(ch,victim,dam,0,DAM_FIRE,false);
		fire_effect(victim,ch->level,ch->level,TARGET_CHAR);
		DAZE_STATE(victim,skill_table[gsn_dragon_fist].beats);
		check_improve(ch,gsn_dragon_fist,true,1);
	}
	else{
		check_improve(ch,gsn_dragon_fist,false,2);
		skill_damage(ch,victim,0,gsn_dragon_fist,DAM_BASH,true);
	}
	WAIT_STATE(ch,skill_table[gsn_dragon_fist].beats);
}

void combo_fist_of_fury(CHAR_DATA *ch,CHAR_DATA *victim)
{
	if (!ch || !victim)
		return;

	int dam=number_range(ch->level,ch->level*2),skill = (get_skill(ch,gsn_fist_of_fury) * get_curr_stat(ch,STAT_AGI)) / STAT_MAX;

	if (number_percent() < skill){
		act("You slam your fist hard into $N's gut.",ch,NULL,victim,TO_CHAR);
		act("$n slams $s fist hard into your gut and your vision blurs!",ch,NULL,victim,TO_VICT);
		act("$n slams $s fist hard into $N's gut.",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,dam,gsn_fist_of_fury,DAM_BASH,true);
		DAZE_STATE(victim,skill_table[gsn_fist_of_fury].beats);
		check_improve(ch,gsn_fist_of_fury,true,1);
	}
	else{
		check_improve(ch,gsn_fist_of_fury,false,2);
		skill_damage(ch,victim,0,gsn_fist_of_fury,DAM_BASH,true);
	}
	WAIT_STATE(ch,skill_table[gsn_fist_of_fury].beats);
}

void combo_unending_strikes(CHAR_DATA *ch,CHAR_DATA *victim)
{
	if (!ch || !victim)
		return;

	int dam,skill = (get_skill(ch,gsn_unending_strikes) * get_curr_stat(ch,STAT_AGI)) / STAT_MAX;

	printf_to_char(ch,"%d\n\r",skill);
	do{
		if (number_percent() < skill){
			check_improve(ch,gsn_unending_strikes,true,1);
			dam = MA_mod(ch,dice(ch->level,2),false);
		}
		else{
			check_improve(ch,gsn_unending_strikes,false,2);
			dam = 0;
		}
		skill_damage(ch,victim,dam,gsn_unending_strikes,DAM_BASH,true);
		skill *= .9;
	}while(dam > 0);
	WAIT_STATE(ch,skill_table[gsn_unending_strikes].beats);
}

void shift_combo(CHAR_DATA *ch,char added)
{
	ch->combo[0] = ch->combo[1];
	ch->combo[1] = ch->combo[2];
	ch->combo[2] = ch->combo[3];
	ch->combo[3] = ch->combo[4];
	ch->combo[4] = added;
}

bool check_innerpeace(CHAR_DATA *ch){
	int chance = get_skill(ch,gsn_innerpeace) * .25;

	if (IS_NPC(ch) || chance < 1)
		return false;

	chance = chance * (get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_AGI)) / (STAT_MAX * 1.5);
	chance += 10 * get_curr_stat(ch,STAT_WIS) / STAT_MAX;


	if (number_percent() < chance){
		ch->send("Your focus of mind helps you speed your movements.\n\r");
		check_improve(ch,gsn_innerpeace,true,1);
		return true;
	}

	check_improve(ch,gsn_innerpeace,false,1);
	return false;
}

bool check_fluidmotion(CHAR_DATA *ch){
	int chance = get_skill(ch,gsn_fluidmotion) *.15;

	if (IS_NPC(ch) || chance < 1)
		return false;

	chance = chance * (get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_AGI)) / (STAT_MAX * 1.5);
	chance += 10 * get_curr_stat(ch,STAT_AGI) / STAT_MAX;


	if (number_percent() < chance){
		act("$n weaves and steps through combat like water.",ch,NULL,NULL,TO_ROOM);
		ch->send("You weave fluidly through combat.\n\r");
		check_improve(ch,gsn_fluidmotion,true,1);
		ch->wait /= 2;
		ch->pcounter *= .5;
		ch->dcounter *= .75;
		return true;
	}

	check_improve(ch,gsn_fluidmotion,false,1);
	return false;
}
