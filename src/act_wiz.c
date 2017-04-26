//Ceran says 'I have 1234567/2147483647 hp 5537/5537 mana 1645/1645 mv 0 xp.'
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "lookup.h"
#include "olc.h"

 DECLARE_DO_FUN(do_wset);

/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location		( CHAR_DATA*,char* );
void				reset_skills		( CHAR_DATA* );
bool				write_to_descriptor	( int,char*,int );
bool				check_parse_name	( char* );
void				raw_kill			( CHAR_DATA*,CHAR_DATA* );
void				fwrite_obj			( CHAR_DATA*,OBJ_DATA*,FILE*,int );
void				write_max_con		( );
void				load_copyover_obj	( void );
void				fread_obj			( CHAR_DATA*,FILE* );
int					get_attackspeed		( CHAR_DATA*,bool );
void				check_penumbral		( CHAR_DATA* );
bool				obj_check			( CHAR_DATA*,OBJ_DATA* );
void				recursive_clone		( CHAR_DATA*,OBJ_DATA*,OBJ_DATA* );
void				mod_attribute		( CHAR_DATA*,CHAR_DATA*,int,char*,char*,const struct weight_type* );
void				save_who			( );
bool				double_exp = false;
bool				cql_load_char(DESCRIPTOR_DATA*,char*);
void				cql_load_char_obj(CHAR_DATA*);

void do_wiznet(CHAR_DATA *ch,char *argument){
    l_int flag;
    char buf[MSL];

	if (!argument[0]){
		if (!ch->setwiz(WZ_ON)){
			ch->send("Signing off of Wiznet.\n\r");
			ch->remwiz(WZ_ON);
		}
		else
			ch->send("Welcome to Wiznet!\n\r");
		return;
	}

	if (!str_prefix(argument,"on")){
		ch->send("Welcome to Wiznet!\n\r");
		ch->setwiz(WZ_ON);
		return;
	}

	if (!str_prefix(argument,"off")){
		ch->send("Signing off of Wiznet.\n\r");
		ch->remwiz(WZ_ON);
		return;
	}

    /* show wiznet status */
    if (!str_prefix(argument,"status")){
		buf[0] = '\0';

		if (!ch->iswiz(WZ_ON))
			strcat(buf,"off ");

		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			if (ch->iswiz(wiznet_table[flag].flag)){
				strcat(buf,wiznet_table[flag].name);
				strcat(buf," ");
			}

		strcat(buf,"\n\r");

		ch->send("Wiznet status:\n\r");
		ch->send(buf);
		return;
    }

    if (!str_prefix(argument,"show")){
		buf[0] = '\0';

		for (flag = 0; wiznet_table[flag].name != NULL; flag++){
			if (wiznet_table[flag].level <= get_trust(ch)){
				strcat(buf,wiznet_table[flag].name);
				strcat(buf," ");
			}
		}

		strcat(buf,"\n\r");

		ch->send("Wiznet options available to you are:\n\r");
		ch->send(buf);
		return;
    }
   
    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level){
		ch->send("No such option.\n\r");
		return;
    }
   
    if (!ch->setwiz(wiznet_table[flag].flag)){
		printf_to_char(ch,"You will no longer see %s on wiznet.\n\r", wiznet_table[flag].name);
		ch->remwiz(wiznet_table[flag].flag);
    }
    else
    	printf_to_char(ch,"You will now see %s on wiznet.\n\r", wiznet_table[flag].name);
}

void wiznet(char *string,CHAR_DATA *ch,OBJ_DATA *obj,long flag,long flag_skip,int min_level) {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next ){
		if (d->connected == CON_PLAYING
		//&&  IS_IMMORTAL(d->character) 
		&&  d->character->iswiz(WZ_ON) 
		&&  (!flag || d->character->iswiz(flag))
		&&  (!flag_skip || !d->character->iswiz(flag_skip))
		&&  get_trust(d->character) >= min_level
		&&  d->character != ch){
			if (d->character->iswiz(WZ_PREFIX))
	  			d->character->send("{C--> {x");
			act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
	  		d->character->send("{x");
		}
	}
}

void do_outfit(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj;
	int sn,vnum;
	char buf[MSL];

	if (ch->level > NEWBIE || IS_NPC(ch)){
		ch->send("Find it yourself!\n\r");
		return;
	}
	if ((obj = get_eq_char(ch,WEAR_LIGHT)) == NULL){
		obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_BANNER),0);
		obj->cost = 0;
		obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_LIGHT);
	}

	if ((obj = get_eq_char(ch,WEAR_TORSO)) == NULL){
		obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_VEST),0);
		obj->cost = 0;
		obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_TORSO);
	}
	if ((obj = get_eq_char(ch,WEAR_LEGS)) == NULL){
		obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_PANTS),0);
		obj->cost = 0;
		obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_LEGS);
	}
	if ((obj = get_eq_char(ch,WEAR_FEET)) == NULL){
		obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_BOOTS),0);
		obj->cost = 0;
		obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_FEET);
	}
	if ((obj = get_eq_char(ch,WEAR_WAIST)) == NULL){
		obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_BELT),0);
		obj->cost = 0;
		obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_WAIST);
	}
	if (!(obj = get_eq_char(ch,WEAR_HEAD))){
		obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_CAP),0);
		obj->cost = 0;
		obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_HEAD);
	}
	if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL){
		sn = 0;
		vnum = OBJ_VNUM_SCHOOL_CLUB;

		obj = create_object(get_obj_index(vnum),0);
		obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_WIELD);
	}

	ch->send("You have been equipped.\n\r");
}

void do_nochannels(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Nochannel whom?");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (get_trust(victim) >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!victim->setcomm(CM_NOCHANNELS)){
		victim->remcomm(CM_NOCHANNELS);
		victim->send("The gods have restored your channel priviliges.\n\r");
		ch->send("NOCHANNELS removed.\n\r");
		sprintf(buf,"$N restores channels to %s",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	else{
		victim->send("The gods have revoked your channel priviliges.\n\r");
		ch->send("NOCHANNELS set.\n\r");
		sprintf(buf,"$N revokes %s's channels.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	return;
}

void do_smote(CHAR_DATA *ch,char *argument){
	CHAR_DATA *vch;
	char *letter,*name,last[MIL],temp[MSL];
	int matches = 0;

	if (!IS_NPC(ch) && ch->iscomm(CM_NOEMOTE)){
		ch->send("You can't show your emotions.\n\r");
		return;
	}

	if (!argument[0]){
		ch->send("Emote what?\n\r");
		return;
	}

	if (!strstr(argument,ch->name)){
		ch->send("You must include your name in an smote.\n\r");
		return;
	}

	ch->send(argument);
	ch->send("\n\r");

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room){
		if (vch->desc == NULL || vch == ch)
			continue;

		if (!(letter = strstr(argument,vch->name))){
			vch->send(argument);
			vch->send("\n\r");
			continue;
		}

		strcpy(temp,argument);
		temp[strlen_color(argument) - strlen_color(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;

		for (; *letter != '\0'; letter++){
			if (*letter == '\'' && matches == strlen_color(vch->name)){
				strcat(temp,"r");
				continue;
			}

			if (*letter == 's' && matches == strlen_color(vch->name)){
				matches = 0;
				continue;
			}

			if (matches == strlen_color(vch->name))
				matches = 0;

			if (*letter == *name){
				matches++;
				name++;
				if (matches == strlen_color(vch->name)){
					strcat(temp,"you");
					last[0] = '\0';
					name = vch->name;
					continue;
				}
				strncat(last,letter,1);
				continue;
			}

			matches = 0;
			strcat(temp,last);
			strncat(temp,letter,1);
			last[0] = '\0';
			name = vch->name;
		}

		vch->send(temp);
		vch->send("\n\r");
	}
}

void do_bamfin(CHAR_DATA *ch,char *argument){
	if (!IS_NPC(ch))
	{
		smash_tilde(argument);

		if (!argument[0]){
			printf_to_char(ch,"Your poofin is %s\n\r",ch->pcdata->bamfin);
			return;
		}

		if (!strstr(argument,ch->name)){
			send_to_char("You must include your name.\n\r",ch);
			return;
		}

		free_string(ch->pcdata->bamfin);
		ch->pcdata->bamfin = str_dup(argument);

		printf_to_char(ch,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
	}
}

void do_bamfout(CHAR_DATA *ch,char *argument){
	if (!IS_NPC(ch)){
		smash_tilde(argument);

		if (!argument[0]){
			printf_to_char(ch,"Your poofout is %s\n\r",ch->pcdata->bamfout);
			return;
		}

		if (!strstr(argument,ch->name)){
			ch->send("You must include your name.\n\r");
			return;
		}

		free_string(ch->pcdata->bamfout);
		ch->pcdata->bamfout = str_dup(argument);

		printf_to_char(ch,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
	}
}

void do_deny(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);
	if (!arg[0]){
		ch->send("Deny whom?\n\r");
		return;
	}
	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}
	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}
	if (get_trust(victim) >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	victim->setplr(PL_DENY);
	victim->send("You are denied access!\n\r");
	sprintf(buf,"$N denies access to %s",victim->name);
	wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	ch->send("OK.\n\r");
	cql_save_char(victim);
	stop_fighting(victim,true);
	do_function(victim,&do_quit,"");
}

void do_disconnect(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument(argument,arg);
	if (!arg[0]){
		ch->send("Disconnect whom?\n\r");
		return;
	}

	if (is_number(arg)){
		int desc;

		desc = atoi(arg);
		for (d = descriptor_list;d;d = d->next){
			if (d->descriptor == desc){
				close_socket(d);
				ch->send("Ok.\n\r");
				return;
			}
		}
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (!victim->desc){
		act("$N doesn't have a descriptor.",ch,NULL,victim,TO_CHAR);
		return;
	}

	for (d = descriptor_list;d;d = d->next){
		if (d == victim->desc){
			close_socket(d);
			ch->send("Ok.\n\r");
			return;
		}
	}

	bug("Do_disconnect: desc not found.",0);
	ch->send("Descriptor not found!\n\r");
}

void do_echo(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d;
	char arg[10];

	if(!strcmp(ch->name,"Nash"))
		sprintf(arg,"{C");
	else if(!strcmp(ch->name,"Boon"))
		sprintf(arg,"{g");
	else if(!strcmp(ch->name,"Idrial"))
		sprintf(arg,"{G");
	else if(!strcmp(ch->name,"Enurai"))
		sprintf(arg,"{Y");
	else
		sprintf(arg,"{x-");

	if (!argument[0]){
		ch->send("Global echo what?\n\r");
		return;
	}

	for (d = descriptor_list;d;d = d->next)
		if ( d->connected == CON_PLAYING )
			printf_to_char(d->character,"%s%s{x\n\r",arg,argument);
}

void do_recho(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d;

	if (!argument[0]){
		ch->send("Local echo what?\n\r");
		return;
	}

	for (d = descriptor_list;d;d = d->next){
		if (d->connected == CON_PLAYING && d->character->in_room == ch->in_room){
			if (get_trust(d->character) >= get_trust(ch))
				d->character->send("local> ");
			printf_to_char(d->character,"%s\n\r",argument);
		}
	}
}

void do_zecho(CHAR_DATA *ch, char *argument){
	DESCRIPTOR_DATA *d;

	if (!argument[0]){
		ch->send("Zone echo what?\n\r");
		return;
	}

	for (d = descriptor_list;d;d = d->next){
		if (d->connected == CON_PLAYING && d->character->in_room && ch->in_room && d->character->in_room->area == ch->in_room->area){
			if (get_trust(d->character) >= get_trust(ch))
				d->character->send("zone> ");
			printf_to_char(d->character,"%s\n\r",argument);
		}
	}
}

void do_pecho(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;

	argument = one_argument(argument,arg);

	if (!argument[0] || !arg[0]){
		ch->send("Personal echo what?\n\r");
		return;
	}

	if  (!(victim = get_char_world(ch,arg))){
		ch->send("Target not found.\n\r");
		return;
	}

	if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL-2)
		victim->send("personal> ");

	printf_to_char(victim,"%s\n\r",argument);
	printf_to_char(ch,"personal> %s\n\r",argument);
}

void do_transfer(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL],buf[MSL];
	ROOM_INDEX_DATA *location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if (!arg1[0]){
		ch->send("Transfer whom (and where)?\n\r");
		return;
	}

	if (!str_cmp(arg1,"all")){
		for (d = descriptor_list;d;d = d->next){
			if (d->connected == CON_PLAYING && d->character != ch && d->character->in_room && can_see(ch,d->character)){
				sprintf(buf,"%s %s",d->character->name,arg2);
				do_function(ch,&do_transfer,buf);
			}
		}
		return;
	}

	if (!arg2[0])
		location = ch->in_room;
	else{
		if (!(location = find_location(ch,arg2))){
			ch->send("No such location.\n\r");
			return;
		}

		if (!is_room_owner(ch,location) && room_is_private(location) && get_trust(ch) < MAX_LEVEL-2){
			ch->send("That room is private right now.\n\r");
			return;
		}
	}
	if(location->area->locked && !IS_BUILDER(ch,location->area)){
		ch->send("That area is locked and you lack clearance.\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("They aren't here.\n\r");
		return;
	}
	if (!IS_NPC(victim) && victim->isplr(PL_ARENA)){
		ch->send("They're dueling, dork!\n\r");
		return;
	}
	if (!victim->in_room){
		ch->send("They are broken.\n\r");
		return;
	}
	if (victim->fighting)
		stop_fighting(victim,true);
	act("$n disappears in a mushroom cloud.",victim,NULL,NULL,TO_ROOM);
	char_from_room(victim);
	char_to_room(victim,location);
	act("$n arrives from a puff of smoke.",victim,NULL,NULL,TO_ROOM);
	if(victim->mount && victim->mount->ismount){
		char_from_room(victim->mount);
		char_to_room(victim->mount,location);
		victim->mount->send("Your rider is being transferred, and so are you.\n\r");
	}

	if (victim->pet){
		char_from_room(victim->pet);
		char_to_room(victim->pet,location);
	}

	if (ch != victim)
		act("$n has transferred you.",ch,NULL,victim,TO_VICT);
	do_function(victim,&do_look,"auto");
    ch->send("Ok.\n\r");
}

void do_at(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	ROOM_INDEX_DATA *location,*original;
	OBJ_DATA *on;
	CHAR_DATA *wch;

	argument = one_argument(argument,arg);

	if (!arg[0] || !argument[0]){
		ch->send("At where what?\n\r");
		return;
	}
	if (!(location = find_location(ch,arg))){
		ch->send("No such location.\n\r");
		return;
	}
	if (!is_room_owner(ch,location) && room_is_private(location) && get_trust(ch) < MAX_LEVEL-2){
		ch->send("That room is private right now.\n\r");
		return;
	}

	original = ch->in_room;
	on = ch->on;
	char_from_room(ch);
	char_to_room(ch,location);
	interpret(ch,argument);

	for (wch = char_list;wch;wch = wch->next){
		if (wch == ch){
			char_from_room(ch);
			char_to_room(ch,original);
			ch->on = on;
			break;
		}
	}
}

void do_goto(CHAR_DATA *ch,char *argument){
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	int count = 0;

	if (!argument[0]){
		ch->send("Goto where?\n\r");
		return;
	}
	if (ch->isplr(PL_ARENA)){
		ch->send("You're dueling, dork!\n\r");
		return;
	}

	if (!(location = find_location(ch,argument))){
		ch->send("No such location.\n\r");
		return;
	}

	count = 0;
	for (rch = location->people;rch;rch = rch->next_in_room)
		count++;

	if (!is_room_owner(ch,location) && room_is_private(location) &&  (count > 1 || get_trust(ch) < MAX_LEVEL-2)){
		ch->send("That room is private right now.\n\r");
		return;
	}
	if(location->area->locked && !IS_BUILDER(ch,location->area)){
		ch->send("That area is locked and you lack clearance.\n\r");
		return;
	}

	if (ch->fighting)
		stop_fighting(ch,true);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room){
		if (get_trust(rch) >= ch->invis_level){
			if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
				act("$t{x",ch,ch->pcdata->bamfout,rch,TO_VICT);
			else
				act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
		}
	}

	char_from_room(ch);
	char_to_room(ch,location);

	if (ch->pet){
		char_from_room(ch->pet);
		char_to_room(ch->pet,location);
	}

	if (MOUNTED(ch)){
		char_from_room(MOUNTED(ch));
		char_to_room(MOUNTED(ch),location);
	}   

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room){
		if (get_trust(rch) >= ch->invis_level){
			if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
				act("$t{x",ch,ch->pcdata->bamfin,rch,TO_VICT);
			else
				act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
		}
	}
	do_function(ch,&do_look,"auto");
}

void do_violate(CHAR_DATA *ch,char *argument){
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;

	if (!argument[0]){
		ch->send("Goto where?\n\r");
		return;
	}

	if (!(location = find_location( ch,argument))){
		ch->send("No such location.\n\r");
		return;
	}

	if (!room_is_private(location)){
		ch->send("That room isn't private, use goto.\n\r");
		return;
	}

	if (ch->fighting != NULL)
		stop_fighting(ch,true);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room){
		if (get_trust(rch) >= ch->invis_level){
			if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
				act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
			else
				act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
		}
	}

	char_from_room(ch);
	char_to_room(ch,location);

	for (rch = ch->in_room->people;rch;rch = rch->next_in_room){
		if (get_trust(rch) >= ch->invis_level){
			if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
				act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
			else
				act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
		}
	}
	do_function(ch,&do_look,"auto");
}

void do_reboo(CHAR_DATA *ch,char *argument){send_to_char("If you want to REBOOT, spell it out.\n\r",ch);return;}

void do_reboot(CHAR_DATA *ch,char *argument){
	char buf[MSL];
	extern bool merc_down;
	DESCRIPTOR_DATA *d,*d_next;
	CHAR_DATA *vch;
	FILE *fp;

	if (ch->invis_level < LEVEL_HERO){
		sprintf(buf,"Reboot by %s.",ch->name);
		do_function(ch,&do_echo,buf);
	}

	merc_down = true;
	for (d = descriptor_list;d; d = d_next){
		d_next = d->next;
		vch = d->original ? d->original : d->character;
		if (vch != NULL)
			cql_save_char(vch);
		close_socket(d);
	}
}

void do_shutdow(CHAR_DATA *ch,char *argument){send_to_char("If you want to SHUTDOWN, spell it out.\n\r",ch);return;}

void do_shutdown(CHAR_DATA *ch,char *argument){
	char buf[MSL];
	extern bool merc_down;
	DESCRIPTOR_DATA *d,*d_next;
	CHAR_DATA *vch;
	FILE *fp;

	if (ch->invis_level < LEVEL_HERO)
		sprintf(buf,"Shutdown by %s.",ch->name);
	append_file(ch,SHUTDOWN_FILE,buf);
	strcat(buf,"\n\r");
	if (ch->invis_level < LEVEL_HERO)
		do_function(ch,&do_echo,buf);
	merc_down = true;
	for (d = descriptor_list;d;d = d_next){
		d_next = d->next;
		vch = d->original ? d->original : d->character;
		if (vch != NULL)
			cql_save_char(vch);
		close_socket(d);
	}
}

void do_protect(CHAR_DATA *ch, char *argument){
	CHAR_DATA *victim;

	if (!argument[0]){
		ch->send("Protect who from snooping?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,argument))){
		ch->send("You can't find them.\n\r");
		return;
	}

	if (!victim->setcomm(CM_SNOOP_PROOF)){
		act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
		victim->send("Your snoop-proofing was just removed.\n\r");
		victim->remcomm(CM_SNOOP_PROOF);
	}
	else{
		act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
		victim->send("You are now immune to snooping.\n\r");
	}
}

void do_switch(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Switch into whom?\n\r");
		return;
	}

	if (!ch->desc)
		return;

	if (ch->desc->original){
		ch->send("You are already switched.\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (victim == ch){
		ch->send("Ok...\n\r");
		return;
	}

	if (!IS_NPC(victim)){
		ch->send("You can only switch into mobiles.\n\r");
		return;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room && room_is_private(victim->in_room) && !IS_TRUSTED(ch,ADMIN)){
		ch->send("That character is in a private room.\n\r");
		return;
	}

	if (victim->desc){
		ch->send("Character in use.\n\r");
		return;
	}

	sprintf(buf,"$N switches into %s",victim->short_descr);
	wiznet(buf,ch,NULL,WZ_SWITCHES,WZ_SECURE,get_trust(ch));

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;
	/* change communications to match */
	if (ch->prompt != NULL)
		victim->prompt = str_dup(ch->prompt);
	clone_bits(ch->comm,victim->comm,MAX_CMM);
	victim->lines = ch->lines;
	victim->send("Ok.\n\r");
}

void do_return(CHAR_DATA *ch,char *argument){
	char buf[MSL];

	if (!ch->desc)
		return;

	if (!ch->desc->original){
		ch->send("You aren't switched.\n\r");
		return;
	}

	ch->send("You return to your original body. Type replay to see any missed tells.\n\r");
	if (ch->prompt){
		free_string(ch->prompt);
		ch->prompt = NULL;
	}

	sprintf(buf,"$N returns from %s.",ch->short_descr);
	wiznet(buf,ch->desc->original,0,WZ_SWITCHES,WZ_SECURE,get_trust(ch));
	ch->desc->character       = ch->desc->original;
	ch->desc->original        = NULL;
	ch->desc->character->desc = ch->desc;
	ch->desc                  = NULL;
}

void do_clone(CHAR_DATA *ch,char *argument){
	char arg[MIL],*rest;
	CHAR_DATA *mob;
	OBJ_DATA  *obj;

	rest = one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Clone what?\n\r");
		return;
	}

	if (!str_prefix(arg,"object")){
		mob = NULL;
		obj = get_obj_here(ch, NULL,rest);
		if (!obj){
			ch->send("You don't see that here.\n\r");
			return;
		}
	}
	else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")){
		obj = NULL;
		mob = get_char_room(ch, NULL,rest);
		if (!mob){
			ch->send("You don't see that here.\n\r");
			return;
		}
	}
	else{
		mob = get_char_room(ch, NULL,argument);
		obj = get_obj_here(ch, NULL,argument);
		if (!mob && !obj){
			ch->send("You don't see that here.\n\r");
			return;
		}
	}

	if (obj){
		OBJ_DATA *clone;

		if (!obj_check(ch,obj)){
			ch->send("Your powers are not great enough for such a task.\n\r");
			return;
		}

		clone = create_object(obj->pIndexData,0);
		clone_object(obj,clone);
		if (obj->carried_by != NULL)
			obj_to_char(clone,ch);
		else
			obj_to_room(clone,ch->in_room);
 		recursive_clone(ch,obj,clone);

		act("$n has cloned $p.",ch,clone,NULL,TO_ROOM);
		act("You clone $p.",ch,clone,NULL,TO_CHAR);
		wiznet("$N clones $p.",ch,clone,WZ_LOAD,WZ_SECURE,get_trust(ch));
		return;
	}
	else if (mob){
		CHAR_DATA *clone;
		OBJ_DATA *new_obj;
		char buf[MSL];

		if (!IS_NPC(mob)){
			ch->send("You can only clone mobiles.\n\r");
			return;
		}

		if ((mob->level > 20 && !IS_TRUSTED(ch,IMMORTAL))
		||  (mob->level > 10 && !IS_TRUSTED(ch,JBUILDER))
		||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
		||  (mob->level >  0 && !IS_TRUSTED(ch,KING))
		||  !IS_TRUSTED(ch,HLEADER)){
			ch->send("Your powers are not great enough for such a task.\n\r");
			return;
		}

		clone = create_mobile(mob->pIndexData);
		clone_mobile(mob,clone);
		
		for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
			if (obj_check(ch,obj)){
				new_obj = create_object(obj->pIndexData,0);
				clone_object(obj,new_obj);
				recursive_clone(ch,obj,new_obj);
				obj_to_char(new_obj,clone);
				new_obj->wear_loc = obj->wear_loc;
			}
		char_to_room(clone,ch->in_room);
		act("$n has created $N.",ch,NULL,clone,TO_ROOM);
		act("You clone $N.",ch,NULL,clone,TO_CHAR);
		sprintf(buf,"$N clones %s.",clone->short_descr);
		wiznet(buf,ch,NULL,WZ_LOAD,WZ_SECURE,get_trust(ch));
		return;
	}
}

void do_load(CHAR_DATA *ch,char *argument){
	char arg[MIL];

	argument = one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Syntax:\n\r  load mob <vnum>\n\r  load obj <vnum> <level>\n\r");
		return;
	}

	if (!str_cmp(arg,"mob") || !str_cmp(arg,"char")){
		do_function(ch,&do_mload,argument);
		return;
	}

	if (!str_cmp(arg,"obj")){
		do_function(ch,&do_oload,argument);
		return;
	}
	do_function(ch,&do_load,"");
}

void do_mload(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0] || !is_number(arg)){
		ch->send("Syntax: load mob <vnum>.\n\r");
		return;
	}

	if (!(pMobIndex = get_mob_index(atoi(arg)))){
		ch->send("No mob has that vnum.\n\r");
		return;
	}

	victim = create_mobile(pMobIndex);
	char_to_room(victim,ch->in_room);
	act("$n has created $N!",ch,NULL,victim,TO_ROOM);
	sprintf(buf,"$N loads %s.",victim->short_descr);
	wiznet(buf,ch,NULL,WZ_LOAD,WZ_SECURE,get_trust(ch));
	act("You've {Cloaded {x'$N'.",ch,NULL,victim,TO_CHAR);
}

void do_oload(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int level;

	argument = one_argument(argument,arg1);
	one_argument(argument,arg2);

	if (!arg1[0] || !is_number(arg1)){
		ch->send("Syntax: load obj <vnum> <level>.\n\r");
		return;
	}

	level = get_trust(ch);

	if (arg2[0]){
		if (!is_number(arg2)){
			ch->send("Syntax: oload <vnum>.\n\r");
			return;
		}
		level = atoi(arg2);
		if (level < 0 || level > get_trust(ch)){
			ch->send("Level must be be between 0 and your level.\n\r");
			return;
		}
	}

	if (!(pObjIndex = get_obj_index(atoi(arg1)))){
		ch->send("No object has that vnum.\n\r");
		return;
	}

	obj = create_object(pObjIndex,level);
	if (CAN_WEAR(obj,ITEM_TAKE))
		obj_to_char(obj,ch);
	else
		obj_to_room(obj,ch->in_room);
	act("$n has {Ccreated {x'$p{x'!",ch,obj,NULL,TO_ROOM);
	act("You've {Cloaded {x'$p'.",ch,obj,NULL,TO_CHAR);
	wiznet("$N loads $p.",ch,obj,WZ_LOAD,WZ_SECURE,get_trust(ch));
}

void do_purge(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[100];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	DESCRIPTOR_DATA *d;

	one_argument(argument,arg);

	if (!arg[0]){
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->people;victim;victim = vnext){
			vnext = victim->next_in_room;
			if (IS_NPC(victim) && !victim->isact(AT_NOPURGE) && victim != ch)
				extract_char(victim,true);
		}

		for (obj = ch->in_room->contents;obj;obj = obj_next){
			obj_next = obj->next_content;
			if (!IS_OBJ_STAT(obj,ITM_NOPURGE))
			  extract_obj(obj);
		}

		act("$n purges the room!",ch,NULL,NULL,TO_ROOM);
		ch->send("You purge the room.\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if (!IS_NPC(victim)){
		if (ch == victim){
			ch->send("Ho ho ho.\n\r");
			return;
		}

		if (get_trust(ch) <= get_trust(victim)){
			ch->send("Maybe that wasn't a good idea...\n\r");
			printf_to_char(victim,"%s tried to purge you!\n\r",ch->name);
			return;
		}

		act("You reduce $N to a pile of ashes.",ch,0,victim,TO_CHAR);
		act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

		if (victim->level > 1)
			cql_save_char(victim);
		d = victim->desc;
		extract_char(victim,true);
		if ( d != NULL )
		  close_socket(d);

		return;
	}

	act("$n purges $N.",ch,NULL,victim,TO_NOTVICT);
	extract_char(victim,true);
}

int get_tier_class(CHAR_DATA *ch,int tier,int destclass){
	int i, tier2 = -1;
	if(tier == 0)
		return 1;//always tenderfoot
	if(tier < classes[destclass].ctier){
		switch(classes[destclass].ctier){
		case 3:
			for(i = 0;i < MAX_CLASS;i++){
				if(classes[i].becomes[destclass])
					tier2 = i;
			}
			if(tier2 > -1){
				if(tier == 2)
					return tier2;
				else
					for(i = 0;i < MAX_CLASS;i++){
						if(classes[i].becomes[tier2])
							return i;
					}
			}
			break;
		case 2://this will only have tier 1 requested
			for(i = 0;i < MAX_CLASS;i++){
				if(classes[i].becomes[destclass])
					return i;
			}
			break;
		};
	}
	else if(tier == classes[destclass].ctier)
		return destclass;
	//else if(tier > classes[destclass].ctier)
	return -1;
}

void do_advance(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL],arg3[MIL];
	CHAR_DATA *victim;
	int level,iLevel,tclass;
	bool yesreset = false;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);

	if (!arg1[0] || !arg2[0] || !is_number(arg2)){
		ch->send("Syntax: advance <char> <level>.\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("That player is not here.\n\r");
		return;
	}

	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}

	tclass = victim->pclass;

	if ((level = atoi(arg2)) < 1 || level > ch->level-1){
		printf_to_char(ch,"Level must be 1 to %d.\n\r",ch->level-1);
		return;
	}
	if((victim->level < 50 && level > 50
	|| victim->level < 25 && level > 25
	|| victim->level < 10 && level > 10)){
		if(!arg3[0]){
			ch->send("To set the victim to this new level, you must specify a target class.\n\r");
			return;
		}

		if((tclass = class_lookup(arg3)) < 0){
			ch->send("That is not a valid class.\n\r");
			return;
		}
	}

	if(level > 50 && classes[tclass].ctier < 3
	|| level > 25 && classes[tclass].ctier < 2
	|| level > 10 && classes[tclass].ctier < 1){
		ch->send("You must pick a class of the appropriate tier.\n\r");
		return;
	}

	if ((get_trust(ch) < MAX_LEVEL-5) && level > 100){
		ch->send("Limited to hero level.\n\r");
		return;
	}
	if (get_trust(ch) <= get_trust(victim)){
		ch->send("You failed.\n\r");
		return;
	}

	if (level < victim->level){
		int temp_prac;

		ch->send("Lowering a player's level!\n\r");
		victim->send("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r");
		victim->level		= 1;
		victim->pclass		= 1;
		victim->exp			= exp_per_level(victim);
		victim->hit			= victim->max_hit 		= victim->pcdata->perm_hit = 100;
		victim->mana		= victim->max_mana 		= victim->pcdata->perm_mana = 100;
		victim->antimana	= victim->max_antimana 	= victim->pcdata->perm_antimana = 100;
		victim->move		= victim->max_move 		= victim->pcdata->perm_move = 100;
		for (int i = 0; i < MAX_STATS; i++)
			victim->perm_stat[i] = pc_race_table[victim->race].stats[i];
		victim->pcdata->s_practices = 0;
		victim->pcdata->s_studies[0] = victim->pcdata->s_studies[1] = victim->pcdata->s_studies[2] = victim->pcdata->s_studies[3] = 0;
		victim->pcdata->practices = 0;
		victim->pcdata->studies = 0;
		yesreset = true;
		//advance_level(victim,true);why?
	}
	else if (level > victim->level){
		ch->send("Raising a player's level!\n\r");
		victim->send("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r");
	}
	else{
		ch->send("Nothing new.\n\r");
		return;
	}

	for (iLevel = victim->level;iLevel < level;iLevel++){
		if(iLevel < level){//while the thing has more levels to go
			if(victim->level == 10)
				victim->pclass = get_tier_class(victim,1,tclass);
			if(victim->level == 25)
				victim->pclass = get_tier_class(victim,2,tclass);
			if(victim->level == 50)
				victim->pclass = get_tier_class(victim,3,tclass);
		}
		victim->level += 1;
		advance_level(victim,true);
	}
	if(yesreset){
		reset_skills(victim);
		printf_to_char(victim,"You are now level %d. Your skills have been reset.\n\r",victim->level);
	}
	else
		printf_to_char(victim,"You are now level %d.\n\r",victim->level);
	printf_to_char(ch,"They are now level %d.\n\r",victim->level);
	victim->exp = exp_per_level(victim);
	victim->exp -= exp_per_level(victim);
	victim->trust = 0;
	cql_save_char(victim);
}

void do_trust(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	CHAR_DATA *victim;
	int level;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if (!arg1[0] || !arg2[0] || !is_number(arg2)){
		ch->send("Syntax: trust <char> <level>.\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("That player is not here.\n\r");
		return;
	}

	if ((level = atoi(arg2)) < 0 || level > MAX_LEVEL){
		printf_to_char(ch,"Level must be 0 (reset) or 1 to %d.\n\r",MAX_LEVEL);
		return;
	}

	if (level > get_trust(ch)){
		ch->send("Limited to your trust.\n\r");
		return;
	}

	victim->trust = level;
}

void do_restore(CHAR_DATA *ch,char *argument){
	char buf[MSL],arg[MIL];
	CHAR_DATA *victim,*vch;
	DESCRIPTOR_DATA *d;

	one_argument(argument,arg);
	if (!arg[0] || !str_cmp(arg,"room")){
		for (vch = ch->in_room->people;vch;vch = vch->next_in_room){
			affect_strip(vch,gsn_plague);
			affect_strip(vch,gsn_malady);
			affect_strip(vch,gsn_rupture);
			affect_strip(vch,gsn_poison);
			affect_strip(vch,gsn_blindness);
			affect_strip(vch,gsn_sleep);
			affect_strip(vch,gsn_curse);
			vch->death_timer = 0;
			vch->hit 	= vch->max_hit;
			vch->settruemana(vch->gettruemaxmana());
			vch->settrueantimana(vch->gettruemaxantimana());
			vch->move	= vch->max_move;
			update_pos(vch,NULL);
			act("$n has restored you.",ch,NULL,vch,TO_VICT);
		}

		sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
		wiznet(buf,ch,NULL,WZ_RESTORE,WZ_SECURE,get_trust(ch));

		ch->send("Room restored.\n\r");
		return;
	}

	if (!str_cmp(arg,"all")){
		for (d = descriptor_list;d;d = d->next){
			victim = d->character;

			if (victim == NULL || IS_NPC(victim))
				continue;

			affect_strip(victim,gsn_plague);
			affect_strip(victim,gsn_malady);
			affect_strip(victim,gsn_rupture);
			affect_strip(victim,gsn_poison);
			affect_strip(victim,gsn_blindness);
			affect_strip(victim,gsn_sleep);
			affect_strip(victim,gsn_curse);
			victim->death_timer = 0;
			victim->hit 	= victim->max_hit;
			victim->setmana(victim->getmaxmana());
			victim->move	= victim->max_move;
			update_pos(victim,NULL);
			if (victim->in_room != NULL)
				act("$n has restored you.",ch,NULL,victim,TO_VICT);
		}
		ch->send("All active players restored.\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	affect_strip(victim,gsn_plague);
	affect_strip(victim,gsn_malady);
	affect_strip(victim,gsn_rupture);
	affect_strip(victim,gsn_poison);
	affect_strip(victim,gsn_blindness);
	affect_strip(victim,gsn_sleep);
	affect_strip(victim,gsn_curse);
	victim->death_timer = 0;
	victim->hit  = victim->max_hit;
	victim->setmana(victim->getmaxmana());
	victim->move = victim->max_move;
	update_pos(victim,NULL);
	act("$n has restored you.",ch,NULL,victim,TO_VICT);
	sprintf(buf,"$N restored %s",IS_NPC(victim) ? victim->short_descr : victim->name);
	wiznet(buf,ch,NULL,WZ_RESTORE,WZ_SECURE,get_trust(ch));
	ch->send("Ok.\n\r");
}

void do_freeze(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Freeze whom?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}

	if (get_trust(victim) >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!victim->setplr(PL_FREEZE)){
		victim->remplr(PL_FREEZE);
		victim->send("You can play again.\n\r");
		ch->send("FREEZE removed.\n\r");
		sprintf(buf,"$N thaws %s.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	else{
		victim->send("You can't do ANYthing!\n\r");
		ch->send("FREEZE set.\n\r");
		sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	cql_save_char(victim);
}

void do_noemote(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Noemote whom?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}


	if (get_trust(victim) >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!victim->setcomm(CM_NOEMOTE)){
		victim->remcomm(CM_NOEMOTE);
		victim->send("You can emote again.\n\r");
		ch->send("NOEMOTE removed.\n\r");
		sprintf(buf,"$N restores emotes to %s.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	else{
		victim->send("You can't emote!\n\r");
		ch->send("NOEMOTE set.\n\r");
		sprintf(buf,"$N revokes %s's emotes.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
}

void do_stupidflag(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Flag who as a moron?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}

	if (get_trust(victim) >= get_trust(ch) || IS_NPC(victim)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!victim->setcomm(CM_STUPID)){
		victim->remcomm(CM_STUPID);
		victim->send("You aren't as big a moron anymore.\n\r");
		ch->send("STUPID removed.\n\r");
		sprintf(buf,"$N restores intelligence to %s.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	else{
		victim->send("You're a dumbass!\n\r");
		printf_to_char(ch,"You declare %s STUPID.\n\r",victim->name);
		sprintf(buf,"$N declares %s an idiot.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
		sprintf(buf,"%s declares that %s is stupid.\n\r",ch->name,victim->name);
		global_message(0,MAX_LEVEL,buf,0);
		if (victim->setcomm(CM_NOCHANNELS)){
			victim->send("You lose your voice too for being an idiot!\n\r");
			ch->send("Channels revoked too.\n\r");
		}
	}
}

void do_noshout(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Noshout whom?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}

	if (get_trust(victim) >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!victim->setcomm(CM_NOSHOUT)){
		victim->remcomm(CM_NOSHOUT);
		victim->send("You can shout again.\n\r");
		ch->send("NOSHOUT removed.\n\r");
		sprintf(buf,"$N restores shouts to %s.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	else{
		victim->send("You can't shout!\n\r");
		ch->send("NOSHOUT set.\n\r");
		sprintf(buf,"$N revokes %s's shouts.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
}

void do_notell(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Notell whom?");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (get_trust(victim) >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!victim->setcomm(CM_NOTELL)){
		victim->remcomm(CM_NOTELL);
		victim->send("You can tell again.\n\r");
		ch->send("NOTELL removed.\n\r");
		sprintf(buf,"$N restores tells to %s.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	else{
		victim->send("You can't tell!\n\r");
		ch->send("NOTELL set.\n\r");
		sprintf(buf,"$N revokes %s's tells.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
}

void do_peace(CHAR_DATA *ch,char *argument){
	CHAR_DATA *rch;
	AFFECT_DATA af;

	for (rch = ch->in_room->people;rch;rch = rch->next_in_room){
		if (rch->fighting != NULL)
			stop_fighting(rch,true);
		if(IS_NPC(rch))
			affect_set(rch,TO_AFFECTS,skill_lookup("calm"),ch->level,1,0,APPLY_HITROLL,IS_NPC(rch) ? -2 : -5,AF_CALM);
	}
	ch->send("Ok.\n\r");
}

void do_wizlock(CHAR_DATA *ch,char *argument){
	extern bool wizlock;
	wizlock = !wizlock;

	if (wizlock){
		wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
		ch->send("Game wizlocked.\n\r");
	}
	else{
		wiznet("$N removes wizlock.",ch,NULL,0,0,0);
		ch->send("Game un-wizlocked.\n\r");
	}
}

void do_newlock(CHAR_DATA *ch,char *argument){
	extern bool newlock;
	newlock = !newlock;

	if (newlock){
		wiznet("$N locks out new characters.",ch,NULL,0,0,0);
		ch->send("New characters have been locked out.\n\r");
	}
	else{
		wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
		ch->send("Newlock removed.\n\r");
	}
}

void do_slvlset(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL],arg3[MIL];
	CHAR_DATA *victim;
	int value,sn;
	bool fAll;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);

	if (!arg1[0] || !arg2[0] || !arg3[0]){
		ch->send("Syntax:\n\r");
		ch->send("  set skill <name> <spell or skill> <value>\n\r");
		ch->send("  set skill <name> all <value>\n\r");
		ch->send("   (use the name of the skill, not the number)\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}

	fAll = !str_cmp(arg2,"all");
	sn   = 0;

	if (!is_number(arg3)){
		ch->send("Value must be numeric.\n\r");
		return;
	}

	value = atoi(arg3);
	if (value < 0 || value > 5){
		ch->send("Value range is 0 to 5.\n\r");
		return;
	}

	if(!fAll){
		if (!str_cmp(arg2,"gained")){
			for(sn=0;sn<MAX_SKILL;sn++){
				if(victim->pcdata->skill_level[sn] > 0){
					victim->pcdata->skill_level[sn] = value;
					victim->pcdata->unlocked[sn] = true;
				}
			}
			return;
		}
	}
	else{
		if(get_trust(ch) < MAX_LEVEL-2){
			ch->send("No way, Jose.\n\r");
			return;
		}
	}
	if (!fAll && (sn = skill_lookup(arg2)) < 0){
		ch->send("No such skill or spell.\n\r");
		return;
	}

	if (fAll){
		for (sn = 0;sn < MAX_SKILL;sn++)
			if (skill_table[sn].name != NULL){
				victim->pcdata->skill_level[sn] = value;
				victim->pcdata->unlocked[sn] = true;
			}
	}
	else{
		victim->pcdata->unlocked[sn] = true;
		victim->pcdata->skill_level[sn] = value;
	}
}

void do_set(CHAR_DATA *ch,char *argument){
	char arg[MIL];

	argument = one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Syntax:\n\r");
		ch->send("  set mob   <name> <field> <value>\n\r");
		ch->send("  set obj   <name> <field> <value>\n\r");
		ch->send("  set room  <room> <field> <value>\n\r");
		ch->send("  set skill <name> <spell or skill> <value>\n\r");
		ch->send("  set weather <value>\n\r");
		return;
	}

	if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")){
		do_function(ch,&do_mset,argument);
		return;
	}

	if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell")){
		do_function(ch,&do_sset,argument);
		return;
	}

	if (!str_prefix(arg,"slvl")){
		do_function(ch,&do_slvlset,argument);
		return;
	}

	if (!str_prefix(arg,"weather")){
		do_function(ch, &do_wset, argument);
		return;
	}

	if (!str_prefix(arg,"object")){
		do_function(ch,&do_oset,argument);
		return;
	}

	if (!str_prefix(arg,"room")){
		do_function(ch,&do_rset,argument);
		return;
	}
	do_function(ch,&do_set,"");
}

void do_sset( CHAR_DATA *ch, char *argument ){
	char arg1[MIL],arg2[MIL],arg3[MIL];
	CHAR_DATA *victim;
	int value,sn;
	bool fAll;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);

	if (!arg1[0] || !arg2[0] || !arg3[0]){
		ch->send("Syntax:\n\r");
		ch->send("  set skill <name> <spell or skill> <value>\n\r");
		ch->send("  set skill <name> all <value>\n\r");
		ch->send("   (use the name of the skill, not the number)\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}

	fAll = !str_cmp(arg2,"all");
	sn   = 0;

	if (!is_number(arg3)){
		ch->send("Value must be numeric.\n\r");
		return;
	}

	value = atoi(arg3);
	if (value < 0 || value > 100){
		ch->send("Value range is 0 to 100.\n\r");
		return;
	}

	if(!fAll){
		if (!str_cmp(arg2,"gained")){
			for(sn=0;sn<MAX_SKILL;sn++)
				if(victim->pcdata->learned[sn] > 0)
					victim->pcdata->learned[sn] = value;
			return;
		}
	}
	else{
		if(get_trust(ch) < MAX_LEVEL-2){
			ch->send("No way, Jose.\n\r");
			return;
		}
	}
	if (!fAll && (sn = skill_lookup(arg2)) < 0){
		ch->send("No such skill or spell.\n\r");
		return;
	}

	if (fAll){
		for (sn = 0;sn < MAX_SKILL;sn++)
			if (skill_table[sn].name != NULL){
				if(victim->pcdata->skill_level[sn] < 1)
					victim->pcdata->skill_level[sn] = 1;
				victim->pcdata->learned[sn]	= value;
			}
	}
	else{
		if(victim->pcdata->skill_level[sn] < 1)
			victim->pcdata->skill_level[sn] = 1;
		victim->pcdata->learned[sn] = value;
	}
	return;
}

void do_mset( CHAR_DATA *ch, char *argument ){
	char arg1[MIL],arg2[MIL],arg3[MIL];
	CHAR_DATA *victim;
	int value;
	bool found=false;

	smash_tilde(argument);
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	strcpy(arg3,argument);

	if (!arg1[0] || !arg2[0] || !arg3[0]){
		ch->send("Syntax:\n\r");
		ch->send("  set char <name> <field> <value>\n\r");
		ch->send("  Field being one of:\n\r");
		ch->send("    stat sex class level\n\r");
		ch->send("    race group gold silver hp mana move prac\n\r");
		ch->send("    align train thirst hunger drunk full\n\r");
		ch->send("    security lastname\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("They aren't here.\n\r");
		return;
	}

	victim->zone = NULL;

	value = is_number(arg3) ? atoi(arg3) : -1;

	if (!IS_NPC(victim) && get_trust(ch) < get_trust(victim)){
		ch->send("You can only set attributes of those of a lower trust level than yourself.\n\r");
		return;
	}

	if(!str_cmp(arg2,"waypoint")){
		ROOM_INDEX_DATA *location;
		if(IS_NPC(victim)){
			ch->send("Not on NPC's.\n\r");
		}
		if(!is_number(arg3))
			ch->send("Value must be numeric.\n\r");
		else{
			if (!(location = find_location(ch,argument)))
				ch->send("No such location.\n\r");
			else{
				ch->send("Waypoint set.\n\r");
				victim->pcdata->map_point = location;
			}
		}
		return;
	}

	if (!str_cmp(arg2,"security")){
		if (IS_NPC(ch)){
			ch->send("Si, claro.\n\r");
			return;
		}

		if (IS_NPC(victim)){
			ch->send("Not on NPC's.\n\r");
			return;
		}

		if (value > ch->pcdata->security || value < 0){
			if (ch->pcdata->security != 0)
				printf_to_char(ch,"Valid security is 0-%d.\n\r",ch->pcdata->security );
			else
				send_to_char("Valid security is 0 only.\n\r",ch);
			return;
		}
		victim->pcdata->security = value;
		return;
	}

	if (!str_cmp(arg2,"trains")){
		if (IS_NPC(ch)){
			ch->send("Si, claro.\n\r");
			return;
		}

		if (IS_NPC(victim)){
			ch->send("Not on NPC's.\n\r");
			return;
		}

		if (value > 120 || value < 0){
			send_to_char("Valid trains are 0 to 120 only.\n\r",ch);
			return;
		}
		victim->pcdata->trains = value;
		return;
	}
	if (!str_cmp(arg2,"skillpoints")){
		if (IS_NPC(ch)){
			ch->send("Si, claro.\n\r");
			return;
		}

		if (IS_NPC(victim)){
			ch->send("Not on NPC's.\n\r");
			return;
		}

		if (value > 120 || value < 0){
			send_to_char("Valid skillpoints are 0 to 120 only.\n\r",ch);
			return;
		}
		victim->pcdata->studies = value;
		return;
	}

	if (!str_prefix(arg2,"eye")){
		mod_attribute(ch,victim,P_EYE,str_dup(arg3),str_dup("eye color"),eye_table);
		return;
	}
	if (!str_prefix(arg2,"hair")){
		mod_attribute(ch,victim,P_HAIR,str_dup(arg3),str_dup("hair color"),hair_table);
		return;
	}
	if (!str_prefix(arg2,"subhair")){
		mod_attribute(ch,victim,P_SHAIR,str_dup(arg3),str_dup("hair style"),sub_hair_table);
		return;
	}
	if (!str_prefix(arg2,"weight")){
		mod_attribute(ch,victim,P_WEIGHT,str_dup(arg3),str_dup("weight"),weight_table);
		return;
	}
	if (!str_prefix(arg2,"height")){
		mod_attribute(ch,victim,P_HEIGHT,str_dup(arg3),str_dup("height"),height_table);
		return;
	}

	if (!str_prefix(arg2,"exp")){
		if (IS_NPC(victim)){
			ch->send("Not to NPC's.\n\r");
			return;
		}
		victim->exp = value;
		return;
	}
	if (!str_cmp(arg2,"class")){
		value = class_lookup(arg3);
		if (value < 0 || (!classes[value].active && ch->level != MAX_LEVEL))
			ch->send("That is not a valid class.\n\r");
		else{
			victim->pclass = value;
			printf_to_char(ch,"Changing %s's class to '%s'\n\r",PERS(victim,ch),classes[value].name);
		}
		return;
	}

	if (!str_prefix(arg2,"sex")){
		if (value < 0 || value > 2){
			ch->send("Sex range is 0 to 2.\n\r");
			return;
		}
		victim->sex = value;
		if (!IS_NPC(victim))
			victim->pcdata->true_sex = value;
		return;
	}

	if (!str_prefix(arg2,"level")){
		if (!IS_NPC(victim) && get_trust(ch) < HEADIMM+1){
			ch->send("Not on PC's.\n\r");
			return;
		}

		if (value < 0 || value > MAX_LEVEL){
			printf_to_char(ch, "Level range is 0 to %d.\n\r", MAX_LEVEL);
			return;
		}
		victim->level = value;
		return;
	}

	if (!str_prefix(arg2,"gold")){
		victim->gold = value;
		return;
	}

	if (!str_prefix(arg2,"silver")){
		victim->silver = value;
		return;
	}

	if (!str_prefix(arg2,"hp")){
		if (value < -10){// || value > 15000){
			ch->send("Hp range is -10 to 15,000 hit points.\n\r");
			return;
		}
		victim->max_hit = value;
		if (!IS_NPC(victim))
			victim->pcdata->perm_hit = value;
		return;
	}

	if (!str_prefix(arg2,"mana")){
		if (value < 0 || value > 15000){
			ch->send("Mana range is 0 to 15,000 mana points.\n\r");
			return;
		}
		victim->settruemaxmana(value);
		if (!IS_NPC(victim))
			victim->pcdata->perm_mana = value;
		return;
	}

	if (!str_prefix(arg2,"antimana")){
		if (value < 0 || value > 15000){
			ch->send("AntiMana range is 0 to 15,000 antimana points.\n\r");
			return;
		}
		victim->settruemaxantimana(value);
		if (!IS_NPC(victim))
			victim->pcdata->perm_antimana = value;
		return;
	}

	if (!str_prefix(arg2,"move")){
		if (value < 0 || value > 10000){
			ch->send("Move range is 0 to 10,000 move points.\n\r");
			return;
		}
		victim->max_move = value;
		if (!IS_NPC(victim))
			victim->pcdata->perm_move = value;
		return;
	}
	if (!str_prefix(arg2,"align")){
		if (value < -1000 || value > 1000)
			ch->send("Alignment range is -1000 to 1000.\n\r");
		else
			victim->alignment = value;
		return;
	}
	if (!str_prefix(arg2,"thirst")){
		if (IS_NPC(victim))
			ch->send("Not on NPC's.\n\r");
		else if (value < -1 || value > 100)
			ch->send("Thirst range is -1 to 100.\n\r");
		else
			victim->pcdata->condition[COND_THIRST] = value;
		return;
	}
	if (!str_prefix(arg2,"drunk")){
		if (IS_NPC(victim))
			ch->send("Not on NPC's.\n\r");
		else if (value < -1 || value > 100)
			ch->send("Drunk range is -1 to 100.\n\r");
		else
			victim->pcdata->condition[COND_DRUNK] = value;
		return;
	}
	if (!str_prefix(arg2,"full")){
		if (IS_NPC(victim))
			ch->send("Not on NPC's.\n\r");
		else if (value < -1 || value > 100)
			ch->send("Full range is -1 to 100.\n\r");
		else
			victim->pcdata->condition[COND_FULL] = value;
		return;
	}
	if (!str_prefix(arg2,"hunger")){
		if (IS_NPC(victim))
			ch->send("Not on NPC's.\n\r");
		else if (value < -1 || value > 100)
			ch->send("Full range is -1 to 100.\n\r");
		else
			victim->pcdata->condition[COND_HUNGER] = value;
		return;
	}
	if (!str_prefix(arg2,"race")){
		int race;
		race = race_lookup(arg3);
		if ( race == 0)
			ch->send("That is not a valid race.\n\r");
		else if (!IS_NPC(victim) && !race_table[race].pc_race)
			ch->send("That is not a valid player race.\n\r");
		else
			victim->race = race;
		return;
	}

	if (!str_prefix(arg2,"lastname")){
		if (IS_NPC(victim))
			ch->send("Not on NPC's.\n\r");
		else{
			ch->send("Changed their last name.\n\r");
			victim->pcdata->lname = str_dup(arg3);
			strcat(victim->pcdata->lname,"{x");
		}
		return;
	}

	if (!str_prefix(arg2,"group")){
		if (!IS_NPC(victim))
			ch->send("Only on NPCs.\n\r");
		else
			victim->group = value;
		return;
	}

	for (int i = 0; stat_flags[i].name; i++){
		if (!str_prefix(arg2,stat_flags[i].name)){
			if (value < 1 || value > get_max_train(victim,stat_flags[i].bit)){
				printf_to_char(ch,"Stat range is 1 to %d\n\r.",get_max_train(victim,stat_flags[i].bit));
				return;
			}
			printf_to_char(ch,"Changing %s's %s to %d.\n\r",victim->name,stat_flags[i].name,value);
			victim->perm_stat[stat_flags[i].bit] = value;
			return;
		}
	}
	do_function(ch,&do_mset,"");
}

void do_string(CHAR_DATA *ch,char *argument){
	char type[MIL],arg1[MIL],arg2[MIL],arg3[MIL];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	smash_tilde(argument);
	argument = one_argument(argument,type);
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	strcpy(arg3,argument);

	if (!type[0] || !arg1[0] || !arg2[0] || !arg3[0])
	{
		ch->send("Syntax:\n\r");
		ch->send("  string char <{cname{x> <{cfield{x> <{cstring{x>\n\r");
		ch->send("    fields: {xname short long desc title spec{x\n\r");
		ch->send("  string obj  <{gname{x> <{gfield{x> <{gstring{x>\n\r");
		ch->send("    fields: {gname short long extended{x\n\r");
		return;
	}

	if (!str_prefix(type,"character") || !str_prefix(type,"mobile")){
		if (!(victim = get_char_world(ch,arg1))){
			ch->send("They aren't here.\n\r");
			return;
		}
		if(!IS_NPC(victim) && get_trust(victim) >= get_trust(ch)){
			ch->send("They're too powerful to mess with..\n\r");
			return;
		}

		victim->zone = NULL;

		if (!str_prefix(arg2,"name")){
			if (!IS_NPC(victim)){
	    		ch->send("Not on PC's.\n\r");
	    		return;
			}
			free_string(victim->name);
			strcat(arg3,"{x");
			victim->name = str_dup(arg3);
			return;
		}
    
		if (!str_prefix(arg2,"description")){
    		free_string(victim->description);
    		victim->description = str_dup(arg3);
    		return;
		}

		if (!str_prefix(arg2,"short")){
			free_string(victim->short_descr);
			strcat(arg3,"{x");
			victim->short_descr = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2,"long")){
			free_string(victim->long_descr);
			strcat(arg3,"\n\r{x");
			victim->long_descr = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2,"title")){
			if (IS_NPC(victim)){
	    		ch->send("Not on NPC's.\n\r");
	    		return;
			}

			set_title(victim,arg3);
			return;
		}

		if (!str_prefix(arg2,"spec")){
			if (!IS_NPC(victim)){
	    		ch->send("Not on PC's.\n\r");
	    		return;
			}

			if ((victim->spec_fun = spec_lookup(arg3)) == 0){
	    		ch->send("No such spec fun.\n\r");
	    		return;
			}
			return;
		}
	}

	if (!str_prefix(type,"object")){
		if (!(obj = get_obj_world(ch,arg1))){
			ch->send("Nothing like that in heaven or earth.\n\r");
			return;
		}
		if (!str_prefix(arg2,"name")){
			free_string(obj->name);
			strcat(arg3,"{x");
			obj->name = str_dup(arg3);
			return;
		}
		if (!str_prefix(arg2,"short")){
			free_string(obj->short_descr);
			strcat(arg3,"{x");
			obj->short_descr = str_dup(arg3);
			return;
		}
		if (!str_prefix(arg2,"long")){
			free_string(obj->description);
			strcat(arg3,"{x");
			obj->description = str_dup(arg3);
			return;
		}
		if (!str_prefix(arg2,"ed") || !str_prefix(arg2,"extended")){
			EXTRA_DESCR_DATA *ed;
			argument = one_argument(argument,arg3);
			if (argument){
	    		ch->send("Syntax: oset <object> ed <keyword> <string>\n\r");
	    		return;
			}
 			strcat(argument,"\n\r");
			ed				 = new_extra_descr();
			ed->keyword		 = str_dup(arg3);
			ed->description	 = str_dup(argument);
			ed->next		 = obj->extra_descr;
			obj->extra_descr = ed;
			return;
		}
	}
	do_function(ch,&do_string,"");
}

void do_oset(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL],arg3[MIL];
	OBJ_DATA *obj;
	int value;

	smash_tilde(argument);
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	strcpy(arg3,argument);

	if (!arg1[0] || !arg2[0] || !arg3[0]){
		ch->send("Syntax:\n\r");
		ch->send("  set obj <object> <field> <value>\n\r");
		ch->send("  Field being one of:\n\r");
		ch->send("    value0 value1 value2 value3 value4 (v1-v4)\n\r");
		ch->send("    extra wear level weight cost timer\n\r");
		return;
	}

	if (!(obj = get_obj_world(ch,arg1))){
		ch->send("Nothing like that in heaven or earth.\n\r");
		return;
	}

	if (!str_prefix(arg2,"extra"))
		return;
	value = atoi(arg3);

	if (!str_cmp(arg2,"value0") || !str_cmp(arg2,"v0")){
		if(obj->item_type == ITEM_WEAPON && value > 40 && ch->level != MAX_LEVEL)
			return;
		obj->value[0] = value;
		return;
	}

	if (!str_cmp(arg2,"value1") || !str_cmp(arg2,"v1")){
		if(obj->item_type == ITEM_WEAPON && value > 40 && ch->level != MAX_LEVEL)
			return;
		obj->value[1] = value;
		return;
	}

	if (!str_cmp(arg2,"value2") || !str_cmp(arg2,"v2")){
		if(obj->item_type == ITEM_WEAPON && value > 40 && ch->level != MAX_LEVEL)
			return;
		obj->value[2] = value;
		return;
	}

	if (!str_cmp(arg2,"value3") || !str_cmp(arg2,"v3")){
		if(obj->item_type == ITEM_WEAPON && value > 40 && ch->level != MAX_LEVEL)
			return;
		obj->value[3] = value;
		return;
	}

	if (!str_cmp(arg2,"value4") || !str_cmp(arg2,"v4")){
		if(obj->item_type == ITEM_WEAPON && value > 40 && ch->level != MAX_LEVEL)
			return;
		obj->value[4] = value;
		return;
	}

	if (!str_prefix(arg2,"wear")){
		obj->wear_flags = value;
		return;
	}

	if (!str_prefix(arg2,"level")){
		obj->level = value;
		return;
	}

	if (!str_prefix(arg2,"weight")){
		obj->weight = value;
		return;
	}

	if (!str_prefix(arg2,"cost")){
		obj->cost = value;
		return;
	}

	if (!str_prefix(arg2,"timer")){
		obj->timer = value;
		return;
	}
	do_function(ch,&do_oset,"");
}

void do_rset( CHAR_DATA *ch, char *argument )
{
	char arg1[MIL],arg2[MIL],arg3[MIL];
	ROOM_INDEX_DATA *location;
	int value;

	smash_tilde(argument);
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	strcpy(arg3,argument);

	if (!arg1[0] || !arg2[0] || !arg3[0]){
		ch->send("Syntax:\n\r");
		ch->send("  set room <location> <field> <value>\n\r");
		ch->send("  Field being one of:\n\r");
		ch->send("    flags sector\n\r");
		return;
	}

	if (!(location = find_location(ch,arg1))){
		ch->send("No such location.\n\r");
		return;
	}

	if (!is_room_owner(ch,location) && ch->in_room != location && room_is_private(location) && !IS_TRUSTED(ch,ADMIN)){
		ch->send("That room is private right now.\n\r");
		return;
	}

	if (!is_number(arg3)){
		ch->send("Value must be numeric.\n\r");
		return;
	}
	value = atoi(arg3);

	if (!str_prefix(arg2,"flags")){
		location->room_flags = value;
		return;
	}

	if (!str_prefix(arg2,"sector")){
		location->sector_type = value;
		return;
	}
	do_function(ch,&do_rset,"");
}

void do_force(CHAR_DATA *ch,char *argument){
	char buf[MSL],arg[MIL],arg2[MIL];

	argument = one_argument(argument,arg);

	if (!arg[0] || !argument[0]){
		ch->send("Force whom to do what?\n\r");
		return;
	}

	one_argument(argument,arg2);

	if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob")){
		ch->send("That will NOT be done.\n\r");
		return;
	}

	sprintf(buf,"$n forces you to '%s'.",argument);

	if (!str_cmp(arg,"all")){
		CHAR_DATA *vch,*vch_next;

		if (get_trust(ch) < MAX_LEVEL-5){
			ch->send("Not at your level!\n\r");
			return;
		}

		for (vch = char_list;vch;vch = vch_next){
			vch_next = vch->next;

			if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch)){
				act(buf,ch,NULL,vch,TO_VICT);
				interpret(vch,argument);
			}
		}
	}
	else if (!str_cmp(arg,"players")){
		CHAR_DATA *vch,*vch_next;

		if (get_trust(ch) < MAX_LEVEL-5){
			ch->send("Not at your level!\n\r");
			return;
		}

		for (vch = char_list;vch;vch = vch_next){
			vch_next = vch->next;

			if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch) && vch->level < LEVEL_HERO){
				act(buf,ch,NULL,vch,TO_VICT);
				interpret(vch,argument);
			}
		}
	}
	else if (!str_cmp(arg,"gods")){
		CHAR_DATA *vch,*vch_next;

		if (get_trust(ch) < MAX_LEVEL-5){
			ch->send("Not at your level!\n\r");
			return;
		}

		for (vch = char_list;vch;vch = vch_next){
			vch_next = vch->next;

			if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch) && vch->level >= LEVEL_HERO){
				act(buf,ch,NULL,vch,TO_VICT);
				interpret(vch,argument);
			}
		}
	}
	else{
		CHAR_DATA *victim;

		if (!(victim = get_char_world(ch,arg))){
			ch->send("They aren't here.\n\r");
			return;
		}

		if (victim == ch){
			ch->send("Aye aye, right away!\n\r");
			return;
		}

		if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room && room_is_private(victim->in_room) && !IS_TRUSTED(ch,ADMIN)){
			ch->send("That character is in a private room.\n\r");
			return;
		}

		if (get_trust(victim) >= get_trust(ch)){
			ch->send("Do it yourself!\n\r");
			return;
		}

		if (!IS_NPC(victim) && get_trust(ch) < MAX_LEVEL-10){
			ch->send("Not at your level!\n\r");
			return;
		}

		act(buf,ch,NULL,victim,TO_VICT);
		interpret(victim,argument);
	}
	send_to_char("Ok.\n\r",ch);
}

void do_invis(CHAR_DATA *ch,char *argument){
	int level;
	char arg[MSL];

	one_argument(argument,arg);

	if (!arg[0]){
		if (ch->invis_level){
			ch->invis_level = 0;
			act("$n slowly fades into existence.",ch,NULL,NULL,TO_ROOM);
			ch->send("You slowly fade back into existence.\n\r");
		}
		else{
			ch->invis_level = get_trust(ch);
			act("$n slowly fades into thin air.",ch,NULL,NULL,TO_ROOM);
			ch->send("You slowly vanish into thin air.\n\r");
		}
	}
	else{
		level = atoi(arg);
		if (level < 2 || level > get_trust(ch)){
			ch->send("Invis level must be between 2 and your level.\n\r");
			return;
		}
		else{
			ch->reply = NULL;
			ch->invis_level = level;
			act("$n slowly fades into thin air.",ch,NULL,NULL,TO_ROOM);
			ch->send("You slowly vanish into thin air.\n\r");
		}
	}
}

void do_incognito(CHAR_DATA *ch,char *argument){
	int level;
	char arg[MSL];

	one_argument(argument,arg);

	if (!arg[0]){
		if (ch->incog_level){
			ch->incog_level = 0;
			act("$n is no longer cloaked.",ch,NULL,NULL,TO_ROOM);
			ch->send("You are no longer cloaked.\n\r");
		}
		else{
			ch->incog_level = get_trust(ch);
			act("$n cloaks $s presence.",ch,NULL,NULL,TO_ROOM);
			ch->send("You cloak your presence.\n\r");
		}
	}
	else{
		level = atoi(arg);
		if (level < 2 || level > get_trust(ch)){
			ch->send("Incog level must be between 2 and your level.\n\r");
			return;
		}
		else{
			ch->reply = NULL;
			ch->incog_level = level;
			act("$n cloaks $s presence.",ch,NULL,NULL,TO_ROOM);
			ch->send("You cloak your presence.\n\r");
		}
	}
}

void do_holylight( CHAR_DATA *ch, char *argument ){
	if (IS_NPC(ch))
		return;

	if (!ch->setplr(PL_HOLYLIGHT)){
		ch->remplr(PL_HOLYLIGHT);
		ch->send("Holy light mode off.\n\r");
	}
	else
		ch->send("Holy light mode on.\n\r");
}

void do_prefi (CHAR_DATA *ch, char *argument){
	ch->send("You cannot abbreviate the prefix command.\r\n");
	return;
}

void do_prefix (CHAR_DATA *ch, char *argument){
	if (!argument[0]){
		if (!ch->prefix[0]){
			ch->send("You have no prefix to clear.\r\n");
			return;
		}

		ch->send("Prefix removed.\r\n");
		free_string(ch->prefix);
		ch->prefix = str_dup("");
		return;
	}

	if (ch->prefix[0]){
		printf_to_char(ch,"Prefix changed to %s.\r\n",argument);
		free_string(ch->prefix);
	}
	else
		printf_to_char(ch,"Prefix set to %s.\r\n",argument);
	ch->prefix = str_dup(argument);
}

#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)
#define COPYOVER_FILE "copyover.data"
#define EXE_FILE	  "../src/wake"

void do_copyover(CHAR_DATA *ch,char * argument){
	FILE *fp, *fpObj, *fp2;
	DESCRIPTOR_DATA *d, *d_next;
	char buf[100], buf2[100],buf3[100],buf4[100];
	extern int port,control;
	bool has_reset=false;
	OBJ_DATA *obj;
	RESET_DATA *pReset;
	buf[0] = buf2[0] = buf3[0] = '\0';

	fp = fopen(COPYOVER_FILE,"w");

	if (!fp){
		ch->send("Copyover file not writeable, aborted.\n\r");
		log_f("Could not write to copyover file: %s", COPYOVER_FILE);
		perror("do_copyover:fopen");
		return;
	}

	do_asave (NULL,"");

	mud.t_con = 0;

	sprintf(buf,"\n\r*Boon has destroyed the world, please be patient while %s rebuilds it again..\n\r",ch->name);

	for (d = descriptor_list;d;d = d_next){
		CHAR_DATA * och = CH(d);
		d_next = d->next;
		
		if (!d->character || d->connected > CON_PLAYING){ // drop those logging on
			write_to_descriptor(d->descriptor,"\n\rSorry, we are rebooting. Come back in a few minutes.\n\r",0);
			close_socket(d);
		}
		else{
			fprintf(fp,"%d %s %s\n",d->descriptor,och->name,d->host);
			cql_save_char(och);
			d->character->remplr(PL_FORGING);
			write_to_descriptor(d->descriptor, buf, 0);
		}
	}

	fprintf(fp,"-1\n");
	fclose(fp);

	fpObj = fopen("../area/objcopy.txt","w");
	for (obj = object_list;obj;obj = obj->next){
		has_reset = false;
		if(obj->carried_by)
			continue;
		if(obj->item_type != ITEM_CORPSE_PC)
			continue;
		for (pReset = obj->in_room->reset_first;pReset;pReset = pReset->next){
			if(((pReset->command == 'O' || pReset->command == 'P') && pReset->arg3 == obj->pIndexData->vnum) || (pReset->command == 'E' && pReset->arg1 == obj->pIndexData->vnum)){
				has_reset = true;
				break;
			}
		}
		if(has_reset)
			continue;
		fwrite_obj(NULL,obj,fpObj,0);
	}
	fprintf(fpObj, "#END\n");
	fclose(fpObj);

	/* Close reserve and other always-open files and release other resources */	
	fclose (fpReserve);

#ifdef IMC
   imc_hotboot();
#endif
	sprintf(buf,"%d",port);
	sprintf(buf2,"%d",control);
#ifdef IMC
   if( this_imcmud )
      snprintf( buf4, 100, "%d", this_imcmud->desc );
   else
      strncpy( buf4, "-1", 100 );
#else
   strncpy( buf4, "-1", 100 );
#endif

   execl( EXE_FILE, "wake", buf, "copyover",  buf2, buf4, (char *)NULL );

	//execl(EXE_FILE,"wake",buf,"copyover",buf2,(char *)NULL);

	/* Failed - sucessful exec will not return */
	perror("do_copyover: execl");
	global_message(0,MAX_LEVEL,"Copyover has {RFAILED{x!!!\n\r",0);
	/* Here you might want to reopen fpReserve */
	fpReserve = fopen(NULL_FILE,"r");
}

void copyover_recover (){
	DESCRIPTOR_DATA *d;
	FILE *fp,*fp2;
	char name [100],host[MSL];
	int desc;
	bool fOld;
	int count=0;

	log_f("Copyover recovery initiated");

	fp = fopen (COPYOVER_FILE,"r");

	if (!fp){/* there are some descriptors open which will hang forever then ? */
		perror("copyover_recover:fopen");
		log_f("Copyover file not found. Exitting.\n\r");
		exit(1);
	}

	unlink (COPYOVER_FILE);/* In case something crashes - doesn't prevent reading	*/

	for (;;){
		fscanf(fp,"%d %s %s\n",&desc,name,host);
		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */
		if (!write_to_descriptor(desc,"\n\rRestoring from copyover...\n\r",0)){
			close(desc);/* nope */
			continue;
		}

		d = new_descriptor();
		d->descriptor = desc;
		d->host = str_dup(host);
		d->next = descriptor_list;
		descriptor_list = d;
		d->connected = CON_COPYOVER_RECOVER;/* -15, so close_socket frees the char */

		/* Now, find the pfile */
		fOld = cql_load_char(d,name);

		if (!fOld){ /* Player file not found?! */
			write_to_descriptor(desc,"\n\rSomehow, your character was lost in the copyover. Sorry.\n\r",0);
			close_socket(d);
		}
		else{ /* ok! */
			cql_load_char_obj(d->character);
			write_to_descriptor(desc,"\n\rCopyover recovery complete.\n\r",0);

			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index(ROOM_VNUM_CHAT);

			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;

			char_to_room(d->character,d->character->in_room);
			//nash init_race(d->character); //because copyover doesn't always read these
			do_look(d->character,"auto");
			check_penumbral(d->character);
			act("$n materializes!",d->character,NULL,NULL,TO_ROOM);
			for(DESCRIPTOR_DATA *dn = descriptor_list;dn;dn = dn->next){
				if(dn->connected != CON_PLAYING || dn->character == d->character)
					continue;
				CHAR_DATA *victim;

				victim = dn->original ? dn->original : dn->character;
				if(is_same_group(d->character,victim) && d->character->in_room != victim->in_room)
					act("$n has logged in, and is in your group.",d->character,NULL,victim,TO_VICT);
			}
			d->connected = CON_PLAYING;
			save_who();

			mud.t_con++;

			if (d->character->pet){
				char_to_room(d->character->pet,d->character->in_room);
				act("$n materializes!.",d->character->pet,NULL,NULL,TO_ROOM);
			}
			if (d->character->mount){
				char_to_room(d->character->mount,d->character->in_room);
				act("$n materializes!.",d->character->mount,NULL,NULL,TO_ROOM);
				add_follower(d->character->mount, d->character, false);
			}
		}
	}
	fclose(fp);
	load_copyover_obj();
}

void load_copyover_obj(void){
	FILE *fpObj;
	char letter,*word;

	if(!(fpObj = fopen("../area/objcopy.txt","r")))
		return;
	unlink("../area/objcopy.txt");//To prevent from char's doubling corpses.
	log_f("Loading pc corpses.");
	for(;;){	
		letter = fread_letter(fpObj);
		if (letter == '*'){
			fread_to_eol(fpObj);
			continue;
		}
		if (letter != '#'){
			bug( "Load_char_obj: # not found.", 0 );
			break;
		}
		log_f("Freading word.");
		word = fread_word(fpObj);
		if (!str_cmp(word,"OBJECT"))
			fread_obj(NULL,fpObj);
		else if (!str_cmp(word,"O"))
			fread_obj(NULL,fpObj);
		else if (!str_cmp(word,"END"))
			break;
		else{
			bug( "Load_copyover_obj: bad section.", 0 );
			break;
		}
	}
	fclose(fpObj);
}

void do_pload(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA d;
	bool isChar = false;
	char name[MIL];

	if (!argument[0]){
		ch->send("Load who?\n\r");
		return;
	}

	argument[0] = UPPER(argument[0]);
	argument = one_argument(argument, name);

	/* Dont want to load a second copy of a player who's allready online! */
	if (get_char_world(ch,name)){
		ch->send("That person is already connected!\n\r");
		return;
	}

	isChar = load_char_obj(&d,name);/* char pfile exists? */

	if (!isChar){
		ch->send("Load Who? Are you sure? I cant seem to find them.\n\r");
		return;
	}

	d.character->desc     = NULL;
	d.character->next     = char_list;
	char_list             = d.character;
	d.connected           = CON_PLAYING;
	reset_char(d.character);

	/* bring player to imm */
	if (d.character->in_room != NULL)
		char_to_room(d.character,ch->in_room);/* put in room imm is in */

	act("$n has pulled $N from the {dM{wa{dt{wr{di{wx{x!",ch,NULL,d.character,TO_ROOM);
	act("You pull $N from the {dM{wa{dt{wr{di{wx{x!",ch,NULL,d.character,TO_CHAR);

	if (d.character->pet){
		char_to_room(d.character->pet,d.character->in_room);
		act("$n has entered the game.",d.character->pet,NULL,NULL,TO_ROOM);
	}//nashneedstomakesuremountsload
}

void do_punload(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char who[MIL];

	argument = one_argument(argument,who);

	if (!(victim = get_char_world(ch,who))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (victim->desc){
		ch->send("I dont think that would be a good idea...\n\r");
		return;
	}

	if (victim->was_in_room){ /* return player and pet to orig room */
		char_to_room(victim,victim->was_in_room);
		if (victim->pet != NULL)
			char_to_room(victim->pet,victim->was_in_room);
	}

	act("$n has released $N back to the {dM{wa{dt{wr{di{wx{x.",ch,NULL,victim,TO_ROOM);
	act("You release $N back to the {dM{wa{dt{wr{di{wx{x.",ch,NULL,victim,TO_CHAR);

	cql_save_char(victim);
	do_quit(victim,"");
}

void do_rename(CHAR_DATA *ch, char *argument){
	CHAR_DATA *victim;
	FILE *fp;
	char strsave[MIL],*name,arg1[MIL],arg2[MIL],buf[MIL],playerfile[MIL];


	if (!IS_IMMORTAL(ch)){
		ch->send("You don't have the power to do that.\n\r");
		return;
	}

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if (!arg1[0]){
		ch->send("Rename who?\n\r");
		return;
	}
	if (!arg2[0]){
		ch->send("What should their new name be?\n\r");
		return;
	}

	arg2[0] = UPPER(arg2[0]);

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("They aren't connected.\n\r");
		return;
	}

	if (IS_NPC(victim)){
		ch->send("Use string for NPC's.\n\r");
		return;
	}

	if (!check_parse_name(arg2)){
		printf_to_char(ch,"The name {c%s{x is {Rnot allowed{x.\n\r",arg2);
		return;
	}

	sprintf(playerfile, "%s%s", PLAYER_DIR, capitalize(arg2));
	if ((fp = fopen(playerfile,"r"))){
		printf_to_char(ch,"There is already someone named %s.\n\r",capitalize(arg2));
		fclose(fp);
		return;
	}

	if ((victim->level >= ch->level) && (victim->level >= ch->trust)
	&&((ch->level < MAX_LEVEL-2) || (ch->trust < MAX_LEVEL-2))
	&&(ch != victim)){
		ch->send("I don't think that's a good idea.\n\r");
		return;
	}

	if (victim->position == POS_FIGHTING){
		ch->send("They are fighting right now.\n\r");
		return;
	}

	name = str_dup(victim->name);
	sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));
	arg2[0] = UPPER(arg2[0]);
	free_string(victim->name);
	victim->name = str_dup(arg2);
	cql_save_char(victim);
	unlink(strsave);
	if (victim != ch)
		printf_to_char(victim,"{YNOTICE: {xYou have been renamed to {c%s{x.\n\r",arg2);
	ch->send("Done.\n\r");
}

void do_doublexp(CHAR_DATA *ch,char *argument){
	char arg[MSL],arg1[MSL],buf[MSL];
	int amount;

	argument = one_argument(argument,arg);
	one_argument(argument,arg1);

	if (!arg[0] || (str_cmp(arg,"on") && !is_number(arg1))){
		ch->send("Syntax: doubles <on|off> <ticks>.\n\r");
		return;
	}

	if (!str_cmp(arg,"on")){
		if (!arg1[0] || !is_number(arg1)){
			ch->send("You need to apply the number of ticks.\n\r");
			return;
		}

		if (double_exp){
			ch->send("Double exp is already in affect!\n\r");
			return;
		}

		amount = atoi(arg1);

		if (amount < 0 || amount > 500){
			ch->send("Please choose an amount between 0 and 500.\n\r");
			return;
		}

		global_exp = amount;
		double_exp = true;
		sprintf(buf,"{C%s {xhas activated {G%d{x ticks of doubles!{x\n\r",ch->name,amount);
		global_message(0,MAX_LEVEL,buf,0);
		ch->send("Double exp is now on!\n\r");
		return;
	}                

	if (!str_cmp(arg,"off")){
		if (!double_exp){
			ch->send("Double exp is not on please turn it on first!\n\r");
			return;
		}
		double_exp = false;
		global_exp = 0;
		sprintf(buf,"{C%s {xhas {Rremoved {xdoubles!\n\r", ch->name);
		global_message(0,MAX_LEVEL,buf,0);
		ch->send("You have turned off double exp!\n\r");
	}
}

void do_sla(CHAR_DATA *ch,char *argument){ch->send("If you want to SLAY, spell it out.\n\r");}
void do_slay(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char arg[MIL];

	one_argument(argument,arg);
	if (!arg[0]){
		ch->send("Slay whom?\n\r");
		return;
	}

	if (!(victim = get_char_room(ch,NULL,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (ch == victim){
		ch->send("Suicide is a mortal sin.\n\r");
		return;
	}

	if (!IS_NPC(victim) && victim->level >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	act("You slay $M in cold blood!",ch,NULL,victim,TO_CHAR);
	act("$n slays you in cold blood!",ch,NULL,victim,TO_VICT);
	act("$n slays $N in cold blood!",ch,NULL,victim,TO_NOTVICT);
	raw_kill(ch,victim);
}

void do_smite(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char arg[MIL];

	one_argument(argument,arg);
	if (!arg[0]){
		ch->send("Smite who?\n\r");
		return;
	}

	if (!(victim = get_char_room(ch,NULL,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (ch == victim){
		ch->send("Don't be stupid.\n\r");
		return;
	}

	if (!IS_NPC(victim) && victim->level >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	act("You smite $M!",ch,NULL,victim,TO_CHAR);
	act("$n uses $s godlike powers to smite you!",ch,NULL,victim,TO_VICT);
	act("$n gives $N a holy-backhanding!",ch,NULL,victim,TO_NOTVICT);
	WAIT_STATE(victim,2000 * PULSE_VIOLENCE);
	victim->position = POS_RESTING;
	if (victim->hit >=20)
		victim->hit	-= number_range(victim->hit*.5,victim->hit*.75);
	else
		victim->hit = 1;
}

void do_setpk(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char arg[MSL];
	one_argument(argument,arg);
	if (!arg[0]){
		ch->send("SetPK whom?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't anywhere.\n\r");
		return;
	}

	if (ch == victim){
		ch->send("Don't be stupid.\n\r");
		return;
	}

	if (!IS_NPC(victim) && victim->level >= get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	sprintf(arg,"%s SETPK: %s",ch->name,victim->name);
	wiznet(arg,NULL,NULL,WZ_FLAGS,0,0);
	if (victim->setplr(PL_PK)){
		act("You set $N to be able to {RKILL{x!!",ch,NULL,victim,TO_CHAR);
		act("$n uses $s godlike powers to grant you playerkillingness!",ch,NULL,victim,TO_VICT);
		if (ch->in_room == victim->in_room)
			act("$n gives $N killing power!",ch,NULL,victim,TO_NOTVICT);
		else{
			act("$n gives $N killing power!",ch,NULL,victim,TO_ROOM);
			act("$N gives $n killing power!",victim,NULL,ch,TO_ROOM);
		}
	}
	else{
		act("You remove $N's ability to {dkill{x. :(",ch,NULL,victim,TO_CHAR);
		act("$n uses $s godlike powers to strip your playerkillingness!",ch,NULL,victim,TO_VICT);
		if (ch->in_room == victim->in_room)
			act("$n seizes $N killing power!",ch,NULL,victim,TO_NOTVICT);
		else{
			act("$n seizes $N killing power!",ch,NULL,victim,TO_ROOM);
			act("$N seizes $n killing power!",victim,NULL,ch,TO_ROOM);
		}
		victim->remplr(PL_PK);
	}
}

void do_forall(CHAR_DATA *ch,char *argument){
	char arg1[MSL],arg2[MSL];
	ROOM_INDEX_DATA *location, *original;
	DESCRIPTOR_DATA *d;
	OBJ_DATA *on;
	CHAR_DATA *wch;

	argument = one_argument( argument, arg1);

	if (!arg1[0] || !argument[0]){
		ch->send("Syntax: forall <newb/mortal/all> <action>.\n\r");
		return;
	}

	original = ch->in_room;
	on = ch->on;
	for (d = descriptor_list;d;d = d->next){
 		if (d->connected != CON_PLAYING)
			continue;
		wch = d->original ? d->original : d->character;
		if (wch == ch || d->connected != CON_PLAYING || (!str_prefix(arg1,"newb") && wch->level > 15) || (!str_prefix(arg1,"mortal") && wch->level > 102))
			continue;
		if ((location = find_location(ch,wch->name)) == NULL)
			continue;
		if (!is_room_owner(ch,location) && room_is_private(location) && get_trust(ch) < MAX_LEVEL-2)
			continue;

		char_from_room(ch);
		char_to_room(ch,location);
		strcpy(arg2,argument);
		strcat(arg2," ");
		strcat(arg2,wch->name);
		interpret(ch,arg2);
	}

	for (wch = char_list;wch;wch = wch->next)
		if (wch == ch){
			char_from_room(ch);
			char_to_room(ch,original);
			ch->on = on;
			break;
		}
}

void do_unlink(CHAR_DATA *ch,char *argument){
	CHAR_DATA *linkdead;
	CHAR_DATA *wch;
	CHAR_DATA *wch_next;
	bool found = false;
	int i = 0;

	if ( argument[0] == '\0' ){
		send_to_char( "Unlink <all> or Unlink <player>\n\r", ch );

		send_to_char( "\n\r{C================================================{x\n\r\n\r",ch );
		for ( wch = char_list; wch != NULL; wch = wch_next ){
			wch_next = wch->next;

			if (IS_NPC( wch ) )
				continue;

			if ( wch->desc == NULL ){
				i++;
				printf_to_char(ch," %-20s\n\r",wch->name);
			}
		}

		send_to_char( "\n\r{C================================================{x\n\r\n\r",ch );
		printf_to_char( ch, "The number of linkdead people on the mud is %d.\n\r", i );
		return;    
	}

	if  ( !str_cmp( argument, "all" ) ){
		for ( wch = char_list;  wch != NULL; wch = wch_next ){
			wch_next = wch->next;

			if ( IS_NPC( wch ) )
				continue;

			if ( wch->desc == NULL ){
				found = true;
				do_function( wch, &do_quit, "" );
			}
		} 

		if ( found )
			send_to_char( "Extracting all linkdead players.\n\r", ch );

		if ( !found )
		send_to_char( "No linkdead players were found!\n\r", ch );
		return;
	}

	if ((linkdead = get_char_world(ch,argument)) == NULL){
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if ( linkdead->desc != NULL ){
		send_to_char( "They aren't linkdead just use disconnect instead.\n\r", ch );
		return;
	}

	else{
		do_function( linkdead, &do_quit, "" );
		send_to_char("They have been booted.\n\r", ch);
		return;
	}
}

void do_gift(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char arg[MSL],arg2[MSL];
	int reward;

	argument = one_argument(argument,arg);
	one_argument(argument,arg2);

	if (!arg[0] || !arg2[0]){
		ch->send("Syntax: reward <person> <amount>\n\r");
		return;
	}
	if (!(victim = get_char_world(ch,arg))){
		ch->send("Who?\n\r");
		return;
	}

	if (!is_number(arg2)){
		ch->send("Argument2 must be numeric.\n\r");
		return;
	}
	if((reward = atoi(arg2)) > 5){
		ch->send("Only rewards of a maximum of five points per person is allowed.\n\r");
		return;
	}
	if(reward < 1 && ch->level < DADMIN){
		ch->send("You do not have the clearance to give negative rewards.\n\r");
		return;
	}
	victim->credits += atoi(arg2);
	printf_to_char(ch,"You have rewarded %s with %d game credits.\n\r",victim->name,atoi(arg2));
	printf_to_char(victim,"%s has rewarded you with %d game credits.\n\r",ch->name,atoi(arg2));
}

void do_noble(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char arg[MSL],arg2[MSL];

	argument = one_argument(argument,arg);
	one_argument(argument,arg2);

	if (!arg[0] || !arg2[0]){
		printf_to_char(ch," Num %20s %20s\n\r","Male title","Female title");
		ch->send("/-----------------------------------------------\\\n\r");
		for(int i = 0;i<MAX_NOBILITY;i++){
			printf_to_char(ch,"[{G%2d{x] {d%20s %20s{x  |\n\r",i,nobility_flags[i].name[1],nobility_flags[i].name[2]);
		}
		
		ch->send("\\-----------------------------------------------/\n\r");
		ch->send("Syntax: noble <person> <level>\n\r");
		return;
	}
	if (!(victim = get_char_world(ch,arg))){
		ch->send("Who?\n\r");
		return;
	}

	if (!is_number(arg2) || atoi(arg2) >= MAX_NOBILITY){
		ch->send("Argument2 must be numeric and less than the max.\n\r");
		return;
	}
	victim->nobility = atoi(arg2);
	printf_to_char(ch,"You bump %s's status in the world to %s.\n\r",victim->name,nobility_flags[victim->nobility].name[victim->sex]);
	printf_to_char(victim,"%s has upped your status in the world to %s.\n\r",ch->name,nobility_flags[victim->nobility].name[victim->sex]);
}

void do_scatter(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj,*obj2;
	char arg[MSL],arg2[MSL],arg3[MSL];
	ROOM_INDEX_DATA *room;

	if(!argument[0]){
		ch->send("Syntax: scatter <obj> <world/area/clone <world/area>/quantity <world/area>>\n\r");
		return;
	}
	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);

	printf_to_char(ch,"%s * %s * %s * %s\n\r",argument,arg,arg2,arg3);
	if(!(obj = get_obj_carry(ch,arg,ch))){
		ch->send("You have no such object.\n\r");
		return;
	}

	if (!obj_check(ch,obj)){
		ch->send("Your powers are not great enough for such a task.\n\r");
		return;
	}

	if(!str_prefix(arg2,"world")){
		if(!(room = get_random_room(ch)))
			ch->send("Failed room. Try again.\n\r");
		else{
			obj_from_char(obj);
			obj_to_room(obj,room);
			act("You sent $p to a random room in the world.",ch,obj,NULL,TO_CHAR);
		}
	}
	else if(!str_prefix(arg2,"area")){
		if(!(room = get_random_room_area(ch)))
			ch->send("Failed room. Try again.\n\r");
		else{
			obj_from_char(obj);
			obj_to_room(obj,room);
			act("You sent $p to a random room in the area.",ch,obj,NULL,TO_CHAR);
		}
	}
	else if(!str_prefix(arg2,"clone")){
		obj2 = create_object(obj->pIndexData,0);
		clone_object(obj,obj2);
		if(!obj2){
			ch->send("Buggy clone...\n\r");
			log_f("%s sucks at cloning %s",ch->name,obj->short_descr);
			return;
		}
		obj_to_char(obj2,ch);
		if(!str_prefix(arg3,"world")){
			if(!(room = get_random_room(ch)))
				ch->send("Failed room. Try again.\n\r");
			else{
				obj_from_char(obj2);
				obj_to_room(obj2,room);
				act("You sent a clone of $p to a random room in the world.",ch,obj2,NULL,TO_CHAR);
			}
		}
		else if(!str_prefix(arg3,"area")){
			if(!(room = get_random_room_area(ch)))
				ch->send("Failed room. Try again.\n\r");
			else{
				obj_from_char(obj2);
				obj_to_room(obj2,room);
				act("You sent a clone of $p to a random room in the area.",ch,obj2,NULL,TO_CHAR);
			}
		}
		else{
			ch->send("Syntax: scatter <obj> <world/area/clone <world/area>/quantity <world/area>>\n\r");
			return;
		}
	}
	else if(!str_prefix(arg2,"quantity")){
		if(!is_number(arg3)){
			ch->send("Needs numerical input.\n\r");
			return;
		}
		if(!str_prefix(argument,"world")){
			for(int i = atoi(arg3);i > 0;i--){
				if(!(room = get_random_room(ch)))
					ch->send("Failed room. Try again.\n\r");
				else{
					obj2 = create_object(obj->pIndexData,0);
					clone_object(obj,obj2);
					obj_to_char(obj2,ch);
					obj_from_char(obj2);
					obj_to_room(obj2,room);
					act("You sent a copy of $p to a random room in the world.",ch,obj2,NULL,TO_CHAR);
				}
			}
		}
		else if(!str_prefix(argument,"area")){
			for(int i = atoi(arg3);i > 0;i--){
				if(!(room = get_random_room_area(ch)))
					ch->send("Failed room. Try again.\n\r");
				else{
					obj2 = create_object(obj->pIndexData,0);
					clone_object(obj,obj2);
					obj_to_char(obj2,ch);
					obj_from_char(obj2);
					obj_to_room(obj2,room);
					act("You sent a copy of $p to a random room in the area.",ch,obj2,NULL,TO_CHAR);
				}
			}
		}
		else{
			ch->send("Syntax: scatter <obj> <world/area/clone <world/area>/quantity <world/area>>\n\r");
			return;
		}
	}
	else{
		ch->send("Syntax: scatter <obj> <world/area/clone <world/area>/quantity <world/area>>\n\r");
		return;
	}
}

void do_spellall(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d,*d_next;
	CHAR_DATA *victim;
	int sn,TARGET;

	if (!argument[0])
		KILLFUNCT(ch,"Cast which what where?\n\r");

	if ((sn = find_spell(ch,argument)) < 1 || skill_table[sn].spell_fun == spell_null || (!IS_NPC(ch) && (ch->level < skill_table[sn].skill_level[ch->pclass] || ch->pcdata->learned[sn] < 1))){
		ch->send("You don't know any spells of that name.\n\r");
		return;
	}

	if (ch->isaff(AF_SILENCE)){
		ch->send("You try to cast, but not even a sound comes out.\n\r");
		return;
	}

	TARGET = skill_table[sn].target;
	switch(TARGET){
		default:					bug("Do_cast: bad target for sn %d.",sn);return;
		case TAR_CHAR_SELF:			ch->send("You can't spellall yourself.");return;
		case TAR_OBJ_INV:			ch->send("You can't spellall objects.");return;
		case TAR_OBJ_EQ:			ch->send("You can't spellall objects.");return;
		case TAR_IGNORE:			ch->send("You can't spellall nadas.");return;
		case TAR_CHAR_OFFENSIVE:	break;
		case TAR_CHAR_DEFENSIVE:	break;
		case TAR_OBJ_CHAR_OFF:		break;
		case TAR_OBJ_CHAR_DEF:		break;
	}

	if (ch->position < skill_table[sn].minimum_position){
		send_to_char("You can't concentrate enough.\n\r",ch);
		return;
	}

	for(d = descriptor_list;d;d = d_next){
		d_next = d->next;
		victim = d->original ? d->original : d->character;
		if ( d->connected == CON_PLAYING && victim != ch && !IS_IMMORTAL(victim)){
			printf_to_char(victim,"%s casts %s on you.\n\r",ch->name,skill_table[sn].name);
			(*skill_table[sn].spell_fun)(sn,3 * ch->level/4,ch,victim,TARGET,ch->getslvl(sn));
		}
	}
}
