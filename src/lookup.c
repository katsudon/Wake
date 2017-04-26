#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "db.h"

bool check_horsemanship	( CHAR_DATA* );
int combat_proficiency ( CHAR_DATA* );
l_int flag_lookup(const char *name, const struct flag_type *flag_table){
    l_int flag;

	for (flag = 0; flag_table[flag].name != NULL; flag++)
		if (LOWER(name[0]) == LOWER(flag_table[flag].name[0]) && !str_prefix(name,flag_table[flag].name) && flag_table[flag].settable)
			return flag_table[flag].bit;

    return NO_FLAG;
}

int position_lookup(const char *name){
   int pos;

	for (pos = 0; position_table[pos].name != NULL; pos++)
		if (LOWER(name[0]) == LOWER(position_table[pos].name[0]) && !str_prefix(name,position_table[pos].name))
			return pos;
   
   return -1;
}

int sex_lookup(const char *name){
	int sex;

	for (sex = 0; sex_table[sex].name != NULL; sex++)
		if (LOWER(name[0]) == LOWER(sex_table[sex].name[0]) && !str_prefix(name,sex_table[sex].name))
			return sex;

	return -1;
}

int size_lookup(const char *name){
	int size;
 
	for ( size = 0; size_table[size].name != NULL; size++)
		if (LOWER(name[0]) == LOWER(size_table[size].name[0]) && !str_prefix( name,size_table[size].name))
			return size;
 
	return -1;
}

/* returns race number */
int race_lookup(const char *name){
	int race;

	for (race = 1; race_table[race].name != NULL; race++)
		if (LOWER(name[0]) == LOWER(race_table[race].name[0]) && !str_prefix(name,race_table[race].name))
			return race;

	return 0;
}

/* returns race number */
int pc_race_lookup(const char *name){
	int race;

	for (race = 1; pc_race_table[race].name; race++)
		if (LOWER(name[0]) == LOWER(pc_race_table[race].name[0]) && !str_prefix(name,pc_race_table[race].name))
			return race;

	return 0;
}

int item_lookup(const char *name){
    int type;

    for (type = 0; item_table[type].name != NULL; type++)
		if (LOWER(name[0]) == LOWER(item_table[type].name[0]) && !str_prefix(name,item_table[type].name))
			return item_table[type].type;
 
    return -1;
}

int liq_lookup(const char *name){
    int liq;

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
		if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0]) && !str_prefix(name,liq_table[liq].liq_name))
			return liq;

    return -1;
}

int forge_lookup(const char *name){
    int forge;

	if (!str_prefix(name,"cold"))
		return HEAT_COLD;
	if (!str_prefix(name,"normal"))
		return HEAT_NORMAL;
	if (!str_prefix(name,"high"))
		return HEAT_HIGH;
	if (!str_prefix(name,"blast"))
		return HEAT_BLAST;

    return -1;
}

int pick_lookup(const char *name){
    int pick;

    for ( pick = 1; pick_table[pick].name != NULL; pick++)
		if (LOWER(name[0]) == LOWER(pick_table[pick].name[0]) && !str_prefix(name,pick_table[pick].name))
			return pick;

    return -1;
}

HELP_DATA * help_lookup(char *keyword){
	HELP_DATA *pHelp;
	char temp[MIL], argall[MIL];

	argall[0] = '\0';

	while (keyword[0] != '\0'){
		keyword = one_argument(keyword, temp);
		if (argall[0] != '\0')
			strcat(argall," ");
		strcat(argall, temp);
	}

	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
		if (is_name(argall,pHelp->keyword))
			return pHelp;

	return NULL;
}

HELP_AREA * had_lookup(char *arg){
	HELP_AREA * temp;
	extern HELP_AREA * had_list;

	for (temp = had_list; temp; temp = temp->next)
		if (!str_cmp(arg,temp->filename))
			return temp;

	return NULL;
}

int get_hometown(char * argument){
	int i = 0;

	while (hometowns[i].name != NULL){
		if (!str_prefix(argument,hometowns[i].name))
			return i;
		i++;
	}

	return -1;
}

int god_lookup(const char *name){
	int god;

	for ( god = 0; god_table[god].name != NULL; god++)
		if (LOWER(name[0]) == LOWER(god_table[god].name[0]) && !str_prefix( name, god_table[god].name))
			return god;

	return -1;
}

bool IS_NATURE_BG(CHAR_DATA *ch){
	switch(ch->in_room->sector_type){
		case SECT_FIELD:
		case SECT_FOREST:
		case SECT_HILLS:
		case SECT_MOUNTAIN:
		case SECT_RAINFOREST:
		case SECT_SWAMP:
		case SECT_FORESTCITY:
			return true;
		default:
			return false;
	}
}

bool IS_WET_BG(CHAR_DATA *ch){
	switch(ch->in_room->sector_type){
		case SECT_WATER_SWIM:
		case SECT_WATER_NOSWIM:
		case SECT_UNDERWATER:
		case SECT_SWAMP:
		case SECT_RAINFOREST:
		case SECT_RIVER:
		case SECT_ICETUNNEL:
			return true;
		default:
			return false;
	}
}

bool is_safe(CHAR_DATA *ch,CHAR_DATA *victim){
    if (victim->in_room == NULL || ch->in_room == NULL)
		return true;

    if (victim->fighting == ch || victim == ch || (victim->isplr(PL_ARENA) && ch->isplr(PL_ARENA)))
		return false;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL)
		return false;

    if (IS_NPC(victim)){
		if (IS_SET(victim->in_room->room_flags,ROOM_SAFE)){
			send_to_char("Not in this room.\n\r",ch);
			return true;
		}

		if (victim->pIndexData->pShop != NULL){
			send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
			return true;
		}

		if(victim->isact(AT_IS_HEALER)
		|| victim->isact(AT_IS_CHANGER)
		|| victim->isact(AT_NOKILL)){
			ch->send("I don't think Boon would approve.\n\r");
			return true;
		}

		if (!IS_NPC(ch)){
			if (victim->isact(AT_PET)){
				act("But $N looks so cute and cuddly...", ch,NULL,victim,TO_CHAR);
				return true;
			}
			if ( victim->isaff(AF_CHARM) && ch != victim->master){
				send_to_char("You don't own that monster.\n\r",ch);
				return true;
			}
		}
    }
    else{
		if (IS_NPC(ch)){
			if (IS_SET(victim->in_room->room_flags,ROOM_SAFE)){
				send_to_char("Not in this room.\n\r",ch);
				return true;
			}

			/* charmed mobs and pets cannot attack players while owned */
			if (ch->isaff(AF_CHARM) && ch->master != NULL && ch->master->fighting != victim){
				send_to_char("Players are your friends!\n\r",ch);
				return true;
			}
		}
		else{
			if (ch->level > victim->level + 8){
				send_to_char("Pick on someone your own size.\n\r",ch);
				return true;
			}
		}
    }
    return false;
}
 
bool is_safe_spell(CHAR_DATA *ch,CHAR_DATA *victim,bool area){
    if (victim->in_room == NULL || ch->in_room == NULL)
        return true;

    if (victim == ch && area)
		return true;

    if (victim->fighting == ch || victim == ch)
		return false;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
		return false;

    if (IS_NPC(victim)){
		if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
			return true;

		if (victim->pIndexData->pShop != NULL)
			return true;

		if (victim->isact(AT_IS_HEALER)
		||  victim->isact(AT_IS_CHANGER))
			return true;

		if (!IS_NPC(ch)){
			if (victim->isact(AT_PET))
	   			return true;

			if ( victim->isaff(AF_CHARM) && (area || ch != victim->master))
				return true;

			if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
				return true;
		}
		else
			if (area && !is_same_group(victim,ch->fighting))
				return true;
    }
    else{
		if (area && IS_IMMORTAL(victim) && ch->level < LEVEL_IMMORTAL)
			return true;

		if (IS_NPC(ch)){
			// charmed mobs and pets cannot attack players while owned
			if (ch->isaff(AF_CHARM) && ch->master != NULL && ch->master->fighting != victim)
				return true;
		
			if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
				return true;

			if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
				return true;
		}
		else{
			if (victim->isplr(PL_KILLER) || victim->isplr(PL_THIEF))
				return false;
			if (ch->level > victim->level + 8)
				return true;
		}
    }
    return false;
}

int calcHands(CHAR_DATA *ch){
	int hcount = 0;
	OBJ_DATA *wield,*dual,*shield,*hold1,*hold2,*hold3;

	wield = get_eq_char(ch,WEAR_WIELD);
	dual = get_eq_char(ch,WEAR_SECONDARY);
	shield = get_eq_char(ch,WEAR_SHIELD);
	hold1 = get_eq_char(ch,WEAR_HOLD_R);
	hold2 = get_eq_char(ch,WEAR_HOLD_L);
	hold3 = get_eq_char(ch,WEAR_LIGHT);

	if(wield != NULL){
		if (IS_SET(wield->extra_flags,ITM_GRASPED))
			hcount += 2;
		else if (IS_WEAPON_STAT(wield,WPN_TWO_HANDS) && ch->size < SIZE_LARGE)
			hcount += 2;
		else if (IS_WEAPON_STAT(wield,WPN_TWO_HANDS) && ch->size >= SIZE_LARGE)
			hcount += 1;
		else
			hcount += 1;
	}

	if(dual != NULL){
		if (IS_SET(dual->extra_flags,ITM_GRASPED))
			hcount += 2;
		else if (IS_WEAPON_STAT(dual,WPN_TWO_HANDS) && ch->size < SIZE_LARGE)
			hcount += 2;
		else if (IS_WEAPON_STAT(dual,WPN_TWO_HANDS) && ch->size >= SIZE_LARGE)
			hcount += 1;
		else
			hcount += 1;
	}

	if(shield != NULL)	hcount += 1;

	if(hold1 != NULL)	hcount += 1;
	if(hold2 != NULL)	hcount += 1;
	if(hold3 != NULL)	hcount += 1;

	if(hcount > 2 || hcount < 0){
		bugf("%s has bad hands! '%d'",ch->name,hcount);
		wiznet("$N has bad bad hands!",ch,NULL,WZ_PENALTIES,0,0);
		return 3;
	}

	return hcount;
}


bool check_dual_bare(CHAR_DATA *ch,OBJ_DATA *dual){
	if (calcHands(ch) > 1 || !weapon_table[dual->value[0]].candual)
		return false;

	return true;
}

bool check_dual_blade(CHAR_DATA *ch,OBJ_DATA *pwield,OBJ_DATA *dwield){
	int dsize, wsize;

	if(pwield == NULL)
		return true;

	dsize = weapon_table[dwield->value[0]].size;

	wsize = weapon_table[pwield->value[0]].size;

	if (dsize <= wsize && dsize != 9 && wsize !=9 && weapon_table[pwield->value[0]].candual && weapon_table[dwield->value[0]].candual)
		return true;
	else
		return false;
}

bool is_excluded(CHAR_DATA *ch,OBJ_DATA *obj){
	return false;
/*	for (int n = 0; n < MAX_EXCLUDE; n++)
	{
		if (IS_SET(obj->exclude,n))
			if (excl_table[n].isclass)
			{
				if (ch->pclas == excl_table[n].wclass)
					return true;
			}
			else
			{
				if (excl_table[n].race != NULL && ch->israce(excl_table[n].race))
					return true;
			}
	}*/
	return false;
}

CHAR_DATA *grab_char(CHAR_DATA *ch,char *arg,bool infight){
	CHAR_DATA *victim;

	if (arg[0] == '\0'){
		if (infight){
			if (!(victim = ch->fighting))
				ch->send("But you aren't fighting anyone!\n\r");
		}
		else
			ch->send("Who?\n\r");
	}
	else if (!(victim = get_char_room(ch,NULL,arg)))
		ch->send("They aren't here.\n\r");
	return victim;
}

bool is_class(CHAR_DATA*ch,int iclass){
	int cclass = ch->pclass;

	if(cclass == iclass)
		return true;
	return false;
}

int get_class(CHAR_DATA*ch){
	return ch->pclass;
}

bool check_armor(CHAR_DATA *ch,OBJ_DATA *obj){
	int sn=-1,skill=-1;
	if (obj->item_type != ITEM_ARMOR && obj->item_type != ITEM_SHIELD)
		return true;

	switch (obj->armortype_flags){
	case ARMOR_CLOTHING:
		return true;
	case ARMOR_LIGHT:
		sn = gsn_light_armor;
		break;
	case ARMOR_MEDIUM:
		sn = gsn_medium_armor;
		break;
	case ARMOR_HEAVY:
		sn = gsn_heavy_armor;
		break;
	default:
		return true;
	}
	skill = grab_skill(ch,sn);
	if (skill < 1){
		ch->send("You do not have the skill to use this armor.\n\r");
		return false;
	}
	return true;
	if (number_percent() >= skill){
		return false;
		check_improve(ch,sn,false,1);
	}
	return true;
}
int skill_lookup(const char *name){
    int sn;

	if (name[0] == '\0')
		return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ ){
		if (skill_table[sn].name == NULL)
		    break;
		if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) && !str_cmp(name,skill_table[sn].name))
			return sn;
    }

    return -1;
}
int skill_prefix(const char *name,bool override){
    int sn;

	if (!name[0])
		return -1;

	for (sn = 0;sn < MAX_SKILL;sn++){
		if (!skill_table[sn].name)
			break;
		if (skill_table[sn].slot != SKILL_NORMAL && !override)
			continue;
		if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) && !str_prefix(name,skill_table[sn].name))
			return sn;
	}

	return -1;
}
int skillspell_prefix(const char *name){
    int sn;

	if (!name[0])
		return -1;

	for (sn = 0;sn < MAX_SKILL;sn++){
		if (!skill_table[sn].name)
			break;
		if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) && !str_prefix(name,skill_table[sn].name))
			return sn;
	}

	return -1;
}

int find_spell(CHAR_DATA *ch,const char *name){
    int sn, found = -1;

    if (IS_NPC(ch))
		return skill_lookup(name);

    for ( sn = 0; sn < MAX_SKILL; sn++ ){
		if (skill_table[sn].name == NULL)
			break;
		if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) && !str_prefix(name,skill_table[sn].name)){
			if ( found == -1)
				found = sn;
			if (ch->level >= skill_table[sn].skill_level[ch->pclass] && ch->pcdata->learned[sn] > 0)
				return sn;
		}
    }
    return found;
}

int find_spell_list(const char *name){
	int sn,found = -1;

	for ( sn = 0; sn < MAX_SKILL; sn++ ){
		if (skill_table[sn].name == NULL)
			break;
		if (LOWER(name[0]) == LOWER(skill_table[sn].name[0]) && !str_prefix(name,skill_table[sn].name))
			return sn;
	}
	return found;
}

int GET_AC(CHAR_DATA *ch,int type){
	int n;
	n = ch->armor[type];

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ACBSE: %d ",n);
	if(dex_app[get_curr_stat(ch,STAT_AGI)].defensive < 0)
		n += ch->position > POS_RESTING ? dex_app[get_curr_stat(ch,STAT_AGI)].defensive : 0;
	else
		n += dex_app[get_curr_stat(ch,STAT_AGI)].defensive;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ACDEX: %d ",n);

	n -= check_piety(ch,0,PIETY_AC);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ACPTY: %d ",n);

	n = uplifting_ac(ch,n);
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ACDWF: %d ",n);

	if (check_icebound(ch) == 1)
		n += n/2;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ACALG: %d ",n);

	if (check_icebound(ch) == 2)
		n -= n * .75;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ACALG: %d ",n);

	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"ACFIN: %d\n\r",n/10);
	return n / 10;
}

int GET_HITROLL(CHAR_DATA *ch){
	int n = ch->hitroll + str_app[get_curr_stat(ch,STAT_STR)].tohit;

	n = uplifting_hrdr(ch,n);

	n += combat_proficiency(ch);

	return n;
}

int GET_DAMROLL(CHAR_DATA *ch){
	int n = ch->damroll + str_app[get_curr_stat(ch,STAT_STR)].todam;

	n = uplifting_hrdr(ch,n);

	n += combat_proficiency(ch);

	return UMAX(n,1);
}

AREA_DATA *area_lookup(int anum)
{	AREA_DATA *pArea;
	for( pArea = area_first ; pArea ; pArea = pArea->next)
	{	if(pArea->vnum == anum)
			return pArea;
	}
	return NULL;
}

int get_hometown_by_id(int id){
	for(int n = 0;n < MAX_HOMETOWN;n++)
		if(hometowns[n].id == id)
			return n;
	return -1;
}

CHAR_DATA * get_char_by_id(int id){
	DESCRIPTOR_DATA *d;

	for( d = descriptor_list ; d ; d = d->next ){
		CHAR_DATA *ch = d->original ? d->original : d->character;
		if(ch->id == id)
			return ch;
	}
	return NULL;
}
