#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <mysql/mysql.h>

#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "db.h"
MYSQL *db;

/*
   Local Functions
*/
void init_blankguild(int);
void init_blankclass(int);
void init_blankchar(CHAR_DATA*,char*);
char *cql_query(char *fmt,...);
void init_mobile(CHAR_DATA*,int);
////


bool cql_select(char *fmt,...){
	char buf[MSL];
	int opt_flags;
	opt_flags |= CLIENT_MULTI_STATEMENTS;
	va_list args;
	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	va_end(args);
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

	if(mysql_query(db,buf)){
		log_f("cql_load_db() Error: %s",mysql_error(db));
		return false;
	}
	return true;
}

void cql_process_ranks(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	int i,hid,rid;
	log_f("Loading Ranks");
	while((row = mysql_fetch_row(res))){
		if((hid = atoi(row[0])) >= MAX_GUILD || (rid = atoi(row[1])) > MAX_RANKS){
			log_f("cql_process_ranks: ID out of range: %d, %d",hid,rid);
			continue;
		}
		
		guilds[hid].rank[rid].name		= str_dup(row[2]);
		guilds[hid].rank[rid].recruit	= atoi(row[3]);
		guilds[hid].rank[rid].expel		= atoi(row[4]);
		guilds[hid].rank[rid].promote	= atoi(row[5]);
		guilds[hid].rank[rid].demote	= atoi(row[6]);
		if(guilds[hid].rank_last < rid)
			guilds[hid].rank_last = rid;
		//log_f("\t\tLoading %s", guilds[hid].rank[rid].name );
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_houses(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	unsigned int i = 0;
	MAX_GUILD = mysql_num_rows(res);

	guilds = (guild_type *) calloc( MAX_GUILD, sizeof( *guilds ) );
	log_f("Loading Houses");

	while((row = mysql_fetch_row(res))){
		guilds[i].name		= str_dup(row[1]);
		guilds[i].who_name	= str_dup(row[2]);
		guilds[i].keywords	= str_dup(row[3]);
		guilds[i].active	= atoi(row[4]);
		guilds[i].index		= atoi(row[5]);
		guilds[i].type		= atoi(row[6]);
		guilds[i].hidden	= atoi(row[7]);
		guilds[i].recall	= atoi(row[8]);
		guilds[i].respawn	= atoi(row[9]);
		guilds[i].area		= atoi(row[10]);
		i++;
		//log_f("\t\tLoading %s", guilds[i].name );
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}

void cql_process_homes(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	unsigned int i = 0;
	MAX_HOMETOWN = mysql_num_rows(res);

	hometowns = (hometown_type *)calloc(MAX_HOMETOWN,sizeof(*hometowns));
	log_f("Loading Hometowns");

	while((row = mysql_fetch_row(res))){
		hometowns[i].id			= atoi(row[0]);
		hometowns[i].name		= str_dup(row[1]);//Also, some hometowns are missing. No bueno!
		hometowns[i].recall		= atoi(row[2]);
		hometowns[i].death		= atoi(row[3]);
		hometowns[i].map		= atoi(row[4]);
		hometowns[i].donor		= atoi(row[5]);
		hometowns[i].canuse		= atoi(row[6]);
		i++;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}

void cql_process_home_guilds(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	unsigned int i = 0;
	unsigned int j = 0;

	log_f("Loading Hometown guilds");

	while((row = mysql_fetch_row(res))){
		i = atoi(row[0]);
		if(i > MAX_GUILD){
			log_f("cql_process_home_guilds ERROR: Invalid numbers: '%s' '%s'",row[0],row[1]);
			continue;
		}
		if((j = get_hometown_by_id(atoi(row[1]))) == -1){
			log_f("cql_process_home_guilds ERROR: Invalid home/guild '%s'/'%s'",row[1],row[0]);
			continue;
		}
		hometowns[j].guild = i;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}

void cql_process_home_races(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int i = 0;
	static int j = 0;
	static bool found;

	log_f("Loading Hometown races");

	while((row = mysql_fetch_row(res))){
		if((i = pc_race_lookup(row[0])) < 0){
			log_f("cql_process_home_races : shitty race '%s'");
			continue;
		}
		if((j = get_hometown_by_id(atoi(row[1]))) < 0){
			log_f("cql_process_home_races : shitty bool '%s'",row[1]);
			continue;
		}
		pc_race_table[i].home_use[j] = true;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}

void cql_process_classes(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int i = 0;

	log_f("Loading Classes");
	for(int n = 0;n < MAX_CLASS;n++){
		init_blankclass(n);
		for(i = 0;pc_race_table[i].name;i++)
			pc_race_table[i].class_use[n] = false;
	}

	while((row = mysql_fetch_row(res))){
		if((i = atoi(row[0])) < 0 || i >= MAX_CLASS){
			log_f("cql_process_classes: Invalid class ID");
			exit(0);
		}
		classes[i].name			= str_dup(row[1]);
		sprintf(classes[i].who_name,"%s",row[2]);
		classes[i].attr_prime	= atoi(row[3]);
		classes[i].skill_adept	= atoi(row[4]);
		classes[i].thac0_00		= atoi(row[5]);
		classes[i].thac0_32		= atoi(row[6]);
		classes[i].hp_min		= atoi(row[7]);
		classes[i].hp_max		= atoi(row[8]);
		classes[i].mp_min		= atoi(row[9]);
		classes[i].mp_max		= atoi(row[10]);
		classes[i].mv_min		= atoi(row[11]);
		classes[i].mv_max		= atoi(row[12]);
		classes[i].fMana		= atoi(row[13]);
		classes[i].ctier		= atoi(row[14]);
		classes[i].align		= atoi(row[15]);
		classes[i].amp			= atoi(row[16]);
		classes[i].msg_self		= str_dup(row[17]);
		classes[i].msg_other	= str_dup(row[18]);
		classes[i].active		= atoi(row[19]);
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_class_stats(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int i = 0;
	static int n = 0;

	log_f("Loading Class stats");

	while((row = mysql_fetch_row(res))){
		if((i = atoi(row[2])) < 0 || i >= MAX_CLASS){
			log_f("cql_process_class_stats: Invalid class");
			exit(0);
		}
		if((n = abbrev_stat_lookup(row[0])) < 0){
			log_f("cql_process_class_stats: Invalid stat %s",row[0]);
			exit(0);
		}
		classes[i].attribute[n]		= atoi(row[1]);
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_class_becomes(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int i = 0;
	static int n = 0;

	log_f("Loading Class becomes");

	while((row = mysql_fetch_row(res))){
		if((n = atoi(row[0])) < 0 || n >= MAX_CLASS){//from
			log_f("cql_process_class_becomes: Invalid class from");
			exit(0);
		}
		if((i = atoi(row[1])) < 0 || i >= MAX_CLASS){//to
			log_f("cql_process_class_becomes: Invalid class to");
			exit(0);
		}
		classes[n].becomes[i]		= true;
		classes[i].previous = n;
		for(int sn=0;sn<MAX_SKILL && skill_table[sn].name;sn++){
			if(skill_table[sn].cost[n] >= 0){
				skill_table[sn].skill_level[i]						= skill_table[sn].skill_level[n];
				skill_table[sn].cost[i]								= skill_table[sn].cost[n];
				for(int l = 0;l < 10; l++){
					if(skill_table[sn].skl[n].unlocks[l].sn > 0){
						skill_table[sn].skl[i].unlocks[l].sn		= skill_table[sn].skl[n].unlocks[l].sn;
						skill_table[sn].skl[i].unlocks[l].level		= skill_table[sn].skl[n].unlocks[l].level;
					}
					if(skill_table[sn].skl[n].unlock_by[l].sn > 0){
						skill_table[sn].skl[i].unlock_by[l].sn		= skill_table[sn].skl[n].unlock_by[l].sn;
						skill_table[sn].skl[i].unlock_by[l].level	= skill_table[sn].skl[n].unlock_by[l].level;
						skill_table[sn].cost[i] = 0;
					}

					if(skill_table[sn].skl[n].locks[l].sn > 0){
						skill_table[sn].skl[i].locks[l].sn			= skill_table[sn].skl[n].locks[l].sn;
						skill_table[sn].skl[i].locks[l].level		= skill_table[sn].skl[n].locks[l].level;
					}
				}
			}
		}
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_class_requirements(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int i = 0;
	static int j = 0;
	static int lvl = 0;
	static int n = 0;

	log_f("Loading Class requires");

	while((row = mysql_fetch_row(res))){
		if((i = skill_lookup(row[0])) < 0){//to
			log_f("cql_process_class_requirements: Invalid skilla");
			exit(0);
		}
		if((j = skill_lookup(row[1])) < 0){//from
			log_f("cql_process_class_requirements: Invalid skillb");
			exit(0);
		}
		if((lvl = atoi(row[2])) < 0 || lvl > 100){
			log_f("cql_process_class_requirements: Invalid level");
			exit(0);
		}
		if((n = atoi(row[3])) < 0 || n >= MAX_CLASS){
			log_f("cql_process_class_requirements: Invalid class '%s'",row[3]);
			exit(0);
		}
		for(int l = 0; l < 10; l++){
			if(skill_table[j].skl[n].unlocks[l].sn > 0)
				continue;
			if(l > 9){
				log_f("Max unlocks reached '%s' '%s' in class '%s'",row[0],row[1],classes[n].name);
				break;
			}
			skill_table[j].skl[n].unlocks[l].sn = i;
			skill_table[j].skl[n].unlocks[l].level = lvl;
			break;
		}
		for(int l = 0; l < 10; l++){
			if(skill_table[i].skl[n].unlock_by[l].sn > 0)
				continue;
			if(l > 9){
				log_f("Max needs reached '%s' '%s' in class '%s'",row[0],row[1],classes[n].name);
				break;
			}
			skill_table[i].skl[n].unlock_by[l].sn = j;
			skill_table[i].skl[n].unlock_by[l].level = lvl;
			break;
		}
		skill_table[i].cost[n] = 0;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_class_cancels(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int i = 0;
	static int j = 0;
	static int n = 0;

	log_f("Loading Class cancels");

	while((row = mysql_fetch_row(res))){
		if((i = skill_lookup(row[0])) < 0){//kills
			log_f("cql_process_class_cancels: Invalid skilla");
			exit(0);
		}
		if((j = skill_lookup(row[1])) < 0){//from
			log_f("cql_process_class_cancels: Invalid skillb");
			exit(0);
		}
		if((n = atoi(row[2])) < 0 || n >= MAX_CLASS){
			log_f("cql_process_class_cancels: Invalid class '%s'",row[3]);
			exit(0);
		}
		for(int l = 0; l < 10; l++){
			if(skill_table[j].skl[n].unlocks[l].sn > 0)
				continue;
			if(l > 9){
				log_f("Max cancels reached '%s' '%s' in class '%s'",row[0],row[1],classes[n].name);
				break;
			}
			skill_table[j].skl[n].locks[l].sn = i;
			skill_table[j].skl[n].locks[l].level = 1;//always 1
			break;
		}
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");

}
void cql_process_class_races(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int race = 0;
	static int n = 0;

	log_f("Loading Class races");

	while((row = mysql_fetch_row(res))){
		if((n = atoi(row[1])) < 0 || n >= MAX_CLASS){
			log_f("cql_process_class_races: Invalid class");
			exit(0);
		}
		race = race_lookup(row[0]);

		if (race == 0 || !race_table[race].pc_race)
			log_f("Unrecognized race %s",row[0]);
		else
			pc_race_table[race].class_use[n] = 1;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}

void cql_process_class_skills(MYSQL_RES *res,MYSQL_FIELD *field){
	MYSQL_ROW row;
	static int lvl = 0;
	static int dif = 0;
	static int sn = 0;
	static int n = 0;

	log_f("Loading Class skills");

	while((row = mysql_fetch_row(res))){
		if((sn = skill_lookup(row[0])) > MAX_SKILL || sn < 0){
			log_f("cql_process_class_skills: Invalid skill");
			exit(0);
		}
		if((lvl = atoi(row[1])) > 100 || lvl < 0){
			log_f("cql_process_class_skills: Invalid level");
			exit(0);
		}
		if((dif = atoi(row[2])) < 0){
			log_f("cql_process_class_skills: Invalid difficulty");
			exit(0);
		}
		if((n = atoi(row[3])) < 0 || n >= MAX_CLASS){
			log_f("cql_process_class_skills: Invalid class");
			exit(0);
		}
		if(skill_table[sn].cost[n] > 0)
			log_f("Overlapping skill %d: '%s' in class %d: '%s', %d",sn,row[0],n,classes[n].name,skill_table[sn].cost[n]);
		else{
			skill_table[sn].skill_level[n] = lvl;
			skill_table[sn].cost[n] = dif;
		}
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}

void cql_process_set(MYSQL_RES *res){
	MYSQL_FIELD *field = mysql_fetch_field(res);

	if(!str_cmp(field->table,"houses"))
		cql_process_houses(res,field);
	else if(!str_cmp(field->table,"house_ranks"))
		cql_process_ranks(res,field);
	else if(!str_cmp(field->table,"classes"))
		cql_process_classes(res,field);
	else if(!str_cmp(field->table,"class_stats"))
		cql_process_class_stats(res,field);
	else if(!str_cmp(field->table,"class_becomes"))
		cql_process_class_becomes(res,field);
	else if(!str_cmp(field->table,"class_requirements"))
		cql_process_class_requirements(res,field);
	else if(!str_cmp(field->table,"class_cancels"))
		cql_process_class_cancels(res,field);
	else if(!str_cmp(field->table,"class_races"))
		cql_process_class_races(res,field);
	else if(!str_cmp(field->table,"class_skills"))
		cql_process_class_skills(res,field);
	else if(!str_cmp(field->table,"hometowns"))
		cql_process_homes(res,field);
	else if(!str_cmp(field->table,"home_races"))
		cql_process_home_races(res,field);
	else if(!str_cmp(field->table,"home_guilds"))
		cql_process_home_guilds(res,field);
}
void cql_load_pet_data(CHAR_DATA *pet){
	MYSQL_RES *res;
	MYSQL_ROW row;
	CHAR_DATA *ch;
	int percent;

	if(!cql_select("SELECT pet_data.* FROM pet_data WHERE pet_data.pet_id = '%d';",pet->id))
		return;

	res = mysql_store_result(db);
	if(res){
		if((row = mysql_fetch_row(res))){
			if(!(ch = get_char_by_id(atoi(row[2])))){
				log_f("cql_load_pet_data(): Invalid pet '%s' owner '%s'",pet->name,row[2]);
				mysql_free_result(res);
				return;
			}
			if(atoi(row[3]) == 1){//ispet
				group_char(ch,pet);
				pet->master		= ch;
				ch->pet			= pet;
				pet->alignment	= ch->alignment;
				init_race(pet);
				if(pet->in_room != ch->in_room)
					ch->send("Your mount is in another room.\n\r");
				if(is_class(ch,CLASS_IMMORTAL))
					pet->setform(FRM_INSTANT_DECAY);//Cheap catch for necros
			}
			else{//ismount
				ch->mount = pet;
				pet->mount = ch;
				pet->leader = ch;
				pet->master = ch;
				pet->ismount = true;
				init_race(pet);
				if(pet->in_room != ch->in_room)
					ch->send("Your mount is in another room.\n\r");
				if(is_class(ch,CLASS_IMMORTAL))
					pet->setform(FRM_INSTANT_DECAY);//Cheap catch for necros
			}
			mysql_free_result(res);
			return;
		}
	}
	else{
		if(!mysql_field_count(db))
			log_f("cql_load_pet_data() : Number of rows affected: %lu\n",(unsigned long)mysql_affected_rows(db));
		else
			log_f("cql_load_pet_data() : Could not retrieve result set.");
	}
	return;
}
void cql_process_char_data(CHAR_DATA *ch,MYSQL_RES *res){
	CHAR_DATA *to = ch;
	MYSQL_ROW row;
	static int i;
	bool isnpc = false;

	log_f("moo");
	if(!(row = mysql_fetch_row(res))){
		//log_f("Error loading character '%d'",ch->id);  Not needed since dead pets come here too
		return;
	}

	if((i = atoi(row[1])) != 0){
		to = create_mobile(get_mob_index(i));
		isnpc = true;
		to->id = atoi(row[0]);
	}

	to->name				= str_dup(row[2]);
	to->description			= str_dup(row[3]);
	if(isnpc)
		read_bits(to->act,MAX_ACT,row[4]);
	else
		read_bits(to->act,MAX_PLR,row[4]);
	read_bits(to->affected_by,MAX_AFF,row[5]);
	read_bits(to->comm,MAX_PLR,row[6]);
	read_bits(to->wiz,MAX_WIZ,row[7]);
	read_ints(to->res,MAX_RES,row[8]);
	to->version				= atoi(row[9]);
	to->spiritguide			= atoi(row[10]);
	to->sex					= atoi(row[11]);
	to->god					= atoi(row[12]);
	to->level				= atoi(row[13]);
	to->in_room				= get_room_index(atoi(row[14]));
	to->hit					= atoi(row[15]);
	to->max_hit				= atoi(row[16]);
	to->mana				= atoi(row[17]);
	to->max_mana			= atoi(row[18]);
	to->antimana			= atoi(row[19]);
	to->max_antimana		= atoi(row[20]);
	to->move				= atoi(row[21]);
	to->max_move			= atoi(row[22]);
	to->gold				= atoi(row[23]);
	to->silver				= atoi(row[24]);
	to->position			= atoi(row[25]);
	to->saving_throw		= atoi(row[26]);
	to->saving_spell_throw	= atoi(row[27]);
	to->alignment			= atoi(row[28]);
	to->hitroll				= atoi(row[29]);
	to->damroll				= atoi(row[30]);
	to->armor[0]			= atoi(row[31]);
	to->armor[1]			= atoi(row[32]);
	to->armor[2]			= atoi(row[33]);
	to->armor[3]			= atoi(row[34]);
	to->wimpy				= atoi(row[35]);
	to->race				= race_lookup(row[36]);

	if(isnpc)
		cql_load_pet_data(to);

	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_modstats(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	static int i;
	static int j = 0;

	while((row = mysql_fetch_row(res))){
		if((i = abbrev_stat_lookup(row[1])) < 0){
			log_f("Bad stat '%s'",row[1]);
			continue;
		}
		if(!row[2][0]){
			log_f("Bad stat '%s' '%s'",row[1],row[2]);
			ch->mod_stat[i] = 0;
			continue;
		}
		ch->mod_stat[i] = atoi(row[2]);
	}

	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_maxstats(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	static int i;
	static int j = 0;

	while((row = mysql_fetch_row(res))){
		if((i = abbrev_stat_lookup(row[1])) < 0){
			log_f("Bad stat '%s'",row[1]);
			continue;
		}
		if(!row[2][0]){
			log_f("Bad stat '%s' '%s'",row[1],row[2]);
			ch->max_stat[i] = pc_race_table[ch->race].max_stats[i];
			continue;
		}
		ch->max_stat[i] = atoi(row[2]);
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_stats(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	static int i = 0;
	static int j = 0;

	while((row = mysql_fetch_row(res))){
		if((i = abbrev_stat_lookup(row[1])) < 0){
			log_f("Bad stat '%s'",row[1]);
			continue;
		}
		if((j = atoi(row[2])) < 0 || j > STAT_MAX){
			log_f("Bad stat field '%s'",row[2]);
			continue;
		}
		ch->perm_stat[i] = j;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_class(CHAR_DATA *ch,MYSQL_RES *res){
MYSQL_ROW row;
	static int i = 0;

	while((row = mysql_fetch_row(res))){
		if((i = atoi(row[1])) < 0 || i > MAX_CLASS){
			log_f("Invalid class '%d' for char '%d'",i,ch->id);
			ch->pclass = 0;
			return;
		}
		ch->pclass	= i;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_skills(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	static int i = 0;
	static int j = 0;
	static int k = 0;
	static int n = 0;

	while((row = mysql_fetch_row(res))){
		if((n = skill_lookup(row[4])) < 0){
			log_f("cql_process_char_skills: Unknown skill '%s'",row[4]);
			continue;
		}
		if((i = atoi(row[1])) < 0 || i > 1){//unlocked should be 1 or 0
			log_f("cql_process_char_skills: Invalid unlock");
			exit(0);
		}
		if((j = atoi(row[2])) < 0 || j > 5){//level should be 0(-1?) to 5
			log_f("cql_process_char_skills: Invalid level");
			exit(0);
		}
		if((k = atoi(row[3])) < -1 || k > 100){//learned should be 0 to 100
			log_f("cql_process_char_skills: Invalid learned");
			exit(0);
		}
		ch->pcdata->unlocked[n]		= i;
		ch->pcdata->skill_level[n]	= j;
		ch->pcdata->learned[n]		= k;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_guild(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	static int i = 0;
	static int j = 0;

	while((row = mysql_fetch_row(res))){
		if((i = atoi(row[1])) < 0 || i > MAX_GUILD){
			log_f("Invalid guild '%d' for char '%d'",i,ch->id);
			ch->guild = 0;
			ch->rank = guilds[0].rank_last;
			return;
		}
		if((j = atoi(row[2])) < 0 || j > guilds[i].rank_last){
			log_f("Invalid guild rank '%d' for char '%s' and guild '%d'",j,ch->id,i);
			ch->guild = 0;
			ch->rank = guilds[0].rank_last;
			return;
		}
		ch->guild	= i;
		ch->rank	= j;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_group(CHAR_DATA *ch,MYSQL_RES *res){
}
void cql_process_char_home(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	static int i = 0;

	while((row = mysql_fetch_row(res))){// with these I want to throw out the ones with incompatible id's
		if((i = atoi(row[1])) < 0 || i > MAX_HOMETOWN){
			log_f("Invalid hometown '%d' for char '%d'",i,ch->id);
			ch->hometown = 0;
			return;
		}
		ch->hometown	= i;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}

void cql_process_char_items(CHAR_DATA *ch,MYSQL_RES *res){
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	MYSQL_ROW row;
	static int i = 0;
	static int j = 0;
	static int n = 0;

	while((row = mysql_fetch_row(res))){
		if(!is_number(row[0]) || !is_number(row[1])){
			log_f("Invalid item id '%s', vnum '%s'",row[0],row[1]);
			continue;
		}
		n = atoi(row[0]);//id
		i = atoi(row[1]);//vnum
		if (!(pObjIndex = get_obj_index(i))){
			log_f("cql_process_char_items: No object has vnum '%d'.\n\r",i);
			continue;
		}
		obj = NULL;
		obj = create_object(pObjIndex,0);
		obj->id = n;
		obj->enchanted = atoi(row[2]);//enchanted
		obj->name = str_dup(row[3]);//name
		obj->short_descr = str_dup(row[4]);//short;
		obj->description = str_dup(row[5]);//desc;
		obj->extra_flags = atoi(row[6]);//extraflag;
		obj->armortype_flags = atoi(row[7]);//armortype;
		obj->wear_flags = atoi(row[8]);//wearflag;
		obj->item_type = atoi(row[9]);//itemtype;
		obj->weight = atoi(row[10]);//weight;
		obj->condition = atoi(row[11]);//condition;
		obj->droprate = atoi(row[12]);//droprate;
		obj->wear_loc = atoi(row[13]);//wear;
		obj->level = atoi(row[14]);//level;
		obj->timer = atoi(row[15]);//timer;
		obj->cost = atoi(row[16]);//cost;
		obj->material = str_dup(row[17]);//material;
		read_bits(obj->wflags,MAX_WPN,row[18]);//weaponflags;
		obj->value[0] = atoi(row[19]);//v0;
		obj->value[1] = atoi(row[20]);//v1;
		obj->value[2] = atoi(row[21]);//v2;
		obj->value[3] = atoi(row[22]);//v3;
		obj->value[4] = atoi(row[23]);//v4;

		obj_to_char(obj,ch);
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_contents(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	OBJ_DATA *pobj;
	OBJ_DATA *cobj;
	static int i = 0;
	static int n = 0;

	while((row = mysql_fetch_row(res))){
		if((i = atoi(row[0])) < 0 || !(pobj = get_obj_id(i))){
			log_f("cql_process_char_contents: Invalid pitem id '%s'",row[0]);
			continue;
		}
		if((n = atoi(row[1])) < 0 || !(cobj = get_obj_id(n))){
			log_f("cql_process_char_contents: Invalid citem id %s",row[1]);
			continue;
		}
		if(pobj->item_type != ITEM_CONTAINER){
			log_f("cql_process_char_contents: parent item not a container '%s'",row[0]);
			continue;
		}
		obj_from_char(cobj);
		obj_to_obj(cobj,pobj);
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_extradescs(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	EXTRA_DESCR_DATA *ed;
	OBJ_DATA *obj;
	static int i = 0;
	static int n = 0;

	while((row = mysql_fetch_row(res))){
		if((i = atoi(row[0])) < 0){
			log_f("cql_process_extradescs: Invalid id: '%s'",row[0]);
			exit(0);
		}
		if((n = atoi(row[1])) < 0 || !(obj = get_obj_id(n))){
			log_f("cql_process_char_contents: Invalid item id %s",row[1]);
			continue;
		}
		ed					= new_extra_descr();
		ed->id				= i;
		ed->description		= str_dup(row[2]);
		ed->keyword			= str_dup(row[3]);
		ed->next			= obj->extra_descr;
		obj->extra_descr	= ed;
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char_affects(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_ROW row;
	CHAR_DATA *tch;
	OBJ_DATA *tobj;
	AFFECT_DATA *paf;
	static bool tochar = false;
	static int i = 0;
	static int n = 0;
	static int sn = 0;

	while((row = mysql_fetch_row(res))){
		paf = new_affect();
		if((i = atoi(row[0])) < 0){
			log_f("cql_process_char_affects: Invalid id: '%s'",row[0]);
			continue;
		}
		if((n = atoi(row[1])) < 0){
			log_f("cql_process_char_affects: Invalid target id '%s'",row[1]);
			continue;
		}
		if((tochar = atoi(row[10])) > 1 || tochar < 0){
			log_f("cql_process_char_affects: Invalid bool '%s'",row[10]);
			continue;
		}
		sn = skill_lookup(row[2]);
		if (sn < 0)
			bug("Fread_obj: unknown skill.",0);
		else
			paf->type = sn;

		paf->id			= i;
		paf->where		= atoi(row[3]);
		paf->type		= atoi(row[4]);
		paf->level		= atoi(row[5]);
		paf->duration	= atoi(row[6]);
		paf->modifier	= atoi(row[7]);
		paf->location	= atoi(row[8]);
		paf->bitvector	= atoi(row[9]);
		paf->slvl		= atoi(row[11]);

		if(tochar){
			if(n != ch->id){
				if(!(tch = get_char_by_id(n)))
					log_f("cql_process_char_affects: Char DNE %d",n);
				else
					affect_to_char(tch,paf);
			}
			else
				affect_to_char(ch,paf);
		}
		else{
			if(!(tobj = get_obj_id(n)))
				log_f("cql_process_char_affects: Obj DNE %d",n);
			else
				affect_to_obj(tobj,paf);
		}
	}
	if(mysql_errno(db) != 0)
		log_f("mysql_fetch_row() failed");
}
void cql_process_char(CHAR_DATA *ch,MYSQL_RES *res){
	MYSQL_FIELD *field = mysql_fetch_field(res);

	if(!str_cmp(field->table,"char_data"))
		cql_process_char_data(ch,res);
	else if(!str_cmp(field->table,"char_modstats"))
		cql_process_char_modstats(ch,res);
	else if(!str_cmp(field->table,"char_maxstats"))
		cql_process_char_maxstats(ch,res);
	else if(!str_cmp(field->table,"char_stats"))
		cql_process_char_stats(ch,res);
	else if(!str_cmp(field->table,"char_skills"))
		cql_process_char_skills(ch,res);
	else if(!str_cmp(field->table,"char_guild"))
		cql_process_char_guild(ch,res);
	else if(!str_cmp(field->table,"char_class"))
		cql_process_char_class(ch,res);
	else if(!str_cmp(field->table,"char_home"))
		cql_process_char_home(ch,res);
	else if(!str_cmp(field->table,"items"))
		cql_process_char_items(ch,res);
	else if(!str_cmp(field->table,"item_contents"))
		cql_process_char_contents(ch,res);
	else if(!str_cmp(field->table,"extra_descs"))
		cql_process_char_extradescs(ch,res);
	else if(!str_cmp(field->table,"affects"))
		cql_process_char_affects(ch,res);
	else
		log_f("cql_process_char error : invalid field '%s'",field->table);
}

void cql_load_char_obj(CHAR_DATA *ch){
	MYSQL_RES *res;
	MYSQL_ROW row;
	bool finished = false;
	int status;

	if(!cql_select("SELECT * FROM char_data WHERE char_data.id = '%d'; \
								SELECT * FROM char_stats WHERE to_id = '%d'; \
								SELECT * FROM char_modstats WHERE to_id = '%d'; \
								SELECT * FROM char_maxstats WHERE to_id = '%d'; \
								SELECT * FROM char_skills WHERE to_id = '%d'; \
								SELECT * FROM char_guild WHERE to_id = '%d'; \
								SELECT * FROM char_home WHERE to_id = '%d'; \
								SELECT * FROM char_class WHERE to_id = '%d'; \
								SELECT items.* FROM items,item_owners WHERE item_owners.to_id = '%d' AND item_owners.i_id = items.id; SELECT item_contents.* FROM item_contents,item_owners WHERE item_owners.to_id = '%d' AND item_owners.i_id = item_contents.parent_id; SELECT extra_descs.* FROM extra_descs,items,item_owners WHERE extra_descs.to_id = items.id AND item_owners.i_id = items.id AND item_owners.to_id = '%d'; \
								SELECT affects.* FROM affects WHERE affects.to_id = '%d' AND affects.tochar = '1'; \
								SELECT affects.* FROM affects,items,item_owners WHERE affects.to_id = items.id AND item_owners.to_id = '%d' AND item_owners.i_id = items.id AND affects.tochar = '0';\
								SELECT char_data.* FROM char_data,pet_data WHERE pet_data.pet_id = char_data.id AND pet_data.ch_id = '%d' AND pet_data.isalive = '1';",
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id,
								ch->id
	))//add charm/pet/mount loads soon maybe in pcdata so this thing can do npcs too
		return;

	do{
		res = mysql_store_result(db);
		if(res){
			cql_process_char(ch,res);
			mysql_free_result(res);
		}
		else{
			if(!mysql_field_count(db))
				log_f("cql_load_char_obj() : Number of rows affected: %lu\n",(unsigned long)mysql_affected_rows(db));
			else{
				log_f("cql_load_char_obj() : Could not retrieve result set.");
				finished = true;
			}
		}
		status = mysql_next_result(db);
		if(status != 0){
			finished = true;
			if(status > 0)
				log_f("cql_load_db() : Could not execute statement.");
		}
	}while(!finished);
/////////////////////
	if (ch->race == 0)
		ch->race = race_lookup("human");

	ch->size = pc_race_table[ch->race].size;
	ch->dam_type = 17; 
}

bool cql_load_char(DESCRIPTOR_DATA *d,char *name){//this only loads the pcdata, and passes it on so that it can jack the PID and thus load the rest of the character
	MYSQL_RES *res;
	MYSQL_ROW row;
    CHAR_DATA *ch;

	ch = new_char();
    ch->pcdata = new_pcdata();
log_f("Poo");
    d->character = ch;
    ch->desc = d;

	init_blankchar(ch,name);
	if(!cql_select("SELECT pc_data.* FROM char_data, pc_data WHERE char_data.name = '%s' AND pc_data.to_id = char_data.id;",name))
		return false;
	res = mysql_store_result(db);
	if(res){
		if((row = mysql_fetch_row(res))){
			ch->id								= atoi(row[0]);
			ch->mid								= get_pc_id();
			log_f("Loading %s (#%d).",name,ch->id);
			ch->pcdata->lname					= str_dup(row[ 2]);
			ch->looks[P_HEIGHT]					=    atoi(row[ 3]);
			ch->looks[P_WEIGHT]					=    atoi(row[ 4]);
			ch->looks[P_EYE]					=    atoi(row[ 5]);
			ch->looks[P_HAIR]					=    atoi(row[ 6]);
			ch->looks[P_SHAIR]					=    atoi(row[ 7]);
			ch->credits							=    atoi(row[ 8]);
			ch->battleprompt					=    atoi(row[ 9]);
			ch->death_timer						=    atoi(row[10]);
			ch->trust							=    atoi(row[11]);
			ch->pcdata->security				=    atoi(row[12]);
			ch->lines							=    atoi(row[13]);
			ch->kills[PK]						=    atoi(row[14]);
			ch->kills[PD]						=    atoi(row[15]);
			ch->kills[MK]						=    atoi(row[16]);
			ch->kills[MD]						=    atoi(row[17]);
			ch->pcdata->perm_hit				=    atoi(row[18]);
			ch->pcdata->perm_mana				=    atoi(row[19]);
			ch->pcdata->perm_antimana			=    atoi(row[20]);
			ch->pcdata->perm_move				=    atoi(row[21]);
			ch->bankgold						=    atoi(row[22]);
			ch->banksilver						=    atoi(row[23]);
			ch->pcdata->trains					=    atoi(row[24]);
			ch->pcdata->practices				=    atoi(row[25]);
			ch->pcdata->s_practices				=    atoi(row[26]);
			ch->pcdata->studies					=    atoi(row[27]);
			ch->exp								=    atoi(row[28]);
			ch->texp							=    atoi(row[29]);
			ch->lefty							=    atoi(row[30]);
			ch->pcdata->afk						= str_dup(row[31]);
			ch->nobility						=    atoi(row[32]);
			ch->invis_level						=    atoi(row[33]);
			ch->incog_level						=    atoi(row[34]);
			ch->pcdata->pwd						= str_dup(row[35]);
			ch->pcdata->bamfin					= str_dup(row[36]);
			ch->pcdata->bamfout					= str_dup(row[37]);
			ch->pcdata->title					= str_dup(row[38]);
			ch->pcdata->created					= str_dup(row[39]);
			ch->pcdata->lastlogin				= str_dup(row[40]);
			ch->played							=    atoi(row[41]);
			ch->pcdata->points					=    atoi(row[42]);
			ch->pcdata->true_sex				=    atoi(row[43]);
			ch->pcdata->last_level				=    atoi(row[44]);
			ch->pcdata->condition[COND_DRUNK]	=    atoi(row[45]);
			ch->pcdata->condition[COND_FULL]	=    atoi(row[46]);
			ch->pcdata->condition[COND_THIRST]	=    atoi(row[47]);
			ch->pcdata->condition[COND_HUNGER]	=    atoi(row[48]);
			ch->pcdata->s_studies[0]			=    atoi(row[49]);
			ch->pcdata->s_studies[1]			=    atoi(row[50]);
			ch->pcdata->s_studies[2]			=    atoi(row[51]);
			ch->pcdata->s_studies[3]			=    atoi(row[52]);
			ch->pcdata->timezone				=    atoi(row[53]);
			ch->pcdata->statpoints				=    atoi(row[54]);
			ch->prompt							= str_dup(row[55]);

			mysql_free_result(res);
			return true;
		}
	}
	else{
		if(!mysql_field_count(db))
			log_f("cql_load_char() : Number of rows affected: %lu\n",(unsigned long)mysql_affected_rows(db));
		else
			log_f("cql_load_char() : Could not retrieve result set.");
	}
	return false;
}

void cql_load_db(){
	MYSQL_RES *res;
	MYSQL_ROW row;
	int i = 0,finished = false,status;

	//intializations
	for(int sn = 0;skill_table[sn].name && sn < MAX_SKILL;sn++){
		for(int i =0;i < MAX_CLASS;i++)
			skill_table[sn].cost[i] = 0;
	}
	log_f("Loading dbs.");

	if(!cql_select("SELECT * FROM houses; SELECT * FROM house_ranks; \
	SELECT * FROM classes; SELECT * FROM class_skills; SELECT * FROM class_becomes; SELECT * from class_requirements; SELECT * FROM class_cancels; SELECT * FROM class_races; SELECT * FROM class_stats;\
	SELECT * FROM hometowns; SELECT * FROM home_races; SELECT * FROM home_guilds;"))
		return;

	do{
		res = mysql_store_result(db);
		if(res){
			cql_process_set(res);
			mysql_free_result(res);
		}
		else{
			if(!mysql_field_count(db))
				log_f("cql_load_db() : Number of rows affected: %lu\n",(unsigned long)mysql_affected_rows(db));
			else{
				log_f("cql_load_db() : Could not retrieve result set.");
				finished = true;
			}
		}
		status = mysql_next_result(db);
		if(status != 0){
			finished = true;
			if(status > 0)
				log_f("cql_load_db() : Could not execute statement.");
		}
	}while(!finished);
}

void cql_init(){
	int opt_flags;
	opt_flags |= CLIENT_MULTI_STATEMENTS;
	db = mysql_init(NULL);
	if(!mysql_real_connect(db,NULL,"root","password","wake",0,NULL,opt_flags)){
		log_f("cql_init() Error: %s", mysql_error(db));
		return;
	}
}
