#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/*
 * Globals
 */
extern int top_reset;
extern int top_area;
extern int top_exit;
extern int top_ed;
extern int top_room;
extern int top_mprog_index;
extern int top_oprog_index;
extern int top_rprog_index;

AREA_DATA			* area_free;
extern EXTRA_DESCR_DATA	* extra_descr_free;
EXIT_DATA			* exit_free;
ROOM_INDEX_DATA		* room_index_free;
OBJ_INDEX_DATA		* obj_index_free;
SHOP_DATA			* shop_free;
MOB_INDEX_DATA		* mob_index_free;
RESET_DATA			* reset_free;
extern HELP_DATA			* help_free;

void free_oprog			( PROG_LIST* );
void free_rprog			( PROG_LIST* );
void free_extra_descr	( EXTRA_DESCR_DATA* );
void free_affect		( AFFECT_DATA* );
void free_mprog			( PROG_LIST* );


RESET_DATA *new_reset_data(void){
	RESET_DATA *pReset;

	if (!reset_free){
		pReset			= (RESET_DATA *)alloc_perm( sizeof(*pReset));
		top_reset++;
	}
	else{
		pReset			= reset_free;
		reset_free		= reset_free->next;
	}

	pReset->next		= NULL;
	pReset->command		= 'X';
	pReset->arg1		= 0;
	pReset->arg2		= 0;
	pReset->arg3		= 0;
	pReset->arg4		= 0;

	return pReset;
}

void free_reset_data(RESET_DATA *pReset){
    pReset->next	= reset_free;
    reset_free		= pReset;
    return;
}

AREA_DATA *new_area(void){
	AREA_DATA *pArea;
	char buf[MIL];

	if (!area_free){
		pArea   =  (AREA_DATA *) alloc_perm(sizeof(*pArea));
		top_area++;
	}
	else{
		pArea       =   area_free;
		area_free   =   area_free->next;
	}

	pArea->next				= NULL;
	pArea->name				= str_dup("New area");
	pArea->area_flags		= AREA_ADDED;
	pArea->security			= 1;
	pArea->builders			= str_dup("None");
	pArea->min_vnum			= 0;
	pArea->max_vnum			= 0;
	pArea->age				= 0;
	pArea->nplayer			= 0;
	pArea->empty			= true;
	sprintf(buf,"area%d.are",pArea->vnum);
	pArea->file_name		= str_dup(buf);
	pArea->vnum				= top_area-1;

	return pArea;
}

void free_area(AREA_DATA *pArea){
    free_string(pArea->name);
    free_string(pArea->file_name);
    free_string(pArea->builders);
    free_string(pArea->credits);

    pArea->next			= area_free->next;
    area_free			= pArea;
    return;
}

EXIT_DATA *new_exit(void){
	EXIT_DATA *pExit;

	if (!exit_free){
		pExit			= (EXIT_DATA *)alloc_perm(sizeof(*pExit));
		top_exit++;
	}
	else{
		pExit			= exit_free;
		exit_free		= exit_free->next;
	}

	pExit->u1.to_room	= NULL;
	pExit->exit_info	= 0;
	pExit->key			= 0;
	pExit->keyword		= &str_empty[0];
	pExit->description	= &str_empty[0];
	pExit->next			= NULL;
	pExit->rs_flags		= 0;
	pExit->orig_door	= 0;

	return pExit;
}

void free_exit(EXIT_DATA *pExit){
	free_string(pExit->keyword);
	free_string(pExit->description);

	pExit->next			= exit_free;
	exit_free			= pExit;
	return;
}

ROOM_INDEX_DATA *new_room_index(void){
    ROOM_INDEX_DATA *pRoom;
    int door;

    if (!room_index_free)
    {
        pRoom				= (ROOM_INDEX_DATA *)alloc_perm(sizeof(*pRoom));
        top_room++;
    }
    else
    {
        pRoom				= room_index_free;
        room_index_free		= room_index_free->next;
    }

    pRoom->next				= NULL;
    pRoom->people			= NULL;
    pRoom->contents			= NULL;
    pRoom->extra_descr		= NULL;
    pRoom->area				= NULL;

    for ( door=0; door < MAX_DIR; door++ )
        pRoom->exit[door]	= NULL;

    pRoom->name				= &str_empty[0];
    pRoom->description		= &str_empty[0];
    pRoom->owner			= &str_empty[0];
    pRoom->vnum				= 0;
    pRoom->room_flags		= 0;
    pRoom->light			= 0;
    pRoom->sector_type		= 0;
    pRoom->heal_rate		= 100;
    pRoom->mana_rate		= 100;

    return pRoom;
}

void free_room_index(ROOM_INDEX_DATA *pRoom)
{
	int door;
	EXTRA_DESCR_DATA *pExtra, *wExtra;
	RESET_DATA *pReset, *wReset;

    free_string(pRoom->name);
    free_string(pRoom->description);
    free_string(pRoom->owner);
	free_rprog(pRoom->rprogs);

	for ( door = 0; door < MAX_DIR; door++ )
	{
		if ( pRoom->exit[door] )
		{
			free_exit( pRoom->exit[door] );
			pRoom->exit[door] = NULL;
		}

	}

	for ( pExtra = pRoom->extra_descr; pExtra; pExtra = wExtra )
	{
		wExtra = pExtra->next;
		free_extra_descr( pExtra );
	}

	for ( pReset = pRoom->reset_first; pReset; pReset = wReset )
	{
		wReset = pReset->next;
		free_reset_data( pReset );
	}

    pRoom->next     = room_index_free;
    room_index_free = pRoom;
    return;
}

extern AFFECT_DATA *affect_free;

SHOP_DATA *new_shop(void)
{
    SHOP_DATA *pShop;
    int buy;

    if (!shop_free)
    {
        pShop           = (SHOP_DATA *) alloc_perm(sizeof(*pShop));
        top_shop++;
    }
    else
    {
        pShop           = shop_free;
        shop_free       = shop_free->next;
    }

    pShop->next         = NULL;
    pShop->keeper       = 0;

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy] = 0;

    pShop->profit_buy   = 100;
    pShop->profit_sell  = 100;
    pShop->open_hour    = 0;
    pShop->close_hour   = 23;

    return pShop;
}

void free_shop(SHOP_DATA *pShop)
{
	if (!pShop)
		return;
    pShop->next = shop_free;
    shop_free   = pShop;
    return;
}

OBJ_INDEX_DATA *new_obj_index(void){
    OBJ_INDEX_DATA *pObj;
    int value;

    if (!obj_index_free)
    {
        pObj           = (OBJ_INDEX_DATA *) alloc_perm(sizeof(*pObj));
        top_obj_index++;
    }
    else
    {
        pObj            = obj_index_free;
        obj_index_free  = obj_index_free->next;
    }

    pObj->next			= NULL;
    pObj->extra_descr	= NULL;
    pObj->affected		= NULL;
    pObj->area			= NULL;
    pObj->name			= str_dup("no name");
    pObj->short_descr	= str_dup("(no short description)");
    pObj->description	= str_dup("(no description)");
    pObj->lore			= str_dup("(none)");
    pObj->vnum			= 0;
    pObj->item_type		= ITEM_TRASH;
	pObj->armortype_flags	= 0;
	pObj->extra_flags	= 0;
	pObj->exclude_flags	= 0;
    pObj->wear_flags    = 0;
    pObj->count         = 0;
    pObj->weight        = 0;
    pObj->cost          = 0;
    pObj->material      = str_dup("unknown");
    pObj->condition     = 100;
    pObj->droprate		= 1000;
    pObj->timer			= 0;
    for ( value = 0; value < 5; value++ )
        pObj->value[value]  = 0;

    pObj->new_format    = true;

    return pObj;
}

void free_obj_index(OBJ_INDEX_DATA *pObj)
{
	EXTRA_DESCR_DATA *pExtra, *wExtra;
	AFFECT_DATA *pAf, *wAf;

    free_string(pObj->name);
    free_string(pObj->short_descr);
    free_string(pObj->description);
	free_oprog(pObj->oprogs);

	for ( pAf = pObj->affected; pAf; pAf = wAf )
	{
		wAf = pAf->next;
		free_affect( pAf );
	}

	for ( pExtra = pObj->extra_descr; pExtra; pExtra = wExtra )
	{
		wExtra = pExtra->next;
		free_extra_descr( pExtra );
	}

    pObj->next = obj_index_free;
    obj_index_free = pObj;
    return;
}

MOB_INDEX_DATA *new_mob_index(void){
	int i;
    MOB_INDEX_DATA *pMob;

    if (!mob_index_free)
    {
        pMob           = (MOB_INDEX_DATA *)alloc_perm(sizeof(*pMob));
        top_mob_index++;
    }
    else
    {
        pMob            = mob_index_free;
        mob_index_free  = mob_index_free->next;
    }

    pMob->next          = NULL;
    pMob->spec_fun      = NULL;
    pMob->pShop         = NULL;
    pMob->area          = NULL;
    pMob->player_name   = str_dup( "no name" );
    pMob->short_descr   = str_dup( "(no short description)" );
    pMob->long_descr    = str_dup( "(no long description)\n\r" );
    pMob->icpmsg		= str_dup( "nada" );
    pMob->ocpmsg		= str_dup( "nada" );
    pMob->cmsg			= str_dup( "nada" );
    pMob->description   = &str_empty[0];
    pMob->vnum          = 0;
    pMob->count         = 0;
    pMob->killed        = 0;
    pMob->trainer       = -1;
    pMob->lefty			= false;
    pMob->sex           = 0;
    pMob->level         = 0;
	for(i = 0;i<MAX_AFF;i++)pMob->affected_by[i] = false;
    pMob->alignment     = 0;
    pMob->hitroll		= 0;
	for(i = 0;i<MAX_ACT;i++)pMob->act[i] = false;
	for(i = 0;i<MAX_CMM;i++)pMob->comm[i] = false;
    //for(i = 0;i<MAX_WZ;i++)pMob->wiz[i] = false;
	for(i = 0;i<MAX_OFF;i++)pMob->off_bits[i] = false;
	for(i = 0;i<MAX_DEF;i++)pMob->def_bits[i] = false;
	for(i = 0;i<MAX_RES;i++)pMob->res[i] = 100;
	for(i = 0;i<MAX_FRM;i++)pMob->form[i] = false;
	for(i = 0;i<MAX_PRT;i++)pMob->parts[i] = false;
    pMob->hitroll		= 0;
    pMob->hitroll		= 0;
    pMob->hitroll		= 0;
    pMob->hitroll		= 0;
    pMob->hitroll		= 0;
    pMob->race          = race_lookup( "human" );
    pMob->material      = str_dup("unknown");
    pMob->size          = SIZE_MEDIUM;
    pMob->ac[AC_PIERCE]	= 0;
    pMob->ac[AC_BASH]	= 0;
    pMob->ac[AC_SLASH]	= 0;
    pMob->ac[AC_EXOTIC]	= 0;
    pMob->hit[DICE_NUMBER]	= 0;
    pMob->hit[DICE_TYPE]	= 0;
    pMob->hit[DICE_BONUS]	= 0;
    pMob->mana[DICE_NUMBER]	= 0;
    pMob->mana[DICE_TYPE]	= 0;
    pMob->mana[DICE_BONUS]	= 0;
    pMob->damage[DICE_NUMBER]	= 0;
    pMob->damage[DICE_TYPE]	= 0;
    pMob->damage[DICE_NUMBER]	= 0;
    pMob->start_pos             = POS_STANDING;
    pMob->default_pos           = POS_STANDING;
    pMob->wealth                = 0;
    pMob->guild                = 0;

    pMob->new_format            = true;

    return pMob;
}

void free_mob_index( MOB_INDEX_DATA *pMob )
{
	PROG_LIST *list, *mp_next;

    free_string( pMob->player_name );
    free_string( pMob->short_descr );
    free_string( pMob->long_descr );
    free_string( pMob->description );

	for( list = pMob->mprogs; list; list = mp_next )
	{
		mp_next = list->next;
		free_mprog( pMob->mprogs );
	}

	if( pMob->pShop )
	{
		free_shop( pMob->pShop );
	}

    pMob->next              = mob_index_free;
    mob_index_free          = pMob;
    return;
}

PROG_CODE              *       mpcode_free;
PROG_CODE	       *       opcode_free;
PROG_CODE	       *       rpcode_free;

PROG_CODE *new_mpcode(void)
{
     PROG_CODE *NewCode;

     if (!mpcode_free)
     {
         NewCode = (PROG_CODE*) alloc_perm(sizeof(*NewCode) );
         top_mprog_index++;
     }
     else
     {
         NewCode     = mpcode_free;
         mpcode_free = mpcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

void free_mpcode(PROG_CODE *pMcode)
{
    free_string(pMcode->code);
    pMcode->next = mpcode_free;
    mpcode_free  = pMcode;
    return;
}

PROG_CODE *new_opcode(void)
{
     PROG_CODE *NewCode;

     if (!rpcode_free)
     {
         NewCode = (PROG_CODE*)alloc_perm(sizeof(*NewCode) );
         top_oprog_index++;
     }
     else
     {
         NewCode     = opcode_free;
         opcode_free = opcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

PROG_CODE *new_rpcode(void)
{
     PROG_CODE *NewCode;

     if (!rpcode_free)
     {
         NewCode = (PROG_CODE*)alloc_perm(sizeof(*NewCode) );
         top_rprog_index++;
     }
     else
     {
         NewCode     = rpcode_free;
         rpcode_free = rpcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

void free_opcode(PROG_CODE *pOcode)
{
    free_string(pOcode->code);
    pOcode->next = opcode_free;
    opcode_free  = pOcode;
    return;
}

void free_rpcode(PROG_CODE *pRcode)
{
    free_string(pRcode->code);
    pRcode->next = rpcode_free;
    rpcode_free  = pRcode;
    return;
}

void init_blankguild(int n){
//static struct guild_type g_zero;
	//guilds[n] = g_zero;
	guilds[n].active	= false;
	free_string(guilds[n].name);
	free_string(guilds[n].who_name);
	free_string(guilds[n].keywords);
	guilds[n].name		= NULL;
	guilds[n].who_name	= NULL;
	guilds[n].keywords	= NULL;
	guilds[n].index		= 0;
	guilds[n].recall	= 1;
	guilds[n].respawn	= 1;
	guilds[n].area		= 1;
	guilds[n].type		= GTYPE_BLANK;
	guilds[n].races_max	= 0;
	guilds[n].hidden	= false;
	guilds[n].rank_last = 0;
	for(int i = 0; i < MAX_RACE;i++)
		guilds[n].races[i] = false;
	guilds[n].rank[0].name = "Blanker";
	guilds[n].rank[0].recruit = false;
	guilds[n].rank[0].expel = false;
	guilds[n].rank[0].promote = false;
	guilds[n].rank[0].demote = false;
}
