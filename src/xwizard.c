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


void spell_portal(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	ROOM_INDEX_DATA *to_room;
	OBJ_DATA *portal,*stone = (OBJ_DATA*)vo;

	if (!ch->in_room
	||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
	||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)){
		ch->send("You failed.\n\r");
		return;
	}
	if(stone == NULL || stone->item_type != ITEM_WARP_STONE){
		send_to_char("You lack the proper component for this spell.\n\r",ch);
		return;
	}

	if(stone->value[0] < 1){
		ch->send("This stone does not remember anywhere.\n\r");
		return;
	}
	if(!(to_room = get_room_index(stone->value[0]))){
		ch->send("This stone does not remember anywhere worthwhile.\n\r");
		stone->value[0] = 0;
		return;
	}

	if(get_room_index(stone->value[0])->israf(RAF_PORTALBLOCK)){
		ch->send("You can't create a portal to that location.\n\r");
		return;
	}

	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);

	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 1 + slvl;
	portal->value[3] = stone->value[0];
	if(--stone->value[1] < 1){
		act("$p burns away in a flash of magic.",NULL,stone,NULL,TO_ROOM);
		extract_obj(stone);
	}

	obj_to_room(portal,ch->in_room);

	act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
}

void spell_nexus(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	OBJ_DATA *portal,*portal2,*stone = (OBJ_DATA*)vo;
	ROOM_INDEX_DATA *sroom,*croom = ch->in_room;

	if (!croom
	||   IS_SET(croom->room_flags, ROOM_SAFE)
	||   IS_SET(croom->room_flags, ROOM_PRIVATE)
	||   IS_SET(croom->room_flags, ROOM_SOLITARY)
	||   IS_SET(croom->room_flags, ROOM_NO_RECALL)){
		ch->send("You failed.\n\r");
		return;
	}

	if (!stone || stone->item_type != ITEM_WARP_STONE){
		ch->send("You lack the proper component for this spell.\n\r");
		return;
	}

	if(stone->value[0] < 1){
		ch->send("This stone does not remember anywhere.\n\r");
		return;
	}

	act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
	if(stone->value[0] < 1){
		ch->send("This stone does not remember anywhere.\n\r");
		return;
	}

	sroom = get_room_index(stone->value[0]);
	if(!sroom){
		ch->send("You failed.\n\r");
		return;
	}

	if(sroom->israf(RAF_PORTALBLOCK)){
		ch->send("You can't create a portal to that location.\n\r");
		return;
	}

	/* portal one */
	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 1 + slvl;
	portal->value[3] = stone->value[0];

	obj_to_room(portal,croom);

	act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.",ch,portal,NULL,TO_CHAR);

	/* no second portal if rooms are the same */
	if (sroom == croom)
		return;

	/* portal two */
	portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 1 + slvl;
	portal->value[3] = croom->vnum;
	extract_obj(stone);

	obj_to_room(portal,sroom);

	if (sroom->people){
		act("$p rises up from the ground.",sroom->people,portal,NULL,TO_ROOM);
		act("$p rises up from the ground.",sroom->people,portal,NULL,TO_CHAR);
	}
}

void spell_imprint(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *stone = (OBJ_DATA*)vo;
	ROOM_INDEX_DATA *to_room;

	if (!ch->in_room
	||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
	||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)){
		ch->send("You failed.\n\r");
		return;
	}

	if(!stone){
		ch->send("You lack the proper component for this spell.\n\r");
		return;
	}
	if(stone->item_type != ITEM_WARP_STONE){
		act("$p is not a warp stone.",ch,stone,NULL,TO_CHAR);
		return;
	}

	act("$p glows a soft blue as you imprint this location on it.",ch,stone,NULL,TO_CHAR);
	act("$p glows a soft blue in $n's hand.",ch,stone,NULL,TO_ROOM);
	stone->value[0] = ch->in_room->vnum;
	stone->value[1] = slvl;
}

void spell_armor(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (is_affected(victim,sn)){
		if (victim == ch)
			send_to_char("You are already armored.\n\r",ch);
		else
			act("$N is already armored.",ch,NULL,victim,TO_CHAR);
		return;
    }
	affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_AC,-50*slvl,0);
    send_to_char("You feel someone protecting you.\n\r",victim);
    if (ch != victim)
		act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
}

void remove_charmy(CHAR_DATA *ch,CHAR_DATA *charmy){
	if(charmy->prev_charmy)
		charmy->prev_charmy->next_charmy = charmy->next_charmy;
	if(charmy->next_charmy)
		charmy->next_charmy->prev_charmy = charmy->prev_charmy;
	ch->charmy_levels -= charmy->level;
	ch->charmy_count--;
}
//check_dispel(level,victim,skill_lookup("charm person"))
bool check_charm_purge(CHAR_DATA *ch,CHAR_DATA *victim,int level){
	if(!victim->isaff(AF_CHARM))
		return false;
	if(saves_spell(level,victim,DAM_CHARM))
		return false;
    act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
	remove_charmy(ch,victim);
	affect_strip(victim,gsn_charm_person);
	stop_follower(victim);
	return true;
}

void add_charmy(CHAR_DATA *ch,CHAR_DATA *charmy){
	if(ch->charmy_first)
		ch->charmy_first->prev_charmy = charmy;
	charmy->next_charmy           = ch->charmy_first;
	ch->charmy_first              = charmy;
	ch->charmy_levels            += charmy->level;
	ch->charmy_count++;

	if (charmy->master)
		stop_follower(charmy);

	add_follower(charmy,ch,false);
	group_char(ch,charmy);
}

void spell_charm_person(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_safe(ch,victim))
		return;

	if (victim == ch){
		ch->send("You like yourself even better!\n\r");
		return;
	}

	if (!IS_NPC(victim)){
		ch->send("Not on PCs!\n\r");
		return;
	}
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Charm %d/%d %d/%d\n\r",ch->charmy_levels,ch->level * ch->getslvl(sn),ch->charmy_count,ch->getslvl(sn));

	if(ch->charmy_levels + victim->level >= ch->level*ch->getslvl(sn) || ch->charmy_count >= ch->getslvl(sn)){
		ch->send("You fail.\n\r");
		return;
	}

	if (victim->isaff(AF_CHARM) || ch->isaff(AF_CHARM) || victim->res[RS_CHARM] == 0 || saves_spell(level,victim,DAM_CHARM))
		return;

	if (IS_SET(victim->in_room->room_flags,ROOM_LAW)){
		ch->send("The mayor does not allow charming in the city limits.\n\r");
		return;
	}

	add_charmy(ch,victim);

	affect_set(victim,TO_AFFECTS,gsn_charm_person,level,slvl,number_fuzzy(level/4),APPLY_NONE,0,AF_CHARM);

	act("Isn't $n just so nice?",ch,NULL,victim,TO_VICT);
	act("$n charms $N into obedience?",ch,NULL,victim,TO_NOTVICT);
	if (ch != victim)
		act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
}

int enchant_armor(CHAR_DATA *ch,OBJ_DATA *obj,int level){
	AFFECT_DATA *paf;
	int result,fail,ac_bonus,added;

	/* this means they have no bonus */
	ac_bonus = 0;
	fail = 25;	/* base 25% chance of failure */

	/* find the bonuses */

	if (!obj->enchanted)
		for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next ){
			if (paf->location == APPLY_AC){
				ac_bonus = paf->modifier;
				fail += 5 * (ac_bonus * ac_bonus);
			}
			else  /* things get a little harder */
				fail += 20;
		}
	for ( paf = obj->affected; paf != NULL; paf = paf->next ){
		if ( paf->location == APPLY_AC ){
			ac_bonus = paf->modifier;
			fail += 5 * (ac_bonus * ac_bonus);
		}
		else /* things get a little harder */
			fail += 20;
	}
	/* apply other modifiers */
	fail -= level;
	if (IS_OBJ_STAT(obj,ITM_GLOW))
		fail -= 5;
	return URANGE(5,fail,85);
}
int enchant_weapon(CHAR_DATA *ch,OBJ_DATA *obj,int level){
	AFFECT_DATA *paf;
	int result,fail,hit_bonus,dam_bonus,added;

	/* this means they have no bonus */
	hit_bonus = 0;
	dam_bonus = 0;
	fail = 25;	/* base 25% chance of failure */
	/* find the bonuses */
	if (!obj->enchanted)
		for(paf = obj->pIndexData->affected;paf;paf = paf->next){
			if(paf->location == APPLY_HITROLL){
	    		hit_bonus = paf->modifier;
				fail += 2 * (hit_bonus * hit_bonus);
 			}
			else if(paf->location == APPLY_DAMROLL){
	    		dam_bonus = paf->modifier;
	    		fail += 2 * (dam_bonus * dam_bonus);
			}
			else  /* things get a little harder */
	    		fail += 25;
		}
	for(paf = obj->affected;paf;paf = paf->next){
		if(paf->location == APPLY_HITROLL){
			hit_bonus = paf->modifier;
			fail += 2 * (hit_bonus * hit_bonus);
		}
		else if(paf->location == APPLY_DAMROLL){
			dam_bonus = paf->modifier;
			fail += 2 * (dam_bonus * dam_bonus);
		}
		else /* things get a little harder */
			fail += 25;
	}
	/* apply other modifiers */
	fail -= 3 * level/2;
	if(IS_OBJ_STAT(obj,ITM_GLOW))
		fail -= 5;
	return URANGE(5,fail,95);
}
void spell_enchant_object(int sn,int level,CHAR_DATA *ch, void *vo,int target,int slvl){
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;
	int result,fail,hit_bonus,dam_bonus,added,wtype = 0,atype = 0;
	bool hit_found = false, dam_found = false, ac_found = false;

	if (obj->wear_loc != -1){
		send_to_char("The item must be carried to be enchanted.\n\r",ch);
		return;
	}
	if (obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_ARMOR){
		send_to_char("That can not be enchanted.\n\r",ch);
		return;
	}
	if(obj->item_type == ITEM_WEAPON)
		wtype = enchant_weapon(ch,obj,level);
	else if(obj->item_type == ITEM_ARMOR)
		atype = enchant_armor(ch,obj,level);

	char bcolor[30];
	sprintf(bcolor,"%s",wtype == 0 ? "gold" : "blue");
	result = number_percent();
	fail = wtype == 0 ? atype : wtype;
	/* the moment of truth */
	if (result < (fail / 5) && ch->level < DADMIN){/* item destroyed */
		act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
		act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
		extract_obj(obj);
		return;
	}
	else if (result < (fail / 3) && ch->level < DADMIN){/* item disenchanted */
		AFFECT_DATA *paf_next;

		act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
		act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
		obj->enchanted = true;

		/* remove all affects */
		for (paf = obj->affected; paf != NULL; paf = paf_next){
			paf_next = paf->next;
			free_affect(paf);
		}
		obj->affected = NULL;

		/* clear all flags */
		return;
	}
	else if (result <= fail){/* failed, no bad result */
		send_to_char("Nothing seemed to happen.\n\r",ch);
		return;
	}
	else if (result <= (90 - level/5)){/* success! */
		act("$p shimmers with a $T aura.",NULL,obj,bcolor,TO_ROOM);
		SET_BIT(obj->extra_flags,ITM_MAGIC);
		added = -1;
	}
	else{/* exceptional enchant */
		act("$p glows a brillant $T!",ch,obj,bcolor,TO_CHAR);
		SET_BIT(obj->extra_flags,ITM_MAGIC);
		SET_BIT(obj->extra_flags,ITM_GLOW);
		added = -2;
	}
	/* okay, move all the old flags into new vectors if we have to */
	if (!obj->enchanted){
		AFFECT_DATA *af_new;
		obj->enchanted = true;

		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next){
			af_new = new_affect();

			af_new->next = obj->affected;
			obj->affected = af_new;

			af_new->where	= paf->where;
			af_new->type 	= UMAX(0,paf->type);
			af_new->level	= paf->level;
			af_new->duration	= paf->duration;
			af_new->location	= paf->location;
			af_new->modifier	= paf->modifier;
			af_new->bitvector	= paf->bitvector;
		}
	}

	/* now add the enchantments */
	if (obj->level < LEVEL_HERO)
		obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

	if(wtype > 0){
		for(paf = obj->pIndexData->affected;paf;paf = paf->next){
			if(paf->location == APPLY_DAMROLL)
				dam_found = true;
		}
		if (dam_found){
			for(paf = obj->affected;paf;paf = paf->next){
				if(paf->location == APPLY_DAMROLL){
					paf->type = sn;
					paf->modifier += added;
					paf->level = UMAX(paf->level,level);
					if (paf->modifier > 4)
						SET_BIT(obj->extra_flags,ITM_HUM);
				}
			}
		}
		else{/* add a new affect */
			paf = new_affect();

			paf->where	= TO_OBJECT;
			paf->type	= sn;
			paf->level	= level;
			paf->duration	= -1;
			paf->location	= APPLY_DAMROLL;
			paf->modifier	=  added;
			paf->bitvector  = 0;
			paf->next	= obj->affected;
			obj->affected	= paf;
		}

		for(paf = obj->pIndexData->affected;paf;paf = paf->next){
			if(paf->location == APPLY_HITROLL)
				hit_found = true;
		}
		if (hit_found){
			for (paf = obj->affected;paf;paf = paf->next){
				if (paf->location == APPLY_HITROLL){
					paf->type = sn;
					paf->modifier += added;
					paf->level = UMAX(paf->level,level);
					if (paf->modifier > 4)
						SET_BIT(obj->extra_flags,ITM_HUM);
				}
			}
		}
		else{/* add a new affect */
			paf = new_affect();

			paf->type       = sn;
			paf->level      = level;
			paf->duration   = -1;
			paf->location   = APPLY_HITROLL;
			paf->modifier   =  added;
			paf->bitvector  = 0;
			paf->next       = obj->affected;
			obj->affected   = paf;
		}
	}
	else if(atype > 0){
		for(paf = obj->affected;paf;paf = paf->next){
			if(paf->location == APPLY_AC)
				ac_found = true;
		}
		if(ac_found){
			for(paf = obj->affected;paf; paf = paf->next){
				if ( paf->location == APPLY_AC){
					paf->type = sn;
					paf->modifier += added;
					paf->level = UMAX(paf->level,level);
				}
			}
		}
		else{/* add a new affect */
 			paf = new_affect();

			paf->where	= TO_OBJECT;
			paf->type	= sn;
			paf->level	= level;
			paf->duration	= -1;
			paf->location	= APPLY_AC;
			paf->modifier	=  added;
			paf->bitvector  = 0;
			paf->next	= obj->affected;
			obj->affected	= paf;
		}
	}
	else
		ch->send("Something fartsy happened...\n\r");
}

void spell_floating_disc(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *disc, *floating;

	floating = get_eq_char(ch,WEAR_FLOAT);
	if (floating != NULL && IS_OBJ_STAT(floating,ITM_NOREMOVE)){
		act("You can't remove $p.",ch,floating,NULL,TO_CHAR);
		return;
	}

	disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
	disc->value[0]	= ch->level * 10; /* 10 pounds per level capacity */
	disc->value[3]	= ch->level * 5; /* 5 pounds per level max per item */
	disc->timer		= ch->level * 2 - number_range(0,level / 2); 

	act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You create a floating disc.\n\r",ch);
	obj_to_char(disc,ch);
	wear_obj(ch,disc,true,true);
}
 
void spell_gate(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	bool gate_pet;

	if(victim == ch){
		ch->send("You are already you.\n\r");
		return;
	}
	if(ch->isplr(PL_ARENA) || victim->isplr(PL_ARENA)){
		ch->send("You can't gate to or from an arena.\n\r");
		return;
	}

	if(IS_SET(victim->in_room->room_flags,ROOM_SAFE)
	|| IS_SET(victim->in_room->room_flags,ROOM_PRIVATE)
	|| IS_SET(victim->in_room->room_flags,ROOM_SOLITARY)
	|| IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL)){
		ch->send("They can't be gated to.\n\r");
		return;
	}
	if(IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)){
		ch->send("You can't gate out of this room.\n\r");
		return;
	}
	if (!victim->in_room 
	|| (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
	|| (IS_NPC(victim) && victim->res[RS_SUMMON] == 0)
	|| number_range(0,10) > slvl * 2){//Nash add savesspell
		send_to_char("You failed.\n\r",ch);
		return;
	}
	if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
		gate_pet = true;
	else
		gate_pet = false;

	act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You step through a gate and vanish.\n\r",ch);
	char_from_room(ch);
	char_to_room(ch,victim->in_room);

	act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
	do_function(ch,&do_look,"auto");

	if(gate_pet){
		act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
		send_to_char("You step through a gate and vanish.\n\r",ch->pet);
		char_from_room(ch->pet);
		char_to_room(ch->pet,victim->in_room);
		act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
		do_function(ch->pet,&do_look,"auto");
	}
	if(MOUNTED(ch)){
		act("$n steps through a gate and vanishes.",MOUNTED(ch),NULL,NULL,TO_ROOM);
		send_to_char("You step through a gate and vanish.\n\r",MOUNTED(ch));
		char_from_room(MOUNTED(ch));
		char_to_room(MOUNTED(ch),victim->in_room);
		act("$n has arrived through a gate.",MOUNTED(ch),NULL,NULL,TO_ROOM);
		do_look(MOUNTED(ch),"auto");
	}
}

void spell_haste(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
 
    if (victim->isaff(AF_SLOW))
    {
		if (!check_dispel(level,victim,skill_lookup("slow")))
		{
			if (victim != ch)
				send_to_char("Spell failed.\n\r",ch);
			send_to_char("You feel momentarily faster.\n\r",victim);
			return;
		}
        act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
        return;
    }

    if ( is_affected(victim,sn) || victim->isaff(AF_HASTE) || victim->isoff(OF_FAST))
    {
		if (victim == ch)
			send_to_char("You can't move any faster!\n\r",ch);
		else
			act("$N is already moving as fast as $E can.",ch,NULL,victim,TO_CHAR);
		return;
    }

	affect_set(victim,TO_AFFECTS,sn,level,slvl,victim == ch ? level/2 : level/4,APPLY_AGI,1 + (level >= 18) + (level >= 25) + (level >= 32),AF_HASTE);
    send_to_char("You feel yourself moving more quickly.\n\r",victim);
    act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
    if ( ch != victim )
        send_to_char("Ok.\n\r",ch);
    return;
}

void spell_invis(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    AFFECT_DATA af;

    /* object invisibility */
    if (target == TARGET_OBJ)
    {
		obj = (OBJ_DATA *) vo;	

		if (IS_OBJ_STAT(obj,ITM_INVIS))
		{
			act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
			return;
		}

		affect_set(obj,TO_OBJECT,sn,level,slvl,level+12,APPLY_NONE,0,ITM_INVIS);

		act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
		return;
    }

    /* character invisibility */
    victim = (CHAR_DATA *) vo;

    if ( victim->isaff(AF_INVISIBLE) )
		return;

    act("$n fades out of existence.",victim,NULL,NULL,TO_ROOM);

	affect_set(victim,TO_AFFECTS,gsn_invis,level,slvl,level+12,APPLY_NONE,slvl,AF_INVISIBLE);
    send_to_char("You fade out of existence.\n\r",victim);
}

void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target,int slvl){
	char buf[MIL];
	BUFFER *buffer;
	OBJ_DATA *obj,*in_obj;
	bool found;
	int number = 0,max_found;

	found = false;
	number = 0;
	max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

	buffer = new_buf();

	int count = 0;
	for (obj = object_list; obj != NULL; obj = obj->next ){
		if (!can_see_obj(ch,obj) || !is_name(target_name,obj->name) || IS_OBJ_STAT(obj,ITM_NOLOCATE) || number_percent() > ch->getslvl(sn) * 20 || ch->level + 20 < obj->level)
			continue;

		found = true;
		number++;

		for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
			;

		if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by))
			sprintf(buf,"one is carried by %s\n\r",PERS(in_obj->carried_by,ch));
		else{
			if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
				sprintf(buf,"one is in %s [Room %d]\n\r",in_obj->in_room->name,in_obj->in_room->vnum);
			else 
	    		sprintf(buf,"one is in %s\n\r",in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name);
		}

		buf[0] = UPPER(buf[0]);
		add_buf(buffer,buf);

		if (number >= max_found)
			break;
	}

	if ( !found )
		send_to_char("Nothing like that in heaven or earth.\n\r",ch);
	else
		page_to_char(buf_string(buffer),ch);

	free_buf(buffer);
}

void spell_mass_invis(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
		if ( !is_same_group(gch,ch) || gch->isaff(AF_INVISIBLE) )
			continue;
		act("$n slowly fades out of existence.",gch,NULL,NULL,TO_ROOM);
		send_to_char("You slowly fade out of existence.\n\r",gch);

		affect_set(gch,TO_AFFECTS,gsn_invis,level/2,slvl,24,APPLY_NONE,slvl,AF_INVISIBLE);
    }
    send_to_char("Ok.\n\r",ch);
    return;
}

void spell_slow(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || victim->isaff(AF_SLOW)){
		if (victim == ch)
			send_to_char("You can't move any slower!\n\r",ch);
		else
			act("$N can't get any slower than that.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (saves_spell(level,victim,DAM_OTHER) || victim->res[RS_MAGIC] == 0){
		if (victim != ch)
			send_to_char("Nothing seemed to happen.\n\r",ch);
		send_to_char("You feel momentarily lethargic.\n\r",victim);
		return;
	}

	if (victim->isaff(AF_HASTE)){
		if (!check_dispel(level,victim,skill_lookup("haste"))){
			if (victim != ch)
				send_to_char("Spell failed.\n\r",ch);
			send_to_char("You feel momentarily slower.\n\r",victim);
			return;
		}

		act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level/2,APPLY_AGI,-1 - (level >= 18) - (level >= 25) - (level >= 32),AF_SLOW);
	send_to_char("You feel yourself slowing d o  w   n...\n\r",victim);
	act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
}

void spell_stone_skin(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (is_affected(ch,sn)){
		if (victim == ch)
			send_to_char("Your skin is already as hard as a rock.\n\r",ch);
		else
			act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_AC,-1 * UMAX(1,ch->getslvl(sn)*75),0);
    act("$n's skin turns to stone.",victim,NULL,NULL,TO_ROOM);
    send_to_char("Your skin turns to stone.\n\r",victim);
}

void spell_summon(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim;

	if(!(victim = get_char_world(ch,target_name))){
		ch->send("You cannot find them.\n\r");
		return;
	}
	if(victim == ch){
		ch->send("You summon yourself?\n\r");
		return;
	}
	if(IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}
	if (victim->in_room == NULL
	||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
	||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)){
		send_to_char("You failed to summon them.\n\r",ch);
		return;
	}

	if(victim->isplr(PL_NOSUMMON)){
		act("$n disappears suddenly.",victim,NULL,NULL,TO_ROOM);
		char_from_room(victim);
		char_to_room(victim,ch->in_room);
		act("$n arrives suddenly.",victim,NULL,NULL,TO_ROOM);
		act("$n has summoned you!",ch,NULL,victim,TO_VICT);
		do_function(victim,&do_look,"auto");
	}
	else{
		ch->send("PORTAL TIME!\n\r");
	}
}

void spell_teleport(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;

    if(victim->in_room == NULL
		|| IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL)
		|| (victim != ch && victim->res[RS_SUMMON] == 0)
		|| (!IS_NPC(ch) && victim->fighting != NULL)
		|| (victim != ch && (saves_spell(level - 5,victim,DAM_OTHER)))){
		send_to_char("You failed.\n\r",ch);
		return;
    }

    pRoomIndex = get_random_room(victim);

    if (victim != ch)
		send_to_char("You have been teleported!\n\r",victim);

    act("$n vanishes!",victim,NULL,NULL,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,pRoomIndex);
    act("$n slowly fades into existence.",victim,NULL,NULL,TO_ROOM);
    do_function(victim,&do_look,"auto");
    return;
}

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *location;

	if (IS_NPC(victim))
	  return;

	if ( ( location = get_room_index(hometowns[ch->hometown].recall) ) == NULL )
	{	send_to_char("You are completely lost.\n\r",victim);
		return;
	}
	if(ch->isplr(PL_ARENA))
	{
		send_to_char("You're in a duel.\n\r",ch);
		return;
	}

	if (IS_SET(victim->in_room->room_flags,ROOM_NO_RECALL) || victim->isaff(AF_CURSE))
	{
		send_to_char("Spell failed.\n\r",victim);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim,true);

	ch->move /= 2;
	act("$n disappears.",victim,NULL,NULL,TO_ROOM);
	char_from_room(victim);
	char_to_room(victim,location);
	act("$n appears in the room.",victim,NULL,NULL,TO_ROOM);
	do_function(victim,&do_look,"auto");
}

void spell_encumber(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || victim->isaff(AF_SLOW)){
		if (victim == ch)
			ch->send("You can't move any slower!\n\r");
		else
			act("$N seems to be encumbered enough.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (saves_spell(level,victim,DAM_MENTAL) || victim->res[RS_MAGIC] == 0){
		if (victim != ch)
			ch->send("Nothing seemed to happen.\n\r");
		victim->send("You feel momentarily sluggish.\n\r");
		return;
	}

	if (victim->isaff(AF_HASTE)){
		if (!check_dispel(level,victim,skill_lookup("haste"))){
			if (victim != ch)
				ch->send("Spell failed.\n\r");
			victim->send("You feel momentarily slower.\n\r");
			return;
		}
		act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,10,APPLY_AGI,-(get_curr_stat(victim,STAT_AGI) * (100 - (ch->level / 2)) / 100),0);
	victim->send("You feel your muscles become sluggish and unresponsive.\n\r");
	act("$n starts to move like $e's very sluggish.",victim,NULL,NULL,TO_ROOM);
}

void spell_mercurial_mind(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim->isaff(AF_SLOWCAST)){
		if (!check_dispel(level,victim,skill_lookup("enfeeble")))
		{
			if (victim != ch)
				ch->send("Spell failed.\n\r");
			victim->send("You feel momentarily faster.\n\r");
			return;
		}
		act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
		return;
	}

	if (victim->spelltimer < 2){
		ch->send("Their mind is fast enough. Or you're too slow.\n\r");
		return;
	}
	victim->send("You feel your mind move with great speed.\n\r");
	act("$n looks... faster of mind?",victim,NULL,NULL,TO_ROOM);

	victim->spelltimer /= 2;

	if ( ch != victim )
		ch->send("Ok.\n\r");
}

void spell_lethargy(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || victim->isaff(AF_LETHARGY)){
		if (victim == ch)
			ch->send("You can't move any slower!\n\r");
		else
			act("$N seems to be lethargic enough.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (saves_spell(level,victim,DAM_MENTAL) || victim->res[RS_MAGIC] == 0){
		if (victim != ch)
			ch->send("Nothing seemed to happen.\n\r");
		victim->send("You feel momentarily fatter.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,5,APPLY_AGI,-10,AF_LETHARGY);
	victim->send("Your legs feel like they are made of lead.\n\r");
	act("$n starts to move like $e's wearing weights.",victim,NULL,NULL,TO_ROOM);
}

void spell_amplify_damage(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || victim->isaff(AF_BDAMTAKE)){
		if (victim == ch)
			ch->send("You can't get any wussier!\n\r");
		else
			act("$N seems to be wussy enough.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (saves_spell(level,victim,DAM_MENTAL) || victim->res[RS_MAGIC] == 0){
		if (victim != ch)
			ch->send("Nothing seemed to happen.\n\r");
		victim->send("You feel momentarily wussier.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,5,APPLY_END,-10,AF_BDAMTAKE);
	victim->send("You feel wussy.\n\r");
	act("$n appears very wussy.",victim,NULL,NULL,TO_ROOM);
}

void spell_overpower(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || victim->isaff(AF_BDAMGIVE)){
		if (victim == ch)
			ch->send("You can't get any wussier!\n\r");
		else
			act("$N seems to be buff enough.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,5,APPLY_STR,1,AF_BDAMGIVE);
	victim->send("You feel buff!\n\r");
	act("$n appears very buff!",victim,NULL,NULL,TO_ROOM);
}

void spell_phase(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA *af=new_affect();

	if (victim != ch && get_trust(ch) < LEVEL_IMMORTAL){
		ch->send("You may only cast this on yourself.\n\r");
		return;
	}
    if (victim->isaff(AF_PHASE)){
		ch->send("You are already out of phase.\n\r");
		return;
    }

	affect_set(victim,TO_AFFECTS,gsn_phase,level,slvl,slvl,APPLY_NONE,slvl,AF_PHASE);
	if(slvl > 3){
		affect_set(victim,TO_AFFECTS,gsn_phase,level,slvl,slvl,APPLY_NONE,slvl,AF_PASS_DOOR);
		act("$n becomes translucent as $e phases slightly out of reality.",victim,NULL,NULL,TO_ROOM);
		victim->send("You become translucent as you phase out of reality slightly.\n\r");
	}
	else{
		act("$n seems to phase out of reality a bit.",victim,NULL,NULL,TO_ROOM);
		victim->send("Your body phases out of reality slightly.\n\r");
	}
}

void spell_bound_recall(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *charmy = ch->charmy_first;

	if(!charmy){
		ch->send("You have no charmies to recall.\n\r");
		return;
	}

	act("You wave your hand and create a glowing rift.",ch,NULL,NULL,TO_CHAR);
	act("$n waves $s hand and creates a glowing rift.",ch,NULL,NULL,TO_ROOM);

	do{
		if(charmy->in_room == ch->in_room)
			continue;
		char_from_room(charmy);
		char_to_room(charmy,ch->in_room);
		act("You are called back to your master.",charmy,NULL,NULL,TO_CHAR);
		act("$n steps out from the rift.",charmy,NULL,NULL,TO_ROOM);
	}while((charmy = charmy->next_charmy));

	act("The glowing rift shimmers out of existence.",ch,NULL,NULL,TO_CHAR);
	act("The glowing rift shimmers out of existence.",ch,NULL,NULL,TO_ROOM);
	WAIT_STATE(ch,12);
}

void spell_aid(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *charmy = (CHAR_DATA*) vo;
	int heal;

	if(!charmy->isaff(AF_CHARM) || !is_same_group(ch,charmy)){
		ch->send("You have no charmy named that.\n\r");
		return;
	}
	if(charmy->hit == charmy->max_hit){
		ch->send("They look fine.\n\r");
		return;
	}
	heal = charmy->max_hit - charmy->hit;
	heal /= -5;
	heal += ((heal/10) * ch->getslvl(sn));
	act("You wave your hand over $N, magically sealing $S wounds.",ch,NULL,charmy,TO_CHAR);
	act("$n waves $s hand over your wounds, magically sealing them.",ch,NULL,charmy,TO_VICT);
	act("$n waves $s hand over $N's wounds, magically sealing them.",ch,NULL,charmy,TO_NOTVICT);

	damage(ch,charmy,heal,sn,DAM_ENERGY,true);
	WAIT_STATE(ch,12);
}

void spell_defensive_shield(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *charmy = (CHAR_DATA*) vo;
	AFFECT_DATA af;

	if(!charmy->isaff(AF_CHARM) || !is_same_group(ch,charmy)){
		ch->send("You have no charmy named that.\n\r");
		return;
	}
	if(is_affected(charmy,sn) || charmy->isaff(AF_GUARDIAN)){
		ch->send("They are already shielded.\n\r");
		return;
	}

	affect_set(charmy,TO_AFFECTS,sn,level,slvl,level/10,APPLY_END,slvl*5,AF_GUARDIAN);
	send_to_char("You feel defensive.\n\r",charmy);
	act("$n is enveloped in a magical coating.",charmy,NULL,NULL,TO_ROOM);
}

void spell_blur(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	AFFECT_DATA af;

	if(is_affected(victim,sn)){
		ch->send("They are already blurry visionized.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl),APPLY_AGI,-1 * (slvl*3),0);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl),APPLY_HITROLL,-1 * (slvl*5),0);

	send_to_char("Your vision becomes blurred.\n\r",victim);
	act("$n looks blinder?",victim,NULL,NULL,TO_ROOM);
}

void spell_muddle(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	AFFECT_DATA af;

	if(is_affected(victim,sn)){
		ch->send("They are already muddled.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl),APPLY_WIS,-1 * slvl*3,0);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl),APPLY_INT,-1 * slvl*5,0);

	send_to_char("Your brain muddles with a thousand thoughts.\n\r",victim);
	act("$n looks muddlier?",victim,NULL,NULL,TO_ROOM);
}

void spell_bind(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	AFFECT_DATA af;

	if(is_affected(victim,sn)){
		ch->send("They are already bound.\n\r");
		return;
	}
	if(saves_spell(level+ch->getslvl(sn),victim,DAM_ENERGY)){
		ch->send("You fail.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_AC,-1 * slvl * 5,AF_SNARED);

	act("You've bound $N to the ground!",ch,NULL,victim,TO_CHAR);
	act("$n binds your legs to the ground!",ch,NULL,victim,TO_VICT);
	act("$n binds $N's legs to the ground!",ch,NULL,victim,TO_NOTVICT);
}

void spell_reveal(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *ich;

	act("$n conjures a cloud of purple smoke.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You conjure a cloud of purple smoke.\n\r",ch);

	for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room ){
		if (ich->invis_level > 0)
			continue;

		if(ich == ch || saves_spell(level,ich,DAM_ENERGY))
			continue;

		unSneak(ich);
		affect_strip(ich,gsn_invis);
		affect_strip(ich,gsn_mass_invis);
		ich->remaff(AF_INVISIBLE);
		act("$n is revealed!",ich,NULL,NULL,TO_ROOM);
		send_to_char("You are revealed!\n\r",ich);
	}
}

void spell_bright_light(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *light;

	light = create_object(get_obj_index(OBJ_VNUM_LIGHT_BALL),0);
	obj_to_room(light,ch->in_room);
	REMOVE_BIT(light->wear_flags,ITEM_HOLD);
	SET_BIT(light->wear_flags,ITEM_WEAR_FLOAT_LIGHT);
	light->short_descr = str_dup("a {Cbright {xlight");
	light->description = str_dup("An almost blinding ball of light is here.");
	light->name = str_dup("bright light");
	light->level = 1;
	light->weight = 10;
	light->value[0] = 1;
	light->value[2] = 25;
	act("$n wiggles $s thumbs and $p appears.",ch,light,NULL,TO_ROOM);
	act("You wiggle your thumbs and $p appears.",ch,light,NULL,TO_CHAR);
}

void spell_friendly_aura(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

	if(is_affected(ch,sn)){
		ch->send("You are already amiable to aggressors.\n\r");
		return;
	}

	affect_set(ch,TO_AFFECTS,sn,level,slvl,2,APPLY_SAVES,-1,AF_AMIABLE);

	act("You feel more likeable!",ch,NULL,NULL,TO_CHAR);
	act("$n seems more likeable.",ch,NULL,NULL,TO_ROOM);
}

void spell_brilliant_flare(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

	if (ch->in_room->israf(RAF_LIGHT) || ch->in_room->israf(RAF_DARK)){
		ch->send("You can't change the brightness here.\n\r");
		return;
	}
	if(ch->in_room->sector_type == SECT_INSIDE || IS_SET(ch->in_room->room_flags,ROOM_INDOORS)){
		ch->send("This room cannot be lit.\n\r");
		return;
	}
	act("You waves your hand and a bright flare lights the room.",ch,NULL,NULL,TO_CHAR);
	act("$n waves $s hand and a bright flare lights the room.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl) * 6,APPLY_NONE,0,RAF_LIGHT);
}

void spell_phasic_cone(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam,count = ch->getslvl(sn);

	act("You create a cone of phased space!",ch,NULL,NULL,TO_CHAR);
	act("$n creates a cone of phased space!",ch,NULL,NULL,TO_ROOM);

	for(count = 0;count < ch->getslvl(sn);count++){
		for(vch = ch->in_room->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if(is_same_group(vch,ch))
				continue;
			dam = get_curr_stat(ch,STAT_INT)*ch->getslvl(sn);
			if(saves_spell(level,vch,DAM_ENERGY))
				dam *= 1.75;
			spell_damage(ch,vch,dam,sn,DAM_ENERGY,true);
		}
	}
}

void spell_magical_lock(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	ROOM_INDEX_DATA *room;
	AFFECT_DATA af;
	int door = target;

	if(ch->in_room->israf(RAF_MWALL)){
		ch->send("There is already a wall here.\n\r");
		return;
	}
	act("You pull a wall of sizzling magic $T.",ch,NULL,dir_name[door],TO_CHAR);
	act("$n pulls a wall of sizzling magic $T.",ch,NULL,dir_name[door],TO_ROOM);

	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,slvl*2,APPLY_NONE,door,RAF_MWALL);
}
