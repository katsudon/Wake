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
void affect_set(ROOM_INDEX_DATA *room,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector);
void affect_set(CHAR_DATA *ch,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector);
void heal_them(CHAR_DATA *ch,CHAR_DATA *victim,int sn,int heal);

void check_fuse(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	int chance = get_skill(ch,gsn_fuse)*.5;

	if (chance < 1 || dam < 1 || IS_NPC(ch) || victim->position == POS_DEAD)
		return;

	if (number_percent() < chance){
		act("Your attack fuses $N's mana!",ch,NULL,victim,TO_CHAR);
		act("$n's attack fuses your mana!",ch,NULL,victim,TO_VICT);
		act("$n's attack fuses $N's mana!",ch,NULL,victim,TO_NOTVICT);
		victim->manadamage(dam * .25);
		check_improve(ch,gsn_fuse,true,1);
		return;
	}
	check_improve(ch,gsn_fuse,false,1);
}

void spell_black_hole(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam;
	act("You place your hand to the ground and a swirling vortex of black energy engulfs the area!",ch,NULL,NULL,TO_CHAR);
	act("$n holds $s hand to the ground and a swirling vortex of black energy engulfs the area!",ch,NULL,NULL,TO_ROOM);
	for(vch = ch->in_room->people; vch; vch = vch_next) { 
		vch_next = vch->next_in_room;
		if(vch != ch && get_class(vch) != CLASS_ANTIMAGE){
			dam = (level + get_curr_stat(ch,STAT_RES)) + level * slvl;
			if(saves_spell(level,vch,DAM_OTHER)){
				dam /= 2;
				act("You are momentarily engulfed in blackness and feel your energy sapped!",vch,NULL,NULL,TO_CHAR);
				act("$n glows black for a moment!",vch,NULL,NULL,TO_ROOM);
				vch->manadamage(-1 * dam / 10);//make sure this only does mana, not amp
			}
			spell_damage(ch,vch,dam,sn,DAM_OTHER,true);
		}
	}
}

void spell_reverse_polarity(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = victim->getmana()/10;
    if (!saves_spell(level,victim,DAM_ENERGY))
		dam *= 2;

	act("You release a concentrated burst of anti-mana at $N!",ch,NULL,victim,TO_CHAR);
	act("$n releases a dark red ball of energy at $N!",ch,NULL,victim,TO_VICT);
	act("$n releases a dark ruby energy ball at you!",ch,NULL,victim,TO_NOTVICT);
	victim->manadamage(dam);
	spell_damage(ch,victim,dam,sn,DAM_ENERGY,true);
}

//energy lance

void spell_empyreal_torpedo(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	ROOM_INDEX_DATA *location;
	CHAR_DATA *vch,*vch_next;
	OBJ_DATA *portal = (OBJ_DATA*)vo;
	int tdam,dam;

	if (portal->item_type != ITEM_PORTAL || (IS_SET(portal->value[1],EX_CLOSED) && !IS_TRUSTED(ch,IMMORTAL))){
		send_to_char("You can't seem to find a way in.\n\r",ch);
		return;
	}
	if(IS_SET(portal->value[2],GATE_NOENTER)){
		ch->send("You cannot enter that.\n\r");
		return;
	}
	if (!IS_TRUSTED(ch,IMMORTAL) && !IS_SET(portal->value[2],GATE_NOCURSE) && (ch->isaff(AF_CURSE) || IS_SET(ch->in_room->room_flags,ROOM_NO_RECALL))){
		send_to_char("Something prevents you from leaving...\n\r",ch);
		return;
	}
	if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1)
		location = get_random_room(ch);
	else if (IS_SET(portal->value[2],GATE_BUGGY) && (number_percent() < 5))
		location = get_random_room(ch);
	else
		location = get_room_index(portal->value[3]);

	if (location == NULL || location == ch->in_room || !can_see_room(ch,location) || (room_is_private(location) && !IS_TRUSTED(ch,ADMIN))){
	   act("$p doesn't seem to go anywhere.",ch,portal,NULL,TO_CHAR);
	   return;
	}
	if (IS_NPC(ch) && (ch->isact(AT_AGGRESSIVE) || ch->isact(AT_AGGRESSIVE_MOB)) && IS_SET(location->room_flags,ROOM_LAW)){
		send_to_char("You're too mean to go there...\n\r",ch);
		return;
	}
	if(!location->people){
		ch->send("There is no one on the other side!\n\r");
		return;
	}
	dam = 2 * level + 20 * slvl;//100+20 to 200+100
	dam += get_curr_stat(ch,STAT_STR) / 2;
	if(slvl < 5){
		act("You release an empyreal torpedoe into $p!",ch,portal,NULL,TO_CHAR);
		act("$n releases an empyreal torpedoe into $p!",ch,portal,NULL,TO_ROOM);
		act("A torpedo of energy bursts from a rip in space!",location->people,portal,NULL,TO_CHAR);
		act("A torpedo of energy bursts from a rip in space!",location->people,portal,NULL,TO_ROOM);
		if((vch = get_random_char(NULL,NULL,location))){
			if(!saves_spell(level,vch,DAM_HARM))
				dam *= 1.5;
			damage(ch,vch,dam,DAM_HARM,sn,true);
		}
	}
	else{
		act("You release a spread of empyreal torpedoes into $p!",ch,portal,NULL,TO_CHAR);
		act("$n releases a spread of empyreal torpedoes into $p!",ch,portal,NULL,TO_ROOM);
		act("A spread of energy torpedoes erupt from a rip in space!",location->people,portal,NULL,TO_CHAR);
		act("A spread of energy torpedoes erupt from a rip in space!",location->people,portal,NULL,TO_ROOM);
		for(vch = location->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if(is_same_group(ch,vch) || !canFight(ch,vch,false))
				continue;
			tdam = dam;
			if(saves_spell(level,vch,DAM_HARM))
				dam *= 0.75;
			damage(ch,vch,tdam,DAM_HARM,sn,true);
		}
	}
	if(number_range(1,20) < slvl){
		act("$p shimmers violently and collapses!",ch,portal,NULL,TO_CHAR);
		act("$p shimmers violently and collapses!",ch,portal,NULL,TO_ROOM);
		act("$p shimmers violently and collapses!",location->people,portal,NULL,TO_CHAR);
		act("$p shimmers violently and collapses!",location->people,portal,NULL,TO_ROOM);
		extract_obj(portal);
	}
}

void spell_forcebolt(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[]={
	  0,
		2,   4,   6,   8,   10,		12,		14,		16,		18,		20,
		22,  24,  26,  28,  30,		35,		40,		45,		50,		55,
		60,  65,  70,  75,  80,		82,		84,		86,		88,		90,
		92,  94,  96,  98,	100,	102,	104,	106,	108,	110,
		112, 114, 116, 118, 120,	122,	124,	126,	128,	130,
		132, 134, 136, 138, 140,	142,	144,	146,	148,	150,
		152, 154, 156, 158, 160,	162,	164,	166,	168,	170,
		172, 174, 176, 178, 180,	182,	184,	186,	188,	190,
		192, 194, 196, 198, 200,	202,	204,	206,	208,	210,
		212, 214, 216, 218, 220,	222,	224,	226,	228,	230,
		232, 234, 236, 238, 240,	242,	244,	246,	248,	250,
		252, 254, 256, 258, 260,	262,	264,	266,	268,	270
    };
    int dam;

    level	= UMIN(level,sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0,level);
    dam		= number_range(dam_each[level] / 2,dam_each[level]);
	dam *= .75;
    if (saves_spell(level,victim,DAM_ENERGY))
		dam /= 2;
	if (victim->isaff(AF_SANCTUARY))
		dam *= 2;
    spell_damage(ch,victim,dam,sn,DAM_NONE,true);
    return;
}

void spell_equilibrium(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    AFFECT_DATA af;
	int chp = ch->hit,cmp = ch->getmana();
	int hperc = (chp * 100) / ch->max_hit,mperc = (cmp *100) / ch->getmaxmana();

	printf_to_char(ch,"chp(%d), hperc(%d), cmp(%d), mperc(%d)\n\r",chp,hperc,cmp,mperc);
	if(ch->isaff(AF_WEARY) || is_affected(ch,sn)){
		ch->send("You're too weary to do that!\n\r");
		return;
	}

	if (hperc > mperc){
		act("$n glows with a red aura before $s body seems more energized.",ch,NULL,NULL,TO_ROOM);
		ch->send("You balance the flow of power inside your body, leaving you feeling energized.\n\r");
		hperc -= mperc;
		mperc += hperc;
		ch->hit = ch->max_hit * mperc / 100;
		ch->setmana(ch->getmaxmana() * mperc / 100);
	}
	else if (mperc > hperc){
		act("$n glows with a red aura before $s body's wounds seem to heal a bit.",ch,NULL,NULL,TO_ROOM);
		ch->send("You balance the flow of power inside your body, leaving you feeling healthier.\n\r");
		mperc -= hperc;
		hperc += mperc;
		ch->hit = ch->max_hit * hperc / 100;
		ch->setmana(ch->getmaxmana() * hperc / 100);
	}
	else{
		ch->send("Your body is already at equilibrium.\n\r");
		return;
	}
	affect_set(ch,TO_AFFECTS,sn,level,slvl,24,APPLY_NONE,0,AF_WEARY);
	affect_set(ch,TO_AFFECTS,sn,level,slvl,24,APPLY_WIS,10,AF_WEARY);
}

int nuke_purge(CHAR_DATA *victim,char *cspell,int level){
	AFFECT_DATA *af;
	int tlevel;
	if(!(af = affect_find(victim->affected,skill_lookup(cspell))))
		return 0;
	tlevel = af->level;

	if(check_dispel(level,victim,af->type))
		return tlevel;
	return 0;
}

SPELL(spell_purge){
	AFFECT_DATA *af;
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	int dam = 0,levels = 0,count = 0,tlevel;

	if(is_affected(ch,sn)){
		ch->send("You can't do that yet.\n\r");
		return;
	}
	if((tlevel = nuke_purge(victim,"amplify damage",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"armor",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"blindness",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"bind",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"charm person",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"defensive shield",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"detect invis",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"faerie fire",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"fly",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"friendly aura",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"giant strength",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"haste",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"infravision",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"invisibility",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"overpower",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"phase",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"shield",level)) > 0){
		levels += tlevel;
		count++;
	}
	if((tlevel = nuke_purge(victim,"slow",level)) > 0){
		levels += tlevel;
		count++;
	}
	if(victim == ch){
		act("Your index and middle fingers glow red and you place them to your forehead.",ch,NULL,NULL,TO_CHAR);
		act("$n's index and middle fingers glow red and $e places them to $s forehead.",ch,NULL,NULL,TO_ROOM);
		heal_them(ch,ch,sn,levels * count / 2);
		affect_set(ch,TO_AFFECTS,sn,level,slvl,60,0,0,AF_WEARY);
		return;
	}
	act("Your index and middle fingers glow red and you thrust them at $N!",ch,NULL,victim,TO_CHAR);
	act("$n's index and middle fingers glow red and $e thrusts them at YOU!",ch,NULL,victim,TO_VICT);
	act("$n's index and middle fingers glow red and $e thrusts them at $N!",ch,NULL,victim,TO_NOTVICT);
	damage(ch,victim,levels * count,sn,DAM_HARM,true);
}

SPELL(spell_spell_shield){
	AFFECT_DATA af;

	if (ch->isaff(AF_SPELLSHIELD)){
		ch->send("You are already resistant to magic.\n\r");
		return;
	}
	ch->send("A shield of absorbing antimagic surrounds you.\n\r");
	act("$n is surrounded by a translucent shield.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch,TO_AFFECTS,gsn_spellshield,level,slvl,ch->level / 10,0,level + slvl * 2,AF_SPELLSHIELD);
}

SPELL(spell_dispersion_field){
	AFFECT_DATA *af = new_affect();
	
	if (ch->isaff(AF_DISPERSION_FIELD)){
		ch->send("You are already resistant to magic.\n\r");
		return;
	}
	ch->send("A second skin of dispersing antimagic surrounds you.\n\r");
	act("$n is covered in a second skin of glowing red energy.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch,TO_AFFECTS,sn,level,slvl,slvl * 2,0,level + slvl * 2,AF_DISPERSION_FIELD);
}

SPELL(spell_vernier_red){
	AFFECT_DATA *af = new_affect();
	
	if (ch->isaff(AF_VERNIER_RED)){
		ch->send("You are already more antimagic inert.\n\r");
		return;
	}
	ch->send("Inertia seems to affect your antimagic differently now.\n\r");
	act("$n looks... more red.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch,TO_AFFECTS,sn,level,slvl,slvl * 2,APPLY_AGI,1,AF_VERNIER_RED);
}

SPELL(spell_immaterial_burst){
	AFFECT_DATA *af = new_affect();
	
	if (ch->isaff(AF_IMMATERIAL_BURST)){
		ch->send("You are already bursting with immaterial energy.\n\r");
		return;
	}
	ch->send("You feel like bursting with immaterial energy.\n\r");
	act("$n looks ready to burst with immaterial energy.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch,TO_AFFECTS,sn,level,slvl,slvl * 2,APPLY_STR,1,AF_IMMATERIAL_BURST);
}

SPELL(spell_interrupt){
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim->spelltimer < 1){
		ch->send("They are not casting a spell.\n\r");
		return;
	}
	if (!saves_spell(level,victim,DAM_OTHER)){
		act("You send a surge of mana through $N's body, interrupting $S spell.",ch,NULL,victim,TO_CHAR);
		act("$n sends a surge of mana through $N's body, interrupting $S spell.",ch,NULL,victim,TO_NOTVICT);
		act("$n sends a surge of mana through your body, interrupting your spell.",ch,NULL,victim,TO_VICT);
		break_spell(victim);
		check_improve(victim,victim->spellsn,false,5);
		victim->modmana(-victim->spellcost / 3);
	}
}

SPELL(spell_silence){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(victim == ch){
		ch->send("Har har...\n\r");
		return;
	}
	if(victim->isaff(AF_SILENCE)){
		ch->send("They are already silenced.\n\r");
		return;
	}
	if (saves_spell(level,victim,DAM_CHARM)){
		ch->send("You failed.\n\r");
		return;
	}

	act("You successfully seal $N's voice.",ch,NULL,victim,TO_CHAR);
	victim->send("You feel your throat constrict before becoming numb.\n\r");
	affect_set(victim,TO_AFFECTS,sn,level,slvl,5,APPLY_INT,-2 * slvl,AF_SILENCE);

	affect_set(ch,TO_AFFECTS,sn,level,slvl,5,APPLY_NONE,0,AF_WEARY);
}

SPELL(spell_enfeeble){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(victim == ch){
		ch->send("Har har...\n\r");
		return;
	}
	if(victim->isaff(AF_SLOWCAST)){
		ch->send("They are already cursed.\n\r");
		return;
	}
	if (saves_spell(level,victim,DAM_MENTAL)){
		ch->send("You failed.\n\r");
		return;
	}

	act("$n glows with negative energy.",victim,NULL,NULL,TO_ROOM);
	victim->send("You feel negative energy fill your body.\n\r");
	affect_set(victim,TO_AFFECTS,sn,level,slvl,5,APPLY_WIS,-2 * slvl,AF_SLOWCAST);
}

SPELL(spell_mana_ward){
	if (ch->in_room->israf(RAF_FEEDBACK)){
		ch->send("This room already is filled with mana repulsion.\n\r");
		return;
	}
	act("You flood the room with mana repulsive energy.",ch,NULL,NULL,TO_CHAR);
	act("$n floods the room with a mana repulsive energy.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,level/6,APPLY_NONE,slvl,RAF_FEEDBACK);
}

SPELL(spell_dampening_ward){
	if (ch->in_room->israf(RAF_MAGIC_DAMPEN)){
		ch->send("This room already is filled with mana dampening powers.\n\r");
		return;
	}
	act("You flood the room with mana dampening energy.",ch,NULL,NULL,TO_CHAR);
	act("$n floods the room with a mana dampening energy.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,level/6,APPLY_NONE,slvl,RAF_MAGIC_DAMPEN);
}

SPELL(spell_planar_barrier){
	if (ch->in_room->israf(RAF_PORTALBLOCK)){
		ch->send("This room is already blocking portals.\n\r");
		return;
	}
	act("You create a barrier against portals.",ch,NULL,NULL,TO_CHAR);
	act("$n waves $s hand once and a flash of grey energy fills the room briefly.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,level/6,APPLY_NONE,slvl,RAF_PORTALBLOCK);
}
/*
recovery
*/

SPELL(spell_repulsor_field){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	if(victim->isaff(AF_REPULSOR_FIELD)){
		ch->send("You are already imbued with speed.\n\r");
		return;
	}

	act("$n is surrounded by a silhouette of red energy.",victim,NULL,NULL,TO_ROOM);
	victim->send("You are surrounded by a silhouette of red energy.\n\r");
	affect_set(ch,TO_AFFECTS,sn,level,slvl,slvl,APPLY_END,1,AF_REPULSOR_FIELD);
}

SPELL(spell_ethereal_alacrity){
	if(ch->isaff(AF_ETHEREAL_ALACRITY)){
		ch->send("Slowing yourself back to normal.\n\r");
		affect_strip(ch,sn);
		return;
	}

	act("$n's legs and arms bulge and glow red briefly.",ch,NULL,NULL,TO_ROOM);
	ch->send("Your arms and legs bulge and glow for a moment and you feel faster.\n\r");
	affect_set(ch,TO_AFFECTS,sn,level,slvl,slvl,APPLY_AC,10 * slvl,AF_ETHEREAL_ALACRITY);
}

void break_spell(CHAR_DATA *ch){
	ch->chargevict = NULL;
	ch->chargetime = 0;
	ch->spelltimer = 0;
	ch->spellfailed = 0;
	ch->spellsn = 0;
	ch->spellvo = NULL;
	ch->spelltarget = 0;
	ch->spellvictim = NULL;
	ch->spellcost = 0;
}


