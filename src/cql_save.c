#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <mysql/mysql.h>

#include "merc.h"
#include "recycle.h"
#include "db.h"
extern MYSQL *db;

/*
   Local Functions
*/
void init_blankguild(int);
void init_blankclass(int);
void cql_save_new_char(CHAR_DATA*,bool);
bool cql_select(char*,...);

////
void cql_write(char *argument){
	int opt_flags;
	opt_flags |= CLIENT_MULTI_STATEMENTS;
		//log_f(argument);
	if(mysql_ping(db)){// Connection died.
		if(!mysql_real_connect(db,NULL,"root","password","wake",0,NULL,opt_flags)){// Failed to reconnect.
			log_f("cql_write() Unable to reconnect: Error: %s", mysql_error(db));
			exit(0);
		}
		else if(mysql_ping(db)){// Failed to reconnect.
			log_f("cql_write() Unable to reconnect2: Error: %s", mysql_error(db));
			exit(0);
		}
	}
	if(mysql_query(db,argument)){
		log_f("cql_write() Error: %s",mysql_error(db));
		return;
	}
}

char *cql_query(char *fmt,...){//this can be used for insert or select only
	char buf[MSL];
	va_list args;
	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	va_end(args);

	return buf;
}

void cql_save_houseranks(int i,int j){
	char nameb[MSL];
	mysql_real_escape_string(db,nameb,guilds[i].rank[j].name,strlen(guilds[i].rank[j].name));
	cql_write(cql_query("INSERT INTO house_ranks (hid,rank_id,name,recruit,expel,promote,demote) VALUES('%d','%d','%s','%d','%d','%d','%d');",
		i,
		j,
		nameb,
		guilds[i].rank[j].recruit,
		guilds[i].rank[j].expel,
		guilds[i].rank[j].promote,
		guilds[i].rank[j].demote));
}

void cql_save_houseranks(){
	char nameb[MSL];
	for(int i = 0;guilds[i].name;i++){
		for(int j = 0;j < MAX_RANKS && guilds[i].rank[j].name;j++){
			mysql_real_escape_string(db,nameb,guilds[i].rank[j].name,strlen(guilds[i].rank[j].name));
			cql_write(cql_query("INSERT INTO house_ranks (hid,rank_id,name,recruit,expel,promote,demote) VALUES('%d','%d','%s','%d','%d','%d','%d') ON DUPLICATE KEY UPDATE name='%s',recruit='%d',expel='%d',promote='%d',demote='%d';",
				i,
				j,
				nameb,
				guilds[i].rank[j].recruit,
				guilds[i].rank[j].expel,
				guilds[i].rank[j].promote,
				guilds[i].rank[j].demote,
				guilds[i].rank[j].name,
				guilds[i].rank[j].recruit,
				guilds[i].rank[j].expel,
				guilds[i].rank[j].promote,
				guilds[i].rank[j].demote));
		}
	}
}

void cql_save_houses(){
	char nameb[MSL],whob[MSL],keyb[MSL];
	for(int i = 0;i < MAX_GUILD && guilds[i].name[0];i++){
		mysql_real_escape_string(db,nameb,guilds[i].name,strlen(guilds[i].name));
		mysql_real_escape_string(db,whob,guilds[i].who_name,strlen(guilds[i].who_name));
		mysql_real_escape_string(db,keyb,guilds[i].keywords,strlen(guilds[i].keywords));
		cql_write(cql_query("UPDATE houses SET name='%s',whoname='%s',keywords='%s',isactive='%d',hindex='%d',housetype='%d',hidden='%d',recall='%d',respawn='%d',area='%d' WHERE id='%d';",
			nameb,
			whob,
			keyb,
			guilds[i].active,
			guilds[i].index,
			guilds[i].type,
			guilds[i].hidden,
			guilds[i].recall,
			guilds[i].respawn,
			guilds[i].area,
			i+1));
	}
	cql_save_houseranks();
}


void cql_save_classes(int cl){
	char dest[MSL],crap[MSL],buf[MSL],nameb[MSL],whob[MSL];
	mysql_real_escape_string(db,dest,classes[cl].msg_self,strlen(classes[cl].msg_self));
	mysql_real_escape_string(db,crap,classes[cl].msg_other,strlen(classes[cl].msg_other));
	mysql_real_escape_string(db,nameb,classes[cl].name,strlen(classes[cl].name));
	mysql_real_escape_string(db,whob,classes[cl].who_name,strlen(classes[cl].who_name));
	sprintf(buf,"UPDATE classes SET name='%s',whoname='%s',primestat='%d',skilladept='%d',thac00='%d',thac32='%d',hpmin='%d',hpmax='%d',mpmin='%d',mpmax='%d',mvmin='%d',mvmax='%d',fmana='%d',align='%d',msgs='%s',msgo='%s',active='%d' WHERE id='%d'",
		nameb,
		whob,
		classes[cl].attr_prime,
		classes[cl].skill_adept,
		classes[cl].thac0_00,
		classes[cl].thac0_32,
		classes[cl].hp_min,
		classes[cl].hp_max,
		classes[cl].mp_min,
		classes[cl].mp_max,
		classes[cl].mv_min,
		classes[cl].mv_max,
		classes[cl].fMana,
		classes[cl].align,
		dest,
		crap,
		classes[cl].active,
		cl);
	cql_write(buf);
	for(int race = 0;pc_race_table[race].name;race++){
		if(pc_race_table[race].class_use[cl])
			cql_write(cql_query("INSERT INTO class_races (name,cid) VALUES('%s','%d');",pc_race_table[race].name,cl));
		else
			cql_write(cql_query("DELETE FROM class_races WHERE name='%s' AND cid='%d';",pc_race_table[race].name,cl));
	}
}

void cql_save_hometowns(){
	char nameb[MSL];
	for(int n = 0;hometowns[n].name;n++){
		mysql_real_escape_string(db,nameb,hometowns[n].name,strlen(hometowns[n].name));
		cql_write(cql_query("INSERT INTO hometowns (id,name,recall,death,map,donor,active) VALUES ('%d','%s','%d','%d','%d','%d','%d') ON DUPLICATE KEY UPDATE name = '%s',recall='%d',death='%d',map='%d',donor='%d',active='%d';",
			hometowns[n].id,
			nameb,
			hometowns[n].recall,
			hometowns[n].death,
			hometowns[n].map,
			hometowns[n].donor,
			hometowns[n].canuse,
			nameb,
			hometowns[n].recall,
			hometowns[n].death,
			hometowns[n].map,
			hometowns[n].donor,
			hometowns[n].canuse));
		cql_write(cql_query("INSERT INTO home_guilds (g_id,h_id) VALUES ('%d','%d') ON DUPLICATE KEY UPDATE g_id = '%d',h_id='%d';",
			hometowns[n].guild,
			hometowns[n].id,
			hometowns[n].guild,
			hometowns[n].id));
	}
	for(int i = 0;pc_race_table[i].name;i++){
		for(int j = 0;j < MAX_HOMETOWN;j++)
			if(pc_race_table[i].home_use[j])
				cql_write(cql_query("INSERT INTO home_races (race,home_id) VALUES ('%s','%d') ON DUPLICATE KEY UPDATE home_id = '%d';",pc_race_table[i].name,hometowns[j].id,hometowns[j].id));
	}
}

void cql_save_affects(CHAR_DATA *ch){
	char nameb[MSL];

	//Nash first we unlink all the ones tied to this account. Sometime in the future we'll make it sweep out the unlinked ones after saving the new ones
	cql_write(cql_query("UPDATE affects SET to_id = '0' WHERE to_id = '%d';",ch->id));
	for(AFFECT_DATA *paf = ch->affected;paf;paf = paf->next){
		if (paf->type < 0 || paf->type>= MAX_SKILL)
			continue;
		mysql_real_escape_string(db,nameb,skill_table[paf->type].name,strlen(skill_table[paf->type].name));
		if(paf->id > 0)
			cql_write(cql_query("UPDATE affects SET \
								to_id = '%d',aname = '%s',awhere = '%d',atype = '%d',alevel = '%d',aduration = '%d',amodifier = '%d',alocation = '%d',abitvector = '%d',tochar = '1',aslvl = '%d' \
								WHERE id = %d",
				ch->id,
				nameb,
				paf->where,
				paf->type,
				paf->level,
				paf->duration,
				paf->modifier,
				paf->location,
				paf->bitvector,
				paf->slvl,
				paf->id));
		else{
			cql_write(cql_query("INSERT INTO affects (to_id,aname,awhere,atype,alevel,aduration,amodifier,alocation,abitvector,tochar,aslvl) VALUES ('%d','%s','%d','%d','%d','%d','%d','%d','%d','1',%d)",
				ch->id,
				nameb,
				paf->where,
				paf->type,
				paf->level,
				paf->duration,
				paf->modifier,
				paf->location,
				paf->bitvector,
				paf->slvl));
			paf->id = mysql_insert_id(db);
		}

	}
}

void cql_save_affects(OBJ_DATA *obj){
	char nameb[MSL];

	cql_write(cql_query("DELETE FROM affects WHERE to_id = '%d';",obj->id));

    for(AFFECT_DATA *paf = obj->affected;paf;paf = paf->next){// need to add code to wipe all old affs tied to item in db
		if (paf->type < 0 || paf->type >= MAX_SKILL)
			continue;
		mysql_real_escape_string(db,nameb,skill_table[paf->type].name,strlen(skill_table[paf->type].name));
		cql_write(cql_query("INSERT INTO affects (to_id,aname,awhere,atype,alevel,aduration,amodifier,alocation,abitvector,tochar) VALUES('%d','%s','%d','%d','%d','%d','%d','%d','%d','0');",
			obj->id,
			nameb,
			paf->where,
			paf->type,
			paf->level,
			paf->duration,
			paf->modifier,
			paf->location,
			paf->bitvector));
	}
}



void save_contains(OBJ_DATA *obj){
	cql_write(cql_query("DELETE FROM item_contents WHERE parent_id = '%d';",obj->id));

	for(OBJ_DATA *tobj = obj->contains;tobj;tobj = tobj->next_content){
		cql_write(cql_query("INSERT INTO item_contents (parent_id,child_id) VALUES ('%d','%d');",obj->id,tobj->id));
		if(tobj->contains)
			save_contains(tobj);
	}
}
void cql_save_contents(OBJ_DATA *obj){
	for(;obj;obj = obj->next_content)
		if(obj->contains)
			save_contains(obj);
}

void cql_save_ed(EXTRA_DESCR_DATA *ed,OBJ_DATA *obj){
	char descbuf[MSL],keybuf[MSL];
	cql_write(cql_query("DELETE FROM extra_descs WHERE to_id = '%d';",obj->id));

		mysql_real_escape_string(db,descbuf,ed->description,strlen(ed->description));
		mysql_real_escape_string(db,keybuf,ed->keyword,strlen(ed->keyword));

	if(ed->id == 0)
		cql_write(cql_query("INSERT INTO extra_descs (to_id,edesc,ekey) VALUES ('%d','%s','%s');",
			obj->id,
			descbuf,
			keybuf));
	else
		cql_write(cql_query("UPDATE extra_descs SET edesc = '%s', ekey = '%s', to_id = '%d' WHERE id = '%d';",
			descbuf,
			keybuf,
			obj->id,
			ed->id));
}

void cql_save_obj(CHAR_DATA *ch,OBJ_DATA *obj){
	char namebuf[MSL],shortbuf[MSL],longbuf[MSL];
	if(obj->next_content)
		cql_save_obj(ch,obj->next_content);
	
    if(ch && ((ch->level < obj->level - 25 && obj->item_type != ITEM_CONTAINER) || obj->item_type == ITEM_KEY))
		return;

	mysql_real_escape_string(db,namebuf,obj->name,strlen(obj->name));
	mysql_real_escape_string(db,shortbuf,obj->short_descr,strlen(obj->short_descr));
	mysql_real_escape_string(db,longbuf,obj->description,strlen(obj->description));

	if(obj->id == 0){//New item
		cql_write(cql_query("INSERT INTO items (vnum,enchanted,name,short,descr,extraflags,armortype,wearflags,itemtype,weight,cond,droprate,wear,level,timer,cost,material,weaponflags,v0,v1,v2,v3,v4) \
										VALUES ('%d','%d',  '%s','%s', '%s','%d',      '%d',     '%d',     '%d',    '%d',  '%d',     '%d',    '%d','%d', '%d', '%d','%s',    '%s',    '%d','%d','%d','%d','%d'    );",
			obj->pIndexData->vnum,
			obj->enchanted,
			namebuf,
			shortbuf,
			longbuf,
			obj->extra_flags,
			obj->armortype_flags,
			obj->wear_flags,
			obj->item_type,
			obj->weight,
			obj->condition,
			obj->droprate,
			obj->wear_flags,
			obj->level,
			obj->timer,
			obj->cost,
			obj->material,
			save_bits(obj->wflags,MAX_WPN),
			obj->value[0],
			obj->value[1],
			obj->value[2],
			obj->value[3],
			obj->value[4]
		));
		//retrieve the item's generated id
		obj->id = mysql_insert_id(db);
		//save owner
		cql_write(cql_query("INSERT INTO item_owners (to_id,i_id) VALUES ('%d','%d');",ch->id,obj->id));
	}
	else{//pre-existing item
		cql_write(cql_query("INSERT INTO item_owners (to_id,i_id) VALUES ('%d','%d') ON DUPLICATE KEY UPDATE to_id = '%d';",ch->id,obj->id,ch->id));//check ownership

		//update the item info
		cql_write(cql_query("UPDATE items SET \
		vnum = '%d',enchanted = '%d',name = '%s',short = '%s',descr = '%s',extraflags = '%d',armortype = '%d',wearflags = '%d',itemtype = '%d',weight = '%d',cond = '%d',\
		droprate = '%d',wear = '%d',level = '%d',timer = '%d',cost = '%d',material = '%s',weaponflags = '%s',v0 = '%d',v1 = '%d',v2 = '%d',v3 = '%d',v4 = '%d' WHERE id = '%d';",
			obj->pIndexData->vnum,
			obj->enchanted,
			namebuf,
			shortbuf,
			longbuf,
			obj->extra_flags,
			obj->armortype_flags,
			obj->wear_flags,
			obj->item_type,
			obj->weight,
			obj->condition,
			obj->droprate,
			obj->wear_loc,
			obj->level,
			obj->timer,
			obj->cost,
			obj->material,
			save_bits(obj->wflags,MAX_WPN),
			obj->value[0],
			obj->value[1],
			obj->value[2],
			obj->value[3],
			obj->value[4],
			obj->id
		));

	}
	//save affects
	cql_save_affects(obj);
	//save ed's
    for(EXTRA_DESCR_DATA *ed = obj->extra_descr;ed;ed = ed->next)
		cql_save_ed(ed,obj);
	//save contents, this is seperate from saving the contents relations, just creating entries for the nested items here
	if(obj->contains)
		cql_save_obj(ch,obj->contains);
}

void cql_save_charhome(CHAR_DATA *ch){
	cql_write(cql_query("INSERT INTO char_home (to_id,home_id) VALUES('%d','%d') ON DUPLICATE KEY UPDATE home_id = '%d';",
		ch->id,
		ch->hometown,
		ch->hometown
	));
}

void cql_save_charguild(CHAR_DATA *ch){
	cql_write(cql_query("INSERT INTO char_guild (to_id,guild_id,rank) VALUES ('%d','%d','%d') ON DUPLICATE KEY UPDATE guild_id = '%d', rank = '%d';",//save guild
		ch->id,
		ch->guild,
		ch->rank,
		ch->guild,
		ch->rank));
}

void cql_save_charclass(CHAR_DATA *ch){
	cql_write(cql_query("INSERT INTO char_class (to_id,class_id) VALUES ('%d','%d') ON DUPLICATE KEY UPDATE class_id = '%d';",
		ch->id,
		ch->pclass,
		ch->pclass));
}

void cql_save_chardata(CHAR_DATA *ch){
	char nameb[MSL],descb[MSL],raceb[MSL],shortb[MSL],longb[MSL];
	char plrs[MSL],affs[MSL],cmms[MSL],wizs[MSL],ress[MSL];
	char plrb[MSL],affb[MSL],cmmb[MSL],wizb[MSL],resb[MSL];

	mysql_real_escape_string(db,nameb,ch->name,strlen(ch->name));
	mysql_real_escape_string(db,descb,ch->description,strlen(ch->description));
	mysql_real_escape_string(db,shortb,ch->short_descr,strlen(ch->short_descr));
	mysql_real_escape_string(db,longb,ch->long_descr,strlen(ch->long_descr));
	mysql_real_escape_string(db,raceb,race_table[ch->race].name,strlen(race_table[ch->race].name));
	
	sprintf(plrs,"%s",IS_NPC(ch) ? save_bits(ch->act,MAX_ACT) : save_bits(ch->act,MAX_PLR));
	sprintf(affs,"%s",save_bits(ch->affected_by,MAX_AFF));
	sprintf(cmms,"%s",save_bits(ch->comm,MAX_CMM));
	sprintf(wizs,"%s",save_bits(ch->wiz,MAX_WIZ));
	sprintf(ress,"%s",save_ints(ch->res,MAX_RES));
	mysql_real_escape_string(db,plrb,plrs,strlen(plrs));
	mysql_real_escape_string(db,affb,affs,strlen(affs));
	mysql_real_escape_string(db,cmmb,cmms,strlen(cmms));
	mysql_real_escape_string(db,wizb,wizs,strlen(wizs));
	mysql_real_escape_string(db,resb,ress,strlen(ress));
	cql_write(cql_query(
"UPDATE char_data SET isnpc = '%d',name = '%s',descr = '%s',acts = '%s',affs = '%s',coms = '%s',wizs = '%s',ress = '%s',version = '%d',spiritguide = '%d',sex = '%d',god = '%d',level = '%d',\
	in_room = '%d',hp = '%d',mhp = '%d',mp = '%d',mmp = '%d',amp = '%d',mamp = '%d',mv = '%d',mmv = '%d',gold = '%d',silver = '%d',position = '%d',savingthrow = '%d',spellsavingthrow = '%d',align = '%d',hr = '%d',dr = '%d',\
	ac0 = '%d',ac1 = '%d',ac2 = '%d',ac3 = '%d',wimpy = '%d',race = '%s',short_desc='%s',long_desc='%s' WHERE id = '%d';",
		IS_NPC(ch) ? ch->pIndexData->vnum : 0,nameb,descb,plrb,affb,cmmb,wizb,resb,ch->version,ch->spiritguide,ch->sex,ch->god,ch->level,
		ch->in_room->vnum,ch->hit,ch->max_hit,ch->mana,ch->max_mana,ch->antimana,ch->max_antimana,ch->move,ch->max_move,ch->gold,ch->silver,ch->position,ch->saving_throw,ch->saving_spell_throw,ch->alignment,ch->hitroll,ch->damroll,
		ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3],ch->wimpy,raceb,shortb,longb,ch->id
	));
}

void cql_save_pdata(CHAR_DATA *ch){//This and char are always updates because the entry is created at chargen MAKE SURE pcdata id equals chardata id
	char lastnameb[MSL],promptb[MSL],afkb[MSL],poofib[MSL],poofob[MSL],titleb[MSL],createb[MSL],lastlogb[MSL],tbuf[MSL];
	strftime(tbuf,100,"%A, %B %d, %Y at %I:%M %p.",localtime(&current_time));
	mysql_real_escape_string(db,lastnameb,ch->pcdata->lname,strlen(ch->pcdata->lname));
	mysql_real_escape_string(db,afkb,ch->pcdata->afk,strlen(ch->pcdata->afk));
	mysql_real_escape_string(db,poofib,ch->pcdata->bamfin,strlen(ch->pcdata->bamfin));
	mysql_real_escape_string(db,poofob,ch->pcdata->bamfout,strlen(ch->pcdata->bamfout));
	mysql_real_escape_string(db,titleb,ch->pcdata->title,strlen(ch->pcdata->title));
	mysql_real_escape_string(db,createb,ch->pcdata->created,strlen(ch->pcdata->created));
	mysql_real_escape_string(db,promptb,ch->prompt,strlen(ch->prompt));
	mysql_real_escape_string(db,lastlogb,tbuf,strlen(tbuf));
	cql_write(cql_query("UPDATE pc_data SET lname = '%s',height = '%d',weight = '%d',eye = '%d',hair = '%d',shair = '%d',credits = '%d',bprompt = '%d',deathtimer = '%d',trust = '%d',security = '%d',\
						scroll='%d',pk = '%d',pd = '%d',mk = '%d',md = '%d',permhit = '%d',permmana = '%d',permamp = '%d',permmv = '%d',bankgold = '%d',banksilver = '%d',trains = '%d',practices = '%d',s_practices = '%d',\
						studies='%d',xp = '%d',texp = '%d',lefty = '%d',afk = '%s',nobility = '%d',invislevel = '%d',incoglevel = '%d',password = '%s',poofin = '%s',poofout = '%s',title = '%s',created = '%s',\
						lastlogin='%s',totaltime = '%d',points = '%d',truesex = '%d',last_level = '%d',cond_drunk = '%d',cond_full = '%d',cond_thirst = '%d',cond_hunger = '%d',s_studies0 = '%d',s_studies1 = '%d',\
						s_studies2='%d',s_studies3='%d', statpoints='%d', timezone='%d', prompt='%s' WHERE to_id = '%d';",
		lastnameb,ch->looks[P_HEIGHT],ch->looks[P_WEIGHT],ch->looks[P_EYE],ch->looks[P_HAIR],ch->looks[P_SHAIR],ch->credits,ch->battleprompt,ch->death_timer,ch->trust,ch->pcdata->security,
		ch->lines,ch->kills[PK],ch->kills[PD],ch->kills[MK],ch->kills[MD],ch->pcdata->perm_hit,ch->pcdata->perm_mana,ch->pcdata->perm_antimana,ch->pcdata->perm_move,ch->bankgold,ch->banksilver,ch->pcdata->trains,ch->pcdata->practices,ch->pcdata->s_practices,
		ch->pcdata->studies,ch->exp,ch->texp,ch->lefty,afkb,ch->nobility,ch->invis_level,ch->incog_level,ch->pcdata->pwd,poofib,poofob,titleb,createb,
		lastlogb,ch->played + (int)(current_time - ch->logon),ch->pcdata->points,ch->pcdata->true_sex,ch->pcdata->last_level,ch->pcdata->condition[COND_DRUNK],ch->pcdata->condition[COND_FULL],ch->pcdata->condition[COND_THIRST],ch->pcdata->condition[COND_HUNGER],ch->pcdata->s_studies[0],ch->pcdata->s_studies[1],
		ch->pcdata->s_studies[2],ch->pcdata->s_studies[3],ch->pcdata->statpoints,ch->pcdata->timezone,promptb,ch->id
	));
}

void cql_save_stats(CHAR_DATA *ch){
	for(int i = 0;i < MAX_STATS;i++){
		cql_write(cql_query("INSERT INTO char_modstats (to_id,name,stat) VALUES('%d','%s','%d') ON DUPLICATE KEY UPDATE stat = '%d';",
			ch->id,
			abbrev_stat_name(i),
			ch->mod_stat[i],
			ch->mod_stat[i]
		));
		cql_write(cql_query("INSERT INTO char_maxstats (to_id,name,stat) VALUES('%d','%s','%d') ON DUPLICATE KEY UPDATE stat = '%d';",
			ch->id,
			abbrev_stat_name(i),
			ch->max_stat[i],
			ch->max_stat[i]
		));
		cql_write(cql_query("INSERT INTO char_stats (to_id,name,stat) VALUES('%d','%s','%d') ON DUPLICATE KEY UPDATE stat = '%d';",
			ch->id,
			abbrev_stat_name(i),
			ch->perm_stat[i],
			ch->perm_stat[i]
		));
	}
}

void cql_save_skills(CHAR_DATA *ch){
	char nameb[MSL];

	//cql_write(cql_query("DELETE FROM char_skills WHERE to_id = '%d';",ch->id));//Nash someday make this happen in the unlearn crap

	for(int sn = 0;sn < MAX_SKILL && skill_table[sn].name;sn++){
		mysql_real_escape_string(db,nameb,skill_table[sn].name,strlen(skill_table[sn].name));
		if((IS_SKILL_NATURAL(sn) && ch->pcdata->learned[sn] < 1))//???
			continue;
		if(skill_table[sn].name && (ch->pcdata->learned[sn] >= 0 || ch->pcdata->skill_level[sn] >= 0 || ch->pcdata->unlocked[sn]))
		cql_write(cql_query("INSERT INTO char_skills (to_id,unlocked,sk_lvl,learned,name) VALUES('%d','%d','%d','%d','%s') ON DUPLICATE KEY UPDATE unlocked='%d',sk_lvl='%d',learned='%d';",//no ON DUPLICATE since I kill the old entries
			ch->id,
			ch->pcdata->unlocked[sn],
			ch->pcdata->skill_level[sn],
			ch->pcdata->learned[sn],
			nameb,
			ch->pcdata->unlocked[sn],
			ch->pcdata->skill_level[sn],
			ch->pcdata->learned[sn]));
	}
}
void cql_save_companion(CHAR_DATA *ch,bool ispet){
	CHAR_DATA *pet = ispet ? ch->pet : ch->mount;
	if(IS_NPC(ch))
		return;
	if(ch->id == 0){
		ch->send("Error saving you and your pet. ZERO ID!\n\r");
		log_f("%s has a zero ID.",ch->name);
		return;
	}
	if(!pet){//just change the note
		cql_write(cql_query("UPDATE pet_data SET isalive = '0' WHERE ch_id='%d' AND ispet='%d';",ch->id,ispet));
		return;
	}
	else{//The pet is here!
		if(pet->id == 0)//Is it new? Create an entry or use the previous pet's entry
			cql_save_new_char(pet,ispet);
		else{//Old pet just update it
			cql_save_char(pet);
			log_f("INSERT INTO pet_data (pet_id,ch_id,ispet,isalive) VALUES('%d','%d','%d','1') ON DUPLICATE KEY UPDATE isalive='1' WHERE ch_id='%d' AND ispet='%d';",pet->id,ch->id,ispet,ch->id,ispet);//this is an insert/update just in case the original entry is missing
			cql_write(cql_query("INSERT INTO pet_data (pet_id,ch_id,ispet,isalive) VALUES('%d','%d','%d','1') ON DUPLICATE KEY UPDATE isalive='1';",
				pet->id,ch->id,ispet,ch->id,ispet));//this is an insert/update just in case the original entry is missing
			//cql_write(cql_query("UPDATE pet_data SET pet_id = '%d',ch_id = '%d',ispet = '%d'  VERY sure I don't need this
		}
	}
}

void cql_save_charmies(CHAR_DATA *ch){
	;
}

void cql_save_new_char(CHAR_DATA *ch){//for creating new entries in the db MAKE SURE I ALSO AM SAVING THEIR INDEXES CORRECTLY! WOO
	int n;
	char nameb[MSL],descb[MSL],raceb[MSL],shortb[MSL],longb[MSL];

	mysql_real_escape_string(db,nameb,ch->name,strlen(ch->name));
	mysql_real_escape_string(db,descb,ch->description,strlen(ch->description));
	mysql_real_escape_string(db,raceb,race_table[ch->race].name,strlen(race_table[ch->race].name));
	mysql_real_escape_string(db,longb,ch->long_descr,strlen(ch->long_descr));
	mysql_real_escape_string(db,shortb,ch->short_descr,strlen(ch->short_descr));

	cql_write(cql_query("INSERT INTO char_data\
	(isnpc,name,descr,acts,affs,coms,wizs,ress,version,spiritguide,sex,god,level,in_room,hp,mhp,mp,mmp,amp,mamp,mv,mmv,gold,silver,position,savingthrow,spellsavingthrow,align,hr,dr,ac0,ac1,ac2,ac3,wimpy,race,short_desc,long_desc)\
	VALUES('%d','%s','%s','%s','%s','%s','%s','%s','%d','%d','%d','%d',\
	'%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d','%d',\
	'%d','%d','%d','%d','%d','%d','%d','%s','%s','%s');",
		IS_NPC(ch) ? ch->pIndexData->vnum : 0,nameb,descb,IS_NPC(ch) ? save_bits(ch->act,MAX_ACT) : save_bits(ch->act,MAX_PLR),save_bits(ch->affected_by,MAX_AFF),save_bits(ch->comm,MAX_CMM),save_bits(ch->wiz,MAX_WIZ),save_ints(ch->res,MAX_RES),ch->version,ch->spiritguide,ch->sex,ch->god,
		ch->level,ch->in_room,ch->hit,ch->max_hit,ch->mana,ch->max_mana,ch->antimana,ch->max_antimana,ch->move,ch->max_move,ch->gold,ch->silver,ch->position,ch->saving_throw,ch->saving_spell_throw,ch->alignment,
		ch->hitroll,ch->damroll,ch->armor[0],ch->armor[1],ch->armor[2],ch->armor[3],ch->wimpy,raceb,shortb,longb
	));
	ch->id = n = mysql_insert_id(db);
	if(!IS_NPC(ch)){
		char lastnameb[MSL],afkb[MSL],poofib[MSL],poofob[MSL],titleb[MSL],createb[MSL],lastlogb[MSL],promptb[MSL];
		mysql_real_escape_string(db,promptb,ch->prompt,strlen(ch->prompt));
		mysql_real_escape_string(db,lastnameb,ch->pcdata->lname,strlen(ch->pcdata->lname));
		mysql_real_escape_string(db,afkb,ch->pcdata->afk,strlen(ch->pcdata->afk));
		mysql_real_escape_string(db,poofib,ch->pcdata->bamfin,strlen(ch->pcdata->bamfin));
		mysql_real_escape_string(db,poofob,ch->pcdata->bamfout,strlen(ch->pcdata->bamfout));
		mysql_real_escape_string(db,titleb,ch->pcdata->title,strlen(ch->pcdata->title));
		mysql_real_escape_string(db,createb,ch->pcdata->created,strlen(ch->pcdata->created));
		mysql_real_escape_string(db,lastlogb,ch->pcdata->lastlogin,strlen(ch->pcdata->lastlogin));
		cql_write(cql_query("INSERT INTO pc_data\
		(to_id,id,lname,height,weight,eye,hair,shair,credits,bprompt,deathtimer,trust,security,scroll,pk,pd,mk,md,permhit,permmana,permamp,permmv,bankgold,banksilver,trains,practices,s_practices,studies,xp,texp,lefty,afk,nobility,\
		invislevel,incoglevel,password,poofin,poofout,title,created,lastlogin,totaltime,points,truesex,last_level,cond_drunk,cond_full,cond_thirst,cond_hunger,s_studies0,s_studies1,s_studies2,s_studies3,timezone,statpoints,prompt)\
		VALUES('%d','','%s', '%d',  '%d',  '%d','%d','%d','%d',   '%d',   '%d',      '%d',  '%d',   '%d','%d','%d','%d','%d','%d','%d', '%d',   '%d',  '%d',    '%d',      '%d',   '%d',    '%d',       '%d', '%d','%d','%d','%s','%d',\
		'%d',      '%d',      '%s',    '%s',  '%s',   '%s', '%s',    '%s',     '%d',     '%d',  '%d',   '%d',     '%d',      '%d',     '%d',       '%d',        '%d','%d','%d','%d','%d','%d');",
			n,lastnameb,ch->looks[P_HEIGHT],ch->looks[P_WEIGHT],ch->looks[P_EYE],ch->looks[P_HAIR],ch->looks[P_SHAIR],ch->credits,ch->battleprompt,ch->death_timer,ch->trust,ch->pcdata->security,ch->lines,ch->kills[PK],ch->kills[PD],
			ch->kills[MK],ch->kills[MD],ch->pcdata->perm_hit,ch->pcdata->perm_mana,ch->pcdata->perm_antimana,ch->pcdata->perm_move,ch->bankgold,ch->banksilver,ch->pcdata->trains,ch->pcdata->practices,ch->pcdata->s_practices,
			ch->pcdata->studies,ch->exp,ch->texp,ch->lefty,afkb,ch->nobility,ch->invis_level,ch->incog_level,ch->pcdata->pwd,poofib,poofob,titleb,createb,lastlogb,ch->played + (int)(current_time - ch->logon),ch->pcdata->points,
			ch->pcdata->true_sex,ch->pcdata->last_level,ch->pcdata->condition[COND_DRUNK],ch->pcdata->condition[COND_FULL],ch->pcdata->condition[COND_THIRST],ch->pcdata->condition[COND_HUNGER],ch->pcdata->s_studies[0],
			ch->pcdata->s_studies[1],ch->pcdata->s_studies[2],ch->pcdata->s_studies[3],ch->pcdata->timezone,ch->pcdata->statpoints,promptb
		));
		cql_save_charclass(ch);//save class
		cql_save_charguild(ch);//save guild
		cql_save_charhome(ch);//save home
		cql_save_skills(ch);
	}
	cql_save_stats(ch);

	if(ch->carrying)//save char items
		cql_save_obj(ch,ch->carrying);
	cql_save_contents(ch->carrying);//save contents list
}
void cql_save_new_char(CHAR_DATA *pet,bool ispet){//this function determines if a new pet (id = 0) needs a new entry in the db, or will use the old pet's entry
	MYSQL_RES *res;
	MYSQL_ROW row;
	CHAR_DATA *ch = pet->master;
	int n;

	if(!ch){
		log_f("Error, pet with no owner trying to save? Pet: '%s'",pet->name);
		return;
	}
	//first we have to check if the character already has an entry for the pet/mount
	if(!cql_select("SELECT char_data.id FROM char_data,pet_data WHERE pet_data.pet_id = char_data.id AND pet_data.ispet = '%d';",pet->master->id))
		return;

	res = mysql_store_result(db);
	if(res){
		if((row = mysql_fetch_row(res))){//draw up the old entry for the previous pet
			log_f("Saving new pet to old file");
			pet->id = atoi(row[0]);
			cql_save_companion(ch,ispet);
		}
		else{//gotta make a new entry
			log_f("Saving new pet to new file");
			cql_save_new_char(pet);
			if(pet->id == 0){//Just a trap in case somehow the saver fails to generate an id, I'm paranoid like that
				log_f("cql_save_new_char(pet) ERROR: Something broke the pet saver, '%s' '%s'",ch->name,pet->name);
				return;
			}
			cql_write(cql_query("INSERT INTO pet_data (pet_id,ch_id,ispet,isalive) VALUES ('%d','%d','%d','1');",pet->id,ch->id,ispet));
			//add a thing to make sure it's saved into the character's group
		}
	}
}

void cql_save_char(CHAR_DATA *ch){
	if(ch->id == 0){//big uh oh
		log_f("Zero ID error with '%s'",ch->name);
		ch->send("A serious error has occured with your db entry. A note has been filed with the administrators and you may lose some data.\n\r");
		cql_save_new_char(ch);
	}

	cql_save_chardata(ch);//save the character crap
	cql_save_stats(ch);

	if(!IS_NPC(ch)){
		cql_save_pdata(ch);//pcdata
		cql_save_skills(ch);//skills
		cql_save_charmies(ch);//save charmies

		cql_save_companion(ch,true);//pet
		cql_save_companion(ch,false);//mount

		cql_write(cql_query("INSERT INTO char_guild (to_id,guild_id,rank) VALUES ('%d','%d','%d') ON DUPLICATE KEY UPDATE guild_id = '%d', rank = '%d';",//save guild
			ch->id,ch->guild,ch->rank,ch->guild,ch->rank));//guild
		cql_write(cql_query("INSERT INTO char_home (to_id,home_id) VALUES ('%d','%d') ON DUPLICATE KEY UPDATE home_id = '%d';",//save home
			ch->id,ch->hometown));//hometown
	}
	else{//insert save systems for progs and crap someday?
	}

	cql_write(cql_query("INSERT INTO char_class (to_id,class_id) VALUES ('%d','%d') ON DUPLICATE KEY UPDATE class_id = '%d';",//save class
		ch->id,ch->pclass,ch->pclass));//class

	cql_save_affects(ch);//save char affects

	if(ch->carrying)//save char items
		cql_save_obj(ch,ch->carrying);
	cql_save_contents(ch->carrying);//save contents list
	;//save group not yet
	cql_save_charhome(ch);//save home
}
