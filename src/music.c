#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "music.h"
#include "recycle.h"

int channel_songs[MAX_GLOBAL + 1];
struct song_data song_table[MAX_SONGS];

void do_lore(CHAR_DATA *ch, char *argument){
	OBJ_DATA *obj;
	char arg[MSL];
	int tweight = 0,n=0,skill=get_skill(ch,gsn_lore) * .9;

	argument = one_argument(argument,arg);

	if (skill < 1){
		ch->send("You're not wise enough of the world.\n\r");
		return;
	}

	if ((obj = get_obj_carry(ch,arg,ch)) == NULL){
		send_to_char("You don't have that item.\n\r",ch);
		return;
	}

	skill = (skill * get_curr_stat(ch,STAT_INT)) / STAT_MAX;
	WAIT_STATE(ch,skill_table[gsn_lore].beats);
	if (number_percent() >= skill){
		ch->send("You failed to recognize this item.\n\r");
		check_improve(ch,gsn_lore,false,3);
		return;
	}
	act("$n studies $p carefully.",ch,obj,NULL,TO_ROOM);
	printf_to_char(ch,"This item is %s\n\r",item_name(obj->item_type));
	if (obj->owner)
		printf_to_char(ch,"It belongs to %s.\n\r",obj->owner);

	tweight = UMAX(1,obj->weight);
	tweight = number_range(tweight,tweight * 2)/10;
	printf_to_char(ch,"It seems to weigh about %d pounds.\n\r",tweight);

	printf_to_char(ch,"It's made of %s.\n\r",obj->material);

//	printf_to_char(ch,"It couldn't possibly be used by %s\n\r",exclude_bit_name(obj->exclude_flags));

	if (obj->enchanted)
		send_to_char("It's enchanted.\n\r",ch);

	if (obj->item_type == ITEM_WEAPON){
		if (number_percent() < 75)
			printf_to_char(ch,"It seems to be a %s\n\r",weapon_table[obj->value[0]].name);
		else
			printf_to_char(ch,"It seems to be a %s\n\r",weapon_table[number_range(0,WEAPON_MAX)].name);
		n = (1 + obj->value[2]) * obj->value[1] / 2;
		if (n < 20)			ch->send("It is pathetic\n\r");
		else if (n < 30)	ch->send("It is quite wussy\n\r");
		else if (n < 50)	ch->send("It is rather weak\n\r");
		else if (n < 70)	ch->send("It is sorta weak\n\r");
		else if (n < 100)	ch->send("It is nothing special\n\r");
		else if (n < 120)	ch->send("It is average in strength\n\r");
		else if (n < 140)	ch->send("It is decently powerful\n\r");
		else if (n < 160)	ch->send("It is quite powerful\n\r");
		else if (n < 180)	ch->send("It is very strong\n\r");
		else if (n < 200)	ch->send("It is really freakign strong\n\r");
		else				ch->send("This weapon was forged by a master\n\r");
		switch (attack_table[obj->value[3]].damage){
		case DAM_PIERCE: ch->send("It is a piercing weapon.\n\r");break;
		case DAM_SLASH: ch->send("It is a slashing weapon.\n\r");break;
		case DAM_BASH: ch->send("It is a bashing weapon.\n\r");break;
		default: ch->send("It is ... some kind of weapon\n\r");break;
		}
	}
	if (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD)
		printf_to_char(ch,"It is %s\n\r",armortype_bit_name(obj->armortype_flags));

	check_improve(ch,gsn_lore,true,2);
}

/*void song_update(void)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    DESCRIPTOR_DATA *d;
    char buf[MSL], *line;
    int i;

    // do the global song, if any 
    if (channel_songs[1] >= MAX_SONGS)
	channel_songs[1] = -1;

    if (channel_songs[1] > -1)
    {
        if (channel_songs[0] >= MAX_LINES || channel_songs[0] >= song_table[channel_songs[1]].lines)
        {
			channel_songs[0] = -1;

				// advance songs
			for (i = 1; i < MAX_GLOBAL; i++)
	    		channel_songs[i] = channel_songs[i+1];
			channel_songs[MAX_GLOBAL] = -1;
		}
		else
		{
			if (channel_songs[0] < 0)
			{
	    		sprintf(buf,"Music: %s, %s",
				song_table[channel_songs[1]].group,
				song_table[channel_songs[1]].name);
	    		channel_songs[0] = 0;
			}
			else
			{
			sprintf(buf,"Music: '%s'",
				song_table[channel_songs[1]].lyrics[channel_songs[0]]);
			channel_songs[0]++;
			}

			for (d = descriptor_list; d != NULL; d = d->next)
			{
			victim = d->original ? d->original : d->character;

        		if ( d->connected == CON_PLAYING &&
             			 !victim->iscomm(CM_NOCHAT) &&
             			 !victim->iscomm(CM_QUIET) )
            			 act_new("$t",d->character,buf,NULL,TO_CHAR,POS_SLEEPING);
				}
		}
    }

    for (obj = object_list; obj != NULL; obj = obj->next)
    {
		if (obj->item_type != ITEM_JUKEBOX || obj->value[1] < 0)
		    continue;

 		if (obj->value[1] >= MAX_SONGS)
		{
			obj->value[1] = -1;
			continue;
		}

		// find which room to play in

		if ((room = obj->in_room) == NULL)
		{
			if (obj->carried_by == NULL)
	    		continue;
			else
	    		if ((room = obj->carried_by->in_room) == NULL)
				continue;
		}

		if (obj->value[0] < 0)
		{
			sprintf(buf,"$p starts playing %s, %s.",
			song_table[obj->value[1]].group,song_table[obj->value[1]].name);
			if (room->people != NULL)
			act(buf,room->people,obj,NULL,TO_ALL);
			obj->value[0] = 0;
			continue;
		}
		else
		{
			if (obj->value[0] >= MAX_LINES || obj->value[0] >= song_table[obj->value[1]].lines)
			{

			obj->value[0] = -1;

			// scroll songs forward 
			obj->value[1] = obj->value[2];
			obj->value[2] = obj->value[3];
			obj->value[3] = obj->value[4];
			obj->value[4] = -1;
			continue;
			}

			line = song_table[obj->value[1]].lyrics[obj->value[0]];
			obj->value[0]++;
		}

		sprintf(buf,"$p bops: '%s'",line);
		if (room->people != NULL)
			act(buf,room->people,obj,NULL,TO_ALL);
    }
}

void load_songs(void)
{
    FILE *fp;
    int count = 0, lines, i;
    char letter;

    // reset global
    for (i = 0; i <= MAX_GLOBAL; i++)
	channel_songs[i] = -1;

    if ((fp = fopen(MUSIC_FILE,"r")) == NULL)
    {
		bug("Couldn't open music file, no songs available.",0);
		fclose(fp);
		return;
    }

    for (count = 0; count < MAX_SONGS; count++)
    {
        letter = fread_letter(fp);
        if (letter == '#')
        {
            if (count < MAX_SONGS)
                song_table[count].name = NULL;
            fclose(fp);
            return;
        }
  		else
			ungetc(letter,fp);

		song_table[count].group = fread_string(fp);
		song_table[count].name 	= fread_string(fp);

		// read lyrics
		lines = 0;

		for ( ; ;)
		{
			letter = fread_letter(fp);

			if (letter == '~')
			{
				song_table[count].lines = lines;
				break;
			}
			else
				ungetc(letter,fp);
			
			if (lines >= MAX_LINES)
   			{
				bug("Too many lines in a song -- limit is  %d.",MAX_LINES);
				break;
			}

			song_table[count].lyrics[lines] = fread_string_eol(fp);
			lines++;
		}
    }
}

void do_play(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *juke;
    char *str,arg[MAX_INPUT_LENGTH];
    int song,i;
    bool global = false;

    str = one_argument(argument,arg);

    for (juke = ch->in_room->contents; juke != NULL; juke = juke->next_content)
	if (juke->item_type == ITEM_JUKEBOX && can_see_obj(ch,juke))
	    break;

    if (argument[0] == '\0')
    {
		send_to_char("Play what?\n\r",ch);
		return;
    }

    if (juke == NULL)
    {
		send_to_char("You see nothing to play.\n\r",ch);
		return;
    }

    if (!str_cmp(arg,"list"))
    {
		BUFFER *buffer;
  		char buf[MSL];
		int col = 0;
		bool artist = false, match = false;

		buffer = new_buf();
		argument = str;
		argument = one_argument(argument,arg);

		if (!str_cmp(arg,"artist"))
			artist = true;

		if (argument[0] != '\0')
			match = true;

		sprintf(buf,"%s has the following songs available:\n\r", juke->short_descr);
		add_buf(buffer,capitalize(buf));

		for (i = 0; i < MAX_SONGS; i++)
		{
			if (song_table[i].name == NULL)
				break;

			if (artist && (!match || !str_prefix(argument,song_table[i].group)))
				sprintf(buf,"%-39s %-39s\n\r", song_table[i].group,song_table[i].name);
			else if (!artist && (!match || !str_prefix(argument,song_table[i].name)))
	    		sprintf(buf,"%-35s ",song_table[i].name);
			else
				continue;
			add_buf(buffer,buf);
			if (!artist && ++col % 2 == 0)
				add_buf(buffer,"\n\r");
		}
		if (!artist && col % 2 != 0)
			add_buf(buffer,"\n\r");

		page_to_char(buf_string(buffer),ch);
		free_buf(buffer);
		return;
    }

    if (!str_cmp(arg,"loud"))
    {
        argument = str;
        global = true;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Play what?\n\r",ch);
        return;
    }

    if ((global && channel_songs[MAX_GLOBAL] > -1) || (!global && juke->value[4] > -1))
    {
        send_to_char("The jukebox is full up right now.\n\r",ch);
        return;
    }

    for (song = 0; song < MAX_SONGS; song++)
    {
		if (song_table[song].name == NULL)
		{
			send_to_char("That song isn't available.\n\r",ch);
			return;
		}
		if (!str_prefix(argument,song_table[song].name))
			break;
    }

    if (song >= MAX_SONGS)
    {
		send_to_char("That song isn't available.\n\r",ch);
		return;
    }

    send_to_char("Coming right up.\n\r",ch);

    if (global)
    {
		for (i = 1; i <= MAX_GLOBAL; i++)
			if (channel_songs[i] < 0)
			{
				if (i == 1)
					channel_songs[0] = -1;
				channel_songs[i] = song;
				return;
			}
    }
    else 
    {
		for (i = 1; i < 5; i++)
			if (juke->value[i] < 0)
			{
				if (i == 1)
					juke->value[0] = -1;
				juke->value[i] = song;
				return;
			}
    }
}*/

void do_randomnumber(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	int n1 = 0,n2 = 0,answer = 0;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if (!arg1[0] || !arg2[0]){
		ch->send("You must supply a minimum and maximum range.\n\r");
		return;
	}
	if (!is_number(arg1) || !is_number(arg2)){
		ch->send("Ranges must be numeric.\n\r");
		return;
	}

	n1 = atoi(arg1);
	n2 = atoi(arg2);
	answer = number_range(n1,n2);
	printf_to_char(ch,"Random number between values {G%d {xand {G%d {xis {R%d{x.\n\r",n1,n2,answer);
}
