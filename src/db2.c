#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include "merc.h"
#include "db.h"
#include "lookup.h"
#include "recycle.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/resource.h>

l_int flag_lookup (const char*,const struct flag_type*);
void init_blankguild (int);

void load_mobiles(FILE *fp){
	MOB_INDEX_DATA *pMobIndex;
	int i;

	if (!area_last){
		bug( "Load_mobiles: no #AREA seen yet.", 0 );
		exit(1);
	}

	for (;;){
		sh_int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#'){
			bug("Load_mobiles: # not found.",0);
			exit(1);
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = false;
		if (get_mob_index(vnum)){
			bug("Load_mobiles: vnum %d duplicated.",vnum);
			exit(1);
		}
		fBootDb = true;

		pMobIndex                       = (MOB_INDEX_DATA *) alloc_perm(sizeof(*pMobIndex));
		pMobIndex->vnum                 = vnum;
		pMobIndex->area                 = area_last;
		pMobIndex->new_format			= true;
		newmobs++;

		for (i=0;i<MAX_RES;i++)//making sure to set nonexistants or whatever
			pMobIndex->res[i] = 100;

		pMobIndex->player_name          = fread_string(fp);
		pMobIndex->short_descr          = fread_string(fp);
		pMobIndex->long_descr           = fread_string(fp);
		pMobIndex->description          = fread_string(fp);
		pMobIndex->race		 			= race_lookup(fread_string(fp));
		pMobIndex->pclass				= fread_number(fp);
		pMobIndex->trainer				= fread_number(fp);
		pMobIndex->icpmsg				= fread_string(fp);
		pMobIndex->ocpmsg				= fread_string(fp);
		pMobIndex->cmsg					= fread_string(fp);
		pMobIndex->long_descr[0]		= UPPER(pMobIndex->long_descr[0]);
		pMobIndex->description[0]		= UPPER(pMobIndex->description[0]);
		pMobIndex->pShop				= NULL;
		pMobIndex->alignment			= fread_number(fp);
		pMobIndex->group				= fread_number(fp);
		pMobIndex->level                = fread_number(fp);
		pMobIndex->hitroll              = fread_number(fp);
		pMobIndex->hit[DICE_NUMBER]     = fread_number(fp);
		/* 'd'          */                fread_letter(fp);
		pMobIndex->hit[DICE_TYPE]   	= fread_number(fp);
		/* '+'          */                fread_letter(fp);
		pMobIndex->hit[DICE_BONUS]      = fread_number(fp);

		pMobIndex->mana[DICE_NUMBER]	= fread_number(fp);
										  fread_letter(fp);
		pMobIndex->mana[DICE_TYPE]		= fread_number(fp);
										  fread_letter(fp);
		pMobIndex->mana[DICE_BONUS]		= fread_number(fp);

		pMobIndex->damage[DICE_NUMBER]	= fread_number(fp);
										  fread_letter(fp);
		pMobIndex->damage[DICE_TYPE]	= fread_number(fp);
										  fread_letter(fp);
		pMobIndex->damage[DICE_BONUS]	= fread_number(fp);
		pMobIndex->dam_type				= attack_lookup(fread_word(fp));

		pMobIndex->ac[AC_PIERCE]		= fread_number(fp) * 10;
		pMobIndex->ac[AC_BASH]			= fread_number(fp) * 10;
		pMobIndex->ac[AC_SLASH]			= fread_number(fp) * 10;
		pMobIndex->ac[AC_EXOTIC]		= fread_number(fp) * 10;
		pMobIndex->start_pos			= position_lookup(fread_word(fp));
		pMobIndex->default_pos			= position_lookup(fread_word(fp));
		pMobIndex->sex					= sex_lookup(fread_word(fp));
		pMobIndex->wealth				= fread_number( fp );
		pMobIndex->guild				= fread_number( fp );
		CHECK_POS(pMobIndex->size,size_lookup(fread_word(fp)),"size");
		pMobIndex->material				= str_dup(fread_word(fp));
		read_bits(pMobIndex->act,MAX_ACT,fread_string(fp));
		read_bits(pMobIndex->affected_by,MAX_AFF,fread_string(fp));
		read_bits(pMobIndex->off_bits,MAX_OFF,fread_string(fp));
		read_bits(pMobIndex->def_bits,MAX_DEF,fread_string(fp));
		read_ints(pMobIndex->res,MAX_RES,fread_string(fp));
		read_bits(pMobIndex->form,MAX_FRM,fread_string(fp));
		read_bits(pMobIndex->parts,MAX_PRT,fread_string(fp));
		for (i=0;i<MAX_STATS;i++)//worry
			pMobIndex->perm_stat[i] = fread_number(fp);
		pMobIndex->setact(AT_IS_NPC);

		letter							= fread_letter(fp);

		if (letter == 'C')
			fread_number(fp);
		else
			ungetc(letter, fp); /* Feydrex - clan guards */

		for ( ; ; ){
			letter = fread_letter(fp);

			if (letter == 'F')
			{
				char *word = fread_word(fp);
				long vector = fread_flag(fp);

				if (!str_prefix(word,"act"))			pMobIndex->remact(vector);
				else if (!str_prefix(word,"aff"))		pMobIndex->remaff(vector);
				else if (!str_prefix(word,"off"))		pMobIndex->remoff(vector);
				else if (!str_prefix(word,"for"))		pMobIndex->remform(vector);
				else if (!str_prefix(word,"par"))		pMobIndex->rempart(vector);
				else{
					bug("Flag remove: flag not found.",0);
					exit(1);
				}
			}
			else if (letter == 'M'){
				PROG_LIST *pMprog = (PROG_LIST*)alloc_perm(sizeof(*pMprog));
				char *word = fread_word(fp);
				int trigger = 0;

				if ((trigger = flag_lookup(word,mprog_flags)) == NO_FLAG){
					bug("MOBprogs: invalid trigger.",0);
					exit(1);
				}
				SET_BIT(pMobIndex->mprog_flags,trigger);
				pMprog->trig_type   = trigger;
				pMprog->vnum        = fread_number(fp);
				pMprog->trig_phrase = fread_string(fp);
				pMprog->next        = pMobIndex->mprogs;
				pMobIndex->mprogs   = pMprog;
			}
			else{
				ungetc(letter,fp);
				break;
			}
		}

		iHash						= vnum % MAX_KEY_HASH;
		pMobIndex->next				= mob_index_hash[iHash];
		mob_index_hash[iHash]		= pMobIndex;
		top_mob_index++;
		top_vnum_mob				= top_vnum_mob < vnum ? vnum : top_vnum_mob;
		assign_area_vnum(vnum);
		kill_table[URANGE(0,pMobIndex->level,MAX_LEVEL-1)].number++;
	}
}

void load_objects(FILE *fp){//hrm
	OBJ_INDEX_DATA *pObjIndex;

	if (!area_last){
		bug("Load_objects: no #AREA seen yet.",0);
		exit(1);
	}

	for ( ; ; )
	{
		sh_int vnum;
		char letter;
		int iHash;

		letter                          = fread_letter(fp);
		if (letter != '#')
		{
			bug("Load_objects: # not found.",0);
			exit(1);
		}

		vnum                            = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = false;
		if (get_obj_index(vnum) != NULL)
		{
			bug("Load_objects: vnum %d duplicated.",vnum);
			exit(1);
		}
		fBootDb = true;

		pObjIndex                       = (OBJ_INDEX_DATA *)alloc_perm(sizeof(*pObjIndex));
		pObjIndex->vnum                 = vnum;
		pObjIndex->area                 = area_last;
		pObjIndex->new_format           = true;
		pObjIndex->reset_num			= 0;
		newobjs++;
		pObjIndex->name					= fread_string(fp);
		pObjIndex->short_descr			= fread_string(fp);
		pObjIndex->lore					= fread_string(fp);
		pObjIndex->description			= fread_string(fp);
		pObjIndex->material				= fread_string(fp);
		pObjIndex->armortype_flags		= fread_number(fp);

		CHECK_POS(pObjIndex->item_type,item_lookup(fread_word(fp)),"item_type");
		pObjIndex->extra_flags          = fread_flag(fp);
		pObjIndex->wear_flags           = fread_flag(fp);
		pObjIndex->exclude				= fread_flag(fp);
		read_bits(pObjIndex->wflags,MAX_WPN,fread_string(fp));

		switch(pObjIndex->item_type)
		{
		case ITEM_THROWINGKNIFE:
		case ITEM_THROWINGDART:
		case ITEM_THROWINGAXE:
			pObjIndex->value[0]		= fread_number(fp);
			pObjIndex->value[1]		= fread_number(fp);
			pObjIndex->value[2]		= fread_number(fp);
			pObjIndex->value[3]		= attack_lookup(fread_word(fp));
			pObjIndex->value[4]		= fread_flag(fp);
			break;
		case ITEM_WEAPON:
			pObjIndex->value[0]		= weapon_type(fread_word(fp));
			pObjIndex->value[1]		= fread_number(fp);
			pObjIndex->value[2]		= fread_number(fp);
			pObjIndex->value[3]		= attack_lookup(fread_word(fp));
			pObjIndex->value[4]		= fread_number(fp);
			break;
		case ITEM_TRAPPARTS:
			pObjIndex->value[0]		= trapy_type(fread_word(fp));
			pObjIndex->value[1]		= fread_number(fp);
			pObjIndex->value[2]		= fread_flag(fp);
			pObjIndex->value[3]		= fread_number(fp);
			pObjIndex->value[4]		= fread_number(fp);
			break;
		case ITEM_CONTAINER:
		case ITEM_LOCKER:
			pObjIndex->value[0]		= fread_number(fp);
			pObjIndex->value[1]		= fread_flag(fp);
			pObjIndex->value[2]		= fread_number(fp);
			pObjIndex->value[3]		= fread_number(fp);
			pObjIndex->value[4]		= fread_number(fp);
			break;
		case ITEM_DRINK_CON:
		case ITEM_FOUNTAIN:
				pObjIndex->value[0]         = fread_number(fp);
				pObjIndex->value[1]         = fread_number(fp);
				CHECK_POS(pObjIndex->value[2], liq_lookup(fread_word(fp)), "liq_lookup" );
				pObjIndex->value[3]         = fread_number(fp);
				pObjIndex->value[4]         = fread_number(fp);
				break;
		case ITEM_FORGE:
		case ITEM_FIRE:
		case ITEM_SCARS:
		case ITEM_PARCHMENT:
		case ITEM_QUIVER:
		case ITEM_ARROW:
		case ITEM_INGOT:
		case ITEM_FORGEFUEL:
		case ITEM_LOCKPICK:
				pObjIndex->value[0]         = fread_number(fp);
				pObjIndex->value[1]         = fread_number(fp);
				pObjIndex->value[2]         = fread_number(fp);
				pObjIndex->value[3]         = fread_number(fp);
				pObjIndex->value[4]         = fread_number(fp);
				break;
		case ITEM_WAND:
		case ITEM_STAFF:
			pObjIndex->value[0]		= fread_number(fp);
			pObjIndex->value[1]		= fread_number(fp);
			pObjIndex->value[2]		= fread_number(fp);
			pObjIndex->value[3]		= skill_lookup(fread_word(fp));
			pObjIndex->value[4]		= fread_number(fp);
			break;
		case ITEM_CALTROPS:
 			pObjIndex->value[0]		= fread_number(fp);
 			pObjIndex->value[1]		= fread_number(fp);
			break;
		case ITEM_POTION:
		case ITEM_PILL:
		case ITEM_SCROLL:
 			pObjIndex->value[0]		= fread_number(fp);
			pObjIndex->value[1]		= skill_lookup(fread_word(fp));
			pObjIndex->value[2]		= skill_lookup(fread_word(fp));
			pObjIndex->value[3]		= skill_lookup(fread_word(fp));
			pObjIndex->value[4]		= skill_lookup(fread_word(fp));
			break;
		default:
			pObjIndex->value[0]             = fread_flag( fp );
			pObjIndex->value[1]             = fread_flag( fp );
			pObjIndex->value[2]             = fread_flag( fp );
			pObjIndex->value[3]             = fread_flag( fp );
			pObjIndex->value[4]		    = fread_flag( fp );
			break;
		}
		pObjIndex->level		= fread_number(fp);
		pObjIndex->weight               = fread_number(fp);
		pObjIndex->cost                 = fread_number(fp);
		pObjIndex->droprate				= fread_number(fp);
		pObjIndex->timer				= fread_number(fp);

		/* condition */
		letter 				= fread_letter(fp);
		switch (letter)
 		{
			case ('P') :		pObjIndex->condition = 100; break;
			case ('G') :		pObjIndex->condition =  90; break;
			case ('A') :		pObjIndex->condition =  75; break;
			case ('W') :		pObjIndex->condition =  50; break;
			case ('D') :		pObjIndex->condition =  25; break;
			case ('B') :		pObjIndex->condition =  10; break;
			case ('R') :		pObjIndex->condition =   0; break;
			default:			pObjIndex->condition = 100; break;
		}

		for ( ; ; )
		{
			char letter;

			letter = fread_letter(fp);

			if (letter == 'A')
			{
				AFFECT_DATA *paf;

				paf                     = (AFFECT_DATA *)alloc_perm(sizeof(*paf));
				paf->where		= TO_OBJECT;
				paf->type               = -1;
				paf->level              = pObjIndex->level;
				paf->duration           = -1;
				paf->location           = fread_number(fp);
				paf->modifier           = fread_number(fp);
				paf->bitvector          = 0;
				paf->next               = pObjIndex->affected;
				pObjIndex->affected     = paf;
				top_affect++;
			}
			else if (letter == 'F')
			{
				AFFECT_DATA *paf;

				paf                     = (AFFECT_DATA *) alloc_perm( sizeof(*paf) );
				letter 			= fread_letter(fp);
				switch (letter)
				{
				case 'A':
					paf->where      = TO_AFFECTS;
					break;
				case 'I':
					paf->where		= TO_IMMUNE;
					break;
				case 'R':
					paf->where		= TO_RESIST;
					break;
				case 'V':
					paf->where		= TO_VULN;
					break;
				default:
					bug("Load_objects: Bad where on flag set.",0);
					exit(1);
				}
				paf->type               = -1;
				paf->level              = pObjIndex->level;
				paf->duration           = -1;
				paf->location           = fread_number(fp);
				paf->modifier           = fread_number(fp);
				paf->bitvector          = fread_flag(fp);
				paf->next               = pObjIndex->affected;
				pObjIndex->affected     = paf;
				top_affect++;
			}
			else if (letter == 'E')
			{
				EXTRA_DESCR_DATA *ed;

				ed                      = (EXTRA_DESCR_DATA *)alloc_perm(sizeof(*ed));
				ed->keyword             = fread_string(fp);
				ed->description         = fread_string(fp);
				ed->next                = pObjIndex->extra_descr;
				pObjIndex->extra_descr  = ed;
				top_ed++;
			}
			else if (letter == 'O')
			{
				PROG_LIST *pOprog;
				char *word;
				int trigger = 0;

				pOprog			= (PROG_LIST *)alloc_perm(sizeof(*pOprog));
				word			= fread_word(fp);
				if (!(trigger = flag_lookup(word,oprog_flags)))
				{
					bug("OBJprogs: invalid trigger.",0);
					exit(1);
				}
				SET_BIT(pObjIndex->oprog_flags,trigger);
				pOprog->trig_type	= trigger;
				pOprog->vnum	 	= fread_number(fp);
				pOprog->trig_phrase	= fread_string(fp);
				pOprog->next		= pObjIndex->oprogs;
				pObjIndex->oprogs	= pOprog;
			}
			else
			{
				ungetc(letter,fp);
				break;
			}
		}

		iHash                   = vnum % MAX_KEY_HASH;
		pObjIndex->next         = obj_index_hash[iHash];
		obj_index_hash[iHash]   = pObjIndex;
		top_obj_index++;
		top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;
		assign_area_vnum(vnum);
	}
}

/*****************************************************************************
 Purpose:	Converts all old format objects to new format
 Called by:	boot_db (db.c).
 Note:          Loops over all resets to find the level of the mob
                loaded before the object to determine the level of
                the object.
		It might be better to update the levels in load_resets().
		This function is not pretty.. Sorry about that :)
 ****************************************************************************/
void convert_objects( void )
{
    int vnum;
    AREA_DATA  *pArea;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;

    if ( newobjs == top_obj_index )
		return;/* all objects in new format */

	log_string("Poops");
    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
		{
			if (!(pRoom = get_room_index(vnum)))
				continue;

			for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
			{
				switch ( pReset->command )
				{
					case 'M':
						if (!(pMob = get_mob_index(pReset->arg1)))
							bug("Convert_objects: 'M': bad vnum %d.",pReset->arg1);
						break;
					case 'O':
						if (!(pObj = get_obj_index( pReset->arg1)))
						{
							bug("Convert_objects: 'O': bad vnum %d.",pReset->arg1);
							break;
						}
						if (pObj->new_format)
							continue;
						if (!pMob)
						{
							bug("Convert_objects: 'O': No mob reset yet.",0);
							break;
						}
						pObj->level = pObj->level < 1 ? pMob->level - 2 : UMIN(pObj->level,pMob->level - 2);
						break;
					case 'P':
					{
						OBJ_INDEX_DATA *pObj, *pObjTo;

						if (!(pObj = get_obj_index(pReset->arg1)))
						{
							bug("Convert_objects: 'P': bad vnum %d.",pReset->arg1);
							break;
						}
						if (pObj->new_format)
							continue;
						if (!(pObjTo = get_obj_index(pReset->arg3)))
						{
							bug("Convert_objects: 'P': bad vnum %d.",pReset->arg3);
							break;
						}
						pObj->level = pObj->level < 1 ? pObjTo->level : UMIN(pObj->level,pObjTo->level);
					}
						break;
					case 'G':
					case 'E':
						if (!(pObj = get_obj_index(pReset->arg1)))
						{
							bug("Convert_objects: 'E' or 'G': bad vnum %d.",pReset->arg1);
							break;
						}
						if (!pMob)
						{
							bug("Convert_objects: 'E' or 'G': null mob for vnum %d.",pReset->arg1);
							break;
						}
						if (pObj->new_format)
							continue;
						if (pMob->pShop)
						{
							switch (pObj->item_type)
							{
								default:
									pObj->level = UMAX(0,pObj->level);
									break;
								case ITEM_PILL:
								case ITEM_POTION:
									pObj->level = UMAX(5,pObj->level);
									break;
								case ITEM_SCROLL:
								case ITEM_ARMOR:
								case ITEM_SHIELD:
								case ITEM_WEAPON:
									pObj->level = UMAX(10,pObj->level);
									break;
								case ITEM_WAND:
									pObj->level = UMAX(15,pObj->level);
									break;
								case ITEM_STAFF:
									pObj->level = UMAX(20,pObj->level);
									break;
							}
						}
						else
							pObj->level = pObj->level < 1 ? pMob->level : UMIN(pObj->level,pMob->level);
						break;
				} /* switch ( pReset->command ) */
			}
		}
	}

    /* do the conversion: */

    for ( pArea = area_first; pArea ; pArea = pArea->next )
		for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ )
			if ((pObj = get_obj_index(vnum)))
 		if (!pObj->new_format)
		    convert_object(pObj);

    return;
}

/*****************************************************************************
 Purpose:	Converts an old_format obj to new_format
 Called by:	convert_objects (db2.c).
 Note:          Dug out of create_obj (db.c)
 ****************************************************************************/
void convert_object( OBJ_INDEX_DATA *pObjIndex )
{
    int level,number,type;/* for dice-conversion */

    if (!pObjIndex || pObjIndex->new_format)
		return;

	log_string("Poops");
    return;
}

void init_blankclass(int wclass){
	int n;

	sprintf(classes[wclass].who_name,"XXX");
	classes[wclass].attr_prime		= 1;
	for (n=0;n<MAX_STATS;n++)
		classes[wclass].attribute[n]= 1;
	for(n = 0;n < MAX_SKILL;n++){
		skill_table[n].cost[wclass] = -1;
		for(int l = 0;l<10;l++){
			skill_table[n].skl[wclass].unlocks[l].sn = 0;
			skill_table[n].skl[wclass].unlocks[l].level = 0;
			skill_table[n].skl[wclass].locks[l].sn = 0;
			skill_table[n].skl[wclass].locks[l].level = 0;
			skill_table[n].skl[wclass].unlock_by[l].sn = 0;
			skill_table[n].skl[wclass].unlock_by[l].level = 0;
		}
	}

	classes[wclass].name = 0;
	classes[wclass].skill_adept		= 50;
	classes[wclass].thac0_00		= 10;
	classes[wclass].thac0_32		= 5;
	classes[wclass].hp_min			= 1;
	classes[wclass].hp_max			= 10;
	classes[wclass].mp_min			= 1;
	classes[wclass].mp_max			= 10;
	classes[wclass].mv_min			= 1;
	classes[wclass].mv_max			= 10;
	classes[wclass].fMana			= 0;
	classes[wclass].ctier			= 0;
	classes[wclass].armor			= 0;
	classes[wclass].align			= 0;
	classes[wclass].amp				= false;
	classes[wclass].msg_self		= 0; // str_dup("You BUGGY say,'{M%s{x'.");
	classes[wclass].msg_other		= 0; // str_dup("$n BUGGY says,'{M%s{x'.");
	classes[wclass].active			= false;
}

void load_classes( void ){
    FILE *fp;
    char buf[MSL];
    int n,i,weapon,sn,sn2,tn;

	for(n = 0;n < MAX_CLASS;n++)
		init_blankclass(n);
	for(n = 0;n < MAX_USECLASS;n++){
		sprintf(buf,"%s%s",CLASS_DIR,class_files[n].filename);
		//log_f("Loading %s.",class_files[n].name);
		if (!(fp = fopen(buf,"r"))){
			log_f("Error opening class file '%s'.",buf);
			exit(1);
		}
		for (i = 0;pc_race_table[i].name;i++)
			pc_race_table[i].class_use[n] = false;
		for (;;){
			char letter = fread_letter(fp),*word,*wname;

			if (letter == '*'){
				fread_to_eol(fp);
				continue;
			}
			if (letter != '#'){
				bug("Load_Skills: # not found.",0);
				break;
			}
			word = fread_word(fp);

			if (!str_cmp(word,"CLASS")){
				char *field;
				for (;;){
					field = fread_word(fp);
						 if(!str_cmp(field,"Name"))
						 {      free_string(classes[n].name);
							classes[n].name					= fread_string(fp);
						}
					else if(!str_cmp(field,"WhoN"))		for(i=0;i<3;i++)	classes[n].who_name[i]			= fread_letter(fp);
					else if(!str_cmp(field,"Prime"))						classes[n].attr_prime			= fread_number(fp);
					else if(!str_cmp(field,"Active"))						classes[n].active				= fread_number(fp);
					else if(!str_cmp(field,"Align"))						classes[n].align				= fread_number(fp);
					else if(!str_cmp(field,"STRSTAT"))						classes[n].attribute[STAT_STR]	= fread_number(fp);
					else if(!str_cmp(field,"ENDSTAT"))						classes[n].attribute[STAT_END]	= fread_number(fp);
					else if(!str_cmp(field,"AGISTAT"))						classes[n].attribute[STAT_AGI]	= fread_number(fp);
					else if(!str_cmp(field,"INTSTAT"))						classes[n].attribute[STAT_INT]	= fread_number(fp);
					else if(!str_cmp(field,"RESSTAT"))						classes[n].attribute[STAT_RES]	= fread_number(fp);
					else if(!str_cmp(field,"FTHSTAT"))						classes[n].attribute[STAT_FTH]	= fread_number(fp);
					else if(!str_cmp(field,"WISSTAT"))						classes[n].attribute[STAT_WIS]	= fread_number(fp);
					else if(!str_cmp(field,"CHASTAT"))						classes[n].attribute[STAT_CHA]	= fread_number(fp);
					else if(!str_cmp(field,"LCKSTAT"))						classes[n].attribute[STAT_LCK]	= fread_number(fp);
					else if(!str_cmp(field,"Weapon")){
						wname = fread_string(fp);
						for(weapon = 0; weapon_table[weapon].name != NULL; weapon++)
							if(!str_cmp(wname,weapon_table[weapon].name))
								break;
					}
					else if (!str_cmp(field,"Race")){
						wname = fread_word(fp);
						i = race_lookup(wname);
						if (i > 0)
							pc_race_table[i].class_use[n] = true;
						else
							log_f("Shitty race: %s",wname);
					}
					else if (!str_cmp(field,"SelfMsg"))			classes[n].msg_self = fread_string(fp);
					else if (!str_cmp(field,"OtherMsg"))		classes[n].msg_other = fread_string(fp);
					else if (!str_cmp(field,"Skill"))			classes[n].skill_adept = fread_number(fp);
					else if (!str_cmp(field,"Thac00"))			classes[n].thac0_00 = fread_number(fp);
					else if (!str_cmp(field,"Thac32"))			classes[n].thac0_32 = fread_number(fp);
					else if (!str_cmp(field,"HPMin"))			classes[n].hp_min = fread_number(fp);
					else if (!str_cmp(field,"HPMax"))			classes[n].hp_max = fread_number(fp);
					else if (!str_cmp(field,"MPMin"))			classes[n].mp_min = fread_number(fp);
					else if (!str_cmp(field,"MPMax"))			classes[n].mp_max = fread_number(fp);
					else if (!str_cmp(field,"MVMin"))			classes[n].mv_min = fread_number(fp);
					else if (!str_cmp(field,"MVMax"))			classes[n].mv_max = fread_number(fp);
					else if (!str_cmp(field,"FMana"))			classes[n].fMana = fread_number(fp);
					else if (!str_cmp(field,"Tier"))			classes[n].ctier = fread_number(fp);
					else if (!str_cmp(field,"Armor"))			classes[n].armor = fread_number(fp);
					else if (!str_cmp(field,"AMP"))				classes[n].amp = true;
					else if (!str_cmp(field,"End"))				break;
				}
			}
			else if (!str_cmp(word,"SKILLS")){
				for (;;){
					char *word= fread_string(fp);

					if (!str_cmp(word,"End"))
						break;
					sn = skill_lookup(word);
					if (sn > 0){
						if(skill_table[sn].cost[n] > 0){
							log_f("Overlapping skill %d: '%s' in class %d: '%s', %d",sn,word,n,classes[n].name,skill_table[sn].cost[n]);
							fread_number(fp);
							fread_number(fp);
						}
						else{
							skill_table[sn].skill_level[n] = fread_number(fp);
							skill_table[sn].cost[n] = fread_number(fp);
						}
					}
					else{
						log_f("Unrecognized skill '%s' in class '%s'",word,classes[n].name);
						fread_number(fp);
						fread_number(fp);
					}
				}
			}
			else if (!str_cmp(word,"REQUIREMENTS")){
				for (;;){
					char *word = fread_string(fp),*word2;

					if (!str_cmp(word,"End"))
						break;
					sn = skill_lookup(word);
					tn = fread_number(fp);
					word2 = fread_string(fp);
					sn2 = skill_lookup(word2);
					if (sn > 0 && sn2 > 0){
						for(int l = 0; l < 10; l++){
							if(skill_table[sn2].skl[n].unlocks[l].sn > 0)
								continue;
							if(l > 9){
								log_f("Max unlocks reached '%s' '%s' in class '%s'",word,word2,classes[n].name);
								break;
							}
							skill_table[sn2].skl[n].unlocks[l].sn = sn;
							skill_table[sn2].skl[n].unlocks[l].level = tn;
							break;
						}
						for(int l = 0; l < 10; l++){
							if(skill_table[sn].skl[n].unlock_by[l].sn > 0)
								continue;
							if(l > 9){
								log_f("Max needs reached '%s' '%s' in class '%s'",word,word2,classes[n].name);
								break;
							}
							skill_table[sn].skl[n].unlock_by[l].sn = sn2;
							skill_table[sn].skl[n].unlock_by[l].level = tn;
							break;
						}
						skill_table[sn].cost[n] = 0;
					}
					else
						log_f("Unrecognized skills '%s' '%s' in class '%s'",word,word2,classes[n].name);
				}
			}
			else if(!str_cmp(word,"RACES")){
				for (;;){
					int race;
					char *word = fread_string(fp);

					if (!str_cmp(word,"End"))
						break;
					if(!str_cmp(word,"All")){
						for(int l = 0;pc_race_table[l].name;l++)
							pc_race_table[l].class_use[n] = 1;
					}
					else{
						race = race_lookup(word);

						if (race == 0 || !race_table[race].pc_race)
							log_f("Unrecognized race %s",word);
						else
							pc_race_table[race].class_use[n] = 1;
					}
				}
			}
			else if (!str_cmp(word,"CANCELS")){
				for (;;){
					char *word = fread_string(fp),*word2;

					if (!str_cmp(word,"End"))
						break;
					sn = skill_lookup(word);
					tn = fread_number(fp);
					word2 = fread_string(fp);
					sn2 = skill_lookup(word2);
					if (sn > 0 && sn2 > 0){
						for(int l = 0; l < 10; l++){
							if(skill_table[sn2].skl[n].unlocks[l].sn > 0)
								continue;
							if(l > 9){
								log_f("Max cancels reached '%s' '%s' in class '%s'",word,word2,classes[n].name);
								break;
							}
							skill_table[sn2].skl[n].locks[l].sn = sn;
							skill_table[sn2].skl[n].locks[l].level = tn;
							break;
						}
					}
					else
						log_f("Unrecognized skills '%s' '%s' in class '%s'",word,word2,classes[n].name);
				}
			}
			else if (!str_cmp(word,"BECOMES")){
				for (;;){
					bool found = false;
					char *word = fread_string(fp);
					int vn;

					if (!str_cmp(word,"End"))
						break;
					for(vn = 0;vn < MAX_CLASS && !found; vn++){
						if(!strcmp(class_files[vn].name,word)){
							found = true;
							break;
						}
					}
					if (found){//n is the current class, sn is what it becomes NASH needs to code the skill reader to error check for already set shit
						int tc = n,nc;
						classes[n].becomes[vn] = 1;
						for(int i=0;i<MAX_SKILL && skill_table[i].name;i++){
							if(skill_table[i].cost[n] >= 0){
								skill_table[i].skill_level[vn]						= skill_table[i].skill_level[n];
								skill_table[i].cost[vn]								= skill_table[i].cost[n];
								for(int l = 0;l < 10; l++){
									if(skill_table[i].skl[n].unlocks[l].sn > 0){
										skill_table[i].skl[vn].unlocks[l].sn		= skill_table[i].skl[n].unlocks[l].sn;
										skill_table[i].skl[vn].unlocks[l].level		= skill_table[i].skl[n].unlocks[l].level;
									}
									if(skill_table[i].skl[n].unlock_by[l].sn > 0){
										skill_table[i].skl[vn].unlock_by[l].sn		= skill_table[i].skl[n].unlock_by[l].sn;
										skill_table[i].skl[vn].unlock_by[l].level	= skill_table[i].skl[n].unlock_by[l].level;
										skill_table[i].cost[vn] = 0;
									}

									if(skill_table[i].skl[n].locks[l].sn > 0){
										skill_table[i].skl[vn].locks[l].sn			= skill_table[i].skl[n].locks[l].sn;
										skill_table[i].skl[vn].locks[l].level		= skill_table[i].skl[n].locks[l].level;
									}
								}
							}
						}
					}
					else
						log_f("Unrecognized classbecome '%s' in class '%s'",word,classes[n].name);
				}
			}
			else if (!str_cmp(word,"END")){
				fclose(fp);
				break;
			}
		}
    }
	for(n = 0; n<MAX_SKILL;n++)
		skill_table[n].skill_level[CLASS_IMMORTAL] = 110;
}

int get_guild_type(char *word){
	if(!str_cmp(word,"immortal"))
		return GTYPE_IMMORTAL;
	else if(!str_cmp(word,"kingdom"))
		return GTYPE_KINGDOM;
	else if(!str_cmp(word,"republic"))
		return GTYPE_REPUBLIC;
	else if(!str_cmp(word,"tribe"))
		return GTYPE_TRIBE;
	else if(!str_cmp(word,"guild"))
		return GTYPE_GUILD;
	else if(!str_cmp(word,"populace"))
		return GTYPE_POPULACE;
	else
		return -1;
}

void load_guilds(){
    FILE *fp;
    char buf[MSL];
    int n,i,rank,sn;

	for(n = 0;n < MAX_GUILD;n++)
		init_blankguild(n);

    for(n = 0;n < MAX_GUILD;n++){

		if (!(fp = fopen(buf,"r"))){
			log_f("Error opening guild file '%s'.",buf);
			exit(1);
		}

		for (;;){
			char letter,*word,*wname;

			letter = fread_letter(fp);
			if (letter == '*'){
				fread_to_eol(fp);
				continue;
			}
			if (letter != '#'){
				bug("Load_Guilds: # not found.",0);
				break;
			}
			
			word = fread_word(fp);//log_string(word);

			if (!str_cmp(word,"GUILD")){//log_f("Loading guild");
				char *field;

				for (;;){
					field = fread_word(fp);
						 if(!str_cmp(field,"Name"))		guilds[n].name				= fread_string(fp);
					else if(!str_cmp(field,"WhoN"))		guilds[n].who_name			= fread_string(fp);
					else if(!str_cmp(field,"Keywords"))	guilds[n].keywords			= fread_string(fp);
					else if(!str_cmp(field,"Active"))	guilds[n].active			= fread_number(fp);
					else if(!str_cmp(field,"Index"))	guilds[n].index				= fread_number(fp);
					else if(!str_cmp(field,"Recall"))	guilds[n].recall			= fread_number(fp);
					else if(!str_cmp(field,"Respawn"))	guilds[n].respawn			= fread_number(fp);
					else if(!str_cmp(field,"Area"))		guilds[n].area				= fread_number(fp);
					else if(!str_cmp(field,"Class"))	guilds[n].type				= get_guild_type(fread_word(fp));
					else if(!str_cmp(field,"Hidden"))	guilds[n].hidden			= fread_number(fp);
					else if(!str_cmp(field,"Recruit"))	guilds[n].rank[fread_number(fp)].recruit	= true;
					else if(!str_cmp(field,"Expel"))	guilds[n].rank[fread_number(fp)].expel		= true;
					else if(!str_cmp(field,"Promote"))	guilds[n].rank[fread_number(fp)].promote	= true;
					else if(!str_cmp(field,"Demote"))	guilds[n].rank[fread_number(fp)].demote		= true;
					else if(!str_cmp(field,"Mask")){
						wname = fread_word(fp);
						if((i = guild_lookup(wname)) == -1)
							log_f("Shitty GuildMask: %s",wname);
						else
							guilds[n].hidden = i;
					}
					else if(!str_cmp(field,"RANK")){
						rank = fread_number(fp);
						guilds[n].rank[rank].name = fread_string(fp);
						if(guilds[n].rank_last < rank)
							guilds[n].rank_last = rank;
					}
					else if (!str_cmp(field,"Race")){
						//log_f("Loading races for %s",guilds[n].name);
						wname = fread_word(fp);
						if(!str_cmp(wname,"all")){
							for(i = 1; i < MAX_RACE - 1;i++){
								guilds[n].races[i] = true;
								guilds[n].races_max++;
							}
						//log_f("%s guild will have all races.",guilds[n].name);
						}
						else{
							i = pc_race_lookup(wname);
						//log_f("%s guild will have %s race.",guilds[n].name,wname);
							if (i > 0){
								guilds[n].races[i] = true;
								guilds[n].races_max++;
							}
							else
								log_f("Shitty race: %s",wname);
						}
					}
					else if(!str_cmp(field,"End"))
						break;
				}
			}
			else if (!str_cmp(word,"END")){
				fclose(fp);
				break;
			}
		}
    }
}
