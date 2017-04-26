#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "olc.h"

void	arrow_hit			( CHAR_DATA*,CHAR_DATA*,OBJ_DATA*,int,int,bool );
void	arrow_damage		( CHAR_DATA*,CHAR_DATA*,OBJ_DATA*,int,int );
int		check_marksmanship	( CHAR_DATA* );
bool    check_shield_block	( CHAR_DATA*,CHAR_DATA* );
void	check_headshot		( CHAR_DATA *ch,CHAR_DATA *victim );
void	sharp_shoot			( CHAR_DATA *ch );
int		calcHands			( CHAR_DATA* );


CHAR_DATA *get_char_around(CHAR_DATA *ch,char *argument,int *maxdepth,bool truesight){
	CHAR_DATA *victim,*rch;
    ROOM_INDEX_DATA *was_in_room,*scan_room;
	EXIT_DATA *pExit;
	int door,depth;
	bool found = false;

	if (!argument[0])
		return NULL;

	for (door = 0; door < 6 && !found; door++){
		scan_room = ch->in_room;
		for (depth = 1; depth < *maxdepth && !found; depth++)
			if (!found && ((pExit = scan_room->exit[door]) != NULL)){
				scan_room = pExit->u1.to_room;

				if ((rch = get_char_room(NULL,scan_room,argument)) != NULL && rch != ch && !IS_SET(pExit->exit_info,EX_CLOSED) && (truesight || can_see(ch,rch))){
					*maxdepth = depth;
					victim = rch;
					found = true;
				}
			}
	}
	if (!found)
		return NULL;
	else
		return victim;
}

void do_draw(CHAR_DATA *ch,char *argument){
	OBJ_DATA *quiver,*arrow = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R));
	bool goodtogo = false;

	if(!(quiver = get_eq_char(ch,WEAR_QUIVER))){
		send_to_char("{xYou aren't wearing a quiver where you can get to it.\n\r",ch);
		return;
	}

	if(quiver->item_type != ITEM_QUIVER){
		send_to_char("{xYou can only draw arrows from a quiver.\n\r",ch);
		return;
	}
	if(quiver->value[0] <= 0)
		act("$p {xis now out of arrows, you need to find another one.",ch,quiver,NULL,TO_CHAR);

	if(calcHands(ch) > 1){
		if((arrow && arrow->item_type == ITEM_ARROW)){
			if((arrow->value[3] < 1 && get_skill(ch,gsn_doubleshot) > 1)
			|| (arrow->value[3] > 0 && get_skill(ch,gsn_arrow_shower) > 1 && arrow->value[3] < ch->getslvl(gsn_arrow_shower) * 3)){
				if(arrow->pIndexData->vnum != quiver->value[3]){
					ch->send("The arrow you hold does not match those in your quiver.\n\r");
					return;
				}
				goodtogo = true;
			}
			else{
				send_to_char("You really need a free hand to draw an arrow.\n\r",ch);
				return;
			}
		}
		else{
			send_to_char("You need a free hand to draw an arrow.\n\r",ch);
			return;
		}
	}

	if(get_eq_char(ch,WEAR_HOLD_R) && get_eq_char(ch,WEAR_HOLD_L)){
		send_to_char("Your hand is not empty!\n\r",ch);
		return;
	}
	if(quiver->value[3] < 46){
		send_to_char("You have a buggy quiver, get a new one.\n\r",ch);
		return;
	}

	if(quiver->value[0] > 0){
		WAIT_STATE(ch,PULSE_PER_SECOND);

		if(!goodtogo){
			act("$n draws an arrow from $p.",ch,quiver,NULL,TO_ROOM);
			act("You draw an arrow from $p.",ch,quiver,NULL,TO_CHAR);
			arrow = create_object(get_obj_index(quiver->value[3]),0);
			arrow->value[4] = quiver->value[4];
			obj_to_char(arrow,ch);
			wear_obj(ch,arrow,true,false);
		}
		else{
			act("$n draws another arrow from $p.",ch,quiver,NULL,TO_ROOM);
			act("You draw another arrow from $p.",ch,quiver,NULL,TO_CHAR);
			arrow->value[3]++;
		}

		if(quiver->level > 6){
			arrow->value[1] = arrow->level / 2;
			arrow->value[2] = ch->level / 10;
		}
		else{
			arrow->value[1] = 1;
			arrow->value[2] = 2;
		}
		quiver->value[0]--;
	}
}

void do_dislodge(CHAR_DATA *ch,char *argument){
	OBJ_DATA * arrow = NULL;
	int dam = 0;
	bool found = false;

	if (!argument[0]){
		send_to_char("Dislodge what?\n\r",ch);
		return;
	}

	if ((arrow = get_eq_char(ch,WEAR_LODGE_RIB)) != NULL){
		act("With a wrenching tug, you rip $p from your chest.",ch,arrow,NULL,TO_CHAR);
		act("With a wrenching tug, $n rips $p from $s chest.",ch,arrow,NULL,TO_ROOM);
		dam = dice((1.5 * arrow->value[1]),(1.5 * arrow->value[2]));
		found = true;
	}
	else{
		if ((arrow = get_eq_char(ch,WEAR_LODGE_ARM)) != NULL){
			act("With a wrenching tug, you rip $p from your arm.",ch,arrow,NULL,TO_CHAR);
			act("With a wrenching tug, $n rips $p from $s arm.",ch,arrow,NULL,TO_ROOM);
			dam = dice((1.5 * arrow->value[1]),arrow->value[2]);
			found = true;
		}
		else{
			if ((arrow = get_eq_char(ch,WEAR_LODGE_LEG)) != NULL){
				act("With a wrenching tug, you rip $p from your leg.",ch,arrow,NULL,TO_CHAR);
				act("With a wrenching tug, $n rips $p from $s leg.",ch,arrow,NULL,TO_ROOM);
				dam = dice(arrow->value[1], arrow->value[2]);
				found = true;
			}
			else
				send_to_char("You have nothing lodged in your body.\n\r",ch);
		}
	}
	if (found){
		REMOVE_BIT(arrow->extra_flags,ITM_NOREMOVE);
		unequip_char(ch,arrow);
		damage(ch,ch,dam,TYPE_HIT,DAM_OTHER,true);
		if (IS_OBJ_STAT(arrow,ITM_BARBED)){
			act("The barbing on $p tears at your flesh as you pull!",ch,arrow,NULL,TO_CHAR);
			act("The barbing on $p tears at $n's flesh as $e pulls!",ch,arrow,NULL,TO_ROOM);
			damage(ch,ch,UMAX(dam,20),TYPE_HIT,DAM_OTHER,true);
		}
	}
}

int get_armor_number(CHAR_DATA *ch){
	int num = 1,count = 0;
	OBJ_DATA *obj;

	for(int iWear = 0; iWear < MAX_WEAR; iWear++ ){
		if(!(obj = get_eq_char(ch,iWear)))
			continue;
		if(obj->item_type != ITEM_ARMOR)
			continue;
		num += obj->armortype_flags;
		count++;
	}
	return num / count;
}

bool check_trueshot(CHAR_DATA *ch){
	int skill = get_skill(ch,gsn_trueshot) / 4;
	if(skill < 1)
		return false;
	skill += get_curr_stat(ch,STAT_AGI) / (7 - ch->getslvl(gsn_trueshot));
	if(roll_chance(ch,skill)){
		ch->send("You focus and your arrow seems to know where to go.\n\r");
		act("$n focuses momentarily.",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,gsn_trueshot,true,1);
		return true;
	}
	check_improve(ch,gsn_trueshot,false,1);
	return false;
}

void do_longbowfire(CHAR_DATA *ch,char *arg){
	CHAR_DATA *victim=NULL,*vch;
	OBJ_DATA *arrow = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R)),*bow = get_eq_char(ch,WEAR_WIELD);
	EXIT_DATA *pexit;
	int dam=0,chance=0,ndepth=4;

	if (!str_cmp(arg,"none") || !str_cmp(arg,"self")){
		send_to_char("How exactly did you plan on firing an arrow at yourself?\n\r",ch);
		return;
	}

	if (!(victim = get_char_around(ch,arg,&ndepth,false))){
		ch->send("You don't see anyone like that anywhere nearby.\n\r");
		return;
	}

	if(ch->isaff(AF_STRAFE)){
		if(ch->move < 10){
			ch->send("You don't have the energy to do it.\n\r");
			return;
		}
		ch->send("You quickly fire.\n\r");
		WAIT_STATE(ch,skill_table[gsn_bows].beats/2);
		ch->move -= 10;
	}
	else
		WAIT_STATE(ch,skill_table[gsn_bows].beats);

	if(arrow->item_type == ITEM_ARROW){
		if (victim != NULL){
			for ( vch = victim->in_room->people; vch != NULL; vch = vch->next_in_room)
				if (vch != victim)
					printf_to_char(vch,"%s flies in at %s\n\r",arrow->short_descr,IS_NPC(victim) ? victim->short_descr : victim->name);
			for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
				if (vch != ch)
					printf_to_char(vch,"%s fires %s at %s\n\r",ch->name,arrow->short_descr,IS_NPC(victim) ? victim->short_descr : victim->name);
			act("{xYou fire $p at $N.",ch,arrow,victim,TO_CHAR);
			act("{x$n fires $p at you.",ch,arrow,victim,TO_VICT);
		}

		chance = (get_skill(ch,gsn_bows)*.55) + (check_marksmanship(ch)*5);
		printf_to_char(ch,"Bow %d\n\r",chance);
		chance = chance * get_curr_stat(ch,STAT_AGI) / STAT_MAX;
		printf_to_char(ch,"Bow %d\n\r",chance);

		ndepth *= 2;
		chance -= ndepth;
		ndepth /= 2;
		if(ch->isaff(AF_SHARPSHOOT)){
			if(ch->move < 10 || ch->mana < 10){
				ch->send("You lack the energy.\n\r");
				return;
			}
			ch->move -= 10;
			ch->modmana(-10);
			chance += ch->getslvl(gsn_sharp_shooting) * 2;
		}
		printf_to_char(ch,"Bow %d\n\r",chance);

		bool trueshot = check_trueshot(ch);
		if (number_percent() >= 10 + chance && !trueshot){
			if(number_percent() < 25 && (victim = get_random_char(ch,NULL,NULL)) && canFight(ch,victim,false)){//should probably code this to better check
				dam = (dice(arrow->value[1],arrow->value[2]) + dice(bow->value[1],bow->value[2]));
				ndepth *= 40;
				dam *= ndepth;
				dam /= 300;
				arrow_hit(ch,victim,arrow,dam,gsn_bows,trueshot);
			}
			else{
				act("{xYou fire $p missing, and it lands harmlessly on the ground.",ch,arrow,NULL,TO_CHAR);
				act("{x$n fires $p missing, and it lands harmlessly on the ground.",ch,arrow,NULL,TO_ROOM);
				obj_from_char(arrow);
				obj_to_room(arrow,victim->in_room);
			}
			check_improve(ch,gsn_bows,false,2);
		}
		else{
			dam = (dice(arrow->value[1],arrow->value[2]) + dice(bow->value[1],bow->value[2]));
			ndepth *= 40;
			dam *= ndepth;
			dam /= 100;
			arrow_hit(ch,victim,arrow,dam,gsn_bows,trueshot);
			check_improve(ch,gsn_bows,true,2);
		}
	}
}

void do_bowfire(CHAR_DATA *ch,char *arg){
	OBJ_DATA *arrow = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R)),*bow = get_eq_char(ch,WEAR_WIELD);
	int dam,door,chance,ndepth=2;
	CHAR_DATA *victim = get_char_around(ch,arg,&ndepth,false),*vch;
	EXIT_DATA *pexit;

	if(ch->isaff(AF_STRAFE)){
		if(ch->move < 10){
			ch->send("You don't have the energy to do it.\n\r");
			return;
		}
		ch->send("You quickly fire.\n\r");
		WAIT_STATE(ch,skill_table[gsn_bows].beats/2);
		ch->move -= 10;
	}
	else
		WAIT_STATE(ch,skill_table[gsn_bows].beats);

	if (!victim){
		ch->send("Fire at who?\n\r");
		return;
	}

	if(arrow->item_type == ITEM_ARROW){
		if (victim != NULL){
			for ( vch = victim->in_room->people; vch != NULL; vch = vch->next_in_room)
				if (vch != victim)
					printf_to_char(vch,"%s flies in at %s\n\r",arrow->short_descr,IS_NPC(victim) ? victim->short_descr : victim->name);
			for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
				if (vch != ch)
					printf_to_char(vch,"%s fires %s at %s\n\r",ch->name,arrow->short_descr,IS_NPC(victim) ? victim->short_descr : victim->name);
			act("You fire $p at $N.",ch,arrow,victim,TO_CHAR);
			act("$n fires $p at you.",ch,arrow,victim,TO_VICT);
		}

		chance = (get_skill(ch,gsn_bows)*.6) + (check_marksmanship(ch)*5);
		printf_to_char(ch,"Bow %d\n\r",chance);
		chance = chance * get_curr_stat(ch,STAT_AGI) / STAT_MAX;
		printf_to_char(ch,"Bow %d\n\r",chance);
		if(ch->isaff(AF_SHARPSHOOT)){
			if(ch->move < 10 || ch->mana < 10){
				ch->send("You lack the energy.\n\r");
				return;
			}
			ch->move -= 10;
			ch->modmana(-10);
			chance += ch->getslvl(gsn_sharp_shooting) * 2;
		}
		printf_to_char(ch,"Bow %d\n\r",chance);

		bool trueshot = check_trueshot(ch);
		if(number_percent() >= 10 + chance && !trueshot){
			act("You fire $p and miss; it clatters across the ground.",ch,arrow,NULL,TO_CHAR);
			act("$n fires $p and misses; it clatters across the ground.",ch,arrow,NULL,TO_ROOM);
			obj_from_char(arrow);
			obj_to_room(arrow,victim->in_room);
			check_improve(ch,gsn_bows,false,2);
		}
		else{
			dam = (dice(arrow->value[1],arrow->value[2]) + dice(bow->value[1],bow->value[2])) *.75;
			arrow_hit(ch,victim,arrow,dam,gsn_bows,trueshot);
			check_improve(ch,gsn_bows,true,2);
		}
	}
}

void do_shortbowfire(CHAR_DATA *ch,CHAR_DATA *victim){
	CHAR_DATA *rch;
	OBJ_DATA *arrow,*bow;
	int dam,chance;

	bow = get_eq_char(ch,WEAR_WIELD);
	arrow = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R));

	if (!victim){
		send_to_char("You don't see that person anywhere..\n\r",ch);
		return;
	}

	if(ch->isaff(AF_STRAFE)){
		if(ch->move < 10){
			ch->send("You don't have the energy to do it.\n\r");
			return;
		}
		ch->send("You quickly fire.\n\r");
		WAIT_STATE(ch,skill_table[gsn_bows].beats/2);
		ch->move -= 10;
	}
	else
		WAIT_STATE(ch,skill_table[gsn_bows].beats);

	if(arrow->item_type== ITEM_ARROW){
		if (victim != NULL){
			act("{x$n fires $p at $N",ch,arrow,victim,TO_NOTVICT);
			act("{xYou fire $p at $N.",ch,arrow,victim,TO_CHAR);
			act("{x$n fires $p at you.",ch,arrow,victim,TO_VICT);
		}

		chance = (get_skill(ch,gsn_bows)*.7) +  + (check_marksmanship(ch)*5);
		printf_to_char(ch,"Bow %d\n\r",chance);
		chance = chance * get_curr_stat(ch,STAT_AGI) / STAT_MAX;
		printf_to_char(ch,"Bow %d\n\r",chance);
		if(ch->isaff(AF_SHARPSHOOT)){
			if(ch->move < 10 || ch->mana < 10){
				ch->send("You lack the energy.\n\r");
				return;
			}
			ch->move -= 10;
			ch->modmana(-10);
			chance += ch->getslvl(gsn_sharp_shooting) * 2;
		}
		printf_to_char(ch,"Bow %d\n\r",chance);

		bool trueshot = check_trueshot(ch);
		if (number_percent() >= 10 + chance && !trueshot){
			act("{xYou fire $p missing, and it lands harmlessly on the ground.",ch,arrow,NULL,TO_CHAR);
			act("{x$n fires $p missing, and it lands harmlessly on the ground.",ch,arrow,NULL,TO_ROOM);
			obj_from_char(arrow);
			obj_to_room(arrow,victim->in_room);
			check_improve(ch,gsn_bows,false,2);
		}
		else{
			dam = (dice(arrow->value[1],arrow->value[2]) + dice(bow->value[1],bow->value[2])) *.6;
			arrow_hit(ch,victim,arrow,dam,gsn_bows,trueshot);
			check_improve(ch,gsn_bows,true,2);
		}
	}
}

void arrow_hit(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *arrow,int dam,int dt,bool trueshot)
{
	int chance;
	chance=number_percent();

	if(chance <= 10){
		if(get_eq_char(victim,WEAR_LODGE_LEG) == NULL){
			obj_from_char(arrow);
			arrow->wear_loc = WEAR_LODGE_LEG;
			SET_BIT(arrow->extra_flags,ITM_NOREMOVE);
			obj_to_char(arrow,victim);
			equip_char(victim,arrow,arrow->wear_loc);
			dam = 2 * dam;
		}
		else{
			extract_obj(arrow);
			dam = 2.2 * dam;
		}
	}
	else if(chance <= 25){
		if(get_eq_char(victim,WEAR_LODGE_RIB) == NULL){
			obj_from_char(arrow);
			arrow->wear_loc = WEAR_LODGE_RIB;
			SET_BIT(arrow->extra_flags,ITM_NOREMOVE);
			obj_to_char(arrow,victim);
			equip_char(victim,arrow,arrow->wear_loc);
			dam = 3 * dam / 2;
		}
		else{
			extract_obj(arrow);
			dam = 3 * dam * .75;
		}
	}
	else if(chance <= 50)
	{
		if(get_eq_char(victim,WEAR_LODGE_ARM) == NULL){
			obj_from_char(arrow);
			arrow->wear_loc = WEAR_LODGE_ARM;
			SET_BIT(arrow->extra_flags,ITM_NOREMOVE);
			obj_to_char(arrow,victim);
			equip_char(victim,arrow,arrow->wear_loc);
		}
		else{
			extract_obj(arrow);
			dam *= 1.1;
		}
	}
	else if(chance <= 100){
		obj_from_char(arrow);
		arrow->wear_loc = WEAR_NONE;
		obj_to_room(arrow,victim->in_room);
	}
	if(trueshot)
		dam *= 1.5;
	arrow_damage(ch,victim,arrow,dam,dt);
}

void arrow_damage(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *arrow,int dam,int dt){
	int chance;

	if(check_shield_block(ch,victim))
		return;
	damage(ch,victim,dam,dt,DAM_PIERCE,true);
	switch (arrow->value[4]){
		case 0:
			break;
		case 1:
			dam = arrow->level;
			act("$n is {rburned{x by $p.",victim,arrow,ch,TO_NOTVICT);
			act("$n is {rburned{x by $p.",victim,arrow,ch,TO_VICT);
			act("$p {rsears {xyour flesh.",victim,arrow,NULL,TO_CHAR);
			fire_effect(/*(void *)*/ victim,arrow->level*2,dam,TARGET_CHAR);
			damage(ch,victim,dam,0,DAM_FIRE,false);
			break;
		case 2:
			dam = arrow->level;
			act("$p {Cfreezes {x$n.",victim,arrow,ch,TO_NOTVICT);
			act("$p {Cfreezes {x$n.",victim,arrow,ch,TO_VICT);
			act("The {Ccold touch {xof $p surrounds you with {Cice{x.",victim,arrow,NULL,TO_CHAR);
			cold_effect(victim,arrow->level*2,dam,TARGET_CHAR);
			damage(ch,victim,dam,0,DAM_COLD,false);
			break;
		case 3:
			dam = arrow->level;
			act("$n is struck by {Ylightning {xfrom $p.",victim,arrow,ch,TO_NOTVICT);
			act("$n is struck by {Ylightning {xfrom $p.",victim,arrow,ch,TO_VICT);
			act("You are {Yshocked {xby $p.",victim,arrow,NULL,TO_CHAR);
			shock_effect(victim,arrow->level*2,dam,TARGET_CHAR);
			damage(ch,victim,dam,0,DAM_LIGHTNING,false);
			break;
		case 4:
			chance = number_percent();
			if (chance > 25)
				dam *= .25;
			else if (chance > 50)
				dam *= .5;
			else if (chance > 75)
				dam = dam;
			damage(ch,victim,dam,dt,DAM_PIERCE,false);
			break;
		case 5:
			dam = arrow->level;
			act("$p shoots {mpoison {xinto $n.",victim,arrow,NULL,TO_NOTVICT);
			act("$p shoots {mpoison {xinto $n.",victim,arrow,ch,TO_VICT);
			act("The {mpoison {xof $p {xseeps into your {Rveins{x.",victim,arrow,NULL,TO_CHAR);
			poison_effect(victim,arrow->level*3,dam,TARGET_CHAR);
			damage(ch,victim,dam,0,DAM_POISON,false);
			break;
		default:
			break;
	}
	check_headshot(ch,victim);
}

int check_marksmanship(CHAR_DATA *ch){
	int chance,bonus=0;

	chance = get_skill(ch,gsn_marksmanship) * .4;
	chance += get_curr_stat(ch,STAT_AGI) / 4;
	chance += get_curr_stat(ch,STAT_STR) / 5;

	if (number_percent() <= chance){
		act("$n squints $s opened eye, taking careful aim.",ch,NULL,NULL,TO_ROOM);
		act("You squint your eye, taking careful aim.",ch,NULL,NULL,TO_CHAR);
		bonus = chance / 10;
	}
	return bonus;
}

void do_reload(CHAR_DATA *ch,char *argument){
	OBJ_DATA *arrow,*quiver,*arr_next;
	bool found = false;

	if(!(quiver = get_eq_char(ch,WEAR_QUIVER)) || quiver->item_type != ITEM_QUIVER){
		ch->send("You lack a quiver.\n\r");
		return;
	}
	if(!argument[0]){
		ch->send("Syntax: reload all/<arrow>\n\r");
		return;
	}
	WAIT_STATE(ch,50);
	if(!roll_chance(ch,ch->level*.75)){
		ch->send("You miss sticking the arrow into the quiver.\n\r");
		return;
	}

	if(!str_cmp(argument,"all")){
		for(arrow = ch->carrying;arrow;arrow = arr_next){
			arr_next = arrow->next_content;
			if(arrow->item_type != ITEM_ARROW)
				continue;
			act("$n loads $p into $P.",ch,arrow,quiver,TO_ROOM);
			act("You load $p into $P.",ch,arrow,quiver,TO_CHAR);
			quiver->value[0]++;
			obj_from_char(arrow);
			extract_obj(arrow);
		}
		return;
	}
	found = false;
	for(arrow = ch->carrying;arrow;arrow = arr_next){
		arr_next = arrow->next_content;
		if (is_name(argument,arrow->name) && can_see_obj(ch,arrow) && arrow->wear_loc == WEAR_NONE && can_drop_obj(ch,arrow)){
			found = true;
			act("$n loads $p into $P.",ch,arrow,quiver,TO_ROOM);
			act("You load $p into $P.",ch,arrow,quiver,TO_CHAR);
			obj_from_char(arrow);
			extract_obj(arrow);
			quiver->value[0]++;
		}
	}
	if(!found){
		ch->send("You have none of the appropriate arrows.\n\r");
		return;
	}
}

void do_strafe(CHAR_DATA *ch,char *argument){
	int sn = gsn_strafe,skill = get_skill(ch,sn);

	if(IS_NPC(ch) || skill < 1){
		ch->send("You don't know how.\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		affect_set(ch,TO_AFFECTS,sn,ch->level,1,UMAX(1,ch->getslvl(sn)),APPLY_AGI,ch->getslvl(sn),AF_STRAFE);

		ch->send("You concentrate, focusing on bows and arrows.\n\r");
		check_improve(ch,sn,true,1);
		WAIT_STATE(ch,skill_table[sn].beats);
		if(argument[0] && !str_prefix(argument,"sharpshooting"))
			sharp_shoot(ch);
		return;
	}
	ch->send("You failed.\n\r");
	check_improve(ch,sn,false,1);
	WAIT_STATE(ch,skill_table[sn].beats*2);
}

void sharp_shoot(CHAR_DATA *ch){
	int sn = gsn_sharp_shooting,skill = get_skill(ch,sn);

	if(IS_NPC(ch) || skill < 1){
		ch->send("You don't know how.\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		affect_set(ch,TO_AFFECTS,sn,ch->level,1,UMAX(1,ch->getslvl(sn)),APPLY_AGI,ch->getslvl(sn),AF_SHARPSHOOT);

		ch->send("You concentrate, focusing on your target.\n\r");
		check_improve(ch,sn,true,1);
		WAIT_STATE(ch,skill_table[sn].beats);
		return;
	}
	ch->send("You failed.\n\r");
	check_improve(ch,sn,false,1);
	WAIT_STATE(ch,skill_table[sn].beats*2);
}

void check_headshot(CHAR_DATA *ch,CHAR_DATA *victim){
	int sn = gsn_headshot,skill = get_skill(ch,sn) / 10;
	if(skill < 1)
		return;

	skill += get_curr_stat(ch,STAT_AGI) / 10;

	if(roll_chance(ch,skill)){
		act("You score a perfect headshot, stunning $N.",ch,NULL,victim,TO_CHAR);
		act("$n scores a perfect headshot, stunning you.",ch,NULL,victim,TO_VICT);
		act("$n scores a perfect headshot, stunning $N.",ch,NULL,victim,TO_NOTVICT);
		WAIT_STATE(victim,skill_table[sn].beats * ch->getslvl(sn));
		victim->bashwait = 2;
		check_improve(ch,sn,true,1);
	}
	check_improve(ch,sn,false,1);
}

void do_doubleshot(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *arrow){
	if(!ch || !victim || !arrow){
		ch->send("Wtheck arrow bug.\n\r");
		return;
	}
	int dam = 0,sn = gsn_doubleshot,skill = get_skill(ch,sn);
	OBJ_DATA *aclone,*bow = get_eq_char(ch,WEAR_WIELD);

	if(!bow || (bow->value[0] != WEAPON_LONGBOW)){
		ch->send("Bad bow.\n\r");
		return;
	}

	aclone = create_object(arrow->pIndexData,0);
	clone_object(arrow,aclone);
	aclone->value[3] = arrow->value[3] = 0;

	act("You fire two arrows at $N!",ch,NULL,victim,TO_CHAR);
	act("$n fires two arrows at you!",ch,NULL,victim,TO_VICT);
	act("$n fires two arrows at $N!",ch,NULL,victim,TO_NOTVICT);

	if(roll_chance(ch,skill)){
		dam = (dice(arrow->value[1],arrow->value[2]) + dice(bow->value[1],bow->value[2])) *.6;
		arrow_hit(ch,victim,arrow,dam,sn,false);
		check_improve(ch,sn,true,1);
	}
	else{
		act("$p clatters on the ground.",ch,arrow,NULL,TO_ROOM);
		act("$p clatters on the ground.",ch,arrow,NULL,TO_CHAR);
		obj_from_char(arrow);
		obj_to_room(arrow,victim->in_room);
		check_improve(ch,sn,false,1);
	}
	if(roll_chance(ch,skill)){
		dam = (dice(aclone->value[1],aclone->value[2]) + dice(bow->value[1],bow->value[2])) *.6;
		arrow_hit(ch,victim,aclone,dam,sn,false);
		check_improve(ch,sn,true,1);
	}
	else{
		act("$p clatters on the ground.",ch,aclone,NULL,TO_ROOM);
		act("$p clatters on the ground.",ch,aclone,NULL,TO_CHAR);
		obj_from_char(aclone);
		obj_to_room(aclone,victim->in_room);
		check_improve(ch,sn,false,1);
	}
}

void do_arrow_shower(CHAR_DATA *ch,OBJ_DATA *arrow){
	if(!ch || !arrow){
		ch->send("Wtheck arrow bug.\n\r");
		return;
	}
	CHAR_DATA *victim;
	OBJ_DATA *aclone[10],*bow = get_eq_char(ch,WEAR_WIELD);
	int tnum = 0,dam = 0,tries,arrow_count = arrow->value[3] + 1,sn = gsn_arrow_shower,skill = get_skill(ch,sn);

	if(!bow){
		ch->send("Bad bow.\n\r");
		return;
	}
	act("$n fires a vollow of arrows!",ch,NULL,NULL,TO_ROOM);
	act("You fire a vollow of arrows!",ch,NULL,NULL,TO_CHAR);
	for(;arrow->value[3] > -1;arrow->value[3]--){
		tnum = arrow->value[3];
		aclone[tnum] = create_object(arrow->pIndexData,0);
		clone_object(arrow,aclone[tnum]);
		aclone[tnum]->value[3] = 0;
		if(roll_chance(ch,skill)){
			for(tries = 0;tries < 10;tries++){
				if((victim = get_random_char(ch,NULL,NULL)) && !is_same_group(ch,victim) && canFight(ch,victim,false)){
					act("$p strikes $n!",victim,aclone[tnum],NULL,TO_ROOM);
					act("$p strikes you!",victim,aclone[tnum],NULL,TO_CHAR);
					dam = (dice(aclone[tnum]->value[1],aclone[tnum]->value[2]) + dice(bow->value[1],bow->value[2])) *.6;
					arrow_hit(ch,victim,aclone[tnum],dam,sn,false);
					check_improve(ch,sn,true,1);
					break;
				}
				else
					continue;
			}
		}
		else{
			act("$p clatters on the ground.",ch,aclone[tnum],NULL,TO_ROOM);
			act("$p clatters on the ground.",ch,aclone[tnum],NULL,TO_CHAR);
			obj_from_char(aclone[tnum]);
			obj_to_room(aclone[tnum],ch->in_room);
			check_improve(ch,sn,false,1);
		}
		skill *= 0.75;
	}
}

/* Bowfire code -- actual firing function */
void do_fire(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim = NULL;
	OBJ_DATA *arrow,*bow;

	argument = one_argument(argument,arg);

	if (!str_cmp(arg,"none") || !str_cmp(arg,"self")){
		ch->send("How exactly did you plan on firing an arrow at yourself?\n\r");
		return;
	}

	if(ch->bashwait > 0){
		ch->send("You have to stand up first.\n\r");
		return;
	}

	bow = get_eq_char(ch,WEAR_WIELD);

	if(bow == NULL){
		ch->send("What are you going to do, throw an arrow at them?\n\r");
		return;
	}

	if (((arrow = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R))) == NULL) || (arrow->item_type != ITEM_ARROW)){
		ch->send("You must be holding an arrow in your hand.\n\r");
		return;
	}

	if(arrow->value[3] > 0){
		if(arrow->value[3] == 1 && get_skill(ch,gsn_doubleshot) > 0){
			if(!(victim = get_char_around(ch,arg,1,false))){
				ch->send("You don't see that person anywhere..\n\r");
				return;
			}
			do_doubleshot(ch,victim,arrow);
		}
		else if(arrow->value[3] > 1 && get_skill(ch,gsn_arrow_shower) > 0)
			do_arrow_shower(ch,arrow);
		else
			ch->send("You can't fire that many arrows.\n\r");
		return;
	}

	if(!arg[0]){
		if (bow->value[0] == WEAPON_SHORTBOW && ch->fighting != NULL)
			do_shortbowfire(ch,ch->fighting);
		else
			ch->send("Fire an arrow at who?\n\r");
		return;
	}
	else{
		if(!(victim = get_char_world(ch,arg))){
			ch->send("You don't see that person anywhere..\n\r");
			return;
		}
		if((IS_NPC(victim) && victim->isact(AT_NOKILL)) || IS_SET(victim->in_room->room_flags,ROOM_SAFE)){
			ch->send("The gods block your aggression, pick on someone else.\n\r");
			return;
		}

		if (bow->value[0] == WEAPON_LONGBOW){
			if (victim->in_room == ch->in_room)
				ch->send("They're too close!\n\r");
			else
				do_longbowfire(ch,arg);
			return;
		}
		else if (bow->value[0] == WEAPON_SHORTBOW){
			if (victim->in_room != ch->in_room)
				ch->send("They're too far away!\n\r");
			else
				do_shortbowfire(ch,victim);
			return;
		}
		else{
			ch->send("You might want to use a bow to fire that arrow with\n\r");
			return;
		}
	}
}

void do_sharpshooting(CHAR_DATA *ch,char *argument){
}
void shot_fire(CHAR_DATA *ch,OBJ_DATA *obj){
	act("$p glows softly before igniting in flames.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly and ignites in flames.",ch,obj,NULL,TO_ROOM);
	TOGGLE_BIT(obj->value[4], ARR_FIRE);
	SET_BIT(obj->extra_flags,ITM_FLAMING);
}

void shot_ice(CHAR_DATA *ch,OBJ_DATA *obj){
	act("$p glows softly before becoming encased in ice.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly and becomes encased in ice.",ch,obj,NULL,TO_ROOM);
	TOGGLE_BIT(obj->value[4], ARR_COLD);
	SET_BIT(obj->extra_flags,ITM_FROSTED);
}

void shot_poison(CHAR_DATA *ch,OBJ_DATA *obj){
	act("$p glows softly before toxic slime wraps around the tip.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly and its tip becomes encased in a purple liquid.",ch,obj,NULL,TO_ROOM);
	TOGGLE_BIT(obj->value[4], ARR_POISON);
}

void shot_electric(CHAR_DATA *ch,OBJ_DATA *obj){
	act("$p glows softly before electricity shimmers about the tip.",ch,obj,NULL,TO_CHAR);
	act("$p glows softly and electricity shimmers about the tip.",ch,obj,NULL,TO_ROOM);
	TOGGLE_BIT(obj->value[4], ARR_SHOCK);
	SET_BIT(obj->extra_flags,ITM_SHOCKED);
}

void do_arrow_infusion(CHAR_DATA *ch,char *argument){
	OBJ_DATA *arrow;
	int chance,percent;

	if(!argument[0]){
		ch->send("Syntax: infuse <element>\n\r");
		return;
	}
	if (((arrow = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R))) == NULL) || (arrow->item_type != ITEM_ARROW)){
		ch->send("You must be holding an arrow in your hand.\n\r");
		return;
	}
	if(arrow->item_type != ITEM_ARROW){
		ch->send("You may only imbue arrows!\n\r");
		return;
	}
	if (arrow->value[4] != 0){
		ch->send("That item has already been recharged once.\n\r");
		return;
	}

	chance = 40 + 2 * ch->level;
	chance = UMAX(ch->level/2,chance);
	percent = number_percent();

	if(percent <= chance){
		if(!str_prefix(argument,"fire"))
			shot_fire(ch,arrow);
		else if(!str_prefix(argument,"ice"))
			shot_fire(ch,arrow);
		else if(!str_prefix(argument,"electricity") || !str_cmp(argument,"lightning"))
			shot_electric(ch,arrow);
		else if(!str_prefix(argument,"poison"))
			shot_poison(ch,arrow);
	}
	else if (percent <= UMIN(95, 3 * chance / 2))
		ch->send("Nothing seems to happen.\n\r");
	else{
		act("$p glows brightly and explodes!",ch,arrow,NULL,TO_CHAR);
		act("$p glows brightly and explodes!",ch,arrow,NULL,TO_ROOM);
		extract_obj(arrow);
	}
}

void do_barb(CHAR_DATA *ch,char *argument){
	OBJ_DATA *arrow;
	AFFECT_DATA af;
	int chance = get_skill(ch,gsn_barb) * .85;
	char arg[MSL];
	bool found = false;

	one_argument(argument,arg);

	if (!arg[0]){
		send_to_char("Remove what?\n\r",ch);
		return;
	}

	if ((arrow = get_obj_wear(ch,arg,true)) == NULL){
		ch->send("You do not have that item.\n\r");
		return;
	}

	if (can_see_obj(ch,arrow)){
		if ((arrow->item_type != ITEM_ARROW && arrow->item_type != ITEM_THROWINGDART) || (arrow->wear_loc != WEAR_HOLD_L && arrow->wear_loc != WEAR_HOLD_R))
			ch->send("It must be a held arrow or dart.\n\r");
		else
			found = true;
	}

	if (!found){
		ch->send("You aren't holding that.\n\r");
		return;
	}

	chance = chance * get_curr_stat(ch,STAT_INT) / STAT_MAX;

	WAIT_STATE(ch,skill_table[gsn_barb].beats);
	if (roll_chance(ch,chance)){
		act("You carefully barb $p.",ch,arrow,NULL,TO_CHAR);
		act("$n carefully barbs $p.",ch,arrow,NULL,TO_ROOM);
		check_improve(ch,gsn_barb,true,2);
		af.where     = TO_OBJECT;
		af.type      = gsn_barb;
		af.level     = ch->level;
		af.duration  = 20;
		af.location  = 0;
		af.modifier  = 0;
		af.bitvector = ITM_BARBED;
		affect_to_obj(arrow,&af);
	}
	else{
		act("You fail to barb $p.",ch,arrow,NULL,TO_CHAR);
		check_improve(ch,gsn_barb,false,2);
	}
}
