/*
 * ACT_MOB.C: Trilby@Refugee MUD
 * Copyright 1997, Lord Thomas Burbridge
 *
 * Mobile memory procedures.
 *
 * This work is a derivative of ROM2.4
 * Copyright 1995, Russ Taylor.
 */

#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "merc.h"
#include "recycle.h"

MEM_DATA *get_mem_data(CHAR_DATA *ch,CHAR_DATA *target){
	MEM_DATA *remember;

	if(!ch){
		bug("get_mem_data:  NULL ch",0);
		return NULL;
	}
	if(!IS_NPC(ch)){
		log_f("get_mem_data: ch '%s' not NPC",ch->name);
		return NULL;
	}
	if(!target){
		bug("get_mem_data:  NULL target",0);
		return NULL;
	}

	for(remember = ch->memory;remember;remember = remember->next){
		if(remember->id == target->mid)
		return remember;
	}

	return NULL;
}

void mob_forget(CHAR_DATA *ch,MEM_DATA *memory){
	if(!IS_NPC(ch)){
		log_f("mob_forget:  %s not NPC",ch->name);
		return;
	}
	if(!ch){
		bug("mob_forget:  NULL ch",0);
		return;
	}

	if(!memory)
		return;

	if ( memory == ch->memory ){
		ch->memory = memory->next;
	}
    else
    {
		MEM_DATA *prev;

		for(prev = ch->memory;prev;prev = prev->next){
			if(prev->next == memory){
				prev->next = memory->next;
				break;
			}

			if(!prev){
				bug( "mob_forget:  memory not found", 0);
				return;
			}
		}
    }
    free_mem_data(memory);
}

void mob_remember(CHAR_DATA *ch,CHAR_DATA *target,int reaction,int dam){
	MEM_DATA *remember;

	if(!IS_NPC(ch)){
		log_f("mob_remember: %s not NPC",ch->name);
		return;
	}

	if(!ch){
		bug("mob_remember:  NULL ch",0);
		return;
	}

	if(!target){
		bug("mob_remember:  NULL target",0);
		return;
	}


	if(!(remember = get_mem_data(ch,target))){
		remember = new_mem_data();

		remember->next = ch->memory;
		ch->memory = remember;
	}

	remember->id = target->mid;
	remember->when = current_time;
	//printf_to_char(ch,"RM %d %d\n\r",dam,remember->dam);
	//printf_to_char(target,"RM %d %d\n\r",dam,remember->dam);
	if(dam >= 0)
		remember->dam += dam;

	SET_BIT( remember->reaction, reaction);
}

void mem_fade(CHAR_DATA*ch) /* called from char_update */{
	MEM_DATA *remember, *remember_next;

	if(!ch){
		bug("mem_fade:  NULL ch",0);
		return;
	}

	if(!IS_NPC(ch)){
		bug("mem_fade: ch not NPC", 0);
		return;
	}

	if(!ch->memory)
		return;

	for(remember = ch->memory;remember;remember = remember_next){
		remember_next = remember->next;

/*		if (IS_NPC(ch)
		&&  IS_SET(ch->off_flags, OFF_HUNT)
		&&  ch->hunting == NULL
		&&  IS_SET(remember->reaction, MEM_HOSTILE))
		ch->hunting = get_char_id( remember->mid );*/

		if(current_time - remember->when < (96 * 60))
			mob_forget(ch,remember);
	}
}
