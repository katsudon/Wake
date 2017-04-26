#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

//Local!
AREA_DATA *get_area_data	( int );


void end_duel(CHAR_DATA *ch,CHAR_DATA *victim){
	CHAR_DATA *vh;
	ROOM_INDEX_DATA *pRoom;
	if(!(vh = victim->challenge)){
		send_to_char("Your opponent is gone...?\n\r",victim);
		pRoom = get_room_index(victim->foyervnum);
		char_from_room(victim);
		char_to_room(victim,pRoom);
		return;
	}
	victim->position = POS_STANDING;
	victim->challenge = NULL;
	victim->remplr(PL_ARENA);
	victim->hit = UMIN(victim->shp,victim->max_hit);
	victim->setmana(UMIN(victim->smp,victim->getmaxmana()));
	victim->move = UMIN(victim->smv,victim->max_move);
	victim->hit = victim->max_hit;
	stop_fighting(victim,true);
	char_from_room(victim);
	pRoom = get_room_index(victim->foyervnum);
	char_to_room(victim,pRoom);
	send_to_char("You have been defeated!\n\r",victim);
	while (victim->affected){
		affect_remove(victim,victim->affected);
		victim->massaff(race_table[victim->race].aff);
	}
	act("$n arrives, $s head hung low in defeat.",victim,NULL,NULL,TO_ROOM);

	vh->position = POS_STANDING;
	vh->challenge = NULL;
	vh->remplr(PL_ARENA);
	vh->hit = UMIN(vh->shp,vh->max_hit);
	vh->setmana(UMIN(vh->smp,vh->getmaxmana()));
	vh->move = UMIN(vh->smv,vh->max_move);
	vh->hit = vh->max_hit;
	stop_fighting(vh,true);
	char_from_room(vh);
	pRoom = get_room_index(get_room_index(vh->foyervnum)->arenaviewvn);
	char_to_room(vh,pRoom);
	send_to_char("VICTORY!\n\r",vh);
	while (vh->affected){
		affect_remove(vh,vh->affected);
		vh->massaff(race_table[vh->race].aff);
	}
	act("$n arrives, $s fist held high in victory.",vh,NULL,NULL,TO_ROOM);
}

void duel_commence(CHAR_DATA *ch,CHAR_DATA *victim,ROOM_INDEX_DATA *pRoom){
	ch->send("You commence your duel.\n\r");
	victim->send("You commence your duel.\n\r");
	act("$n and $N commence their duel.\n\r",ch,NULL,victim,TO_ROOM);

	while (victim->affected){
		affect_remove(victim,victim->affected);
		victim->massaff(race_table[victim->race].aff);
	}
	victim->shp = victim->hit;
	victim->smp = victim->getmana();
	victim->smv = victim->move;
	victim->hit = victim->max_hit;
	victim->setmana(victim->getmaxmana());
	victim->move = victim->max_move;
	victim->foyervnum = victim->in_room->vnum;
	victim->setplr(PL_ARENA);
	char_from_room(victim);
	char_to_room(victim,pRoom);
	pRoom = get_random_room_area(victim);
	char_from_room(victim);
	char_to_room(victim,pRoom);
	do_function(victim,&do_look,"");

	while (ch->affected){
		affect_remove(ch,ch->affected);
		ch->massaff(race_table[ch->race].aff);
	}
	ch->shp = ch->hit;
	ch->smp = ch->getmana();
	ch->smv = ch->move;
	ch->hit = ch->max_hit;
	ch->setmana(ch->getmaxmana());
	ch->move = ch->max_move;
	ch->foyervnum = ch->in_room->vnum;
	ch->setplr(PL_ARENA);
	char_from_room(ch);
	char_to_room(ch,pRoom);
	pRoom = get_random_room_area(ch);
	char_from_room(ch);
	char_to_room(ch,pRoom);
	do_function(ch,&do_look,"");
}

void do_duel(CHAR_DATA *ch, char *argument){
	CHAR_DATA *victim;
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	int i;
	bool found=false;
	char arg[MSL],arg1[MSL];

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg1);

	if(arg1[0] == '\0' || ((!str_prefix(arg,"withdraw") || !str_prefix(arg,"decline")) && arg1[0] == '\0')){
		send_to_char("Syntax: duel <challenge/withdraw/accept/decline> <victim>.\n\r",ch);
		return;
	}

	if((victim = get_char_world(ch,arg1)) == NULL){
		send_to_char("Who? Where?\n\r",ch);
		return;
	}

	if(IS_NPC(victim))
		return;

	if(victim == NULL || !can_see(ch,victim)){
		send_to_char("They're not even around..\n\r",ch);
		return;
	}
	if(ch == victim){
		send_to_char("Not yourself!\n\r",ch);
		return;
	}
	if (ch->isplr(PL_ARENA)){
		send_to_char("You're in a duel!\n\r",ch);
		return;
	}

	if(victim->isplr(PL_ARENA)){
		send_to_char("They're in a duel!\n\r",ch);
		return;
	}

	if(ch->fighting != NULL || victim->fighting != NULL){
		send_to_char("Neither of you may be in combat.\n\r",ch);
		send_to_char("Neither of you may be in combat.\n\r",victim);
		return;
	}

	if(!IS_SET(ch->in_room->room_flags,ROOM_ARENA_FOYER)){
		send_to_char("You must be in an arena foyer to duel.\n\r",ch);
		return;
	}
	if(victim->in_room != ch->in_room){
		send_to_char("Drag them to the arena, cuz they aren't here!\n\r",ch);
		send_to_char("You must be in the arena foyer with your sparring buddy.\n\r",victim);
		return;
	}
	if(ch->in_room->arenavn == 0 || ch->in_room->arenaviewvn == 0 || (pArea = get_area_data(ch->in_room->arenavn)) == NULL){
		send_to_char("Some idiot builder didn't make you an arena to fight in. Tell an admin!\n\r",ch);
		send_to_char("Some idiot builder didn't make you an arena to fight in. Tell an admin!\n\r",victim);
		return;
	}
	if(!str_prefix(arg,"decline") || !str_prefix(arg,"withdraw")){
		ch->remplr(PL_ARENA);
		victim->remplr(PL_ARENA);
		ch->challenge = NULL;
		victim->challenge = NULL;
	}
	if(!str_prefix(arg,"challenge")){
		act("You have challenged $N to a duel!",ch,NULL,victim,TO_CHAR);
		act("$n has challenged $N to a duel!",ch,NULL,victim,TO_NOTVICT);
		act("You have been challenged by $n to a duel!",ch,NULL,victim,TO_VICT);
		ch->challenge = victim;
		victim->challenge = ch;
	}
	if(!str_prefix(arg,"accept")){
		if(ch->challenge != victim || victim->challenge != ch){
			send_to_char("You are not set to duel with them.\n\r",ch);
			return;
		}
		for(i = pArea->min_vnum;i < pArea->max_vnum;i++){
			if((pRoom = get_room_index(i)) != NULL){
				found = true;
				break;
			}
		}
		if(!found){
			send_to_char("Some idiot builder didn't make you an arena to fight in. Tell an admin!\n\r",ch);
			send_to_char("Some idiot builder didn't make you an arena to fight in. Tell an admin!\n\r",victim);
			return;
		}
		duel_commence(ch,victim,pRoom);
	}
}
