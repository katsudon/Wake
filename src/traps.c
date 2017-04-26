#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */
void do_trapset			( CHAR_DATA*,char* );
void do_trapmake		( CHAR_DATA*,char* );
void do_trapdisable		( CHAR_DATA*,char* );
void do_trapidentify	( CHAR_DATA*,char* );

void do_traps( CHAR_DATA *ch, char *argument )
{
    char arg1[MSL];

	if (get_skill(ch,gsn_traps) < 1)
	{
		send_to_char("You don't even know the first thing about traps!\n\r",ch);
		return;
	}
	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0' || argument[0] == '\0')
    {
		send_to_char("Syntax: trap <set/make/disable> <target> <victim>\n\r",ch);
		return;
    }

	if(!str_prefix(arg1,"set"))
		do_trapset(ch,argument);
	else if(!str_prefix(arg1,"make"))
		do_trapmake(ch,argument);
	else if(!str_prefix(arg1,"disable"))
		do_trapdisable(ch,argument);
	else if(!str_prefix(arg1,"identify"))
		do_trapidentify(ch,argument);
	else
		send_to_char("Improper trap command, use set, identify, make or disable.\n\r",ch);
	return;
}

void do_trapset(CHAR_DATA *ch,char *argument)
{
	OBJ_DATA *obj;
	char arg2[MSL], arg3[MSL];
	int chance;

	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);

	if ((obj = get_obj_carry(ch,arg2,ch)) == NULL)
	{
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}

	if (obj->item_type != ITEM_TRAP)
	{
		send_to_char("You can set traps.\n\r",ch);
		return;
	}

	if (obj->value[0] == TRAP_TARGET && arg3[0] == '\0')
	{
		send_to_char("You must specify a target for this type of trap.\n\r",ch);
		return;
	}

	if (!can_see_obj(ch,obj))
	{
		send_to_char("You don't see that in your inventory.\n\r",ch);
		return;
	}

    chance = get_skill(ch,gsn_traps) * .85;

    WAIT_STATE(ch,skill_table[gsn_traps].beats);

	if (number_percent() <= chance)
	{
		SET_BIT(obj->extra_flags,ITM_HIDDEN);
		obj->owner = str_dup(ch->name);
		REMOVE_BIT(obj->wear_flags,ITEM_TAKE);
		if (obj->value[0] == TRAP_SNARE)
			send_to_char("You quietly conceal a snare trap.\n\r",ch);
		if (obj->value[0] == TRAP_EXPLODING)
			send_to_char("You quietly conceal an exploding trap.\n\r",ch);
		if (obj->value[0] == TRAP_WEB)
			send_to_char("You quietly conceal a web trap.\n\r",ch);
		obj_from_char(obj);
		obj_to_room(obj,ch->in_room);
		check_improve(ch,gsn_traps,true,4);
		return;
	}
	else
	{
		send_to_char("You fail to set your trap and it snaps back at you!\n\r",ch);
		act("$n is attacked by $s own trap as $e fails to properly set it!",ch,NULL,NULL,TO_ROOM);
		damage(ch,ch,ch->level/2,gsn_traps,DAM_BASH,true);
		check_improve(ch,gsn_traps,false,2);
		return;
	}
}

void do_trapmake(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj;
	char arg2[MSL], buf[MSL];
	const char *name;
	int chance,skl = get_curr_stat(ch,STAT_INT);

	argument = one_argument(argument,arg2);

	if (!(obj = get_obj_carry(ch,arg2,ch))){
		ch->send("You do not have that item.\n\r");
		return;
	}

	if (!can_see_obj(ch,obj)){
		ch->send("You don't see that in your inventory.\n\r");
		return;
	}

	if (obj->item_type != ITEM_TRAPPARTS){
		ch->send("You can only assemble trap parts.\n\r");
		return;
	}

	chance = (get_skill(ch,gsn_traps) * .80);
	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"Traps: (%d)\n\r",chance);
	chance /= obj->value[1];
	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"Traps: (%d)\n\r",chance);
	if (chance + skl > 100)
		chance = chance + ((100 - chance)/2);
	else
		chance += skl;

	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"Traps: (%d)\n\r",chance);
	WAIT_STATE(ch,skill_table[gsn_traps].beats);

	if (number_percent() <= chance){
		switch(obj->value[0]){
		case TRAP_TARGET:	name = "target trap";	break;
		case TRAP_EXPLODING:name = "exploding trap";break;
		case TRAP_SNARE:	name = "snare trap";	break;
		case TRAP_WEB:		name = "web trap";		break;
		default:
			ch->send("This set of trap parts is bugged, please report it to an immortal.\n\r");
			return;
		}
		printf_to_char(ch,"You successfully create %s %s.\n\r",IS_VOWELL(name[0]) ? "an" : "a",name);

		sprintf(buf,"%s %s",IS_VOWELL(name[0]) ? "an" : "a",name);
		free_string(obj->short_descr);
		obj->short_descr = str_dup(buf);
		sprintf(buf,"%s %s is here.",IS_VOWELL(name[0]) ? "an" : "a",name);
		free_string(obj->description);
		obj->description = str_dup(buf);
		sprintf(buf,"%s",name);
		free_string(obj->name);
		obj->name = str_dup(buf);
		obj->item_type = ITEM_TRAP;
		obj->level = ch->level;
		act("$n successfully turns $s set of trap parts into $p.",ch,obj,NULL,TO_ROOM);
		check_improve(ch,gsn_traps,true,2);
	}
	else{
		ch->send("You fail, and ruin your trap parts!\n\r");
		act("$n ruins $s trap, destroying the parts beyond use.",ch,NULL,NULL,TO_ROOM);
		extract_obj(obj);
		check_improve(ch,gsn_traps,false,1);
	}
}

void do_trapidentify(CHAR_DATA *ch,char *argument)
{
	OBJ_DATA *obj;
	char arg2[MSL], buf[MSL];
	int chance,skl = get_curr_stat(ch,STAT_INT);

	argument = one_argument(argument,arg2);

	if ((obj = get_obj_carry(ch,arg2,ch)) == NULL){
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}

	if (!can_see_obj(ch,obj)){
		send_to_char("You don't see that in your inventory.\n\r",ch);
		return;
	}

	if (obj->item_type != ITEM_TRAPPARTS || obj->item_type != ITEM_TRAP){
		send_to_char("You can only examine traps and trap parts.\n\r",ch);
		return;
	}

    chance = get_skill(ch,gsn_traps) * .85;
	chance /= obj->value[1];
	if (chance + skl > 100)
		chance = chance + ((100 - chance)/2);
	else
		chance += skl;

    WAIT_STATE(ch,skill_table[gsn_traps].beats);

	printf_to_char(ch,"You examine %s.\n\r",obj->short_descr);

	if (number_percent() <= chance){
		switch(obj->value[0]){
		case TRAP_TARGET:
			sprintf(buf,"You successfully identify %s as a target %s.",obj->short_descr,obj->item_type == ITEM_TRAPPARTS ? "set of trap parts" : "trap");
			break;
		case TRAP_EXPLODING:
			sprintf(buf,"You successfully identify %s as a exploding %s.",obj->short_descr,obj->item_type == ITEM_TRAPPARTS ? "set of trap parts" : "trap");
			break;
		case TRAP_SNARE:
			sprintf(buf,"You successfully identify %s as a snare %s.",obj->short_descr,obj->item_type == ITEM_TRAPPARTS ? "set of trap parts" : "trap");
			break;
		case TRAP_WEB:
			sprintf(buf,"You successfully identify %s as a web %s.",obj->short_descr,obj->item_type == ITEM_TRAPPARTS ? "set of trap parts" : "trap");
			break;
		default:
			send_to_char("You hold a buggy trap, please report it.\n\r",ch);
			break;
		}
		return;
	}
	else{
		send_to_char("You fail!\n\r",ch);
		return;
	}
}

void do_trapdisable(CHAR_DATA *ch,char *argument)
{
	OBJ_DATA *obj;
	char arg2[MSL];
	int chance;

	argument = one_argument(argument,arg2);

	if ((obj = get_obj_carry(ch,arg2,ch)) == NULL){
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}

	if (!can_see_obj(ch,obj)){
		send_to_char("You don't see that in your inventory.\n\r",ch);
		return;
	}

	if (obj->item_type != ITEM_WEAPON){
		send_to_char("You can only conceal weapons.\n\r",ch);
		return;
	}
    chance = get_skill(ch,gsn_traps) * .75;

    WAIT_STATE(ch,skill_table[gsn_traps].beats);

	return;
}

void check_traps(CHAR_DATA *ch,OBJ_DATA *obj){//Nash gotta fix the slvls here
	CHAR_DATA *tch, *vch, *fvch;
	OBJ_DATA *vobj;
	AFFECT_DATA af;
	int dam,lvl;

	if(ch->isaff(AF_WINDWALK))
		return;
	if (!obj){
		for(obj = ch->in_room->contents; obj != NULL; obj = vobj){
			vobj = obj->next_content;
			if(obj->item_type == ITEM_TRAP)
				break;
		}
		if (obj == NULL || obj->item_type != ITEM_TRAP || CAN_WEAR(obj,ITEM_TAKE))
			return;

		if ((tch = get_char_world(NULL,obj->owner)) != NULL && get_leader(ch) != tch)
			send_to_char("You somehow feel one of your traps has found a victim...\n\r",tch);
		else{
			extract_obj(obj);
			return;
		}

		if (obj->value[0] != TRAP_TARGET && (get_leader(ch) == tch || ch == tch))
			return;

		lvl = obj->level / 2;
		if(do_detecttraps(ch))
			lvl = obj->level/2;
	}
	else{
		if ((tch = get_char_world(NULL,obj->owner)) != NULL && get_leader(ch) != tch)
			send_to_char("You somehow feel one of your traps has found a victim...\n\r",tch);
		else{
			extract_obj(obj);
			return;
		}

		if (obj->value[0] != TRAP_TARGET && (get_leader(ch) == tch || ch == tch))
			return;

		lvl = obj->level * 1.3;
		if(do_detecttraps(ch))
			lvl = obj->level/2;
	}
	switch(obj->value[0]){
	case TRAP_TARGET:
		break;
	case TRAP_EXPLODING:
		if (str_cmp(obj->owner,ch->name) && !saves_spell(lvl + (5 * obj->value[1]),ch,DAM_OTHER)){
			act("$n trips $p and sets off a large explosion!",ch,obj,NULL,TO_ROOM);
			act("You trip $p and set off a large explosion!",ch,obj,NULL,TO_CHAR);
			for(vch = ch->in_room->people; vch != NULL; vch = fvch){
				dam = 200 * obj->value[1];
				fvch = vch->next_in_room;
				if(vch == tch || get_leader(vch) == tch)
					continue;
				dam = UMIN(vch->hit/2,dam);
				damage(tch,vch,dam,skill_lookup("high explosive"),DAM_FIRE,true);
				stop_fighting(vch,true);
			}
			extract_obj(obj);
		}
		break;
	case TRAP_SNARE:
		if (str_cmp(obj->owner,ch->name) && !saves_spell(lvl + (10 * obj->value[1]),ch,DAM_OTHER)){
			act("$n trips $p and is ensnared!",ch,obj,NULL,TO_ROOM);
			act("You trip $p and are ensnared!",ch,obj,NULL,TO_CHAR);
			dam = ((lvl / 2) * (obj->value[1]+1))	/10;
			dam = UMIN(ch->hit/2,dam);
			damage(tch,ch,dam,gsn_traps,DAM_TRAPS,true);
			affect_set(ch,TO_AFFECTS,gsn_traps,lvl,1,3,APPLY_NONE,0,AF_SNARED);
			extract_obj(obj);
			stop_fighting(ch,true);
		}
		break;
	case TRAP_WEB:
		if (str_cmp(obj->owner,ch->name) && !saves_spell(lvl + (5 * obj->value[1]),ch,DAM_OTHER)){
			act("$n trips $p and is entangled in a web!",ch,obj,NULL,TO_ROOM);
			act("You trip $p and are entangled in a web!",ch,obj,NULL,TO_CHAR);
			dam = (lvl * (obj->value[1]+1))	/10;
			dam = UMIN(ch->hit/2,dam);
			damage(tch,ch,dam,gsn_traps,DAM_TRAPS,true);
			affect_set(ch,TO_AFFECTS,gsn_traps,lvl,1,3,APPLY_NONE,0,AF_ENTANGLED);
			extract_obj(obj);
			stop_fighting(ch,true);
		}
		break;
	}
}
