#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

/*
 * Local functions.
 */
bool	IS_NATURE_BG	( CHAR_DATA* );
void heal_them		( CHAR_DATA*,CHAR_DATA*,int,int );

void spell_venom_dart(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	AFFECT_DATA *af;
	int dam = level + (level/5 * slvl);

	if (get_skill(victim,gsn_resilience))
		level /= 2;

	if (saves_spell(level /2 + 10 * slvl,victim,DAM_POISON)){
		act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
		victim->send("You feel momentarily ill, but it passes.\n\r");
		dam /= 2;
	}
	else{
		affect_join(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_STR,-2,AF_POISON);
		send_to_char("You feel very sick.\n\r",victim);
		act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
	}
	damage(ch,victim,dam+10,sn,DAM_POISON,true);
}
void spell_mud_pack(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	AFFECT_DATA *af = new_affect();
	int dam = level * slvl;

	damage(ch,victim,dam,sn,DAM_EARTH,true);

	if(saves_spell(level + 10 * slvl,victim,DAM_POISON))
		return;

	damage(ch,victim,dam/2,sn,DAM_EARTH,true);
	affect_join(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_AC,10*slvl,0);
	send_to_char("You are caked in mud.\n\r",victim);
	act("$n is caked in mud.",victim,NULL,NULL,TO_ROOM);
}
void spell_natural_curse(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	AFFECT_DATA *af = new_affect();
	int dam = level;

	if(!IS_NATURE_BG(ch)){
		ch->send("It's not very effective here!\n\r");
		return;
	}

	if(saves_spell(level + 10 * slvl,victim,DAM_POISON))
		return;

	damage(ch,victim,dam,sn,DAM_EARTH,true);
	affect_set(victim,TO_VULN,sn,level,slvl,level,APPLY_AC,10*slvl,RS_EARTH);
	send_to_char("You feel nature's wrath!\n\r",victim);
	act("$n is struck by an earthen curse!",victim,NULL,NULL,TO_ROOM);
}
void do_tranquility(CHAR_DATA *ch,char *argument){
	AFFECT_DATA *af = new_affect();
	int sn=gsn_tranquility,skill=get_skill(ch,sn);

	if(skill < 1){
		ch->send("You tilt your head and squint your eyes, but you just don't get nature.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You are not in a very tranquil place.\n\r");
		return;
	}
	if(is_affected(ch,sn)){
		ch->send("You are already tranquil.\n\r");
		return;
	}
	affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(sn),5,APPLY_WIS,5,AF_SILENCE);
	send_to_char("You become at peace with nature.\n\r",ch);
	act("$n seems much more tranquil.",ch,NULL,NULL,TO_ROOM);
}
bool check_resist_blights(CHAR_DATA *ch){
	int sn = gsn_resist_blights,skill=get_skill(ch,sn);

	if(skill<1)
		return false;

	skill /= 5;
	skill += skill/2 * ch->getslvl(sn);
	if(number_percent() < skill){
		check_improve(ch,sn,true,1);
		return true;
	}
	check_improve(ch,sn,false,1);
	return false;
}
void do_salve(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	bool found=false;
	int sn=gsn_salve,skill=get_skill(ch,sn);

	if(argument[0]){
		if(!(victim = get_char_room(ch,NULL,argument))){
			ch->send("Who?\n\r");
			return;
		}
	}
	else
		victim = ch;
	if(skill<1){
		ch->send("You are not skilled enough to do this.\n\r");
		return;
	}
	for(obj = ch->carrying;obj && !found; obj = obj->next_content ){
		if(obj->item_type == ITEM_HERB){
			found = true;
			break;
		}
	}
	if(!found){
		ch->send("You must have an herb in your inventory.\n\r");
		return;
	}

	found = false;
	if(number_percent() < skill/2 && (victim->isaff(AF_POISON) || is_affected(victim,gsn_poison))){
		victim->send("You feel much better!");
		act("$n seems much better!",victim,NULL,NULL,TO_ROOM);
		affect_strip(victim,gsn_poison);
		found = true;
	}
	if(number_percent() < skill/2 && (victim->isaff(AF_PLAGUE) || is_affected(victim,gsn_plague))){
		victim->send("Your sores heal and vanish!");
		act("$n's sores heal and vanish!",victim,NULL,NULL,TO_ROOM);
		affect_strip(victim,gsn_plague);
		found = true;
	}
	if(!found)
		ch->send("Nothing happened.\n\r");
	else{
		victim->send("You feel much better!\n\r");
		heal_them(ch,victim,sn,ch->getslvl(sn)*25);
		check_improve(ch,sn,true,1);
	}
}
void do_earth_wisdom(CHAR_DATA *ch){
}
bool check_animal_magnetism(CHAR_DATA *ch,CHAR_DATA *aggro){
	int sn=gsn_animal_magnetism,skill=get_skill(ch,sn);
	if(skill<1)
		return false;
	if(aggro->race != race_lookup("canine")
	&& aggro->race != race_lookup("pig")
	&& aggro->race != race_lookup("horse")
	&& aggro->race != race_lookup("bear")
	&& aggro->race != race_lookup("rabbit")
	&& aggro->race != race_lookup("serpent")
	&& aggro->race != race_lookup("feline")
	&& aggro->race != race_lookup("lizard")
	&& aggro->race != race_lookup("rodent")
	&& aggro->race != race_lookup("bird"))
		return false;
	if(number_percent() < (skill/4)+(ch->getslvl(sn)*5)){
		check_improve(ch,sn,true,1);
		return true;
	}
	return false;
}
