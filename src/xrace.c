#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */
bool IS_WET_BG (CHAR_DATA*);



void check_bravery(CHAR_DATA *ch){
	if(get_skill(ch,gsn_bravery < 1))
		return;

	if(100 * ch->hit / ch->max_hit > 25)
		return;
	if(number_percent() > 75){
		act("$n is brave!",ch,NULL,NULL,TO_ROOM);
		ch->send("You feel brave?\n\r");
	}
}

bool check_durability(CHAR_DATA *ch){
	if(get_skill(ch,gsn_durability < 1))
		return false;
	if(number_range(1,ch->level) >= ch->level/2)
		return true;
	return false;
}

int crunch_durability(CHAR_DATA *ch,int dam){
	int tdam = dam/2;

	if (IS_NPC(ch) || get_skill(ch,gsn_durability) < 1)
		return dam;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Durability: %d, ",dam);

	tdam = tdam * ch->hit / ch->max_hit;
	tdam += dam/2;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d\n\r",tdam);
	return tdam;
}

int check_icebound(CHAR_DATA *ch){
	if(!ch->israce("algid"))
		return 0;
	if (get_skill(ch,gsn_icebound) < 1 || !ch->in_room || sect_table[ch->in_room->sector_type].isindoors || IS_SET(ch->in_room->room_flags,ROOM_INDOORS))
		return 0;
	if (ch->in_room->area->climate == CLIMATE_ARID)
		return 2;
	if (ch->in_room->area->climate == CLIMATE_FRIGID)
		return 1;
	return 0;
}

void do_disappear(CHAR_DATA *ch,char *argument){
	if (get_skill(ch,gsn_disappear) < 1){
		ch->send("You concentrate and focus but... you fail to make yourself disappear!\n\r");
		return;
	}

	if (ch->in_room->area->climate != CLIMATE_ARID){
		ch->send("You lack the proper surroundings to disappear.\n\r");
		return;
	}

	ROOM_INDEX_DATA *troom = get_random_room_area(ch);

	ch->send("You vanish!\n\r");
	act("$n seems to vanish!",ch,NULL,NULL,TO_ROOM);
	while(troom == ch->in_room){
		troom = get_random_room_area(ch);
	}
	char_from_room(ch);
	char_to_room(ch,troom);
	do_function(ch,&do_look,"auto");
}

int crunch_resilience(CHAR_DATA *ch, int dam){
	if (get_skill(ch,gsn_resilience) < 1)
		return dam;
	return dam * .9;
}

int do_thalassic_aura(CHAR_DATA *ch,int mana){
	CHAR_DATA *quolin;
	int count = 0;

	if (!IS_WET_BG(ch))
		return mana;

	for (quolin = ch->in_room->people;quolin;quolin = quolin->next_in_room){
		if(get_skill(quolin,gsn_thalassic_aura) < 1 || (IS_NPC(quolin) && !quolin->israce("quolin")))
			continue;
		count++;
	}

	if (count == 0)
		return mana;
	return mana * (count + 1);
}

bool check_thalassic(CHAR_DATA *ch){
	if(get_skill(ch,gsn_thalassic_aura) < 1 || (IS_NPC(ch) && !ch->israce("quolin")))
		return false;
	if(!IS_WET_BG(ch) && !ch->in_room->israf(RAF_FOREST_MIST))
		return false;
	return true;
}

int uplifting_hrdr(CHAR_DATA *ch,int n){
/*	CHAR_DATA *dwarf;
	int count = 10;

	for (dwarf = ch->in_room->people;dwarf;dwarf = dwarf->next_in_room){
		if (get_skill(dwarf,gsn_uplifting_spirit) < 1)
			continue;
		if (!is_same_group(ch,dwarf))
			continue;
		n += count--;
		if (count == 0)
			count = 1;
	}*/
	return n;
}
int uplifting_ac(CHAR_DATA *ch,int ac){
/*	CHAR_DATA *dwarf;
	int count = 20;

	for (dwarf = ch->in_room->people;dwarf;dwarf = dwarf->next_in_room){
		if (get_skill(dwarf,gsn_uplifting_spirit) < 1)
			continue;
		if (!is_same_group(ch,dwarf))
			continue;
		ac -= count--;
		if (count == 0)
			count = 1;
	}*/
	return ac;
}
bool check_amiability(CHAR_DATA *ch){
	if (get_skill(ch,gsn_amiability) < 1)
		return false;
	if (number_percent() < 51)
		return true;
	else
		return false;
}

void vithe_return(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	if ((!IS_NPC(victim) && get_skill(victim,gsn_return) < 1) || (IS_NPC(victim) /*&& !victim->israce("vithe")*/) || ch == victim)
		return;
	dam = (victim->level/10) + (dam / 100) + 1;
	damage(victim,ch,dam,gsn_return,DAM_OTHER,true);
}
void vithe_regeneration(CHAR_DATA *ch){
	static int vithe = 0;
	if (IS_NPC(ch) || ch->hit >= ch->max_hit || get_skill(ch,gsn_regeneration) < 1 || ch->position < POS_SLEEPING)
		return;

	ch->hit += 1;
}

int check_fervor(CHAR_DATA *ch,int num){
	if((IS_NPC(ch) && !ch->israce("ogre")) || get_skill(ch,gsn_fervor) < 1 || ch->hit * 100 / ch->max_hit > 25)
		return num;

	return num;//Nash make this just activate an arbitrary haste - (num * (30 - (ch->hit * 100 / ch->max_hit)));
}
