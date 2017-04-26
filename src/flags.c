#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

l_int flag_lookup ( const char*,const struct flag_type* );

void do_flag(CHAR_DATA *ch,char *argument){
	char arg1[MSL],arg2[MSL],arg3[MSL],arg4[MSL], word[MIL];
	CHAR_DATA *victim;
	char type;
	bool found=false;
	int i;
	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	argument = one_argument(argument,arg3);
	argument = one_argument(argument,arg4);

	type = argument[0];

	if (type == '=' || type == '-' || type == '+')
		argument = one_argument(argument,word);

	if (arg1[0] == '\0'){
		send_to_char("Syntax:\n\r",ch);
		send_to_char("  flag mob  <name> <field> <flag>\n\r",ch);
		send_to_char("  flag char <name> <field> <flag>\n\r",ch);
		return;
	}

	if (arg2[0] == '\0'){
		send_to_char("What do you wish to set flags on?\n\r",ch);
		return;
	}

	if (arg3[0] == '\0'){
		send_to_char("You need to specify a flag to set.\n\r",ch);
		return;
	}

	if (arg4[0] == '\0'){
		send_to_char("Which flags do you wish to change?\n\r",ch);
		return;
	}

	if (!str_prefix(arg1,"mob") || !str_prefix(arg1,"char")){
		victim = get_char_world(ch,arg2);
		if (victim == NULL){
			send_to_char("You can't find them.\n\r",ch);
			return;
		}

		if (!str_prefix(arg3,"act")){
			if (!IS_NPC(victim)){
				send_to_char("Not for PCs.\n\r",ch);
				return;
			}

			for(i=0; act_flags[i].name != NULL; i++){
				if(!str_prefix(arg4,act_flags[i].name)){
					if(!victim->setact(act_flags[i].bit))
						victim->remact(act_flags[i].bit);
					found=true;
				}
			}
			if(found){
				send_to_char("Act flag toggled.\n\r",ch);
				return;
			}
			else{
				send_to_char("No such Act flag found.\n\r",ch);
				return;
			}
		}
 		else if (!str_prefix(arg3,"aff")){
			for(i=0; affect_flags[i].name != NULL; i++){
				if(!str_prefix(arg4,affect_flags[i].name)){
					if(!victim->setaff(affect_flags[i].bit))
						victim->remaff(affect_flags[i].bit);
					found=true;
				}
			}
			if(found){
				send_to_char("Aff flag toggled.\n\r",ch);
				return;
			}
			else{
				send_to_char("No such Affect flag found.\n\r",ch);
				return;
			}
		}
		else if (!str_prefix(arg3,"form")){
			if (!IS_NPC(victim) && ch->level < MAX_LEVEL-2){
	 			send_to_char("Form can't be set on PCs.\n\r",ch);
				return;
			}
			for(i=0; form_flags[i].name != NULL; i++){
				if(!str_prefix(arg4,form_flags[i].name)){
					if(!victim->setform(form_flags[i].bit))
						victim->remform(form_flags[i].bit);
					found=true;
				}
			}
			if(found){
				send_to_char("Form flag toggled.\n\r",ch);
				return;
			}
			else{
				send_to_char("No such Frm flag found.\n\r",ch);
				return;
			}
		}
		else if (!str_prefix(arg3,"parts")){
			if (!IS_NPC(victim) && ch->level < MAX_LEVEL-2){
				send_to_char("Parts can't be set on PCs.\n\r",ch);
				return;
			}
			for(i=0; part_flags[i].name != NULL; i++){
				if(!str_prefix(arg4,part_flags[i].name)){
					if(!victim->setpart(part_flags[i].bit))
						victim->rempart(part_flags[i].bit);
					found=true;
				}
			}
			if(found){
				send_to_char("Part flag toggled.\n\r",ch);
				return;
			}
			else{
				send_to_char("No such Prt flag found.\n\r",ch);
				return;
			}
		}
		else if (!str_prefix(arg3,"comm")){
			if (IS_NPC(victim)){
				send_to_char("Comm can't be set on NPCs.\n\r",ch);
				return;
			}
			for(i=0; comm_flags[i].name != NULL; i++){
				if(!str_prefix(arg4,comm_flags[i].name)){
					if(!victim->setcomm(comm_flags[i].bit))
						victim->remcomm(comm_flags[i].bit);
					found=true;
				}
			}
			if(found){
				send_to_char("Comm flag toggled.\n\r",ch);
				return;
			}
			else{
				send_to_char("No such Cmm flag found.\n\r",ch);
				return;
			}
		}
		else {
			send_to_char("That's not an acceptable flag.\n\r",ch);
			return;
		}
	}
	if (!str_prefix(arg1,"obj")){
		OBJ_DATA *obj;
		if ((obj = get_obj_carry(ch,arg2,ch)) == NULL){
			if ((obj = get_obj_wear(ch,arg2,true)) == NULL){
				ch->send("You don't have that.\n\r");
				return;
			}
		}
		if (!str_prefix(arg3,"extra")){
			for(i=0; extra_flags[i].name != NULL; i++){
				if(!str_prefix(arg4,extra_flags[i].name)){
					if(!IS_SET(obj->extra_flags,extra_flags[i].bit))
						SET_BIT(obj->extra_flags,extra_flags[i].bit);
					else
						REMOVE_BIT(obj->extra_flags,extra_flags[i].bit);
					found = true;
				}
			}
			if(found){
				send_to_char("Extra flag toggled.\n\r",ch);
				return;
			}
			else{
				send_to_char("No such Extra flag found.\n\r",ch);
				return;
			}
		}
	}
}



    
