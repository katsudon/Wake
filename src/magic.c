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
void		say_spell				( CHAR_DATA*,int				);
int			check_faith				( CHAR_DATA*,int				);
bool		check_poverty			( CHAR_DATA*					);
bool		remove_obj				( CHAR_DATA*,int,bool			);
void		wear_obj				( CHAR_DATA*,OBJ_DATA*,bool,bool);
int			check_spell_resistance	( CHAR_DATA*,CHAR_DATA*,int		);
int			spell_proficiency		( CHAR_DATA*,int,int			);
bool		check_charm_purge		( CHAR_DATA*,CHAR_DATA*,int		);
int			check_dogma				( CHAR_DATA*,CHAR_DATA*,int		);
int			check_sacred_guardian	( CHAR_DATA*,CHAR_DATA*,int		);
int			magical_mastery			( CHAR_DATA*,int );
int			elemental_mastery		( CHAR_DATA*,int );
bool	check_resist_blights		( CHAR_DATA* );
bool	occult_wisdom				( CHAR_DATA* );


bool check_spellcraft(CHAR_DATA *ch){
	int sn = gsn_spellcraft,skill = get_skill(ch,sn);

	if(skill < 1)
		return false;
	skill /= 4;
	skill += get_curr_stat(ch,STAT_INT) / 3;

	printf_to_char(ch,"%d\n\r",skill);
	if(number_percent() < skill){
		check_improve(ch,sn,true,1);
		return true;
	}
	else
		check_improve(ch,sn,false,1);
	return false;

}

int do_spellcraft(CHAR_DATA *ch,int dam){
	if(!check_spellcraft(ch))
		return dam;

	dam += (get_curr_stat(ch,STAT_INT)/2) * ch->getslvl(gsn_spellcraft);

	return dam;
}

void aspell_damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,bool show){
	CHAR_DATA *vch;
	bool found = false;
	int tries,sn;

	if (victim->position > POS_MORTAL && dam > 1){
		dam = check_dogma(ch,victim,dam);
		dam = check_sacred_guardian(ch,victim,dam);
		if(dam_type == DAM_NEGATIVE && victim->in_room->israf(RAF_NEGATIVE)){
			dam += (dam / 8) * affect_find(victim->in_room->affected,RAF_NEGATIVE)->duration;
			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ANeg({R%d{d) ",dam);
		}//Nash check stat penalty thing           ALSO make this shit be CALLED
		dam += dam * get_curr_stat(victim,STAT_END) / (STAT_MAX * 2);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"AStats({R%d{d)\n\r",dam);

		dam = (dam * victim->res[RS_MAGIC]) / 100;
	}
	damage(ch,victim,dam,dt,dam_type,show);
}

void spell_damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,bool show){
	CHAR_DATA *vch,*vch_next;
	AFFECT_DATA *af;
	bool found = false;
	int tries,sn;

	if (victim->position > POS_MORTAL && dam > 1){
		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SBse({R%d{d) ",dam);

		dam = spell_proficiency(ch,dam,SPL_DAM);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SPrf({R%d{d) ",dam);

		if(!IS_NPC(victim) && victim != ch && dam > 1 && skill_table[dt].slot == SKILL_SPELL && check_poverty(victim))//Is the victim poor? Cuz their god has some damreduction to dish out
			dam *= .75;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SPov({R%d{d) ",dam);

		if(ch->isaff(AF_OVERDRIVE))
			dam *= 2;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SOvr({R%d{d)\n\r",dam);

		if(ch->isaff(AF_DULLSPELL))
			dam /= 2;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SDul({R%d{d)\n\r",dam);

		if(dam_type == DAM_ENERGY || dam_type == DAM_LIGHT)
			dam = magical_mastery(ch,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SMgm({R%d{d)\n\r",dam);
		if(dam_type == DAM_FIRE || dam_type == DAM_COLD || dam_type == DAM_LIGHTNING || dam_type == DAM_WATER || dam_type == DAM_WIND || dam_type == DAM_EARTH)
			dam = elemental_mastery(ch,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SElm({R%d{d)\n\r",dam);

		if(victim->alignment > 0 && victim->in_room->israf(RAF_REGENERATION))
			dam *= .9;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SREGEN({R%d{d)\n\r",dam);

		dam = check_spell_resistance(ch,victim,dam);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SRes({R%d{d)\n\r",dam);
		if(victim->in_room->israf(RAF_MAGIC_DAMPEN)){
			if((af = affect_find(victim->in_room->affected,skill_lookup("dampening ward")))){
				if(number_range(0,5) < af->modifier){
					act("Your spell fizzles and dissipates!",ch,NULL,NULL,TO_CHAR);
					act("$n's spell fizzles and dissipates!",ch,NULL,NULL,TO_ROOM);
				}
				else
					dam -= dam / 10 * af->slvl;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SSeal({R%d{d)\n\r",dam);
			}
			else
				dam *= 0.9;
		}

		if(victim->getslvl(gsn_spell_resistance) == 5 && number_percent() < 10)
			dam = 0;if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SImm({R%d{d)\n\r",dam);

		dam += dam * get_curr_stat(victim,STAT_RES) / (STAT_MAX * 2);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"SStats({R%d{d)\n\r",dam);

		dam = (dam * victim->res[RS_MAGIC]) / 100;
		if (dam > 0 && !IS_NPC(victim) && get_skill(victim,gsn_feedback) > 1){//Nashfix
			if(number_percent() <= get_skill(victim,gsn_feedback) / 5){
				check_improve(ch,gsn_feedback,true,1);
				act("Your mana repulsion sends $N's spell back at $M!",victim,NULL,ch,TO_CHAR);
				act("The mana repulsion in $n's body sends $N's spell back at $M!",victim,NULL,ch,TO_NOTVICT);
				act("The mana repulsion in $n's body sends your spell back at you!",victim,NULL,ch,TO_VICT);
				spell_damage(victim,ch,dam/10 * victim->getslvl(gsn_feedback),find_spell(ch,"feedback"),DAM_OTHER,true);
				return;
			}
		}
		if (dam > 0 && victim->isaff(AF_SPELLSHIELD)){
			if(skill_table[dt].slot == SKILL_SPELL && (af = affect_find(victim->affected,gsn_spellshield)) && number_percent() < af->modifier){
				if(number_percent() < af->slvl * 10){
					act("$N's spell shield deflects $n's spell!",ch,NULL,victim,TO_NOTVICT);
					act("$N's spell shield deflects your spell!",ch,NULL,victim,TO_CHAR);
					act("Your spell shield deflects $n's spell!",ch,NULL,victim,TO_VICT);
					ch->modmana(-1);
					if ((vch = get_random_char(victim,true)) && number_percent() < 25){
						found = true;
						spell_damage(victim,vch,dam,sn,dam_type,true);
					}
					if(!found){
						act("The spell shoots off into the sky.",victim,NULL,NULL,TO_ROOM);
						act("The spell shoots off into the sky.",victim,NULL,NULL,TO_CHAR);
					}
				}
				else
					dam -= dam / 10 * af->slvl;
			}
		}
		if (dam > 0 && victim->isaff(AF_DISPERSION_FIELD)){
			if(skill_table[dt].slot == SKILL_SPELL && (af = affect_find(victim->affected,gsn_dispersion_field)) && number_percent() < af->modifier){
				if(number_percent() < af->slvl * 8){
					act("$n's spell disperses around $N!",ch,NULL,victim,TO_NOTVICT);
					act("Your spell disperses around $N!",ch,NULL,victim,TO_CHAR);
					act("$n's spell disperses around you!",ch,NULL,victim,TO_VICT);
					for(vch = ch->in_room->people;vch && dam > af->slvl * 2;vch = vch_next){
						vch_next = vch->next_in_room;
						if(saves_spell(af->level+af->slvl*2,vch,DAM_HARM))
							continue;
						dam -= af->slvl * 2;
						damage(ch,vch,af->slvl * 5,DAM_HARM,gsn_dispersion_field,true);
					}
				}
			}
		}
	}
	damage(ch,victim,dam,dt,dam_type,show);
}

int check_spell_resistance(CHAR_DATA *ch,CHAR_DATA *victim,int dam){
	int chance = get_skill(victim,gsn_spell_resistance)*.4,sres = get_curr_stat(victim,STAT_RES);

	if (chance < 1)
		return dam;
	chance = chance * get_curr_stat(victim,STAT_RES) / (STAT_MAX * .75);

	if (roll_chance(victim,chance)){
		victim->send("Your resiliance resiliates!\n\r");
		dam = (dam * (90 - (number_range(sres,sres/4)/2))) / 100;
	}
	return dam;
}

void say_spell(CHAR_DATA *ch,int sn){
	char buf[MSL],buf2[MSL],buf3[MSL];
	CHAR_DATA *rch;
	char *pName;
	int iSyl, length, cclass = get_class(ch);

	struct syl_type{
		char *	old;
		char *	ne;
	};

	static const struct syl_type syl_table[] ={
		{"",""}
	};

	buf[0] = '\0';
	for ( pName = skill_table[sn].name; *pName != '\0'; pName += length ){
		for (iSyl = 0;(length = strlen_color(syl_table[iSyl].old)) != 0;iSyl++)
			if (!str_prefix(syl_table[iSyl].old,pName)){
				strcat(buf,syl_table[iSyl].ne);
				break;
			}

		if (length == 0)
			length = 1;
	}

	sprintf(buf2,classes[cclass].msg_other,skill_table[sn].cst_prs);
	sprintf(buf, classes[cclass].msg_other,skill_table[sn].name);
	sprintf(buf3,classes[cclass].msg_self, skill_table[sn].name);
	strcat(buf3,"\n\r");
	for (rch = ch->in_room->people;rch;rch = rch->next_in_room)
		if (rch != ch)// && cclass != CLASS_SHAMAN )
			act((!IS_NPC(rch) && (cclass == get_class(rch)) || IS_IMMORTAL(rch)) ? buf : buf2, ch, NULL, rch, TO_VICT );
	send_to_char(buf3, ch);
}

int get_save_spell(int level,CHAR_DATA *victim,int dam_type){
	//int save = 50 + (victim->level - level) + victim->saving_spell_throw * 2;
		int save = victim->level - level + victim->saving_spell_throw * 2;
	switch(check_immune(victim,dam_type)){
		case IS_IMMUNE:		return true;
		case IS_RESISTANT:	save += 5;	break;
		case IS_VULNERABLE:	save -= 5;	break;
	}

	if (!IS_NPC(victim) && classes[victim->pclass].fMana > 0)//nashneedstofixthis
		save = ((4 + classes[victim->pclass].fMana) * save / 10);

	save -= 10 * get_curr_stat(victim,STAT_RES) / STAT_MAX;//what
	save = URANGE(5,save,95);

	if(!IS_NPC(victim)){
		save += get_curr_stat(victim,STAT_LCK) * .75;
		save = check_faith(victim,save);
	}
}
bool saves_spell(int level,CHAR_DATA *victim,int dam_type){//True = spell sucks
	int save = get_save_spell(level,victim,dam_type);

	if (!IS_NPC(victim)){
		if(dam_type == DAM_ENERGY && occult_wisdom(victim))
			return true;
		if(dam_type == DAM_POISON || dam_type == DAM_DISEASE && check_resist_blights(victim))
			return true;
	}
	return number_percent() < save;
}

int get_save_skill(int level,CHAR_DATA *victim,int dam_type){
	int save = victim->level - level + victim->saving_throw * 2;

	if (victim->isaff(AF_BERSERK))
		save += UMAX(1,victim->level/10);

	switch(check_immune(victim,dam_type)){
		case IS_IMMUNE:		return true;
		case IS_RESISTANT:	save += 2;	break;
		case IS_VULNERABLE:	save -= 2;	break;
	}

	save = URANGE(5,save,95);
	if (!IS_NPC(victim))
		save += get_curr_stat(victim,STAT_LCK) * .75;
}
bool saves_skill(int level,CHAR_DATA *victim,int dam_type){
	int save = get_save_skill(level,victim,dam_type);
	return number_percent() < save;
}

bool saves_dispel( int dis_level, int spell_level, int duration)
{
    int save;
    
    if (duration == -1)
      spell_level += 5;

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

bool check_dispel(int dis_level, CHAR_DATA *victim, int sn){
	AFFECT_DATA *af;

	if (is_affected(victim,sn)){
		for ( af = victim->affected; af != NULL; af = af->next ){
			if ( af->type == sn ){
				if (!saves_dispel(dis_level,af->level,af->duration)){
					affect_strip(victim,sn);
	        		if ( skill_table[sn].msg_off ){
			    		send_to_char( skill_table[sn].msg_off, victim );
						send_to_char( "\n\r", victim );
        			}
					return true;
				}
				else
					af->level--;
			}
		}
	}
	return false;
}

int mana_cost (CHAR_DATA *ch, int min_mana, int level){
    if (ch->level + 2 == level)
		return 1000;
    return UMAX(min_mana,(100/(2 + ch->level - level)));
}

char *target_name;

void do_cast(CHAR_DATA *ch,char *argument){
	void *vo = NULL;
	char arg1[MIL],arg2[MIL];
	int mana,sn,target = TARGET_NONE;
	CHAR_DATA *victim = NULL;
	OBJ_DATA *obj = NULL;

	if (IS_NPC(ch) && !ch->desc)
		return;

	target_name = one_argument(argument,arg1);
	one_argument(target_name,arg2);

	if (!arg1[0]){
		ch->send("Cast which what where?\n\r");
		return;
	}
	if ((sn = find_spell(ch,arg1)) < 1 || skill_table[sn].spell_fun == spell_null || (!IS_NPC(ch) && (ch->level < skill_table[sn].skill_level[ch->pclass] || ch->pcdata->learned[sn] < 1))){
		ch->send("You don't know any spells of that name.\n\r");
		return;
	}
	if (ch->position < skill_table[sn].minimum_position){
		send_to_char("You can't concentrate enough.\n\r",ch);
		printf_to_char(ch,"%d %d\n\r",ch->position,skill_table[sn].minimum_position);
		return;
	}
	if (ch->isaff(AF_SILENCE)){
		ch->send("You try to cast, but not even a sound comes out.\n\r");
		return;
	}

	mana = skill_table[sn].min_mana;

	switch(skill_table[sn].target){
		default:
			bug("Do_cast: bad target for sn %d.",sn);
			return;
		case TAR_IGNORE:
			break;
		case TAR_CHAR_OFFENSIVE:
			if (!target_name[0]){
				if(!ch->fighting){
					send_to_char("Cast the spell on whom?\n\r",ch);
					return;
				}
				else
					victim = ch->fighting;
			}
			else if(!(victim = get_char_room(ch,NULL,target_name))){
				send_to_char("They aren't here.\n\r",ch);
				return;
			}
			if (!canFight(ch,victim,true))
					return;
			if(ch->isaff(AF_CHARM) && ch->master == victim){
				send_to_char("You can't do that to your owner.\n\r",ch);
				return;
			}
			vo = (void *) victim;
			target = TARGET_CHAR;
			break;
		case TAR_CHAR_DEFENSIVE:
			if(!arg2[0])
				victim = ch;
			else if(!(victim = get_char_room(ch,NULL,target_name))){
				send_to_char("They aren't here.\n\r",ch);
				return;
			}
			vo = (void *) victim;
			target = TARGET_CHAR;
			break;
		case TAR_CHAR_WORLD:
			if(!arg2[0])
				victim = ch;
			else if(!(victim = get_char_world(ch,target_name))){
				send_to_char("They aren't anywhere.\n\r",ch);
				return;
			}
			vo = (void *) victim;
			target = TARGET_CHAR;
			break;
		case TAR_CHAR_SELF:
			if ( arg2[0] != '\0' && !is_name(target_name,ch->name) ){
				send_to_char("You cannot cast this spell on another.\n\r",ch);
				return;
			}
			vo = (void *) ch;
			target = TARGET_CHAR;
			break;
		case TAR_CHAR_IGNORE:
			if(target_name[0]){
				if(!(victim = get_char_room(ch,NULL,target_name))){
					send_to_char("They aren't here.\n\r",ch);
					return;
					}
				if (!canFight(ch,victim,true))
						return;
				if(ch->isaff(AF_CHARM) && ch->master == victim){
					send_to_char("You can't do that to your owner.\n\r",ch);
					return;
				}
				vo = (void *) victim;
				target = TARGET_CHAR;
			}
			else{
				vo = NULL;
				target = TARGET_NONE;
			}
			break;
		case TAR_OBJ_INV:
			if(!arg2[0]){
				send_to_char("What should the spell be cast upon?\n\r",ch);
				return;
			}
			if(!(obj = get_obj_carry(ch,target_name,ch))){
				send_to_char("You are not carrying that.\n\r",ch);
				return;
			}
			vo = (void *) obj;
			target = TARGET_OBJ;
			break;
		case TAR_OBJ_EQ:
			if(!arg2[0]){
				send_to_char("What should the spell be cast upon?\n\r",ch);
				return;
			}
			if(!(obj = get_obj_wear(ch,target_name,true))){
				send_to_char("You are not wearing that.\n\r",ch);
				return;
			}
			vo = (void *) obj;
			target = TARGET_OBJ;
			break;
		case TAR_OBJ_CHAR_OFF:
			if(!arg2[0]){
				if ((victim = ch->fighting) == NULL){
					send_to_char("Cast the spell on whom or what?\n\r",ch);
					return;
				}
				target = TARGET_CHAR;
			}
			else if((victim = get_char_room(ch, NULL,target_name))){
				target = TARGET_CHAR;
			}
			if (victim && !canFight(ch,victim,true))
				return;
			if (target == TARGET_CHAR) /* check the sanity of the attack */{
				if(is_safe_spell(ch,victim,false) && victim != ch){
					send_to_char("Not on that target.\n\r",ch);
					return;
				}
				if (ch->isaff(AF_CHARM) && ch->master == victim ){
					send_to_char("You can't do that on your own follower.\n\r",ch);
					return;
				}
				//if (!IS_NPC(ch))
				//	check_killer(ch,victim);
				vo = (void *) victim;
 			}
			else if((obj = get_obj_here(ch, NULL,target_name))){
				vo = (void *) obj;
				target = TARGET_OBJ;
			}
			else{
				send_to_char("You don't see that here.\n\r",ch);
				return;
			}
			break;
		case TAR_OBJ_CHAR_DEF:
			if(!arg2[0]){
				vo = (void *) ch;
				target = TARGET_CHAR;
			}
			else if((victim = get_char_room(ch, NULL,target_name))){
				vo = (void *) victim;
				target = TARGET_CHAR;
			}
			else if((obj = get_obj_carry(ch,target_name,ch))){
				vo = (void *) obj;
				target = TARGET_OBJ;
			}
			else{
				send_to_char("You don't see that here.\n\r",ch);
				return;
			}
			break;
		case TAR_DOOR:
				 if(!str_prefix(target_name,"north"))	target = 0;
			else if(!str_prefix(target_name,"east"))	target = 1;
			else if(!str_prefix(target_name,"south"))	target = 2;
			else if(!str_prefix(target_name,"west"))	target = 3;
			else if(!str_prefix(target_name,"up"))		target = 4;
			else if(!str_prefix(target_name,"down"))	target = 5;
			else{
				ch->send("That is not a valid direction.\n\r");
				return;
			}
			break;
	}

	if ( !IS_NPC(ch) && ch->getmana() < mana){
		send_to_char("You don't have enough mana.\n\r",ch);
		return;
	}

	mana = check_piety(ch,mana,PIETY_MANA);

	if (str_cmp(skill_table[sn].name,"ventriloquate"))
		say_spell(ch,sn);

	ch->send("You begin to cast...\n\r");
	if((number_percent() > grab_skill(ch,sn)) && !check_spellcraft(ch))
		ch->spellfailed = UMAX(1,number_range(1,skill_table[sn].beats));
	if(ch->getslvl(sn) < 1){
		ch->spellslvl = 1;
		log_f("Invalid ch/spell: %s: %d",ch->name,skill_table[sn].name);
	}
	else
		ch->spellslvl = ch->getslvl(sn);
	ch->spelltimer = skill_table[sn].beats * 2;
	ch->spellsn = sn;
	ch->spellvo = vo;
	ch->spelltarget = target;
	ch->spellvictim = victim;
	ch->spellcost = mana;
}

void obj_cast_spell(int sn,int level,CHAR_DATA *ch,CHAR_DATA *victim,OBJ_DATA *obj){
	void *vo;
	int target = TARGET_NONE,slvl = 1;

	if(sn <= 0)
		return;

	if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 ){
		bug("Obj_cast_spell: bad sn %d.",sn);
		return;
	}

	switch ( skill_table[sn].target ){
		default:
			bug( "Obj_cast_spell: bad target for sn %d.", sn );
			return;
		case TAR_IGNORE:
			vo = NULL;
			break;
		case TAR_CHAR_OFFENSIVE:
			if(!victim)
				victim = ch->fighting;
			if(!victim){
				send_to_char("You can't do that.\n\r",ch);
				return;
			}
			if (is_safe(ch,victim) && ch != victim){
				send_to_char("Something isn't right...\n\r",ch);
				return;
			}
			vo = (void *) victim;
			target = TARGET_CHAR;
			break;
		case TAR_CHAR_DEFENSIVE:
		case TAR_CHAR_SELF:
			if (victim == NULL)
				victim = ch;
			vo = (void *) victim;
			target = TARGET_CHAR;
			break;
		case TAR_OBJ_INV:
			if (!obj){
				send_to_char("You can't do that.\n\r",ch);
				return;
			}
			vo = (void *) obj;
			target = TARGET_OBJ;
			break;
		case TAR_OBJ_EQ:
			if (!obj){
				send_to_char("You can't do that.\n\r",ch);
				return;
			}
			vo = (void *) obj;
			target = TARGET_OBJ;
			break;
		case TAR_OBJ_CHAR_OFF:
			if ( victim == NULL && obj == NULL){
				if (ch->fighting != NULL)
					victim = ch->fighting;
				else{
					send_to_char("You can't do that.\n\r",ch);
					return;
				}
			}
			if (victim != NULL){
				if (is_safe_spell(ch,victim,false) && ch != victim){
					send_to_char("Something isn't right...\n\r",ch);
					return;
				}
				vo = (void *) victim;
				target = TARGET_CHAR;
			}
			else{
	    		vo = (void *) obj;
	    		target = TARGET_OBJ;
			}
			break;
		case TAR_OBJ_CHAR_DEF:
			if (victim == NULL && obj == NULL){
				vo = (void *) ch;
				target = TARGET_CHAR;
			}
			else if (victim != NULL){
				vo = (void *) victim;
				target = TARGET_CHAR;
			}
			else{
				vo = (void *) obj;
				target = TARGET_OBJ;
			}
			break;
	}
	target_name = "";
	if(level > 50)
		slvl = 2;
	else
		slvl = 1;
	(*skill_table[sn].spell_fun) (sn,level,ch,vo,target,slvl);

	if ( (skill_table[sn].target == TAR_CHAR_OFFENSIVE || (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR)) && victim != ch && victim->master != ch ){
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for ( vch = ch->in_room->people; vch; vch = vch_next ){
			vch_next = vch->next_in_room;
			if ( victim == vch && victim->fighting == NULL ){
				//check_killer(victim,ch);
				multi_hit(victim,ch,TYPE_UNDEFINED,false);
				break;
			}
		}
	}
}

void spell_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->isaff(AF_BLIND) || saves_spell(level,victim,DAM_OTHER))
		return;

    affect_set(victim,TO_AFFECTS,sn,level,slvl,level/2,APPLY_HITROLL,-4,AF_BLIND);
    send_to_char("You are blinded!\n\r",victim);
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
    return;
}

void spell_cancellation(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool found = false;

    level += 2;

    if ((!IS_NPC(ch) && IS_NPC(victim) && !(ch->isaff(AF_CHARM) && ch->master == victim) ) || (IS_NPC(ch) && !IS_NPC(victim)) )
    {
		send_to_char("You failed, try dispel magic.\n\r",ch);
		return;
    }

    if (check_dispel(level,victim,skill_lookup("armor")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("bless")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("blindness")))
    {
        found = true;
        act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("calm")))
    {
		found = true;
		act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("change sex")))
    {
        found = true;
        act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_charm_purge(ch,victim,level)){
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("agues echo")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("chill touch")))
    {
        found = true;
        act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
    }

    if (check_dispel(level,victim,skill_lookup("curse")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("detect evil")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("detect good")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("detect hidden")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("detect invis")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("detect magic")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("faerie fire")))
    {
        act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("fly")))
    {
        act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("frenzy")))
    {
		act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
		found = true;
    }

    if (check_dispel(level,victim,skill_lookup("giant strength")))
    {
        act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("haste")))
    {
		act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
		found = true;
    }

    if (check_dispel(level,victim,skill_lookup("infravision")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("mass invis")))
    {
        act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("pass door")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("sanctuary")))
    {
        act("The white aura around $n's body vanishes.", victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("shield")))
    {
        act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("sleep")))
        found = true;

    if (check_dispel(level,victim,skill_lookup("slow")))
    {
        act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("stone skin")))
    {
        act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (check_dispel(level,victim,skill_lookup("weaken")))
    {
        act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
        found = true;
    }

    if (found)
        send_to_char("Ok.\n\r",ch);
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA*)vo,*tmp_vict,*last_vict,*next_vict;
    bool found;
    int dam;

    /* first strike */

    act("A lightning bolt leaps from $n's hand and arcs to $N.", ch,NULL,victim,TO_ROOM);
    act("A lightning bolt leaps from your hand and arcs to $N.", ch,NULL,victim,TO_CHAR);
    act("A lightning bolt leaps from $n's hand and hits you!", ch,NULL,victim,TO_VICT);

    dam = dice(level,6);
    if (saves_spell(level,victim,DAM_LIGHTNING))
 		dam /= 3;
    spell_damage(ch,victim,dam,sn,DAM_LIGHTNING,true);
    last_vict = victim;
    level -= 4;

    while (level > 0){
		found = false;
		for (tmp_vict = ch->in_room->people; 
			 tmp_vict != NULL;
			 tmp_vict = next_vict) 
		{
		  next_vict = tmp_vict->next_in_room;
			if (!is_safe_spell(ch,tmp_vict,true) && tmp_vict != last_vict)
			{
				found = true;
				last_vict = tmp_vict;
				act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
				act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
				dam = dice(level,6);
				if (saves_spell(level,tmp_vict,DAM_LIGHTNING))
					dam /= 3;
				spell_damage(ch,tmp_vict,dam,sn,DAM_LIGHTNING,true);
				level -= 4;
			}
		}
		
		if (!found)
		{
			if (ch == NULL)
				return;

			if (last_vict == ch) /* no double hits */
			{
				act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
				act("The bolt grounds out through your body.",ch,NULL,NULL,TO_CHAR);
				return;
			}

			last_vict = ch;
			act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
			send_to_char("You are struck by your own lightning!\n\r",ch);
			dam = dice(level,6);
			if (saves_spell(level,ch,DAM_LIGHTNING))
				dam /= 3;
			spell_damage(ch,ch,dam,sn,DAM_LIGHTNING,true);
			level -= 4;  /* decrement damage */
			if (ch == NULL) 
				return;
		}
    }
}

void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *light;

	if (target_name[0]){/* do a glow on some object */
		light = get_obj_carry(ch,target_name,ch);

		if (!light){
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}

		if (IS_OBJ_STAT(light,ITM_GLOW)){
			act("$p is already glowing.",ch,light,NULL,TO_CHAR);
			return;
		}

		SET_BIT(light->extra_flags,ITM_GLOW);
		act("$p glows with a white light.",ch,light,NULL,TO_ALL);
		return;
	}

	light = create_object(get_obj_index(OBJ_VNUM_LIGHT_BALL),0);
	obj_to_room(light,ch->in_room);
	light->value[2] = slvl * 20;
	light->timer = slvl * 40;
	act("$n twiddles $s thumbs and $p appears.",ch,light,NULL,TO_ROOM);
	act("You twiddle your thumbs and $p appears.",ch,light,NULL,TO_CHAR);
}

void spell_continual_dark(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *light;

	if (target_name[0])  /* do a glow on some object */{
		light = get_obj_carry(ch,target_name,ch);

		if (!light){
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}

		if (!IS_OBJ_STAT(light,ITM_GLOW)){
			act("$p is not glowing.",ch,light,NULL,TO_CHAR);
			return;
		}

		REMOVE_BIT(light->extra_flags,ITM_GLOW);
		act("$p fades and loses its glow.",ch,light,NULL,TO_ALL);
		return;
	}

	light = create_object(get_obj_index(OBJ_VNUM_DARK_BALL),0);
	obj_to_room(light,ch->in_room);
	light->timer = 10;
	act("$n wiggles $s thumbs and $p appears.",ch,light,NULL,TO_ROOM);
	act("You wiggle your thumbs and $p appears.",ch,light,NULL,TO_CHAR);
}

void spell_create_food(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    OBJ_DATA *mushroom;
	int n = number_range(1,slvl);

    mushroom = create_object(get_obj_index(OBJ_VNUM_MUSHROOM),0);
	mushroom->value[0] = n * 8;
	mushroom->value[1] = 15 - (n * 2);
	mushroom->timer = slvl * 10;
	switch(n){
	case 1:
		break;
	case 2:
		mushroom->short_descr = str_dup("a {Rr{Ya{Gi{Bn{bb{mo{Mw colored {Rapple{x");
		mushroom->description = str_dup("A fat, chromatic apple is here... spinning?");
		mushroom->name = str_dup("chromatic rainbow apple");
		break;
	case 3:
		mushroom->short_descr = str_dup("a glowing, {Bs{Gp{Bott{Ge{Bd blue {xturnip");
		mushroom->description = str_dup("This blue turnip is covered in green spots and seems luminescent.");
		mushroom->name = str_dup("blue spotted turnip");
		break;
	case 4:
		mushroom->short_descr = str_dup("an {Ce{ct{Ch{ce{Cr{ce{Ca{cl {xpotato");
		mushroom->description = str_dup("Mmm... potato from the ethereal plane.");
		mushroom->name = str_dup("ethereal potato");
		break;
	case 5:
		mushroom->short_descr = str_dup("a ball of {Rcherry {xflavored {Rm{Ya{Bg{Gi{Cc {xmi{Cs{ct{x");
		mushroom->description = str_dup("A humming ball of multi colored magic floats here looking... tasty?");
		mushroom->name = str_dup("cherry magic mist ball");
		break;
	default:
		ch->send("What the crap?\n\r");
		return;
	}
    obj_to_room(mushroom,ch->in_room);
    act("$p suddenly appears.",ch,mushroom,NULL,TO_ROOM);
    act("$p suddenly appears.",ch,mushroom,NULL,TO_CHAR);
}

void spell_create_rose(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    OBJ_DATA *rose;
    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
    send_to_char("You create a beautiful red rose.\n\r",ch);
    obj_to_char(rose,ch);
    return;
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    OBJ_DATA *spring;

    spring = create_object(get_obj_index(OBJ_VNUM_SPRING),0);
    spring->timer = level;
    obj_to_room(spring,ch->in_room);
    act("$p flows from the ground.",ch,spring,NULL,TO_ROOM);
    act("$p flows from the ground.",ch,spring,NULL,TO_CHAR);
    return;
}

void spell_create_water(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
		send_to_char("It is unable to hold water.\n\r",ch);
		return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
		send_to_char("It contains some other liquid.\n\r",ch);
		return;
    }

    water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2),obj->value[0] - obj->value[1]);
  
    if ( water > 0 )
    {
		obj->value[2] = LIQ_WATER;
		obj->value[1] += water;
		if ( !is_name("water",obj->name) )
		{
			char buf[MSL];

			sprintf(buf,"%s water",obj->name);
			free_string(obj->name);
			obj->name = str_dup(buf);
		}
		act("$p is filled.",ch,obj,NULL,TO_CHAR);
    }

    return;
}

void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->isaff(AF_DETECT_HIDDEN) )
    {
		if (victim == ch)
			send_to_char("You are already as alert as you can be. \n\r",ch);
		else
			act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
		return;
    }
	affect_set(victim,TO_AFFECTS,gsn_awareness,level,slvl,level,APPLY_NONE,slvl,AF_DETECT_HIDDEN);

    send_to_char("Your awareness improves.\n\r",victim);
    if ( ch != victim )
		send_to_char("Ok.\n\r",ch);
    return;
}

void spell_detect_invis(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->isaff(AF_DETECT_INVIS) )
    {
		if (victim == ch)
			send_to_char("You can already see invisible.\n\r",ch);
		else
			act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
		return;
    }
	affect_set(victim,TO_AFFECTS,gsn_detect_invis,level,slvl,level,APPLY_NONE,slvl,AF_DETECT_INVIS);
    send_to_char("Your eyes tingle.\n\r",victim);
    if ( ch != victim )
		send_to_char("Ok.\n\r",ch);
    return;
}

void spell_detect_magic(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->isaff(AF_DETECT_MAGIC) )
    {
		if (victim == ch)
			send_to_char("You can already sense magical auras.\n\r",ch);
		else
			act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
		return;
    }
	affect_set(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_NONE,slvl,AF_DETECT_MAGIC);
    send_to_char("Your eyes tingle.\n\r",victim);
    if ( ch != victim )
		send_to_char("Ok.\n\r",ch);
    return;
}

void spell_detect_poison(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
		if ( obj->value[3] != 0 )
			send_to_char("You smell poisonous fumes.\n\r",ch);
		else
			send_to_char("It looks delicious.\n\r",ch);
	}
	else
	{
		send_to_char("It doesn't look poisoned.\n\r",ch);
    }

    return;
}

void spell_dispel_magic(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	bool found = false;

	act("You feel a brief tingling sensation.\n\r",victim,NULL,NULL,TO_CHAR);
	if (saves_spell(level, victim,DAM_OTHER)){
		send_to_char( "You failed.\n\r", ch);
		return;
	}

	if (check_dispel(level,victim,skill_lookup("armor"))){
		act( "$n's magical armor slowly vanishes.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("charm person"))){
		found = true;
		act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(level,victim,skill_lookup("chill touch"))){
		found = true;
		act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(level,victim,skill_lookup("detect hidden")))
		found = true;

	if (check_dispel(level,victim,skill_lookup("detect invis")))
		found = true;

	if (check_dispel(level,victim,skill_lookup("detect magic")))
		found = true;

	if (check_dispel(level,victim,skill_lookup("fly"))){
		act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("frenzy"))){
		act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("giant strength"))){
		act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("haste"))){
		act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("infravision")))
		found = true;

	if (check_dispel(level,victim,skill_lookup("invis"))){
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("mass invis"))){
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("pass door"))){
		act("$n slowly regains corporeality.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (victim->isaff(AF_SANCTUARY) && !saves_dispel(level, victim->level,-1) && !is_affected(victim,skill_lookup("sanctuary"))){
		victim->remaff(AF_SANCTUARY);
		act("The white aura around $n's body vanishes.", victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("shield"))){
		act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("slow"))){
		act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (check_dispel(level,victim,skill_lookup("stone skin"))){
		act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
		found = true;
	}

	if (found)
		send_to_char("Ok.\n\r",ch);
	else
		send_to_char("Spell failed.\n\r",ch);
	return;
}

void spell_energy_drain(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    if ( saves_spell(level,victim,DAM_NEGATIVE) )
    {
		send_to_char("You feel a momentary chill.\n\r",victim);
		return;
    }


    if ( victim->level <= 2 )
		dam = ch->hit + 1;
    else
    {
		gain_exp(victim,0 - number_range(level/2,3 * level / 2));
		victim->setmana(victim->getmana() / 2);
		victim->move /= 2;
		dam = dice(1, level);
		ch->hit += dam;
    }

    send_to_char("You feel your life slipping away!\n\r",victim);
    send_to_char("Wow....what a rush!\n\r",ch);
    spell_damage(ch,victim,dam,sn,DAM_NEGATIVE,true);

    return;
}

void spell_fireproof(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
 
    if (IS_OBJ_STAT(obj,ITM_BURN_PROOF))
    {
        act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
        return;
    }
 
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITM_BURN_PROOF;
 
    affect_to_obj(obj,&af);
 
    act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
    act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}

void spell_flamestrike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6 + level / 2,8);
    if(saves_spell(level,victim,DAM_FIRE))
		dam /= 2;
    spell_damage(ch,victim,dam,sn,DAM_FIRE,true);
}

void spell_faerie_fire(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->isaff(AF_FAERIE_FIRE) )
		return;
	affect_set(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_AC,2 * level,AF_FAERIE_FIRE);
    send_to_char("You are surrounded by a pink outline.\n\r",victim);
    act("$n is surrounded by a pink outline.",victim,NULL,NULL,TO_ROOM);
}

void spell_fly(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( victim->isaff(AF_FLYING) )
    {
		if (victim == ch)
		  send_to_char("You are already airborne.\n\r",ch);
		else
		  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
		return;
    }
	affect_set(victim,TO_AFFECTS,sn,level,slvl,level + 3,0,0,AF_FLYING);
    send_to_char("Your feet rise off the ground.\n\r",victim);
    act("$n's feet rise off the ground.",victim,NULL,NULL,TO_ROOM);
}

void spell_heat_metal(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose, *obj_next;
    int dam = 0;
    bool fail = true;
 
   if (!saves_spell(level + 2,victim,DAM_FIRE) && !victim->res[RS_FIRE] == 0){
        for ( obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next)
        {
		    obj_next = obj_lose->next_content;
            if ( number_range(1,2 * level) > obj_lose->level 
			&&   !saves_spell(level,victim,DAM_FIRE)
			&&   material_metal(obj_lose)
			&&   !IS_OBJ_STAT(obj_lose,ITM_BURN_PROOF)){
				switch ( obj_lose->item_type ){
               		case ITEM_ARMOR:
               		case ITEM_SHIELD:
						if (obj_lose->wear_loc != -1)
						{
							if (can_drop_obj(victim,obj_lose) && (obj_lose->weight / 10) < number_range(1,2 * get_curr_stat(victim,STAT_AGI)) && remove_obj( victim, obj_lose->wear_loc, true ))
							{
								act("$n yelps and throws $p to the ground!",
								victim,obj_lose,NULL,TO_ROOM);
								act("You remove and drop $p before it burns you.",
								victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 3);
								obj_from_char(obj_lose);
								obj_to_room(obj_lose, victim->in_room);
								fail = false;
							}
							else
							{
								act("Your skin is seared by $p!", victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level));
								fail = false;
							}
						}
						else
						{
							if (can_drop_obj(victim,obj_lose))
							{
								act("$n yelps and throws $p to the ground!", victim,obj_lose,NULL,TO_ROOM);
								act("You and drop $p before it burns you.", victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 6);
								obj_from_char(obj_lose);
								obj_to_room(obj_lose, victim->in_room);
								fail = false;
							}
							else
							{
								act("Your skin is seared by $p!", victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 2);
								fail = false;
							}
						}
						break;
					case ITEM_WEAPON:
						if (obj_lose->wear_loc != -1)
						{
							if (IS_WEAPON_STAT(obj_lose,WPN_FLAMING))
								continue;

							if (can_drop_obj(victim,obj_lose) && remove_obj(victim,obj_lose->wear_loc,true))
							{
								act("$n is burned by $p, and throws it to the ground.", victim,obj_lose,NULL,TO_ROOM);
								send_to_char( "You throw your red-hot weapon to the ground!\n\r", victim);
								dam += 1;
								obj_from_char(obj_lose);
								obj_to_room(obj_lose,victim->in_room);
								fail = false;
							}
							else
							{
								send_to_char("Your weapon sears your flesh!\n\r", victim);
								dam += number_range(1,obj_lose->level);
								fail = false;
							}
						}
						else
						{
							if (can_drop_obj(victim,obj_lose))
							{
								act("$n throws a burning hot $p to the ground!", victim,obj_lose,NULL,TO_ROOM);
								act("You and drop $p before it burns you.", victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 6);
								obj_from_char(obj_lose);
								obj_to_room(obj_lose, victim->in_room);
								fail = false;
							}
							else
							{
								act("Your skin is seared by $p!", victim,obj_lose,NULL,TO_CHAR);
								dam += (number_range(1,obj_lose->level) / 2);
								fail = false;
							}
						}
						break;
					}
				}
			}
		}
    if (fail)
    {
		send_to_char("Your spell had no effect.\n\r", ch);
		send_to_char("You feel momentarily warmer.\n\r",victim);
    }
    else
    {
		if (saves_spell(level,victim,DAM_FIRE))
			dam = 2 * dam / 3;
		spell_damage(ch,victim,dam,sn,DAM_FIRE,true);
    }
}

void spell_identify(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    char buf[MSL];
    AFFECT_DATA *paf;

    printf_to_char(ch," {d_____________________________________________\n\r{d// {w%-41s {d\\\\ \n\r{d|| {yType: {w%-9s             {yLevel: {w%-3d    {d||\n\r{d|| {yValue: {w%-8d             {yWeight: {w%-3d   {d||\n\r{d|| {yExtra: {w%-35s{d||\n\r",
		obj->name,
		item_name(obj->item_type),
		obj->level,
		obj->cost,
		obj->weight / 16,
		extra_bit_name(obj->extra_flags)
	);

    switch ( obj->item_type )
    {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
		printf_to_char(ch,"{d|| {wLevel {y%3d {wspells of:                      {d||\n\r",obj->value[0]);
		if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
			printf_to_char(ch,"{d||   {w%-39s {d||\n\r",skill_table[obj->value[1]].name);
		if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
			printf_to_char(ch,"{d||   {w%-39s {d||\n\r",skill_table[obj->value[2]].name);
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
			printf_to_char(ch,"{d||   {w%-39s {d||\n\r",skill_table[obj->value[3]].name);
		if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
			sprintf(buf,"{d||   {w%-39s {d||\n\r",skill_table[obj->value[4]].name);
		break;
    case ITEM_WAND: 
    case ITEM_STAFF: 
		printf_to_char(ch,"{d|| {wHas {y%-2d {wcharges of level {y%-3d ",obj->value[2],obj->value[0]);
		if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
			printf_to_char(ch,"{y%-13s {d||\n\r",skill_table[obj->value[3]].name);
		break;
    case ITEM_DRINK_CON:
        printf_to_char(ch,"{d|| {wIt holds %10s-colored {y%-13s {d||\n\r",liq_table[obj->value[2]].liq_color,liq_table[obj->value[2]].liq_name);
        break;
    case ITEM_CONTAINER:
		printf_to_char(ch,"{d|| {wCapacity: {y%-5d{w#     Maximum weight: {y%-4d{w# {d||\n\r|| {wWeight Multiplier: {y%-3d%% {wFlags: {y%-10s {d||\n\r",obj->value[0],obj->value[3],obj->value[4],cont_bit_name(obj->value[1]));
		break;
    case ITEM_WEAPON:
		printf_to_char(ch,"{d|| {wWeapon Type: {y%-15s              {d||\n\r",weapon_name(obj->value[0]));
		printf_to_char(ch,"{d|| {wDamage: {y%3d{wd{y%-3d {w(Average {y%4d{w)            {d||\n\r",obj->value[1],obj->value[2],(1 + obj->value[2]) * obj->value[1] / 2);
		if (obj->value[4])  /* weapon flags */
			printf_to_char(ch,"{d|| {wWeapons Flags: {y%-27s{d||\n\r",weapon_bits_name(obj->wflags));
		break;
    case ITEM_ARMOR:
    case ITEM_SHIELD:
		printf_to_char(ch,"{d||    {wBash: {y-%-4d            {wSlash: {y-%-4d    {d||\n\r||    {wPierce: {y-%-4d          {wMagic: {y-%-4d    {d||\n\r",obj->value[1],obj->value[2],obj->value[0],obj->value[3]);
		break;
    }

    if (!obj->enchanted)
		for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
		{
			if ( paf->location != APPLY_NONE && paf->modifier != 0 )
			{
				printf_to_char(ch,"{d|| {wAffects {y%-12s {wby {y%-4d              {d||\n\r",affect_loc_name(paf->location),paf->modifier);
				if (paf->bitvector)
				{
					switch(paf->where)
					{
						case TO_AFFECTS:
							printf_to_char(ch,"{d|| {wAdds {y%-9s {waffect                     {d||\n\r",affect_bit_name(paf->bitvector));
							break;
						case TO_OBJECT:
							printf_to_char(ch,"{d|| {wAdds {y%-9s {wobject flag                {d||\n\r",extra_bit_name(paf->bitvector));
							break;
						case TO_IMMUNE:
							printf_to_char(ch,"{d|| {wAdds immunity to {y%-9s                {d||\n\r",res_bit_name(paf->bitvector));
							break;
						case TO_RESIST:
							printf_to_char(ch,"{d|| {wAdds resistance to {y%-9s              {d||\n\r",res_bit_name(paf->bitvector));
							break;
						case TO_VULN:
							printf_to_char(ch,"{d|| {wAdds vulnerability to {y%-9s           {d||\n\r",res_bit_name(paf->bitvector));
							break;
						default:
							break;
					}
				}
			}
		}

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
		if ( paf->location != APPLY_NONE && paf->modifier != 0 )
		{
			printf_to_char(ch,"{d|| {wAffects {y%-9s {wby {y%-4d                 {d||\n\r",affect_loc_name(paf->location),paf->modifier);
			if ( paf->duration > -1)
				printf_to_char(ch,"{d||     {wfor {y%-3d {whours                         {d||\n\r",paf->duration);
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
						printf_to_char(ch,"{d|| {wAdds {y%-9s {waffect                     {d||\n\r",affect_bit_name(paf->bitvector));
                        break;
                    case TO_OBJECT:
                        printf_to_char(ch,"{d|| {wAdds {y%-9s {wobject flag                {d||\n\r",extra_bit_name(paf->bitvector));
                        break;
					case TO_WEAPON:
						printf_to_char(ch,"{d|| {wAdds Weapon Flags: {y%-22s {d||\n\r", weapon_bit_name(paf->bitvector));
						break;
                    case TO_IMMUNE:
                        printf_to_char(ch,"{d|| {wAdds immunity to {y%-9s                {d||\n\r",res_bit_name(paf->bitvector));
                        break;
                    case TO_RESIST:
                        printf_to_char(ch,"{d|| {wAdds resistance to {y%-9s              {d||\n\r",res_bit_name(paf->bitvector));
                        break;
                    case TO_VULN:
                        printf_to_char(ch,"{d|| {wAdds vulnerability to {y%-9s           {d||\n\r",res_bit_name(paf->bitvector));
                        break;
                    default:
                        break;
                }
            }
		}
	send_to_char("{d\\\\___________________________________________//{W\n\r",ch);
    return;
}

void spell_infravision(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim->isaff(AF_INFRARED))
    {
		if (victim == ch)
			send_to_char("You can already see in the dark.\n\r",ch);
		else
			act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
		return;
    }
    act("$n's eyes glow red.\n\r",ch,NULL,NULL,TO_ROOM);

	affect_set(victim,TO_AFFECTS,sn,level,slvl,2 * level,APPLY_NONE,0,AF_INFRARED);
    send_to_char("Your eyes glow red.\n\r",victim);
    return;
}

void spell_thunder_strike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo,*wch,*wch_next;
    int dam;

    dam = number_range(level,level * 4);
	dam += ch->level;
	dam += get_curr_stat(ch,STAT_INT);
    if (!saves_spell(level,victim,DAM_LIGHTNING))
		dam *= 2;
    spell_damage(ch,victim,dam,sn,DAM_LIGHTNING,true);
	for(wch = ch->in_room->people; wch; wch = wch_next)
	{
		wch_next = wch->next_in_room;
		if (is_same_group(wch,victim))
			(*skill_table[find_spell(ch,"call lightning")].spell_fun)(find_spell(ch,"call lightning"),10,ch,wch,TARGET_CHAR,1);//Nash fix slvl
	}
    return;
}

void spell_plague(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (get_skill(victim,gsn_resilience))
		level /= 2;

	if (saves_spell((level * 1.5) * ch->getslvl(sn),victim,DAM_DISEASE) || (IS_NPC(victim) && victim->isact(AT_UNDEAD))){
		if (ch == victim)
			send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
		else
			act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_join(victim,TO_AFFECTS,sn,level*3/4,slvl,2,APPLY_STR,-5,AF_PLAGUE);

	send_to_char("You scream in agony as plague sores erupt from your skin.\n\r",victim);
	act("$n screams in agony as plague sores erupt from $s skin.",victim,NULL,NULL,TO_ROOM);
}

void spell_poison(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	if (target == TARGET_OBJ){
		obj = (OBJ_DATA *) vo;
		if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON){
			if (IS_OBJ_STAT(obj,ITM_BURN_PROOF)){
				act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
				return;
			}
			obj->value[3] = 1;
			act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
			return;
		}

		if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_NEEDLE){
			if (IS_WEAPON_STAT(obj,WPN_FLAMING)
			||  IS_WEAPON_STAT(obj,WPN_FROST)
			||  IS_WEAPON_STAT(obj,WPN_VAMPIRIC)
			||  IS_WEAPON_STAT(obj,WPN_SERRATED)
			||  IS_WEAPON_STAT(obj,WPN_HOLY)
			||  IS_WEAPON_STAT(obj,WPN_DEMONIC)
			||  IS_WEAPON_STAT(obj,WPN_POWERDRAIN)
			||  IS_WEAPON_STAT(obj,WPN_SHARP)
			||  IS_WEAPON_STAT(obj,WPN_VORPAL)
			||  IS_WEAPON_STAT(obj,WPN_SHOCKING)
			|| IS_OBJ_STAT(obj,ITM_BURN_PROOF)){
				act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
				return;
			}

			if (IS_WEAPON_STAT(obj,WPN_POISON) || IS_WEAPON_STAT(obj,WPN_PESTILENCE)){
				act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
				return;
			}

			affect_set(obj,TO_WEAPON,sn,level/2,slvl,5,APPLY_NONE,0,WPN_POISON);

			act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
			return;
		}

		act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (get_skill(victim,gsn_resilience))
		level /= 2;

	if (saves_spell((level * 1.5) * ch->getslvl(sn),victim,DAM_POISON)){
		act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
		send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
		return;
	}

	affect_join(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_STR,-2,AF_POISON);
	send_to_char("You feel very sick.\n\r",victim);
	act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
}

void spell_recharge(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
		send_to_char("That item does not carry charges.\n\r",ch);
		return;
    }

    if (obj->value[3] >= 3 * level / 2)
    {
		send_to_char("Your skills are not great enough for that.\n\r",ch);
		return;
    }

    if (obj->value[1] == 0)
    {
		send_to_char("That item has already been recharged once.\n\r",ch);
		return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[3]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) * (obj->value[1] - obj->value[2]);

    chance = UMAX(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
		act("$p glows softly.",ch,obj,NULL,TO_CHAR);
		act("$p glows softly.",ch,obj,NULL,TO_ROOM);
		obj->value[2] = UMAX(obj->value[1],obj->value[2]);
		obj->value[1] = 0;
		return;
    }

    else if (percent <= chance)
    {
		int chargeback,chargemax;

		act("$p glows softly.",ch,obj,NULL,TO_CHAR);
		act("$p glows softly.",ch,obj,NULL,TO_CHAR);

		chargemax = obj->value[1] - obj->value[2];
		
		if (chargemax > 0)
			chargeback = UMAX(1,chargemax * percent / 100);
		else
			chargeback = 0;

		obj->value[2] += chargeback;
		obj->value[1] = 0;
		return;
    }	

    else if (percent <= UMIN(95, 3 * chance / 2))
    {
		send_to_char("Nothing seems to happen.\n\r",ch);
		if (obj->value[1] > 1)
			obj->value[1]--;
		return;
    }

    else /* whoops! */
    {
		act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
		act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
		extract_obj(obj);
    }
}

void spell_refresh(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    victim->move = UMIN(victim->move + level,victim->max_move);
    if (victim->max_move == victim->move)
        send_to_char("You feel fully refreshed!\n\r",victim);
    else
        send_to_char("You feel less tired.\n\r",victim);
    if ( ch != victim )
        send_to_char("Ok.\n\r",ch);
    return;
}

void spell_shield(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn)){
		if (victim == ch)
			send_to_char("You are already shielded from harm.\n\r",ch);
		else
			act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,8 * UMAX(1,ch->getslvl(sn)),APPLY_AC,-(50*ch->getslvl(sn)),0);
	act("$n is surrounded by a force shield.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You are surrounded by a force shield.\n\r",victim);
}

void spell_sleep(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
  
    if ( victim->isaff(AF_SLEEP) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)) || saves_spell(level,victim,DAM_CHARM))
		return;

	affect_join(victim,TO_AFFECTS,sn,level,slvl,9,APPLY_NONE,0,AF_SLEEP);

    if (IS_AWAKE(victim))
    {
		send_to_char("You feel very sleepy ..... zzzzzz.\n\r",victim);
		act("$n goes to sleep.",victim,NULL,NULL,TO_ROOM);
		victim->position = POS_SLEEPING;
    }
}

void spell_fear(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim == ch)
	{
		send_to_char("You're afraid! EEK!\n\r", ch);
		return;
	}

    act("You try and send a surge of fear into $N.",ch,NULL,victim,TO_CHAR);
    act("You feel momentarily afraid.",ch,NULL,victim,TO_VICT);
	if(!saves_spell(level,victim,DAM_MENTAL))
	{
		act("$N screams in terror and tries to flee!",ch,NULL,victim,TO_CHAR);
		act("You feel fear engulf your body and you panic!",ch,NULL,victim,TO_VICT);
		act("$N screams in terror and tries to flee!",ch,NULL,victim,TO_NOTVICT);
		do_function(victim,&do_flee,"");
	}
	else
		send_to_char("You failed.\n\r",ch);
}
