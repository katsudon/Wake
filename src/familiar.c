#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

extern char *target_name;
extern const struct flag_type affect_flags[];
bool	IS_NATURE_BG	( CHAR_DATA* );

void pet_to_char(CHAR_DATA *ch,CHAR_DATA *pet){//Nash dammit
}

void init_familiar(CHAR_DATA *ch,CHAR_DATA *pet){
	pet->setaff(AF_CHARM);
	pet->alignment				= ch->alignment;
	init_race(pet);
	char_to_room(pet,ch->in_room);
	add_follower(pet,ch,false);
	group_char(ch,pet);
	pet->master					= ch;
	ch->pet						= pet;
}

void do_buy_pet(CHAR_DATA *ch,char *argument){
	int cost,roll;
	char arg[MAX_INPUT_LENGTH],buf[MSL];
	CHAR_DATA *pet;
	ROOM_INDEX_DATA *pRoomIndexNext = get_room_index(ch->in_room->vnum + 1),*in_room;

	smash_tilde(argument);

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument,arg);

	if (pRoomIndexNext == NULL){
		bug("Do_buy: bad pet shop at vnum %d.",ch->in_room->vnum);
		send_to_char("Sorry, you can't buy that here.\n\r",ch);
		return;
	}

	in_room     = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet         = get_char_room(ch,NULL,arg);
	ch->in_room = in_room;

	if (pet == NULL || !pet->isact(AT_PET)){
		send_to_char("Sorry, you can't buy that here.\n\r",ch);
		return;
	}

	if (ch->pet != NULL){
		send_to_char("You already own a pet.\n\r",ch);
		return;
	}

 	cost = 10 * pet->level * pet->level;

	if ((ch->silver + 100 * ch->gold) < cost){
		send_to_char("You can't afford it.\n\r",ch);
		return;
	}

	if (ch->level < pet->level){
		send_to_char("You're not powerful enough to master this pet.\n\r",ch);
		return;
	}

	roll = number_percent();
	if (roll < get_skill(ch,gsn_haggle)){
		cost -= cost / (7 - ch->getslvl(gsn_haggle)) * roll / 100;
		printf_to_char(ch,"You haggle the price down to %d coins.\n\r",cost);
		check_improve(ch,gsn_haggle,true,4);
	
	}
	deduct_cost(ch,cost);
	pet	= create_mobile(pet->pIndexData);
	pet->setact(AT_PET);
	pet->setaff(AF_CHARM);
	pet->setcomm(CM_NOTELL);
	pet->setcomm(CM_NOSHOUT);
	pet->setcomm(CM_NOCHANNELS);

	argument = one_argument(argument,arg);
	if (arg[0] != '\0')
	{
		sprintf(buf,"%s %s",pet->name,arg);
		free_string(pet->name);
		pet->name = str_dup(buf);
	}

	sprintf(buf,"%sA neck tag says 'I belong to %s'.\n\r",pet->description,ch->name);
	free_string(pet->description);
	pet->description = str_dup(buf);

	char_to_room(pet,ch->in_room);
	add_follower(pet,ch,false);
	group_char(ch,pet);
	ch->pet = pet;
	send_to_char("Enjoy your pet.\n\r",ch);
	act("$n bought $N as a pet.",ch,NULL,pet,TO_ROOM);
	return;
}

void spell_spirit_guide(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	char buf[MSL];
	CHAR_DATA *pet;
	AFFECT_DATA af;
	const char *sizer;
	const char *sizer2;
	const char *animal;
	int race;

    if ( ch->isaff(AF_CHARM) )
		return;

	if ( ch->pet != NULL )
	{
		send_to_char("You already have a follower.\n\r",ch);
		return;
	}

	send_to_char("You call for the guidance and assistance of your spirit guide.\n\r",ch);
	act("$n calls for the guide and assistance of $s spirit guide.",ch,NULL,NULL,TO_ROOM);

	pet	= create_mobile(get_mob_index(1));
	pet->level = ch->level;

	if(ch->level >= 100)
	{
		sizer = "an enormous";
	    pet->size = SIZE_GIANT;
	}
	if(ch->level < 100)
	{
		sizer = "a massive";
	    pet->size = SIZE_HUGE;
	}
	if(ch->level <= 50)
	{
		sizer = "an angry";
	    pet->size = SIZE_MEDIUM;
	}
	if(ch->level <= 25)
	{
		sizer = "a tiny";
	    pet->size = SIZE_TINY;
	}

	if (ch->spiritguide == 0)
		ch->spiritguide = number_range(1,MAX_SPIRITS-1);

	animal = spirit_table[ch->spiritguide].name;
	sizer2 = spirit_table[ch->spiritguide].verb;

	sprintf(buf, "%s %s spirit", sizer, animal);
	free_string( pet->short_descr );
	pet->short_descr = str_dup( buf );

	sprintf( buf, "spirit %s %s", sizer, animal );
	free_string( pet->name );
	pet->name = str_dup( buf );

	sprintf( buf, "The spirit of %s %s %s\n\r", sizer, animal, sizer2 );
	free_string( pet->long_descr );
	pet->long_descr = str_dup( buf );

	sprintf( buf, "A faintly visible spirit of %s %s %s'.\n\r", sizer, animal,sizer2 );
	free_string( pet->description );
	pet->description = str_dup( buf );

	race                  = race_lookup( "spirit" );
	pet->race             = race;
	pet->armor[AC_PIERCE] = 0 - (pet->level *2);
	pet->armor[AC_BASH]   = 0 - (pet->level *2);
	pet->armor[AC_SLASH]  = 0 - (pet->level *2);
	pet->armor[AC_EXOTIC] = 0 - (pet->level *3);
	pet->alignment        = ch->alignment;
	pet->hitroll          = pet->level;
    pet->setmana(dice((pet->level / 2)+1,pet->level/2) + (pet->level));
    pet->max_hit          = dice((pet->level/2)+1,pet->level) + (pet->level*2);
	pet->hit              = pet->max_hit;

    pet->setaff(AF_PASS_DOOR);
    pet->setaff(AF_FLYING);
    pet->setaff(AF_DETECT_INVIS);
    pet->setaff(AF_PASS_DOOR);
	pet->setaff(AF_DARK_VISION);
	pet->setaff(AF_CHARM);

	if (pet->level >= 100)
		affect_set(pet,TO_AFFECTS,skill_lookup("sanctuary"),pet->level,1,-1,APPLY_NONE,0,AF_SANCTUARY);

    pet->damage[DICE_NUMBER] = (pet->level/2)+1;
    pet->damage[DICE_TYPE]   = 3;
    pet->damage[DICE_BONUS]  = (pet->level /4) + 1;

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch, false );
	group_char(ch,pet);
	pet->master = ch;
	ch->pet = pet;
	printf_to_char(ch,"The spirit of %s %s fades into existance to guide you.\n\r",sizer,animal);
	act( "$N fades into existance and joins $n as $s guide.", ch, NULL, pet, TO_ROOM );
	return;
}

void do_falconcall(CHAR_DATA *ch){
	CHAR_DATA *pet;
	char buf[MSL];

	if(ch->isaff(AF_CHARM))
		return;

	if(ch->pet){
		if(ch->pet->isoff(OF_FALCON)){
			if(ch->pet->in_room == ch->in_room)
				ch->send("Your falcon is already here.\n\r");
			else{
				ch->pet->remoff(OF_WAIT);
				ch->pet->remaff(AF_HIDE);
				char_from_room(ch->pet);
				char_to_room(ch->pet,ch->in_room);
				act("$n comes soaring in.",ch->pet,NULL,NULL,TO_ROOM);
			}
			return;
		}
		ch->send("You already have a follower.\n\r");
		return;
	}

	if(!IS_NATURE_BG(ch)){
		ch->send("You're not in the proper setting to call a falcon.\n\r");
		return;
	}

	if(number_percent() > get_skill(ch,gsn_falconry))
	{
		send_to_char("You failed.\n\r",ch);
		return;
	}

	send_to_char("You whistle loudly, calling forth a falcon friend.\n\r",ch);
	act("$n whistles, summoning a falcon to $s aide.",ch,NULL,NULL,TO_ROOM);
	
	pet	= create_mobile(get_mob_index(2));
	pet->level = ch->level;

	pet->size = SIZE_MEDIUM;

	sprintf(buf, "a large falcon");
	free_string( pet->short_descr );
	pet->short_descr = str_dup( buf );

	sprintf( buf, "large falcon" );
	free_string( pet->name );
	pet->name = str_dup( buf );

	sprintf( buf, "A large falcon obediently keeps watch.\n\r" );
	free_string( pet->long_descr );
	pet->long_descr = str_dup( buf );

	sprintf( buf, "A large, beautiful falcon is here.\n\r");
	free_string( pet->description );
	pet->description = str_dup( buf );

	pet->armor[AC_PIERCE] = 0 - (pet->level *2);
	pet->armor[AC_BASH]   = 0 - (pet->level *2);
	pet->armor[AC_SLASH]  = 0 - (pet->level *2);
	pet->armor[AC_EXOTIC] = 0 - (pet->level *2.5);
	pet->alignment        = ch->alignment;
	pet->hitroll          = pet->level;
    pet->setmana(100);
    pet->max_hit          = dice((pet->level/2)+1,pet->level) + (pet->level*2);
	pet->hit              = pet->max_hit;

    pet->damage[DICE_NUMBER] = (pet->level/2)+1;
    pet->damage[DICE_TYPE]   = 3;
    pet->damage[DICE_BONUS]  = (pet->level /4) + 1;

	pet->setaff(AF_FLYING);
	pet->setaff(AF_DARK_VISION);
	pet->setaff(AF_DETECT_HIDDEN);
	pet->setaff(AF_CHARM);
	pet->setoff(OF_FALCON);

	char_to_room( pet, ch->in_room );
	add_follower( pet, ch, false );
	group_char(ch,pet);
	pet->master = ch;
	ch->pet = pet;
	act("$N flies in and follows you.",ch,NULL,pet,TO_CHAR);
	act( "$N flies in and joins $n.",ch,NULL,pet,TO_ROOM);
	return;
}
