#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"
#include "music.h"
#include "recycle.h"

extern SLEEP_DATA *first_sleep;
/*
 * Local functions.
 */
void	write_max_con();
bool	IS_NATURE_BG	( CHAR_DATA* );
bool	check_animal_magnetism	( CHAR_DATA*,CHAR_DATA* );
void raw_kill(CHAR_DATA*,CHAR_DATA*);
void remove_charmy(CHAR_DATA *ch,CHAR_DATA *charmy);
int	 hit_gain				( CHAR_DATA* );
int	 mana_gain				( CHAR_DATA* );
int	 move_gain				( CHAR_DATA* );
bool check_spellcraft		( CHAR_DATA* );
void mobile_update			( void );
void weather_update			( void );
void char_update			( void );
void obj_update				( void );
void aggr_update			( void );
bool check_endure			( CHAR_DATA* );
void room_aff_update		( ROOM_INDEX_DATA* );
//void charforge_update		( CHAR_DATA* );
bool check_spell_efficiency ( CHAR_DATA* );
int  spell_proficiency		( CHAR_DATA*,int,int );
int  inverse_heal			( CHAR_DATA*,int );
void d_insert				( DESCRIPTOR_DATA* );
bool check_mental_focus		( CHAR_DATA* );
void check_frost_field		( CHAR_DATA* );
/* used for saving */

int	save_number = 0;

int     global_exp;

/*
 * Advancement stuff.
 */
void stat_rolls(CHAR_DATA *ch){
	int stat,chance;

	for (stat=0;stat < MAX_STATS;stat++){
		if(ch->perm_stat[stat] >= 40)
			continue;
		if(number_percent() < classes[ch->pclass].attribute[stat]){
			ch->perm_stat[stat]++;
			printf_to_char(ch,"Your {C%s{x increases!\n\r",stat_flags[stat].name);
		}
	}
}

void mod_skills(CHAR_DATA *ch){
	int sn;

	for (sn = 0;skill_table[sn].name;sn++){
		if (skill_table[sn].skill_level[ch->pclass] != ch->level || ch->pcdata->learned[sn] < 1)
			continue;
//		ch->pcdata->learned[sn] = number_range(classes[ch->pclass].skill_adept * .75,classes[ch->pclass].skill_adept);
	}
}

int buff_mana(CHAR_DATA *ch){
	int add_mana,add_antimana;
	int fmana = classes[ch->pclass].fMana;
	int cint = get_curr_stat(ch,STAT_INT),cfcs = get_curr_stat(ch,STAT_WIS),cres = get_curr_stat(ch,STAT_RES);
	int manabase = 0,antimanabase = 0;
	int mpmin = classes[ch->pclass].mp_min,mpmax = classes[ch->pclass].mp_max;

	manabase = number_range(mpmin,mpmax);
	antimanabase = number_range(mpmin,mpmax);
//	if (classes[ch->pclass].amp){//antimage
//		manabase /= 4;
//	}
//	else{//regularmage
//		antimanabase /= 4;
//	}

	add_mana = manabase;
    if (classes[ch->pclass].fMana > 2)
		add_mana += add_mana * (fmana/2) * (cfcs + cint) / (STAT_MAX *2);
	else
		add_mana = number_range(add_mana * (fmana+1) /2,add_mana * (fmana+1));
    add_mana	= UMAX(2,add_mana);

	add_antimana = antimanabase;
    if (classes[ch->pclass].fMana > 2)
		add_antimana += add_antimana * (fmana/2) * (cfcs + cint + cres) / (STAT_MAX *2.5);
	else
		add_antimana = number_range(add_antimana * (fmana+1) /2,add_antimana * (fmana+1));
    add_antimana	= UMAX(2,add_antimana);

	ch->modtruemaxmana(add_mana);
	ch->modtruemaxantimana(add_antimana);
    ch->pcdata->perm_mana		+= add_mana;
    ch->pcdata->perm_antimana	+= add_antimana;
	if (classes[ch->pclass].amp)
		return add_antimana;
	else
		return add_mana;
}

void boost_practices(CHAR_DATA *ch){

}

int get_hp_gain(CHAR_DATA *ch){//nashrenamegethitgain
	int cend = get_curr_stat(ch,STAT_END),cstr = get_curr_stat(ch,STAT_STR),cdex = get_curr_stat(ch,STAT_AGI);
	int add_hp,hpmin = classes[ch->pclass].hp_min,hpmax = classes[ch->pclass].hp_max;

	add_hp = number_range(hpmin,hpmax);
	add_hp += classes[ch->pclass].hp_max * end_app[cend].hitp / 20;
	add_hp		= UMAX(2,add_hp);
}

int get_mv_gain(CHAR_DATA *ch){
	int cend = get_curr_stat(ch,STAT_END),cstr = get_curr_stat(ch,STAT_STR),cdex = get_curr_stat(ch,STAT_AGI);
	int add_move,mvmin = classes[ch->pclass].mv_min,mvmax = classes[ch->pclass].mv_max;
	add_move = number_range(mvmin,mvmax) * (cend + cdex + cstr) / (STAT_MAX * 3);

	add_move	= UMAX(6,add_move);
}

void advance_level(CHAR_DATA *ch,bool hide){
	int add_hp,add_mv,add_mp;
	int add_prac,add_gain = 0;

	ch->pcdata->last_level = (ch->played + (int)(current_time - ch->logon)) / 3600;

	add_prac = UMAX(((get_curr_stat(ch,STAT_INT) * 3) / 30 ),1) + 3;
	if(ch->level %2 == 0)
		add_gain++;
	//if(ch->level %5 == 0)
	//	add_gain++;

	add_hp = get_hp_gain(ch);
	add_mp = buff_mana(ch);
	add_mv = get_mv_gain(ch);

	ch->max_hit 	+= add_hp;
	ch->max_move	+= add_mv;
	ch->pcdata->perm_hit	+= add_hp;
	ch->pcdata->perm_move	+= add_mv;

	ch->pcdata->practices += add_prac;
	ch->pcdata->studies += add_gain;

	if(ch->level % 10 == 0)
		stat_rolls(ch);

	d_insert(ch->desc);
	if (!hide)
		printf_to_char(ch,"You gain {R%d {xhit point%s, {R%d {xmana, {R%d {xmove, {R%d {xpractices, and {R%d {xskillpoints.\n\r",add_hp,add_hp == 1 ? "" : "s",add_mp,add_mv,add_prac,add_gain);
	cql_save_char(ch);
}   

bool check_tierlevel(CHAR_DATA *ch){
	switch(ch->level){
	case 10:
		if(classes[ch->pclass].ctier == 0)
			return false;
		break;
	case 25:
		if(classes[ch->pclass].ctier == 1)
			return false;
		break;
	case 50:
		if(classes[ch->pclass].ctier == 2)
			return false;
		break;
	}
	if(ch->level > 99)
		return false;
	return true;
}

void gain_exp(CHAR_DATA *ch,int gain){
    char buf[MSL];

    if (IS_NPC(ch) || ch->level >= LEVEL_HERO)
		return;

	if(!IS_NPC(ch) && !check_tierlevel(ch)){
		ch->send("You have reached the max level for your class, you will not gain further experience until you change classes.\n\r");
		return;
	}

    ch->exp += gain;
	ch->texp += gain;

    while (ch->level < LEVEL_HERO && ch->exp >= exp_per_level(ch)){
		if(!IS_NPC(ch) && !check_tierlevel(ch)){
			ch->send("You have reached the max level for your class, you will not gain further experience until you change classes.\n\r");
			ch->exp = exp_per_level(ch);
			return;
		}
		send_to_char("You have gained a level!\n\r",ch);
		act("$n has leveled up!",ch,NULL,NULL,TO_ROOM);
		ch->exp -= exp_per_level(ch);
		ch->level++;

		sprintf(buf,"%s gained level %d",ch->name,ch->level);
		log_string(buf);
		sprintf(buf,"$N has attained level %d!",ch->level);
		wiznet(buf,ch,NULL,WZ_LEVELS,0,0);

		advance_level(ch,false);

		affect_strip(ch,gsn_plague);
        affect_strip(ch,gsn_malady);
		affect_strip(ch,gsn_rupture);
		affect_strip(ch,gsn_poison);
		affect_strip(ch,gsn_blindness);
		affect_strip(ch,gsn_sleep);
		affect_strip(ch,gsn_curse);

		ch->hit        = ch->max_hit;
		ch->setmana(ch->getmaxmana());
		ch->move       = ch->max_move;
		update_pos(ch,NULL);
		send_to_char("Your increase in training has given you the stamina to continue.\n\r",ch);
		cql_save_char(ch);
    }
}

int hit_gain(CHAR_DATA *ch){
	OBJ_DATA *obj;
	int gain,number,inv;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC(ch)){
		gain = 5 + ch->level;
 		if (ch->isaff(AF_REGENERATION))
			gain *= 2;

		switch(ch->position){
			default : 			gain /= 2;			break;
			case POS_SLEEPING: 	gain = 3 * gain/2;	break;
			case POS_RESTING:						break;
			case POS_FIGHTING:	gain /= 3;			break;
 		}
	}
	else{
		gain = ch->perm_stat[STAT_END] + (ch->max_hit / 10);
		gain += classes[ch->pclass].hp_max - 10;

		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"hpgain g%d",gain);

 		number = number_percent();
		if (number < get_skill(ch,gsn_fast_healing)){
			gain += ch->getslvl(gsn_fast_healing) * number / 10;
			if (ch->hit < ch->max_hit)
				check_improve(ch,gsn_fast_healing,true,8);
		}

		if(ch->iscomm(CM_DEBUG))printf_to_char(ch," fast%d",gain);
		if(ch->isaff(AF_SILENCE) && get_skill(ch,gsn_tranquility) > 0 && number_percent() < 50 && IS_NATURE_BG(ch)){
			gain += gain/2 * ch->getslvl(gsn_tranquility);
			check_improve(ch,gsn_tranquility,true,1);
		}
		if(ch->iscomm(CM_DEBUG))printf_to_char(ch," tranq%d",gain);

		switch (ch->position){
			default:	   		gain /= 4;	break;
			case POS_SLEEPING: 				break;
			case POS_RESTING:  	gain /= 2;	break;
			case POS_SITTING:  	gain /= 3;	break;
			case POS_FIGHTING: 	gain /= 6;	break;
		}
		
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," pos%d",gain);

		if (!check_endure(ch) && ch->pcdata->condition[COND_HUNGER] < 10 && ch->pcdata->condition[COND_HUNGER] > -1)
			gain /= 2;
		if (!check_endure(ch) && ch->pcdata->condition[COND_THIRST] < 10 && ch->pcdata->condition[COND_THIRST] > -1)
			gain /= 2;

		if(ch->isaff(AF_TRANQUILITY))
			gain *= 2;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," hung%d",gain);
	}

	if(check_leadership(ch))
		gain *= 1.5;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," lead%d",gain);

	if(ch->in_room->israf(RAF_REGENERATION))
		gain *= 1.5;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," regen%d",gain);

	gain *= ch->in_room->heal_rate / 100;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," room%d",gain);

	if(ch->in_room->sector_type == SECT_GRAVEYARD && ch->alignment < 0)
		gain *= 1.5;
	else
		gain = (gain * sect_table[ch->in_room->sector_type].hpheal) / 100;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," sect%d",gain);

	for (obj = ch->in_room->contents;obj;obj = obj->next_content){
		if (obj->item_type == ITEM_FIRE){
			gain *= 2;
			break;
		}
	}
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," camp%d",gain);

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[3] / 100;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," furn%d",gain);
	if (!check_endure(ch) && ch->isaff(AF_POISON))
		gain /= 4;

	if(ch->isaff(AF_VENOM))
		gain /= 4;

	if (ch->isaff(AF_PLAGUE))
		gain /= 8;

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," blgt%d",gain);

	if (ch->isaff(AF_HASTE) || ch->isaff(AF_SLOW))
		gain /= 2;
	
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," spel%d",gain);

	if (get_eq_char(ch,WEAR_LODGE_LEG) != NULL)
		gain *= .75;
	if (get_eq_char(ch,WEAR_LODGE_ARM) != NULL)
		gain *= .75;
	if (get_eq_char(ch,WEAR_LODGE_RIB) != NULL)
		gain *= .75;
	
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," arrw%d",gain);
	if (gain > 0){
		if (check_icebound(ch) == 1)
			gain *= 1.5;
		else if (check_icebound(ch) == 2)
			gain = -(gain *1.25);
	}
	else if (check_icebound(ch) == 2)
			gain *= 2;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," algd%d",gain);

	gain = inverse_heal(ch,gain);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," ivh%d",gain);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch," hpgain Gain%d inv%d hit%d max%d umin%d\n\r",gain,inv,ch->hit,ch->max_hit,UMIN(gain,ch->max_hit - ch->hit));
	return UMIN(gain,ch->max_hit - ch->hit);
}

int mana_gain(CHAR_DATA *ch){
	OBJ_DATA *obj;
	int gain=0,number;
	if (ch->in_room == NULL)
		return gain;
	if (IS_NPC(ch))
	{
		gain = 5 + ch->level;
		switch (ch->position)
		{
			default:		gain /= 2;				break;
			case POS_SLEEPING:	gain = 3 * gain/2;	break;
   			case POS_RESTING:						break;
			case POS_FIGHTING:	gain /= 3;			break;
		}
	}
	else{
		gain = number_range(ch->level,get_curr_stat(ch,STAT_WIS) + get_curr_stat(ch,STAT_INT))*2;
		if (ch->alignment > 0)
			gain = gain * get_curr_stat(ch,STAT_FTH) / STAT_MAX;

		number = number_percent();
		if (number < get_skill(ch,gsn_meditation)){
			gain += number * gain / 100;
			if (ch->getmana() < ch->getmaxmana())
				check_improve(ch,gsn_meditation,true,8);
		}
		if (classes[ch->pclass].fMana == 0)
			gain /= 2;
		else
			gain = (gain * classes[ch->pclass].fMana) / 3;
		if(ch->isaff(AF_SILENCE) && get_skill(ch,gsn_tranquility) > 0 && number_percent() < 50 && IS_NATURE_BG(ch)){
			gain += gain/2 * ch->getslvl(gsn_tranquility);
			check_improve(ch,gsn_tranquility,true,1);
		}

		switch ( ch->position ){
			default:							break;
			case POS_SLEEPING: 	gain *= 4;		break;
			case POS_RESTING:	gain *= 2;		break;
			case POS_FIGHTING:	gain /= 2;		break;
		}

		if(ch->isaff(AF_ETHEREAL_ALACRITY))
			gain /= 2;
		if (!check_endure(ch) && ch->pcdata->condition[COND_HUNGER] < 10 && ch->pcdata->condition[COND_HUNGER] > -1)
			gain /= 2;
		if (!check_endure(ch) && ch->pcdata->condition[COND_THIRST] < 10 && ch->pcdata->condition[COND_THIRST] > -1)
			gain /= 2;

		if(ch->isaff(AF_TRANQUILITY))
			gain *= 1.5;
	}

	gain = do_thalassic_aura(ch,gain);

	if(check_leadership(ch))
		gain *= 1.5;

	if(ch->in_room->israf(RAF_REGENERATION))
		gain *= 1.5;

	if(ch->isaff(AF_MANAREGEN))
		gain *= 1.5;

	gain *= ch->in_room->mana_rate / 100;

	gain = (gain * sect_table[ch->in_room->sector_type].manaheal) / 100;

	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
		if (obj->item_type == ITEM_FIRE)
		{
			gain *= 2;
			break;
		}
	}

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain *= ch->on->value[4] / 100;

	if (!check_endure(ch) && ch->isaff(AF_POISON))
		gain /= 4;

	if(ch->isaff(AF_VENOM))
		gain /= 4;

	if (ch->isaff(AF_PLAGUE))
		gain /= 8;

	if (ch->isaff(AF_CONFUSION))
		gain /= 2;

	if (ch->isaff(AF_HASTE) || ch->isaff(AF_SLOW))
		gain /=2 ;

	if (get_eq_char(ch,WEAR_LODGE_LEG) != NULL)
		gain *= .5;
	if (get_eq_char(ch,WEAR_LODGE_ARM) != NULL)
		gain *= .5;
	if (get_eq_char(ch,WEAR_LODGE_RIB) != NULL)
		gain *= .5;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"managain g%d m%d um%d\n\r",gain,ch->getmaxmana() - ch->getmana(),UMIN(gain,ch->getmaxmana() - ch->getmana()));
	return UMIN(gain,ch->getmaxmana() - ch->getmana());
}

int move_gain(CHAR_DATA *ch){
	OBJ_DATA *obj;
	int gain;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC(ch))
		gain = ch->level;
	else{
		gain = (get_curr_stat(ch,STAT_END) + get_curr_stat(ch,STAT_AGI)) * .75;

		switch (ch->position){
			default:			gain /= 2; break;
			case POS_SLEEPING:	gain *= 2; break;
			case POS_RESTING:	break;
		}
		if(ch->isaff(AF_SILENCE) && get_skill(ch,gsn_tranquility) > 0 && number_percent() < 50 && IS_NATURE_BG(ch)){
			gain += gain/2 * ch->getslvl(gsn_tranquility);
			check_improve(ch,gsn_tranquility,true,1);
		}

		if (!check_endure(ch) && ch->pcdata->condition[COND_HUNGER] < 10 && ch->pcdata->condition[COND_HUNGER] > -1)
			gain /= 3;
		if (!check_endure(ch) && ch->pcdata->condition[COND_THIRST] < 10 && ch->pcdata->condition[COND_THIRST] > -1)
			gain /= 3;
	}

	if(check_leadership(ch))
		gain *= 1.5;

	gain *= ch->in_room->heal_rate/100;

	if(ch->in_room->israf(RAF_REGENERATION))
		gain *= 1.5;

	gain = (gain * sect_table[ch->in_room->sector_type].mvheal) / 100;
	for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
	{
		if (obj->item_type == ITEM_FIRE)
		{
			gain *= 2;
			break;
		}
	}

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain *= ch->on->value[3] / 100;

	if (!check_endure(ch) && ch->isaff(AF_POISON))
		gain /= 4;

	if(ch->isaff(AF_VENOM))
		gain /= 4;

	if (ch->isaff(AF_PLAGUE))
		gain /= 8;

	if (ch->isaff(AF_HASTE) || ch->isaff(AF_SLOW))
		gain /=2 ;

	if (get_eq_char(ch,WEAR_LODGE_LEG) != NULL)
		gain *= .75;
	if (get_eq_char(ch,WEAR_LODGE_ARM) != NULL)
		gain *= .75;
	if (get_eq_char(ch,WEAR_LODGE_RIB) != NULL)
		gain *= .75;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"movegain g%d m%d um%d\n\r",gain,ch->max_move-ch->move,UMIN(gain,ch->max_move - ch->move));
	return UMIN(gain,ch->max_move - ch->move);
}

void gain_condition(CHAR_DATA *ch,int iCond,int value){
	int condition;

	if(value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return;

	condition = ch->pcdata->condition[iCond];

	if (condition == -200)
		return;

	if(iCond == COND_DRUNK && value > 0 && ch->pcdata->condition[iCond] + value >= 100)
		value = 100 - ch->pcdata->condition[iCond];
	ch->pcdata->condition[iCond] += value;

	if(ch->pcdata->condition[iCond] <= 0){
		ch->pcdata->condition[iCond] = 0;
		switch (iCond){
			case COND_HUNGER:
				send_to_char("You are hungry.\n\r",ch);
				break;
			case COND_THIRST:
				send_to_char("You are thirsty.\n\r",ch);
				break;
			case COND_DRUNK:
				if(condition != 0) send_to_char("You are sober.\n\r",ch);
				break;
		}
	}
}

void mobile_update(void){
	CHAR_DATA *ch, *ch_next;
	EXIT_DATA *pexit;
	int door;

	/* Examine all mobs. */
	for ( ch = char_list; ch != NULL; ch = ch_next ){
		ch_next = ch->next;

		vithe_regeneration(ch);

		if (!IS_NPC(ch) || ch->in_room == NULL || ch->isaff(AF_CHARM))
			continue;

		if (ch->in_room->area->empty && !ch->isact(AT_UPDATE_ALWAYS))
			continue;

		/* Examine call for special procedure */
		if (ch->spec_fun != 0){
			//if ((*ch->spec_fun)(ch))//nash
			//continue;
		}

		if (ch->pIndexData->pShop != NULL) /* give him some gold */
			if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth){
				ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
				ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
			}

		// Check triggers only if mobile still in default position
		if (ch->position == ch->pIndexData->default_pos){
			/* Delay */
			if (HAS_TRIGGER_MOB(ch,TRIG_DELAY) && ch->mprog_delay > 0){
				if (--ch->mprog_delay <= 0){
					p_percent_trigger(ch,NULL,NULL,NULL,NULL,NULL,TRIG_DELAY);
					continue;
				}
			} 
			if (HAS_TRIGGER_MOB(ch,TRIG_RANDOM)){
				if (p_percent_trigger(ch,NULL,NULL,NULL,NULL,NULL,TRIG_RANDOM))
					continue;
			}
		}

		if (ch->position != POS_STANDING)
			continue;

		if (ch->isact(AT_SCAVENGER) && ch->in_room->contents != NULL && number_bits( 6 ) == 0 ){
			OBJ_DATA *obj,*obj_best;
			int max;

			max      = 1;
			obj_best = 0;
			for ( obj = ch->in_room->contents; obj; obj = obj->next_content ){
				if (CAN_WEAR(obj,ITEM_TAKE) && can_loot(ch,obj) && obj->cost > max  && obj->cost > 0){
					bool found = false;
					for(CHAR_DATA *vch = ch->in_room->people;vch;vch = vch->next_in_room){
						if(vch->on && vch->on == obj)
							found = true;
					}
					if(!found){
						obj_best = obj;
						max      = obj->cost;
					}
				}
			}

			if (obj_best){
				obj_from_room(obj_best);
				obj_to_char(obj_best,ch);
				act("$n gets $p.",ch,obj_best,NULL,TO_ROOM);
			}
		}

		if (!ch->isact(AT_SENTINEL) 
		&& (number_bits(3) == 0)
		&& (!ch->master)
		&& (!ch->fighting)
		&& (door = number_bits(5)) <= 5
		&& (pexit = ch->in_room->exit[door])
		&& (pexit->u1.to_room)
		&& (!IS_SET(pexit->exit_info,EX_CLOSED))
		&& (!IS_SET(pexit->u1.to_room->room_flags,ROOM_NO_MOB))
		&& (!ch->isact(AT_STAY_AREA) || pexit->u1.to_room->area == ch->in_room->area) 
		&& (!ch->isact(AT_OUTDOORS) || !IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)) 
		&& (!ch->isact(AT_INDOORS) || IS_SET(pexit->u1.to_room->room_flags,ROOM_INDOORS)))
			move_char(ch,door,false,true);
	}
}

void char_update( void ){
	CHAR_DATA *ch,*ch_next,*ch_quit;
	char buf[MSL];
	static int food_timer = 3;

	ch_quit	= NULL;

	save_number++;

	if (save_number > 29)
		save_number = 0;

	for (ch = char_list;ch;ch = ch_next){
		AFFECT_DATA *paf,*paf_next;

		ch_next = ch->next;

		if (ch->timer > 30 && !IS_IMMORTAL(ch))
			ch_quit = ch;

		if (ch->position >= POS_STUNNED){
			if (IS_NPC(ch) && ch->zone != NULL && ch->zone != ch->in_room->area && ch->desc == NULL &&  ch->fighting == NULL && !ch->isaff(AF_CHARM) && number_percent() < 5){
				act("$n wanders on home.",ch,NULL,NULL,TO_ROOM);
				extract_char(ch,true);
				continue;
			}

			if(!ch->isaff(AF_PREVENTHEAL) && !ch->isaff(AF_DEGENERATION)){
				if (ch->hit  < ch->max_hit){
					if(double_exp)
						ch->hit = UMIN(ch->max_hit,ch->hit + 2 * hit_gain(ch));
					else
						ch->hit = UMIN(ch->max_hit,ch->hit + hit_gain(ch));
				}
				else
					ch->hit = ch->max_hit;

				if (ch->getmana() < ch->getmaxmana()){
					if(double_exp)
						ch->modmana(2 * mana_gain(ch));
					else
						ch->modmana(mana_gain(ch));
				}
				else
					ch->setmana(ch->getmaxmana());

				if (ch->move < ch->max_move){
					if(double_exp)
						ch->move = UMIN(ch->max_move,ch->move + 2 * move_gain(ch));
					else
						ch->move = UMIN(ch->max_move,ch->move + move_gain(ch));
				}
				else
					ch->move = ch->max_move;
			}
			else{
				if(ch->isaff(AF_DEGENERATION)){
					if (ch->hit > hit_gain(ch))
						ch->hit = UMAX(ch->hit - hit_gain(ch),10);
					else
						ch->hit = 0;

					if (ch->getmana() < mana_gain(ch))
						ch->modmana(-mana_gain(ch));
					else
						ch->setmana(0);

					if (ch->move < move_gain(ch))
						ch->move -= move_gain(ch);
					else
						ch->move = 0;
				}
			}
		}

		if (ch->position == POS_STUNNED)
			update_pos(ch,NULL);

		if (!IS_NPC(ch)){
			OBJ_DATA *obj;

			if(ch->level < LEVEL_IMMORTAL){
				if ((obj = get_eq_char(ch,WEAR_LIGHT)) && obj->item_type == ITEM_LIGHT && obj->value[2] > 0){
					if (--obj->value[2] == 0 && ch->in_room){
						--ch->in_room->light;
						act("$p goes out.",ch,obj,NULL,TO_ROOM);
						act("$p flickers and goes out.",ch,obj,NULL,TO_CHAR);
						extract_obj(obj);
					}
	 				else if (obj->value[2] <= 5 && ch->in_room)
						act("$p flickers.",ch,obj,NULL,TO_CHAR);
				}
				else if ((obj = get_eq_char(ch,WEAR_FLOAT_LIGHT)) && obj->item_type == ITEM_LIGHT && obj->value[2] > 0){
					if (--obj->value[2] == 0 && ch->in_room){
						--ch->in_room->light;
						act("$p goes out.",ch,obj,NULL,TO_ROOM);
						act("$p flickers and goes out.",ch,obj,NULL,TO_CHAR);
						extract_obj(obj);
					}
	 				else if (obj->value[2] <= 5 && ch->in_room)
						act("$p flickers.",ch,obj,NULL,TO_CHAR);
				}

				gain_condition(ch,COND_DRUNK,-1);
				if(--food_timer == 0){//cheap hack to make food last longer than 40 minutes nash code this to also lower hunger by size
					food_timer = 3;
					gain_condition(ch,COND_FULL,ch->size > SIZE_MEDIUM ? -1 : -2);
					if(!check_endure(ch))
						gain_condition(ch,COND_THIRST, -1);
					if(!check_endure(ch))
						gain_condition(ch,COND_HUNGER,ch->size > SIZE_MEDIUM ? -2 : -1);
					if (ch->pcdata->condition[COND_HUNGER] == 0 && ch->pcdata->condition[COND_THIRST] == 0)
						ch->pcdata->condition[COND_FULL] = 0;
				}
			}

			if (++ch->timer >= 12 && !IS_IMMORTAL(ch)){
				if (!ch->was_in_room && ch->in_room){
					ch->was_in_room = ch->in_room;
					if (ch->fighting)
						stop_fighting(ch,true);
					if((obj = ch->on)){
						ch->on = NULL;
						if(CAN_WEAR(obj,ITEM_TAKE))
							get_obj(ch,obj,NULL,true);
					}
					act("$n disappears into {dTh{we {xV{wo{did{x.",ch,NULL,NULL,TO_ROOM);
					send_to_char("You disappear into {dTh{we {xV{wo{did{x.\n\r",ch);
					if (ch->level > 1)
						cql_save_char(ch);
					char_from_room(ch);
					char_to_room(ch,get_room_index(ROOM_VNUM_LIMBO));
				}
			}
		}
		else{//NPCS
			if(ch->rottimer > 0){
				if(--ch->rottimer == 0)
					raw_kill(ch,ch);//Nash needs to code a raw_death to sit in raw_kill for straight up death
			}
		}

		for (paf = ch->affected;paf;paf = paf_next){
			paf_next = paf->next;
			if (paf->duration > 0){
				paf->duration--;
				if (number_range(0,4) == 0 && paf->level > 0)
					paf->level--;// spell strength fades with time
				if(paf->where == TO_AFFECTS && paf->type == gsn_charm_person && paf->duration < 4)
					act("$n seems restless.",ch,NULL,NULL,TO_ROOM);
			}
			else if (paf->duration < 0)
			;
			else{
				if (!paf_next || paf_next->type != paf->type || paf_next->duration > 0){
					if (paf->type > 0 && skill_table[paf->type].msg_off){
						send_to_char(skill_table[paf->type].msg_off, ch);
						if(paf->bitvector == AF_CHARM){
							act(skill_table[paf->type].msg_obj,ch,NULL,NULL,TO_ROOM);
							if(ch->leader)
								remove_charmy(ch->leader,ch);
							ch->master = NULL;
							ch->leader = ch;
						}
						send_to_char("\n\r",ch);
					}
				}
				affect_remove(ch,paf);
			}
		}

		/*
		 * Careful with the damages here,
		 *   MUST NOT refer to ch after damage taken,
		 *   as it may be lethal damage (on NPC).
		 */
		if (ch->isaff(AF_NARCOLEPSY) && ch){
			AFFECT_DATA *narcolepsy;
			AFFECT_DATA af;

			narcolepsy = affect_find(ch->affected,gsn_narcolepsy);

			if (number_percent() < narcolepsy->level/2 && !ch->isaff(AF_SLEEP)){
				act("$n suddenly flops over, deep in sleep.",ch,NULL,NULL,TO_ROOM);
				ch->send("You suddenly get the uncontrollable urge to sle-ZZZzzzzzzzzzzzzzzzzzzzzzzzz...\n\r");
				stop_fighting(ch,true);
				affect_set(ch,TO_AFFECTS,gsn_sleep,narcolepsy->level,1,1,APPLY_NONE,0,AF_SLEEP);
				ch->position = POS_SLEEPING;
			}
		}
		if(ch->isaff(AF_CATALEPSY)){
			int d;
			switch(dice(1,5)){
			case 1:
				d = number_range(0,5);
				if(!(ch->in_room->exit[d])){
					act("You randomly walk into a wall!",ch,NULL,NULL,TO_CHAR);
					act("$n jerks and walks into a wall!",ch,NULL,NULL,TO_ROOM);
				}
				else{
					act("Your body jerks you to the $T!",ch,NULL,dir_name[d],TO_CHAR);
					act("$n jerks and flings $mself $T!",ch,NULL,dir_name[d],TO_ROOM);
					move_char(ch,d,true,true);
				}
				break;
			case 2:
				do_function(ch,&do_stand,"");
				do_function(ch,&do_sit,"");
				do_function(ch,&do_stand,"");
				do_function(ch,&do_sit,"");
				break;
			case 3:
				act("Your arm flails and you hit yourself!",ch,NULL,dir_name[d],TO_CHAR);
				act("$n suddenly hits $mself!",ch,NULL,NULL,TO_ROOM);
				ch->hit -= hit_gain(ch) * 1.5;
				break;
			case 4:
				act("Your arm flails and you hit yourself!",ch,NULL,dir_name[d],TO_CHAR);
				act("$n suddenly hits $mself!",ch,NULL,NULL,TO_ROOM);
				damage(ch,get_random_char(ch,NULL,NULL),10,gsn_strike,DAM_OTHER,true);
				break;
			case 5:
				do_function(ch,&do_scan,"");
				do_function(ch,&do_scan,"");
				do_function(ch,&do_scan,"");
				do_function(ch,&do_sleep,"");
				break;
			}
		}
		if(ch->isaff(AF_NIGHTMARE) && ch->position == POS_SLEEPING){
			if(number_percent() < 75){
				if(number_percent() < 50){
					switch(number_range(0,3)){
					case 1:
						ch->send("The hands of rotting corpses reach up from the ground and claw at you!\n\r");break;
					case 2:
						ch->send("You dream of hundreds of insects crawling from your orifices, covering your body and eatting you!\n\r");break;
					case 3:
						ch->send("The ground begins to shake and crumble as a dark indigo form begins to rise in front of you, grasping your throat in its claws!\n\r");break;
					default:
						ch->send("Horrible dreams and images of your worst fears flood your mind.\n\r");break;
					}
					ch->hit /= 2;
				}
				else{
					switch(number_range(0,2)){
					case 1:
						ch->send("You see Enurai dancing about in a chicken suit!\n\r");break;
					case 2:
						ch->send("You dream that you are soaring through the skies when you suddenly begin plummeting to the ground!\n\r");break;
					default:
						ch->send("Bizarre dreams filled with a swirl of colors and sounds distorts your mind.\n\r");break;
					}
					ch->modmana(ch->getmana()/-2);
				}
			}
			else
				ch->send("Your dreams are disturbing.\n\r");
		}
		if (is_affected(ch,gsn_plague) && ch){
			AFFECT_DATA *af, plague;
			CHAR_DATA *vch;
			int dam;

			if (!ch->in_room)
				continue;

			act("$n writhes in agony as plague sores erupt from $s skin.", ch,NULL,NULL,TO_ROOM);
			send_to_char("You writhe in agony from the plague.\n\r",ch);

			for (af = ch->affected;af;af = af->next)
				if (af->type == gsn_plague)
					break;

			if (!af){
				ch->remaff(AF_PLAGUE);
				continue;
			}

			if (af->level == 1)
				continue;

			for (vch = ch->in_room->people;vch;vch = vch->next_in_room){
				if(IS_NPC(vch) && (vch->isact(AT_IS_CHANGER) || vch->isact(AT_BANKER) || vch->isact(AT_NOKILL)))
					continue;

				if(!IS_NPC(vch) && !vch->isplr(PL_PK))
					continue;

				if (!saves_spell(plague.level*2,vch,DAM_DISEASE) && !IS_IMMORTAL(vch) && !vch->isaff(AF_PLAGUE) && number_bits(4) == 0){
            		send_to_char("You feel hot and feverish.\n\r",vch);
            		act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
            		affect_join(vch,TO_AFFECTS,gsn_plague,af->level - 1,af->slvl,number_range(1,2 * (af->level - 1)),APPLY_STR,-5,AF_PLAGUE);
				}
			}

			dam = number_range(af->level/2,af->level * 4);
			ch->modmana(-dam);
			ch->move -= dam;
			damage(ch,ch,dam,gsn_plague,DAM_DISEASE,true);
		}
		if(!ch->isaff(AF_MARK))
			ch->mark = NULL;
		if (ch->isaff(AF_POISON) && ch){
			AFFECT_DATA *poison;

			poison = affect_find(ch->affected,gsn_poison);

				act( "$n shivers and suffers.",ch,NULL,NULL,TO_ROOM);
				send_to_char("You shiver and suffer.\n\r",ch);
			if (poison)
				damage(ch,ch,number_range(poison->level,poison->level*2),gsn_poison,DAM_POISON,true);
			else
				damage(ch,ch,number_range(ch->level,ch->level*2),gsn_poison,DAM_POISON,true);
		}
		if(ch->isaff(AF_VENOM) && ch){
			AFFECT_DATA *venom;

			venom = affect_find(ch->affected,gsn_venom);

				act("$n turns grey and gags.",ch,NULL,NULL,TO_ROOM);
				send_to_char("You feel sick to the stomach and gag.\n\r",ch);
			if(venom)
				damage(ch,ch,number_range(venom->level,venom->level)*4,gsn_venom,DAM_POISON,true);
			else
				damage(ch,ch,number_range(ch->level,ch->level)*4,gsn_venom,DAM_POISON,true);
		}
		if(ch->isaff(AF_DROWNING) && ch){
			AFFECT_DATA *drown;

			drown = affect_find(ch->affected,skill_lookup("drown"));

			act("$n sputters and coughs up water from $s lungs.",ch,NULL,NULL,TO_ROOM);
			send_to_char("You sputter and cough up some of the water in your lungs.\n\r",ch);

			if(drown)
				damage(ch,ch,number_range(drown->level,drown->level * 3),skill_lookup("drown"),DAM_WATER,true);
			else
				damage(ch,ch,ch->level / 2,skill_lookup("drown"),DAM_WATER,true);
			drown->level *= .75;
		}
		if(ch->in_room && ch->in_room->israf(RAF_FROSTFIELD))
			check_frost_field(ch);

		if(ch->isaff(AF_IMMOLATION) && ch){
			act("$n screams in agony as the flames engulfing $m burn $m horribly.",ch,NULL,NULL,TO_ROOM);
			send_to_char("You scream in agony as the flames engulfing you burn you.\n\r",ch);
			damage(ch,ch,ch->level * 2,skill_lookup("burn"),DAM_FIRE,true);
		}
		if (ch->isaff(AF_SPELLSHIELD) && ch){
			AFFECT_DATA *shield;
			shield = affect_find(ch->affected,gsn_spellshield);
			if (shield){
				if(ch->getmana() < 25){
					act("The shield around $n flickers and fades out.",ch,NULL,NULL,TO_ROOM);
					ch->send("The shield around you flickers and fades out.\n\r");
					affect_remove(ch,shield);
				}
				else{
					act("The shield around $n shimmers.",ch,NULL,NULL,TO_ROOM);
					ch->send("The shield around you shimmers.\n\r");
					ch->modmana(-25);
				}
			}
		}
		if (is_affected(ch,gsn_nausea) && ch){
			AFFECT_DATA *nausea;
			AFFECT_DATA af;

			nausea = affect_find(ch->affected,gsn_nausea);

			if (nausea){
				if(number_percent() < 60){
					act("$n vomits, blowing chunks all over the place!", ch, NULL, NULL, TO_ROOM );
					send_to_char("You feel your stomache heave and you violently vomit everywhere!\n\r", ch );
					damage(ch,ch,number_range(nausea->level,nausea->level*2),gsn_poison,DAM_POISON,true);
					
					if (saves_spell(ch->level,ch,DAM_POISON)){
						act("$n turns slightly green, but it passes.",ch,NULL,NULL,TO_ROOM);
						send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
						return;
					}
					else{
						affect_join(ch,TO_AFFECTS,gsn_poison,ch->level,1,3,APPLY_STR,-1,AF_POISON);
						send_to_char("You feel very sick.\n\r",ch);
						act("$n looks very ill.",ch,NULL,NULL,TO_ROOM);
					}
				}
			}
		}
		if (ch->isaff(AF_AGUE) && ch){
			int rchance = number_percent();

			if(rchance < 25){
				send_to_char("A little pink quasit leers at you and taunts '{GYou smell like poop!{x'.\n\r",ch);
			}
			else if(rchance < 50){
				if (ch->isaff(AF_SLEEP)){
					act("$n screams and thrashes about in $s sleep.",ch,NULL,NULL,TO_ROOM);
					send_to_char("A little pink quasit seems to consume your nightmares!\n\r",ch);
				}
				else{
					act("$n screams and looks about quickly.",ch,NULL,NULL,TO_ROOM);
					send_to_char("A little pink quasit slashes at you and laughs crazily!\n\r",ch);
				}
				damage(ch,ch,number_range(ch->level/2,ch->level),TYPE_HIT,DAM_MENTAL,false);
			}
			else if(rchance < 75){
				if (ch->isaff(AF_SLEEP)){
					act("$n mumbles and curses in $s sleep.",ch,NULL,NULL,TO_ROOM);
					send_to_char("A little pink quasit mocks you and laughs in an echoing spiral of colors and sounds!\n\r",ch);
				}
				else{
					act("$n mutters to $mself.",ch,NULL,NULL,TO_ROOM);
					send_to_char("A little pink quasit wags its butt at you and cackles gleefully.\n\r",ch);
				}
				damage(ch,ch,number_range(ch->level,ch->level*2),TYPE_HIT,DAM_MENTAL,false);
			}
			else if(rchance < 80){
				if (ch->isaff(AF_SLEEP)){
					act("$n chokes and gags in $s sleep.",ch,NULL,NULL,TO_ROOM);
					send_to_char("A little pink quasit strangles you, even in your sleep you feel yourself suffocating!\n\r",ch);
				}
				else{
					act("$n gags and chokes!",ch,NULL,NULL,TO_ROOM);
					send_to_char("A little pink quasit grabs your throat and throttles you!\n\r",ch);
				}
				damage(ch,ch,number_range(ch->level,ch->level*2),TYPE_HIT,DAM_MENTAL,false);
			}
		}
		if (ch->isaff(AF_RUPTURE) && ch){
			AFFECT_DATA *rupture;

			rupture = affect_find(ch->affected,gsn_rupture);

			if (rupture){
				act("A hot stream of {rblood {xshoots out from $n's wounds.",ch,NULL,NULL,TO_ROOM);
				send_to_char("You scream in pain as {rblood {xshoots out of your wounds.\n\r",ch);
				damage(ch,ch,ch->hit/8,gsn_rupture,DAM_NONE,false);
				bool found = false;
				OBJ_DATA *obj;

				for(obj = ch->in_room->contents;obj;obj = obj->next_content){
					if(obj->pIndexData->vnum == OBJ_VNUM_BLOOD)
						found = true;;
				}
				if(!found){
					obj = create_object(get_obj_index(OBJ_VNUM_BLOOD),ch->level);
					obj_to_room(obj,ch->in_room);
				}
			}
		}
		if (ch->position == POS_INCAP && number_range(0,5) == 2){
			ch->send("The pain recedes and you fall into a light slumber.\n\r");
			ch->position = POS_SLEEPING;
			ch->hit = 1;
		}
		if(ch->death_timer > 5){
			ch->send("You have {RDIED{x!!!\n\r");
			act("$n is {RDEAD{x!!",ch,0,0,TO_ROOM);
			victim_die(NULL,ch);
			return;
		}
		if (ch->death_timer > 0){
			ch->position = POS_MORTAL;
			++ch->death_timer;
			ch->send("You are slowly dying, pray for a miracle... or a priest. Or '{Gdie{x' to give up on life.\n\r");
			act("$n is slowly dying, only a miracle... or perhaps a priest could save them now.",ch,NULL,NULL,TO_ROOM);
			return;
		}

//		if (ch->isplr(PL_FORGING))
//			charforge_update(ch);
	}
	if (--global_exp > 0){
		if (global_exp < 4){
			sprintf(buf,"{BThere are {R%d {Bticks of doubles left.{x\n\r",global_exp);
			global_message(0,MAX_LEVEL,buf,0);
			return;
		}
	}
	if (global_exp <= 0 && double_exp){
		global_message(0,MAX_LEVEL,"{BDoubles have run out!{x\n\r",0);
		double_exp = false;
		return;
	}

	/*
	 * Autosave and autoquit.
	 * Check that these chars still exist.
	 */
	for (ch = char_list;ch;ch = ch_next){
		ch_next = ch->next;

		if (ch->desc && ch->desc->descriptor % 30 == save_number)
			cql_save_char(ch);

		if (ch == ch_quit)
			do_function(ch,&do_quit,"");
	}
	return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update( void ){
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf, *paf_next;

	for ( obj = object_list; obj != NULL; obj = obj_next ){
		CHAR_DATA *rch;
		char *message;

		obj_next = obj->next;

		// go through affects and decrement
		for ( paf = obj->affected; paf != NULL; paf = paf_next ){
			paf_next    = paf->next;
			if ( paf->duration > 0 ){
				paf->duration--;
				if (number_range(0,4) == 0 && paf->level > 0)
					paf->level--; // spell strength fades with time
			}
			else if ( paf->duration < 0 )
			;
			else
				affect_remove_obj( obj, paf );
		}

		// Make sure the object is still there before proceeding
		if ( !obj )
			continue;

		if ( (obj->item_type == ITEM_CORPSE_NPC && IS_OBJ_STAT(obj,ITM_EMBALM)) || (obj->timer <= 0 || --obj->timer > 0 ))
			continue;

		switch ( obj->item_type ){
			case ITEM_TRAP:							message = "$p rusts and crumbles.";				break;
			case ITEM_SCARS:						message = "$p slowly fades away.";				break;
			case ITEM_LIGHT:						message = "$p flickers and is extinguished.";	break;
			case ITEM_FOUNTAIN:						message = "$p dries up.";						break;
			case ITEM_PARCHMENT:					message = "$p yellows and rots.";				break;
			case ITEM_RIBS:
			case ITEM_BONES:						message = "$p crumbles into dust.";				break;
			case ITEM_SCROLL:						message = "The words on $p fade.";				break;
			case ITEM_FIRE:							message = "$p burns out.";						break;
			case ITEM_CORPSE_NPC:					message = "$p decays into dust.";				break;
			case ITEM_CORPSE_PC:					message = "$p is devoured by worms.";			break;
			case ITEM_FOOD:							message = "$p goes bad and rots.";				break;
			case ITEM_POTION:						message = "$p has evaporated from disuse.";		break;
			case ITEM_ARMOR:
			case ITEM_SHIELD:						message = "$p crumbles into dust.";				break;
			case ITEM_PORTAL:						message = "$p fades out of existence.";			break;
			case ITEM_CONTAINER:
			if (CAN_WEAR(obj,ITEM_WEAR_FLOAT) || CAN_WEAR(obj,ITEM_WEAR_FLOAT_LIGHT))
			{
				if (obj->contains)					message = "$p flickers and vanishes, spilling its contents on the floor.";
				else								message = "$p flickers and vanishes.";
			}
			else									message = "$p crumbles into dust.";				break;
			default:								message = "$p crumbles into dust.";				break;
		}

		if ( obj->carried_by != NULL )
		{
			if (IS_NPC(obj->carried_by) && obj->carried_by->pIndexData->pShop != NULL)
				obj->carried_by->silver += obj->cost/5;
			else
			{
				act( message, obj->carried_by, obj, NULL, TO_CHAR );
				if ( obj->wear_loc == WEAR_FLOAT || obj->wear_loc == WEAR_FLOAT_LIGHT)
					act(message,obj->carried_by,obj,NULL,TO_ROOM);
			}
		}
		else if ( obj->in_room != NULL && (rch = obj->in_room->people) != NULL )
		{
			if (! (obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT && !CAN_WEAR(obj->in_obj,ITEM_TAKE)))
			{
				act( message, rch, obj, NULL, TO_ROOM );
				act( message, rch, obj, NULL, TO_CHAR );
			}
		}

		if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT || obj->wear_loc == WEAR_FLOAT_LIGHT) && obj->contains)
		{   /* save the contents */
			OBJ_DATA *t_obj, *next_obj;

			for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj)
			{
				next_obj = t_obj->next_content;
				obj_from_obj(t_obj);

				if (obj->in_obj) /* in another object */
					obj_to_obj(t_obj,obj->in_obj);

				else if (obj->carried_by)  /* carried */
					if (obj->wear_loc == WEAR_FLOAT || obj->wear_loc == WEAR_FLOAT_LIGHT)
						if (obj->carried_by->in_room == NULL)
							extract_obj(t_obj);
						else
							obj_to_room(t_obj,obj->carried_by->in_room);
					else
						obj_to_char(t_obj,obj->carried_by);
				else if (obj->in_room == NULL)  /* destroy it */
					extract_obj(t_obj);
				else /* to a room */
					obj_to_room(t_obj,obj->in_room);
			}
		}

		if(obj->item_type == ITEM_FIRE && obj->in_room != NULL && obj->carried_by == NULL)
			obj->in_room->light--;
		extract_obj( obj );
	}

	return;
}

void spell_update(){
    CHAR_DATA *ch,*ch_next,*victim;
	int sn;
	void *vo = NULL;
	int target,mana,spnerf,slvl;

    for (ch = char_list; ch; ch = ch_next){
		ch_next = ch->next;

		if (ch->spelltimer && ch->spellsn){
			sn = ch->spellsn;
			vo = ch->spellvo;
			slvl = ch->spellslvl;
			target = ch->spelltarget;
			victim = ch->spellvictim;
			mana = ch->spellcost;
			if (ch->isaff(AF_OVERDRIVE))
				mana *= 2.5;
			spnerf = ch->isaff(AF_SLOWCAST) ? 1 : check_spell_efficiency(ch) ? 3 : 2;
			ch->spelltimer -= spnerf;

			if(ch->spelltimer < 1){
				int slevel = skill_table[sn].skill_level[ch->pclass] >= ch->level ? ch->level : skill_table[sn].skill_level[ch->pclass];
				int tlev = slevel == ch->level ? slevel : number_range(ch->level - ((ch->level - slevel)/2),ch->level);

				if (ch->isaff(AF_FORCEVOID))
					mana *=2;
				mana = spell_proficiency(ch,mana,SPL_CST);
				ch->modmana(-mana);

				if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"Castlevel:(%d)Cost(%d)\n\r",tlev,mana);

				if (ch->in_room->israf(RAF_FEEDBACK) && get_skill(ch,gsn_spell_resistance) < 1){
					CHAR_DATA *pet;
					pet							= create_mobile(get_mob_index(1));
					pet->level = 1;
					char_to_room(pet,ch->in_room);
					free_string(pet->short_descr);
					pet->short_descr			= str_dup("The room");
					free_string(pet->name);
					pet->name					= str_dup("The room");
					free_string(pet->long_descr);
					pet->long_descr				= str_dup("The room");
					free_string(pet->description);
					pet->description			= str_dup("The room");

					act("$n's release of mana sends a surge from the entire room at $m!",ch,NULL,NULL,TO_ROOM);
					act("Your release of mana sends a surge from the entire room at you!",ch,NULL,NULL,TO_CHAR);
					damage(pet,ch,mana * 2,find_spell(ch,"death ward"),DAM_OTHER,true);
					extract_char(pet,true);
				}
				ch->spellsn = 0;
				ch->spellvo = NULL;
				ch->spelltarget = 0;
				ch->spellvictim = NULL;
				ch->spellcost = 0;
				ch->spellslvl = 0;
				printf_to_char(ch,"You cast '{C%s{x'\n\r",skill_table[sn].name);
				if (IS_NPC(ch) || classes[ch->pclass].fMana >= 1)
					(*skill_table[sn].spell_fun)(sn,tlev,ch,vo,target,slvl);
				else
					(*skill_table[sn].spell_fun)(sn,3 * ch->level/4,ch,vo,target,slvl);
				check_improve(ch,sn,true,5);
				if (ch->isaff(AF_DOUBLECAST) && number_percent() < ch->level / 4){
					if(skill_table[sn].target == TAR_CHAR_DEFENSIVE)
						vo = get_random_char(ch,NULL,NULL);
					if (IS_NPC(ch) || classes[ch->pclass].fMana >= 1)
						(*skill_table[sn].spell_fun)(sn,tlev,ch,vo,target,slvl);
					else
						(*skill_table[sn].spell_fun)(sn,3 * ch->level/4,ch,vo,target,slvl);
					check_improve(ch,sn,true,5);
				}

				if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE || (skill_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR)) && victim != ch && victim->master != ch){
					CHAR_DATA *vch,*vch_next;

					for ( vch = ch->in_room->people; vch; vch = vch_next ){
						vch_next = vch->next_in_room;
						if ( victim == vch && victim->fighting == NULL ){
							multi_hit(victim,ch,TYPE_UNDEFINED,false);
							continue;
						}
					}
				}
			}
			else if(ch->spellfailed > 0){
				if(ch->spellfailed > 1)
					ch->spellfailed = UMAX(1,ch->spellfailed - spnerf);
				else{
					send_to_char("You lost your concentration.\n\r",ch);
					ch->spelltimer = 0;
					ch->spellfailed = 0;

					check_improve(ch,sn,false,5);
					ch->modmana(-mana/3);
					ch->spellsn = 0;
					ch->spellvo = NULL;
					ch->spelltarget = 0;
					ch->spellvictim = NULL;
					ch->spellcost = 0;
				}
				continue;
			}
			else if (ch->spelltimer + 1 < skill_table[sn].beats){
				if ((ch->daze && number_percent() < 33) || ((ch->isaff(AF_CONFUSION) || ch->daze) && number_percent() < 60)){
					if(!check_mental_focus(ch)){
						send_to_char("You become disoriented and lose your concentration.\n\r",ch);
						ch->spelltimer = 0;
						ch->spellfailed = 0;

						check_improve(ch,sn,false,1);
						ch->modmana(-mana);
						ch->spellsn = 0;
						ch->spellvo = NULL;
						ch->spelltarget = 0;
						ch->spellvictim = NULL;
						ch->spellcost = 0;
					}
					else
						ch->daze = 0;
					continue;
				}
			}
		}
		if (ch->chargetime > 0){
			if (ch->bashwait > 0){
				ch->send("Your spell charging was canceled!\n\r");
				ch->chargetime = 0;
				ch->chargevict = NULL;
				continue;
			}
			if (ch->chargevict){
				if (++ch->chargetime > 40){
					int sn = find_spell(ch,"charge shot");

					(*skill_table[sn].spell_fun)(sn,number_range(ch->level,skill_table[sn].skill_level[ch->pclass]),ch,ch->chargevict,TAR_CHAR_OFFENSIVE,ch->getslvl(sn));//nashneedstomakethisusethestoredlevel
				}
			}
			else
				ch->chargetime=0;
		}
    }
}
void room_update(AREA_DATA *pArea){
	ROOM_INDEX_DATA *room;
	int vnum;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum ++){
		if ((room = get_room_index(vnum)))
			room_aff_update(room);
	}

	return;
}


void room_aff_update(ROOM_INDEX_DATA *room){
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = room->affected; paf; paf = paf_next){
		paf_next = paf->next;
		if (paf->duration > 0){
			paf->duration--;
			if (number_range(0,4) == 0 && paf->level > 0)
				paf->level--;  /* spell strength fades with time */
			if(paf->bitvector == RAF_FOREST_MIST && room->people){
				act("The forest mist of the room fills you with a soothing coolness.",NULL,NULL,room,TO_ROOM);
				for(CHAR_DATA *ch = room->people;ch;ch = ch->next_in_room){
					damage(ch,ch,paf->level,paf->type,DAM_OTHER,true);
				}
			}
		}
		else if ( paf->duration < 0 )
		;
		else{
			if (!paf_next
			||  paf_next->type != paf->type
			||  paf_next->duration > 0){
				if (paf->type > 0 && skill_table[paf->type].msg_off)
					act(skill_table[paf->type].msg_off,NULL,NULL,NULL,TO_ROOM);
			}
			affect_remove_room(room,paf);
		}
	}
}
void aggr_update(void){
    CHAR_DATA *wch,*wch_next,*ch,*ch_next,*vch,*vch_next,*victim;

    for (wch = char_list;wch;wch = wch_next){
		wch_next = wch->next;
		if ( IS_NPC(wch)
		||   wch->level >= LEVEL_IMMORTAL
		||  !wch->in_room
		||   wch->in_room->area->empty)
			continue;

		for ( ch = wch->in_room->people; ch != NULL; ch = ch_next ){
			int count;

			ch_next	= ch->next_in_room;


			/* Trilby sez:  ACK! He found me! */
			if(IS_NPC(ch) && can_see(ch,wch) && !ch->isaff(AF_CHARM)){
				MEM_DATA *remember;
				if((remember = get_mem_data(ch,wch))){
					if ( IS_SET(remember->reaction, MEM_AFRAID)
					  && ch->isact(AT_WIMPY)
					  && ch->wait < PULSE_VIOLENCE / 2
					  && number_bits(2) == 1)
						do_flee(ch, "self");
					if (IS_SET(remember->reaction,MEM_HOSTILE)
					 && !ch->fighting
					 && !ch->isaff(AF_CALM)
					 && ch->position > POS_SLEEPING
					 && wch->position > POS_INCAP
					 && can_see(ch,wch)
					 && canFight(ch,wch,false)
					 && !check_animal_magnetism(wch,ch)){
						act("$n screams and attacks{C!{x",ch,NULL,NULL,TO_ROOM);
						multi_hit(ch,wch,TYPE_UNDEFINED,false);
					}
				}
			}

			if ( !IS_NPC(ch)
			||   (!ch->isact(AT_AGGRESSIVE) && !ch->isact(AT_AGGRESSIVE_MOB))
			||   IS_SET(ch->in_room->room_flags,ROOM_SAFE)
			||   ch->isaff(AF_CALM)
			||   ch->fighting != NULL
			||   ch->isaff(AF_CHARM)
			||   !IS_AWAKE(ch)
			||   ( ch->isact(AT_WIMPY) && IS_AWAKE(wch) )
			||   !can_see( ch, wch ) 
			||   number_bits(1) == 0)
				continue;

			count	= 0;
			victim	= NULL;
			for ( vch = wch->in_room->people; vch != NULL; vch = vch_next ){
				vch_next = vch->next_in_room;

				if ( !IS_NPC(vch) && vch->level < LEVEL_IMMORTAL && ch->level >= vch->level - 5 && ( !ch->isact(AT_WIMPY) || !IS_AWAKE(vch) ) && can_see(ch,vch)){
					if ( number_range(0,count) == 0 )
						victim = vch;
					count++;
				}
			}

			if(!victim || !can_see(ch,victim))
				continue;

			if (RIDDEN(ch)){
				/*if (!mount_success(RIDDEN(wch), ch, false)){
					send_to_char("Your mount escapes your control!\n\r", RIDDEN(ch));
					multi_hit( ch, victim, TYPE_UNDEFINED,false);
				}
				else
					send_to_char("You manage to keep your mount under control.\n\r", RIDDEN(ch));*/
				ch->send("Mounts suck.\n\r");
			}
			else
   				multi_hit(ch,victim,TYPE_UNDEFINED,false);
		}
    }
}

void sleep_update(void){
	SLEEP_DATA *temp = first_sleep, *temp_next;

	for( ; temp != NULL; temp = temp_next){
		bool delet = false;

		temp_next = temp->next;

		/* checks to make sure the mob still exists*/
		if(!temp->mob)
			delet = true;
		/*checks to make sure the character is still in the same room as the mob*/
		else if(temp->mob && temp->ch && temp->mob->in_room != temp->ch->in_room)
			delet = true;
		if(delet){
			/* some slick linked list manipulation */
			if(temp->prev)
				temp->prev->next = temp->next;
			if(temp->next)
				temp->next->prev = temp->prev;
			if( temp == first_sleep && (temp->next == NULL || temp->prev == NULL) )
				first_sleep = temp->next;
			free_sleep_data(temp);
			continue;
		}

		if(--temp->timer <= 0){
			program_flow(temp->vnum, temp->prog->code, temp->mob, NULL,NULL,temp->ch, NULL, NULL, temp->line);

			/* more slick linked list manipulation */
			if(temp->prev)
				temp->prev->next = temp->next;
			if(temp->next)
				temp->next->prev = temp->prev;
			if( temp == first_sleep && (temp->next == NULL || temp->prev == NULL) )
				first_sleep = temp->next;
			free_sleep_data(temp);
		}
	}
}

void update_handler( void ){
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int	    pulse_music;
    static  int	    pulse_who;

	struct tm *tm = localtime(&current_time);
	// convert current time into seconds since midnight
	time_t now = ((tm->tm_hour) * 60 * 60) + ((tm->tm_min) * 60) + tm->tm_sec;

	if(now == 0){//midnight
		if(++pulse_who == 4){
			pulse_who = 0;
			mud.d_con = mud.t_con;
			global_message(0,MAX_LEVEL,"Resetting daily max-connected.",0);
			write_max_con();
		}
	}
    if ( --pulse_area <= 0 )
    {
		pulse_area = PULSE_AREA;
		area_update();
    }

    if ( --pulse_music <= 0 )
    {
	//pulse_music	= PULSE_MUSIC;
	////////////////////////song_update();
    }

    if ( --pulse_mobile <= 0 )
    {
		pulse_mobile = PULSE_MOBILE;
		mobile_update();
    }

    if ( --pulse_violence <= 0 )
    {
		pulse_violence = PULSE_VIOLENCE;
		violence_update();
    }

    if ( --pulse_point <= 0 )
    {
		wiznet("TICK!",NULL,NULL,WZ_TICKS,0,0);
		pulse_point = PULSE_TICK;
		weather_update();
		char_update();
		obj_update();
    }

	spell_update();
    aggr_update();
	sleep_update();
    tail_chain();
}
