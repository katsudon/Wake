#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"




void check_compress(CHAR_DATA*,CHAR_DATA*,OBJ_DATA*);

void acid_effect(void *vo, int level, int dam, int target){
	if (target == TARGET_ROOM){ /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			acid_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR){  /* do the effect on a victim */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		
		/* let's toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			acid_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ){ /* toast an object */
		OBJ_DATA *obj = (OBJ_DATA *) vo,*t_obj,*n_obj;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj,ITM_BURN_PROOF) || IS_OBJ_STAT(obj,ITM_NOPURGE) || number_range(0,4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		chance -= obj->level * 2;

		switch (obj->item_type){
			default:
			return;
			case ITEM_CONTAINER:
			case ITEM_CORPSE_PC:
			case ITEM_CORPSE_NPC:
				msg = "$p fumes and dissolves.";
				break;
			case ITEM_ARMOR:
			case ITEM_SHIELD:
				msg = "$p is pitted and etched.";
				break;
			case ITEM_STAFF:
			case ITEM_WAND:
				chance -= 10;
				msg = "$p corrodes and breaks.";
				break;
			case ITEM_SCROLL:
				chance += 10;
				msg = "$p is burned into waste.";
				break;
		}

		chance = URANGE(5,chance,95);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act(msg,obj->carried_by,obj,NULL,TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);

		if (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD){ /* etch it */
			AFFECT_DATA *paf;
			bool af_found = false;
			int i;

 			affect_enchant(obj);

			for ( paf = obj->affected; paf != NULL; paf = paf->next){
				if ( paf->location == APPLY_AC){
					af_found = true;
					paf->type = -1;
					paf->modifier += 1;
					paf->level = UMAX(paf->level,level);
					break;
				}
			}

			if (!af_found){
				paf = new_affect();

				paf->type       = -1;
				paf->level      = level;
				paf->duration   = -1;
				paf->location   = APPLY_AC;
				paf->modifier   =  1;
				paf->bitvector  = 0;
				paf->next       = obj->affected;
				obj->affected   = paf;
			}

			if (obj->carried_by != NULL && obj->wear_loc != WEAR_NONE)
				for (i = 0; i < 4; i++)
					obj->carried_by->armor[i] += 1;
			return;
		}

		/* get rid of the object */
		if (obj->contains){  /* dump contents */
			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj){
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);
				if (obj->in_room != NULL)
					obj_to_room(t_obj,obj->in_room);
				else if (obj->carried_by != NULL)
					obj_to_room(t_obj,obj->carried_by->in_room);
				else
				{
					extract_obj(t_obj);
					continue;
				}

				acid_effect(t_obj,level/2,dam/2,TARGET_OBJ);
			}
		}

		extract_obj(obj);
		return;
	}
}

void cold_effect(void *vo, int level, int dam, int target){
	if (target == TARGET_ROOM){ /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			cold_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR){ /* whack a character */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* chill touch effect */
		if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD)){
			AFFECT_DATA af;

			act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
			act("A chill sinks deep into your bones.",victim,NULL,NULL,TO_CHAR);
			affect_join(victim,TO_AFFECTS,skill_lookup("chill touch"),level,1,6,APPLY_STR,-1,0);
		}

		/* hunger! (warmth sucked out */
		if (!IS_NPC(victim))
			gain_condition(victim,COND_HUNGER,dam/20);

		/* let's toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			cold_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ){ /* toast an object */
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj,ITM_BURN_PROOF) || IS_OBJ_STAT(obj,ITM_NOPURGE) || number_range(0,4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

 		chance -= obj->level * 2;

		switch(obj->item_type){
			default:
				return;
			case ITEM_POTION:
				msg = "$p freezes and shatters!";
				chance += 25;
				break;
			case ITEM_DRINK_CON:
				msg = "$p freezes and shatters!";
				chance += 5;
				break;
		}

		chance = URANGE(5,chance,95);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act(msg,obj->carried_by,obj,NULL,TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);

		extract_obj(obj);
		return;
	}
}

void fire_effect(void *vo, int level, int dam, int target){
	if (target == TARGET_ROOM){  /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			fire_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR){   /* do the effect on a victim */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* chance of blindness */
		if (!victim->isaff(AF_BLIND) && !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE)){
			AFFECT_DATA af;
			act("$n is blinded by smoke!",victim,NULL,NULL,TO_ROOM);
			act("Your eyes tear up from smoke...you can't see a thing!", victim,NULL,NULL,TO_CHAR);

			affect_set(victim,TO_AFFECTS,skill_lookup("fire breath"),level,1,number_range(0,level/10),APPLY_HITROLL,-4,AF_BLIND);
		}

		/* getting thirsty */
		if (!IS_NPC(victim))
			gain_condition(victim,COND_THIRST,dam/20);

		/* let's toast some gear! */
		for (obj = victim->carrying; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			fire_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ){  /* toast an object */
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		OBJ_DATA *t_obj,*n_obj;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj,ITM_BURN_PROOF) || IS_OBJ_STAT(obj,ITM_NOPURGE) || number_range(0,4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		chance -= obj->level * 2;

		switch ( obj->item_type ){
			default:
				return;
			case ITEM_CONTAINER:
				msg = "$p ignites and burns!";
				break;
			case ITEM_POTION:
				chance += 25;
				msg = "$p bubbles and boils!";
				break;
			case ITEM_SCROLL:
				chance += 50;
				msg = "$p crackles and burns!";
				break;
			case ITEM_STAFF:
				chance += 10;
				msg = "$p smokes and chars!";
				break;
			case ITEM_WAND:
				msg = "$p sparks and sputters!";
				break;
			case ITEM_FOOD:
				msg = "$p blackens and crisps!";
				break;
			case ITEM_PILL:
				msg = "$p melts and drips!";
				break;
		}

		chance = URANGE(5,chance,95);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act( msg, obj->carried_by, obj, NULL, TO_ALL );
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);

		if (obj->contains){
			/* dump the contents */
			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj){
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);
				if (obj->in_room != NULL)
					obj_to_room(t_obj,obj->in_room);
				else if (obj->carried_by != NULL)
					obj_to_room(t_obj,obj->carried_by->in_room);
				else{
					extract_obj(t_obj);
					continue;
				}
				fire_effect(t_obj,level/2,dam/2,TARGET_OBJ);
			}
		}

		extract_obj( obj );
		return;
	}
}

void poison_effect(void *vo,int level, int dam, int target){
	if (target == TARGET_ROOM){  /* nail objects on the floor */
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			poison_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR){   /* do the effect on a victim */
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* chance of poisoning */
		if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON)){
			AFFECT_DATA af;

			send_to_char("You feel poison coursing through your veins.\n\r", victim);
			act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);

			affect_join(victim,TO_AFFECTS,gsn_poison,level,1,level/2,APPLY_STR,-1,AF_POISON);
		}

		/* equipment */
		for (obj = victim->carrying; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			poison_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ){  /* do some poisoning */
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;

		if (IS_OBJ_STAT(obj,ITM_BURN_PROOF) || number_range(0,4) == 0)
			return;

		chance = level / 4 + dam / 10;
		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) / 2 + 50;

		chance -= obj->level * 2;

		switch (obj->item_type){
			default:
				return;
			case ITEM_FOOD:
				break;
			case ITEM_DRINK_CON:
				if (obj->value[0] == obj->value[1])
					return;
			break;
		}

		chance = URANGE(5,chance,95);

		if (number_percent() > chance)
			return;

		obj->value[3] = 1;
		return;
	}
}

void shock_effect(void *vo,int level, int dam, int target){
	if (target == TARGET_ROOM){
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			shock_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR){
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		/* daze and confused? */
		if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING)){
			send_to_char("Your muscles stop responding.\n\r",victim);
			DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
		}

		/* toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			shock_effect(obj,level,dam,TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ){
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj,ITM_SHOCK_PROOF) || IS_OBJ_STAT(obj,ITM_NOPURGE) || number_range(0,4) == 0)
			return;

		chance = level / 4 + dam / 10;

		if (chance > 25)
			chance = (chance - 25) / 2 + 25;
		if (chance > 50)
			chance = (chance - 50) /2 + 50;

 		chance -= obj->level * 2;

		switch(obj->item_type){
			default:
				return;
			case ITEM_WAND:
			case ITEM_STAFF:
				chance += 10;
				msg = "$p overloads and explodes!";
				break;
			case ITEM_JEWELRY:
				chance -= 10;
				msg = "$p is fused into a worthless lump.";
				break;
		}
		
		chance = URANGE(5,chance,95);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act(msg,obj->carried_by,obj,NULL,TO_ALL);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg,obj->in_room->people,obj,NULL,TO_ALL);

		extract_obj(obj);
		return;
	}
}

void shield_effects(CHAR_DATA *ch,CHAR_DATA*victim,OBJ_DATA *shield){
	int dam=0;

	if (IS_SET(shield->extra_flags,ITM_FLAMING)){
		dam = number_range(1,shield->level / 4 + 1);
		act("$n is {rburned {xby $p.",victim,shield,NULL,TO_ROOM);
		act("$p {rsears {xyour flesh.",victim,shield,NULL,TO_CHAR);
		fire_effect((void *) ch,shield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_FIRE,false);
	}
	if (IS_SET(shield->extra_flags,ITM_FROSTED)){
		dam = number_range(1,shield->level / 4 + 1);
		act("$n is {Cfrozen {xby $p.",victim,shield,NULL,TO_ROOM);
		act("$p {Cfreezes {xyou.",victim,shield,NULL,TO_CHAR);
		cold_effect(ch,shield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_COLD,false);
	}
	if (IS_SET(shield->extra_flags,ITM_SHOCKED)){
		dam = number_range(1,shield->level / 4 + 1);
		act("$n is {Yelectrocuted {xby $p.",victim,shield,NULL,TO_ROOM);
		act("$p {Yshocks {xyou.",victim,shield,NULL,TO_CHAR);
		shock_effect((void *) ch,shield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_LIGHTNING,false);
	}
	if (shield->wflags[WPN_THORNED]){
		dam = number_range(1,shield->level / 4 + 1);
		act("$n is pricked by the {gthorns {xon $p.",victim,shield,NULL,TO_ROOM);
		act("$p's {gthorns{x prick {xyou.",victim,shield,NULL,TO_CHAR);
		shock_effect((void *) ch,shield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_PLANT,false);
	}
}

void weapon_effects(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield){
	int dam,level;
	AFFECT_DATA *poison,*plague,af;

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_POISON)){
		if (!(poison = affect_find(wield->affected,gsn_poison)))
			level = wield->level;
		else
			level = poison->level;
	
		if (!saves_spell(level / 2,victim,DAM_POISON)) {
			victim->send("You feel poison coursing through your veins.\n\r");
			act("$n is poisoned by the venom on $p.", victim,wield,NULL,TO_ROOM);

			affect_join(victim,TO_AFFECTS,gsn_poison,level*3/4,1,level/2,APPLY_STR,-1,AF_POISON);
		}

		/* weaken the poison if it's temporary */
		if (poison){
	    	poison->level = UMAX(0,poison->level - 2);
	    	poison->duration = UMAX(0,poison->duration - 1);
	
	    	if (poison->level == 0 || poison->duration == 0)
			act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
		}
 	}
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_PESTILENCE)){
		if (!(plague = affect_find(wield->affected,gsn_plague)))
			level = wield->level;
		else
			level = plague->level;
	
		if (!saves_spell(level / 2,victim,DAM_POISON)) {
			victim->send("You feel incredibly sick.\n\r");
			act("$n is afflicted by $p.",victim,wield,NULL,TO_ROOM);

			affect_join(victim,TO_AFFECTS,gsn_plague,level*3/4,1,level/2,APPLY_STR,-1,AF_PLAGUE);
		}

		/* weaken the poison if it's temporary */
		if (plague){
	    	plague->level = UMAX(0,plague->level - 2);
	    	plague->duration = UMAX(0,plague->duration - 1);
		}
 	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_VAMPIRIC)){
		dam = number_range(1, wield->level / 5 + 1);
		act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
		act("You feel $p drawing your life away.",
		victim,wield,NULL,TO_CHAR);
		damage(ch,victim,dam,0,DAM_NEGATIVE,false);
		ch->hit += dam/2;
	}
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_POWERDRAIN)){
		dam = number_range(1, wield->level / 5 + 1);
		act("$p draws power from $n.",victim,wield,NULL,TO_ROOM);
		act("You feel $p drawing your power away.", victim,wield,NULL,TO_CHAR);
		//damage(ch,victim,dam,0,DAM_NEGATIVE,false);
		victim->manadamage(dam);
		ch->modmana(dam/2);
	}
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_FLAMING)){
		dam = number_range(1,wield->level / 4 + 1);
		act("$n is {rburned{x by $p.",victim,wield,NULL,TO_ROOM);
		act("$p {rsears {xyour flesh.",victim,wield,NULL,TO_CHAR);
		fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_FIRE,false);
	}
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_FROST)){
		dam = number_range(1,wield->level / 6 + 2);
		act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
		act("The {Ccold touch {xof $p surrounds you with {Cice{x.",
		victim,wield,NULL,TO_CHAR);
		cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_COLD,false);
	}
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_SHOCKING)){
		dam = number_range(1,wield->level/5 + 2);
		act("$n is struck by {Ylightning {xfrom $p.",victim,wield,NULL,TO_ROOM);
		act("You are {Yshocked {xby $p.",victim,wield,NULL,TO_CHAR);
		shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_LIGHTNING,false);
	}
	if(ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_THORNED)){
		dam = number_range(1,wield->level/5 + 2);
		act("$n is scratched by the {gthorns {xfrom $p.",victim,wield,NULL,TO_ROOM);
		act("You are scratched by the {gthorns {xfrom $p.",victim,wield,NULL,TO_CHAR);
		shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
		damage(ch,victim,dam,0,DAM_PLANT,false);
	}
	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_SERRATED)){
		if (number_percent() < 10){
			dam = number_range(wield->level/5, wield->level/2);
			act("$p {rrips apart{x $n's flesh.",victim,wield,NULL,TO_ROOM);
			act("Your flesh is {rripped apart{x by $p.",victim,wield,NULL,TO_CHAR);
			damage(ch,victim,dam,0,DAM_OTHER,false);
		}else{
			dam = number_range(1,wield->level/5 + 2);
			act("$p tears at $n's flesh.",victim,wield,NULL,TO_ROOM);
			act("$p tears at your flesh.",victim,wield,NULL,TO_CHAR);
			damage(ch,victim,dam,0,DAM_OTHER,false);
		}
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_HOLY)){
		if (!IS_GOOD(victim)){
			if (IS_EVIL(victim)){
				dam = number_range(wield->level/10, wield->level/3);
				act("$p {wSMITES{x $n with holy power.",victim,wield,NULL,TO_ROOM);
				act("$p {wSMITES{x you with holy power.",victim,wield,NULL,TO_CHAR);

			}
			else{
				dam = number_range(1,wield->level/5 + 2);
				act("$n is {wburned{x by $p's holy power.",victim,wield,NULL,TO_ROOM);
				act("You are {wburned{x by $p's holy power.",victim,wield,NULL,TO_CHAR);
			}
			damage(ch,victim,dam,0,DAM_HOLY,false);
		}
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WPN_DEMONIC)){
		if (!IS_EVIL(victim)){
			if (IS_GOOD(victim)){
				dam = number_range(wield->level/10, wield->level/3);
				act("$p {dSMITES{x $n with unholy malice.",victim,wield,NULL,TO_ROOM);
				act("$p {dSMITES{x you with unholy malice.",victim,wield,NULL,TO_CHAR);

			}
			else{
				dam = number_range(1,wield->level/5 + 2);
				act("$n is {ddesecrated{x by $p.",victim,wield,NULL,TO_ROOM);
				act("You are {ddesecrated{x by $p.",victim,wield,NULL,TO_CHAR);
			}
			
			damage(ch,victim,dam,0,DAM_NEGATIVE,false);
		}
	}
}

void do_bandage(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *obj,*obj_next;
	AFFECT_DATA af;
	int chance;
	char arg[MIL],arg2[MIL];
	bool found = false;

	argument = one_argument(argument,arg);
	one_argument(argument,arg2);

	if (!arg[0]){
		ch->send("Bandage who?\n\r");
		return;
	}

	if (ch->fighting){
		ch->send("You can't bandage in combat!\n\r");
		return;
	}
	else if (!(victim = get_char_room(ch, NULL,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if(arg2[0]){
		for ( obj = ch->carrying; obj != NULL && !found; obj = obj_next ){
			obj_next = obj->next_content;

			if (is_name(arg2,obj->name) && can_see_obj(ch,obj) && can_drop_obj(ch,obj)){
				found = true;
				if (obj && obj->item_type != ITEM_HERB)
					found = false;
			}
		}
		if (!found){
			ch->send("What herb do you intend to use with the bandages?\n\r");
			return;
		}
	}

	if (victim->isaff(AF_REGENERATION) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)) ){
		ch->send("They can't be bandaged.\n\r");
		return;
	}

	chance = get_skill(ch,gsn_bandage) * .75;

	WAIT_STATE(ch,skill_table[gsn_bandage].beats);

	if (number_percent() >= chance){
		send_to_char("You fail.\n\r",ch);
		check_improve(ch,gsn_bandage,false,5);
	}
	else{
		check_improve(ch,gsn_bandage,true,6);
		if(ch == victim){
			ch->send("You bandage your wounds.\n\r");
			act("$n carefully bandages $s wounds.",ch,NULL,NULL,TO_ROOM);
		}
		else{
			act("You bandage $N's wounds.",ch,NULL,victim,TO_CHAR);
			act("$n carefully bandages your wounds.",ch,NULL,victim,TO_VICT);
			act("$n carefully bandages $N's wounds.",ch,NULL,victim,TO_NOTVICT);
		}
		victim->hit += 10;
		affect_join(victim,TO_AFFECTS,gsn_bandage,ch->level,ch->getslvl(gsn_bandage),1,APPLY_END,1,AF_REGENERATION);
		if(found)
			check_compress(ch,victim,obj);
		else
			check_compress(ch,victim,NULL);
	}
}

void mob_stuff :: buff_res(int which,int amount){
	if (amount == 0 && res[which] < 0)
		res[which] = 0;
	res[which] -= amount;
}
void mob_stuff :: nerf_res(int which,int amount){
	res[which] += amount;
}
void mob_stuff :: reset_res(int which){
	char arg[100],*buf;
	int i,n,val;
	bool found;

	for (i = 0;i<MAX_RES;i++){
		if (race_table[race].res[i]){
			buf = str_dup(race_table[race].res[i]);
			buf = one_argument(buf,arg);
			if(!is_number(buf)){
				wiznet("bad reset_res, check logfiles now",NULL,NULL,WZ_FLAGS,0,0);
				log_f("reset_res: icky value  %s '%s'",race_table[race].name,race_table[race].res[i]);
				return;
			}
			val = atoi(buf);
			found = false;
			if(!strcmp(res_flags[which].name,arg))
				res[which] = val;
		}
		else
			res[which] = 100;
	}
}

int get_res(int dam_type){
	switch (dam_type){
		case 0:					return RS_BLANK;
		case(DAM_OTHER):		return RS_BLANK;
		case(DAM_BASH):			return RS_BASH;
		case(DAM_PIERCE):		return RS_PIERCE;
		case(DAM_SLASH):		return RS_SLASH;
		case(DAM_FIRE):			return RS_FIRE;
		case(DAM_COLD):			return RS_COLD;
		case(DAM_LIGHTNING):	return RS_LIGHTNING;
		case(DAM_ACID):			return RS_ACID;
		case(DAM_POISON):		return RS_POISON;
		case(DAM_NEGATIVE):		return RS_NEGATIVE;
		case(DAM_HOLY):			return RS_HOLY;
		case(DAM_ENERGY):		return RS_ENERGY;
		case(DAM_MENTAL):		return RS_MENTAL;
		case(DAM_DISEASE):		return RS_DISEASE;
		case(DAM_WATER):		return RS_WATER;
		case(DAM_LIGHT):		return RS_LIGHT;
		case(DAM_CHARM):		return RS_CHARM;
		case(DAM_SOUND):		return RS_SOUND;
		case(DAM_WIND):			return RS_WIND;
		case(DAM_TRAPS):		return RS_TRAPS;
		case(DAM_EARTH):		return RS_EARTH;
		case(DAM_HARM):			return RS_HARM;
		default:				return -1;
	}
	return -1;
}

int dam_res(CHAR_DATA *ch,CHAR_DATA *victim,int dam_type,int dam){
	char buf[100];
	int i,n = get_res(dam_type);

	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"Damtype:(%d) N:(%d) ",dam_type,n);
	if (n == -1){
		ch->send("BUGGY DAM_RES!\n\r");
		printf_to_char(ch,"%d\n\r",dam_type);
		return 0;
	}
	else if (n != 0){
		sprintf(buf,"BUGGY");
		for (i = 0;res_flags[i].name;i++)
			if(res_flags[i].bit == n){
				sprintf(buf,"%s",res_flags[i].name);
				break;
			}
		if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"Res: %s (%d)\n\r",buf,n);

		if (ch->iscomm(CM_DEBUG)){
			if (victim->res[n] == 0)
				ch->send("Immune ");
			else if (victim->res[n] > 100)
				ch->send("Vulnerable ");
			else if (victim->res[n] < 100)
				ch->send("Resistant ");
			else
				ch->send("Neutral ");

			printf_to_char(ch,"Pre-dam: %d",dam);
		}
		dam = (dam * victim->res[n]) / 100;
		if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"Post-dam: %d ",dam);

		if (ch->iscomm(CM_DEBUG)){
			sprintf(buf,"BUGGY");
			for (i = 0;dam_flags[i].name;i++)
				if(dam_flags[i].bit == dam_type){
					sprintf(buf,"%s",dam_flags[i].name);
					break;
				}
			printf_to_char(ch,"Dam: %s\n\r",buf);
		}
	}
	return dam;
}
//nash make this use slvl someday
void check_energy_shield(CHAR_DATA *ch,CHAR_DATA *victim){
	if(!victim->isaff(AF_ESHIELD))
		return;
	spell_damage(victim,ch,dice(victim->level,2),skill_lookup("energy shield"),DAM_ENERGY,true);
}

void snake_bite(CHAR_DATA *ch,CHAR_DATA *victim){
	AFFECT_DATA af;
	int chance = ch->level/5;

	if(victim->isaff(AF_VENOM))
		return;

	chance -= UMIN(5,(victim->level - ch->level));
	chance += get_curr_stat(ch,STAT_AGI);

	if(roll_chance(ch,chance)){
		act("You bite $N and $E twitches, $S flesh turning black at the bite mark.",ch,NULL,victim,TO_CHAR);
		act("$n bites you and a searing pain spreads from the blackening bite mark.",ch,NULL,victim,TO_VICT);
		act("$n bites $N and $E twitches, $S flesh turning black at the bite mark.",ch,NULL,victim,TO_NOTVICT);
		affect_join(victim,TO_AFFECTS,gsn_venom,ch->level,1,ch->level/10+1,APPLY_STR,-10,AF_VENOM);
	}
}
