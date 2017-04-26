#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "lookup.h"
#include "olc.h"
#include "db.h"
#include <stddef.h>
DECLARE_DO_FUN(do_wset);

/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location		( CHAR_DATA*,char* );
bool				write_to_descriptor	( int,char*,int );
bool				check_parse_name	( char* );
void				raw_kill			( CHAR_DATA*,CHAR_DATA* );
void				fwrite_obj			( CHAR_DATA*,OBJ_DATA*,FILE*,int );
void				write_max_con		( );
void				load_copyover_obj	( void );
void				fread_obj			( CHAR_DATA*,FILE* );
int					get_attackspeed		( CHAR_DATA*,bool );
void				check_penumbral		( CHAR_DATA* );
bool IS_HL(CHAR_DATA *ch);
int get_save_spell ( int,CHAR_DATA*,int );
int get_save_skill ( int,CHAR_DATA*,int );

ROOM_INDEX_DATA *find_location(CHAR_DATA *ch,char *arg){
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	if (is_number(arg))
		return get_room_index(atoi(arg));
	if ((victim = get_char_world(ch,arg)))
		return victim->in_room;
	if ((obj = get_obj_world(ch,arg)))
		return obj->in_room;
	return NULL;
}

void do_stat (CHAR_DATA *ch,char *argument){
   char arg[MIL],*string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

	string = one_argument(argument, arg);
	if (!arg[0]){
		ch->send("Syntax:\n\r  stat <name>\n\r  stat obj <name>\n\r  stat mob <name>\n\r  stat room <number>\n\r");
		return;
	}

	if (!str_cmp(arg,"room")){
		do_function(ch,&do_rstat,string);
		return;
	}
  
	if (!str_cmp(arg,"obj")){
		do_function(ch,&do_ostat,string);
		return;
	}

	if(!str_cmp(arg,"char") || !str_cmp(arg,"mob")){
		do_function(ch,&do_mstat,string);
		return;
	}

	obj = get_obj_world(ch,argument);
	if (obj){
		do_function(ch,&do_ostat,argument);
		return;
	}

	victim = get_char_world(ch,argument);
	if (victim){
		do_function(ch,&do_mstat,argument);
		return;
	}

	location = find_location(ch,argument);
	if (location){
		do_function(ch,&do_rstat,argument);
		return;
	}
	ch->send("Nothing by that name found anywhere.\n\r");
}

void do_rstat(CHAR_DATA *ch,char *argument){
	char buf[MSL],arg[MIL];
	ROOM_INDEX_DATA *location;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	int door;

	one_argument(argument,arg);
	location = (!arg[0]) ? ch->in_room : find_location(ch,arg);
	if (!location){
		ch->send("No such location.\n\r");
		return;
	}

	if (!is_room_owner(ch,location) && ch->in_room != location && room_is_private(location) && !IS_TRUSTED(ch,ADMIN)){
		ch->send("That room is private right now.\n\r");
		return;
	}

	printf_to_char(ch,"Name: '%s'\n\rArea: '%s'\n\r",location->name,location->area->name);
	printf_to_char(ch,"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",location->vnum,location->sector_type,location->light,location->heal_rate,location->mana_rate);
	printf_to_char(ch,"Room flags: %d.\n\rDescription:\n\r%s",location->room_flags,location->description);

	if (location->extra_descr){
		EXTRA_DESCR_DATA *ed;

		ch->send("Extra description keywords: '");
		for (ed = location->extra_descr;ed;ed = ed->next){
			ch->send(ed->keyword);
			if (ed->next)
				ch->send(" ");
		}
		ch->send("'.\n\r");
	}

	ch->send("Characters:");
	for (rch = location->people;rch;rch = rch->next_in_room){
		if (can_see(ch,rch)){
			ch->send(" ");
			one_argument(rch->name,buf);
			ch->send(buf);
		}
	}

	ch->send(".\n\rObjects:   ");
	for (obj = location->contents;obj;obj = obj->next_content){
		ch->send(" ");
		one_argument(obj->name,buf);
		ch->send(buf);
	}
	ch->send(".\n\r");

	for (door = 0;door <= 5;door++){
		EXIT_DATA *pexit;

		if ((pexit = location->exit[door])){
			printf_to_char(ch,"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",
				door,
				pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum,
	    		pexit->key,
	    		pexit->exit_info,
	    		pexit->keyword,
	    		pexit->description[0] != '\0' ? pexit->description : "(none).\n\r" );
		}
	}
}

void do_ostat(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int i;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Stat what?\n\r");
		return;
	}

	if (!(obj = get_obj_world(ch,argument))){
		ch->send("Nothing like that in hell, earth, or heaven.\n\r");
		return;
	}

	printf_to_char(ch,"Name(s): %s\n\r",obj->name);
	printf_to_char(ch,"Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",item_name(obj->item_type),obj->pIndexData->reset_num);
	printf_to_char(ch,"Short description: %s\n\rLong description: %s\n\r",obj->short_descr,obj->description);
	if (obj->owner && obj->owner != NULL)
		printf_to_char(ch,"Owner: %s\n\r",obj->owner);
	printf_to_char(ch,"Wear bits: %s\n\rExtra bits: %s\n\r",wear_bit_name(obj->wear_flags),extra_bit_name(obj->extra_flags));
	printf_to_char(ch,"Exclude bits: %s\n\r",exclude_bit_name(obj->exclude));
	printf_to_char(ch,"ArmorType bits: %s(%d)\n\r",armortype_bit_name(obj->armortype_flags),obj->armortype_flags);
	printf_to_char(ch,"Material: '%s'\n\r",obj->material);
	printf_to_char(ch,"Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",1,get_obj_number(obj),obj->weight,get_obj_weight(obj),get_true_weight(obj,true));
	printf_to_char(ch,"Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",obj->level,obj->cost,obj->condition,obj->timer);
	printf_to_char(ch,"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
		obj->in_room == NULL ? 0 : obj->in_room->vnum,
		obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
		obj->carried_by == NULL ? "(none)" : PERS(obj->carried_by,ch),
		obj->wear_loc);
	printf_to_char(ch,"Values: %d %d %d %d %d\n\r",obj->value[0],obj->value[1],obj->value[2],obj->value[3],obj->value[4]);
	switch (obj->item_type){
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			printf_to_char(ch,"Level %d spells of:",obj->value[0]);
			for (i = 1; i < 4; i++)
				if (obj->value[i] >= 0 && obj->value[i] < MAX_SKILL)
	    			printf_to_char(ch," '%s'",skill_table[obj->value[i]].name);
			ch->send(".\n\r");
			break;
		case ITEM_QUIVER:
			printf_to_char(ch,"{WIt holds {R%d %d{Wd{R%d {Warrows.{x\n\r",obj->value[0],obj->value[1],obj->value[2]);
			break;
		case ITEM_ARROW:
			printf_to_char(ch,"{WThis arrow will do {R%d{Wd{R%d {Wdamage for an average of {R%d{W.{x\n\r",obj->value[1],obj->value[2],(1 + obj->value[2]) * obj->value[1] / 2);
			break;
		case ITEM_WAND:
		case ITEM_STAFF:
			printf_to_char(ch,"Has %d(%d) charges of level %d",obj->value[1],obj->value[2],obj->value[0]);
			if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
				printf_to_char(ch," '%s'",skill_table[obj->value[3]].name);
			ch->send(".\n\r");
			break;
		case ITEM_DRINK_CON:
			printf_to_char(ch,"It holds %s-colored %s.\n\r",liq_table[obj->value[2]].liq_color,liq_table[obj->value[2]].liq_name);
			break;
		case ITEM_WEAPON:
 			printf_to_char(ch,"Weapon type is %s",flag_string(weapon_class,obj->value[0]));
			if (obj->pIndexData->new_format)
				printf_to_char(ch,"Damage is %dd%d (average %d)\n\r",obj->value[1],obj->value[2],(1 + obj->value[2]) * obj->value[1] / 2);
			else
				printf_to_char(ch,"Damage is %d to %d (average %d)\n\r",obj->value[1], obj->value[2],(obj->value[1] + obj->value[2]) / 2);
			printf_to_char(ch,"Damage noun is %s.\n\r",(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ? attack_table[obj->value[3]].noun : "undefined");
			printf_to_char(ch,"Weapons flags: %s\n\r",weapon_bits_name(obj->wflags));
			break;
		case ITEM_TRAP:
		case ITEM_TRAPPARTS:
 			printf_to_char(ch,"Trap type is %s.\n\rDifficulty rating: %d.\n\rSpecial type: %s\n\r",flag_string(trap_class, obj->value[0]),obj->value[1],flag_string(trap_type,obj->value[2]));
			break;
		case ITEM_CALTROPS:
			printf_to_char(ch,"The grade is %d, uses are %d\n\r",obj->value[0],obj->value[1]);
			break;
		case ITEM_ARMOR:
		case ITEM_SHIELD:
			printf_to_char(ch,"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",obj->value[0],obj->value[1],obj->value[2],obj->value[3]);
			break;
		case ITEM_CONTAINER:
			printf_to_char(ch,"Capacity: %d/%d  Maximum weight: %d#  flags: %s\n\r",
				obj->cont_count,
				obj->value[0],
				obj->value[3],
				cont_bit_name(obj->value[1]));

			if (obj->value[4] != 100)
				printf_to_char(ch,"Weight multiplier: %d%%\n\r",obj->value[4]);
			break;
	}

	if (obj->extra_descr || obj->pIndexData->extra_descr){
		EXTRA_DESCR_DATA *ed;

		ch->send("Extra description keywords: '");

		for (ed = obj->extra_descr;ed;ed = ed->next){
			ch->send(ed->keyword);
			if (ed->next)
	    		ch->send(" ");
		}

		for (ed = obj->pIndexData->extra_descr;ed;ed = ed->next){
			ch->send(ed->keyword);
			if (ed->next != NULL)
				ch->send(" ");
		}

		ch->send("'\n\r");
	}

	for (paf = obj->affected;paf;paf = paf->next){
		printf_to_char(ch,"Affects %s by %d, level %d",affect_loc_name(paf->location),paf->modifier,paf->level);
		if (paf->duration > -1)
			printf_to_char(ch,", %d hours.\n\r",paf->duration);
		else
			ch->send(".\n\r");
		if (paf->bitvector){
			switch(paf->where){
				case TO_AFFECTS:
					printf_to_char(ch,"Adds %s BUG! affect.\n",affect_bit_name(paf->bitvector));	break;
				case TO_WEAPON:
					printf_to_char(ch,"Adds %s weapon flags.\n",weapon_bit_name(paf->bitvector));	break;
				case TO_OBJECT:
					printf_to_char(ch,"Adds %s object flag.\n",extra_bit_name(paf->bitvector));		break;
				case TO_IMMUNE:
					printf_to_char(ch,"Adds immunity to %s.\n",res_bit_name(paf->bitvector));		break;
				case TO_RESIST:
					printf_to_char(ch,"Adds resistance to %s.\n\r",res_bit_name(paf->bitvector));	break;
				case TO_VULN:
					printf_to_char(ch,"Adds vulnerability to %s.\n\r",res_bit_name(paf->bitvector));break;
				default:
					printf_to_char(ch,"Unknown bit %d: %d\n\r",paf->where,paf->bitvector);			break;
			}
		}
	}

	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected;paf;paf = paf->next){
			printf_to_char(ch,"Affects %s by %d, level %d.\n\r",affect_loc_name(paf->location),paf->modifier,paf->level);
			if (paf->bitvector){
				switch(paf->where){
					case TO_AFFECTS:
						printf_to_char(ch,"Adds %s affect.\n", affect_bit_name(paf->bitvector));		break;
					case TO_OBJECT:
						printf_to_char(ch,"Adds %s object flag.\n",extra_bit_name(paf->bitvector));		break;
					case TO_IMMUNE:
						printf_to_char(ch,"Adds immunity to %s.\n",res_bit_name(paf->bitvector));		break;
					case TO_RESIST:
						printf_to_char(ch,"Adds resistance to %s.\n\r",res_bit_name(paf->bitvector));	break;
					case TO_VULN:
						printf_to_char(ch,"Adds vulnerability to %s.\n\r",res_bit_name(paf->bitvector));break;
					default:
						printf_to_char(ch,"Unknown bit %d: %d\n\r",paf->where,paf->bitvector);			break;
				}
			}
		}
}

void do_mstat(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Stat whom?\n\r");
		return;
	}
	if (!(victim = get_char_world(ch,argument))){
		ch->send("They aren't here.\n\r");
		return;
	}
	if (IS_NPC(victim))
		printf_to_char(ch,"Name: {G%s{x ID: {G%d{x MID: {G%d{x\n\r",victim->name,victim->id,victim->mid);
	else
		printf_to_char(ch,"Name: {G%s{x LastName {G%s{x ID: {G%d{x MID: {G%d{x\n\r",victim->name,victim->pcdata->lname,victim->id,victim->mid);

	printf_to_char(ch,"{xClass: {C%s {xRace: {G%s{x Group: {C%d{x Sex: {C%s{x\n\r",
		classes[victim->pclass].name,
		race_table[victim->race].name,
		IS_NPC(victim) ? victim->group : 0,
		sex_table[victim->sex].name);
	ch->send("-----------------------------------------------------\n\r");
	if (IS_NPC(victim))
		printf_to_char(ch,"Count: %d  Killed: %d\n\r",victim->pIndexData->count,victim->pIndexData->killed);
	//statlist was here
	printf_to_char(ch,"Hp: {c%d{x/{C%d  {xMana: {g%d{x/{G%d  {xAntimana: {b%d{x/{B%d {xMove: {m%d{x/{M%d{x\n\r",
		victim->hit,
		victim->max_hit,
		victim->gettruemana(),
		victim->gettruemaxmana(),
		victim->gettrueantimana(),
		victim->gettruemaxantimana(),
		victim->move,
		victim->max_move);
	printf_to_char(ch,"Lv: {R%d {xAlign: %d  Gold: {y%d  {xSilver: {w%d  {xExp: {g%d{x\n\r",
		victim->level,
		victim->alignment,
		victim->gold,
		victim->silver,
		victim->exp);
	printf_to_char(ch,"Armor: pierce: {Y%d  {xbash: {Y%d  {xslash: {Y%d  {xmagic: {Y%d{x\n\r",
		GET_AC(victim,AC_PIERCE),
		GET_AC(victim,AC_BASH),
		GET_AC(victim,AC_SLASH),
		GET_AC(victim,AC_EXOTIC));
	printf_to_char(ch,"HR: {R%d{x  DR: {R%d{x  Saves: {R%d{x  Size: {R%s{x  Position: {R%s{x  Wimpy: {R%d{x\n\r",
		GET_HITROLL(victim),
		GET_DAMROLL(victim),
		victim->saving_throw,
		size_table[victim->size].name,
		position_table[victim->position].name,
		victim->wimpy);
	printf_to_char(ch,"Death Timer: %d\n\r",victim->death_timer);
	printf_to_char(ch,"Counters %dp %dd  PrimarySpeed: %d  SecondarySpeed: %d\n\r",victim->pcounter,victim->dcounter,get_attackspeed(victim,false),get_attackspeed(victim,true));
	if (IS_NPC(victim) && victim->pIndexData->new_format)
		printf_to_char(ch,"Damage: %dd%d  Message:  %s\n\r",victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],attack_table[victim->dam_type].noun);
	printf_to_char(ch,"SavesSpell: %d Fighting: %s\n\r",victim->saving_spell_throw,victim->fighting ? victim->fighting->name : "(none)");
	if (!IS_NPC(victim))
		printf_to_char(ch,"Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\n\r",
			victim->pcdata->condition[COND_THIRST]*3,
			victim->pcdata->condition[COND_HUNGER]*3,
			victim->pcdata->condition[COND_FULL]*3,
			victim->pcdata->condition[COND_DRUNK]);
	printf_to_char(ch,"Carry number: %d  Carry weight: %d\n\r",victim->carry_number,get_carry_weight(victim) / 10);
	ch->send("-----------------------------------------------------\n\r");
	printf_to_char(ch,"Vnum: {C%d{x  Format: {C%s{x Room: {C%d{x\n\r",
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
		victim->in_room == NULL ? 0 : victim->in_room->vnum);
	if (!IS_NPC(victim))
		printf_to_char(ch,"Age: %d  Played: %d  Last Level: %d  Timer: %d  Rot: %d\n\r",
			get_age(victim), 
			(int)(victim->played + current_time - victim->logon) / 3600, 
			victim->pcdata->last_level, 
			victim->timer,victim->rottimer);
	printf_to_char(ch,"Act:         {R%s{x\n\r",act_bits_name(victim->act));
	printf_to_char(ch,"Comm:        {R%s{x\n\r",comm_bits_name(victim->comm));
	printf_to_char(ch,"Offensive:   {R%s{x\n\r",off_bits_name(victim->off_bits));
	printf_to_char(ch,"Defensive:   {R%s{x\n\r",def_bits_name(victim->def_bits));
	printf_to_char(ch,"Resistance:  {R%s{x\n\r", res_bits_name(victim->res));
	printf_to_char(ch,"Form:	     {R%s{x\n\r", form_bits_name(victim->form));
	printf_to_char(ch,"Parts:       {R%s{x\n\r",part_bits_name(victim->parts));
	printf_to_char(ch,"Affected by: {R%s{x\n\r", affect_bits_name(victim->affected_by));

	printf_to_char(ch,"Master: %s  Leader: %s\n\r",
		victim->master ? victim->master->name : "(none)",
		get_leader(victim)->name);
	if(!IS_NPC(ch))
	printf_to_char(ch,"GLeader %s  Pet: %s  Horse: %s  Guardby: %s Guarding: %s\n\r",
		"(none)",
		victim->pet ? victim->pet->name : "(none)",
		victim->mount ? victim->mount->name : "(none)",
		victim->guardby ? victim->guardby->name : "(none)",
		victim->guarding ? victim->guarding->name : "(none)");

	//printf_to_char(ch,"Mounted: %d. Riding: %d.\n\r",ch->mounted,ch->riding);
	if (!IS_NPC(victim))
		printf_to_char(ch,"Security: %d.\n\r",victim->pcdata->security);
	printf_to_char(ch,"Short description: %s\n\rLong  description: %s",
		IS_NPC(victim) ? victim->short_descr : "(none)",
		victim->long_descr[0] ? victim->long_descr : "(none)\n\r" );
	if (IS_NPC(victim) && victim->spec_fun != 0)
		printf_to_char(ch,"Mobile has special procedure %s.\n\r",spec_name(victim->spec_fun));
	for (paf = victim->affected;paf;paf = paf->next)
		printf_to_char(ch,"Spell: '%s' mods %s by %d for {w%d {xticks, bits: %s, level {w%d{x.\n\r",
			skill_table[(int) paf->type].name,
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration,
			paf->where == TO_AFFECTS ? affect_bit_name(paf->bitvector) : res_bit_name(paf->bitvector),
			paf->level);

	if(!IS_NPC(victim)){
		printf_to_char(ch,"Worn: %d %d %d %d %d\n\r",
			victim->worn_armor[0],
			victim->worn_armor[1],
			victim->worn_armor[2],
			victim->worn_armor[3],
			victim->worn_armor[4]);
	}
	if(IS_IMMORTAL(victim))
		ch->send("Imm ");
	else
		ch->send("not Imm ");
	printf_to_char(ch,"%d %d ",ch->morph,get_trust(victim));
	if(IS_HL(victim))
		ch->send("HL ");
	else
		ch->send("Not HL ");
	if(guilds[victim->guild].type == GTYPE_POPULACE)
		ch->send("Populace.\n\r");
	else
		ch->send("Not populace.\n\r");
}

void do_vnum(CHAR_DATA *ch,char *argument){
	char arg[MIL],*string;

	string = one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Syntax:\n\r  vnum obj <name>\n\r  vnum mob <name>\n\r  vnum skill <skill or spell>\n\r");
		return;
	}

	if (!str_cmp(arg,"obj")){
		do_function(ch,&do_ofind,string);
 		return;
	}

	if (!str_cmp(arg,"mob") || !str_cmp(arg,"char")){ 
		do_function(ch, &do_mfind, string);
		return;
	}

	if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell")){
		do_function(ch,&do_slookup,string);
		return;
	}
	/* do both */
	do_function(ch,&do_mfind,argument);
	do_function(ch,&do_ofind,argument);
}

void do_mfind(CHAR_DATA *ch,char *argument){
	extern int top_mob_index;
	char arg[MIL];
	MOB_INDEX_DATA *pMobIndex;
	int vnum,nMatch;
	bool fAll,found;

	one_argument(argument,arg);
	if (!arg[0]){
		ch->send("Find whom?\n\r");
		return;
	}

	fAll	= false;
	found	= false;
	nMatch	= 0;

	for (vnum = 0;nMatch < top_mob_index; vnum++ )
		if ((pMobIndex = get_mob_index(vnum)) != NULL){
			nMatch++;
			if (fAll || is_name(argument,pMobIndex->player_name)){
				found = true;
				printf_to_char(ch,"[%5d] %s\n\r",pMobIndex->vnum,pMobIndex->short_descr);
			}
		}

	if (!found)
		ch->send("No mobiles by that name.\n\r");
}

void do_ofind(CHAR_DATA *ch,char *argument){
	extern int top_obj_index;
	char arg[MIL];
	OBJ_INDEX_DATA *pObjIndex;
	int vnum,nMatch;
	bool fAll,found;

	one_argument(argument,arg);
	if (!arg[0]){
		send_to_char("Find what?\n\r",ch);
		return;
	}

	fAll	= false;//!str_cmp( arg, "all" );
	found	= false;
	nMatch	= 0;

	for (vnum = 0;nMatch < top_obj_index;vnum++)
		if ((pObjIndex = get_obj_index(vnum))){
			nMatch++;
			if (fAll || is_name(argument,pObjIndex->name)){
				found = true;
				printf_to_char(ch,"[%5d] %s\n\r",pObjIndex->vnum,pObjIndex->short_descr);
			}
		}

	if (!found)
		ch->send("No objects by that name.\n\r");
	return;
}

void do_owhere(CHAR_DATA *ch,char *argument){
	char buf[MIL];
	BUFFER *buffer;
	OBJ_DATA *obj,*in_obj;
	bool found;
	int number = 0,max_found;

	found = false;
	number = 0;
	max_found = 200;

	buffer = new_buf();

	if (!argument[0]){
		ch->send("Find what?\n\r");
		return;
	}

	for (obj = object_list;obj;obj = obj->next){
		if (!can_see_obj(ch,obj) || !is_name(argument,obj->name) || ch->level < obj->level || (obj->carried_by && get_trust(obj->carried_by) > get_trust(ch)))
			continue;

		found = true;
		number++;

		for (in_obj = obj;in_obj->in_obj;in_obj = in_obj->in_obj)
			;
		if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by) && in_obj->carried_by->in_room != NULL)
			sprintf(buf,"%3d) %s is carried by %s [Room %d]\n\r",number,obj->short_descr,PERS(in_obj->carried_by,ch),in_obj->carried_by->in_room->vnum);
		else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
			sprintf(buf,"%3d) %s is in %s [Room %d]\n\r",number,obj->short_descr,in_obj->in_room->name,in_obj->in_room->vnum);
		else
			sprintf(buf,"%3d) %s is somewhere\n\r",number,obj->short_descr);

		buf[0] = UPPER(buf[0]);
		add_buf(buffer,buf);
		if (number >= max_found)
			break;
	}

	if (!found)
		ch->send("Nothing like that in heaven or earth.\n\r");
	else
		page_to_char(buf_string(buffer),ch);

	free_buf(buffer);
}

void do_mwhere(CHAR_DATA *ch,char *argument){
	char buf[MSL];
	BUFFER *buffer;
	CHAR_DATA *victim;
	bool found;
	int count = 0;

	if (!argument[0]){
		DESCRIPTOR_DATA *d;

		buffer = new_buf();
		for (d = descriptor_list; d != NULL; d = d->next)
			if (d->character != NULL && d->connected == CON_PLAYING && d->character->in_room && can_see(ch,d->character) && can_see_room(ch,d->character->in_room)){
				victim = d->character;
				count++;
				if (d->original)
					sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",count,d->original->name,victim->short_descr,victim->in_room->name,victim->in_room->vnum);
				else
					sprintf(buf,"%3d) %s is in %s [%d]\n\r",count, victim->name,victim->in_room->name,victim->in_room->vnum);
				add_buf(buffer,buf);
			}

		page_to_char(buf_string(buffer),ch);
		free_buf(buffer);
		return;
	}

	found = false;
	buffer = new_buf();
	for (victim = char_list;victim;victim = victim->next)
		if (victim->in_room && is_name(argument,victim->name)){
			found = true;
			count++;
			sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
				IS_NPC(victim) ? victim->pIndexData->vnum : 0,
				IS_NPC(victim) ? victim->short_descr : victim->name,
				victim->in_room->vnum,
				victim->in_room->name);
			add_buf(buffer,buf);
		}

	if (!found)
		act("You didn't find any $T.",ch,NULL,argument,TO_CHAR);
	else
		page_to_char(buf_string(buffer),ch);

	free_buf(buffer);
}

void do_snoop(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Snoop whom?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (!victim->desc){
		ch->send("No descriptor to snoop.\n\r");
		return;
	}

	if (victim == ch){
		ch->send("Cancelling all snoops.\n\r");
		wiznet("$N stops being such a snoop.",ch,NULL,WZ_SNOOPS,WZ_SECURE,get_trust(ch));
		for ( d = descriptor_list; d != NULL; d = d->next )
			if (d->snoop_by == ch->desc)
				d->snoop_by = NULL;
		return;
	}

	if (victim->desc->snoop_by){
		ch->send("Being snooped already.\n\r");
		return;
	}

	if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room && room_is_private(victim->in_room) && !IS_TRUSTED(ch,ADMIN)){
		ch->send("That character is in a private room.\n\r");
		return;
	}

	if (get_trust(victim) >= get_trust(ch) || victim->iscomm(CM_SNOOP_PROOF)){
		ch->send("You failed.\n\r");
		return;
	}

	if (ch->desc != NULL)
		for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
			if (d->character == victim || d->original == victim){
				ch->send("No snoop loops.\n\r");
				return;
			}

	victim->desc->snoop_by = ch->desc;
	sprintf(buf,"$N starts snooping on %s",(IS_NPC(ch) ? victim->short_descr : victim->name));
	wiznet(buf,ch,NULL,WZ_SNOOPS,WZ_SECURE,get_trust(ch));
	ch->send("Ok.\n\r");
}

bool obj_check(CHAR_DATA *ch,OBJ_DATA *obj){
	if (IS_TRUSTED(ch,IMMORTAL)
	|| (IS_TRUSTED(ch,JBUILDER) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMI) && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,KING) && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,HLEADER) && obj->level ==  0 && obj->cost <= 100))
		return true;
	else
		return false;
}

void recursive_clone(CHAR_DATA *ch,OBJ_DATA *obj,OBJ_DATA *clone){
    OBJ_DATA *c_obj, *t_obj;

    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
		if (obj_check(ch,c_obj)){
			t_obj = create_object(c_obj->pIndexData,0);
			clone_object(c_obj,t_obj);
			obj_to_obj(t_obj,clone);
			recursive_clone(ch,c_obj,t_obj);
		}
}


void do_log(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Log whom?\n\r");
		return;
	}

	if (!str_cmp(arg,"all")){
		if (fLogAll){
			fLogAll = false;
			ch->send("Log ALL off.\n\r");
		}
		else{
			fLogAll = true;
			ch->send("Log ALL on.\n\r");
		}
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

	if (!victim->setplr(PL_LOG)){
		victim->remplr(PL_LOG);
		ch->send("LOG removed.\n\r");
	}
	else
		ch->send("LOG set.\n\r");
}

void do_debugset(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MSL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Flag whom as a debugger?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}


	if (get_trust(victim) > get_trust(ch)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!victim->setcomm(CM_DEBUG)){
		victim->remcomm(CM_DEBUG);
		victim->send("You've been kicked off the LAPD!\n\r");
		ch->send("Booted them from the LAPD.\n\r");
		sprintf(buf,"$N kicks %s off the LAPD.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
	else{
		victim->send("You're now a member of the LAPD!\n\r");
		ch->send("Debugger person set.\n\r");
		sprintf(buf,"$N accepts %s into the LAPD.",victim->name);
		wiznet(buf,ch,NULL,WZ_PENALTIES,WZ_SECURE,0);
	}
}

void do_slookup(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	int sn;

	one_argument(argument,arg);
	if (!arg[0]){
		ch->send("Lookup which skill or spell?\n\r");
		return;
	}

	if (!str_cmp(arg,"all")){
		for (sn = 0;sn < MAX_SKILL; sn++){
			if (skill_table[sn].name == NULL)
				break;
			printf_to_char(ch,"Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",sn,skill_table[sn].slot,skill_table[sn].name);
		}
	}
	else{
		if ((sn = skill_lookup(arg)) < 0){
			ch->send("No such skill or spell.\n\r");
			return;
		}
		printf_to_char(ch,"Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",sn,skill_table[sn].slot,skill_table[sn].name);
	}
}

void mod_attribute(CHAR_DATA *ch,CHAR_DATA *victim,int slot, char *arg, char *thetype,const struct weight_type *thetable){
	int i;
	bool found = false;
	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}
	if (!str_prefix(arg,"list")){
		for(i=1; thetable[i].name != NULL; i++)
			printf_to_char(ch," %s",thetable[i].name);
		ch->send("\n\r");
		return;
	}
	for(i=1; thetable[i].name; i++)
		if (!str_prefix(arg,thetable[i].name)){
			victim->looks[slot] = thetable[i].flag;
			found = true;
		}
	if (!found)
		printf_to_char(ch,"%s not found.\n\r",capitalize(thetype));
	else
		printf_to_char(ch,"Modifying %s's %s to %s\n\r",victim->name,thetype,thetable[victim->looks[slot]].name);
}

char *gen_scramble(CHAR_DATA *ch){
	char buf[35];
	int i = ch->name[0];
	sprintf(buf,"%d%d-%d%d-%d%d",i % 3 + 2,i/10,   i % 2 + 1,i-60,   i % 2 + 1,i - 50);
	return buf;
}

void do_sockets(CHAR_DATA *ch,char *argument){
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;
	char buf[MSL],buf2[MSL],*st,idle[10];
	int count = 0;

	if (!argument[0]){
		buf[0]      = '\0';
		buf2[0]     = '\0';
		strcat(buf2,"\n\r[Num Connected_State] Player Name Host\n\r");
		strcat(buf2,"{r--------------------------------------------------------------------------\n\r");
		for (d = descriptor_list;d;d = d->next){
			if (d->character && can_see(ch,d->character)){
				switch(d->connected){
					case CON_PLAYING:					st = "    {GPLAYING    "; break;
					case CON_GET_NAME:					st = "   {YGet Name    "; break;
					case CON_GET_OLD_PASSWORD:			st = "{YGet Old Passwd "; break;
					case CON_CONFIRM_NEW_NAME:			st = " {YConfirm Name  "; break;
					case CON_GET_NEW_PASSWORD:			st = "{YGet New Passwd "; break;
					case CON_CONFIRM_NEW_PASSWORD:		st = "{YConfirm Passwd "; break;
					case CON_GET_NEW_RACE:				st = "  {YGet New Race "; break;
					case CON_GET_NEW_SEX:				st = "  {YGet New Sex  "; break;
					case CON_GET_NEW_CLASS:				st = " {YGet New Class "; break;
					case CON_GET_ALIGNMENT:   			st = " {YGet New Align "; break;
					case CON_DEFAULT_CHOICE:			st = " {YChoosing Cust "; break;
					case CON_PICK_SKILLS:				st = " {YPicking Skills"; break;
					case CON_CONFIRM_SKILLS:			st = " {YConfirm Skills"; break;
					case CON_READ_IMOTD:				st = " {gReading IMOTD "; break;
					case CON_READ_MOTD:					st = "  {gReading MOTD "; break;
					case CON_BREAK_CONNECT:				st = "   {dLINKDEAD    "; break;
					case CON_COPYOVER_RECOVER:			st = "	  {gCopyover   "; break;
					case CON_GET_HAIR:					st = "    {YGet Hair   "; break;
					case CON_GET_WEIGHT:				st = "   {YGet Weight  "; break;
					case CON_GET_HEIGHT:				st = "   {YGet Weight  "; break;
					case CON_GET_EYE:					st = "    {YGet Eye    "; break;
					case CON_GET_SCHOOL:				st = "  {YGet School   "; break;
					case CON_GET_HOMETOWN:				st = "  {YGet Hometown "; break;
					case CON_NOTE_TO:					st = "    {rNote - To  "; break;
					case CON_NOTE_SUBJECT:				st = "  {rNote Subject "; break;
					case CON_NOTE_EXPIRE:				st = " {rNote - Expire "; break;
					case CON_NOTE_TEXT:					st = "  {rNote - Text  "; break;
					case CON_NOTE_FINISH:				st = " {rNote - Finish "; break;
					case CON_GET_NEW_GOD:				st = " {YGod Selection "; break;
					case CON_GET_LNAME:					st = "   {YGet Lname   "; break;
					case CON_CONFIRM_LNAME:				st = " {YConfirm LName "; break;
					case CON_ANSI:						st = "   {YGet ANSI    "; break;
					case CON_GET_HAIRTYPE:				st = " {YGet Hairtype  "; break;
					default:				 			st = "   {g!UNKNOWN!   "; break;
			   }
			   count++;

			   vch = d->original ? d->original : d->character;
			   //strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );

			   if ( vch->timer > 0 )
				  sprintf(idle,"%-4d",vch->timer);
			   else
				  sprintf(idle,"    ");

			   //sprintf( buf, "[%3d %s %7s %2s] %-12s %-32.32s\n\r",
				sprintf(buf,"{y[{w%3d %s{y]{C%-12s {Y%s{d%-35.35s{x\n\r",
					d->descriptor,
					st,
					(d->original) ? d->original->name : (d->character)  ? d->character->name : "(None!)",
					idle,
					d->character->level <= ch->level ? d->character->isplr(PL_SCRAMBLE) ? gen_scramble(d->character) : d->host : "NULL");
			   strcat(buf2,buf);
			}
		}

		sprintf(buf,"\n\r%d user%s\n\r",count,count == 1 ? "" : "s");
		strcat(buf2,buf);
		ch->send(buf2);
	}
}

void global_message(int minL, int maxL, const char *txt,int nochan){
	char buf[MIL];

	sprintf(buf,"{C-{x%s",txt);
	DESCRIPTOR_DATA *d;
	for (d = descriptor_list;d;d = d->next)
		if(d->character
		&& d->connected == CON_PLAYING
		&& d->character->level >= minL
		&& d->character->level <= maxL){
			if((nochan == CM_LIFELINE && !d->character->iscomm(CM_LIFELINE)) || (nochan == CM_NONOTES && d->character->iscomm(CM_NONOTES)))
				continue;
			d->character->send(buf);
		}
}

void do_specialthing(CHAR_DATA *ch,char *argument){
	int skill,save,cols = 0;

	printf_to_char(ch,"Base ASP: {G%d{x ",ch->perm_stat[STAT_AGI]);
	printf_to_char(ch,"Mod ASP: {G%d{x ",ch->mod_stat[STAT_AGI]);
	printf_to_char(ch,"Curr ASP: {G%d{x\n\r",get_curr_stat(ch,STAT_AGI));
	printf_to_char(ch,"CalcTimerP: {G%d{x quarters of a second\n\r",get_attackspeed(ch,false));
	printf_to_char(ch,"CalcTimerD: {G%d{x quarters of a second\n\r\n\r",get_attackspeed(ch,true));

	printf_to_char(ch,"Base LCK: {G%d{x ",ch->perm_stat[STAT_LCK]);
	printf_to_char(ch,"Mod LCK: {G%d{x ",ch->mod_stat[STAT_LCK]);
	printf_to_char(ch,"Curr LCK: {G%d{x\n\r",get_curr_stat(ch,STAT_LCK));
	printf_to_char(ch,"Save_spell vs 100 neutral:%d\n\r",get_save_spell(100,ch,DAM_OTHER));
	printf_to_char(ch,"Save_skill vs 100 neutral:%d\n\r",get_save_skill(100,ch,DAM_OTHER));

	save = 50 + (ch->level - 100) + ch->saving_spell_throw * 2;
	if (!IS_NPC(ch) && classes[ch->pclass].fMana > 0)//nashneedstofixthis
		save = ((4 + classes[ch->pclass].fMana) * save / 10);
	save = URANGE(5,save,95);
	if (!IS_NPC(ch))
		save += get_curr_stat(ch,STAT_LCK) * .75;

	printf_to_char(ch,"CalcSpellSaves (lvl100 spell): {G%d{x\n\r",save,saves_skill(100,ch,DAM_OTHER));

	save = 30 + (ch->level - 100) + ch->saving_throw * 2;
	if (ch->isaff(AF_BERSERK))
		save += UMAX(1,ch->level/10);
	save = URANGE(5,save,95);
	save = (save * get_curr_stat(ch,STAT_LCK)) / STAT_MAX;

	printf_to_char(ch,"CalcSkillSaves (lvl100 skill): {G%d, %d{x\n\r\n\r",save,saves_skill(100,ch,DAM_OTHER));

	printf_to_char(ch,"Base INT: {G%d{x ",ch->perm_stat[STAT_INT]);
	printf_to_char(ch,"Mod INT: {G%d{x ",ch->mod_stat[STAT_INT]);
	printf_to_char(ch,"Curr INT: {G%d{x\n\r",get_curr_stat(ch,STAT_INT));

	skill = 100;
	if (IS_DRUNK(ch)){
		if(number_percent() <= get_skill(ch,gsn_drunkfighting) * .5){
			skill *= 1.1;
			check_improve(ch,gsn_drunkfighting,true,5);
		}
		else{
			skill *= .8;
			check_improve(ch,gsn_drunkfighting,false,2);
		}
	}
	skill = (skill * .8) + ((skill * .2) * get_curr_stat(ch,STAT_INT) / STAT_MAX);

    printf_to_char(ch,"Example Skill Mod: {G%d{x%%\n\r\n\r",URANGE(0,skill,100));
    printf_to_char(ch,"Chargetime: {G%d{x\n\r\n\r",ch->chargetime);
	printf_to_char(ch,"Combolook: %s\n\r\n\r",ch->combo);

	skill = 50;
	skill = calcReflex(ch,ch,skill);
	printf_to_char(ch,"50%% skill reflex calc: {G%d{x\n\r\n\r",skill);

	for(skill = 0;stat_flags[skill].name;skill++){
		printf_to_char(ch,"%12s: ({c%d{x)",stat_flags[skill].name,ch->perm_stat[skill]);
		if (++cols % 4 == 0)
			ch->send("\n\r");
		else
			ch->send("  ");
	}
	ch->send("\n\r");

/*	bool found,found2;
	for(int sn=1;skill_table[sn].name;sn++){
		found=false;
		for(int j=1;j<MAX_CLASS-1;j++){
			if(skill_table[sn].cost[j] > -1)
				found = true;
			if(skill_table[sn].cost[j] > 0)
				found2 = true;
		}
		if(!found)
			printf_to_char(ch,"%s\n\r",skill_table[sn].name);
		if(!found2)
			printf_to_char(ch,"*%s*",skill_table[sn].name);
	}*/
	//system("cd /home/nash/kof/src; make clean");
}

void do_classstat(CHAR_DATA *ch,char *argument){
	int i = -1,n,cols = -1;
	char buf[100];
	bool found = false,doop;

	if (!argument[0]){
		ch->send("Syntax: classstat <class>\n\rAvailable classes are:\n\r");
		for(int j=0;j<=3;j++){
			ch->printf("Tier: {R%d{x\n\r",j);
			for(i = 0;i < MAX_CLASS;i++){
				if(classes[i].ctier == j){
					printf_to_char(ch," -{%s%-12s{x",classes[i].active ? "G" : "d", classes[i].name);
					if(++cols % 3 == 0)
						ch->send("\n\r");
				}
			}
			if(cols % 3 != 0)
				ch->send("\n\r");
			cols = -1;
		}
		return;
	}
	i = class_lookup(argument);
	if (i < 0){
		ch->send("That is not a valid class.\n\r");
		return;
	}
	printf_to_char(ch,"Active:            [ {y%s{x ]\n\r",classes[i].active ? "Yes" : "No");
	printf_to_char(ch,"Class Name:        [ {c%s{x ]\n\r",classes[i].name);
	printf_to_char(ch,"Who Name:          [ {c%s{x ]\n\r",classes[i].who_name);
	printf_to_char(ch,"Tier:              [ {y%d{x ]\n\r",classes[i].ctier);
	printf_to_char(ch,"ManaI:             [ {y%d{x ]\n\r",classes[i].fMana);
	printf_to_char(ch,"SkillAdept:        [ {y%d{x%% ]\n\r",classes[i].skill_adept);
	printf_to_char(ch,"Thac00:            [ {y%d{x ]\n\r",classes[i].thac0_00);
	printf_to_char(ch,"Thac32:            [ {y%d{x ]\n\r",classes[i].thac0_32);
	printf_to_char(ch,"HP:                [ {y%d{x-{y%d{x ]\n\r",classes[i].hp_min,classes[i].hp_max);
	printf_to_char(ch,"MP:                [ {y%d{x-{y%d{x ]\n\r",classes[i].mp_min,classes[i].mp_max);
	printf_to_char(ch,"MV:                [ {y%d{x-{y%d{x ]\n\r",classes[i].mv_min,classes[i].mv_max);
	sprintf(buf,"%s",classes[i].msg_self);
	printf_to_char(ch,"Cast Msg Self:     [ %s ]\n\r",buf);
	sprintf(buf,"%s",classes[i].msg_other);
	printf_to_char(ch,"Cast Msg Othr:     [ %s ]\n\r",buf);

	ch->send(         "Available Races:   [{c");
	for(n = 0;pc_race_table[n].name;n++)
		if(pc_race_table[n].class_use[i])
			ch->printf(" %s",pc_race_table[n].name);
	ch->send(" {x]\n\r");

	ch->send(         "Available classes: [{c ");
	for(n = 0;n < MAX_CLASS;n++){
		if(classes[i].becomes[n]){
			found = true;
			printf_to_char(ch,"%s ",classes[n].name);
		}
	}
	if (!found)
		ch->send("None ");
	ch->send("{x]\n\r");

	ch->send("Stat Multipliers:  [ ");
	cols = 0;
	for(n = 0;n < MAX_STATS;n++){
		ch->printf("{c%s{x({y%2d{x) ",abbrev_stat_flags[n].name,classes[i].attribute[n]);
		if(++cols % 5 == 0){
			ch->send("]\n\r                   [ ");
		}
	}
	ch->send("        ]\n\r");

	ch->send("\n\rAvailable skills:\n\r");
	for(int l=0;l<4;l++){
		cols = 0;
		doop = false;
		for(n = 0;n<MAX_SKILL;n++){
			if(skill_table[n].skill_level[i] < LEVEL_IMMORTAL && skill_table[n].slot != SKILL_NATURAL){
				if((l == 0 && skill_table[n].skill_level[i] == 1)
				|| (l == 1 && skill_table[n].skill_level[i] == 10)
				|| (l == 2 && skill_table[n].skill_level[i] == 25)
				|| (l == 3 && skill_table[n].skill_level[i] == 50)){
					if(!doop){
						if(cols + 1 % 4 != 0)
							ch->send("\n\r");
						ch->printf("Tier %d:\n\r",l);
						doop = true;
					}
					else{
						ch->printf("[{d%18s{x]",skill_table[n].name);
						if(++cols % 4 == 0)
							ch->send("\n\r");
					}
				}
			}
		}
		if(cols %3 != 0)
			ch->send("\n\r");
	}
}

void do_guildstat(CHAR_DATA *ch,char *argument){
	int i = -1,n;
	char buf[100];
	bool found = false;
	if (!argument[0]){
		ch->send("Syntax: guildstat <guild>\n\rAvailable guilds are:\n\r");
		for(int i = 0;i < MAX_GUILD;i++)
			printf_to_char(ch," -{d%s{x\n\r",guilds[i].name);
		return;
	}
	i = guild_lookup(argument);
	if (i < 0){
		ch->send("That is not a valid guild.\n\r");
		return;
	}
	printf_to_char(ch,"(#%d)Guild Name: '%s%s{x'\n\rWho Title: '{g%s{x'\n\r",guilds[i].index,guilds[i].active ? "{G" : "{R",guilds[i].name,guilds[i].who_name);
	printf_to_char(ch,"Recall: {c%d{x\n\rRespawn: {c%d{x\n\rGuild Hall: {c%d{x'\n\r",guilds[i].recall,guilds[i].respawn,guilds[i].area);
	printf_to_char(ch,"Type: ({g%d{x)\n\r",guilds[i].type);
	printf_to_char(ch,"Hidden: ({g%s{x) Mask Guild: ({c%d{x)\n\r",guilds[i].hidden ? "true" : "false",guilds[i].hidden);
	printf_to_char(ch,"Lowest Rank: ({c%d{x)\n\r",guilds[i].rank_last);

	printf_to_char(ch,"\n\rRank                        | Promote | Demote | Recruit | Expel\n\r");
	for(n = 0;n <= guilds[i].rank_last;n++)
		printf_to_char(ch,"{g%-30s %5s    %5s     %5s     %5s{x\n\r",guilds[i].rank[n].name,
											guilds[i].rank[n].promote ? "true" : "false",
											guilds[i].rank[n].demote ? "true" : "false",
											guilds[i].rank[n].recruit ? "true" : "false",
											guilds[i].rank[n].expel ? "true" : "false");
	printf_to_char(ch,"\n\rRaces:\n\r");
	for(n = 0;n < MAX_RACE;n++){
		if(guilds[i].races[n]){
			found = true;
			printf_to_char(ch,"%s\n\r",pc_race_table[n].name);
		}
	}
	if (!found)
		ch->send("None");
	ch->send("\n\r");
}

void do_skillstat(CHAR_DATA *ch,char *argument){
	char buf[MIL],arg[MIL];
	int sn,cl,n;
	bool found;

	argument = one_argument(argument,arg);

	if(!arg[0]){
		ch->send("Syntax: plan <class> <skill>\n\r        plan <class>\n\r");
		return;
	}
	if(!argument[0]){
		do_classstat(ch,arg);
		return;
	}
	if((cl = class_lookup(arg)) < 0){
		if(!str_cmp(arg,"dead") || !str_cmp(arg,"unused")){
			ch->send("Searching for unused skills...\n\r");
			cl = 0;
			for(sn = 0;sn < MAX_SKILL;sn++){
				found = false;
				for(n=0;n<MAX_CLASS;n++){
					if(skill_table[sn].skill_level[n])
						found = true;
				}
				if(!found){
					ch->printf(" -%s\n\r",skill_table[sn].name);
					cl++;
				}
			}
			if(cl == 0)
				ch->send("No unused skills found.\n\r");
			return;
		}
		ch->send("That's not a class.\n\r");
		return;
	}
	if(classes[cl].ctier < 3){
		ch->send("You can only plan final classes. They receive all skills from their parent classes.\n\r");
		return;
	}
	if((sn = skill_lookup(argument)) < 0){
		ch->send("That's not a skill.\n\r");
		return;
	}
	printf_to_char(ch,"Name: %s\n\r",skill_table[sn].name);
	printf_to_char(ch,"Skill Level %d\n\r",skill_table[sn].skill_level[cl]);
	printf_to_char(ch,"cost %d\n\r",skill_table[sn].cost[cl]);
	printf_to_char(ch,"Max %d\n\r",skill_table[sn].max[cl]);
	found = false;
	ch->send("\n\r");
	for(n = 0; n < 10; n++)
		if(skill_table[sn].skl[cl].unlocks[n].sn > 0 || ch->iscomm(CM_DEBUG)){
			printf_to_char(ch,"Unlocks '%s' at level %d\n\r",skill_table[skill_table[sn].skl[cl].unlocks[n].sn].name,skill_table[sn].skl[cl].unlocks[n].level);
			found = true;
		}
	if(!found)
		ch->send("This skill does not unlock any skills.\n\r");
	ch->send("\n\r");
	found = false;
	for(n = 0; n < 10; n++)
		if(skill_table[sn].skl[cl].unlock_by[n].sn > 0 || ch->iscomm(CM_DEBUG)){
			printf_to_char(ch,"Requires '%s' at level %d\n\r",skill_table[skill_table[sn].skl[cl].unlock_by[n].sn].name,skill_table[sn].skl[cl].unlock_by[n].level);
			found = true;
		}
	if(!found)
		ch->send("This skill does not require any skills.\n\r");
	ch->send("\n\r");
	found = false;
	for(n = 0; n < 10; n++)
		if(skill_table[sn].skl[cl].locks[n].sn > 0 || ch->iscomm(CM_DEBUG)){
			printf_to_char(ch,"Locks '%s' at level %d\n\r",skill_table[skill_table[sn].skl[cl].locks[n].sn].name,skill_table[sn].skl[cl].locks[n].level);
			found = true;
		}
	if(!found)
		ch->send("This skill does not lock any skills.\n\r");
}

char * makedrunk (char *string, CHAR_DATA * ch){
	if(IS_NPC(ch) || !IS_DRUNK(ch))
		return string;
	/* This structure defines all changes for a character */
	struct struckdrunk drunk[]={
		{3, 10,{"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
		{8, 5,{"b", "b", "b", "B", "B", "vb"}},
		{3, 5,{"c", "c", "C", "cj", "sj", "zj"}},
		{5, 2,{"d", "d", "D"}},
		{3, 3,{"e", "e", "eh", "E"}},
		{4, 5,{"f", "f", "ff", "fff", "fFf", "F"}},
		{8, 2,{"g", "g", "G"}},
		{9, 6,{"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
		{7, 6,{"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
		{9, 5,{"j", "j", "jj", "Jj", "jJ", "J"}},
		{7, 2,{"k", "k", "K"}},
		{3, 2,{"l", "l", "L"}},
		{5, 8,{"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
		{6, 6,{"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
		{3, 6,{"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
		{3, 2,{"p", "p", "P"}},
		{5, 5,{"q", "q", "Q", "ku", "ququ", "kukeleku"}},
		{4, 2,{"r", "r", "R"}},
		{2, 5,{"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
		{5, 2,{"t", "t", "T"}},
		{3, 6,{"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
		{4, 2,{"v", "v", "V"}},
		{4, 2,{"w", "w", "W"}},
		{5, 6,{"x", "x", "X", "ks", "iks", "kz", "xz"}},
		{3, 2,{"y", "y", "Y"}},
		{2, 9,{"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
	};
	char buf[1024],temp;
	int pos = 0,drunklevel,randomnum;

	/* Check how drunk a person is... */
	if(IS_NPC(ch))
		drunklevel = 0;
	else
		drunklevel = ch->pcdata->condition[COND_DRUNK];

	if(drunklevel > 0){
		do{
			temp = toupper (*string);
			if((temp >= 'A') && (temp <= 'Z')){
				if(drunklevel > drunk[temp - 'A'].min_drunk_level && number_percent() <= ch->pcdata->condition[COND_DRUNK]){
					randomnum = number_range(0,drunk[temp - 'A'].number_of_rep);
					strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
					pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
				}
				else
					buf[pos++] = *string;
			}
			else{
				if((temp >= '0') && (temp <= '9')){
					temp = '0' + number_range (0, 9);
					buf[pos++] = temp;
				}
				else
					buf[pos++] = *string;
			}
		}
		while(*string++);
		buf[pos] = '\0';          /* Mark end of the string... */
		strcpy(string, buf);
		return(string);
	}
	return (string);
}

void do_immstat(CHAR_DATA *ch,char *argument){
	/*CHAR_DATA *victim;

	if (!argument[0]){
		ch->send("Stat who?\n\r");
		return;
	}
	if (!(victim = get_char_world(ch,argument))){
		ch->send("They aren't here.\n\r");
		return;
	}
	if (IS_NPC(victim))
		printf_to_char(ch,"Name: {G%s{x\n\r",victim->name);
	else
		printf_to_char(ch,"Name: {G%s{x LastName {G%s{x\n\r",victim->name,victim->pcdata->lname);

	printf_to_char(ch,"{xClass: {C%s {xRace: {G%s{x Group: {C%d{x Sex: {C%s{x\n\r",
		classes[victim->pclass].name,
		race_table[victim->race].name,
		IS_NPC(victim) ? victim->group : 0,
		sex_table[victim->sex].name);
	ch->send("-----------------------------------------------------\n\r");
	if (IS_NPC(victim))
		printf_to_char(ch,"Count: %d  Killed: %d\n\r",victim->pIndexData->count,victim->pIndexData->killed);
	//statlist was here
	printf_to_char(ch,"Hp: {c%d{x/{C%d  {xMana: {g%d{x/{G%d  {xAntimana: {b%d{x/{B%d {xMove: {m%d{x/{M%d{x\n\r",
		victim->hit,
		victim->max_hit,
		victim->gettruemana(),
		victim->gettruemaxmana(),
		victim->gettrueantimana(),
		victim->gettruemaxantimana(),
		victim->move,
		victim->max_move);
	printf_to_char(ch,"Lv: {R%d {xAlign: %d  Gold: {y%d  {xSilver: {w%d  {xExp: {g%d{x\n\r",
		victim->level,
		victim->alignment,
		victim->gold,
		victim->silver,
		victim->exp);
	printf_to_char(ch,"Armor: pierce: {Y%d  {xbash: {Y%d  {xslash: {Y%d  {xmagic: {Y%d{x\n\r",
		GET_AC(victim,AC_PIERCE),
		GET_AC(victim,AC_BASH),
		GET_AC(victim,AC_SLASH),
		GET_AC(victim,AC_EXOTIC));
	printf_to_char(ch,"HR: {R%d{x  DR: {R%d{x  Saves: {R%d{x  Size: {R%s{x  Position: {R%s{x  Wimpy: {R%d{x\n\r",
		GET_HITROLL(victim),
		GET_DAMROLL(victim),
		victim->saving_throw,
		size_table[victim->size].name,
		position_table[victim->position].name,
		victim->wimpy);
	printf_to_char(ch,"Death Timer: %d\n\r",victim->death_timer);
	printf_to_char(ch,"Counters %dp %dd  PrimarySpeed: %d  SecondarySpeed: %d\n\r",victim->pcounter,victim->dcounter,get_attackspeed(victim,false),get_attackspeed(victim,true));
	if (IS_NPC(victim) && victim->pIndexData->new_format)
		printf_to_char(ch,"Damage: %dd%d  Message:  %s\n\r",victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],attack_table[victim->dam_type].noun);
	printf_to_char(ch,"SavesSpell: %d Fighting: %s\n\r",victim->saving_spell_throw,victim->fighting ? victim->fighting->name : "(none)");
	if (!IS_NPC(victim))
		printf_to_char(ch,"Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\n\r",
			victim->pcdata->condition[COND_THIRST]*3,
			victim->pcdata->condition[COND_HUNGER]*3,
			victim->pcdata->condition[COND_FULL]*3,
			victim->pcdata->condition[COND_DRUNK]);
	printf_to_char(ch,"Carry number: %d  Carry weight: %d\n\r",victim->carry_number,get_carry_weight(victim) / 10);
	ch->send("-----------------------------------------------------\n\r");
	printf_to_char(ch,"Vnum: {C%d{x  Format: {C%s{x Room: {C%d{x\n\r",
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
		victim->in_room == NULL ? 0 : victim->in_room->vnum);
	if (!IS_NPC(victim))
		printf_to_char(ch,"Age: %d  Played: %d  Last Level: %d  Timer: %d  Rot: %d\n\r",
			get_age(victim), 
			(int)(victim->played + current_time - victim->logon) / 3600, 
			victim->pcdata->last_level, 
			victim->timer,victim->rottimer);
	printf_to_char(ch,"Act:         {R%s{x\n\r",act_bits_name(victim->act));
	printf_to_char(ch,"Comm:        {R%s{x\n\r",comm_bits_name(victim->comm));
	printf_to_char(ch,"Offensive:   {R%s{x\n\r",off_bits_name(victim->off_bits));
	printf_to_char(ch,"Defensive:   {R%s{x\n\r",def_bits_name(victim->def_bits));
	printf_to_char(ch,"Resistance:  {R%s{x\n\r", res_bits_name(victim->res));
	printf_to_char(ch,"Form:	     {R%s{x\n\r", form_bits_name(victim->form));
	printf_to_char(ch,"Parts:       {R%s{x\n\r",part_bits_name(victim->parts));
	printf_to_char(ch,"Affected by: {R%s{x\n\r", affect_bits_name(victim->affected_by));

	printf_to_char(ch,"Master: %s  Leader: %s\n\r",
		victim->master ? victim->master->name : "(none)",
		get_leader(victim)->name);
	if(!IS_NPC(ch))
	printf_to_char(ch,"GLeader %s  Pet: %s  Horse: %s  Guardby: %s Guarding: %s\n\r",
		"(none)",
		victim->pet ? victim->pet->name : "(none)",
		victim->mount ? victim->mount->name : "(none)",
		victim->guardby ? victim->guardby->name : "(none)",
		victim->guarding ? victim->guarding->name : "(none)");

	//printf_to_char(ch,"Mounted: %d. Riding: %d.\n\r",ch->mounted,ch->riding);
	if (!IS_NPC(victim))
		printf_to_char(ch,"Security: %d.\n\r",victim->pcdata->security);
	printf_to_char(ch,"Short description: %s\n\rLong  description: %s",
		IS_NPC(victim) ? victim->short_descr : "(none)",
		victim->long_descr[0] ? victim->long_descr : "(none)\n\r" );
	if (IS_NPC(victim) && victim->spec_fun != 0)
		printf_to_char(ch,"Mobile has special procedure %s.\n\r",spec_name(victim->spec_fun));
	for (paf = victim->affected;paf;paf = paf->next)
		printf_to_char(ch,"Spell: '%s' mods %s by %d for {w%d {xticks, bits: %s, level {w%d{x.\n\r",
			skill_table[(int) paf->type].name,
			affect_loc_name( paf->location ),
			paf->modifier,
			paf->duration,
			paf->where == TO_AFFECTS ? affect_bit_name(paf->bitvector) : res_bit_name(paf->bitvector),
			paf->level);

	if(IS_IMMORTAL(victim))
		ch->send("Imm ");
	else
		ch->send("not Imm ");
	printf_to_char(ch,"%d ",get_trust(victim));
	if(IS_HL(victim))
		ch->send("HL ");
	else
		ch->send("Not HL ");
	if(guilds[victim->guild].type == GTYPE_POPULACE)
		ch->send("Populace.\n\r");
	else
		ch->send("Not populace.\n\r");*/
}
char *printName2(CHAR_DATA *wch, bool iswhois){
	static char buf[MSL];
	char icon[5];

	if(IS_IMMORTAL(wch)){
		if(IS_HL(wch))
			sprintf(icon,"{C*");
		else
			sprintf(icon,"{c*");
	}
	else{
		if(IS_HL(wch)){
			if(wch->level == KING)
				sprintf(icon,"{R*");
			else if(wch->level == HLEADER)
				sprintf(icon,"{R+");
			else
				sprintf(icon,"{R=");
		}
		else{
			if(wch->level == LEVEL_HERO)
				sprintf(icon,"{G-");
			else
				sprintf(icon," ");
		}
	}

		sprintf(buf,"%s {x%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s{x\n\r",
			str_dup(icon),
			wch->isplr(PL_KILLER) ? "{x[{RK{x]" : "",
			wch->isplr(PL_THIEF) ? "[{YT{x]" : "",
			wch->incog_level >= LEVEL_HERO ? "{x({dIncog{x) " : "",
			wch->invis_level >= LEVEL_HERO ? "{x({GWizi{x) " : "",
			wch->iscomm(CM_BUSY) ? "[{RBusy{x] " : "",
			wch->iscomm(CM_AFK) ? (!IS_NPC(wch) && wch->pcdata->afk[0]) ? "{x[{YAFK{x] " : "{x[{yAFK{x] " : "",
			wch->iscomm(CM_DEBUG) ? "{cLAPD{x " : "",
			wch->iscomm(CM_STUPID) ? "{x-{YSTUPID{x-> " : "",
			wch->isplr(PL_ARENA) ? "<{MARENA{x> " : "",
			wch->nobility >= 5 ? nobility_flags[wch->nobility].name[wch->sex] : wch->nobility == 0 ? nobility_flags[wch->nobility].name[wch->sex] : "",
			wch->nobility >= 5 || wch->nobility == 0 ? " " : "",
			wch->name,
			strcmp(wch->pcdata->lname,"{x") ? " " : "",
			strcmp(wch->pcdata->lname,"(null)") ? wch->pcdata->lname : "",
			IS_NPC (wch) ? "" : wch->pcdata->title);
	return buf;
}
void do_immwho(CHAR_DATA *ch,char *argument){
	CHAR_DATA *wch;
	char buf[MSL],arg[MSL];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int wlevel, nMatch, fBCheck,guildlist[MAX_GUILD];
	bool group = false;

	one_argument(argument,arg);

    printf_to_char(ch,"\n\r           {rWake{x\n\r");
	if(arg[0] != '\0'){
		if(!str_prefix(arg,"group")){
			ch->send("\n\rPlayers in your grouping range:\n\r");
			group = true;
		}
		else
			return;
	}

    nMatch = 0;
    buf[0] = '\0';
    output = new_buf ();

	for(int i = 0;i<MAX_GUILD;i++)
		guildlist[i] = 0;

	for (d = descriptor_list;d;d = d->next){
		if (d->connected != CON_PLAYING)
			continue;
		wch = d->original ? d->original : d->character;
		if (!can_see (ch,wch))
			continue;
		if (group && (ch->level - wch->level < -10 || ch->level - wch->level > 10))
			continue;
		if(guilds[wch->guild].hidden){
			if(ch->guild != wch->guild){
				guildlist[guilds[wch->guild].hidden] = 1;// && (ch->guild == guilds[wch->guild].hidden && ch->rank != 0))
				continue;
			}
		}
		guildlist[wch->guild] = 1;
	}

	for(int i = 0;i<MAX_GUILD;i++){
		if(guildlist[i] == 0)
			continue;
		printf_to_char(ch,"\n\r%s\n\r",guilds[i].who_name);

		for(int j = MAX_LEVEL+1;j > 0;j--){
			for (d = descriptor_list; d != NULL; d = d->next){
				if(d->connected != CON_PLAYING)
					continue;
				wch = (d->original != NULL) ? d->original : d->character;
				if(wch->level != j)
					continue;
				if(!can_see(ch,wch))
					continue;
				if (group && (ch->level - wch->level < -10 || ch->level - wch->level > 10))
					continue;
				if(wch->guild != i){
					if(ch->guild == wch->guild)
						continue;
					//else if(ch->guild == guilds[wch->guild].hidden && ch->rank == 0)
					//	continue;
					else if(!guilds[wch->guild].hidden || guilds[wch->guild].hidden != i)
						continue;
				}
				//printf_to_char(ch,"%s%s",wch->petition == ch->guild ? wch->petition != 0 ? "[{YPETITION{x]" : "" : "",printName(wch,false));
				nMatch++;
			}
		}
		
	}
	sprintf(buf,"\n\rPlayers found: {R%d{x, Most today: {G%d{x, Most ever: {G%d{x\n\r",nMatch,mud.d_con,mud.max_con);
    add_buf(output,buf);
    page_to_char(buf_string(output),ch);
    free_buf(output);
	if (double_exp)
		send_to_char("Doubles are currently {Gactive{x.\n\r",ch);
}
