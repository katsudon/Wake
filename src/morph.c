#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"
#include "magic.h"
#include "recycle.h"

/*
* Local Functions
*/
void follow_char(CHAR_DATA *ch,int door,ROOM_INDEX_DATA *old_in_room);
int find_path(ROOM_INDEX_DATA *from, ROOM_INDEX_DATA *to, int max_depth);
bool mask_trek(CHAR_DATA *ch);

const struct morph_type morph_table[MAX_MORPH * 5] ={
	{"serpent","anaconda","an anaconda","anaconda slithers around."},
	{"serpent","python","a python","python slithers around."},
	{"serpent","cobra","a cobra","cobra slithers around."},
	{"serpent","adder","an adder","adder slithers around."},
	{"serpent","asp","an asp","asp slithers around."},

	{"canine","dog","a dog","dog stalks around."},
	{"canine","fox","a fox","fox stalks around."},
	{"canine","coyote","a coyote","coyote stalks around."},
	{"canine","wolf","a wolf","wolf stalks around."},
	{"canine","timber wolf","a timber wolf","timber wolf stalks around."},

	{"feline","lynx","a lynx","lynx prowls around."},
	{"feline","cheetah","a cheetah","cheetah prowls around."},
	{"feline","jaguar","a jaguar","jaguar prowls around."},
	{"feline","tiger","a tiger","tiger prowls around."},
	{"feline","lion","a lion","lion prowls around."},

	{"bear","brown bear","a brown bear","brown bear lumbers around."},
	{"bear","black bear","a black bear","black bear lumbers around."},
	{"bear","grizzly bear","a grizzly bear","grizzly bear lumbers around."},
	{"bear","polar bear","a polar bear","polar bear lumbers around."},
	{"bear","kodiak bear","a kodiak bear","kodiak bear lumbers around."},

	{"bird","sparrow","a sparrow","sparrow flies around."},
	{"bird","falcon","a falcon","falcon flies around."},
	{"bird","hawk","a hawk","hawk flies around."},
	{"bird","condor","a condor","condor flies around."},
	{"bird","eagle","an eagle","eagle flies around."},

	{"fish","salmon","a salmon","salmon swims about."},
	{"fish","pike","a pike","pike swims about."},
	{"fish","dolphin","a dolphin","dolphin swims about."},
	{"fish","swordfish","a swordfish","swordfish swims about."},
	{"fish","shark","a shark","shark swims about."}
};

bool validate_morph(CHAR_DATA *ch,int sn){
	char message[MIL];

	message[0] = '\0';

	if(IS_NPC(ch))
		sprintf(message,"You are too stupid to do this.\n\r");
	if(ch->iscomm(CM_MORPH))
		sprintf(message,"You are already morphed.\n\r");
	for(int iWear = 0;iWear < MAX_WEAR; iWear++){
		if((get_eq_char(ch,iWear)) && (get_eq_char(ch,iWear)->wear_loc != WEAR_TATTOO) && (get_eq_char(ch,iWear)->wear_loc != WEAR_FLOAT_LIGHT)){
			ch->send("You must be naked to do this.\n\r");
			return false;
		}
	}

	if(message[0]){
		ch->send(message);
		return false;
	}
	WAIT_STATE(ch,skill_table[sn].beats);
	if(!roll_chance(ch,get_skill(ch,sn))){
		ch->send("You cluck like a chicken.. but nothing happens!\n\r");
		check_improve(ch,sn,false,1);
		return false;
	}
	check_improve(ch,sn,true,1);
	return true;
}

//Morphing Shape Shifting whatever
bool morph_beast(CHAR_DATA *ch,int beast,int sn){
	char buf[MIL];
	int num = 0,junk = 0;

	if(!validate_morph(ch,sn))
		return false;

	ch->true_race = ch->race;
	ch->morph = beast;

	ch->setcomm(CM_MORPH);

	for(int i = 0;i < MAX_FRM;i++)
		ch->remform(i);
	for(int i = 0;i < MAX_PRT;i++)
		ch->rempart(i);

	junk = (beast-1) * 5;
	ch->race = race_lookup(morph_table[junk].race);
	ch->massform(race_table[ch->race].form);
	ch->masspart(race_table[ch->race].part);

	if(ch->level >= 90)			num = 4;
	else if(ch->level >= 80)	num = 3;
	else if(ch->level >= 70)	num = 2;
	else if(ch->level >= 60)	num = 1;
	else						num = 0;

	junk += num;
	switch(beast){
		case MORPH_SERPENT:
			act("Your body stretches and your arms shrivel and vanish as you morph into $T.",ch,NULL,morph_table[junk].short_descr,TO_CHAR);
			act("$n's body stretches and $s arms shrivel and vanish as $e morphs into $T.",ch,NULL,morph_table[junk].short_descr,TO_ROOM);
			break;
		case MORPH_CANINE:
			act("You get down on all fours and your body is covered in thick fur as you morph into $T.",ch,NULL,morph_table[junk].short_descr,TO_CHAR);
			act("$n gets on all fours and thick fur covers $s body as $e morphs into $T.",ch,NULL,morph_table[junk].short_descr,TO_ROOM);
			break;
		case MORPH_FELINE:
			act("Your body is covered in fur and your ears move to the top of your head as you morph into $T.",ch,NULL,morph_table[junk].short_descr,TO_CHAR);
			act("$n is covered in fur and $s ears become pointy and move to the top of $s head as $e morphs into $T.",ch,NULL,morph_table[junk].short_descr,TO_ROOM);
			break;
		case MORPH_URSA:
			act("Your body grows large and fur sprouts over your body as you morph into $T.",ch,NULL,morph_table[junk].short_descr,TO_CHAR);
			act("$n doubles in size and is covered in fur as $e morphs into $T.",ch,NULL,morph_table[junk].short_descr,TO_ROOM);
			break;
		case MORPH_AVIAN:
			act("Your arms become wings and your mouth becomes a beak as you morph into $T.",ch,NULL,morph_table[junk].short_descr,TO_CHAR);
			act("$n's arms become wings and $s mouth becomes a beak as $e morphs into $T.",ch,NULL,morph_table[junk].short_descr,TO_ROOM);
			break;
		case MORPH_AQUATIC:
			act("You grow large gills and fins, and your flesh turns to scales as you morph into $T.",ch,NULL,morph_table[junk].short_descr,TO_CHAR);
			act("$n grows large gills and fins, and $s flesh turns to scales as $e morphs into $T.",ch,NULL,morph_table[junk].short_descr,TO_ROOM);
			break;
	}

	free_string(ch->short_descr);
	sprintf(buf,"%s, the %s %s",ch->name,morph_table[junk].name,morph_table[junk].name);
	ch->short_descr = str_dup(buf);

	free_string(ch->long_descr);
	sprintf(buf,"%s, the %s %s\n\r",ch->name,morph_table[junk].name,morph_table[junk].long_descr);
	ch->long_descr = str_dup(buf);

	return true;
}

void unmorph_beast(CHAR_DATA *ch){
	if(!ch->iscomm(CM_MORPH)){
		ch->send("You are not morphed.\n\r");
		return;
	}
	ch->remcomm(CM_MORPH);
	ch->race = ch->true_race;
	ch->morph = 0;
	for(int i = 0;i < MAX_FRM;i++)
		ch->remform(i);
	for(int i = 0;i < MAX_PRT;i++)
		ch->rempart(i);
	ch->massform(race_table[ch->race].form);
	ch->masspart(race_table[ch->race].part);
	ch->short_descr = str_dup("");
	ch->long_descr = str_dup("");

	affect_strip(ch,gsn_morph_serpent);
	affect_strip(ch,gsn_morph_canine);
	affect_strip(ch,gsn_morph_feline);
	affect_strip(ch,gsn_morph_ursa);
	affect_strip(ch,gsn_morph_avian);
	affect_strip(ch,gsn_morph_aquatic);

	act("You return to your original form.",ch,NULL,NULL,TO_CHAR);
	act("$n returns to $s original form.",ch,NULL,NULL,TO_ROOM);
}
void do_unmorph(CHAR_DATA *ch,char *argument){
	unmorph_beast(ch);
}
void morph_serpent(CHAR_DATA *ch){
	AFFECT_DATA af;
	int sn = gsn_morph_serpent;
	if(!validate_morph(ch,sn))
		return;

	if(morph_beast(ch,MORPH_SERPENT,gsn_morph_serpent)){
		affect_join(ch,TO_AFFECTS,gsn_morph_serpent,ch->level,ch->getslvl(sn),ch->getslvl(sn) * 10,APPLY_AGI, get_curr_stat(ch,STAT_AGI) / 10 * ch->getslvl(sn),0);
		//af.location  = APPLY_AGI;
		//af.modifier  = -3 + (ch->getslvl(sn) * 2);
		//affect_join(ch,&af);
	}
}
void morph_canine(CHAR_DATA *ch){
	AFFECT_DATA af;
	int sn = gsn_morph_canine;
	if(!validate_morph(ch,sn))
		return;
	if(morph_beast(ch,MORPH_CANINE,gsn_morph_canine))
		affect_join(ch,TO_AFFECTS,gsn_morph_canine,ch->level,ch->getslvl(sn),ch->getslvl(sn)*10,APPLY_HIT,(double)ch->max_hit * ((5 + ch->getslvl(sn)) / 10),0);
}
void morph_feline(CHAR_DATA *ch){
	AFFECT_DATA af;
	int sn = gsn_morph_feline;
	if(!validate_morph(ch,sn))
		return;
	if(morph_beast(ch,MORPH_FELINE,gsn_morph_feline))
		affect_join(ch,TO_AFFECTS,gsn_morph_feline,ch->level,ch->getslvl(sn),ch->getslvl(sn) *10,APPLY_HIT,(double)ch->max_hit * (2+ch->getslvl(sn)) / 10,AF_DARK_VISION);
}
void morph_ursa(CHAR_DATA *ch){
	AFFECT_DATA af;
	int sn = gsn_morph_ursa;
	if(!validate_morph(ch,sn))
		return;
	if(morph_beast(ch,MORPH_URSA,gsn_morph_ursa))
		affect_join(ch,TO_AFFECTS,gsn_morph_ursa,ch->level,ch->getslvl(sn),ch->getslvl(sn)*10,APPLY_HIT,(double)ch->max_hit * ((10 + 3*ch->getslvl(sn))/10),0);
}
void morph_avian(CHAR_DATA *ch){
	AFFECT_DATA af;
	int sn = gsn_morph_avian;
	if(!validate_morph(ch,sn))
		return;
	if(morph_beast(ch,MORPH_AVIAN,gsn_morph_avian))
		affect_join(ch,TO_AFFECTS,gsn_morph_avian,ch->level,ch->getslvl(sn),ch->getslvl(sn) * 10,APPLY_HIT,-(double)ch->max_hit + ch->max_hit * (0.8 + (ch->getslvl(sn) / 10)),AF_FLYING);
}
void morph_aquatic(CHAR_DATA *ch){
	AFFECT_DATA af;
	int sn = gsn_morph_aquatic;
	ch->send("Sorry not yet.\n\r");
	return;

	if(!validate_morph(ch,sn))
		return;
	if(morph_beast(ch,MORPH_AQUATIC,gsn_morph_aquatic))
		affect_join(ch,TO_AFFECTS,gsn_morph_aquatic,ch->level,ch->getslvl(sn),ch->getslvl(sn) * 10,APPLY_HIT,-(double)ch->max_hit + (ch->max_hit * (0.9 + (ch->getslvl(sn) / 10))),AF_FLYING);
}

void do_morph(CHAR_DATA *ch,char *argument){
	int skill;
	if(argument[0] && ch->level > 105){
		if(!str_prefix(argument,"serpent") && (skill = get_skill(ch,gsn_morph_serpent)))
			morph_serpent(ch);
		else if(!str_prefix(argument,"ursa") && (skill = get_skill(ch,gsn_morph_ursa)))
			morph_ursa(ch);
		else if(!str_prefix(argument,"feline") && (skill = get_skill(ch,gsn_morph_feline)))
			morph_feline(ch);
		else if(!str_prefix(argument,"canine") && (skill = get_skill(ch,gsn_morph_canine)))
			morph_canine(ch);
		else if(!str_prefix(argument,"avian") && (skill = get_skill(ch,gsn_morph_avian)))
			morph_avian(ch);
		else if(!str_prefix(argument,"aquatic") && (skill = get_skill(ch,gsn_morph_aquatic)))
			morph_aquatic(ch);
		else{
			ch->send("You don't know how to morph into that.\n\r");
			return;
		}
		return;
	}
	if((skill = get_skill(ch,gsn_morph_serpent)))
		morph_serpent(ch);
	else if((skill = get_skill(ch,gsn_morph_ursa)))
		morph_ursa(ch);
	else if((skill = get_skill(ch,gsn_morph_feline)))
		morph_feline(ch);
	else if((skill = get_skill(ch,gsn_morph_canine)))
		morph_canine(ch);
	else if((skill = get_skill(ch,gsn_morph_avian)))
		morph_avian(ch);
	else if((skill = get_skill(ch,gsn_morph_aquatic)))
		morph_aquatic(ch);
	else{
		ch->send("You don't know how to morph.\n\r");
		return;
	}
}

void do_roar(CHAR_DATA *ch,char *argument){
	CHAR_DATA *pet = ch->pet,*vch;
	AFFECT_DATA af;
	int sn = gsn_primal_roar,skill = get_skill(ch,sn);

	if(skill < 1 || !ch->iscomm(CM_MORPH)){
		ch->send("You are not primal enough. Sorry.\n\r");
		return;
	}
	if(roll_chance(ch,skill)){
		affect_set(ch,TO_AFFECTS,sn,ch->level,1,ch->getslvl(sn),APPLY_HITROLL,ch->getslvl(sn) * 5,0);
		if(pet){
			affect_set(pet,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_HITROLL,ch->getslvl(sn) * 5,0);
		}
		for(vch = ch->in_room->people;vch;vch = vch->next_in_room){
			affect_set(vch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_HITROLL,-ch->getslvl(sn) * 5,0);
			affect_set(vch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_DAMROLL,-ch->getslvl(sn) * 5,0);
			affect_set(vch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_AC,-ch->getslvl(sn) * 50,0);
		}
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("You meow... mrow?\n\r");
		check_improve(ch,sn,false,1);
	}
}
void do_rabid(CHAR_DATA *ch,char *argument){//nash dun like this, make it give aspd
	AFFECT_DATA af;
	int sn = gsn_rabid,skill = get_skill(ch,sn);

	if(skill < 1 || !ch->iscomm(CM_MORPH)){
		ch->send("Go find a raccoon or something, that might work..\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		ch->send("You start to foam at the mouth and snarl loudly!\n\r");
		act("$n foams at the mouth and snarls with a wild look in $s eye!",ch,NULL,NULL,TO_ROOM);

		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_AC,ch->getslvl(sn) * 500,AF_BDAMTAKE);

		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_AC,ch->getslvl(sn) * 500,AF_BDAMGIVE);

		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_AC,ch->getslvl(sn) * 500,AF_HASTE);
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("You try and use shaving lather but it doesn't work.\n\r");
		return;
	}
}
bool check_mark_of_the_beast(CHAR_DATA *ch){
	int sn=gsn_mark_of_the_beast,skill=get_skill(ch,sn);

}
//Morph skills
void do_lurk(CHAR_DATA *ch,char *argument){
	AFFECT_DATA *af = new_affect();
	int sn=gsn_lurk,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_SERPENT){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(roll_chance(ch,skill)){
		ch->send("You start to prowl, using your surroundings for cover.\n\r");

		affect_set(ch,TO_AFFECTS,gsn_hide,ch->level,ch->getslvl(sn),ch->level / 2,APPLY_AC,-200,AF_HIDE);
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("You fail.\n\r");
		check_improve(ch,sn,false,1);
		WAIT_STATE(ch,skill_table[sn].beats);
	}
}
void do_slither(CHAR_DATA *ch,char *argument){
	ROOM_INDEX_DATA *room = ch->in_room;
	int door,sn=gsn_slither,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_SERPENT){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(ch->move < 30){
		ch->send("You don't have the energy to slither anymore.\n\r");
		return;
	}
	if(!str_prefix(argument,"north")) door = 0;
	else if(!str_prefix(argument,"east")) door = 1;
	else if(!str_prefix(argument,"south")) door = 2;
	else if(!str_prefix(argument,"west")) door = 3;
	else if(!str_prefix(argument,"up")) door = 4;
	else if(!str_prefix(argument,"down")) door = 5;
	else{
		ch->send("That's not a valid direction.\n\r");
		return;
	}
	if(!roll_chance(ch,skill) && number_percent() < 50 - ch->getslvl(sn) * 5){
		ch->send("You fail to slither.\n\r");
		check_improve(ch,sn,false,1);
		return;
	}
	if(room->exit[door]
	&& !IS_SET(room->exit[door]->exit_info,EX_CLIFFTOP)
	&& !IS_SET(room->exit[door]->exit_info,EX_HIDDEN)
	&& !IS_SET(room->exit[door]->exit_info,EX_SDOOR)
	&& !IS_SET(room->exit[door]->exit_info,EX_QUIET)){
		act("You slither $T.",ch,NULL,dir_name[door],TO_CHAR);
		ch->move -= 30;
		char_from_room(ch);
		char_to_room(ch,room->exit[door]->u1.to_room);
		follow_char(ch,door,room);
		check_improve(ch,sn,true,1);
	}
}

void do_maul(CHAR_DATA *ch,char *argument){
	AFFECT_DATA *af = new_affect();
	CHAR_DATA *victim;
	int sn=gsn_maul,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_CANINE){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(!(victim = grab_char(ch,argument,true)))
		return;
	WAIT_STATE(ch,skill_table[sn].beats);
	if(roll_chance(ch,skill)){
		damage(ch,victim,ch->getslvl(sn) * 40,sn,DAM_PIERCE,true);
		if(!victim->isaff(AF_RUPTURE) && number_percent() > 75 - 5*ch->getslvl(sn)){
			affect_set(victim,TO_AFFECTS,gsn_maul,ch->level,ch->getslvl(sn),ch->getslvl(sn) * 2,APPLY_STR,-ch->getslvl(sn),AF_RUPTURE);
		}
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("You snap out and miss!\n\r");
		act("$n snaps out and misses!",ch,NULL,NULL,TO_ROOM);
		damage(ch,victim,0,sn,DAM_PIERCE,true);
		check_improve(ch,sn,false,1);
	}
}
void do_trail(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int i,sn = gsn_trail,skill = get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_CANINE){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(!(victim = get_char_world(ch,argument))){
		ch->send("No one around by that name.\n\r");
		return;
	}
	if(ch->in_room == victim->in_room){
		act("$N is here!",ch,NULL,victim,TO_CHAR);
		return;
	}

	act("$n carefully sniffs the air.",ch,NULL,NULL,TO_ROOM);
	WAIT_STATE(ch,skill_table[sn].beats);

	skill /= 4;
	skill += ch->getslvl(sn)*5;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d\n\r",skill);

	if(roll_chance(ch,skill)){
		if(get_skill(ch,sn) > 1)
			skill += ch->getslvl(sn) * 2;
		i = find_path(ch->in_room,victim->in_room,skill);

		if(mask_trek(victim))
			i = number_range(0,5);
		if (i == -1)
			printf_to_char(ch,"%s is too far away for you to track.\n\r",PERS(victim,ch));
		else
			printf_to_char(ch,"%s is %s from here.\n\r",PERS(victim,ch),dir_name[i]);
		check_improve(ch,sn,true,1);
	}
	else{
		ch->send("POINT!!!\n\r");
		check_improve(ch,sn,false,1);
	}
}

void do_pounce(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int sn=gsn_pounce,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_FELINE){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(ch->fighting){
		ch->send("You are in combat!\n\r");
		return;
	}
	if(!(victim = grab_char(ch,argument,true)))
		return;
	WAIT_STATE(ch,skill_table[sn].beats);
	if(roll_chance(ch,skill)){
		act("You pounce on $N, claws drawn!",ch,NULL,victim,TO_CHAR);
		act("$n pounces on you, claws drawn!",ch,NULL,victim,TO_VICT);
		act("$n pounces on $N, claws drawn!",ch,NULL,victim,TO_NOTVICT);
		damage(ch,victim,ch->getslvl(sn) * ch->level / 2 + 50,sn,DAM_SLASH,true);
		check_improve(ch,sn,true,1);
	}
	else{
		act("You pounce on $N, claws drawn, and hit the ground!",ch,NULL,victim,TO_CHAR);
		act("$n pounces on you, claws drawn, and hits the ground!",ch,NULL,victim,TO_VICT);
		act("$n pounces on $N, claws drawn, and hits the ground!",ch,NULL,victim,TO_NOTVICT);
		damage(ch,victim,0,sn,DAM_SLASH,true);
		check_improve(ch,sn,false,1);
	}
}
void do_scratch(CHAR_DATA *ch,char *argument){
	AFFECT_DATA *af = new_affect();
	CHAR_DATA *victim;
	int sn=gsn_scratch,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_FELINE){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(!(victim = grab_char(ch,argument,true)))
		return;
	WAIT_STATE(ch,skill_table[sn].beats);
	if(roll_chance(ch,skill)){
		act("You scratch at $N!",ch,NULL,victim,TO_CHAR);
		act("$n scratches at you!",ch,NULL,victim,TO_VICT);
		act("$n scratches at $N!",ch,NULL,victim,TO_NOTVICT);
		damage(ch,victim,ch->getslvl(sn) * 50 + 50,sn,DAM_SLASH,true);
		if(!victim->isaff(AF_POISON) && number_percent() > 75 - 5*ch->getslvl(sn))
			affect_set(victim,TO_AFFECTS,gsn_scratch,ch->level,ch->getslvl(sn),ch->level / 2,APPLY_END,-ch->getslvl(sn),AF_POISON);
		check_improve(ch,sn,true,1);
	}
	else{
		damage(ch,victim,0,sn,DAM_SLASH,true);
		check_improve(ch,sn,false,1);
	}
}

void do_swipe(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int sn=gsn_swipe,skill=get_skill(ch,sn) * .75;
	bool found = false;
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_URSA){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(!(victim = grab_char(ch,argument,true)))
		return;

	WAIT_STATE(ch,skill_table[sn].beats);
	act("You swipe out at $N!",ch,NULL,victim,TO_CHAR);
	act("$n swipes out at you!",ch,NULL,victim,TO_VICT);
	act("$n swipes out at $N!",ch,NULL,victim,TO_NOTVICT);
	if(roll_chance(ch,skill)){
		victim->bashwait = ch->getslvl(sn);
		damage(ch,victim,ch->getslvl(sn) * 10,sn,DAM_BASH,true);
	}
	if(roll_chance(ch,skill))
		damage(ch,victim,ch->getslvl(sn) * 50,gsn_scratch,DAM_SLASH,true);

	if(!found){
		ch->send("You missed!\n\r");
		damage(ch,victim,0,sn,DAM_BASH,true);
		check_improve(ch,sn,false,1);
	}
	else
		check_improve(ch,sn,true,1);
}
void do_craze(CHAR_DATA *ch,char *argument){
	AFFECT_DATA *af = new_affect();
	int sn = gsn_craze,skill = get_skill(ch,sn) * .5,hp_percent;
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_URSA){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}

	if (ch->isaff(AF_BERSERK) || is_affected(ch,gsn_berserk) || is_affected(ch,skill_lookup("frenzy"))){
		send_to_char("You get a little madder.\n\r",ch);
		return;
	}
	if (ch->isaff(AF_CALM)){
		send_to_char("You're feeling to mellow to go into a craze.\n\r",ch);
		return;
	}

	if (ch->position == POS_FIGHTING)
		skill += 10;

	hp_percent = 50 * ch->hit/ch->max_hit;
	skill += 25 - hp_percent/2;

	if (number_percent() < skill){
		WAIT_STATE(ch,PULSE_VIOLENCE);
		ch->move /= 2;

		ch->hit += ch->level * 2;
		ch->hit = UMIN(ch->hit,ch->max_hit);

		send_to_char("You go into a craze!\n\r",ch);
		act("$n gets a crazy look in $s eyes.",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,sn,true,2);

		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_HITROLL,ch->getslvl(sn),AF_BERSERK);

		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_DAMROLL,ch->getslvl(sn),0);

		
		affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),ch->getslvl(sn),APPLY_AC,100 - ch->getslvl(sn) * 10,0);
	}
	else{
		WAIT_STATE(ch,3 * PULSE_VIOLENCE);
		ch->move /= 2;

		send_to_char("You growl and snarl, but don't get crazier.\n\r",ch);
		check_improve(ch,sn,false,2);
	}
}

void do_aerial_strike(CHAR_DATA *ch,char *argument){
	int sn=gsn_slither,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_AVIAN){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
}
void do_ascend(CHAR_DATA *ch,char *argument){
	int sn=gsn_slither,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_AVIAN){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
}
void do_swarm(CHAR_DATA *ch,char *argument){
	int sn=gsn_slither,skill=get_skill(ch,sn);
	if(!ch->iscomm(CM_MORPH) || ch->morph != MORPH_AVIAN){
		ch->send("You are not the right species.\n\r");
		return;
	}
	if(skill < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
}
