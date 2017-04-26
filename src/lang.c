#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"


void do_daurin (CHAR_DATA * ch, char *argument){
	char buf[MSL],buf2[MSL];
	int chance = grab_skill(ch,gsn_daurin);
	CHAR_DATA *vch;

	if(chance < 1){
		send_to_char("You don't even know the basics of daurin.\n\r",ch);
		return;
	}

	if(!argument[0]){
		ch->send("Say what in Daurin?\n\r");
		return;
	}

	if(number_percent() < chance){
		act ("In daurin, you say, '{G$T{x'", ch, NULL, argument, TO_CHAR);
		sprintf(buf,"In daurin, %s says, '{G%s{x'\n\r",ch->name, argument);
		sprintf(buf2,"%s says something in a fluid and beautiful language you dont understand.\n\r",ch->name);
		check_improve(ch,gsn_daurin,true,2);
	}
	else{
		send_to_char("You hack and cough and fail to say what's on your mind!\n\r",ch);
		check_improve(ch,gsn_daurin,false,2);
		return;
	}

	for(vch = ch->in_room->people;vch;vch = vch->next_in_room){
		if(!IS_NPC(vch) && vch != ch){
			if((chance = grab_skill(vch,gsn_daurin)) > 0 && number_percent() <= chance){
				send_to_char(buf,vch);
				check_improve(vch,gsn_daurin,true,1);
			}
			else{
				send_to_char(buf2,vch);
				check_improve(vch,gsn_daurin,false,1);
			}
		}
	}
}

