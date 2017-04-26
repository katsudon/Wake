#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"

/*
 * Local functions.
 */

void do_eavesdrop(CHAR_DATA *ch,char *argument){
	char arg[MSL];
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *next_room;
	CHAR_DATA *rch;
	int chance = get_skill(ch,gsn_eavesdrop) * .8,door = 0;
	bool badIRoom = true,badTRoom = true;;

	one_argument(argument,arg);

	if (chance < 1 || IS_NPC(ch)){
		send_to_char("Your ears lack the training for this.\n\r",ch);
		return;
	}

	if (ch->race == race_lookup("daurin"))
		chance *= 1.2;

	if (!str_cmp(arg,"end") || !str_cmp(arg,"cancel") || !str_cmp(arg,"done")){
		send_to_char("You stop listening.\n\r",ch);
		ch->pcdata->eavesdropping = -1;
		return;
	}
	else if(!str_prefix(arg,"north"))
		door = DIR_NORTH;
	else if(!str_prefix(arg,"south"))
		door = DIR_SOUTH;
	else if(!str_prefix(arg,"east"))
		door = DIR_EAST;
	else if(!str_prefix(arg,"west"))
		door = DIR_WEST;
	else if(!str_prefix(arg,"up"))
		door = DIR_UP;
	else if(!str_prefix(arg,"down"))
		door = DIR_DOWN;
	else
		door = 100;

	if ((pexit = ch->in_room->exit[door]) == 0 || (next_room = pexit->u1.to_room) == NULL || door == 100){
		send_to_char("You can't listen that way!\n\r",ch);
		return;
	}

	switch (ch->in_room->sector_type){
	case SECT_INSIDE:
	case SECT_CITY:
	case SECT_FOREST:
	case SECT_TUNNEL:
	case SECT_FORESTCITY:
	case SECT_MOUNTAINCITY:
	case SECT_GRAVEYARD:
	case SECT_GOODTEMPLE:
	case SECT_NEUTRALTEMPLE:
	case SECT_EVILTEMPLE:
		badIRoom = false;
		break;
	}
	switch (next_room->sector_type){
	case SECT_INSIDE:
	case SECT_CITY:
	case SECT_FOREST:
	case SECT_TUNNEL:
	case SECT_FORESTCITY:
	case SECT_MOUNTAINCITY:
	case SECT_GRAVEYARD:
	case SECT_GOODTEMPLE:
	case SECT_NEUTRALTEMPLE:
	case SECT_EVILTEMPLE:
		badTRoom = false;
		break;
	}

	if (badIRoom){
		send_to_char("This isn't the proper setting for eavesdropping.\n\r",ch);
		return;
	}

	if (badTRoom){
		send_to_char("Voices can't carry from there.\n\r",ch);
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_eavesdrop].beats);
	if (number_percent() < chance){
		check_improve(ch,gsn_eavesdrop,true,5);
		ch->pcdata->eavesdropping = door;
		act("You kneel down and listen intently $t.",ch,dir_name[door],NULL,TO_CHAR);
		for (rch = ch->in_room->people;rch != NULL; rch = rch->next_in_room)
			if (can_see(rch,ch))
				act("$n kneels down and listens intently $t.",ch,dir_name[door],rch,TO_VICT);
	}
	else{
		send_to_char("You fail.\n\r",ch);
		check_improve(ch,gsn_eavesdrop,false,4);
	}
	return;
}

bool check_eavesdrop(CHAR_DATA *ch){
	int chance = get_skill(ch,gsn_eavesdrop) * .7;

	if (chance < 1)
		return false;

	if (ch->race == race_lookup("daurin"))
		chance *= 1.2;

	if (number_percent() < chance){
		check_improve(ch,gsn_eavesdrop,true,2);
		return true;
	}
	check_improve(ch,gsn_eavesdrop,false,3);
	return false;
}

void do_conceal(CHAR_DATA *ch,char *argument ){
	char arg [MAX_INPUT_LENGTH];
	AFFECT_DATA af;
	OBJ_DATA *obj;
	int chance;

	argument = one_argument(argument, arg);

	if (arg[0] == '\0'){
		send_to_char("Conceal what?\n\r",ch);
		return;
	}

	if ((obj = get_obj_wear(ch,arg,true)) == NULL){
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

	if (IS_OBJ_STAT(obj,ITM_HIDDEN)){
		act("$p is already concealed.",ch,obj,NULL,TO_CHAR);
		return;
	}
	chance = get_skill(ch,gsn_conceal) * .85;

	WAIT_STATE(ch,skill_table[gsn_conceal].beats);

	if (number_percent( ) >= chance){
		send_to_char("You fail to conceal your weapons.\n\r",ch);
		check_improve(ch,gsn_conceal,false,1);
	}
	else{
		af.where		= TO_OBJECT;
		af.type			= gsn_conceal;
		af.level		= ch->level;
		af.duration		= ch->level/2;
		af.location		= APPLY_NONE;
		af.modifier		= 0;
		af.bitvector	= ITM_HIDDEN;
		affect_to_obj(obj,&af);
		act("You quietly conceal $p.",ch,obj,NULL,TO_CHAR);
		check_improve(ch,gsn_conceal,true,1);
	}
}

bool check_alertness(CHAR_DATA *ch){
	if (get_skill(ch,gsn_alertness) < 1 || !ch->isaff(AF_ALERTNESS))
		return false;

	if(number_percent() < get_skill(ch,gsn_alertness) * .8){
		check_improve(ch,gsn_alertness,true,1);
		return true;
	}
	check_improve(ch,gsn_alertness,false,1);
	return false;
}

void do_alertness(CHAR_DATA *ch, char *argument){
    AFFECT_DATA af;

	if (ch->isaff(AF_ALERTNESS)){
		send_to_char("You are already alert to your surroundings.\n\r",ch);
		return;
	}

	if(number_percent() <= get_skill(ch,gsn_alertness) *.9){
		affect_set(ch,TO_AFFECTS,gsn_alertness,ch->level,ch->getslvl(gsn_alertness),ch->level/10,APPLY_WIS,4,AF_ALERTNESS);
		act( "$n seems more alert...", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You become more alert of your surroundings.\n\r", ch);
		check_improve(ch,gsn_alertness,true,4);
	}
	else
	{
		send_to_char( "You fail.\n\r", ch);
		check_improve(ch,gsn_alertness,false,2);
	}
	WAIT_STATE(ch,skill_table[gsn_alertness].beats);
}

int check_heightened_senses(CHAR_DATA *ch,CHAR_DATA *victim){
	int sn = gsn_heightened_senses,skill = get_skill(ch,sn);

	if(skill < 1)
		return 0;
	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return number_range(ch->getslvl(sn),ch->getslvl(sn)*3);
	}
	else
		check_improve(ch,sn,false,1);
	return 0;
}

void do_sense(CHAR_DATA *ch,char *argument){
    AFFECT_DATA af;
	int sn = gsn_heightened_senses,skill = get_skill(ch,sn);

	if (ch->isaff(AF_SENSES)){
		send_to_char("You are already sensing your surroundings.\n\r",ch);
		return;
	}

	if(roll_chance(ch,skill)){
		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),UMAX(1,ch->getslvl(sn)) * 5,APPLY_WIS,ch->getslvl(sn),AF_SENSES);
		act("$n squints $s eyes...",ch,NULL,NULL,TO_ROOM);
		ch->send("You become more aware of the world around you.\n\r");
		check_improve(ch,sn,true,4);
	}
	else
	{
		send_to_char( "You fail.\n\r", ch);
		check_improve(ch,sn,false,2);
	}
	WAIT_STATE(ch,skill_table[sn].beats);
}

int check_ambush(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	int sn = gsn_ambush,skill = get_skill(ch,sn);

	if(skill < 1 || can_see(victim,ch))
		return dam;

	skill /= 2;
	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		act("You ambush $N!",ch,NULL,victim,TO_CHAR);
		act("$n ambushes you!",ch,NULL,victim,TO_VICT);
		act("$n ambushes $N!",ch,NULL,victim,TO_NOTVICT);
		return dam + dam / 6 * ch->getslvl(sn);
	}
	check_improve(ch,sn,false,1);
	return dam;
}

void do_ambush(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int sn = gsn_ambush,skill = get_skill(ch,sn) / 2;

	if(!IS_NPC(ch) && get_skill(ch,gsn_ambush) < 1){
		send_to_char("You don't know how to do that!\n\r",ch);
		return;
	}

	if (!argument[0]){
		send_to_char("Ambush who?\n\r",ch);
		return;
	}

	if (ch->fighting){
		send_to_char("You're facing the wrong end.\n\r",ch);
		return;
	}
	if(skill < 1){
		ch->send("You're not very sneaky.\n\r");
		return;
	}

	if(!(victim = get_char_room(ch,NULL,argument))){
		send_to_char("They're not here.\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	if(can_see(victim,ch)){
		act("$N can see you... you can't sneak up.",ch,NULL,victim,TO_CHAR);
		return;
	}
	act("You rush $N in an ambush!",ch,NULL,victim,TO_CHAR);
	act("$n rushes you in an ambush!",ch,NULL,victim,TO_VICT);
	act("$n rushes $N in an ambush!",ch,NULL,victim,TO_NOTVICT);

	if(roll_chance(ch,skill)){
		act("Your ambush disorients $N!",ch,NULL,victim,TO_CHAR);
		act("$n's ambush leaves you disoriented!",ch,NULL,victim,TO_VICT);
		act("$n's ambush leaves $N looking disoriented and confused!",ch,NULL,victim,TO_NOTVICT);
		WAIT_STATE(victim,skill_table[sn].beats/2);
		DAZE_STATE(victim,skill_table[sn].beats);
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);

	skill_damage(ch,victim,check_ambush(ch,victim,75 + 10 * ch->getslvl(sn)),sn,DAM_PIERCE,false);
}
