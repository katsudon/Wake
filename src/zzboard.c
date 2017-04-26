#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

/*
   Local Functions
*/

/* gotta test things first
BGROUP_DATA bfirst;

void show_boards(CHAR_DATA *ch){
	ch->send(" ______________________________________________________________________");
	ch->send("/ ____________________________________________________________________ \\");
	for(int i = 1; bgroups[i].name;i++){
	}
	ch->send("\______________________________________________________________________/");
	ch->send("Your current board is:");
}

void page(CHAR_DATA *ch,bool isnote){//changes the page displayed in 'note'
	if(!argument[0]){
		ch->board->show();
		ch->printf("You are on page %d of the %s board.",ch->board->page,ch->board->name);//maybe ch->board is inherited, and then page could be a field of it
		return;
	}
	if(!is_number(argument)){
		ch->send("You may only enter a number for this command.\n\r");
		return;
	}
	ch->board->page(argument);//can we overload a variable and a function together?
}

void do_note(CHAR_DATA *ch,char *argument){//displays all notes on a specific board, or switches to a specific note. If no arg, jump to next unread note.
	if(!argument[0]){
		note_next(ch);
		return;
	}
	if(!str_prefix(argument,"list")){
		ch->board->show();
		return;
	}
	if(!str_prefix(argument,"catchup")){
		ch->board->catchup();
		return;
	}
	if(!str_prefix(argument,"read")){//maybe just make it so numeric argument changes to that note, and add a 'read' command?
		ch->board->read(argument);
		return;
	}
}

void do_boards(CHAR_DATA *ch,char *argument){//displays all boards and their notecounts, or changes which board the user is set to
	BOARD_DATA *board;
	if(!argument[0]){
		show_boards(ch);
		return;
	}
	if(!(board = board_lookup(ch,argument))){
		ch->send("That is not a valid board. Enter in the name or number of a board to switch to.\n\r");
		return;
	}
	ch->board = board;
	printf_to_char(ch,"Switching to page 1 of the %s board.\n\r",board->name);
}

void init_boards(){
}

struct board_group_type bgroups []={
	{ NULL },
	{ "Important" },
	{ "Community" },
	{ "House" },
	{ NULL }
};
struct board_type boards []={
	{ NULL,				0,				120,	0,		false,	false,	NULL },
//   name				bit			minlvl		group	forguilds private description
	{ "News",			BOARD_NEWS,		1,		1,		false,	false,	"Announcements, changes." },
	{ "Offenders",		BOARD_NEWS,		1,		1,		false,	false,	"Punishments and reports." },
	{ "Mud Tech",		BOARD_NEWS,		1,		1,		false,	false,	"Mud bugs and technical discussion." },
	{ "IC General",		BOARD_NEWS,		1,		2,		false,	false,	"General IC discussions, announcements." },
	{ "OOC General",	BOARD_NEWS,		1,		2,		false,	false,	"General OOC discussions, announcements." },
	{ "Guidance",		BOARD_NEWS,		1,		2,		false,	false,	"Guides, questions, and answers." },
	{ "Buy/Sell",		BOARD_NEWS,		1,		2,		false,	false,	"Trading, buying, and selling." },
	{ "Stories",		BOARD_NEWS,		1,		2,		false,	false,	"Posted RPs and stories." },
	{ "Personal",		BOARD_NEWS,		1,		2,		false,	false,	"Private messages." },
	{ "House General",	BOARD_NEWS,		1,		3,		false,	false,	"General notes for house members." },
	{ "Admin",			BOARD_NEWS,		106,	3,		false,	false,	"Notes related to administration and imming." },
	{ "Building",		BOARD_NEWS,		106,	3,		false,	false,	"Builders general board." },
	{ NULL,				0,				120,	0,		false,	false,	NULL }
};*/