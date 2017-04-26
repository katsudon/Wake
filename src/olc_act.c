#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include "merc.h"
#include "olc.h"
#include "recycle.h"
#include "lookup.h"
#include "db.h"
char * prog_type_to_name ( int type );

#define ALT_FLAGVALUE_TOGGLE( _blargh, _table, _arg )	{		int blah = flag_value( _table, _arg );		_blargh ^= (blah == NO_FLAG) ? 0 : blah;	}
void unlink_reset(ROOM_INDEX_DATA*,RESET_DATA*);
void unlink_obj_index(OBJ_INDEX_DATA*);
void unlink_mob_index(MOB_INDEX_DATA*);
void unlink_room_index(ROOM_INDEX_DATA*);
int calc_light(ROOM_INDEX_DATA *room);

/* Return true if area changed, false if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT	REDIT
#define MEDIT	REDIT
#define AEDIT	REDIT
#define KEDIT	REDIT
#define FEDIT	REDIT

struct olc_help_type{
	char *command;
	const void *structure;
	char *desc;
};

bool show_version( CHAR_DATA *ch, char *argument ){
    send_to_char( VERSION, ch );
    send_to_char( "\n\r", ch );
    send_to_char( AUTHOR, ch );
    send_to_char( "\n\r", ch );
    send_to_char( DATE, ch );
    send_to_char( "\n\r", ch );
    send_to_char( CREDITS, ch );
    send_to_char( "\n\r", ch );

    return false;
}

int calc_points(OBJ_INDEX_DATA *obj){
	AFFECT_DATA *paf;
	double points = 0;

	switch (obj->item_type){
	case ITEM_ARMOR:
	case ITEM_SHIELD:
		points += obj->value[0];
		points += obj->value[1];
		points += obj->value[2];
		points += obj->value[3];
		points /= 10;
		break;
	case ITEM_WEAPON:
		break;
	}
	for(paf = obj->affected;paf;paf = paf->next){
		switch(paf->where){
			case TO_AFFECTS:
				points += 10;
				break;
			case TO_WEAPON:
			case TO_OBJECT:
				switch(paf->location){
					case APPLY_HITROLL:
					case APPLY_DAMROLL:
						points += paf->modifier * 5;
						break;
					case APPLY_AC:
						points -= paf->modifier * 4;
						break;
					case APPLY_SAVES:
						points += paf->modifier;
						break;
					case APPLY_SAVING_SPELL:
						points += paf->modifier;
						break;
					case APPLY_MOVE:
					case APPLY_HIT:
					case APPLY_MANA:
						points += paf->modifier;
						break;
					case APPLY_STR:
					case APPLY_END:
					case APPLY_AGI:
					case APPLY_INT:
					case APPLY_RES:
					case APPLY_FTH:
					case APPLY_WIS:
					case APPLY_CHA:
					case APPLY_LCK:
						points += paf->modifier * 3;
						break;
				}
				break;
			case TO_IMMUNE:
				points += 50;
				break;
			case TO_RESIST:
			case TO_VULN:
				points += 10;
				break;
		}
	}
	return (int)(points + 0.5);
}

int gen_basepoints(OBJ_INDEX_DATA *obj){
	int count=0,x=0,wear_flags = obj->wear_flags;
	double n = obj->level,type = obj->armortype_flags + 1,size = 0;

	if (obj->item_type == ITEM_ARMOR){
		if (wear_flags & ITEM_WEAR_FINGER		){ size +=1; count++; }
		if (wear_flags & ITEM_WEAR_EAR			){ size +=1; count++; }
		if (wear_flags & ITEM_WEAR_NECK			){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_ELBOW		){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_KNEE			){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_SHIN			){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_TORSO			){ size +=4; count++; }
		if (wear_flags & ITEM_WEAR_HEAD			){ size +=4; count++; }
		if (wear_flags & ITEM_WEAR_LEGS			){ size +=4; count++; }
		if (wear_flags & ITEM_WEAR_SHOULDER		){ size +=3; count++; }
		if (wear_flags & ITEM_WEAR_QUIVER		){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_FEET			){ size +=3; count++; }
		if (wear_flags & ITEM_WEAR_HANDS		){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_FACE			){ size +=3; count++; }
		if (wear_flags & ITEM_WEAR_PENDANT		){ size +=1; count++; }
		if (wear_flags & ITEM_WEAR_TATTOO		){ size +=1; count++; }
		if (wear_flags & ITEM_WEAR_ARMS			){ size +=3; count++; }
		if (wear_flags & ITEM_WEAR_SHIELD		){ size +=5; count++; }
		if (wear_flags & ITEM_WEAR_ABOUT		){ size +=4; count++; }
		if (wear_flags & ITEM_WEAR_WAIST		){ size +=3; count++; }
		if (wear_flags & ITEM_WEAR_WRIST		){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_ANKLE		){ size +=2; count++; }
		if (wear_flags & ITEM_WEAR_FLOAT		){ size +=1; count++; }
		if (wear_flags & ITEM_WEAR_FLOAT_LIGHT	){ size +=1; count++; }

		if (count > 0)
			size /= count;

		size *= 0.1;
		type *= 0.125;

		n *= (size + type);
		x = (int)n;
		return x;
	}
	if(obj->item_type == ITEM_JEWELRY){
			 if (wear_flags & ITEM_WEAR_FINGER		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_EAR			){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_NECK		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_FACE		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_PENDANT		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_TATTOO		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_WRIST		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_ANKLE		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_FLOAT		){ n = UMAX(obj->level/2,2); }
		else if (wear_flags & ITEM_WEAR_FLOAT_LIGHT	){ n = UMAX(obj->level/2,2); }
		else                                         { n = UMAX(obj->level/4,2); }
	}
	if(obj->droprate != 1000){
		double b = 1000 - obj->droprate;
		n *= (1 + (b * 0.00075));
	}
	if(obj->item_type == ITEM_WEAPON)
		return n;
	return n;
}


/* This table contains help commands and a brief description of each.*/
const struct olc_help_type help_table[] ={
    {	"area",			area_flags,			"Area attributes."			},
    {	"room",			room_flags,			"Room attributes."			},
    {	"sector",		sector_flags,		"Sector types, terrain."		},
    {	"exit",			exit_flags,			"Exit types."					},
    {	"type",			type_flags,			"Types of objects."				},
    {	"extra",		extra_flags,		"Object attributes."			},
	{	"armortype",	armortype_flags,	"ArmorType attributes."			},
    {	"wear",			wear_flags,			"Where to wear object."			},
    {	"spec",			spec_table,			"Available special programs."	},
    {	"sex",			sex_flags,			"Sexes."						},
    {	"act",			act_flags,			"Mobile attributes."			},
    {	"affect",		affect_flags,		"Mobile affects."				},
    {	"wear-loc",		wear_loc_flags,		"Where mobile wears object."	},
    {	"spells",		skill_table,		"Names of current spells." 		},
    {	"container",	container_flags,	"Container status."				},
    {	"armor",		ac_type,			"Ac for different attacks."		},
    {   "apply",		apply_flags,		"Apply flags"					},
    {	"form",			form_flags,			"Mobile body form."				},
    {	"part",			part_flags,			"Mobile body parts."			},
    {	"imm",			res_flags,			"Mobile immunity."				},
    {	"res",			res_flags,			"Mobile resistance."			},
    {	"vuln",			res_flags,			"Mobile vulnerability."			},
    {	"off",			off_flags,			"Mobile offensive behaviour."	},
    {	"def",			def_flags,			"Mobile defensive behaviour."	},
    {	"size",			size_flags,			"Mobile size."					},
    {   "position",		position_flags,		"Mobile positions."				},
    {   "wclass",		weapon_class,		"Weapon class."					}, 
    {   "arrowflags",	arrow_flags,		"Arrow flags."					}, 
    {   "wtype",		weapon_type2,		"Special weapon type."			},
    {   "trapclass",	trap_class,			"Trap type."					},
    {   "exclude",		exclude_flags,		"Exclude flags."				},
    {   "traptype",		trap_type,			"Special trap type."			},
    {	"portal",		portal_flags,		"Portal types."					},
    {	"furniture",	furniture_flags,	"Furniture types."				},
    {   "liquid",		liq_table,			"Liquid types."					},
    {	"apptype",		apply_types,		"Apply types."					},
    {	"weapon",		attack_table,		"Weapon types."					},
    {	"climate",		climate_flags,		"Climate types."				},
    {	"mprog",		mprog_flags,		"MobProgram flags."				},
    {	"oprog",		oprog_flags,		"ObjProgram flags."				},
    {	"rprog",		rprog_flags,		"RoomProgram flags."			},
    {   "trapclass",	trap_class,			"Weapon class."					}, 
    {	NULL,			NULL,				NULL							}
};

void show_flag_cmds( CHAR_DATA *ch, const struct flag_type *flag_table )
{
    char buf[MSL], buf1[MSL];
    l_int flag, col;
 
    buf1[0] = '\0';
    col = 0;
    for (flag = 0; flag_table[flag].name != NULL; flag++)
		if (flag_table[flag].settable)
		{
			sprintf(buf,"%-19.18s",flag_table[flag].name);
			strcat( buf1, buf );
			if (++col % 4 == 0)
				strcat(buf1,"\n\r");
		}
 
    if (col % 4 != 0)
		strcat(buf1,"\n\r");

    send_to_char(buf1,ch);
    return;
}

void show_skill_cmds(CHAR_DATA *ch,int tar)
{
    char buf[MSL], buf1[MSL*2];
    int sn, col;
 
    buf1[0] = '\0';
    col = 0;
    for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (!skill_table[sn].name)
			break;

		if (!str_cmp(skill_table[sn].name,"reserved") || skill_table[sn].spell_fun == spell_null)
			continue;

		if (tar == -1 || skill_table[sn].target == tar)
		{
			sprintf(buf,"%-19.18s",skill_table[sn].name);
			strcat(buf1,buf);
			if (++col % 4 == 0)
				strcat(buf1,"\n\r");
		}
	}
 
    if (col % 4 != 0)
		strcat(buf1,"\n\r");

    send_to_char(buf1,ch);
    return;
}

void show_spec_cmds(CHAR_DATA *ch)
{
    char buf[MSL], buf1[MSL];
    int spec, col;
 
    buf1[0] = '\0';
    col = 0;
    send_to_char("Preceed special functions with 'spec_'\n\r\n\r",ch);
    for (spec = 0; spec_table[spec].function != NULL; spec++)
    {
		sprintf(buf,"%-19.18s", &spec_table[spec].name[5]);
		strcat(buf1,buf);
		if (++col % 4 == 0)
			strcat(buf1,"\n\r");
    }
 
    if (col % 4 != 0)
		strcat(buf1,"\n\r");

    send_to_char(buf1,ch);
    return;
}

bool show_help(CHAR_DATA *ch,char *argument){
	char arg[MIL], spell[MIL];
	int cnt;

	argument = one_argument(argument,arg);
	one_argument(argument,spell);

	if (arg[0] == '\0'){
		send_to_char("Syntax:  ? [command]\n\r\n\r",ch);
		send_to_char("[command]  [description]\n\r",ch);
		for (cnt = 0; help_table[cnt].command != NULL; cnt++)
			printf_to_char(ch, "%-10.10s -%s\n\r",capitalize( help_table[cnt].command ),help_table[cnt].desc );
		return false;
	}

	if(!str_cmp(arg,"race")){
		send_to_char("Available races are:",ch);

		for (int race = 0;race_table[race].name;race++){
			if ((race % 3) == 0)
				send_to_char("\n\r",ch);
			printf_to_char(ch," %-15s",race_table[race].name);
		}

		send_to_char("\n\r",ch);
		return false;
	}
	else if(!str_prefix(arg,"materials")){
		int cols = 0;
		ch->send("Available materials are:\n\r");
		for(int i = 1;material_flags[i].name;i++){
			printf_to_char(ch,"%18s",material_flags[i].name);
			if (++cols > 3){
				ch->send("\n\r");
				cols = 0;
			}
		}
		if (cols != 0)
			ch->send("\n\r\n\r");
		else
			ch->send("\n\r");
		return false;
	}
	for (cnt = 0; help_table[cnt].command != NULL; cnt++)
	    if (arg[0] == help_table[cnt].command[0] && !str_prefix(arg,help_table[cnt].command)){
			if (help_table[cnt].structure == spec_table){
				show_spec_cmds(ch);
				return false;
			}
			else if (help_table[cnt].structure == liq_table){
				show_liqlist(ch);
				return false;
			}
			else if (help_table[cnt].structure == attack_table){
				show_damlist(ch);
				return false;
			}
			else if (help_table[cnt].structure == skill_table){
				if (spell[0] == '\0'){
					send_to_char("Syntax:  ? spells " "[ignore/attack/defend/self/object/all]\n\r",ch);
					return false;
				}

				if (!str_prefix(spell,"all"))
					show_skill_cmds(ch,-1);
				else if (!str_prefix(spell,"ignore"))
					show_skill_cmds(ch,TAR_IGNORE);
				else if (!str_prefix(spell,"attack"))
					show_skill_cmds(ch,TAR_CHAR_OFFENSIVE );
				else if (!str_prefix(spell,"defend"))
					show_skill_cmds(ch,TAR_CHAR_DEFENSIVE );
				else if (!str_prefix(spell,"self"))
					show_skill_cmds(ch,TAR_CHAR_SELF);
				else if (!str_prefix(spell,"object"))
					show_skill_cmds(ch,TAR_OBJ_INV);
				else
					send_to_char("Syntax:  ? spell " "[ignore/attack/defend/self/object/all]\n\r",ch);
				return false;
			}
			else{
				show_flag_cmds(ch,(const flag_type *)help_table[cnt].structure);
				return false;
			}
		}

	show_help(ch,"");
	return false;
}

REDIT(redit_rlist){
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA *pArea;
    BUFFER *buf1;
    char buf[MSL], arg[MIL];
    bool found;
    int vnum, col = 0;

    one_argument(argument,arg);

    pArea = ch->in_room->area;
    buf1=new_buf();
/*    buf1[0] = '\0'; */
    found   = false;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		if ((pRoomIndex = get_room_index(vnum)))
		{
			found = true;
			sprintf(buf,"[%5d] %-17.16s",vnum,capitalize(pRoomIndex->name));
			add_buf(buf1,buf);
			if (++col % 3 == 0)
				add_buf(buf1,"\n\r");
		}

    if (!found)
    {
		send_to_char("Room(s) not found in this area.\n\r",ch);
		return false;
    }

    if (col % 3 != 0)
		add_buf(buf1,"\n\r");

    page_to_char(buf_string(buf1),ch);
    free_buf(buf1);
    return false;
}

REDIT(redit_mlist){
	MOB_INDEX_DATA *pMobIndex;
	AREA_DATA *pArea;
	BUFFER *buf1;
	char buf[MSL], arg[MIL];
	bool fAll, found;
	int vnum, col = 0;

	one_argument(argument,arg);
	if (!arg[0]){
		send_to_char("Syntax:  mlist <all/name>\n\r",ch);
		return false;
	}

	buf1=new_buf();
	pArea = ch->in_room->area;
	/*    buf1[0] = '\0'; */
	fAll    = !str_cmp(arg,"all");
	found   = false;

	for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		if ((pMobIndex = get_mob_index(vnum))){
			if (fAll || is_name(arg,pMobIndex->player_name)){
				found = true;
				sprintf(buf,"[%5d] %-17.16s",pMobIndex->vnum,capitalize(pMobIndex->short_descr));
			add_buf(buf1,buf);
			if (++col % 3 == 0)
				add_buf(buf1,"\n\r");
			}
		}

	if (!found){
		send_to_char("Mobile(s) not found in this area.\n\r",ch);
		return false;
	}

	if (col % 3 != 0)
		add_buf(buf1,"\n\r");

	page_to_char(buf_string(buf1),ch);
	free_buf(buf1);
	return false;
}

REDIT(redit_olist){
	OBJ_INDEX_DATA *pObjIndex;
	AREA_DATA *pArea;
	BUFFER *buf1;
	char buf[MSL], arg[MIL];
	bool fAll, found;
	int vnum, col = 0;

	one_argument(argument,arg);
	if (!arg[0]){
		send_to_char("Syntax:  olist <all/name/item_type>\n\r",ch);
		return false;
	}

	buf1 = new_buf();
	pArea = ch->in_room->area;
	/*    buf1[0] = '\0'; */
	fAll    = !str_cmp(arg,"all");
	found   = false;

	for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		if ((pObjIndex = get_obj_index(vnum))){
			if (fAll || is_name(arg,pObjIndex->name)/**/){
				found = true;
				sprintf(buf,"[%5d] %-17.16s",pObjIndex->vnum,capitalize(pObjIndex->short_descr));
			add_buf(buf1,buf);
			if (++col % 3 == 0)
				add_buf(buf1,"\n\r");
			}
		}

	if (!found){
		send_to_char("Object(s) not found in this area.\n\r",ch);
		return false;
	}

	if (col % 3 != 0)
		add_buf(buf1,"\n\r");

	page_to_char(buf_string(buf1),ch);
	free_buf(buf1);
	return false;
}

REDIT(redit_mshow){
	MOB_INDEX_DATA *pMob;
	int value;

	if (!argument[0]){
		send_to_char("Syntax:  mshow <vnum>\n\r",ch);
		return false;
	}

	if (!is_number(argument)){
	   send_to_char("REdit: Argument must be numeric.\n\r",ch);
	   return false;
	}
	else{
		value = atoi(argument);
		if (!(pMob = get_mob_index(value))){
			send_to_char("REdit:  That mobile does not exist.\n\r",ch);
			return false;
		}
		ch->desc->pEdit = (void *)pMob;
	}

	medit_show(ch,argument);
	ch->desc->pEdit = (void *)ch->in_room;
	return false; 
}

REDIT(redit_oshow)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  oshow <vnum>\n\r",ch);
		return false;
    }

    if (!is_number(argument))
    {
       send_to_char( "REdit: Argument must be numeric.\n\r", ch);
       return false;
    }
	else
    {
		value = atoi(argument);
		if (!(pObj = get_obj_index(value)))
		{
			send_to_char("REdit:  That object does not exist.\n\r",ch);
			return false;
		}

		ch->desc->pEdit = (void *)pObj;
    }
 
    oedit_show(ch,argument);
    ch->desc->pEdit = (void *)ch->in_room;
    return false; 
}

bool check_range(int lower,int upper)
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper) || (lower <= pArea->max_vnum && pArea->max_vnum <= upper))
		    ++cnt;

		if (cnt > 1)
			return false;
    }
    return true;
}

AREA_DATA *get_vnum_area(int vnum)
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
        if (vnum >= pArea->min_vnum && vnum <= pArea->max_vnum)
            return pArea;

    return 0;
}

AEDIT(aedit_show){
    AREA_DATA *pArea;

    EDIT_AREA(ch,pArea);

    printf_to_char(ch,"Name:     [{y%5d{x] %s%s{x\n\r",pArea->vnum,pArea->finished == 1 ? "{G" : pArea->finished == 2 ? "{R" : "{d",pArea->name);
    printf_to_char(ch,"File:     [{c%s{x]\n\r",pArea->file_name);
    printf_to_char(ch,"Vnums:    [{y%d{x-{y%d{x]\n\r",pArea->min_vnum,pArea->max_vnum);
    printf_to_char(ch,"Age:      [{y%d{x]\n\r",pArea->age);
    printf_to_char(ch,"Players:  [{y%d{x]\n\r",pArea->nplayer);
    printf_to_char(ch,"Security: [{y%d{x]\n\r",pArea->security);
    printf_to_char(ch,"Builders: [{g%s{x]\n\r",pArea->builders);
    printf_to_char(ch,"Credits : [{c%s{x]\n\r",pArea->credits);
    printf_to_char(ch,"LRange:   [{y%d{x-{y%d{x]\n\r",pArea->low_range,pArea->high_range);
    printf_to_char(ch,"Climate : [{g%s{x]\n\r",climate_flags[pArea->climate].name);
    printf_to_char(ch,"Flags:    [{g%s{x]\n\r",flag_string(area_flags,pArea->area_flags));
    printf_to_char(ch,"Locked:   [{y%s{x]\n\r",pArea->locked ? "true" : "false");
    printf_to_char(ch,"Group:    [{y%s{x]\n\r",area_groups[pArea->area_group].name);

    return false;
}

AEDIT(aedit_reset)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch,pArea);
    reset_area(pArea);
    send_to_char("Area reset.\n\r",ch);
    return false;
}

AEDIT(aedit_create)
{
    AREA_DATA *pArea;

    pArea               =   new_area();
    area_last->next     =   pArea;
    area_last			=   pArea;
    ch->desc->pEdit     =   (void *)pArea;
    SET_BIT(pArea->area_flags,AREA_ADDED);
    send_to_char("Area Created.\n\r",ch);
    return false;
}

AEDIT(aedit_name)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch,pArea);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:   name [$name]\n\r",ch);
		return false;
    }

    free_string(pArea->name);
    pArea->name = str_dup(argument);
    send_to_char("Name set.\n\r",ch);
    return true;
}

AEDIT(aedit_credits)
{
    AREA_DATA *pArea;

    EDIT_AREA(ch,pArea);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:   credits [$credits]\n\r",ch);
		return false;
    }

    free_string(pArea->credits);
    pArea->credits = str_dup(argument);
    send_to_char("Credits set.\n\r",ch);
    return true;
}

AEDIT(aedit_file)
{
    AREA_DATA *pArea;
    char file[MSL];
    int i, length;

    EDIT_AREA(ch,pArea);

    one_argument(argument,file);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  filename [$file]\n\r",ch);
		return false;
    }

    length = strlen_color(argument);
    if (length > 8)
    {
		send_to_char("No more than eight characters allowed.\n\r",ch);
		return false;
    }

    for ( i = 0; i < length; i++ )
		if (!isalnum(file[i]))
		{
			send_to_char("Only letters and numbers are valid.\n\r",ch);
			return false;
		}

    free_string(pArea->file_name);
    strcat(file,".are");
    pArea->file_name = str_dup(file);
    send_to_char("Filename set.\n\r",ch);
    return true;
}

AEDIT(aedit_age)
{
    AREA_DATA *pArea;
    char age[MSL];

    EDIT_AREA(ch,pArea);

    one_argument(argument,age);

    if (!is_number(age) || age[0] == '\0')
    {
		send_to_char("Syntax:  age [#xage]\n\r",ch);
		return false;
    }

    pArea->age = atoi(age);
    send_to_char("Age set.\n\r",ch);
    return true;
}

AEDIT(aedit_climate)
{
	AREA_DATA *pArea;

	EDIT_AREA(ch,pArea);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  climate <climate>\n\rTo see a list of messages, type '? weapon'.\n\r",ch);
		return false;
	}

	pArea->climate = climate_lookup(argument);
	send_to_char("Climate set.\n\r",ch);
	return true;
}

AEDIT(aedit_security)
{
    AREA_DATA *pArea;
    char sec[MSL];
    int value;

    EDIT_AREA(ch,pArea);

    one_argument(argument,sec);

    if (!is_number(sec) || sec[0] == '\0')
    {
		send_to_char("Syntax:  security [#xlevel]\n\r",ch);
		return false;
    }

    value = atoi(sec);

    if (value > ch->pcdata->security || value < 0)
    {
		if (ch->pcdata->security != 0)
			printf_to_char(ch,"Security is 0-%d.\n\r",ch->pcdata->security);
		else
			send_to_char("Security is 0 only.\n\r",ch);
		return false;
    }

    pArea->security = value;
    send_to_char("Security set.\n\r",ch);
    return true;
}

AEDIT(aedit_builder)
{
    AREA_DATA *pArea;
    char name[MSL];
    char buf[MSL];

    EDIT_AREA(ch,pArea);

    one_argument(argument,name);

    if (name[0] == '\0')
    {
		send_to_char("Syntax:  builder [$name]  -toggles builder\n\r",ch);
		send_to_char("Syntax:  builder All      -allows everyone\n\r",ch);
		return false;
    }

    name[0] = UPPER(name[0]);

    if (strstr(pArea->builders,name) != '\0')
    {
		pArea->builders = string_replace(pArea->builders,name,"\0");
		pArea->builders = string_unpad(pArea->builders);

		if (pArea->builders[0] == '\0')
		{
			free_string(pArea->builders);
			pArea->builders = str_dup("None");
		}
		send_to_char("Builder removed.\n\r",ch);
		return true;
    }
    else
    {
		buf[0] = '\0';
		if (strstr(pArea->builders,"None") != '\0')
		{
			pArea->builders = string_replace(pArea->builders,"None","\0");
			pArea->builders = string_unpad(pArea->builders);
		}

		if (pArea->builders[0] != '\0')
		{
			strcat(buf,pArea->builders);
			strcat(buf," ");
		}
		strcat(buf,name);
		free_string(pArea->builders);
		pArea->builders = string_proper(str_dup(buf));
		send_to_char("Builder added.\n\r",ch);
		send_to_char(pArea->builders,ch);
		return true;
    }

    return false;
}

AEDIT(aedit_vnum)
{
    AREA_DATA *pArea;
    char lower[MSL], upper[MSL];
    int ilower, iupper;

    EDIT_AREA(ch,pArea);

    argument = one_argument(argument,lower);
    one_argument(argument,upper);

    if (!is_number(lower) || lower[0] == '\0'|| !is_number(upper) || upper[0] == '\0')
    {
		send_to_char("Syntax:  vnum [#xlower] [#xupper]\n\r",ch);
		return false;
    }

    if ((ilower = atoi(lower)) > (iupper = atoi(upper)))
    {
		send_to_char("AEdit:  Upper must be larger then lower.\n\r",ch);
		return false;
    }
    
    if (!check_range(atoi(lower), atoi(upper)))
    {
		send_to_char("AEdit:  Range must include only this area.\n\r",ch);
		return false;
    }

    if (get_vnum_area(ilower) && get_vnum_area(ilower) != pArea)
    {
		send_to_char("AEdit:  Lower vnum already assigned.\n\r",ch);
		return false;
    }

    pArea->min_vnum = ilower;
    send_to_char("Lower vnum set.\n\r",ch);

    if (get_vnum_area(iupper) && get_vnum_area(iupper) != pArea)
    {
		send_to_char("AEdit:  Upper vnum already assigned.\n\r",ch);
		return true;
    }

    pArea->max_vnum = iupper;
    send_to_char("Upper vnum set.\n\r",ch);
    return true;
}

AEDIT(aedit_lrange)
{
    AREA_DATA *pArea;
    char lower[MSL],upper[MSL];
    int ilower, iupper;

    EDIT_AREA(ch,pArea);

    argument = one_argument(argument,lower);
    one_argument(argument,upper);

    if (!is_number(lower) || lower[0] == '\0'|| !is_number(upper) || upper[0] == '\0')
    {
		send_to_char("Syntax:  lrange [#xlower] [#xupper]\n\r",ch);
		return false;
    }

    if ((ilower = atoi(lower)) > (iupper = atoi(upper)))
    {
		send_to_char("AEdit:  Upper must be larger then lower.\n\r",ch);
		return false;
    }

    pArea->low_range = ilower;
    send_to_char("Lower vnum set.\n\r",ch);
    pArea->high_range = iupper;
    send_to_char("Upper vnum set.\n\r",ch);
    return true;
}

AEDIT(aedit_finished){
    AREA_DATA *pArea;

    EDIT_AREA(ch,pArea);

	if (!argument[0]){
		ch->send("Syntax: finished <{Gfinished{x/{Gunfinished{x/{Gdead{x>\n\r");
	}
	pArea->finished = 1 - pArea->finished;
	if(!str_prefix(argument,"finished")){
		pArea->finished = 1;
		ch->send("Area set to '{Gfinished{x' status.\n\r");
	}
    else if(!str_prefix(argument,"unfinished")){
		pArea->finished = 2;
		ch->send("Area set to '{RUnfinished{x' status.\n\r");
	}
	else if(!str_prefix(argument,"dead")){
		pArea->finished = 3;
		ch->send("Area set to '{ddead{x' status.\n\r");
	}
	else if(!str_prefix(argument,"immortal")){
		pArea->finished = 4;
		ch->send("Area set to '{Cimmortal{x' status.\n\r");
	}
	else{
		ch->send("Invalid command. Syntax: finished <{Gfinished{x/{Runfinished{x/{ddead{x/{Cimmortal{x>\n\r");
		return false;
	}
    return true;
}

AEDIT(aedit_group){
	AREA_DATA *pArea;

	EDIT_AREA(ch,pArea);

	if (!argument[0]){
		ch->send("Syntax: group <{G#{x>\n\r");
	}

    if (!is_number(argument)){
		send_to_char("Argument must be a number\n\r",ch);
		return false;
    }
	if(atoi(argument) >= AREA_GROUP_MAX){
		ch->send("Value exceeds maximum allowed.\n\r");
		return false;
	}
	pArea->area_group = atoi(argument);
	return true;
}

AEDIT(aedit_locked){
	AREA_DATA *pArea;

	EDIT_AREA(ch,pArea);

	if(pArea->locked){
		pArea->locked = false;
		ch->send("You have unlocked this area.\n\r");
		return true;
	}
	pArea->locked = true;
	ch->send("You have locked this area.\n\r");
	return true;
}

AEDIT(aedit_lvnum)
{
    AREA_DATA *pArea;
    char lower[MSL];
    int ilower, iupper;

    EDIT_AREA(ch,pArea);

    one_argument(argument,lower);

    if (!is_number(lower) || lower[0] == '\0')
    {
		send_to_char("Syntax:  min_vnum [#xlower]\n\r",ch);
		return false;
    }

    if ((ilower = atoi(lower)) > (iupper = pArea->max_vnum))
    {
		send_to_char("AEdit:  Value must be less than the max_vnum.\n\r",ch);
		return false;
    }
    
    if (!check_range(ilower,iupper))
    {
		send_to_char("AEdit:  Range must include only this area.\n\r",ch);
		return false;
    }

    if (get_vnum_area(ilower) && get_vnum_area(ilower) != pArea)
    {
		send_to_char("AEdit:  Lower vnum already assigned.\n\r",ch);
		return false;
    }

    pArea->min_vnum = ilower;
    send_to_char("Lower vnum set.\n\r",ch);
    return true;
}

AEDIT(aedit_uvnum)
{
    AREA_DATA *pArea;
    char upper[MSL];
    int ilower, iupper;

    EDIT_AREA(ch,pArea);

    one_argument(argument,upper);

    if (!is_number(upper) || upper[0] == '\0')
    {
		send_to_char("Syntax:  max_vnum [#xupper]\n\r",ch);
		return false;
    }

    if ((ilower = pArea->min_vnum) > (iupper = atoi(upper)))
    {
		send_to_char("AEdit:  Upper must be larger then lower.\n\r",ch);
		return false;
    }
    
    if (!check_range(ilower,iupper))
    {
		send_to_char("AEdit:  Range must include only this area.\n\r",ch);
		return false;
    }

    if (get_vnum_area(iupper) && get_vnum_area(iupper) != pArea)
    {
		send_to_char("AEdit:  Upper vnum already assigned.\n\r",ch);
		return false;
    }

    pArea->max_vnum = iupper;
    send_to_char("Upper vnum set.\n\r",ch);

    return true;
}

REDIT(redit_show){
	ROOM_INDEX_DATA	*pRoom;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	PROG_LIST *list;
	char buf[MSL], buf1[2*MSL];
	int door;
	bool fcnt;

	EDIT_ROOM(ch,pRoom);

	buf1[0] = '\0';

	printf_to_char(ch,"Description:\n\r%s",pRoom->description);
	printf_to_char(ch,"Name:       [{c%s{x]\n\rArea:       [{y%5d{x] %s\n\r",pRoom->name,pRoom->area->vnum,pRoom->area->name);
	printf_to_char(ch,"{dVnum{x:       [{y%5d{x]\n\rSector:     [{g%s{x]\n\r",pRoom->vnum,flag_string(sector_flags,pRoom->sector_type));
	printf_to_char(ch,"{dLight{x:      [{y%d{x][{y%d{x]\n\r",pRoom->light,calc_light(pRoom));
	printf_to_char(ch,"Room flags: [{g%s{x]\n\r", flag_string(room_flags,pRoom->room_flags));

	if (pRoom->heal_rate != 100 || pRoom->mana_rate != 100)
		printf_to_char(ch,"Health rec: [{y%d{x]\n\rMana rec  : [{y%d{x]\n\r",pRoom->heal_rate,pRoom->mana_rate);

	if (IS_SET(pRoom->room_flags,ROOM_ARENA_FOYER))
		printf_to_char(ch,"Target Area [{y%d{x]\n\rViewer Room [{y%d{x]\n\r",pRoom->arenavn,pRoom->arenaviewvn);

	if (!IS_NULLSTR(pRoom->owner))
		printf_to_char(ch,"Owner     : [{c%s{x]\n\r",pRoom->owner);

	if (pRoom->extra_descr){
		EXTRA_DESCR_DATA *ed;

		printf_to_char(ch,"Desc Kwds:  [");
		for ( ed = pRoom->extra_descr; ed; ed = ed->next ){
			printf_to_char(ch,ed->keyword);
			if (ed->next)
				send_to_char(" ",ch);
		}
		send_to_char("]\n\r",ch);
	}

	send_to_char("{dCharacters{x: [{c",ch);
	fcnt = false;
	for ( rch = pRoom->people; rch; rch = rch->next_in_room ){
		one_argument(rch->name,buf);
		printf_to_char(ch,"%s ",buf);
		fcnt = true;
	}

	if (fcnt)
		send_to_char("{x]\n\r",ch);
	else
		send_to_char("none{x]\n\r",ch);

	send_to_char("{dObjects{x:    [{c",ch);
	fcnt = false;
	for ( obj = pRoom->contents; obj; obj = obj->next_content ){
		one_argument(obj->name,buf);
		printf_to_char(ch,"%s ",buf);
		fcnt = true;
	}
	if (fcnt)
		send_to_char("{x]\n\r",ch);
	else
		send_to_char("none{x]\n\r",ch);

	for (door = 0;door < MAX_DIR;door++){
		EXIT_DATA *pexit;

		if ((pexit = pRoom->exit[door])){
			char word[MIL],reset_state[MSL];
			char *state;
			int i,length;

			printf_to_char(ch,"-{c%-5s{x to [{y%5d{x] Key: [{y%5d{x] ",
				capitalize(dir_name[door]),
				pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,
				pexit->key);

			strcpy(reset_state,flag_string(exit_flags,pexit->rs_flags));
			state = flag_string(exit_flags,pexit->exit_info);

			ch->send(" Exit flags: [");

			for (; ;){
				state = one_argument(state,word);

				if (!word[0]){
					ch->send("]\n\r");
					break;
				}

				if (str_infix(word,reset_state)){
					length = strlen_color(word);
					for (i = 0; i < length; i++)
						word[i] = UPPER(word[i]);
				}
				printf_to_char(ch,"%s ",word);
			}

			if (pexit->keyword && pexit->keyword[0])
				printf_to_char(ch,"Kwds: [%s]\n\r",pexit->keyword);
			if (pexit->description && pexit->description[0])
				printf_to_char(ch,"%s",pexit->description);
		}
	}

	if (pRoom->rprogs){
		int cnt;

		printf_to_char(ch,"\n\rROOMPrograms for [%5d]:\n\r",pRoom->vnum);

		for (cnt=0, list=pRoom->rprogs; list; list=list->next){
			if (cnt ==0)
				send_to_char(" Number Vnum Trigger Phrase\n\r ------ ---- ------- ------\n\r",ch);

			printf_to_char(ch,"[%5d] %4d %7s %s\n\r",cnt,list->vnum,prog_type_to_name(list->trig_type),list->trig_phrase);
			cnt++;
		}
	}
	return false;
}

bool change_exit(CHAR_DATA * ch,char *argument,int door){
    ROOM_INDEX_DATA *pRoom, *pRoomIndex;
    AREA_DATA *pArea;
    char command[MIL], arg[MIL], buf[MSL], digroom[MSL];
    int value, foundroom, newvnum;

    newvnum = 0;
    foundroom = 0;
    pArea = ch->in_room->area;

    EDIT_ROOM(ch,pRoom);

    /*
     * Set the exit flags, needs full argument.
     * ----------------------------------------
     */
    if ((value = flag_value (exit_flags,argument)) != NO_FLAG)
    {
        ROOM_INDEX_DATA *pToRoom;
        sh_int rev;

        if (!pRoom->exit[door])
        {
            send_to_char("Exit doesn't exist.\n\r",ch);
            return false;
        }

        /*
         * This room.
         */

        TOGGLE_BIT(pRoom->exit[door]->rs_flags,value);

        /* Don't toggle exit_info because it can be changed by players. */
        pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

        /*
         * Connected room.
         */
        pToRoom = pRoom->exit[door]->u1.to_room;
        rev = rev_dir[door];

        if (pToRoom->exit[rev] != NULL)
        {
            pToRoom->exit[rev]->rs_flags = pRoom->exit[door]->rs_flags;
            pToRoom->exit[rev]->exit_info = pRoom->exit[door]->exit_info;
        }

        send_to_char("Exit flag toggled.\n\r",ch);
        return true;
    }

    /*
     * Now parse the arguments.
     */
    argument = one_argument(argument,command);
    one_argument(argument,arg);

    if (command[0] == '\0' && argument[0] == '\0')
    {                            /* Move command. */
        move_char(ch,door,true,true);
        return false;
    }

    if (command[0] == '?')
    {
        do_help(ch,"EXIT");
        return false;
    }

    if (!str_cmp(command,"delete"))
    {
        ROOM_INDEX_DATA *pToRoom;
        sh_int rev;

        if (!pRoom->exit[door])
        {
            send_to_char("REdit:  Cannot delete a null exit.\n\r",ch);
            return false;
        }

        /* Remove ToRoom Exit. */
		if(!IS_SET(pRoom->exit[door]->exit_info,EX_ONEWAY)){
			rev = rev_dir[door];
			pToRoom = pRoom->exit[door]->u1.to_room;

			if (pToRoom->exit[rev])
			{
				free_exit(pToRoom->exit[rev]);
				pToRoom->exit[rev] = NULL;
			}
		}

        /* Remove this exit.*/
        free_exit(pRoom->exit[door]);
        pRoom->exit[door] = NULL;

        send_to_char("Exit unlinked.\n\r",ch);
        return true;
    }

    if (!str_cmp(command,"link"))
    {
        EXIT_DATA *pExit;
        ROOM_INDEX_DATA *toRoom;

        if (arg[0] == '\0' || !is_number(arg))
        {
            send_to_char("Syntax:  [direction] link [vnum]\n\r",ch);
            return false;
        }

        value = atoi(arg);

        if (!(toRoom = get_room_index(value)))
        {
            send_to_char("REdit:  Cannot link to non-existant room.\n\r",ch);
            return false;
        }

        if (!IS_BUILDER(ch,toRoom->area))
        {
            send_to_char("REdit:  Cannot link to that area.\n\r",ch);
            return false;
        }

        if (toRoom->exit[rev_dir[door]])
        {
            send_to_char("REdit:  Remote side's exit already exists.\n\r",ch);
            return false;
        }

        if (!pRoom->exit[door])
            pRoom->exit[door] = new_exit();

        pRoom->exit[door]->u1.to_room = toRoom;
        pRoom->exit[door]->orig_door = door;

        door = rev_dir[door];
        pExit = new_exit();
        pExit->u1.to_room = pRoom;
        pExit->orig_door = door;
        toRoom->exit[door] = pExit;

        send_to_char("Two-way link established.\n\r",ch);
        return true;
    }

    if (!str_cmp(command,"dig"))
    {
    	if (arg[0] == '\0')
    	{
			newvnum = pArea->min_vnum;

			while (foundroom != 1)
			{
				newvnum++;
				if ((pRoomIndex = get_room_index(newvnum)))
					foundroom = 0;
				else
					foundroom = 1;
			}

			if (newvnum > pArea->max_vnum)
			{
				send_to_char("Dig Error: No more free vnums in area.\n\r",ch);
				return false;
			}

			sprintf(digroom,"%d",newvnum);
        	redit_create(ch,digroom);
        	sprintf(buf,"link %s",digroom);
        	change_exit(ch,buf,door);
			return true;
    	}
   		else if (!is_number(arg))
        {
            send_to_char("Error: Argument must be numerical\n\r",ch);
            return false;
        }
		else
		{
        	redit_create(ch,arg);
        	sprintf(buf,"link %s",arg);
        	change_exit(ch,buf,door);
        	return true;
    	}
    }

	if (!str_cmp(command,"room")){
		ROOM_INDEX_DATA *toRoom;
		if (arg[0] == '\0' || !is_number(arg)){
			send_to_char("Syntax:  [direction] room [vnum]\n\r",ch);
			return false;
		}

		value = atoi(arg);

		if (!(toRoom = get_room_index(value))){
			send_to_char("REdit:  Cannot link to non-existant room.\n\r",ch);
			return false;
		}

		if (!pRoom->exit[door])
			pRoom->exit[door] = new_exit();

		pRoom->exit[door]->u1.to_room = toRoom;
		pRoom->exit[door]->orig_door = door;
		SET_BIT(pRoom->exit[door]->exit_info,EX_ONEWAY);

		send_to_char("One-way link established.\n\r",ch);
		return true;
	}

    if (!str_cmp(command,"key"))
    {
        OBJ_INDEX_DATA *key;

        if (arg[0] == '\0' || !is_number(arg))
        {
            send_to_char("Syntax:  [direction] key [vnum]\n\r",ch);
            return false;
        }

        if (!pRoom->exit[door])
        {
            send_to_char("Exit doesn't exist.\n\r",ch);
            return false;
        }

        value = atoi(arg);

        if (!(key = get_obj_index(value)))
        {
            send_to_char("REdit:  Key doesn't exist.\n\r",ch);
            return false;
        }

        if (key->item_type != ITEM_KEY){
            send_to_char("REdit:  Object is not a key.\n\r",ch);
            return false;
        }

        pRoom->exit[door]->key = value;

        send_to_char("Exit key set.\n\r",ch);
        return true;
    }

    if (!str_cmp(command,"name"))
    {
        if (arg[0] == '\0')
        {
            send_to_char("Syntax:  [direction] name [string]\n\r",ch);
            send_to_char("         [direction] name none\n\r",ch);
            return false;
        }

        if (!pRoom->exit[door])
        {
            send_to_char("Exit doesn't exist.\n\r",ch);
            return false;
        }

        free_string(pRoom->exit[door]->keyword);

        if (str_cmp(arg,"none"))
            pRoom->exit[door]->keyword = str_dup(arg);
        else
            pRoom->exit[door]->keyword = str_dup("");

        send_to_char("Exit name set.\n\r",ch);
        return true;
    }

    if (!str_prefix(command,"description"))
    {
        if (arg[0] == '\0')
        {
            if (!pRoom->exit[door])
            {
                send_to_char("Exit doesn't exist.\n\r",ch);
                return false;
            }

            string_append(ch,&pRoom->exit[door]->description);
            return true;
        }

        send_to_char("Syntax:  [direction] desc\n\r",ch);
        return false;
    }

    return false;
}

REDIT(redit_north)
{
    if (change_exit(ch,argument,DIR_NORTH))
		return true;

    return false;
}

REDIT(redit_south)
{
    if (change_exit(ch,argument,DIR_SOUTH))
		return true;

    return false;
}

REDIT(redit_east)
{
    if (change_exit(ch,argument,DIR_EAST))
		return true;

    return false;
}

REDIT(redit_west)
{
    if (change_exit(ch,argument,DIR_WEST))
		return true;

    return false;
}

REDIT(redit_up)
{
    if (change_exit(ch,argument,DIR_UP))
		return true;

    return false;
}

REDIT(redit_down)
{
    if (change_exit(ch,argument,DIR_DOWN))
		return true;

    return false;
}

REDIT(redit_ed)
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MIL], keyword[MIL];

    EDIT_ROOM(ch,pRoom);

    argument = one_argument(argument,command);
    one_argument(argument,keyword);

    if (command[0] == '\0' || keyword[0] == '\0')
    {
		send_to_char("Syntax:  ed add [keyword]\n\r",ch);
		send_to_char("         ed edit [keyword]\n\r",ch);
		send_to_char("         ed delete [keyword]\n\r",ch);
		send_to_char("         ed format [keyword]\n\r",ch);
		return false;
    }

    if (!str_cmp(command,"add"))
    {
		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed add [keyword]\n\r",ch);
			return false;
		}

		ed					=   new_extra_descr();
		ed->keyword			=   str_dup(keyword);
		ed->description		=   str_dup("");
		ed->next			=   pRoom->extra_descr;
		pRoom->extra_descr	=   ed;

		string_append(ch,&ed->description);

		return true;
    }


    if (!str_cmp(command,"edit"))
    {
		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed edit [keyword]\n\r",ch);
			return false;
		}

		for ( ed = pRoom->extra_descr; ed; ed = ed->next )
		{
			if (is_name(keyword,ed->keyword))
				break;
		}

		if (!ed)
		{
			send_to_char("REdit:  Extra description keyword not found.\n\r",ch);
			return false;
		}

		string_append(ch,&ed->description);

		return true;
    }


    if (!str_cmp(command,"delete"))
    {
		EXTRA_DESCR_DATA *ped = NULL;

		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed delete [keyword]\n\r",ch);
			return false;
		}

		for ( ed = pRoom->extra_descr; ed; ed = ed->next )
		{
			if (is_name(keyword,ed->keyword))
				break;
			ped = ed;
		}

		if (!ed)
		{
			send_to_char("REdit:  Extra description keyword not found.\n\r",ch);
			return false;
		}

		if (!ped)
			pRoom->extra_descr = ed->next;
		else
			ped->next = ed->next;

		free_extra_descr(ed);

		send_to_char("Extra description deleted.\n\r",ch);
		return true;
    }


    if (!str_cmp(command,"format"))
    {
		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed format [keyword]\n\r",ch);
			return false;
		}

		for ( ed = pRoom->extra_descr; ed; ed = ed->next )
			if (is_name(keyword,ed->keyword))
				break;

		if (!ed)
		{
			send_to_char("REdit:  Extra description keyword not found.\n\r",ch);
			return false;
		}

		ed->description = format_string(ed->description);

		send_to_char("Extra description formatted.\n\r",ch);
		return true;
    }

    redit_ed(ch,"");
    return false;
}

REDIT(redit_create)
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value, iHash;
    
    EDIT_ROOM(ch,pRoom);

    value = atoi(argument);

    if (argument[0] == '\0' || value <= 0)
    {
		send_to_char("Syntax:  create [vnum > 0]\n\r",ch);
		return false;
    }

    pArea = get_vnum_area(value);
    if (!pArea)
    {
		send_to_char("REdit:  That vnum is not assigned an area.\n\r",ch);
		return false;
    }

    if (!IS_BUILDER(ch,pArea))
    {
		send_to_char("REdit:  Vnum in an area you cannot build in.\n\r",ch);
		return false;
    }

    if (get_room_index(value))
    {
		send_to_char("REdit:  Room vnum already exists.\n\r",ch);
		return false;
    }

    pRoom				= new_room_index();
    pRoom->area			= pArea;
    pRoom->vnum			= value;
	pRoom->arenavn		= 0;
	pRoom->arenaviewvn	= 0;

    if (value > top_vnum_room)
        top_vnum_room = value;

    iHash					= value % MAX_KEY_HASH;
    pRoom->next				= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoom;
    ch->desc->pEdit			= (void *)pRoom;

    send_to_char("Room created.\n\r",ch);
    return true;
}

REDIT(redit_name)
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch,pRoom);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  name [name]\n\r",ch);
		return false;
    }

    free_string(pRoom->name);
    pRoom->name = str_dup(argument);

    send_to_char("Name set.\n\r",ch);
    return true;
}

REDIT(redit_desc)
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch,pRoom);

    if (argument[0] == '\0')
    {
		string_append(ch,&pRoom->description);
		return true;
    }

    send_to_char("Syntax:  desc\n\r",ch);
    return false;
}

REDIT(redit_heal)
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch,pRoom);

	if (is_number(argument))
	{
		pRoom->heal_rate = atoi(argument);
		send_to_char("Heal rate set.\n\r",ch);
		return true;
	}

    send_to_char("Syntax : heal <#xnumber>\n\r",ch);
    return false;
}

REDIT(redit_arenavn)
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch,pRoom);

	if(!IS_SET(pRoom->room_flags,ROOM_ARENA_FOYER))
	{
		send_to_char("Only arena foyers may have a target area vnum.\n\r",ch);
		return false;
	}
    if (is_number(argument))
	{
		pRoom->arenavn = atoi(argument);
		send_to_char("Arena AreaVN set.\n\r",ch);
		return true;
	}

    send_to_char("Syntax : arenadestvn <#xnumber>\n\r",ch);
    return false;
}

REDIT(redit_arenaviewvn)
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch,pRoom);

	if(!IS_SET(pRoom->room_flags,ROOM_ARENA_FOYER))
	{
		send_to_char("Only arena foyers may have a target area vnum.\n\r",ch);
		return false;
	}

    if (is_number(argument))
	{
		pRoom->arenaviewvn = atoi(argument);
		send_to_char("Arena AreaVN set.\n\r",ch);
		return true;
	}

    send_to_char("Syntax : arenaviewvn <#xnumber>\n\r",ch);
    return false;
}

REDIT(redit_mana)
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch,pRoom);

    if (is_number(argument))
	{
		pRoom->mana_rate = atoi(argument);
		send_to_char("Mana rate set.\n\r",ch);
		return true;
	}

    send_to_char("Syntax : mana <#xnumber>\n\r",ch);
    return false;
}

REDIT(redit_format)
{
    ROOM_INDEX_DATA *pRoom;
    EDIT_ROOM(ch,pRoom);

    pRoom->description = format_string(pRoom->description);
    send_to_char("String formatted.\n\r",ch);
    return true;
}

REDIT(redit_mreset)
{
    ROOM_INDEX_DATA *pRoom;
    MOB_INDEX_DATA *pMobIndex;
    RESET_DATA *pReset;
    CHAR_DATA *newmob;
    char arg[MIL], arg2[MIL];

    EDIT_ROOM(ch,pRoom);

    argument = one_argument(argument,arg);
    argument = one_argument(argument,arg2);

    if (arg[0] == '\0' || !is_number(arg))
    {
		send_to_char("Syntax:  mreset <vnum> <max #x> <mix #x>\n\r",ch);
		return false;
    }

    if (!(pMobIndex = get_mob_index(atoi(arg))))
    {
		send_to_char("REdit: No mobile has that vnum.\n\r",ch);
		return false;
    }

    if (pMobIndex->area != pRoom->area)
    {
		send_to_char("REdit: No such mobile in this area.\n\r",ch);
		return false;
    }

    /*
     * Create the mobile reset.
     */
    pReset              = new_reset_data();
    pReset->command		= 'M';
    pReset->arg1		= pMobIndex->vnum;
    pReset->arg2		= is_number(arg2) ? atoi(arg2) : MAX_MOB;
    pReset->arg3		= pRoom->vnum;
    pReset->arg4		= is_number(argument) ? atoi(argument) : 1;
    add_reset(pRoom,pReset,0);

    /*
     * Create the mobile.
     */
    newmob = create_mobile(pMobIndex);
    char_to_room(newmob,pRoom);

    printf_to_char(ch,"%s (%d) has been loaded and added to resets.\n\rThere will be a maximum of %d loaded to this room.\n\r",capitalize(pMobIndex->short_descr),pMobIndex->vnum,pReset->arg2);
    act("$n has created $N!",ch,NULL,newmob,TO_ROOM);
    return true;
}

struct wear_type
{
    int	wear_loc,wear_bit;
};

const struct wear_type wear_table[] =
{
    {	WEAR_NONE,		ITEM_TAKE			},
    {	WEAR_LIGHT,		ITEM_LIGHT			},
    {	WEAR_LIGHT,		ITEM_DARK			},
    {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
    {	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
    {	WEAR_ELBOW_L,	ITEM_WEAR_ELBOW		},
    {	WEAR_ELBOW_R,	ITEM_WEAR_ELBOW		},
    {	WEAR_KNEE_L,	ITEM_WEAR_KNEE		},
    {	WEAR_KNEE_R,	ITEM_WEAR_KNEE		},
    {	WEAR_SHIN_L,	ITEM_WEAR_SHIN		},
    {	WEAR_SHIN_R,	ITEM_WEAR_SHIN		},
    {	WEAR_EAR_L,		ITEM_WEAR_EAR		},
    {	WEAR_EAR_R,		ITEM_WEAR_EAR		},
    {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
    {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
    {	WEAR_TORSO,		ITEM_WEAR_TORSO		},
    {	WEAR_HEAD,		ITEM_WEAR_HEAD		},
    {	WEAR_LEGS,		ITEM_WEAR_LEGS		},
    {	WEAR_SHOULDER,	ITEM_WEAR_SHOULDER	},
    {	WEAR_QUIVER,	ITEM_WEAR_QUIVER	},
    {	WEAR_FEET,		ITEM_WEAR_FEET		},
    {	WEAR_HANDS,		ITEM_WEAR_HANDS		},
    {	WEAR_ARMS,		ITEM_WEAR_ARMS		},
    {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
    {	WEAR_FACE,		ITEM_WEAR_FACE		},
    {	WEAR_PENDANT,	ITEM_WEAR_PENDANT	},
    {	WEAR_TATTOO,	ITEM_WEAR_TATTOO	},
    {	WEAR_ABOUT,		ITEM_WEAR_ABOUT		},
    {	WEAR_WAIST,		ITEM_WEAR_WAIST		},
    {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
    {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
    {	WEAR_ANKLE_L,	ITEM_WEAR_ANKLE		},
    {	WEAR_ANKLE_R,	ITEM_WEAR_ANKLE		},
    {	WEAR_WIELD,		ITEM_WIELD			},
    {	WEAR_HOLD_R,	ITEM_HOLD			},
    {	WEAR_HOLD_L,	ITEM_HOLD			},
    {	NO_FLAG,		NO_FLAG				}
};

/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
    l_int flag;
 
    for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
        if (IS_SET(bits,wear_table[flag].wear_bit) && --count < 1)
            return wear_table[flag].wear_loc;
 
    return NO_FLAG;
}

/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
    l_int flag;
 
    for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
        if (loc == wear_table[flag].wear_loc)
            return wear_table[flag].wear_bit;
 
    return 0;
}

REDIT(redit_oreset)
{
    ROOM_INDEX_DATA	*pRoom;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA *newobj, *to_obj;
    CHAR_DATA		*to_mob;
    char arg1[MIL], arg2[MIL];
    int olevel = 0;
    RESET_DATA *pReset;

    EDIT_ROOM(ch,pRoom);

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if (arg1[0] == '\0' || !is_number(arg1))
    {
		send_to_char("Syntax:  oreset <vnum> <args>\n\r",ch);
		send_to_char("        -no_args               = into room\n\r",ch);
		send_to_char("        -<obj_name>            = into obj\n\r",ch);
		send_to_char("        -<mob_name> <wear_loc> = into mob\n\r",ch);
		return false;
    }

    if (!(pObjIndex = get_obj_index(atoi(arg1))))
    {
		send_to_char("REdit: No object has that vnum.\n\r",ch);
		return false;
    }

    if (pObjIndex->area != pRoom->area)
    {
		send_to_char("REdit: No such object in this area.\n\r",ch);
		return false;
    }

    /*
     * Load into room.
     */
    if (arg2[0] == '\0')
    {
		pReset			= new_reset_data();
		pReset->command	= 'O';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= pRoom->vnum;
		pReset->arg4	= 0;
		add_reset(pRoom,pReset,0);

		newobj = create_object(pObjIndex,number_fuzzy(olevel));
		obj_to_room(newobj,pRoom);

		printf_to_char(ch,"%s (%d) has been loaded and added to resets.\n\r",capitalize(pObjIndex->short_descr),pObjIndex->vnum);
    }
    else  //Load into object's inventory.
		if (argument[0] == '\0' && ((to_obj = get_obj_list(ch,arg2,pRoom->contents)) != NULL))
		{
			pReset			= new_reset_data();
			pReset->command	= 'P';
			pReset->arg1	= pObjIndex->vnum;
			pReset->arg2	= 0;
			pReset->arg3	= to_obj->pIndexData->vnum;
			pReset->arg4	= 1;
			add_reset(pRoom,pReset,0);

			newobj = create_object(pObjIndex,number_fuzzy(olevel));
			newobj->cost = 0;
			obj_to_obj(newobj,to_obj);

			printf_to_char(ch,"%s (%d) has been loaded into " "%s (%d) and added to resets.\n\r",capitalize(newobj->short_descr),newobj->pIndexData->vnum,to_obj->short_descr,to_obj->pIndexData->vnum);
		}
	    else  //Load into mobile's inventory.
    if ((to_mob = get_char_room(ch,NULL,arg2)) != NULL)
    {
		int	wear_loc;
		// Make sure the location on mobile is valid.
		if ((wear_loc = flag_value(wear_loc_flags,argument)) == NO_FLAG)
		{
			send_to_char("REdit: Invalid wear_loc.  '? wear-loc'\n\r",ch);
			return false;
		}

		/*
		 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
		 */
		if ( !IS_SET(pObjIndex->wear_flags,wear_bit(wear_loc)))
		{
			printf_to_char(ch,"%s (%d) has wear flags: [%s]\n\r",capitalize(pObjIndex->short_descr),pObjIndex->vnum,flag_string(wear_flags,pObjIndex->wear_flags));
			return false;
		}
		//Can't load into same position.
		if (get_eq_char(to_mob,wear_loc))
		{
			send_to_char("REdit:  Object already equipped.\n\r",ch);
			return false;
		}

		pReset			= new_reset_data();
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= wear_loc;
		if (pReset->arg2 == WEAR_NONE)
			pReset->command = 'G';
		else
			pReset->command = 'E';
		pReset->arg3	= wear_loc;

		add_reset(pRoom,pReset,0);

		olevel  = URANGE(0,to_mob->level - 2,LEVEL_HERO);
		newobj = create_object(pObjIndex,number_fuzzy(olevel));

		if (to_mob->pIndexData->pShop)	/* Shop-keeper? */
		{
			switch (pObjIndex->item_type)
			{
			default:
				olevel = 0;
				break;
			case ITEM_PILL:
				olevel = number_range(0,10);
				break;
			case ITEM_POTION:
				olevel = number_range(0,10);
				break;
			case ITEM_SCROLL:
				olevel = number_range(5,15);
				break;
			case ITEM_WAND:
				olevel = number_range(10,20);
				break;
			case ITEM_STAFF:
				olevel = number_range(15,25);
				break;
			case ITEM_ARMOR:
			case ITEM_SHIELD:
				olevel = number_range(5,15);
				break;
			case ITEM_WEAPON:
				if (pReset->command == 'G')
					olevel = number_range(5,15);
				else
					olevel = number_fuzzy(olevel);
				break;
			}

			newobj = create_object(pObjIndex,olevel);
			if (pReset->arg2 == WEAR_NONE)
				SET_BIT(newobj->extra_flags,ITM_INVENTORY);
		}
		else
			newobj = create_object(pObjIndex,number_fuzzy(olevel));

		obj_to_char(newobj,to_mob);
		if (pReset->command == 'E')
			equip_char(to_mob,newobj,pReset->arg3);

		printf_to_char(ch,"%s (%d) has been loaded %s of %s (%d) and added to resets.\n\r",capitalize(pObjIndex->short_descr),pObjIndex->vnum,flag_string(wear_loc_strings,pReset->arg3),to_mob->short_descr,to_mob->pIndexData->vnum);
    }
    else	/* Display Syntax */
    {
		send_to_char("REdit:  That mobile isn't here.\n\r",ch);
		return false;
    }

    act("$n has created $p!",ch,newobj,NULL,TO_ROOM);
    return true;
}

//Object Editor Functions.
void show_obj_values(CHAR_DATA *ch,OBJ_INDEX_DATA *obj)
{
    switch(obj->item_type)
    {
		default:	/* No values. */
			break;
		case ITEM_LOCKER://nash left v0 open for future pk related additions
			printf_to_char(ch,"[{cv0{x] DonorChest: [%s]\n\r[{cv1{x] Flags:      [%s]\n\r[{cv2{x] Key:     %s [%d]\n\r[{cv3{x] Capacity    [%d]\n\r",
				obj->value[0] ? "true" : "false",
				flag_string(container_flags,obj->value[1]),get_obj_index(obj->value[2]) ? get_obj_index(obj->value[2])->short_descr : "none",
				obj->value[2],
				obj->value[3]);
			break;
		case ITEM_HONINGSTONE:
			printf_to_char(ch,"[{cv2{x] Uses:   [%d]\n\r",obj->value[2]);
			break;
		case ITEM_LIGHT:
			if (obj->value[2] == -1 || obj->value[2] == 999)
				printf_to_char(ch,"[{cv2{x] Light:  Infinite[-1]\n\r");
			else
				printf_to_char(ch,"[{cv2{x] Light:  [%d]\n\r", obj->value[2] );
			break;
		case ITEM_DARK:
			if (obj->value[2] == -1 || obj->value[2] == 999)
				printf_to_char(ch,"[{cv2{x] Dark:   Infinite[-1]\n\r");
			else
				printf_to_char(ch,"[{cv2{x] Dark:   [%d]\n\r", obj->value[2] );
			break;
		case ITEM_NEEDLE:
				printf_to_char(ch,"[{cv0{x] Uses:  [%d]\n\r", obj->value[0] );
			break;
		case ITEM_QUIVER:
			printf_to_char(ch,"[{cv0{x] Number of Arrows:     %d\n\r[{cv3{x] Arrow VNUM:           %d\n\r[{cv4{x] Flags:                %s\n\r",
				obj->value[0],
				obj->value[3],
				flag_string(arrow_flags,obj->value[4]));
			break;
		case ITEM_ARROW:
/*			printf_to_char(ch,"[{cv1{x] Number of Dice:       %d\n\r[{cv2{x] Type of Dice:         %d\n\r[{cv4{x] Spell:                %s\n\r",
				obj->value[1],
				obj->value[2],
				obj->value[4] != -1 ? skill_table[obj->value[3]].name : "none");*/
			break;
		case ITEM_WAND:
		case ITEM_STAFF:
			printf_to_char(ch,"[{cv0{x] Level:          [%d]\n\r[{cv1{x] Charges Total:  [%d]\n\r[{cv2{x] Charges Left:   [%d]\n\r[{cv3{x] Spell:          %s\n\r",
				obj->value[0],
				obj->value[1],
				obj->value[2],
				obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none");
			break;
		case ITEM_PORTAL:
			printf_to_char(ch,"[{cv0{x] Charges:        [%d]\n\r[{cv1{x] Exit Flags:     %s\n\r[{cv2{x] Portal Flags:   %s\n\r[{cv3{x] Goes to (vnum): [%d]\n\r",
				obj->value[0],
				flag_string(exit_flags,obj->value[1]),
				flag_string(portal_flags,obj->value[2]),
				obj->value[3]);
			break;
		case ITEM_FURNITURE:          
			printf_to_char(ch,"[{cv0{x] Max people:      [%d]\n\r[{cv1{x] Max weight:      [%d]\n\r[{cv2{x] Furniture Flags: %s\n\r[{cv3{x] Heal bonus:      [%d]\n\r[{cv4{x] Mana bonus:      [%d]\n\r",
				obj->value[0],
				obj->value[1],
				flag_string(furniture_flags, obj->value[2]),
				obj->value[3],
				obj->value[4]);
			break;
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			printf_to_char(ch,"[{cv0{x] Level:  [%d]\n\r[{cv1{x] Spell:  %s\n\r[{cv2{x] Spell:  %s\n\r[{cv3{x] Spell:  %s\n\r[{cv4{x] Spell:  %s\n\r",
				obj->value[0],
				obj->value[1] != -1 ? skill_table[obj->value[1]].name : "none",
				obj->value[2] != -1 ? skill_table[obj->value[2]].name : "none",
				obj->value[3] != -1 ? skill_table[obj->value[3]].name : "none",
				obj->value[4] != -1 ? skill_table[obj->value[4]].name : "none");
			break;
		case ITEM_ARMOR:
		case ITEM_SHIELD:
			printf_to_char(ch,"[{cv0{x] Ac pierce   [{y%4d{x] (10ths)\n\r[{cv1{x] Ac bash     [{y%4d{x] (10ths)\n\r[{cv2{x] Ac slash    [{y%4d{x] (10ths)\n\r[{cv3{x] Ac exotic   [{y%4d{x] (10ths)\n\r",
				obj->value[0] * -1,
				obj->value[1] * -1,
				obj->value[2] * -1,
				obj->value[3] * -1);
			break;
		case ITEM_WEAPON:
			printf_to_char(ch,"[{cv0{x] Weapon class:   %s\n\r[{cv1{x] Number of dice: [%d]\n\r[{cv2{x] Type of dice:   [%d]\n\r[{cv3{x] Type:           %s\n\r",
				flag_string(weapon_class,obj->value[0]),
				obj->value[1],
				obj->value[2],
				attack_table[obj->value[3]].name);
			break;
		case ITEM_THROWINGKNIFE:
		case ITEM_THROWINGDART:
		case ITEM_THROWINGAXE:
			printf_to_char(ch,"[{cv0{x] Uses:       %d\n\r[{cv1{x] Number of dice: [%d]\n\r[{cv2{x] Type of dice:   [%d]\n\r[{cv3{x] Type:           %s\n\r[{cv4{x] Special type:   %s\n\r",
				obj->value[0],
				obj->value[1],
				obj->value[2],
				attack_table[obj->value[3]].name,
				flag_string(weapon_type2,obj->value[4]));
			break;
		case ITEM_TRAPPARTS:
			printf_to_char(ch,"[{cv0{x] Trap class:   %s\n\r[{cv1{x] Difficulty rating: [%d]\n\r[{cv2{x] Special type:   %s\n\r",
				flag_string(trap_class,obj->value[0]),
				obj->value[1],
				flag_string(trap_type,obj->value[2]));
			break;
		case ITEM_CALTROPS:
			printf_to_char(ch,"[{cv0{x] Caltrops grade:   %d\n\r[{cv1{x] Caltrops quantity: [%d]\n\r",obj->value[0],obj->value[1]);
			break;
		case ITEM_CONTAINER:
			printf_to_char(ch,"[{cv0{x] Weight:     [%d kg]\n\r[{cv1{x] Flags:      [%s]\n\r[{cv2{x] Key:     %s [%d]\n\r[{cv3{x] Capacity    [%d]\n\r[{cv4{x] Weight Mult [%d]\n\r",
				obj->value[0],
				flag_string(container_flags,obj->value[1]),get_obj_index(obj->value[2]) ? get_obj_index(obj->value[2])->short_descr : "none",
				obj->value[2],
				obj->value[3],
				obj->value[4]);
			break;
		case ITEM_DRINK_CON:
			printf_to_char(ch,"[{cv0{x] Liquid Total: [%d]\n\r[{cv1{x] Liquid Left:  [%d]\n\r[{cv2{x] Liquid:       %s\n\r[{cv3{x] Poisoned:     %s\n\r",
				obj->value[0],
				obj->value[1],
				liq_table[obj->value[2]].liq_name,
				obj->value[3] != 0 ? "Yes" : "No");
			break;
		case ITEM_FOUNTAIN:
			printf_to_char(ch,"[{cv0{x] Liquid Total: [%d]\n\r[{cv1{x] Liquid Left:  [%d]\n\r[{cv2{x] Liquid:	    %s\n\r",
				obj->value[0],
				obj->value[1],
				liq_table[obj->value[2]].liq_name);
			break;
		case ITEM_LOCKPICK:
			printf_to_char(ch,"[{cv0{x] Picking Power: [%s]\n\r[{cv1{x] Uses:          [%d]\n\r",
				pick_table[obj->value[0]].name,
				obj->value[1]);
			break;
		case ITEM_FOOD:
			printf_to_char(ch,"[{cv0{x] Food hours: [%d]\n\r[{cv1{x] Full hours: [%d]\n\r[{cv3{x] Poisoned:   %s\n\r",
				obj->value[0],
				obj->value[1],
				obj->value[3] != 0 ? "Yes" : "No");
			break;
		case ITEM_FORGE:
			printf_to_char(ch,"[{cv0{x] Heat type:        [%d]\n\r[{cv1{x] Fuel efficiency:  [%d]\n\r[{cv2{x] Load capacity:    [%d]\n\r",
				obj->value[0],
				obj->value[1],
				obj->value[2]);
			break;
		case ITEM_MONEY:
			printf_to_char(ch,"[{cv0{x] Gold:   [%d]\n\rSilver:   [%d]",obj->value[0],obj->value[1]);
			break;
		case ITEM_FORGEFUEL:
			printf_to_char(ch,"[{cv0{x] Efficiency:   [%d]\n\r",obj->value[0]);
			break;
    }
    return;
}

bool new_AC(CHAR_DATA *ch,OBJ_INDEX_DATA *pObj,int value_num,int argumento){
	int i = 0,points = 0,dedNum = 0,tval = 0;

	printf_to_char(ch,"%d -> v[%d]\n\r",argumento,value_num);
	if(argumento == 0)
		return true;

	for (i=0; i<4; i++){
		if (i == value_num){
			dedNum += argumento;
			printf_to_char(ch,"+v[%d] (%d) = %d\n\r",i,argumento,dedNum);
		}
		else{
			dedNum += pObj->value[i];
			printf_to_char(ch,"+v[%d] (%d) = %d\n\r",i,pObj->value[i],dedNum);
		}
	}
	printf_to_char(ch,"New AC: {d%d{x\n\r",dedNum);

	tval = pObj->value[value_num];
	pObj->value[value_num] = argumento;
	points = calc_points(pObj);
	printf_to_char(ch,"Points: {d%d{x %d\n\r",points,gen_basepoints(pObj));
	if (points > gen_basepoints(pObj)){
		ch->send("New value exceeds points allotment.\n\r");
		pObj->value[value_num] = tval;
		return false;
	}

	if ((pObj->item_type != ITEM_ARMOR && pObj->item_type != ITEM_SHIELD) || pObj->armortype_flags == 0)
		return false;
	else
		switch (pObj->armortype_flags){
			default:
				return false;
			case ARMOR_CLOTHING:
				printf_to_char(ch,"Clothing Max AC: %d\n\r",pObj->level*4);
				if(argumento > pObj->level){
					printf_to_char(ch,"Value (%d) exceeds max per slot value (%d)\n\r",argumento,pObj->level);
					return false;
				}
				if(dedNum > pObj->level*4){
					printf_to_char(ch,"Exceeded Legal Maxvalue: %d\n\r",pObj->level*4);
					return false;
				}
				else
					return true;
			case ARMOR_LIGHT:
				printf_to_char(ch,"%d Light Max AC: %d\n\r",pObj->level,pObj->level * 6);
				if(argumento > (int)(pObj->level * 1.5)){
					printf_to_char(ch,"Value (%d) exceeds max per slot value (%d)\n\r",argumento,(int)(pObj->level * 1.5));
					return false;
				}
				if(dedNum > (int)(pObj->level * 6)){
					printf_to_char(ch,"Exceeded Legal Maxvalue: %d\n\r",(int)(pObj->level * 6));
					return false;
				}
				else
					return true;
			case ARMOR_MEDIUM:
				printf_to_char(ch,"Medium Max AC: %d\n\r",pObj->level*8);
				if(argumento > pObj->level * 2){
					printf_to_char(ch,"Value (%d) exceeds max per slot value (%d)\n\r",argumento,pObj->level * 2);
					return false;
				}
				if(dedNum > pObj->level * 8){
					printf_to_char(ch,"Exceeded Legal Maxvalue: %d\n\r",pObj->level * 8);
					return false;
				}
				else
					return true;
			case ARMOR_HEAVY:
				printf_to_char(ch,"Heavy Max AC: %d\n\r",(int)(pObj->level*10));
				if(argumento > (int)(pObj->level * 2.5)){
					printf_to_char(ch,"Value (%d) exceeds max per slot value (%d)\n\r",argumento,(int)(pObj->level * 2.5));
					return false;
				}
				if(dedNum > (int)(pObj->level * 10)){
					printf_to_char(ch,"Exceeded Legal Maxvalue: %d\n\r",(int)(pObj->level * 10));
					return false;
				}
				else
					return true;
		}
	return false;
}

bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, char *argument){
	int n = 0;
    switch(pObj->item_type)
    {
        default:
            break;
        case ITEM_HONINGSTONE:
			switch (value_num){
			default:
				ch->send("v2 = uses\n\r");
				return false;
			case 2:
				if(atoi(argument) > 0 && atoi(argument) < 1000){
					send_to_char("Number of uses set.\n\r\n\r",ch);
					pObj->value[2] = atoi(argument);
					break;
				}
				ch->send("Value must be between 1 and 999\n\r");
				return false;
			}
            break;
        case ITEM_LIGHT:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_LIGHT");
				return false;
			case 2:
				send_to_char("HOURS OF LIGHT SET.\n\r\n\r",ch);
				pObj->value[2] = atoi(argument);
				break;
			}
            break;
        case ITEM_DARK:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_DARK");
				return false;
			case 2:
				send_to_char("HOURS OF DARK SET.\n\r\n\r",ch);
				pObj->value[2] = atoi(argument);
				break;
			}
            break;
        case ITEM_NEEDLE:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_NEEDLE");
				return false;
			case 2:
				if(atoi(argument) > 10)
					ch->send("Max value is 10.\n\r");
				else{
					send_to_char("USES SET.\n\r\n\r",ch);
					pObj->value[0] = atoi(argument);
				}
				break;
			}
            break;
		case ITEM_QUIVER:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_QUIVER");
				return false;
			case 0:
				send_to_char("NUMBER OF ARROWS SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 3:
	    		send_to_char("ARROW VNUM SET.\n\r\n\r",ch);
	    		pObj->value[3] = atoi(argument);
				break;
			case 4:
	    		send_to_char("ARROW SPELL SET.\n\r\n\r",ch);
				ALT_FLAGVALUE_SET(pObj->value[4],arrow_flags,argument);
				break;
			}
            break;
		case ITEM_ARROW:
			switch (value_num)
			{
				default:
					do_help(ch,"ITEM_ARROW");
					return false;/*
				case 1:
					send_to_char("NUMBER OF DICE SET.\n\r\n\r",ch);
					pObj->value[1] = atoi(argument);
					break;
				case 2:
					send_to_char("TYPE OF DICE SET.\n\r\n\r",ch);
					pObj->value[2] = atoi(argument);
					break;*/
			}
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_STAFF_WAND");
				return false;
			case 0:
				send_to_char("SPELL LEVEL SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				send_to_char("TOTAL NUMBER OF CHARGES SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			case 2:
				send_to_char("CURRENT NUMBER OF CHARGES SET.\n\r\n\r",ch);
				pObj->value[2] = atoi(argument);
				break;
			case 3:
				send_to_char("SPELL TYPE SET.\n\r",ch);
				pObj->value[3] = skill_lookup(argument);
				break;
			}
            break;
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_SCROLL_POTION_PILL");
				return false;
			case 0:
				send_to_char("SPELL LEVEL SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				send_to_char("SPELL TYPE 1 SET.\n\r\n\r",ch);
				pObj->value[1] = skill_lookup(argument);
				break;
			case 2:
				send_to_char("SPELL TYPE 2 SET.\n\r\n\r",ch);
				pObj->value[2] = skill_lookup(argument);
				break;
			case 3:
				send_to_char("SPELL TYPE 3 SET.\n\r\n\r",ch);
				pObj->value[3] = skill_lookup(argument);
				break;
			case 4:
				send_to_char("SPELL TYPE 4 SET.\n\r\n\r",ch);
				pObj->value[4] = skill_lookup(argument);
				break;
 			}
			break;
        case ITEM_ARMOR:
        case ITEM_SHIELD:
			if (!new_AC(ch,pObj,value_num,atoi(argument)))
			{
				send_to_char("Illegal value for AC.\n\r\n\r",ch);
				break;
			}
			else
			{
				switch (value_num)
				{
					default:
						do_help(ch,"ITEM_ARMOR");
						return false;
					case 0:
						send_to_char("AC PIERCE SET.\n\r\n\r",ch);
						pObj->value[0] = atoi(argument);
						break;
					case 1:
						send_to_char("AC BASH SET.\n\r\n\r",ch);
						pObj->value[1] = atoi(argument);
						break;
					case 2:
						send_to_char("AC SLASH SET.\n\r\n\r",ch);
						pObj->value[2] = atoi(argument);
						break;
					case 3:
						send_to_char("AC EXOTIC SET.\n\r\n\r",ch);
						pObj->value[3] = atoi(argument);
						break;
				}
				break;
			}
        case ITEM_WEAPON:
			switch (value_num){
			default:
				do_help(ch,"ITEM_WEAPON");
				return false;
			case 0:
				send_to_char("WEAPON CLASS SET.\n\r\n\r",ch);
				ALT_FLAGVALUE_SET(pObj->value[0],weapon_class,argument);
				break;
			case 1:
				/*if(ch->level != MAX_LEVEL){
					ch->send("You do not have rights to do that. Use 'autoweapon' instead.\n\r");
					return false;
				}*/
				send_to_char("NUMBER OF DICE SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			case 2:
				/*if(ch->level != MAX_LEVEL){
					ch->send("You do not have rights to do that. Use 'autoweapon' instead.\n\r");
					return false;
				}*/
				send_to_char("TYPE OF DICE SET.\n\r\n\r",ch);
				pObj->value[2] = atoi(argument);
				break;
			case 3:
				send_to_char("WEAPON TYPE SET.\n\r\n\r",ch);
				pObj->value[3] = attack_lookup(argument);
				break;
			}
            break;
        case ITEM_THROWINGKNIFE:
        case ITEM_THROWINGDART:
        case ITEM_THROWINGAXE:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_WEAPON");
				return false;
			case 0:
				send_to_char("WEAPON USES SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				send_to_char("NUMBER OF DICE SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			case 2:
				send_to_char("TYPE OF DICE SET.\n\r\n\r",ch);
				pObj->value[2] = atoi(argument);
				break;
			case 3:
				send_to_char("WEAPON TYPE SET.\n\r\n\r",ch);
				pObj->value[3] = attack_lookup(argument);
				break;
			case 4:
				send_to_char("SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r",ch);
				ALT_FLAGVALUE_TOGGLE(pObj->value[4],weapon_type2,argument);
				break;
			}
            break;
        case ITEM_TRAPPARTS:
			switch (value_num){
			default:
				do_help(ch,"ITEM_TRAPPARTS");
				return false;
			case 0:
				send_to_char("TrapParts CLASS SET.\n\r\n\r",ch);
				ALT_FLAGVALUE_SET(pObj->value[0],trap_class,argument);
				break;
			case 1:
				send_to_char("DIFFICULTY RATING SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			case 2:
				send_to_char("SPECIAL TRAP TYPE TOGGLED.\n\r\n\r",ch);
				ALT_FLAGVALUE_TOGGLE(pObj->value[2],trap_type,argument);
				break;
			}
            break;
        case ITEM_CALTROPS:
			switch (value_num){
			default:
				do_help(ch,"ITEM_CALTROPS");
				return false;
			case 0:
				ch->send("GRADE SET.\n\r\n\r");
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				ch->send("QUANTITY SET.\n\r\n\r");
				pObj->value[1] = atoi(argument);
				break;
			}
            break;
		case ITEM_PORTAL:
			switch (value_num){
			default:
				do_help(ch,"ITEM_PORTAL");
				return false;
			case 0:
	    		send_to_char("CHARGES SET.\n\r\n\r",ch);
	    		pObj->value[0] = atoi(argument);
	    		break;
			case 1:
	    		send_to_char("EXIT FLAGS SET.\n\r\n\r",ch);
				ALT_FLAGVALUE_SET(pObj->value[1],exit_flags,argument);
	    		break;
			case 2:
	    		send_to_char("PORTAL FLAGS SET.\n\r\n\r",ch);
				ALT_FLAGVALUE_SET(pObj->value[2],portal_flags,argument);
	    		break;
			case 3:
	    		send_to_char("EXIT VNUM SET.\n\r\n\r",ch);
	    		pObj->value[3] = atoi(argument);
	    		break;
		   }
		   break;
		case ITEM_FURNITURE:
			switch (value_num){
			default:
				do_help(ch,"ITEM_FURNITURE");
				return false;
			case 0:
				send_to_char("NUMBER OF PEOPLE SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				send_to_char("MAX WEIGHT SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			case 2:
				send_to_char("FURNITURE FLAGS TOGGLED.\n\r\n\r",ch);
			ALT_FLAGVALUE_TOGGLE(pObj->value[2],furniture_flags,argument);
				break;
			case 3:
				send_to_char("HEAL BONUS SET.\n\r\n\r",ch);
				pObj->value[3] = atoi(argument);
				break;
			case 4:
				send_to_char("MANA BONUS SET.\n\r\n\r", ch);
				pObj->value[4] = atoi(argument);
				break;
			}
			break;
        case ITEM_CONTAINER:
			switch (value_num){
			int value;

			default:
				do_help(ch,"ITEM_CONTAINER");
					return false;
			case 0:
				send_to_char("WEIGHT CAPACITY SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				if ((value = flag_value(container_flags,argument)) != NO_FLAG)
					TOGGLE_BIT(pObj->value[1],value);
				else{
					do_help(ch,"ITEM_CONTAINER");
					return false;
				}
				send_to_char("CONTAINER TYPE SET.\n\r\n\r",ch);
				break;
			case 2:
				if (atoi(argument) != 0){
					if (!get_obj_index(atoi(argument))){
						send_to_char("THERE IS NO SUCH ITEM.\n\r\n\r",ch);
						return false;
					}
					if (get_obj_index(atoi(argument))->item_type != ITEM_KEY){
						send_to_char("THAT ITEM IS NOT A KEY.\n\r\n\r",ch);
						return false;
					}
				}
				send_to_char("CONTAINER KEY SET.\n\r\n\r",ch);
				pObj->value[2] = atoi(argument);
				break;
			case 3:
				send_to_char("CONTAINER MAX WEIGHT SET.\n\r",ch);
				pObj->value[3] = atoi(argument);
				break;
			case 4:
				send_to_char("WEIGHT MULTIPLIER SET.\n\r\n\r",ch);
				pObj->value[4] = atoi(argument);
				break;
			}
			break;
        case ITEM_LOCKER:
			switch (value_num){
			int value;

			default:
				do_help(ch,"ITEM_LOCKER");
					return false;
			case 0:
				if(!str_prefix(argument,"true"))
					pObj->value[0] = 1;
				else if(!str_prefix(argument,"false"))
					pObj->value[0] = 0;
				else{
					ch->send("Enter either true or false.\n\r");
					return false;
				}
				ch->send("Locker donor status toggled.\n\r");
				break;
			case 1:
				if ((value = flag_value(container_flags,argument)) != NO_FLAG)
					TOGGLE_BIT(pObj->value[1],value);
				else{
					do_help(ch,"ITEM_CONTAINER");
					return false;
				}
				send_to_char("CONTAINER TYPE SET.\n\r\n\r",ch);
				break;
			case 2:
				if (atoi(argument) != 0){
					if (!get_obj_index(atoi(argument))){
						send_to_char("THERE IS NO SUCH ITEM.\n\r\n\r",ch);
						return false;
					}
					if (get_obj_index(atoi(argument))->item_type != ITEM_KEY){
						send_to_char("THAT ITEM IS NOT A KEY.\n\r\n\r",ch);
						return false;
					}
				}
				send_to_char("CONTAINER KEY SET.\n\r\n\r",ch);
				pObj->value[2] = atoi(argument);
				break;
			case 3:
				send_to_char("CONTAINER MAX CAPACITY SET.\n\r",ch);
				pObj->value[3] = atoi(argument);
				break;
			}
			//NASH for pk= case 4:
			break;
		case ITEM_DRINK_CON:
			switch (value_num){
				default:
					do_help(ch,"ITEM_DRINK");
					return false;
				case 0:
					send_to_char("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r",ch);
					pObj->value[0] = atoi(argument);
					break;
				case 1:
					send_to_char("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r",ch);
					pObj->value[1] = atoi(argument);
					break;
				case 2:
					send_to_char("LIQUID TYPE SET.\n\r\n\r",ch);
					pObj->value[2] = ( liq_lookup(argument) != -1 ? liq_lookup(argument) : 0);
					break;
				case 3:
					send_to_char("POISON VALUE TOGGLED.\n\r\n\r",ch);
					pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
					break;
			}
            break;
		case ITEM_FOUNTAIN:
			switch (value_num){
			default:
				do_help(ch,"ITEM_FOUNTAIN");
				return false;
			case 0:
				send_to_char("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				send_to_char("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			case 2:
				send_to_char("LIQUID TYPE SET.\n\r\n\r",ch);
				pObj->value[2] = (liq_lookup(argument) != -1 ? liq_lookup(argument) : 0);
				break;
			}
			break;
		case ITEM_LOCKPICK:
			switch (value_num){
			default:
				do_help(ch,"ITEM_LOCKPICK");
				return false;
			case 0:
				send_to_char("Picking Power set.\n\r\n\r",ch);
				pObj->value[0] = pick_lookup(argument) != -1 ? pick_lookup(argument) : 2;
				break;
			case 1:
				send_to_char("Number of uses set.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			}
			break;
		case ITEM_FOOD:
			switch (value_num){
			default:
				do_help(ch,"ITEM_FOOD");
				return false;
			case 0:
				send_to_char("HOURS OF FOOD SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				send_to_char("HOURS OF FULL SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			case 3:
				send_to_char("POISON VALUE TOGGLED.\n\r\n\r",ch);
				pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
				break;
			}
			break;
		case ITEM_FORGE:
			switch (value_num){
			default:
				do_help(ch,"ITEM_FORGE");
				return false;
			case 0:
				if ((n = forge_lookup(argument)) < 0){
					ch->send("That is not a valid forge type.\n\rValid types are: Cold, Normal, High, Blast.\n\r");
					return false;
				}
				send_to_char("Heat type set.\n\r\n\r",ch);
				pObj->value[0] = n;
				break;
			case 1:
				n = atoi(argument);
				if (n > 100 || n < 1){
					ch->send("Legal efficiency range: 0 to 100\n\r");
					return false;
				}
				send_to_char("Forge efficiency set.\n\r\n\r",ch);
				pObj->value[1] = n;
				break;
			case 2:
				pObj->value[2] = atoi(argument);
				break;
			}
			break;
		case ITEM_MONEY:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_MONEY");
				return false;
			case 0:
				send_to_char("GOLD AMOUNT SET.\n\r\n\r",ch);
				pObj->value[0] = atoi(argument);
				break;
			case 1:
				send_to_char("SILVER AMOUNT SET.\n\r\n\r",ch);
				pObj->value[1] = atoi(argument);
				break;
			}
            break;
		case ITEM_FORGEFUEL:
			switch (value_num)
			{
			default:
				do_help(ch,"ITEM_FORGE");
				return false;
			case 0:
				n = atoi(argument);
				if (n > 100 || n < 1){
					ch->send("Legal efficiency range: 0 to 100\n\r");
					return false;
				}
				send_to_char("Fuel efficiency set.\n\r\n\r",ch);
				pObj->value[0] = n;
				break;
			}
            break;
    }
    show_obj_values(ch,pObj);
    return true;
}

OEDIT(oedit_show){
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *paf;
	PROG_LIST *list;
	int cnt;

	EDIT_OBJ(ch,pObj);

	ch->send(         "{d+-|{ROedit{d|---------------------------------+{x\n\r");
	printf_to_char(ch,"{dVnum{x:          [ {y%5d {x]\n\r",pObj->vnum);
	printf_to_char(ch,"{dArea{x:          [ {y%5d{x '{c%s{x' ]\n\r",!pObj->area ? -1 : pObj->area->vnum, !pObj->area ? "No Area" : pObj->area->name);
	printf_to_char(ch,"{xLevel:         [ {y%5d {x]\n\r",pObj->level);
	printf_to_char(ch,"{xType:          [ {g%s {x]\n\r",flag_string(type_flags,pObj->item_type));

	if (pObj->item_type == ITEM_ARMOR || pObj->item_type == ITEM_SHIELD)
	printf_to_char(ch,"{xArmorType:     [ {g%s {x]\n\r",armortype_bit_name(pObj->armortype_flags));
	if (pObj->item_type == ITEM_WEAPON)
	printf_to_char(ch,"{xWeaponFlags:   [ {g%s {x]\n\r",weapon_bits_name(pObj->wflags));

	printf_to_char(ch,"{xWear flags:    [ {g%s {x]\n\r",flag_string(wear_flags,pObj->wear_flags));

	printf_to_char(ch,"{xExtra flags:   [ {g%s {x]\n\r",extra_bit_name(pObj->extra_flags));

	//printf_to_char(ch,"{xExclude flags: [ {g%s {x]\n\r",exclude_bit_name(pObj->exclude_flags));

	printf_to_char(ch,"{xMaterial:      [ {c%s {x]\n\r",pObj->material);

	printf_to_char(ch,"{xCondition:     [ {y%5d {x]\n\r",pObj->condition);

	printf_to_char(ch,"{xWeight:        [ {y%5d {gounces {x]\n\r",pObj->weight);
	printf_to_char(ch,"{xCost:          [ {y%5d {x]\n\r",pObj->cost);
	printf_to_char(ch,"{xDroprate:      [ {y(%4d)%4d%% {x]\n\r",pObj->droprate,pObj->droprate * 100 / 1000);
	printf_to_char(ch,"{xTimer:         [ {y%4d {x]\n\r",pObj->timer);
	printf_to_char(ch,"{xPoints:        [ {y%d{w/{y%d {x]\n\r",calc_points(pObj),gen_basepoints(pObj));

	printf_to_char(ch,"{xKeywords:      [ {c%s {x]\n\r",pObj->name);

	printf_to_char(ch,"{xShort desc:\n\r{x   %s{x\n\r",pObj->short_descr);
	printf_to_char(ch,"{xLong desc:\n\r{x   %s{x\n\r",pObj->description);
	printf_to_char(ch,"{xLore:\n\r{x   %s\n\r\n\r",pObj->lore);

	if ( pObj->extra_descr ){
		EXTRA_DESCR_DATA *ed;

		send_to_char("Ex desc kwd: ",ch);

		for ( ed = pObj->extra_descr; ed; ed = ed->next )
			printf_to_char(ch,"[%s]",ed->keyword);

		send_to_char("\n\r",ch);
	}


	for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next ){
		if ( cnt == 0 )
			send_to_char("Number Modifier Affects\n\r------ -------- -------\n\r",ch);
		printf_to_char(ch,"[{c%4d{x] %-8d %s\n\r",cnt,paf->modifier,flag_string(apply_flags,paf->location));
		cnt++;
	}

	show_obj_values(ch,pObj);

	if (pObj->oprogs)
	{
		int cnt;

		printf_to_char(ch,"\n\rOBJPrograms for [{c%5d{x]:\n\r",pObj->vnum);

		for (cnt = 0, list=pObj->oprogs; list; list=list->next)
		{
			if (cnt == 0)
				send_to_char(" Number Vnum Trigger Phrase\n\r ------ ---- ------- ------\n\r",ch);

			printf_to_char(ch,"[{c%5d{x] %4d %7s %s\n\r",cnt,list->vnum,prog_type_to_name(list->trig_type),list->trig_phrase);
			cnt++;
		}
	}
	ch->send(         "{d+-----------------------------------------+{x\n\r");
	return false;
}

bool obj_hasres(OBJ_INDEX_DATA *obj){
	AFFECT_DATA *paf;
	for(paf = obj->affected;paf;paf = paf->next){
		if(paf->where == TO_RESIST)
				return true;
	}
	return false;
}
bool obj_hasvuln(OBJ_INDEX_DATA *obj){
	AFFECT_DATA *paf;
	for(paf = obj->affected;paf;paf = paf->next){
		if(paf->where == TO_VULN)
				return true;
	}
	return false;
}
bool obj_hasimm(OBJ_INDEX_DATA *obj){
	AFFECT_DATA *paf;
	for(paf = obj->affected;paf;paf = paf->next){
		if(paf->where == TO_IMMUNE)
				return true;
	}
	return false;
}
bool obj_hasaff(OBJ_INDEX_DATA *obj){
	AFFECT_DATA *paf;
	for(paf = obj->affected;paf;paf = paf->next){
		if(paf->where == TO_AFFECTS)
				return true;
	}
	return false;
}

bool validate_moddy(CHAR_DATA *ch,OBJ_INDEX_DATA *pObj,int value,int moddy,bool removing){
	bool valid = true;
	double opoints = calc_points(pObj),bpoints = gen_basepoints(pObj);

	if (removing)
		moddy *= -1;

	switch (value){
	case APPLY_HIT:
	case APPLY_MOVE:
	case APPLY_MANA:
		if (opoints + moddy > bpoints)
			valid = false;
		break;
	case APPLY_STR:
	case APPLY_END:
	case APPLY_AGI:
	case APPLY_INT:
	case APPLY_RES:
	case APPLY_FTH:
	case APPLY_WIS:
	case APPLY_CHA:
	case APPLY_LCK:
		if (opoints + moddy*3 > bpoints)
			valid = false;
		break;
	case APPLY_AC:
		if (opoints + moddy*4 > bpoints)
			valid = false;
		break;
	case APPLY_HITROLL:
	case APPLY_DAMROLL:
		if (opoints + moddy*5 > bpoints)
			valid = false;
		break;
	default:
		if(opoints + moddy > bpoints)
			valid = false;
		break;
	}
	if(!valid){
		send_to_char("That would imbalance the object!\n\r",ch);
		if(get_trust(ch) == MAX_LEVEL){
			ch->send("We'll let it slide for the boss.\n\r");
			return true;
		}
		return false;
	}
	return true;
}

OEDIT(oedit_addaffect){
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	char loc[MSL], mod[MSL];

	EDIT_OBJ(ch,pObj);
	int value;

	argument = one_argument(argument,loc);
	one_argument(argument, mod);

	if (loc[0] == '\0' || mod[0] == '\0' || !is_number(mod)){
		send_to_char("Syntax:  addaffect [location] [#xmod]\n\r",ch);
		return false;
	}

	if ((value = flag_value(apply_flags,loc)) == NO_FLAG){
		send_to_char("Valid affects are:\n\r",ch);
		show_help(ch,"apply");
		return false;
	}

	if(value == APPLY_HITROLL || value == APPLY_DAMROLL && pObj->item_type == ITEM_WEAPON){
		/*if(ch->level != MAX_LEVEL){
			ch->send("You do not have rights to do that. Use 'autoweapon' instead.\n\r");
			return false;
		}*/
	}
	if(!validate_moddy(ch,pObj,value,atoi(mod),false))
		return false;

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi(mod);
    pAf->where	    =   TO_OBJECT;
    pAf->type       =   -1;
    pAf->duration   =   -1;
    pAf->bitvector  =   0;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char("Affect added.\n\r",ch);
    return true;
}

OEDIT(oedit_addapply)
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char type[MSL],bvector[MSL];
    int value,bv,typ;

	//send_to_char("No.\n\r",ch);
	//return false;

    EDIT_OBJ(ch,pObj);
    argument = one_argument(argument,type); // res
    //argument = one_argument(argument,loc); // strength
    //argument = one_argument(argument,mod); // 0
    one_argument(argument,bvector); // charm

	//addapply res strength 0 charm
    if (type[0] == '\0' || (typ = flag_value(apply_types,type)) == NO_FLAG)
    {
    	send_to_char("Invalid apply type. Valid apply types are:\n\r",ch);
    	show_help(ch,"apptype");
    	return false;
    }

    /*if (loc[0] == '\0' || ( value = flag_value(apply_flags,loc)) == NO_FLAG)
    {
        send_to_char("Valid applys are:\n\r",ch);
		show_help(ch,"apply");
		return false;
    }*/

    if (bvector[0] == '\0' || (bv = flag_value(bitvector_type[typ].table,bvector)) == NO_FLAG)
    {
    	send_to_char("Invalid bitvector type.\n\rValid bitvector types are:\n\r",ch);
		show_help(ch,bitvector_type[typ].help);
    	return false;
    }

    /*if (mod[0] == '\0' || !is_number(mod))
    {
		send_to_char("Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r",ch);
		return false;
    }*/
	if (apply_types[typ].bit == TO_RESIST && obj_hasres(pObj)){
		ch->send("You may not add another resist to this item.\n\r");
		return false;
	}
	if (apply_types[typ].bit == TO_VULN && obj_hasvuln(pObj)){
		ch->send("You may not add another vuln to this item.\n\r");
		return false;
	}
	if (apply_types[typ].bit == TO_IMMUNE && obj_hasimm(pObj)){
		ch->send("You may not add another immune to this item.\n\r");
		return false;
	}
	if (apply_types[typ].bit == TO_AFFECTS && obj_hasaff(pObj)){
		ch->send("You may not add another affect to this item.\n\r");
		return false;
	}

    pAf             =   new_affect();
    pAf->location   =   APPLY_NONE;
    pAf->modifier   =   0;
    pAf->where	    =   apply_types[typ].bit;
    pAf->type	    =	-1;
    pAf->duration   =   -1;
    pAf->bitvector  =   bv;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char("Apply added.\n\r",ch);
    return true;
}

OEDIT(oedit_autoweapon){
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	int dice, size, bonus;
	double avg;

	EDIT_OBJ(ch, pObj);
	if (pObj->item_type != ITEM_WEAPON && pObj->item_type != ITEM_THROWINGKNIFE && pObj->item_type != ITEM_THROWINGAXE && pObj->item_type != ITEM_THROWINGDART)
	{
		 send_to_char(" {rAutoweapon only works on weapons...{x\n\r", ch);
		return false;
	}
	if (pObj->level < 1)
	{
		send_to_char( " {cAutoweapon requires a level to be set on the weapon first.{x\n\r", ch);
		return false;
	}
   bonus = UMAX(0, pObj->level/10 - 1);
/* adjust this next line to change the avg dmg your weapons will get! */
	avg = (pObj->level * .7);
	dice = (pObj->level/10 + 1);
	size = dice/2;
/* loop through dice sizes until we find that the Next dice size's avg
will be too high... ie, find the "best fit" */
	for (size=dice/2 ; dice * (size +2)/2 < avg ; size++ )
	{ }

	dice = UMAX(1, dice);
	size = UMAX(2, size);

	if (pObj->item_type != ITEM_WEAPON)
	{
		switch (pObj->item_type)
		{
		case ITEM_THROWINGKNIFE:
			dice = UMAX(1,dice);
			size = UMAX(1,size+1);
			break;
		case ITEM_THROWINGDART:
			dice = UMAX(1,dice-1);
			size = UMAX(1,size-2);
			break;
		case ITEM_THROWINGAXE:
			dice = UMAX(1,dice+3);
			size = UMAX(1,size+2);
			break;
		}
	}
	else
	{
		dice = UMAX(1,dice + weapon_table[pObj->value[0]].dice);
		size = UMAX(1,size + weapon_table[pObj->value[0]].dsize);
		dice++;
		size++;
	}
	dice = UMAX(1, dice);
	size = UMAX(2, size);
	
	
	pObj->cost = 25 * (size * (dice + 1)) + 20 * bonus + 20 * pObj->level;
	pObj->value[1] = dice;
	pObj->value[2] = size;
	if (bonus > 0){
		AFFECT_DATA *pAf,*pAf_next;
		char itoa[5];
		int cnt = 0;

		if(pObj->affected){
			for(pAf = pObj->affected;pAf;pAf = pAf_next){
				pAf_next = pAf->next;
				if(pAf->location == APPLY_DAMROLL || pAf->location == APPLY_HITROLL){
					if(pAf->location == APPLY_DAMROLL)
						ch->send("Removing existing damroll.\n\r");
					else if(pAf->location == APPLY_HITROLL)
						ch->send("Removing existing hitroll.\n\r");
					else
						ch->send("Something's screwy with autoweapon.\n\r");
					sprintf(itoa,"%d",cnt);
					oedit_delaffect(ch,itoa);
					cnt--;
				}
				cnt++;
			}
		}
		pAf             =   new_affect();
		pAf->location   =   APPLY_DAMROLL;
		pAf->modifier   =   bonus;
		pAf->where	    =   TO_OBJECT;
		pAf->type       =   -1;
		pAf->duration   =   -1;
		pAf->bitvector  =   0;
		pAf->level      =	pObj->level;
		pAf->next       =   pObj->affected;
		pObj->affected  =   pAf;

		pAf             =   new_affect();
		pAf->location   =   APPLY_HITROLL;
		pAf->modifier   =   bonus;
		pAf->where	    =   TO_OBJECT;
		pAf->type       =   -1;
		pAf->duration   =   -1;
		pAf->bitvector  =   0;
		pAf->level      =	pObj->level;
		pAf->next       =   pObj->affected;
		pObj->affected  =   pAf;
	}
	send_to_char(" {cExperimental values set on weapon...{x\n\r", ch);
	return true;
}

OEDIT(oedit_autoarmor)
{
	OBJ_INDEX_DATA *pObj;
	int size;

	send_to_char("No\n\r",ch);
	return false;

	EDIT_OBJ(ch, pObj);
	if (pObj->item_type != ITEM_ARMOR && pObj->item_type != ITEM_SHIELD)
	{
		 send_to_char( " {rAutoArmor only works on Armor ...{x\n\r", ch);
		return false;
	}
	if (pObj->level < 1)
	{
		send_to_char( " {cAutoArmor requires a level to be set on the armor first.{x\n\r", ch);
		return false;
	}
	size = UMAX(1, pObj->level/2.8 + 1);
	pObj->weight = pObj->level + 1;
	pObj->cost = pObj->level^2 * 2;	
	pObj->value[0] = size;
	pObj->value[1] = size;
	pObj->value[2] = size;
	pObj->value[3] = (size - 1);
	send_to_char( " {cAutoArmor has set experimental values for AC.{x\n\r", ch);
	return true;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT(oedit_delaffect){
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf, *pAf_next;
	char affect[MSL];
	EDIT_OBJ(ch,pObj);
	int value,cnt = 0;

	one_argument(argument,affect);

	if (!is_number(affect) || affect[0] == '\0'){
		send_to_char("Syntax:  delaffect [#xaffect]\n\r",ch);
		return false;
	}

	value = atoi(affect);

	if (value < 0){
		send_to_char("Only non-negative affect-numbers allowed.\n\r",ch);
		return false;
	}

	if (!(pAf = pObj->affected)){
		send_to_char("OEdit:  Non-existant affect.\n\r",ch);
		return false;
	}

	if(value == 0){
		pAf = pObj->affected;
		if(!validate_moddy(ch,pObj,pAf->location,pAf->modifier,true) && ch->level < MAX_LEVEL)
			return false;
		pObj->affected = pAf->next;
		free_affect(pAf);
	}
	else{
		while ((pAf_next = pAf->next) && (++cnt < value))
			pAf = pAf_next;

		if (pAf_next){
			if(!validate_moddy(ch,pObj,pAf_next->location,pAf_next->modifier,true))
				return false;

			pAf->next = pAf_next->next;
			free_affect(pAf_next);
		}
		else{
			 send_to_char("No such affect.\n\r",ch);
			 return false;
		}
	}

	send_to_char("Affect removed.\n\r",ch);
	return true;
}

OEDIT(oedit_keywords)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch,pObj);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  keywords [string]\n\r",ch);
		return false;
    }

    free_string(pObj->name);
    pObj->name = str_dup(argument);

    send_to_char("Keywords set.\n\r",ch);
    return true;
}

OEDIT(oedit_short){
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch,pObj);

	if (!argument[0]){
		send_to_char("Syntax:  short [string]\n\r",ch);
		return false;
	}

	free_string(pObj->short_descr);
	strcat(argument,"{x");
	pObj->short_descr = str_dup(argument);
	pObj->short_descr[0] = pObj->short_descr[0];

	send_to_char("Short description set.\n\r",ch);
	return true;
}

OEDIT(oedit_lore){
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch,pObj);

	if (!argument[0]){
		string_append(ch,&pObj->lore);
		return true;
	}

	send_to_char("Syntax:  lore\n\r",ch);
	return false;
}

OEDIT(oedit_long)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch,pObj);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  long [string]\n\r",ch);
		return false;
    }
        
    free_string(pObj->description);
    pObj->description = str_dup(argument);
    pObj->description[0] = UPPER(pObj->description[0]);

    send_to_char("Long description set.\n\r",ch);
    return true;
}

bool set_value(CHAR_DATA *ch,OBJ_INDEX_DATA *pObj,char *argument,int value)
{
    if (argument[0] == '\0')
    {
		set_obj_values(ch, pObj,-1,"");
		return false;
    }

    if (set_obj_values(ch,pObj,value,argument))
		return true;

    return false;
}

/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values(CHAR_DATA *ch,char *argument,int value)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch,pObj);

    if (set_value(ch,pObj,argument,value))
        return true;

    return false;
}

OEDIT(oedit_value0)
{
    if (oedit_values(ch,argument,0))
        return true;

    return false;
}

OEDIT(oedit_value1)
{
    if (oedit_values(ch,argument,1))
        return true;

    return false;
}

OEDIT(oedit_value2)
{
    if (oedit_values(ch,argument,2))
        return true;

    return false;
}

OEDIT(oedit_value3)
{
    if (oedit_values(ch,argument,3))
        return true;

    return false;
}

OEDIT(oedit_value4)
{
    if (oedit_values(ch,argument,4))
        return true;

    return false;
}

OEDIT(oedit_weight)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch,pObj);

    if (argument[0] == '\0' || !is_number(argument))
    {
		send_to_char("Syntax:  weight #\n\r",ch);
		return false;
    }

    pObj->weight = atoi(argument);

    send_to_char("Weight set.\n\r",ch);
    return true;
}

OEDIT(oedit_cost)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch,pObj);

    if (argument[0] == '\0' || !is_number(argument))
    {
		send_to_char("Syntax:  cost [number]\n\r",ch);
		return false;
    }

    pObj->cost = atoi(argument);

    send_to_char("Cost set.\n\r",ch);
    return true;
}

OEDIT(oedit_create)
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int value, iHash;

    value = atoi(argument);
    if (argument[0] == '\0' || value == 0)
    {
		send_to_char("Syntax:  oedit create [vnum]\n\r",ch);
		return false;
    }

    pArea = get_vnum_area(value);
    if (!pArea)
    {
		send_to_char("OEdit:  That vnum is not assigned an area.\n\r",ch);
		return false;
    }

    if (!IS_BUILDER(ch,pArea))
    {
		send_to_char("OEdit:  Vnum in an area you cannot build in.\n\r",ch);
		return false;
    }

    if (get_obj_index(value))
    {
		send_to_char("OEdit:  Object vnum already exists.\n\r",ch);
		return false;
    }
        
    pObj				= new_obj_index();
    pObj->vnum			= value;
    pObj->area			= pArea;
        
    if (value > top_vnum_obj)
		top_vnum_obj = value;

    iHash					= value % MAX_KEY_HASH;
    pObj->next				= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObj;
    ch->desc->pEdit			= (void *)pObj;

    send_to_char("Object Created.\n\r",ch);
    return true;
}

OEDIT(oedit_ed)
{
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MIL], keyword[MIL];

    EDIT_OBJ(ch,pObj);

    argument = one_argument(argument,command);
    one_argument(argument,keyword);

    if (command[0] == '\0')
    {
		send_to_char("Syntax:  ed add [keyword]\n\r",ch);
		send_to_char("         ed delete [keyword]\n\r",ch);
		send_to_char("         ed edit [keyword]\n\r",ch);
		send_to_char("         ed format [keyword]\n\r",ch);
		return false;
    }

    if (!str_cmp(command,"add"))
    {
		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed add [keyword]\n\r",ch);
			return false;
		}

		ed                  =   new_extra_descr();
		ed->keyword         =   str_dup(keyword);
		ed->next            =   pObj->extra_descr;
		pObj->extra_descr   =   ed;

		string_append(ch,&ed->description);

		return true;
    }

    if (!str_cmp(command,"edit"))
    {
		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed edit [keyword]\n\r",ch);
			return false;
		}

		for ( ed = pObj->extra_descr; ed; ed = ed->next )
			if (is_name(keyword,ed->keyword))
				break;

		if (!ed)
		{
			send_to_char("OEdit:  Extra description keyword not found.\n\r",ch);
			return false;
		}

		string_append(ch,&ed->description);

		return true;
    }

    if (!str_cmp(command,"delete"))
    {
		EXTRA_DESCR_DATA *ped = NULL;

		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed delete [keyword]\n\r",ch);
		    return false;
		}

		for ( ed = pObj->extra_descr; ed; ed = ed->next )
		{
			if (is_name(keyword,ed->keyword))
				break;
			ped = ed;
		}

		if (!ed)
		{
			send_to_char("OEdit:  Extra description keyword not found.\n\r",ch);
			return false;
		}

		if (!ped)
			pObj->extra_descr = ed->next;
		else
			ped->next = ed->next;

		free_extra_descr(ed);

		send_to_char("Extra description deleted.\n\r",ch);
		return true;
    }


    if (!str_cmp(command,"format"))
    {
		EXTRA_DESCR_DATA *ped = NULL;

		if (keyword[0] == '\0')
		{
			send_to_char("Syntax:  ed format [keyword]\n\r",ch);
			return false;
		}

		for ( ed = pObj->extra_descr; ed; ed = ed->next )
		{
			if (is_name(keyword,ed->keyword))
			break;
			ped = ed;
		}

		if (!ed)
		{
			send_to_char("OEdit:  Extra description keyword not found.\n\r",ch);
			return false;
		}

		ed->description = format_string(ed->description);

		send_to_char("Extra description formatted.\n\r",ch);
		return true;
    }

    oedit_ed(ch,"");
    return false;
}

OEDIT(oedit_extra)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0')
    {
		EDIT_OBJ(ch,pObj);

		if ( ( value = flag_value( extra_flags, argument ) ) != NO_FLAG )
		{
			TOGGLE_BIT(pObj->extra_flags, value);

			send_to_char( "Extra flag toggled.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax:  extra [flag]\n\r" "Type '? extra' for a list of flags.\n\r",ch);
    return false;
}

OEDIT(oedit_exclude){
	OBJ_INDEX_DATA *pObj;
	int value;

	if (argument[0] != '\0'){
		EDIT_OBJ(ch,pObj);

		if ( ( value = flag_value( exclude_flags, argument ) ) != NO_FLAG ){
			TOGGLE_BIT(pObj->exclude, value);

			send_to_char( "Exclude flag toggled.\n\r", ch);
			return true;
		}
	}

	send_to_char("Syntax:  exclude [flag]\n\r" "Type '? exclude' for a list of flags.\n\r",ch);
	return false;
}

OEDIT(oedit_wear){
    OBJ_INDEX_DATA *pObj;
    int value,i;
	AFFECT_DATA *paf,*paf_next;

	if (argument[0] != '\0')
	{
		EDIT_OBJ(ch,pObj);

		if ((value = flag_value(wear_flags,argument)) != NO_FLAG){
			TOGGLE_BIT(pObj->wear_flags,value);

			send_to_char("Wear flag toggled.\n\r",ch);
			for(i = 0; i < 4; i++)
				pObj->value[i] = 0;
			for (paf = pObj->affected;paf;paf = paf_next){
				paf_next = paf->next;
				free_affect(paf);
				ch->send("Stats cleared.\n\r");
			}
			pObj->affected = NULL;
			return true;
		}
	}

    send_to_char("Syntax:  wear [flag]\n\r" "Type '? wear' for a list of flags.\n\r",ch);
    return false;
}

OEDIT(oedit_type)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0')
    {
		EDIT_OBJ(ch,pObj);

		if ((value = flag_value(type_flags,argument)) != NO_FLAG)
		{
			pObj->item_type = value;

			ch->send("Type set to: '%s' \n\r",item_name(value));

			if (pObj->item_type == ITEM_ARMOR || pObj->item_type == ITEM_SHIELD)
				pObj->armortype_flags = 1;
			else
				pObj->armortype_flags = 0;
			pObj->value[0] = 0;
			pObj->value[1] = 0;
			pObj->value[2] = 0;
			pObj->value[3] = 0;
			pObj->value[4] = 0;

			return true;
		}
    }

    send_to_char("Syntax:  type [flag]\n\r" "Type '? type' for a list of flags.\n\r",ch);
    return false;
}

OEDIT(oedit_material){
	OBJ_INDEX_DATA *pObj;
	bool found=false;
	int i,cols=0;

	EDIT_OBJ(ch,pObj);

	if (argument[0] == '\0'){
		ch->send("Syntax:  material [string]\n\r");
		return false;
	}

	if (!strcmp(argument,"?")){
		ch->send("Available materials are:\n\r");
		for(i = 1;material_flags[i].name;i++){
			printf_to_char(ch,"%18s",material_flags[i].name);
			if (++cols > 3){
				ch->send("\n\r");
				cols = 0;
			}
		}
		if (cols != 0)
			ch->send("\n\r\n\r");
		else
			ch->send("\n\r");
		return false;
	}

	for(i = 0; material_flags[i].name != NULL && !found; i++)
		if(!str_cmp(argument,material_flags[i].name))
			found = true;

	if(found){
		free_string(pObj->material);
		pObj->material = str_dup(material_flags[i-1].name);
		ch->send("Material set.\n\r");
		return true;
	}
	else{
		ch->send("Incorrect material type, type 'material ?' for a list.\n\r");
		return false;
	}
}

OEDIT(oedit_level)
{
	int i;
	AFFECT_DATA *paf,*paf_next;
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch,pObj);

    if (argument[0] == '\0' || !is_number(argument))
    {
		send_to_char("Syntax:  level [number]\n\r",ch);
		return false;
    }

    pObj->level = atoi(argument);

	for(i = 0; i < 4; i++)
		pObj->value[i] = 0;
	for (paf = pObj->affected;paf;paf = paf_next){
		paf_next = paf->next;
		free_affect(paf);
	}
	pObj->affected = NULL;

    send_to_char("Level set, object stats have been cleared.\n\r",ch);
    return true;
}

OEDIT(oedit_condition)
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if (argument[0] != '\0' && (value = atoi(argument)) >= 0 && (value <= 100))
    {
		EDIT_OBJ(ch,pObj);

		pObj->condition = value;
		send_to_char("Condition set.\n\r",ch);

		return true;
    }

    send_to_char("Syntax:  condition [number]\n\r" "Where number can range from 0 (ruined) to 100 (perfect).\n\r",ch);
    return false;
}

OEDIT(oedit_droprate){
	OBJ_INDEX_DATA *pObj;
	int value;

	if(argument[0] && (value = atoi(argument)) >= 0 && (value <= 1000)){
		EDIT_OBJ(ch,pObj);

		pObj->droprate = value;
		send_to_char("Drop rate set.\n\r",ch);

		return true;
	}

	send_to_char("Syntax:  droprate [number]\n\r" "Where number can range from 0 (never drops) to 1000 (always drops).\n\r",ch);
	return false;
}

OEDIT(oedit_timer){
	OBJ_INDEX_DATA *pObj;
	int value;

	if(argument[0] && (value = atoi(argument)) >= 0){
		EDIT_OBJ(ch,pObj);

		pObj->timer = value;
		send_to_char("Timer rate set.\n\r",ch);

		return true;
	}

	send_to_char("Syntax:  timer [number]\n\r" "Where number can range from 0 (never rots) to infinity.\n\r",ch);
	return false;
}

MEDIT(medit_show){
	int i;
	MOB_INDEX_DATA *pMob;
	PROG_LIST *list;

	EDIT_MOB(ch,pMob);

	send_to_char("\n\r",ch);

	ch->send(         "{d+-|{RMedit{d|---------------------------------+{x\n\r");
	printf_to_char(ch," {dArea{x:        [ {y%d{x '%s'{x ]\n\r",!pMob->area ? -1 : pMob->area->vnum,!pMob->area ? "No Area" : pMob->area->name);
	printf_to_char(ch," {dVnum{x:        [ {y%d{x ]\n\r",pMob->vnum);
	printf_to_char(ch," {xKeywords:    [ {c%s{x ]\n\r",pMob->player_name);
	printf_to_char(ch," {xAct:         [ {g%s{x ]\n\r",act_bits_name(pMob->act));
	printf_to_char(ch," {xClass:       [ {c%s{x ]\n\r",classes[pMob->pclass].name);
	printf_to_char(ch," {xSex:         [ {c%s{x ]\n\r",sex_table[pMob->sex].name);
	printf_to_char(ch," {xRace:        [ {c%s{x ]\n\r", race_table[pMob->race].name);
	printf_to_char(ch," {xLevel:       [ {y%d{x ]\n\r",pMob->level);
	printf_to_char(ch," {xAlign:       [ {y%d{x ]\n\r",pMob->alignment);
	printf_to_char(ch," {xHitroll:     [ {y%d{x ]\n\r",pMob->hitroll);
	printf_to_char(ch," {xDam Type:    [ {c%s{x ]\n\r",attack_table[pMob->dam_type].name);
	if (pMob->group)
	printf_to_char(ch," {xGroup:         [ {c%d]\n\r",pMob->group);
	printf_to_char(ch," {xHit dice:    [ {y%d {xd {y%d {x+ {y%d{x ]\n\r",pMob->hit[DICE_NUMBER],pMob->hit[DICE_TYPE],pMob->hit[DICE_BONUS]);
	printf_to_char(ch," {xDamage dice: [ {y%d {xd {y%d{x+{y%d{x ]\n\r",pMob->damage[DICE_NUMBER],pMob->damage[DICE_TYPE],pMob->damage[DICE_BONUS]);
	printf_to_char(ch," {xMana dice:   [ {y%d {xd {y%d {x+ {y%d{x ]\n\r",pMob->mana[DICE_NUMBER],pMob->mana[DICE_TYPE],pMob->mana[DICE_BONUS]);
	printf_to_char(ch," {xAffected by: [ {g%s{x ]\n\r",affect_bits_name(pMob->affected_by));
	printf_to_char(ch," {xArmor:       [ {xpierce: {y%d  {xbash: {y%d  {xslash: {y%d  {xmagic: {y%d{x ]\n\r",pMob->ac[AC_PIERCE],pMob->ac[AC_BASH],pMob->ac[AC_SLASH],pMob->ac[AC_EXOTIC]);
	printf_to_char(ch," {xStats:       [ ");
	for (i=0;i<MAX_STATS;i++){
		if (!stat_mini[i].name)
			break;
		if (stat_mini[i].settable)
			printf_to_char(ch,"%s {y%2d{x ",stat_mini[i].name,pMob->perm_stat[i]);
	}
	ch->send("]\n\r");
	printf_to_char(ch," {xForm:        [ {g%s{x ]\n\r",form_bits_name(pMob->form));
	printf_to_char(ch," {xParts:       [ {g%s{x ]\n\r",part_bits_name(pMob->parts));
	printf_to_char(ch," {xRes:         [ {g%s{x ]\n\r",res_bits_name(pMob->res));
	printf_to_char(ch," {xOff:         [ {g%s{x ]\n\r",off_bits_name(pMob->off_bits));;
	printf_to_char(ch," {xDef:         [ {g%s{x ]\n\r",def_bits_name(pMob->def_bits));
	printf_to_char(ch," {xSize:        [ {c%s{x ]\n\r",flag_string(size_flags,pMob->size));
	printf_to_char(ch," {xMaterial:    [ {c%s{x ]\n\r",pMob->material);
	printf_to_char(ch," {xStart pos:   [ {c%s{x ]\n\r",flag_string(position_flags,pMob->start_pos));
	printf_to_char(ch," {xDefault pos: [ {c%s{x ]\n\r",flag_string(position_flags,pMob->default_pos));
	printf_to_char(ch," {xWealth:      [ {y%d{x ]\n\r",pMob->wealth);
	if(pMob->guild != 0)
	printf_to_char(ch," {xGuild:       [ {c%s{x ]\n\r",guilds[pMob->guild].name);
	if(pMob->trainer > -1)
	printf_to_char(ch," {xTrainer:     [ {c%s{x ]\n\r",pMob->trainer == -1 ? "Not" : classes[pMob->trainer].name);
	if(pMob->trainer > -1){
	printf_to_char(ch," {xICReadyMsg(icmsg):  [ {c%s{x ]\n\r",pMob->icpmsg);
	printf_to_char(ch," {xOOCReadyMsg(ocmsg): [ {c%s{x ]\n\r",pMob->ocpmsg);
	printf_to_char(ch," {xChangeMsg(cmsg):    [ {c%s{x ]\n\r",pMob->cmsg);
	}

	if (pMob->spec_fun)
	printf_to_char(ch," {xSpec fun:    [ {g%s{x ]\n\r",spec_name(pMob->spec_fun));

	printf_to_char(ch," {xShort description: \n\r   %s\n\r",pMob->short_descr);
	printf_to_char(ch," {xLong description:\n\r   %s",pMob->long_descr);
	printf_to_char(ch," {xDescription:\n\r%s",pMob->description);

	if (pMob->pShop){
		SHOP_DATA *pShop;
		int iTrade;

		pShop = pMob->pShop;

		printf_to_char(ch,"Shop data for [%5d]:\n\r  Markup for purchaser: %d%%\n\r  Markdown for seller:  %d%%\n\r",pShop->keeper,pShop->profit_buy,pShop->profit_sell);
		printf_to_char(ch,"  Hours: %d to %d.\n\r",pShop->open_hour,pShop->close_hour);

		for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
			if (pShop->buy_type[iTrade] != 0){
				if (iTrade == 0)
					send_to_char("  Number Trades Type\n\r  ------ -----------\n\r",ch);
				printf_to_char(ch,"  [ %4d ] %s\n\r",iTrade,flag_string(type_flags,pShop->buy_type[iTrade]));
			}
	}

	if (pMob->mprogs){
		int cnt;

		printf_to_char(ch,"\n\rMOBPrograms for [ %5d ]:\n\r",pMob->vnum);

		for (cnt=0, list=pMob->mprogs; list; list=list->next){
			if (cnt == 0)
				send_to_char(" Number Vnum Trigger Phrase\n\r ------ ---- ------- ------\n\r",ch);

			printf_to_char(ch,"[ %5d ] %4d %7s %s\n\r",cnt,list->vnum,prog_type_to_name(list->trig_type),list->trig_phrase);
			cnt++;
		}
	}
	ch->send(         "{d+-----------------------------------------+{x\n\r");
	return false;
}

MEDIT(medit_create){
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int value,iHash,i;

	value = atoi(argument);
	if (!argument[0] || value == 0){
		send_to_char("Syntax:  medit create [vnum]\n\r",ch);
		return false;
	}

	pArea = get_vnum_area(value);

	if (!pArea){
		ch->send("MEdit:  That vnum is not assigned an area.\n\r");
		return false;
	}

	if (!IS_BUILDER(ch,pArea)){
		ch->send("MEdit:  Vnum in an area you cannot build in.\n\r");
		return false;
	}

	if (get_mob_index(value)){
		ch->send("MEdit:  Mobile vnum already exists.\n\r");
		return false;
	}

	pMob				= new_mob_index();
	pMob->vnum			= value;
	pMob->area			= pArea;
    
	if (value > top_vnum_mob)
		top_vnum_mob = value;

	pMob->setplr(AT_IS_NPC);
	pMob->setact(AT_STAY_AREA);
	for (i = 0;i < MAX_STATS;i++)
		pMob->perm_stat[i] = 10;

    pMob->hit[DICE_NUMBER] = 1;
    pMob->hit[DICE_TYPE]   = 1;
    pMob->hit[DICE_BONUS]  = 1;

	iHash					= value % MAX_KEY_HASH;
	pMob->next				= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMob;
	ch->desc->pEdit			= (void *)pMob;

	ch->send("Mobile Created.\n\r");
	return true;
}

MEDIT(medit_spec)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  spec [special function]\n\r",ch);
		return false;
    }


    if (!str_cmp(argument,"none"))
    {
        pMob->spec_fun = NULL;

        send_to_char("Spec removed.\n\r",ch);
        return true;
    }

    if (spec_lookup(argument))
    {
		pMob->spec_fun = spec_lookup(argument);
		send_to_char("Spec set.\n\r",ch);
		return true;
    }

    send_to_char("MEdit: No such special function.\n\r",ch);
    return false;
}

MEDIT(medit_damtype)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  damtype [damage message]\n\rTo see a list of messages, type '? weapon'.\n\r",ch);
		return false;
    }

    pMob->dam_type = attack_lookup(argument);
    send_to_char("Damage type set.\n\r",ch);
    return true;
}

MEDIT(medit_align)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
		send_to_char("Syntax:  alignment [number]\n\r",ch);
		return false;
    }

    pMob->alignment = atoi(argument);

    send_to_char("Alignment set.\n\r",ch);
    return true;
}

MEDIT(medit_level)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
		send_to_char("Syntax:  level [number]\n\r",ch);
		return false;
    }

    pMob->level = atoi(argument);

    send_to_char("Level set.\n\r",ch);
    return true;
}

MEDIT(medit_guild){
    MOB_INDEX_DATA *pMob;
	int guild = -1;

    EDIT_MOB(ch,pMob);

    if(!argument[0]){
		send_to_char("Syntax:  guild [guild]\n\r",ch);
		return false;
    }

	if(!strcmp(argument,"?"))
		for(int i = 0;i < MAX_GUILD;i++)
			if(!guilds[i].hidden)
			printf_to_char(ch," -{d%s{x\n\r",guilds[i].name);

	if((guild = guild_lookup(argument)) == -1){
		ch->send("What guild?\n\r");
		return false;
	}

    pMob->guild = guild;

    send_to_char("Guild set.\n\r",ch);
    return true;
}

MEDIT(medit_desc)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		string_append(ch,&pMob->description);
		return true;
    }

    send_to_char("Syntax:  desc    - line edit\n\r",ch);
    return false;
}

MEDIT(medit_long)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  long [string]\n\r",ch);
		return false;
    }

    free_string(pMob->long_descr);
    strcat(argument,"{x\n\r");
    pMob->long_descr = str_dup(argument);
    pMob->long_descr[0] = UPPER(pMob->long_descr[0]);

    send_to_char("Long description set.\n\r",ch);
    return true;
}

MEDIT(medit_short)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  short [string]\n\r",ch);
		return false;
    }

    free_string(pMob->short_descr);
    pMob->short_descr = str_dup(argument);

    send_to_char("Short description set.\n\r",ch);
    return true;
}

MEDIT(medit_keywords)
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  keywords [string]\n\r",ch);
		return false;
    }

    free_string(pMob->player_name);
    pMob->player_name = str_dup(argument);

    send_to_char("Keywords set.\n\r",ch);
    return true;
}

MEDIT(medit_shop)
{
    MOB_INDEX_DATA *pMob;
    char command[MIL], arg1[MIL];

    argument = one_argument(argument,command);
    argument = one_argument(argument,arg1);

    EDIT_MOB(ch,pMob);

    if (command[0] == '\0')
    {
		send_to_char("Syntax:  shop hours [#xopening] [#xclosing]\n\r         shop profit [#xbuying%] [#xselling%]\n\r         shop type [#x0-4] [item type]\n\r         shop assign\n\r         shop remove\n\r", ch );
		return false;
    }


    if (!str_cmp(command,"hours"))
    {
		if (arg1[0] == '\0' || !is_number(arg1) || argument[0] == '\0' || !is_number(argument))
		{
			send_to_char("Syntax:  shop hours [#xopening] [#xclosing]\n\r",ch);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Set the mob as a shopkeeper first (shop assign).\n\r",ch);
			return false;
		}

		pMob->pShop->open_hour = atoi(arg1);
		pMob->pShop->close_hour = atoi(argument);

		send_to_char("Shop hours set.\n\r",ch);
		return true;
    }

    if (!str_cmp(command,"profit"))
    {
		if (arg1[0] == '\0' || !is_number(arg1) || argument[0] == '\0' || !is_number(argument))
		{
			send_to_char("Syntax:  shop profit [#xbuying%] [#xselling%]\n\r",ch);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r",ch);
			return false;
		}

		pMob->pShop->profit_buy     = atoi(arg1);
		pMob->pShop->profit_sell    = atoi(argument);

		send_to_char("Shop profit set.\n\r",ch);
		return true;
    }

    if (!str_cmp(command,"type"))
    {
		int value;

		if (arg1[0] == '\0' || !is_number(arg1) || argument[0] == '\0')
		{
			send_to_char("Syntax:  shop type [#x0-4] [item type]\n\r",ch);
			return false;
		}

		if (atoi(arg1) >= MAX_TRADE)
		{
			printf_to_char(ch,"MEdit:  May sell %d items max.\n\r",MAX_TRADE);
			return false;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r",ch);
			return false;
		}

		if ((value = flag_value(type_flags,argument)) == NO_FLAG)
		{
			send_to_char("MEdit:  That type of item is not known.\n\r",ch);
			return false;
		}

		pMob->pShop->buy_type[atoi(arg1)] = value;

		send_to_char("Shop type set.\n\r",ch);
		return true;
    }

    if (!str_prefix(command,"assign"))
    {
    	if (pMob->pShop)
    	{
        	send_to_char("Mob already has a shop assigned to it.\n\r",ch);
        	return false;
		}

		pMob->pShop		= new_shop();
		if (!shop_first)
			shop_first	= pMob->pShop;
		if (shop_last)
			shop_last->next	= pMob->pShop;
		shop_last		= pMob->pShop;

		pMob->pShop->keeper	= pMob->vnum;

		send_to_char("New shop assigned to mobile.\n\r",ch);
		return true;
    }

    if (!str_prefix(command,"remove"))
    {
		SHOP_DATA *pShop;

		pShop		= pMob->pShop;
		pMob->pShop	= NULL;

		if (pShop == shop_first)
		{
			if (!pShop->next)
			{
				shop_first = NULL;
				shop_last = NULL;
			}
			else
				shop_first = pShop->next;
		}
		else
		{
			SHOP_DATA *ipShop;

			for ( ipShop = shop_first; ipShop; ipShop = ipShop->next )
				if (ipShop->next == pShop)
				{
					if (!pShop->next)
					{
						shop_last = ipShop;
						shop_last->next = NULL;
					}
					else
						ipShop->next = pShop->next;
				}
		}

		free_shop(pShop);

		send_to_char("Mobile is no longer a shopkeeper.\n\r",ch);
		return true;
    }

    medit_shop(ch,"");
    return false;
}

MEDIT(medit_sex)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
		EDIT_MOB(ch,pMob);

		if ((value = flag_value(sex_flags,argument)) != NO_FLAG)
		{
			pMob->sex = value;

			send_to_char("Sex set.\n\r",ch);
			return true;
		}
    }

    send_to_char("Syntax: sex [sex]\n\rType '? sex' for a list of flags.\n\r",ch);
    return false;
}

MEDIT(medit_act){
    MOB_INDEX_DATA *pMob;
    int value;

	if (argument[0]){
		EDIT_MOB( ch, pMob );

		if(pMob->toggleacts(argument)){
			send_to_char( "Act flag(s) toggled.\n\r", ch);
			return true;
		}
	}

    send_to_char("Syntax: act [flag]\n\r" "Type '? act' for a list of flags.\n\r",ch);
    return false;
}

MEDIT(medit_affect){
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
		EDIT_MOB(ch,pMob);

		if(pMob->toggleaffs(argument)){
			send_to_char( "Affect flag(s) toggled.\n\r", ch);
			return true;
		}
    }

    send_to_char("Syntax: affect [flag]\n\rType '? affect' for a list of flags.\n\r",ch);
    return false;
}

MEDIT(medit_ac)
{
    MOB_INDEX_DATA *pMob;
    char arg[MIL];
    int pierce, bash, slash, exotic;

    do
    {
		if (argument[0] == '\0')
			break;

		EDIT_MOB(ch,pMob);
		argument = one_argument(argument,arg);

		if (!is_number(arg))
			break;
		pierce = atoi(arg);
		argument = one_argument(argument,arg);

		if (arg[0] != '\0')
		{
			if (!is_number(arg))
				break;
			bash = atoi(arg);
			argument = one_argument(argument,arg);
		}
		else
			bash = pMob->ac[AC_BASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))
				break;
			slash = atoi(arg);
			argument = one_argument(argument,arg);
		}
		else
			slash = pMob->ac[AC_SLASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))
				break;
			exotic = atoi(arg);
		}
		else
			exotic = pMob->ac[AC_EXOTIC];

		pMob->ac[AC_PIERCE] = pierce;
		pMob->ac[AC_BASH]   = bash;
		pMob->ac[AC_SLASH]  = slash;
		pMob->ac[AC_EXOTIC] = exotic;
		
		send_to_char("Ac set.\n\r",ch);
		return true;
    } while (false);

    send_to_char("Syntax:  ac <ac-pierce> <ac-bash> <ac-slash> <ac-exotic>\n\r" "help MOB_AC  gives a list of reasonable ac-values.\n\r",ch);
    return false;
}

MEDIT(medit_form)
{
    MOB_INDEX_DATA *pMob;
    int value;

	EDIT_MOB(ch,pMob);

    if (argument[0] != '\0'){
		if(pMob->toggleforms(argument)){
			send_to_char( "Form flag(s) toggled.\n\r", ch);
			return true;
		}
    }
    printf_to_char(ch,"{dForm flags{x:\n\r{g%s\n\r\n\r",form_bits_name(pMob->form));

    send_to_char("{dSyntax: '{xform [{gflags{x]{d'\n\rType '{x? form{d' for a list of flags{x.\n\r",ch);
    return false;
}

MEDIT(medit_part)
{
    MOB_INDEX_DATA *pMob;
    int value;

	EDIT_MOB(ch,pMob);

    if (argument[0] != '\0')
    {
		if(pMob->toggleparts(argument)){
			send_to_char( "Part flag(s) toggled.\n\r", ch);
			return true;
		}
    }
    printf_to_char(ch,"{dPart flags{x:\n\r{g%s\n\r\n\r",part_bits_name(pMob->parts));

    send_to_char("{dSyntax: '{xpart [{gflags{x]{d'\n\rType '{x? part{d' for a list of flags{x.\n\r",ch);
    return false;
}

MEDIT(medit_res){
	MOB_INDEX_DATA *pMob;
	char arg[MSL];
	int value,i;

	EDIT_MOB(ch,pMob);

	if (argument[0]){
		argument = one_argument(argument,arg);
		if (!is_number(argument) || (value = atoi(argument)) < -50 || value > 300){
			ch->send("Proper syntax is: {Gres {x<{gname{x> <{gpercent{x>.\n\rLegal values for <percent> are -50 to 300");
			return false;
		}
		for (i = 0;i<MAX_RES;i++){
			if (!str_prefix(arg,res_flags[i].name)){
				pMob->res[i] = value;
				printf_to_char(ch,"Resist flag %s set to %d.\n\r",res_flags[i].name,value);
				return true;
			}
		}
		ch->send("Failed.\n\r");

		send_to_char("{dSyntax: '{Gres <{gname{x> <{gpercent{x>{d'\n\rType '{x? res{d' for a list of flags{x.\n\r\n\r",ch);
		return false;
	}
	printf_to_char(ch,"{dResistance flags{x:\n\r%s\n\r\n\r",res_bits_name(pMob->res));

	send_to_char("{dSyntax: '{Gres <{gname{x> <{gpercent{x>{d'\n\rType '{x? res{d' for a list of flags{x.\n\r\n\r",ch);
	return false;
}

MEDIT(medit_material)
{
    MOB_INDEX_DATA *pMob;
	bool found=false;
	int n,cols = 0;

    EDIT_MOB(ch,pMob);

    if (!argument[0]){
		send_to_char("Syntax:  material [string]\n\r",ch);
		return false;
    }

	if (!strcmp(argument,"?")){
		ch->send("Available materials are:\n\r");
		for(n = 1;material_flags[n].name;n++){
			printf_to_char(ch,"%-16s",material_flags[n].name);
			if (++cols > 3){
				ch->send("\n\r");
				cols = 0;
			}
			else
				ch->send(" ");
		}
		if (cols != 0)
			ch->send("\n\r\n\r");
		else
			ch->send("\n\r");
		return false;
	}

	for(n=0;material_flags[n].name && !found; n++)
		if(!str_cmp(argument,material_flags[n].name))
			found = true;

	if(found)
	{
		free_string(pMob->material);
		pMob->material = str_dup(material_flags[n-1].name);
		send_to_char("Material set.\n\r",ch);
		return true;
	}
	else
	{
		send_to_char("Incorrect material, type '? material' for a list.\n\r",ch);
		return false;
	}
}

MEDIT(medit_off)
{
    MOB_INDEX_DATA *pMob;
    int value;

	EDIT_MOB(ch,pMob);

    if (argument[0] != '\0')
    {
		if(pMob->toggleoffs(argument)){
			send_to_char( "Off flag(s) toggled.\n\r", ch);
			return true;
		}
    }
    printf_to_char(ch,"{dOffensive flags{x:\n\r{g%s\n\r\n\r",off_bits_name(pMob->off_bits));

    send_to_char("{dSyntax: '{xoff <{gflags{x>{d'\n\rType '? off{d' for a list of flags{x.\n\r",ch);
    return false;
}

MEDIT(medit_def)
{
    MOB_INDEX_DATA *pMob;
    int value;

	EDIT_MOB(ch,pMob);

    if (argument[0] != '\0')
    {
		if(pMob->toggledefs(argument)){
			send_to_char( "Def flag(s) toggled.\n\r", ch);
			return true;
		}
    }
    printf_to_char(ch,"{dDefensive flags{x:\n\r{g%s\n\r\n\r",def_bits_name(pMob->def_bits));

    send_to_char("{dSyntax: '{xdef <{gflags{x>{d'\n\rType '{x? def{d' for a list of flags{x.\n\r",ch);
    return false;
}

MEDIT(medit_class)
{
    MOB_INDEX_DATA *pMob;
	char arg[MSL];
	int clas = -1;
	bool found=false;

    argument = one_argument(argument,arg);
    if (arg[0] != '\0')
    {
		if ((clas = class_lookup(arg)) > -1)
		{
			found = true;
			EDIT_MOB(ch,pMob);
			pMob->pclass = clas;
		}
		if (found)
		{
			send_to_char("Class set.\n\r",ch);
			return true;
		}
	}
	char buf[MSL];

	strcpy(buf,"Possible classes are: ");
	for ( clas = 0; classes[clas].name != NULL && clas < MAX_CLASS; clas++ )
	{
		if (clas > 0)
			strcat(buf," ");
		strcat(buf,classes[clas].name);
	}
	strcat(buf,".\n\r");

	send_to_char(buf,ch);
	return false;
}

MEDIT(medit_icpmsg){
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch,pMob);

	if (!argument[0]){
		send_to_char("Syntax:  icmsg [string]\n\r",ch);
		return false;
	}

	free_string(pMob->icpmsg);
	pMob->icpmsg = str_dup(argument);

	send_to_char("Trainer IC Prompt Message set.\n\r",ch);
	return true;
}

MEDIT(medit_ocpmsg){
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch,pMob);

	if (!argument[0]){
		send_to_char("Syntax:  ocmsg [string]\n\r",ch);
		return false;
	}

	free_string(pMob->ocpmsg);
	pMob->ocpmsg = str_dup(argument);

	send_to_char("Trainer OOC Prompt Message set.\n\r",ch);
	return true;
}

MEDIT(medit_cmsg){
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch,pMob);

	if (!argument[0]){
		send_to_char("Syntax:  cmsg [string]\n\r",ch);
		return false;
	}

	free_string(pMob->cmsg);
	pMob->cmsg = str_dup(argument);

	send_to_char("Trainer change message set.\n\r",ch);
	return true;
}

MEDIT(medit_trainer){
    MOB_INDEX_DATA *pMob;
	char arg[MSL],buf[MSL];
	int clas = -1;
	bool found=false;

	if(ch->level < 118){
		ch->send("Only the owner can set trainers for now.\n\r");
		return false;
	}
    argument = one_argument(argument,arg);
    if (arg[0]){
		if ((clas = class_lookup(arg)) > -1){
			found = true;
			EDIT_MOB(ch,pMob);
			pMob->trainer = clas;
		}
		else{
			EDIT_MOB(ch,pMob);
			pMob->trainer = -1;
		}
		if (found)
			ch->send("Class set.\n\r");
		else
			ch->send("Trainer status disabled.\n\r");
		return true;
	}

	strcpy(buf,"Possible classes are: ");
	for ( clas = 0; classes[clas].name != NULL && clas < MAX_CLASS; clas++ ){
		if (clas > 0)
			strcat(buf," ");
		strcat(buf,classes[clas].name);
	}
	strcat(buf,".\n\r");

	send_to_char(buf,ch);
	ch->send("Or type any non-class to disable trainer status on this mob.\n\r");
	return false;
}

MEDIT(medit_size)
{
    MOB_INDEX_DATA *pMob;
    int value;

    if (argument[0] != '\0')
    {
		EDIT_MOB(ch,pMob);

		if ((value = flag_value(size_flags,argument)) != NO_FLAG)
		{
			pMob->size = value;
			send_to_char("Size set.\n\r",ch);
			return true;
		}
    }

    send_to_char("Syntax: size [size]\n\rType '? size' for a list of sizes.\n\r",ch);
    return false;
}

MEDIT(medit_hitdice)
{
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  hitdice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    num = cp = argument;

    while (isdigit(*cp))
		++cp;
    while (*cp != '\0' && !isdigit(*cp))
		*(cp++) = '\0';

    type = cp;

    while (isdigit(*cp))
		++cp;
    while (*cp != '\0' && !isdigit(*cp))
		*(cp++) = '\0';

    bonus = cp;

    while (isdigit(*cp))
		++cp;
    if (*cp != '\0')
		*cp = '\0';

    if ((!is_number(num) || atoi(num) < 1 ) || (!is_number(type) || atoi(type) < 1 ) || (!is_number(bonus) || atoi(bonus) < 0 ) )
    {
		send_to_char("Syntax:  hitdice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    pMob->hit[DICE_NUMBER] = atoi(num);
    pMob->hit[DICE_TYPE]   = atoi(type);
    pMob->hit[DICE_BONUS]  = atoi(bonus);

    send_to_char("Hitdice set.\n\r",ch);
    return true;
}

MEDIT(medit_manadice)
{
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  manadice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    num = cp = argument;

    while (isdigit(*cp))
		++cp;
    while (*cp != '\0' && !isdigit(*cp))
		*(cp++) = '\0';

    type = cp;

    while (isdigit(*cp))
		++cp;
    while (*cp != '\0' && !isdigit(*cp))
		*(cp++) = '\0';

    bonus = cp;

    while (isdigit(*cp))
		++cp;
    if (*cp != '\0')
		*cp = '\0';

    if (!(is_number(num) && is_number(type) && is_number(bonus)))
    {
		send_to_char("Syntax:  manadice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    if ((!is_number(num) || atoi(num) < 1) || (!is_number(type) || atoi(type) < 1) || (!is_number(bonus) || atoi(bonus) < 0))
    {
		send_to_char("Syntax:  manadice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    pMob->mana[DICE_NUMBER] = atoi(num);
    pMob->mana[DICE_TYPE]   = atoi(type);
    pMob->mana[DICE_BONUS]  = atoi(bonus);

    send_to_char("Manadice set.\n\r",ch);
    return true;
}

MEDIT( medit_damdice )
{
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  damdice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    num = cp = argument;

    while (isdigit(*cp))
		++cp;
    while (*cp != '\0' && !isdigit(*cp))
		*(cp++) = '\0';

    type = cp;

    while (isdigit(*cp))
		++cp;
    while (*cp != '\0' && !isdigit(*cp))
		*(cp++) = '\0';

    bonus = cp;

    while (isdigit(*cp))
		++cp;
    if (*cp != '\0')
		*cp = '\0';

    if (!(is_number(num) && is_number(type) && is_number(bonus)))
    {
		send_to_char("Syntax:  damdice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    if ((!is_number(num) || atoi(num) < 1) || (!is_number(type) || atoi(type) < 1) || (!is_number(bonus) || atoi(bonus) < 0))
    {
		send_to_char("Syntax:  damdice <number> d <type> + <bonus>\n\r",ch);
		return false;
    }

    pMob->damage[DICE_NUMBER] = atoi(num);
    pMob->damage[DICE_TYPE]   = atoi(type);
    pMob->damage[DICE_BONUS]  = atoi(bonus);

    send_to_char("Damdice set.\n\r",ch);
    return true;
}

MEDIT(medit_race){
	MOB_INDEX_DATA *pMob;
	int race;
	char arg[100],*buf;
	int i,n,val;
	bool found;

	if (argument[0] != '\0' && (race = race_lookup(argument)) > 0){
		EDIT_MOB(ch,pMob);

		pMob->race = race;
		pMob->massaff(race_table[race].aff);
		pMob->massact(race_table[race].act);
		pMob->massform(race_table[race].form);
		pMob->masspart(race_table[race].part);
		for (i = 0;i < MAX_RES;i++)
			pMob->res[i] = 100;

		for (i = 0;i<MAX_RES;i++){
			if (race_table[race].res[i]){
				buf = str_dup(race_table[race].res[i]);
				buf = one_argument(buf,arg);
				if(!strcmp(arg,"none"))
					break;
				if(!is_number(buf)){
					wiznet("bad medit_race, check logfiles now",NULL,NULL,WZ_FLAGS,0,0);
					log_f("medit_race: icky value  %s '%s'",race_table[race].name,race_table[race].res[i]);
					return false;
				}
				val = atoi(buf);
				found = false;
				for (n = 0;n<MAX_RES;n++){
					if(!strcmp(res_flags[n].name,arg)){
						pMob->res[n] = val;
						found = true;
					}
				}
				if (!found){
					wiznet("bad medit_race2, check logfiles now",NULL,NULL,WZ_FLAGS,0,0);
					log_f("medit_race2: icky res  %s '%s'",race_table[race].name,race_table[race].res[i]);
					return false;
				}
			}
			else
				break;
		}
		send_to_char("Race set.\n\r",ch);
		return true;
	}

	if (argument[0] == '?'){
		send_to_char("Available races are:",ch);

		for (race = 0;race_table[race].name;race++){
			if ((race % 3) == 0)
				send_to_char("\n\r",ch);
			printf_to_char(ch," %-15s",race_table[race].name);
		}

		send_to_char("\n\r",ch);
		return false;
	}

	send_to_char("Syntax:  race [race]\n\rType 'race ?' for a list of races.\n\r",ch);
	return false;
}

MEDIT(medit_stat)
{
    MOB_INDEX_DATA *pMob;
    char arg[MIL];
    int value,i=0,stat=-1,cols=0;

    EDIT_MOB(ch,pMob);

    argument = one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Mob stats are:\n\r");
		for (i=0;i<MAX_STATS;i++){
			if (stat_flags[i].settable){
				printf_to_char(ch," %-15s: {g%2d{x",stat_flags[i].name,pMob->perm_stat[i]);
				if (++cols == 2){
					ch->send("\n\r");
					cols = 0;
				}
				else
					ch->send("     ");
			}
		}
		ch->send("\n\r");
		return false;
	}
	for (i=0;stat_flags[i].name;i++){
		if (!str_prefix(arg,stat_flags[i].name)){
			stat = stat_flags[i].bit;
			break;
		}
	}

	if (stat < 0)
		send_to_char("Syntax:  position [start/default] [position]\n\rType '? position' for a list of positions.\n\r",ch);
	else{
		if (is_number(argument)){
			if ((value = atoi(argument)) > 50 || value < 1)
				ch->send("Legal range is 1 to 50.\n\r");
			else{
				pMob->perm_stat[stat] = atoi(argument);
				printf_to_char(ch,"Modifying mob's %s to %d.\n\r",stat_flags[i].name,pMob->perm_stat[stat]);
				return true;
			}
		}
		else
			ch->send("You can only set stats to a number!\n\r");
	}
    return false;
}

MEDIT(medit_position)
{
    MOB_INDEX_DATA *pMob;
    char arg[MIL];
    int value;

    argument = one_argument(argument,arg);

	if (!str_prefix(arg,"start"))
	{
		if ((value = flag_value(position_flags,argument)) == NO_FLAG)
			return false;
		EDIT_MOB(ch,pMob);
		pMob->start_pos = value;
		send_to_char("Start position set.\n\r",ch);
		return true;
	}
	if (!str_prefix(arg,"default"))
	{
		if ((value = flag_value(position_flags,argument)) == NO_FLAG)
			return false;
		EDIT_MOB(ch,pMob);
		pMob->default_pos = value;
		send_to_char("Default position set.\n\r",ch);
		return true;
    }

    send_to_char("Syntax:  position [start/default] [position]\n\rType '? position' for a list of positions.\n\r",ch);
    return false;
}

MEDIT(medit_gold)
{
    MOB_INDEX_DATA *pMob;
    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
		send_to_char("Syntax:  wealth [number]\n\r",ch);
		return false;
    }
    pMob->wealth = atoi(argument);
    send_to_char("Wealth set.\n\r",ch);
    return true;
}

MEDIT(medit_hitroll)
{
    MOB_INDEX_DATA *pMob;
    EDIT_MOB(ch,pMob);

    if (argument[0] == '\0' || !is_number(argument))
    {
		send_to_char("Syntax:  hitroll [number]\n\r",ch);
		return false;
    }

    pMob->hitroll = atoi(argument);
    send_to_char("Hitroll set.\n\r",ch);
    return true;
}

void show_liqlist(CHAR_DATA *ch){
    int liq;
    BUFFER *buffer;
    char buf[MSL];

    buffer = new_buf();

    for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
		if ((liq % 21) == 0)
			add_buf(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");

		sprintf(buf,"%-20s %-14s %5d %4d %6d %4d %5d\n\r",
			liq_table[liq].liq_name,liq_table[liq].liq_color,
			liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
			liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
			liq_table[liq].liq_affect[4]);
		add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
	return;
}

void show_damlist(CHAR_DATA *ch)
{
    int att;
    BUFFER *buffer;
    char buf[MSL];

    buffer = new_buf();

    for ( att = 0; attack_table[att].name != NULL; att++)
    {
		if ((att % 21) == 0)
			add_buf(buffer,"Name                 Noun\n\r");

		sprintf(buf,"%-20s %-20s\n\r",attack_table[att].name,attack_table[att].noun);
		add_buf(buffer,buf);
    }

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
	return;
}

MEDIT(medit_group)
{
    MOB_INDEX_DATA *pMob, *pMTemp;
    char arg[MSL], buf[MSL];
    int temp;
    BUFFER *buffer;
    bool found = false;
    
    EDIT_MOB(ch,pMob);
    
    if (argument[0] == '\0')
    {
    	send_to_char("Syntax: group [number]\n\r        group show [number]\n\r",ch);
    	return false;
    }
    
    if (is_number(argument))
    {
		pMob->group = atoi(argument);
    	send_to_char("Group set.\n\r",ch);
		return true;
    }
    
    argument = one_argument(argument,arg);
    
    if (!strcmp(arg,"show") && is_number(argument))
    {
		if (atoi(argument) == 0)
		{
			send_to_char("Are you crazy?\n\r",ch);
			return false;
		}

		buffer = new_buf ();

    	for (temp = 0; temp < 65536; temp++)
    	{
    		pMTemp = get_mob_index(temp);
    		if (pMTemp && (pMTemp->group == atoi(argument)))
    		{
				found = true;
    			sprintf(buf,"[%5d] %s\n\r",pMTemp->vnum,pMTemp->player_name);
				add_buf(buffer,buf);
    		}
    	}

		if (found)
			page_to_char(buf_string(buffer),ch);
		else
			send_to_char("No mobs in that group.\n\r",ch);

		free_buf(buffer);
        return false;
    }
    
    return false;
}

REDIT(redit_owner)
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM(ch,pRoom);

    if (argument[0] == '\0')
    {
		send_to_char("Syntax:  owner [owner]\n\r         owner none\n\r",ch);
		return false;
    }

    free_string(pRoom->owner);
    if (!str_cmp(argument,"none"))
    	pRoom->owner = str_dup("");
    else
		pRoom->owner = str_dup(argument);

    send_to_char("Owner set.\n\r",ch);
    return true;
}

MEDIT(medit_addmprog)
{
	int value;
	MOB_INDEX_DATA *pMob;
	PROG_LIST *list;
	PROG_CODE *code;
	char trigger[MSL], phrase[MSL], num[MSL];

	EDIT_MOB(ch,pMob);
	argument=one_argument(argument,num);
	argument=one_argument(argument,trigger);
	argument=one_argument(argument,phrase);

	if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0')
	{
		send_to_char("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r",ch);
		return false;
	}

	if ((value = flag_value(mprog_flags,trigger)) == NO_FLAG)
	{
		send_to_char("Valid flags are:\n\r",ch);
		show_help(ch,"mprog");
		return false;
	}

	if ((code =get_prog_index(atoi(num),PRG_MPROG)) == NULL)
	{
		send_to_char("No such MOBProgram.\n\r",ch);
		return false;
	}

	list                  = new_mprog();
	list->vnum            = atoi(num);
	list->trig_type       = value;
	list->trig_phrase     = str_dup(phrase);
	list->code            = code->code;
	SET_BIT(pMob->mprog_flags,value);
	list->next            = pMob->mprogs;
	pMob->mprogs          = list;

	send_to_char("Mprog Added.\n\r",ch);
	return true;
}

MEDIT(medit_delmprog)
{
	MOB_INDEX_DATA *pMob;
	PROG_LIST *list, *list_next;
	char mprog[MSL];
	int value, cnt = 0;

	EDIT_MOB(ch,pMob);

	one_argument(argument,mprog);
	if (!is_number(mprog) || mprog[0] == '\0')
	{
	   send_to_char("Syntax:  delmprog [#mprog]\n\r",ch);
	   return false;
	}

	value = atoi(mprog);

	if (value < 0)
	{
		send_to_char("Only non-negative mprog-numbers allowed.\n\r",ch);
		return false;
	}

	if (!(list= pMob->mprogs))
	{
		send_to_char("MEdit:  Non existant mprog.\n\r",ch);
		return false;
	}

	if (value == 0)
	{
		REMOVE_BIT(pMob->mprog_flags,pMob->mprogs->trig_type);
		list = pMob->mprogs;
		pMob->mprogs = list->next;
		free_mprog(list);
	}
	else
	{
		while ((list_next = list->next) && (++cnt < value))
			list = list_next;

		if (list_next)
		{
			REMOVE_BIT(pMob->mprog_flags,list_next->trig_type);
			list->next = list_next->next;
			free_mprog(list_next);
		}
		else
		{
			send_to_char("No such mprog.\n\r",ch);
			return false;
		}
	}

	send_to_char("Mprog removed.\n\r", ch);
	return true;
}

REDIT(redit_room)
{
	ROOM_INDEX_DATA *room;
	int value;

	EDIT_ROOM(ch,room);

	if ((value = flag_value(room_flags,argument)) == NO_FLAG)
	{
		send_to_char("Syntax: room [flags]\n\r",ch);
		return false;
	}

	TOGGLE_BIT(room->room_flags, value);
	send_to_char("Room flags toggled.\n\r",ch);
	return true;
}

REDIT(redit_reset_list){
    ROOM_INDEX_DATA	*pRoomIndex;
    AREA_DATA *pArea;
    BUFFER *buf1;
    char buf[MSL], arg[MIL];
    bool found;
    int vnum, col = 0;

    one_argument(argument,arg);

    pArea = ch->in_room->area;
    buf1=new_buf();
/*    buf1[0] = '\0'; */
    found   = false;

    for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		if ((pRoomIndex = get_room_index(vnum)) && pRoomIndex->reset_first){
			found = true;
			sprintf(buf,"[%5d] %-17.16s",vnum,capitalize(pRoomIndex->name));
			add_buf(buf1,buf);
			if (++col % 3 == 0)
				add_buf(buf1,"\n\r");
		}

    if (!found){
		send_to_char("No resets found in this area.\n\r",ch);
		return false;
    }

    if (col % 3 != 0)
		add_buf(buf1,"\n\r");

    page_to_char(buf_string(buf1),ch);
    free_buf(buf1);
    return false;

}

REDIT(redit_sector)
{
	ROOM_INDEX_DATA *room;
	int value;

	EDIT_ROOM(ch,room);

	if ((value = flag_value(sector_flags,argument)) == NO_FLAG)
	{
		send_to_char("Syntax: sector [flag]\n\r",ch);
		return false;
	}

	room->sector_type = value;
	send_to_char("Sector type set.\n\r",ch);

	return true;
}

MEDIT(medit_autoset)
{
	MOB_INDEX_DATA *pMob;
	char temp[500];
	int dice, size, bonus, ac_n, ac_x;
	
	EDIT_MOB(ch,pMob);

	if (pMob->level < 1)
	{
		send_to_char("Set a level on the mob first!!!\n\r", ch);
		return false;
	}
	/* adjust these next 2 lines to affect ACs */
	ac_n = 90 - (pMob->level -1);
	ac_x = 90 - (pMob->level -1);
	pMob->ac[AC_PIERCE] = ac_n;
	pMob->ac[AC_BASH]   = ac_n;
	pMob->ac[AC_SLASH]  = ac_n;
	pMob->ac[AC_EXOTIC] = ac_x;
	send_to_char("AC Values set.\n\r", ch);
	
	dice = pMob->level;
	size = pMob->level*2;
	bonus = pMob->level*2;
	sprintf(temp,"%dd%d + %d",dice,size,bonus);
	medit_manadice(ch,temp);

	dice = pMob->level/2;
	size = (pMob->level/2);
	bonus = (pMob->level/3)+1;
	//dice = pMob->level/3;
	if (dice < 1)
		dice = 1;
	//size = (.87 + pMob->level/dice);
	if (size < 1)
		size = 1;
	//bonus = (5.5 + pMob->level/2);
	sprintf(temp, "%dd%d + %d", dice, size, bonus);
	medit_damdice(ch,temp);
	
	dice = pMob->level + 2;
	size = pMob->level * 6;
	bonus = (pMob->level*6)+1;
	if (size < 1)
		size = 1;
	//dice = (pMob->level + .5) * 2 + .2;
	if (dice < 1)
		dice = 1;
	//sprintf(temp, "%dd%d + %d", dice, size, bonus);
    pMob->hit[DICE_NUMBER] = dice;
    pMob->hit[DICE_TYPE]   = size;
    pMob->hit[DICE_BONUS]  = bonus;
	//medit_hitdice(ch, temp);
	
	dice = pMob->level + 8;
	sprintf(temp,"%d",dice);
	medit_hitroll(ch,temp);
	send_to_char(" Values set, check for accuracy.\n\r",ch);

	return true;
}

MEDIT(medit_autohard)
{
	MOB_INDEX_DATA *pMob;
	char temp[500];
	int dice, size, bonus, ac_n, ac_x;

	EDIT_MOB(ch,pMob);
	if (pMob->level < 1)
	{
		send_to_char("Set a level on the mob first!!!\n\r",ch);
		return false;
	}
	ac_n = 88 - (pMob->level * 7.05) - ((pMob->level/10)^2);
	ac_x = 88 - (pMob->level * 5.02) - ((pMob->level/10)^2);
// sprintf(temp, "%d %d %d %d", ac_n, ac_n, ac_n, ac_x);
	pMob->ac[AC_PIERCE] = ac_n;
	pMob->ac[AC_BASH]   = ac_n;
	pMob->ac[AC_SLASH]  = ac_n;
	pMob->ac[AC_EXOTIC] = ac_x;
	send_to_char("AC Values set.\n\r",ch);
	dice = pMob->level;
	size = 11;
	bonus = 95;
	sprintf(temp,"%dd%d + %d",dice,size,bonus);
	medit_manadice(ch,temp);
	dice = pMob->level/3;
	if (dice < 1)
		dice = 1;
	dice++;
	size = (.87 + pMob->level/dice);
	if (size < 2)
		size = 2;
	bonus = (7.5 + pMob->level/1.5);
	sprintf(temp,"%dd%d + %d",dice,size,bonus);
	medit_damdice(ch,temp);
	bonus = (pMob->level/53 + 1) * ((pMob->level * 10) + (pMob->level/10)) +1; 
	size = pMob->level/3;
	if (size < 2)
		size = 2;
	size++;
	dice = (pMob->level + .6) * 2.05 + .5;
	if (dice < 1)
		dice = 1;
	sprintf(temp,"%dD%d + %d",dice,size,bonus);
	medit_hitdice(ch,temp);
	sprintf(temp,"%d",pMob->level);
	medit_hitroll(ch,temp);

	send_to_char("Hard values set, check for accuracy.\n\r",ch);
	return true;
}

MEDIT(medit_autoeasy)
{
	MOB_INDEX_DATA *pMob;
	char temp[500];
	int dice, size, bonus, ac_n, ac_x;

	EDIT_MOB(ch,pMob);
	if (pMob->level < 1)
	{
		send_to_char("Set a level on the mob first!!!\n\r",ch);
		return false;
	}
	ac_n = 99 - (pMob->level * 6.37) - ((pMob->level/10)^2);
	ac_x = 99 - (pMob->level * 4.27) - ((pMob->level/10)^2);
	// sprintf(temp, "%d %d %d %d", ac_n, ac_n, ac_n, ac_x);
	pMob->ac[AC_PIERCE] = ac_n;
	pMob->ac[AC_BASH]   = ac_n;
	pMob->ac[AC_SLASH]  = ac_n;
	pMob->ac[AC_EXOTIC] = ac_x;
	send_to_char("AC Values set.\n\r",ch);
	dice = pMob->level;
	size = 9;
	bonus = 60;
	sprintf(temp,"%dd%d + %d",dice,size,bonus);
	medit_manadice(ch,temp);
	dice = pMob->level/3 * .95;
	if (dice < 1)
		dice = 1;
	size = (.87 + pMob->level/dice) * .95;
	if (size < 2)
		size = 2;
	bonus = (2.5 + pMob->level/2.1);
	sprintf(temp,"%dd%d + %d",dice,size,bonus);
	medit_damdice(ch,temp);
	bonus = (pMob->level/59 + 1) * ((pMob->level * 9) + (pMob->level/11)) +1; 
	size = pMob->level/3;
	if (size < 2)
		size = 2;
	dice = (pMob->level + .5) * 2 + .2;
	if (dice < 1)
		dice = 1;
	sprintf(temp,"%dD%d + %d",dice,size,bonus);
	medit_hitdice(ch,temp);
	sprintf(temp,"%d",pMob->level);
	medit_hitroll(ch,temp);
	send_to_char("Easy values set, check for accuracy.\n\r",ch);

	return true;
}

OEDIT(oedit_addoprog)
{
	int value;
	OBJ_INDEX_DATA *pObj;
	PROG_LIST *list;
	PROG_CODE *code;
	char trigger[MSL], phrase[MSL], num[MSL];

	EDIT_OBJ(ch,pObj);
	argument = one_argument(argument,num);
	argument = one_argument(argument,trigger);
	argument = one_argument(argument,phrase);

	if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0')
	{
		send_to_char("Syntax:   addoprog [vnum] [trigger] [phrase]\n\r",ch);
		return false;
	}

	if ((value = flag_value(oprog_flags,trigger)) == NO_FLAG)
	{
		send_to_char("Valid flags are:\n\r",ch);
		show_help(ch,"oprog");
		return false;
	}

	if ((code = get_prog_index(atoi(num),PRG_OPROG)) == NULL)
	{
		send_to_char("No such OBJProgram.\n\r",ch);
		return false;
	}

	list                  = new_oprog();
	list->vnum            = atoi(num);
	list->trig_type       = value;
	list->trig_phrase     = str_dup(phrase);
	list->code            = code->code;
	SET_BIT(pObj->oprog_flags,value);
	list->next            = pObj->oprogs;
	pObj->oprogs          = list;

	send_to_char("Oprog Added.\n\r",ch);
	return true;
}

OEDIT(oedit_deloprog)
{
    OBJ_INDEX_DATA *pObj;
    PROG_LIST *list, *list_next;
    char oprog[MSL];
    int value, cnt = 0;

    EDIT_OBJ(ch,pObj);

    one_argument(argument,oprog);
    if (!is_number(oprog) || oprog[0] == '\0')
    {
       send_to_char("Syntax:  deloprog [#oprog]\n\r",ch);
       return false;
    }

    value = atoi(oprog);

    if (value < 0)
    {
        send_to_char("Only non-negative oprog-numbers allowed.\n\r",ch);
        return false;
    }

    if (!(list= pObj->oprogs))
    {
        send_to_char("OEdit:  Non existant oprog.\n\r",ch);
        return false;
    }

    if (value == 0)
    {
		REMOVE_BIT(pObj->oprog_flags, pObj->oprogs->trig_type);
        list = pObj->oprogs;
        pObj->oprogs = list->next;
        free_oprog(list);
    }
    else
    {
        while ((list_next = list->next) && (++cnt < value))
			list = list_next;

        if (list_next)
        {
			REMOVE_BIT(pObj->oprog_flags, list_next->trig_type);
			list->next = list_next->next;
			free_oprog(list_next);
        }
        else
        {
			send_to_char("No such oprog.\n\r",ch);
			return false;
        }
    }

    send_to_char("Oprog removed.\n\r",ch);
    return true;
}

REDIT(redit_addrprog)
{
	int value;
	ROOM_INDEX_DATA *pRoom;
	PROG_LIST *list;
	PROG_CODE *code;
	char trigger[MSL], phrase[MSL], num[MSL];

	EDIT_ROOM(ch,pRoom);
	argument = one_argument(argument,num);
	argument = one_argument(argument,trigger);
	argument = one_argument(argument,phrase);

	if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0')
	{
		send_to_char("Syntax:   addrprog [vnum] [trigger] [phrase]\n\r",ch);
		return false;
	}

	if ((value = flag_value (rprog_flags,trigger)) == NO_FLAG)
	{
        send_to_char("Valid flags are:\n\r",ch);
        show_help(ch,"rprog");
        return false;
	}

	if ((code = get_prog_index(atoi(num),PRG_RPROG)) == NULL)
	{
        send_to_char("No such ROOMProgram.\n\r",ch);
        return false;
	}

	list                  = new_rprog();
	list->vnum            = atoi(num);
	list->trig_type       = value;
	list->trig_phrase     = str_dup(phrase);
	list->code            = code->code;
	SET_BIT(pRoom->rprog_flags,value);
	list->next            = pRoom->rprogs;
	pRoom->rprogs          = list;

	send_to_char( "Rprog Added.\n\r",ch);
	return true;
}

REDIT(redit_delrprog)
{
    ROOM_INDEX_DATA *pRoom;
    PROG_LIST *list, *list_next;
    char rprog[MSL];
    int value, cnt = 0;

    EDIT_ROOM(ch,pRoom);

    one_argument(argument,rprog);
    if (!is_number(rprog) || rprog[0] == '\0')
    {
       send_to_char("Syntax:  delrprog [#rprog]\n\r",ch);
       return false;
    }

    value = atoi(rprog);

    if (value < 0)
    {
        send_to_char("Only non-negative rprog-numbers allowed.\n\r",ch);
        return false;
    }

    if (!(list= pRoom->rprogs))
    {
        send_to_char("REdit:  Non existant rprog.\n\r",ch);
        return false;
    }

    if (value == 0)
    {
		REMOVE_BIT(pRoom->rprog_flags, pRoom->rprogs->trig_type);
        list = pRoom->rprogs;
        pRoom->rprogs = list->next;
        free_rprog(list);
    }
    else
    {
        while ((list_next = list->next) && (++cnt < value))
			list = list_next;

        if (list_next)
        {
			REMOVE_BIT(pRoom->rprog_flags, list_next->trig_type);
			list->next = list_next->next;
			free_rprog(list_next);
        }
        else
        {
			send_to_char("No such rprog.\n\r",ch);
			return false;
        }
    }

    send_to_char("Rprog removed.\n\r",ch);
    return true;
}

OEDIT(oedit_armortype)
{
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *paf,*paf_next;
	int value,i;

	if (argument[0] != '\0'){
		EDIT_OBJ(ch,pObj);

		if ((value = flag_value(armortype_flags,argument)) > 0)
		{
			pObj->armortype_flags = value;
			pObj->value[0] = 0;
			pObj->value[1] = 0;
			pObj->value[2] = 0;
			pObj->value[3] = 0;

			for(i = 0; i < 4; i++)
				pObj->value[i] = 0;
			for (paf = pObj->affected;paf;paf = paf_next){
				paf_next = paf->next;
				free_affect(paf);
			}
			pObj->affected = NULL;
			send_to_char("ArmorType flag toggled, stats cleared.\n\r",ch);
			return true;
		}
	}

	send_to_char("Syntax:  armortype [flag]\n\r""Type '? armortype' for a list of flags.\n\r",ch);
	return false;
}

OEDIT(oedit_weapon){
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *paf,*paf_next;
	int value,i;

	if (argument[0]){
		EDIT_OBJ(ch,pObj);

		if ((value = flag_value(weapon_type2,argument)) != NO_FLAG){
				pObj->wflags[value] = 1 - pObj->wflags[value];
			pObj->affected = NULL;
			ch->send("WeaponFlag toggled.\n\r");
			return true;
		}
	}

	ch->send("Syntax:  weapon [flag]\n\r""Type '? wtype' for a list of flags.\n\r");
	return false;
}

OEDIT(oedit_delete){
	CHAR_DATA *wch, *ch_next;
	OBJ_DATA *obj, *obj_next;
	OBJ_INDEX_DATA *pObj;
	RESET_DATA *pReset, *wReset, *tReset;
	ROOM_INDEX_DATA *pRoom;
	char arg[MIL];
	char buf[MSL];
	int index, rcount, ocount, i, iHash;

	if ( argument[0] == '\0' ){
		send_to_char( "Syntax:  oedit delete [vnum]\n\r", ch );
		return false;
	}

	one_argument( argument, arg );

	if( is_number( arg ) )
	{
		index = atoi( arg );
		pObj = get_obj_index( index );
	}
	else
	{
		send_to_char( "That is not a number.\n\r", ch );
		return false;
	}

	if( !pObj )
	{
		send_to_char( "No such object.\n\r", ch );
		return false;
	}


	if (!IS_BUILDER(ch,pObj->area))
	{
		ch->send("You don't have access rights to that vnum.\n\r");
		return false;
	}
	SET_BIT( pObj->area->area_flags, AREA_CHANGED );

	if( top_vnum_obj == index )
		for( i = 1; i < index; i++ )
			if( get_obj_index( i ) )
				top_vnum_obj = i;


	top_obj_index--;

	/* remove objects */
	ocount = 0;
	for( obj = object_list; obj; obj = obj_next )
	{
		obj_next = obj->next;

		if( obj->pIndexData == pObj )
		{
			extract_obj( obj );
			ocount++;
		}
	}

	/* crush resets */
	rcount = 0;
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{
			for( pReset = pRoom->reset_first; pReset; pReset = wReset )
			{
				wReset = pReset->next;
				switch( pReset->command )
				{
				case 'O':
				case 'E':
				case 'P':
				case 'G':
					if( ( pReset->arg1 == index ) ||
						( ( pReset->command == 'P' ) && (
					pReset->arg3 == index ) ) )
					{
						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );

					}
				}
			}
		}
	}

	unlink_obj_index( pObj );

	pObj->area = NULL;
	pObj->vnum = 0;

	free_obj_index( pObj );

	sprintf( buf, "Removed object vnum {C%d{x and {C%d{x resets.\n\r", index,rcount );

	send_to_char( buf, ch );

	sprintf( buf, "{C%d{x occurences of the object were extracted from the mud.\n\r", ocount );

	send_to_char( buf, ch );
        edit_done(ch);

	return true;
}


MEDIT( medit_delete )
{
	CHAR_DATA *wch, *wnext;
	MOB_INDEX_DATA *pMob;
	RESET_DATA *pReset, *wReset;
	ROOM_INDEX_DATA *pRoom;
	char arg[MIL];
	char buf[MSL];
	int index, mcount, rcount, iHash, i;
	bool foundmob = false;
	bool foundobj = false;

	if( argument[0] == '\0' )
	{
		send_to_char( "Syntax:  medit delete [vnum]\n\r", ch );
		return false;
	}

	one_argument( argument, arg );

	if( is_number( arg ) )
	{
		index = atoi( arg );
		pMob = get_mob_index( index );
	}
	else
	{
		send_to_char( "That is not a number.\n\r", ch );
		return false;
	}

	if( !pMob )
	{
		send_to_char( "No such mobile.\n\r", ch );
		return false;
	}


	if (!IS_BUILDER(ch,pMob->area))
	{
		ch->send("You don't have access rights to that vnum.\n\r");
		return false;
	}
	SET_BIT( pMob->area->area_flags, AREA_CHANGED );

	if( top_vnum_mob == index )
		for( i = 1; i < index; i++ )
			if( get_mob_index( i ) )
				top_vnum_mob = i;

	top_mob_index--;

	/* Now crush all resets and take out mobs while were at it */
	rcount = 0;
	mcount = 0;
	
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
		{

			for( wch = pRoom->people; wch; wch = wnext )
			{
				wnext = wch->next_in_room;
				if( wch->pIndexData == pMob )
				{
					extract_char( wch, true );
					mcount++;
				}
			}

			for( pReset = pRoom->reset_first; pReset; pReset = wReset )
			{
				wReset = pReset->next;
				switch( pReset->command )
				{
				case 'M':
					if( pReset->arg1 == index )
					{
						foundmob = true;

						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );

					}
					else
						foundmob = false;

					break;
				case 'E':
				case 'G':
					if( foundmob )
					{
						foundobj = true;

						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );

					}
					else
						foundobj = false;

					break;
				case '0':
					foundobj = false;
					break;
				case 'P':
					if( foundobj && foundmob )
					{
						unlink_reset( pRoom, pReset );
						free_reset_data( pReset );

						rcount++;
						SET_BIT( pRoom->area->area_flags,
						AREA_CHANGED );
					}
				}
			}
		}
	}

	unlink_mob_index( pMob );

	pMob->area = NULL;
	pMob->vnum = 0;

	free_mob_index( pMob );

	printf_to_char( ch, "Removed mobile vnum {C%d{x and {C%d{x resets.\n\r", index, rcount );
	printf_to_char( ch, "{C%d{x mobiles were extracted from the mud.\n\r",mcount );
        edit_done(ch);
	return true;
}

REDIT( redit_delete )
{
	ROOM_INDEX_DATA *pRoom, *pRoom2;
	RESET_DATA *pReset;
	EXIT_DATA *ex;
	OBJ_DATA *Obj, *obj_next;
	CHAR_DATA *wch, *wnext;
	EXTRA_DESCR_DATA *pExtra;
	char arg[MIL];
	char buf[MSL];
	int index, i, iHash, rcount, ecount, mcount, ocount, edcount;

	if ( argument[0] == '\0' )
	{
		send_to_char( "Syntax:  redit delete [vnum]\n\r", ch );
		return false;
	}

	one_argument( argument, arg );

	if( is_number( arg ) )
	{
		index = atoi( arg );
		pRoom = get_room_index( index );
	}
	else
	{
		send_to_char( "That is not a number.\n\r", ch );
		return false;
	}

	if( !pRoom )
	{
		send_to_char( "No such room.\n\r", ch );
		return false;
	}

	if (!IS_BUILDER(ch,pRoom->area))
	{
		ch->send("You don't have access rights to that vnum.\n\r");
		return false;
	}
	/* Move the player out of the room. */
	if( ch->in_room->vnum == index )
	{
		send_to_char( "Moving you out of the room"
			" you are deleting.\n\r", ch);
		if( ch->fighting != NULL )
			stop_fighting( ch, true );

		char_from_room( ch );
		char_to_room( ch, get_room_index( 3 ) ); /* limbo */
		ch->was_in_room = ch->in_room;
		//ch->from_room = ch->in_room;
	}

	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

	/* Count resets. They are freed by free_room_index. */
	rcount = 0;

	for( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
	{
		rcount++;
	}

	/* Now contents */
	ocount = 0;
	for( Obj = pRoom->contents; Obj; Obj = obj_next )
	{
		obj_next = Obj->next_content;

		extract_obj( Obj );
		ocount++;
	}

	/* Now PCs and Mobs */
	mcount = 0;
	for( wch = pRoom->people; wch; wch = wnext )
	{
		wnext = wch->next_in_room;
		if( IS_NPC( wch ) )
		{
			extract_char( wch, true );
			mcount++;
		}
		else
			{
			send_to_char( "This room is being deleted. Moving" 
				" you somewhere safe.\n\r", ch );
			if( wch->fighting != NULL )
				stop_fighting( wch, true );

			char_from_room( wch );

			/* Midgaard Temple */
			char_to_room( wch, get_room_index( 3054 ) ); 
			wch->was_in_room = wch->in_room;
			//wch->from_room = wch->in_room;
		}
	}

	/* unlink all exits to the room. */
	ecount = 0;
	for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
		for( pRoom2 = room_index_hash[iHash]; pRoom2; pRoom2 = pRoom2->next )
		{
			for( i = 0; i <= MAX_DIR; i++ )
			{
				if( !( ex = pRoom2->exit[i] ) )
					continue;

				if( pRoom2 == pRoom )
				{
					/* these are freed by free_room_index */
					ecount++;
					continue;
				}

				if( ex->u1.to_room == pRoom )
				{
					free_exit( pRoom2->exit[i] );
					pRoom2->exit[i] = NULL;
					SET_BIT( pRoom2->area->area_flags, AREA_CHANGED );
					ecount++;
				}
			}
		}
	}

	/* count extra descs. they are freed by free_room_index */
	edcount = 0;
	for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next )
	{
		edcount++;
	}

	if( top_vnum_room == index )
		for( i = 1; i < index; i++ )
			if( get_room_index( i ) )
				top_vnum_room = i;

	top_room--;

	unlink_room_index( pRoom );

	pRoom->area = NULL;
	pRoom->vnum = 0;

	free_room_index( pRoom );

	/* Na na na na! Hey Hey Hey, Good Bye! */

	sprintf( buf, "Removed room vnum {C%d{x, %d resets, {C%d{x extra descriptions and {C%d{x exits.\n\r", index, rcount, edcount, ecount );
	send_to_char( buf, ch );
	sprintf( buf, "^C%d^x objects and {C%d{x mobiles were extracted from the room.\n\r", ocount, mcount );
	send_to_char( buf, ch );
        edit_done(ch);

	return true;
}


/* unlink a given reset from a given room */
void unlink_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset )
{
	RESET_DATA *prev, *wReset;

	prev = pRoom->reset_first;
	for( wReset = pRoom->reset_first; wReset; wReset = wReset->next )
	{
		if( wReset == pReset )
		{
			if( pRoom->reset_first == pReset )
			{
				pRoom->reset_first = pReset->next;
				if( !pRoom->reset_first )
					pRoom->reset_last = NULL;
			}
			else if( pRoom->reset_last == pReset )
			{
				pRoom->reset_last = prev;
				prev->next = NULL;
			}
			else
				prev->next = prev->next->next;

			if( pRoom->reset_first == pReset )
				pRoom->reset_first = pReset->next;

			if( !pRoom->reset_first )
				pRoom->reset_last = NULL;
			/*
			if( pRoom->area->reset_first == pReset )
				pRoom->area->reset_first = pReset->next;

			if( !pRoom->area->reset_first )
				pRoom->area->reset_last = NULL;*/
		}

		prev = wReset;
	}
}


void unlink_obj_index( OBJ_INDEX_DATA *pObj )
{
	int iHash;
	OBJ_INDEX_DATA *iObj, *sObj;

	iHash = pObj->vnum % MAX_KEY_HASH;

	sObj = obj_index_hash[iHash];

	if( sObj->next == NULL ) /* only entry */
		obj_index_hash[iHash] = NULL;
	else if( sObj == pObj ) /* first entry */
		obj_index_hash[iHash] = pObj->next;
	else /* everything else */
	{
		for( iObj = sObj; iObj != NULL; iObj = iObj->next )
		{
			if( iObj == pObj )
			{
				sObj->next = pObj->next;
				break;
			}
			sObj = iObj;
		}
	}
}


void unlink_room_index( ROOM_INDEX_DATA *pRoom )
{
	int iHash;
	ROOM_INDEX_DATA *iRoom, *sRoom;

	iHash = pRoom->vnum % MAX_KEY_HASH;

	sRoom = room_index_hash[iHash];

	if( sRoom->next == NULL ) /* only entry */
		room_index_hash[iHash] = NULL;
	else if( sRoom == pRoom ) /* first entry */
		room_index_hash[iHash] = pRoom->next;
	else /* everything else */
	{
		for( iRoom = sRoom; iRoom != NULL; iRoom = iRoom->next )
		{
			if( iRoom == pRoom )
			{
				sRoom->next = pRoom->next;
				break;
			}
			sRoom = iRoom;
		}
	}
}


void unlink_mob_index( MOB_INDEX_DATA *pMob )
{
	int iHash;
	MOB_INDEX_DATA *iMob, *sMob;

	iHash = pMob->vnum % MAX_KEY_HASH;

	sMob = mob_index_hash[iHash];

	if( sMob->next == NULL ) /* only entry */
		mob_index_hash[iHash] = NULL;
	else if( sMob == pMob ) /* first entry */
		mob_index_hash[iHash] = pMob->next;
	else /* everything else */
	{
		for( iMob = sMob; iMob != NULL; iMob = iMob->next )
		{
			if( iMob == pMob )
			{
				sMob->next = pMob->next;
				break;
			}
			sMob = iMob;
		}
	}
}
