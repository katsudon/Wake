#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"


struct flag_stat_type
{
    const struct flag_type *structure;
    bool stat;
};

const struct flag_stat_type flag_stat_table[] =
{
/*  {	structure			stat	}, */
    {	area_flags,			false	},
    {   sex_flags,			true	},
    {   exit_flags,			false	},
    {   door_resets,		true	},
    {   room_flags,			false	},
    {   sector_flags,		true	},
    {   type_flags,			true	},
    {   extra_flags,		false	},
    {   wear_flags,			false	},
    {   act_flags,			false	},
    {   affect_flags,		false	},
    {   apply_flags,		true	},
    {   wear_loc_flags,		true	},
    {   wear_loc_strings,	true	},
    {   container_flags,	false	},
    {   form_flags,			false   },
    {   part_flags,			false   },
    {   ac_type,			true    },
    {   size_flags,			true    },
    {   position_flags,		true    },
    {   off_flags,			false   },
    {   def_flags,			false   },
    {   res_flags,			false   },
    {   weapon_class,		true    },
    {   trap_class,			true    },
    {   arrow_flags,		true    },
    {   weapon_type2,		false   },
    {   trap_type,			false   },
    {   apply_types,		true	},
    {   0,					0		}
};

bool is_stat(const struct flag_type *flag_table){
	int flag;

	for (flag = 0; flag_stat_table[flag].structure; flag++)
	{
		if ( flag_stat_table[flag].structure == flag_table && flag_stat_table[flag].stat )
			return true;
	}
	return false;
}
int flag_value( const struct flag_type *flag_table, char *argument){
	char word[MAX_INPUT_LENGTH];
	int bit,marked = 0;
	bool found = false;

	if ( is_stat(flag_table) )
		return flag_lookup(argument,flag_table);

	for (; ;){
		argument = one_argument(argument,word);

		if ( word[0] == '\0' )
			break;

		if ( ( bit = flag_lookup(word,flag_table) ) != NO_FLAG)
		{
			SET_BIT(marked,bit);
			found = true;
		}
	}

	if ( found )
		return marked;
	else
		return NO_FLAG;
}

char *flag_string(const struct flag_type *flag_table,int bits){
	static char buf[2][512];
	static int cnt = 0;
	int  flag;

	if ( ++cnt > 1 )
		cnt = 0;

	buf[cnt][0] = '\0';

	for(flag = 0;flag_table[flag].name;flag++){
		if(!is_stat(flag_table) && IS_SET(bits,flag_table[flag].bit)){
			strcat(buf[cnt]," ");
			strcat(buf[cnt],flag_table[flag].name);
		}
		else
		if ( flag_table[flag].bit == bits )
		{
			strcat(buf[cnt]," ");
			strcat(buf[cnt],flag_table[flag].name);
			break;
		}
	}
	return (buf[cnt][0] != '\0') ? buf[cnt]+1 : (char *)"none";
}

bool affect_compare(CHAR_DATA *ch, int sn)
{
	int i,n=0;
	
	for (i=1; i<32;++i)
		if( sn == 1 << i)
		{
			n = i;
			break;
		}
	return ch->isaff(n);
}

bool room_index_data :: israf(int bit){
	return affected_by[bit];
}

bool mob_stuff :: isaff(int bit){
	return affected_by[bit];
}
bool mob_stuff :: isform(int bit){
	return form[bit];
}
bool mob_stuff :: ispart(int bit){
	return parts[bit];
}
bool mob_stuff :: iscomm(int bit){
	return comm[bit];
}
bool mob_stuff :: isact(int bit){
	if (bit != AT_IS_NPC){
		if (!act[AT_IS_NPC])
			return false;
	}
	return act[bit];
}
bool mob_stuff :: isplr(int bit){
	if (bit != AT_IS_NPC){
		if (act[AT_IS_NPC])
			return false;
	}
	return act[bit];
}
bool mob_stuff :: isoff(int bit){
	return off_bits[bit];
}
bool mob_stuff :: isdef(int bit){
	return def_bits[bit];
}
bool mob_stuff :: iswiz(int bit){
	return wiz[bit];
}


bool room_index_data :: remraf(int bit){
	if (!israf(bit))
		return false;
	affected_by[bit] = false;
	return true;
}

bool mob_stuff :: remaff(int bit){
	if (!isaff(bit))
		return false;
	affected_by[bit] = false;
	return true;
}

bool mob_stuff :: remact(int bit){
	if (!isact(bit) || !act[AT_IS_NPC])
		return false;
	act[bit] = false;
	return true;
}

bool mob_stuff :: remplr(int bit){
	if (!isplr(bit) || act[AT_IS_NPC])
		return false;
	act[bit] = false;
	return true;
}

bool mob_stuff :: remcomm(int bit){
	if (!iscomm(bit))
		return false;
	comm[bit] = false;
	return true;
}

bool mob_stuff :: remoff(int bit){
	if (!isoff(bit))
		return false;
	off_bits[bit] = false;
	return true;
}

bool mob_stuff :: remdef(int bit){
	if (!isdef(bit))
		return false;
	def_bits[bit] = false;
	return true;
}

bool mob_stuff :: rempart(int bit){
	if (!ispart(bit))
		return false;
	parts[bit] = false;
	return true;
}

bool mob_stuff :: remform(int bit){
	if (!isform(bit))
		return false;
	form[bit] = false;
	return true;
}


bool mob_stuff :: remwiz(int bit){
	if (!wiz[bit])
		return false;
	wiz[bit] = false;
	return true;
}



bool room_index_data :: setraf(int bit){
	if (israf(bit))
		return false;
	affected_by[bit] = true;
	return true;
}

bool mob_stuff :: setaff(int bit){
	if (affected_by[bit])
		return false;
	affected_by[bit] = true;
	return true;
}
bool mob_stuff :: setact(int bit){
	if (isact(bit) || !act[AT_IS_NPC])
		return false;
	act[bit] = true;
	return true;
}
bool mob_stuff :: setplr(int bit){
	if (isplr(bit) || act[AT_IS_NPC])
		return false;
	act[bit] = true;
	return true;
}
bool mob_stuff :: setcomm(int bit){
	if (iscomm(bit))
		return false;
	comm[bit] = true;
	return true;
}
bool mob_stuff :: setwiz(int bit){
	if (wiz[bit])
		return false;
	wiz[bit] = true;
	return true;
}
bool mob_stuff :: setoff(int bit){
	if (isoff(bit))
		return false;
	off_bits[bit] = true;
	return true;
}
bool mob_stuff :: setdef(int bit){
	if (isdef(bit))
		return false;
	def_bits[bit] = true;
	return true;
}
bool mob_stuff :: setform(int bit){
	if (isform(bit))
		return false;
	form[bit] = true;
	return true;
}
bool mob_stuff :: setpart(int bit){
	if (ispart(bit))
		return false;
	parts[bit] = true;
	return true;
}


bool mob_stuff :: massaff(char *args){
	if (!strcmp(args,"none"))
		return true;
	return super_set_bit(affected_by,args,affect_flags,false);
}
bool mob_stuff :: massact(char *args){
	if (!strcmp(args,"none"))
		return true;
	if (!act[AT_IS_NPC])
		return false;
	return super_set_bit(act,args,act_flags,false);
}
bool mob_stuff :: massplr(char *args){
	if (!strcmp(args,"none"))
		return true;
	if (act[AT_IS_NPC])
		return false;
	return super_set_bit(act,args,plr_flags,false);
}
bool mob_stuff :: masscomm(char *args){
	if (!strcmp(args,"none"))
		return true;
	return super_set_bit(comm,args,comm_flags,false);
}
bool mob_stuff :: massoff(char *args){
	if (!strcmp(args,"none"))
		return true;
	return super_set_bit(off_bits,args,off_flags,false);
}
bool mob_stuff :: massdef(char *args){
	if (!strcmp(args,"none"))
		return true;
	return super_set_bit(def_bits,args,def_flags,false);
}
bool mob_stuff :: masspart(char *args){
	if (!strcmp(args,"none"))
		return true;
	return super_set_bit(parts,args,part_flags,false);
}
bool mob_stuff :: massform(char *args){
	if (!strcmp(args,"none"))
		return true;
	return super_set_bit(form,args,form_flags,false);
}


bool mob_stuff :: hasaffs(char *affs){
	return has_bits(affected_by,affs,affect_flags);
}
bool mob_stuff :: hasacts(char *affs){
	if (!act[AT_IS_NPC])
		return false;
	return has_bits(act,affs,act_flags);
}
bool mob_stuff :: hasplrs(char *affs){
	if (act[AT_IS_NPC])
		return false;
	return has_bits(act,affs,plr_flags);
}
bool mob_stuff :: hascomms(char *affs){
	return has_bits(comm,affs,comm_flags);
}
bool mob_stuff :: hasoffs(char *affs){
	return has_bits(off_bits,affs,off_flags);
}
bool mob_stuff :: hasdefs(char *affs){
	return has_bits(def_bits,affs,def_flags);
}
bool mob_stuff :: hasparts(char *affs){
	return has_bits(parts,affs,part_flags);
}
bool mob_stuff :: hasforms(char *affs){
	return has_bits(form,affs,form_flags);
}


bool mob_stuff :: toggleaffs(char *affs){
	return super_set_bit(affected_by,affs,affect_flags,true);
}
bool mob_stuff :: toggleacts(char *affs){
	if (!act[AT_IS_NPC])
		return false;
	return super_set_bit(act,affs,act_flags,true);
}
bool mob_stuff :: toggleplrs(char *affs){
	if (act[AT_IS_NPC])
		return false;
	return super_set_bit(act,affs,plr_flags,true);
}
bool mob_stuff :: togglecomms(char *affs){
	return super_set_bit(comm,affs,comm_flags,true);
}
bool mob_stuff :: toggleoffs(char *affs){
	return super_set_bit(off_bits,affs,off_flags,true);
}
bool mob_stuff :: toggledefs(char *affs){
	return super_set_bit(def_bits,affs,def_flags,true);
}
bool mob_stuff :: toggleparts(char *affs){
	return super_set_bit(parts,affs,part_flags,true);
}
bool mob_stuff :: toggleforms(char *affs){
	return super_set_bit(form,affs,form_flags,true);
}


void clone_bits(bool *original,bool *clone,int maxNum){
	for(int i = 0;i < maxNum;i++)
		clone[i] = original[i];
}
void clone_ints(int *original,int *clone,int maxNum){
	for(int i = 0;i < maxNum;i++)
		clone[i] = original[i];
}

char * save_bits(bool *bits,int maxNum){
	static char buf[MSL],buf2[10];
	buf[0] = '\0';
	for (int i = 0;i<maxNum;i++){
		if (bits[i]){
			sprintf(buf2," %d",i);
			strcat(buf,buf2);
		}
	}
	if (buf[0] == '\0')
		strcat(buf,"E~");
	else
		strcat(buf," E~");
	return buf;
}

char * save_ints(int *bits,int maxNum){
	static char buf[MSL],buf2[10];
	buf[0] = '\0';
	for (int i = 0;i<maxNum;i++){
		sprintf(buf2," %d",bits[i]);
		strcat(buf,buf2);
	}
	if (buf[0] == '\0')
		strcat(buf,"E~");
	else
		strcat(buf," E~");
	return buf;
}

void read_bits(bool *bits,int maxNum,char *readin){
	char arg[10];
	int tnum = 0,i;
	for (;;){
		readin = one_argument(readin,arg);
		if (!is_number(arg) || atoi(arg) > maxNum)
			return;
		bits[atoi(arg)] = true;
	}
}

void read_ints(int *bits,int maxNum,char *readin){
	char arg[10];
	int tnum = 0,i;
	for (i = 0;;i++){
		readin = one_argument(readin,arg);
		if (!is_number(arg))
			return;
		bits[i] = atoi(arg);
	}
}

bool super_set_bit(bool *bits,char *stuff,const struct flag_type *flag_table,bool toggle){
	char arg[MIL],msg[MIL];
	int i;
	bool found,tfound = false;
	for (;;){
		stuff = one_argument(stuff,arg);
		found = false;
		if (!arg[0])
			return tfound;
		for(i = 0;flag_table[i].name;i++){
			if (!str_prefix(arg,flag_table[i].name) && flag_table[i].settable && !found){
				if (toggle)
					bits[flag_table[i].bit] = 1 - bits[flag_table[i].bit];
				else
					bits[flag_table[i].bit] = true;
				tfound = true;
				found = true;
			}
		}
		if (!found){
			sprintf(msg,"Bad bitset: %s",arg);
			wiznet(msg,NULL,NULL,WZ_FLAGS,0,0);
		}
	}
	return tfound;
}

bool super_strip_bit(bool *bits,char *stuff,const struct flag_type *flag_table,bool toggle){
	char arg[MIL],msg[MIL];
	int i;
	bool found,tfound = false;

	for (;;){
		stuff = one_argument(stuff,arg);
		found = false;
		if (!arg[0])
			return tfound;
		for(i = 0;flag_table[i].name;i++){
			if (!str_prefix(arg,flag_table[i].name) && flag_table[i].settable && !found){
				if (toggle)
					bits[flag_table[i].bit] = 1 - bits[flag_table[i].bit];
				else
					bits[flag_table[i].bit] = false;
				tfound = true;
				found = true;
			}
		}
		if (!found){
			sprintf(msg,"Bad bitset: %s",arg);
			wiznet(msg,NULL,NULL,WZ_FLAGS,0,0);
		}
	}
	return tfound;
}

bool has_bits(bool *bits,char *argument,const struct flag_type *flag_table){
	char arg[MIL];
	int i;
	for (;;){
		argument = one_argument(argument,arg);
		if (!arg[0])
			break;
		for(i = 0;flag_table[i].name[0];i++)
			if (!str_prefix(arg,flag_table[i].name))
				return true;
		break;
	}
	return false;
}
