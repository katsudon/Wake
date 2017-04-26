#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"

void impose_job(CHAR_DATA *, int); 
extern int _filbuf ( FILE* );
void unmorph_beast( CHAR_DATA *ch );

char *print_flags(l_int flag){
	int count, pos = 0;
	static char buf[52];

	for (count = 0; count < 32;  count++){
		if (IS_SET(flag,1<<count)){
			if (count < 26)
				buf[pos] = 'A' + count;
			else
				buf[pos] = 'a' + (count - 26);
			pos++;
		}
	}

	if (pos == 0){
		buf[pos] = '0';
		pos++;
	}

	buf[pos] = '\0';

	return buf;
}

#define	MAX_NEST 100
static	OBJ_DATA * rgObjNest [MAX_NEST];

/*
 * Local functions.
 */
void fwrite_char	(CHAR_DATA*,FILE*);
void fwrite_obj		(CHAR_DATA*,OBJ_DATA*,FILE*,int);
void fwrite_pet		(CHAR_DATA*,FILE*);
void fwrite_mount	(CHAR_DATA*,FILE*);
void fread_char		(CHAR_DATA*,FILE*);
void fread_pet		(CHAR_DATA*,FILE*);
void fread_mount	(CHAR_DATA*,FILE*);
void fread_obj		(CHAR_DATA*,FILE*);

void save_char_obj(CHAR_DATA *ch){
    char strsave[MIL];
    FILE *fp;

    if (IS_NPC(ch))
		return;

    if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

    fclose(fpReserve);
    sprintf(strsave,"%s%s",PLAYER_DIR,capitalize(ch->name));
    if ((fp = fopen(TEMP_FILE,"w")) == NULL)
    {
		bug("Save_char_obj: fopen",0);
		perror(strsave);
    }
    else
    {
		fwrite_char(ch,fp);
		if (ch->carrying != NULL)
			fwrite_obj(ch,ch->carrying,fp,0);
		/* save the pets */
		if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
			fwrite_pet(ch->pet,fp);
		if (ch->mount && ch->mount->in_room == ch->in_room)
			fwrite_mount(ch->mount,fp);
		fprintf(fp,"#END\n");
    }
    fclose(fp);
    rename(TEMP_FILE,strsave);
    fpReserve = fopen( NULL_FILE, "r" );
}

void fwrite_char(CHAR_DATA *ch,FILE *fp){
	AFFECT_DATA *paf,*afnext;
    int sn,gn,pos,i;
	char buf[100];

	if(ch->iscomm(CM_MORPH)){
		unmorph_beast(ch);
	}
		fprintf(fp,"#%s\n",			IS_NPC(ch) ? "MOB" : "PLAYER");
		fprintf(fp,"Name %s~\n",	ch->name);
		fprintf(fp,"LName %s~\n",	ch->pcdata->lname);
		fprintf(fp,"Id   %ld\n",	ch->id);
		fprintf(fp,"LogO %ld\n",	current_time);
		fprintf(fp,"Vers %d\n",		ch->version);

    if (ch->short_descr[0] != '\0')
      	fprintf(fp,"ShD  %s~\n",	ch->short_descr);
    if( ch->long_descr[0] != '\0')
		fprintf(fp,"LnD  %s~\n",	ch->long_descr);
    if (ch->description[0] != '\0')
    	fprintf(fp,"Desc %s~\n",	ch->description);
    if (ch->prompt != NULL || !str_cmp(ch->prompt,"{x[{G%h{x/{g%H{xhp {C%m{x/{c%M{xmp {B%v{xmv {Y%X{xtnl][{y%e{x]{x"))
		fprintf(fp,"Prom %s~\n",	ch->prompt);

								fprintf(fp,"Race %s~\n",	pc_race_table[ch->race].name);
	if (!IS_NPC(ch))			fprintf(fp,"SpiritGuide %d\n",ch->spiritguide);
								fprintf(fp,"HeightN %d\n",	ch->looks[P_HEIGHT] ? ch->looks[P_HEIGHT] : 1);
								fprintf(fp,"WeightN %d\n",	ch->looks[P_WEIGHT] ? ch->looks[P_WEIGHT] : 1);
								fprintf(fp,"EyeN %d\n",		ch->looks[P_EYE] ? ch->looks[P_EYE] : 1);
								fprintf(fp,"HairN %d\n",	ch->looks[P_HAIR] ? ch->looks[P_HAIR] : 1);
								fprintf(fp,"SHairN %d\n",	ch->looks[P_SHAIR] ? ch->looks[P_SHAIR] : 1);
								fprintf(fp,"Sex %d\n",		ch->sex);
	if(ch->credits)				fprintf(fp,"Credits %d\n",		ch->credits);
								fprintf(fp,"BPrompt %d\n",	ch->battleprompt);
								fprintf(fp,"Class %d\n",	ch->pclass);
								fprintf(fp,"God  %d\n",		ch->god);
								fprintf(fp,"Levl %d\n",		ch->level);
	if(!ch->iscomm(CM_PROMPT))	fprintf(fp,"NoPrompt\n");
	if(ch->death_timer > 0)		fprintf(fp,"DeathTimer  %d\n",		ch->death_timer);

    if (ch->trust != 0)			fprintf( fp, "Tru  %d\n",	ch->trust	);

								fprintf(fp,"Sec  %d\n",		ch->pcdata->security);
								fprintf(fp,"Plyd %d\n",		ch->played + (int)(current_time - ch->logon));
								fprintf(fp,"Scro %d\n", 	ch->lines);
								fprintf(fp,"Room %d\n",		(ch->in_room == get_room_index(ROOM_VNUM_LIMBO) && ch->was_in_room != NULL) ? ch->was_in_room->vnum : ch->in_room == NULL ? 3001 : ch->in_room->vnum);
								fprintf(fp,"PkPdMkMd %d %d %d %d\n", ch->kills[PK],ch->kills[PD],ch->kills[MK],ch->kills[MD]);
								fprintf(fp,"HMAV  %d %d %d %d %d %d %d %d\n", ch->hit,ch->max_hit,ch->gettruemana(),ch->gettruemaxmana(),ch->gettrueantimana(),ch->gettruemaxantimana(),ch->move,ch->max_move);
								fprintf(fp,"HMAVP %d %d %d %d\n",	ch->pcdata->perm_hit,ch->pcdata->perm_mana,ch->pcdata->perm_antimana,ch->pcdata->perm_move);

	if(ch->gold)				fprintf(fp,"Gold %d\n",		ch->gold > 0 ? ch->gold : 0);
	if(ch->silver)				fprintf(fp,"Silv %d\n",		ch->silver > 0 ? ch->silver : 0);
	if(ch->bankgold)			fprintf(fp,"BankGold %d\n",	ch->bankgold > 0 ? ch->bankgold : 0);
	if(ch->banksilver)			fprintf(fp,"BankSilv %d\n",	ch->banksilver > 0 ? ch->banksilver : 0);
	if(ch->pcdata->trains)		fprintf(fp,"Trains %d\n",	ch->pcdata->trains);
	if(ch->pcdata->practices)	fprintf(fp,"Practices %d\n",ch->pcdata->practices);
	if(ch->pcdata->s_practices)	fprintf(fp,"SPractices %d\n",ch->pcdata->s_practices);
	if(ch->pcdata->studies)		fprintf(fp,"Gains %d\n",	ch->pcdata->studies);
								fprintf(fp,"Exp %d\n",		ch->exp);
								fprintf(fp,"TExp %d\n",		ch->texp);
	if (ch->lefty)				fprintf(fp,"Lefty\n");
	if(ch->pcdata->afk[0])
		fprintf(fp,"AFK %s~\n",ch->pcdata->afk);

	for (paf = ch->affected; paf != NULL; paf = afnext){
		if (paf->type < 0 || paf->type>= MAX_SKILL)
			continue;

		afnext = paf->next;

		fprintf(fp,"Affc '%s' %3d %d %3d %3d %3d %3d %10d\n",
			skill_table[paf->type].name,
			paf->where,
			paf->type,
			paf->level,
			paf->duration,
			paf->modifier,
			paf->location,
			paf->bitvector);
		//affect_remove(ch,paf);
	}
									fprintf(fp,"ActBits %s\n",	save_bits(ch->act,MAX_PLR));
									fprintf(fp,"AfBits %s\n",	save_bits(ch->affected_by,MAX_AFF));
									fprintf(fp,"CommBits %s\n",	save_bits(ch->comm,MAX_CMM));
									fprintf(fp,"WizBits %s\n",	save_bits(ch->wiz,MAX_WIZ));
									fprintf(fp,"ResPerc %s\n",	save_ints(ch->res,MAX_RES));
									fprintf(fp,"Guild %s~\n",	guilds[ch->guild].name);
									fprintf(fp,"GRank %d~\n",	ch->rank);
									fprintf(fp,"Noble %d~\n",	ch->nobility);
									fprintf(fp,"Home %s~\n",	hometowns[ch->hometown].name);
	if (ch->invis_level)			fprintf(fp,"Invi %d\n",		ch->invis_level);
	if (ch->incog_level)			fprintf(fp,"Inco %d\n",		ch->incog_level);
									fprintf(fp,"Pos  %d\n",		ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
	if (ch->saving_throw)			fprintf(fp,"Save  %d\n",	ch->saving_throw);
	if (ch->saving_spell_throw)		fprintf(fp,"SaveSP  %d\n",	ch->saving_spell_throw);
									fprintf(fp,"Alig  %d\n",	ch->alignment);
	if (ch->hitroll)				fprintf(fp,"Hit   %d\n",	ch->hitroll);
	if (ch->damroll)				fprintf(fp,"Dam   %d\n",	ch->damroll);
									fprintf(fp,"ACs %d %d %d %d\n", ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3]);
	if (ch->wimpy )					fprintf(fp,"Wimp  %d\n",ch->wimpy);
									fprintf(fp,"Stats");
	for (i = 0;i<MAX_STATS;i++)		fprintf(fp," %d",ch->perm_stat[i]);
									fprintf(fp,"\n");
									fprintf(fp,"SMod");
	for (i = 0; i < MAX_STATS; i++)	fprintf(fp," %d",ch->mod_stat[i]);
									fprintf(fp,"\n");
	if (IS_NPC(ch))				fprintf( fp, "Vnum %d\n",	ch->pIndexData->vnum	);// Do npcs even call this function?
	else{
								fprintf(fp,"Pass %s~\n",	ch->pcdata->pwd		);
	if (ch->pcdata->bamfin[0])	fprintf(fp,"Bin  %s~\n",	ch->pcdata->bamfin);
	if (ch->pcdata->bamfout[0])	fprintf(fp,"Bout %s~\n",	ch->pcdata->bamfout);
								fprintf(fp,"Titl %s~\n",		ch->pcdata->title	);
								fprintf(fp,"CREATED %s~\n",		ch->pcdata->created);
								strftime(buf,100,"%A, %B %d, %Y at %I:%M %p.",localtime(&current_time));
								fprintf(fp,"LastLogin %s~\n",		buf);
								fprintf(fp,"Pretitle %s~\n",	ch->pcdata->pretitle );
								fprintf(fp,"Pnts %d\n",   		ch->pcdata->points      );
								fprintf(fp,"TSex %d\n",			ch->pcdata->true_sex	);
								fprintf(fp,"LLev %d\n",			ch->pcdata->last_level	);
								fprintf(fp,"Cnd  %d %d %d %d\n",ch->pcdata->condition[0],ch->pcdata->condition[1],ch->pcdata->condition[2],ch->pcdata->condition[3]);
	if(ch->pcdata->s_studies[0] + ch->pcdata->s_studies[1] + ch->pcdata->s_studies[2] + ch->pcdata->s_studies[3] > 0)
								fprintf(fp,"SGains  %d %d %d %d\n",ch->pcdata->s_studies[0],ch->pcdata->s_studies[1],ch->pcdata->s_studies[2],ch->pcdata->s_studies[3]);


		/* write alias */
		for (pos = 0; pos < MAX_ALIAS; pos++){
			if (ch->pcdata->alias[pos] == NULL	||  ch->pcdata->alias_sub[pos] == NULL)
				break;
			fprintf(fp,"Alias %s %s~\n",ch->pcdata->alias[pos], ch->pcdata->alias_sub[pos]);
		}
		/* Save note board status */
		/* Save number of boards in case that number changes */
		fprintf(fp,"Boards       %d ", MAX_BOARD);
		for (i = 0; i < MAX_BOARD; ++i)
			fprintf(fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
		fprintf(fp,"\n");

		for ( sn = 0; sn < MAX_SKILL; sn++ ){
			if(IS_SKILL_NATURAL(sn) && ch->pcdata->learned[sn] < 1)
				continue;
			if (skill_table[sn].name && (ch->pcdata->learned[sn] >= 0 || ch->pcdata->skill_level[sn] >= 0 || ch->pcdata->unlocked[sn]))
				fprintf(fp,"Skl %d %d %d '%s'\n",ch->pcdata->unlocked[sn],ch->pcdata->skill_level[sn],ch->pcdata->learned[sn], skill_table[sn].name );
		}
	}
	//fprintf(fp,"SklU %s\n",		save_bits(ch->pcdata->unlocked,MAX_SKILL));

#ifdef IMC
    imc_savechar( ch, fp );
#endif
	fprintf(fp,"End\n\n");
}

void fwrite_pet( CHAR_DATA *pet, FILE *fp)
{
    AFFECT_DATA *paf;
	int i;

															fprintf(fp, "#PET\n"					);
															fprintf(fp, "Vnum %d\n",				pet->pIndexData->vnum);
															fprintf(fp, "Name %s~\n",				pet->name);
															fprintf(fp, "LogO %ld\n",				current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)	fprintf(fp, "ShD  %s~\n",				pet->short_descr);
	if (pet->long_descr != pet->pIndexData->long_descr)		fprintf(fp, "LnD  %s~\n",				pet->long_descr);
    if (pet->description != pet->pIndexData->description)	fprintf(fp, "Desc %s~\n",				pet->description);
    if (pet->race != pet->pIndexData->race)					fprintf(fp, "Race %s~\n",				race_table[pet->race].name);
															fprintf(fp, "Sex  %d\n",				pet->sex);
    if (pet->level != pet->pIndexData->level)				fprintf(fp, "Levl %d\n",				pet->level);
															fprintf(fp, "HMAV  %d %d %d %d %d %d %d %d\n", pet->hit, pet->max_hit, pet->gettruemana(), pet->gettruemaxmana(),pet->gettrueantimana(),pet->gettruemaxantimana(),pet->move, pet->max_move);
    if (pet->gold > 0)										fprintf(fp, "Gold %d\n",				pet->gold);
    if (pet->silver > 0)									fprintf(fp, "Silv %d\n",				pet->silver);
    if (pet->exp > 0)										fprintf(fp, "Exp  %d\n",				pet->exp);
	for ( paf = pet->affected; paf != NULL; paf = paf->next )
	{
		if (paf->type < 0 || paf->type >= MAX_SKILL)
    		continue;
		fprintf(fp, "Affc '%s' %3d %d %3d %3d %3d %3d %10d\n",
    		skill_table[paf->type].name,
    		paf->where, paf->type, paf->level, paf->duration, paf->modifier,paf->location,
    		paf->bitvector);
		affect_remove(pet,paf);
	}
															fprintf(fp, "Pos  %d\n",				pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
	if (pet->saving_throw != 0)								fprintf(fp, "Save %d\n",				pet->saving_throw);
	if (pet->saving_spell_throw != 0)								fprintf(fp, "SaveSP %d\n",				pet->saving_spell_throw);
	if (pet->alignment != pet->pIndexData->alignment)		fprintf(fp, "Alig %d\n",				pet->alignment);
	if (pet->hitroll != pet->pIndexData->hitroll)			fprintf(fp, "Hit  %d\n",				pet->hitroll);
	if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])fprintf(fp, "Dam  %d\n",				pet->damroll);
															fprintf(fp, "ACs  %d %d %d %d\n",		pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
	fprintf(fp,"AfBits %s\n",	save_bits(pet->affected_by,MAX_AFF));
		fprintf(fp,"Stats");
	for (i = 0;i<MAX_STATS;i++)
		fprintf(fp," %d",pet->perm_stat[i]);
	fprintf(fp,"\n");

		fprintf(fp,"SMod");
	for (i = 0;i<MAX_STATS;i++)
		fprintf(fp," %d",pet->mod_stat[i]);
		fprintf(fp,"\n");

	fprintf(fp,"End\n");
	return;
}

void fwrite_mount( CHAR_DATA *pet, FILE *fp){
    AFFECT_DATA *paf;
	int i;
    
    fprintf(fp,"#MOUNT\n");

    fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);
    
    fprintf(fp,"Name %s~\n", pet->name);
    fprintf(fp,"LogO %ld\n", current_time);
    if (pet->short_descr != pet->pIndexData->short_descr)
    	fprintf(fp,"ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->pIndexData->long_descr)
    	fprintf(fp,"LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->pIndexData->description)
    	fprintf(fp,"Desc %s~\n", pet->description);
    if (pet->race != pet->pIndexData->race)
    	fprintf(fp,"Race %s~\n", race_table[pet->race].name);
	    fprintf(fp,"Sex  %d\n", pet->sex);
    if (pet->level != pet->pIndexData->level)
    	fprintf(fp,"Levl %d\n", pet->level);
		fprintf(fp, "HMAV  %d %d %d %d %d %d %d %d\n", pet->hit, pet->max_hit, pet->gettruemana(),pet->gettruemaxmana(),pet->gettrueantimana(), pet->gettruemaxantimana(), pet->move, pet->max_move);
    if (pet->gold > 0)
    	fprintf(fp,"Gold %d\n",pet->gold);
    if (pet->silver > 0)
        fprintf(fp,"Silv %d\n",pet->silver);
    if (pet->exp > 0)
    	fprintf(fp, "Exp  %d\n", pet->exp);
	fprintf(fp,"ActBits %s\n",	save_bits(pet->act,MAX_ACT));

    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
    	if (paf->type < 0 || paf->type >= MAX_SKILL)
    	    continue;
    	    
    	fprintf(fp, "Affc '%s' %3d %d %3d %3d %3d %3d %10d\n",
    	    skill_table[paf->type].name,
    	    paf->where, paf->type, paf->level, paf->duration, paf->modifier,paf->location,
    	    paf->bitvector);
		affect_remove(pet,paf);
    }

		fprintf(fp,"Pos  %d\n", pet->position = POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
    	fprintf(fp, "Save %d\n", pet->saving_throw);
    if (pet->saving_spell_throw != 0)
    	fprintf(fp, "SaveSP %d\n", pet->saving_spell_throw);
    if (pet->alignment != pet->pIndexData->alignment)
    	fprintf(fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->pIndexData->hitroll)
    	fprintf(fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
    	fprintf(fp, "Dam  %d\n", pet->damroll);
    fprintf(fp, "ACs  %d %d %d %d\n",
    	pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);
	fprintf(fp,"Stats");
	for (i = 0;i<MAX_STATS;i++)
		fprintf(fp," %d",pet->perm_stat[i]);
	fprintf(fp,"\n");

		fprintf(fp,"SMod");
	for (i = 0;i<MAX_STATS;i++)
		fprintf(fp," %d",pet->mod_stat[i]);
		fprintf(fp,"\n");

    fprintf(fp,"End\n");
    return;
}

void save_obj(OBJ_DATA *obj,FILE *fp){
}

void fwrite_obj( CHAR_DATA *ch,OBJ_DATA *obj, FILE *fp, int iNest ){
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    // Slick recursion to write lists backwards, so loading them will load in forwards order.
    if ( obj->next_content != NULL )
		fwrite_obj( ch, obj->next_content, fp, iNest );

    // Castrate storage characters. ouch
    if(ch)
		if ( (ch->level < obj->level - 25 && obj->item_type != ITEM_CONTAINER) || obj->item_type == ITEM_KEY)
			return;

		fprintf( fp, "#O\n" );
		fprintf( fp, "Vnum %d\n",   obj->pIndexData->vnum        );

	if(!ch && obj->in_room)
		fprintf(fp, "Inrm %d\n", obj->in_room->vnum );

    if (obj->enchanted)
		fprintf( fp,"Enchanted\n");
		fprintf( fp, "Nest %d\n",	iNest	  	     );

    /* these data are only used if they do not match the defaults */

    if (obj->name != obj->pIndexData->name)		    				fprintf(fp,"Name %s~\n",obj->name);
    if (obj->short_descr != obj->pIndexData->short_descr)			fprintf(fp,"ShD  %s~\n",obj->short_descr);
    if (obj->description != obj->pIndexData->description)			fprintf(fp,"Desc %s~\n",obj->description);
    if (obj->extra_flags != obj->pIndexData->extra_flags)
        fprintf( fp, "ExtF %d\n",	obj->extra_flags	     );
	if ((obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD) && obj->armortype_flags == 0)
		obj->armortype_flags = 1;
	if (obj->armortype_flags != obj->pIndexData->armortype_flags)	fprintf(fp,"ArmT %d\n",obj->armortype_flags);
    if (obj->wear_flags != obj->pIndexData->wear_flags)				fprintf(fp,"WeaF %d\n",obj->wear_flags);
    if (obj->item_type != obj->pIndexData->item_type)				fprintf(fp,"Ityp %d\n",obj->item_type);
    if (obj->weight != obj->pIndexData->weight)						fprintf(fp,"Wt   %d\n",obj->weight);
    if (obj->condition != obj->pIndexData->condition)				fprintf(fp,"Cond %d\n",obj->condition);
    if (obj->droprate != obj->pIndexData->droprate)					fprintf(fp,"Drop %d\n",obj->droprate);
																	fprintf(fp,"Wear %d\n",obj->wear_loc);
    if (obj->level != obj->pIndexData->level)						fprintf(fp,"Lev  %d\n",obj->level);
    if (obj->timer != 0)											fprintf(fp,"Time %d\n",obj->timer);
																	fprintf(fp,"Cost %d\n",obj->cost);
																	fprintf(fp,"Mtrl %s~\n",obj->material);
																	fprintf(fp,"WFlg %s\n",save_bits(obj->wflags,MAX_WPN));
    if (obj->item_type == ITEM_WARP_STONE
	||	obj->value[0] != obj->pIndexData->value[0]
    ||  obj->value[1] != obj->pIndexData->value[1]
    ||  obj->value[2] != obj->pIndexData->value[2]
    ||  obj->value[3] != obj->pIndexData->value[3]
    ||  obj->value[4] != obj->pIndexData->value[4]) 
    																fprintf(fp,"Val  %d %d %d %d %d\n",obj->value[0],obj->value[1],obj->value[2],obj->value[3],obj->value[4]);
/*
    switch(obj->item_type){
		case ITEM_POTION:
		case ITEM_SCROLL:
		case ITEM_PILL:
			if ( obj->value[1] > 0 )
				fprintf( fp, "Spell 1 '%s'\n", skill_table[obj->value[1]].name );
			if ( obj->value[2] > 0 )
				fprintf( fp, "Spell 2 '%s'\n", skill_table[obj->value[2]].name );
			if ( obj->value[3] > 0 )
				fprintf( fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name );
			break;
		case ITEM_STAFF:
		case ITEM_WAND:
			if ( obj->value[3] > 0 )
				fprintf( fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name );
			break;
    }*/

    for(paf = obj->affected;paf;paf = paf->next){
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;
        fprintf(fp,"Affc '%s' %3d %d %3d %3d %3d %3d %10d\n",skill_table[paf->type].name,paf->where,paf->type,paf->level,paf->duration,paf->modifier,paf->location,paf->bitvector);
    }

    for(ed = obj->extra_descr;ed;ed = ed->next)
		fprintf(fp,"ExDe %s~ %s~\n",ed->keyword,ed->description);

		fprintf(fp,"End\n\n");

    if(obj->contains != NULL)
		fwrite_obj(ch,obj->contains,fp,iNest + 1);
}

bool load_char_obj( DESCRIPTOR_DATA *d, char *name ){
    char strsave[MIL],buf[100];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;

    ch = new_char();
    ch->pcdata = new_pcdata();

    d->character						= ch;
    ch->desc							= d;
    ch->name							= str_dup(name);
    ch->id								= 0;
	ch->leader							= ch;
    ch->race							= race_lookup("human");
    ch->setplr(PL_NOSUMMON);
    ch->setcomm(CM_COMBINE);
	ch->setcomm(CM_PROMPT);
    ch->prompt 							= str_dup("{x[{G%h{x/{g%H{xhp {C%m{x/{c%M{xmp {B%v{xmv {Y%X{xtnl][{y%e{x]{x");
    ch->pcdata->confirm_delete			= false;
    ch->pcdata->board					= &boards[DEFAULT_BOARD];
    ch->pcdata->pwd						= str_dup("");
    ch->pcdata->bamfin					= str_dup("");
    ch->pcdata->bamfout					= str_dup("");
    ch->pcdata->afk						= str_dup("");
	ch->pcdata->created					= str_dup("");
	ch->short_descr						= str_dup("");
	ch->hometown						= 0;
    ch->pcdata->title					= str_dup("");
	ch->pcdata->pretitle				= str_dup("");
	ch->battleprompt					= 0;
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_HUNGER]	= 48;
    ch->kills[PK]						= 0;
    ch->kills[PD]						= 0;
    ch->kills[MK]						= 0;
    ch->kills[MD]						= 0;
    ch->pcdata->security				= 0;
    ch->pcdata->eavesdropping			= -1;
	ch->pcdata->lastlogin				= str_dup("Never! Hah!");

	ch->pcdata->s_studies[0]				= 0;
	ch->pcdata->s_studies[1]				= 0;
	ch->pcdata->s_studies[2]				= 0;
	ch->pcdata->s_studies[3]				= 0;
	for(stat = 0; stat < MAX_SKILL; stat++){
		ch->pcdata->learned[stat] = -1;
		ch->pcdata->skill_level[stat] = -1;
	}
#ifdef IMC
    imc_initchar( ch );
#endif
    found = false;
    fclose(fpReserve);

    #if defined(unix)
		/* decompress if .gz file exists */
		sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
		if((fp = fopen(strsave,"r"))){
			fclose(fp);
			sprintf(buf,"gzip -dfq %s",strsave);
			system(buf);
		}
    #endif

	sprintf(strsave,"%s%s",PLAYER_DIR,capitalize(name));
	if ((fp = fopen(strsave,"r"))){
		int iNest;

		for (iNest = 0; iNest < MAX_NEST; iNest++)
			rgObjNest[iNest] = NULL;

		found = true;
		for(;;){
			char letter;
			char *word;

			letter = fread_letter(fp);
			if (letter == '*'){
				fread_to_eol(fp);
				continue;
			}

			if (letter != '#'){
				bug("Load_char_obj: # not found.",0);
				break;
			}

			word = fread_word(fp);
			if     (!str_cmp(word,"PLAYER")) fread_char(ch,fp);
			else if(!str_cmp(word,"OBJECT")) fread_obj(ch,fp);
			else if(!str_cmp(word,"O")) fread_obj(ch,fp);
			else if(!str_cmp(word,"PET")) fread_pet(ch,fp);
			else if(!str_cmp(word,"MOUNT")) fread_mount(ch,fp);
			else if(!str_cmp(word,"END")) break;
			else{
				bug("Load_char_obj: bad section.",0);
				break;
			}
		}
		fclose(fp);
	}

	fpReserve = fopen(NULL_FILE,"r");


	/* initialize race */
	if (found){
		int i;

		if (ch->race == 0)
			ch->race = race_lookup("human");

		ch->size = pc_race_table[ch->race].size;
		ch->dam_type = 17; /*punch */

/*		if(!IS_NPC(ch) && IS_IMMORTAL(ch))
			for(i = 0; i< MAX_SKILL;i++){
				ch->pcdata->unlocked[i] = true;
				ch->pcdata->learned[i] = UMAX(ch->pcdata->learned[i],100);
				ch->pcdata->skill_level[i] = UMAX(ch->pcdata->skill_level[i],5);
			}

		for (i = 0; i < 5; ++i){
			if (!pc_race_table[ch->race].skills[i])
				break;
			group_add(ch,pc_race_table[ch->race].skills[i],false,false,true);dead
		}*/
	}

	return found;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY(literal,field,value) \
		if (!str_cmp(word,literal))		\
		{	\
			field  = value;		\
			fMatch = true;		\
			break;		\
		}

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS(literal,field,value) if (!str_cmp(word,literal)) { free_string(field); field  = value; fMatch = true; break; }

void fread_char(CHAR_DATA *ch,FILE *fp){
	char buf[MSL], *word;
	bool fMatch;
	int count = 0,i,lastlogoff = current_time, percent;

	sprintf(buf,"SLoading %s.",ch->name);
	log_string(buf);

	for ( ; ; ){
		word   = feof(fp) ? (char *)"End" : fread_word(fp);
		fMatch = false;

		switch (UPPER(word[0])){
			case '*':
				fMatch = true;
				fread_to_eol(fp);
				break;
			case 'A':
				KEY("Alignment",	ch->alignment,		fread_number(fp));
				KEY("Alig",			ch->alignment,		fread_number(fp));
				KEY("AFK",ch->pcdata->afk,fread_string(fp));

				if (!str_cmp(word,"AfBits"))
					read_bits(ch->affected_by,MAX_AFF,fread_string(fp));
				if (!str_cmp(word,"ActBits"))
					read_bits(ch->act,MAX_PLR,fread_string(fp));

				if (!str_cmp( word, "Alia")){
					if (count >= MAX_ALIAS){
						fread_to_eol(fp);
						fMatch = true;
						break;
					}
					ch->pcdata->alias[count] 	= str_dup(fread_word(fp));
					ch->pcdata->alias_sub[count]	= str_dup(fread_word(fp));
					count++;
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"Alias")){
					if (count >= MAX_ALIAS){
						fread_to_eol(fp);
						fMatch = true;
						break;
					}

					ch->pcdata->alias[count]        = str_dup(fread_word(fp));
					ch->pcdata->alias_sub[count]    = fread_string(fp);
					count++;
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"AC") || !str_cmp(word,"Armor")){
					fread_to_eol(fp);
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"ACs")){
					for (i = 0; i < 4; ++i)
						ch->armor[i] = fread_number(fp);
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"Affc")){
					AFFECT_DATA *paf;
					int sn;

					paf = new_affect();

					sn = skill_lookup(fread_word(fp));
					if (sn < 0)
						bug("Fread_char: unknown skill.",0);
					else
						paf->type = sn;

					paf->where		= fread_number(fp);
					paf->type		= fread_number(fp);
					paf->level      = fread_number(fp);
					paf->duration   = fread_number(fp);
					paf->modifier   = fread_number(fp);
					paf->location   = fread_number(fp);
					paf->bitvector  = fread_number(fp);
					affect_to_char(ch,paf);
					/*paf->next       = ch->affected;
					ch->affected    = paf;
					ch->setaff(paf->bitvector);*/
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"AMod")){
					int stat,tnum;
					for (stat = 0; stat < 15; stat++){
						tnum = fread_number(fp);
						if(stat == 0)//str
							ch->mod_stat[STAT_STR] = tnum;
						else if(stat == 1)//int
							ch->mod_stat[STAT_INT] = tnum;
						else if(stat == 2)//fcs
							ch->mod_stat[STAT_AGI] = tnum;
						else if(stat == 3)//dex
							ch->mod_stat[STAT_AGI] = tnum;
						else if(stat == 4)//end
							ch->mod_stat[STAT_END] = tnum;
						else if(stat == 5)//spd
							ch->mod_stat[STAT_AGI] = (ch->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 6)//skl
							ch->mod_stat[STAT_WIS] = (ch->mod_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 7)//res
							ch->mod_stat[STAT_RES] = tnum;
						else if(stat == 8)//itu
							ch->mod_stat[STAT_WIS] = (ch->mod_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 9)//asp
							ch->mod_stat[STAT_AGI] = (ch->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 10)//lck
							ch->mod_stat[STAT_LCK] = tnum;
						else if(stat == 11)//fth
							ch->mod_stat[STAT_FTH] = tnum;
						else if(stat == 12)//acc
							ch->mod_stat[STAT_AGI] = (ch->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 13)//cha
							ch->mod_stat[STAT_CHA] = tnum;
						else if(stat == 14)//ldr
							ch->mod_stat[STAT_CHA] = (ch->mod_stat[STAT_CHA] + tnum) / 2;
						else
							ch->perm_stat[stat] = tnum;
					}
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"Attr")){
					int stat,tnum;
					for (stat = 0; stat < 15; stat++){
						tnum = fread_number(fp);
						log_f("stat %d tnum %d",stat,tnum);
						if(stat == 0)//str
							ch->perm_stat[STAT_STR] = tnum;
						else if(stat == 1)//int
							ch->perm_stat[STAT_INT] = tnum;
						if(stat == 2)//fcs
							ch->perm_stat[STAT_WIS] = tnum;
						else if(stat == 3)//dex
							ch->perm_stat[STAT_AGI] = tnum;
						else if(stat == 4)//end
							ch->perm_stat[STAT_END] = tnum;
						else if(stat == 5)//spd
							ch->perm_stat[STAT_AGI] = (ch->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 6)//skl
							ch->perm_stat[STAT_WIS] = (ch->perm_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 7)//res
							ch->perm_stat[STAT_RES] = tnum;
						else if(stat == 8)//itu
							ch->perm_stat[STAT_WIS] = (ch->perm_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 9)//asp
							ch->perm_stat[STAT_AGI] = (ch->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 10)//lck
							ch->perm_stat[STAT_LCK] = tnum;
						else if(stat == 11)//fth
							ch->perm_stat[STAT_FTH] = tnum;
						else if(stat == 12)//acc
							ch->perm_stat[STAT_AGI] = (ch->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 13)//cha
							ch->perm_stat[STAT_CHA] = tnum;
						else if(stat == 14)//ldr
							ch->perm_stat[STAT_CHA] = (ch->perm_stat[STAT_CHA] + tnum) / 2;
						else
							ch->perm_stat[stat] = tnum;
					}
					fMatch = true;
					break;
				}
				break;
			case 'B':
				KEY("BankGold",	ch->bankgold,			fread_number(fp));
				KEY("BankSilv",	ch->banksilver,			fread_number(fp));
				KEY("Bin",		ch->pcdata->bamfin,		fread_string(fp));
				KEY("Bout",		ch->pcdata->bamfout,	fread_string(fp));
				KEY("BPrompt",	ch->battleprompt,		fread_number(fp));

				if (!str_cmp(word, "Boards" ))
				{
					int num = fread_number (fp);
					char *boardname;

					for (; num ; num-- )
					{
						boardname = fread_word (fp);
						i = board_lookup(boardname);

						if (i == BOARD_NOTFOUND)
						{
							sprintf (buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
							log_string (buf);
							fread_number (fp);
						}
						else
							ch->pcdata->last_note[i] = fread_number (fp);
					}
					fMatch = true;
				}
				break;
			case 'C':
				KEY("CREDITS",			ch->credits,				fread_number(fp));
				KEY("CREATED",		ch->pcdata->created, fread_string(fp));
				if (!str_cmp(word,"CommBits"))
					read_bits(ch->comm,MAX_CMM,fread_string(fp));

				if(!str_cmp(word,"Class")){
					if (ch->version < 6){
						ch->pclass	= fread_number(fp);
						fread_number(fp);
						fread_number(fp);
						ch->version++;
					}
					else
						ch->pclass = fread_number(fp);
					break;
				}
				if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond"))
				{
					ch->pcdata->condition[0] = fread_number(fp);
					ch->pcdata->condition[1] = fread_number(fp);
					ch->pcdata->condition[2] = fread_number(fp);
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"Cnd"))
				{
					ch->pcdata->condition[0] = fread_number(fp);
					ch->pcdata->condition[1] = fread_number(fp);
					ch->pcdata->condition[2] = fread_number(fp);
					ch->pcdata->condition[3] = fread_number(fp);
					fMatch = true;
					break;
				}
				break;
			case 'D':
				KEY("Dam",			ch->damroll,		fread_number(fp));
				KEY("DeathTimer",			ch->death_timer,		fread_number(fp));
				KEY("Desc",		ch->description,	fread_string(fp));
				break;
			case 'E':
				KEY( "EyeN",	ch->looks[P_EYE],	fread_number(fp));
				KEY( "Exp",		ch->exp, fread_number(fp));
				if ( !str_cmp(word,"End") )
				{
    				/* adjust hp mana move up  -- here for speed's sake */
    				percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

					percent = UMIN(percent,100);
 
    				if (percent > 0 && !ch->isaff(AF_POISON) && !ch->isaff(AF_PLAGUE))
    				{
        				ch->hit	 += (ch->max_hit - ch->hit) * percent / 100;
        				ch->modmana((ch->getmaxmana() - ch->getmana()) * percent / 100);
        				ch->move += (ch->max_move - ch->move)* percent / 100;
    				}
					init_race(ch);
					//impose_job(ch,ch->pclass);
					return;
				}
				break;
			case 'G':
				KEY("Gains",    ch->pcdata->studies, fread_number(fp));
				KEY("God",	ch->god,	fread_number(fp));
				KEY("Gold",	ch->gold,	fread_number(fp));
				KEY("GRank",	ch->rank,		fread_number(fp));
				if(!str_cmp(word,"Guild")){
					if((ch->guild = guild_lookup(fread_string(fp))) == -1)
						ch->guild = 0;
				}
			case 'H':
				KEY("HairN",	ch->looks[P_HAIR],		fread_number( fp ) );
				KEY("HeightN",	ch->looks[P_HEIGHT],	fread_number( fp ) );
				KEY("Hit",		ch->hitroll,			fread_number( fp ) );
				if(!str_cmp(word,"Home"))
					ch->hometown = home_lookup(fread_string(fp));

				if (!str_cmp(word,"HMAV")){
					ch->hit		  = fread_number(fp);
					ch->max_hit	  = fread_number(fp);
					ch->settruemana(fread_number(fp));
					ch->settruemaxmana(fread_number(fp));
					ch->settrueantimana(fread_number(fp));
					ch->settruemaxantimana(fread_number(fp));
					ch->move	= fread_number( fp );
					ch->max_move	= fread_number( fp );
					fMatch = true;
					break;
				}

				if(!str_cmp(word,"HpManaMovePerm") || !str_cmp(word,"HMAVP")){
					ch->pcdata->perm_hit		= fread_number(fp);
					ch->max_hit = ch->pcdata->perm_hit;
					ch->pcdata->perm_mana		= fread_number(fp);
					ch->max_mana = ch->pcdata->perm_mana;
					ch->pcdata->perm_antimana	= fread_number(fp);
					ch->max_antimana = ch->pcdata->perm_antimana;
					ch->pcdata->perm_move		= fread_number(fp);
					fMatch = true;
					break;
				}
				break;
			case 'I':
				KEY("Id",			ch->id,					fread_number(fp));
				KEY("Inco",			ch->incog_level,		fread_number(fp));
				KEY("Invi",			ch->invis_level,		fread_number(fp));
#ifdef IMC
           if( ( fMatch = imc_loadchar( ch, fp, word ) ) )
                break;
#endif
				break;
			case 'L':
				KEY("LLev",			ch->pcdata->last_level, fread_number(fp));
				KEY("Level",		ch->level,				fread_number(fp));
				KEY("Lev",			ch->level,				fread_number(fp));
				KEY("Levl",			ch->level,				fread_number(fp));
				KEY("LogO",			lastlogoff,				fread_number(fp));
				KEY("LnD",			ch->long_descr,			fread_string(fp));
				KEY("LastLogin",		ch->pcdata->lastlogin,	fread_string(fp));
				if (!str_cmp(word,"Lefty"))
					ch->lefty = true;

				if (!str_cmp(word,"LName")){
					ch->pcdata->lname = fread_string(fp);
					if (ch->pcdata->lname[0] != '.'
						&& ch->pcdata->pretitle[0] != ','
						&& ch->pcdata->pretitle[0] != '!'
						&& ch->pcdata->pretitle[0] != '?'){
						sprintf(buf,"%s",ch->pcdata->lname);
						free_string(ch->pcdata->lname);
						ch->pcdata->lname = str_dup(buf);
					}
					fMatch = true;
					break;
				}
				break;
			case 'N':
				KEYS("Name",		ch->name,				fread_string(fp));
				KEY("Noble",		ch->nobility,			fread_number(fp));
				if (!str_cmp(word,"NoPrompt")) ch->remcomm(CM_PROMPT);
				break;
			case 'P':
				KEY("Pass",			ch->pcdata->pwd,		fread_string(fp));
				KEY("Plyd",			ch->played,				fread_number(fp));
				KEY("Pnts",			ch->pcdata->points,		fread_number(fp));
				KEY("Pos",			ch->position,			fread_number(fp));
				KEY("Prac",			ch->wimpy,						fread_number(fp));
 			        KEY("Practices",                ch->pcdata->practices,         fread_number(fp));
                            	KEY("Prom",			ch->prompt,				fread_string(fp));
				if (!str_cmp(word,"PkPdMkMd"))
				{
					ch->kills[PK]	= fread_number(fp);
					ch->kills[PD]	= fread_number(fp);
					ch->kills[MK]	= fread_number(fp);
					ch->kills[MD]	= fread_number(fp);
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"pretitle")) 
				{ 
					ch->pcdata->pretitle = fread_string( fp );
					if (ch->pcdata->pretitle[0] != '.' 
						&& ch->pcdata->pretitle[0] != ',' 
						&& ch->pcdata->pretitle[0] != '!' 
						&& ch->pcdata->pretitle[0] != '?') 
					{ 
						sprintf( buf, "%s", ch->pcdata->pretitle );
						free_string( ch->pcdata->pretitle );
						ch->pcdata->pretitle = str_dup( buf );
					} 
					fMatch = true;
					break;
				}  

				break;
			case 'R':
				KEY( "Race",	ch->race,	race_lookup(fread_string( fp )) );
				if (!str_cmp(word,"ResPercs"))
					read_ints(ch->res,MAX_RES,fread_string(fp));

				if (!str_cmp(word,"Room"))
				{
					ch->in_room = get_room_index( fread_number(fp));
					if (ch->in_room == NULL)
						ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
					fMatch = true;
					break;
				}

				break;
			case 'S':
				KEY("Save",			ch->saving_throw,		fread_number(fp));
				KEY("SaveSP",		ch->saving_spell_throw,	fread_number(fp));
				KEY("Scro",			ch->lines,				fread_number(fp));
				KEY("Sex",			ch->sex,				fread_number(fp));
				KEY("ShD",			ch->short_descr,		fread_string(fp));
				KEY("Sec",			ch->pcdata->security,	fread_number(fp));
				KEY("Silv",			ch->silver,             fread_number(fp));
				KEY("SpiritGuide",	ch->spiritguide,		fread_number(fp));
				KEY("SHairN",		ch->looks[P_SHAIR],		fread_number(fp));
 				KEY("SPractices",	ch->pcdata->s_practices,fread_number(fp));
				if (!str_cmp(word,"SklU"))
					read_bits(ch->pcdata->unlocked,MAX_SKILL,fread_string(fp));

				if (!str_cmp(word,"SGains")){
					ch->pcdata->s_studies[0] = fread_number(fp);
					ch->pcdata->s_studies[1] = fread_number(fp);
					ch->pcdata->s_studies[2] = fread_number(fp);
					ch->pcdata->s_studies[3] = fread_number(fp);
					fMatch = true;
				}

				if (!str_cmp(word,"Sk")){
					int sn;
					int value,value2;
					char *temp;

					value2 = fread_number(fp);
					value = fread_number(fp);
					if (value > 0 && value2 == 0)
						value2 = 1;
					temp = fread_word(fp) ;
					sn = skill_lookup(temp);

					if ( sn < 0 ){
						fprintf(stderr,"%s",temp);
						bug("Fread_char: unknown skill. ",0);
					}
					else{
						ch->pcdata->up_skill(value2,value,sn);
						ch->pcdata->unlocked[sn] = true;
					}

					fMatch = true;
				}

				if (!str_cmp(word,"Skl")){
					int sn,value,value2,vunlocked;
					char *temp;

					vunlocked = fread_number(fp);
					value2 = fread_number(fp);
					value = fread_number(fp);
					if (value > 0 && value2 == 0)
						value2 = 1;
					temp = fread_word(fp) ;
					sn = skill_lookup(temp);

					if ( sn < 0 ){
						fprintf(stderr,"%s",temp);
						bug("Fread_char: unknown skill. ",0);
					}
					else{
						ch->pcdata->up_skill(value2,value,sn);
						ch->pcdata->unlocked[sn] = vunlocked;
					}
					fMatch = true;
				}
				if (!str_cmp(word,"SMod")){
					int stat;
					for (stat = 0; stat < MAX_STATS; stat++)
						ch->mod_stat[stat] = fread_number(fp);
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"Stats")){
					int stat;
					for (stat = 0; stat < MAX_STATS; stat++)
						ch->perm_stat[stat] = fread_number(fp);
					fMatch = true;
					break;
				}
				break;
			case 'T':
				KEY("TExp",		ch->texp, fread_number(fp));
				KEY("Trains",		ch->pcdata->trains,			fread_number(fp));
				KEY("TSex",			ch->pcdata->true_sex,   fread_number(fp) );
				KEY("Tru",			ch->trust,				fread_number(fp) );

				if (!str_cmp(word,"Titl")){
					ch->pcdata->title = fread_string(fp);
    				if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 	&&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?')
					{
						sprintf(buf," %s",ch->pcdata->title);
						free_string(ch->pcdata->title);
						ch->pcdata->title = str_dup(buf);
					}
					fMatch = true;
					break;
				}
				break;
			case 'V':
				KEY("Vers",		ch->version,			fread_number(fp) );
				if (!str_cmp(word,"Vnum")){
					ch->pIndexData = get_mob_index( fread_number( fp ) );
					fMatch = true;
					break;
				}
				break;
			case 'W':
				KEY( "WeightN",		ch->looks[P_WEIGHT],fread_number(fp) );
				if (!str_cmp(word,"WizBits"))
					read_bits(ch->wiz,MAX_WIZ,fread_string(fp));
				KEY("WP",			ch->credits,				fread_number(fp));
				break;
		}
	}
}

void fread_pet( CHAR_DATA *ch, FILE *fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;
    int lastlogoff = current_time;
    int percent;

    word = feof(fp) ? (char*) "END" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
    	int vnum;
    	
    	vnum = fread_number(fp);
    	if (get_mob_index(vnum) == NULL)
		{
    		bug("Fread_pet: bad vnum %d.",vnum);
			pet = create_mobile(get_mob_index(1));
		}
    	else
    	    pet = create_mobile(get_mob_index(vnum));
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile(get_mob_index(1));
    }
    
    for ( ; ; )
    {
    	word 	= feof(fp) ? (char*)"END" : fread_word(fp);
    	fMatch = false;
    	
		switch (UPPER(word[0]))
		{
			case '*':
				fMatch = true;
				fread_to_eol(fp);
				break;
			case 'A':
				KEY("Alig",	pet->alignment,		fread_number(fp));
				if(!strcmp(word,"AfBits"))		read_bits(pet->affected_by,MAX_AFF,fread_string(fp));

				if (!str_cmp(word,"ACs"))
				{
					int i;

					for (i = 0; i < 4; ++i)
					pet->armor[i] = fread_number(fp);
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"Affc"))
				{
					AFFECT_DATA paf;
					int sn;

					sn = skill_lookup(fread_word(fp));
					if (sn < 0)
						bug("Fread_char: unknown skill.",0);
					else
						paf.type = sn;

					paf.where		= fread_number(fp);
					paf.type		= fread_number(fp);
					paf.level      = fread_number(fp);
					paf.duration   = fread_number(fp);
					paf.modifier   = fread_number(fp);
					paf.location   = fread_number(fp);
					paf.bitvector  = fread_number(fp);
					affect_to_char(pet,&paf);
					fMatch          = true;
					break;
				}

				if (!str_cmp(word,"AMod")){
					int stat,tnum;
					for (stat = 0; stat < 15; stat++){
						tnum = fread_number(fp);
						if(stat == 0)//str
							pet->mod_stat[STAT_STR] = tnum;
						else if(stat == 1)//int
							pet->mod_stat[STAT_INT] = tnum;
						else if(stat == 2)//fcs
							pet->mod_stat[STAT_AGI] = tnum;
						else if(stat == 3)//dex
							pet->mod_stat[STAT_AGI] = tnum;
						else if(stat == 4)//end
							pet->mod_stat[STAT_END] = tnum;
						else if(stat == 5)//spd
							pet->mod_stat[STAT_AGI] = (pet->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 6)//skl
							pet->mod_stat[STAT_WIS] = (pet->mod_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 7)//res
							pet->mod_stat[STAT_RES] = tnum;
						else if(stat == 8)//itu
							pet->mod_stat[STAT_WIS] = (pet->mod_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 9)//asp
							pet->mod_stat[STAT_AGI] = (pet->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 10)//lck
							pet->mod_stat[STAT_LCK] = tnum;
						else if(stat == 11)//fth
							pet->mod_stat[STAT_FTH] = tnum;
						else if(stat == 12)//acc
							pet->mod_stat[STAT_AGI] = (pet->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 13)//peta
							pet->mod_stat[STAT_CHA] = tnum;
						else if(stat == 14)//ldr
							pet->mod_stat[STAT_CHA] = (pet->mod_stat[STAT_CHA] + tnum) / 2;
						else
							pet->perm_stat[stat] = tnum;
					}
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"Attr")){
					int stat,tnum;
					for (stat = 0; stat < 15; stat++){
						tnum = fread_number(fp);
						if(stat == 0)//str
							pet->perm_stat[STAT_STR] = tnum;
						else if(stat == 1)//int
							pet->perm_stat[STAT_INT] = tnum;
						if(stat == 2)//fcs
							pet->perm_stat[STAT_WIS] = tnum;
						else if(stat == 3)//dex
							pet->perm_stat[STAT_AGI] = tnum;
						else if(stat == 4)//end
							pet->perm_stat[STAT_END] = tnum;
						else if(stat == 5)//spd
							pet->perm_stat[STAT_AGI] = (pet->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 6)//skl
							pet->perm_stat[STAT_WIS] = (pet->perm_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 7)//res
							pet->perm_stat[STAT_RES] = tnum;
						else if(stat == 8)//itu
							pet->perm_stat[STAT_WIS] = (pet->perm_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 9)//asp
							pet->perm_stat[STAT_AGI] = (pet->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 10)//lck
							pet->perm_stat[STAT_LCK] = tnum;
						else if(stat == 11)//fth
							pet->perm_stat[STAT_FTH] = tnum;
						else if(stat == 12)//acc
							pet->perm_stat[STAT_AGI] = (pet->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 13)//cha
							pet->perm_stat[STAT_CHA] = tnum;
						else if(stat == 14)//ldr
							pet->perm_stat[STAT_CHA] = (pet->perm_stat[STAT_CHA] + tnum) / 2;
						else
							pet->perm_stat[stat] = tnum;
					}
					fMatch = true;
					break;
				}
				break;
			case 'C':
				if(!strcmp(word,"CommBits"))
					read_bits(pet->comm,MAX_CMM,fread_string(fp));
				break;
			case 'D':
				KEY("Dam",	pet->damroll,		fread_number(fp));
				KEY("Desc",	pet->description,	fread_string(fp));
				break;
			case 'E':
				KEY("Exp",	pet->exp,		fread_number(fp));
				if (!str_cmp(word,"End"))
				{
					group_char(ch,pet);
					pet->master = ch;
					ch->pet = pet;
					pet->alignment				= ch->alignment;
					init_race(pet);
					
					if(is_class(ch,CLASS_IMMORTAL))
						pet->setform(FRM_INSTANT_DECAY);//Cheap catch for necros
					/* adjust hp mana move up  -- here for speed's sake */
					percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

					if (percent > 0 && !ch->isaff(AF_POISON) && !ch->isaff(AF_PLAGUE))
					{
						percent = UMIN(percent,100);
						pet->hit	+= (pet->max_hit - pet->hit) * percent / 100;
						pet->setmana((pet->getmaxmana() - pet->getmana()) * percent / 100);
						pet->move   += (pet->max_move - pet->move)* percent / 100;
					}
					return;
				}
				break;
			case 'G':
				KEY("Gold",	pet->gold,		fread_number(fp));
				break;
			case 'H':
				KEY("Hit",	pet->hitroll,		fread_number(fp));

				if (!str_cmp(word,"HMAV"))
				{
					pet->hit	= fread_number(fp);
					pet->max_hit	= fread_number(fp);
					pet->settruemana(fread_number(fp));
					pet->settruemaxmana(fread_number(fp));
					pet->settrueantimana(fread_number(fp));
					pet->settruemaxantimana(fread_number(fp));
					pet->move	= fread_number(fp);
					pet->max_move	= fread_number(fp);
					fMatch = true;
					break;
				}
				break;
			case 'L':
				KEY("Levl",	pet->level,		fread_number(fp));
				KEY("LnD",	pet->long_descr,	fread_string(fp));
				KEY("LogO",	lastlogoff,		fread_number(fp));
				break;
			case 'N':
				KEY("Name",	pet->name,		fread_string(fp));
				break;
			case 'O':
				break;
			case 'P':
				KEY("Pos", pet->position, fread_number(fp));
				break;
			case 'R':
				KEY("Race", pet->race, race_lookup(fread_string(fp)));
				break;
			case 'S' :
				KEY("Save",		pet->saving_throw,	fread_number(fp));
				KEY("SaveSP",		pet->saving_spell_throw,	fread_number(fp));
				KEY("Sex",		pet->sex,			fread_number(fp));
				KEY("ShD",		pet->short_descr,	fread_string(fp));
				KEY("Silv",		pet->silver,		fread_number(fp));
				if (!str_cmp(word,"SMod")){
					int stat;
					for (stat = 0; stat < MAX_STATS; stat++)
						pet->mod_stat[stat] = fread_number(fp);
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"Stats")){
					int stat;
					for (stat = 0; stat < MAX_STATS; stat++)
						pet->perm_stat[stat] = fread_number(fp);
					fMatch = true;
					break;
				}
				break;

				if (!fMatch)
				{
					bug("Fread_pet: no match.",0);
					fread_to_eol(fp);
				}
				break;
		}
	}
}

void fread_mount(CHAR_DATA *ch,FILE *fp){
	char *word,  fMatch;
	CHAR_DATA *pet;
	int lastlogoff = current_time, percent;

	/* first entry had BETTER be the vnum or we barf */
	word = feof(fp) ? (char*)"END" : fread_word(fp);
	if (!str_cmp(word,"Vnum")){
		int vnum;
    
		vnum = fread_number(fp);
		if (get_mob_index(vnum) == NULL){
    		bug("Fread_mount: bad vnum %d.",vnum);
			pet = create_mobile(get_mob_index(1));
		}
		else
    		pet = create_mobile(get_mob_index(vnum));
	}
	else{
		bug("Fread_mount: no vnum in file.",0);
		pet = create_mobile(get_mob_index(1));
	}

	for (;;){
		word = feof(fp) ? (char*)"END" : fread_word(fp);
		fMatch = false;
    
		switch (UPPER(word[0])){
    		case '*':
				fMatch = true;
				fread_to_eol(fp);
				break;
    		case 'A':
				//if(!strcmp(word, "Act"))pet->act = fread_flag(fp);

				if(!strcmp(word,"AfBits"))		read_bits(pet->affected_by,MAX_AFF,fread_string(fp));
				if(!strcmp(word,"ActBits"))		read_bits(pet->act,MAX_ACT,fread_string(fp));

				KEY("Alig",		pet->alignment,		fread_number(fp));

				if (!str_cmp(word,"ACs"))
				{
					int i;

					for (i = 0; i < 4; ++i)
						pet->armor[i] = fread_number(fp);
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"Affc"))
				{
					AFFECT_DATA paf;
					int sn;

					sn = skill_lookup(fread_word(fp));
					if (sn < 0)
						bug("Fread_char: unknown skill.",0);
					else
						paf.type = sn;

					paf.where		= fread_number(fp);
					paf.type		= fread_number(fp);
					paf.level		= fread_number(fp);
					paf.duration	= fread_number(fp);
					paf.modifier	= fread_number(fp);
					paf.location	= fread_number(fp);
					paf.bitvector	= fread_number(fp);
					affect_to_char(pet,&paf);
					fMatch			= true;
					break;
				}

				if (!str_cmp(word,"AMod")){
					int stat,tnum;
					for (stat = 0; stat < 15; stat++){
						tnum = fread_number(fp);
						if(stat == 0)//str
							pet->mod_stat[STAT_STR] = tnum;
						else if(stat == 1)//int
							pet->mod_stat[STAT_INT] = tnum;
						else if(stat == 2)//fcs
							pet->mod_stat[STAT_AGI] = tnum;
						else if(stat == 3)//dex
							pet->mod_stat[STAT_AGI] = tnum;
						else if(stat == 4)//end
							pet->mod_stat[STAT_END] = tnum;
						else if(stat == 5)//spd
							pet->mod_stat[STAT_AGI] = (pet->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 6)//skl
							pet->mod_stat[STAT_WIS] = (pet->mod_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 7)//res
							pet->mod_stat[STAT_RES] = tnum;
						else if(stat == 8)//itu
							pet->mod_stat[STAT_WIS] = (pet->mod_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 9)//asp
							pet->mod_stat[STAT_AGI] = (pet->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 10)//lck
							pet->mod_stat[STAT_LCK] = tnum;
						else if(stat == 11)//fth
							pet->mod_stat[STAT_FTH] = tnum;
						else if(stat == 12)//acc
							pet->mod_stat[STAT_AGI] = (pet->mod_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 13)//cha
							pet->mod_stat[STAT_CHA] = tnum;
						else if(stat == 14)//ldr
							pet->mod_stat[STAT_CHA] = (pet->mod_stat[STAT_CHA] + tnum) / 2;
						else
							pet->perm_stat[stat] = tnum;
					}
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"Attr")){
					int stat,tnum;
					for (stat = 0; stat < 15; stat++){
						tnum = fread_number(fp);
						if(stat == 0)//str
							pet->perm_stat[STAT_STR] = tnum;
						else if(stat == 1)//int
							pet->perm_stat[STAT_INT] = tnum;
						if(stat == 2)//fcs
							pet->perm_stat[STAT_WIS] = tnum;
						else if(stat == 3)//dex
							pet->perm_stat[STAT_AGI] = tnum;
						else if(stat == 4)//end
							pet->perm_stat[STAT_END] = tnum;
						else if(stat == 5)//spd
							pet->perm_stat[STAT_AGI] = (pet->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 6)//skl
							pet->perm_stat[STAT_WIS] = (pet->perm_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 7)//res
							pet->perm_stat[STAT_RES] = tnum;
						else if(stat == 8)//itu
							pet->perm_stat[STAT_WIS] = (pet->perm_stat[STAT_WIS] + tnum) / 2;
						else if(stat == 9)//asp
							pet->perm_stat[STAT_AGI] = (pet->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 10)//lck
							pet->perm_stat[STAT_LCK] = tnum;
						else if(stat == 11)//fth
							pet->perm_stat[STAT_FTH] = tnum;
						else if(stat == 12)//acc
							pet->perm_stat[STAT_AGI] = (pet->perm_stat[STAT_AGI] + tnum) / 2;
						else if(stat == 13)//cha
							pet->perm_stat[STAT_CHA] = tnum;
						else if(stat == 14)//ldr
							pet->perm_stat[STAT_CHA] = (pet->perm_stat[STAT_CHA] + tnum) / 2;
						else
							pet->perm_stat[stat] = tnum;
					}
					fMatch = true;
					break;
				}				break;
			case 'C':
				if(!strcmp(word,"CommBits"))
					read_bits(pet->comm,MAX_CMM,fread_string(fp));
				break;
			case 'D':
				KEY("Dam",		pet->damroll,				fread_number(fp));
				KEY("Desc",		pet->description,			fread_string(fp));
				break;
			case 'E':
				if (!str_cmp(word,"End")){
					ch->mount = pet;
					pet->mount = ch;
					pet->leader = ch;
					pet->master = ch;
					pet->ismount = true;
					/* adjust hp mana move up  -- here for speed's sake */
					percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

					if (percent > 0 && !ch->isaff(AF_POISON) && !ch->isaff(AF_PLAGUE))
					{
						percent = UMIN(percent,100);
						pet->hit    += (pet->max_hit - pet->hit) * percent / 100;
						pet->modmana((pet->getmaxmana() - pet->getmana()) * percent / 100);
						pet->move   += (pet->max_move - pet->move)* percent / 100;
					}
					init_race(pet);
					if(pet->in_room != ch->in_room)
						ch->send("Your mount is in another room.\n\r");
					return;
				}
				KEY( "Exp",		pet->exp,					fread_number(fp));
				break;
			case 'G':
				KEY( "Gold",	pet->gold,					fread_number(fp));
				break;
			case 'H':
				KEY("Hit",		pet->hitroll,				fread_number(fp));

				if (!str_cmp(word,"HMAV"))
				{
					pet->hit			= fread_number(fp);
					pet->max_hit		= fread_number(fp);
					pet->settruemana(fread_number(fp));
					pet->settruemaxmana(fread_number(fp));
					pet->settrueantimana(fread_number(fp));
					pet->settruemaxantimana(fread_number(fp));
					pet->move			= fread_number(fp);
					pet->max_move		= fread_number(fp);
					fMatch = true;
					break;
				}
				break;
			case 'L':
				KEY("Levl",		pet->level,					fread_number(fp));
				KEY("LnD",		pet->long_descr,			fread_string(fp));
				KEY("LogO",		lastlogoff,					fread_number(fp));
				break;
    		case 'N':
				KEY("Name",		pet->name,					fread_string(fp));
				break;
			case 'O':
				break;
			case 'P':
				KEY("Pos",		pet->position,				fread_number(fp));
				break;
			case 'R':
				KEY("Race",		pet->race,					race_lookup(fread_string(fp)));
				break;
    		case 'S' :
				KEY("SaveSP",	pet->saving_spell_throw,	fread_number(fp));
				KEY("Sex",		pet->sex,					fread_number(fp));
				KEY("ShD",		pet->short_descr,			fread_string(fp));
				KEY("Silv",		pet->silver,				fread_number(fp));
				if (!str_cmp(word,"SMod")){
					int stat;
					for (stat = 0; stat < MAX_STATS; stat++)
						pet->mod_stat[stat] = fread_number(fp);
					fMatch = true;
					break;
				}

				if (!str_cmp(word,"Stats")){
					int stat;
					for (stat = 0; stat < MAX_STATS; stat++)
						pet->perm_stat[stat] = fread_number(fp);
					fMatch = true;
					break;
				}
				break;

				if ( !fMatch )
				{
					bug("Fread_mount: no match.",0);
					fread_to_eol(fp);
				}
				break;
		}
	}
}

extern	OBJ_DATA	*obj_free;

void fread_obj(CHAR_DATA *ch,FILE *fp){
	OBJ_DATA *obj;
	char *word;
	int iNest;
	bool fMatch, fNest, fVnum, first, new_format, make_new;

	fVnum = false;
	obj = NULL;
	first = true;
	new_format = false;
	make_new = false;

	word   = feof(fp) ?(char*) "End" : fread_word(fp);
	if (!str_cmp(word,"Vnum")){
		int vnum;
		first = false;

		vnum = fread_number(fp);
		if ( get_obj_index(vnum)  == NULL)
			bug("Fread_obj: bad vnum %d.",vnum);
		else{
			obj = create_object(get_obj_index(vnum),-1);
			new_format = true;
		}
	}

	if (obj == NULL){/* either not found or old style */
		obj = new_obj();
		obj->name			= str_dup("");
		obj->short_descr	= str_dup("");
		obj->description	= str_dup("");
	}

	fNest		= false;
	fVnum		= true;
	iNest		= 0;

	for (;;){
		if (first)
			first = false;
		else
			word   = feof(fp) ? (char*)"End" : fread_word(fp);
		fMatch = false;

		switch (UPPER(word[0])){
			case '*':
				fMatch = true;
				fread_to_eol(fp);
				break;
			case 'A':
				KEY("ArmT",	obj->armortype_flags,	fread_number(fp));
				if (!str_cmp(word,"Affc")){
					AFFECT_DATA paf;
					int sn;

					sn = skill_lookup(fread_word(fp));
					if (sn < 0)
						bug("Fread_obj: unknown skill.",0);
					else
						paf.type = sn;

					paf.where		= fread_number(fp);
					paf.type		= fread_number(fp);
					paf.level		= fread_number(fp);
					paf.duration	= fread_number(fp);
					paf.modifier	= fread_number(fp);
					paf.location	= fread_number(fp);
					paf.bitvector	= fread_number(fp);
					affect_to_obj(obj,&paf);
					fMatch          = true;
					break;
				}
				break;
			case 'C':
				KEY("Cond",			obj->condition,		fread_number(fp));
				KEY("Cost",			obj->cost,			fread_number(fp));
				break;
			case 'D':
				KEY("Description",	obj->description,	fread_string(fp));
				KEY("Desc",			obj->description,	fread_string(fp));
				KEY("Drop",			obj->droprate,		fread_number(fp));
				break;
			case 'E':
				if (!str_cmp(word,"Enchanted")){
					obj->enchanted = true;
	 				fMatch 	= true;
					break;
				}
				if(!str_cmp(word,"ExtF")){
					obj->extra_flags = fread_number(fp);
					fMatch		= true;
					break;
				}
				if(!str_cmp(word,"Excludes")){
					fMatch		= true;
					break;
				}

				if (!str_cmp(word,"ExtraDescr") || !str_cmp(word,"ExDe")){
					EXTRA_DESCR_DATA *ed;

					ed					= new_extra_descr();
					ed->keyword			= fread_string(fp);
					ed->description		= fread_string(fp);
					ed->next			= obj->extra_descr;
					obj->extra_descr	= ed;
					fMatch = true;
				}

				if (!str_cmp(word,"End")){
					if (!fNest || (fVnum && obj->pIndexData == NULL)){
						bug("Fread_obj: incomplete object.",0);
						free_obj(obj);
						return;
					}
					else{
						if (!fVnum){
							free_obj(obj);
							obj				= create_object(get_obj_index(OBJ_VNUM_DUMMY),0);
						}
						if (!new_format){
		    				obj->next		= object_list;
		    				object_list		= obj;
		    				obj->pIndexData->count++;
						}
						if (!obj->pIndexData->new_format && (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD) && obj->value[1] == 0)
						{
							obj->value[1]	= obj->value[0];
							obj->value[2]	= obj->value[0];
						}
						if (make_new)
						{
							int wear;

							wear			= obj->wear_loc;
							extract_obj(obj);
							obj				= create_object(obj->pIndexData,0);
							obj->wear_loc	= wear;
						}
						if (iNest == 0 || rgObjNest[iNest] == NULL)
						{
							if(ch)
								obj_to_char(obj,ch);
							else
								obj_to_room(obj,obj->in_room);
						}
						else
							obj_to_obj(obj,rgObjNest[iNest-1]);
						fMatch = true;
						return;
					}
				}
				break;
			case 'I':
				if(!str_cmp(word,"Inrm"))
				{
					obj->in_room = get_room_index(fread_number(fp));
					fMatch = true;
					break;
				}
				KEY("ItemType",	obj->item_type,		fread_number(fp));
				KEY("Ityp",		obj->item_type,		fread_number(fp));
				break;
			case 'L':
				KEY("Level",	obj->level,			fread_number(fp));
				KEY("Lev",		obj->level,			fread_number(fp));
				break;
			case 'M':
				KEY("Mtrl",		obj->material,		fread_string(fp));
				break;
			case 'N':
				KEY("Name",		obj->name,			fread_string(fp));

				if (!str_cmp(word,"Nest"))
				{
					iNest = fread_number(fp);
					if (iNest < 0 || iNest >= MAX_NEST)
						bug("Fread_obj: bad nest %d.",iNest);
					else
					{
						rgObjNest[iNest] = obj;
						fNest = true;
					}
					fMatch = true;
				}
				break;
   			case 'O':
				if (!str_cmp(word,"Oldstyle"))
				{
					if (obj->pIndexData != NULL && obj->pIndexData->new_format)
						make_new = true;
					fMatch = true;
				}
				break;
			case 'S':
				KEY("ShortDescr",	obj->short_descr,	fread_string(fp));
				KEY("ShD",			obj->short_descr,	fread_string(fp));

				if (!str_cmp(word,"Spell"))
				{
					int iValue, sn;

					iValue = fread_number(fp);
					sn     = skill_lookup(fread_word(fp));
					if (iValue < 0 || iValue > 3)
						bug("Fread_obj: bad iValue %d.",iValue);
					else if (sn < 0)
						bug("Fread_obj: unknown skill.",0);
					else
						obj->value[iValue] = sn;
					fMatch = true;
					break;
				}
				break;
			case 'T':
				KEY("Timer",	obj->timer,		fread_number(fp));
				KEY("Time",		obj->timer,		fread_number(fp));
				break;
			case 'V':
				if (!str_cmp(word,"Values") || !str_cmp(word,"Vals"))
				{
					obj->value[0]	= fread_number(fp);
					obj->value[1]	= fread_number(fp);
					obj->value[2]	= fread_number(fp);
					obj->value[3]	= fread_number(fp);
					obj->value[4]	= fread_number(fp);
					if (obj->item_type == ITEM_WEAPON && obj->value[0] == 0)
					   obj->value[0] = obj->pIndexData->value[0];
					fMatch		= true;
					break;
				}
				if (!str_cmp(word,"Val"))
				{
					obj->value[0] 	= fread_number(fp);
	 				obj->value[1]	= fread_number(fp);
	 				obj->value[2] 	= fread_number(fp);
					obj->value[3]	= fread_number(fp);
					obj->value[4]	= fread_number(fp);
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"Vnum"))
				{
					int vnum;

					vnum = fread_number(fp);
					if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
						bug("Fread_obj: bad vnum %d.",vnum);
					else
						fVnum = true;
					fMatch = true;
					break;
				}
				break;
			case 'W':
				if(!strcmp(word,"WpnFlag")){
					fMatch = true;
					break;
				}
				if (!str_cmp(word,"WFlg")){
					read_bits(obj->wflags,MAX_WPN,fread_string(fp));
					fMatch = true;
					break;
				}
				KEY("WearFlags",	obj->wear_flags,	fread_number(fp));
				KEY("WeaF",			obj->wear_flags,	fread_number(fp));
				if(!str_cmp(word,"WearLoc") || !str_cmp(word,"Wear")){
					obj->wear_loc = fread_number(fp);
					if(obj->wear_loc != -1){
						if(obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD && obj->armortype_flags != ARMOR_NONE)
							ch->worn_armor[obj->armortype_flags]++;
					}
					fMatch = true;
				}
				KEY("Weight",		obj->weight,		fread_number(fp));
				KEY("Wt",			obj->weight,		fread_number(fp));
				break;
		}

		if (!fMatch)
		{
			bugf("Fread_obj: no match: %s.",word);
			fread_to_eol(fp);
		}
	}
}

void fread_lockers(){
	/*FILE *fp;
	ROOM_INDEX_DATA * pRoom;
	struct dirent *Dir;
	DIR *Directory;
	char FName[80],*strsave,filename[100];

	Directory = opendir( LOCKER_DIR );

	for(Dir = readdir( Directory );Dir;Dir = readdir(Directory)){
		sprintf(FName, LOCKER_DIR "%s", Dir->d_name);
		if(Dir->d_name[0] >= '0' && Dir->d_name[0] <= '9')
			sprintf(filename,"%s",Dir->d_name);
		else
			continue;
		if(!is_number(filename)){
			log_f("Crappy fread_locker %s",filename);
			exit(0);
		}
		if(!(pRoom = get_room_index(atoi(filename)))){
			log_f("Crappy fread_locker room %s",filename);
			continue;
		}
		sprintf(strsave,"%s%s",LOCKER_DIR,filename);
		if ((fp = fopen(strsave,"r"))){
			for(;;){
				char letter,*word;

				letter = fread_letter(fp);
				if (letter == '*'){
					fread_to_eol(fp);
					continue;
				}
				if (letter != '#'){
					bug("Load_char_obj: # not found.",0);
					break;
				}
				word = fread_word(fp);
				if(!str_cmp(word,"OBJECT")) fread_obj(NULL,fp);
				else{
					log_f("Fread_lockers : stupid # thing");
					break;
				}
			}
		}
	}
	closedir(Directory);*/

	FILE *fp;
	struct dirent *Dir;
	DIR *Directory;
	char FName[80],filename[MSL],strsave[MSL];

	Directory = opendir( LOCKER_DIR );
	Dir = readdir( Directory );
	while( Dir != NULL ){
		sprintf(FName, LOCKER_DIR "%s", Dir->d_name);
		{
			if (Dir->d_name[0] >= '0' && Dir->d_name[0] <= '9'){
				sprintf(filename, "%s", Dir->d_name);
				sprintf(strsave,"%s%s",LOCKER_DIR,filename);
				if ((fp = fopen(strsave,"r"))){
					for(;;){
						char letter,*word;

						letter = fread_letter(fp);
						if (letter == '*'){
							fread_to_eol(fp);
							continue;
						}
						if (letter != '#'){
							bug("Load_char_obj: # not found.",0);
							break;
						}
						word = fread_word(fp);
						if(!str_cmp(word,"O"))
							fread_obj(NULL,fp);
						else if(!str_cmp(word,"END"))
							break;
						else{
							log_f("Fread_lockers : stupid # thing");
							break;
						}
					}
				}
			}
		}
		Dir = readdir( Directory );
	}
	closedir( Directory );
}

void save_char_locker(CHAR_DATA *ch){
	char filename[100];
	char strsave[100];
	FILE *fp;

	if(!IS_NPC(ch))
		cql_save_char(ch);

	fclose(fpReserve);
	sprintf(strsave,LOCKER_DIR "%d",ch->in_room->vnum);
	if(!(fp = fopen(TEMP_FILE,"w"))){
		bug("save_char_locker: fopen",0);
		perror(strsave);
	}
	else{
		for(OBJ_DATA *locker = ch->in_room->contents;locker;locker = locker->next_content){
			if(locker->item_type != ITEM_LOCKER || !locker->contains)
				continue;
			fwrite_obj(NULL,locker,fp,0);
		}
		fprintf(fp,"#END\n");
	}
	fclose(fp);
	rename(TEMP_FILE,strsave);
	fpReserve = fopen(NULL_FILE,"r");
}
