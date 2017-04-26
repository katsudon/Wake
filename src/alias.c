#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

void reset_skills(CHAR_DATA*);


/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d,char *argument){
	CHAR_DATA *ch;
	char buf[MSL],prefix[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
	char *point;
	int alias;

	ch = d->original ? d->original : d->character;

	/* check for prefix */
	if (ch->prefix[0] != '\0' && str_prefix("prefix",argument)){
		if (strlen_color(ch->prefix) + strlen_color(argument) > MAX_INPUT_LENGTH)
			send_to_char("Line to long, prefix not processed.\r\n",ch);
		else{
			sprintf(prefix,"%s %s",ch->prefix,argument);
			argument = prefix;
		}
	}

	if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL || !str_prefix("alias",argument) || !str_prefix("unalias",argument) || !str_prefix("prefix",argument)) {
		interpret(d->character,argument);
		return;
	}

	strcpy(buf,argument);

	for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */{
		if (ch->pcdata->alias[alias] == NULL)
			break;

		if (!str_prefix(ch->pcdata->alias[alias],argument)){
			point = one_argument(argument,name);
			if (!strcmp(ch->pcdata->alias[alias],name)){
				buf[0] = '\0';
				strcat(buf,ch->pcdata->alias_sub[alias]);
				strcat(buf," ");
				strcat(buf,point);

				if (strlen_color(buf) > MAX_INPUT_LENGTH - 1)
				{
					send_to_char("Alias substitution too long. Truncated.\r\n",ch);
					buf[MAX_INPUT_LENGTH -1] = '\0';
				}
				break;
			}
		}
	}
	interpret(d->character,buf);
}

void do_alia(CHAR_DATA *ch,char *argument){
    send_to_char("I'm sorry, alias must be entered in full.\n\r",ch);
    return;
}

void do_alias(CHAR_DATA *ch,char *argument){
	CHAR_DATA *rch;
	char arg[MIL];
	int pos;

	smash_tilde(argument);

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument,arg);


	if (arg[0] == '\0'){
		if (rch->pcdata->alias[0] == NULL){
			send_to_char("You have no aliases defined.\n\r",ch);
			return;
		}
		send_to_char("Your current aliases are:\n\r",ch);

		for (pos = 0; pos < MAX_ALIAS; pos++){
			if (rch->pcdata->alias[pos] == NULL || rch->pcdata->alias_sub[pos] == NULL)
				break;

			printf_to_char(ch,"    %s:  %s\n\r",rch->pcdata->alias[pos],rch->pcdata->alias_sub[pos]);
		}
		return;
	}

	if (!str_prefix("unalias",arg) || !str_cmp("alias",arg)){
		send_to_char("Sorry, that word is reserved.\n\r",ch);
		return;
	}

	if (argument[0] == '\0'){
		for (pos = 0; pos < MAX_ALIAS; pos++){
			if (rch->pcdata->alias[pos] == NULL || rch->pcdata->alias_sub[pos] == NULL)
				break;

			if (!str_cmp(arg,rch->pcdata->alias[pos])){
				printf_to_char(ch,"%s aliases to '%s'.\n\r",rch->pcdata->alias[pos],rch->pcdata->alias_sub[pos]);
				return;
			}
		}

		send_to_char("That alias is not defined.\n\r",ch);
		return;
	}

	if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix")){
		send_to_char("That shall not be done!\n\r",ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++){
		if (rch->pcdata->alias[pos] == NULL)
			break;

		if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */{
			free_string(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias_sub[pos] = str_dup(argument);
			printf_to_char(ch,"%s is now realiased to '%s'.\n\r",arg,argument);
			return;
		}
	}

	if (pos >= MAX_ALIAS){
		send_to_char("Sorry, you have reached the alias limit.\n\r",ch);
		return;
	}

	 /* make a new alias */
	 rch->pcdata->alias[pos]		= str_dup(arg);
	 rch->pcdata->alias_sub[pos]	= str_dup(argument);
	 printf_to_char(ch,"%s is now aliased to '%s'.\n\r",arg,argument);
}

void do_unalias(CHAR_DATA *ch,char *argument){
	CHAR_DATA *rch;
	char arg[MAX_INPUT_LENGTH];
	int pos;
	bool found = false;

	if (ch->desc == NULL)
		rch = ch;
	else
		rch = ch->desc->original ? ch->desc->original : ch;

	if (IS_NPC(rch))
		return;

	argument = one_argument(argument,arg);

	if (arg[0] == '\0'){
		send_to_char("Unalias what?\n\r",ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++){
		if (rch->pcdata->alias[pos] == NULL)
			break;

		if (found){
			rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
			rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
			rch->pcdata->alias[pos]		= NULL;
			rch->pcdata->alias_sub[pos]		= NULL;
			continue;
		}

		if(!strcmp(arg,rch->pcdata->alias[pos])){
			send_to_char("Alias removed.\n\r",ch);
			free_string(rch->pcdata->alias[pos]);
			free_string(rch->pcdata->alias_sub[pos]);
			rch->pcdata->alias[pos] = NULL;
			rch->pcdata->alias_sub[pos] = NULL;
			found = true;
		}
	}

	if (!found)
		send_to_char("No alias of that name to remove.\n\r",ch);
}

void do_redeem(CHAR_DATA*ch,char *argument){
	OBJ_DATA *coupon;
	char arg[MIL],arg2[MIL],arg3[MIL];
	int n = -1,cost = -1;

	if(IS_NPC(ch)){
		ch->send("You may not do that.\n\r");
		return;
	}
	if(!argument[0]){
		ch->send("Available redemption prizes:\n\r");
		ch->send("  {R1{x) (  {G5 {xcredits) Restring\n\r");
		ch->send("  {R2{x) ( {G10 {xcredits) Corpse Retrieval\n\r");
		ch->send("  {R3{x) ( {G25 {xcredits) Skill Reset (Free until level 25)\n\r");
		ch->send("  {R4{x) ( {G50 {xcredits) Home Furniture\n\r");
		ch->send("  {R5{x) ( {G75 {xcredits) Home Chest\n\r");
		ch->send("  {R6{x) ( {G75 {xcredits) Spell Up\n\r");
		ch->send("  {R7{x) ( {G75 {xcredits) Magic Potato (Spellup Item)\n\r");
		ch->send("  {R8{x) ({G100 {xcredits) Baby Drake (Floating Item)\n\r");
		ch->send("  {R9{x) ({G100 {xcredits) Giant Stuffed Hamster (Held Furniture)\n\r");
		ch->send(" {R10{x) ({G100 {xcredits) Doubles (100 ticks)\n\r");
		//ch->send(" {R11{x) ({G200 {xcredits) Apartment\n\r");
		ch->send(" {R11{x) ({G250 {xcredits) Land Extension\n\r");
		ch->send(" {R12{x) ({G500 {xcredits) Land Deed\n\r");
		ch->send("Type {Gredeem {R<#>{x to trade game credits for the corresponding prize.\n\r");
		return;
	}
	if(!is_number(argument)){
		if(!str_cmp("restring",argument)) n = 1;
		if(!str_cmp("corpse retrieval",argument)) n = 2;
		if(!str_cmp(argument,"skill reset")) n = 3;
		if(!str_cmp("home furniture",argument)) n = 4;
		if(!str_cmp("home chest",argument)) n = 5;
		if(!str_cmp("spell up",argument)) n = 6;
		if(!str_cmp("magic potato",argument)) n = 7;
		if(!str_cmp("baby drake",argument)) n = 8;
		if(!str_cmp("giant stuffed hamster",argument)) n = 9;
		if(!str_cmp("doubles",argument)) n = 10;
		if(!str_cmp("land extension",argument)) n = 11;
		if(!str_cmp("land deed",argument)) n = 12;
	}
	else if((n = atoi(argument)) < 1 || n > 12){
		ch->send("The number must be between 1 and 12, to correspond with a prize number. Type 'redeem' to view a list of prizes, their numbers and costs.\n\r");
		return;
	}
	switch(n){
		case 1:		cost =   5;break;
		case 2:		cost =  10;break;
		case 3:		cost =  0;break;//ch->level < 25 ? 0 : 25;break;
		case 4:		cost =  50;break;
		case 5:		cost =  75;break;
		case 6:		cost =  75;break;
		case 7:		cost =  75;break;
		case 8:		cost = 100;break;
		case 9:		cost = 100;break;
		case 10:	cost = 100;break;
		case 11:	cost = 250;break;
		case 12:	cost = 500;break;
	};
	if(ch->credits < cost){
		ch->send("You do not have enough Wake Points to redeem that.\n\r");
		return;
	}
	switch(n){
		case 1:		cost =   5;break;
		case 2:		cost =  10;break;
		case 3:		reset_skills(ch);break;//ch->level < 25 ? 0 : 25;break;
		case 4:		cost =  50;break;
		case 5:		cost =  75;break;
		case 6:		cost =  75;break;
		case 7:		cost =  75;break;
		case 8:		cost = 100;break;
		case 9:		cost = 100;break;
		case 10:	cost = 100;break;
		case 11:	cost = 250;break;
		case 12:	cost = 500;break;
	};
}
