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
void aspell_damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,bool show);

void do_crash(CHAR_DATA *ch,char *argument){
	int sn=gsn_energy_crash,skill=get_skill(ch,sn),needed = ch->max_hit - ch->hit;

	if(skill<1){
		ch->send("You aren't able to crash, at least not this way.\n\r");
		return;
	}
	if(ch->hit == ch->max_hit){
		ch->send("Your vitality is where it should be already.\n\r");
		return;
	}
	if(ch->getmana() < 1){
		ch->send("You do not have enough energy to crash.\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[sn].beats);
	if(!roll_chance(ch,skill)){
		ch->send("You focus hard, but fail to properly burn yourself out.\n\r");
		check_improve(ch,sn,false,1);
	}
	else{
		ch->send("You focus hard and feel the energy drain from your body, leaving you revitalized.\n\r");
		if(needed > ch->mana)
			needed = ch->mana / 2;
		else
			needed /= 2;
		ch->hit += needed + (needed / 10 * ch->getslvl(sn));
		ch->modmana(-needed);
		check_improve(ch,sn,true,1);
	}
}
int reject_magic(CHAR_DATA *ch,int dam){
	int sn = gsn_reject_magic,skill = get_skill(ch,sn) / 4;

	if(skill < 1)
		return dam;

	if(roll_chance(ch,skill)){
		dam -= dam / 20 * ch->getslvl(sn);
		check_improve(ch,sn,true,1);
		return dam;
	}
	check_improve(ch,sn,false,1);
	return dam;
}
bool occult_wisdom(CHAR_DATA *ch){
	int sn = gsn_occult_wisdom,skill = get_skill(ch,sn) / 20;

	if(skill < 1)
		return false;

	skill += (ch->getslvl(sn) * 2) + get_curr_stat(ch,STAT_INT) / 5;
	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}
int will_force(CHAR_DATA *ch){
	int sn = gsn_will_force,skill = get_skill(ch,sn) / 2;

	if(skill < 1)
		return 0;
	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return ch->getslvl(sn) * 2;
	}
	check_improve(ch,sn,false,1);
	return 0;
}

int check_shunt(CHAR_DATA *ch,int dam){
	int sn = gsn_shunt,skill = get_skill(ch,sn);

	if(skill < 1)
		return dam;

	if(roll_chance(ch,skill)){
		ch->send("You shunt some of the damage into your energy.\n\r");
		act("$n spreads the damage through $s body.",ch,NULL,NULL,TO_ROOM);
		dam /= 2;
		ch->manadamage(dam / ch->getslvl(sn));
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	return dam;
}

void do_shunt(CHAR_DATA *ch,char *argument){
	if(get_skill(ch,gsn_shunt) < 1){
		ch->send("You might need to use the bathroom to do that.\n\r");
		return;
	}
	if(ch->setdef(DF_SHUNT))
		ch->send("You will now shunt some damage into your energy.\n\r");
	else{
		ch->send("You will no longer shunt damage into your energy.\n\r");
		ch->remdef(DF_SHUNT);
	}
}

int validate_mortal_coil(CHAR_DATA *ch){
	int skill = get_skill(ch,gsn_mortal_coil);

	if(skill < 1 || ch->hit * 100 / ch->max_hit > 25)
		return 0;
	return skill;
}
int mortal_coil_saves(CHAR_DATA *ch){
	int sn = gsn_mortal_coil,skill = validate_mortal_coil(ch);

	if(skill < 1)
		return 0;

	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return ch->getslvl(sn) * 1.5;
	}
	check_improve(ch,sn,false,1);
	return 0;
}
int mortal_coil_damage(CHAR_DATA *ch,int dam){
	int sn = gsn_mortal_coil,skill = validate_mortal_coil(ch) / 2;

	if(skill < 1)
		return dam;
	if(roll_chance(ch,skill)){
		check_improve(ch,sn,true,1);
		return UMAX(1,dam - dam / 100 * ch->getslvl(sn));
	}
	check_improve(ch,sn,false,1);
	return dam;
}
int abjuring_intellect(CHAR_DATA *ch,int dam,int dam_type){
	int sn = gsn_abjuring_intellect,skill = get_skill(ch,sn) / 2;

	if(skill < 1)
		return dam;

	if(dam_type == DAM_ENERGY){
		dam /= 2;
		check_improve(ch,sn,false,1);
	}
	if(dam_type == DAM_HARM){
		if(roll_chance(ch,skill)){
			check_improve(ch,sn,true,1);
			dam += dam / 20 * ch->getslvl(sn);
		}
		else
			check_improve(ch,sn,false,1);
	}
	return dam;
}

void kill_talisman(CHAR_DATA *ch,OBJ_DATA *obj,int slvl,int level){
	CHAR_DATA *victim = obj->carried_by;
	int cchance;
	cchance = level / 2;
	cchance += cchance / 2 * slvl;
	cchance += number_range(25,50);
	cchance -= obj->level;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"DisableTalisman: %d\n\r",cchance);

	if(obj->item_type == ITEM_WAND
	|| obj->item_type == ITEM_STAFF
	|| obj->item_type == ITEM_SCROLL
	|| obj->item_type == ITEM_POTION
	|| obj->item_type == ITEM_PILL){
		if(number_percent() < cchance){
			if(obj->item_type == ITEM_PILL || obj->item_type == ITEM_SCROLL || obj->item_type == ITEM_POTION){
				if(number_percent() < slvl * 20){
					//dead item
				}
			}
			else{//wand staff
				switch(number_range(0,slvl)){
					case 1:
						obj->value[2]--;
						break;
					case 2:
						obj->value[2] = 0;
						break;
					case 3:
						obj_from_char(obj);
						obj_to_room(obj,victim->in_room);
						break;
					case 4:
						break;
					case 5:
						obj_from_char(obj);
						extract_obj(obj);
						break;
					default:
						break;
				}
			}
		}
	}
}
void spell_disable_talisman(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	OBJ_DATA *obj,*obj_next;
//give chances to fade it partly, fade completely, make it drop, nohold, or pop
//wands, staves, scrolls, potions, pills, room wards
	if(target == TARGET_CHAR){
		if(!canFight(ch,victim,false)){
			ch->send("You cannot do that to them!\n\r");
			return;
		}
		for(obj = victim->carrying;obj;obj = obj_next){
			obj_next = obj->next_content;
			kill_talisman(ch,obj,slvl,level);
		}
		for(int iWear = 0;iWear < MAX_WEAR; iWear++){
			if ((obj = get_eq_char(victim,iWear)) == NULL)
				continue;
			kill_talisman(ch,obj,slvl,level);
		}
	}
	else{//make it also purge room affects
		for(obj = ch->in_room->contents;obj;obj = obj_next){
			obj_next = obj->next_content;
		}
	}
}
void spell_dull_spell(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	AFFECT_DATA *af = new_affect();

	if (get_skill(victim,gsn_resilience))
		level /= 2;

	if (saves_spell((level * 1.5) * slvl,victim,DAM_DISEASE) || (IS_NPC(victim) && victim->isact(AT_UNDEAD))){
		if (ch == victim)
			send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
		else
			act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_join(victim,TO_AFFECTS,sn,level * 3 / 4,slvl,2,APPLY_INT,-5,AF_DULLSPELL);

	send_to_char("You feel less powerful.\n\r",victim);
	act("$n seems less powerful.",victim,NULL,NULL,TO_ROOM);
}

void spell_power_rush(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	if(is_affected(ch,sn)){
		ch->send("You are already rushing with power.\n\r");
		return;
	}
	ch->send("You feel power rush through your body!\n\r");
	affect_set(ch,TO_AFFECTS,sn,level,slvl,slvl,APPLY_DAMROLL,slvl * 2,AF_POWER_RUSH);
}

void spell_phantasmal_burst(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	int dam = slvl * level / 2 + get_curr_stat(ch,STAT_RES) - get_curr_stat(victim,STAT_INT);

	if(saves_spell(level + slvl,victim,DAM_HARM))
		dam /= 2;
	else
		dam *= 1.25;

	act("You release a surge of red energy at $N!",ch,NULL,victim,TO_CHAR);
	act("$n releases a surge of red energy at you!",ch,NULL,victim,TO_VICT);
	act("$n releases a surge of red energy at $N!",ch,NULL,victim,TO_NOTVICT);
	damage(ch,victim,dam,sn,DAM_HARM,true);
}

SPELL(spell_volatile_burst){
	CHAR_DATA *victim = (CHAR_DATA*)vo,*vch,*vch_next;
	int dam,tdam;

	if(number_percent() < 25 - sn * 5){
		act("Your spell fizzles and sputters.",ch,NULL,NULL,TO_CHAR);
		act("$n's spell fizzles and sputters.",ch,NULL,NULL,TO_ROOM);
	}

	act("A sparking burst of red energy strikes $N!",ch,NULL,victim,TO_CHAR);
	act("A sparking burst of red energy strikes you!",ch,NULL,victim,TO_VICT);
	act("A sparking burst of red energy strikes $N!",ch,NULL,victim,TO_NOTVICT);

	dam = get_curr_stat(ch,STAT_WIS) + level * sn;
	if(!saves_spell(level,victim,DAM_HARM))
		dam *= 1.5;

	aspell_damage(ch,victim,dam,sn,DAM_HARM,true);

	if(number_percent() < 25 + sn * 5){
		act("The red energy sparks and bursts around $N!",ch,NULL,victim,TO_CHAR);
		act("The red energy sparks and bursts around you!",ch,NULL,victim,TO_VICT);
		act("The red energy sparks and bursts around $N!",ch,NULL,victim,TO_NOTVICT);
		for(vch = victim->in_room->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if(vch == victim || is_same_group(ch,vch))
				continue;
			tdam = dam;
			if(saves_spell(level,vch,DAM_HARM))
				tdam /= 2;
			aspell_damage(ch,vch,tdam,sn,DAM_HARM,true);
		}
	}
}