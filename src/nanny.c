#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "telnet.h"
#include "interp.h"
#include "recycle.h"
#include "db.h"

/*
 * Local functions.
 */
bool wizlock;						/* Game is wizlocked		*/
bool newlock;						/* Game is newlocked		*/
int	 Pgod;							/* All new chars are gods!	*/
const char echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };

int unread_notes		( CHAR_DATA*,BOARD_DATA* );
bool check_parse_name	( char* );
bool check_parse_lname	( char* );
bool check_reconnect	( DESCRIPTOR_DATA*,char*,bool );
void write_max_con		( );
bool check_playing		( DESCRIPTOR_DATA*,char* );
void check_multiplay	( DESCRIPTOR_DATA* );
bool check_illegal_name ( char *name );
void mod_skills			( CHAR_DATA* );
void impose_job			( CHAR_DATA *,int );
void check_penumbral	( CHAR_DATA* );
void check_max_con		( );
void d_insert			( DESCRIPTOR_DATA* );
void cql_save_new_char	( CHAR_DATA* );
bool cql_load_char(DESCRIPTOR_DATA*,char*);
void cql_load_char_obj(CHAR_DATA*);

void send_block_line(CHAR_DATA *ch,bool ender,char *text){
	int n = strlen_color(text);
	if(ender)
		ch->send("{w<{d---------------------------------------------------------------------------{w>{x\n\r");
	else{
		if(text[0])
			ch->printf("{w<{d%.*s{x%s{d%.*s{w>{x\n\r",
				(75-n)/2,
				"---------------------------------------------------------------------------",
				text,(74-n)/2,
				"---------------------------------------------------------------------------");
		else
			ch->send("{w<{d---------------------------------------------------------------------------{w>{x\n\r");
	}
}

/* recursively adds a group given its number -- uses group_add */
void gn_add(CHAR_DATA *ch,int gn,bool message,bool cost){
	int i;

	ch->pcdata->group_known[gn] = true;
	for (i = 0;i < MAX_IN_GROUP;i++){
		if (group_table[gn].skills[i] == NULL)
			break;
		skillspell_add(ch,group_table[gn].skills[i],message,cost);
	}
}

/* returns a group index number given the name */
int group_lookup(const char *name){
	int gn;

	for (gn = 0;gn < MAX_GROUP && group_table[gn].name;gn++){
		if (LOWER(name[0]) == LOWER(group_table[gn].name[0]) && !str_prefix(name,group_table[gn].name))
			return gn;
	}

	return -1;
}

/* use for processing a skill or group for addition  */
void group_add(CHAR_DATA *ch,const char *name,bool message,bool cost,bool override){
	char buf[MIL];
	int sn,gn;

	if (IS_NPC(ch))
		return;

	if (!str_cmp(name,"none"))
		return;

	sprintf(buf,"%s",name);

	sn = skill_prefix(name,override);

	if (sn != -1){
		if (ch->pcdata->learned[sn] > 0){
			if (message)
				ch->send("You already have that skill.\n\r");
		}
		else if (cost && skill_table[sn].cost[ch->pclass] < 1){
			if (message)
				printf_to_char(ch,"'{R%s{x' is not available to your class..\n\r",buf);
		}
		else{
			if (message)
				printf_to_char(ch,"Adding the skill '{G%s{x'.\n\r",skill_table[sn].name);
			if (cost)
				ch->pcdata->points += skill_table[sn].cost[ch->pclass];
			ch->pcdata->unlocked[sn] = true;
			ch->pcdata->learned[sn] = 1;
		}
		return;
	}

	gn = group_lookup(name);

	if (gn != -1){
		if (ch->pcdata->group_known[gn]){
			if (message)
				ch->send("You already have that group.\n\r");
		}
		else if (cost && group_table[gn].cost[ch->pclass] < 1){
			if (message)
				ch->send("That group is not available to your class.\n\r");
		}
		else{
			if (message)
				printf_to_char(ch,"Adding the group '{R%s{x'{.\n\r",group_table[gn].name);
			if (cost)
				ch->pcdata->points += group_table[gn].cost[ch->pclass];
			ch->pcdata->group_known[gn] = true;
			gn_add(ch,gn,true,false);
		}
		return;
	}
	printf_to_char(ch,"'{R%s{x' is not a valid skill or group..\n\r",buf);
}

void nanny(DESCRIPTOR_DATA *d,char *argument){
    DESCRIPTOR_DATA *d_old, *d_next;
    CHAR_DATA *ch;
    char buf[MSL],arg[MIL],*pwdnew,*p;
    int iClass,race,i,cols;
	bool hairChoice,eyeChoice,heightChoice,weightChoice,fOld;

    /* Delete leading spaces UNLESS character is writing a note */
    if (d->connected != CON_NOTE_TEXT)
        while (isspace(*argument))
            argument++;

    ch = d->character;

	switch (d->connected){
		default:
			bug("Nanny: bad d->connected %d.",d->connected);
			close_socket(d);
			return;
		case CON_GET_NAME:
			if (!argument[0]){
				close_socket(d);
				return;
			}
			argument[0] = UPPER(argument[0]);
			if (!check_parse_name(argument)){
				write_to_buffer(d,"That name is illegal, try another.\n\rName: ",0);
				return;
			}
			fOld = cql_load_char(d,argument);//this will only load the pcdata, we load the rest of the char after confirming password
			ch   = d->character;

			if (ch->isplr(PL_DENY)){
				sprintf(log_buf,"Denying access to %s@%s.",argument,d->host);
				log_string(log_buf);
				write_to_buffer(d,"You are denied access.\n\r",0);
				close_socket(d);
				return;
			}

			if (check_ban(d->host,BAN_PERMIT) && !ch->isplr(PL_PERMIT)){
				write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
				close_socket(d);
				return;
			}
			if (check_reconnect(d,argument,false))
				fOld = true;
			else{
				if (wizlock && !IS_IMMORTAL(ch)) {
					write_to_buffer(d,"The game is wizlocked.\n\r",0);
					close_socket(d);
					return;
				}
			}
			if (fOld){
				/* Old player */
				write_to_buffer(d,"Enter your password:\n\r",0);
				write_to_buffer(d,echo_off_str,0);
				d->connected = CON_GET_OLD_PASSWORD;
				return;
			}
			else{
				/* New player */
 				if (newlock){
					write_to_buffer(d,"The game is temporarily locking out new players.\n\r",0);
					close_socket(d);
					return;
				}
				if (check_ban(d->host,BAN_NEWBIES)){
					write_to_buffer(d,"New players are not allowed from your site.\n\r",0);
					close_socket(d);
					return;
				}
				sprintf(buf,"Your mind is a bit blurry, are you sure it's %s? ({GYes{x/{RNo{x)\n\r",argument);
				send_to_desc(buf,d);
				d->connected = CON_CONFIRM_NEW_NAME;
				return;
			}
			break;
		case CON_GET_OLD_PASSWORD:
			if (strcmp(crypt(argument,ch->pcdata->pwd),ch->pcdata->pwd)){
				write_to_buffer(d,"Wrong password.\n\r",0);
				if (ch->level >= LEVEL_IMMORTAL){
					log_f("%s@%s bad password.",ch->name,d->host);
					sprintf(log_buf,"Bad password: %s @ %s.",ch->name,d->host);
					wiznet("Bad alert! Immortal accessed using an incorrect password!",ch,NULL,WZ_LINKS,WZ_SECURE,0);
					wiznet(log_buf,NULL,NULL,WZ_SITES,0,get_trust(ch));
				}
				close_socket(d);
				return;
			}
			write_to_buffer(d,echo_on_str,0);
			if (check_playing(d,ch->name))
				return;
			if (check_reconnect(d,ch->name,true))
				return;
			//Now we load the char
			cql_load_char_obj(ch);
			sprintf(log_buf,"%s @ %s has connected.",ch->name,d->host);
			log_string(log_buf);
			wiznet(log_buf,NULL,NULL,WZ_SITES,0,get_trust(ch));
			if (ch->desc->ansi)
				ch->setplr(PL_COLOR);
			else
				ch->remplr(PL_COLOR);
			if (IS_IMMORTAL(ch)){
				do_function(ch,&do_help,"imotd");
				d->connected = CON_READ_IMOTD;
 			}
			else{
				do_function(ch,&do_help,"motd");
				d->connected = CON_READ_MOTD;
			}
			break;
		case CON_BREAK_CONNECT:
			switch(*argument){
				case 'y':
				case 'Y':
					for (d_old = descriptor_list;d_old;d_old = d_next){
						d_next = d_old->next;
						if (d_old == d || !d_old->character)
							continue;

						if (str_cmp(ch->name,d_old->original ? d_old->original->name : d_old->character->name))
							continue;
						close_socket(d_old);
					}
					if (check_reconnect(d,ch->name,true))
	    				return;
					write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
					if (d->character){
						free_char(d->character);
						d->character = NULL;
					}
					d->connected = CON_GET_NAME;
					break;
				case 'n':
				case 'N':
					write_to_buffer(d,"Name: ",0);
					if (d->character){
						free_char(d->character);
						d->character = NULL;
					}
					d->connected = CON_GET_NAME;
					break;
				default:
					send_to_desc("    Please enter {cY{xes or {cN{xo",d);
					break;
			}
			break;
		case CON_CONFIRM_NEW_NAME:
			switch (*argument){
				case 'y':
				case 'Y':
					if (ch->desc->ansi)
						ch->setplr(PL_COLOR);
					send_block_line(ch,false,"{w[{RNew Password{w]");
					ch->send("\n\r    Welcome to Wake, new adventurer! Please think of a good password:\n\r\n\r");
					d->connected = CON_GET_NEW_PASSWORD;
					break;
				case 'n':
				case 'N':
					ch->send("    That's not right... what was it then?\n\r");
					free_char(d->character);
					d->character = NULL;
					d->connected = CON_GET_NAME;
					break;
				default:
					ch->send("    Please enter {cYes or {cNo.\n\r");
					break;
			}
			break;
		case CON_GET_NEW_PASSWORD:
			if (strlen_color(argument) < 5){
				ch->send("    Your password must be at least five characters long.\n\r    Password:\n\r");
				return;
			}
			pwdnew = crypt(argument,ch->name);
			for (p = pwdnew;*p;p++){
				if (*p == '~' || *p == '\''){
					ch->send("    That password is not acceptable, try another:\n\r");
					return;
				}
			}
			free_string(ch->pcdata->pwd);
			ch->pcdata->pwd	= str_dup(pwdnew);
			ch->send("Re-enter your password to be sure:\n\r");
			d->connected = CON_CONFIRM_NEW_PASSWORD;
			break;
		case CON_CONFIRM_NEW_PASSWORD:
			if (strcmp(crypt(argument,ch->pcdata->pwd),ch->pcdata->pwd)){
				ch->send("Your passwords don't match.\n\rRe-enter your password:\n\r");
				d->connected = CON_GET_NEW_PASSWORD;
				return;
			}
			write_to_buffer(d,echo_on_str,0);
			send_block_line(ch,false,"{w[{RLast Name{w]");
			ch->send("\n\r    Please enter your last name, enter 'none' for no last name:\n\r");
			d->connected = CON_GET_LNAME;
			break;
		case CON_GET_LNAME:
			if (!argument[0]){
				ch->send("What was your last name again..? (Enter the last name again or type 'none').");
				break;
			}
			if (!str_cmp(argument,"none")){
				ch->pcdata->lname = str_dup("{x");
				ch->send("Okay... You will have no last name. Are you sure? ({GYes{x/{RNo{x)\n\r");
			}
			else{
				if (!check_parse_lname(argument)){
					ch->send("That name is illegal, try another last name:\n\r");
					return;
				}
				char buffing[MSL];

				argument[0] = UPPER(argument[0]);
				sprintf(buf,"A last name of '%s', is that correct? ({GYes{x/{RNo{x)\n\r",argument);
				ch->send(buf);
				sprintf(buffing,"%s",argument);
				ch->pcdata->lname = str_dup(buffing);
			}
			d->connected = CON_CONFIRM_LNAME;
			break;
		case CON_CONFIRM_LNAME:
			switch (argument[0]){
				case 'y':
				case 'Y':
					break;
				case 'n':
				case 'N':
					ch->send("What do you want then?\n\r");
					d->connected = CON_GET_LNAME;
					return;
				default:
					ch->send("Please type {cY{xes or {cN{xo\n\r");
					d->connected = CON_CONFIRM_LNAME;
					return;
			}
			send_block_line(ch,false,"{w[{RRace Selection{w]");
			ch->send("\n\r    You can be one of the following races:\n\r\n\r");
			for(int col = 0, race = 1; race_table[race].name != NULL; ++race){
				if (!race_table[race].pc_race)
					break;
				++col;
				sprintf(buf, "{d[ {g%s {d]{x", race_table[race].name);
				printf_to_char(ch,"        %15s%s",buf,!(col % 4) ? "\r\n" : "");
			}
			ch->send("\n\r\n\r    Which race do you wish to be? (type '{GHELP <Race>{x' for more information)\n\r");
			d->connected = CON_GET_NEW_RACE;
			break;
		case CON_GET_NEW_RACE:
			one_argument(argument,arg);
			if (!strcmp(arg,"help")){
				argument = one_argument(argument,arg);
				if (!argument[0])
					do_function(ch, &do_help, "race help");
				else
					do_function(ch, &do_help, argument);
				ch->send("What is your race (help for more information)?\n\r");
				break;
  			}
			race = race_lookup(argument);
			if (race == 0 || !race_table[race].pc_race){
				ch->send("That is not a valid race.\n\r");
				ch->send("The following races are available:\n\r\n\r");
				for ( int col = 0, race = 1; race_table[race].name != NULL; ++race){
					if (!race_table[race].pc_race)
						break;
					++col;
					sprintf(buf, "{d[{g%10s{d]{x", race_table[race].name);
					printf_to_char(ch,"   %15s%s",buf,!(col % 4) ? "\r\n" : "");
				}
				ch->send("\n\r");
				ch->send("What is your race? (help for more information)\n\r");
				break;
			}
			ch->race = race;
			/* initialize stats */
			for (i = 0; i < MAX_STATS; i++)
				ch->perm_stat[i] = pc_race_table[race].stats[i];
			init_race(ch);
			ch->perm_stat[STAT_LCK] += number_range(0,4);//NashBeta
			ch->perm_stat[STAT_CHA] += number_range(0,3);//NashBeta

			/* add skills */
			for (i = 0; i < 5; i++){
				if(!pc_race_table[race].skills[i])
	 				break;
				if(str_cmp(pc_race_table[race].skills[i],"none")){
					group_add(ch,pc_race_table[race].skills[i],true,false,true);
					ch->pcdata->up_skill(5,100,skill_lookup(pc_race_table[race].skills[i]));
				}
			}
			/* add sub skills  **removed because it's useless*/
			/* add cost */
			ch->size = pc_race_table[race].size;
			send_block_line(ch,false,"{w[{RGender Selection{w]");
			ch->send("\n\r    What will your character's gender be? ({RM{xale/{RF{xemale)\n\r");
			d->connected = CON_GET_NEW_SEX;
			break;
		case CON_GET_NEW_SEX:
			switch (argument[0]){
				case 'm':
				case 'M':
					ch->sex = ch->pcdata->true_sex = SEX_MALE;
					break;
				case 'f':
				case 'F':
					ch->sex = ch->pcdata->true_sex = SEX_FEMALE;
					break;
				default:
					ch->send("That's not a gender.\n\rPlease retry: ({RM{xale/{RF{xemale)\n\r");
					return;
			}

			ch->pclass = CLASS_TENDERFOOT;

			log_f("%s@%s new player.",ch->name,d->host);
			sprintf(log_buf,"%s",d->host);
			wiznet("Newbie alert! '{R$N{x' is attempting to breach our defenses!",ch,NULL,WZ_NEWBIE,0,0);
			wiznet(log_buf,NULL,NULL,WZ_SITES,0,get_trust(ch));
			write_to_buffer(d,echo_on_str,0);
			d->connected = CON_GET_HAIR;
			send_block_line(ch,false,"{w[{RPhysical Appearance{w]");
			ch->send("\n\r    Select your character's hair color:\n\r\n\r");
			for(int col = 0,i=1;hair_table[i].name;++i,col){
				if(hair_table[i].races[ch->race]){
					sprintf(buf," {d[{w%18s{d]{x",hair_table[i].name);
					printf_to_char(ch, "   %s", buf);
					if (!(++col % 3))
						ch->send("\n\r");
				}
			}
			ch->send("\n\r\n\r{xMake a selection\n\r");
			d->connected = CON_GET_HAIR;
			break;
		case CON_GET_HAIR:
			hairChoice = false;
			for(i=1;hair_table[i].name;i++)
				if (!str_prefix(argument,hair_table[i].name) && hair_table[i].races[ch->race]){
					ch->looks[P_HAIR] = hair_table[i].flag;
					hairChoice = true;
				}
			if(!hairChoice){
				ch->send("No. Valid choices are:\n\r\n\r");
				for(int col = 0,i = 1;hair_table[i].name;++i,col){
					if(hair_table[i].races[ch->race]){
						sprintf(buf," {d[{w%18s{d]{x",hair_table[i].name);
						printf_to_char(ch, "   %s",buf);
						if (!(++col % 3))
							ch->send("\n\r");
					}
				}
				ch->send("\n\r\n\r{xMake a selection\n\r");
				break;
			}
			ch->send("    Choose a hair style for your character:\n\r\n\r");
			for(int col = 0, i=1; sub_hair_table[i].name != NULL; ++i, col){
				if(sub_hair_table[i].races[ch->race]){
					sprintf(buf," {d[{w%18s{d]{x",sub_hair_table[i].name);
					printf_to_char(ch, "   %s", buf);
					if (!(++col % 3))
						ch->send("\n\r");
				}
			}
			ch->send("\n\r\n\r{xEnter your choice:\n\r");
			d->connected = CON_GET_HAIRTYPE;
			break;
		case CON_GET_HAIRTYPE:
			hairChoice = false;
			for(i=1;sub_hair_table[i].name;i++)
				if (!str_prefix(argument,sub_hair_table[i].name) && sub_hair_table[i].races[ch->race]){
					ch->looks[P_SHAIR] = sub_hair_table[i].flag;
					hairChoice = true;
				}
			if(!hairChoice){
				ch->send("No. Valid choices are:\n\r");
				for(int col = 0,i=1;sub_hair_table[i].name;++i,col){
					if(sub_hair_table[i].races[ch->race]){
						sprintf(buf," {d[{w%18s{d]{x",sub_hair_table[i].name);
						printf_to_char(ch, "   %s",buf);
						if (!(++col % 3))
							ch->send("\n\r");
					}
				}
				ch->send("\n\r\n\r{xEnter your choice:\n\r");
				break;
			}
			write_to_buffer(d,echo_on_str, 0);
			ch->send("Choose an eye color for your character:\n\r\n\r");
			for(int col = 0, i=1; eye_table[i].name != NULL; ++i, col){
				if(eye_table[i].races[ch->race]){
					sprintf(buf," {d[{w%18s{d]{x",eye_table[i].name);
					printf_to_char(ch, "   %s", buf);
					if (!(++col % 3))
						ch->send("\n\r");
				}
			}
			ch->send("\n\r\n\r{xEnter your choice:\n\r");
			d->connected = CON_GET_EYE;
			break;
		case CON_GET_EYE:
			eyeChoice = false;
			for(i=1;eye_table[i].name;i++)
				if (!str_prefix(argument,eye_table[i].name) && eye_table[i].races[ch->race]){
					ch->looks[P_EYE] = eye_table[i].flag;
					eyeChoice = true;
				}
			if(!eyeChoice){
				ch->send("No, Valid choices are:\n\r\n\r");
				for(int col = 0,i=1;eye_table[i].name;++i,col){
					if(eye_table[i].races[ch->race]){
						sprintf(buf," {d[{w%18s{d]{x",eye_table[i].name);
						printf_to_char(ch,"   %s",buf);
						if (!(++col % 3))
							ch->send("\n\r");
					}
				}
				ch->send("\n\r\n\r{xEnter your choice:\n\r");
				break;
			}
			write_to_buffer(d,echo_on_str, 0);
			ch->send("Choose a height for your character:\n\r\n\r");
				for(int col = 0, i=1; height_table[i].name != NULL; ++i, col){
					if(height_table[i].races[ch->race]){
						sprintf(buf," {c[{w%18s{c]{x",height_table[i].name);
						printf_to_char(ch, "   %s", buf);
						if (!(++col % 3))
							ch->send("\n\r");
					}
				}
			ch->send("\n\r{xWhich do you want your character to be?\n\r");
			d->connected = CON_GET_HEIGHT;
			break;
		case CON_GET_HEIGHT:
			heightChoice = false;
			for(i=1; height_table[i].name != NULL; i++){
				if (!str_prefix( argument, height_table[i].name) && height_table[i].races[ch->race]){
					ch->looks[P_HEIGHT] = height_table[i].flag;
					heightChoice = true;
				}
			}
			if(!heightChoice){
				ch->send("That is not a valid height, available choices are\n\r\n\r");
				for(int col = 0, i=1; height_table[i].name != NULL; ++i, col){
					if(height_table[i].races[ch->race]){
						sprintf(buf," {c[{w%18s{c]{x",height_table[i].name);
						printf_to_char(ch, "   %s", buf);
						if (!(++col % 3))
							ch->send("\n\r");
					}
				}
				ch->send("Please select one.\n\r");
				break;
			}
			ch->send(echo_on_str);
			ch->send("Choose a size for your character:\n\r");
				for(int col = 0, i=1; weight_table[i].name != NULL; ++i, col){
					if(weight_table[i].races[ch->race]){
						sprintf(buf," {g[{w%18s{g]{x",weight_table[i].name);
						printf_to_char(ch, "   %s", buf);
						if (!(++col % 3))
							ch->send("\n\r");
					}
				}
			ch->send("\n\r{xPlease select one for your class.\n\r");
			d->connected = CON_GET_WEIGHT;
			break;
		case CON_GET_WEIGHT:
			weightChoice = false;
			for(i=1; weight_table[i].name != NULL; i++)
				if (!str_prefix( argument,weight_table[i].name) && weight_table[i].races[ch->race]){
					ch->looks[P_WEIGHT] = weight_table[i].flag;
					weightChoice = true;
				}
			if(!weightChoice){
				ch->send("That is not a valid size, available choices are:\n\r");
				for(int col = 0, i=1; weight_table[i].name != NULL; ++i, col){
					if(weight_table[i].races[ch->race]){
						sprintf(buf," {g[{w%18s{g]{x",weight_table[i].name);
						printf_to_char(ch, "   %s", buf);
						if (!(++col % 3))
							ch->send("\n\r");
					}
				}
				ch->send("Please make a choice from this list.\n\r");
				break;
			}
			send_block_line(ch,false,"{w[{RHometown Selection{w]");
			ch->send("\n\rChoose a city for your character to have as their hometown:\n\r");
			for(i = 0; i < MAX_HOMETOWN;i++){
				if(!hometowns[i].canuse || !pc_race_table[ch->race].home_use[i])
					continue;
				printf_to_char(ch," -%s\n\r",hometowns[i].name);
			}
			d->connected = CON_GET_HOMETOWN;
			break;
		case CON_GET_HOMETOWN:
			weightChoice = false;
			if((i = home_lookup(argument)) == -1 || !pc_race_table[ch->race].home_use[i]){
				//ch->send("\n\rInvalid choice.\n\r");
			}
			else{
				ch->hometown = i;
				log_f("%d, %d",i,hometowns[i].guild);
				ch->guild = hometowns[i].guild;
				printf_to_char(ch,"Setting guild to %s\n\r",guilds[hometowns[i].guild].name);
				if(ch->guild == -1)
					log_f("Stupid guild %s",guilds[hometowns[i].guild].name);
				weightChoice = true;
			}

			if(!weightChoice){
				ch->send("\n\rThat is not a valid hometown choice, try again:\n\r");
				for(i = 0; i < MAX_HOMETOWN;i++){
					if(!hometowns[i].canuse || !pc_race_table[ch->race].home_use[i])
						continue;
					printf_to_char(ch," -%s\n\r",hometowns[i].name);
				}
				ch->send("Please choose a hometown.\n\r");
				break;
			}
			send_block_line(ch,false,"{w[{RNewbie School{w]");
			ch->send("\n\rYou now have the option to go to the mud school, where you can learn the fundamentals of mudding, and some information about this game that seperates it from other ROM muds.\n\r");
			ch->send("Type {GYes {xto be transfered there, or {GNo {xto enter the regular world.\n\r");
			d->connected = CON_GET_SCHOOL;
			break;
		case CON_GET_SCHOOL:
			if(!str_prefix(argument,"yes")){
				ch->send("You will be transfered to the newbie school.\n\r");
				d->school = true;
				ch->pcdata->points = pc_race_table[ch->race].points;
				ch->setplr(PL_COLOR);
				do_function(ch,&do_help,"motd");
				d->connected = CON_READ_MOTD;
				break;
			}
			else if(!str_prefix(argument,"no")){
				ch->send("You will not be transfered to the newbie school.\n\r");
				ch->pcdata->points = pc_race_table[ch->race].points;
				ch->setplr(PL_COLOR);
				do_function(ch,&do_help,"motd");
				d->connected = CON_READ_MOTD;
				break;
			}
			else{
				ch->send("Please type {GY{xes or {GN{xo.\n\r");
				break;
			}
			break;
		case CON_READ_IMOTD:
			do_function(ch, &do_help, "motd");
			d->connected = CON_READ_MOTD;
			break;
		case CON_NOTE_TO:
			handle_con_note_to (d, argument);
			break;
		case CON_NOTE_SUBJECT:
			handle_con_note_subject (d, argument);
			break; /* subject */
		case CON_NOTE_EXPIRE:
			handle_con_note_expire (d, argument);
			break;
		case CON_NOTE_TEXT:
			handle_con_note_text (d, argument);
			break;
		case CON_NOTE_FINISH:
			handle_con_note_finish (d, argument);
			break;
		case CON_READ_MOTD:
			if(ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0'){
				write_to_buffer(d,"Warning! Null password!\n\r",0);
				write_to_buffer(d,"Please report old password with bug.\n\r",0);
				write_to_buffer(d,"Type 'password null <new password>' to fix.\n\r",0);
			}
			wiznet("$N has penetrated the mothership!",ch,NULL,WZ_NEWBIE,0,get_trust(ch));
			check_multiplay(d);
			ch->setplr(PL_AUTOEXIT);
			ch->setplr(PL_COLOR);
			ch->remplr(PL_ARENA);
			ch->next	= char_list;
			char_list	= ch;
			d->connected	= CON_PLAYING;
			reset_char(ch);
			if (ch->level == 0){
				ch->level				= 1;
				ch->version				= 6;
				ch->gold				= 100;
				ch->silver				= 100;
				ch->nobility			= 1;
				ch->hit					= ch->max_hit;
				ch->move				= ch->max_move;
				ch->pcdata->timezone	= 0;
				ch->pcdata->statpoints	= 5;
				ch->pcdata->studies		= 2;
				ch->spiritguide			= (number_range(1,MAX_SPIRITS-1));
				ch->pcdata->created		= str_dup((char *) ctime(&current_time));
				ch->mid					= get_pc_id();

				ch->pcdata->created[strlen(ch->pcdata->created) - 2] = '\0';
				ch->setmana(ch->getmaxmana());

				if (number_percent() < 15)
					ch->lefty = true;

				sprintf(buf,", The Tenderfoot Adventurer.");
				set_title(ch,buf);

				do_function(ch,&do_outfit,"");

				impose_job(ch,CLASS_TENDERFOOT);

				ch->pcdata->up_skill(5,100,gsn_recall);
				ch->pcdata->learned[get_weapon_sn(ch,false)] = classes[ch->pclass].skill_adept;
				ch->pcdata->skill_level[get_weapon_sn(ch,false)] = 1;

				obj_to_char(create_object(get_obj_index(hometowns[ch->hometown].map),0),ch);
				obj_to_char(create_object(get_obj_index(OBJ_VNUM_NOODLE),0),ch);
				obj_to_char(create_object(get_obj_index(OBJ_VNUM_NOODLE),0),ch);
				obj_to_char(create_object(get_obj_index(OBJ_VNUM_NOODLE),0),ch);
				if(!d->school)
					char_to_room(ch,get_room_index(hometowns[ch->hometown].recall));
				else
					char_to_room(ch,get_room_index(VNUM_SCHOOL));
				send_to_char("\n\r",ch);
				do_function(ch,&do_help,"newbie info");
				send_to_char("\n\r",ch);
				cql_save_new_char(ch);
			}
			else{
				if (ch->in_room)
					char_to_room(ch,ch->in_room);
				else if (IS_IMMORTAL(ch))
					char_to_room(ch,get_room_index(ROOM_VNUM_CHAT));
				else
					char_to_room(ch,get_room_index(1));
				int bcount=0,i;
				for(i=0;i < MAX_BOARD;i++)
					bcount += UMAX(unread_notes(ch,&boards[i]),0);
				ch->printf("{xYou have {Y%d {xunread notes.\n\r",bcount);
				ch->printf("You were last on %s\n\r",ch->pcdata->lastlogin);
			}

			act("$n has entered the game.",ch,NULL,NULL,TO_ROOM);
			for(DESCRIPTOR_DATA *dn = descriptor_list;dn;dn = dn->next){
				if(dn->connected != CON_PLAYING || dn->character == ch)
					continue;
				CHAR_DATA *victim;

				victim = dn->original ? dn->original : dn->character;
				if(is_same_group(ch,victim) && ch->in_room != victim->in_room)
					act("$n has logged in, and is in your group.",ch,NULL,victim,TO_VICT);
			}
			check_penumbral(ch);
			d_insert(ch->desc);

			mud.t_con++;
			check_max_con();

			if(ch->iscomm(CM_QUIET))
				ch->send("You are set to QUIET mode.\n\r");
			if(ch->iscomm(CM_DEAF))
				ch->send("You are set to DEAF mode.\n\r");
			if(ch->iscomm(CM_BUSY))
				ch->send("You are set to BUSY mode.\n\r");
			if(ch->iscomm(CM_AFK))
				ch->send("You are set to AFK mode.\n\r");
			if(ch->iscomm(CM_STUPID))
				ch->send("You are STUPID.\n\r");
			if(ch->iscomm(CM_DEBUG))
				ch->send("You are set to DEBUG mode.\n\r");

			//do_function(ch,&do_look,"auto");
			wiznet("$N has left real life behind.",ch,NULL,WZ_LOGINS,WZ_SITES,get_trust(ch));
			if (ch->pet){
				char_to_room(ch->pet,ch->in_room);
				act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
			}
			if (ch->mount){
				char_to_room(ch->mount,ch->in_room);
				act("$n has entered the game.",ch->mount,NULL,NULL,TO_ROOM);
				add_follower(ch->mount, ch, false);
				do_mount(ch, ch->mount->name);
			}
			send_to_char("\n",ch);
			//do_board(ch,"");  /* Show board status */
			break;
		}
}
