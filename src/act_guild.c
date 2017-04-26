#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "lookup.h"
#include "olc.h"

#include <stddef.h>
DECLARE_DO_FUN(do_wset);

//Global var
int MAX_GUILD;
int MAX_HOMETOWN;
guild_type		*guilds;
hometown_type	*hometowns;
/*
 * Local functions.
 */
void cql_save_houses();
void cql_save_houseranks(int,int);

void do_pardon(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	CHAR_DATA *victim;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if (!arg1[0] || !arg2[0]){
		ch->send("Syntax: pardon <character> <killer|thief>.\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg1))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (IS_NPC(victim)){
		ch->send("Not on NPC's.\n\r");
		return;
	}

	if (!str_cmp(arg2,"killer")){
		if (victim->isplr(PL_KILLER)){
			victim->remplr(PL_KILLER);
			ch->send("Killer flag removed.\n\r");
			victim->send("You are no longer a KILLER.\n\r");
		}
		return;
	}

	if (!str_cmp(arg2,"thief")){
		if (victim->isplr(PL_THIEF)){
			victim->remplr(PL_THIEF);
			ch->send("Thief flag removed.\n\r");
			victim->send("You are no longer a THIEF.\n\r");
		}
		return;
	}

	ch->send("Syntax: pardon <character> <killer|thief>.\n\r");
}

void do_lstring(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL],arg3[MIL];
	OBJ_DATA *obj;

	smash_tilde(argument);
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	strcpy(arg3,argument);

	if (!arg1[0] || !arg2[0] || !arg3[0]){
		ch->send("Syntax:\n\r");
		ch->send("  restring <{gobject{x> <{gfield{x> <{gstring{x>\n\r");
		ch->send("    fields: {gkeyword short long extended{x\n\r");
		return;
	}

	if (!(obj = get_obj_carry(ch,arg1,ch))){
		ch->send("You have nothing like that.\n\r");
		return;
	}

	if (!str_prefix(arg2,"keyword")){
		free_string(obj->name);
		strcat(arg3,"{x");
		obj->name = str_dup(arg3);
		return;
	}

	if (!str_prefix(arg2,"short")){
		free_string(obj->short_descr);
		strcat(arg3,"{x");
		obj->short_descr = str_dup(arg3);
		return;
	}

	if (!str_prefix(arg2,"long")){
		free_string(obj->description);
		strcat(arg3,"{x");
		obj->description = str_dup(arg3);
		return;
	}

	if (!str_prefix(arg2,"ed") || !str_prefix(arg2,"extended")){
		EXTRA_DESCR_DATA *ed;

		argument = one_argument(argument,arg3);
		if (argument == NULL){
			ch->send("Syntax: oset <object> ed <keyword> <string>\n\r");
			return;
		}

 		strcat(argument,"\n\r");

		ed = new_extra_descr();
		ed->keyword		 = str_dup(arg3);
		ed->description	 = str_dup(argument);
		ed->next		 = obj->extra_descr;
		obj->extra_descr = ed;
		return;
	}
	do_function(ch,&do_string,"");
}

void do_guildset(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int guild;
	char arg[MSL],arg2[MSL];

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);

	if(!arg[0] || !arg2[0]){
		ch->send("Who what where?\n\r");
		return;
	}

	if((guild = guild_lookup(arg2)) == -1){
		ch->send("What guild?\n\r");
		return;
	}

	if (!(victim = get_char_world(ch,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	victim->guild = guild;
	victim->rank = guilds[guild].rank_last;
	printf_to_char(ch,"%s is now in the guild %s\n\r",victim->name,guilds[victim->guild].name);
	printf_to_char(victim,"You are now in the guild %s\n\r",guilds[victim->guild].name);
	if(argument[0] && !str_cmp(argument,"leader")){
		victim->rank = 0;
		printf_to_char(ch,"%s is also the leader now.\n\r",victim->name);
	}
}

void guild_message(CHAR_DATA *ch,int guild,char *msg,CHAR_DATA *exclude_me){
	DESCRIPTOR_DATA *d;
	for(d = descriptor_list;d;d = d->next){
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;

		if(d->connected == CON_PLAYING && victim->guild == guild){
			if(exclude_me && exclude_me == victim)
				continue;
			if(ch){
				if(d->character != ch)
					act_new(msg,ch,NULL,d->character,TO_VICT,POS_SLEEPING);
			}
			else
					act_new(msg,NULL,NULL,d->character,TO_VICT,POS_SLEEPING);
		}
	}
}

int get_guild_key(char *arg){
	for(int i = 0;i<MAX_GUILD;i++){
		if(!str_prefix(arg,guilds[i].keywords))
			return i;
	}
	return -1;
}

void do_apply(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int guild;
	char arg[MSL];

	if(!argument[0]){
		ch->send("Syntax: apply <house>\n\r        apply <accept/invite> <person>\n\r        apply <withdraw/cancel>\n\r");
		return;
	}
	if((!str_cmp(argument,"cancel") || !str_cmp(argument,"withdraw")) && ch->petition > 0){
		ch->send("Withdrawing application.\n\r");
		guild_message(ch,guild,"$n has withdrawn $s application to your house.",NULL);
		ch->petition = 0;
		return;
	}
	if(!str_prefix("accept",argument)){
		argument = one_argument(argument,arg);
		if(!argument[0] || !(victim = get_char_world(ch,argument))){
			ch->send("Accept who?\n\r");
			return;
		}
		if(victim == ch){
			ch->send("Uh what?\n\r");
			return;
		}
		if(victim->petition != ch->guild || victim->petition == 0){
			ch->send("They are not petitioning your house.\n\r");
			return;
		}
		if(!guilds[ch->guild].rank[ch->rank].recruit){
			ch->send("You are not authorized to recruit.\n\r");
			return;
		}
		act("You have accepted $N into your house.",ch,NULL,victim,TO_CHAR);
		printf_to_char(victim,"You have been accepted into %s.\n\r",guilds[ch->guild].name);
		guild_message(victim,ch->guild,"$n has been accepted into the house.",NULL);
		victim->guild = ch->guild;
		victim->petition = 0;
		victim->rank = guilds[ch->guild].rank_last;
		return;
	}
	if((guild = get_guild_key(argument)) == -1
	|| guilds[guild].type == GTYPE_POPULACE
	|| guilds[guild].type == GTYPE_IMMORTAL
	|| guilds[guild].hidden){
		ch->send("That is not a valid house.\n\r");
		return;
	}
	if(guilds[ch->guild].type != GTYPE_POPULACE){
		ch->send("You may only apply to houses if you are not in one yourself.\n\r");
		return;
	}

	printf_to_char(ch,"Petitioning %s.\n\r",guilds[guild].name);
	ch->petition = guild;
	guild_message(ch,guild,"$n has submitted an application to your house.",NULL);
}

void do_expel(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char buf[MSL];

	if(!argument[0] || !(victim = get_char_world(ch,argument))){
		ch->send("Expel who?\n\r");
		return;
	}
	if(victim == ch){
		ch->send("Uh what?\n\r");
		return;
	}
	if(victim->guild != ch->guild){
		ch->send("They are not in your guild.\n\r");
		return;
	}
	if(!guilds[ch->guild].rank[ch->rank].expel){
		ch->send("You are not authorized to expel.\n\r");
		return;
	}
	act("You have expelled $N from your guild.",ch,NULL,victim,TO_CHAR);
	act("$n has kicked you out of $t.",ch,guilds[ch->guild].name,victim,TO_VICT);
	sprintf(buf,"%s has expelled $n from the guild.",ch->name);
	guild_message(ch,ch->guild,buf,NULL);
	victim->guild = hometowns[victim->hometown].guild;

	if(victim->guild == -1){
		log_f("%s bad guild.",victim->name);
		victim->guild = 0;
	}
	victim->petition = 0;
	victim->rank = 0;
}

void do_promote(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char buf[MSL];

	if(!argument[0] || !(victim = get_char_world(ch,argument))){
		ch->send("Promote who?\n\r");
		return;
	}
	if(victim == ch){
		ch->send("Uh what?\n\r");
		return;
	}
	if(victim->guild != ch->guild){
		ch->send("They are not in your guild.\n\r");
		return;
	}
	if(!guilds[ch->guild].rank[ch->rank].promote){
		ch->send("You are not authorized to promote.\n\r");
		return;
	}
	if(victim->rank-1 == ch->rank){
		ch->send("They'd be your rank then!\n\r");
		return;
	}
	victim->rank--;
	act("You have promoted $N to $t.",ch,guilds[ch->guild].rank[victim->rank].name,victim,TO_CHAR);
	act("$n has promoted you to $t.",ch,guilds[ch->guild].rank[victim->rank].name,victim,TO_VICT);
	sprintf(buf,"%s has promoted $n to %s.",ch->name,guilds[ch->guild].rank[victim->rank].name);
	guild_message(victim,ch->guild,buf,ch);
}

void do_demote(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	char buf[MSL];

	if(!argument[0] || !(victim = get_char_world(ch,argument))){
		ch->send("Promote who?\n\r");
		return;
	}
	if(victim == ch){
		ch->send("Uh what?\n\r");
		return;
	}
	if(victim->guild != ch->guild){
		ch->send("They are not in your guild.\n\r");
		return;
	}
	if(!guilds[ch->guild].rank[ch->rank].demote){
		ch->send("You are not authorized to demote.\n\r");
		return;
	}
	if(victim->rank == guilds[victim->guild].rank_last){
		ch->send("They're at the minimum rank!\n\r");
		return;
	}
	victim->rank++;
	act("You have demoted $N to $t.",ch,guilds[ch->guild].rank[victim->rank].name,victim,TO_CHAR);
	act("$n has demoted you to $t.",ch,guilds[ch->guild].rank[victim->rank].name,victim,TO_VICT);
	sprintf(buf,"%s has demoted $n to %s.",ch->name,guilds[ch->guild].rank[victim->rank].name);
	guild_message(victim,ch->guild,buf,ch);
}

bool is_same_guild(CHAR_DATA *ach,CHAR_DATA *bch){
	int aguild = ach->guild,bguild = bch->guild;
	if(ach->guild == bch->guild)
		return true;
	if(guilds[aguild].hidden)
		return false;//aguild = ;
	if(guilds[bguild].hidden)
		return false;//bguild = guilds[bch->guild].mask;
	return aguild == bguild;
}

void guildedit_rank(CHAR_DATA *ch,char *argument,int house){
	int rank;
	char arg[MIL],arg2[MIL];

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);

	if(!arg[0]){
		ch->send("Syntax: guildedit <guild> rank <rank> <field> <value>\n\r");
		return;
	}
	if(!str_cmp(arg,"new")){
		if(guilds[house].rank_last >= MAX_RANKS - 1){
			ch->send("This guild has its ranks filled.\n\r");
			return;
		}
		rank = guilds[house].rank_last + 1;
		ch->printf("Initializing rank %d.\n\r",rank);
		guilds[house].rank[rank].name = "Blank";
		if(rank > guilds[house].rank_last){
			guilds[house].rank_last = rank;
		}
		//cql_save_houseranks(house,rank);
		return;
	}
	if(!is_number(arg)){
		ch->send("Rank argument must be numeric.\n\r");
		return;
	}
	if((rank = atoi(arg)) < 0 || rank > 9){
		ch->send("Your options are 0 (leader) to 9 (lowest). Remember, you don't NEED all ten.\n\r");
		return;
	}
	if(!argument[0] || !arg2[0]){
		ch->send("Syntax: guildedit <guild> rank <rank> <field> <value>\n\r");
		return;
	}
	if(!str_cmp(arg2,"name")){
		ch->printf("Modifying guild %s rank %d name from %s to %s.\n\r",guilds[house].name,rank,guilds[house].rank[rank].name,argument);
		free_string(guilds[house].rank[rank].name);
		guilds[house].rank[rank].name = str_dup(argument);
	}
	else if(!str_cmp(arg2,"recruit")){
		ch->printf("Modifying guild %s rank %d recruit from %s to %s.\n\r",guilds[house].name,rank,guilds[house].rank[rank].recruit ? "true" : "false",argument);
		if(!str_cmp(argument,"true"))
			guilds[house].rank[rank].recruit = true;
		else if(!str_cmp(argument,"false"))
			guilds[house].rank[rank].recruit = false;
		else{
			ch->send("Argument must be 'true' or 'false'.\n\r");
			return;
		}
	}
	else if(!str_cmp(arg2,"expel")){
		ch->printf("Modifying guild %s rank %d expel from %s to %s.\n\r",guilds[house].name,rank,guilds[house].rank[rank].expel ? "true" : "false",argument);
		if(!str_cmp(argument,"true"))
			guilds[house].rank[rank].expel = true;
		else if(!str_cmp(argument,"false"))
			guilds[house].rank[rank].expel = false;
		else{
			ch->send("Argument must be 'true' or 'false'.\n\r");
			return;
		}
	}
	else if(!str_cmp(arg2,"promote")){
		ch->printf("Modifying guild %s rank %d promote from %s to %s.\n\r",guilds[house].name,rank,guilds[house].rank[rank].promote ? "true" : "false",argument);
		if(!str_cmp(argument,"true"))
			guilds[house].rank[rank].promote = true;
		else if(!str_cmp(argument,"false"))
			guilds[house].rank[rank].promote = false;
		else{
			ch->send("Argument must be 'true' or 'false'.\n\r");
			return;
		}
	}
	else if(!str_cmp(arg2,"demote")){
		ch->printf("Modifying guild %s rank %d expel from %s to %s.\n\r",guilds[house].name,rank,guilds[house].rank[rank].demote ? "true" : "false",argument);
		if(!str_cmp(argument,"true"))
			guilds[house].rank[rank].demote = true;
		else if(!str_cmp(argument,"false"))
			guilds[house].rank[rank].demote = false;
		else{
			ch->send("Argument must be 'true' or 'false'.\n\r");
			return;
		}
	}
}
void do_guildedit(CHAR_DATA *ch,char *argument){//when making this into an editor, be sure to add "reload" which will reset from db, exiting the editor will save
	int house;
	char arg[MIL],arg2[MIL];

	argument = one_argument(argument,arg);
	argument = one_argument(argument,arg2);

	if(!str_cmp(arg,"save")){
		ch->send("Saving the guilds to db.\n\r");
		cql_save_houses();
		return;
	}
	if(!arg[0] || !arg2[0] || !argument[0]){
		ch->send("Syntax: guildedit <guild> <field> <value>\n\r");
		return;
	}
	if((house = guild_lookup(arg)) < 0){
		ch->send("That is not a valid guild.\n\r");
		return;
	}
	if(!str_cmp(arg2,"name")){
		ch->printf("Changing guild %d name from '%s' to '%s'\n\r",house,guilds[house].name,argument);
		guilds[house].name = str_dup(argument);
	}
	else if(!str_cmp(arg2,"rank")){
		guildedit_rank(ch,argument,house);
		return;
	}
	else if(!str_cmp(arg2,"whoname")){
		ch->printf("Changing guild %d whoname from '%s' to '%s'\n\r",house,guilds[house].who_name,argument);
		guilds[house].who_name = str_dup(argument);
	}
	else if(!str_cmp(arg2,"keywords")){
		ch->printf("Changing guild %d keywords from '%s' to '%s'\n\r",house,guilds[house].keywords,argument);
		guilds[house].keywords = str_dup(argument);
	}
	else if(!str_cmp(arg2,"active")){
		if(!str_cmp(argument,"true")){
			ch->printf("Changing guild %d active status from '%s' to '%s'\n\r",house,guilds[house].active ? "true" : "false","true");
			guilds[house].active = true;
			return;
		}
		else if(!str_cmp(argument,"false")){
			ch->printf("Changing guild %d active status from '%s' to '%s'\n\r",house,guilds[house].active ? "true" : "false","false");
			guilds[house].active = false;
		}
		else
			ch->send("Argument must be 'true' or 'false'.\n\r");
	}
	else if(!str_cmp(arg2,"index")){
		if(!is_number(argument)){
			ch->send("The argument must be numerical.\n\r");
			return;
		}
		ch->printf("Changing guild %d index from '%d' to '%s'\n\r",house,guilds[house].index,argument);
	}
	else if(!str_cmp(arg2,"type")){
		if(!is_number(argument)){
			ch->send("The argument must be numerical.\n\r");
			return;
		}
		ch->printf("Changing guild %d type from '%d' to '%s'\n\r",house,guilds[house].type,argument);
		guilds[house].type = atoi(argument);
	}
	else if(!str_cmp(arg2,"hidden")){
		if(!str_cmp(argument,"true")){
			ch->printf("Changing guild %d hidden status from '%s' to '%s'\n\r",house,guilds[house].hidden ? "true" : "false","true");
			guilds[house].hidden = true;
			return;
		}
		else if(!str_cmp(argument,"false")){
			ch->printf("Changing guild %d hidden status from '%s' to '%s'\n\r",house,guilds[house].hidden ? "true" : "false","false");
			guilds[house].hidden = false;
		}
		else
			ch->send("Argument must be 'true' or 'false'.\n\r");
	}
	else if(!str_cmp(arg2,"recall")){
		if(!is_number(argument)){
			ch->send("The argument must be numerical.\n\r");
			return;
		}
		ch->printf("Changing guild %d recall room from '%d' to '%s'\n\r",house,guilds[house].recall,argument);
		guilds[house].recall = atoi(argument);
	}
	else if(!str_cmp(arg2,"respawn")){
		if(!is_number(argument)){
			ch->send("The argument must be numerical.\n\r");
			return;
		}
		ch->printf("Changing guild %d respawn room from '%d' to '%s'\n\r",house,guilds[house].respawn,argument);
		guilds[house].respawn = atoi(argument);
	}
	else if(!str_cmp(arg2,"area")){
		if(!is_number(argument)){
			ch->send("The argument must be numerical.\n\r");
			return;
		}
		ch->printf("Changing guild %d area from '%d' to '%s'\n\r",house,guilds[house].area,argument);
		guilds[house].area = atoi(argument);
	}
	else{
		ch->send("Invalid field.\n\r");
		return;
	}
}
