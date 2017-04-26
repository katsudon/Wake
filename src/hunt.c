/*
    You can do a lot of neat things... Make your mobs walk to work or something.
    By Palrich
    Head Coder @ Divine Blood
    divineblood.tetradesign.com:4000
*/
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"

#define PATH_MAX_VNUM 32768
#define PATH_MAX_DIR  6
// #define BITS_PER_INT (sizeof(int)*8)
#define BITS_PER_INT             32
#define PATH_IS_FLAG(flag, bit)  ((unsigned)flag[bit/BITS_PER_INT]>>bit%BITS_PER_INT&01)
#define PATH_SET_FLAG(flag, bit) (flag[bit/BITS_PER_INT] |= 1 << bit%BITS_PER_INT)
bool mask_trek(CHAR_DATA *ch);
int find_path(ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth);
extern char * const dir_name[];

void do_track(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int i,sn = gsn_hunt,skill = get_skill(ch,sn);

	if (skill < 1 || IS_NPC(ch)){
		ch->send("You do not know how to track.\n\r");
		return;
	}

	if(!(victim = get_char_world(ch,argument))){
		ch->send("No one around by that name.\n\r");
		return;
	}

	if(ch->in_room == victim->in_room){
		act("$N is here!",ch,NULL,victim,TO_CHAR);
		return;
	}

	act("$n carefully checks the ground.",ch,NULL,NULL,TO_ROOM);
	WAIT_STATE(ch,skill_table[sn].beats);

	// What I do is use the player's track skill and level
	// to get the maximum length path track will find.
	// modify this however you want your track to work...

	skill /= 4;
	skill += ch->getslvl(sn)*5;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d\n\r",skill);

	if(roll_chance(ch,skill)){
		if(ch->mark && ch->mark == victim)
			skill *= 2;
		if(get_skill(ch,sn) > 1)
			skill += ch->getslvl(sn) * 2;
		i = find_path(ch->in_room,victim->in_room,skill);

		if(mask_trek(victim))
			i = number_range(0,5);
		if (i == -1)
			printf_to_char(ch,"%s is too far away for you to track.\n\r",PERS(victim,ch));
		else
			printf_to_char(ch,"%s is %s from here.\n\r",PERS(victim,ch),dir_name[i]);
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("They are... um.... THAT WAY!\n\r");
		check_improve(ch,sn,false,1);
	}
}

int find_path(ROOM_INDEX_DATA *from,ROOM_INDEX_DATA *to,int max_depth){
	int bitvector[PATH_MAX_VNUM/BITS_PER_INT];
	ROOM_INDEX_DATA *rlist;
	ROOM_INDEX_DATA *track_room_list;
	int i, depth;

	bzero(bitvector, sizeof(bitvector));
	PATH_SET_FLAG(bitvector, from->vnum);
	track_room_list = from;
	track_room_list->next_track = NULL;
	for (depth = 0; depth < max_depth; depth++){
		rlist = track_room_list;
		track_room_list = NULL;
		for (; rlist; rlist = rlist->next_track){
			for (i = 0; i < PATH_MAX_DIR; i++){
				if (!rlist->exit[i] || !rlist->exit[i]->u1.to_room ||PATH_IS_FLAG(bitvector, rlist->exit[i]->u1.to_room->vnum))
					continue;
				PATH_SET_FLAG(bitvector, rlist->exit[i]->u1.to_room->vnum);
				rlist->exit[i]->u1.to_room->track_came_from = rlist;
				if (rlist->exit[i]->u1.to_room == to){
					if (rlist == from)
						return i;
//  if you need access to the entire path, this is the place to get it.
//basically it's back-tracking how it got to the destination. Also a good place to hinder track based on sector, weather, etc.
					while (rlist->track_came_from != from)
						rlist = rlist->track_came_from;
					for (i = 0; i < PATH_MAX_DIR; i++)
						if (from->exit[i] && from->exit[i]->u1.to_room == rlist)
							return i;
					return -1;
				}
				else{
					rlist->exit[i]->u1.to_room->next_track = track_room_list;
					track_room_list = rlist->exit[i]->u1.to_room;
				}
			}
		}
	}
	return -1;
}
