#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"

/*
 * Local functions.
 */
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

bool IS_NATURE_BG		( CHAR_DATA* );
void do_falconcall		( CHAR_DATA* );
void do_falconwait		( CHAR_DATA* );
void do_falconeye		( CHAR_DATA* );
void do_falconscout		( CHAR_DATA* );
void do_falcondeliver	( CHAR_DATA*,OBJ_DATA*,CHAR_DATA* );
void one_hit			( CHAR_DATA*,CHAR_DATA*,int,bool,bool );

void do_untangle(CHAR_DATA *ch, char *argument){
	int sn = gsn_untangle,skill = get_skill(ch,sn);
	if(skill < 1){
		ch->send("You untangle your hair. Good job!\n\r");
		return;
	}
	ch->send("Hello.\n\r");
}

void do_blend(CHAR_DATA *ch,char *argument){
    AFFECT_DATA af;

	if (ch->isaff(AF_CAMOFLAGE)){
		ch->send("You are already blending with your environment.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You must be in a more natural environment to blend.\n\r");
		return;
	}

	if(ch->getmana() < 50){
		ch->send("You lack the power to do this.\n\r");
		return;
	}
	
	if(roll_chance(ch,get_skill(ch,gsn_blend) * .85)){
		ch->modmana(-50);
		affect_set(ch,TO_AFFECTS,gsn_blend,ch->level,ch->getslvl(gsn_blend),ch->level/2,APPLY_NONE,0,AF_CAMOFLAGE);

		act( "$n seems to dissapear...", ch, NULL, NULL, TO_ROOM );
		ch->send("You blend in with the natural environment around you.\n\r");
		check_improve(ch,gsn_blend,true,3);
	}
	else{
		ch->modmana(-25);
		ch->send("You fail.\n\r");
		check_improve(ch,gsn_blend,false,3);
	}
	WAIT_STATE(ch,skill_table[gsn_blend].beats);
}

void do_firemaking(CHAR_DATA *ch, char *argument){
	int sn = gsn_survival;
	char arg[MSL];
	OBJ_DATA *obj,*fire;

	argument = one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Syntax: makefire <lumber>\n\r");
		return;
	}
	if (!(obj = get_obj_carry(ch,arg,ch))){
		ch->send("You do not have that item.\n\r");
		return;
	}
	if(obj->item_type != ITEM_LUMBER){
		ch->send("You may only make fires with lumber.\n\r");
		return;
	}
	if(ch->getmana() < 100){
		ch->send("You lack the energy to make a fire.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You must be in a natural environment to make fires.\n\r");
		return;
	}
	if(number_percent() <= get_skill(ch,sn) * .9){
		ch->send("You lay out your lumber and call a natural spell, and the wood erupts into a warming fire.\n\r");
		act("$n quietly concentrates, as the lumber he laid out erupts into a warm fire.",ch,NULL,NULL,TO_ROOM);
		extract_obj( obj );

		fire = create_object( get_obj_index( OBJ_VNUM_FIRE ), 0 );
		fire->timer = ch->level;
		ch->in_room->light++;
		obj_to_room( fire, ch->in_room );
		check_improve(ch,sn,true,1);
	}
	else{
		if(number_percent() <= 50){
			ch->send("You lay out your lumber and call a natural spell, but lose your concentration and fail.\n\r");
			act("$n quietly concentrates, and calls a natural spell but fails miserably.",ch,NULL,NULL,TO_ROOM);
			obj_from_char( obj );
			obj_to_room(obj,ch->in_room);
		}
		else{
			ch->send("You lay out your lumber and call a natural spell, but lose your concentration and set yourself on fire!.\n\r");
			act("$n quietly concentrates, and calls a natural spell but instead sets $mself on fire!",ch,NULL,NULL,TO_ROOM);
			spell_damage( ch, ch, number_range(ch->level/2,ch->level), find_spell(ch,"fireball"),DAM_FIRE,true);
		}
		check_improve(ch,sn,false,3);
	}
	ch->modmana(-100);
	WAIT_STATE(ch,skill_table[sn].beats);
}

void do_gather(CHAR_DATA *ch, char *argument){
	char arg[MSL],buf1[MSL],buf2[MSL],buf3[MSL];
	OBJ_DATA *obj;

	argument = one_argument(argument,arg);

	if(get_skill(ch,gsn_survival) < 1)
		return;

	if(!IS_NATURE_BG(ch)){
		ch->send("You're not in the proper setting to gather stuff.\n\r");
		return;
	}

	if (!arg[0] || (str_prefix(arg,"tinder") && str_prefix(arg,"herbs") && str_prefix(arg,"food") && str_prefix(arg,"feed"))){
		ch->send("Syntax: gather <tinder/herbs/food/feed>\n\r");
		return;
	}

	if(ch->getmana() < 25){
		ch->send("You lack the power to do this.\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_survival].beats);

	if(number_percent() <= get_skill(ch,gsn_survival) * .95){
		ch->modmana(-25);
		check_improve(ch,gsn_survival,true,10);
		if(!str_prefix(arg,"tinder")){
			obj = create_object( get_obj_index( OBJ_VNUM_TINDER ), 0 );
			obj->timer = ch->level;
			obj_to_char( obj, ch );
			act("You search around for a while and eventually manage to gather some tinder.",ch,NULL,NULL,TO_CHAR);
			act("$n searches around for a while and eventually manages to gather some tinder.",ch,NULL,NULL,TO_ROOM);
			return;
		}
		else if(!str_prefix(arg,"herbs")){
			obj = create_object( get_obj_index( OBJ_VNUM_HERBS ), 0 );
			obj->timer = ch->level;
			obj_to_char( obj, ch );
			obj->value[0] = ch->level;
			switch(number_range(1,10)){
				case 1:
					obj->value[1] = skill_lookup("poison");
					break;
				case 2:
				case 3:
					obj->value[1] = skill_lookup("cure poison");
					break;
				case 4:
				case 5:
					obj->value[1] = skill_lookup("cure blindness");
					break;
				case 6:
				case 7:
					ch->send("You failed.\n\r");
					obj_from_char(obj);
					extract_obj(obj);
					return;
				default:
					break;
			}
			act("You search around for a while and eventually manage to gather some herbs.",ch,NULL,NULL,TO_CHAR);
			act("$n searches around for a while and eventually manages to gather some herbs.",ch,NULL,NULL,TO_ROOM);
			return;
		}
		else if(!str_prefix(arg,"feed")){
			act("You search around for a while and eventually manage to gather some feed.",ch,NULL,NULL,TO_CHAR);
			act("$n searches around for a while and eventually manages to gather some feed.",ch,NULL,NULL,TO_ROOM);
			return;
		}
		else if(!str_prefix(arg,"food")){
			obj = create_object( get_obj_index( 80 ), 0 );
			switch(number_range(1,10)){
				case 1:
					sprintf( buf1, "%s", "a pile of mushrooms" );
					sprintf( buf2, "%s", "A pile of mushrooms are here." );
					sprintf( buf3, "%s", "pile mushrooms" );
					obj->timer = ch->level*.75;
					obj->value[0] = ((ch->level/10) /2) + 1;
					obj->value[1] = 10;
					break;
				case 2:
				case 3:
					sprintf( buf1, "%s", "a red apple" );
					sprintf( buf2, "%s", "A beautiful red apple is here." );
					sprintf( buf3, "%s", "red apple" );
					obj->timer = ch->level/2;
					obj->value[0] = ((ch->level/10) /2) + 1;
					obj->value[1] = 10;
					break;
				case 4:
				case 5:
					sprintf( buf1, "%s", "some berries" );
					sprintf( buf2, "%s", "A cluster of berries lies in the dirt." );
					sprintf( buf3, "%s", "cluster some berries" );
					obj->timer = ch->level/4;
					obj->value[0] = ((ch->level/10) /2) + 1;
					obj->value[1] = 5;
					break;
				case 6:
				case 7:
				case 8:
				case 9:
					sprintf( buf1, "%s", "a few walnuts" );
					sprintf( buf2, "%s", "A few walnuts lie here." );
					sprintf( buf3, "%s", "few walnuts" );
					obj->timer = ch->level;
					obj->value[0] = ((ch->level/10) /2) + 1;
					obj->value[1] = 10;
					break;
				default:
					send_to_char("You fail.\n\r",ch);
					return;
			}
			free_string( obj->short_descr );
			free_string( obj->description );
			free_string( obj->name );
			obj->short_descr	= str_dup( buf1 );
			obj->description	= str_dup( buf2 );
			obj->name			= str_dup( buf3 );
			obj_to_char( obj, ch );
			act("You search around for a while and eventually manage to gather some food.",ch,NULL,NULL,TO_CHAR);
			act("$n searches around for a while and eventually manages to gather some food.",ch,NULL,NULL,TO_ROOM);
			return;
		}
		else{
			ch->send("Syntax: gather <tinder/herbs/feed>\n\r");
			return;
		}
	}
	else{
		ch->modmana(-10);
		check_improve(ch,gsn_survival,false,5);
		send_to_char("You fail!\n\r",ch);
	}
}
void do_falconry(CHAR_DATA *ch, char *argument){
	int skill;
	char arg[MSL];

	argument = one_argument(argument,arg);

	if((skill = get_skill(ch,gsn_falconry) * .9) < 1)
		return;

	if ( arg[0] == '\0' || (str_prefix(arg,"call") && str_prefix(arg,"scout") && str_prefix(arg,"deliver"))){
		ch->send("Syntax: falcon <call/scout>\n\r");
		return;
    }

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gFalconry({c%d{x)\n\r",skill);
	if(number_percent() <= skill){
		if(!str_prefix(arg,"call"))
			do_falconcall(ch);
		if(!str_prefix(arg,"scout"))
			do_falconscout(ch);
		if(!str_prefix(arg,"wait"))
			do_falconscout(ch);
		if(!str_prefix(arg,"deliver")){
			CHAR_DATA *victim;
			OBJ_DATA *obj;
			char buf[MSL],buf2[MSL];
			one_argument(argument,buf);
			if ((obj = get_obj_carry(ch,buf,ch)) == NULL){
				ch->send("You don't have that item.\n\r");
				return;
			}
			one_argument(argument,buf2);
			if ((victim = get_char_world(ch,buf2)) == NULL){
				ch->send("Who?\n\r");
				return;
			}
			do_falcondeliver(ch,obj,victim);
		}
		check_improve(ch,gsn_falconry,true,3);
	}
	else{
		ch->send("You fail.\n\r");
		check_improve(ch,gsn_falconry,false,2);
	}
	WAIT_STATE(ch,skill_table[gsn_falconry].beats);
	return;
}

void do_falconwait(CHAR_DATA *ch){
	CHAR_DATA *falcon;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;

	if(!(falcon = ch->pet) || !falcon->isoff(OF_FALCON)){
		ch->send("You don't have a falcon.\n\r");
		return;
	}
	if(falcon->in_room != ch->in_room){
		ch->send("This won't work if they're not in the same room as you.\n\r");
		return;
	}
	act("You order $N to stay here. Use 'falcon call' to retrieve $M.",ch,NULL,falcon,TO_CHAR);
	act("$n orders $N to stay here.",ch,NULL,falcon,TO_NOTVICT);
	falcon->setoff(OF_WAIT);
	falcon->setaff(AF_HIDE);
	
}
void do_hawkeye(CHAR_DATA *ch,char *argument){
	CHAR_DATA *falcon;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	int sn=gsn_raptor;

	if(!(falcon = ch->pet) || !falcon->isoff(OF_FALCON)){
		ch->send("You don't have a falcon.\n\r");
		return;
	}
	if(falcon->in_room == ch->in_room){
		ch->send("This won't work if they're in the same room as you.\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_raptor].beats);
	if(!roll_chance(ch,get_skill(ch,gsn_raptor)*.8)){
		ch->send("You fail.\n\r");
		check_improve(ch,gsn_raptor,false,1);
		return;
	}
	original = ch->in_room;
	on = ch->on;
	char_from_room(ch);
	char_to_room(ch,falcon->in_room);
	do_function(ch,&do_look,NULL);
	char_from_room(ch);
	char_to_room(ch,original);
	check_improve(ch,gsn_raptor,true,1);
}

void do_falcondeliver(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim){
	CHAR_DATA *falcon;
	if((falcon = ch->pet) == NULL || !falcon->isoff(OF_FALCON)){
		ch->send("You don't have a falcon..\n\r");
		return;
	}
	if (obj->item_type != ITEM_PARCHMENT){
		ch->send("You can only deliver parchments.\n\r");
		return;
	}
	if (!can_see(ch,victim)){
		ch->send("Who?\n\r");
		return;
	}
	act("You send $N on $S way to $n.",victim,obj,falcon,TO_CHAR);
	act("$n send $N on $S off.",ch,obj,falcon,TO_ROOM);
	act("$N flies in and delivers a message to you.",victim,obj,falcon,TO_CHAR);
	act("$N flies in and delivers something to $n.",victim,obj,falcon,TO_ROOM);
	return;
}

void do_falconscout(CHAR_DATA *ch){
	CHAR_DATA *wch;
	CHAR_DATA *falcon;
	DESCRIPTOR_DATA *d;
	static char buf[512];

    buf[0] = '\0';

	if((falcon = ch->pet) == NULL || !falcon->isoff(OF_FALCON)){
		ch->send("You don't have a falcon..\n\r");
		return;
	}

	act("$N swoops off high into the sky.",ch,NULL,falcon,TO_ROOM);
	act("You send $N off, and $E swoops off high into the sky.",ch,NULL,falcon,TO_CHAR);

	for (d = descriptor_list; d != NULL; d = d->next){
		if (d->connected != CON_PLAYING || !can_see(falcon, d->character))
			continue;
		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see(falcon, wch))
			continue;

		if(wch->in_room->area == ch->in_room->area){
			strcat(buf,wch->name);
			strcat(buf,"\n\r");
		}
	}

	act("$N swoops back and lands on $n's shoulder, whispering in $s ear.",ch,NULL,falcon,TO_ROOM);
	act("$N returns, landing on your shoulder, and whispers into your ear.",ch,NULL,falcon,TO_CHAR);

	send_to_char(buf,ch);
}

void do_prepare(CHAR_DATA *ch, char *argument){
	int sn = gsn_survival,chance = 0;
	OBJ_DATA *corpse, *obj;
	bool found=false;
	char arg[MSL];

	argument = one_argument(argument,arg);

	if(arg[0] == '\0'){
		ch->send("Prepare what?\n\r");
		return;
	}
	if(ch->getmana() < 20){
		ch->send("You lack the power to do this.\n\r");
		return;
	}
	if (!(corpse = get_obj_carry(ch,arg, ch))){
		ch->send("You are not carrying that.\n\r");
		return;
	}
	if(corpse->item_type != ITEM_CORPSE_NPC){
		ch->send("This skill only works on NPC corpses.\n\r");
		return;
	}
	if (corpse->contains){
		ch->send("There's foreign objects inside that corpse.\n\r");
		return;
	}
	for(obj = ch->in_room->contents;obj;obj = obj->next_content){
		if (obj->item_type == ITEM_FIRE){
			found=true;
			break;
		}
	}
	if(!found){
		ch->send("You must be in a room with a fire to cook your meat.\n\r");
		return;
	}

	if((chance = get_skill(ch,sn) *.8) < 1)
		return;

	WAIT_STATE(ch,skill_table[sn].beats);

	if(number_percent() <= chance){
		act("You skin, gut, and cook $p, turning it into some juicy meat.. Tasty!",ch,corpse,NULL,TO_CHAR);
		act("$n skins, guts, and cooks $p, turning it into some juicy meat.. Tasty!",ch,corpse,NULL,TO_ROOM);
		extract_obj(corpse);
		corpse = create_object(get_obj_index(67),0);
		corpse->timer = ch->level/2;
		obj_to_char(corpse,ch);
		corpse->value[0] = (ch->level/10) + 1;
		corpse->value[1] = 20;
		check_improve(ch,sn,true,4);
		ch->modmana(-20);
	}
	else{
		act("You fiddle with $p, and mess it up, ruining the meat.",ch,corpse,NULL,TO_CHAR);
		act("$n fiddles with $p and messes up and ruins the meat.",ch,corpse,NULL,TO_ROOM);
		extract_obj(corpse);
		ch->modmana(-10);
		check_improve(ch,sn,false,4);
	}
}

bool check_awareness(CHAR_DATA *ch,CHAR_DATA *victim,int vsn){
	int cchance = number_range(1,3),vchance = number_range(1,3),sn = gsn_awareness,skill = number_range(1,50),vskill = number_range(1,50);
	AFFECT_DATA *vaf = affect_find(victim->affected,vsn),*caf = affect_find(ch->affected,sn);
	if (get_skill(ch,sn) < 1 || !ch->isaff(AF_DETECT_HIDDEN))
		return false;

	if(get_skill(victim,vsn) > 0){
		vchance = vaf ? vaf->modifier : 1;//nashfix?
		vskill = get_skill(victim,vsn);
	}
	vchance *= vskill;
	if(get_skill(ch,sn) > 0){
		cchance = caf ? caf->modifier : 1;
		skill = get_skill(ch,sn);
	}
	cchance *= skill;
	if(cchance > vchance){
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}

bool check_awareness(CHAR_DATA *ch,OBJ_DATA *obj){
	int cchance,vchance = number_range(1,3) * number_range(1,50),sn = gsn_awareness,skill = number_range(1,50);
	if (get_skill(ch,sn) < 1 || !ch->isaff(AF_DETECT_HIDDEN))
		return false;

	if(get_skill(ch,sn) < 1)
		cchance = number_range(1,3);
	else{
		cchance = affect_find(ch->affected,sn)->modifier;
		skill = get_skill(ch,sn);
	}
	cchance *= skill;
	if(cchance > vchance){
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}

void do_awareness(CHAR_DATA *ch, char *argument){
    AFFECT_DATA af;

	if (ch->isaff(AF_DETECT_HIDDEN)){
		ch->send("You are already alert to your surroundings.\n\r");
		return;
	}

	if(ch->getmana() < 45){
		ch->send("You do not have the power to do this.\n\r");
		return;
	}
	ch->modmana(-45);

	if(number_percent() <= get_skill(ch,gsn_awareness) *.9){
		affect_set(ch,TO_AFFECTS,gsn_awareness,ch->level,ch->getslvl(gsn_awareness),25,APPLY_WIS,ch->getslvl(gsn_awareness),AF_DETECT_HIDDEN);
		act("$n seems more alert...",ch,NULL,NULL,TO_ROOM);
		ch->send("You become more aware of your surroundings.\n\r");
		check_improve(ch,gsn_awareness,true,4);
	}
	else{
		ch->send("You fail.\n\r");
		check_improve(ch,gsn_awareness,false,2);
	}
	WAIT_STATE(ch,skill_table[gsn_awareness].beats);
    return;
}

int inverse_heal(CHAR_DATA *ch,int num){
	int skill = get_skill(ch,gsn_stamina),hp = ch->max_hit - ch->hit,mhp = ch->max_hit;
	if(skill < 1)
		return num;
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"INV%d\n\r",num);
	num = num * hp / mhp;
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"INV%d\n\r",num);
	num = num * skill / 100;
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"INV%d\n\r",num);
	check_improve(ch,gsn_stamina,true,1);

	return num;
}

void do_snare(CHAR_DATA *ch, char *argument){
	char arg[MIL];
	CHAR_DATA *victim;
	int sn = gsn_snare,skill;
	AFFECT_DATA af;

	one_argument(argument,arg);

	if ( IS_NPC(ch) || (skill = get_skill(ch,gsn_snare)*.6) < 1){
		send_to_char("Wha?\n\r",ch);
		return;
	}

    if (!(victim = grab_char(ch,arg,true)))
		return;

	if(victim->isaff(AF_SNARED) || victim->isaff(AF_ENTANGLED)){
		send_to_char("They're already wrapped up!\n\r",ch);
		return;
	}

	if(!canFight(ch,victim,true))
		return;

	skill += get_curr_stat(ch,STAT_AGI);
	skill -= get_curr_stat(victim,STAT_AGI) * 3 / 2;

	if (ch->isoff(OF_FAST) || ch->isaff(AF_HASTE))
		skill += 10;
	if (victim->isoff(OF_FAST) || victim->isaff(AF_HASTE))
		skill -= 20;

	skill += (ch->level - victim->level) * 2;

	if (roll_chance(ch,skill)){
		act("$n has ensnared you!",ch,NULL,victim,TO_VICT);
		act("You have ensnared $N!",ch,NULL,victim,TO_CHAR);
		act("$n ensnares $N!",ch,NULL,victim,TO_NOTVICT);

		affect_set(victim,TO_AFFECTS,gsn_snare,ch->level,ch->getslvl(sn),UMAX(1,ch->getslvl(sn)),APPLY_NONE,0,AF_SNARED);

		check_improve(ch,gsn_snare,true,1);
		DAZE_STATE(victim,2 * PULSE_VIOLENCE);
		WAIT_STATE(ch,skill_table[gsn_snare].beats);
	}
	else{
		act("You fail to ensnare $N!",ch,NULL,victim,TO_CHAR);
		act("$n fails to ensnare $N!",ch,NULL,victim,TO_NOTVICT);
		act("$n fails to ensnare you!",ch,NULL,victim,TO_VICT);
		skill_damage(ch,victim,0,gsn_snare,DAM_OTHER,true);
		WAIT_STATE(ch,skill_table[gsn_snare].beats*2/3);
		check_improve(ch,gsn_snare,false,1);
	}
}

void do_desertcover(CHAR_DATA *ch,char *argument){
	AFFECT_DATA af;

	if(MOUNTED(ch)){
		send_to_char("You can't sneak while riding.\n\r", ch);
		return;
	}
	if(get_skill(ch,gsn_desertcover) < 1){
		send_to_char("You don't know how to sneak!\n\r",ch);
		return;
	}
	if(ch->in_room->sector_type != SECT_DESERT){
		send_to_char("You can only sneak in the desert!\n\r",ch);
		return;
	}

	send_to_char("You become one with the sands.\n\r",ch);
	unSneak(ch);

	if(number_percent() < get_skill(ch,gsn_desertcover)){
		check_improve(ch,gsn_desertcover,true,3);
		affect_set(ch,TO_AFFECTS,gsn_desertcover,ch->level,ch->getslvl(gsn_desertcover),ch->level,APPLY_NONE,0,AF_CAMOFLAGE);
	}
	else{
		send_to_char("You fail.\n\r",ch);
		check_improve(ch,gsn_desertcover,false,3);
	}
	WAIT_STATE(ch,skill_table[gsn_desertcover].beats);
}

void do_quickescape(CHAR_DATA *ch, char *argument)
{
	int chance,door,dirty=0;
	char arg[MSL],buf[MSL];
	EXIT_DATA *pexit;
    ROOM_INDEX_DATA *in_room,*to_room;
	
    argument = one_argument(argument,arg);
    
	if ( arg[0] == '\0' )
    {
		send_to_char("You must make your escape in a certain direction!\n\r",ch);
		return;
    }

	if (ch->fighting == NULL)
	{
		send_to_char("You're not in combat!\n\r",ch);
		return;
	}

		 if ( !strcmp(arg,"n") || !strcmp(arg,"north") ) door = 0;
    else if ( !strcmp(arg,"e") || !strcmp(arg,"east" ) ) door = 1;
    else if ( !strcmp(arg,"s") || !strcmp(arg,"south") ) door = 2;
    else if ( !strcmp(arg,"w") || !strcmp(arg,"west" ) ) door = 3;
    else if ( !strcmp(arg,"u") || !strcmp(arg,"up"   ) ) door = 4;
    else if ( !strcmp(arg,"d") || !strcmp(arg,"down" ) ) door = 5;
    else
    {
		send_to_char( "Can't go that way!\n\r", ch );
		return;
    }
    in_room = ch->in_room;

    if ( (pexit = in_room->exit[door]) == NULL || (to_room = pexit->u1.to_room) == NULL )
    {
        send_to_char( "That won't work!\n\r", ch );
		return;
    }

	if (IS_NPC(ch) || get_skill(ch,gsn_quickescape) <1)
    {
        send_to_char("You lack those skills!\n\r",ch);
		return;
    }

	switch(ch->in_room->sector_type)
	{
		case (SECT_DESERT):
		case (SECT_TUNNEL):
		case (SECT_DIRTROAD):
		case (SECT_DIRTPATH):
			chance = get_skill(ch,gsn_quickescape) * .95;
			dirty=2;
			break;
		case (SECT_HILLS):
		case (SECT_FIELD):
		case (SECT_VOLCANO):
		case (SECT_MOUNTAIN):
		case (SECT_ROAD):
			chance = get_skill(ch,gsn_quickescape) * .9;
			dirty=1;
			break;
		default:
			chance = get_skill(ch,gsn_quickescape) *.75;
			break;
	}

	sprintf(buf, "%s creates a whirlwind of dirt, disappearing into the sandstorm!\n\r",ch->name);

	if(number_percent() < chance)
	{
		check_improve(ch,gsn_quickescape,true,2);
		if(dirty == 1)
		{
		send_to_char("You create a flurry of dust to blind your enemies and take the opportunity to escape!\n\r",ch);
			CHAR_DATA *gch;
			chance *= .75;
			for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
			{
				if ( is_same_group(gch,ch) || ch == gch)
					continue;

				if(gch->isaff(AF_BLIND) || number_percent() > chance)
					send_to_char(buf,gch);
				else
				{
					AFFECT_DATA af;

					send_to_char("The sands of a sudden dust storm blind you!\n\r",gch);
					WAIT_STATE(gch,skill_table[gsn_quickescape].beats*2);
					act("$n's dust storm blinds $N!",ch,NULL,gch,TO_NOTVICT);

					affect_set(gch,TO_AFFECTS,gsn_dirt,ch->level,ch->getslvl(gsn_quickescape),0,APPLY_HITROLL,-4,AF_BLIND);
				}
			}
		}
		else if(dirty == 2)
		{
		send_to_char("You create a flurry of dust to blind your enemies and take the opportunity to escape!\n\r",ch);
			CHAR_DATA *gch;
			chance *= .8;
			for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
			{
				if ( is_same_group( gch, ch ) || ch == gch)
					continue;

				if(gch->isaff(AF_BLIND) || number_percent() > chance)
				{
					WAIT_STATE(gch,skill_table[gsn_quickescape].beats);
					send_to_char(buf,gch);
				}
				else
				{
					AFFECT_DATA af;

					send_to_char("The sands of a sudden dust storm blind you!\n\r",gch);
					WAIT_STATE(gch,skill_table[gsn_quickescape].beats*2);
					act("$n's dust storm blinds $N!",ch,NULL,gch,TO_NOTVICT);

					affect_set(gch,TO_AFFECTS,gsn_dirt,ch->level,ch->getslvl(gsn_quickescape),1,APPLY_HITROLL,-4,AF_BLIND);
				}
			}
		}
		else
		{
			send_to_char("You create a drake distraction and take the opportunity to escape!\n\r",ch);
			act("$n points to the sky, '{GLOOK! It's a cold drake!{x' and, in the panic, $e manages to escape!",ch,NULL,NULL,TO_ROOM);
			WAIT_STATE(ch->fighting,skill_table[gsn_quickescape].beats);
		}
		stop_fighting(ch,true);
		move_char(ch,door,false,false);
		return;
	}
	else
	{
		WAIT_STATE(ch,skill_table[gsn_quickescape].beats);
		send_to_char("You stumble and fall!\n\r",ch);
		check_improve(ch,gsn_quickescape,false,1);
		return;
	}
}

void do_mirage(CHAR_DATA *ch, char * argument){
	AFFECT_DATA af;
	int sn = gsn_mirage,skill = get_skill(ch,sn);

	if(skill < 1){
		ch->send("You could only do this one drunk.\n\r");
		return;
	}
	if (ch->isaff(AF_MIRAGE)){
		send_to_char("Your mirage army is already deployed.\n\r",ch);
		return;
	}
	if(ch->getmana() < 100)
	{
		send_to_char("You lack the energy to deploy a mirage army.\n\r",ch);
		return;
	}

	ch->modmana(-100);
	WAIT_STATE(ch,skill_table[gsn_mirage].beats);

	affect_set(ch,TO_AFFECTS,gsn_mirage,ch->level,ch->getslvl(gsn_mirage),9,APPLY_AC,(ch->level / 5) * -1,AF_MIRAGE);
	act("A mirage of several bandits, each identical to $n seem to phase into existance.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You are encircled by a team of mirages.\n\r",ch);
    return;
}

bool check_endure(CHAR_DATA *ch){
	int chance;

	if(IS_NPC(ch) || (chance = get_skill(ch,gsn_endure)) < 1)
		return false;

	chance /= 2;

	if(roll_chance(ch,chance)){
		check_improve(ch,gsn_endure,true,8);
		return true;
	}
	else
		check_improve(ch,gsn_endure,false,8);
	return false;
}

void do_tame(CHAR_DATA *ch,char *argument){
	CHAR_DATA *mount;
	int chance = get_skill(ch,gsn_mustang_heart) * .7;

	if (!argument[0]){
		ch->send("You must specify which wild horse you wish to tame.\n\r");
		return;
	}
	if (!(mount = get_char_room(ch,NULL,argument))){
		ch->send("You do not see anything here by that name.\n\r");
		return;
	}
	if (!IS_NPC(mount)){
		ch->send("That's perverted...\n\r");
		return;
	}
	if(!mount->isact(AT_MOUNT)){
		ch->send("You can only tame mountable beings.\n\r");
		return;
	}
	if(mount->isact(AT_WARHORSE)){
		act("$E's already tame.",ch,NULL,mount,TO_CHAR);
		return;
	}

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gMHeart({c%d{x)\n\r",chance);
	chance -= 10 * mount->level / ch->level;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gMHeart({c%d{x)\n\r",chance);
	chance += 10 * (get_curr_stat(ch,STAT_WIS) + get_curr_stat(ch,STAT_CHA)) / (STAT_MAX * 2);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gMHeart({c%d{x)\n\r",chance);
	WAIT_STATE(ch,skill_table[gsn_mustang_heart].beats);
	if(roll_chance(ch,chance)){
		if (mount->setact(AT_WARHORSE)){
			act("You tame $N into a reliable combat partner.",ch,NULL,mount,TO_CHAR);
			act("$n tames you!",ch,NULL,mount,TO_VICT);
			act("$n successfully tames $N as a partner in combat.",ch,NULL,mount,TO_NOTVICT);
			group_char(ch,mount);
			check_improve(ch,gsn_mustang_heart,true,2);
		}
		else
			ch->send("Something is buggy... might wanna tell an admin.\n\r");
		return;
	}
	act("You failed to tame $N.",ch,NULL,mount,TO_CHAR);
	act("$n attempts, and fails to tame you.",ch,NULL,mount,TO_VICT);
	act("$n attempts, and fails to tame $N.",ch,NULL,mount,TO_NOTVICT);
	check_improve(ch,gsn_mustang_heart,false,2);
}

int kindred_spirits(CHAR_DATA *ch,CHAR_DATA *aggro,int dam){
	CHAR_DATA *thePC = NULL;
	int chance,damshare=0,thp;

	if(IS_NPC(ch)){//Horse
		if (!ch->isact(AT_MOUNT) || !ch->isact(AT_WARHORSE) || !ch->mount || ch->in_room != ch->mount->in_room)
			return dam;
		if (!IS_NPC(ch->mount))
			thePC = ch->mount;
	}
	else{//Player
		if (!ch->mount || ch->in_room != ch->mount->in_room)
			return dam;
		thePC = ch;
	}

	if (!thePC || (chance = get_skill(thePC,gsn_kindred_spirits) * .3) < 1)
		return dam;
	if(thePC->iscomm(CM_DEBUG))printf_to_char(thePC,"{gKindred({c%d{x)\n\r",chance);

	if (roll_chance(ch,chance)){
		thp = ch->hit + ch->mount->hit;
		damshare = dam * ch->mount->hit / thp;
		if (ch->mount->hit <= damshare)
			return dam;
		check_improve(thePC,gsn_kindred_spirits,true,1);
		skill_damage(ch->mount,ch->mount,damshare,gsn_kindred_spirits,DAM_OTHER,true);
		return dam * ch->hit / thp;
	}
	check_improve(thePC,gsn_kindred_spirits,false,1);
	return dam;
}

void do_disable(CHAR_DATA *ch,char *argument){
	OBJ_DATA *trap;
	char arg[MSL];
	int chance;

	one_argument(argument,arg);

	trap = get_obj_list(ch,arg,ch->in_room->contents);
	if (!trap){
		send_to_char("You don't see that here.\n\r",ch);
		return;
	}

	if (trap->item_type != ITEM_TRAP){
		send_to_char("That's not a trap.\n\r",ch);
		return;
	}

	if ((chance = number_range(get_skill(ch,gsn_disabletraps) * .4,get_skill(ch,gsn_disabletraps) *.6)) < 1){
		WAIT_STATE(ch,skill_table[gsn_disabletraps].beats/2);
		send_to_char("Don't even try!\n\r",ch);
		return;
	}

	//stats were here once

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{gDisable({c%d{x)\n\r",chance);
	if (number_percent() < chance){
		WAIT_STATE(ch,skill_table[gsn_disabletraps].beats);
		act("After tinkering with it, you finally manage to disable $p.",ch,trap,NULL,TO_CHAR);
		act("After tinkering with it, $n finally manage to disable $p.",ch,trap,NULL,TO_ROOM);
		SET_BIT(trap->wear_flags,ITEM_TAKE);
		obj_to_char(trap,ch);
		check_improve(ch,gsn_disabletraps,true,2);
	}
	else{
		WAIT_STATE(ch,skill_table[gsn_disabletraps].beats * 1.5);
		if (number_percent() < 50){
			act("Your fingers slip up, but you manage to not set off the trap.. whew.",ch,NULL,NULL,TO_CHAR);
			act("$n fumbles about with a trap on the ground and jumps back suddenly.",ch,NULL,NULL,TO_ROOM);
			damage(ch,ch,UMAX(50,number_range(ch->hit/2,ch->hit/3)),TYPE_HIT,DAM_TRAPS,true);
		}
		else
			check_traps(ch,trap);
		check_improve(ch,gsn_disabletraps,false,5);
	}
}

bool check_raptor(CHAR_DATA *ch,CHAR_DATA *victim){
	int sn = gsn_raptor,skill = get_skill(ch,sn) / 2;
	if(skill < 1)
		return false;
	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}

void do_raptor(CHAR_DATA *ch,CHAR_DATA *victim){
	if(!check_raptor(ch,victim))
		return;

	one_hit(ch,victim,gsn_raptor,false,false);
}

int move_trek(CHAR_DATA *ch,int move){
	int skill = get_skill(ch,gsn_stalk) / 2;
	if(skill < 1 || IS_NPC(ch) || !IS_NATURE_BG(ch))
		return move;

	skill += get_curr_stat(ch,STAT_END) / 2;

	if(!roll_chance(ch,skill)){
		if(number_percent() < 30)
			check_improve(ch,gsn_stalk,false,1);
		return move;
	}
	ch->send("You trek.\n\r");
	move -= move / (7 - ch->getslvl(gsn_stalk));
	if(number_percent() < 30)
		check_improve(ch,gsn_stalk,true,1);
	return move;
}

bool mask_trek(CHAR_DATA *ch){
	int skill = get_skill(ch,gsn_stalk) / 4;
	if(skill < 1 || IS_NPC(ch) || !IS_NATURE_BG(ch))
		return false;

	if(!roll_chance(ch,skill)){
		check_improve(ch,gsn_stalk,false,1);
		return false;
	}
	check_improve(ch,gsn_stalk,true,1);
	return true;
}

void do_stalk(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
    AFFECT_DATA af;
	int sn = gsn_stalk,skill = get_skill(ch,sn);

	if(skill < 1){
		ch->send("You are not creepy enough to do that.\n\r");
		return;
	}
	if(!argument[0]){
		ch->send("Stalk who?\n\r");
		return;
	}
	if(!(victim = get_char_room(ch,NULL,argument))){
		if(!(victim = get_char_world(ch,argument))){
			ch->send("There is not anyone like that anywhere.\n\r");
			return;
		}
	}
	WAIT_STATE(ch,skill_table[sn].beats);
	if(roll_chance(ch,skill)){
		act("You silently begin to follow $N.",ch,NULL,victim,TO_CHAR);
		check_improve(ch,sn,true,1);
		ch->master = victim;
		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),UMAX(1,ch->getslvl(sn)),APPLY_AGI,ch->getslvl(sn),AF_STALK);
		return;
	}
	ch->send("You failed.\n\r");
	check_improve(ch,sn,false,1);
}

bool crunch_stalk(CHAR_DATA *ch,CHAR_DATA *victim){
	int sn = gsn_stalk,skill = get_skill(ch,sn);

	if(roll_chance(ch,skill)){
		if(number_percent() < ch->getslvl(sn) * 10){
			act("You grab $N, pulling $M back into the battle!",ch,NULL,victim,TO_CHAR);
			act("$n grabs $N, pulling $M back into the battle!",ch,NULL,victim,TO_NOTVICT);
			act("$n grabs you, pulling you back into the battle!",ch,NULL,victim,TO_VICT);
			check_improve(ch,sn,true,1);
			do_raptor(ch,victim);
			return true;
		}
	}
	ch->send("Fawr\n\r");
	check_improve(ch,sn,false,1);
	return false;
}

bool check_stalk(CHAR_DATA *victim){
	CHAR_DATA *vch,*vch_next;
	bool found = false;

	for(vch = victim->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(vch == victim || !can_see(vch,victim) || !vch->isaff(AF_STALK) || vch->master != victim || !vch->fighting || vch->fighting != victim)
			continue;
		if(crunch_stalk(vch,victim)){
			victim->send("Your stalker stops you dead in your tracks!\n\r");
			found = true;
		}
	}
	return found;
}

bool check_instinct(CHAR_DATA *ch){
	int sn = gsn_instinct,skill = get_skill(ch,sn) * .25;

	if(skill < 1)
		return false;

	skill += get_curr_stat(ch,STAT_WIS);

	if(roll_chance(ch,skill)){
		if(number_percent() <= ch->getslvl(sn) * 15){
			check_improve(ch,sn,true,1);
			return true;
		}
	}
	check_improve(ch,sn,false,1);
	return false;
}

void check_compress(CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *obj){
	int sn = gsn_bandage,skill = get_skill(ch,sn);

	if(skill < 1 || ch->getslvl(sn) < 5)
		return;

	if(!obj){
		if(roll_chance(ch,skill)){
			if(ch == victim){
				act("You compress your wound, enhancing the bandage's effectiveness.",ch,NULL,NULL,TO_CHAR);
				act("$n ties the bandage in a way to compress the wound.",ch,NULL,NULL,TO_ROOM);
			}
			else{
				act("You compress $N's wound, enhancing the bandage's effectiveness.",ch,NULL,victim,TO_CHAR);
				act("$n ties $N's bandage in a way to compress the wound.",ch,NULL,victim,TO_NOTVICT);
				act("$n ties your bandage to better compress the wound.",ch,NULL,victim,TO_VICT);
			}
			damage(ch,victim,-1 * UMAX(victim->max_hit,victim->hit + ch->getslvl(sn) * 100),sn,DAM_NONE,true);
			check_improve(ch,sn,true,1);
		}
		else
			check_improve(ch,sn,false,1);
	}
	else{
		if(roll_chance(ch,skill)){
			if(ch == victim){
				act("You compress your wound, enhancing the bandage's effectiveness.",ch,NULL,NULL,TO_CHAR);
				act("$n ties the bandage in a way to compress the wound.",ch,NULL,NULL,TO_ROOM);
			}
			else{
				act("You compress $N's wound, enhancing the bandage's effectiveness.",ch,NULL,victim,TO_CHAR);
				act("$n ties $N's bandage in a way to compress the wound.",ch,NULL,victim,TO_NOTVICT);
				act("$n ties your bandage to better compress the wound.",ch,NULL,victim,TO_VICT);
			}
			damage(ch,victim,-1 * UMAX(victim->max_hit,victim->hit + ch->getslvl(sn) * 100),sn,DAM_NONE,true);
			check_improve(ch,sn,true,1);
		}
		else
			check_improve(ch,sn,false,1);
	}
}

bool check_stamina(CHAR_DATA *ch){
	int sn = gsn_stamina,skill = get_skill(ch,sn) / 2;
	if(skill < 1)
		return false;
	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}
