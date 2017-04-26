#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"

/*
 * Local functions.
 */
/*int get_fuel_efficiency		( OBJ_DATA* );
int get_forge_efficiency	( OBJ_DATA*,int );
int get_metal_sn			( CHAR_DATA*,int );
bool is_forge_type			( CHAR_DATA*,OBJ_DATA*,int );
bool validate_metal_forge	( CHAR_DATA*,OBJ_DATA* );
bool validate_fuel_forge	( CHAR_DATA*,OBJ_DATA* );
int validate_metal_knowledge( CHAR_DATA*,string,int );
OBJ_DATA *find_ingot		( OBJ_DATA* );

using namespace std;


void clear_forge(OBJ_DATA *forge){
	if (!forge || forge->item_type != ITEM_FORGE)
		return;
	REMOVE_BIT(forge->extra_flags,ITM_FORGING);
	forge->value[4] = -1;
	return;
}
void clear_forger(CHAR_DATA *ch){
	if (!ch)
		return;
	ch->remplr(PL_FORGING);
	ch->pcdata->forge = NULL;
}

void end_forging(CHAR_DATA *ch,OBJ_DATA *forge){
	act("$n stops $p.",ch,forge,NULL,TO_ROOM);
	act("You stop $p.",ch,forge,NULL,TO_CHAR);
	clear_forge(forge);
	clear_forger(ch);
	return;
}

void gen_slag(OBJ_DATA *forge,int weight){
	OBJ_DATA *slag;

	slag = create_object(get_obj_index(OBJ_VNUM_SLAG),1);
	slag->weight = weight;
	obj_to_obj(slag,forge);
}

void init_ingot(CHAR_DATA *ch,OBJ_DATA *forge,OBJ_DATA *ingot){
	ch->send("init_ingot\n\r");
	char buf[MSL];

	ingot->level = (ch->level + forge->level) / 2;
	ingot->material = str_dup(material_flags[forge->value[4]].name);
	ingot->weight = 1;
	sprintf(buf,"an ingot of %s",material_flags[forge->value[4]].name);
	ingot->short_descr = str_dup(buf);
	sprintf(buf,"An ingot of %s is here.",material_flags[forge->value[4]].name);
	ingot->description = str_dup(buf);
	sprintf(buf,"ingot %s",material_flags[forge->value[4]].name);
	ingot->name = str_dup(buf);
}

void grind_metal(CHAR_DATA *ch, OBJ_DATA *forge, bool useanything){
	ch->send("grind_metal\n\r");
	OBJ_DATA *ingot = find_ingot(forge),*shit,*shit_next;
	bool found = false;

	if (!validate_metal_forge(ch,forge)){
		ch->send("There is no appropriate metal in the forge.\n\r");
		end_forging(ch,forge);
		return;
	}
	if (forge->value[4] < 1){
		ch->send("Invalid forge material\n\r");
		end_forging(ch,forge);
		return;
	}

	if (!ingot){//make one
		ingot = create_object(get_obj_index(OBJ_VNUM_INGOT),0);
		init_ingot(ch,forge,ingot);
		obj_to_obj(ingot,forge);
	}

	for (shit = forge->contains;shit;shit = shit_next){
		shit_next = shit->next_content;
		if (shit->item_type == ITEM_FORGEFUEL || shit == ingot || !str_cmp(shit->name,"slag"))
			continue;
		if ((!useanything && shit->item_type != ITEM_INGOT) || str_cmp(shit->material,material_flags[forge->value[4]].name)){
			if (shit->weight < 10){
				act("$p slags $P.",ch,forge,shit,TO_CHAR);
				act("$p slags $P.",ch,forge,shit,TO_ROOM);
				shit->condition = 5;
				shit->item_type == ITEM_TRASH;
				continue;
			}
			gen_slag(forge,2);
			shit->weight -= 2;
			continue;
		}
		if (!found){
			if (shit->weight > 10){
				act("$p groans and puffs as it adds to your ingot of $T.",ch,forge,material_flags[forge->value[4]].name,TO_CHAR);
				act("$p groans and puffs as it adds to $n's ingot of $T.",ch,forge,material_flags[forge->value[4]].name,TO_ROOM);
				shit->weight -= 10;
				ingot->weight += get_forge_efficiency(forge,0);
				gen_slag(forge,10-get_forge_efficiency(forge,0));
			}
			else{
				act("$p groans and puffs and melts off $T, adding its mass to your ingot.",ch,forge,shit->short_descr,TO_CHAR);
				act("$p groans and puffs and melts off $T, adding its mass to $N's ingot.",ch,forge,shit->short_descr,TO_ROOM);
				ingot->weight += shit->weight/2;
				extract_obj(shit);
			}
			found = true;
		}
	}
}

void burn_fuel(CHAR_DATA *ch,OBJ_DATA *forge){
	ch->send("burn_fuel\n\r");
	OBJ_DATA *fuel,*fuel_next;
	int toburn = get_forge_efficiency(forge,1);

	for (fuel = forge->contains;fuel && toburn > 0;fuel = fuel_next){
		fuel_next = fuel->next_content;

		if (fuel->item_type != ITEM_FORGEFUEL)
			continue;

		if (fuel->weight <= toburn){
			toburn -= fuel->weight;
			act("$p burns off $P.",ch,forge,fuel,TO_CHAR);
			act("$p burns off $P.",ch,forge,fuel,TO_ROOM);
			extract_obj(fuel);
		}
		else{
			act("$p burns $P.",ch,forge,fuel,TO_CHAR);
			act("$p burns $P.",ch,forge,fuel,TO_ROOM);
			fuel->weight -= toburn;
			return;
		}
	}
}

void charforge_update(CHAR_DATA *ch){
	ch->send("charforge_update\n\r");
	OBJ_DATA *forge;
	if (IS_NPC(ch))
		return;
	if (!(forge = ch->pcdata->forge) || forge->value[4] < 1){
		ch->remplr(PL_FORGING);
		return;
	}

	int skill = validate_metal_knowledge(ch,"x",forge->value[4]);

	if (forge->value[3] == FORGE_JOINELEMENT || forge->value[3] == FORGE_JOINCOMPOUND){
		if (forge->in_room != ch->in_room){
			ch->send("You left your forge unattended, genius.\n\r");
			act("$p groans and sputters before dying out.",NULL,forge,NULL,TO_ROOM);
			end_forging(ch,forge);
			return;
		}
		if (!validate_fuel_forge(ch,forge)){
			ch->send("The forge sputters and dies out.\n\r");
			act("$p groans and sputters before dying out.",NULL,forge,NULL,TO_ROOM);
			end_forging(ch,forge);
			return;
		}
		if (skill < 1){
			end_forging(ch,forge);
			return;
		}
		if (number_percent() < skill){
			ch->send("UPDATE!\n\r");
			burn_fuel(ch,forge);
			grind_metal(ch,forge,forge->value[3] == FORGE_JOINELEMENT);
			check_improve(ch,get_metal_sn(ch,forge->value[4]),true,3);
			check_improve(ch,gsn_smelt,true,3);
		}
		else{
			burn_fuel(ch,forge);
			act("You miserably fail to do anything to $p.",ch,forge,NULL,TO_CHAR);
			act("$n miserably fails to do anything to $p.",ch,forge,NULL,TO_ROOM);
			check_improve(ch,get_metal_sn(ch,forge->value[4]),false,3);
			check_improve(ch,gsn_smelt,false,3);
		}
	}
	else if (forge->value[3] == FORGE_GENCOMPOUND){
	}
}

void smelt_join_start(CHAR_DATA *ch,OBJ_DATA *forge,int mnum,bool element){
	act("You begin to smelt $T in $p.",ch,forge,material_flags[mnum].name,TO_CHAR);
	act("$n begins to smelt $T in $p.",ch,forge,material_flags[mnum].name,TO_ROOM);
	SET_BIT(forge->extra_flags,ITM_FORGING);
	ch->setplr(PL_FORGING);
	ch->pcdata->forge = forge;
	if (element)
		forge->value[3] = FORGE_JOINELEMENT;
	else
		forge->value[3] = FORGE_JOINCOMPOUND;
	forge->value[4] = mnum;
}


void smelt_join(CHAR_DATA *ch,OBJ_DATA *forge,int mnum,string material){
	OBJ_DATA *fuel;
	int n,fuelweight = 0;
	char buf[MSL];

	if (mnum == -1 || material.empty()){
		ch->send("That is not a material.\n\r");
		return;
	}
	if (!material_flags[mnum].metal || !material_flags[mnum].forge){
		printf_to_char(ch,"%d, %s\n\r",mnum,material_flags[mnum].name);
		ch->send("You cannot smelt that material.\n\r");
		return;
	}
	if (!forge->contains){
		ch->send("It's empty!\n\r");
		return;
	}

	if(s_exact(material,"aluminum")
	|| s_exact(material,"copper")
	|| s_exact(material,"gold")
	|| s_exact(material,"iron")
	|| s_exact(material,"lead")
	|| s_exact(material,"silver")
	|| s_exact(material,"tin")
	|| s_exact(material,"zinc")){//smelting elements is a lot more lenient on materials
		if (!is_forge_type(ch,forge,HEAT_HIGH))
			return;
		if (!validate_fuel_forge(ch,forge))
			return;
		if (validate_metal_knowledge(ch,material,0) < 1){
			ch->send("You don't know how to smelt that.\n\r");
			return;
		}

		smelt_join_start(ch,forge,mnum,true);
	}
	else
	if(s_exact(material,"steel")
	|| s_exact(material,"orichalcum")
	|| s_exact(material,"bronze")
	|| s_exact(material,"mithril")
	|| s_exact(material,"adamantine")
	|| s_exact(material,"brass")){//smelting compounds only takes ingots
		if (s_exact(material,"steel") || s_exact(material,"orichalcum") || s_exact(material,"adamantine") || s_exact(material,"mithril")){
			if (!is_forge_type(ch,forge,HEAT_BLAST))
				return;
		}
		else{
			if (!is_forge_type(ch,forge,HEAT_HIGH))
				return;
		}
		if (!validate_fuel_forge(ch,forge))
			return;
		if (validate_metal_knowledge(ch,material,0) < 1){
			ch->send("You don't know how to smelt that.\n\r");
			return;
		}

		smelt_join_start(ch,forge,mnum,false);
	}
}
void smelt_compound(CHAR_DATA *ch,string compound){
}
void smelt_brass(CHAR_DATA *ch,OBJ_DATA *forge){
}
void smelt_bronze(CHAR_DATA *ch,OBJ_DATA *forge){
}
void smelt_steel(CHAR_DATA *ch,OBJ_DATA *forge){
}
void smelt_mithril(CHAR_DATA *ch,OBJ_DATA *forge){
}
void smelt_adamantine(CHAR_DATA *ch,OBJ_DATA *forge){
}
void smelt_orichalcum(CHAR_DATA *ch,OBJ_DATA *forge){
}

void do_smelt(CHAR_DATA *ch,char *argument){
	OBJ_DATA *forge;
	int skill = get_skill(ch,gsn_smelt);
	char arg1[MSL],arg2[MSL];
	string metal,arg;

	argument = one_argument(argument,arg1);
	one_argument(argument,arg2);

	if (IS_NPC(ch)){
		ch->send("Awe! That's so adorable! Can you roll over? Can you? Yes you CAN!\n\r");
		return;
	}

	arg = arg1;
	metal = arg2;

	if ((forge = ch->pcdata->forge) && s_prefix(arg,"stop")){
		end_forging(ch,forge);
		return;
	}

	for (forge = ch->in_room->contents;forge;forge = forge->next_content)
		if (forge->item_type == ITEM_FORGE && forge->value[0] != HEAT_COLD && forge->value[0] != HEAT_NORMAL && !IS_SET(forge->extra_flags,ITM_FORGING))
			break;

	if (!forge){
		ch->send("You must be in a room with a high heat forge not in use.\n\r");
		return;
	}

	if (s_prefix(arg,"join") && !metal.empty())
		smelt_join(ch,forge,material_lookup(arg2),metal);
	else if (s_prefix(arg,"compound"))
		smelt_compound(ch,arg);
	else
		ch->send("Who when what?\n\r");
	return;
}*/
