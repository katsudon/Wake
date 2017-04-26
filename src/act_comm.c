#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "quotes.h"

//Locals
void do_Qquote   ( CHAR_DATA* );
char *makedrunk(char*,CHAR_DATA*);
void save_who();
int social_lookup (const char *name);
void cql_delete_char(int);

void add_log(CHAR_DATA *ch,char *argument,int ctype){
	char buf[MIL];
	int i = -1;

	if(strlen(argument) > MIL - 25){
		ch->send("Cropping log of this message.\n\r");
		argument[MIL-25] = '\0';
	}
	if(ctype < 1 || ctype > MAX_CHAN)
		return;

	for(int i=23; i > -1; i--){
		if(mud.chan_log[ctype][i])
			mud.chan_log[ctype][i+1] = str_dup(mud.chan_log[ctype][i]);
	}
	sprintf(buf,chan_table[ctype].chan_pre,ch->name,argument);
	mud.chan_log[ctype][0] = str_dup(buf);
}

void do_delet(CHAR_DATA *ch,char *argument){send_to_char("You must type the full command to delete yourself.\n\r",ch);return;}
void do_delete(CHAR_DATA *ch,char *argument){
	char strsave[MIL];
	int d_id;

	if (IS_NPC(ch))
		return;

	if (ch->pcdata->confirm_delete){
		if (argument[0] != '\0'){
			send_to_char("Delete status removed.\n\r",ch);
			ch->pcdata->confirm_delete = false;
			return;
		}
		else{
    		sprintf(strsave,"%s%s",PLAYER_DIR,capitalize(ch->name));
			wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
			stop_fighting(ch,true);
			if(ch->id > 0)
				d_id = ch->id;
			else{
				log_f("delete Error: in deleting char %s",ch->name);
				ch->send("Or not...\n\r");
				return;
			}
			do_function(ch,&do_quit,"");
			cql_delete_char(d_id);
			return;
		}
	}

	if (argument[0] != '\0'){
		ch->send("Just type delete. No argument.\n\r");
		return;
	}

	ch->send("Type delete again to confirm this command.\n\r");
	ch->send("WARNING: this command is irreversible.\n\r");
	ch->send("Typing delete with an argument will undo delete status.\n\r");
	ch->pcdata->confirm_delete = true;
	wiznet("$N's riding the nuke!",ch,NULL,0,0,get_trust(ch));
}

void do_channels(CHAR_DATA *ch,char *argument){
	/* lists all channels and their status */
	send_to_char("   channel     status\n\r",ch);
	send_to_char("---------------------\n\r",ch);

	if (ch->iscomm(CM_BUSY))
		send_to_char("You are Busy.\n\r",ch);

	printf_to_char(ch,"Gossip         %s{x\n\r",ch->iscomm(CM_NOGOSSIP) ? "{ROFF" : "{GON");

	printf_to_char(ch,"Auction        %s{x\n\r",ch->iscomm(CM_NOAUCTION) ? "{ROFF" : "{GON");

	printf_to_char(ch,"Chat           %s{x\n\r",ch->iscomm(CM_NOCHAT) ? "{ROFF" : "{GON");

	printf_to_char(ch,"Q/A            %s{x\n\r",ch->iscomm(CM_NOQUESTION) ? "{ROFF" : "{GON");

	printf_to_char(ch,"Quote          %s{x\n\r",ch->iscomm(CM_NOQUOTE) ? "{ROFF" : "{GON");

	if(ch->iscomm(CM_NONEWBIE) || IS_IMMORTAL(ch) || ch->level <= NEWBIE)
	printf_to_char(ch,"Newbie Help    {GON{x\n\r");

	if (IS_IMMORTAL(ch))
	printf_to_char(ch,"Imm Channel    %s{x\n\r",ch->iscomm(CM_NOWIZ) ? "{ROFF" : "{GON");

	printf_to_char(ch,"Shouts         %s{x\n\r",ch->iscomm(CM_SHOUTSOFF) ? "{ROFF" : "{GON");

	printf_to_char(ch,"Tells          %s{x\n\r",ch->iscomm(CM_DEAF) ? "{ROFF" : "{GON");

	printf_to_char(ch,"Quiet mode     %s{x\n\r",ch->iscomm(CM_QUIET) ? "{ROFF" : "{GON");

	if (ch->iscomm(CM_AFK))
		send_to_char("You are {YAFK{x.\n\r",ch);

	if (ch->iscomm(CM_SNOOP_PROOF))
		send_to_char("You are immune to snooping.\n\r",ch);

	if (ch->lines != PAGELEN)
	{
		if (ch->lines)
			printf_to_char(ch,"You display {w%d {xlines of scroll.\n\r",ch->lines+2);
		else
			send_to_char("Scroll buffering is {Roff{x.\n\r",ch);
	}

	if (ch->prompt != NULL)
		printf_to_char(ch,"Your current prompt is: %s\n\r",ch->prompt);

	if (ch->iscomm(CM_NOSHOUT))
		ch->send("You cannot shout.\n\r");

	if (ch->iscomm(CM_NOTELL))
		ch->send("You cannot use tell.\n\r");

	if (ch->iscomm(CM_NOCHANNELS))
		ch->send("You cannot use channels.\n\r");

	if (ch->iscomm(CM_NOEMOTE))
		ch->send("You cannot show emotions.\n\r");
}

void do_deaf(CHAR_DATA *ch,char *argument){
	if (ch->setcomm(CM_DEAF))
		ch->send("From now on, you won't hear tells.\n\r");
	else{
		ch->send("You can now hear tells again.\n\r");
		ch->remcomm(CM_DEAF);
	}
}

void do_quiet(CHAR_DATA *ch,char *argument){
	if (ch->setcomm(CM_QUIET))
		ch->send("From now on, you will only hear says and emotes.\n\r");
	else{
		ch->send("Quiet mode removed.\n\r");
		ch->remcomm(CM_QUIET);
	}
}

void do_busy(CHAR_DATA *ch,char *argument){
	if (ch->setcomm(CM_BUSY))
		ch->send("You are now marked as busy.\n\r");
	else{
		if(buf_string(ch->pcdata->buffer)[0])
			ch->send("{RBUSY {xmode removed. Type '{Creplay{x' to see tells.\n\r");
		else
			ch->send("{RBUSY {xmode removed. No tells to replay.\n\r");
		ch->remcomm(CM_BUSY);
	}
}

void do_afk(CHAR_DATA *ch,char * argument){
	char buf[MSL];

	if(argument[0]){
		if(!str_prefix(argument,"clear")){
			free_string(ch->pcdata->afk);
			ch->send("Away message cleared.\n\r");
			return;
		}
		strcpy(buf,argument);
		free_string(ch->pcdata->afk);
		ch->pcdata->afk = str_dup(buf);
		printf_to_char(ch,"Away message set to: '{Y%s{x'\n\r",buf);
		return;
	}
	if (ch->setcomm(CM_AFK)){
		if(ch->pcdata->afk[0])
			ch->printf("You are now {YAFK {xwith the away message: '{Y%s{x'\n\r",ch->pcdata->afk);
		else
			ch->send("You are now in {YAFK {xmode. No away message put up.\n\r");
	}
	else{
		if(buf_string(ch->pcdata->buffer)[0])
			ch->send("{YAFK {xmode removed. Type '{Creplay{x' to see tells.\n\r");
		else
			ch->send("{YAFK {xmode removed. No tells to replay.\n\r");
		ch->remcomm(CM_AFK);
	}
}

void do_replay(CHAR_DATA *ch,char *argument){
	if(IS_NPC(ch)){
		send_to_char("You can't replay.\n\r",ch);
		return;
	}

	if(!buf_string(ch->pcdata->buffer)[0]){
		send_to_char("You have no tells to replay.\n\r",ch);
		return;
	}

	page_to_char(buf_string(ch->pcdata->buffer),ch);
	clear_buf(ch->pcdata->buffer);
}

void do_quote(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d;

	if(!argument[0]){
		if (ch->setcomm(CM_NOQUOTE))
			send_to_char("Quote channel is now {ROFF{x.\n\r",ch);
		else{
			send_to_char("Quote channel is now {GON{x.\n\r",ch);
			ch->remcomm(CM_NOQUOTE);
		}
	}
	else{
		if(ch->iscomm(CM_QUIET)){
			send_to_char("You must turn off quiet mode first.\n\r",ch);
			return;
		}

		if (ch->iscomm(CM_NOCHANNELS)){
			send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
			return;
		}

		ch->remcomm(CM_NOQUOTE);

		printf_to_char(ch,"You quote '{w%s{x'\n\r",argument);
		for(d = descriptor_list;d;d = d->next){
			CHAR_DATA *victim;

			victim = d->original ? d->original : d->character;

			if ( d->connected == CON_PLAYING && d->character != ch && !victim->iscomm(CM_NOQUOTE) && !victim->iscomm(CM_QUIET) )
				act_new( "$n quotes '{w$t{x'", ch,argument, d->character, TO_VICT,POS_SLEEPING );
		}
	}
}

void do_say(CHAR_DATA *ch,char *argument){
	CHAR_DATA *mob,*mob_next;
	OBJ_DATA *obj,*obj_next;

	if(ch->iscomm(CM_MORPH)){
		ch->send("Nyaaaaaarrr! rawr! *growl*!\n\r");
		return;
	}
	if (!argument[0]){
		send_to_char("Say what?\n\r",ch);
		return;
	}

	if (ch->isaff(AF_SILENCE)){
		ch->send("You can't even make a peep.\n\r");
		return;
	}

	argument = makedrunk(argument,ch);
	int slen = strlen(argument);
	if(argument[slen-1] == '!'){
		act("$n exclaims, '{G$T{x'",ch,NULL,argument,TO_ROOM);
		act("You exclaim, '{G$T{x'",ch,NULL,argument,TO_CHAR);
	}
	else if(argument[slen-1] == '?'){
		act("$n asks, '{G$T{x'",ch,NULL,argument,TO_ROOM);
		act("You ask, '{G$T{x'",ch,NULL,argument,TO_CHAR);
	}
	else{
		act("$n says, '{G$T{x'",ch,NULL,argument,TO_ROOM);
		act("You say, '{G$T{x'",ch,NULL,argument,TO_CHAR);
	}

	if (!IS_NPC(ch)){
		for (mob = ch->in_room->people; mob != NULL; mob = mob_next){
			mob_next = mob->next_in_room;
			if (IS_NPC(mob) && HAS_TRIGGER_MOB(mob,TRIG_SPEECH) && mob->position == mob->pIndexData->default_pos)
				p_act_trigger(argument,mob,NULL,NULL,ch,NULL,NULL,TRIG_SPEECH);
			for (obj = mob->carrying; obj; obj = obj_next){
				obj_next = obj->next_content;
				if (HAS_TRIGGER_OBJ(obj,TRIG_SPEECH))
					p_act_trigger(argument,NULL,obj,NULL,ch,NULL,NULL,TRIG_SPEECH);
			}
		}
		for (obj = ch->in_room->contents; obj; obj = obj_next){
			obj_next = obj->next_content;
			if (HAS_TRIGGER_OBJ(obj,TRIG_SPEECH))
				p_act_trigger(argument,NULL,obj,NULL,ch,NULL,NULL,TRIG_SPEECH);
		}

		if (HAS_TRIGGER_ROOM(ch->in_room,TRIG_SPEECH))
			p_act_trigger(argument,NULL,NULL,ch->in_room,ch,NULL,NULL,TRIG_SPEECH);
	}
	else{
		for (mob = ch->in_room->people; mob != NULL; mob = mob_next){
			mob_next = mob->next_in_room;
			if (IS_NPC(mob) && HAS_TRIGGER_MOB(mob,TRIG_SPEECH) && mob->position == mob->pIndexData->default_pos)
				p_act_trigger(argument,mob,NULL,NULL,ch,NULL,NULL,TRIG_SPEECH);
			for (obj = mob->carrying; obj; obj = obj_next){
				obj_next = obj->next_content;
				if (HAS_TRIGGER_OBJ(obj,TRIG_SPEECH))
					p_act_trigger(argument,NULL,obj,NULL,ch,NULL,NULL,TRIG_SPEECH);
			}
		}
	}

	DESCRIPTOR_DATA *d;
	CHAR_DATA *rch;
	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->connected == CON_PLAYING
		&& (rch = d->character) != ch
		&& rch->in_room != ch->in_room
		&& rch->pcdata->eavesdropping != -1
		&& rch->in_room->exit[rch->pcdata->eavesdropping]->u1.to_room == ch->in_room
		&& check_eavesdrop(rch)){
			if(argument[slen-1] == '!')
				act("From another room, $n exclaims, '{G$t{x'",ch,argument,rch,TO_VICT);
			else if(argument[slen-1] == '?')
				act("From another room, $n asks, '{G$t{x'",ch,argument,rch,TO_VICT);
			else
				act("From another room, $n says '{G$t{x'",ch,argument,rch,TO_VICT);
		}
}

void do_osay(CHAR_DATA * ch,char *argument){
	CHAR_DATA *vch;

	if (!argument[0]){
		if (ch->setcomm(CM_NOOOC))
			send_to_char("Osay is {ROFF{x.\n\r",ch);
		else{
			send_to_char("Osay {GON{x.\n\r",ch);
			ch->remcomm(CM_NOOOC);
		}
		return;
	}
	ch->remcomm(CM_NOOOC);
	for (vch = ch->in_room->people;vch;vch = vch->next_in_room){
		if (vch == ch)
			act ("You say oocly '{C$T{x'", ch, NULL, argument, TO_CHAR);

		if (vch != ch && !vch->iscomm(CM_NOOOC))
			act("$n says oocly, '{C$t{x'",ch,argument,vch,TO_VICT);
	}
}
void do_lifeline(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d;

	if(get_class(ch) != CLASS_PRIEST && !IS_IMMORTAL(ch)){
		ch->send("You cannot do this.\n\r");
		return;
	}
	if(!argument[0]){
		if (ch->setcomm(CM_LIFELINE))
			send_to_char("You can hear deaths again.\n\r",ch);
		else{
			send_to_char("You will no longer be able to divine the dying.\n\r",ch);
			ch->remcomm(CM_LIFELINE);
		}
		return;
	}
}
void do_nonotes(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d;

	if (!argument[0]){
		if (ch->setcomm(CM_NONOTES))
			send_to_char("You will no longer hear note posting notifications.\n\r",ch);
		else{
			send_to_char("You will now hear note posting notifications.\n\r",ch);
			ch->remcomm(CM_NONOTES);
		}
		return;
	}
}

void do_shout(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d;

	if (!argument[0]){
		if (ch->setcomm(CM_SHOUTSOFF))
			send_to_char("You will no longer hear shouts.\n\r",ch);
		else{
			send_to_char("You can hear shouts again.\n\r",ch);
			ch->remcomm(CM_SHOUTSOFF);
		}
		return;
	}

	if (ch->iscomm(CM_NOSHOUT)){
		send_to_char( "You can't shout.\n\r", ch );
		return;
	}

	if (ch->isaff(AF_SILENCE)){
		ch->send("You can't even make a peep.\n\r");
		return;
	}

	ch->remcomm(CM_SHOUTSOFF);

	WAIT_STATE(ch,12);

	act( "You shout '{R$T{x'", ch, NULL, argument, TO_CHAR );
	for (d = descriptor_list;d;d = d->next){
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;

		if (d->connected == CON_PLAYING && d->character != ch && !victim->iscomm(CM_SHOUTSOFF) && !victim->iscomm(CM_QUIET))
			act("$n shouts '{R$t{x'",ch,argument,d->character,TO_VICT);
	}
}

void do_tell(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	if (ch->iscomm(CM_NOTELL) || ch->iscomm(CM_DEAF)){
		send_to_char( "Your message didn't get through.\n\r", ch );
		return;
	}

	if (ch->iscomm(CM_QUIET)){
		send_to_char( "You must turn off quiet mode first.\n\r", ch);
		return;
	}

	if (ch->iscomm(CM_DEAF)){
		send_to_char("You must turn off deaf mode first.\n\r",ch);
		return;
	}

	argument = one_argument(argument,arg);

	if (!arg[0] || !argument[0]){
		send_to_char( "Tell whom what?\n\r", ch );
		return;
	}

	if (!(victim = get_char_world(ch,arg)) || (IS_NPC(victim) && victim->in_room != ch->in_room)){
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if (!victim->desc && !IS_NPC(victim)){
		act("$N seems to have misplaced $S link...try again later.",ch,NULL,victim,TO_CHAR);
		sprintf(buf,"%s tells you '{Y%s{x'\n\r",PERS(ch,victim),argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

	if (!(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim)){
		act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if ((victim->iscomm(CM_QUIET) || victim->iscomm(CM_DEAF)) && !IS_IMMORTAL(ch)){
		act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  		return;
	}

	if (victim->iscomm(CM_BUSY)){
		if (IS_NPC (victim)){
			act ("$E is Busy, and not receiving tells.", ch, NULL, victim,TO_CHAR);
			return;
		}

		act ("$E is Busy, but your tell will go through when $E returns.",ch, NULL, victim, TO_CHAR);
		sprintf (buf, "%s tells you '%s'\n\r", PERS (ch, victim),argument);
		buf[0] = UPPER (buf[0]);
		add_buf (victim->pcdata->buffer, buf);
		act ("You tell $N '$t'", ch, argument, victim, TO_CHAR);
		return;
	}

	if (victim->iscomm(CM_AFK)){
		if (IS_NPC(victim)){
			act("$E is {YAFK{x, and not receiving tells.",ch,NULL,victim,TO_CHAR);
			return;
		}

		if(!victim->pcdata->afk[0])
			act("$E is {YAFK{x, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR);
		else
			act("$N[{YAFK{x] tells you '{Y$t{x'",ch,victim->pcdata->afk,victim,TO_CHAR);
		act ("You leave $N a message '{Y$t{x'", ch, argument, victim, TO_CHAR);
		sprintf(buf,"%s tells you '{Y%s{x'\n\r",PERS(ch,victim),argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

	ch->ltell = victim;
	act( "You tell $N '{Y$t{x'", ch, argument, victim, TO_CHAR );
	act_new("$n tells you '{Y$t{x'",ch,argument,victim,TO_VICT,POS_DEAD);
	victim->reply = ch;

	if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_SPEECH) )
		p_act_trigger( argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );
}

void do_rtell(CHAR_DATA *ch,char *argument){
    char buf[MSL];
    CHAR_DATA *victim;

    if ( ch->iscomm(CM_NOTELL) || ch->iscomm(CM_DEAF))
    {
		ch->send("Your message didn't get through.\n\r");
		return;
    }

    if (ch->iscomm(CM_QUIET))
    {
		ch->send("You must turn off quiet mode first.\n\r");
		return;
    }

    if (ch->iscomm(CM_DEAF))
    {
		ch->send("You must turn off deaf mode first.\n\r");
		return;
    }

    if (!argument[0])
    {
		ch->send("Tell who what?\n\r");
		return;
    }

    if (!(victim = ch->ltell) || (!victim->desc && !IS_NPC(victim)))
    {
		ch->send("There is no character buffered to the lasttell.\n\r");
		return;
    }

    if (!(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
		act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
    }
  
    if ((victim->iscomm(CM_QUIET) || victim->iscomm(CM_DEAF)) && !IS_IMMORTAL(ch))
    {
		act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  		return;
    }

    if (victim->iscomm(CM_BUSY))
    {
        if (IS_NPC (victim))
        {
            act ("$E is Busy, and not receiving tells.", ch, NULL, victim,TO_CHAR);
            return;
        }

        act ("$E is Busy, but your tell will go through when $E returns.",ch, NULL, victim, TO_CHAR);
        sprintf (buf, "%s tells you '%s'\n\r", PERS (ch, victim),argument);
        buf[0] = UPPER (buf[0]);
        add_buf (victim->pcdata->buffer, buf);
        act ("You tell $N '$t'", ch, argument, victim, TO_CHAR);
        return;
    }

    if (victim->iscomm(CM_AFK)){
		if (IS_NPC(victim)){
			act("$E is {YAFK{x, and not receiving tells.",ch,NULL,victim,TO_CHAR);
			return;
		}

		if(!victim->pcdata->afk[0])
			act("$E is {YAFK{x, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR);
		else
			act("$N[{YAFK{x] tells you '{Y$t'",ch,victim->pcdata->afk,victim,TO_CHAR);
		act ("You leave $N a message '{Y$t{x'", ch, argument, victim, TO_CHAR);
		sprintf(buf,"%s tells you '{Y%s{x'\n\r",PERS(ch,victim),argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
    }

    act( "You tell $N '{Y$t{x'", ch, argument, victim, TO_CHAR );
    act_new("$n tells you '{Y$t{x'",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply = ch;

    if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_SPEECH) )
		p_act_trigger( argument, victim, NULL, NULL, ch, NULL, NULL, TRIG_SPEECH );
}

void do_reply(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char buf[MSL],arg[MSL];

	if (ch->iscomm(CM_NOTELL)){
		send_to_char( "Your message didn't get through.\n\r", ch );
		return;
	}

	if (!(victim = ch->reply)){
		send_to_char( "They aren't here.\n\r", ch );
		return;
	}

	if (victim->desc == NULL && !IS_NPC(victim)){
		act("$N seems to have misplaced $S link...try again later.",ch,NULL,victim,TO_CHAR);
		sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

	if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)){
		act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if ((victim->iscomm(CM_QUIET) || victim->iscomm(CM_DEAF)) &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)){
		act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
		return;
	}

	if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch)){
		send_to_char( "In your dreams, or what?\n\r", ch );
		return;
	}

	if (victim->iscomm(CM_BUSY)){
		if (IS_NPC (victim)){
			act ("$E is Busy, and not receiving tells.", ch, NULL, victim,
				 TO_CHAR);
			return;
		}

		act ("$E is Busy, but your tell will go through when $E returns.", ch, NULL, victim, TO_CHAR);
		sprintf (buf, "%s tells you '%s'\n\r", PERS (ch, victim), argument);
		buf[0] = UPPER (buf[0]);
		add_buf (victim->pcdata->buffer, buf);
		act ("You tell $N '$t'", ch, argument, victim, TO_CHAR);
		act_new ("$n tells you '$t'", ch, argument, victim, TO_VICT, POS_DEAD);
		return;
	}

	if (victim->iscomm(CM_AFK)){
		if (IS_NPC(victim)){
			act_new("$E is {YAFK{x, and not receiving tells.",ch,NULL,victim,TO_CHAR,POS_DEAD);
			return;
		}

		if(!victim->pcdata->afk[0])
			act("$E is {YAFK, but your tell will go through when $E returns.", ch,NULL,victim,TO_CHAR);
		else
			act("$N[{YAFK{x] tells you '{Y$t{x'",ch,victim->pcdata->afk,victim,TO_CHAR);
		act ("You leave $N a message '{Y$t{x'", ch, argument, victim, TO_CHAR);
		sprintf(buf,"%s tells you '%s'\n\r",PERS(ch,victim),argument);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

	ch->ltell = victim;
	act_new("You tell $N '{Y$t{x'",ch,argument,victim,TO_CHAR,POS_DEAD);
	act_new("$n tells you '{Y$t{x'",ch,argument,victim,TO_VICT,POS_DEAD);
	victim->reply	= ch;
}

void do_yell(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d;

	if ( !IS_NPC(ch) && ch->iscomm(CM_NOSHOUT) ){
		send_to_char( "You can't yell.\n\r", ch );
		return;
	}

	if (ch->isaff(AF_SILENCE)){
		ch->send("You can't even make a peep.\n\r");
		return;
	}

	if ( argument[0] == '\0' ){
		send_to_char( "Yell what?\n\r", ch );
		return;
	}

	act("You yell '{R$t{x'",ch,argument,NULL,TO_CHAR);
	for ( d = descriptor_list; d != NULL; d = d->next )
		if ( d->connected == CON_PLAYING && d->character != ch && d->character->in_room != NULL && d->character->in_room->area == ch->in_room->area && !d->character->iscomm(CM_QUIET))
			act("$n yells '{R$t{x'",ch,argument,d->character,TO_VICT);
}

void do_emote(CHAR_DATA *ch,char *argument){
	char buf[MIL];
	if(ch->iscomm(CM_MORPH)){
		ch->send("Nyaaaaaarrr! rawr! *growl*!\n\r");
		return;
	}
	if(!IS_NPC(ch) && ch->iscomm(CM_NOEMOTE)){
		ch->send("You can't show your emotions.\n\r");
		return;
	}

	if(!argument[0]){
		ch->send("Emote what?\n\r");
		return;
	}

	if(!str_prefix("'s",argument))
		sprintf(buf,"%s",argument);
	else
		sprintf(buf," %s",argument);
	MOBtrigger = false;
	act("$n$T",ch,NULL,buf,TO_ROOM);
	act("$n$T",ch,NULL,buf,TO_CHAR);
	MOBtrigger = true;
}

void do_pmote(CHAR_DATA *ch,char *argument){
	CHAR_DATA *vch;
	char *letter,*name,last[MIL],temp[MSL];
	int matches = 0;

	if ( !IS_NPC(ch) && ch->iscomm(CM_NOEMOTE) ){
		send_to_char( "You can't show your emotions.\n\r", ch );
		return;
	}

	if ( argument[0] == '\0' ){
		send_to_char( "Emote what?\n\r", ch );
		return;
	}

	act( "$n $t", ch, argument, NULL, TO_CHAR );

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room){
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument,vch->name)) == NULL){
			MOBtrigger = false;
			act("$N $t",vch,argument,ch,TO_CHAR);
			MOBtrigger = true;
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

			if (*letter == *name)
			{
				matches++;
				name++;
				if (matches == strlen_color(vch->name))
				{
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

		MOBtrigger = false;
		act("$N $t",vch,temp,ch,TO_CHAR);
		MOBtrigger = true;
	}
}

void do_bug(CHAR_DATA *ch,char *argument){
    append_file(ch,BUG_FILE,argument);
    send_to_char("Bug logged.\n\r",ch);
}

void do_typo(CHAR_DATA *ch,char *argument){
    append_file(ch,TYPO_FILE,argument);
    send_to_char("Typo logged.\n\r",ch);
}

void do_rent(CHAR_DATA *ch,char *argument){
    send_to_char("There is no rent here.  Just save and quit.\n\r",ch);
}

void do_qui(CHAR_DATA *ch,char *argument){
    send_to_char("If you want to QUIT, you have to spell it out.\n\r",ch);
}

void do_quit(CHAR_DATA *ch,char *argument){
	DESCRIPTOR_DATA *d,*d_next;
	int id;
    FILE *fp;

	if (IS_NPC(ch))
		return;

	if (ch->isplr(PL_ARENA)){
		send_to_char("No way! You are in a duel.\n\r",ch);
		return;
	}

	if (ch->position == POS_FIGHTING){
		send_to_char("No way! You are fighting.\n\r",ch);
		return;
	}

	if (ch->position < POS_STUNNED){
		send_to_char("You're not DEAD yet.\n\r",ch);
		return;
	}
	if(ch->pet && ch->pet->isoff(OF_FALCON) && ch->pet->in_room != ch->in_room){
		ch->pet->remoff(OF_WAIT);
		ch->pet->remaff(AF_HIDE);
		char_from_room(ch->pet);
		char_to_room(ch->pet,ch->in_room);
		act("$n comes soaring in.",ch->pet,NULL,NULL,TO_ROOM);
	}
	ch->remplr(PL_FORGING);
	if (ch->pcdata->forge){
		REMOVE_BIT(ch->pcdata->forge->extra_flags,ITM_FORGING);
		ch->pcdata->forge->value[4] = -1;
	}
	do_Qquote(ch);

	if(ch->desc)
		mud.t_con--;

	act("$n has left the game.",ch,NULL,NULL,TO_ROOM);
	//ccon--;
	sprintf(log_buf,"%s has quit.",ch->name);
	log_string(log_buf);
	wiznet("$N rejoins the spooty real world.",ch,NULL,WZ_LOGINS,0,get_trust(ch));
	save_who();
	for(DESCRIPTOR_DATA *dn = descriptor_list;dn;dn = dn->next){
		if(dn->connected != CON_PLAYING || dn->character == ch)
			continue;
		CHAR_DATA *victim;

		victim = dn->original ? dn->original : dn->character;
		if(is_same_group(ch,victim))
		act("Groupmate $n has logged out.",ch,NULL,victim,TO_VICT);
	}
	if (ch->guarding)
		do_guard(ch,str_dup(ch->name));

	/*
	 * After extract_char the ch is no longer valid!
	 */
	cql_save_char(ch);
	// Free note that might be there somehow
	if (ch->pcdata->in_progress)
		free_note(ch->pcdata->in_progress);

	id = ch->id;
	d = ch->desc;
	extract_char(ch,true);
	if ( d != NULL )
		close_socket(d);

	/* toast evil cheating bastards */
	for (d = descriptor_list; d != NULL; d = d_next){
		CHAR_DATA *tch;

		d_next = d->next;
		tch = d->original ? d->original : d->character;
		if (tch && tch->id == id){
			extract_char(tch,true);
			close_socket(d);
		} 
	}
}

void do_save(CHAR_DATA *ch,char *argument){
	if (IS_NPC(ch))
		return;

	cql_save_char(ch);
	ch->send("Saving. Remember that ROM has automatic saving now.\n\r");
	WAIT_STATE(ch,4 * PULSE_VIOLENCE);
}

void do_color(CHAR_DATA *ch,char *argument){
	char arg[MSL];

	argument = one_argument(argument,arg);

	if(!*arg){
		if(ch->setplr(PL_COLOR))
			ch->send("{bC{ro{yl{co{gr{x is now {rON{x, Way Cool!\n\r");
		else{
			send_to_char_bw("Color is now OFF, <sigh>\n\r",ch);
			ch->remplr(PL_COLOR);
		}
	}
	else
		send_to_char_bw("Color Configuration is unavailable in this version of color, sorry\n\r",ch);
}

void do_sayto( CHAR_DATA *ch, char *argument ){
	char arg[MIL];
    CHAR_DATA *victim;

    argument = one_argument(argument,arg);

	if(ch->iscomm(CM_MORPH)){
		ch->send("Nyaaaaaarrr! rawr! *growl*!\n\r");
		return;
	}
	if (arg[0] == '\0' || argument[0] == '\0'){
	    send_to_char("Say what to whom?\n\r",ch);
	    return;
	}

	argument = makedrunk(argument,ch);
	if ((victim = get_char_world(ch,arg)) == NULL || victim->in_room != ch->in_room){
		send_to_char("They are not here.\n\r",ch);
		return;
	}
	else{
		int slen = strlen(argument);
		if(argument[slen-1] == '!'){
			act("You exclaim to $N '{G$t{x'",ch,argument,victim,TO_CHAR);
			act("$n exclaims to you '{G$t{x'",ch,argument,victim,TO_VICT);
			act("$n exclaims to $N '{G$t{x'",ch,argument,victim,TO_NOTVICT);
		}
		else if(argument[slen-1] == '?'){
			act("You ask $N '{G$t{x'",ch,argument,victim,TO_CHAR);
			act("$n asks you '{G$t{x'",ch,argument,victim,TO_VICT);
			act("$n asks $N '{G$t{x'",ch,argument,victim,TO_NOTVICT);
		}
		else{
			act("You say to $N '{G$t{x'",ch,argument,victim,TO_CHAR);
			act("$n says to you '{G$t{x'",ch,argument,victim,TO_VICT);
			act("$n says to $N '{G$t{x'",ch,argument,victim,TO_NOTVICT);
		}
	}

    if (!IS_NPC (ch)){
	    CHAR_DATA *mob, *mob_next;
	    for (mob = ch->in_room->people; mob != NULL; mob = mob_next){
		    mob_next = mob->next_in_room;
		    if (IS_NPC(mob) && HAS_TRIGGER_MOB(mob, TRIG_SPEECH) && mob->position == mob->pIndexData->default_pos)
				p_act_trigger(argument,mob,NULL,NULL,ch,NULL,NULL,TRIG_SPEECH);
		}
	}
	DESCRIPTOR_DATA *d;
	CHAR_DATA *rch;
	char buf[MSL];
	sprintf(buf,"From another room, $n says to %s, '{G$t{x'",IS_NPC(victim) ? victim->short_descr : victim->name);
	for ( d = descriptor_list; d != NULL; d = d->next )
		if (d->connected == CON_PLAYING
		&& (rch = d->character) != ch
		&& rch->in_room != ch->in_room
		&& rch->pcdata->eavesdropping != -1
		&& rch->in_room->exit[rch->pcdata->eavesdropping]->u1.to_room == ch->in_room
		&& check_eavesdrop(rch))
			act(buf,ch,argument,rch,TO_VICT);
}

void do_osayto(CHAR_DATA *ch,char *argument){
	char arg[MIL];
    CHAR_DATA *victim;

    argument = one_argument(argument,arg);

	if (arg[0] == '\0' || argument[0] == '\0'){
	    send_to_char("oSay what to whom?\n\r",ch);
	    return;
	}

	if ((victim = get_char_world(ch,arg)) == NULL || victim->in_room != ch->in_room){
		send_to_char("They are not here.\n\r",ch);
		return;
	}
	else{
		int slen = strlen(argument);
		if(argument[slen-1] == '!'){
			act("You exclaim oocly to $N '{C$t{x'",ch,argument,victim,TO_CHAR);
			act("$n exclaims oocly to you '{C$t{x'",ch,argument,victim,TO_VICT);
			act("$n exclaims oocly to $N '{C$t{x'",ch,argument,victim,TO_NOTVICT);
		}
		else if(argument[slen-1] == '?'){
			act("You ask $N oocly, '{C$t{x'",ch,argument,victim,TO_CHAR);
			act("$n asks you oocly, '{C$t{x'",ch,argument,victim,TO_VICT);
			act("$n asks $N oocly, '{C$t{x'",ch,argument,victim,TO_NOTVICT);
		}
		else{
			act("You say oocly to $N '{C$t{x'",ch,argument,victim,TO_CHAR);
			act("$n says oocly to you '{C$t{x'",ch,argument,victim,TO_VICT);
			act("$n says oocly to $N '{C$t{x'",ch,argument,victim,TO_NOTVICT);
		}
	}

    if (!IS_NPC (ch)){
	    CHAR_DATA *mob, *mob_next;
	    for (mob = ch->in_room->people; mob != NULL; mob = mob_next){
		    mob_next = mob->next_in_room;
		    if (IS_NPC(mob) && HAS_TRIGGER_MOB(mob, TRIG_SPEECH) && mob->position == mob->pIndexData->default_pos)
				p_act_trigger(argument,mob,NULL,NULL,ch,NULL,NULL,TRIG_SPEECH);
		}
	}
	DESCRIPTOR_DATA *d;
	CHAR_DATA *rch;
	char buf[MSL];
	sprintf(buf,"From another room, $n says oocly to %s, '{C$t{x'",IS_NPC(victim) ? victim->short_descr : victim->name);
	for ( d = descriptor_list; d != NULL; d = d->next )
		if (d->connected == CON_PLAYING
		&& (rch = d->character) != ch
		&& rch->in_room != ch->in_room
		&& rch->pcdata->eavesdropping != -1
		&& rch->in_room->exit[rch->pcdata->eavesdropping]->u1.to_room == ch->in_room
		&& check_eavesdrop(rch))
			act(buf,ch,argument,rch,TO_VICT);
}

void load_max_con (void){
	FILE *fp2;
	if (!(fp2 = fopen(TOTALCON,"r"))){
		log_string("Error reading from totalcon.txt");
		return;
	}
	mud.max_con = fread_number(fp2);
	mud.d_con = fread_number(fp2);
	fclose(fp2);
}

void write_max_con (void){
	FILE *fp;
	
	fp = fopen(TOTALCON,"w");
	
	if (!fp){
		bug("Could not open " TOTALCON " for writing.",0);
		return;
	}

	fprintf(fp,"%d %d\n",mud.max_con,mud.d_con);
		
	fclose(fp);
}

void check_max_con(void){
	bool found = false;
	if(mud.t_con > mud.max_con){
		mud.max_con = mud.t_con;
		found = true;
	}
	if(mud.t_con > mud.d_con){
		mud.d_con = mud.t_con;
		found = true;
	}
	if(found)
		write_max_con();
}

void do_history(CHAR_DATA *ch,char *argument){
	int chan = 0;

	if(!str_cmp(argument,"chat"))
		chan = CHAN_CHAT;
	else if(!str_cmp(argument,"question"))
		chan = CHAN_QUESTION;
	else if(!str_cmp(argument,"answer"))
		chan = CHAN_ANSWER;
	else if(!str_cmp(argument,"imm") && get_trust(ch) >= LEVEL_IMMORTAL)
		chan = CHAN_IMM;
	else if(!str_cmp(argument,"newbie"))
		chan = CHAN_NEWBIE;
	else if(!str_cmp(argument,"gossip"))
		chan = CHAN_GOSSIP;
	else if(!str_cmp(argument,"auction"))
		chan = CHAN_AUCTION;
	if(chan == 0){
		ch->send("That is not a recognized channel.\n\r");
		return;
	}
	for(int i = 24;i>=0;i--)
		act(mud.chan_log[chan][i],ch,NULL,NULL,TO_CHAR);
}

void do_info(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
}

void do_setup(CHAR_DATA *ch,char *argument){
	char buf[MIL];

	argument = one_argument(argument,buf);
	if(!argument[0]){
		ch->send("Valid setup is:\n\rsetup info <flags> <text>\n\r");
		ch->send("Where valid flags are:\n\r");
		return;
	}

	if(!str_cmp(buf,"info")){
		if(!argument[0]){
			ch->send("Valid info input is:\n\r");
			//full char name, real name, hours on, reputation, 
			return;
		}
	}
	else
		ch->send("Hrm? Valid setup is:\n\rsetup finger <flags> <text>");
}
void parse_multitell(CHAR_DATA *ch,char *arg){
	int i = 0;
	char temp[100];
	bool found = false,starting = true,valid = false;

	if(arg[0] == ')' || arg[0] == ' ' || arg[0] == ','){
		ch->send("Bad juju.\n\r");
		return;
	}
 for(;*arg;arg++){ 
 
if(*arg == ')'){ 
 

if(found && i > 0){
ch->send(temp);
 


ch->send("\n\r"); 
 

} 
 

valid = true; 
 

break; 
 
} 
 
if(*arg == ',' || *arg == ' '){ 
 

if(i > 0){ 
 


temp[++i] = '\0'; 
 


ch->send(temp); 
 


ch->send("\n\r"); 
 


i = 0;
 


found = true; 
 


continue; 
 

} 
 

else 
 


continue; 
 
} 
 
 
temp[i++] = *arg; 
  }
 if(!found){ 
 
ch->send("You did not enter any valid names.\n\r"); 
 
return; 
  } 
 if(!valid){ 
 
ch->send("You must include a final ) in your name list.\n\r"); 
 
return; 
  } 
 while(*arg == ' ' || *arg == ',' || *arg == ')' || *arg == '(') 
 
arg++; 
 if(!*arg){ 
 
ch->send("Invalid message.\n\r"); 
 
return; 
  } 
 ch->send("Message: "); 
 ch->send(arg);
 

/*	for(;*arg;arg++){
		if(*arg == ')'){
			if(found && !starting){
				ch->send(temp);
				ch->send("\n\r");
			}
			valid = true;
			break;
		}
		if(*arg == ',' || *arg == ' '){
			if(!starting){
				temp[++i] = '\0';
				ch->send(temp);
				ch->send("\n\r");
				starting = true;
				found = true;
				continue;
			}
			else
				continue;
		}
		if(starting){
			i = 0;
			starting = false;
		}
		if(!starting)
			temp[i++] = *arg;
	}
	if(!found){
		ch->send("You did not enter any valid names.\n\r");
		return;
	}
	if(!valid){
		ch->send("You must include a final ) in your name list.\n\r");
		return;
	}
	while(*arg == ' ' || *arg == ',' || *arg == ')' || *arg == '(')
		arg++;
	if(!*arg){
		ch->send("Invalid message.\n\r");
		return;
	}
	ch->send("Message: ");
	ch->send(arg);*/
}
void do_multitell(CHAR_DATA *ch,char *argument){
	int n = 0;
	char *buf,tname[MIL],message[MIL],*arg;
	bool pfound = false,found = true;
	CHAR_DATA *victim;

	arg = str_dup(argument);

	if(arg[0] != '('){
		ch->send("Bad juju!\n\r");
		return;
	}
	for(;*arg;arg++){
		if(*arg == '('){
			arg++;
			parse_multitell(ch,arg);
			return;
		}
	}
	if(!found){
		ch->send("Syntax: multitell (<person> <person> ...) <message>\n\r");
		return;
	}
	while((one_argument(buf,tname))){
		if((victim = get_char_world(ch,tname)))
			victim->send(message);
	}
}
bool check_color_string(char *string ){
        int i,length;
        int count=1;

        /* Sanity check for empty strings */
        if ( *(string+0) == '\0') {
                return false;
        }

        /* Determine the length of the string. */
        length = strlen(string);

        /*
         * subtacrt one from length to get to the last character
         * Then if the string doesn't end in {, we are done
         */
        if(*(string+(--length)) != '{') {
                return false;
        }

        /* Loop through the string backwards until we know how
         * many {'s are at the end.
         * Also we already checked length so we can set i to length - 1
         */
        for(i=length-1;i>=0;i--) {

                /* If the current character isn't a '{'
                 * We are done.
                 */
                if (*(string+i) != '{')
                        break;

                /* If it is add to count and continue */
                if(*(string+(i-1)) == '{') {
                        count++;
                        continue;
                }
        }

        /* Now if we divide by two and have a remainder,
         * It's an odd number of {'s and we need to kill
         * the last one, otherwise we can just leave it be.
         */
        if( count % 2 ) {
                *(string+length) = '\0';
                return true;
        }

        return false;
}
void do_newchan(CHAR_DATA *ch,char *argument,int channel){
	char arg[MSL],arg2[MSL],*talk,*pre,emote[2*MSL],buf[MSL],soc[MSL],cname[MIL],sname[MIL];
	DESCRIPTOR_DATA *d;
	pre = get_pre(ch, channel);
	if (channel == CHAN_CLAN && IS_NPC(ch)){
		send_to_char("You aren't in a clan.\n\r",ch);
		return;
	}
	if(channel == CHAN_NEWBIE){
	}

	if (!argument[0]){
		if (ch->setcomm(chan_table[channel].bit))
			printf_to_char(ch,"The %s {xchannel is now {ROFF{x.\n\r",chan_table[channel].chan_name);
		else{
			printf_to_char(ch,"The %s {xchannel is now {GON{x.\n\r",chan_table[channel].chan_name);
			ch->remcomm(chan_table[channel].bit);
		}
		return;
	}
	else{  /* message sent, toggle channel on/off */
		if (ch->iscomm(CM_QUIET)){
			send_to_char("You must turn off quiet mode first.\n\r",ch);
			return;
		}
		if (ch->iscomm(CM_NOCHANNELS)){
			send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
			return;
		}
		if (ch->iscomm(chan_table[channel].bit)){
			send_to_char("Try turning the channel on?\n\r",ch);
			return;
		}
	}

	if( check_color_string(argument) )
		send_to_char("{RFinal {{ removed. Do not do this again.{x\r\n",ch);

	arg2[0] = '\0';
	talk = argument;
	one_argument(argument, arg);
	argument = one_argument(argument,arg);
	sprintf( emote, "%s", argument );
	argument = one_argument(argument,arg2);
	if(channel == CHAN_CLAN)
		sprintf(cname,"%s %s",guilds[ch->guild].rank[ch->rank].name,ch->name);
	else
		sprintf(cname,"%s",ch->name);
	if(channel == CHAN_IMM || channel == CHAN_NEWBIE)
		sprintf(sname,"%s",ch->name);
	else
		sprintf(sname,"You");

	if(talk[0] == '&')
		if(check_chansocial(ch,arg,arg2,channel))
			return;

	if(!strcmp(arg,"&emote")){
		argument = one_argument(argument,arg);

		sprintf(buf,"%s {%s$n %s{x", pre, chan_table[channel].chan_say,emote);  
		act(buf,ch,argument,NULL,TO_CHAR);
		for ( d = descriptor_list; d != NULL; d = d->next ){
			CHAR_DATA *vch;
			vch = d->character; 

			if ( d->connected == CON_PLAYING &&
			vch != ch && 
			!vch->iscomm(chan_table[channel].bit) &&
			!vch->iscomm(CM_QUIET)){
   				if(can_be_sent(ch,vch,channel) ){
					act(buf,ch,argument,vch,TO_VICT);
				}
				else
					 continue;
			}	
		}
		return;
	}

	if(!strcmp(arg, "&smote")){
		if(smote_check(ch, arg, emote, channel))
			return;
	}   
	if(!strcmp(arg,"&echo")){
		if(echo_check(ch,arg,emote,channel))
			return;
	}

	sprintf( soc, "%s", get_emote(ch, talk ) );

	if(*talk == '#')
		sprintf(talk, "%s", emote);

	if(channel == CHAN_GOSSIP){
		if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
			talk = makedrunk(talk,ch);
	}

	printf_to_char(ch,chan_table[channel].chan_spre,sname,talk);
//printf_to_char( ch, "%s {%s%s %s '{%s%s{%s'{x\n\r",pre,chan_table[channel].chan_name,ch->name,soc,chan_table[channel].chan_say,talk,chan_table[channel].chan_name);

	add_log(ch,talk,channel);
	for (d = descriptor_list;d;d = d->next){
		CHAR_DATA *vch;
		vch = (d->original) ? d->original : d->character; 

		if ( d->connected == CON_PLAYING && !vch->iscomm(chan_table[channel].bit) && !vch->iscomm(CM_QUIET) && vch != ch){
			if(can_be_sent(ch,vch,channel)){
				printf_to_char(vch,chan_table[channel].chan_pre,cname,talk);
				//sprintf( buf, "%s {%s%s %s '{%s%s{%s'{x",pre,chan_table[channel].chan_name,PERS(ch,vch),soc,chan_table[channel].chan_say,talk,chan_table[channel].chan_name);
				//act_new(buf,ch,argument,vch,TO_VICT,POS_DEAD);
			}
			else
				continue;
		}
	}
}

char *get_emote( CHAR_DATA *ch, char *argument){
    char arg[MSL], buf[MSL];
    int cmd, len, i, y;
    bool found = false;

    arg[0] = '\0';
    buf[0] = '\0';

    argument = one_argument(argument, arg);
    i = 0;
    if( arg[0] == '#' ){
		len = strlen_color(arg);
		for (i=y=0; i < len; i++){
			if(arg[i] == '#' )
				i++;

			buf[y] = arg[i];
			y++;
			buf[y] = '\0';
		}

		if( ( cmd = social_lookup(buf) ) < 0)
			return "says,";
		else
			found = true;
		if(found)
			return social_table[cmd].others_no_arg;//chan_emote;
	}
	return "says,";
}

char *get_pre(CHAR_DATA *ch, int channel){
	static char buf[100];

	if(channel == CHAN_CLAN){
		snprintf(buf,99, "%s %s",chan_table[channel].chan_pre,guilds[ch->guild].rank[ch->rank].name);
		return buf;
	}
	else
		return chan_table[channel].chan_pre;
}

bool smote_check(CHAR_DATA *ch, char *argument, char *emote, int channel){
	CHAR_DATA *vch;
	char *letter,*name,last[MIL], temp[MSL],*pre,buf[MSL];
	int matches = 0;
	DESCRIPTOR_DATA *d; 

	pre = get_pre(ch, channel);

	if ( !IS_NPC(ch) && ch->iscomm(CM_NOEMOTE) )
		return false;

	if ( argument[0] == '\0' )
		return false;

	if (strstr(emote,ch->name) == NULL){
		send_to_char("You must include your name in an smote.\n\r",ch);
		return true;
	}

	sprintf( buf, "%s {%s%s{x", pre, chan_table[channel].chan_name, emote); 

	send_to_char(buf,ch);
	send_to_char("\n\r",ch);

	for ( d = descriptor_list; d != NULL; d = d->next ){
		vch = ( d->original != NULL ) ? d->original : d->character; 
		if ( d->connected == CON_PLAYING && vch != ch && !vch->iscomm(chan_table[channel].bit) && !vch->iscomm(CM_QUIET)){
			if(!can_be_sent(ch,vch,channel))
				continue;
			if (vch->desc == NULL || vch == ch)
				continue;

			if ((letter = strstr(argument,vch->name)) == NULL){
				act(buf,ch,NULL,vch,TO_VICT);
				continue;
			}

			strcpy(temp,emote);
			temp[strlen(emote) - strlen(letter)] = '\0';
			last[0] = '\0';
			name = vch->name;

			for (; *letter != '\0'; letter++){
				if (*letter == '\'' && matches == strlen(vch->name)){
					strcat(temp,"r");
					continue;
				}

				if (*letter == 's' && matches == strlen(vch->name)){
					matches = 0;
					continue;
				}

				if (matches == strlen(vch->name))
					matches = 0;

				if (*letter == *name){
					matches++;
					name++;
					if (matches == strlen(vch->name)){
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
			act(buf,ch,NULL,vch,TO_VICT);
		}
	} 
	return true;
}

bool echo_check(CHAR_DATA *ch, char *argument, char *emote, int channel){
	DESCRIPTOR_DATA *d;
	char *pre,buf[MSL];
	buf[0] = '\0';
	pre = get_pre(ch, channel);

	if ( argument[0] == '\0' )
		return false;
	if(!IS_IMMORTAL(ch))
		return false;

	sprintf( buf, "%s {%s%s{x", pre, chan_table[channel].chan_name, emote); 
	send_to_char(buf,ch);
	send_to_char("\n\r",ch);
	for ( d = descriptor_list; d != NULL; d = d->next ){
		CHAR_DATA *vch;
		vch = ( d->original != NULL ) ? d->original : d->character;; 
		if ( d->connected == CON_PLAYING && vch != ch && !vch->iscomm(chan_table[channel].bit) && !vch->iscomm(CM_QUIET)){
			if(can_be_sent(ch, vch,channel) )
				act(buf,ch,NULL,vch,TO_VICT);
			else
				continue;
		}
	}
	return true;
}

bool check_chansocial(CHAR_DATA *ch,char *command,char *argument,int channel){
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;
	int cmd,counter,count;
	bool found;
	char arg[MIL],buf[MSL],buf2[MSL],buf3[MSL],*pre;

	if(!chan_table[channel].canemote){
		ch->send("You may not use socials on this channel.\n\r");
		return true;
	}
	pre = get_pre(ch,channel);

	buf[0] = '\0';
	buf2[0] = '\0';
	buf3[0] = '\0';
	found  = false;

	for(cmd = 0;social_table[cmd].name[0];cmd++){
		if(command[1] == social_table[cmd].name[0] && !str_prefix(&command[1],social_table[cmd].name)){
			found = true;
			break;
		}
	}

	if (!found)
		return false;

	one_argument(argument,arg);
	victim = NULL;
	if (!arg[0]){
		sprintf(buf,chan_table[channel].chan_emote,social_table[cmd].others_no_arg);
		sprintf(buf2,chan_table[channel].chan_emote,social_table[cmd].char_no_arg);

		for(d = descriptor_list;d;d = d->next){
			CHAR_DATA *vch;
			vch = (d->original) ? d->original : d->character;
			if(d->connected == CON_PLAYING && vch != ch && !vch->iscomm(chan_table[channel].bit) && !vch->iscomm(CM_QUIET)){
				if(can_be_sent(ch,vch,channel))
					act(buf,ch,NULL,vch,TO_VICT);
				else
					continue;
			}
		}
		act(buf2,ch,NULL,NULL,TO_CHAR);
	}
	else if(!(victim = get_char_world(ch,arg)) || IS_NPC(victim))
		return false;
	else if(victim->iscomm(chan_table[channel].bit)){
		send_to_char("They cannot see this channel, therefor you cannot do that to them.\n\r",ch);
		return true;
	}
	else if(victim == ch){
		sprintf(buf,chan_table[channel].chan_emote,social_table[cmd].others_auto);
		sprintf(buf2,chan_table[channel].chan_emote,social_table[cmd].char_auto);
		send_to_char(buf2,ch);

		for ( d = descriptor_list; d != NULL; d = d->next ){
			CHAR_DATA *vch;
			vch = ( d->original != NULL ) ? d->original : d->character;
			if(!can_be_sent(ch, vch,channel) )
				continue;
			if ( d->connected == CON_PLAYING && vch != ch && victim->name != arg && !vch->iscomm(chan_table[channel].bit) && !vch->iscomm(CM_QUIET))
				act(buf,ch,NULL,vch,TO_VICT);
		}
	}
	else{
		sprintf(buf,chan_table[channel].chan_emote,social_table[cmd].others_found);
		sprintf(buf2,chan_table[channel].chan_emote,social_table[cmd].char_found);
		sprintf(buf3,chan_table[channel].chan_emote,social_table[cmd].vict_found);
		act(buf3,ch,NULL,victim,TO_VICT);
		act(buf2,ch,NULL,victim,TO_CHAR);
		for (counter = 0; buf[counter+1] != '\0'; counter++){
			if (buf[counter] == '$' && buf[counter + 1] == 'N'){
				strcpy(buf2,buf);
				buf2[counter] = '\0';
				strcat(buf2,victim->name);
				for (count = 0; buf[count] != '\0'; count++)
					buf[count] = buf[count+counter+2];
				strcat(buf2,buf);
				strcpy(buf,buf2);
			}
			else if (buf[counter] == '$' && buf[counter + 1] == 'E'){
				switch (victim->sex){
					default:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"it");
						for (count = 0; buf[count] != '\0'; count ++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
					case 1:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"it");
						for (count = 0; buf[count] != '\0'; count++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
					case 2:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"it");
						for (count = 0; buf[count] != '\0'; count++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
				}
			}
			else if (buf[counter] == '$' && buf[counter + 1] == 'M'){
				buf[counter] = '%';
				buf[counter + 1] = 's';
				switch (victim->sex){
					default:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"it");
						for (count = 0; buf[count] != '\0'; count++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
					case 1:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"him");
						for (count = 0; buf[count] != '\0'; count++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
					case 2:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"her");
						for (count = 0; buf[count] != '\0'; count++);
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
				}
			}
			else if (buf[counter] == '$' && buf[counter + 1] == 'S'){
				switch (victim->sex){
					default:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"its");
						for (count = 0;buf[count] != '\0'; count++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
					case 1:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"his");
						for (count = 0; buf[count] != '\0'; count++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
					case 2:
						strcpy(buf2,buf);
						buf2[counter] = '\0';
						strcat(buf2,"her");
						for (count = 0; buf[count] != '\0'; count++)
							buf[count] = buf[count+counter+2];
						strcat(buf2,buf);
						strcpy(buf,buf2);
						break;
				}
			}
		}

		for ( d = descriptor_list; d != NULL; d = d->next ){
			CHAR_DATA *vch;
			vch = ( d->original != NULL ) ? d->original : d->character;

			if ( d->connected == CON_PLAYING && vch != ch && victim != vch && !vch->iscomm(chan_table[channel].bit) && !vch->iscomm(CM_QUIET) ){
				if(can_be_sent(ch,vch,channel) )
					act(buf,ch,NULL,vch,TO_VICT);
				else
					continue;
			}
		}
	}
	return true;
}

void do_housetalk(CHAR_DATA *ch,char *argument){
	do_newchan(ch,argument,CHAN_CLAN);
	return;
}
void do_question(CHAR_DATA *ch,char *argument){
	do_newchan(ch,argument,CHAN_QUESTION);
	return;
}
void do_answer(CHAR_DATA *ch,char *argument){
	do_newchan(ch,argument,CHAN_ANSWER);
	return;
}
void do_immtalk(CHAR_DATA *ch,char *argument){
	do_newchan(ch,argument,CHAN_IMM);
	return;
}
void do_newbie(CHAR_DATA *ch,char *argument){
    do_newchan(ch,argument,CHAN_NEWBIE);
    return;
}
void do_chat(CHAR_DATA *ch,char *argument){
	do_newchan(ch,argument,CHAN_CHAT);
	return;
}
void do_gossip(CHAR_DATA *ch,char *argument){
    do_newchan(ch,argument,CHAN_GOSSIP);
    return;
}
void do_auction(CHAR_DATA *ch,char *argument){
    do_newchan(ch,argument,CHAN_AUCTION);
    return;
}

bool can_be_sent(CHAR_DATA *ch, CHAR_DATA *vch, int channel ){
    if ((channel == CHAN_IMM) && (!IS_IMMORTAL(vch)))
    	return false;
    if (channel == CHAN_CLAN && !is_same_guild(ch,vch))
        return false;

    return true;
}
