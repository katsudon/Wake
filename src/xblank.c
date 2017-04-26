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

void do_leadership(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA af;

	if (ch->isaff(AF_LEADERSHIP))
	{
		send_to_char("You are already in the mindset of a leader.\n\r",ch);
		return;
	}
	if(get_leader(ch) != NULL)
	{
		send_to_char("You can't be the leader if you follow someone else!\n\r",ch);
		return;
	}

	if(number_percent() <= get_skill(ch,gsn_leadership) *.9){
		affect_set(ch,TO_AFFECTS,gsn_leadership,ch->level,ch->getslvl(gsn_leadership),ch->level/10,APPLY_INT,4,AF_LEADERSHIP);
		act( "$n seems more leaderific...", ch, NULL, NULL, TO_ROOM );
		send_to_char( "You become a better leader.\n\r", ch);
		check_improve(ch,gsn_leadership,true,4);
	}
	else
	{
		send_to_char("You fail.\n\r",ch);
		check_improve(ch,gsn_leadership,false,2);
	}
	WAIT_STATE(ch,skill_table[gsn_leadership].beats);
    return;
}

bool check_leadership(CHAR_DATA *ch) //This checks if the ch's LEADER has leadership and succeeds a roll
{
	if(get_leader(ch) == NULL || !get_leader(ch)->isaff(AF_LEADERSHIP) || get_skill(get_leader(ch),gsn_leadership) < 1)
		return false;
	
	if(number_percent() <= get_skill(get_leader(ch),gsn_leadership) * .8)
	{
		check_improve(get_leader(ch), gsn_leadership,true,4);
		return true;
	}

	check_improve(get_leader(ch),gsn_leadership,false,1);
	return false;
}

void do_throw(CHAR_DATA *ch,char *argument){
	char arg[MIL],arg2[MIL];
	OBJ_DATA *obj;
	CHAR_DATA *victim;
	int chance,gsn,dt,dam;

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);

	if(arg[0] == '\0' || arg2[0] == '\0'){
	    ch->send("Throw what at who?\n\r");
	    return;
	}

	if((obj = get_obj_wear(ch,arg,true)) == NULL){
	    ch->send("You are not wearing that item.\n\r");
	    return;
	}

	if((victim = get_char_room(ch,NULL,arg2)) == NULL){
		if((victim = get_char_around(ch,arg2,1,false)) == NULL){
			ch->send("You don't see that person here.\n\r");
			return;
		}
	}

	if(obj->item_type != ITEM_THROWINGKNIFE && obj->item_type != ITEM_THROWINGDART && obj->item_type != ITEM_THROWINGAXE){
        ch->send("You must be wielding a throwing weapon.\n\r");
        return;
    }

	switch (obj->item_type){
	case ITEM_THROWINGKNIFE:
		if ((chance = get_skill(ch,gsn_throwingknife)) < 1){
			ch->send("You lack the skill to throw knives.\n\r");
			return;
		}
		gsn = gsn_throwingknife;
		break;
	case ITEM_THROWINGDART:
		if ((chance = get_skill(ch,gsn_throwingdart)) < 1){
			ch->send("You lack the skill to throw darts.\n\r");
			return;
		}
		gsn = gsn_throwingdart;
		break;
	case ITEM_THROWINGAXE:
		if ((chance = get_skill(ch,gsn_throwingaxe)) < 1){
			ch->send("You lack the skill to throw axes.\n\r");
			return;
		}
		gsn = gsn_throwingaxe;
		break;
	}

	act("You throw $p at $N.",ch,obj,victim,TO_CHAR);
	if (ch->in_room != victim->in_room){
		act("$p flies in at you.",NULL,obj,victim,TO_VICT);
		act("$p flies in at $N.",NULL,obj,victim,TO_ROOM);
	}
	else{
		act("$n throws $p at you.",ch,obj,victim,TO_VICT);
		act("$n throws $p at $N.",ch,obj,victim,TO_ROOM);
	}

	if (number_percent() < chance){
		if (obj->value[0]-- == 0){
			ch->send("It's been blunted from overuse.\n\r");
			return;
		}
		dam = dice(obj->value[1],obj->value[2]);
		if (IS_WEAPON_STAT(obj,WPN_SHARP))
			dam *= 1.5;
		check_improve(ch,gsn,true,2);

		chance = number_percent();
		printf_to_char(ch,"%d\n\r",chance);
		if(chance <= 10){
			if(get_eq_char(victim,WEAR_LODGE_LEG) == NULL){
				obj_from_char(obj);
				SET_BIT(obj->wear_loc,WEAR_LODGE_LEG);
				obj_to_char(obj,victim);
				equip_char(victim,obj,WEAR_LODGE_LEG);
				dam = 2 * dam;
			}
			else{
				extract_obj(obj);
				dam = 2.2 * dam;
			}
		}
		else if(chance <= 25){
			if(get_eq_char(victim,WEAR_LODGE_RIB) == NULL){
				obj_from_char(obj);
				SET_BIT(obj->wear_loc,WEAR_LODGE_RIB);
				obj_to_char(obj,victim);
				equip_char(victim,obj,WEAR_LODGE_RIB);
				dam = 3 * dam / 2;
			}
			else{
				extract_obj(obj);
				dam = 3 * dam * .75;
			}
		}
		else if(chance <= 50){
			if(get_eq_char(ch,WEAR_LODGE_ARM) == NULL){
				obj_from_char(obj);
				SET_BIT(obj->wear_loc,WEAR_LODGE_ARM);
				obj_to_char(obj,victim);
				equip_char(victim,obj,WEAR_LODGE_ARM);
			}
			else{
				extract_obj(obj);
				dam *= 1.1;
			}
		}
		else{
			obj_from_char(obj);
			obj_to_room(obj,victim->in_room);
		}
		skill_damage(ch,victim,dam,gsn,attack_table[obj->value[3]].damage,true);
		weapon_effects(ch,victim,obj);
	}
	else{
		obj_from_char(obj);
		obj->wear_loc = WEAR_NONE;
		obj_to_room(obj,victim->in_room);
		skill_damage(ch,victim,0,gsn,attack_table[obj->value[3]].damage,true);
		check_improve(ch,gsn,false,1);
	}
}

void spell_double_strike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if (is_affected(ch,sn)){
		if (victim == ch)
			ch->send("You already have phantom assistance.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level/10,APPLY_NONE,0,AF_DSTRIKE);
    act( "$n's got doublestrike.", victim, NULL, NULL, TO_ROOM );
    victim->send("You feel some doublestrike.\n\r");
}

void spell_call_lightning(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

    dam = dice(level,8);
	if (!saves_spell(level,victim,DAM_LIGHTNING))
		dam *=2;
	spell_damage(ch,victim,dam,sn,DAM_LIGHTNING,true);
}

void spell_change_sex(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int tsex;

	return;
	if ( is_affected( victim, sn )){
		if (victim == ch)
			ch->send("You've already been changed.\n\r");
		else
			act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
		return;
	}
	if (saves_spell(level,victim,DAM_OTHER))
		return;
	do
	{
		tsex  = number_range(0,2) - victim->sex;
	}while (tsex == 0 );

	affect_set(victim,TO_AFFECTS,sn,level,slvl,2 * level,APPLY_SEX,tsex,0);

	victim->send("You feel different.\n\r");
	act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
}

void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) ){
		if (victim == ch)
			ch->send("You are already as strong as you can get!\n\r");
		else
			act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
		return;
    }
	if(!victim->isaff(AF_CHARM) || !is_same_group(ch,victim)){
		ch->send("You have no charmy named that.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_STR,1 + (level >= 18) + (level >= 25) + (level >= 32),0);
    victim->send("Your muscles surge with heightened power!\n\r");
    act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
}

void spell_null(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    ch->send("That's not a spell!\n\r");
    return;
}

void spell_acid_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,hp_dam,dice_dam,hpch;

	act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
	act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

	hpch = UMAX(12,ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = dice(level,16);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

	if (saves_spell(level,victim,DAM_ACID)){
		acid_effect(victim,level/2,dam/4,TARGET_CHAR);
		spell_damage(ch,victim,dam/2,sn,DAM_ACID,true);
	}
	else{
		acid_effect(victim,level,dam,TARGET_CHAR);
		spell_damage(ch,victim,dam,sn,DAM_ACID,true);
	}
}

void spell_fire_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo, *nvir;
    CHAR_DATA *vch;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
    act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX( 10, ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
    fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir){
		nvir = vch->next_in_room;
		if (vch == ch)
			continue;
		if (vch == victim){
			if (saves_spell(level,vch,DAM_FIRE)){
				fire_effect(vch,level/2,dam/4,TARGET_CHAR);
				spell_damage(ch,vch,dam/2,sn,DAM_FIRE,true);
			}
			else{
				fire_effect(vch,level,dam,TARGET_CHAR);
				spell_damage(ch,vch,dam,sn,DAM_FIRE,true);
			}
		}
		else{
			if (saves_spell(level - 2,vch,DAM_FIRE)){
				fire_effect(vch,level/4,dam/8,TARGET_CHAR);
				spell_damage(ch,vch,dam/4,sn,DAM_FIRE,true);
			}
			else{
				fire_effect(vch,level/2,dam/4,TARGET_CHAR);
				spell_damage(ch,vch,dam/2,sn,DAM_FIRE,true);
			}
		}
	}
}

void spell_frost_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo, *nvir;
    CHAR_DATA *vch;
    int dam,hp_dam,dice_dam, hpch;

    act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a freezing cone of frost over you!",
	ch,NULL,victim,TO_VICT);
    act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(12,ch->hit);
    hp_dam = number_range(hpch/11 + 1, hpch/6);
    dice_dam = dice(level,16);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    cold_effect(victim->in_room,level,dam/2,TARGET_ROOM);

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir){
		nvir = vch->next_in_room;
		if (vch == ch)
			continue;
		if (vch == victim){
			if (saves_spell(level,vch,DAM_COLD)){
				cold_effect(vch,level/2,dam/4,TARGET_CHAR);
				spell_damage(ch,vch,dam/2,sn,DAM_COLD,true);
			}
			else{
				cold_effect(vch,level,dam,TARGET_CHAR);
				spell_damage(ch,vch,dam,sn,DAM_COLD,true);
			}
		}
		else{
			if (saves_spell(level - 2,vch,DAM_COLD)){
				cold_effect(vch,level/4,dam/8,TARGET_CHAR);
				spell_damage(ch,vch,dam/4,sn,DAM_COLD,true);
			}
			else{
				cold_effect(vch,level/2,dam/4,TARGET_CHAR);
				spell_damage(ch,vch,dam/2,sn,DAM_COLD,true);
			}
		}
	}
}

void spell_gas_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch, *nvir;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
    act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

    hpch = UMAX(16,ch->hit);
    hp_dam = number_range(hpch/15+1,8);
    dice_dam = dice(level,12);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    poison_effect(ch->in_room,level,dam,TARGET_ROOM);

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir){
		nvir = vch->next_in_room;
		if (vch == ch)
			continue;
		if(is_safe_spell(ch,vch,true))
			break;
		if (ch->fighting == NULL)
			ch->fighting=vch;
		if (saves_spell(level,vch,DAM_POISON)){
			poison_effect(vch,level/2,dam/4,TARGET_CHAR);
			spell_damage(ch,vch,dam/2,sn,DAM_POISON,true);
		}
		else{
			poison_effect(vch,level,dam,TARGET_CHAR);
			spell_damage(ch,vch,dam,sn,DAM_POISON,true);
		}
	}
}

void spell_lightning_breath(int sn, int level, CHAR_DATA *ch, void *vo, int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo, *nvir;
    CHAR_DATA *vch;
    int dam,hp_dam,dice_dam,hpch;

    act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
    act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
    act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

    hpch = UMAX(10,ch->hit);
    hp_dam = number_range(hpch/9+1,hpch/5);
    dice_dam = dice(level,20);

    dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir){
		nvir = vch->next_in_room;
		if (vch == ch)
			continue;
		if(is_safe_spell(ch,vch,true))
			break;
		if (saves_spell(level,vch,DAM_LIGHTNING)){
			shock_effect(vch,level/2,dam/4,TARGET_CHAR);
			spell_damage(ch,vch,dam/2,sn,DAM_LIGHTNING,true);
		}
		else{
			shock_effect(vch,level,dam,TARGET_CHAR);
			spell_damage(ch,vch,dam,sn,DAM_LIGHTNING,true);
		}
	}
}

void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    spell_damage(ch,victim,dam,sn,DAM_PIERCE,true);
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
 
    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    spell_damage(ch,victim,dam,sn,DAM_PIERCE,true);
}

void do_sharpen(CHAR_DATA *ch, char *argument){
	OBJ_DATA *wield = get_obj_wear(ch,argument,true),*stone = get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R);
	AFFECT_DATA af;
	int chance = get_skill(ch,gsn_sharpen) * .9;

	if (!wield || wield->item_type != ITEM_WEAPON){
		ch->send("That is not a weapon.\n\r");
		return;
	}
	if (!stone || stone->item_type != ITEM_HONINGSTONE){
		ch->send("You must be holding a honing stone to sharpen.\n\r");
		return;
	}
	if (!is_weapon_sharp(wield)){
		ch->send("You can only sharpen blades.\n\r");
		return;
	}
	if (stone->value[0] < 1){
		ch->send("This stone is worn beyond use.\n\r");
		return;
	}
	if (wield->value[4]){
		ch->send("This weapon cannot be sharpened.\n\r");
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_sharpen].beats);
	if (number_percent() < chance){
		act("You use $P and run it along the blade of $p until it is sharp.",ch,wield,stone,TO_CHAR);
		act("$n uses $P to sharpen $p.",ch,wield,stone,TO_ROOM);
		stone->value[0]--;
		affect_set(wield,TO_WEAPON,gsn_sharpen,ch->level,ch->getslvl(gsn_sharpen),ch->level / 5,0,0,WPN_SHARP);
		check_improve(ch,gsn_sharpen,true,1);
		return;
	}

	act("You fail to sharpen $p.",ch,wield,NULL,TO_CHAR);
	act("$n fails to sharpen $p.",ch,wield,NULL,TO_ROOM);
	check_improve(ch,gsn_sharpen,false,1);
}

/*void do_torch(CHAR_DATA *ch,char *argument){
	int chance = get_skill(ch,gsn_create_torch);
}*/

void strip_moveskill(CHAR_DATA *ch){
	if(ch->getslvl(gsn_sneak) < 3 && !ch->iscomm(CM_MORPH)){
		affect_strip(ch,gsn_hide);
		ch->remaff(AF_HIDE);
	}
	affect_strip(ch,gsn_fortify);
	ch->remaff(AF_FORTIFY);
}
void spell_burn(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = 3 * (2 * get_curr_stat(ch,STAT_INT) + 3 * get_curr_stat(ch,STAT_WIS)) / 5;
	if (saves_spell(level,victim,DAM_FIRE))
		dam /= 2;
	spell_damage(ch,victim,dam,sn,DAM_FIRE,true);
	return;
}

void spell_shock(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = 3 * (2 * get_curr_stat(ch,STAT_INT) + 3 * get_curr_stat(ch,STAT_WIS)) / 5;
	if (saves_spell(level,victim,DAM_LIGHTNING))
		dam /= 2;
	spell_damage(ch,victim,dam,sn,DAM_LIGHTNING,true);
	return;
}

void spell_venom(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
}

void spell_omega(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,n=0,i=0,s=0;
	bool found = false;

	for(n=1;n < MAX_RES && dam_flags[n].name;n++){
		if(victim->res[n] > 100){
			found = true;
			dam = number_range(level*2,level * 3);
			for (s=1;dam_flags[s].name;s++){
				if(dam_flags[s].settable == n){
					i = dam_flags[n].bit;
					break;
				}
			}
			act("Your omega unleashes a $t attack on $N!",ch,dam_flags[n].name,victim,TO_CHAR);
			act("$n's omega unleashes a $t attack on YOU!",ch,dam_flags[n].name,victim,TO_VICT);
			act("$n's omega unleashes a $t attack on $N!",ch,dam_flags[n].name,victim,TO_NOTVICT);
			spell_damage(ch,victim,dam,sn,i,true);
		}
	}
	if (!found){
		ch->send("Your spell fizzles and flops... Looks like they don't have any weaknesses.\n\r");
		damage(ch,victim,0,sn,DAM_OTHER,false);
	}
}

void spell_force_void(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(victim == ch){
		ch->send("Har har...\n\r");
		return;
	}
	if(victim->isaff(AF_FORCEVOID)){
		ch->send("They are already silenced.\n\r");
		return;
	}
	if (saves_spell(level,victim,DAM_CHARM)){
		ch->send("You failed.\n\r");
		return;
	}

	act("You create a force void within $N.",ch,NULL,victim,TO_CHAR);
	victim->send("The energy inside your body feels heavier.\n\r");
	affect_set(victim,TO_AFFECTS,sn,ch->level,slvl,5,APPLY_INT,-10,AF_FORCEVOID);
}
