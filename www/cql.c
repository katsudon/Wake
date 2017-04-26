#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mysql++/mysql++.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

using namespace mysqlpp;

Connection conn(false);

/*
   Local Functions
*/
void init_blankguild(int);

StoreQueryResult cql_query_res(char *arg){
	//Query query = conn.query(arg);
	//StoreQueryResult res = query.store();
	return conn.query(arg).store();//res;
}
char *cql_string(char *arg){
	Query query = conn.query(arg);
	String str;
			return str_dup(query.str().c_str());
}

int cql_int(char *arg){
	char *str = cql_string(arg);
	if(is_number(str))
		return atoi(str);
	else
		log_f("Error retrieving int from query '%s'",arg);
}

void cql_write(char *arg){
	Query query = conn.query();
	query << arg;
	SimpleResult res = query.execute();
}

void cql_query(char *fmt,...){
	char buf[MSL];
	va_list args;
	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	va_end(args);
	
	cql_write(buf);
}
void do_cql(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	StoreQueryResult res;
	String str;

	ch->printf("Args '%s' '%s'\n\r",arg1,arg2);
		if(!str_cmp(arg1,"houses")){
			if(!arg2[0]){
				res = cql_query_res("select id, name, keywords, whoname from houses;");
				ch->printf("%s(%d)\n\r",res.table(),res.num_fields());
				for (size_t i = 0; i < res.num_rows(); ++i){
					for(size_t j = 0; j < res.num_fields();j++){
						str = res[i][j];
						ch->printf(" (%s|%s|{x)\n\r",res.field_name(j).c_str(),str.data());
					}
					ch->send("\n\r\n\r");
				}
				ch->send("\n\r");
			}
			else if(!str_cmp(arg2,"update")){
				for(int i = 0;i < MAX_GUILD;i++){
					ch->printf("Dumping %s into the db.\n\r",guilds[i].name);
					cql_query("INSERT INTO houses (name,whoname,keywords,isactive,hindex,housetype,hidden,recall,respawn,area)VALUES('%s', '%s', '%s', '%d', '%d', '%d', '%d', '%d', '%d','%d');",
						guilds[i].name,guilds[i].who_name,guilds[i].keywords,guilds[i].active,guilds[i].index,guilds[i].type,guilds[i].hidden,guilds[i].recall,guilds[i].respawn,guilds[i].area);
					ch->printf(" Dumping %s's ranks into the db.\n\r",guilds[i].name);
					for(int j = 0;j <= guilds[i].rank_last;j++){
					ch->printf("  Dumping %s into the db.\n\r",guilds[i].rank[j].name);
					cql_query("INSERT INTO house_ranks (hid,rank_id,name,recruit,expel,promote,demote)VALUES('%d', '%d', '%s', '%d', '%d', '%d', '%d');",
						i,j,guilds[i].rank[j].name,guilds[i].rank[j].recruit,guilds[i].rank[j].expel,guilds[i].rank[j].promote,guilds[i].rank[j].demote);
					}
				}
			}
			else if(!str_cmp(arg2,"ranks")){
				res = cql_query_res("select * from house_ranks;");
				ch->printf("%s(%d)\n\r",res.table(),res.num_fields());
				for (size_t i = 0; i < res.num_rows(); ++i){
					for(size_t j = 0; j < res.num_fields();j++){
						str = res[i][j];
						ch->printf(" (%s|%s|{x)\n\r",res.field_name(j).c_str(),str.data());
					}
					ch->send("\n\r\n\r");
				}
				ch->send("\n\r");
			}
		}
//		ch->printf("Entering '%s' into the db at %s\n\r",argument,(char *) ctime(&current_time));
//		cql_query("INSERT INTO notes (poster,subject,body,timestamp)VALUES('%s', '%s', '%s', now());",ch->name,"trial",argument);
}
void load_houses(){
	int hid,rid;
	StoreQueryResult res = cql_query_res("select * from houses;");
	StoreQueryResult res2 = cql_query_res("select * from house_ranks;");
	String str;

	for(int n = 0;n < MAX_GUILD;n++)
		init_blankguild(n);

	//Jack all data from houses db
	for (size_t i = 0; i < res.num_rows(); ++i){
		guilds[i].name		= str_dup((str = res[i]["name"]).data());
		guilds[i].who_name	= str_dup((str = res[i]["whoname"]).data());
		guilds[i].keywords	= str_dup((str = res[i]["keywords"]).data());
		guilds[i].active	= atoi((str = res[i]["isactive"]).data());
		guilds[i].index		= atoi((str = res[i]["hindex"]).data());
		guilds[i].type		= atoi((str = res[i]["housetype"]).data());
		guilds[i].hidden	= atoi((str = res[i]["hidden"]).data());
		guilds[i].recall	= atoi((str = res[i]["recall"]).data());
		guilds[i].respawn	= atoi((str = res[i]["respawn"]).data());
		guilds[i].area		= atoi((str = res[i]["area"]).data());
	}

	//Jack all data from house_ranks db and sort it
	for (size_t i = 0; i < res2.num_rows(); ++i){
		hid = atoi((str = res2[i]["hid"]).data());
		rid = atoi((str = res2[i]["rank_id"]).data());
		guilds[hid].rank[rid].name		= str_dup((str = res2[i]["name"]).data());
		guilds[hid].rank[rid].recruit	= atoi((str = res2[i]["recruit"]).data());
		guilds[hid].rank[rid].expel		= atoi((str = res2[i]["expel"]).data());
		guilds[hid].rank[rid].promote	= atoi((str = res2[i]["promote"]).data());
		guilds[hid].rank[rid].demote	= atoi((str = res2[i]["demote"]).data());
	}
}

void cql_init(){
	if (conn.connect("wake", 0, "root", "password"))
		return;
	else{
		log_f("DB connection failed: %s",conn.error());
		exit(0);
	}
}
