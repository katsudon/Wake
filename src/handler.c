#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "db.h"
/*
 * Local functions.
 */
void affect_modify		( CHAR_DATA*,AFFECT_DATA*,bool );
void room_update		( AREA_DATA* );
void room_aff_update	( ROOM_INDEX_DATA* );
int check_counterfeit	( CHAR_DATA*,int );
void save_who();

bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim)
{
    if (is_same_group(ch,victim))
		return true;

    if (!IS_NPC(ch))
		return false;

    if (!IS_NPC(victim))
    {
		if (ch->isoff(AST_PLAYERS))
			return true;
		else
			return false;
    }

    if (ch->isaff(AF_CHARM))
		return false;

    if (ch->isoff(AST_ALL))
		return true;

    if (ch->group && ch->group == victim->group)
		return true;

    if (ch->isoff(AST_VNUM) && ch->pIndexData == victim->pIndexData)
		return true;

    if (ch->isoff(AST_RACE) && ch->race == victim->race)
		return true;
     
    if (ch->isoff(AST_ALIGN)
    &&  !ch->isact(AT_NOALIGN)
	&& !victim->isact(AT_NOALIGN)
    &&  ((IS_GOOD(ch) && IS_GOOD(victim)) || (IS_EVIL(ch) && IS_EVIL(victim)) || (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
		return true;

    return false;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
    CHAR_DATA *fch;
    int count = 0;

    if (obj->in_room == NULL)
		return 0;

    for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
		if (fch->on == obj)
		    count++;

    return count;
}

int material_lookup(const char *name){
	int type = -1;

	for (type = 0;material_flags[type].name;type++)
		if (LOWER(name[0]) == LOWER(material_flags[type].name[0]) && !str_cmp(name,material_flags[type].name))
			return type;

	return -1;
}

bool material_metal(OBJ_DATA *obj)
{
	bool found = false;
	int i;
	
	for(i = 0; material_flags[i].name != NULL; i++)
		if(!str_cmp(obj->material,material_flags[i].name)){
			found = true;
			break;
		}
	if(found && material_flags[i].metal)
		return true;
    return false;
}

int weapon_lookup(const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
		if (LOWER(name[0]) == LOWER(weapon_table[type].name[0]) && !str_prefix(name,weapon_table[type].name))
			return type;
    return -1;
}

int weapon_type (const char *name)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
        if (LOWER(name[0]) == LOWER(weapon_table[type].name[0]) && !str_prefix(name,weapon_table[type].name))
            return weapon_table[type].type;
    return WEAPON_EXOTIC;
}

int trapy_type(const char *name)
{
    int type;
 
    for (type = 0; trap_class[type].name != NULL; type++)
        if (LOWER(name[0]) == LOWER(trap_class[type].name[0]) && !str_prefix(name,trap_class[type].name))
            return trap_class[type].bit;
    return TRAP_NULL;
}

char *item_name(int item_type)
{
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
		if (item_type == item_table[type].type)
			return item_table[type].name;
    return "none";
}

char *weapon_name(int weapon_type)
{
    int type;
 
    for (type = 0; weapon_table[type].name != NULL; type++)
        if (weapon_type == weapon_table[type].type)
            return weapon_table[type].name;
    return "exotic";
}

char *trap_name(int trap_type)
{
    int type;
 
    for (type = 0; trap_class[type].name != NULL; type++)
        if (trap_type == trap_class[type].bit)
            return trap_class[type].name;
    return "bugged";
}

int attack_lookup(const char *name)
{
    int att;

    for ( att = 0; attack_table[att].name != NULL; att++)
		if (LOWER(name[0]) == LOWER(attack_table[att].name[0]) && !str_prefix(name,attack_table[att].name))
			return att;
    return 0;
}

int climate_lookup(const char *name)
{
    int att;

    for ( att = 0; climate_flags[att].name != NULL; att++)
		if (LOWER(name[0]) == LOWER(climate_flags[att].name[0]) && !str_prefix(name,climate_flags[att].name))
			return att;
    return 0;
}

long wiznet_lookup(const char *name)
{
    l_int flag;

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
		if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0]) && !str_prefix(name,wiznet_table[flag].name))
			return flag;
    return -1;
}

int guild_lookup(const char *argument){
	for(int i = 0;i < MAX_GUILD;i++)
		if(is_name((char*)argument,guilds[i].name))
			return i;
	return -1;
}

int home_lookup(const char *argument){
	for(int i = 0;i < MAX_HOMETOWN;i++)
		if(!str_cmp(argument,hometowns[i].name))
			return i;
	return -1;
}

int stat_lookup(const char *name){
	int stat;

	for(stat = 0;stat < MAX_STATS;stat++){
		if(LOWER(name[0]) == LOWER(stat_flags[stat].name[0]) && !str_prefix(name,stat_flags[stat].name))
			return stat;
	}
	return -1;
}
char * abbrev_stat_name(int stat){
	for(int i= 0;i < MAX_STATS && abbrev_stat_flags[i].name;i++){
		if(abbrev_stat_flags[i].bit == stat)
			return abbrev_stat_flags[i].name;
	}
	return NULL;
}
int abbrev_stat_lookup(const char *name){
	int stat;

	for(stat = 0;stat < MAX_STATS;stat++){
		if(LOWER(name[0]) == LOWER(abbrev_stat_flags[stat].name[0]) && !str_prefix(name,abbrev_stat_flags[stat].name))
			return stat;
	}
	return -1;
}
int class_lookup(const char *name){
	int clas;

	for (clas = 0;clas < MAX_CLASS;clas++){
		if (LOWER(name[0]) == LOWER(classes[clas].name[0]) && !str_prefix( name,classes[clas].name))
			return clas;
	}

	return -1;
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */
int check_immune(CHAR_DATA *ch, int dam_type)
{
    int immune,def,bit;

    immune = -1;
    def = IS_NORMAL;

    if (dam_type == DAM_NONE)
	return immune;

    if (dam_type <= 3)
    {
		if (ch->res[RS_WEAPON] == 0)
		    def = IS_IMMUNE;
		else if (ch->res[RS_WEAPON] < 100)
			def = IS_RESISTANT;
		else if (ch->res[RS_WEAPON] > 100)
			def = IS_VULNERABLE;
    }
    else /* magical attack */
    {	
		if (ch->res[RS_MAGIC] == 0)
			def = IS_IMMUNE;
		else if (ch->res[RS_MAGIC] < 100)
			def = IS_RESISTANT;
		else if (ch->res[RS_MAGIC] > 100)
			def = IS_VULNERABLE;
    }

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
		case(DAM_BASH):			bit = RS_BASH;		break;
		case(DAM_PIERCE):		bit = RS_PIERCE;	break;
		case(DAM_SLASH):		bit = RS_SLASH;		break;
		case(DAM_FIRE):			bit = RS_FIRE;		break;
		case(DAM_COLD):			bit = RS_COLD;		break;
		case(DAM_LIGHTNING):	bit = RS_LIGHTNING;	break;
		case(DAM_ACID):			bit = RS_ACID;		break;
		case(DAM_POISON):		bit = RS_POISON;	break;
		case(DAM_NEGATIVE):		bit = RS_NEGATIVE;	break;
		case(DAM_HOLY):			bit = RS_HOLY;		break;
		case(DAM_ENERGY):		bit = RS_ENERGY;	break;
		case(DAM_MENTAL):		bit = RS_MENTAL;	break;
		case(DAM_DISEASE):		bit = RS_DISEASE;	break;
		case(DAM_WATER):		bit = RS_WATER;	break;
		case(DAM_LIGHT):		bit = RS_LIGHT;		break;
		case(DAM_CHARM):		bit = RS_CHARM;		break;
		case(DAM_SOUND):		bit = RS_SOUND;		break;
		case(DAM_WIND):			bit = RS_WIND;		break;
		case(DAM_TRAPS):		bit = RS_TRAPS;		break;
		case(DAM_EARTH):		bit = RS_EARTH;		break;
		case(DAM_HARM):			bit = RS_HARM;		break;
		default:				return def;
    }

	if (ch->res[bit] == 0)
		immune = IS_IMMUNE;
	else if (ch->res[bit] < 100 && immune != IS_IMMUNE)
		immune = IS_RESISTANT;
	else if (ch->res[bit] > 100)
	{
		if (immune == IS_IMMUNE)
			immune = IS_RESISTANT;
		else if (immune == IS_RESISTANT)
			immune = IS_NORMAL;
		else
			immune = IS_VULNERABLE;
	}

	if (immune == -1)
		return def;
	else
		return immune;
}

bool is_old_mob(CHAR_DATA *ch)
{
    if (ch->pIndexData == NULL)
		return false;
    else if (ch->pIndexData->new_format)
		return false;
    return true;
}

int grab_skill(CHAR_DATA *ch,int sn)
{
    int skill;

    if (sn == -1) /* shorthand for level based skills */
		skill = ch->level * 5 / 2;
    else if (sn < -1 || sn > MAX_SKILL)
    {
		bug("Bad sn %d in get_skill.",sn);
		skill = 0;
    }
    else if (!IS_NPC(ch))
		    skill = ch->pcdata->learned[sn];
    else /* mobiles */
    {
		if (skill_table[sn].spell_fun != spell_null)
			skill = 40 + 2 * ch->level;
		else if (sn == gsn_sneak || sn == gsn_hide || gsn_desertcover || gsn_blend)
			skill = ch->level * 2 + 20;
		else if ((sn == gsn_dodge && ch->isdef(DF_DODGE)) || (sn == gsn_parry && ch->isdef(DF_PARRY)) || (sn == gsn_sidestep && ch->isdef(DF_SIDESTEP)) || (sn == gsn_divert && ch->isdef(DF_DIVERT)) || (sn == gsn_duck && ch->isdef(DF_DUCK)))
			skill = ch->level * 2;
		else if (sn == gsn_shield_block)
			skill = 10 + 2 * ch->level;
		else if ((sn == gsn_second_attack || sn == gsn_third_attack) && (ch->isact(AT_FIGHTER) || ch->isact(AT_THIEF)))
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_combatives)
			skill = 40 + 2 * ch->level;
		else if (sn == gsn_trip && ch->isoff(OF_TRIP))
			skill = 10 + 3 * ch->level;
		else if ((sn == gsn_bash ||sn == gsn_shieldbash) && ch->isoff(OF_BASH))
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_disarm && (ch->isoff(OF_DISARM) || ch->isact(AT_FIGHTER) || ch->isact(AT_THIEF)))
			skill = 20 + 3 * ch->level;
		else if (sn == gsn_berserk && ch->isoff(OF_BERSERK))
			skill = 3 * ch->level;
		else if (sn == gsn_kick)
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_backstab && ch->isact(AT_THIEF))
			skill = 20 + 2 * ch->level;
		else if (sn == gsn_rescue)
			skill = 40 + ch->level; 
		else if (sn == gsn_recall)
			skill = 40 + ch->level;
		else if (sn == gsn_exotic
		||  sn == gsn_clubs
		||  sn == gsn_swords
		||  sn == gsn_daggers
		||  sn == gsn_polearms
		||  sn == gsn_bludgeons
		||  sn == gsn_cleavers
		||  sn == gsn_lashes
		||  sn == gsn_bows
		||  sn == gsn_martial_arms
		||  sn == gsn_staffs
		||  sn == gsn_sabers
		||  sn == gsn_shortswords
		||  sn == gsn_lances)
			skill = 40 + 5 * ch->level / 2;
		else 
		   skill = 0;
    }

	if (ch->daze > 0 || ch->bashwait > 1)
	{
		if (skill_table[sn].spell_fun != spell_null)
			skill /= 2;
		else
			skill = 2 * skill / 3;
	}

    return URANGE(0,skill,100);
}

int get_skill(CHAR_DATA *ch,int sn){
    int skill = grab_skill(ch,sn);

	if (!IS_NPC(ch)){
		if (IS_DRUNK(ch)){
			if(number_percent() <= grab_skill(ch,gsn_drunkfighting) * .5){
				skill *= 1.1;
				check_improve(ch,gsn_drunkfighting,true,5);
			}
			else{
				skill *= .7;
				check_improve(ch,gsn_drunkfighting,false,2);
			}
		}
		skill = (skill * .8) + ((skill * .2) * get_curr_stat(ch,STAT_INT) / STAT_MAX);
	}

    return URANGE(0,skill,100);
}

int get_weapon_sn(CHAR_DATA *ch,bool dual){
	OBJ_DATA *wield;
	int sn;

	if (!dual)
		wield = get_eq_char(ch,WEAR_WIELD);
	else
		wield = get_eq_char(ch,WEAR_SECONDARY);
	if (wield == NULL || wield->item_type != ITEM_WEAPON)
		sn = gsn_combatives;
	else{
		sn = *weapon_table[wield->value[0]].agsn;//nash needs to check this or code a "get" o wait
		//printf_to_char(ch,"%d %d %s %d\n\r",sn,*weapon_table[wield->value[0]].agsn,weapon_table[wield->value[0]].name,weapon_table[wield->value[0]].gsn);
	}
	return sn;
}

int get_weapon_skill(CHAR_DATA *ch,int sn){
	int skill;

	 /* -1 is exotic */
	if (IS_NPC(ch)){
		if (sn == -1)
			skill = ch->level/2;
		else if (sn == gsn_combatives)
			skill = 40 + 2 * ch->level;
		else 
			skill = 40 + 5 * ch->level / 2;
	}
	else{
		if (sn == -1)
			skill = ch->level / 3;
		else
			skill = check_weapons_knowledge(ch,sn,true);
	}

	return URANGE(0,skill,100);
} 

void reset_char(CHAR_DATA *ch)
{
     OBJ_DATA *obj;
     AFFECT_DATA *af;
     int i,loc,mod,stat;

	if (IS_NPC(ch))
		return;

    if (ch->pcdata->perm_hit == 0 || ch->pcdata->perm_mana == 0 || ch->pcdata->perm_antimana == 0 ||  ch->pcdata->perm_move == 0 || ch->pcdata->last_level == 0)
    {
		/* do a FULL reset */
		for (loc = 0; loc < MAX_WEAR; loc++)
		{
			obj = get_eq_char(ch,loc);
			if (obj == NULL)
				continue;
			if (!obj->enchanted)
				for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
				{
					mod = af->modifier;
					switch(af->location)
					{
						case APPLY_SEX:	ch->sex	-= mod;
							if (ch->sex < 0 || ch->sex >2)
								ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
							break;
						case APPLY_MANA:
							ch->modmaxmana(-mod);
							break;
						case APPLY_HIT:
							ch->max_hit	-= mod;
							break;
						case APPLY_MOVE:
							ch->max_move -= mod;
							break;
					}
				}

            for ( af = obj->affected; af != NULL; af = af->next )
            {
                mod = af->modifier;
                switch(af->location)
                {
                    case APPLY_SEX:
						ch->sex -= mod;
						break;
                    case APPLY_MANA:
						ch->modmaxmana(-mod);
						break;
                    case APPLY_HIT:
						ch->max_hit -= mod;
						break;
                    case APPLY_MOVE:
						ch->max_move -= mod;
						break;
                }
            }
		}
		/* now reset the permanent stats */
		ch->pcdata->perm_hit 	= ch->max_hit;
		ch->pcdata->perm_mana 	= ch->gettruemaxmana();
		ch->pcdata->perm_antimana 	= ch->gettruemaxantimana();
		ch->pcdata->perm_move	= ch->max_move;
		ch->pcdata->last_level	= ch->played/3600;
		if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		{
			if (ch->sex > 0 && ch->sex < 3)
	    		ch->pcdata->true_sex = ch->sex;
			else
				ch->pcdata->true_sex = 0;
		}
    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < MAX_STATS; stat++)
		ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
		ch->pcdata->true_sex = 0;
    ch->sex	= ch->pcdata->true_sex;
    ch->max_hit = ch->pcdata->perm_hit;
    ch->settruemaxmana(ch->pcdata->perm_mana);
    ch->settruemaxantimana(ch->pcdata->perm_antimana);
    ch->max_move = ch->pcdata->perm_move;

    for (i = 0; i < 4; i++)
    	ch->armor[i]	= 100;

    ch->hitroll	= 0;
    ch->damroll	= 0;
    ch->saving_throw = 0;
    ch->saving_spell_throw = 0;

    /* now start adding back the effects */
    for (loc = 0; loc < MAX_WEAR; loc++)
    {
        obj = get_eq_char(ch,loc);
        if (obj == NULL)
            continue;
		for (i = 0; i < 4; i++)
		    ch->armor[i] -= apply_ac(ch,obj,loc,i);

        if (!obj->enchanted)
			for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
			{
				mod = af->modifier;
				switch(af->location)
				{
					//item stat applies go here

                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_END:         ch->mod_stat[STAT_END]  += mod; break;
                case APPLY_AGI:         ch->mod_stat[STAT_AGI]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_RES:         ch->mod_stat[STAT_RES]  += mod; break;
                case APPLY_FTH:         ch->mod_stat[STAT_FTH]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;
                case APPLY_LCK:         ch->mod_stat[STAT_LCK]  += mod; break;
					case APPLY_SEX:
						ch->sex += mod;
						break;
					case APPLY_MANA:
						ch->modmaxmana(mod);
						break;
					case APPLY_HIT:
						ch->max_hit += mod;
						break;
					case APPLY_MOVE:
						ch->max_move += mod;
						break;
					case APPLY_AC:		
						for (i = 0; i < 4; i ++)
							ch->armor[i] += mod;
						break;
					case APPLY_HITROLL:
						ch->hitroll += mod;
						break;
					case APPLY_DAMROLL:
						ch->damroll += mod;
						break;
					case APPLY_SAVES:
						ch->saving_throw += mod;
						break;
					case APPLY_SAVING_SPELL:
						ch->saving_spell_throw += mod;
						break;
			    }
			}
 
        for ( af = obj->affected; af != NULL; af = af->next )
        {
            mod = af->modifier;
            switch(af->location)
            {
                case APPLY_STR:         ch->mod_stat[STAT_STR]  += mod; break;
                case APPLY_END:         ch->mod_stat[STAT_END]  += mod; break;
                case APPLY_AGI:         ch->mod_stat[STAT_AGI]  += mod; break;
                case APPLY_INT:         ch->mod_stat[STAT_INT]  += mod; break;
                case APPLY_RES:         ch->mod_stat[STAT_RES]  += mod; break;
                case APPLY_FTH:         ch->mod_stat[STAT_FTH]  += mod; break;
                case APPLY_WIS:         ch->mod_stat[STAT_WIS]  += mod; break;
                case APPLY_CHA:         ch->mod_stat[STAT_CHA]  += mod; break;
                case APPLY_LCK:         ch->mod_stat[STAT_LCK]  += mod; break;

                case APPLY_SEX:         ch->sex                 += mod; break;
                case APPLY_MANA:        ch->modmaxmana(mod); break;
                case APPLY_HIT:         ch->max_hit             += mod; break;
                case APPLY_MOVE:        ch->max_move            += mod; break;
                case APPLY_AC:
                    for (i = 0; i < 4; i ++)
                        ch->armor[i] += mod;
                    break;
				case APPLY_HITROLL:     ch->hitroll             += mod; break;
                case APPLY_DAMROLL:     ch->damroll             += mod; break;
                case APPLY_SAVES:         ch->saving_throw += mod; break;
                case APPLY_SAVING_SPELL:   ch->saving_spell_throw += mod; break;
            }
		}
    }

    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
    {
        mod = af->modifier;
        switch(af->location)
        {
            case APPLY_STR:				ch->mod_stat[STAT_STR]  += mod; break;
            case APPLY_END:				ch->mod_stat[STAT_END]  += mod; break;
            case APPLY_AGI:				ch->mod_stat[STAT_AGI]  += mod; break;
            case APPLY_INT:				ch->mod_stat[STAT_INT]  += mod; break;
            case APPLY_RES:				ch->mod_stat[STAT_RES]  += mod; break;
            case APPLY_FTH:				ch->mod_stat[STAT_FTH]  += mod; break;
            case APPLY_WIS:				ch->mod_stat[STAT_WIS]  += mod; break;
            case APPLY_CHA:				ch->mod_stat[STAT_CHA]  += mod; break;
            case APPLY_LCK:				ch->mod_stat[STAT_LCK]  += mod; break;

            case APPLY_SEX:				ch->sex                 += mod; break;
            case APPLY_MANA:			ch->modmaxmana(mod); break;
            case APPLY_HIT:				ch->max_hit             += mod; break;
            case APPLY_MOVE:			ch->max_move            += mod; break;
            case APPLY_AC:
                for (i = 0; i < 4; i ++)
                    ch->armor[i] += mod;
                break;
            case APPLY_HITROLL:			ch->hitroll             += mod; break;
            case APPLY_DAMROLL:			ch->damroll             += mod; break;
            case APPLY_SAVES:			ch->saving_throw		+= mod; break;
            case APPLY_SAVING_SPELL:	ch->saving_spell_throw	+= mod; break;
        }
    }

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
		ch->sex = ch->pcdata->true_sex;
}

int get_trust(CHAR_DATA *ch){
    if (!IS_NPC(ch) && ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

    if (ch->trust)
		return ch->trust;

    if (IS_NPC(ch) && ch->level >= LEVEL_HERO)
		return LEVEL_HERO - 1;
    else
		return ch->level;
}

int get_age(CHAR_DATA *ch)
{
    return 17 + (ch->played + (int)(current_time - ch->logon)) / 72000;
}

int get_curr_stat(CHAR_DATA *ch,int stat){
	int max = 0,realstat = 0,classbonus = 0;

	realstat = ch->perm_stat[stat] + ch->mod_stat[stat];

	if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
		max = STAT_MAX + 20;
	else
		max = ch->max_stat[stat];

	return realstat > max ? max : realstat;
}

/* command for returning max training score     TEST REMOVAL FOR NEW STAT SYSTEM :D */
int get_max_train(CHAR_DATA *ch,int stat)
{
    int max;

    if (IS_NPC(ch) || ch->level > LEVEL_IMMORTAL)
		return STAT_MAX;

    max = pc_race_table[ch->race].max_stats[stat];
	//if (stat == STAT_STR)	max += classes[ch->clas].attribute[STAT_STR];

    return UMIN(max,30);
}

int can_carry_n(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
		return 1000;

    if (IS_NPC(ch) && ch->isact(AT_PET))
		return 0;

    return MAX_WEAR +  2 * get_curr_stat(ch,STAT_STR) + ch->level;
}

int can_carry_w(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
		return 10000000;

    if (IS_NPC(ch) && ch->isact(AT_PET))
		return 0;

    return str_app[get_curr_stat(ch,STAT_STR)].carry * 10 + ch->level * 25;
}

bool is_name(char *str,char *namelist){
	char name[MIL],part[MIL];
	char *list, *string;

	/* fix crash on NULL namelist */
	if (namelist == NULL || namelist[0] == '\0')
		return false;

	/* fixed to prevent is_name on "" returning true */
	if (str[0] == '\0')
		return false;

	string = str;
	/* we need ALL parts of string to match part of namelist */
	for ( ; ; )  /* start parsing string */{
		str = one_argument(str,part);

		if (part[0] == '\0')
			return true;

		/* check to see if this is part of namelist */
		list = namelist;
		for ( ; ; )  /* start parsing namelist */{
			list = one_argument(list,name);
			if (name[0] == '\0')  /* this name was not found */
				return false;

			if (!str_prefix(string,name))
				return true; /* full pattern match */

			if (!str_prefix(part,name))
				break;
		}
	}
}

bool is_exact_name(char *str,char *namelist){
	char name[MIL];

	if (namelist == NULL)
		return false;

	for ( ; ; ){
		namelist = one_argument(namelist,name);
		if (name[0] == '\0')
			return false;
		if (!str_cmp(str,name))
			return true;
	}
}

void affect_enchant(OBJ_DATA *obj){
	/* okay, move all the old flags into new vectors if we have to */
	if (!obj->enchanted){
		AFFECT_DATA *paf, *af_new;
		obj->enchanted = true;

		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next){
			af_new = new_affect();

			af_new->next = obj->affected;
			obj->affected = af_new;

			af_new->where	= paf->where;
			af_new->type        = UMAX(0,paf->type);
			af_new->level       = paf->level;
			af_new->duration    = paf->duration;
			af_new->location    = paf->location;
			af_new->modifier    = paf->modifier;
			af_new->bitvector   = paf->bitvector;
		}
	}
}
void affect_to_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ){
	AFFECT_DATA *paf_new;

	paf_new = new_affect();

	*paf_new = *paf;
	paf_new->next = room->affected;
	room->affected = paf_new;

	if (paf->where == TO_AFFECTS)
		room->setraf(paf->bitvector);
	//if (paf->where == TO_EXITS)
	//	room->setraf(paf->bitvector);
}

void affect_remove_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf){
	int where,vector;

	if (room->affected == NULL){
		bug("Affect_remove_room: no affect.",0);
		return;
	}

	where = paf->where;
	vector = paf->bitvector;

	if (paf->bitvector)
		switch(paf->where){
			case TO_AFFECTS:
				room->remraf(paf->bitvector);
				break;
		}

	if (paf == room->affected)
		room->affected = paf->next;
	else{
		AFFECT_DATA *prev;

		for (prev = room->affected; prev != NULL; prev = prev->next){
			if (prev->next == paf){
				prev->next = paf->next;
				break;
			}
		}

		if (!prev){
			bug( "Affect_remove_room: cannot find paf.", 0 );
			return;
		}
	}

	free_affect(paf);
}

void affect_modify(CHAR_DATA *ch,AFFECT_DATA *paf,bool fAdd)
{
    OBJ_DATA *wield;
    int mod,i;

    mod = paf->modifier;

	if (paf->bitvector != 0){
		if (fAdd){
			switch (paf->where){
				case TO_AFFECTS:
					ch->setaff(paf->bitvector);
					break;
				case TO_IMMUNE:
					ch->buff_res(paf->bitvector,0);
					break;
				case TO_RESIST://NASHNEEDSTHISISALREADYBROKENDAMMIT
					ch->buff_res(paf->bitvector,10);
					break;
				case TO_VULN:
					ch->nerf_res(paf->bitvector,10);
					break;
			}
		}
		else{
			switch (paf->where){
				case TO_AFFECTS:
					ch->remaff(paf->bitvector);
					break;
				case TO_IMMUNE:
					ch->reset_res(paf->bitvector);
					break;
				case TO_RESIST:
					ch->nerf_res(paf->bitvector,10);
					break;
				case TO_VULN:
					ch->buff_res(paf->bitvector,10);
					break;
			}
		}
	}

	if (!fAdd)
		mod = -mod;

    switch (paf->location){
		default:
			log_f("BUG: Affect_modify: unknown location %d. Type %d",paf->location,paf->type);
			log_f(" ch %s.",ch->name);
			return;
		case APPLY_NONE:											break;
        case APPLY_STR:				ch->mod_stat[STAT_STR]  += mod; break;
        case APPLY_END:				ch->mod_stat[STAT_END]  += mod; break;
        case APPLY_AGI:				ch->mod_stat[STAT_AGI]  += mod; break;
        case APPLY_INT:				ch->mod_stat[STAT_INT]  += mod; break;
        case APPLY_RES:				ch->mod_stat[STAT_RES]  += mod; break;
        case APPLY_FTH:				ch->mod_stat[STAT_FTH]  += mod; break;
        case APPLY_WIS:				ch->mod_stat[STAT_WIS]  += mod; break;
        case APPLY_CHA:				ch->mod_stat[STAT_CHA]  += mod; break;
        case APPLY_LCK:				ch->mod_stat[STAT_LCK]  += mod; break;

		case APPLY_SEX:				ch->sex					+= mod;	break;
		case APPLY_CLASS:											break;
		case APPLY_LEVEL:											break;
		case APPLY_AGE:												break;
		case APPLY_HEIGHT:											break;
		case APPLY_WEIGHT:											break;
		case APPLY_MANA:			ch->modmaxmana(mod);	break;
		case APPLY_HIT:				ch->max_hit				+= mod;	break;
		case APPLY_MOVE:			ch->max_move			+= mod;	break;
		case APPLY_GOLD:											break;
		case APPLY_EXP:												break;
		case APPLY_AC:
			for (i = 0; i < 4; i ++) ch->armor[i] += mod;
			break;
		case APPLY_HITROLL:			ch->hitroll				+= mod;	break;
		case APPLY_DAMROLL:			ch->damroll				+= mod;	break;
		case APPLY_SAVES:			ch->saving_throw		+= mod;	break;
		case APPLY_SAVING_SPELL:	ch->saving_spell_throw	+= mod;	break;
		case APPLY_SPELL_AFFECT:  									break;
    }

	if(!fAdd){
		int oldbit = paf->bitvector,oldwhere = paf->where;
		for(paf = ch->affected;paf;paf = paf->next){
			if (paf->where = oldwhere && paf->bitvector == oldbit){
				switch (paf->where){
					case TO_AFFECTS:
						ch->setaff(paf->bitvector);
						break;
					case TO_IMMUNE:
						ch->buff_res(paf->bitvector,0);
						break;
					case TO_RESIST:
						ch->buff_res(paf->bitvector,10);
						break;
					case TO_VULN:
						ch->nerf_res(paf->bitvector,10);
						break;
				}
			}
		}
	}

    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if (!IS_NPC(ch) && (wield = get_eq_char(ch,WEAR_WIELD)) != NULL && get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10)){
		act("You drop $p.",ch,wield,NULL,TO_CHAR);
		act("$n drops $p.",ch,wield,NULL,TO_ROOM);
		obj_from_char(wield);
		obj_to_room(wield,ch->in_room);
    }
}

AFFECT_DATA *affect_find(AFFECT_DATA *paf,int sn)
{
    AFFECT_DATA *paf_find;
    
    for ( paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
        if (paf_find->type == sn)
			return paf_find;

    return NULL;
}

void affect_check(CHAR_DATA *ch,int where,int vector){
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
		return;

    for (paf = ch->affected; paf != NULL; paf = paf->next)
		if (paf->where == where && paf->bitvector == vector){
			switch (where){
				case TO_AFFECTS:
					ch->setaff(vector);
					break;
				case TO_IMMUNE:
					ch->buff_res(vector,0);
					break;
				case TO_RESIST:
					ch->buff_res(vector,10);
					break;
				case TO_VULN:
					ch->nerf_res(vector,10);
					break;
			}
			return;
		}

    for (obj = ch->carrying; obj != NULL; obj = obj->next_content){
		if (obj->wear_loc == -1)
			continue;

        for (paf = obj->affected; paf != NULL; paf = paf->next)
			if (paf->where == where && paf->bitvector == vector){
				switch (where){
					case TO_AFFECTS:
						ch->setaff(vector);
						break;
					case TO_IMMUNE:
						ch->buff_res(vector,0);
						break;
					case TO_RESIST:
						ch->buff_res(vector,50);
						break;
					case TO_VULN:
						ch->nerf_res(vector,50);
              
				}
				return;
			}

        if (obj->enchanted)
		    continue;

        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->where == where && paf->bitvector == vector){
                switch (where){
                    case TO_AFFECTS:
                        ch->setaff(vector);
                        break;
                    case TO_IMMUNE:
						ch->buff_res(vector,0);
                        break;
                    case TO_RESIST:
                        ch->buff_res(vector,10);
                        break;
                    case TO_VULN:
                        ch->nerf_res(vector,10);
                        break;
                }
                return;
            }
    }
}

void affect_join(CHAR_DATA *ch,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector){
	AFFECT_DATA af;
	af.id			= 0;
	af.where		= towhere;
	af.type			= type;
	af.level		= level;
	af.slvl			= slvl;
	af.duration		= duration;
	af.location		= location;
	af.modifier		= modifier;
	af.bitvector	= bitvector;
	affect_join(ch,&af);
}

void affect_set(CHAR_DATA *owner,CHAR_DATA *ch,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector){
	AFFECT_DATA af;
	af.id			= 0;
	af.parent		= owner;
	af.where		= towhere;
	af.type			= type;
	af.level		= level;
	af.slvl			= slvl;
	af.duration		= duration;
	af.location		= location;
	af.modifier		= modifier;
	af.bitvector	= bitvector;
	affect_to_char(ch,&af);
}
void affect_set(CHAR_DATA *owner,ROOM_INDEX_DATA *room,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector){
	AFFECT_DATA af;
	af.id			= 0;
	af.parent		= owner;
	af.where		= towhere;
	af.type			= type;
	af.level		= level;
	af.slvl			= slvl;
	af.duration		= duration;
	af.location		= location;
	af.modifier		= modifier;
	af.bitvector	= bitvector;
	affect_to_room(room,&af);
}

void affect_set(CHAR_DATA *ch,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector){
	AFFECT_DATA af;
	af.id			= 0;
	af.where		= towhere;
	af.type			= type;
	af.level		= level;
	af.slvl			= slvl;
	af.duration		= duration;
	af.location		= location;
	af.modifier		= modifier;
	af.bitvector	= bitvector;
	affect_to_char(ch,&af);
}

void affect_set(ROOM_INDEX_DATA *room,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector){
	AFFECT_DATA af;
	af.id			= 0;
	af.where		= towhere;
	af.type			= type;
	af.level		= level;
	af.slvl			= slvl;
	af.duration		= duration;
	af.location		= location;
	af.modifier		= modifier;
	af.bitvector	= bitvector;
	affect_to_room(room,&af);
}

void affect_set(OBJ_DATA *obj,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector){
	AFFECT_DATA af;
	af.id			= 0;
	af.where		= towhere;
	af.type			= type;
	af.level		= level;
	af.slvl			= slvl;
	af.duration		= duration;
	af.location		= location;
	af.modifier		= modifier;
	af.bitvector	= bitvector;
	affect_to_obj(obj,&af);
}

void affect_to_char(CHAR_DATA *ch,AFFECT_DATA *paf){
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf);	/* in case we missed it when we set up paf */
    paf_new->next	= ch->affected;
    ch->affected	= paf_new;
	paf_new->id = 0;

    affect_modify(ch,paf_new,true);
}

void affect_to_obj(OBJ_DATA *obj,AFFECT_DATA *paf){
    AFFECT_DATA *paf_new;

    paf_new = new_affect();

    *paf_new		= *paf;

    VALIDATE(paf);	/* in case we missed it when we set up paf */
    paf_new->next	= obj->affected;
    obj->affected	= paf_new;

    /* apply any affect vectors to the object's extras */
    if (paf->bitvector)
        switch (paf->where){
	        case TO_OBJECT:
			    SET_BIT(obj->extra_flags,paf->bitvector);
			    break;
			case TO_WEAPON:
				if (obj->item_type == ITEM_WEAPON)
					SET_WFLAG(obj,paf->bitvector);
				break;
        }
}

void affect_remove(CHAR_DATA *ch,AFFECT_DATA *paf)
{
    int where,vector;

    if (ch->affected == NULL)
    {
		bug("Affect_remove: no affect.",0);
		return;
    }

    affect_modify(ch,paf,false);
    where = paf->where;
    vector = paf->bitvector;

    if (paf == ch->affected)
		ch->affected	= paf->next;
    else
    {
		AFFECT_DATA *prev;

		for ( prev = ch->affected; prev != NULL; prev = prev->next )
			if (prev->next == paf)
			{
				prev->next = paf->next;
				break;
			}

		if (prev == NULL)
		{
			bug("Affect_remove: cannot find paf.",0);
			return;
		}
    }

    free_affect(paf);

    affect_check(ch,where,vector);
    return;
}

void affect_remove_obj(OBJ_DATA *obj,AFFECT_DATA *paf){
	CHAR_DATA *rch;
	int where, vector;
	if (obj->affected == NULL){
		bug("Affect_remove_object: no affect.",0);
		return;
	}

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_modify(obj->carried_by,paf,false);

	where = paf->where;
	vector = paf->bitvector;

	/* remove flags from the object if needed */
	if (paf->bitvector)
		switch( paf->where){
				case TO_OBJECT:
					REMOVE_BIT(obj->extra_flags,paf->bitvector);
					break;
				case TO_WEAPON:
					if (obj->item_type == ITEM_WEAPON)
						REMOVE_WFLAG(obj,paf->bitvector);
					break;
			}

	if (paf == obj->affected)
		obj->affected    = paf->next;
	else{
		AFFECT_DATA *prev;

		for ( prev = obj->affected; prev != NULL; prev = prev->next )
			if (prev->next == paf){
				prev->next = paf->next;
				break;
			}

		if (prev == NULL){
			bug("Affect_remove_object: cannot find paf.",0);
			return;
		}
	}
	if ( paf->type > 0 && skill_table[paf->type].msg_obj ){
		if (obj->carried_by != NULL){
			rch = obj->carried_by;
			act(skill_table[paf->type].msg_obj,rch,obj,NULL,TO_CHAR);
		}
		if (obj->in_room != NULL && obj->in_room->people != NULL){
			rch = obj->in_room->people;
			act(skill_table[paf->type].msg_obj,
			rch,obj,NULL,TO_ALL);
		}
	}

	free_affect(paf);

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_check(obj->carried_by,where,vector);
	return;
}

void affect_strip(CHAR_DATA *ch,int sn)
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
		paf_next = paf->next;
		if (paf->type == sn)
			affect_remove(ch,paf);
    }

    return;
}

bool is_affected(CHAR_DATA *ch,int sn)
{
    AFFECT_DATA *paf;

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
		if (paf->type == sn)
		    return true;

    return false;
}

void affect_join(CHAR_DATA *ch,AFFECT_DATA *paf){
	AFFECT_DATA *paf_old;
	bool found;

	found = false;
	for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next ){
		if (paf_old->type == paf->type){
			paf->level = (paf->level += paf_old->level) / 2;
			paf->duration += paf_old->duration;
			paf->modifier = UMAX(paf_old->modifier + paf->modifier,10);//NASHNEEDSTOCHECKWITHBOON
			affect_remove(ch,paf_old);
			break;
		}
	}

	affect_to_char(ch,paf);
}

void char_from_room(CHAR_DATA *ch)
{
    OBJ_DATA *obj;

    if (ch->in_room == NULL)
    {
		bug("Char_from_room: NULL.",0);
		return;
    }

    if (!IS_NPC(ch))
		--ch->in_room->area->nplayer;

    if ((obj = get_eq_char(ch,WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room->light > 0)
		--ch->in_room->light;
	else if ((obj = get_eq_char(ch,WEAR_FLOAT_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room->light > 0)
		--ch->in_room->light;

    if (ch == ch->in_room->people)
		ch->in_room->people = ch->next_in_room;
    else
    {
		CHAR_DATA *prev;

		for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
		{
			if (prev->next_in_room == ch)
			{
				prev->next_in_room = ch->next_in_room;
				break;
			}
		}

		if (prev == NULL)
			bug("Char_from_room: ch not found.",0);
    }

    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->on 	     = NULL;/* sanity check! */
    return;
}

void char_to_room(CHAR_DATA *ch,ROOM_INDEX_DATA *pRoomIndex){//nash make sure the plague is being calced right here
	OBJ_DATA *obj;

	if (pRoomIndex == NULL)
	{
		ROOM_INDEX_DATA *room;

		bug("Char_to_room: NULL.",0);

		if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
			char_to_room(ch,room);
		return;
	}

	ch->in_room			= pRoomIndex;
	ch->next_in_room	= pRoomIndex->people;
	pRoomIndex->people	= ch;

	if (!IS_NPC(ch))
	{
		if (ch->in_room->area->empty)
		{
			ch->in_room->area->empty = false;
			ch->in_room->area->age = 0;
		}
		++ch->in_room->area->nplayer;
	}

	if ((obj = get_eq_char(ch,WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		++ch->in_room->light;
	else if((obj = get_eq_char(ch,WEAR_FLOAT_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		++ch->in_room->light;

	if (ch->isaff(AF_PLAGUE))
	{
		AFFECT_DATA *af, plague;
		CHAR_DATA *vch;
	    
		for ( af = ch->affected; af != NULL; af = af->next )
			if (af->type == gsn_plague)
				break;

		if (af == NULL)
		{
			ch->remaff(AF_PLAGUE);
			return;
		}
	    
		if (af->level == 1)
			return;
	    
		for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
		{
			if (!saves_spell(plague.level - 2,vch,DAM_DISEASE) && !IS_IMMORTAL(vch) && !vch->isaff(AF_PLAGUE) && number_bits(6) == 0)
			{
        		send_to_char("You feel hot and feverish.\n\r",vch);
        		act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
				affect_join(vch,TO_AFFECTS,gsn_plague,af->level-1,af->slvl,number_range(1,2*(af->level - 1)),APPLY_STR,-5,AF_PLAGUE);
			}
		}
	}
}

void obj_to_char(OBJ_DATA *obj,CHAR_DATA *ch)
{
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number(obj);
    ch->carry_weight	+= get_obj_weight(obj);
}

void obj_from_char(OBJ_DATA *obj){
	CHAR_DATA *ch;

	if(!obj){
		bug("Obj_from_char: Null obj",0);
		return;
	}
	if(!(ch = obj->carried_by)){
		log_f("Obj_from_char: '%s' has null ch.",obj->name);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch,obj);

	if (ch->carrying == obj)
		ch->carrying = obj->next_content;
	else{
		OBJ_DATA *prev;

		for ( prev = ch->carrying; prev != NULL; prev = prev->next_content ){
			if (prev->next_content == obj){
				prev->next_content = obj->next_content;
				break;
			}
		}

		if (prev == NULL)
			bug("Obj_from_char: obj not in list.",0);
	}

	obj->carried_by	 = NULL;
	obj->next_content	 = NULL;
	ch->carry_number	-= get_obj_number(obj);
	ch->carry_weight	-= get_obj_weight(obj);
}

int get_ac(CHAR_DATA *ch,OBJ_DATA *obj,int type){
	if(!ch)
		return obj->value[type];

	switch(obj->armortype_flags){
	case 0:
	case 1:
		return obj->value[type];
	case 2:
		if(grab_skill(ch,gsn_light_armor) > 0){
			return obj->value[type] + (50 * ch->getslvl(gsn_light_armor) / 5);
		}
		return obj->value[type];
	case 3:
		if(grab_skill(ch,gsn_medium_armor) > 0){
			return obj->value[type] + (75 * ch->getslvl(gsn_medium_armor) / 5);
		}
		return obj->value[type];
	case 4:
		if(grab_skill(ch,gsn_heavy_armor) > 0){
			return obj->value[type] + (100 * ch->getslvl(gsn_heavy_armor) / 5);
		}
		return obj->value[type];
	}
}

int apply_ac(CHAR_DATA *ch,OBJ_DATA *obj,int iWear,int type){
	int n = 0;
	if (obj->item_type != ITEM_ARMOR && obj->item_type != ITEM_SHIELD)
		;
	else
	switch (iWear){
		case WEAR_SHIELD:
		case WEAR_SECONDARY:
			if(obj->item_type == ITEM_SHIELD)
				n = 2 * get_ac(ch,obj,type);;
			break;
		case WEAR_TORSO:
		case WEAR_HEAD:
		case WEAR_LEGS:
		case WEAR_SHOULDER:
		case WEAR_FEET:
		case WEAR_HANDS:
		case WEAR_TATTOO:
		case WEAR_FACE:
		case WEAR_ARMS:
		case WEAR_NECK_1:
		case WEAR_NECK_2:
		case WEAR_ELBOW_L:
		case WEAR_ELBOW_R:
		case WEAR_KNEE_L:
		case WEAR_KNEE_R:
		case WEAR_SHIN_L:
		case WEAR_SHIN_R:
		case WEAR_ABOUT:
		case WEAR_WAIST:
		case WEAR_WRIST_L:
		case WEAR_WRIST_R:
		case WEAR_ANKLE_L:
		case WEAR_ANKLE_R:
		case WEAR_HOLD_L:
		case WEAR_HOLD_R:
		case WEAR_EAR_L:
		case WEAR_EAR_R:
		case WEAR_FINGER_L:
		case WEAR_FINGER_R:
			n = get_ac(ch,obj,type);
			break;
	}
	return n;
}

/*
 * Find a piece of eq on a character.                  NOTE TO SELF: RETOOL TO REACT TO CROSSBOWS!
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj;

    if (ch == NULL)
		return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		if (obj->wear_loc == iWear)
			return obj;

    return NULL;
}

OBJ_DATA *get_eq_hold(CHAR_DATA *ch)
{
    OBJ_DATA *obj;

    if (ch == NULL)
		return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		if (obj->wear_loc == WEAR_HOLD_R || obj->wear_loc == WEAR_HOLD_L)
			return obj;

    return NULL;
}

OBJ_DATA *get_it_char(CHAR_DATA *ch,int item_type)
{
    OBJ_DATA *obj;

    if (ch == NULL)
		return NULL;

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		if (obj->item_type == item_type)
			return obj;

    return NULL;
}

void equip_char(CHAR_DATA *ch,OBJ_DATA *obj,int iWear){
	AFFECT_DATA *paf;
	int i;

	if (get_eq_char(ch,iWear)){
		bugf("Equip_char: %s already equipped (%d).",ch->name,iWear);
		return;
	}

	for(i = 0; i < 4; i++)
		ch->armor[i] -= apply_ac(ch,obj,iWear,i);
	obj->wear_loc	 = iWear;
	if(obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD && obj->armortype_flags != ARMOR_NONE)
		ch->worn_armor[obj->armortype_flags]++;

	if(!obj->enchanted)
		for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
			if (paf->location != APPLY_SPELL_AFFECT)
				affect_modify(ch,paf,true);

	for(paf = obj->affected; paf != NULL; paf = paf->next )
		if (paf->location == APPLY_SPELL_AFFECT)
			affect_to_char(ch,paf);
		else
			affect_modify(ch,paf,true);

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL)
		++ch->in_room->light;
}

/*
 * Unequip a char with an obj.
 */
void unequip_char(CHAR_DATA *ch,OBJ_DATA *obj){
	AFFECT_DATA *paf = NULL,*paf_next = NULL,*lpaf = NULL,*lpaf_next = NULL;
	int i;

	if (obj->wear_loc == WEAR_NONE){
		bug("Unequip_char: already unequipped.",0);
		return;
	}

	for (paf = obj->affected;paf != NULL;paf = paf_next){
		paf_next = paf->next;
		if (paf->type == gsn_grasp || paf->type == gsn_grip)
			affect_remove_obj(obj,paf);
	}

	if (IS_SET(obj->extra_flags,ITM_HIDDEN)){
		act("$p becomes less concealed.",ch,obj,NULL,TO_CHAR);
		REMOVE_BIT(obj->extra_flags,ITM_HIDDEN);
	}
	for (i = 0; i < 4; i++)
		ch->armor[i]	+= apply_ac(ch,obj,obj->wear_loc,i);
	if((obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD)
	&& obj->armortype_flags != ARMOR_NONE
	&& ch->worn_armor[obj->armortype_flags] > 0)
		ch->worn_armor[obj->armortype_flags]--;

	obj->wear_loc	 = -1;

	if (!obj->enchanted){
		for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
			if (paf->location == APPLY_SPELL_AFFECT){
				for ( lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next )
					lpaf_next = lpaf->next;
					if ((lpaf->type == paf->type) && (lpaf->level == paf->level) && (lpaf->location == APPLY_SPELL_AFFECT)){
						affect_remove(ch,lpaf);
						lpaf_next = NULL;
					}
			}
			else{
				affect_modify(ch,paf,false);
				affect_check(ch,paf->where,paf->bitvector);
			}
	}
	for (paf = obj->affected;paf != NULL;paf = paf->next){
		if (paf->location == APPLY_SPELL_AFFECT){
			bug("Norm-Apply: %d",0);
			for (lpaf = ch->affected;lpaf != NULL;lpaf = lpaf_next){
				lpaf_next = lpaf->next;
				if ((lpaf->type == paf->type) && (lpaf->level == paf->level) && (lpaf->location == APPLY_SPELL_AFFECT)){
					bug("location = %d",lpaf->location);
					bug("type = %d",lpaf->type);
					affect_remove(ch,lpaf);
					lpaf_next = NULL;
				}
			}
		}
		else{
			affect_modify(ch,paf,false);
			affect_check(ch,paf->where,paf->bitvector);	
		}
	}

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room != NULL && ch->in_room->light > 0)
		--ch->in_room->light;
	return;
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list(OBJ_INDEX_DATA *pObjIndex,OBJ_DATA *list){
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
		if (obj->pIndexData == pObjIndex)
			nMatch++;

    return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room(OBJ_DATA *obj)
{
    ROOM_INDEX_DATA *in_room;
    CHAR_DATA *ch;

    if ((in_room = obj->in_room) == NULL)
    {
		bugf("obj_from_room: NULL, %s.",obj->short_descr);
		return;
    }

    for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
		if (ch->on == obj)
		    ch->on = NULL;

    if (obj == in_room->contents)
		in_room->contents = obj->next_content;
    else
    {
		OBJ_DATA *prev;

		for ( prev = in_room->contents; prev; prev = prev->next_content )
			if (prev->next_content == obj)
			{
				prev->next_content = obj->next_content;
				break;
			}

		if (prev == NULL)
		{
			bug("Obj_from_room: obj not found.",0);
			return;
		}
    }

    obj->in_room      = NULL;
    obj->next_content = NULL;
    return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room(OBJ_DATA *obj,ROOM_INDEX_DATA *pRoomIndex)
{
	obj->next_content		= pRoomIndex->contents;
	pRoomIndex->contents	= obj;
	obj->in_room			= pRoomIndex;
	obj->carried_by			= NULL;
	obj->in_obj				= NULL;
	return;
}

/*
 * Move an object into an object.
 */
void obj_to_obj(OBJ_DATA *obj,OBJ_DATA *obj_to){
	obj->next_content	= obj_to->contains;
	obj_to->contains	= obj;
	obj->in_obj			= obj_to;
	obj->in_room		= NULL;
	obj->carried_by		= NULL;
	obj_to->cont_count++;

	if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
		obj->cost = 0;

	for ( ; obj_to != NULL; obj_to = obj_to->in_obj )
		if (obj_to->carried_by != NULL){
			obj_to->carried_by->carry_number += get_obj_number(obj);
			obj_to->carried_by->carry_weight += get_obj_weight(obj) * WEIGHT_MULT(obj_to) / 100;
		}
}

/*
 * Move an object out of an object.
 */
void obj_from_obj(OBJ_DATA *obj){
    OBJ_DATA *obj_from;

    if ((obj_from = obj->in_obj) == NULL){
		bug("Obj_from_obj: null obj_from.",0);
		return;
    }

    if (obj == obj_from->contains)
		obj_from->contains = obj->next_content;
    else{
		OBJ_DATA *prev;

		for ( prev = obj_from->contains; prev; prev = prev->next_content )
			if (prev->next_content == obj)
			{
				prev->next_content = obj->next_content;
				break;
			}

		if (prev == NULL)
		{
			bug("Obj_from_obj: obj not found.",0);
			return;
		}
    }

    obj->next_content = NULL;
    obj->in_obj       = NULL;

	obj_from->cont_count--;
    for ( ; obj_from != NULL; obj_from = obj_from->in_obj )
		if (obj_from->carried_by != NULL)
		{
			obj_from->carried_by->carry_number -= get_obj_number(obj);
			obj_from->carried_by->carry_weight -= get_obj_weight(obj) * WEIGHT_MULT(obj_from) / 100;
		}

    return;
}

/*
 * Extract an obj from the world.
 */
void extract_obj(OBJ_DATA *obj)
{
    OBJ_DATA *obj_content;
    OBJ_DATA *obj_next;

    if (obj->in_room != NULL)
		obj_from_room(obj);
    else if (obj->carried_by != NULL)
		obj_from_char(obj);
    else if (obj->in_obj != NULL)
		obj_from_obj(obj);

    for ( obj_content = obj->contains; obj_content; obj_content = obj_next )
    {
		obj_next = obj_content->next_content;
		extract_obj(obj_content);
    }

    if (object_list == obj)
		object_list = obj->next;
    else
    {
		OBJ_DATA *prev;

		for ( prev = object_list; prev != NULL; prev = prev->next )
			if (prev->next == obj)
			{
				prev->next = obj->next;
				break;
			}

		if (prev == NULL)
		{
			bug("Extract_obj: obj %d not found.",obj->pIndexData->vnum);
			return;
		}
    }

    --obj->pIndexData->count;
    free_obj(obj);
    return;
}

/*
 * Extract a char from the world.
 */
void extract_char(CHAR_DATA *ch,bool fPull){
    CHAR_DATA *wch;
    OBJ_DATA *obj,*obj_next;

    /* doesn't seem to be necessary, but just in case*/
    if (!ch->in_room){
		bug("Extract_char: NULL.",0);
		return;
    }

    nuke_pets(ch);
    ch->pet = NULL;

    if (fPull)
		die_follower(ch,NULL);

    stop_fighting(ch,true);

    for (obj = ch->carrying;obj;obj = obj_next){
		obj_next = obj->next_content;
		extract_obj(obj);
    }

    if (ch->in_room != NULL)
        char_from_room(ch);

	// Death room is set in the clan tabe now
	if (!fPull){
		char_to_room(ch,get_room_index(hometowns[ch->hometown].death));
		return;
	}

    if (IS_NPC(ch))
		--ch->pIndexData->count;
    else{
		nuke_pets(ch);
		ch->pet = NULL;
    }

    if (ch->desc != NULL && ch->desc->original){
		do_function(ch,&do_return,"");
		ch->desc = NULL;
    }

    for (wch = char_list;wch;wch = wch->next){
		if (wch->reply == ch)
			wch->reply = NULL;
		if (ch->mprog_target == wch)
			wch->mprog_target = NULL;
    }

    if (ch == char_list)
       char_list = ch->next;
    else{
		CHAR_DATA *prev;

		for(prev = char_list;prev;prev = prev->next)
			if (prev->next == ch){
				prev->next = ch->next;
				break;
			}

		if (!prev){
			bug("Extract_char: char not found.",0);
			return;
		}
    }

    if (ch->desc)
		ch->desc->character = NULL;
    free_char(ch);
}

CHAR_DATA *get_char_room(CHAR_DATA *ch,ROOM_INDEX_DATA *room,char *argument)
{
    char arg[MIL];
    CHAR_DATA *rch;
    int number;
    int count;

    number = number_argument(argument,arg);
    count  = 0;
    if (!str_cmp(arg,"self"))
		return ch;

    if (ch && room)
    {
		bug("get_char_room received multiple types (ch/room)",0);
		return NULL;
    }

    if (ch)
		rch = ch->in_room->people;
    else
		rch = room->people;

    for ( ; rch != NULL; rch = rch->next_in_room )
    {
		if ((ch && !can_see(ch,rch)) || !is_name(arg,rch->name))
			continue;
		if (++count == number)
			return rch;
    }

    return NULL;
}

CHAR_DATA *get_char_around(CHAR_DATA *ch, char *argument, int mdepth, bool throughdoor)
{
	CHAR_DATA *rch;
	EXIT_DATA *pExit;
    ROOM_INDEX_DATA *scan_room;
	int door,depth;
	bool found = false;

	for (door = 0; door < 6; door++)
	{
		scan_room = ch->in_room;
		for (depth = 0; depth < mdepth && !found; depth++)
			if (!found && ((pExit = scan_room->exit[door]) != NULL))
			{
				scan_room = pExit->u1.to_room;
				if ((rch = get_char_room(NULL,scan_room,argument)) != NULL && rch != ch && (!throughdoor && !IS_SET(pExit->exit_info,EX_CLOSED)) && can_see(ch,rch))
					return rch;
			}
	}
	return NULL;
}

CHAR_DATA *get_char_world(CHAR_DATA *ch,char *argument){
	CHAR_DATA *wch;
	char arg[MIL];
	int number,count;

	if (ch && (wch = get_char_room(ch,NULL,argument)))
		return wch;

	number = number_argument(argument,arg);
	count = 0;

	for (wch = char_list;wch;wch = wch->next){
		if(!wch->in_room || (ch && !can_see(ch,wch)) || !is_name(arg,wch->name))
			continue;
		if (++count == number)
			return wch;
	}
	return NULL;
}

OBJ_DATA *get_obj_type(OBJ_INDEX_DATA *pObjIndex){
    OBJ_DATA *obj;

    for(obj = object_list;obj;obj = obj->next)
		if (obj->pIndexData == pObjIndex)
		    return obj;
    return NULL;
}

OBJ_DATA *get_obj_list(CHAR_DATA *ch,char *argument,OBJ_DATA *list){
    char arg[MIL];
    OBJ_DATA *obj;
    int number,count;

    number = number_argument(argument,arg);
    count  = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
		if (can_see_obj(ch,obj) && is_name(arg,obj->name)){
			if (++count == number)
			return obj;
		}
    return NULL;
}
OBJ_DATA *get_obj_id(int id){
	OBJ_DATA *obj;

	for(obj = object_list; obj; obj = obj->next ){
		if(obj->id == id)
			return obj;
	}

	return NULL;
}
OBJ_DATA *get_obj_carry(CHAR_DATA *ch,char *argument,CHAR_DATA *viewer)
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument(argument,arg);
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        if ( obj->wear_loc == WEAR_NONE && ( viewer ? can_see_obj( viewer, obj ) : true ) &&   is_name( arg, obj->name ) )
            if ( ++count == number )
				return obj;

    return NULL;
}

OBJ_DATA *get_obj_wear(CHAR_DATA *ch,char *argument,bool character){
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

    number = number_argument(argument,arg);
    count  = 0;
    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
        if (obj->wear_loc != WEAR_NONE && (character ? can_see_obj(ch,obj) : true) && is_name(arg,obj->name))
            if (++count == number)
                return obj;

    return NULL;
}

OBJ_DATA *get_obj_here(CHAR_DATA *ch,ROOM_INDEX_DATA *room,char *argument){
	OBJ_DATA *obj;
	int number, count;
	char arg[MIL];

	if(ch && room){
		bug("get_obj_here received a ch and a room",0);
		return NULL;
	}

	number = number_argument(argument,arg);
	count = 0;

	if(ch){
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if(obj)
			return obj;

		if ((obj = get_obj_carry(ch,argument,ch)))
			return obj;

		if ((obj = get_obj_wear(ch,argument,true)))
			return obj;
	}
	else{
		for(obj = room->contents;obj;obj = obj->next_content){
			if (!is_name(arg,obj->name))
				continue;
			if (++count == number)
				return obj;
		}
	}

	return NULL;
}

OBJ_DATA *get_obj_world(CHAR_DATA *ch,char *argument)
{
    char arg[MIL];
    OBJ_DATA *obj;
    int number;
    int count;

    if (ch && (obj = get_obj_here(ch,NULL,argument)) != NULL)
		return obj;

    number = number_argument(argument,arg);
    count  = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
		if ((ch && !can_see_obj(ch,obj)) || !is_name(arg,obj->name))
			continue;
		if (++count == number)
			return obj;
    }

    return NULL;
}

void deduct_cost(CHAR_DATA *ch,int cost){
	int silver = 0, gold = 0;

	cost = check_counterfeit(ch,cost);

	silver = UMIN(ch->silver,cost);

	if (silver < cost){
		gold = ((cost - silver + 99) / 100);
		silver = cost - 100 * gold;
	}

	ch->gold -= gold;
	ch->silver -= silver;

	if (ch->gold < 0){
		bug("deduct costs: gold %d < 0",ch->gold);
		ch->gold = 0;
	}
	if (ch->silver < 0){
		bug("deduct costs: silver %d < 0",ch->silver);
		ch->silver = 0;
	}
}   

OBJ_DATA *create_money(int gold,int silver)
{
    char buf[MSL];
    OBJ_DATA *obj;

    if (gold < 0 || silver < 0 || (gold == 0 && silver == 0))
    {
		bug("Create_money: zero or negative money.",UMIN(gold,silver));
		gold = UMAX(1,gold);
		silver = UMAX(1,silver);
    }

    if (gold == 0 && silver == 1)
		obj = create_object(get_obj_index(OBJ_VNUM_SILVER_ONE),0);
    else if (gold == 1 && silver == 0)
		obj = create_object(get_obj_index(OBJ_VNUM_GOLD_ONE),0);
    else if (silver == 0)
    {
        obj = create_object(get_obj_index(OBJ_VNUM_GOLD_SOME),0);
        sprintf(buf,obj->short_descr,gold);
        free_string(obj->short_descr);
        obj->short_descr        = str_dup(buf);
        obj->value[1]           = gold;
        obj->cost               = gold;
		obj->weight				= gold/5;
    }
    else if (gold == 0)
    {
        obj = create_object(get_obj_index(OBJ_VNUM_SILVER_SOME),0);
        sprintf(buf,obj->short_descr,silver);
        free_string(obj->short_descr);
        obj->short_descr        = str_dup(buf);
        obj->value[0]           = silver;
        obj->cost               = silver;
		obj->weight				= silver/20;
    }
    else
    {
		obj = create_object(get_obj_index(OBJ_VNUM_COINS),0);
		sprintf(buf,obj->short_descr,silver,gold);
		free_string(obj->short_descr);
		obj->short_descr	= str_dup(buf);
		obj->value[0]		= silver;
		obj->value[1]		= gold;
		obj->cost			= 100 * gold + silver;
		obj->weight			= gold / 5 + silver / 20;
    }

    return obj;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number(OBJ_DATA *obj)
{
    int number;
 
    if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY || obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
        number = 0;
    else
        number = 1;
 
    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        number += get_obj_number(obj);
 
    return number;
}

int get_obj_weight(OBJ_DATA *obj)
{
    int weight;
    OBJ_DATA *tobj;

    weight = obj->weight;
    for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
		weight += get_obj_weight(tobj) * WEIGHT_MULT(obj) / 100;

    return weight;
}

int get_true_weight(OBJ_DATA *obj,bool exclude)
{
    int weight=0;
 
	if(!exclude)
		weight = obj->weight;

    for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
        weight += get_obj_weight(obj);
 
    return weight;
}

int calc_light(ROOM_INDEX_DATA *room){
	int light = room->light;

    if (((weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK) && !IS_SET(room->room_flags,ROOM_LIGHT)) || IS_SET(room->room_flags,ROOM_DARK))
		light--;

	if(IS_SET(room->room_flags,ROOM_LIGHT))
		return 1;

	if (room->israf(RAF_NEGATIVE))
		light = -1;

	return light;
}

bool room_is_dark(ROOM_INDEX_DATA *pRoomIndex){
	if (pRoomIndex == NULL)
		return false;

	if(pRoomIndex->israf(RAF_DARK))
		return true;
	if(pRoomIndex->israf(RAF_LIGHT))
		return false;

    if (pRoomIndex->sector_type == SECT_INSIDE || pRoomIndex->sector_type == SECT_CITY)
		return false;

    if (calc_light(pRoomIndex) < 0)
		return true;

    return false;
}

bool is_room_owner(CHAR_DATA *ch,ROOM_INDEX_DATA *room){
    if (room->owner == NULL || room->owner[0] == '\0')
		return false;

    return is_name(ch->name,room->owner);
}

bool room_is_private(ROOM_INDEX_DATA *pRoomIndex){
	CHAR_DATA *rch;
	int count;

	if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
		return true;

	count = 0;
	for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
		count++;

	if (IS_SET(pRoomIndex->room_flags,ROOM_PRIVATE) && count >= 2)
		return true;

	if (IS_SET(pRoomIndex->room_flags,ROOM_SOLITARY) && count >= 1)
		return true;

	if (IS_SET(pRoomIndex->room_flags,ROOM_IMP_ONLY))
		return true;

	return false;
}

bool can_see_room(CHAR_DATA *ch,ROOM_INDEX_DATA *pRoomIndex){
	if(ch->iscomm(CM_DEBUG))ch->send("1");
    if (IS_SET(pRoomIndex->room_flags,ROOM_IMP_ONLY) && get_trust(ch) < MAX_LEVEL-2)
		return false;
	if(ch->iscomm(CM_DEBUG))ch->send("2");

	if(IS_NPC(ch) && ch->isact(AT_TRUE_SIGHT))
		return true;
	if(ch->iscomm(CM_DEBUG))ch->send("3");

	if(!IS_NPC(ch) && get_trust(ch) >= LEVEL_IMMORTAL && ch->isplr(PL_HOLYLIGHT))
		return true;
	if(ch->iscomm(CM_DEBUG))ch->send("4");

	if(ch->isaff(AF_BLIND) || ch->in_room->israf(RAF_SMOKESCREEN))
		return false;
	if(ch->iscomm(CM_DEBUG))ch->send("5");

    if (room_is_dark(ch->in_room) && !ch->isaff(AF_DARK_VISION))
		return false;
	if(ch->iscomm(CM_DEBUG))ch->send("6");

    if (IS_SET(pRoomIndex->room_flags,ROOM_GODS_ONLY) && !IS_IMMORTAL(ch))
		return false;
	if(ch->iscomm(CM_DEBUG))ch->send("7");

    if (IS_SET(pRoomIndex->room_flags,ROOM_HEROES_ONLY) && !IS_IMMORTAL(ch))
		return false;
	if(ch->iscomm(CM_DEBUG))ch->send("8");

    if (IS_SET(pRoomIndex->room_flags,ROOM_NEWBIES_ONLY) && ch->level > 10 && !IS_IMMORTAL(ch))
		return false;
	if(ch->iscomm(CM_DEBUG))ch->send("9");

    return true;
}

bool can_see(CHAR_DATA *ch,CHAR_DATA *victim){
    if (ch == victim)
		return true;

	if(ch->position < POS_RESTING)
		return false;
    if (get_trust(ch) < victim->invis_level)
		return false;

    if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
		return false;

	if (IS_NPC(ch) && ch->isact(AT_TRUE_SIGHT))
		return true;

    if ((!IS_NPC(ch) && ch->isplr(PL_HOLYLIGHT)) || (IS_NPC(ch) && IS_IMMORTAL(ch)))
		return true;

    if (ch->isaff(AF_BLIND))
		return false;

    if (room_is_dark(victim->in_room) && !ch->isaff(AF_DARK_VISION))
		return false;

	if (victim->isaff(AF_INVISIBLE)){
		if(!ch->isaff(AF_DETECT_INVIS))
			return false;
		else{
			int cperc = number_percent(),vperc = number_percent();
			AFFECT_DATA *caf = affect_find(ch->affected,gsn_detect_invis),*vaf = affect_find(victim->affected,gsn_invis);
			if(!caf)
				cperc *= number_range(1,5);
			else
				cperc *= caf->modifier;
			if(!vaf)
				vperc *= number_range(1,4);
			else
				vperc *= vaf->modifier;
			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Detectinvis:c%d v%d\n\r",cperc,vperc);
			if(cperc < vperc)
				return false;
			return true;
		}
	}

	if (victim->isaff(AF_PERFECTSNEAK) && !IS_IMMORTAL(ch) && !victim->fighting){
		if(time_info.hour > 5 && time_info.hour < 19){
			affect_strip(victim,gsn_penumbralveil);
			if(victim->remaff(AF_PERFECTSNEAK))
				return can_see(ch,victim);
		}
		return false;
	}

    if ((victim->isaff(AF_HIDE) || victim->isaff(AF_CAMOFLAGE)) && !victim->fighting){
		if (!ch->isaff(AF_DETECT_HIDDEN)){
			int chance;
			if (victim->isaff(AF_HIDE))
				chance = get_skill(victim,gsn_hide);
			else if (is_affected(victim,gsn_blend))
				chance = get_skill(victim,gsn_blend) * .75;
			else if (is_affected(victim,gsn_desertcover))
				chance = get_skill(victim,gsn_desertcover) * .75;

 			chance -= 5 * get_curr_stat(ch,STAT_INT) / STAT_MAX;
			chance -= (ch->level - victim->level) * 3/2;

			if (number_percent() < chance)
				return false;
		}
		else if((victim->isaff(AF_HIDE) && !check_awareness(ch,victim,gsn_hide)) || (victim->isaff(AF_CAMOFLAGE) && !check_awareness(ch,victim,gsn_blend)))
			return false;
    }

    return true;
}

bool can_see_move(CHAR_DATA *ch,CHAR_DATA *victim){//NASHNASHNASHHHHHHHHHHHHHHHHHH
	int sn = gsn_awareness,vsn = gsn_sneak,skill = get_skill(ch,sn),vskill = get_skill(victim,vsn);
	if(victim->isaff(AF_SNEAK)){
		if(vskill < 1)
			vskill = number_range(1,3) * number_range(1,50);
		else
			vskill *= victim->getslvl(vsn);
		if(skill < 1)
			skill = number_range(1,3) * number_range(1,50);
		else
			skill *= ch->getslvl(sn);
		if(skill < vskill)
			return false;
	}
	return can_see(ch,victim);
}

bool can_see_who(CHAR_DATA *ch,CHAR_DATA *victim){
	if (ch == victim)
		return true;
	if(ch->position < POS_RESTING)
		return false;
	if (get_trust(ch) < victim->invis_level)
		return false;
	if (get_trust(ch) < victim->incog_level && ch->in_room != victim->in_room)
		return false;
	if (IS_NPC(ch) && ch->isact(AT_TRUE_SIGHT))
		return true;
	if ((!IS_NPC(ch) && ch->isplr(PL_HOLYLIGHT)) || (IS_NPC(ch) && IS_IMMORTAL(ch)))
		return true;
	return true;
}

bool can_see_obj(CHAR_DATA *ch,OBJ_DATA *obj){
	if (!IS_NPC(ch) && ch->isplr(PL_HOLYLIGHT))
		return true;

	if(IS_NPC(ch) && ch->isact(AT_TRUE_SIGHT))
		return true;

	if (IS_SET(obj->extra_flags,ITM_VIS_DEATH))
		return false;

	if (ch->isaff(AF_BLIND) && obj->item_type != ITEM_POTION)
		return false;

	if (room_is_dark(ch->in_room))
	{
		if (ch->isaff(AF_DARK_VISION) || IS_OBJ_STAT(obj,ITM_GLOW))
			return true;
		else
			return false;
	}

	if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		return true;

	if (IS_OBJ_STAT(obj,ITM_GLOW))
		return true;

	if (obj->item_type == ITEM_TRAP && do_detecttraps(ch))
		return true;

	if (IS_SET(obj->extra_flags,ITM_INVIS) && !ch->isaff(AF_DETECT_INVIS))
		return false;

	if (IS_SET(obj->extra_flags,ITM_HIDDEN)){
		OBJ_DATA *pObj;
		for(int iWear = 0;iWear < MAX_WEAR; iWear++){
			if ((pObj = get_eq_char(ch,iWear)) == NULL)
				continue;
			if(pObj == obj)
				return true;
		}
		if(!ch->isaff(AF_DETECT_HIDDEN) && !check_awareness(ch,obj))
			return false;
	}

	return true;
}

/* True if char can drop obj. */
bool can_drop_obj(CHAR_DATA *ch,OBJ_DATA *obj)
{
    if (!IS_SET(obj->extra_flags,ITM_NODROP))
		return true;

    if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
		return true;

    return false;
}

/* Return ascii name of an affect location.*/
char *affect_loc_name(int location)
{
    if (apply_flags[location].name != NULL)
		return apply_flags[location].name;
	else
		bug("Affect_location_name: unknown location %d.",location);
	return "(unknown)";
}

/* Return ascii name of an affect bit vector.*/
char *affect_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_AFF;i++){
		if(bits[i])
			for(n=0;affect_flags[n].name;n++){
				if (affect_flags[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,affect_flags[n].name);
				}
			}
	}
    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *affect_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;affect_flags[n].name;n++)
		if (affect_flags[n].bit == bit)
			sprintf(buf,"%s",affect_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

/* Return ascii name of extra flags vector.*/
char *extra_bit_name(l_int bit)
{
    static char buf[512];
	char buf2[50];

    buf[0] = '\0';
	if (IS_SET(bit,ITM_GLOW)) strcat(buf," glow");
	if (IS_SET(bit,ITM_HUM)) strcat(buf," hum");
	if (IS_SET(bit,ITM_FORGING)) strcat(buf," forging");
	if (IS_SET(bit,ITM_DARK)) strcat(buf," dark");
	if (IS_SET(bit,ITM_LOCK)) strcat(buf," lock");
	if (IS_SET(bit,ITM_EVIL)) strcat(buf," evil");
	if (IS_SET(bit,ITM_INVIS)) strcat(buf," invis");
	if (IS_SET(bit,ITM_MAGIC)) strcat(buf," magic");
	if (IS_SET(bit,ITM_NODROP)) strcat(buf," nodrop");
	if (IS_SET(bit,ITM_GRIPPED)) strcat(buf," gripped");
	if (IS_SET(bit,ITM_ANTI_NEUTRAL)) strcat(buf," antineutral");
	if (IS_SET(bit,ITM_NOREMOVE)) strcat(buf," noremove");
	if (IS_SET(bit,ITM_INVENTORY)) strcat(buf," inventory");
	if (IS_SET(bit,ITM_NOPURGE)) strcat(buf," nopurge");
	if (IS_SET(bit,ITM_ROT_DEATH)) strcat(buf," rotdeath");
	if (IS_SET(bit,ITM_VIS_DEATH)) strcat(buf," visdeath");
	if (IS_SET(bit,ITM_LODGED)) strcat(buf," lodged");
	if (IS_SET(bit,ITM_EMBALM)) strcat(buf," embalmed");
	if (IS_SET(bit,ITM_NOLOCATE)) strcat(buf," nolocate");
	if (IS_SET(bit,ITM_MELT_DROP)) strcat(buf," meltdrop");
	if (IS_SET(bit,ITM_HAD_TIMER)) strcat(buf," hadtimer");
	if (IS_SET(bit,ITM_SELL_EXTRACT)) strcat(buf," sellextract");
	if (IS_SET(bit,ITM_GRASPED)) strcat(buf," grasped");
	if (IS_SET(bit,ITM_BURN_PROOF)) strcat(buf," burnproof");
	if (IS_SET(bit,ITM_NOUNCURSE)) strcat(buf," nouncurse");
	if (IS_SET(bit,ITM_HIDDEN)) strcat(buf," hidden");
	if (IS_SET(bit,ITM_FLAMING)) strcat(buf," flaming");
	if (IS_SET(bit,ITM_FROSTED)) strcat(buf," frosted");
	if (IS_SET(bit,ITM_BARBED)) strcat(buf," barbed");
	if (IS_SET(bit,ITM_SHOCKED)) strcat(buf," shocking");
	if (IS_SET(bit,ITM_SHOCK_PROOF)) strcat(buf," shockproof");
	if (IS_SET(bit,ITM_INLAY)) strcat(buf," inlayed");
    return (buf[0] != '\0') ? buf+1 : (char *)"none";
}

char *exclude_bit_name(l_int bit)
{
    static char buf[512];

    buf[0] = '\0';
/*	if (IS_SET(bit,EXL_MAGE)) strcat(buf," mages");
	if (IS_SET(bit,EXL_CLERIC)) strcat(buf," clerics");
	if (IS_SET(bit,EXL_THIEF)) strcat(buf," thieves");
	if (IS_SET(bit,EXL_FIGHTER)) strcat(buf," fighters");
	if (IS_SET(bit,EXL_BANDIT)) strcat(buf," bandits");
	if (IS_SET(bit,EXL_SHAMAN)) strcat(buf," shamans");
	if (IS_SET(bit,EXL_PALADIN)) strcat(buf," paladins");
	if (IS_SET(bit,EXL_DUELIST)) strcat(buf," duelists");
	if (IS_SET(bit,EXL_NECROMANCER)) strcat(buf," necromancers");
	if (IS_SET(bit,EXL_BERSERKER)) strcat(buf," berserkers");
	if (IS_SET(bit,EXL_KNIGHT)) strcat(buf," knights");
	if (IS_SET(bit,EXL_DRAGONRIDER)) strcat(buf," dragonriders");
	if (IS_SET(bit,EXL_RANGER)) strcat(buf," rangers");
	if (IS_SET(bit,EXL_NOMAD)) strcat(buf," nomads");
	if (IS_SET(bit,EXL_BARD)) strcat(buf," bards");
	if (IS_SET(bit,EXL_DRUID)) strcat(buf," druids");
	if (IS_SET(bit,EXL_BHUNTER)) strcat(buf," bounty-hunters");
	if (IS_SET(bit,EXL_PSION)) strcat(buf," psions");
	if (IS_SET(bit,EXL_MONK)) strcat(buf," monks");
	if (IS_SET(bit,EXL_HUMAN)) strcat(buf," humans");
	if (IS_SET(bit,EXL_PIXIE)) strcat(buf," pixies");
	if (IS_SET(bit,EXL_MINOTAUR)) strcat(buf," minotaur");
	if (IS_SET(bit,EXL_HALFELF)) strcat(buf," half-elves");
	if (IS_SET(bit,EXL_STARELF)) strcat(buf," star-elves");
	if (IS_SET(bit,EXL_JEWELELF)) strcat(buf," jewel-elves");
	if (IS_SET(bit,EXL_GREENELF)) strcat(buf," greenelfs");
	if (IS_SET(bit,EXL_MIDNIGHTELF)) strcat(buf," midnight-elves");
	if (IS_SET(bit,EXL_DWARF)) strcat(buf," dwarves");
	if (IS_SET(bit,EXL_JARELHALFLING)) strcat(buf," ja'rel-halflings");
	if (IS_SET(bit,EXL_GYPSYHALFLING)) strcat(buf," gypsy_halflings");
	if (IS_SET(bit,EXL_THRISTLEHALFLING)) strcat(buf," thristle_halflings");
	if (IS_SET(bit,EXL_ORC)) strcat(buf," orcs");
	if (IS_SET(bit,EXL_HALFORC)) strcat(buf," half-orcs");
	if (IS_SET(bit,EXL_OGRE)) strcat(buf," ogres");*/
}

/* return ascii name of an act vector */
char *act_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	if (bits[AT_IS_NPC]){
		for (i = 0;i<MAX_ACT;i++){
			if(bits[i])
				for(n=0;act_flags[n].name;n++){
					if (act_flags[n].bit == i){
						if (buf[0] != '\0')
							strcat(buf," ");
						strcat(buf,act_flags[n].name);
					}
				}
		}
	}
	else{
		for (i = 0;i<MAX_PLR;i++){
			if(bits[i])
				for(n=0;plr_flags[n].name;n++){
					if (plr_flags[n].bit == i){
						if (buf[0] != '\0')
							strcat(buf," ");
						strcat(buf,plr_flags[n].name);
					}
				}
		}
	}
    return (buf[0] != '\0') ? buf : (char *)"none";
}
char *act_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;act_flags[n].name;n++)
		if (act_flags[n].bit == bit)
			sprintf(buf,"%s",act_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *armortype_bit_name(int atype)
{
    static char buf[512];

    buf[0] = '\0';
	strcat(buf,armortype_flags[atype].name);

    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *comm_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_CMM;i++){
		if(bits[i])
			for(n=0;comm_flags[n].name;n++){
				if (comm_flags[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,comm_flags[n].name);
				}
			}
	}
    return (buf[0] != '\0') ? buf : (char *)"none";
}
char *comm_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;comm_flags[n].name;n++)
		if (comm_flags[n].bit == bit)
			sprintf(buf,"%s",comm_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *res_bits_name(int *bits){
    static char buf[MIL];
	char buf2[100];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_RES;i++){
		if(bits[i] != 100)
			for(n=0;res_flags[n].name;n++){
				if (res_flags[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,res_flags[n].name);
					strcat(buf,": ");
					sprintf(buf2,"%d",bits[i]);
					strcat(buf,buf2);
				}
			}
	}
    return (buf[0] != '\0') ? buf : (char *)"none";
}
char *res_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;res_flags[n].name;n++)
		if (res_flags[n].bit == bit)
			sprintf(buf,"%s",res_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *wear_bit_name(int wear_flags)
{
    static char buf[512];

    buf [0] = '\0';
    if (wear_flags & ITEM_TAKE				) strcat(buf," take"		);
    if (wear_flags & ITEM_WEAR_FINGER		) strcat(buf," finger"		);
    if (wear_flags & ITEM_WEAR_EAR			) strcat(buf," ear"		);
    if (wear_flags & ITEM_WEAR_NECK			) strcat(buf," neck"		);
    if (wear_flags & ITEM_WEAR_ELBOW		) strcat(buf," elbow"		);
    if (wear_flags & ITEM_WEAR_KNEE			) strcat(buf," knee"		);
    if (wear_flags & ITEM_WEAR_SHIN			) strcat(buf," shin"		);
    if (wear_flags & ITEM_WEAR_TORSO			) strcat(buf," torso"		);
    if (wear_flags & ITEM_WEAR_HEAD			) strcat(buf," head"		);
    if (wear_flags & ITEM_WEAR_LEGS			) strcat(buf," legs"		);
    if (wear_flags & ITEM_WEAR_SHOULDER		) strcat(buf," shoulder"	);
    if (wear_flags & ITEM_WEAR_QUIVER		) strcat(buf," quiver"		);
    if (wear_flags & ITEM_WEAR_FEET			) strcat(buf," feet"		);
    if (wear_flags & ITEM_WEAR_HANDS		) strcat(buf," hands"		);
    if (wear_flags & ITEM_WEAR_FACE			) strcat(buf," face"		);
    if (wear_flags & ITEM_WEAR_PENDANT		) strcat(buf," pendant"	);
    if (wear_flags & ITEM_WEAR_TATTOO		) strcat(buf," tattoo"		);
    if (wear_flags & ITEM_WEAR_ARMS			) strcat(buf," arms"		);
    if (wear_flags & ITEM_WEAR_SHIELD		) strcat(buf," shield"		);
    if (wear_flags & ITEM_WEAR_ABOUT		) strcat(buf," body"		);
    if (wear_flags & ITEM_WEAR_WAIST		) strcat(buf," waist"		);
    if (wear_flags & ITEM_WEAR_WRIST		) strcat(buf," wrist"		);
    if (wear_flags & ITEM_WEAR_ANKLE		) strcat(buf," ankle"		);
    if (wear_flags & ITEM_WIELD				) strcat(buf," wield"		);
    if (wear_flags & ITEM_HOLD				) strcat(buf," hold"		);
    if (wear_flags & ITEM_NO_SAC			) strcat(buf," nosac"		);
    if (wear_flags & ITEM_WEAR_FLOAT		) strcat(buf," float"		);
    if (wear_flags & ITEM_WEAR_FLOAT_LIGHT	) strcat(buf," fllight"		);

    return ( buf[0] != '\0' ) ? buf+1 : (char *)"none";
}

char *form_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_FRM;i++){
		if(bits[i])
			for(n=0;form_flags[n].name;n++){
				if (form_flags[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,form_flags[n].name);
				}
			}
	}
    return (buf[0] != '\0') ? buf : (char *)"none";
}
char *form_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;form_flags[n].name;n++)
		if (form_flags[n].bit == bit)
			sprintf(buf,"%s",form_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *part_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_PRT;i++){
		if(bits[i])
			for(n=0;part_flags[n].name;n++){
				if (part_flags[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,part_flags[n].name);
				}
			}
	}
	return (buf[0] != '\0') ? buf : (char *)"none";
}
char *part_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;part_flags[n].name;n++)
		if (part_flags[n].bit == bit)
			sprintf(buf,"%s",part_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *weapon_bit_name(int weapon_flags)
{
    static char buf[512];

    buf[0] = '\0';
    if (weapon_flags & WPN_FLAMING		) strcat(buf," flaming");
    if (weapon_flags & WPN_FROST		) strcat(buf," frost");
    if (weapon_flags & WPN_SHOCKING 	) strcat(buf," shocking");
    if (weapon_flags & WPN_WATER 		) strcat(buf," water");
    if (weapon_flags & WPN_EARTH 		) strcat(buf," earth");
    if (weapon_flags & WPN_LIGHT 		) strcat(buf," light");
    if (weapon_flags & WPN_SOUND 		) strcat(buf," sound");
    if (weapon_flags & WPN_WIND 		) strcat(buf," wind");
    if (weapon_flags & WPN_MAGIC 		) strcat(buf," magic");
    if (weapon_flags & WPN_HOLY			) strcat(buf," holy");
    if (weapon_flags & WPN_DEMONIC		) strcat(buf," demonic");
    if (weapon_flags & WPN_SERRATED		) strcat(buf," serrated");
    if (weapon_flags & WPN_VAMPIRIC		) strcat(buf," vampiric");
    if (weapon_flags & WPN_POWERDRAIN	) strcat(buf," powerdrain");
    if (weapon_flags & WPN_SHARP		) strcat(buf," sharp");
    if (weapon_flags & WPN_VORPAL		) strcat(buf," vorpal");
    if (weapon_flags & WPN_TWO_HANDS	) strcat(buf," two-handed");
    if (weapon_flags & WPN_POISON		) strcat(buf," poison");
    if (weapon_flags & WPN_PESTILENCE	) strcat(buf," pestilence");
    if (weapon_flags & WPN_READIED		) strcat(buf," readied");
    if (weapon_flags & WPN_THORNED		) strcat(buf," thorned");

    return ( buf[0] != '\0' ) ? buf+1 : (char *)"none";
}

char *cont_bit_name(int cont_flags)
{
    static char buf[512];

    buf[0] = '\0';

    if (cont_flags & CONT_CLOSEABLE	) strcat(buf," closable");
    if (cont_flags & CONT_PICKPROOF	) strcat(buf," pickproof");
    if (cont_flags & CONT_CLOSED	) strcat(buf," closed");
    if (cont_flags & CONT_LOCKED	) strcat(buf," locked");

    return (buf[0] != '\0' ) ? buf+1 : (char *)"none";
}

char *weapon_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_WPN;i++){
		if(bits[i])
			for(n=0;weapon_type2[n].name;n++){
				if (weapon_type2[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,weapon_type2[n].name);
				}
			}
	}
	return (buf[0] != '\0') ? buf : (char *)"none";
}
char *off_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_OFF;i++){
		if(bits[i])
			for(n=0;off_flags[n].name;n++){
				if (off_flags[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,off_flags[n].name);
				}
			}
	}
	return (buf[0] != '\0') ? buf : (char *)"none";
}
char *off_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;off_flags[n].name;n++)
		if (off_flags[n].bit == bit)
			sprintf(buf,"%s",off_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

char *def_bits_name(bool *bits){
    static char buf[MIL];
	int i,n;

    buf[0] = '\0';
	for (i = 0;i<MAX_DEF;i++){
		if(bits[i])
			for(n=0;def_flags[n].name;n++){
				if (def_flags[n].bit == i){
					if (buf[0] != '\0')
						strcat(buf," ");
					strcat(buf,def_flags[n].name);
				}
			}
	}
	return (buf[0] != '\0') ? buf : (char *)"none";
}
char *def_bit_name(int bit){
    static char buf[MIL];
	int n;

    buf[0] = '\0';
	for(n=0;def_flags[n].name;n++)
		if (def_flags[n].bit == bit)
			sprintf(buf,"%s",def_flags[n].name);
    return (buf[0] != '\0') ? buf : (char *)"none";
}

/*
 * See if a string is one of the names of an object.
 */
bool is_full_name(const char *str,char *namelist)
{
    char name[MIL];

    for ( ; ; )
    {
        namelist = one_argument(namelist,name);
        if (name[0] == '\0')
            return false;
        if (!str_cmp(str,name))
            return true;
    }
}

int strlen_color(char *argument)
{
    char        *str;
    int         length;

    if (argument==NULL || argument[0]=='\0')
        return 0;

    length=0;
    str=argument;

    while (*str != '\0')
    {
        if (*str != '{')
        {
            str++;
            length++;
            continue;
        }

        if (*(++str) == '{')
			length++;

        str++;
    }

    return length;
}

int strlen_colorsp(char *argument,int length)
{
    char        *str;
    int         total=0,n=0;

    if (argument==NULL || argument[0]=='\0')
        return 0;

    str = argument;

    while (*str != '\0' && n < length)
    {
        if (*str != '{')
        {
			total++;
			str++;
			n++;
        }
        else
		{
			str+=2;
			total+=3;
			n++;
		}
    }

    return total;
}


CHAR_DATA *get_char_area(CHAR_DATA *ch,char *argument)
{
	char arg[MIL];
	CHAR_DATA *ach;
	int number,count;

	if ((ach = get_char_room(ch,NULL,argument)) != NULL)
		return ach;

	number = number_argument(argument,arg);
	count = 0;
	for ( ach = char_list; ach != NULL; ach = ach->next )
	{
		if (ach->in_room->area != ch->in_room->area || !can_see(ch,ach) || !is_name(arg,ach->name))
			continue;
		if (++count == number)
			return ach;
	}

	return ach;
}

void check_multiplay(DESCRIPTOR_DATA *d){
	DESCRIPTOR_DATA *dn;
	char buf[MSL],hn1[MSL],hn2[MSL];

	save_who();
	sprintf(hn1,"%s",d->host);
	for (dn = descriptor_list; dn; dn = dn->next){
		sprintf(hn2,"%s",dn->host);
		if (d != dn && !str_cmp(hn1,hn2) && d->character && dn->character){
			sprintf(buf,"MULTIPLAYER ALERT: %s is %s!",d->character->name,dn->character->name);
			wiznet(buf,NULL,NULL,WZ_PENALTIES,0,UMAX(d->character->level,dn->character->level));
			break;
		}
	}
}

bool roll_chance(CHAR_DATA *chancer,int chance){
	if (number_percent() < chance)
		return true;
	if(number_percent() < get_curr_stat(chancer,STAT_LCK))
		return true;
	return false;
}

void purge_affect(int level,CHAR_DATA *ch,int where,int bit){
	AFFECT_DATA *paf;
	for (paf = ch->affected;paf;paf = paf->next)
		if(paf->where == where && paf->bitvector == bit){
			if(check_dispel(level,ch,paf->type))
				return;
		}

	if (where == TO_AFFECTS)
		ch->remaff(bit);
}

char *add_comma(int number) 
  { 
  int index,index_new,rest; 
  char buf[16]; 
  static char buf_new[16]; 
   
  sprintf(buf,"%d",number); 
  rest = strlen(buf) % 3; 
   
  for (index=index_new=0;index<strlen(buf);index++,index_new++) 
    { 
     if (index!=0 && (index-rest)%3==0 ) 
     { 
       buf_new[index_new]=','; 
       index_new++; 
       buf_new[index_new]=buf[index]; 
     } 
     else 
     buf_new[index_new] = buf[index]; 
    } 
  buf_new[index_new]='\0'; 
  return buf_new; 
  } 
