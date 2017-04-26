#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"

bool	remove_obj			( CHAR_DATA*,int,bool );
bool	check_dual_blade	( CHAR_DATA*,OBJ_DATA*,OBJ_DATA* );
int		check_fervor		( CHAR_DATA*,int );
bool is_weapon_pierce(OBJ_DATA *obj){
	switch (obj->value[0]){
		case(WEAPON_RAPIER):
		case(WEAPON_MAINGAUCHE):
		case(WEAPON_GAUNTLET):
		case(WEAPON_DIRK):
		case(WEAPON_SAI):
		case(WEAPON_DAGGER):
		case(WEAPON_POLEARM):
		case(WEAPON_SPEAR):
		case(WEAPON_LANCE):
			return true;
	}
	return false;
}

bool is_weapon_small_pierce(OBJ_DATA *obj){
	switch (obj->value[0]){
		case(WEAPON_MAINGAUCHE):
		case(WEAPON_DIRK):
		case(WEAPON_SAI):
		case(WEAPON_DAGGER):
			return true;
	}
	return false;
}

bool is_weapon_big_pierce(OBJ_DATA *obj){
	switch (obj->value[0]){
		case(WEAPON_LANCE):
		case(WEAPON_POLEARM):
		case(WEAPON_SPEAR):
		case(WEAPON_RAPIER):
			return true;
	}
	return false;
}

bool is_weapon_sharp(OBJ_DATA *obj){
	switch (obj->value[0]){
		case(WEAPON_LONGSWORD):
		case(WEAPON_BASTARDSWORD):
		case(WEAPON_DAGGER):
		case(WEAPON_DIRK):
		case(WEAPON_AXE):
		case(WEAPON_BATTLEAXE):
		case(WEAPON_SCYTHE):
		case(WEAPON_MACHETE):
		case(WEAPON_RAPIER):
		case(WEAPON_KATANA):
		case(WEAPON_SCIMITAR):
		case(WEAPON_SAI):
		case(WEAPON_GLADIUS):
		case(WEAPON_MAINGAUCHE):
		case(WEAPON_LANCE):
			return true;
	}
	return false;
}

bool is_weapon_sword(OBJ_DATA *obj){
	switch (obj->value[0]){
		case(WEAPON_LONGSWORD):
		case(WEAPON_BASTARDSWORD):
		case(WEAPON_DAGGER):
		case(WEAPON_DIRK):
		case(WEAPON_RAPIER):
		case(WEAPON_KATANA):
		case(WEAPON_SCIMITAR):
		case(WEAPON_MAINGAUCHE):
		case(WEAPON_GLADIUS):
		case(WEAPON_MACHETE):
			return true;
	}
	return false;
}

bool is_weapon_bigslash(OBJ_DATA *obj){
	switch (obj->value[0]){
		case(WEAPON_LONGSWORD):
		//case(WEAPON_SCIMITAR)://NASH
		//case(WEAPON_KATANA):
		case(WEAPON_BATTLEAXE):
		case(WEAPON_BASTARDSWORD):
		case(WEAPON_MACHETE):
		case(WEAPON_SCYTHE):
			return true;
	}
	return false;
}

bool is_weapon_blunt(OBJ_DATA *obj){
	switch (obj->value[0]){
		case(WEAPON_MACE):
		case(WEAPON_HAMMER):
		case(WEAPON_WARHAMMER):
		case(WEAPON_QUARTERSTAFF):
		case(WEAPON_NUNCHAKU):
		case(WEAPON_STAFF):
		case(WEAPON_TONFA):
		case(WEAPON_FLAIL):
		case(WEAPON_LONGBOW):
		case(WEAPON_SHORTBOW):
		case(WEAPON_SAI):
		case(WEAPON_CLUB):
		case(WEAPON_GAUNTLET):
			return true;
	}
	return false;
}

int get_attackspeed(CHAR_DATA *ch,bool secondary){
	OBJ_DATA *wield;
	int n=0,iWear=0,sn,tarm=0;
char buf[MSL];
	if(IS_NPC(ch))
		n = 10 + STAT_MAX - get_curr_stat(ch,STAT_AGI);
	else{
		n = 200;
		n -= 75 * (get_curr_stat(ch,STAT_AGI)) / (STAT_MAX);

					if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"{dBASP({c%d{d){x",n);

		if(!IS_IMMORTAL(ch))
			n += 25 * get_carry_weight(ch) / can_carry_w(ch);

			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PWht({c%d{d){x",n);
		for (iWear = 0;iWear < MAX_WEAR;iWear++){
			if (!(wield = get_eq_char(ch,iWear)) || wield->armortype_flags < ARMOR_LIGHT)
				continue;
			tarm += wield->armortype_flags - 1;
		}
		n += tarm / 2;
				if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PArm({c%d{d){x",n);
		n /= 5;
				if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PCrp({c%d{d){x",n);
	}

	if (!secondary){
		if (!(wield = get_eq_char(ch,WEAR_WIELD)) && get_skill(ch,gsn_shieldfighting) > 0){
			wield = get_eq_char(ch,WEAR_SHIELD);
			sn = gsn_shieldfighting;
		}
		else
			sn = get_weapon_sn(ch,false);
		if (wield)
			n -= IS_NPC(ch) ? n / 5 : UMAX(get_weapon_skill(ch,sn)/5,1);
		else
			n -= IS_NPC(ch) ? n / 4 : UMAX(get_skill(ch,gsn_combatives)/5,1);
	}
	else{
		if((wield = get_eq_char(ch,WEAR_SECONDARY)))
			n -= IS_NPC(ch) ? n / 5 : UMAX(((get_weapon_skill(ch,get_weapon_sn(ch,true)) * get_skill(ch,gsn_dual_wield)) / 100)/5,1);
		else
			n -= IS_NPC(ch) ? n / 4 : UMAX((get_skill(ch,gsn_combatives) * get_skill(ch,gsn_dual_wield) /100)/5,1);
	}
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"Pwpn({c%d{d){x",n);
	if (wield)
		if (wield->item_type == ITEM_WEAPON){
			n += weapon_table[wield->value[0]].size * 5;

if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PSze({c%d{d){x",n);
			n -= check_swordefficiency(ch,wield,secondary);
if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PSef({c%d{d){x",n);
		}

	n = UMAX(n,1);

	if(ch->isaff(AF_ETHEREAL_ALACRITY))
		n *= .75;
	if (ch->isaff(AF_HASTE) || (IS_NPC(ch) && ch->isoff(OF_FAST)))
		n *= .75;			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PHst({c%d{d){x",n);

	if(!IS_NPC(ch) && ch->iscomm(CM_MORPH)){
		if(ch->morph == MORPH_FELINE)
			n *= 1.0 + (double)(ch->getslvl(gsn_morph_feline)/5);
		if(ch->morph == MORPH_SERPENT)
			n *= 0.8 + (double)(ch->getslvl(gsn_morph_serpent)/10);
		if(ch->morph == MORPH_URSA)
			n *= 0.8 - (double)(ch->getslvl(gsn_morph_ursa)/10);
		if(ch->morph == MORPH_AVIAN)
			n *= (15 + (double)ch->getslvl(gsn_morph_avian)) / 10;
	}

	if (ch->isaff(AF_SLOW))
		n *= 2;				if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PSlw({c%d{d){x",n);

	if (ch->isaff(AF_POWER_RUSH))
		n *= .8;				if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PRsh({c%d{d){x",n);

	n = check_fervor(ch,n);	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PFvr({c%d{d){x",n);

	if(ch->fighting && ch->mark && ch->mark == ch->fighting)
		n *= .75;			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PMrk({c%d{d){x",n);

	if(is_affected(ch,gsn_culmination))
		n -= n / 10 * ch->getslvl(gsn_culmination);	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PClm({c%d{d){x",n);

	if(ch->isaff(AF_BULWARK))
		n += ((n/2) * ch->getslvl(gsn_bulwark))/4;	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PBul({c%d{d){x",n);

	if (check_icebound(ch) == 1)
		n *= .75;			if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PIce({c%d{d){x",n);

	if (check_icebound(ch) == 2)
		n *= 2;				if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PIce({c%d{d){x",n);

	if(ch->level < 10)
		n /= 2;				if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"PNewb({c%d{d){x\n\r",n);

	return UMAX(n,1);
}

bool check_swordsmanship(CHAR_DATA *ch,bool isskill,bool issecond){
	int skill = get_skill(ch,gsn_swordsmanship);
	OBJ_DATA *obj = get_eq_char(ch,WEAR_WIELD);

	if(skill < 1)
		return false;
	if (!obj && issecond)
		obj = get_eq_char(ch,WEAR_SECONDARY);

	skill /= 3;
	if (obj && obj->item_type == ITEM_WEAPON && is_weapon_sharp(obj)){
		if (number_percent() <= skill){
			check_improve(ch,gsn_swordsmanship,true,1);
			return true;
		}
		else{
			check_improve(ch,gsn_swordsmanship,false,1);
			return false;
		}
	}
}

int check_swordefficiency(CHAR_DATA *ch,OBJ_DATA *wield,bool secondary){
	int n=0,skill=get_skill(ch,gsn_swordefficiency) *.45;

	if (skill < 1) return n;

	if (!is_weapon_sword(wield)) return n;

	skill = skill * (get_curr_stat(ch,STAT_AGI) + get_curr_stat(ch,STAT_INT) + get_curr_stat(ch,STAT_STR)) / (STAT_MAX * 4);

	if (secondary)
		skill = (skill * get_skill(ch,gsn_dual_wield)) / 100;

	
if(!IS_NPC(ch) && ch->iscomm(CM_DEBUG))printf_to_char(ch,"SEF({c%d{d) ",skill);
	if (number_percent() < skill){
		n = skill/3;
		check_improve(ch,gsn_swordefficiency,true,1);
	}
	else
		check_improve(ch,gsn_swordefficiency,false,1);
	return n;
}

bool check_swordmastery(CHAR_DATA *ch,bool isskill,bool issecond){
	int skill = get_skill(ch,gsn_swordmastery);
	OBJ_DATA *obj;

	if (issecond)	obj = get_eq_char(ch,WEAR_SECONDARY);
	else			obj = get_eq_char(ch,WEAR_WIELD);

	if(skill < 1 || !obj || !is_weapon_sharp(obj) || grab_skill(ch,get_weapon_sn(ch,issecond)) < 100)
		 return false;
	skill *= .25;

	if (number_percent() <= skill){
		check_improve(ch,gsn_swordmastery,true,2);
		return true;
	}
	else{
		check_improve(ch,gsn_swordmastery,false,4);
		return false;
	}
}

bool check_lancemastery(CHAR_DATA *ch){
	int skill = get_skill(ch,gsn_lancemastery);
	OBJ_DATA *obj = get_eq_char(ch,WEAR_WIELD);

	if (!obj || obj->value[0] != WEAPON_LANCE)
		return false;

	skill *= .25;
	if (skill > 0){
		if (number_percent() <= skill){
			check_improve(ch,gsn_lancemastery,true,2);
			return true;
		}
		else{
			check_improve(ch,gsn_lancemastery,false,4);
			return false;
		}
	}
}

bool IS_WEAPON_STAT(OBJ_DATA *obj,int stat){
	if (!obj || obj->item_type != ITEM_WEAPON)
		return false;
	if (stat > MAX_WPN || stat < 0)
		return false;
	return obj->wflags[stat];
}

void SET_WFLAG(OBJ_DATA *obj,int stat){
	if (!obj || obj->item_type != ITEM_WEAPON || stat > MAX_WPN || stat < 0){
		log_f("SET_WFLAG: Icky weaponstuff!");
		return;
	}
	obj->wflags[stat] = true;
}

void REMOVE_WFLAG(OBJ_DATA *obj,int stat){
	if (!obj || obj->item_type != ITEM_WEAPON || stat > MAX_WPN || stat < 0){
		log_f("SET_WFLAG: Icky weaponstuff!");
		return;
	}
	obj->wflags[stat] = false;
}

void do_exchange(CHAR_DATA *ch,char *argument){
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD),*dwield = get_eq_char(ch,WEAR_SECONDARY);

	if(get_skill(ch,gsn_dual_wield) < 1){
		ch->send("You don't know how to do this.\n\r");
		return;
	}
	if(!wield || !dwield){
		ch->send("You may only do this when dual wielding.\n\r");
		return;
	}
	if(!check_dual_blade(ch,dwield,wield)){
		ch->send("You can't switch these two weapons.\n\r");
		return;
	}
    if(IS_SET(wield->extra_flags,ITM_NOREMOVE)){
		ch->send("You cannot remove your primary wield.\n\r");
		return;
	}
    if(IS_SET(dwield->extra_flags,ITM_NOREMOVE)){
		ch->send("You cannot remove your dual wield.\n\r");
		return;
	}
	if (!IS_NPC(ch) && get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield * 10)){
		send_to_char("Your primary wield is too heavy.\n\r",ch);
		return;
	}
	if (!IS_NPC(ch) && get_obj_weight(dwield) > (str_app[get_curr_stat(ch,STAT_STR)].wield * 10)){
		send_to_char("Your secondary wield is too heavy.\n\r",ch);
		return;
	}
	unequip_char(ch,dwield);
	if(CAN_WEAR(dwield,ITEM_WIELD)){
		int sn,skill;

		if (!remove_obj(ch,WEAR_WIELD,true))
			return;

		char buf[10];
		sprintf(buf,"%s",ch->lefty ? "left" : "right");
		act("$n wields $p in $s $d hand.",ch,dwield,buf,TO_ROOM);
		act("You wield $p in your $d hand.",ch,dwield,buf,TO_CHAR);
		equip_char(ch,dwield,WEAR_WIELD);

		sn = get_weapon_sn(ch,false);

		if (sn == gsn_combatives)
		   return;

		skill = get_weapon_skill(ch,sn);

		if (skill >= 100)		act("$p feels like a part of you!",ch,dwield,NULL,TO_CHAR);
		else if (skill > 85)	act("You feel quite confident with $p.",ch,dwield,NULL,TO_CHAR);
		else if (skill > 70)	act("You are skilled with $p.",ch,dwield,NULL,TO_CHAR);
		else if (skill > 50)	act("Your skill with $p is adequate.",ch,dwield,NULL,TO_CHAR);
		else if (skill > 25)	act("$p feels a little clumsy in your hands.",ch,dwield,NULL,TO_CHAR);
		else if (skill > 1)		act("You fumble and almost drop $p.",ch,dwield,NULL,TO_CHAR);
		else					act("You don't even know which end is up on $p.",ch,dwield,NULL,TO_CHAR);
	}
	if(CAN_WEAR(wield,ITEM_WIELD)){
		int sn,skill;

		char buf[10];
		sprintf(buf,"%s",!ch->lefty ? "left" : "right");
		act("$n wields $p in $s $d hand.",ch,wield,buf,TO_ROOM);
		act("You wield $p in your $d hand.",ch,wield,buf,TO_CHAR);
		equip_char(ch,wield,WEAR_SECONDARY);

		sn = get_weapon_sn(ch,false);

		if (sn == gsn_combatives)
		   return;

		skill = get_weapon_skill(ch,sn);

		if (skill >= 100)		act("$p feels like a part of you!",ch,wield,NULL,TO_CHAR);
		else if (skill > 85)	act("You feel quite confident with $p.",ch,wield,NULL,TO_CHAR);
		else if (skill > 70)	act("You are skilled with $p.",ch,wield,NULL,TO_CHAR);
		else if (skill > 50)	act("Your skill with $p is adequate.",ch,wield,NULL,TO_CHAR);
		else if (skill > 25)	act("$p feels a little clumsy in your hands.",ch,wield,NULL,TO_CHAR);
		else if (skill > 1)		act("You fumble and almost drop $p.",ch,wield,NULL,TO_CHAR);
		else					act("You don't even know which end is up on $p.",ch,wield,NULL,TO_CHAR);
	}
}
