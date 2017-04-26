#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "lookup.h"


/* 
 * Local functions.
 */
bool saves_dispel	( int,int,int );
bool check_poverty	( CHAR_DATA* );
CHAR_DATA *get_char_around(CHAR_DATA*,char*,int*,bool);
int melee_hit(CHAR_DATA*,CHAR_DATA*,int,bool);
void shift_combo	( CHAR_DATA*,char);
bool check_evade	( CHAR_DATA*,CHAR_DATA* );
bool check_guard	( CHAR_DATA*,CHAR_DATA* );
void check_paired_attack(CHAR_DATA*,CHAR_DATA*,int);
int  crunch_lead_stance (CHAR_DATA*,CHAR_DATA*,int);


void do_feral_slash(CHAR_DATA *ch,char *argument){
	char arg[MSL];
	CHAR_DATA *victim;
	int dam = ch->level,skill = get_skill(ch,gsn_feralslash);

	argument = one_argument(argument,arg);

	if(skill < 1){
		ch->send("ROAR!... meow?\n\r");
		return;
	}
    if (!(victim = grab_char(ch,arg,true)))
		return;

	if(roll_chance(ch,skill)){
		skill_damage(ch,victim,dam,gsn_feralslash,DAM_SLASH,true);
		damage(ch,ch,dam,gsn_feralslash,DAM_SLASH,true);
		check_improve(ch,gsn_feralslash,false,1);
	}
	else{
		damage(ch,victim,0,gsn_feralslash,DAM_SLASH,true);
		check_improve(ch,gsn_feralslash,false,1);
	}
	WAIT_STATE(ch,skill_table[gsn_feralslash].beats);
}

void do_slash(CHAR_DATA *ch,char *argument){
}

void do_flurry(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char arg[MSL];
	bool barefist = false;
	int sn = gsn_flurry,lag = skill_table[sn].beats,i,dam,skill = get_skill(ch,sn),thits = number_range(3,10);

	if (skill < 1){
		ch->send("You wave your arms around like an idiot.. good job.\n\r");
		return;
	}

	if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL)
		if ((obj = get_eq_char(ch,WEAR_SECONDARY)) == NULL)
			barefist = true;

	if (!barefist && (!obj || !is_weapon_sharp(obj))){
		ch->send("Invalid weapon!\n\r");
		return;
	}

	one_argument(argument,arg);

    if (!(victim = grab_char(ch,arg,true)))
		return;

	act("You begin swinging at $N, your arms blurring like a storm.",ch,NULL,victim,TO_CHAR);
	act("$n begins swinging at you, $s arms blurring like a storm.",ch,NULL,victim,TO_VICT);
	act("$n begins swinging at $N, $s arms blurring like a storm.",ch,NULL,victim,TO_NOTVICT);

	for(i = 0;i < 5;i++){
		skill *= .95;
		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gFLURRY({c%d{x)\n\r",skill);
		if (!barefist)
			dam = dice(obj->value[1],obj->value[2]);
		else
			dam = number_range(get_curr_stat(ch,STAT_STR),ch->level) * 2;

		if(roll_chance(ch,skill)){
			if (check_shield_block(ch,victim) || check_dodge(ch,victim,obj)){
				check_improve(ch,sn,false,1);
				skill_damage(ch,victim,2,TYPE_HIT,DAM_SLASH,true);
			}
			else{
				check_improve(ch,sn,true,1);
				skill_damage(ch,victim,dam,TYPE_HIT,DAM_SLASH,true);
			}
			lag += skill_table[sn].beats;
		}
		else{
			lag += skill_table[sn].beats * .75;
			check_improve(ch,sn,false,1);
			skill_damage(ch,victim,0,TYPE_HIT,DAM_SLASH,true);
		}
	}
	WAIT_STATE(ch,lag);
}

bool check_counterspin(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,OBJ_DATA *wield,int sn){
	CHAR_DATA *wch,*wch_next;
	int chance,skill,dam_type;

	if ((IS_NPC(victim) && !canclass_counter(victim) && victim->level < 70)
	||	dt == gsn_backstab
	||	dt == gsn_charge
	||	dt == gsn_assassinate
	||	wield == NULL
	||	!get_eq_char(victim,WEAR_WIELD)
	||	!IS_AWAKE(victim)
	|| (skill = get_weapon_skill(victim,sn)) < 1
	|| get_skill(victim,gsn_counter_spin) < 1
	|| dam < 3)
		return false;

	if (!(wield = get_eq_char(victim,WEAR_WIELD)))
		return false;

	chance = get_skill(victim,gsn_counter_spin) / 6;
		if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gCSpn({c%d)",chance);
	chance += (victim->level - ch->level) / 2;
		if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"(%d)",chance);
	chance += skill - get_weapon_skill(ch,get_weapon_sn(ch,false));
		if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"(%d)",chance);
	chance += get_curr_stat(victim,STAT_AGI) - get_curr_stat(ch,STAT_AGI);
		if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"(%d)",chance);
	chance = (chance * get_curr_stat(victim,STAT_AGI)) / get_curr_stat(ch,STAT_AGI);
		if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"(%d)",chance);
	chance += (ch->bashwait*10) - (victim->bashwait*10);

	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"(%d{x)\n\r",chance);
	if (number_percent() >= chance){
		check_improve(victim,gsn_counter_spin,false,1);
		return false;
	}

	dt = gsn_counter_spin;

	if (dt < TYPE_HIT){
		if (wield != NULL)
    		dam_type = attack_table[wield->value[3]].damage;
	}
	else
		dam_type = attack_table[dt - TYPE_HIT].damage;

	if (dam_type == -1)
		dam_type = DAM_BASH;

	act("You reverse $n's attack and spin furiously!",ch,NULL,victim,TO_VICT);
	act("$N reverses your attack and spins furiously!",ch,NULL,victim,TO_CHAR);
	act("$N counters $n's attack and spins furiously!",ch,NULL,victim,TO_NOTVICT);

	for(wch = victim->in_room->people;wch;wch = wch_next){
		wch_next = wch->next_in_room;
		if (wch->fighting == victim){
			check_improve(victim,gsn_counter_spin,true,1);
			skill_damage(victim,wch,dam,gsn_counter_spin,dam_type,true);
			check_paired_attack(victim,wch,dam);
		}
	}

	return true;
}

void do_spiralingspear(CHAR_DATA *ch,char *argument){
	int skill = get_skill(ch,gsn_spiraling_spear);
}

int check_weapons_knowledge(CHAR_DATA *ch,int sn,bool dual){
	int chance = get_skill(ch,gsn_weapons_knowledge) * .4;

	if (sn < 1)
		sn = get_weapon_sn(ch,dual);

	if (get_skill(ch,sn) > 1)
		return get_skill(ch,sn);
	if (chance < 1)
		return 0;

	chance = chance * get_curr_stat(ch,STAT_INT) / (STAT_MAX * .8);
	chance += 10 * get_curr_stat(ch,STAT_STR) / (STAT_MAX * .7);

	if (roll_chance(ch,chance)){
//		printf_to_char(ch,"CWP%d\n\r",chance);
		check_improve(ch,gsn_weapons_knowledge,true,1);
		return chance;
	}
	check_improve(ch,gsn_weapons_knowledge,false,1);
	return 0;
}

void check_sonic_impact(CHAR_DATA *ch,CHAR_DATA *victim,bool dual){
	int chance = get_skill(ch,gsn_sonic_impact);
	if (chance < 1 || IS_NPC(ch) || victim->position == POS_DEAD)
		return;
	if(dual){
		if(get_eq_char(ch,WEAR_SECONDARY) && !is_weapon_blunt(get_eq_char(ch,WEAR_SECONDARY)))
			return;
	}
	else{
		if(get_eq_char(ch,WEAR_WIELD) && !is_weapon_blunt(get_eq_char(ch,WEAR_WIELD)))
			return;
	}

	chance *= 0.01;
	chance += get_curr_stat(ch,STAT_STR)/10;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gSonicImp({c%d{x)\n\r",chance);

	if (roll_chance(ch,chance)){
		act("Your attack cracks $N in the head, stunning $M.",ch,NULL,victim,TO_CHAR);
		act("You see stars as $n's weapon hits you hard in the head.",ch,NULL,victim,TO_VICT);
		act("$n's attack cracks $N hard in the head, stunning $M.",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,ch->level * 1.5,gsn_sonic_impact,DAM_BASH,true);
		victim->bashwait += 2;
		check_improve(ch,gsn_sonic_impact,true,2);
	}
	check_improve(ch,gsn_sonic_impact,false,1);
}

void check_parting_shot(CHAR_DATA *ch,CHAR_DATA *victim){
	int dam,skill = get_skill(ch,gsn_parting_shot);

	if(skill < 1)
		return;

	skill /= 4;

	if(roll_chance(ch,skill)){
		ch->send("Yay!\n\r");
		check_improve(ch,gsn_parting_shot,true,1);
	}
	else
		check_improve(ch,gsn_parting_shot,false,1);
}

void check_defender(CHAR_DATA *ch){
	AFFECT_DATA af;
	int dam,skill = get_skill(ch,gsn_defender);

	if(skill < 1)
		return;

	if(roll_chance(ch,skill)){
		affect_set(ch,TO_AFFECTS,gsn_defender,ch->level,1,number_fuzzy(ch->level / 8),APPLY_AC,-50,AF_GUARDIAN);
		ch->send("Your heroics invigorate you.\n\r");
		check_improve(ch,gsn_defender,true,1);
	}
	else
		check_improve(ch,gsn_defender,false,1);
}

void do_reckless_abandon(CHAR_DATA *ch,char *argument){
	AFFECT_DATA af;
	int thr,sn = gsn_reckless_abandon,skill = get_skill(ch,sn);

	if(skill < 1){
		ch->send("You aren't crazy enough to do that!\n\r");
		return;
	}

	skill /= 4;

	if(ch->getmana() < 50){
		ch->send("You lack the energy to go crazy.\n\r");
		return;
	}

	if(is_affected(ch,gsn_reckless_abandon)){
		ch->send("You're already crazy!!!\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		thr = GET_HITROLL(ch);
		thr /= 5;
		ch->modmana(-50);
		affect_set(ch,TO_AFFECTS,gsn_reckless_abandon,ch->level,1,1,APPLY_DAMROLL,thr + thr * (6 -ch->getslvl(sn)),0);
		affect_set(ch,TO_AFFECTS,gsn_reckless_abandon,ch->level,1,1,APPLY_HITROLL,-thr,0);
		affect_set(ch,TO_AFFECTS,gsn_reckless_abandon,ch->level,1,1,APPLY_AGI,ch->getslvl(sn)*2,0);

		WAIT_STATE(ch,skill_table[gsn_reckless_abandon].beats);
		ch->send("Your eyes glow red and your blood boils!\n\r");
		act("$n screams and $s eyes seem to glow!",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,gsn_reckless_abandon,true,1);
	}
	else{
		WAIT_STATE(ch,skill_table[gsn_reckless_abandon].beats/2);
		ch->modmana(-25);
		ch->send("You scream and shake your head... and nothing happens.\n\r");
		act("$n screams and shakes $s head, and nothing happens.",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,gsn_reckless_abandon,false,1);
	}
}

void do_shieldstrike(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *shield = get_eq_char(ch,WEAR_SHIELD);
	int dam,skill = get_skill(ch,gsn_shieldstrike);

	if(skill < 1){
		ch->send("You don't know how to do that.\n\r");
		return;
	}

	if (!(victim = grab_char(ch,argument,true)))
		return;

	if(!shield || shield->item_type != ITEM_SHIELD){
		ch->send("You must be wearing a shield.\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_shieldstrike].beats);
	if(roll_chance(ch,skill)){
		dam = (shield->level * ch->pcdata->skill_level[gsn_shieldstrike]) + get_curr_stat(ch,STAT_STR) + shield->weight / 10;
		act("You strike $N with $p!",ch,shield,victim,TO_CHAR);
		skill_damage(ch,victim,dam,gsn_shieldstrike,DAM_BASH,true);
		check_improve(ch,gsn_shieldstrike,true,1);
	}
	else{
		damage(ch,victim,0,gsn_shieldstrike,DAM_BASH,true);
		check_improve(ch,gsn_shieldstrike,false,1);
	}
}

void do_smash(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	int dam,skill = get_skill(ch,gsn_smash);


	if(skill < 1){
		ch->send("No matter how much you want to, you don't know how to do this.\n\r");
		return;
	}

	if (!(victim = grab_char(ch,argument,true)))
		return;

	if(!wield || wield->item_type != ITEM_WEAPON || !is_weapon_blunt(wield)){
		ch->send("You can only perform this skill with a bludgeon.\n\r");
		return;
	}
	if(ch->getmana() < 25){
		ch->send("You don't have enough mana!\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_smash].beats);
	if(roll_chance(ch,skill)){
		dam = dice(wield->value[1],wield->value[2]);
		dam = (dam * ((ch->getslvl(gsn_smash) * 5) + 100))/100;
		dam += (wield->weight + ch->perm_stat[STAT_STR]) / 2;
		skill_damage(ch,victim,dam,gsn_smash,DAM_BASH,true);
		check_improve(ch,gsn_smash,true,1);
	}
	else{
		ch->modmana(-5);
		damage(ch,victim,0,gsn_smash,DAM_BASH,true);
		check_improve(ch,gsn_smash,false,1);
	}
}

void do_heavy_smash(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	int dam,skill = get_skill(ch,gsn_heavy_smash);

	if(skill < 1){
		ch->send("No matter how much you want to, you don't know how to do this.\n\r");
		return;
	}

	if (!(victim = grab_char(ch,argument,true)))
		return;

	if(!wield || wield->item_type != ITEM_WEAPON || !is_weapon_blunt(wield)){
		ch->send("You can only perform this skill with a bludgeon.\n\r");
		return;
	}

	skill *= .6;

	WAIT_STATE(ch,skill_table[gsn_heavy_smash].beats);
	if(roll_chance(ch,skill)){
		dam = dice(wield->value[1],wield->value[2]);
		dam = (dam * ((ch->getslvl(gsn_heavy_smash) * 20) + 100))/100;
		dam += wield->weight / 2;
		skill_damage(ch,victim,dam,gsn_heavy_smash,DAM_BASH,true);
		ch->modmana(-25);
		if(roll_chance(ch,skill/2)){
			act("You send $N sprawling!",ch,NULL,victim,TO_CHAR);
			act("$n sends $N sprawling!",ch,NULL,victim,TO_NOTVICT);
			act("$n sends you sprawling!",ch,NULL,victim,TO_VICT);
			victim->bashwait = 3;
			DAZE_STATE(victim,PULSE_VIOLENCE*5);
		}
		check_improve(ch,gsn_heavy_smash,true,1);
	}
	else{
		damage(ch,victim,0,gsn_heavy_smash,DAM_BASH,true);
		check_improve(ch,gsn_heavy_smash,false,1);
	}
}

void do_pommel_strike(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	int dam,skill = get_skill(ch,gsn_pommel_strike);


	if(skill < 1){
		ch->send("Pommel? Isn't that a kind of radish?.\n\r");
		return;
	}

	if (!(victim = grab_char(ch,argument,true)))
		return;

	if(!wield || wield->item_type != ITEM_WEAPON || !is_weapon_sword(wield)){
		ch->send("You can only perform this skill with a sword.\n\r");
		return;
	}

	skill *= .8;

	WAIT_STATE(ch,skill_table[gsn_pommel_strike].beats);
	if(roll_chance(ch,skill)){
		check_improve(ch,gsn_pommel_strike,true,1);
		act("You strike $N with the pommel of you weapon!",ch,NULL,victim,TO_CHAR);
		act("$n strikes you with the pommel of $s weapon!",ch,NULL,victim,TO_VICT);
		act("$n strikes $N with the pommel of $s weapon!",ch,NULL,victim,TO_NOTVICT);
		dam = dice(wield->value[1],wield->value[2]);
		dam += wield->weight + ch->pcdata->skill_level[gsn_pommel_strike] * (get_curr_stat(ch,STAT_STR) / 2);
		skill_damage(ch,victim,dam,gsn_pommel_strike,DAM_BASH,true);
		check_paired_attack(ch,victim,dam);
	}
	else{
		damage(ch,victim,0,gsn_pommel_strike,DAM_BASH,true);
		check_improve(ch,gsn_pommel_strike,false,1);
	}
}

void do_mighty_swing(CHAR_DATA *ch,char *argument){
	CHAR_DATA *vch,*vch_next;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	int dam,sn = gsn_mighty_swing,skill = get_skill(ch,sn);
	bool found = false;


	if(skill < 1){
		ch->send("MIGHTY POTATO!!! Wait what...\n\r");
		return;
	}

	if(!wield || wield->item_type != ITEM_WEAPON){
		ch->send("You can only perform this skill with a weapon.\n\r");
		return;
	}

	dam = dice(wield->value[1],wield->value[2]);
	dam += wield->weight + ch->pcdata->skill_level[sn] * (get_curr_stat(ch,STAT_STR) / 2);

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(vch == ch)
			continue;
		skill = get_skill(ch,sn) * .75;
		if(is_same_group(ch,vch))
			skill /= 2;
		if(roll_chance(ch,skill)){
			skill_damage(ch,vch,number_range(dam/2,dam),sn,attack_table[wield->value[3]].damage,true);
			found = true;
		}
	}

	if(found)
		check_improve(ch,sn,true,1);
	else
		check_improve(ch,sn,false,1);

	WAIT_STATE(ch,skill_table[sn].beats);
}

void do_whirlwind(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD),*dual = get_eq_char(ch,WEAR_SECONDARY);
	int dam,dam2,sn = gsn_whirlwind,skill = get_skill(ch,sn),skill2 = get_skill(ch,sn);
	int found = 0,i;

	if(skill < 1){
		ch->send("You blow as hard as you can, but cannot make a whirlwind...\n\r");
		return;
	}
	if(!(victim = grab_char(ch,argument,true)))
		return;

	if(!wield || !dual || wield->item_type != ITEM_WEAPON || dual->item_type != ITEM_WEAPON){
		ch->send("You must be wielding two weapons to perform this.\n\r");
		return;
	}

	skill = (skill * get_skill(ch,get_weapon_skill(ch,get_weapon_sn(ch,false)))) / 100;
	skill2 = (skill2 * get_skill(ch,get_weapon_skill(ch,get_weapon_sn(ch,true)))) / 110;

	dam  = melee_hit(ch,victim,dice(wield->value[1],wield->value[2]),false);
	dam2 = melee_hit(ch,victim,dice( dual->value[1], dual->value[2]),true);

	dam += dam / 4 * ch->getslvl(gsn_whirlwind);
	dam2 += dam / 4 * ch->getslvl(gsn_whirlwind);
	for(i = 0; i<ch->level/10;i++){
		if(i%2 == 0){
			skill *= .8;
			if(roll_chance(ch,skill)){
				skill_damage(ch,victim,number_range(dam/2,dam),sn,attack_table[wield->value[3]].damage,true);
				found++;
			}
			else
				damage(ch,victim,0,sn,attack_table[wield->value[3]].damage,true);
		}
		else{
			skill2 *= .7;
			if(roll_chance(ch,skill)){
				skill_damage(ch,victim,number_range(dam2/2,dam2),gsn_dualwind,attack_table[dual->value[3]].damage,true);
				found++;
			}
			else
				damage(ch,victim,0,gsn_dualwind,attack_table[dual->value[3]].damage,true);
		}
	}

	if(found)
		check_improve(ch,sn,true,1);
	else
		check_improve(ch,sn,false,1);

	WAIT_STATE(ch,skill_table[sn].beats * found);
}

void do_improvise(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj;
	int dam,sn = gsn_improvise,skill = get_skill(ch,sn);
	char arg[MIL];
	int dsize,dnum,ihr,idr,ow,ol;


	ch->send("Not yet, sorry.\n\r");
	return;
	argument = one_argument(argument,arg);

	if(!arg[0] || skill < 1){
		ch->send("You SMASH... what with smash you?\n\r");
		return;
	}

	if (!(obj = get_obj_carry(ch,arg,ch))){
		ch->send("You don't have that.\n\r");
		return;
	}

	if(calcHands(ch) > 1){
		ch->send("You need to free up a hand.\n\r");
		return;
	}

	ow = UMIN(UMAX(obj->weight,1),200);
	ol = UMIN(UMAX(obj->level,1),100);
	if(roll_chance(ch,skill)){
		act("You improvise $p into a weapon.",ch,obj,NULL,TO_CHAR);
		act("$n improvises $p into a weapon.",ch,obj,NULL,TO_ROOM);
		obj->item_type = ITEM_WEAPON;
		dsize = UMAX(ol/4,1);
		dnum = UMAX(ol/10,1);
		ihr = 15 - (ow/20);
		idr = ol / 9;
		obj->value[0] = WEAPON_EXOTIC;
		obj->value[1] = dnum;
		obj->value[2] = dsize;
		obj->value[3] = attack_lookup("hit");
		SET_BIT(obj->wear_flags,ITEM_WIELD);
		SET_BIT(obj->wear_flags,ITEM_TAKE);
		wear_obj(ch,obj,true,true);
		check_improve(ch,gsn_improvise,true,1);
	}
	else{
		ch->send("You fail.\n\r");
		check_improve(ch,gsn_improvise,false,2);
	}

	obj->timer = 2;
	//make it set the extra flag to some kind of weapon degrading flag
}

int check_brawler(CHAR_DATA *ch,int dam){
	OBJ_DATA *obj;
	int sn = gsn_brawler,skill = get_skill(ch,sn);

	if(skill < 1)
		return dam;

	if((obj = get_eq_char(ch,WEAR_WIELD)) && ((obj->item_type == ITEM_WEAPON && obj->value[0] != WEAPON_GAUNTLET) || obj->item_type == ITEM_SHIELD))
		return dam;
	if((obj = get_eq_char(ch,WEAR_SECONDARY)) && ((obj->item_type == ITEM_WEAPON && obj->value[0] != WEAPON_GAUNTLET) || obj->item_type == ITEM_SHIELD))
		return dam;

	skill *= .8;
	if(roll_chance(ch,skill)){
		if(number_percent() < 50)
			dam += dam * ch->getslvl(sn) / 5;
		else
			dam += dam * ch->getslvl(sn) / 7;
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	return dam;
}

void brawler_faller(CHAR_DATA *ch,CHAR_DATA *victim){
	int sn = gsn_brawler,skill = get_skill(ch,sn);

	if((IS_NPC(victim) && !victim->isoff(OF_BRAWL)) || skill < 1)
		return;

	skill /= 6;
	skill += get_curr_stat(victim,STAT_STR) - get_curr_stat(ch,STAT_WIS);

	if(roll_chance(victim,skill)){
		act("As $N falls, $E suddenly grabs you and pulls you with $M!",ch,NULL,victim,TO_CHAR);
		act("As you fall, you grab $n and drag $m down with you!",ch,NULL,victim,TO_VICT);
		act("$N falls, grabbing $n and dragging $m down too!",ch,NULL,victim,TO_NOTVICT);
		ch->bashwait = victim->getslvl(sn) * 3;
		ch->position = POS_RESTING;
		check_improve(victim,sn,true,1);
	}
	else
		check_improve(victim,sn,false,1);
}

bool check_blind_fighting(CHAR_DATA *ch,CHAR_DATA *victim){
	int dam,sn = gsn_blind_fighting,skill = get_skill(ch,sn);
	
	if(can_see(ch,victim))
		return true;
	if(skill < 1 || IS_NPC(ch))
		return false;

	if(roll_chance(ch,skill/2)){
		check_improve(ch,sn,true,1);
		return true;
	}
	else
		check_improve(ch,sn,false,1);
	return false;
}

int combat_proficiency(CHAR_DATA *ch){
	int dam,sn = gsn_combat_proficiency,skill = get_skill(ch,sn);

	if(IS_NPC(ch) || skill < 1)
		return 0;
	dam = skill / (15 - (2*ch->getslvl(sn)));

	return dam;
}

int check_furor(CHAR_DATA *ch){
	CHAR_DATA *victim;
	int dam = 0,dprc,sn = gsn_furor,skill = get_skill(ch,sn);

	if(IS_NPC(ch) || skill < 1 || !(victim = ch->fighting))
		return dam;

	skill /= 4;

	if(roll_chance(ch,skill)){
		dprc = ((victim->hit * 100) / victim->max_hit);
		if(dprc < 20 && number_percent() < 25){
			act("$N's weakened state drives you crazy!",ch,NULL,victim,TO_CHAR);
			act("$n moves with crazed bloodlust!",ch,NULL,NULL,TO_ROOM);
			dprc = dprc - 5 < 1 ? dprc : dprc-2;
		}
		dam = UMAX((100 - dprc)/10,1);
		dam += ch->getslvl(sn);
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	return dam;
}

void check_paired_attack(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	OBJ_DATA *dual;
	int sn = gsn_paired_attack,skill = get_skill(ch,sn);

	if(skill < 1 || !victim || !(dual = get_eq_char(ch,WEAR_SECONDARY)))
		return;

	if(dam < 1)
		dam = melee_hit(ch,victim,(dice(dual->value[1],dual->value[2])/3) * ch->getslvl(sn),true);

	skill /= 5;

	if(roll_chance(ch,skill)){
		skill_damage(ch,victim,dam,sn,attack_table[dual->value[3]].damage,true);
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
}

void do_double_blitz(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD),*dual = get_eq_char(ch,WEAR_SECONDARY);
	int dam,dam2,sn = gsn_double_blitz,skill = get_skill(ch,sn),skill2 = skill;

	if(skill < 1){
		ch->send("TACKLE!\n\r");
		return;
	}

	if(!(victim = grab_char(ch,argument,true)))
		return;

	if(!wield || !dual || wield->item_type != ITEM_WEAPON || dual->item_type != ITEM_WEAPON){
		ch->send("You must be wielding two weapons to perform this.\n\r");
		return;
	}

	//skill = (skill * get_skill(ch,get_weapon_skill(ch,get_weapon_sn(ch,false)))) / (125 - (ch->getslvl(sn)*5));
	//skill2 = (skill2 * get_skill(ch,get_weapon_skill(ch,get_weapon_sn(ch,true)))) / (150 - (ch->getslvl(sn)*5));


	if(roll_chance(ch,skill)){
		dam  = melee_hit(ch,victim,dice(wield->value[1],wield->value[2]),false);
		damage(ch,victim,number_range(dam,dam*1.25),sn,attack_table[wield->value[3]].damage,true);
		check_improve(ch,sn,true,1);
	}
	else{
		damage(ch,victim,0,sn,DAM_NONE,true);
		check_improve(ch,sn,false,1);
	}

	if(roll_chance(ch,skill2)){
		dam2 = melee_hit(ch,victim,dice(dual->value[1],dual->value[2]),true);
		skill_damage(ch,victim,number_range(dam2,dam2*1.25),sn,attack_table[dual->value[3]].damage,true);
		check_improve(ch,sn,true,1);
	}
	else{
		damage(ch,victim,0,sn,DAM_NONE,true);
		check_improve(ch,sn,false,1);
	}
	WAIT_STATE(ch,skill_table[sn].beats - ch->getslvl(sn));
}

void check_batter(CHAR_DATA *ch,CHAR_DATA *victim){
	CHAR_DATA *vch,*vch_next;

	act("You roll from the force of the blow!",victim,NULL,NULL,TO_CHAR);
	act("$n rolls from the force of the blow!",victim,NULL,NULL,TO_ROOM);
	if(victim->in_room != ch->in_room)
		return;
	vch = get_random_char(ch,NULL,NULL);
	act("$n",ch,NULL,NULL,TO_ROOM);
	act("$n",ch,NULL,NULL,TO_CHAR);
	if(vch == victim || vch == ch || !canFight(victim,vch,false))
		return;
	if(is_same_group(ch,vch))
		return;

	act("You fly into $N, knocking $M over!",victim,NULL,vch,TO_CHAR);
	act("$n flies into you, knocking you over!",victim,NULL,vch,TO_VICT);
	act("$n flies into $N, knocking $M over!",victim,NULL,vch,TO_NOTVICT);
	damage(victim,vch,(victim->level/2)*get_skill(ch,gsn_batter),gsn_batter,DAM_BASH,true);
	if(!do_trample(vch))
		do_facekick(ch,vch);

	WAIT_STATE(vch,PULSE_VIOLENCE);
	vch->position = POS_RESTING;
	DAZE_STATE(vch,PULSE_VIOLENCE*30);
	vch->bashwait = 26;
    check_dehorsing(vch);
}

void do_batter(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int dam,cAC,cSTR = get_curr_stat(ch,STAT_STR),vSTR,sn = gsn_batter,skill = get_skill(ch,sn);

	if(skill < 1){
		ch->send("You can't make cake, sorry.\n\r");
		return;
	}
	if(!(victim = grab_char(ch,argument,true)))
		return;

    if (victim->position < POS_FIGHTING || victim->bashwait > 0){
		act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
		return;
    }

	if (!get_eq_char(ch,WEAR_SHIELD)){
		send_to_char("You must be wearing a shield to do such a thing!\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	vSTR = get_curr_stat(victim,STAT_STR);
	skill *= .75;
	skill += (cSTR - vSTR) / 2;
	cAC = GET_AC(victim,AC_BASH);
	if (cAC < -800)
		cAC = -800;
	else if (cAC > 200)
		cAC = 200;
	skill += cAC / 100;
    skill = crunch_lead_stance(ch,victim,skill);

	if(roll_chance(ch,skill)){
		act("You rush $N, battering $M down with your shield!",ch,NULL,victim,TO_CHAR);
		act("$n rushes you, battering you down with $s shield!",ch,NULL,victim,TO_VICT);
		act("$n rushes $N, battering $M down with $s shield!",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,number_range(50,50 + 3 * cSTR + ch->level/20),sn,DAM_BASH,true);
		check_improve(ch,sn,true,1);
		if(!do_trample(victim))
			do_facekick(ch,victim);

		WAIT_STATE(victim,PULSE_VIOLENCE);
		WAIT_STATE(ch,PULSE_VIOLENCE * 2);
		victim->position = POS_RESTING;
		DAZE_STATE(victim,PULSE_VIOLENCE*30);
		victim->bashwait = 26;
        check_dehorsing(victim);
		check_batter(ch,victim);
		check_improve(ch,sn,true,1);
	}
	else{
		act("You rush $N, but miss and fall down!",ch,NULL,victim,TO_CHAR);
		act("$n rushes you, but misses and falls face first!",ch,NULL,victim,TO_VICT);
		act("$n rushes $N, missing and falling face first to the ground!",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,0,sn,DAM_BASH,true);
		if(!do_downstrike(victim,ch))
			if(!do_trample(ch))
				do_facekick(victim,ch);
		ch->bashwait = 24;
		WAIT_STATE(ch,PULSE_VIOLENCE * 1.5);
		check_improve(ch,sn,false,1);
	}
	WAIT_STATE(ch,skill_table[sn].beats);
}

void do_bulwark(CHAR_DATA *ch,char *argument){
	int dam,sn = gsn_bulwark,skill = get_skill(ch,sn);
    AFFECT_DATA af;

	if(skill < 1){
		ch->send("You need to eat more veggies to be that defensive.\n\r");
		return;
	}
	if (is_affected(ch,sn)){
		ch->send("You are already bul...warky?\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		affect_set(ch,TO_AFFECTS,sn,ch->level,1,5 + (UMAX(1,ch->getslvl(sn)) * 2),APPLY_NONE,0,AF_BULWARK);
		act("$n bulwarks $mself.",ch,NULL,NULL,TO_ROOM);
		ch->send("You bulwark yourself.\n\r");
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	WAIT_STATE(ch,skill_table[sn].beats);
}

void do_defensive_stance(CHAR_DATA *ch,char *argument){
	int sn = gsn_defensive_stance,skill = get_skill(ch,sn);
    AFFECT_DATA af;

	if(skill < 1){
		ch->send("You wave your hands and fail miserably at being defensive.\n\r");
		return;
	}
	if (ch->isaff(AF_DEFENDER)){
		ch->send("You are already defensive.\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		affect_set(ch,TO_AFFECTS,sn,ch->level,1,5 + (UMAX(1,ch->getslvl(sn)) * 2),APPLY_HITROLL,-(ch->level/5),AF_DEFENDER);
		act("$n assumes a defensive stance.",ch,NULL,NULL,TO_ROOM);
		ch->send("You assume a defensive stance.\n\r");
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	WAIT_STATE(ch,skill_table[sn].beats);
}

void do_warrior_heart(CHAR_DATA *ch,char *argument){
	int dam,sn = gsn_warrior_heart,skill = get_skill(ch,sn);
	AFFECT_DATA af;

	if(IS_NPC(ch) || skill < 1)
		return;

	if(is_affected(ch,sn)){
		ch->send("You are already a warrior at heart!\n\r");
		return;
	}
	if(ch->move < 25){
		ch->send("You don't have the energy to do this.\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		ch->move -= 25;
		act("You let out a battle cry and your eyes burn with ferocity!",ch,NULL,NULL,TO_CHAR);
		act("$n lets out a battle cry and $s eyes burn with ferocity!",ch,NULL,NULL,TO_ROOM);
		dam = ch->max_hit;
		dam *= (ch->getslvl(sn)*2);
		dam /= 100;

		affect_set(ch,TO_AFFECTS,sn,ch->level,1,UMAX(1,ch->getslvl(sn)) * 3,APPLY_HIT,dam,0);
		affect_set(ch,TO_AFFECTS,sn,ch->level,1,UMAX(1,ch->getslvl(sn)) * 3,APPLY_HITROLL,ch->getslvl(sn) * 2,0);
		affect_set(ch,TO_AFFECTS,sn,ch->level,1,UMAX(1,ch->getslvl(sn)) * 3,APPLY_DAMROLL,ch->getslvl(sn) * 2,0);
		ch->hit += dam;
	}
	else{
		ch->move -= 12;
		act("You thrash your head and gnash your teeth!",ch,NULL,NULL,TO_CHAR);
		act("$n thrashes $s head and gnashes $s teeth! Scary!?",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,sn,false,1);
	}
	WAIT_STATE(ch,skill_table[sn].beats);
}

bool check_absorb(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
    int chance;

    if ( !IS_AWAKE(victim) || !canclass_absorb(victim) || dam < 2)
		return false;

    chance = get_skill(victim,gsn_absorb) *.35;

	if (IS_NPC(victim)){
		if (victim->level > 60)
			chance /=5;
		else
			chance=0;
	}

    if (!can_see(victim,ch))
		chance /= 2;

	//stats were here
	
    if ( number_percent() >= chance || ch == victim){
		check_improve(victim,gsn_absorb,false,1);
		return false;
	}
	else{
		act("You tighten your muscles and absorb $n's attack.",ch,NULL,victim,TO_VICT);
		act("$N absorbs $n's attack.",ch,NULL,victim,TO_NOTVICT);
		act("$N absorbs your attack.",ch,NULL,victim,TO_CHAR);
		check_improve(victim,gsn_absorb,true,1);
		return true;
	}
}

void do_facekick(CHAR_DATA *ch,CHAR_DATA *victim){
	int dam,chance = get_skill(ch,gsn_facekick)*.75,tempShit = number_range(ch->level/2, ch->level)/2;

	if (chance < 1 || MOUNTED(ch) || IS_NPC(ch))
		return;

	dam = number_range(1,ch->level)*.5;
	//stats were here

	if (number_percent() >= chance)
		check_improve(ch,gsn_facekick,false,1);
	else{
		act("You feel $n's foot slam into your face as you fall to the ground!",ch,NULL,victim,TO_VICT);
		act("You kick hard into $N's face as $E falls to the ground!",ch,NULL,victim,TO_CHAR);
		act("$n kicks up hard into $N's face as $E hits the ground.",ch,NULL,victim,TO_NOTVICT);
		check_improve(ch,gsn_downstrike,true,1);
		skill_damage(ch, victim, dam, gsn_facekick, DAM_BASH, true);
		check_paired_attack(ch,victim,dam*2);
	}
}

int check_rage(CHAR_DATA *ch,int tdam){
	int dam = 0;
	if(ch->fighting == NULL)
		return dam;
	if (!IS_NPC(ch) && get_skill(ch,gsn_rage) > 0 ){
		if (number_percent() <= get_skill(ch,gsn_rage) * .5){
			if (ch->hit <= (ch->max_hit / 2)){
				if (ch->hit <= (ch->max_hit / 3)){
					if (ch->hit <= (ch->max_hit / 4)){
						if (ch->hit <= (ch->max_hit / 5)){
							if (number_percent() < 75){
								send_to_char("You feel your {Rrage {xerupt!\n\r", ch);
								dam = tdam / 3 * ch->getslvl(gsn_rage);
							}
							else{
								act("$n screams as $s {Rrage {xexplodes at you in a fury!",ch,NULL,ch->fighting,TO_VICT);
								act("$n screams in {Rrage {xas $e attacks $N!",  ch, NULL,ch->fighting,TO_NOTVICT);
								send_to_char("You feel your rage {Rexplode{x!\n\r", ch);
								dam = tdam / 2 * ch->getslvl(gsn_rage);
							}
						}
						else{
							send_to_char("You feel your {Rrage {xsurge!\n\r", ch);
							dam = tdam / 4 * ch->getslvl(gsn_rage);
						}
					}
					else{
						send_to_char("You feel your {Rrage {xboil!\n\r", ch);
						dam = tdam / 5 * ch->getslvl(gsn_rage);
					}
				}
				else{
					send_to_char("You feel your {Rrage {xincrease!\n\r", ch);
					dam = tdam / 6 * ch->getslvl(gsn_rage);
				}
			}
			check_improve(ch,gsn_rage,true,1);
		}
		else
			check_improve(ch,gsn_rage,false,1);
	}
	return dam;
}

void do_berserk( CHAR_DATA *ch, char *argument){
	int chance, hp_percent;

	if ((chance = get_skill(ch,gsn_berserk)) < 1 || (IS_NPC(ch) && !ch->isoff(OF_BERSERK))){
		send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
		return;
	}
	if (ch->isaff(AF_BERSERK) || is_affected(ch,gsn_berserk) || is_affected(ch,skill_lookup("frenzy"))){
		send_to_char("You get a little madder.\n\r",ch);
		return;
	}
	if (ch->isaff(AF_CALM)){
		send_to_char("You're feeling to mellow to berserk.\n\r",ch);
		return;
	}
	if (ch->getmana() < 50){
		send_to_char("You can't get up enough energy.\n\r",ch);
		return;
	}

	if (ch->position == POS_FIGHTING)
		chance += 10;

	hp_percent = 100 * ch->hit/ch->max_hit;
	chance += 25 - hp_percent/2;

	if (number_percent() < chance){
		AFFECT_DATA af;

		WAIT_STATE(ch,PULSE_VIOLENCE);
		ch->modmana(-50);
		ch->move /= 2;

		ch->hit += ch->level * 2;
		ch->hit = UMIN(ch->hit,ch->max_hit);

		send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
		act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,gsn_berserk,true,2);

		affect_set(ch,TO_AFFECTS,gsn_berserk,ch->level,1,number_fuzzy(ch->level / 8),APPLY_HITROLL,UMAX(1,ch->level/5),AF_BERSERK);

		affect_set(ch,TO_AFFECTS,gsn_berserk,ch->level,1,number_fuzzy(ch->level / 8),APPLY_DAMROLL,UMAX(1,ch->level/5),0);

		affect_set(ch,TO_AFFECTS,gsn_berserk,ch->level,1,number_fuzzy(ch->level / 8),APPLY_AC,UMAX(10,10 * (ch->level/5)),0);
	}

	else{
		WAIT_STATE(ch,3 * PULSE_VIOLENCE);
		ch->modmana(-25);
		ch->move /= 2;

		send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
		check_improve(ch,gsn_berserk,false,2);
	}
}

void do_guard(CHAR_DATA *ch,char*argument){
	CHAR_DATA *victim;
	int chance = get_skill(ch,gsn_guard);

	if(!argument[0]){
		ch->send("HRM?");
		return;
	}
	if (!(victim = grab_char(ch,argument,false)))
		return;

	if (victim == ch){
		if (ch->guarding){
			act("You stop guarding $N.",ch,NULL,ch->guarding,TO_CHAR);
			act("$n stops guarding you.",ch,NULL,ch->guarding,TO_VICT);
			act("$n stops guarding $N.",ch,NULL,ch->guarding,TO_NOTVICT);
			ch->guarding->guardby = NULL;
			ch->guarding = NULL;
		}
		else
			ch->send("You're guarding yourself, bravo!\n\r");
		return;
	}

	if (number_percent() < chance){
		act("You now defend $N.",ch,NULL,victim,TO_CHAR);
		act("$n now defends you.",ch,NULL,victim,TO_VICT);
		act("$n now defends $N.",ch,NULL,victim,TO_NOTVICT);
		victim->guardby = ch;
		ch->guarding = victim;
	}
	else{
		ch->send("You fail.\n\r");
		act("$n dances around $N like an idiot for a moment...",ch,NULL,victim,TO_NOTVICT);
		act("$n dances around you like an idiot for a moment...weird.",ch,NULL,victim,TO_VICT);
	}
	WAIT_STATE(ch,skill_table[gsn_guard].beats);
}

void do_shieldtoss(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *shield = get_eq_char(ch,WEAR_SHIELD);
	char arg[MSL];
	int ndepth = 4,chance = get_skill(ch,gsn_shieldtoss) * .7,dam = 0;

	one_argument(argument,arg);

	if (!(victim = get_char_around(ch,arg,&ndepth,true))){
		ch->send("They're not near!\n\r");
		return;
	}
	if (victim->in_room == ch->in_room){
		ch->send("They're too close..\n\r");
		return;
	}

	if (!shield){
		if (!(shield = get_eq_char(ch,WEAR_SECONDARY)) || shield->item_type != ITEM_SHIELD){
			ch->send("With what?\n\r");
			return;
		}
	}

	chance = (chance * get_curr_stat(ch,STAT_AGI)) / (STAT_MAX * .75);
	chance += get_curr_stat(ch,STAT_STR)/10;

	if (number_percent() < chance){
		dam = ((shield->weight + get_curr_stat(ch,STAT_STR))/2) + shield->level;
		act("With a mighty swing, you hurl $p at $N and peg $M!",ch,shield,victim,TO_CHAR);
		act("$p flies in and pegs you in the back!",ch,shield,victim,TO_VICT);
		act("$n hurls $p with a mighty swing and you hear a pained scream!",ch,shield,NULL,TO_ROOM);
		act("$p flies in and pegs $n in the back!",victim,shield,NULL,TO_ROOM);
		WAIT_STATE(ch,skill_table[gsn_shieldtoss].beats);
		WAIT_STATE(victim,skill_table[gsn_shieldtoss].beats*2);
		check_improve(ch,gsn_shieldtoss,true,4);
	}
	else{
		act("With a mighty swing, you hurl $p at $N and miss!",ch,shield,victim,TO_CHAR);
		act("You see $p fly right past you!",ch,shield,victim,TO_VICT);
		act("$n hurls $p and you hear... nothing!",ch,shield,NULL,TO_ROOM);
		act("You see $p fly by $n's head and clatter to the ground!",victim,shield,NULL,TO_ROOM);
		WAIT_STATE(ch,skill_table[gsn_shieldtoss].beats/2);
		check_improve(ch,gsn_shieldtoss,false,5);
	}
	skill_damage(ch,victim,dam,gsn_shieldtoss,DAM_OTHER,true);
	obj_from_char(shield);
	obj_to_room(shield,victim->in_room);
}

bool check_mithril_bash(CHAR_DATA *ch,CHAR_DATA *victim){
	int chance = get_skill(ch,gsn_mithril_bash) *.25;

	if (chance < 1) return false;

	chance += get_curr_stat(ch,STAT_STR) - get_curr_stat(victim,STAT_END);

	printf_to_char(ch,"MB: %d\n\r",chance);
	if (number_percent() < chance){
		act("You put some mithril into your bash.",ch,NULL,victim,TO_CHAR);
		act("$n seems to hit you even harder!",ch,NULL,victim,TO_VICT);
		act("$n slams hard at $N!",ch,NULL,victim,TO_NOTVICT);
		check_improve(ch,gsn_mithril_bash,true,2);
		return true;
	}
	else
		check_improve(ch,gsn_mithril_bash,false,2);
	return false;
}

bool check_damagereduction(CHAR_DATA *ch,CHAR_DATA *victim,int dam_type){
	int skill = get_skill(victim,gsn_damage_reduction) * .5,ac=200;

	if(IS_NPC(victim))
		return false;

	skill = skill * get_curr_stat(victim,STAT_END) / STAT_MAX;
	if (dam_type == DAM_BASH)
		ac = GET_AC(victim,AC_BASH);
	else if (dam_type == DAM_PIERCE)
		ac = GET_AC(victim,AC_PIERCE);
	else if (dam_type == DAM_SLASH)
		ac = GET_AC(victim,AC_SLASH);
	else
		ac = GET_AC(victim,AC_EXOTIC);

	skill += 20 * (ac - 200) / 1000;

	if(number_percent() < skill){
		check_improve(victim,gsn_damage_reduction,true,1);
		return true;
	}
	else{
		check_improve(victim,gsn_damage_reduction,false,1);
		return false;
	}
}

bool check_shieldmastery(CHAR_DATA *victim,CHAR_DATA *ch,OBJ_DATA *shield){
	int chance = get_skill(victim,gsn_shieldmastery) * .33;

	chance *= get_skill(victim,gsn_shield_block) / 100;
	if(get_eq_char(victim,WEAR_SECONDARY) == shield)
		chance *= get_skill(victim,gsn_dual_shield) / 100;

	chance = calcReflex(victim,ch,chance);
	if(victim->iscomm(CM_DEBUG))printf_to_char(victim,"{gShieldmastery({c%d{x)\n\r",chance);
	if (number_percent() < chance){
		check_improve(victim,gsn_shieldmastery,true,1);
		return true;
	}
	check_improve(victim,gsn_shieldmastery,false,1);
	return false;
}

void do_fortify(CHAR_DATA *ch,char *argument){
	AFFECT_DATA af;
	int chance = get_skill(ch,gsn_fortify);

	if (ch->isaff(AF_FORTIFY)){
		ch->send("You are already fortified.\n\r");
		return;
	}

	if (!get_eq_char(ch,WEAR_SHIELD) || !get_eq_char(ch,WEAR_SECONDARY) || get_eq_char(ch,WEAR_SECONDARY)->item_type != ITEM_SHIELD){
		ch->send("You need to wear two shields to do this.\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_fortify].beats);
	if (number_percent() < chance){
		act("$n hunkers down and fortifies $mself.",ch,NULL,NULL,TO_ROOM);
		ch->send("You hunker down and fortify yourself.\n\r");
		affect_set(ch,TO_AFFECTS,gsn_fortify,ch->level,1,ch->level,APPLY_AC,-(ch->level + get_curr_stat(ch,STAT_END)),AF_FORTIFY);
		check_improve(ch,gsn_fortify,true,3);
		return;
	}
	ch->send("You failed to find a good fortification position.\n\r");
	check_improve(ch,gsn_fortify,false,2);
}

int crunch_lead_stance(CHAR_DATA *ch,CHAR_DATA *victim,int chance){
	int skill = get_skill(victim,gsn_lead_stance) * .5;

	if (skill < 1 || chance < 1)
		return chance;

	skill = skill * (get_curr_stat(victim,STAT_END) + get_curr_stat(victim,STAT_END)) / (STAT_MAX * 2);

	if (number_percent() < skill){
		chance = chance * (100 - skill) / 100;
		victim->send("You take a lead-like stance.\n\r");
		act("$n takes a lead-like stance to the attack.",victim,NULL,NULL,TO_ROOM);
		check_improve(victim,gsn_lead_stance,true,5);
	}
		check_improve(victim,gsn_lead_stance,false,5);

	return chance;
}
