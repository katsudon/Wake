#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"

/* race table */
const struct race_type race_table [] =
{
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts
    },
*/

    {
/*RaceName*/"lame",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"none"},
/*Frm-Bits*/"none",
/*Prt-Bits*/"none"
    },

    {
/*RaceName*/"human",true, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"none"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye"
    },

    {
/*RaceName*/"algid",true, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"cold 25", "fire 175", "wind 75"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye"
    },

    {
/*RaceName*/"quolin",true, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"water 80","fire 90", "electricity 120","cold 110","weapon 105"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye"
    },

    {
/*RaceName*/"daurin",true, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"magic 75","earth 75", "wind 65", "water 140"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye"
    },

    {
/*RaceName*/"dwarf",true, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"magic 80", "mental 80", "weapon 110", "disease 110", "poison 110"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye"
    },

    {
/*RaceName*/"ogre",true, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"mental 125", "magic 85", "cold 90", "disease 80"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye fangs"
    },

    {
/*RaceName*/"vithe",true, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"pierce 80", "slash 80", "bash 130", "magic 110", "mental 120", "poison 80"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye horns tusks claws"
    },

    {
/*RaceName*/"faerie",false,
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"mental 125", "magic 85", "cold 90", "disease 80"},
/*Frm-Bits*/"edible animal sentient biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet fingers ear eye fangs"
    },

    {
/*RaceName*/"other",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"energy 60", "bash 80", "crystal 130", "cold 120", "mental 90"},
/*Frm-Bits*/"none",
/*Prt-Bits*/"none"
    },

    {
/*RaceName*/"bear",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"crush disarm berserk",
/*Res-Bits*/{"fire 120", "cold 80"},
/*Frm-Bits*/"edible animal mammal",
/*Prt-Bits*/"head arms legs heart brains guts hands feet claws ear eye"
    },

    {
/*RaceName*/"feline",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{},
/*Frm-Bits*/"edible animalbiped mammal",
/*Prt-Bits*/"head legs heart brains guts hands feet claws ear eye"
    },

    {
/*RaceName*/"canine",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"fire 110", "cold 90"},
/*Frm-Bits*/"edible animal biped mammal",
/*Prt-Bits*/"head legs heart brains guts hands feet claws ear eye"
    },

    {
/*RaceName*/"dragon",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"pierce 120", "summon 50", "fire 75", "cold 75", "electricity 75", "wind 75", "water 75", "poison 50", "disease 50"},
/*Frm-Bits*/"edible animal biped mammal",
/*Prt-Bits*/"head arms legs heart guts hands feet claws ear eye"
    },

    {
/*RaceName*/"fish",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"water 25", "electricity 120", "bash 90", "pierce 110"},
/*Frm-Bits*/"edible fish cold_blooded",
/*Prt-Bits*/"head heart brains guts eye fins scales"
    },

    {
/*RaceName*/"crustacean",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"water 60", "electricity 110", "bash 80", "pierce 125"},
/*Frm-Bits*/"crustacean edible cold_blood",
/*Prt-Bits*/"brains eyestalks arms guts claws legs eye"
    },

    {
/*RaceName*/"shellfish",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"water 25", "electricity 120", "bash 50", "pierce 120"},
/*Frm-Bits*/"edible crustacean fish cold_blooded",
/*Prt-Bits*/"guts"
    },

    {
/*RaceName*/"lizard",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"fire 80", "cold 125"},
/*Frm-Bits*/"edible animal reptile cold_blood",
/*Prt-Bits*/"head legs heart brains guts feet eye tail fangs"
    },

    {
/*RaceName*/"pig",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"none"},
/*Frm-Bits*/"edible animal mammal",
/*Prt-Bits*/"head legs heart brains guts hands feet ear eye"
    },

    {
/*RaceName*/"rabbit",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"cold 90"},
/*Frm-Bits*/"edible animal mammal",
/*Prt-Bits*/"head legs heart guts hands feet claws ear eye"
    },

    {
/*RaceName*/"rodent",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"poison 75", "disease 75", "fire 120"},
/*Frm-Bits*/"edible animal biped mammal poison",
/*Prt-Bits*/"head legs heart brains guts feet claws tail ear eye"
    },

    {
/*RaceName*/"horse",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"none"},
/*Frm-Bits*/"edible animal mammal",
/*Prt-Bits*/"head legs heart brains guts hands feet ear eye tail"
    },

    {
/*RaceName*/"serpent",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"cold 150"},
/*Frm-Bits*/"edible animal mammal",
/*Prt-Bits*/"head brains guts eye long_tongue scales fangs tail"
    },

    {
/*RaceName*/"undead",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"weapon 80", "cold 75", "negative 0", "poison 0", "disease 0", "light 200", "fire 150", "holy 300", "silver 150", "wood 130"},
/*Frm-Bits*/"instant_decay silent undead cold_blood",
/*Prt-Bits*/"none"
    },

    {
/*RaceName*/"spirit",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"weapon 40", "magic 150", "crystal 110", "summon 25", "charm 50", "poison 0", "disease 0"},
/*Frm-Bits*/"instant_decay mist intangible cold_blood",
/*Prt-Bits*/"none"
    },

    {
/*RaceName*/"shadow",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"weapon 75", "magic 125", "light 200"},
/*Frm-Bits*/"instant_decay mist intangible cold_blood",
/*Prt-Bits*/"none"
    },

    {
/*RaceName*/"bird",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"wind 50", "cold 125"},
/*Frm-Bits*/"edible animal mammal",
/*Prt-Bits*/"head legs heart brains guts eye wings tail claws"
    },

    {
/*RaceName*/"insect",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"cold 130", "poison 75", "disease 75", "pierce 75"},
/*Frm-Bits*/"edible insect cold_blood",
/*Prt-Bits*/"head legs guts feet eye claws fangs"
    },

    {
/*RaceName*/"doraga",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"none"},
/*Frm-Bits*/"none",
/*Prt-Bits*/"none"
    },

    {
/*RaceName*/"demon",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"negative -25", "holy 250", "light 200"},
/*Frm-Bits*/"none",
/*Prt-Bits*/"none"
    },

    {
/*RaceName*/"unique",false, 
/*Affected*/"none",
/*Act-Bits*/"none",
/*Off-Bits*/"none",
/*Res-Bits*/{"none"},
/*Frm-Bits*/"none",
/*Prt-Bits*/"none"
    },

    {
	NULL, 0, "none", "none", "none", "none"
    }
};

struct pc_race_type pc_race_table []={
    { "null race", "", 1, {0},{ "" },{ "" }, {0}, {0}, {0}, {0}, 0 },

    {	"human",
		" Human ",
		1,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "bravery", "durability" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 13, 13, 12, 14, 16, 14, 14, 15, 16},
/*MaxTrainStats*/{ 23, 23, 23, 23, 22, 24, 22, 23, 24},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The size flag*/SIZE_MEDIUM
    },

    {	"algid",
		" Algid ",
		5,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "frozen blood", "icebound" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 17, 15, 15, 17, 14, 17, 16, 16, 13},
/*MaxTrainStats*/{ 25, 22, 25, 24, 22, 24, 23, 20, 22},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The size flag*/SIZE_MEDIUM
    },

    {
		"quolin",
		" Quolin ",
		3,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "hydrobiology", "thalassic aura" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 14, 19, 15, 15, 12, 15, 14, 12, 14},
/*MaxTrainStats*/{ 23, 23, 24, 25, 20, 25, 25, 22, 20},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The sizeflags*/SIZE_MEDIUM
    },

    {	"daurin",
		" Daurin ",
		3,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "disappear", "resilience", "daurin" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 16, 15, 14, 18, 18, 17, 19, 19, 18},
/*MaxTrainStats*/{ 24, 25, 24, 21, 25, 21, 23, 22, 22},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The sizeflags*/SIZE_MEDIUM
    },

    {	"dwarf",
		" Dwarf ",
		4,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "amiability", "drunken fighting" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 19, 19, 18, 13, 18, 13, 16, 16, 18},
/*MaxTrainStats*/{ 23, 25, 20, 24, 23, 24, 23, 25, 20},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The sizeflags*/SIZE_SMALL
    },

    {	"ogre",
		"  Ogre  ",
		4,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "fervor", "warcry" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 19, 16, 14, 12, 19, 13, 13, 13, 14},
/*MaxTrainStats*/{ 25, 25, 21, 23, 24, 24, 23, 21, 21},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The sizeflags*/SIZE_LARGE
    },

    {
		"vithe",
		" Vithe ",
		3,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "regeneration", "return" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 19, 15, 15, 17, 14, 17, 16, 16, 13},
/*MaxTrainStats*/{ 25, 23, 23, 21, 22, 25, 25, 20, 23},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The sizeflags*/SIZE_LARGE
    },

    {
		"faerie",
		" Faerie ",
		4,
/*Class Moddies*/{ 100, 100, 100 },
/*PrimarySkills*/{ "none" },
/*Second Skills*/{ "none" },
/*Starter Stats*/{ 19, 16, 14, 12, 19, 13, 13, 13, 14},
/*MaxTrainStats*/{ 21, 20, 25, 24, 24, 23, 22, 23, 25},
/*ClassAvailabl*/{ },
/*Home pickings*/{ },
/*The sizeflags*/SIZE_TINY
    },

	{ NULL, "", 0, { 100, 100, 100, 100 },{ "" },{ "" }, {0}, {0}, {0},{ 0 }, 0 }

};

void press_res(CHAR_DATA *ch){
	char arg[100],*buf;
	int i,n,val;
	bool found;

	for (i = 0;i < MAX_RES;i++)
		ch->res[i] = 100;

	for (i = 0;i<MAX_RES;i++){
		if (race_table[ch->race].res[i]){
			buf = str_dup(race_table[ch->race].res[i]);
			buf = one_argument(buf,arg);
			if(!strcmp(arg,"none"))
				break;
			if(!is_number(buf)){
				wiznet("bad press_res1, check logfiles now",NULL,NULL,WZ_FLAGS,0,0);
				log_f("press_res: icky value  %s '%s'",race_table[ch->race].name,race_table[ch->race].res[i]);
				return;
			}
			val = atoi(buf);
			found = false;
			for (n = 0;n<MAX_RES;n++){
				if(!strcmp(res_flags[n].name,arg)){
					ch->res[n] = val;
					found = true;
				}
			}
			if (!found){
				wiznet("bad press_res2, check logfiles now",NULL,NULL,WZ_FLAGS,0,0);
				log_f("press_res: icky res  %s '%s'",race_table[ch->race].name,race_table[ch->race].res[i]);
				return;
			}
		}
		else
			break;
	}
}

void init_race(CHAR_DATA *ch){
	ch->massaff(race_table[ch->race].aff);
	ch->massact(race_table[ch->race].act);
	ch->massform(race_table[ch->race].form);
	ch->masspart(race_table[ch->race].part);
	press_res(ch);
}

bool char_data :: israce(char *arg){
	if(race == race_lookup(arg) && race_lookup(arg) != 0)
		return true;
	return false;
}
