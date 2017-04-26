#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"

/*
 * Local functions.
 */
bool		check_poverty			( CHAR_DATA* );


void spell_curse(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	if (target == TARGET_OBJ){
		obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT(obj,ITM_EVIL)){
			act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
			return;
		}

		af.where        = TO_OBJECT;
		af.type         = sn;
		af.level        = level;
		af.duration     = 2 * level;
		af.location     = APPLY_SAVES;
		af.modifier     = +1;
		af.bitvector    = ITM_EVIL;
		affect_to_obj(obj,&af);

		act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);

		if (obj->wear_loc != WEAR_NONE)
			ch->saving_spell_throw += 1;
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (victim->isaff(AF_CURSE) || saves_spell(level,victim,DAM_NEGATIVE))
		return;
	affect_set(victim,TO_AFFECTS,sn,level,slvl,2*level,APPLY_HITROLL,-1 * (level / 8),AF_CURSE);

	affect_set(victim,TO_AFFECTS,sn,level,slvl,2*level,APPLY_SAVING_SPELL,level / 8,AF_CURSE);

	send_to_char("You feel unclean.\n\r",victim);
	if ( ch != victim )
		act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
}

void spell_demonfire(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (victim != ch){
		act("$n calls forth the demons of Hell upon $N!",ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the demons of Hell!",ch,NULL,victim,TO_VICT);
		send_to_char("You conjure forth the demons of hell!\n\r",ch);
	}
	dam = dice(level * 0.75,slvl);
	if(check_poverty(ch))
		dam *= 1.5;
	if (saves_spell(level,victim,DAM_NEGATIVE))
		dam /= 2;
	spell_damage(ch,victim,dam,sn,DAM_NEGATIVE,true);
	spell_curse(gsn_curse,3 * level / 4,ch,(void *) victim,TARGET_CHAR,slvl);
}

void spell_weaken(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || saves_spell(level,victim,DAM_OTHER))
		return;

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level / 2,APPLY_STR,-1 * (level / 5),AF_WEAKEN);
	send_to_char("You feel your strength slip away.\n\r",victim);
	act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
}

void spell_scourge(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;
	CHAR_DATA *vch,*vch_next;
	int dam,dt = DAM_NONE;

	dam = 100;
	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch))
			continue;
		if(ch->alignment >0){
			dt = DAM_NEGATIVE;
			dam = 2 * get_curr_stat(ch,STAT_FTH) + ((ch->level / 5) * slvl);
			dam = number_range(dam,dam * 1.5);
			//nash needs more affects
		}
		else if(ch->alignment <0){
			dt = DAM_HOLY;
			if (!is_affected(vch,sn) && !saves_spell(level,vch,DAM_OTHER))
				affect_set(vch,TO_AFFECTS,sn,level,slvl,level / 2,APPLY_DAMROLL,-1 * slvl,AF_WEAKEN);
			if (!is_affected(vch,sn) && !saves_spell(level,vch,DAM_OTHER)){
				affect_set(vch,TO_AFFECTS,sn,level,slvl,level / 2,APPLY_SAVES,-1 * slvl,AF_CURSE);
				affect_set(vch,TO_AFFECTS,sn,level,slvl,level / 2,APPLY_LCK,-1 * slvl,AF_CURSE);
			}
			if (!is_affected(vch,sn) && !saves_spell(level,vch,DAM_OTHER)){
				affect_set(vch,TO_AFFECTS,sn,level,slvl,level / 2,APPLY_STR,-1 * slvl,AF_POISON);
			}
			dam = 2 * get_curr_stat(ch,STAT_FTH) + ((ch->level / 10) * slvl);
			dam = number_range(dam,dam * .75);
		}
		else
			ch->send("You aren't aligned enough to do anything cool.\n\r");
		spell_damage(ch,vch,dam,sn,dt,true);
	}
}

int check_sacred_guardian(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	int sn = gsn_sacred_guardian,skill = get_skill(ch,sn);

	if(ch->alignment == 0)
		return dam;
	if(ch->alignment > 0){
		if(victim->alignment >= 0)
			return dam;
	}
	if(ch->alignment < 0){
		if(victim->alignment <= 0)
			return dam;
	}

	if(skill > 0){
		if(roll_chance(victim,skill/2)){
			victim->send("Your holy fervor strengthens your spiritual resolve.\n\r");
			dam -= dam * ch->getslvl(sn) / 10;
			check_improve(victim,sn,true,1);
		}
		else
			check_improve(victim,sn,false,1);
	}
	return dam;
}

int check_dogma(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	int sn = gsn_dogma,skill = get_skill(ch,sn);

	if(skill < 1)
		return dam;

	if(ch->alignment == 0)
		return dam;
	if(ch->alignment > 0){
		if(victim->alignment >= 0)
			return dam;
	}
	if(ch->alignment < 0){
		if(victim->alignment <= 0)
			return dam;
	}

	if(roll_chance(ch,skill/2)){
		ch->send("Your holy fervor strengthens your power.\n\r");
		dam += dam * ch->getslvl(sn) / 10;
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	return dam;
}

int check_faith(CHAR_DATA *ch,int save){
	if(get_skill(ch,gsn_faith) < 1)
		return save;
	if(roll_chance(ch,get_skill(ch,gsn_faith))){
		save -= ch->pcdata->skill_level[gsn_faith] * 2;
		check_improve(ch,gsn_faith,true,1);
	}
	else
		check_improve(ch,gsn_faith,false,1);
	return save;
}

void spell_quench(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	if(IS_NPC(ch) || ch->pcdata->condition[COND_THIRST] > 40){
		ch->send("You have no thirst to sate.\n\r");
		return;
	}
	gain_condition(ch,COND_THIRST,ch->getslvl(sn) * 4);
	if(ch->pcdata->condition[COND_THIRST] > 40)
		ch->send("Your thirst is quenched.\n\r");
	else
		ch->send("You are less thirsty.\n\r");
}
