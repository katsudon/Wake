#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"

/*
 * Local functions.
 */
bool check_horsemanship		( CHAR_DATA* );
void combo_unending_strikes(CHAR_DATA*,CHAR_DATA*);
void combo_twin_tiger_fang(CHAR_DATA*,CHAR_DATA*);
void combo_fierce_palm(CHAR_DATA*,CHAR_DATA*);
void combo_fist_of_fury(CHAR_DATA*,CHAR_DATA*);
void combo_dragon_fist(CHAR_DATA*,CHAR_DATA*);
void combo_lion_combo(CHAR_DATA*,CHAR_DATA*);


bool canPK(CHAR_DATA *ch, CHAR_DATA *victim,bool message){
	if(!victim){
		if(IS_NPC(ch))
			return true;
		if(ch->isplr(PL_PK) || ch->isplr(PL_ARENA) || get_trust(ch) >= LEVEL_IMMORTAL)
			return true;
		return false;
	}
	if(ch == victim)
		return true;
	if(IS_NPC(ch)){
		if(!ch->isaff(AF_CHARM))
			return true;
		if(!IS_NPC(victim)){
			if(victim->isplr(PL_PK))
				return true;
		}
		else
			return true;
		return false;
	}
	if(IS_NPC(victim)){
		if(!victim->isaff(AF_CHARM))
			return true;
		if(!IS_NPC(ch)){
			if(ch->isplr(PL_PK))
				return true;
		}
		else
			return true;
		return false;
	}

	if(get_trust(ch) < LEVEL_IMMORTAL && get_trust(victim) < LEVEL_IMMORTAL){//BOTH are mortal MORTAL
		if(!ch->isplr(PL_PK) && !ch->isplr(PL_ARENA)){
			if(message)ch->send("You cannot participate in player versus player combat.\n\r");
			return false;
		}
		if(!victim->isplr(PL_PK) && !victim->isplr(PL_ARENA)){
			if(message)ch->send("They cannot participate in player versus player combat.\n\r");
			return false;
		}
	}
	return true;
}

bool canFight(CHAR_DATA *ch, CHAR_DATA *victim,bool message){
	char buf[MSL];
	buf[0] = '\0';

	if(IS_NPC(ch) && victim->position == POS_MORTAL)
		return false;

	if(ch->bashwait > 0)
		sprintf(buf,"You have to stand up first.\n\r");
	if (victim == NULL)
		sprintf(buf,"Who?\n\r");
	if(IS_NPC(victim) && victim->isact(AT_NOKILL))
		sprintf(buf,"The gods block your aggression.\n\r");
	if(victim->max_hit == 0)
		sprintf(buf,"This poor weakling has broken health, contact Nash about it ASAP.\n\r");
//    if (victim == ch)
//		sprintf(buf,"Very funny.\n\r");
    if (victim == NULL)
		sprintf(buf,"Who?\n\r");
    if (is_safe(ch,victim))
		sprintf(buf,"Not in this room..\n\r");
    if (ch->isaff(AF_CHARM) && ch->master == victim)
		sprintf(buf,"But %s is such a good friend!",PERS(victim,ch));
    if (MOUNTED(ch) && (!MOUNTED(ch)->isact(AT_WARHORSE) || get_skill(ch,gsn_horsemanship) < 1))
        sprintf(buf,"You can't do that while riding!\n\r");
    if (ch->isaff(AF_ENTANGLED))
        sprintf(buf,"You can't fight entangled like that!\n\r");
    if (ch->isaff(AF_CALM))
        sprintf(buf,"You feel too calm to fight right now.\n\r");
	if(!canPK(ch,victim,message))
		return false;
	if(victim->position == POS_MORTAL)
		return false;

	if (buf[0]){
		if (message)
			ch->send(buf);
		return false;
	}
	else
		return true;
}

void check_martial_arts(CHAR_DATA *ch,CHAR_DATA *victim){
	int i;
	for(i=0;i<5;i++){
		if (ch->combo[i] == '0')
			return;
	}
	if (ch->combo[0] == 'b'){
		if (ch->combo[1] == 'k'){
			if (ch->combo[2] == 'a' && ch->combo[3] == 'c' && ch->combo[4] == 'd'){
				combo_twin_tiger_fang(ch,victim);
				sprintf(ch->combo,"00000");
			}
		}
		else{
			if (ch->combo[1] == 'a' && ch->combo[2] == 'i' && ch->combo[3] == 'k' && ch->combo[4] == 'c'){
				combo_fierce_palm(ch,victim);
				sprintf(ch->combo,"00000");
			}
		}
	}
	if (ch->combo[0] == 'a' && ch->combo[1] == 'b'){
		if (ch->combo[2] == 'g' && ch->combo[3] == 'f' && ch->combo[4] == 'a'){
			combo_unending_strikes(ch,victim);
			sprintf(ch->combo,"00000");
		}
		else{
			if (ch->combo[2] == 'c' && ch->combo[3] == 'd' && ch->combo[4] == 'k'){
				combo_fist_of_fury(ch,victim);
				sprintf(ch->combo,"00000");
			}
		}
	}
	if (ch->combo[0] == 'd' && ch->combo[1] == 'a' && ch->combo[2] == 'a' && ch->combo[3] == 'k' && ch->combo[4] == 'c'){
		combo_dragon_fist(ch,victim);
		sprintf(ch->combo,"00000");
	}
	if (ch->combo[0] == 'i' && ch->combo[1] == 'f' && ch->combo[2] == 'k' && ch->combo[3] == 'a' && ch->combo[4] == 'g'){
		combo_lion_combo(ch,victim);
		sprintf(ch->combo,"00000");
	}
}

bool canclass_counter(CHAR_DATA *ch){
	if (!IS_NPC(ch)){
		if(get_skill(ch,gsn_counter) > 0)
			return true;
		else
			return false;
	}

	if(is_class(ch,CLASS_IMMORTAL) || is_class(ch,CLASS_IMMORTAL) || is_class(ch,CLASS_IMMORTAL))
		return true;
	else
		return false;
}

bool canclass_riposte(CHAR_DATA *ch){
	if(!IS_NPC(ch)){
		if(get_skill(ch,gsn_riposte) > 0)
			return true;
		else
			return false;
	}

	if(ch->pclass == CLASS_IMMORTAL)
		return true;
	else
		return false;
}

bool canclass_coule(CHAR_DATA *ch)
{
	if(!IS_NPC(ch))
	{
		if(get_skill(ch,gsn_coule) > 0)
			return true;
		else
			return false;
	}

	if(ch->pclass == CLASS_IMMORTAL)
		return true;
	else
		return false;
}

bool canclass_absorb(CHAR_DATA *ch){
	if(!IS_NPC(ch)){
		if(get_skill(ch,gsn_absorb) > 0)
			return true;
		else
			return false;
	}

	if(ch->isdef(DF_ABSORB))
		return true;
	else
		return false;
}

bool check_mirage( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

	if(!victim->isaff(AF_MIRAGE))
		return false;

    chance = victim->level/4;
	chance += ch->bashwait*10;
	chance -= victim->bashwait*10;

    if (victim->level >= ch->level)
		chance += 2;

    if (number_percent() > chance)
        return false;

    act("Your mirage absorbs $n's attack!", ch, NULL, victim, TO_VICT);
    act("$n's atack slips through $N's mirage!", ch, NULL, victim, TO_NOTVICT);
    act("Your attack slips though $N's body, it was a mirage!", ch, NULL, victim, TO_CHAR);

    return true;
}

bool check_phase(CHAR_DATA *ch,CHAR_DATA *victim){
	AFFECT_DATA *af = affect_find(victim->affected,gsn_phase);
	int chance;

	if(!victim->isaff(AF_PHASE))
		return false;

	chance = victim->level/20;
	chance += ch->bashwait*5;
	chance += af->modifier * 2;

	if (victim->level >= ch->level)
		chance += 2;

	if (number_percent() > chance)
		return false;

	act("Your body phases through $n's attack!", ch, NULL, victim, TO_VICT);
	act("$n's atack slips through $N's body!", ch, NULL, victim, TO_NOTVICT);
	act("Your attack slips though $N's body!", ch, NULL, victim, TO_CHAR);

	return true;
}

int check_enhanced(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	int diceroll=0;
	if (get_skill(ch,gsn_enhanced_damage) > 0 && !IS_NPC(ch)){
		diceroll = number_percent();
		if (diceroll <= get_skill(ch,gsn_enhanced_damage) * 1){//.35){
			check_improve(ch,gsn_enhanced_damage,true,1);
			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Enhanced: %d ",dam);
			dam += ( ((dam * ch->getslvl(gsn_enhanced_damage))/5) * diceroll / 200);
			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d\n\r",dam);
		}
		else
			check_improve(ch,gsn_enhanced_damage,false,1);
	}
	return dam;
}
bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim){
	int chance = get_skill(ch,gsn_critical) * .25;
    OBJ_DATA *obj = get_eq_char(ch,WEAR_WIELD);

    if (chance < 1 || (IS_NPC(ch) && !ch->isoff(OF_CRITICAL)))
		return false;

	if (!obj){
		if (get_skill(ch,gsn_combatives) < 100)
			return false;
	}
	else{
		if (get_weapon_skill(ch,get_weapon_sn(ch,false)) < 1)
			return false;
	}

	chance = chance * get_curr_stat(ch,STAT_LCK) / STAT_MAX;

    if (roll_chance(ch,chance)){
		act("{x$n executes a flawless {RCritical Strike {xon $N.",ch, NULL,victim,TO_NOTVICT);
		act("{xYou land a {RCritical Strike {xon $N.", ch,NULL,victim,TO_CHAR);
		act("{x$n lands a {RCritical Strike {xon you.",ch,NULL,victim,TO_VICT);

		check_improve(ch,gsn_critical,true,1);
		return true;
	}
	else{
		check_improve(ch,gsn_critical,false,1);
		return false;
	}
}
