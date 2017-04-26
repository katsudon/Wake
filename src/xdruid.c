#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "lookup.h"

/*
* Local Functions
*/
bool IS_NATURE_BG(CHAR_DATA*);
bool remove_obj(CHAR_DATA*,int,bool);
AFFECT_DATA	*new_affect		( void );
void		init_familiar	( CHAR_DATA*,CHAR_DATA* );


//Leaf spells
void spell_petal_storm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch;
	int dam,count = 10;

	act("You create a whirling storm of razor sharp petals!",ch,NULL,NULL,TO_CHAR);
	act("$n creates a whirling storm of razor sharp petals!",ch,NULL,NULL,TO_ROOM);

	while(count--){
		vch = get_random_char(ch,NULL,NULL);
		if (!canPK(ch,vch,false))
			count++;
		else{
			dam		= number_range(level/2,level);
			if (saves_spell(level,vch,DAM_PLANT))
				dam /= 2;
			spell_damage(ch,vch,dam,sn,DAM_PLANT,true);
		}
	}
}
void spell_leaf_barrage(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	int dam,count = ch->getslvl(sn) * 2;

	act("You release a barrage of razor sharp leaves at $N!",ch,NULL,victim,TO_CHAR);
	act("$n releases a barrage of razor sharp leaves at you!",ch,NULL,NULL,TO_VICT);
	act("$n releases a barrage of razor sharp leaves at $N!",ch,NULL,NULL,TO_NOTVICT);

	while(count--){
		dam = number_range(level,level*1.5);
		if (saves_spell(level,victim,DAM_PLANT))
			dam /= 2;
		spell_damage(ch,victim,dam,sn,DAM_PLANT,true);
	}
}

void spell_vine_lash(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	int dam;

	act("You lash at $N with two vine whips!",ch,NULL,victim,TO_CHAR);
	act("$n lashes at $N with two vine whips!",ch,NULL,victim,TO_NOTVICT);
	act("$n lashes at you with two vine whips!",ch,NULL,victim,TO_VICT);

	dam	= number_range(level,level * 2);
	if(!saves_spell(level,victim,DAM_PLANT))
		dam *= 2;
	spell_damage(ch,victim,dam,sn,DAM_PLANT,true);
	if(!saves_spell(level,victim,DAM_PLANT))
		dam *= 2;
	spell_damage(ch,victim,dam,sn,DAM_PLANT,true);
}

//Celestial
void spell_lucent_rays(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam;

	if(weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT){
		ch->send("You can't call on the moon's power in the day.\n\r");
		return;
	}
	act("You call down rays of dark from the moon!",ch,NULL,NULL,TO_CHAR);
	act("$n calls down rays of dark from the moon!",ch,NULL,NULL,TO_ROOM);

	for(int i = 0;i < 2;i++){
		for(vch = ch->in_room->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if(is_same_group(ch,vch) || !canPK(ch,vch,false))
				continue;
			dam = number_range(ch->level,ch->level * 1.5);
			if(saves_spell(level,vch,DAM_NEGATIVE))
				dam /= 2;
			spell_damage(ch,vch,dam,sn,DAM_NEGATIVE,true);
		}
	}
}

void spell_solar_rays(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam;

	if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK){
		ch->send("You can't call on the sun's power in the night.\n\r");
		return;
	}
	act("You call down rays of light from the sun!",ch,NULL,NULL,TO_CHAR);
	act("$n calls down rays of light from the sun!",ch,NULL,NULL,TO_ROOM);

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch) || !canPK(ch,vch,false))
			continue;
		dam = number_range(ch->level*2,ch->level * 3);
		if(saves_spell(level,vch,DAM_LIGHT))
			dam *= .75;
		spell_damage(ch,vch,dam,sn,DAM_LIGHT,true);
	}
}

void spell_celestial_strike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam;

	act("You call upon the power of the stars!",ch,NULL,NULL,TO_CHAR);
	act("$n calls upon the power of the stars!",ch,NULL,NULL,TO_ROOM);

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch) || !canPK(ch,vch,false))
			continue;
		dam = number_range(level*2,level * 3);
		if(saves_spell(level,vch,DAM_LIGHT))
			dam *= .75;
		spell_damage(ch,vch,dam,sn,DAM_LIGHT,true);
		dam = number_range(level*2,level * 3);
		if(saves_spell(level,vch,DAM_NEGATIVE))
			dam *= .75;
		spell_damage(ch,vch,dam,sn,DAM_NEGATIVE,true);
	}
}

void spell_eclipse(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int dam,tdam;

	if(weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT){
		act("You chant loudly and the sun is obscured by a dark disc!",ch,NULL,NULL,TO_CHAR);
		act("$n chants loudly and the sun is obscured by a dark disc!",ch,NULL,NULL,TO_ROOM);
		dam = number_range(level*2,level*4);
	}
	else if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK){
		act("You chant quietly and the moon darkens as Earth's shadow blankets it!",ch,NULL,NULL,TO_CHAR);
		act("$n chants quietly and the moon darkens as Earth's shadow blankets it!",ch,NULL,NULL,TO_ROOM);
		dam = number_range(level/2,level);
	}
	else{
		ch->send("What the eclipsing crap???\n\r");
		return;
	}

	for(vch = ch->in_room->people;vch;vch = vch_next){
		vch_next = vch->next_in_room;
		if(is_same_group(ch,vch) || !canPK(ch,vch,false))
			continue;
		tdam = dam;
		if(saves_spell(level,vch,DAM_LIGHT))
			tdam *= .75;
		spell_damage(ch,vch,tdam,sn,DAM_NEGATIVE,true);
	}
}

void spell_moonbeam(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	int dam;

	if(weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT){
		ch->send("You can't call on the moon's power in the day.\n\r");
		return;
	}

	act("You release a stream of lunar light at $N!",ch,NULL,victim,TO_CHAR);
	act("$n releases a stream of lunar light at you!",ch,NULL,victim,TO_VICT);
	act("$n releases a stream of lunar light at $N!",ch,NULL,victim,TO_NOTVICT);

	dam = number_range(level/2,level) + ch->level;
	if(!saves_spell(level,victim,DAM_NEGATIVE))
		dam *= 2;
	spell_damage(ch,victim,dam,sn,DAM_NEGATIVE,true);
}

void spell_sunbeam(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	int dam;

	if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK){
		ch->send("You can't call on the sun's power in the night.\n\r");
		return;
	}

	act("You release a stream of solar light at $N!",ch,NULL,victim,TO_CHAR);
	act("$n releases a stream of solar light at you!",ch,NULL,victim,TO_VICT);
	act("$n releases a stream of solar light at $N!",ch,NULL,victim,TO_NOTVICT);

	dam = number_range(level/2,level) + ch->level;
	if(!saves_spell(level,victim,DAM_LIGHT))
		dam *= 2;
	spell_damage(ch,victim,dam,sn,DAM_LIGHT,true);
}


void spell_solar_benison(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK){
		ch->send("You can't call on the sun's power in the night.\n\r");
		return;
	}
	if(is_affected(victim,sn)){
		if(victim == ch)
			send_to_char("You are already blessed by the sun.\n\r",ch);
		else
			act("$N is already blessed by the sun.",ch,NULL,victim,TO_CHAR);
		return;
	}
	affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_HITROLL,2*slvl,0);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_DAMROLL,2*slvl,0);
	send_to_char("You feel the blessings of the sun.\n\r",victim);
	if(ch != victim)
		act("$N is blessed by the sun.",ch,NULL,victim,TO_CHAR);
}
void spell_lunar_grace(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT){
		ch->send("You can't call on the sun's power in the night.\n\r");
		return;
	}
	if(is_affected(victim,sn)){
		if (victim == ch)
			send_to_char("You are already protected by the moon.\n\r",ch);
		else
			act("$N is already protected by the moon.",ch,NULL,victim,TO_CHAR);
		return;
	}
	affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_AC,-50 * ch->getslvl(sn),0);
	send_to_char("You feel the moon protecting you.\n\r",victim);
	if(ch != victim)
		act("$N is protected by the moon.",ch,NULL,victim,TO_CHAR);
}
void spell_stellar_curse(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn)){
		if (victim == ch)
			send_to_char("You are already cursed.\n\r",ch);
		else
			act("$N is already cursed.",ch,NULL,victim,TO_CHAR);
		return;
	}
	if(weather_info.sunlight == SUN_SET || weather_info.sunlight == SUN_DARK){
		affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_HITROLL,-ch->getslvl(sn),0);
		affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_DAMROLL,-ch->getslvl(sn),0);
	}
	else
		affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_AC,25 * ch->getslvl(sn),0);
	send_to_char("The stars curse you.\n\r",victim);
	if (ch != victim)
		act("$N is cursed by the stars.",ch,NULL,victim,TO_CHAR);
}
//Summons
void spell_poison_ivy(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int dam;

	act("You call upon a poison ivy plant to wrap around $N!",ch,NULL,victim,TO_CHAR);
	act("$n calls upon nature and a poison ivy plant wraps around you!",ch,NULL,victim,TO_VICT);
	act("$n calls a poison ivy plant to wrap around $N!",ch,NULL,victim,TO_NOTVICT);

	dam = number_range(level/2,level) + ch->level;
	if(!saves_spell(level,victim,DAM_POISON))
		dam *= 2;
	spell_damage(ch,victim,dam,sn,DAM_POISON,true);

	if(!saves_spell(level*2,victim,DAM_POISON)){
		act("$n is poisoned by the ivy!",victim,NULL,NULL,TO_ROOM);
		victim->send("You are poisoned by the ivy!\n\r");

		affect_join(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_STR,-2,AF_POISON/*_IVY*/);
	}
}

const struct charmy_type flora_table [7] ={
	{	"NULL",			"NULL",				"NULL.\n\r",													54,CLASS_IMMORTAL,"unique",1,""	},
	{	"acid spitter",	"an acid spitter",	"A large plant with its petals closed like a mouth is here.\n\r",60,CLASS_ROGUE,"unique",1,""	},
	{	"flytrap",		"a flytrap",		"An enormous, hungry flytrap is here.\n\r",						70,CLASS_ROGUE,"unique",1,""	},
};
void spell_flora(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	char buf[MSL];
	CHAR_DATA *pet;
	AFFECT_DATA af;
	const char *sizer;const char *sizer2;const char *animal;
	int race,a_max,n,i,thp;

    if(ch->isaff(AF_CHARM) || IS_NPC(ch))
		return;

	if(is_affected(ch,sn)){
		ch->send("You are too weary.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You may only summon floral aid in the wild.\n\r");
		return;
	}

	ch->send("You call for the aid of the plants.\n\r",ch);
	act("$n calls for the aid of the plants.",ch,NULL,NULL,TO_ROOM);

	pet	= create_mobile(get_mob_index(1));
	pet->level = ch->level;


	affect_set(ch,TO_AFFECTS,sn,ch->level,slvl,60,APPLY_NONE,0,AF_WEARY);;

	free_string(pet->short_descr);
	pet->short_descr = str_dup(flora_table[n].short_descr);
	free_string(pet->name);
	pet->name = str_dup(flora_table[n].name);
	free_string(pet->long_descr);
	pet->long_descr = str_dup(flora_table[n].long_descr);
	free_string(pet->description);
	pet->description = str_dup(flora_table[n].description);

	pet->pclass					= flora_table[i].pclass;
	pet->race					= race_lookup(flora_table[i].race);
	thp							= (ch->level * flora_table[i].hpmult) + 1000;
	pet->max_hit				= number_range(thp * .75, thp * 1.25);
	pet->hit					= pet->max_hit;
	pet->hitroll				= ch->level * 2;
	pet->alignment				= ch->alignment;
    pet->damage[DICE_NUMBER]	= (pet->level/2)+1;
    pet->damage[DICE_TYPE]		= 3;
    pet->damage[DICE_BONUS]		= (pet->level /4) + 1;
	pet->armor[AC_PIERCE]		= 0 - (pet->level *2);
	pet->armor[AC_BASH]			= 0 - (pet->level *2);
	pet->armor[AC_SLASH]		= 0 - (pet->level *2);
	pet->armor[AC_EXOTIC]		= 0 - (pet->level *3);
	pet->rottimer				= 30;

	pet->setact(AT_SENTINEL);

    pet->setmaxmana(dice((pet->level / 2)+1,pet->level/2) + (pet->level));
	pet->setmana(pet->getmaxmana());

	pet->setaff(AF_CHARM);
	pet->alignment				= ch->alignment;
	init_race(pet);
	char_to_room(pet,ch->in_room);
	add_follower(pet,ch,false);
	group_char(ch,pet);

	act("$N sprouts up to aid you.",ch,NULL,pet,TO_CHAR);
	act("$N sprouts up to aid $n.",ch,NULL,pet,TO_ROOM);
}
void spell_overgrowth(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA*)vo;

	if(victim == ch)
		affect_set(victim,TO_AFFECTS,sn,level,slvl,24,APPLY_AC,-50 * slvl,0);
	else if(ch->getslvl(sn) > 3 && !saves_spell(level,victim,DAM_PLANT))
		affect_set(victim,TO_AFFECTS,sn,level,slvl,1,APPLY_AC,50 * ch->getslvl(sn),AF_SNARED);
}
//Buffs
void spell_living_armor(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA *paf = new_affect();
	OBJ_DATA *newobj;
//add weariness
	if(!remove_obj(ch,WEAR_TORSO,true)){
		send_to_char("You can't remove the equipment on your torso.\n\r",ch);
		return;
	}
	newobj = create_object(get_obj_index(OBJ_VNUM_WEAR),ch->level);
	SET_BIT(newobj->wear_flags,ITEM_WEAR_ABOUT);
	newobj->timer = 5;
	affect_set(ch,TO_RESIST,gsn_eimbue,level,slvl,5,APPLY_NONE,0,RS_BASH);
	affect_set(ch,TO_VULN,gsn_eimbue,level,slvl,5,APPLY_NONE,0,RS_FIRE);

	newobj->value[0] = ch->level/10;
	newobj->value[1] = ch->level/10;
	newobj->value[2] = ch->level/10;
	newobj->value[3] = ch->level/9;

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_STR,slvl,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_END,slvl,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_SAVES,-ch->level/10,0);

	affect_set(newobj,TO_AFFECTS,sn,level,slvl,-1,APPLY_AC,-ch->level/10,AF_WEARY);

	newobj->short_descr = str_dup("a mantle of {yo{ga{yk{gm{ya{gi{yl{x");
	newobj->description = str_dup("A mantle of oak mail lies here.");
	newobj->name = str_dup("oakmail mail mantle");

	obj_to_char(newobj,ch);
	wear_obj(ch,newobj,true,false);

	act("$n is covered in a living armor of vines.",ch,newobj,NULL,TO_ROOM);
	act("Living vines crawl across your body, padding you.",ch,newobj,NULL,TO_CHAR);
}
void spell_barkskin(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

    if (is_affected(ch,sn)){
		ch->send("Your flesh is already like bark.\n\r");
		return;
	}
	ch->send("Your flesh takes on the toughness of bark.\n\r");
	act("$n's flesh takes on the toughness of bark.",ch,NULL,NULL,TO_ROOM);

	affect_set(ch,TO_AFFECTS,sn,level,slvl,ch->getslvl(sn) * 5,APPLY_AC,-100 * slvl,0);
}
void spell_oakmail(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA *paf;
	OBJ_DATA *newobj;

	if(!remove_obj(ch,WEAR_TORSO,true)){
		send_to_char("You can't remove the equipment on your torso.\n\r",ch);
		return;
	}
	newobj = create_object(get_obj_index(84),ch->level);
	newobj->timer = 5;
	newobj->value[0] = ch->level/10;
	newobj->value[1] = ch->level/10;
	newobj->value[2] = ch->level/10;
	newobj->value[3] = ch->level/9;

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_STR,slvl,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_END,slvl,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_SAVES,-ch->level/10,0);

	newobj->short_descr = str_dup("a mantle of {yo{ga{yk{gm{ya{gi{yl{x");
	newobj->description = str_dup("A mantle of oak mail lies here.");
	newobj->name = str_dup("oakmail mail mantle");

	obj_to_char(newobj,ch);
	wear_obj(ch,newobj,true,false);

	act("$n pulls $p onto $s chest.",ch,newobj,NULL,TO_ROOM);
	act("You create $p and wear it.",ch,newobj,NULL,TO_CHAR);
}
void spell_thorn_growth(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *wield;
	AFFECT_DATA *paf;

	if(!(wield = get_eq_char(ch,WEAR_WIELD))){
		ch->send("You have no weapon to cast this on.\n\r");
		return;
	}
	if(IS_WEAPON_STAT(wield,WPN_THORNED) || IS_WEAPON_STAT(wield,WPN_SHARP)){//Nash add more lata?
		ch->send("It already is affected.\n\r");
		return;
	}

	affect_set(wield,TO_WEAPON,sn,level,slvl,level,APPLY_DAMROLL,ch->level/20,WPN_THORNED);

	act("$n covers $p in thorns.",ch,wield,NULL,TO_ROOM);
	act("You cover $p in thorns.",ch,wield,NULL,TO_CHAR);
}
void spell_shield_of_thorns(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *shield;
	AFFECT_DATA *paf;

	if (calcHands(ch) > 1 || calcHands(ch) < 0){
		send_to_char("You must free up one of your hands first.\n\r",ch);
		return;
	}

	act("$n creates a shield made of woven thorned vines.",ch,shield,NULL,TO_ROOM);
	act("You create a shield made of woven thorned vines.",ch,shield,NULL,TO_CHAR);

	shield = create_object(get_obj_index(OBJ_VNUM_BONESHIELD),ch->level);
	shield->timer = 5;
	shield->level = ch->level;
	shield->value[0] = ch->level/11;
	shield->value[1] = ch->level/11;
	shield->value[2] = ch->level/11;
	shield->value[3] = ch->level/9;

	affect_set(shield,TO_OBJECT,sn,level,slvl,-1,APPLY_DAMROLL,ch->level/20,0);

	affect_set(shield,TO_OBJECT,sn,level,slvl,-1,APPLY_HITROLL,ch->level/20,0);

	affect_set(shield,TO_OBJECT,sn,level,slvl,-1,APPLY_SAVES,-ch->level/10,WPN_THORNED);//nashbroken bit

	obj_to_char(shield,ch);
	wear_obj(ch,shield,true,false);

	act("$n grasps $p as a shield.",ch,shield,NULL,TO_ROOM);
	act("You grasp $p as your shield.",ch,shield,NULL,TO_CHAR);
}
void spell_tranquility(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

    if (is_affected(ch,sn)){
		ch->send("You are already tranquil.\n\r");
		return;
	}
	ch->send("A wave of tranquility envelopes you.\n\r");
	act("$n seems quite tranquil.",ch,NULL,NULL,TO_ROOM);

	affect_set(ch,TO_AFFECTS,sn,level,slvl,2,APPLY_AC,-100 * slvl,AF_SILENCE);

	affect_set(ch,TO_AFFECTS,sn,level,slvl,2*slvl,APPLY_WIS,slvl,AF_TRANQUILITY);
}

//raffs
void spell_smokescreen(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA *paf;

	if (ch->in_room->israf(RAF_SMOKESCREEN)){
		ch->send("This room is already smoked.\n\r");
		return;
	}
	if(is_affected(ch,sn)){
		ch->send("You are too weary to do that.\n\r");
		return;
	}
	act("You release a thick cloud of smoke into the room, obscuring everything!",ch,NULL,NULL,TO_CHAR);
	act("$n releases a thick cloud of smoke, obscuring everything!",ch,NULL,NULL,TO_ROOM);
	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,UMAX(1,ch->getslvl(sn)) * 6,APPLY_NONE,0,RAF_SMOKESCREEN);

	affect_set(ch,TO_AFFECTS,sn,level,slvl,100,APPLY_NONE,0,AF_WEARY);
}
void spell_grove_sanctuary(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA *paf;

	if (ch->in_room->israf(RAF_GROVE)){
		ch->send("This room is already in sanctuary.\n\r");
		return;
	}
	if(is_affected(ch,sn)){
		ch->send("You are too weary to do this.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You may only make natural areas sanctuaries.\n\r");
		return;
	}
	act("With a wave of your hand, everything starts to release an ethereal glow.",ch,NULL,NULL,TO_CHAR);
	act("$n waves $s hand and everything begins to glow with an ethereal light.",ch,NULL,NULL,TO_ROOM);

	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl) * 6,APPLY_NONE,0,RAF_GROVE);

	affect_set(ch,TO_AFFECTS,sn,level,slvl,360,APPLY_NONE,0,AF_WEARY);
}
void spell_zephyr_wall(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	ROOM_INDEX_DATA *room;
	AFFECT_DATA *af;
	int door = target;

	if(is_affected(ch,sn)){
		ch->send("You are too weary to conjure a zephyr wall.\n\r");
		return;
	}
	if(ch->in_room->israf(RAF_WINDWALL)){
		ch->send("There is already a wall of wind here.\n\r");
		return;
	}
	act("You call upon the winds to create a wall $T.",ch,NULL,dir_name[door],TO_CHAR);
	act("$n calls upon the winds to create a wall $T.",ch,NULL,dir_name[door],TO_ROOM);

	affect_set(ch,ch->in_room,TO_AFFECTS,sn,level,slvl,slvl*2,APPLY_NONE,door,RAF_WINDWALL);

	affect_set(ch,ch,TO_AFFECTS,sn,level,slvl,360,APPLY_NONE,0,0);
}
void check_zephyr_wall(CHAR_DATA *ch,int from_door){//So do we want this to block people from entering? Maybe let in high str peeps? level? Rar?
	AFFECT_DATA *af = affect_find(ch->in_room->affected,skill_lookup("alarm"));
	if(!ch->in_room->israf(RAF_MWALL))
		return;
	if(!af){
		ch->in_room->remraf(RAF_WINDWALL);
		return;
	}
	if(!af->parent){
		affect_remove_room(ch->in_room,af);
		return;
	}
	if(af->modifier != from_door)
		return;
	if(number_percent() > 80){
		act("The mystic wall sizzles and pops.",ch,NULL,NULL,TO_ROOM);
		act("The mystic wall sizzles and pops.",ch,NULL,NULL,TO_CHAR);
	}
	if(!canPK(af->parent,ch,false) || saves_spell(af->parent->level,ch,DAM_WIND))
		return;//make it randomly move you, and needs weary
	act("$N is blow out as $e tries to enter!",af->parent,NULL,ch,TO_CHAR);
	act("You are flung out as you try to enter!",af->parent,NULL,ch,TO_VICT);
	act("$N is blown away by $n's wall of wind!",af->parent,NULL,ch,TO_NOTVICT);
	spell_damage(af->parent,ch,number_range(af->level * af->duration,af->level),af->type,DAM_ENERGY,true);
}
void spell_quagmire(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

	if (ch->in_room->israf(RAF_QUAGMIRE)){
		ch->send("This room is already boggy.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You may only make places in nature boggy.\n\r");
		return;
	}
	act("You touch the ground, and it becomes thick and boggy.",ch,NULL,NULL,TO_CHAR);
	act("$n touches the ground, and it turns into a thick bog.",ch,NULL,NULL,TO_ROOM);

	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl) * 6,APPLY_NONE,0,RAF_QUAGMIRE);
}
void spell_forest_mist(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

	if (ch->in_room->israf(RAF_FOREST_MIST)){
		ch->send("This room is already misty.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You may only fill places in nature with mist.\n\r");
		return;
	}
	act("You call upon nature and a soothing mist fills the air.",ch,NULL,NULL,TO_CHAR);
	act("$n calls on nature to release a soothing mist that fills the air.",ch,NULL,NULL,TO_ROOM);
	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl) * 6,APPLY_NONE,0,RAF_FOREST_MIST);
}

//animals
const struct charmy_type companion_table [7] =
{
	//  name,			short,				long											hpmult	class				race	mkelvl		desc
	{	"NULL",			"NULL",				"NULL.\n\r",										54,	CLASS_IMMORTAL,		"unique",	1,		""	},
	{	"squirrel",		"a squirrel",		"A bushy tailed squirrel is here.\n\r",				60,	CLASS_ROGUE,		"rodent",	1,		""	},
	{	"fat badger",	"a badger",			"A fat badger is here.\n\r",						70,	CLASS_ROGUE,		"rodent",	1,		""	},
	{	"fox",			"a fox",			"A sly fox is here.\n\r",							83,	CLASS_ASSASSIN,		"canine",	1,		""	},
	{	"bear",			"a bear",			"A lumbering bear is here.\n\r",					95,	CLASS_PALADIN,		"bear",		1,		""	},
	{	"lion",			"a lion",			"A proud lion is here.\n\r",						100,CLASS_MERCENARY,	"feline",	1,		""	},
	{	"elder stag",	"an elder stag",	"A beautiful elder stag holds its head high.\n\r",	150,CLASS_IMMORTAL,		"horse",	1,		""	},
};

void spell_animal_companion(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	char buf[MSL];
	CHAR_DATA *pet;
	AFFECT_DATA af;
	const char *sizer;const char *sizer2;const char *animal;
	int race,a_max,n,thp;

    if(ch->isaff(AF_CHARM) || IS_NPC(ch))
		return;

	if(ch->pet){
		send_to_char("You already have a follower.\n\r",ch);
		return;
	}
	if(is_affected(ch,sn)){
		ch->send("You are too weary.\n\r");
		return;
	}
	if(!IS_NATURE_BG(ch)){
		ch->send("You may only call upon the aid of an animal in nature.\n\r");
		return;
	}

	ch->send("You call into the forest for a companion.\n\r",ch);
	act("$n calls out into the forest for a companion.",ch,NULL,NULL,TO_ROOM);

	pet	= create_mobile(get_mob_index(1));
	pet->level = ch->level;

	if(ch->level > 100)
		a_max = 6;
	else if(ch->level > 90)
		a_max = 5;
	else if(ch->level > 80)
		a_max = 4;
	else if(ch->level > 70)
		a_max = 3;
	else if(ch->level > 60)
		a_max = 2;
	else
		a_max = 1;

	n = number_range(0,a_max);

	if(n == 0){
		affect_set(ch,TO_AFFECTS,sn,ch->level,slvl,5,APPLY_NONE,0,AF_WEARY);
		ch->send("You failed.\n\r");
		return;
	}
	affect_set(ch,TO_AFFECTS,sn,ch->level,slvl,100,APPLY_NONE,0,AF_WEARY);

	free_string(pet->short_descr);
	pet->short_descr = str_dup(companion_table[n].short_descr);

	free_string(pet->name);
	pet->name = str_dup(companion_table[n].name);

	free_string(pet->long_descr);
	pet->long_descr = str_dup(companion_table[n].long_descr);

	free_string(pet->description);
	pet->description = str_dup(companion_table[n].description);

	pet->pclass					= companion_table[n].pclass;
	pet->race					= race_lookup(companion_table[n].race);
	thp							= (ch->level * companion_table[n].hpmult) + 1000;
	pet->max_hit				= number_range(thp * .75, thp * 1.25);
	pet->hit					= pet->max_hit;
	pet->hitroll				= ch->level * 2;
	pet->alignment				= ch->alignment;
    pet->damage[DICE_NUMBER]	= (pet->level/2)+1;
    pet->damage[DICE_TYPE]		= 3;
    pet->damage[DICE_BONUS]		= (pet->level /4) + 1;
	pet->armor[AC_PIERCE]		= 0 - (pet->level *2);
	pet->armor[AC_BASH]			= 0 - (pet->level *2);
	pet->armor[AC_SLASH]		= 0 - (pet->level *2);
	pet->armor[AC_EXOTIC]		= 0 - (pet->level *3);

    pet->setmaxmana(dice((pet->level / 2)+1,pet->level/2) + (pet->level));
	pet->setmana(pet->getmaxmana());

	init_familiar(ch,pet);

	act("$N walks in to follow you.",ch,NULL,pet,TO_CHAR);
	act("$N walks in to follow $n.",ch,NULL,pet,TO_ROOM);
}

//supers
void spell_yggdrassil_prayer(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *pet = ch->pet;
	AFFECT_DATA *af;

	if(is_affected(ch,sn)){
		ch->send("You are too weary to do this.\n\r");
		return;
	}
	affect_set(ch,TO_AFFECTS,sn,level,slvl,240,APPLY_NONE,0,AF_WEARY);

	ch->send("You call upon the divine power of the tree of life.\n\r");
	ch->send("Your body glows red.\n\r");
	act("$n glows red.",ch,NULL,NULL,TO_ROOM);
	ch->hit = ch->max_hit;
	while((af = ch->affected)){
		ch->affected = af->next;
		affect_remove(ch,af);
	}
	if(pet){
		act("$n glows red.",pet,NULL,NULL,TO_ROOM);
		while((af = pet->affected)){
			pet->affected = af->next;
			affect_remove(ch,af);
		}
	}
}
void spell_arms_of_gaia(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *pet = ch->pet;
	AFFECT_DATA *af;

	if(is_affected(ch,sn)){
		ch->send("You are too weary to do this.\n\r");
		return;
	}
	affect_set(ch,TO_AFFECTS,sn,level,slvl,240,APPLY_NONE,0,AF_WEARY);

	ch->send("You call upon the divine power of the earth.\n\r");
	ch->send("You are enveloped in a warming glow.\n\r");
	act("$n is enveloped in a green glow.",ch,NULL,NULL,TO_ROOM);
	ch->hit = ch->max_hit;
	if(pet){
		act("$n is enveloped in a warming green glow.",pet,NULL,NULL,TO_ROOM);
		pet->hit = pet->max_hit;
	}
}
void spell_invigorate(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *pet = ch->pet;
	AFFECT_DATA *af;

	if(!pet){
		ch->send("You can only cast this on your companion.\n\r");
		return;
	}
	if(is_affected(pet,sn)){
		ch->send("They are already invigorated.\n\r");
		return;
	}
	affect_set(pet,TO_AFFECTS,sn,level,slvl,slvl,APPLY_WIS,slvl * 2,AF_HASTE);//nashhhhh no haste
}
void spell_windwalk(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA *af;
	if(ch->isaff(AF_WINDWALK)){
		ch->send("You already walk on wind.\n\r");
		return;
	}
	affect_set(ch,TO_AFFECTS,sn,level,slvl,slvl,APPLY_AC,slvl * 2,AF_WINDWALK);
	ch->send("Your feet rise just above the ground, lightening your step.\n\r");
	act("$n's feet rise just off the ground.",ch,NULL,NULL,TO_ROOM);
}
void spell_vitalize(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *pet = ch->pet;
	AFFECT_DATA *af;

	if(vo){
		ch->send("You cannot target someone with this spell.\n\r");
		return;
	}
	if(!pet){
		ch->send("You can only cast this on your companion.\n\r");
		return;
	}
	if(is_affected(pet,sn)){
		ch->send("They are already vitalized.\n\r");
		return;
	}
	affect_set(pet,TO_AFFECTS,sn,level,slvl,slvl,APPLY_HIT,slvl * 500,AF_REGENERATION);
}

//weather
void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	switch(current_weather[ch->in_room->area->climate]){
		case SKY_CLOUDLESS://nash maybe make this do no damage... no weather, right?
			if(weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT){
				act("The sun's rays strike down at the ground!",ch,NULL,NULL,TO_CHAR);
				act("The sun's rays strike down at the ground!",ch,NULL,NULL,TO_ROOM);
			}
			else{
				act("The moon's rays strike down at the ground!",ch,NULL,NULL,TO_CHAR);
				act("The moon's rays strike down at the ground!",ch,NULL,NULL,TO_ROOM);
			}
			return;
		case SKY_CLOUDY:
			act("You fling a cloud!",ch,NULL,NULL,TO_CHAR);
			act("$n flings a cloud!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_RAINING:
			act("You call upon the rain!",ch,NULL,NULL,TO_CHAR);
			act("$n call upon the rain!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_WINDY:
			act("A strong gust of wind roars through!",ch,NULL,NULL,TO_CHAR);
			act("A strong gust of wind roars through!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_LIGHTNING:
			act("Lightning strikes this area!",ch,NULL,NULL,TO_CHAR);
			act("Lightning strikes this area!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_SNOWING:
			act("The snow blasts through!",ch,NULL,NULL,TO_CHAR);
			act("The snow blasts through!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_BLIZZARD:
			act("A fierce blizzard strikes!",ch,NULL,NULL,TO_CHAR);
			act("A fierce blizzard strikes!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_FOGGY:
			act("The thick fog grows frigid!",ch,NULL,NULL,TO_CHAR);
			act("The thick fog grows frigid!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_HAILSTORM:
			act("The hail stones double in size!",ch,NULL,NULL,TO_CHAR);
			act("The hail stones double in size!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_THUNDERSTORM:
			act("There is a loud crack as thunderbolts strike!",ch,NULL,NULL,TO_CHAR);
			act("There is a loud crack as thunderbolts strike!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_SANDSTORM:
			act("The sandstorm rages more fiercely!",ch,NULL,NULL,TO_CHAR);
			act("The sandstorm rages more fiercely!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_HEATWAVE:
			act("The intensity of the heat increases!",ch,NULL,NULL,TO_CHAR);
			act("The intensity of the heat increases!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_FREEZE:
			act("The intensity of the cold increases!",ch,NULL,NULL,TO_CHAR);
			act("The intensity of the cold increases!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_ICESTORM:
			act("Sheets of ice strike!",ch,NULL,NULL,TO_CHAR);
			act("Sheets of ice strike!",ch,NULL,NULL,TO_ROOM);
			return;
	};
}
void spell_pleasant_climate(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	switch(current_weather[ch->in_room->area->climate]){
		case SKY_CLOUDLESS:
			if(weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT){
				act("The sun's rays strike down at the ground!",ch,NULL,NULL,TO_CHAR);
				act("The sun's rays strike down at the ground!",ch,NULL,NULL,TO_ROOM);
			}
			else{
				act("The moon's rays strike down at the ground!",ch,NULL,NULL,TO_CHAR);
				act("The moon's rays strike down at the ground!",ch,NULL,NULL,TO_ROOM);
			}
			return;
		case SKY_CLOUDY:
			act("You fling a cloud!",ch,NULL,NULL,TO_CHAR);
			act("$n flings a cloud!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_RAINING:
			act("You call upon the rain!",ch,NULL,NULL,TO_CHAR);
			act("$n call upon the rain!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_WINDY:
			act("A strong gust of wind roars through!",ch,NULL,NULL,TO_CHAR);
			act("A strong gust of wind roars through!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_LIGHTNING:
			act("Lightning strikes this area!",ch,NULL,NULL,TO_CHAR);
			act("Lightning strikes this area!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_SNOWING:
			act("The snow blasts through!",ch,NULL,NULL,TO_CHAR);
			act("The snow blasts through!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_BLIZZARD:
			act("A fierce blizzard strikes!",ch,NULL,NULL,TO_CHAR);
			act("A fierce blizzard strikes!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_FOGGY:
			act("The thick fog grows frigid!",ch,NULL,NULL,TO_CHAR);
			act("The thick fog grows frigid!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_HAILSTORM:
			act("The hail stones double in size!",ch,NULL,NULL,TO_CHAR);
			act("The hail stones double in size!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_THUNDERSTORM:
			act("There is a loud crack as thunderbolts strike!",ch,NULL,NULL,TO_CHAR);
			act("There is a loud crack as thunderbolts strike!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_SANDSTORM:
			act("The sandstorm rages more fiercely!",ch,NULL,NULL,TO_CHAR);
			act("The sandstorm rages more fiercely!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_HEATWAVE:
			act("The intensity of the heat increases!",ch,NULL,NULL,TO_CHAR);
			act("The intensity of the heat increases!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_FREEZE:
			act("The intensity of the cold increases!",ch,NULL,NULL,TO_CHAR);
			act("The intensity of the cold increases!",ch,NULL,NULL,TO_ROOM);
			return;
		case SKY_ICESTORM:
			act("!",ch,NULL,NULL,TO_CHAR);
			act("!",ch,NULL,NULL,TO_ROOM);
			return;
	};
}
