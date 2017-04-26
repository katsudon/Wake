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

char * const rdir_name [] ={
    "the north", "the east", "the south", "the west", "above", "below"
};


void check_frost_field(CHAR_DATA *ch){
	int tdam = 0;
	AFFECT_DATA *frost = affect_find(ch->in_room->affected,skill_lookup("frost field"));

	if(!frost){
		ch->send("Broken frost field!\n\r");
		ch->in_room->remraf(RAF_FROSTFIELD);
	}
	else{
		ch->send("Frost explodes randomly around you!\n\r");
		if(number_percent() < 50)
			return;

		if(canPK(ch,NULL,false)){
			tdam = dice(frost->modifier * 50,frost->modifier * 10);
			if(saves_spell(frost->level,ch,DAM_COLD))
				tdam *= .75;
			act("The frost field explodes around $n!",ch,NULL,NULL,TO_ROOM);
			ch->send("You are struck by an exploding blast of frost.\n\r");
			spell_damage(ch,ch,tdam,frost->type,DAM_COLD,true);
		}
	}
}

bool check_mental_focus(CHAR_DATA *ch){
	int sn = gsn_mental_focus,skill = get_skill(ch,sn) /2;

	skill += get_curr_stat(ch,STAT_WIS);

	if(roll_chance(ch,skill) && number_range(1,5) <= ch->getslvl(sn)){
		ch->send("Your mental fortification helps you maintain concentration.\n\r");
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}

int magical_mastery(CHAR_DATA *ch,int dam){
	int sn = gsn_magical_mastery,skill = get_skill(ch,sn) * .75;

	if(roll_chance(ch,skill)){
		dam += (dam / 2) * ch->getslvl(sn) / 5;
		check_improve(ch,sn,true,1);
		return dam;
	}
	check_improve(ch,sn,false,1);
	return dam;
}

int elemental_mastery(CHAR_DATA *ch,int dam){
	int sn = gsn_elemental_mastery,skill = get_skill(ch,sn) * .75;

	if(roll_chance(ch,skill)){
		dam += (dam / 2) * ch->getslvl(sn) / 5;
		check_improve(ch,sn,true,1);
		return dam;
	}
	check_improve(ch,sn,false,1);
	return dam;
}

void spell_arcane_burst(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

	if (is_affected(ch,sn) || ch->isaff(AF_DOUBLECAST)){
			send_to_char("You are already boiling.\n\r",ch);
		return;
	}
	affect_set(ch,TO_AFFECTS,sn,level,slvl,24,APPLY_INT,2,AF_DOUBLECAST);
	ch->send("Your mana feels like it boils and burns.\n\r");
	act("$n seems to glow a slight blue.",ch,NULL,NULL,TO_ROOM);
}

void spell_missile_swarm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo,*vch,*vch_next;
	int i;
	static const sh_int dam_each[] = {
		0,
		3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
		7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
		9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
		11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
		13, 13, 13, 13, 13,	14, 14, 14, 14, 14,
		15, 15, 15, 15, 15, 16, 16, 16, 16, 16,
		17, 17, 17, 17, 17, 18, 18, 18, 18, 18,
		19, 19, 19, 19, 19, 20, 20, 20, 20, 20,
		21, 21, 21, 21, 21, 22, 22, 22, 22, 22,
		23, 23, 23, 23, 23, 24, 24, 24, 24, 24
	};
	int dam,count=1,chance;

	for (vch = ch->in_room->people;vch;vch = vch->next_in_room){
		if (vch == ch || !canPK(ch,vch,false))
			continue;

		chance = number_percent();
		if (is_same_group(ch,vch))
			chance /= 2;
		if (number_percent() > chance)
			continue;
		count++;
	}
	count = count * 5.5;

	act("$n releases a swarm of magic missiles!",ch,NULL,NULL,TO_ROOM);
	act("You release a swarm of magic missiles!",ch,NULL,NULL,TO_CHAR);

	while(count--){
		vch = get_random_char(ch,NULL,NULL);
		if (!canPK(ch,vch,false))
			count++;
		else{
			level	= UMIN(level,sizeof(dam_each)/sizeof(dam_each[0]) - 1);
			level	= UMAX(0,level);
			dam		= number_range(dam_each[level] / 2,dam_each[level] * 5);
			if (saves_spell(level,vch,DAM_ENERGY))
				dam /= 2;
			spell_damage(ch,vch,dam,sn,DAM_ENERGY,true);
		}
	}
}

void spell_prismshotarray(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){//nash make the % based on slvl
    AFFECT_DATA af;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,tdam;

    act("$n points at $N and an array of colors shoot from $s finger.", ch,NULL,victim,TO_ROOM);
    act("You point at $N and an array of colors shoot from your finger.", ch,NULL,victim,TO_CHAR);
    act("$n points at you and an array of colors shoot from $s finger.", ch,NULL,victim,TO_VICT);

    dam = number_range(40,100);
	if (number_percent() < 50){
		tdam = dam;
		if (saves_spell(level,victim,DAM_MENTAL))
 			tdam = dam * .75;
		act("$n is struck by a {Mmagenta{x shot!",victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Mmagenta{x shot!",victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_MENTAL,false);
		if (!saves_spell(level,victim,DAM_MENTAL)){
			act("$n looks dizzy.", victim,NULL,NULL,TO_ROOM);
			act("You feel dizzy.", victim,NULL,NULL,TO_CHAR);
			affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_WIS,-2,AF_CONFUSION);
		}
	}
	if (number_percent() < 50){
		tdam = dam;
		if (saves_spell(level,victim,DAM_FIRE))
 			tdam = dam * .75;
		act("$n is struck by a {Rred{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Rred{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_FIRE,false);
		if (!saves_spell(level,victim,DAM_MENTAL)){
			act("$n sizzles and burns.", victim,NULL,NULL,TO_ROOM);
			act("You sizzle and burn.", victim,NULL,NULL,TO_CHAR);
			affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_STR,-2,AF_DEGENERATION);
		}
	}
	if (number_percent() < 50){
		tdam = dam;
		if (saves_spell(level,victim,DAM_LIGHTNING))
 			tdam = dam * .75;
		act("$n is struck by a {Yyellow{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Yyellow{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_LIGHTNING,false);
		if (!saves_spell(level,victim,DAM_MENTAL)){
			act("$n is shocked.", victim,NULL,NULL,TO_ROOM);
			act("You are shocked.", victim,NULL,NULL,TO_CHAR);
			affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_AGI,-2,AF_BLIND);
		}
	}
	if (number_percent() < 50){
		tdam = dam;
		if (saves_spell(level,victim,DAM_DISEASE))
 			tdam = dam * .75;
		act("$n is struck by a {Ggreen{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Ggreen{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_DISEASE,false);
		if (!saves_spell(level,victim,DAM_MENTAL)){
			act("$n looks sick.", victim,NULL,NULL,TO_ROOM);
			act("You feel sick.", victim,NULL,NULL,TO_CHAR);
			affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_STR,-2,AF_POISON);
		}
	}
	if (number_percent() < 50){
		tdam = dam;
		if (saves_spell(level,victim,DAM_WATER))
 			tdam = dam * .75;
		act("$n is struck by a {Bblue{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Bblue{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_WATER,false);
		if (!saves_spell(level,victim,DAM_MENTAL)){
			act("$n sputters and spits.", victim,NULL,NULL,TO_ROOM);
			act("You sputter and spit.", victim,NULL,NULL,TO_CHAR);
			affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_WIS,-2,AF_FAERIE_FIRE);
		}
	}
	if (number_percent() < 50){
		tdam = dam;
		if (saves_spell(level,victim,DAM_COLD))
 			tdam = dam * .75;
		act("$n is struck by a {Ccyan{x shot!", victim,NULL,NULL,TO_ROOM);
		act("You are struck by a {Ccyan{x shot!", victim,NULL,NULL,TO_CHAR);
		spell_damage(ch,victim,tdam,sn,DAM_COLD,false);
		if (!saves_spell(level,victim,DAM_MENTAL)){
			act("$n shivers.", victim,NULL,NULL,TO_ROOM);
			act("You shiver.", victim,NULL,NULL,TO_CHAR);
			affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_AGI,-2,AF_SLOW);
		}
	}
}

void spell_energy_bomb(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*nvir;
	int energy,hp_dam,dam;

	act("$n hurls a large ball of energy at the ground and it explodes!",ch,NULL,NULL,TO_ROOM);
	act("You hurl a ball of energy at the ground and it explodes!",ch,NULL,NULL,TO_CHAR);

	energy = ch->getmana() / 3;
	energy = UMAX(10,energy);
	energy = (energy * ch->hit) / ch->max_hit;
	hp_dam = number_range(energy/2,energy);

	dam = UMAX(100,hp_dam);

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir){
		nvir = vch->next_in_room;
		if (vch == ch || is_same_group(vch,ch))
			continue;

		if (saves_spell(level,vch,DAM_ENERGY))
			spell_damage(ch,vch,dam/2,sn,DAM_ENERGY,true);
		else
			spell_damage(ch,vch,dam,sn,DAM_ENERGY,true);
	}
}

void spell_light_strike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam;

	act("$n fires a focused beam of light at $N!",ch,NULL,victim,TO_NOTVICT);
	act("$n fires a focused beam of light at you!",ch,NULL,victim,TO_VICT);
	act("You fire a focused beam of light at $N!",ch,NULL,victim,TO_CHAR);

	dam = level + 5 * ((get_curr_stat(ch,STAT_INT) * 4) + get_curr_stat(ch,STAT_WIS)) / 5;

	spell_damage(ch,victim,dam,sn,DAM_LIGHT,true);
	if (!saves_spell(level,victim,DAM_LIGHT)){
		affect_set(victim,TO_AFFECTS,sn,level,slvl,level/2,APPLY_HITROLL,-10,AF_BLIND);
		send_to_char("You are blinded!\n\r",victim);
		act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
	}
}

void spell_drown(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam;

	act("$n encases $N in a crushing sphere of water!",ch,NULL,victim,TO_NOTVICT);
	act("A crushing sphere of water evelopes you!",ch,NULL,victim,TO_VICT);
	act("You encase $N in a sphere of crushing water!",ch,NULL,victim,TO_CHAR);

	dam = level + 5 * ((get_curr_stat(ch,STAT_INT) * 4) + get_curr_stat(ch,STAT_WIS)) / 5;

	if (!saves_spell(level,victim,DAM_WATER)){
		dam *= 2;
		affect_set(victim,TO_AFFECTS,sn,level,slvl,level/2,APPLY_HITROLL,-10,AF_DROWNING);
		send_to_char("You are blinded!\n\r",victim);
		act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
	}
	spell_damage(ch,victim,dam,sn,DAM_WATER,true);
}

void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act("$n points at $N and a bolt of crackling lightning strikes $M!",ch,NULL,victim,TO_NOTVICT);
	act("$n points at you and a bolt of crackling lightning strikes you!",ch,NULL,victim,TO_VICT);
	act("You point at $N and a bolt of crackling lightning strikes $M!",ch,NULL,victim,TO_CHAR);

	dam = level + 5 * ((get_curr_stat(ch,STAT_INT) * 4) + get_curr_stat(ch,STAT_WIS)) / 5;

	if (!saves_spell(level,victim,DAM_LIGHTNING))
		dam *= 1.5;

	spell_damage(ch,victim,dam,sn,DAM_LIGHTNING,true);
}

void spell_tempest(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam,tdam;

	act("$n creates a furious tempest!",ch,NULL,NULL,TO_ROOM);
	act("You create a furious tempest!",ch,NULL,NULL,TO_CHAR);

	dam = level + (level * ch->getslvl(sn) / 5);

	for(int i = ch->getslvl(sn) * 2;i > 0;i--){
		for(vch = ch->in_room->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if(is_same_group(ch,vch))
				continue;
			tdam = dam;
			if (!saves_spell(level,vch,DAM_WATER))
				tdam *= 1.5;

			spell_damage(ch,vch,tdam,sn,DAM_WATER,true);
		}
	}
}

void spell_wind_burst(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,tdam;

	act("$n waves $s hand at $N and several bursts of sharp wind strike $M!",ch,NULL,victim,TO_NOTVICT);
	act("$n waves $s hand at you and several bursts of razor sharp wind strike you!",ch,NULL,victim,TO_VICT);
	act("You wave your hand, releasing several bursts of wind at $N!",ch,NULL,victim,TO_CHAR);

	dam = level + (level * ch->getslvl(sn) / 5);

	for(int i = ch->getslvl(sn) * 2;i > 0;i--){
		tdam = dam;
		if (!saves_spell(level,victim,DAM_WIND))
			tdam *= 1.5;

		spell_damage(ch,victim,tdam,sn,DAM_WIND,true);
	}
}
//NASH maybe make this not hurt next_room people who are low hp or vch->hp < tdam
void spell_epicenter(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *was_in;
	int dam,tdam;

	act("$n places $s hand on the ground and a massive shockwave ripples out in all directions!",ch,NULL,NULL,TO_ROOM);
	act("You place your hand on the ground and a massive shockwave ripples out in all directions!",ch,NULL,NULL,TO_CHAR);

	dam = level + (level * ch->getslvl(sn));

	was_in = ch->in_room;
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(vch,ch))
			continue;
		tdam = dam;
		if (!saves_spell(level,vch,DAM_EARTH))
			tdam *= 1.5;

		spell_damage(ch,vch,tdam,sn,DAM_EARTH,true);
	}
	for(int door = 0;door <= 5;door++){
		if((pexit = was_in->exit[door]) && pexit->u1.to_room && pexit->u1.to_room != was_in){
			ch->in_room = pexit->u1.to_room;
			for(vch = ch->in_room->people;vch;vch = vch_next){
				vch_next = vch->next_in_room;
				if(is_same_group(vch,ch))
					continue;
				tdam = dam;
				if (!saves_spell(level,vch,DAM_EARTH))
					tdam *= 1.5;

				spell_damage(ch,vch,tdam,sn,DAM_EARTH,true);
			}
		}
	}
    ch->in_room = was_in;
}

void spell_armageddon(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam,tdam;

	dam = dice(level * 2,ch->getslvl(sn));

	act("$n unleashes a rain of rocks!",ch,NULL,NULL,TO_ROOM);
	act("You unleash a rain of rocks!",ch,NULL,NULL,TO_CHAR);
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(vch,ch))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_EARTH))
			tdam /= 2;
		spell_damage(ch,vch,tdam,skill_lookup("rock spike"),DAM_EARTH,true);
	}
	act("$n unleashes a rain of fire!",ch,NULL,NULL,TO_ROOM);
	act("You unleash a rain of fire!",ch,NULL,NULL,TO_CHAR);
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(vch,ch))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_FIRE))
			tdam /= 2;
		spell_damage(ch,vch,tdam,skill_lookup("fireball"),DAM_FIRE,true);
	}
	act("$n unleashes a rain of solid water!",ch,NULL,NULL,TO_ROOM);
	act("You unleash a rain of solid water!",ch,NULL,NULL,TO_CHAR);
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(vch,ch))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_WATER))
			tdam /= 2;
		spell_damage(ch,vch,tdam,skill_lookup("gush"),DAM_WATER,true);
	}
	act("$n unleashes a rain of wind blasts!",ch,NULL,NULL,TO_ROOM);
	act("You unleash a rain of wind blasts!",ch,NULL,NULL,TO_CHAR);
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(vch,ch))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_WIND))
			tdam /= 2;
		spell_damage(ch,vch,tdam,skill_lookup("wind burst"),DAM_WIND,true);
	}
	act("$n unleashes a rain of frost!",ch,NULL,NULL,TO_ROOM);
	act("You unleash a rain of frost!",ch,NULL,NULL,TO_CHAR);
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(vch,ch))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_COLD))
			tdam /= 2;
		spell_damage(ch,vch,tdam,skill_lookup("frost breath"),DAM_COLD,true);
	}
	act("$n unleashes a rain of lightning!",ch,NULL,NULL,TO_ROOM);
	act("You unleash a rain of lightning!",ch,NULL,NULL,TO_CHAR);
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(vch,ch))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_LIGHTNING))
			tdam /= 2;
		spell_damage(ch,vch,tdam,skill_lookup("lightning bolt"),DAM_LIGHTNING,true);
	}
}

void do_overdrive(CHAR_DATA *ch,char *argument){
    AFFECT_DATA af;
	int skill = get_skill(ch,gsn_overdrive) * .8;

    if (ch->isaff(AF_OVERDRIVE)){
		ch->send("You are already crazy!\n\r");
		return;
    }

	if (skill < 1){
		ch->send("You're too wussy to do that.\n\r");
		return;
	}

	if(number_percent() < skill){
		WAIT_STATE(ch,skill_table[gsn_overdrive].beats);
		ch->modmana(-200);

		check_improve(ch,gsn_overdrive,true,2);

		affect_set(ch,TO_AFFECTS,gsn_overdrive,ch->level,ch->getslvl(gsn_overdrive),2,APPLY_NONE,0,AF_OVERDRIVE);
		act("$n seems crazy!",ch,NULL,NULL,TO_ROOM);
		ch->send("You go crazy!\n\r");
	}
	else{
		ch->send("You fail..\n\r");
		check_improve(ch,gsn_overdrive,false,4);
	}
    return;
}

bool check_spell_efficiency(CHAR_DATA *ch){
	int skill = get_skill(ch,gsn_cast_efficiency) * 0.5;
	if (skill < 1)
		return false;

	skill = skill * get_curr_stat(ch,STAT_WIS) / STAT_MAX;

	skill += 10 * get_curr_stat(ch,STAT_AGI) / STAT_MAX;
	skill += 5 * get_curr_stat(ch,STAT_INT) / STAT_MAX;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SEFF: ({g%d{x)\n\r",skill);

	if (roll_chance(ch,skill)){
		check_improve(ch,gsn_cast_efficiency,1,true);
		return true;
	}
		check_improve(ch,gsn_cast_efficiency,1,false);
	return false;
}

//NASHNEEDSTOMAKESURETHISISIMPLEMENTEDINBUFFSANDNERFS
int spell_proficiency(CHAR_DATA *ch,int dam,int what_type){
	if (get_skill(ch,gsn_spell_proficiency) < 1)
		return dam;

	if (number_percent() < get_skill(ch,gsn_spell_proficiency)){
		check_improve(ch,gsn_spell_proficiency,true,1);
		switch (what_type){
		case SPL_DAM:
			return dam + ((dam / 50) * ch->getslvl(gsn_spell_proficiency));
		case SPL_BUF:
			return dam * (ch->level * 3) / 100;
		case SPL_NRF:
			return dam * (ch->level * 2) / 100;
		case SPL_CST:
			return dam / 2;
		default:
			return dam;
		}
	}
	check_improve(ch,gsn_spell_proficiency,false,1);
	return dam;
}

void spell_omniblast(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
//fire wind water lightning cold earth holy negative poison
    dam = ch->level *.45;
    spell_damage(ch,victim,dam,find_spell_list("fireball"),DAM_FIRE,true);
    spell_damage(ch,victim,dam,find_spell_list("gust"),DAM_WIND,true);
    spell_damage(ch,victim,dam,find_spell_list("gush"),DAM_WATER,true);
    spell_damage(ch,victim,dam,find_spell_list("shock"),DAM_LIGHTNING,true);
    spell_damage(ch,victim,dam,find_spell_list("icelance"),DAM_COLD,true);
    spell_damage(ch,victim,dam,find_spell_list("rock spike"),DAM_EARTH,true);
    spell_damage(ch,victim,dam,find_spell_list("claw of shade"),DAM_NEGATIVE,true);
    spell_damage(ch,victim,dam,find_spell_list("poison"),DAM_POISON,true);
    spell_damage(ch,victim,dam,find_spell_list("demonfire"),DAM_MENTAL,true);
}

void spell_icelance(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(level,10);
    if (saves_spell(level,victim,DAM_COLD))
		dam /= 2;

	act("You hurl a ball of mana at $N and it quickly becomes a sharp ice lance!",ch,NULL,victim,TO_CHAR);
	act("$n hurls a blast of mana at $N and it quickly solidifies into a lance of ice!",ch,NULL,victim,TO_NOTVICT);
	act("$n hurls a blast of mana at you and it solidifies to a lance of ice!",ch,NULL,victim,TO_VICT);
	spell_damage(ch,victim,dam,sn,DAM_COLD,true);
}

void spell_wave(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	DESCRIPTOR_DATA *d;
	int dam,tdam;

	if(is_safe(ch,ch)){
		send_to_char("You can't cast that here!",ch);
		return;
	}

	dam = dice(level,3);
	if (IS_WET_BG(ch)){
		act("You call forth a massive tidalwave from the ground!",ch,NULL,NULL,TO_CHAR);
		act("$n calls forth a massive tidalwave from the ground!",ch,NULL,NULL,TO_ROOM);
		dam *= 2;
	}
	else{
		act("You call forth a wave from the ground!",ch,NULL,NULL,TO_CHAR);
		act("$n calls forth a wave from the ground!",ch,NULL,NULL,TO_ROOM);
	}

	for (vch = ch->in_room->people;vch;vch = vch_next){
		tdam = dam;
		vch_next = vch->next_in_room;

		if (saves_spell(level,vch,DAM_WATER))
			tdam /= 2;

		if (is_same_group(ch,vch)){
			act("The wave seems to rush around you.",vch,NULL,NULL,TO_CHAR);
			act("The wave seems to rush around $n.",vch,NULL,NULL,TO_ROOM);
		}
		else
			spell_damage(ch,vch,tdam,sn,DAM_WATER,true);
	}
	for (d = descriptor_list;d;d = d->next){
		vch = d->original ? d->original : d->character;
		if (d->connected == CON_PLAYING && d->character != ch && d->character->in_room->area == ch->in_room->area && d->character->in_room != ch->in_room)
			d->character->send("The ground gets soggy.\n\r");
	}
}

void spell_earthquake(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch, *nvir;
	DESCRIPTOR_DATA *d;

	if(is_safe(ch,ch)){
		send_to_char("You can't cast that here!",ch);
		return;
	}

	ch->send("The earth trembles beneath your feet!\n\r");
	act("$n makes the earth tremble and shiver.",ch,NULL,NULL,TO_ROOM);

	for (vch = ch->in_room->people;vch;vch = nvir){
		nvir = vch->next_in_room;
		if (vch == ch)
			continue;
		if (vch->isaff(AF_FLYING))
			spell_damage(ch,vch,0,sn,DAM_BASH,true);
		else{
			send_to_char("The earthquake hurts!!\n\r",vch);
			spell_damage(ch,vch,dice(level,4),sn,DAM_BASH,true);
		}
	}
	for (d = descriptor_list;d;d = d->next){
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;
		if (d->connected == CON_PLAYING && d->character != ch && d->character->in_room->area == ch->in_room->area && d->character->in_room != ch->in_room)
			d->character->send("The earth trembles and shivers.\n\r");
	}
}

void spell_polarity(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = ch->getmana()/10;
    if (!saves_spell(level,victim,DAM_ENERGY))
		dam *= 2;

	act("You release a concentrated burst of mana at $N!",ch,NULL,victim,TO_CHAR);
	act("$n releases a dark blue ball of energy at $N!",ch,NULL,victim,TO_VICT);
	act("$n releases a blood blue energy ball at you!",ch,NULL,victim,TO_NOTVICT);
	ch->manadamage(dam);
	victim->manadamage(dam/10);
	spell_damage(ch,victim,dam,sn,DAM_ENERGY,true);
}

void arc_damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int level){
	int sn = skill_lookup("call lightning");
	CHAR_DATA *tmp_vict;
	int tdam;

	for(int tries = 0;tries < 2;tries++){
		tmp_vict = get_random_char(ch,NULL,NULL);
		if(!canPK(ch,tmp_vict,false) || number_percent() > 75)
			continue;
		act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
		act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
		tdam = dam / (tries + 1);
		if(saves_spell(level,tmp_vict,DAM_LIGHTNING))
			spell_damage(ch,tmp_vict,tdam * .75,sn,DAM_LIGHTNING,true);
		else
			spell_damage(ch,tmp_vict,tdam,sn,DAM_LIGHTNING,true);
	}
}

void spell_gadzap(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found;
    int dam,arcs = 1;

    act("$n raises $s hand and an enormous bolt of lightning rockets from the sky at $N.", ch,NULL,victim,TO_NOTVICT);
    act("You raise your hand and an enormous bolt of lightning rockets from the sky at $N.", ch,NULL,victim,TO_CHAR);
    act("$n raises $s hand and an enormous bolt of lightning rockets from the sky and hits you!", ch,NULL,victim,TO_VICT);

    dam = dice(level * 3,3);
    if (saves_spell(level,victim,DAM_LIGHTNING))
		spell_damage(ch,victim,dam * .75,sn,DAM_LIGHTNING,true);
	else
		spell_damage(ch,victim,dam,sn,DAM_LIGHTNING,true);
	arc_damage(ch,victim,dam,level);
}

void spell_flammeria(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int n;

	act("$n is surrounded in a spiraling cone of fire!",ch,NULL,NULL,TO_ROOM);
	ch->send("You are surrounded in a spiraling cone of fire!\n\r");

	for(n = 5;n>0;n--){
		for (vch = ch->in_room->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if (ch != vch){
				if(saves_spell(level,ch,DAM_FIRE))
					spell_damage(ch,vch,number_range(level/2,level),sn,DAM_FIRE,true);
				else
					spell_damage(ch,vch,number_range(level * 1.5,ch->level),sn,DAM_FIRE,true);
			}
		}
	}
}

void spell_meteor_storm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *wch;
	int i,tries,dam;
	bool found;
    act("$n raises $s hand and meteors begin raining from the sky!", ch,NULL,NULL,TO_ROOM);
    act("You raise your hand and meteors begin raining from the sky!", ch,NULL,NULL,TO_CHAR);

    dam = dice(level*2,2);

	for (wch = ch->in_room->people;wch;wch = wch->next_in_room)
		wch->meteorhits = 0;

	for (i = 0; i < ch->level/2; i+=5){
		tries = 0;
		found = false;
		for(;!found && tries < 2;){
			tries++;
			wch = get_random_char(ch,NULL,NULL);
			if (!is_same_group(wch,ch) && number_percent() < 45){
				found = true;
				wch->meteorhits++;
			}
		}
		if (!found){
			act("A meteor crashes into the ground!",ch,NULL,NULL,TO_ROOM);
			act("A meteor crashes into the ground!",ch,NULL,NULL,TO_CHAR);
		}
		else{
			spell_damage(ch,wch,dam / wch->meteorhits,sn,DAM_EARTH,true);
			spell_damage(ch,wch,dam/3,sn,DAM_FIRE,false);
		}
	}
}

void spell_fireball(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	static const sh_int dam_each[] = {
		0,
		2,		4,		6,		8,		10,		12,		14,		16,		18,		20,
		22,		24,		26,		28,		30,		35,		40,		45,		50,		55,
		60,		65,		70,		75,		80,		82,		84,		86,		88,		90,
		92,		94,		96,		98,		100,	102,	104,	106,	108,	110,
		112,	114,	116,	118,	120,	122,	124,	126,	128,	130,
		132,	134,	136,	138,	140,	142,	144,	146,	148,	150,
		152,	154,	156,	158,	160,	162,	164,	166,	168,	170,
		172,	174,	176,	178,	180,	182,	184,	186,	188,	190,
		192,	194,	196,	198,	200,	202,	204,	206,	208,	210,
		212,	214,	216,	218,	220,	222,	224,	226,	228,	230,
		232,	234,	236,	238,	240,	242,	244,	246,	248,	250,
		252,	254,	256,	258,	260,	262,	264,	266,	268,	270
    };
    int dam;

    level	= number_range(level,ch->level);
    dam		= number_range(dam_each[level] / 2,dam_each[level] * .75);
	dam += dam / 5 * slvl;
	//printf_to_char(ch,"fireball:%d\n\r",dam);
    if (saves_spell(level,victim,DAM_FIRE))
		dam /= 2;
	//printf_to_char(ch,"fireball:%d\n\r",dam);
    spell_damage(ch,victim,dam,sn,DAM_FIRE,true);
}

void spell_thunderstorm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *vch;
    bool found;
    int dam,tdam;

    act("$n creates a rumbling thunderstorm.",ch,NULL,NULL,TO_ROOM);
    act("You create a rumbling thunderstorm.",ch,NULL,NULL,TO_CHAR);

    dam = dice(level * 3,3);

	for(int i = ch->getslvl(sn) * 2;i > 0;i--){
		for(int tries = 0;tries < 5;tries++){
			if(!(vch = get_random_char(NULL,NULL,ch->in_room)) || !canPK(ch,vch,false))
				continue;
			tdam = dam;
			if(saves_spell(level,vch,DAM_LIGHTNING))
				tdam *= .75;
			spell_damage(ch,vch,tdam,sn,DAM_LIGHTNING,true);
			arc_damage(ch,vch,dam,level);
			break;
		}
		ch->send("Psh\n\r");
	}
}

void spell_tidal_wave(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *was_in,*now_in;
	int dam,tdam,door;
	bool found;

    act("$n raises $s hand high, pulling a tidal wave from the ground!",ch,NULL,NULL,TO_ROOM);
    act("You pull your hand high in the air, pulling a tidal wave from the ground!",ch,NULL,NULL,TO_CHAR);

	dam = dice(level, ch->getslvl(sn) + get_curr_stat(ch,STAT_INT));

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch) || !canPK(ch,vch,false))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_WATER))
			tdam *= .75;
		spell_damage(ch,vch,tdam,sn,DAM_WATER,true);
		if(!saves_spell(level,vch,DAM_WATER)){
			found = false;
			was_in = vch->in_room;
			for(int tries = 0; tries < 12; tries++ ){
				if ((door = get_random_door(vch->in_room)) == -1)
					break;
				if (IS_NPC(vch) && IS_SET(was_in->exit[door]->u1.to_room->room_flags,ROOM_NO_MOB))
					continue;
				move_char(vch,door,false,false);
				if ((now_in = vch->in_room) == was_in)
					continue;
				vch->in_room = was_in;
				act("You are carried $T by the wave!",vch,NULL,dir_name[door],TO_CHAR);
				act("$n is carried $T by the wave!",vch,NULL,dir_name[door],TO_ROOM);
				vch->in_room = now_in;
				stop_fighting(vch,true);
				found = true;
				break;
			}
			if(!found){
				act("$n is thrown into a wall!",vch,NULL,NULL,TO_ROOM);
				vch->send("You are thrown into a wall!\n\r");
			}
		}
	}
}

void spell_hellfire(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo,*vch,*vch_next;
	AFFECT_DATA af;
	int tdam,tsn = skill_lookup("burn");
	static const sh_int dam_each[] = {
		0,
		2,		4,		6,		8,		10,		12,		14,		16,		18,		20,
		22,		24,		26,		28,		30,		35,		40,		45,		50,		55,
		60,		65,		70,		75,		80,		82,		84,		86,		88,		90,
		92,		94,		96,		98,		100,	102,	104,	106,	108,	110,
		112,	114,	116,	118,	120,	122,	124,	126,	128,	130,
		132,	134,	136,	138,	140,	142,	144,	146,	148,	150,
		152,	154,	156,	158,	160,	162,	164,	166,	168,	170,
		172,	174,	176,	178,	180,	182,	184,	186,	188,	190,
		192,	194,	196,	198,	200,	202,	204,	206,	208,	210,
		212,	214,	216,	218,	220,	222,	224,	226,	228,	230,
		232,	234,	236,	238,	240,	242,	244,	246,	248,	250,
		252,	254,	256,	258,	260,	262,	264,	266,	268,	270
	};
	int dam;

	level	= UMIN(level,sizeof(dam_each)/sizeof(dam_each[0]) - 1);
	level	= UMAX(0,level);
	dam		= number_range(dam_each[level] * .5,dam_each[level] * .75) * ch->getslvl(sn);

	if(saves_spell(level,victim,DAM_FIRE))
		dam /= 2;

	spell_damage(ch,victim,dam,sn,DAM_FIRE,true);

	if(ch->getslvl(sn) == 5 && number_percent() < 50){
		act("Your hellfire engulfs $N in flames!",ch,victim,NULL,TO_CHAR);
		act("$n's hellfire engulfs you in flames!",ch,victim,NULL,TO_VICT);
		act("$n's hellfire engulfs $N in flames!",ch,victim,NULL,TO_NOTVICT);
		affect_set(victim,TO_AFFECTS,sn,ch->level,slvl,2,APPLY_AGI,-5,AF_IMMOLATION);
	}

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch))
			continue;
		tdam = dam * .75;
		if(saves_spell(level,victim,DAM_FIRE))
			dam /= 2;

		spell_damage(ch,vch,tdam,tsn,DAM_FIRE,true);
	}
}

void spell_frost_field(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	AFFECT_DATA af;
	int dam,tdam;

	if(ch->in_room->israf(RAF_FROSTFIELD)){
		ch->send("A field of frost already engulfs this room.\n\r");
		return;
	}
	act("With a wave of your hand, you engulf the room in frost, icy explosions blasting randomly!",ch,NULL,NULL,TO_CHAR);
	act("$n waves $s hand and the room is engulfed in frost, icy explosions blast randomly!",ch,NULL,NULL,TO_ROOM);

	affect_set(ch,ch->in_room,TO_AFFECTS,sn,level,slvl,slvl,APPLY_NONE,slvl,RAF_FROSTFIELD);

	dam = dice(ch->getslvl(sn) * 50,ch->getslvl(sn) * 10) + get_curr_stat(ch,STAT_INT);

	for(int i = ch->getslvl(sn) * 2;i > 0;i--){
		for(int tries = 0;tries < 5;tries++){
			if(!(vch = get_random_char(NULL,NULL,ch->in_room)) || !canPK(ch,vch,false))
				continue;
			tdam = dam;
			if(saves_spell(level,vch,DAM_COLD))
				tdam *= .75;
			spell_damage(ch,vch,tdam,sn,DAM_COLD,true);
			break;
		}
		ch->send("Psh\n\r");
	}
}

void spell_fissure(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo,*vch;
	ROOM_INDEX_DATA *next_room;
	int dam = dice(ch->getslvl(sn) * get_curr_stat(ch,STAT_INT),ch->getslvl(sn)),door;

	if(!saves_spell(level,victim,DAM_EARTH))
		dam *= 2;

	spell_damage(ch,victim,dam,sn,DAM_EARTH,true);

	//if((door = find_random_exit(ch->in_room)) == -1)
	//	return;

//	act("The cracking ground shoots off to the $T!",ch,NULL,dir_name[door],TO_ROOM);
//	act("The cracking ground shoots off to the $T!",ch,NULL,dir_name[door],TO_CHAR);

//	act("The ground splits in from the $T and strikes $n!",vch,NULL,dir_name[door],TO_ROOM);
//	act("The ground splits in from the $T and strikes you!",vch,NULL,rdir_name[rev_dir[door]],TO_CHAR);

//	if(saves_spell(level,vch,DAM_EARTH))
//		dam /= 2;

//	spell_damage(ch,vch,dam,sn,DAM_EARTH,true);
}

void spell_squall(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *was_in,*now_in;
	int dam,tdam,door;
	bool found;

    act("$n creates a furious squall!",ch,NULL,NULL,TO_ROOM);
    act("You create a furious squall!",ch,NULL,NULL,TO_CHAR);

	dam = dice(level,ch->getslvl(sn) + get_curr_stat(ch,STAT_INT));

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch) || !canPK(ch,vch,false))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_WIND))
			tdam *= .75;
		spell_damage(ch,vch,tdam,sn,DAM_WIND,true);
	}
}

void spell_tornado(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *was_in,*now_in;
	int dam,tdam,door;
	bool found;

    act("$n summons forth a wild tornado!",ch,NULL,NULL,TO_ROOM);
    act("You summon forth a wild tornado!",ch,NULL,NULL,TO_CHAR);

	dam = dice(level, ch->getslvl(sn) + get_curr_stat(ch,STAT_INT));

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch) || !canPK(ch,vch,false))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_WIND))
			tdam *= .75;
		spell_damage(ch,vch,tdam,sn,DAM_WIND,true);
		if(!saves_spell(level,vch,DAM_WIND)){
			found = false;
			was_in = vch->in_room;
			for(int tries = 0; tries < 12; tries++ ){
				if ((door = get_random_door(vch->in_room)) == -1)
					break;
				if (IS_NPC(vch) && IS_SET(was_in->exit[door]->u1.to_room->room_flags,ROOM_NO_MOB))
					continue;
				move_char(vch,door,false,false);
				if ((now_in = vch->in_room) == was_in)
					continue;
				vch->in_room = was_in;
				act("You are hurled $T by the tornado!",vch,NULL,dir_name[door],TO_CHAR);
				act("$n is hurled $T by the tornado!",vch,NULL,dir_name[door],TO_ROOM);
				vch->in_room = now_in;
				stop_fighting(vch,true);
				found = true;
				break;
			}
			if(!found){
				act("$n is thrown into a wall!",vch,NULL,NULL,TO_ROOM);
				vch->send("You are thrown into a wall!\n\r");
			}
		}
	}
}

void spell_alarm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	ROOM_INDEX_DATA *room;
	AFFECT_DATA af,caf,*taf;
	int tduration = ch->getslvl(sn);

	if(ch->in_room->israf(RAF_ALARM)){
		ch->send("There is already an alarm here.\n\r");
		return;
	}
	if((taf = affect_find(ch->affected,skill_lookup("alarm")))){
		if(!(room = get_room_index(taf->modifier))){
			affect_remove(ch,taf);
			return;
		}
		tduration = ch->getslvl(sn) - taf->duration;
		ch->send("You already have an alarm out there. Removing it.\n\r");
		affect_remove_room(room,taf);
	}
	act("You wave your hand and create an alarm that quickly fades into invisibility.",ch,NULL,NULL,TO_CHAR);
	act("$n waves $s hand and creates a small alarm that quickly fades into invisibility.",ch,NULL,NULL,TO_ROOM);

	affect_set(ch,ch->in_room,TO_AFFECTS,sn,level,slvl,slvl,APPLY_NONE,0,RAF_ALARM);

	affect_join(ch,TO_AFFECTS,sn,level,slvl,slvl,APPLY_NONE,ch->in_room->vnum,0);
}

void check_alarm(CHAR_DATA *victim){
	AFFECT_DATA *alarm = affect_find(victim->in_room->affected,skill_lookup("alarm"));
	if(!victim->in_room->israf(RAF_ALARM))
		return;
	if(!alarm){
		victim->in_room->remraf(RAF_ALARM);
		return;
	}
	if(!alarm->parent){
		affect_remove_room(victim->in_room,alarm);
		return;
	}
	act("$N has tripped your alarm.",alarm->parent,NULL,victim,TO_CHAR);
}

void spell_energy_shield(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;

	if(victim->isaff(AF_ESHIELD)){
		if (victim == ch)
			send_to_char("You are already in sanctuary.\n\r",ch);
		else
			act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
		return;
	}
	if(victim != ch && ch->getslvl(sn) != 5){
		ch->send("You cannot cast this on others.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,slvl*2,APPLY_NONE,slvl,AF_ESHIELD);
	act("$n is surrounded by a shimmering energy shield.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You are surrounded by a shimmering energy shield.\n\r",victim );
}

void imbue_element(CHAR_DATA *ch,int res,int vuln,int duration,int level,int slvl){
	AFFECT_DATA af;

	affect_set(ch,TO_RESIST,gsn_eimbue,level,slvl,duration,APPLY_NONE,0,res);

	affect_set(ch,TO_VULN,gsn_eimbue,level,slvl,duration,APPLY_NONE,0,vuln);
}

void spell_imbue_earth(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;
	if(is_affected(ch,gsn_eimbue)){
		if(victim == ch)
			ch->send("You are already imbued.\n\r");
		else
			ch->send("They are already imbued.\n\r");
		return;
	}
	if(!saves_spell(level,victim,DAM_ENERGY))
		imbue_element(victim,RS_LIGHTNING,RS_WIND,ch->getslvl(sn),level,slvl);
	else
		ch->send("You failed.\n\r");
}

void spell_imbue_fire(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;
	if(is_affected(ch,gsn_eimbue)){
		if(victim == ch)
			ch->send("You are already imbued.\n\r");
		else
			ch->send("They are already imbued.\n\r");
		return;
	}
	if(!saves_spell(level,victim,DAM_ENERGY))
		imbue_element(victim,RS_COLD,RS_WATER,ch->getslvl(sn),level,slvl);
	else
		ch->send("You failed.\n\r");
}

void spell_imbue_water(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;
	if(is_affected(ch,gsn_eimbue)){
		if(victim == ch)
			ch->send("You are already imbued.\n\r");
		else
			ch->send("They are already imbued.\n\r");
		return;
	}
	if(!saves_spell(level,victim,DAM_ENERGY))
		imbue_element(victim,RS_FIRE,RS_LIGHTNING,ch->getslvl(sn),level,slvl);
	else
		ch->send("You failed.\n\r");
}

void spell_imbue_wind(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;
	if(is_affected(ch,gsn_eimbue)){
		if(victim == ch)
			ch->send("You are already imbued.\n\r");
		else
			ch->send("They are already imbued.\n\r");
		return;
	}
	if(!saves_spell(level,victim,DAM_ENERGY))
		imbue_element(victim,RS_EARTH,RS_COLD,ch->getslvl(sn),level,slvl);
	else
		ch->send("You failed.\n\r");
}

void spell_imbue_ice(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;
	if(is_affected(ch,gsn_eimbue)){
		if(victim == ch)
			ch->send("You are already imbued.\n\r");
		else
			ch->send("They are already imbued.\n\r");
		return;
	}
	if(!saves_spell(level,victim,DAM_ENERGY))
		imbue_element(victim,RS_WIND,RS_FIRE,ch->getslvl(sn),level,slvl);
	else
		ch->send("You failed.\n\r");
}

void spell_imbue_electric(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	AFFECT_DATA af;
	if(is_affected(ch,gsn_eimbue)){
		if(victim == ch)
			ch->send("You are already imbued.\n\r");
		else
			ch->send("They are already imbued.\n\r");
		return;
	}
	if(!saves_spell(level,victim,DAM_ENERGY))
		imbue_element(victim,RS_WATER,RS_EARTH,ch->getslvl(sn),level,slvl);
	else
		ch->send("You failed.\n\r");
}

void spell_mystic_wall(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	ROOM_INDEX_DATA *room;
	AFFECT_DATA af;
	int door = target;

	if(ch->in_room->israf(RAF_MWALL)){
		ch->send("There is already a wall here.\n\r");
		return;
	}
	act("You pull a wall of sizzling magic $T.",ch,NULL,dir_name[door],TO_CHAR);
	act("$n pulls a wall of sizzling magic $T.",ch,NULL,dir_name[door],TO_ROOM);

	affect_set(ch,ch->in_room,TO_AFFECTS,sn,level,slvl,slvl*2,APPLY_NONE,door,RAF_MWALL);
}

void check_mystic_wall(CHAR_DATA *ch,int from_door){
	AFFECT_DATA *af = affect_find(ch->in_room->affected,skill_lookup("alarm"));
	if(!ch->in_room->israf(RAF_MWALL))
		return;
	if(!af){
		ch->in_room->remraf(RAF_MWALL);
		return;
	}
	if(!af->parent){
		affect_remove_room(ch->in_room,af);
		return;
	}
	if(af->modifier != from_door)
		return;
	if(number_percent() > 80){
		act("The mystic wall sizzles and pops.",ch,NULL,NULL,TO_ROOM);
		act("The mystic wall sizzles and pops.",ch,NULL,NULL,TO_CHAR);
	}
	if(!canPK(af->parent,ch,false))
		return;
	act("$N is attacked by your mystic wall!",af->parent,NULL,ch,TO_CHAR);
	act("You are attacked by $n's mystic wall!",af->parent,NULL,ch,TO_VICT);
	act("$N is attacked by $n's mystic wall!",af->parent,NULL,ch,TO_NOTVICT);
	spell_damage(af->parent,ch,number_range(af->level * af->duration,af->level),af->type,DAM_ENERGY,true);
}

void spell_blizzard(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int n;

	act("$n calls forth a blizzard of ice shards!",ch,NULL,NULL,TO_ROOM);
	ch->send("You call forth a blizzard of ice shards!\n\r");

	for(n = 10;n>0;n--){
		for (vch = ch->in_room->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if (!is_same_group(vch,ch)){
				if(saves_spell(level,ch,DAM_COLD))
					spell_damage(ch,vch,number_range(level/3,level*.75),sn,DAM_COLD,true);
				else
					spell_damage(ch,vch,number_range(level/2,ch->level),sn,DAM_COLD,true);
			}
		}
	}
}
