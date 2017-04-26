#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "lookup.h"

/*
 * Local functions.
 */
extern char *target_name;
bool IS_WET_BG (CHAR_DATA*);
bool remove_obj(CHAR_DATA *ch,int iWear,bool fReplace);

void spell_acid_blast(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
    int dam;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Acid damage %d %d ",level,ch->getslvl(sn));
    dam = (level * 2) + (slvl * get_curr_stat(ch,STAT_END));
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," %d",dam);
	if(number_percent() < get_curr_stat(ch,STAT_INT) * 3)
	 dam += dam / 5 * number_range(2,3);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," %d",dam);
    if(saves_spell(level,victim,DAM_ACID))
		dam *= .75;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," %d\n\r",dam);
    spell_damage(ch,victim,dam,sn,DAM_ACID,true);
	if(!saves_spell(level,victim,DAM_ACID)){
		act("$n is weakened by the burning acid!",victim,NULL,NULL,TO_ROOM);
		victim->send("You are weakened by the burning acid!\n\r");
		affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_AC,100,0);
	}
}

void spell_color_spray(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,tdam,which = number_range(1,6),loc,bvector;
	bool found = true;

    dam = number_range(40,100);

	switch(which){
	case 1:
		if (saves_spell(level,victim,DAM_MENTAL))
 			tdam = dam * .75;
		act("$n is struck by a {Mmagenta{x shot!",victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Mmagenta{x shot!",victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_MENTAL,false);
		if (!(found = saves_spell(level,victim,DAM_MENTAL))){
			act("$n looks dizzy.", victim,NULL,NULL,TO_ROOM);
			act("You feel dizzy.", victim,NULL,NULL,TO_CHAR);
			loc = APPLY_WIS;
			bvector = AF_CONFUSION;
		}
		break;
	case 2:
		if (saves_spell(level,victim,DAM_FIRE))
 			tdam = dam * .75;
		act("$n is struck by a {Rred{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Rred{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_FIRE,false);
		if (!(found = saves_spell(level,victim,DAM_MENTAL))){
			act("$n sizzles and burns.", victim,NULL,NULL,TO_ROOM);
			act("You sizzle and burn.", victim,NULL,NULL,TO_CHAR);
			loc  = APPLY_STR;
			bvector = AF_DEGENERATION;
		}
		break;
	case 3:
		if (saves_spell(level,victim,DAM_LIGHTNING))
 			tdam = dam * .75;
		act("$n is struck by a {Yyellow{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Yyellow{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_LIGHTNING,false);
		if (!(found = saves_spell(level,victim,DAM_MENTAL))){
			act("$n is shocked.", victim,NULL,NULL,TO_ROOM);
			act("You are shocked.", victim,NULL,NULL,TO_CHAR);
			loc  = APPLY_AGI;
			bvector = AF_BLIND;
		}
		break;
	case 4:
		if (saves_spell(level,victim,DAM_DISEASE))
 			tdam = dam * .75;
		act("$n is struck by a {Ggreen{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Ggreen{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_DISEASE,false);
		if (!(found = saves_spell(level,victim,DAM_MENTAL))){
			act("$n looks sick.", victim,NULL,NULL,TO_ROOM);
			act("You feel sick.", victim,NULL,NULL,TO_CHAR);
			loc  = APPLY_STR;
			bvector = AF_POISON;
		}
		break;
	case 5:
		if (saves_spell(level,victim,DAM_WATER))
 			tdam = dam * .75;
		act("$n is struck by a {Bblue{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Bblue{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_WATER,false);
		if (!(found = saves_spell(level,victim,DAM_MENTAL))){
			act("$n sputters and spits.", victim,NULL,NULL,TO_ROOM);
			act("You sputter and spit.", victim,NULL,NULL,TO_CHAR);
			loc  = APPLY_WIS;
			bvector = AF_FAERIE_FIRE;
		}
		break;
	case 6:
		if (saves_spell(level,victim,DAM_COLD))
 			tdam = dam * .75;
		act("$n is struck by a {Ccyan{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Ccyan{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_COLD,false);
		if (!(found = saves_spell(level,victim,DAM_MENTAL))){
			act("$n shivers.", victim,NULL,NULL,TO_ROOM);
			act("You shiver.", victim,NULL,NULL,TO_CHAR);
			loc  = APPLY_AGI;
			bvector = AF_SLOW;
		}
		break;
	default:
		ch->send("Colorspraybug!\n\r");
		break;
	}
	if (!found)
		affect_set(victim,TO_AFFECTS,sn,level,slvl,2,loc,-2,bvector);
}

void spell_sonic_boom(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
	int dam;

	act("$n throws $s hand down and a blast of sound rockets at $N!",ch,NULL,victim,TO_NOTVICT);
	act("$n throws $s hand down and a blast of sound rockets at you!",ch,NULL,victim,TO_VICT);
	act("You throw your hand down and release a blast of sound at $N!",ch,NULL,victim,TO_CHAR);

	dam = level * (get_curr_stat(ch,STAT_INT) / ch->level) + (ch->level / 2);

	spell_damage(ch,victim,dam,sn,DAM_SOUND,true);
	if (!saves_spell(level,victim,DAM_SOUND)){
		affect_set(victim,TO_AFFECTS,sn,level,slvl,level/2,APPLY_WIS,-3,AF_DEAF);
		send_to_char("You can't hear!\n\r",victim);
		act("$n appears deaf.",victim,NULL,NULL,TO_ROOM);
	}
}

void spell_gush(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,4);
    if (!saves_spell(level,victim,DAM_WATER))
		dam *= 2;

	act("You hold your hand out and a strong blast of water gushes out at $N!",ch,NULL,victim,TO_CHAR);
	act("$n holds a hand out at $N and a strong blast of water gushes out at $M!",ch,NULL,victim,TO_NOTVICT);
	act("$n holds $s palm out at you and a strong blast of water gushes at you!",ch,NULL,victim,TO_VICT);
	spell_damage(ch,victim,dam,sn,DAM_WATER,true);
}

void spell_gust(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo,*vch,*vch_next;
	int dam,tdam;
	bool found,first = true;

	dam = number_range(level/2,ch->level*1.5);

	act("You release a spiraling gust of wind at $N!",ch,NULL,victim,TO_CHAR);
	act("$n releases a spiraling gust of wind at you!",ch,NULL,victim,TO_VICT);
	act("$n releases a spiraling gust of wind at $N!",ch,NULL,victim,TO_NOTVICT);

	for (vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if (is_same_group(victim,vch)){
			found = true;
			if (first){
				act("The gust of wind sprays outwards, around $n!",victim,NULL,NULL,TO_ROOM);
				act("The gust of wind sprays outwards, around you!",victim,NULL,NULL,TO_CHAR);
				first = false;
			}
			tdam = dam;
			if (vch != victim && saves_spell(level,vch,DAM_WIND))
				tdam /= 2;
			spell_damage(ch,vch,dam,sn,DAM_WIND,true);
		}
	}
	if (!found){
		act("The wind sprays out wildly before dying off.",ch,NULL,NULL,TO_ROOM);
		act("The wind sprays out wildly before dying off.",ch,NULL,NULL,TO_CHAR);
	}
}

void spell_rock_spike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,4);
    if (!saves_spell(level,victim,DAM_EARTH))
		dam *= 2;

	act("You strike the ground, feeding it mana and a rock spike hits $N!",ch,NULL,victim,TO_CHAR);
	act("$n strikes the ground and glows before a rock spike hits $N!",ch,NULL,victim,TO_VICT);
	act("$n strikes the ground, glowing before a large rock spike rises up and hits you!",ch,NULL,victim,TO_NOTVICT);
	spell_damage(ch,victim,dam,sn,DAM_EARTH,true);
}

void spell_freeze(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,4);

    if (!saves_spell(level,victim,DAM_COLD))
		dam *= 2;

	act("You wave your hand and a blast of cold erupts around $N!",ch,NULL,victim,TO_CHAR);
	act("$n waves $s hand and a blast of cold erupts around $N!",ch,NULL,victim,TO_VICT);
	act("$n waves $s hand and a blast of cold erupts around you!",ch,NULL,victim,TO_NOTVICT);
	spell_damage(ch,victim,dam,sn,DAM_COLD,true);
}

void spell_charge_shot(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
return;
	if(ch->chargetime < 1){
		act("$n seems to focus hard..",ch,NULL,NULL,TO_ROOM);
		act("You focus hard and begin charging you spell.",ch,NULL,NULL,TO_CHAR);
		ch->chargevict = victim;
		ch->chargetime = 1;
	}
	else{
		//if (!ch->chargevict || ch->chargevict->in_room != ch->in_room){
		//	ch->send("Failed.\n\r");
		//	return;
		//}
		act("$n releases a charged blast of energy at $N!",ch,NULL,victim,TO_NOTVICT);
		act("$n releases a charge blast of energy at you!",ch,NULL,victim,TO_VICT);
		act("You release your charged shot at $N!",ch,NULL,victim,TO_CHAR);
		dam = level;
		dam += get_curr_stat(ch,STAT_INT)/2;
		dam += (dam * (ch->chargetime / 2))/3;
		ch->chargetime = 0;
		spell_damage(ch,victim,dam,sn,DAM_ENERGY,true);
		victim->modmana(-dam / 4);
	}
}

void spell_magic_armor(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *obj;
	AFFECT_DATA *paf;

	act("$n places $s hand to $s chest, and $s hand glows.",ch,NULL,NULL,TO_ROOM);
	act("You place your hand to your chest, and your hand glows.",ch,NULL,NULL,TO_CHAR);

	if((obj = get_eq_char(ch,WEAR_TORSO))){
		if(obj->pIndexData->vnum == 84){
			act("$p evaporates in a flash!",ch,obj,NULL,TO_ROOM);
			act("$p evaporates in a flash!",ch,obj,NULL,TO_CHAR);
			extract_obj(obj);
		}
		else if(!remove_obj(ch,WEAR_TORSO,true))
			return;
	}

	obj = create_object(get_obj_index(84),ch->level);
	SET_BIT(obj->extra_flags,ITM_GLOW);
	obj->short_descr = str_dup("a breastplate of mana");
	obj->description = str_dup("A breastplate forged of pure mana is here.");
	obj->name = str_dup("mana breastplate plate");

	obj->timer = 5;
	obj->value[0] = ch->level/10;
	obj->value[1] = ch->level/10;
	obj->value[2] = ch->level/10;
	obj->value[3] = ch->level/9;

	paf = new_affect();

	paf->where			= TO_OBJECT;
	paf->type			= sn;
	paf->level			= level;
	paf->duration		= -1;
	paf->location		= APPLY_DAMROLL;
	paf->modifier		= ch->level/10;
	paf->bitvector		= 0;
	paf->next			= obj->affected;
	obj->affected		= paf;

	paf = new_affect();

	paf->where			= TO_OBJECT;
	paf->type			= sn;
	paf->level			= level;
	paf->duration		= -1;
	paf->location		= APPLY_HITROLL;
	paf->modifier		= ch->level/10;
	paf->bitvector		= 0;
	paf->next			= obj->affected;
	obj->affected		= paf;

	paf = new_affect();
    paf->where			= TO_RESIST;
    paf->type			= sn;
    paf->level			= level;
    paf->duration		= -1;
    paf->location		= APPLY_AC;
    paf->modifier		= -100;
    paf->bitvector		= RS_MAGIC;
	paf->next			= obj->affected;
	obj->affected		= paf;

	obj_to_char(obj,ch);
	wear_obj(ch,obj,true,false);

	act("$p appears on $n's chest in a burst of crackling magic.",ch,obj,NULL,TO_ROOM);
	act("In a burst of crackling magic, $p appears on your chest.",ch,obj,NULL,TO_CHAR);
}
