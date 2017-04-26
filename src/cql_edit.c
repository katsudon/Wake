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
void cql_write(char *argument);
char *cql_query(char *fmt,...);
void cql_save_classes(int);
void cql_save_char(CHAR_DATA*);
void cql_save_obj(CHAR_DATA*,OBJ_DATA*);
void cql_save_hometowns();
/*
	Body code
*/
void do_cql(CHAR_DATA *ch,char *argument){
	MYSQL_RES *res;
	MYSQL_ROW row;
	int i = 0,finished = false,status;
	char arg1[MIL],arg2[MIL],buf[MSL],dest[MSL],crap[MSL];
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	ch->printf("Args '%s' '%s'\n\r",arg1,arg2);
	if(!str_cmp(arg1,"characters")){
		cql_save_char(ch);
	}
	if(!str_cmp(arg1,"items")){
		ch->pcdata->temp_item_buf = 0;
		cql_save_obj(ch,ch->carrying);
	}
}

void do_classedit(CHAR_DATA *ch,char *argument){
	int cl = 0,race = 0;
	char arg1[MIL],arg2[MIL],arg3[MIL];

	argument = one_argument(argument,arg1);

	if((cl = class_lookup(arg1)) < 0){
		ch->send("That is an invalid class.\n\r");
		return;
	}

	if(!str_cmp(argument,"save")){
		ch->printf("Saving the class %s to the db.\n\r",classes[cl].name);
		cql_save_classes(cl);
		return;
	}

	argument = one_argument(argument,arg2);

	ch->printf("Input: '%s','%s','%s'\n\r",arg1,arg2,argument);

	if(!arg1[0] || !arg2[0] || !argument[0]){
		ch->send("Invalid syntax.\n\r");
		return;
	}
	if(!str_cmp(arg2,"active")){
		ch->printf("Setting class %s active status to %s.\n\r",classes[cl].name,classes[cl].active ? "false" : "true");
		classes[cl].active = !classes[cl].active;
	}
	else if(!str_cmp(arg2,"amp")){
		ch->printf("Setting class %s amp status to %s.\n\r",classes[cl].name,classes[cl].amp ? "false" : "true");
		classes[cl].amp = !classes[cl].amp;
	}
	else if(!str_cmp(arg2,"name")){
		ch->printf("Setting class %s name to %s.\n\r",classes[cl].name,argument);
		free_string(classes[cl].name);
		classes[cl].name = str_dup(argument);
	}
	else if(!str_cmp(arg2,"whoname")){
		if(strlen(argument) > 3){
			ch->send("Argument must be 3 characters long.\n\r");
			return;
		}
		ch->printf("Setting class %s whoname to %s.\n\r",classes[cl].name,argument);
		sprintf(classes[cl].who_name,"%s",argument);
	}
	else if(!str_cmp(arg2,"fmana")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s fmana to %s.\n\r",classes[cl].name,argument);
		classes[cl].fMana = atoi(argument);
	}
	else if(!str_cmp(arg2,"align")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s align to %s.\n\r",classes[cl].name,argument);
		classes[cl].align = atoi(argument);
	}
	else if(!str_cmp(arg2,"adept")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s adept to %s.\n\r",classes[cl].name,argument);
		classes[cl].skill_adept = atoi(argument);
	}
	else if(!str_cmp(arg2,"primestat")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s prime stat to %s.\n\r",classes[cl].name,argument);
		classes[cl].attr_prime = atoi(argument);
	}
	else if(!str_cmp(arg2,"thac00")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s thac00 to %s.\n\r",classes[cl].name,argument);
		classes[cl].thac0_00 = atoi(argument);
	}
	else if(!str_cmp(arg2,"thac32")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s thac32 to %s.\n\r",classes[cl].name,argument);
		classes[cl].thac0_32 = atoi(argument);
	}
	else if(!str_cmp(arg2,"hpmin")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s hpmin to %s.\n\r",classes[cl].name,argument);
		classes[cl].hp_min = atoi(argument);
	}
	else if(!str_cmp(arg2,"hpmax")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s hpmax to %s.\n\r",classes[cl].name,argument);
		classes[cl].hp_max = atoi(argument);
	}
	else if(!str_cmp(arg2,"mpmin")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s mpmin to %s.\n\r",classes[cl].name,argument);
		classes[cl].mp_min = atoi(argument);
	}
	else if(!str_cmp(arg2,"mpmax")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s mpmax to %s.\n\r",classes[cl].name,argument);
		classes[cl].mp_max = atoi(argument);
	}
	else if(!str_cmp(arg2,"mvmin")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s mvmin to %s.\n\r",classes[cl].name,argument);
		classes[cl].mv_min = atoi(argument);
	}
	else if(!str_cmp(arg2,"mvmax")){
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s mvmax to %s.\n\r",classes[cl].name,argument);
		classes[cl].mv_max = atoi(argument);
	}
	else if(!str_cmp(arg2,"castmsgs")){
		ch->printf("Setting class %s self cast msg to '%s'.\n\r",classes[cl].name,argument);
		free_string(classes[cl].msg_self);
		classes[cl].msg_self = str_dup(argument);
	}
	else if(!str_cmp(arg2,"castmsgo")){
		ch->printf("Setting class %s other cast msg to '%s'.\n\r",classes[cl].name,argument);
		free_string(classes[cl].msg_other);
		classes[cl].msg_other = str_dup(argument);
	}
	else if(!str_cmp(arg2,"race")){
		argument = one_argument(argument,arg3);
		if(!argument[0] || !arg3[0]){
			ch->send("Invalid syntax.\n\r");
			return;
		}
		if((race = race_lookup(argument)) < 0 || !pc_race_table[race].name){
			ch->send("Invalid race.\n\r");
			return;
		}
		if(!str_cmp(arg3,"add")){
			ch->printf("Adding race %s to the class %s.\n\r",pc_race_table[race].name,classes[cl].name);
			pc_race_table[race].class_use[cl] = true;
		}
		else if(!str_cmp(arg3,"drop")){
			ch->printf("Dropping race %s to the class %s.\n\r",pc_race_table[race].name,classes[cl].name);
			pc_race_table[race].class_use[cl] = false;
		}
		else{
			ch->send("Error!\n\r");
			return;
		}
	}
	else if(!str_cmp(arg2,"stat")){
		argument = one_argument(argument,arg3);
		if(!argument[0] || !arg3[0]){
			ch->send("Invalid syntax.\n\r");
			return;
		}
		if((race = abbrev_stat_lookup(arg3)) < 0){
			ch->send("Invalid stat.\n\r");
			return;
		}
		if(!is_number(argument)){
			ch->send("Argument must be numeric.\n\r");
			return;
		}
		ch->printf("Setting class %s stat %s chance to improve to %s.\n\r",classes[cl].name,arg3,argument);
		classes[cl].attribute[race] = atoi(argument);
	}
}

void do_hometownedit(CHAR_DATA *ch,char *argument){
	char arg1[MSL],arg2[MSL];
	int race,tohome;
	bool found = false;
	if(!argument[0]){
		ch->send("The hometowns are:\n\r");
		for(tohome = 0;tohome < MAX_HOMETOWN;tohome++){
			ch->printf(" %s\n\r",hometowns[tohome].name);
		}
		return;
	}
	if(!str_cmp(argument,"save")){
		ch->send("Saving the hometowns to the db.\n\r");
		cql_save_hometowns();
		return;
	}
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	ch->send(argument);
	if(!str_prefix(arg1,"race")){
		if((race = pc_race_lookup(arg2)) < 0){
			ch->send("Invalid race.\n\r");
			return;
		}
		for(tohome = 0;tohome < MAX_HOMETOWN;tohome++){
			if(!str_cmp(hometowns[tohome].name,argument)){
				found = true;
				break;
			}
			ch->printf("It is not %s.\n\r",hometowns[tohome].name);
		}
		if(!found){
			ch->send("No hometown found.\n\r");
			return;
		}
		pc_race_table[race].home_use[tohome] = 1 - pc_race_table[race].home_use[tohome];
		ch->printf("Toggling hometown '%s' to '%s'\n\r",hometowns[tohome].name,pc_race_table[race].home_use[tohome] ? "true" : "false");
	}
	if(!str_prefix(arg1,"guild")){
		if((tohome = home_lookup(arg2)) < 0){
			ch->send("Invalid home.\n\r");
			return;
		}
		if((race = guild_lookup(argument)) < 0){
			ch->send("Invalid guild.\n\r");
			return;
		}
		hometowns[tohome].guild = race;
		ch->printf("Setting hometown '%s' guild '%s'\n\r",hometowns[tohome].name,argument);
	}
}
void cql_delete_char(int d_id){
	MYSQL_RES *res;
	MYSQL_ROW row;
	bool finished = false;
	int status;
	cql_write(cql_query("DELETE FROM char_data WHERE id = '%d'; DELETE FROM pc_data WHERE to_id = '%d'; \
						DELETE FROM char_class WHERE to_id = '%d';\
						DELETE FROM char_guild WHERE to_id = '%d';\
						DELETE FROM char_home WHERE to_id = '%d';\
						DELETE FROM char_maxstats WHERE to_id = '%d';\
						DELETE FROM char_modstats WHERE to_id = '%d';\
						DELETE FROM char_stats WHERE to_id = '%d';\
						DELETE FROM char_skills WHERE to_id = '%d';\
						DELETE FROM items WHERE item_owners.to_id = '%d' AND item_owners.i_id = items.id;\
						DELETE FROM item_contents WHERE item_owners.to_id = '%d' AND item_owners.i_id = item_contents.parent_id;\
						DELETE FROM extra_descs,items,item_owners WHERE extra_descs.to_id = items.id AND item_owners.i_id = items.id AND item_owners.to_id = '%d'; \
						DELETE FROM affects WHERE affects.to_id = '%d' AND affects.tochar = '1'; \
						DELETE FROM affects WHERE affects.to_id = items.id AND item_owners.to_id = '%d' AND item_owners.i_id = items.id AND affects.tochar = '0';",
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id,
						d_id));
	do{
		res = mysql_store_result(db);
		if(res)
			mysql_free_result(res);
		else{
			if(!mysql_field_count(db))
				;
			else
				finished = true;
		}
		status = mysql_next_result(db);
		if(status != 0)
			finished = true;
	}while(!finished);
}
void do_changelog(CHAR_DATA *ch,char *argument){
	char buf[MSL];
	mysql_real_escape_string(db,buf,argument,strlen(argument));
	ch->printf("Changelog entry: '%s'\n\r",argument);
	cql_write(cql_query("INSERT INTO changelog SET name='%s',msg='%s',timestamp=CURDATE();",ch->name,buf));
}
