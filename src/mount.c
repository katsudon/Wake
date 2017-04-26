#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"


bool make_visible(CHAR_DATA *ch);


//make it bool mounted; char_data *mount; and nothing else


int mount_chance(CHAR_DATA *ch,CHAR_DATA *mount){
	int chance = grab_skill(ch,gsn_riding),diff;

	diff = ch->level - mount->level;
	chance = UMAX(chance+diff,100);

	if(ch->level < mount->level)
		chance = 0;

	return chance;
}

bool validate_mount(CHAR_DATA *ch,CHAR_DATA *mount){
	if(RIDING(ch)){
		send_to_char("You are already riding a mount.\n\r",ch);
		return false;
	}
	if(RIDDEN(ch)){
		ch->send("You cannot ride while being ridden.\n\r");
		return false;
	}
	if(!mount){
		ch->send("Mount what?\n\r");
		return false;
	}
	if(get_skill(ch,gsn_riding) < 1){
		send_to_char("You would not know which end to sit on.\n\r",ch);
		return false;
	}
	if(mount->in_room != ch->in_room){
		ch->send("They are not here.\n\r");
		return false;
	}
	if(!IS_NPC(mount) || !mount->isact(AT_MOUNT)){
		send_to_char("That cannot be ridden.\n\r",ch);
		return false;
	}
	if(mount->mount && mount->mount != ch){
		printf_to_char(ch,"%s does not belong to you.\n\r",mount->short_descr);
		return false;
	}
	if(mount->position < POS_STANDING){
		send_to_char("The mount must be standing.\n\r",ch);
		return false;
	}
	if(RIDDEN(mount) || RIDING(mount)){
		send_to_char("That mount is already being ridden.\n\r",ch);
		return false;
	}
	if (mount->isact(AT_WARHORSE) && get_skill(ch,gsn_horsemanship) < 1){
		send_to_char("You are not refined enough to ride such a fine steed.\n\r",ch);
		return false;
	}

	if(mount_chance(ch,mount) < (get_skill(ch,gsn_riding)-20)){
		send_to_char("You fail to mount the beast.\n\r",ch);
		WAIT_STATE(ch,skill_table[gsn_riding].beats);
		check_improve(ch,gsn_riding,false,1);
		return false;
	}
	check_improve(ch,gsn_riding,true,1);
	return true;
}

void do_mount(CHAR_DATA *ch,char *argument){
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *mount = NULL;

	argument = one_argument(argument,arg);

	if(!arg[0])
		mount = ch->mount;      
	else if (!(mount = get_char_room(ch,NULL,arg))){
		send_to_char("What would you like to mount?\n\r",ch);
		return;
	}

	if(!validate_mount(ch,mount))
		return;

	make_visible(ch);

	act("You hop on $N's back.",ch,NULL,mount,TO_CHAR);
	act("$n hops on $N's back.",ch,NULL,mount,TO_NOTVICT);
	act("$n hops on your back!",ch,NULL,mount,TO_VICT);

	ch->mount = mount;
	ch->mounted = true;
	mount->mount = ch;
	mount->mounted = true;
	mount->ismount = true;
	mount->master = ch;

	if(mount->isact(AT_WARHORSE))
		group_char(ch,mount);
}

void do_dismount(CHAR_DATA *ch,char *argument){
	CHAR_DATA *mount;

	if(ch->mounted){
		mount = ch->mount;

		if(RIDDEN(ch) || ch->ismount){
			ch->send("But YOU'RE the mount!\n\r");
			return;
		}

		act("You dismount from $N.",ch,NULL,mount,TO_CHAR);
		act("$n dismounts from $N.",ch,NULL,mount,TO_NOTVICT);
		act("$n dismounts from you.",ch,NULL,mount,TO_VICT);

		mount->mounted = false;
		ch->mounted = false;
	}
	else
		send_to_char("You aren't mounted.\n\r",ch);
}

char *get_mount_owner(CHAR_DATA *pet){
    static char owner_name[20];
    char *temp;
    int len=0,i=0;

    if (!IS_NPC(pet))
		return NULL;

    if (pet->description)
		len = strlen_color(pet->description);
    else
		return NULL;

    temp = pet->description;

    while (*temp && i<15 && len<1024)
    {
		if (*temp==' ')
			i++;		
		temp++;
		len++;
    }

    if (len==1024){
		log_string("BUG: description overflow for get_mount_owner.");
		return NULL;
    }

    len=0;
    owner_name[0]=0;
    while (*temp && *temp!=' ' && len<16)
    {
		owner_name[len++]=*temp;
		temp++;
    }

    owner_name[len]=0;
    return(owner_name);
}

void do_buy_mount(CHAR_DATA *ch,char *argument){
	int cost,roll;
	char arg[MIL], color[80], name[30], size[30];
	CHAR_DATA *mount;
	ROOM_INDEX_DATA *pRoomIndexNext, *in_room;

	name[0] = '\0';
	size[0] = '\0';
	color[0] = '\0';

	if (IS_NPC(ch))
		return;

	if (!IS_NPC(ch) && !ch->pcdata->learned[gsn_riding]){
		send_to_char("How do you expect to buy a horse when you can't even ride?\n\r",ch);
		return;
	}

	argument = one_argument(argument,arg);

	pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);

	if (pRoomIndexNext == NULL){
		bug("Do_buy: bad mount shop at vnum %d.",ch->in_room->vnum);
		send_to_char("I'm afraid the stalls where I keep my mounts don't exist.\n\r",ch);
		return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	mount       = get_char_room(ch,NULL,arg);
	ch->in_room = in_room;

	if (mount == NULL){
		send_to_char("Sorry, we don't sell any of those here.\n\r",ch);
		return;
	}

	if (!mount->isact(AT_MOUNT)){
		send_to_char("Sorry, we don't sell any of those here.\n\r",ch);
		return;
	}

	if (mount->isact(AT_WARHORSE) && get_skill(ch,gsn_horsemanship) < 1){
		send_to_char("You lack the skill required to handle such a horse.\n\r",ch);
		return;
	}

	if (MOUNTED(ch) || ch->mount != NULL){
		send_to_char("You already have a mount.\n\r",ch);
		return;
	}

	cost = mount->level*mount->level;

	if ((ch->silver + 100 * ch->gold) < cost){
		send_to_char("You can't afford it.\n\r",ch);
		return;
	}

	roll = number_percent();

	if (!IS_NPC(ch) && roll < ch->pcdata->learned[gsn_haggle]){
		cost -= cost / (7 - ch->getslvl(gsn_haggle)) * roll / 100;
		printf_to_char(ch,"You haggle the price down to %d coins.\n\r",cost);
		check_improve(ch,gsn_haggle,true,4);
	}

	deduct_cost(ch, cost);
	mount = create_mobile(mount->pIndexData);

	argument = one_argument(argument,arg);

	char_to_room(mount,ch->in_room);
	add_follower(mount,ch,false);

	act("$n bought $N as a mount.",ch,NULL,mount,TO_ROOM);

	send_to_char("Enjoy your mount.\n\r",ch);

	do_mount(ch,mount->name);

	return;
}

void do_checkmount(CHAR_DATA *ch,char *argument){
  int chance=0;
 char arg[MAX_INPUT_LENGTH];
        CHAR_DATA *mount;

        argument = one_argument(argument,arg);

        if (IS_NPC(ch))
        return;

      if (arg[0] == '\0' && ch->mount && ch->mount->in_room == ch->in_room)
      mount = ch->mount;

      else if (!(mount = get_char_room(ch,NULL,arg)))
      {
         send_to_char("What would you like to mount?\n\r",ch);
         return;
      }
      
      chance = mount_chance(ch,mount);
      printf_to_char(ch,"Chance: %d\n\r",chance);
      return;
}
