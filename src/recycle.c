#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"


/* stuff for recycling ban structures */
BAN_DATA *ban_free;

BAN_DATA *new_ban(void)
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;

    if (ban_free == NULL)
		ban = (BAN_DATA *)alloc_perm(sizeof(*ban));
    else
    {
		ban = ban_free;
		ban_free = ban_free->next;
    }

    *ban = ban_zero;
    VALIDATE(ban);
    ban->name = &str_empty[0];
    return ban;
}

void free_ban(BAN_DATA *ban)
{
    if (!IS_VALID(ban))
		return;

    free_string(ban->name);
    INVALIDATE(ban);

    ban->next = ban_free;
    ban_free = ban;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
    static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *d;

    if (descriptor_free == NULL)
		d = (DESCRIPTOR_DATA*)alloc_perm(sizeof(*d));
    else
    {
		d = descriptor_free;
		descriptor_free = descriptor_free->next;
    }
	
    *d = d_zero;
    VALIDATE(d);
    
    d->connected	= CON_GET_NAME;
    d->showstr_head	= NULL;
    d->showstr_point = NULL;
    d->outsize	= 2000;
    d->outbuf	= (char*)alloc_mem( d->outsize );
    
    return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
    if (!IS_VALID(d))
		return;

    free_string( d->host );
    free_mem( d->outbuf, d->outsize );
    INVALIDATE(d);
    d->next = descriptor_free;
    descriptor_free = d;
}

/* stuff for recycling extended descs */
EXTRA_DESCR_DATA *extra_descr_free;

EXTRA_DESCR_DATA *new_extra_descr(void){
    EXTRA_DESCR_DATA *ed;

    if (extra_descr_free == NULL)
		ed = (EXTRA_DESCR_DATA*)alloc_perm(sizeof(*ed));
    else
    {
		ed = extra_descr_free;
		extra_descr_free = extra_descr_free->next;
    }

	ed->id = 0;
    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
    VALIDATE(ed);
    return ed;
}

void free_extra_descr(EXTRA_DESCR_DATA *ed)
{
    if (!IS_VALID(ed))
		return;

    free_string(ed->keyword);
    free_string(ed->description);
    INVALIDATE(ed);
    
    ed->next = extra_descr_free;
    extra_descr_free = ed;
}


/* stuff for recycling affects */
AFFECT_DATA *affect_free;

AFFECT_DATA *new_affect(void){
	static AFFECT_DATA af_zero;
	AFFECT_DATA *af;

	if (affect_free == NULL)
		af =(AFFECT_DATA*) alloc_perm(sizeof(*af));
	else{
		af = affect_free;
		affect_free = affect_free->next;
	}

	*af = af_zero;

	VALIDATE(af);
	return af;
}

void free_affect(AFFECT_DATA *af){
    if (!IS_VALID(af))
		return;

    INVALIDATE(af);
    af->next = affect_free;
    affect_free = af;
}

/* stuff for recycling objects */
OBJ_DATA *obj_free;

OBJ_DATA *new_obj(void){
	static OBJ_DATA obj_zero;
	OBJ_DATA *obj;

	if(!obj_free)
		obj = (OBJ_DATA*)alloc_perm(sizeof(*obj));
	else{
		obj = obj_free;
		obj_free = obj_free->next;
	}
	*obj = obj_zero;
	VALIDATE(obj);

	return obj;
}

void free_obj(OBJ_DATA *obj){
	AFFECT_DATA *paf, *paf_next;
	EXTRA_DESCR_DATA *ed, *ed_next;

	if (!IS_VALID(obj))
		return;

	for (paf = obj->affected;paf;paf = paf_next){
		paf_next = paf->next;
		free_affect(paf);
	}
	obj->affected = NULL;

	for (ed = obj->extra_descr;ed;ed = ed_next){
		ed_next = ed->next;
		free_extra_descr(ed);
	}
	obj->extra_descr = NULL;

	free_string(obj->name);
	free_string(obj->description);
	free_string(obj->short_descr);
	free_string(obj->owner);
	free_string(obj->lore);
	INVALIDATE(obj);

	obj->next = obj_free;
	obj_free = obj;
}


/* stuff for recyling characters */
CHAR_DATA *char_free;

CHAR_DATA *new_char(void){
	static CHAR_DATA ch_zero;
	CHAR_DATA *ch;
	int i;

	if (char_free == NULL)
		ch =(CHAR_DATA*) alloc_perm(sizeof(*ch));
	else{
		ch = char_free;
		char_free = char_free->next;
	}

	*ch				= ch_zero;
	VALIDATE(ch);
	ch->guardby					= NULL;
	ch->guarding				= NULL;
	ch->name                    = &str_empty[0];
	ch->short_descr             = &str_empty[0];
	ch->long_descr              = &str_empty[0];
	ch->description             = &str_empty[0];
	ch->prompt                  = &str_empty[0];
	ch->prefix					= &str_empty[0];
	ch->logon                   = current_time;
	ch->lines                   = PAGELEN;
	for (i = 0; i < 4; ++i)
		ch->armor[i]            = 100;
	ch->position                = POS_STANDING;
	ch->hit                     = 100;
	ch->shp                     = 0;
	ch->max_hit                 = 100;
	ch->settruemana(100);
	ch->settruemaxmana(100);
	ch->settrueantimana(100);
	ch->settruemaxantimana(100);
	ch->move                    = 100;
	ch->max_move                = 100;
	ch->spelltimer				= 0;
	ch->spellfailed				= 0;
	ch->spellsn					= 0;
	ch->spellvo					= NULL;
	ch->spellvictim				= NULL;
	ch->chargevict				= NULL;
	ch->spelltarget				= 0;
	ch->spellcost				= 0;
	sprintf(ch->combo,"00000");
	for (i = 0; i < MAX_STATS; i++){
		ch->perm_stat[i] = 1;
		ch->mod_stat[i] = 0;
	}

	return ch;
}


void free_char(CHAR_DATA *ch){
	OBJ_DATA *obj,*obj_next;
	AFFECT_DATA *paf,*paf_next;

	if (!IS_VALID(ch))
		return;

	if (IS_NPC(ch))
		mobile_count--;

	for (obj = ch->carrying; obj != NULL; obj = obj_next){
		obj_next = obj->next_content;
		extract_obj(obj);
	}

	for (paf = ch->affected; paf != NULL; paf = paf_next){
		paf_next = paf->next;
		affect_remove(ch,paf);
	}
	free_string(ch->name);
	free_string(ch->short_descr);
	free_string(ch->long_descr);
	free_string(ch->description);
	free_string(ch->prompt);
	free_string(ch->prefix);
	/*free_note  (ch->pnote);*/

#ifdef IMC
    imc_freechardata( ch );
#endif
	ch->next = char_free;
	char_free  = ch;

	INVALIDATE(ch);
	//Nash Test code for groups
	//if(ch->group){
	//}
}

PC_DATA *pcdata_free;

PC_DATA *new_pcdata(void)
{
	int alias;

	static PC_DATA pcdata_zero;
	PC_DATA *pcdata;

	if (pcdata_free == NULL)
		pcdata =(PC_DATA*) alloc_perm(sizeof(*pcdata));
	else
	{
		pcdata = pcdata_free;
		pcdata_free = pcdata_free->next;
	}

	*pcdata = pcdata_zero;

	for (alias = 0; alias < MAX_ALIAS; alias++)
	{
		pcdata->alias[alias] = NULL;
		pcdata->alias_sub[alias] = NULL;
	}

	pcdata->buffer = new_buf();

	VALIDATE(pcdata);
	return pcdata;
}
	

void free_pcdata(PC_DATA *pcdata)
{
    int alias;

    if (!IS_VALID(pcdata))
		return;

	free_string(pcdata->afk);
	free_string(pcdata->created);
    free_string(pcdata->pwd);
    free_string(pcdata->bamfin);
    free_string(pcdata->bamfout);
    free_string(pcdata->title);
    free_buf(pcdata->buffer);
    
    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
		free_string(pcdata->alias[alias]);
		free_string(pcdata->alias_sub[alias]);
    }
    INVALIDATE(pcdata);
    pcdata->next = pcdata_free;
    pcdata_free = pcdata;

    return;
}

/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void){
    int val;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id(void)
{
    last_mob_id++;
    return last_mob_id;
}

MEM_DATA *mem_data_free;

/* procedures and constants needed for buffering */

BUFFER *buf_free;

MEM_DATA *new_mem_data(void){
    MEM_DATA *memory;
  
    if (mem_data_free == NULL)
		memory =(MEM_DATA*) alloc_mem(sizeof(*memory));
    else
    {
		memory = mem_data_free;
		mem_data_free = mem_data_free->next;
    }

    memory->next = NULL;
    memory->id = 0;
    memory->reaction = 0;
    memory->when = 0;
    memory->dam = 0;
    VALIDATE(memory);

    return memory;
}

void free_mem_data(MEM_DATA *memory)
{
    if (!IS_VALID(memory))
		return;

    memory->next = mem_data_free;
    mem_data_free = memory;
    INVALIDATE(memory);
}



/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
    16,32,64,128,256,1024,2048,4096,8192,16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; ++i)
		if (buf_size[i] >= val)
		{
			return buf_size[i];
		}
    
    return -1;
}

BUFFER *new_buf()
{
    BUFFER *buffer;

    if (buf_free == NULL) 
		buffer =(BUFFER*) alloc_perm(sizeof(*buffer));
    else
    {
		buffer = buf_free;
		buf_free = buf_free->next;
    }

    buffer->next	= NULL;
    buffer->state	= BUFFER_SAFE;
    buffer->size	= get_size(BASE_BUF);

    buffer->string	=(char *) alloc_mem(buffer->size);
    buffer->string[0]	= '\0';
    VALIDATE(buffer);

    return buffer;
}

BUFFER *new_buf_size(int size)
{
    BUFFER *buffer;
 
    if (buf_free == NULL)
        buffer = (BUFFER*)alloc_perm(sizeof(*buffer));
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }
 
    buffer->next        = NULL;
    buffer->state       = BUFFER_SAFE;
    buffer->size        = get_size(size);
    if (buffer->size == -1)
    {
        bug("new_buf: buffer size %d too large.",size);
        exit(1);
    }
    buffer->string      =(char*) alloc_mem(buffer->size);
    buffer->string[0]   = '\0';
    VALIDATE(buffer);
 
    return buffer;
}


void free_buf(BUFFER *buffer)
{
    if (!IS_VALID(buffer))
		return;

    free_mem(buffer->string,buffer->size);
    buffer->string = NULL;
    buffer->size   = 0;
    buffer->state  = BUFFER_FREED;
    INVALIDATE(buffer);

    buffer->next  = buf_free;
    buf_free      = buffer;
}


bool add_buf(BUFFER *buffer, char *string)
{
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
		return false;

    len = strlen_color(buffer->string) + strlen_color(string) + 1;

    while (len >= buffer->size) /* increase the buffer size */
    {
		buffer->size 	= get_size(buffer->size + 1);
		{
			if (buffer->size == -1) /* overflow */
			{
				buffer->size = oldsize;
				buffer->state = BUFFER_OVERFLOW;
				bug("buffer overflow past size %d",buffer->size);
				return false;
		    }
  		}
    }

    if (buffer->size != oldsize)
    {
		buffer->string	= (char*)alloc_mem(buffer->size);

		strcpy(buffer->string,oldstr);
		free_mem(oldstr,oldsize);
    }

    strcat(buffer->string,string);
    return true;
}


void clear_buf(BUFFER *buffer)
{
    buffer->string[0] = '\0';
    buffer->state     = BUFFER_SAFE;
}


char *buf_string(BUFFER *buffer)
{
    return buffer->string;
}

/* stuff for recycling mobprograms */
PROG_LIST *mprog_free;
PROG_LIST *oprog_free;
PROG_LIST *rprog_free;

PROG_LIST *new_mprog(void)
{
   static PROG_LIST mp_zero;
   PROG_LIST *mp;

   if (mprog_free == NULL)
       mp = (PROG_LIST*)alloc_perm(sizeof(*mp));
   else
   {
       mp = mprog_free;
       mprog_free=mprog_free->next;
   }

   *mp = mp_zero;
   mp->vnum             = 0;
   mp->trig_type        = 0;
   mp->code             = str_dup("");
   VALIDATE(mp);
   return mp;
}

void free_mprog(PROG_LIST *mp)
{
   if (!IS_VALID(mp))
      return;

   INVALIDATE(mp);
   mp->next = mprog_free;
   mprog_free = mp;
}

HELP_AREA * had_free;

HELP_AREA * new_had ( void )
{
	HELP_AREA * had;
	static	HELP_AREA   zHad;

	if ( had_free )
	{
		had		= had_free;
		had_free	= had_free->next;
	}
	else
		had		= (HELP_AREA*)alloc_perm( sizeof( *had ) );

	*had = zHad;

	return had;
}

HELP_DATA * help_free;

HELP_DATA * new_help ( void )
{
	HELP_DATA * help;

	if ( help_free )
	{
		help		= help_free;
		help_free	= help_free->next;
	}
	else
		help		= (HELP_DATA*)alloc_perm( sizeof( *help ) );

	return help;
}

void free_help(HELP_DATA *help)
{
	free_string(help->keyword);
	free_string(help->text);
	help->next = help_free;
	help_free = help;
}

PROG_LIST *new_oprog(void)
{
   static PROG_LIST op_zero;
   PROG_LIST *op;

   if (oprog_free == NULL)
       op = (PROG_LIST*) alloc_perm(sizeof(*op));
   else
   {
       op = oprog_free;
       oprog_free=oprog_free->next;
   }

   *op = op_zero;
   op->vnum             = 0;
   op->trig_type        = 0;
   op->code             = str_dup("");
   VALIDATE(op);
   return op;
}

void free_oprog(PROG_LIST *op)
{
   if (!IS_VALID(op))
      return;

   INVALIDATE(op);
   op->next = oprog_free;
   oprog_free = op;
}

PROG_LIST *new_rprog(void)
{
   static PROG_LIST rp_zero;
   PROG_LIST *rp;

   if (rprog_free == NULL)
       rp = (PROG_LIST*) alloc_perm(sizeof(*rp));
   else
   {
       rp = rprog_free;
       rprog_free=rprog_free->next;
   }

   *rp = rp_zero;
   rp->vnum             = 0;
   rp->trig_type        = 0;
   rp->code             = str_dup("");
   VALIDATE(rp);
   return rp;
}

void free_rprog(PROG_LIST *rp)
{
   if (!IS_VALID(rp))
      return;

   INVALIDATE(rp);
   rp->next = rprog_free;
   rprog_free = rp;
}

SLEEP_DATA *sd_free;

SLEEP_DATA *new_sleep_data(void){
	SLEEP_DATA *sd;
	if (sd_free == NULL)
		sd = (SLEEP_DATA*)alloc_perm(sizeof(*sd));
	else
	{
		sd = sd_free;
		sd_free=sd_free->next;
	}

	sd->vnum             = 0;
	sd->timer            = 0;
	sd->line             = 0;
	sd->prog             = NULL;
	sd->mob              = NULL;
	sd->ch               = NULL;
	sd->next             = NULL;
	sd->prev             = NULL;
	VALIDATE(sd);
	return sd;
}

void free_sleep_data(SLEEP_DATA *sd){
	if (!IS_VALID(sd))
		return;

	INVALIDATE(sd);
	sd->next = sd_free;
	sd_free = sd;
}

void init_mobile(CHAR_DATA *mob,int n){
	MOB_INDEX_DATA *pMobIndex = get_mob_index(n);
	int i;
	long wealth;
	AFFECT_DATA af;

	mobile_count++;

	if (pMobIndex == NULL){
		bug("Create_mobile: NULL pMobIndex.",0);
		exit(1);
	}

	mob = new_char();

	mob->pIndexData	= pMobIndex;
	mob->pclass = 3;

	mob->name			= str_dup(pMobIndex->player_name);
	mob->short_descr	= str_dup(pMobIndex->short_descr);
	mob->long_descr		= str_dup(pMobIndex->long_descr);
	mob->description	= str_dup(pMobIndex->description);
	mob->mid			= get_mob_id();
	mob->spec_fun		= pMobIndex->spec_fun;
	mob->prompt			= NULL;
	mob->mprog_target   = NULL;
	mob->trainer		= pMobIndex->trainer;
	mob->icpmsg			= str_dup(pMobIndex->icpmsg);
	mob->ocpmsg			= str_dup(pMobIndex->ocpmsg);
	mob->cmsg			= str_dup(pMobIndex->cmsg);
	mob->guild			= pMobIndex->guild;

	if (pMobIndex->wealth == 0){
		mob->silver = 0;
		mob->gold   = 0;
	}
	else{
		wealth = number_range(pMobIndex->wealth/2, 3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	}

	/* read from prototype */
 	mob->group			= pMobIndex->group;
	mob->pclass			= pMobIndex->pclass;
	clone_bits(pMobIndex->act,mob->act,MAX_ACT);
	clone_bits(pMobIndex->affected_by,mob->affected_by,MAX_AFF);
	mob->setcomm(CM_NOCHANNELS);
	mob->setcomm(CM_NOSHOUT);
	mob->setcomm(CM_NOTELL);
	mob->alignment		= pMobIndex->alignment;
	mob->level			= pMobIndex->level;
	mob->hitroll		= pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damage[DICE_BONUS];
	mob->max_hit		= dice(pMobIndex->hit[DICE_NUMBER], pMobIndex->hit[DICE_TYPE]) + pMobIndex->hit[DICE_BONUS];
	mob->hit			= mob->max_hit;
	mob->setmaxmana(dice(pMobIndex->mana[DICE_NUMBER], pMobIndex->mana[DICE_TYPE]) + pMobIndex->mana[DICE_BONUS]);
	mob->setmana(mob->getmaxmana());
	mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
	mob->damage[DICE_TYPE]	= pMobIndex->damage[DICE_TYPE];
	mob->dam_type		= pMobIndex->dam_type;
	if (mob->dam_type == 0)
    	switch(number_range(1,3)){
			case (1): mob->dam_type = 3;        break;  /* slash */
			case (2): mob->dam_type = 7;        break;  /* pound */
			case (3): mob->dam_type = 11;       break;  /* pierce */
		}
	for (i = 0; i < 4; i++)
		mob->armor[i]	= pMobIndex->ac[i];
	clone_bits(pMobIndex->off_bits,mob->off_bits,MAX_OFF);
	clone_bits(pMobIndex->def_bits,mob->def_bits,MAX_DEF);
	clone_ints(pMobIndex->res,mob->res,MAX_RES);
	clone_bits(pMobIndex->form,mob->form,MAX_FRM);
	clone_bits(pMobIndex->parts,mob->parts,MAX_PRT);
	mob->start_pos		= pMobIndex->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->sex			= pMobIndex->sex;
	if (mob->sex == 3) /* random sex */	mob->sex = number_range(1,2);
	mob->race			= pMobIndex->race;
	mob->size			= pMobIndex->size;
	mob->material		= str_dup(pMobIndex->material);
	mob->lefty			= pMobIndex->lefty;

	//Mob stats computed here including modifiers like size and fast off flag
	for (i = 0;i<MAX_STATS;i++)
		mob->perm_stat[i] = pMobIndex->perm_stat[i];
	for (i=0;i<MAX_STATS;i++)
		if (!stat_flags[i].settable)
			mob->perm_stat[i] = number_range(mob->level/2,mob->level*1.5);
	mob->perm_stat[STAT_STR] += mob->size/2;
	mob->perm_stat[STAT_END] += mob->size;

	/* let's get some spell action */
	if (mob->isaff(AF_SANCTUARY))
		affect_set(mob,TO_AFFECTS,skill_lookup("sanctuary"),mob->level,1,-1,APPLY_NONE,0,AF_SANCTUARY);

	if (mob->isaff(AF_HASTE))
		affect_set(mob,TO_AFFECTS,skill_lookup("haste"),mob->level,1,-1,APPLY_AGI,1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 60),AF_HASTE);

	mob->max_move = number_range(mob->level * 50,mob->level * 100);
	mob->move = mob->max_move;
	mob->position = mob->start_pos;
	mob->setact(AT_IS_NPC);

	/* link the mob to the world list */
	mob->next		= char_list;
	char_list		= mob;
	pMobIndex->count++;
}
void init_blankchar(CHAR_DATA *ch,char *name){
    ch->name							= str_dup(name);
    ch->id								= 0;
    ch->mid								= 0;
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
	for(int stat = 0;stat < MAX_STATS;stat++)
		ch->max_stat[stat] = pc_race_table[ch->race].max_stats[stat];
	for(int stat = 0; stat < MAX_SKILL; stat++){
		ch->pcdata->learned[stat] = -1;
		ch->pcdata->skill_level[stat] = -1;
	}

}
