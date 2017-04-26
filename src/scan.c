#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

char *const distance[9]={
	"right here.",
	"just to the %s.",
	"nearby to the %s.",
	"not far %s.",
	"off to the %s.",
	"further %s.",
	"a long ways %s.",
	"far away %s.",
	"far in the distance %s."
};

void scan_list	( ROOM_INDEX_DATA*,CHAR_DATA*,sh_int,sh_int );
void scan_char	( CHAR_DATA*,CHAR_DATA*,sh_int,sh_int );


bool check_hawkeye(CHAR_DATA *ch){
	int sn = gsn_stalk,skill = get_skill(ch,sn) * .75;

	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}

void do_scan(CHAR_DATA *ch,char *argument){
	extern char *const dir_name[];
	char arg1[MIL],buf[MIL];
	ROOM_INDEX_DATA *scan_room;
	EXIT_DATA *pExit;
	int door,depth,dmax;

	argument = one_argument(argument,arg1);

	if(arg1[0] == '\0'){
		act("$n looks all around.",ch,NULL,NULL,TO_ROOM);
		send_to_char("Looking around you see:\n\r",ch);
		scan_list(ch->in_room,ch,0,-1);

		dmax = 1;
		if(check_hawkeye(ch))
			dmax = 3;

		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d\n\r",dmax);
		for (door=0; door < 6; door++){
			scan_room = ch->in_room;
			for (depth = 1; depth < dmax; depth++){
				if ((pExit = scan_room->exit[door]) != NULL){
					scan_room = pExit->u1.to_room;
					scan_list(pExit->u1.to_room,ch,depth,door);
				}
			}
		}
		return;
	}
	else if (!str_prefix(arg1,"north")) door = 0;
	else if (!str_prefix(arg1,"east") ) door = 1;
	else if (!str_prefix(arg1,"south")) door = 2;
	else if (!str_prefix(arg1,"west") ) door = 3;
	else if (!str_prefix(arg1,"up")   ) door = 4;
	else if (!str_prefix(arg1,"down") ) door = 5;
	else { ch->send("Which way do you want to scan?\n\r"); return; }

	act("You peer intently $T.",ch,NULL,dir_name[door],TO_CHAR);
	act("$n peers intently $T.",ch,NULL,dir_name[door],TO_ROOM);
	sprintf(buf,"Looking %s you see:\n\r",dir_name[door]);

	scan_room = ch->in_room;
	dmax = 4;
	if(check_hawkeye(ch))
		dmax = 4 + ch->getslvl(gsn_stalk);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d\n\r",dmax);
	for (depth = 1; depth < 4; depth++){
		if ((pExit = scan_room->exit[door]) != NULL){
			scan_room = pExit->u1.to_room;
			scan_list(pExit->u1.to_room, ch, depth, door);
		}
	}
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, sh_int depth, sh_int door){
	CHAR_DATA *rch;

	if (scan_room == NULL) return;
	for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room){
		if (rch == ch)
			continue;
		if (!IS_NPC(rch) && rch->invis_level > get_trust(ch))
			continue;
		if (can_see(ch, rch))
			scan_char(rch, ch, depth, door);
	}
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, sh_int depth, sh_int door){
	extern char *const dir_name[];
	extern char *const distance[];
	char buf[MIL],buf2[MIL];

	buf[0] = '\0';

	strcat(buf,PERS(victim,ch));
	strcat(buf,", ");
	sprintf(buf2,distance[depth],dir_name[door]);
	strcat(buf,buf2);
	strcat(buf,"\n\r");

	send_to_char(buf,ch);
}
