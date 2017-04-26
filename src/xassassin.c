#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"
#include "recycle.h"

/*
 * Local functions.
 */
void one_hit		( CHAR_DATA*,CHAR_DATA*,int,bool,bool );
void raw_kill		( CHAR_DATA*,CHAR_DATA* );
bool check_evade	(CHAR_DATA*,CHAR_DATA* );
bool check_guard	(CHAR_DATA*,CHAR_DATA* );
int	find_door	( CHAR_DATA*,char* );
void init_familiar ( CHAR_DATA*,CHAR_DATA* );
int melee_hit(CHAR_DATA*,CHAR_DATA*,int,bool);
int check_ambush(CHAR_DATA *ch,CHAR_DATA *victim,int dam);
/* 
 *The Skills
 */


bool check_stealth(CHAR_DATA *ch){
	int sn = gsn_stealth,skill = get_skill(ch,sn);
	if(skill < 1)
		return false;

	if(roll_chance(ch,skill)){
		ch->send("Your stealth masks your movements.\n\r");
		check_improve(ch,sn,true,1);
		return true;
	}
	else
		check_improve(ch,sn,false,1);
	return false;
}
void check_dual_backstab(CHAR_DATA *ch,CHAR_DATA *victim){
	OBJ_DATA *dual;
	int sn = gsn_dual_backstab,skill = get_skill(ch,sn);
	int dam;

	if(skill < 1 || !(dual = get_eq_char(ch,WEAR_SECONDARY)))
		return;

	skill /= 3;
	skill += ch->getslvl(sn)*5;

	dam = melee_hit(ch,victim,dice(dual->value[1],dual->value[2]),true);

	dam += (dice(dual->value[1],dual->value[2]))*ch->getslvl(sn);

	if(roll_chance(ch,skill)){
		skill_damage(ch,victim,dam,sn,attack_table[dual->value[3]].damage,true);
		check_improve(ch,sn,true,1);
		return;
	}
	skill_damage(ch,victim,0,sn,attack_table[dual->value[3]].damage,true);
	check_improve(ch,sn,false,1);
}

bool perfect_sneak(CHAR_DATA *ch){
	int chance = get_skill(ch,gsn_penumbralveil) * .8;
	AFFECT_DATA af;
	if (time_info.hour > 5 && time_info.hour < 20)
		return false;

	if (get_skill(ch,gsn_penumbralveil) < 1)
		return false;

	if (is_affected(ch,gsn_penumbralveil))
		return true;

//	unSneak(ch);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gPerfSneak({c%d{x)\n\r",chance);
	if (number_percent() < get_skill(ch,gsn_penumbralveil)){
		check_improve(ch,gsn_penumbralveil,true,1);
		ch->send("You use the shadows to attain a more perfect cloak.\n\r");
		affect_set(ch,TO_AFFECTS,gsn_penumbralveil,ch->level,ch->getslvl(gsn_penumbralveil),ch->level,APPLY_NONE,0,AF_PERFECTSNEAK);
		return true;
	}
	else{
		check_improve(ch,gsn_penumbralveil,false,1);
		return false;
	}
}

void do_sneak(CHAR_DATA *ch,char *argument){
	int chance = get_skill(ch,gsn_sneak) * .9;
	AFFECT_DATA af;

	if (MOUNTED(ch)){
		ch->send("You can't sneak while riding.\n\r");
		return;
	}
	if (ch->isaff(AF_SNEAK)){
		ch->send("You can't sneak while sneaking... wait... can you?\n\r");
		return;
	}

	if (chance < 1){
		ch->send("You don't know how to sneak!\n\r");
		return;
	}


//	unSneak(ch);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{bSneak({c%d{x(\n\r",chance);
	if (number_percent() < chance){
		if (!perfect_sneak(ch)){
			send_to_char("You mask your movements.\n\r",ch);
			check_improve(ch,gsn_sneak,true,3);
			affect_set(ch,TO_AFFECTS,gsn_sneak,ch->level,ch->getslvl(gsn_sneak),ch->level,APPLY_NONE,0,AF_SNEAK);
		}
	}
	else{
		ch->send("You failed.\n\r");
		check_improve(ch,gsn_sneak,false,3);
	}
}

void do_hide(CHAR_DATA *ch,char *argument){
	int chance = get_skill(ch,gsn_hide) * .9;
	AFFECT_DATA af;
	if (MOUNTED(ch)){
		ch->send("You can't hide while riding.\n\r");
		return;
	}

	if (ch->isaff(AF_HIDE)){
		ch->send("You can't hide while hiding... wait... can you?\n\r");
		return;
	}
	if (chance < 1){
		ch->send("You suck at hiding!\n\r");
		return;
	}

	ch->send("You attempt to hide.\n\r");


//	unSneak(ch);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{bHide({c%d{x(\n\r",chance);
	if (number_percent() < get_skill(ch,gsn_hide)){
		if (!perfect_sneak(ch)){
			ch->send("You find a hiding place.\n\r");
			check_improve(ch,gsn_hide,true,3);
			affect_set(ch,TO_AFFECTS,gsn_hide,ch->level,ch->getslvl(gsn_hide),ch->level,APPLY_NONE,0,AF_HIDE);
		}
	}
	else{
		ch->send("You failed to find a good place to hide.\n\r");
		check_improve(ch,gsn_hide,false,3);
	}
}

void do_swap(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim,*wch,*wch_next;
	char arg[MSL];
	int skill = get_skill(ch,gsn_swap) * .75,aggressors=0;

	one_argument(argument,arg);

	if (skill < 1){
		ch->send("You're too slow to do this.\n\r");
		return;
	}

	for (wch = ch->in_room->people;wch;wch=wch->next_in_room)
		if (wch->fighting == ch)
			aggressors++;

	if (aggressors < 2){
		ch->send("You don't have enough of a distraction ready.\n\r");
		return;
	}

	if (arg[0] == '\0' || !ch->fighting){
		ch->send("It doesn't work that way.\n\r");
		return;
	}
	else if ((victim = get_char_room(ch,NULL,arg)) == NULL){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (!canFight(ch,victim,true))
		return;

	printf_to_char(ch,"swap %d\n\r",skill);
	skill = skill * get_curr_stat(ch,STAT_AGI) / STAT_MAX;
	printf_to_char(ch,"swap %d\n\r",skill);
	skill -= ((get_curr_stat(victim,STAT_WIS) * get_curr_stat(victim,STAT_AGI)) / STAT_MAX) / 4;
	printf_to_char(ch,"swap %d\n\r",skill);

	WAIT_STATE(ch,skill_table[gsn_swap].beats);

	if (number_percent() >= skill){
		ch->send("You failed!\n\r");
		check_improve(ch,gsn_swap,false,2);
	}
	else{
		act("$n swaps positions with $N.",ch,NULL,victim,TO_NOTVICT);
		act("$n swaps with you!",ch,NULL,victim,TO_VICT);
		act("You swap positions with $N.",ch,NULL,victim,TO_CHAR);

		for (wch=ch->in_room->people;wch;wch=wch_next){
			wch_next = wch->next_in_room;
			if (wch != victim && wch != ch && wch->fighting == ch)
				wch->fighting = victim;
		}
		if(check_stealth(ch))
			do_function(ch,&do_hide,"");
		check_improve(ch,gsn_swap,true,2);
	}
}

void do_vanish(CHAR_DATA *ch,char *argument){
	int skill=get_skill(ch,gsn_vanish) *.75,tries=0;
	ROOM_INDEX_DATA *room;

	if(ch->in_room == NULL
	|| IS_SET(ch->in_room->room_flags,ROOM_NO_RECALL)
	|| !ch->fighting){
		ch->send("You failed.\n\r");
		return;
    }

	printf_to_char(ch,"vanish %d\n\r",skill);
	skill = skill * get_curr_stat(ch,STAT_AGI) / STAT_MAX;
	printf_to_char(ch,"vanish %d\n\r",skill);

	for (;tries < 10;tries++)
		if ((room = get_random_room_area(ch)) == ch->in_room)
			continue;
		else
			break;

	WAIT_STATE(ch,skill_table[gsn_vanish].beats);
	if(number_percent() >= skill){
		check_improve(ch,gsn_vanish,false,2);
		ch->send("You failed.\n\r");
	}
	else{
		act("$n vanishes!",ch,NULL,NULL,TO_ROOM);
		ch->send("You quickly jump to another place!\n\r");
		char_from_room(ch);
		char_to_room(ch,room);
		act("$n leaps in.",ch,NULL,NULL,TO_ROOM);
		do_function(ch,&do_look,"auto");
		if(check_stealth(ch))
			do_function(ch,&do_hide,"");
		check_improve(ch,gsn_vanish,true,1);
	}
}
//NASHNEEDSTOMAKEOBJLEVELMATTER
void do_lift(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim,*rch;
	OBJ_DATA *obj;
	char arg[MIL],arg2[MIL],buf[MSL];
	int chance;

	if (ch->level > KING && ch->level < ADMIN){
		ch->send("Don't be a dumbass.\n\r");
		return;
	}
	argument = one_argument(argument,arg);
	one_argument(argument,arg2);

	if (!arg[0] || !arg2[0]){
		send_to_char("Swipe what from who?\n\r",ch);
		return;
	}

	if(!(victim = get_char_room(ch,NULL,arg2))){
		ch->send("Who?\n\r");
		return;
	}
	if (!canFight(ch,victim,true))
		return;

	if ((obj = get_obj_wear(victim,arg,false)) == NULL || !can_see_obj(ch,obj)){
		send_to_char("They don't have that.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_TATTOO) || IS_OBJ_STAT(obj,ITM_NODROP) || IS_OBJ_STAT(obj,ITM_NOREMOVE)){
		send_to_char("You can't pry it off of them!\n\r",ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)){
		send_to_char("You can't carry that much weight.\n\r",ch);
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_swipe].beats);
	chance = number_range(get_skill(ch,gsn_swipe) * .25,get_skill(ch,gsn_swipe) * .5);
	chance = (chance * (ch->level * .1)) / (obj->level * .1);
	if (IS_OBJ_STAT(obj,ITM_MAGIC))
		chance *= .9;
	if (IS_AWAKE(victim))
		chance = 5;
	//printf_to_char(ch,"chance %d\n\r",chance);
	if (number_percent() < chance){
		act("You swipe $p from $N.",ch,obj,victim,TO_CHAR);
		if (!IS_AWAKE(victim)){
			sprintf(buf,"%s gets a five fingered discount on $p from $n",IS_NPC(ch) ? capitalize(ch->short_descr) : ch->name);
			for(rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
				if (rch != ch && can_see(rch,ch))
					act(buf,victim,obj,rch,TO_VICT);
		}
		else{
			act("$n pulls $p off you.",ch,obj,victim,TO_VICT);
			act("$n pulls $p off $N.",ch,obj,victim,TO_NOTVICT);
		}
		obj_from_char(obj);
		obj_to_char(obj,ch);
		check_improve(ch,gsn_swipe,true,2);
	}
	else{
		send_to_char("Oops.\n\r",ch);
		if(!check_stealth(ch))
			unSneak(ch);
		act("You fail to swipe $p from $N.",ch,obj,victim,TO_CHAR);
		act("$n is tried to swipe $p from you.",ch,obj,victim,TO_VICT);
		act("$n is caught with $s hands on $p from $N.",ch,obj,victim,TO_NOTVICT);
		check_improve(ch,gsn_swipe,false,3);
		if(IS_NPC(victim) && IS_AWAKE(victim)){
			switch(number_range(0,3)){
				case 0:case 1:case 2:
					if (ch->looks[P_SHAIR] == S_HAIR_BALD)
						sprintf(buf,"A bald %s with %s eyes just tried to rob me!",
							race_table[ch->race].name,
							eye_table[ch->looks[P_EYE]].name != NULL ? eye_table[ch->looks[P_EYE]].name : "{Rb{Cu{Bg{Gg{Ye{Md{x");
					else if (ch->looks[P_SHAIR] == S_HAIR_SHAVED)
						sprintf(buf,"%s %s with a shaved head and %s eyes just tried to rob me!",
							IS_VOWELL(race_table[ch->race].name[0]) ? "An" : "A",
							race_table[ch->race].name,
							eye_table[ch->looks[P_EYE]].name != NULL ? eye_table[ch->looks[P_EYE]].name : "{Rb{Cu{Bg{Gg{Ye{Md{x");
					else
						sprintf(buf,"%s %s with %s hair and %s eyes just tried to rob me!",
							IS_VOWELL(race_table[ch->race].name[0]) ? "An" : "A",
							race_table[ch->race].name,
							hair_table[ch->looks[P_HAIR]].name != NULL ? hair_table[ch->looks[P_HAIR]].name : "{Rb{Cu{Bg{Gg{Ye{Md{x",
							eye_table[ch->looks[P_EYE]].name != NULL ? eye_table[ch->looks[P_EYE]].name : "{Rb{Cu{Bg{Gg{Ye{Md{x");
					break;
				case 3 :
					sprintf(buf,"%s tried to rob me!",ch->name);break;
			}
			do_function(victim,&do_yell,buf);
		}
		else{
			if (IS_NPC(victim)){
			    check_improve(ch,gsn_swipe,false,2);
				multi_hit(victim,ch,TYPE_UNDEFINED,false);
			}
			else{
				sprintf(buf,"$N tried to steal from %s.",victim->name);
				wiznet(buf,ch,NULL,WZ_FLAGS,0,0);
				if (!ch->isplr(PL_THIEF)){
					ch->setplr(PL_THIEF);
					send_to_char("{R*** {xYou are now a {YTHIEF{x!! {R***{x\n\r",ch);
					cql_save_char(ch);
				}
			}
		}
	}
}

void do_palm(CHAR_DATA *ch,char *argument)
{
    char arg1[MIL];
    OBJ_DATA *obj;

    one_argument(argument,arg1);

	if (get_skill(ch,gsn_palm) <= 0)
	{
		send_to_char("You're too clumsy to do that!\n\r",ch);
		return;
	}
    /* Get type. */
    if (!arg1[0])
    {
		send_to_char("Palm what?\n\r",ch);
		return;
    }

	if (str_cmp(arg1,"all") && str_prefix("all.",arg1))
	{
		obj = get_obj_list(ch,arg1,ch->in_room->contents);
		if (obj == NULL)
		{
			act("You see no $T here.",ch,NULL,arg1,TO_CHAR);
			return;
		}
	}
	else
	{
		send_to_char("You don't have that many hands!\n\r",ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch))
	{
		send_to_char("You can't carry that much weight.\n\r",ch);
		return;
	}

	if(number_percent() <= get_skill(ch,gsn_palm) * .9)
	{
		act("You palm $p. Five finger discount!",ch,obj,NULL,TO_CHAR);
		CHAR_DATA *rch;
		for(rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
			if (rch != ch && can_see(rch,ch))
				act("$n gets a five fingered discount on $p.",ch,obj,rch,TO_VICT);
		get_obj(ch,obj,NULL,false);
		check_improve(ch,gsn_palm,true,2);
	}
	else
	{
		if(!check_stealth(ch))
			unSneak(ch);
		send_to_char("You fail to conceal your actions.\n\r",ch);
		get_obj(ch,obj,NULL,true);
		check_improve(ch,gsn_palm,false,1);
		return;
	}
    return;
}
void do_cutpurse(CHAR_DATA *ch,char *argument)
{
    char arg1[MIL];
    OBJ_DATA *obj;
	CHAR_DATA *victim;
	int stealGold = 0,stealSilver = 0;

    argument = one_argument(argument,arg1);

	if(get_skill(ch,gsn_cutpurse) <= 0)
	{
		send_to_char("You're too clumsy to do that!\n\r",ch);
		return;
	}
    /* Get type. */
    if (!arg1[0])
    {
		send_to_char("Cut whose purse?\n\r",ch);
		return;
    }

	if ((victim = get_char_room(ch,NULL,arg1)) == NULL)
	{
		send_to_char("You don't see them here.\n\r",ch);
		return;
	}

	if (can_see(victim,ch))
	{
		ch->send("You expect to be so sneaky when they can see you?\n\r");
		act("$n eyes $N's coins lustfully..",ch,NULL,victim,TO_NOTVICT);
		act("$n eyes your coins lustfully..",ch,NULL,victim,TO_VICT);
		return;
	}

	if (!canFight(ch,victim,true))
		return;

	if (victim->gold < 2 && victim->silver < 2)
	{
		send_to_char("Their coin pouch seems too light to bother.\n\r",ch);
		return;
	}

	if(number_percent() <= get_skill(ch,gsn_cutpurse) * .5)
	{
		check_improve(ch,gsn_cutpurse,true,2);
		if (number_percent() < 50)//Half-Fail
		{
			unSneak(ch);
			stealGold = number_range(victim->gold/2,victim->gold);
			stealSilver = number_range(victim->silver/2,victim->silver);
			obj_to_room(create_money(stealGold,stealSilver),victim->in_room);
			act("You cut $N's coin pouch and $S coins scatter across the ground.",ch,NULL,victim,TO_CHAR);
			act("$n cuts $N coin pouch and $S coins scatter across the ground.",ch,NULL,victim,TO_NOTVICT);
			act("$n cuts your coin pouch and your coins scatter across the ground.",ch,NULL,victim,TO_VICT);
			WAIT_STATE(ch,skill_table[gsn_cutpurse].beats/2);
		}
		else
		{
			stealGold = number_range(victim->gold/2,victim->gold);
			stealSilver = number_range(victim->silver/2,victim->silver);
			victim->gold -= stealGold;
			victim->silver -= stealSilver;
			act("You silently cut $N's coin pouch and snatch some of $S money.",ch,NULL,victim,TO_CHAR);
			WAIT_STATE(ch,skill_table[gsn_cutpurse].beats);
		}
	}
	else
	{
		unSneak(ch);
		check_improve(ch,gsn_cutpurse,true,2);
		WAIT_STATE(ch,skill_table[gsn_cutpurse].beats/2);
		send_to_char("You fail.\n\r",ch);
	}
    return;
}

void do_slip(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	int sn = gsn_steal,skill = get_skill(ch,sn);
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	argument = one_argument(argument,arg1);
	one_argument(argument,arg2);

	if(skill <= 0){
		send_to_char("You're too clumsy to do that!\n\r",ch);
		return;
	}

	if (!arg1[0] || !arg2[0]){
		send_to_char("Slip what to whom?\n\r",ch);
		return;
	}

	if (str_cmp(arg1,"all") && str_prefix("all.",arg1)){
		if ((obj = get_obj_carry(ch,arg1,ch)) == NULL){
			send_to_char("You do not have that item.\n\r",ch);
			return;
		}
	}
	else{
		send_to_char("It's not like this stuff is going to explode!\n\r",ch);
		return;
	}

	if (!can_see_obj(ch,obj)){
		send_to_char("You don't see that in your inventory.\n\r",ch);
		return;
	}

	if ((victim = get_char_room(ch,NULL,arg2)) == NULL){
		send_to_char("You don't see them here.\n\r",ch);
		return;
	}

	if (victim->carry_weight + get_obj_weight(obj) > can_carry_w(victim)){
		send_to_char("They can't carry that much weight.\n\r",ch);
		return;
	}

	if(number_percent() <= skill * .9){
		act("You slip $p to $N",ch,obj,victim,TO_CHAR);
		CHAR_DATA *rch;
		for(rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
			if (rch != ch && can_see(rch,ch)){//nash make this chance better
				if(rch != victim)
					printf_to_char(rch,"%s slips %s to %s.\n\r",ch->name,obj->short_descr,victim->name);
				else
					printf_to_char(rch,"%s slips %s to you.\n\r",ch->name,obj->short_descr);
			}
		obj_from_char(obj);
		obj_to_char(obj,victim);
		check_improve(ch,sn,true,2);
	}
	else{
		if(!check_stealth(ch))
			unSneak(ch);
		send_to_char("You fail to conceal your actions.\n\r",ch);
		obj_from_char(obj);
		obj_to_char(obj, victim);
		act("$n slips $p to $N.",ch,obj,victim,TO_NOTVICT);
		act("$n slips you $p.",ch,obj,victim,TO_VICT);
		act("You slips $p to $N.",ch,obj,victim,TO_CHAR);
		if (HAS_TRIGGER_OBJ(obj,TRIG_GIVE))
			p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_GIVE);
		if (HAS_TRIGGER_ROOM(ch->in_room,TRIG_GIVE))
			p_give_trigger(NULL,NULL,ch->in_room,ch,obj,TRIG_GIVE);

		if (IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_GIVE))
			p_give_trigger(victim,NULL,NULL,ch,obj,TRIG_GIVE);
		check_improve(ch,sn,false,1);
	}
}

void do_steal(CHAR_DATA *ch,char *argument)
{
	char buf[MSL],arg1[MIL],arg2[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent;

	argument = one_argument(argument,arg1);
	one_argument(argument,arg2);

	if (!arg1[0] || !arg2[0]){
		send_to_char("Steal what from whom?\n\r",ch);
		return;
	}

	if (!(victim = get_char_room(ch,NULL,arg2))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if (victim == ch){
		send_to_char("That's pointless.\n\r",ch);
		return;
	}

	if (!canFight(ch,victim,true))
		return;

	if (victim->level > 100 && ch->level < 118){
		ch->send("Don't be a dumbass.\n\r");
		return;
	}
	WAIT_STATE(ch,skill_table[gsn_steal].beats);
	percent = get_skill(ch,gsn_steal) * .3;

	percent += ch->perm_stat[STAT_AGI];
	if (!IS_AWAKE(victim))
		percent += 10;
	if (!can_see(victim,ch))
		percent += 5;

	if(number_percent() > percent){
		send_to_char("Oops.\n\r",ch);
		if(!check_stealth(ch))
			unSneak(ch);

		if (!IS_AWAKE(victim))
		   do_function(victim,&do_wake,"");

		act("$n tried to steal from you.\n\r",ch,NULL,victim,TO_VICT);
		act("$n tried to steal from $N.\n\r",ch,NULL,victim,TO_NOTVICT);
		if(IS_NPC(victim)){
			switch(number_range(0,3)){
				case 0 :
				case 1 :
				case 2 :
					sprintf(buf,"%s %s with %s hair and %s eyes just tried to rob me!",
						IS_VOWELL(race_table[ch->race].name[0]) ? "An" : "A",
						race_table[ch->race].name,
						hair_table[ch->looks[P_HAIR]].name != NULL ? hair_table[ch->looks[P_HAIR]].name : "{Rb{Cu{Bg{Gg{Ye{Md{x",
						eye_table[ch->looks[P_EYE]].name != NULL ? eye_table[ch->looks[P_EYE]].name : "{Rb{Cu{Bg{Gg{Ye{Md{x");
					break;
				case 3 :
					sprintf(buf,"%s tried to rob me!",ch->name);
					break;
			}
			if (IS_AWAKE(victim))
				do_function(victim,&do_yell,buf);
		}
		else{
			if (IS_NPC(victim)){
				check_improve(ch,gsn_steal,false,2);
				multi_hit(victim,ch,TYPE_UNDEFINED,false);
			}
			else{
				sprintf(buf,"$N tried to steal from %s.",victim->name);
				wiznet(buf,ch,NULL,WZ_FLAGS,0,0);
				if (!ch->isplr(PL_THIEF)){
					ch->setplr(PL_THIEF);
					send_to_char("{R*** {xYou are now a {YTHIEF{x!! {R***{x\n\r",ch);
					cql_save_char(ch);
				}
			}
		}
		return;
	}
	else{
		if ( !str_cmp(arg1,"coin") || !str_cmp(arg1,"coins") || !str_cmp(arg1,"gold") || !str_cmp(arg1,"silver")){
			int gold, silver;

			gold = victim->gold * number_range(1,ch->level) / LEVEL_HERO;
			silver = victim->silver * number_range(1,ch->level) / LEVEL_HERO;
			if (gold <= 0 && silver <= 0){
				send_to_char("You couldn't get any coins.\n\r",ch);
				return;
			}
			ch->gold     	+= gold;
			ch->silver   	+= silver;
			victim->silver 	-= silver;
			victim->gold 	-= gold;
			if (silver <= 0)
				sprintf(buf,"Bingo!  You got {Y%d {xgold coins.\n\r",gold);
			else if (gold <= 0)
				printf_to_char(ch,"Bingo!  You got {w%d {xsilver {coins.\n\r",silver);
			else
				printf_to_char(ch,"Bingo!  You got {w%d {xsilver and {Y%d {xgold coins.\n\r",silver,gold);
			check_improve(ch,gsn_steal,true,2);
			return;
		}

		if (!(obj = get_obj_carry(victim,arg1,ch))){
			send_to_char("You can't find it.\n\r",ch);
			return;
		}
		
		if (!can_drop_obj(ch,obj) || IS_SET(obj->extra_flags,ITM_INVENTORY) || obj->level > ch->level){
			send_to_char("You can't pry it away.\n\r",ch);
			return;
		}

		if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)){
			send_to_char("You have your hands full.\n\r",ch);
			return;
		}

		if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)){
			send_to_char("You can't carry that much weight.\n\r",ch);
			return;
		}

		obj_from_char(obj);
		obj_to_char(obj,ch);
		act("You pocket $p.",ch,obj,NULL,TO_CHAR);
		check_improve(ch,gsn_steal,true,2);
		send_to_char("Got it!\n\r",ch);
		return;
	}
}

void do_backstab(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int sn,skill,chance,dam,diceroll;

	sn = get_weapon_sn(ch, false);
	if(!IS_NPC(ch) && get_skill(ch,gsn_backstab) < 1)
		return;
	skill = get_weapon_skill(ch,sn);
	chance = get_skill(ch,gsn_backstab);

	one_argument(argument,arg);

	if (!arg[0]){
		send_to_char("Backstab whom?\n\r",ch);
		return;
	}

	if (ch->fighting){
		send_to_char("You're facing the wrong end.\n\r",ch);
		return;
	}

	if(!(victim = get_char_room(ch,NULL,arg))){
		send_to_char("Who? Where?\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if (!(obj = get_eq_char(ch,WEAR_WIELD)) || is_weapon_small_pierce(obj)==false){
		send_to_char("You need to wield a small piercing weapon to backstab.\n\r",ch);
		return;
	}

	if (victim->hit < victim->max_hit / 2){
		act("$N is hurt and suspicious ... you can't sneak up.",ch,NULL,victim,TO_CHAR);
		return;
	}

	dam = 5 * dice(obj->value[1],obj->value[2]);
	dam = (dam * skill) / 100;
	dam = check_ambush(ch,victim,dam);

	if (get_skill(ch,gsn_enhanced_damage) > 0){
		diceroll = number_percent();
		if (diceroll <= get_skill(ch,gsn_enhanced_damage) * .5){
			check_improve(ch,gsn_enhanced_damage,true,1);
			dam += ((dam/2) * diceroll/200);
		}
		else
			check_improve(ch,gsn_enhanced_damage,false,1);
	}

	if(can_see(victim,ch)){
		dam *= .85;
		chance *= .85;
	}

	chance = chance * skill / 100;

	WAIT_STATE(ch,skill_table[gsn_backstab].beats);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBSD({c%d{x)\n\r",dam);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gBSC({c%d{x)\n\r",chance);
	if (roll_chance(ch,chance)){
		check_improve(ch,gsn_backstab,true,1);
		skill_damage(ch,victim,dam,gsn_backstab,DAM_PIERCE,true);
		check_dual_backstab(ch,victim);
	}
	else{
		check_improve(ch,gsn_backstab,false,1);
		skill_damage(ch,victim,0,gsn_backstab,DAM_NONE,true);
	}
}

void do_strangle(CHAR_DATA *ch,char *argument)
{
	char arg[MIL];
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int chance;
  
    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        send_to_char("Strangle who?\n\r",ch);
        return;
    }

    if (ch->fighting != NULL)
    {
		send_to_char("You can't sneak up in combat!\n\r",ch);
		return;
    }
    else if ((victim = get_char_room(ch,NULL,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if (victim->isaff(AF_SLEEP) || victim->isaff(AF_SAP) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)))
    {
		send_to_char("They can't be strangled.\n\r",ch);
		return;
    }

    if (victim->fighting != NULL)
    {
        send_to_char("You can't strangle someone who's in combat!\n\r",ch);
        return;
    }

	if(!canFight(ch,victim,true))
		return;

	chance = get_skill(ch,gsn_strangle) * .75;

	chance += ch->perm_stat[STAT_STR] - victim->perm_stat[STAT_AGI];
	chance += ch->size-victim->size;

    if (calcHands(ch) > 0)
    {
		send_to_char("You need to be completely barehanded to strangle someone.\n\r",ch);
		return;
    }

    WAIT_STATE(ch,skill_table[gsn_strangle].beats);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gStrangle({c%d{x)\n\r",chance);
    if (!roll_chance(ch,chance))
    {
		check_improve(ch,gsn_strangle,false,1);
		skill_damage(ch,victim,0,gsn_strangle,DAM_NONE,true);
    }
    else
    {
		check_improve(ch,gsn_strangle,true,2);
		if (IS_AWAKE(victim))
		{
			act("You put $N in a sleeper hold and $E passes out!",ch,NULL,victim,TO_CHAR);
			send_to_char("You feel two arms wrap around you in a sleeper hold and you pass out!\n\r",victim);
			act("$n puts $N in a sleeper hold until $E passes out!",ch,NULL,victim,TO_NOTVICT);
			victim->position	= POS_STUNNED;
			affect_set(victim,TO_AFFECTS,gsn_strangle,ch->level,ch->getslvl(gsn_strangle),2,APPLY_NONE,0,AF_SAP);
		}
    }
    return;
}

void do_pick(CHAR_DATA *ch,char *argument)
{
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit, *pexit_rev;
    char arg1[MSL],arg2[MSL],buf[MSL],p1[MSL],p2[MSL],p3[MSL],p4[MSL];
    CHAR_DATA *gch;
    OBJ_DATA *obj,*pick;
    int door,chance,difficulty=2,n;
	char sPins[MSL];
	bool failed = false;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,p1);
	argument = one_argument(argument,p2);
	argument = one_argument(argument,p3);
	one_argument(argument,p4);

	if (!arg1[0]){
		send_to_char("Syntax: pick <lock> <pins>\n\r",ch);
		return;
	}
	if (MOUNTED(ch)){
		send_to_char("You can't pick locks while mounted.\n\r",ch);
		return;
	}

    WAIT_STATE(ch,skill_table[gsn_pick_lock].beats);

    /* look for guards */
	for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ){
		if (IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level){
			act("$N is standing too close to the lock.",ch,NULL,gch,TO_CHAR);
			return;
		}
	}

	if ((door = find_door(ch,arg1)) >= 0){
		chance = get_skill(ch,gsn_pick_lock) * .9;

		if ((pick = get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R)) == NULL || pick->item_type != ITEM_LOCKPICK){
			send_to_char("You must be holding a lockpick in one hand.\n\r",ch);
			return;
		}

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info,EX_CLOSED) && !IS_IMMORTAL(ch)){
			send_to_char("It's not closed.\n\r",ch);
			return;
		}
		if (pexit->key < 0 && !IS_IMMORTAL(ch)){
			send_to_char("It can't be picked.\n\r",ch);
			return;
		}
		if (!IS_SET(pexit->exit_info,EX_LOCKED)){
			send_to_char("It's already unlocked.\n\r",ch);
			return;
		}
		if ( number_percent() > get_skill(ch,gsn_pick_lock) || (IS_SET(pexit->exit_info,EX_PICKPROOF) && !IS_IMMORTAL(ch))){
			send_to_char("You failed.\n\r",ch);
			return;
		}

		if (IS_SET(pexit->exit_info,EX_EASY))
			difficulty = 1;
		if (IS_SET(pexit->exit_info,EX_HARD))
			difficulty = 3;
		if (IS_SET(pexit->exit_info,EX_INFURIATING))
			difficulty = 4;

		sprintf(sPins,"%d%d",ch->in_room->vnum,ch->in_room->vnum);

		printf_to_char(ch,">%s< >%d<\n\r",sPins,ch->in_room->vnum);
		if (pick->value[0] < difficulty){
			send_to_char("Your lockpick isn't capable of picking this lock.\n\r",ch);
			return;
		}
		switch (difficulty)
		{
		case 1:
			if (p1[0] == '\0')
			{
				send_to_char("You must specify one pin manipulator for this lock.\n\r",ch);
				return;
			}
			if (((sPins[0] - '0') % 2 && str_prefix(p1,"up")) || (!((sPins[0] - '0') % 2) && str_prefix(p1,"down")))
			{
				send_to_char("CLANK!\n\r",ch);
				failed = true;
			}
			else
				send_to_char("CLICK!\n\r ",ch);
			break;
		case 2:
			if (p1[0] == '\0' || p2[0] == '\0')
			{
				send_to_char("You must specify two pin manipulators for this lock.\n\r",ch);
				return;
			}
			if (((sPins[0] - '0') % 2 && str_prefix(p1,"up")) || (!((sPins[0] - '0') % 2) && str_prefix(p1,"down")))
			{
				send_to_char("CLANK! ",ch);
				failed = true;
			}
			else
				send_to_char("CLICK! ",ch);

			if (((sPins[1] - '0') % 2 && str_prefix(p2,"up")) || (!((sPins[1] - '0') % 2) && str_prefix(p2,"down")))
			{
				send_to_char("CLANK!",ch);
				failed = true;
			}
			else
				send_to_char("CLICK!",ch);
			send_to_char("\n\r",ch);
			break;
		case 3:
			if (p1[0] == '\0' || p2[0] == '\0' || p3[0] == '\0')
			{
				send_to_char("You must specify three pin manipulators for this lock.\n\r",ch);
				return;
			}
			if (((sPins[0] - '0') % 2 && str_prefix(p1,"up")) || (!((sPins[0] - '0') % 2) && str_prefix(p1,"down")))
			{
				send_to_char("CLANK! ",ch);
				failed = true;
			}
			else
				send_to_char("CLICK! ",ch);

			if (((sPins[1] - '0') % 2 && str_prefix(p2,"up")) || (!((sPins[1] - '0') % 2) && str_prefix(p2,"down")))
			{
				send_to_char("CLANK! ",ch);
				failed = true;
			}
			else
				send_to_char("CLICK! ",ch);

			if (((sPins[2] - '0') % 2 && str_prefix(p3,"up")) || (!((sPins[2] - '0') % 2) && str_prefix(p3,"down")))
			{
				send_to_char("CLANK!",ch);
				failed = true;
			}
			else
				send_to_char("CLICK!",ch);
			send_to_char("\n\r",ch);
			break;
		case 4:
			if (!p1[0] || !p2[0] || !p3[0] || !p4[0])
			{
				send_to_char("You must specify four pin manipulators for this lock.\n\r",ch);
				return;
			}
			if (((sPins[0] - '0') % 2 && str_prefix(p1,"up")) || (!((sPins[0] - '0') % 2) && str_prefix(p1,"down")))
			{
				send_to_char("CLANK! ",ch);
				failed = true;
			}
			else
				send_to_char("CLICK! ",ch);

			if (((sPins[1] - '0') % 2 && str_prefix(p2,"up")) || (!((sPins[1] - '0') % 2) && str_prefix(p2,"down")))
			{
				send_to_char("CLANK! ",ch);
				failed = true;
			}
			else
				send_to_char("CLICK! ",ch);

			if (((sPins[2] - '0') % 2 && str_prefix(p3,"up")) || (!((sPins[2] - '0') % 2) && str_prefix(p3,"down")))
			{
				send_to_char("CLANK! ",ch);
				failed = true;
			}
			else
				send_to_char("CLICK! ",ch);

			if (((sPins[3] - '0') % 2 && str_prefix(p4,"up")) || (!((sPins[3] - '0') % 2) && str_prefix(p4,"down")))
			{
				send_to_char("CLANK!",ch);
				failed = true;
			}
			else
				send_to_char("CLICK!",ch);
			send_to_char("\n\r",ch);
			break;
		default:
			send_to_char("BUG!\n\r",ch);
			return;
		}

		if (failed)
		{
			send_to_char("You fail!\n\r",ch);
			return;
		}
		if (--pick->value[1] <= 0)
		{
			act("$p snaps in your hands from overuse.",ch,pick,NULL,TO_CHAR);
			act("$p snaps in $n's hands from overuse.",ch,pick,NULL,TO_ROOM);
			extract_obj(pick);
		}

		REMOVE_BIT(pexit->exit_info,EX_LOCKED);
		send_to_char("*Click*\n\r",ch);
		act("$n picks the $d.",ch,NULL,pexit->keyword,TO_ROOM);
		check_improve(ch,gsn_pick_lock,true,2);

		/* pick the other side */
		if ((to_room = pexit->u1.to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL && pexit_rev->u1.to_room == ch->in_room)
			REMOVE_BIT(pexit_rev->exit_info,EX_LOCKED);
    }

    if ((obj = get_obj_here(ch,NULL,arg1)) != NULL)
    {
		if (!IS_NPC(ch) && number_percent() > get_skill(ch,gsn_pick_lock))
		{
			send_to_char("You failed.\n\r",ch);
			check_improve(ch,gsn_pick_lock,false,2);
			return;
		}
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{
			if (!IS_SET(obj->value[1],EX_ISDOOR))
			{	
				send_to_char("You can't do that.\n\r",ch);
				return;
			}

			if (!IS_SET(obj->value[1],EX_CLOSED))
			{
				send_to_char("It's not closed.\n\r",ch);
				return;
			}

			if (obj->value[4] < 0)
			{
				send_to_char("It can't be unlocked.\n\r",ch);
				return;
			}

			if (IS_SET(obj->value[1],EX_PICKPROOF))
			{
				send_to_char("You failed.\n\r",ch);
				return;
			}

			REMOVE_BIT(obj->value[1],EX_LOCKED);
			act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
			act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
			check_improve(ch,gsn_pick_lock,true,2);
			return;
		}

		/* 'pick object' */
		if (obj->item_type != ITEM_CONTAINER)
		{
			send_to_char("That's not a container.\n\r",ch);
			return;
		}
		if (!IS_SET(obj->value[1],CONT_CLOSED))
		{
			send_to_char("It's not closed.\n\r",ch);
			return;
		}
		if (obj->value[2] < 0)
		{
			send_to_char("It can't be unlocked.\n\r",ch);
			return;
		}
		if (!IS_SET(obj->value[1],CONT_LOCKED))
		{
			send_to_char("It's already unlocked.\n\r",ch);
			return;
		}
		if (IS_SET(obj->value[1],CONT_PICKPROOF))
		{
			send_to_char("You failed.\n\r",ch);
			return;
		}

		REMOVE_BIT(obj->value[1],CONT_LOCKED);
		act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
		act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
		check_improve(ch,gsn_pick_lock,true,2);
		return;
    }

    return;
}

void do_assassinate(CHAR_DATA *ch,char *argument){
	CHAR_DATA *fch,*victim;
	OBJ_DATA *obj;
    AFFECT_DATA af;
	char arg[MIL];
	int sn,skill,chance,wearyCalc;

	one_argument(argument,arg);

	if (!arg[0]){
		send_to_char("End whose life?\n\r",ch);
		return;
	}

	if (!(victim = get_char_room(ch,NULL,arg))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if (ch->fighting){
		send_to_char("You're facing the wrong end.\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if (!IS_NPC(victim) && ((victim->level > IMMORTAL && get_trust(ch) <= victim->level) || is_affected(victim,gsn_paranoia))){
		send_to_char("You can't kill them..\n\r",ch);
		return;
	}

	if (can_see(victim,ch)){
		ch->send("You're not stupid enough to kill while they watch!\n\r");
		return;
	}

	if (!(obj = get_eq_char(ch,WEAR_WIELD)) || !is_weapon_sharp(obj)){
		send_to_char("You need to wield a weapon to assassinate.\n\r",ch);
		return;
	}

	chance = get_skill(ch,gsn_assassinate) * .95;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"chance {G%d{x",chance);

	if(time_info.hour > 6 && time_info.hour < 18)
		chance -= 30;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Tme{G%d{x",chance);

	for(fch = ch->in_room->people;fch;fch = fch->next_in_room){
		if(!IS_NPC(fch) && fch != ch && fch != victim){
			if(can_see(fch,ch))
				chance = UMAX(chance - 10,1);
			else
				chance = UMAX(chance - 5,1);
		}
	}
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Ppl{G%d{x",chance);
	switch(ch->in_room->sector_type){
	case SECT_GOODTEMPLE:
	case SECT_NEUTRALTEMPLE:
	case SECT_CITYROAD:
		chance = UMAX(chance-25,1);
		break;
	case SECT_ROAD:
	case SECT_FIELD:
		chance -=5;
		break;
	}
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Sct{G%d{x",chance);

	sn = get_weapon_sn(ch,false);
	skill = get_weapon_skill(ch,sn);

	chance = ((chance *((skill * .75) + ch->getslvl(sn)*5))/100);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Wpn{G%d{x",chance);

	wearyCalc = ((victim->hit*100) / victim->max_hit)/-5;
	chance += wearyCalc;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Vwy{G%d:%d{x",chance,wearyCalc);

	chance = chance * ch->hit / ch->max_hit;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Cwy{G%d{x",chance);

	if(victim->position == POS_STANDING)
		chance -= 25;
	else if(victim->position == POS_RESTING)
		chance -= 15;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Pos{G%d{x",chance);

	if(victim->level > ch->level)
		chance -= (victim->level - ch->level) * 5;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Lvl{G%d{x",chance);

	if (IS_NPC(victim))
		chance += 10;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," Npc{G%d{x",chance);

	chance += ch->getslvl(gsn_assassinate) * 2;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," slvl{G%d{x\n\r",chance);

	check_killer(ch,victim);

	if (roll_chance(ch,chance)){
		if (dice(1,2) == 1){
			WAIT_STATE( ch, skill_table[gsn_assassinate].beats*2 );
			act("You jerk away and $n's blade grazes your throat.",ch,NULL,victim,TO_VICT);
			act("$N twists away in time that your attack only grazes $M.",ch,NULL,victim,TO_CHAR);
			act("$N barely escapes $n's assassination attempt.",ch,NULL,victim,TO_NOTVICT);
			skill_damage(ch,victim,victim->hit / 3,gsn_assassinate,DAM_SLASH,true);

			affect_set(victim,TO_AFFECTS,gsn_paranoia,ch->level,1,!IS_IMMORTAL(ch) ? 172 : 0,APPLY_INT,APPLY_INT,0);
		}
		else{
			WAIT_STATE(ch,skill_table[gsn_assassinate].beats);
			act("You sink your blade into $N's chest and {Rblood{x spurts out of the wound!",ch,NULL,victim,TO_CHAR);
				victim->send("You feel a cold blade piercing your chest, mortally wounding you!\n\r");
			if(!check_stealth(ch)){
				unSneak(ch);
				act("In a spray of {Rblood{x, $n sinks $s blade into $N's chest!",ch,NULL,victim,TO_NOTVICT);
			}
			else{
				act("$N grabs $S side as {Rblood{x sprays out of a wound!",ch,NULL,victim,TO_NOTVICT);
			}
			victim->hit = 10;

			affect_set(victim,TO_AFFECTS,gsn_rupture,ch->level,ch->getslvl(gsn_rupture),ch->level/10,APPLY_STR,-5,AF_RUPTURE);
			affect_set(victim,TO_AFFECTS,gsn_rupture,ch->level,ch->getslvl(gsn_rupture),ch->level/10,APPLY_STR,-5,AF_PREVENTHEAL);
			stop_fighting(victim,true);
		}
		check_improve(ch,gsn_assassinate,true,1);
	}
	else{
		ch->send("You fail to assassinate them.\n\r");
		WAIT_STATE(ch,skill_table[gsn_assassinate].beats*2);
		check_improve(ch,gsn_assassinate,false,1);
		skill_damage(ch,victim,0,gsn_assassinate,DAM_NONE,true);

		affect_set(victim,TO_AFFECTS,gsn_paranoia,ch->level,1,!IS_IMMORTAL(ch) ? 172 : 0,APPLY_INT,0,0);
	}
}

void do_rupture(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;
	int chance,hDamage,poop;

	one_argument(argument,arg);

	poop = 0;

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

	if(!canFight(ch,victim,true))
		return;

	if (victim->isaff(AF_RUPTURE)){
		send_to_char("Their arteries are already damaged!\n\r",ch);
		return;
	}

	if (IS_NPC(victim) && victim->isact(AT_UNDEAD)){
		ch->send("That one.... has no blood.\n\r");
		return;
	}

	if (!(obj = get_eq_char(ch,WEAR_WIELD))){
		send_to_char("You need to wield a weapon to strike an enemy's arteries.\n\r",ch);
		return;
	}

	if (!is_weapon_sharp(obj)){
		send_to_char( "You cannot strike arteries with a blunt weapon!\n\r", ch );
		return;
	}

	chance = get_skill(ch,gsn_rupture) * .75;

	chance += (get_curr_stat (victim, STAT_AGI))-(get_curr_stat (ch, STAT_AGI));

	if (!can_see(victim,ch))
		chance += 2;

	WAIT_STATE(ch,skill_table[gsn_rupture].beats);


	if (!roll_chance(ch,chance)){
		act("$n strikes at you, but isn't even close!",ch,NULL,victim,TO_VICT);
		act("You strike forward at $N and miss!",ch,NULL,victim,TO_CHAR);
		act("$n strikes out at $N and misses!",ch,NULL,victim,TO_NOTVICT);
		skill_damage(ch,victim,0,gsn_rupture,DAM_PIERCE,true);
		check_improve(ch,gsn_rupture,false,1);
	}
	else{
		if (check_sidestep(ch,victim,obj)
		||  check_dodge(ch,victim,obj)
		||  check_feint(ch,victim)
		||  check_mirage(ch,victim)
		|| (victim->isaff(AF_PHASE) && check_phase(ch,victim))){
			skill_damage(ch,victim,0,gsn_strike,DAM_PIERCE,false);
			check_improve(ch,gsn_rupture,false,1);
			return;
		}
		act("$n slips through your defenses and you feel a hot pain as $s blade tears your flesh open!",ch,NULL,victim,TO_VICT);
		act("You strike at $N, and pull your blade out quickly in a spray of blood!",ch,NULL,victim,TO_CHAR);
		act("$n sinks $s blade into $N, masterfully rupturing an artery!",ch,NULL,victim,TO_NOTVICT);

		affect_set(victim,TO_AFFECTS,gsn_rupture,ch->level,ch->getslvl(gsn_rupture),ch->level/10,APPLY_STR,-5,AF_RUPTURE);

		hDamage = number_range(ch->level*3, ch->level*6.5 );

		skill_damage(ch,victim,hDamage, gsn_rupture,DAM_PIERCE,true);
		check_improve(ch,gsn_rupture,true,1);
	}
	check_killer(ch,victim);
}

void do_kneeshot(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument,arg);

	if ((chance = get_skill(ch,gsn_kneeshot)) < 1 || IS_NPC(ch)){
		send_to_char("Huh?  What's that?\n\r",ch);
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
	if(!canFight(ch,victim,true))
		return;

	if (victim->isaff(AF_FLYING)){
		act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (victim->bashwait > 0){
		act("$N is already down.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 10;/* bigger = harder to kneeshot */

	chance += get_curr_stat(ch,STAT_STR);
	chance -= get_curr_stat(victim,STAT_AGI) / 2;

	if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
		chance += 10;
	if (victim->isoff(OF_FAST) || victim->isaff(AF_HASTE))
		chance -= 20;

	chance += (ch->level - victim->level) * 2;

	if (roll_chance(ch,chance)){
		if (!victim->isaff(AF_FLYING)){
			act("$n kicks at your knees and you fall over in pain!",ch,NULL,victim,TO_VICT);
			act("You kick $N's knees and $E goes down!",ch,NULL,victim,TO_CHAR);
			act("$n kicks $N in the knee, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
			if (!do_downstrike(ch,victim))
				if(!do_trample(victim))
					do_facekick(ch,victim);
			DAZE_STATE(victim,2 * PULSE_VIOLENCE);
			victim->bashwait = 4;
		}
		else{
			act("$n kicks at your knees!",ch,NULL,victim,TO_VICT);
			act("You kick $N's knees!",ch,NULL,victim,TO_CHAR);
			act("$n kicks $N in the knee.",ch,NULL,victim,TO_NOTVICT);
		}
		WAIT_STATE(ch,skill_table[gsn_kneeshot].beats);
		check_improve(ch,gsn_kneeshot,true,2);
		skill_damage(ch,victim,3*number_range(2, 2 +  2 * victim->size),gsn_kneeshot, DAM_BASH,true);
	}
	else{
		WAIT_STATE(ch,skill_table[gsn_kneeshot].beats*2/3);
		skill_damage(ch,victim,0,gsn_kneeshot,DAM_BASH,true);
		check_improve(ch,gsn_kneeshot,false,3);
	}
}

void do_shadowstrike(CHAR_DATA *ch,char *argument){
	int chance = get_skill(ch,gsn_shadowstrike)*.85,hDamage=0;
	char arg[MIL];
	CHAR_DATA *victim,*pet;
	OBJ_DATA *obj;
	bool improve=true;

	one_argument(argument,arg);

	if (ch->fighting){
		ch->send("Not in combat!\n\r");
		return;
	}

	if (!(victim = grab_char(ch,arg,true)))
		return;

    if (can_see(victim,ch)){
		ch->send("You can't stealthily attack someone who can see you.\n\r");
		return;
	}

	if (ch->getmana() < 100){
		ch->send("You don't enough energy.\n\r");
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if (!(obj = get_eq_char(ch,WEAR_WIELD)) || !is_weapon_pierce(obj)){
		send_to_char("You must use a piercing weapon to strike at your foe.\n\r",ch);
		return;
	}

	if (obj->value[0] == WEAPON_DIRK)
		chance +=5;

	if (obj->value[0] == WEAPON_DAGGER)
		chance +=3;

	chance += get_curr_stat(ch,STAT_AGI) - get_curr_stat(victim,STAT_WIS);
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{bShadowstrike({c%d{x)\n\r",chance);
    if (!roll_chance(ch,chance)){
		act("A flash of silver whizzes by your head!",ch,NULL,victim,TO_VICT);
		act("You strike at $N and miss!",ch,NULL,victim,TO_CHAR);
		act("$n strikes out at $N and stumbles right past $M!",ch,NULL,victim,TO_NOTVICT);
		improve=false;
		ch->modmana(-50);
	}
	else{
		if (check_sidestep(ch,victim,obj)
		||  check_duck(ch,victim,obj,true)
		||  check_mirage(ch,victim)
		|| (victim->isaff(AF_PHASE) && check_phase(ch,victim))){
			damage(ch,victim,0,gsn_shadowstrike,DAM_PIERCE,false);
			check_improve(ch,gsn_shadowstrike,false,1);
			return;
		}
		act("You feel something sink into your flesh from nowhere!",ch,NULL,victim,TO_VICT);
		act("You stealthily hit $N!",ch,NULL,victim,TO_CHAR);
		act("Something strikes $N!",ch,NULL,victim,TO_NOTVICT);

		ch->modmana(-100);

		hDamage = dice(obj->value[1],obj->value[2]) * 1.5;

		if (check_critical(ch,victim))
			hDamage *= 1.40;
	}
	pet							= create_mobile(get_mob_index(1));
	pet->level = 1;
	char_to_room(pet,ch->in_room);
	free_string(pet->short_descr);
	pet->short_descr			= str_dup("Something");
	free_string(pet->name);
	pet->name					= str_dup("Something");
	free_string(pet->long_descr);
	pet->long_descr				= str_dup("Something");
	free_string(pet->description);
	pet->description			= str_dup("Something");

    WAIT_STATE(ch,skill_table[gsn_shadowstrike].beats * 1.25);
	damage(pet,victim,hDamage,TYPE_HIT,DAM_PIERCE,true);
	extract_char(pet,true);
	check_improve(ch,gsn_shadowstrike,improve,1);
}

void do_terrorize(CHAR_DATA *ch,char *argument){
	char arg[MSL];
	CHAR_DATA *victim;
	int chance = get_skill(ch,gsn_paralyzingfear) * .75;
	AFFECT_DATA af;

	one_argument(argument,arg);

	if (!(victim = grab_char(ch,arg,true)) || !canFight(ch,victim,true))
		return;

	if (victim->isaff(AF_SLOWCAST)){
		ch->send("They're already scared senseless.\n\r");
		return;
	}

	chance += get_curr_stat(ch,STAT_STR) - get_curr_stat(victim,STAT_WIS);
	if (roll_chance(ch,chance)){
		if(!saves_skill(ch->level,victim,DAM_MENTAL)){
			affect_set(victim,TO_AFFECTS,gsn_paralyzingfear,ch->level,ch->getslvl(gsn_paralyzingfear),ch->level/10,APPLY_WIS,-5,AF_SLOWCAST);
			act("You feel an incredible terror paralyze you, destroying your concentration.",ch,NULL,victim,TO_VICT);
			act("$N screams in terror and shakes violently.",ch,NULL,victim,TO_NOTVICT);
			act("You scare the crap out of $N.",ch,NULL,victim,TO_CHAR);
			check_improve(ch,gsn_paralyzingfear,true,3);
			skill_damage(ch,victim,ch->level,gsn_paralyzingfear,DAM_MENTAL,true);
			return;
		}
		else
			act("$E seems to be immune.",ch,NULL,victim,TO_CHAR);
	}
	skill_damage(ch,victim,0,gsn_paralyzingfear,DAM_MENTAL,false);
	check_improve(ch,gsn_paralyzingfear,false,1);
}

void do_poisoncloud(CHAR_DATA*ch,char*argument){
	CHAR_DATA *vch,*vch_next;
    AFFECT_DATA af;
	int chance = number_range(get_skill(ch,gsn_poisoncloud) * .7,get_skill(ch,gsn_poisoncloud) * .45);

	WAIT_STATE(ch,skill_table[gsn_poisoncloud].beats);
	if (!roll_chance(ch,chance)){
		ch->send("Sugar doesn't work as a poison!\n\r");
		act("$n throws some sugar in the air... what?",ch,NULL,NULL,TO_ROOM);
		for(vch=ch->in_room->people;vch;vch=vch_next){
			vch_next = vch->next_in_room;
			if (vch != ch)
				skill_damage(ch,vch,10,gsn_bludgeons,DAM_OTHER,true);
		}
		check_improve(ch,gsn_poisoncloud,false,2);
	}
	else{
		ch->send("You release a cloud of poisonous gas into the room!\n\r");
		act("$n releases a cloud of poison into the room!!",ch,NULL,NULL,TO_ROOM);
		for(vch=ch->in_room->people;vch;vch=vch_next){
			vch_next = vch->next_in_room;
			if(vch->master != ch && ch != vch && ((!saves_skill(ch->level,vch,DAM_POISON) && canFight(ch,vch,true)) || (ch == vch && !saves_skill(ch->level/2,vch,DAM_POISON)))){
				affect_set(vch,TO_AFFECTS,gsn_poison,ch->level,ch->getslvl(gsn_poisoncloud),ch->level/10,APPLY_STR,-4,AF_POISON);
				send_to_char("You feel very sick.\n\r",vch);
				act("$n looks very ill.",vch,NULL,NULL,TO_ROOM);
				//skill_damage(ch,vch,ch->level,gsn_poisoncloud,DAM_POISON,true);
			}
			if (!check_evade(vch,ch))
				if (check_guard(vch,ch))
					multi_hit(vch,ch->guardby,TYPE_UNDEFINED,false);
				else
					multi_hit(vch,ch,TYPE_UNDEFINED,false);
		}
		check_improve(ch,gsn_poisoncloud,true,3);
	}
}

void do_blindingdust(CHAR_DATA*ch,char*argument){
	CHAR_DATA *vch,*vch_next;
    AFFECT_DATA af;
	int chance = number_range(get_skill(ch,gsn_blindingdust) * .7,get_skill(ch,gsn_blindingdust) * .4);

	WAIT_STATE(ch,skill_table[gsn_blindingdust].beats);
	if (!roll_chance(ch,chance)){
		ch->send("Dust doesn't work as a blinding agent!\n\r");
		act("$n throws some dust in the air... uhh..?",ch,NULL,NULL,TO_ROOM);
		for(vch=ch->in_room->people;vch;vch=vch_next){
			vch_next = vch->next_in_room;
			if (vch != ch)
				skill_damage(ch,vch,10,gsn_exotic,DAM_OTHER,true);
		}
		check_improve(ch,gsn_blindingdust,false,2);
	}
	else{
		ch->send("You toss a cloud of blinding dust into the room!\n\r");
		act("$n tosses some kind of dust into the room..",ch,NULL,NULL,TO_ROOM);
		for(vch=ch->in_room->people;vch;vch=vch_next){
			vch_next = vch->next_in_room;
			if(vch->master != ch && ch != vch && ((!saves_skill(ch->level,vch,DAM_EARTH) && canFight(ch,vch,true)) || (ch == vch && !saves_skill(ch->level/2,vch,DAM_EARTH)))){
				affect_set(vch,TO_AFFECTS,gsn_blindness,ch->level,ch->getslvl(gsn_blindingdust),ch->level/10,APPLY_WIS,-4,AF_BLIND);
				send_to_char("You can't see!\n\r",vch);
				act("$n fumbles around and looks blinded!",vch,NULL,NULL,TO_ROOM);
				//skill_damage(ch,vch,ch->level,gsn_blindingdust,DAM_EARTH,true);
			}
			if (!check_evade(vch,ch))
				if (check_guard(vch,ch))
					multi_hit(vch,ch->guardby,TYPE_UNDEFINED,false);
				else
					multi_hit(vch,ch,TYPE_UNDEFINED,false);
		}
		check_improve(ch,gsn_blindingdust,true,3);
	}
}

void do_neuralize(CHAR_DATA *ch,char *argument){
	AFFECT_DATA *af = new_affect();
	OBJ_DATA *obj;
	int sn = gsn_neural_agent,skill = get_skill(ch,sn);

	if (argument[0] == '\0'){
		send_to_char("Envenom what item?\n\r",ch);
		return;
	}
	if(skill < 1){
		ch->send("What do you think you are, some kind of male in dark attire?\n\r");
		return;
	}
	if(!(obj = get_obj_carry(ch,argument,ch))){
		ch->send("You aren't carrying anything like that.\n\r");
		return;
	}
	if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_NEEDLE){
		if (IS_WEAPON_STAT(obj,WPN_FLAMING)
		||  IS_WEAPON_STAT(obj,WPN_FROST)
		||  IS_WEAPON_STAT(obj,WPN_VAMPIRIC)
		||  IS_WEAPON_STAT(obj,WPN_POWERDRAIN)
		||  IS_WEAPON_STAT(obj,WPN_SHARP)
		||  IS_WEAPON_STAT(obj,WPN_VORPAL)
		||  IS_WEAPON_STAT(obj,WPN_SERRATED)
		||  IS_WEAPON_STAT(obj,WPN_HOLY)
		||  IS_WEAPON_STAT(obj,WPN_DEMONIC)
		||  IS_WEAPON_STAT(obj,WPN_SHOCKING)
		|| IS_OBJ_STAT(obj,ITM_BURN_PROOF)|| IS_OBJ_STAT(obj,ITM_SHOCK_PROOF)){
			act("You can't seem to neuralize $p.",ch,obj,NULL,TO_CHAR);
			return;
		}
		if (obj->value[3] < 0 || attack_table[obj->value[3]].damage == DAM_BASH){
			send_to_char("You can only neuralize edged weapons.\n\r",ch);
			return;
		}

		if (IS_WEAPON_STAT(obj,WPN_NEURAL)){
			act("$p is already neuralized.",ch,obj,NULL,TO_CHAR);
			return;
		}

		if (roll_chance(ch,skill)){
			affect_set(obj,TO_WEAPON,gsn_neural_agent,ch->level * number_percent() / 100,ch->getslvl(gsn_neural_agent),ch->level/2 * number_percent() / 100,0,ch->getslvl(sn),WPN_NEURAL);

			act("$n coats $p with a paralyzing agent.",ch,obj,NULL,TO_ROOM);
			act("You coat $p with a paralyzing agent.",ch,obj,NULL,TO_CHAR);
			check_improve(ch,sn,true,3);
			WAIT_STATE(ch,skill_table[sn].beats);
			return;
		}
		else{
			act("You fail to neuralize $p.",ch,obj,NULL,TO_CHAR);
			check_improve(ch,sn,false,3);
			WAIT_STATE(ch,skill_table[sn].beats);
			return;
		}
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
}

void do_caltrops(CHAR_DATA *ch,char *argument){
	int skill = get_skill(ch,gsn_caltrops);
	OBJ_DATA *obj;
	bool found = false;

	if(skill < 1){
		ch->send("That's a bird!\n\r");
		return;
	}

	if(ch->in_room->israf(RAF_CALTROPS)){
		ch->send("This room is already littered with caltrops.\n\r");
		return;
	}

	for (obj = ch->carrying;obj;obj = obj->next_content){
		if(obj->item_type == ITEM_CALTROPS){
			found = true;
			break;
		}
	}

	if (!found){
		ch->send("You throw down some air. Good for you!\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		check_improve(ch,gsn_caltrops,true,1);
		obj->value[2] = 1;
		act("$n throws down a handful of $p.",ch,obj,NULL,TO_ROOM);
		act("You throw down a handful of $p.",ch,obj,NULL,TO_CHAR);
		ch->in_room->setraf(RAF_CALTROPS);
		obj_from_char(obj);
		obj_to_room(obj,ch->in_room);
		REMOVE_BIT(obj->wear_flags,ITEM_TAKE);
	}
	else{
		ch->send("You somehow mess up this task...\n\r");
		check_improve(ch,gsn_caltrops,false,2);
		return;
	}
}

void do_needle(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *needle;
	AFFECT_DATA af;
	int sn = gsn_needle,skill = get_skill(ch,sn),loc,modif,bv;

	if(!(victim = grab_char(ch,argument,true)))
		return;

	if(!canFight(ch,victim,true))
		return;

	if ((needle = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R))) == NULL){
		ch->send("With what?\n\r");
		return;
	}
	if(needle->item_type != ITEM_NEEDLE){
		ch->send("You must be holding a needle.\n\r");
		return;
	}

	if(IS_WEAPON_STAT(needle,WPN_POISON)){
		loc  = APPLY_STR;
		modif  = -1 * ch->getslvl(sn);
		bv = AF_POISON;
	}
	else if(IS_WEAPON_STAT(needle,WPN_PESTILENCE)){
		loc = APPLY_STR;
		modif = -2 * ch->getslvl(sn);
		bv = AF_PLAGUE;
	}
	else if(IS_WEAPON_STAT(needle,WPN_NEURAL)){
		loc = APPLY_AGI;
		modif = -1 * ch->getslvl(sn);
		bv = AF_SLOWWALK;
	}
	else{
		ch->send("That needle does nothing but stab!\n\r");
		return;
	}

	if(can_see(victim,ch)){
		act("$E can see you!",ch,NULL,victim,TO_CHAR);
		return;
	}

	if(roll_chance(ch,skill)){
		if(number_percent() > ch->getslvl(sn) * 15){
			act("You failed to poison $N and $E saw you!",ch,NULL,victim,TO_CHAR);
			act("You catch $n trying to prick you!",ch,NULL,victim,TO_VICT);
			act("$N catches $n trying to prick $M!",ch,NULL,victim,TO_NOTVICT);
			damage(ch,victim,0,sn,DAM_PIERCE,false);
		}
		else
			act("You prick $N, poisoning $M.",ch,NULL,victim,TO_CHAR);
		act("$N seems very sick.",NULL,NULL,victim,TO_ROOM);
		act("You feel very sick.",NULL,NULL,victim,TO_VICT);
		affect_join(victim,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),UMAX(1,ch->getslvl(sn)),loc,modif,bv);
		if(--needle->value[0] < 1){
			act("$p snaps!",NULL,needle,NULL,TO_ROOM);
			extract_obj(needle);
		}
	}
	else{
		check_improve(ch,sn,false,1);
		if(number_percent() < 50){
			act("You failed to poison $N and $E saw you!",ch,NULL,victim,TO_CHAR);
			act("You catch $n trying to prick you!",ch,NULL,victim,TO_VICT);
			act("$N catches $n trying to prick $M!",ch,NULL,victim,TO_NOTVICT);
			damage(ch,victim,0,sn,DAM_PIERCE,false);
		}
		else
			ch->send("You failed.\n\r");
	}
}

void do_gouge(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;
	int dam,sn = gsn_gouge,skill = get_skill(ch,sn),chance;

	if (!argument[0]){
		if(!(victim = ch->fighting)){
			send_to_char("Who??\n\r",ch);
			return;
		}
	}
	else if(!(victim = get_char_room(ch,NULL,argument))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if(skill < 1){
		ch->send("Isn't that a painter?\n\r");
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if (!(obj = get_eq_char(ch,WEAR_WIELD)) || !is_weapon_pierce(obj)){
		send_to_char("You need to wield a piercing weapon.\n\r",ch);
		return;
	}

	chance = get_weapon_skill(ch,get_weapon_sn(ch,false));

	skill /= 2;
	chance /= 2;
	skill += chance;

	dam = melee_hit(ch,victim,dice(obj->value[1],obj->value[2]),false) * 2;


	if(roll_chance(ch,skill)){
		skill_damage(ch,victim,dam,sn,DAM_PIERCE,true);
		check_improve(ch,sn,true,1);
		if(number_percent() < ch->getslvl(sn) * 8){
			act("You dig $p into $N's arm and $E screams in agony!",ch,obj,victim,TO_CHAR);
			act("$n digs $p into your arm and you scream in agony!",ch,obj,victim,TO_VICT);
			act("$n digs $p into $N's arm and $E screams in agony!",ch,obj,victim,TO_NOTVICT);
			affect_set(victim,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),UMAX(1,ch->getslvl(sn)),APPLY_AGI,-1 * ch->getslvl(sn),0);
		}
	}
	else{
		skill_damage(ch,victim,0,sn,DAM_PIERCE,true);
		check_improve(ch,sn,false,1);
	}
	WAIT_STATE(ch,skill_table[sn].beats);
}

void do_sap(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;
	int chance;

	if(!argument[0]){
		ch->send("Who?\n\r");
		return;
	}
	one_argument(argument,arg);

	if (!(victim = grab_char(ch,arg,false)))
		return;

	if (victim->isaff(AF_SLEEP) || victim->isaff(AF_SAP) || (IS_NPC(victim) && victim->isact(AT_UNDEAD))){
		send_to_char("They can't be sapped.\n\r",ch);
		return;
	}

	if (can_see(victim,ch)){
		send_to_char("You can't sneak up on someone who's looking at you.\n\r",ch);
		return;
	}

	if (victim->fighting != NULL){
		send_to_char("You can't sap someone who's in combat!\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	chance = get_skill(ch,gsn_sap);

//	chance += (get_curr_stat(ch,STAT_STR))-(get_curr_stat(victim,STAT_STR));
//	chance += (get_curr_stat(ch,STAT_AGI))-(get_curr_stat(victim,STAT_AGI));
//	chance += (ch->size)-(victim->size);

	if (!(obj = get_eq_char(ch,WEAR_WIELD))){
		send_to_char("You need to wield a blunt weapon to sap.\n\r",ch);
		return;
	}

	if (is_weapon_blunt(obj) == false){
		send_to_char("You must use a blunt wield to sap someone.\n\r",ch);
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_sap].beats);

	if (number_percent() >= chance){
		check_improve(ch,gsn_sap,false,1);
		if(!check_stealth(ch))
			skill_damage(ch,victim,0,gsn_sap,DAM_NONE,true);
		else
			act("There is a sudden flash of movement.",ch,NULL,NULL,TO_ROOM);
	}
	else{
		check_improve(ch,gsn_sap,true,2);
		if (IS_AWAKE(victim)){
			act("You smack $N on the back of the head.",ch,NULL,victim,TO_CHAR);
			send_to_char("You feel a smack on the back of your head and everything goes black!\n\r",victim);
			act("$n smacks $N on the back of the head!",ch,NULL,victim,TO_NOTVICT);
			victim->position	= POS_STUNNED;
			affect_set(victim,TO_AFFECTS,gsn_sap,ch->level,ch->getslvl(gsn_sap),1,APPLY_NONE,0,AF_SAP);
		}
	}
}

void check_catnap(CHAR_DATA *ch){
	CHAR_DATA *victim;
	int sn = gsn_catnap,skill;

	for(victim = ch->in_room->people;victim;victim = victim->next_in_room){
		if(ch == victim)
			continue;
		if(!victim->isaff(AF_LIGHTSLEEP))
			continue;
		if((skill = get_skill(victim,sn)) < 1){
			victim->remaff(AF_LIGHTSLEEP);
			continue;
		}
		skill *= .75;
		skill += victim->getslvl(sn)*3;

		if(roll_chance(victim,skill)){
			victim->position = POS_STANDING;
			act("$N's movements rouse you from your slumber.",victim,NULL,ch,TO_CHAR);
			act("Your movements wake $n.",victim,NULL,ch,TO_VICT);
			act("$N's movements wake $n.",victim,NULL,ch,TO_NOTVICT);
			do_function(victim,&do_look,"");
			if(victim->getslvl(sn) > 4)
				do_function(victim,&do_hide,"");
			check_improve(victim,sn,true,1);
		}
		else
			check_improve(victim,sn,false,1);
	}
}

void do_catnap(CHAR_DATA *ch,char *argument){
	AFFECT_DATA af;
	int sn = gsn_catnap,skill = get_skill(ch,sn);
	if(skill < 1){
		ch->send("Meow!?\n\r");
		return;
	}

	if(ch->fighting || ch->position > POS_STANDING || RIDDEN(ch) || MOUNTED(ch) || ch->isaff(AF_INSOMNIA)){
		ch->send("You can't sleep like that.\n\r");
		return;
	}

	act("You lay down to take a catnap",ch,NULL,NULL,TO_CHAR);
	act("$n lays down and seems to snooze lightly.",ch,NULL,NULL,TO_ROOM);
	ch->position	= POS_SLEEPING;
	affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),UMAX(1,ch->getslvl(sn)),APPLY_NONE,0,AF_LIGHTSLEEP);
	check_improve(ch,sn,true,1);
}

int check_counterfeit(CHAR_DATA *ch,int amount){
	int sn = gsn_counterfeit,skill = get_skill(ch,sn);

	if(skill < 1)
		return amount;

	skill /= 5;

	if(roll_chance(ch,skill)){
		printf_to_char(ch,"You slip them %d coins.\n\r",(amount / 10) * ch->getslvl(sn));
		amount -= (amount / 10) * ch->getslvl(sn);
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	return amount;
}

void do_culminate(CHAR_DATA *ch,char *argument){
	AFFECT_DATA af;
	int sn = gsn_culmination,skill = get_skill(ch,sn);

	if(skill < 1){
		ch->send("You can't... culminate?\n\r");
		return;
	}

	if(!ch->fighting){
		ch->send("You're in no fight to do that.\n\r");
		return;
	}
	if(((ch->fighting->hit*100) / ch->fighting->max_hit) > 25){
		ch->send("They are not weak enough to culminate your kill.\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		ch->send("Your muscles tighten and your mind blurs!\n\r");
		act("$n's body seems tighter.",ch,NULL,NULL,TO_ROOM);
		affect_set(ch,TO_AFFECTS,gsn_culmination,ch->level,ch->getslvl(gsn_culmination),0,APPLY_STR,ch->getslvl(sn) * 2,AF_BDAMTAKE);
		affect_set(ch,TO_AFFECTS,gsn_culmination,ch->level,ch->getslvl(gsn_culmination),0,APPLY_DAMROLL,ch->getslvl(sn) * 2,AF_BDAMTAKE);
		affect_set(ch,TO_AFFECTS,gsn_culmination,ch->level,ch->getslvl(gsn_culmination),0,APPLY_HITROLL,ch->getslvl(sn) * 2,AF_BDAMTAKE);
		affect_set(ch,TO_AFFECTS,gsn_culmination,ch->level,ch->getslvl(gsn_culmination),0,APPLY_AC,ch->getslvl(sn) * 500,AF_BDAMTAKE);
		affect_set(ch,TO_AFFECTS,gsn_culmination,ch->level,ch->getslvl(gsn_culmination),0,APPLY_END,-ch->getslvl(sn) * 4,AF_BDAMTAKE);
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("You fail.\n\r");
		check_improve(ch,sn,false,1);
	}
}

void do_mark(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int sn = gsn_mark,skill = get_skill(ch,sn);

	if(skill < 1){
		ch->send("Hamill?\n\r");
		return;
	}
	if(!argument[0]){
		ch->send("Mark info:\n\r");
		return;
	}
	if(!(victim = get_char_world(ch,argument))){
		ch->send("Who?\n\r");
		return;
	}

	if(!canPK(ch,victim,true))
		return;

	if(roll_chance(ch,skill)){
		act("You set your sights on $N.",ch,NULL,victim,TO_CHAR);
		act("$n seems set on something.",ch,NULL,NULL,TO_ROOM);
		if(!check_stealth(ch))
			act("$n has their sights on you.",ch,NULL,victim,TO_VICT);
		ch->mark = victim;
		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),UMAX(1,ch->getslvl(sn)),APPLY_WIS,ch->getslvl(sn),AF_MARK);
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("You fail.\n\r");
		if(!check_stealth(ch))
			victim->send("Someone seems to want you dead...\n\r");
		check_improve(ch,sn,false,1);
	}
}

void do_nerve(CHAR_DATA *ch,char *argument){
	AFFECT_DATA af;
	CHAR_DATA *victim;
	int sn = gsn_nerve,skill = get_skill(ch,sn);

	if(!(victim = grab_char(ch,argument,true)))
		return;

	if(roll_chance(ch,skill)){
		act("You grab $N, squeezing one of $S nerves.",ch,NULL,victim,TO_CHAR);
		act("$n grabs you, squeezing one of your nerves, and your body gets weak.",ch,NULL,victim,TO_VICT);
		act("$n grabs $N, and $N staggers a bit.",ch,NULL,victim,TO_NOTVICT);
		affect_set(victim,TO_AFFECTS,sn,victim->level,ch->getslvl(sn),2,APPLY_STR,-1 * ch->getslvl(sn) * 3,AF_WEAKEN);

		if(!check_stealth(ch))
			damage(ch,victim,0,sn,DAM_OTHER,false);
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("You fail.\n\r");
		damage(ch,victim,0,sn,DAM_OTHER,true);
		check_improve(ch,sn,false,1);
	}
}
