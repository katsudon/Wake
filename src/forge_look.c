#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "interp.h"

/*
 * Local functions.
 */
/*
using namespace std;


int get_fuel_efficiency(OBJ_DATA *fuel){return fuel->weight * fuel->value[0] / 100;}
int get_forge_efficiency(OBJ_DATA *forge,int metal){
	return 10 * UMAX(forge->value[0] * metal,1) * forge->value[1] / 100;
}
int get_metal_sn(CHAR_DATA *ch,int mnum){
	if (mnum < 1)
		return -1;
	if (material_flags[mnum].name == "silver") return gsn_silver_knowledge;

	return -1;
}

bool is_forge_type(CHAR_DATA *ch,OBJ_DATA *forge,int type){
	if (forge->value[0] == type)
		return true;
	else
		ch->send("This forge is not the right type...\n\r");
	return false;
}

bool validate_metal_forge(CHAR_DATA *ch,OBJ_DATA *forge){
	OBJ_DATA *shit;
	bool first = false;
	for (shit = forge->contains;shit;shit = shit->next_content)
		if (s_prefix(shit->material,material_flags[forge->value[4]].name)){
			if (shit->item_type == ITEM_INGOT){
				if (!first)
					first = true;
				else
					return true;
			}
			else
				return true;
		}
	return false;
}

bool validate_fuel_forge(CHAR_DATA *ch,OBJ_DATA *forge){
	OBJ_DATA *fuel;
	int fuelweight = 0;

	for (fuel = forge->contains; fuel; fuel = fuel->next_content){
		if (fuel->item_type == ITEM_FORGEFUEL)
			fuelweight += get_fuel_efficiency(fuel);
	}

	if (fuelweight < get_forge_efficiency(forge,1)){
		ch->send("There is not enough fuel in the forge.\n\r");
		return false;
	}
	return true;
}

int validate_metal_knowledge(CHAR_DATA *ch,string material,int mnum){
	int sn = 0;

	if (mnum < 1){
		if (material == "silver") sn = gsn_silver_knowledge;
	}
	else{
		if (!str_cmp(material_flags[mnum].name,"silver")) sn = gsn_silver_knowledge;
	}

	if (sn > 0 && get_skill(ch,sn) > 0)
		return get_skill(ch,sn);

	return -1;
}

OBJ_DATA *find_ingot(OBJ_DATA *forge){
	OBJ_DATA *obj;
	if (forge->value[4] < 1)
		return NULL;

	for (obj = forge->contains;obj;obj = obj->next_content)
		if (!str_cmp(obj->material,material_flags[forge->value[4]].name) && obj->item_type == ITEM_INGOT)
			return obj;
	return NULL;
}*/
