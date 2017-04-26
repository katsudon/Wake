#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "lookup.h"

/* * Local functions.
 */
bool		remove_obj		( CHAR_DATA*,int,bool );
void		init_familiar	( CHAR_DATA*,CHAR_DATA* );
AFFECT_DATA	*new_affect		( void );


void do_devour(CHAR_DATA *ch,char *argument){
	char arg[MSL];
	OBJ_DATA *corpse;
	int chance = get_skill(ch,gsn_devour) *.7;

	one_argument(argument,arg);
	if (!(corpse = get_obj_carry(ch,arg,ch))){
		send_to_char("Get what?\n\r",ch);
		return;
	}

	if (corpse->item_type != ITEM_CORPSE_NPC){
		send_to_char("You can only devour NPC corpse.\n\r",ch);
		return;
	}

	if (corpse->contains){
		send_to_char("It's not empty, don't want to eat any knives accidentally.\n\r",ch);
		return;
	}

	WAIT_STATE(ch,skill_table[gsn_devour].beats);
	if (number_percent() < chance){
		act("You voraciously devour $p.",ch,corpse,NULL,TO_CHAR);
		act("$n voraciously devours $p.",ch,corpse,NULL,TO_ROOM);
		ch->hit += corpse->level * 2;
		ch->modmana(corpse->level /2);
		check_improve(ch,gsn_devour,true,4);
	}
	else{
		act("You improperly eat $p and ruin the body.",ch,corpse,NULL,TO_ROOM);
		act("$n improperly eats $p and ruins the body.",ch,corpse,NULL,TO_ROOM);
		check_improve(ch,gsn_devour,false,4);
	}
	extract_obj(corpse);
}

void spell_phantom_sword(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;

	if (is_affected(ch,sn)){
		if (victim == ch)
			send_to_char("You already have phantom assistance.\n\r",ch);
		return;
	}

	affect_set(ch,TO_AFFECTS,sn,level,slvl,level/10,APPLY_NONE,0,AF_PSWORD);
    act("A phantom sword appears beside $n.",victim,NULL,NULL,TO_ROOM);
    send_to_char("You feel the presence of a phantom's sword next to you.\n\r",victim);
}

void spell_nightvision(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;

	if (is_affected(ch,sn) || ch->isaff(AF_DARK_VISION)){
		if (victim == ch)
			send_to_char("You already have darkvision.\n\r",ch);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level/10,APPLY_NONE,0,AF_DARK_VISION);
	affect_set(victim,TO_VULN,sn,level,slvl,level/10,APPLY_NONE,0,RS_LIGHT);
    act("$n's eyes begin to glow faintly and $s pupils dialate.",victim,NULL,NULL,TO_ROOM);
    send_to_char("You feel your eyes glow faintly and your pupils dialate.\n\r",victim);
}

void spell_scarification(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;
	OBJ_DATA *obj;

	if(is_affected(ch,sn))
	{
		if (victim == ch)
			send_to_char("Your skin is already scarred.\n\r",ch);
		else
			act("$N is already scarred.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if(!remove_obj(ch,WEAR_ARMS,true))
	{
		send_to_char("You can't remove the equipment on your arms.\n\r",ch);
		return;
	}

	obj = create_object(get_obj_index(83),0);
	obj->timer = 10;
	obj->level = level;
	obj_to_char(obj,ch);
	wear_obj(ch,obj,true,false);

	send_to_char("You sear your flesh, carving patterns into your arms.\n\r",ch);
	act("$n sears $s arms.",ch,NULL,NULL,TO_ROOM);
	damage(ch,ch,number_range(level,level*1.5),sn,DAM_FIRE,true);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,9,APPLY_SAVES,-6,0);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,9,APPLY_HITROLL,10,0);
	affect_set(victim,TO_RESIST,sn,level,slvl,9,APPLY_NONE,0,RS_MENTAL);

    act("$n's skin becomes deeply scarred in strange dark patterns.",victim,NULL,NULL,TO_ROOM);
    send_to_char("Your skin scars deeply with necromatic symbols.\n\r",victim);
    return;
}

void spell_bonearmor(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *oldobj = (OBJ_DATA *)vo, *newobj;
	AFFECT_DATA *paf;

	if(oldobj->item_type != ITEM_RIBS){
		send_to_char("You may only cast this spell on a ribcage.\n\r",ch);
		return;
	}

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

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_DAMROLL,ch->level/10,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_HITROLL,ch->level/10,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_SAVES,-ch->level/10,0);

	obj_to_char(newobj,ch);
	wear_obj(ch,newobj,true,false);

	act("$n pulls $p onto $s chest.",ch,newobj,NULL,TO_ROOM);
	act("You turn a set of ribs into $p and wear it.",ch,newobj,NULL,TO_CHAR);
	extract_obj(oldobj);
}

void spell_demonscythe(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    OBJ_DATA *newobj;
    AFFECT_DATA *paf;

	if (calcHands(ch) > 1 || calcHands(ch) < 0)
    {
        send_to_char("You must free up one of your hands first, genius.\n\r",ch);
        return;
    }

	if(!remove_obj(ch,WEAR_WIELD,true))
	{
		send_to_char("You can't remove your weapon.\n\r",ch);
		return;
	}

	newobj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_SCYTHE),ch->level);
	newobj->timer = 20;

	free_string(newobj->short_descr);
	newobj->short_descr = str_dup("a {rdemon's{x scythe");

	free_string(newobj->name);
	newobj->name = str_dup("demon scythe");

	free_string(newobj->description);
	newobj->description = str_dup("An evil scythe, forged of hardened flesh and molten rock burns with an evil aura here.\n\r");
	newobj->level = ch->level;

	newobj->value[1] = ch->level/8 + ch->getslvl(sn) + 1;
	newobj->value[2] = ch->level/9 + ch->getslvl(sn) + 1;

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_DAMROLL,ch->level/10,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_HITROLL,ch->level/10,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_SAVES,-ch->level/12,0);

	SET_WFLAG(newobj,WPN_DEMONIC);
	act("$n reaches down and pulls $p from the ground and wields it.",ch,newobj,NULL,TO_ROOM);
	act("You reach down and pull $p from the ground and wield it.",ch,newobj,NULL,TO_CHAR);

	obj_to_char(newobj,ch);
    wear_obj(ch,newobj,true,false);
}

void spell_boneshield(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    OBJ_DATA *oldobj = (OBJ_DATA *) vo, *newobj;
    AFFECT_DATA *paf;

	if(oldobj->item_type != ITEM_BONES)
	{
		send_to_char("You may only cast this spell on a set of bones.\n\r",ch);
		return;
	}

	if (calcHands(ch) > 1 || calcHands(ch) < 0)
    {
        send_to_char("You must free up one of your hands first, genius.\n\r",ch);
        return;
    }
	if (get_eq_char(ch,WEAR_WIELD))
		if (IS_WEAPON_STAT(get_eq_char(ch,WEAR_WIELD),WPN_TWO_HANDS))
		{
			send_to_char("You must free up one of your hands first, genius.\n\r",ch);
			return;
		}

	act("$n turns $p into a solid shield.",ch,oldobj,NULL,TO_ROOM);
	act("You turn $p into a solid shield.",ch,oldobj,NULL,TO_CHAR);

	newobj = create_object(get_obj_index(OBJ_VNUM_BONESHIELD),ch->level);
	newobj->timer = 5;
	newobj->level = ch->level;
	newobj->value[0] = ch->level/11;
	newobj->value[1] = ch->level/11;
	newobj->value[2] = ch->level/11;
	newobj->value[3] = ch->level/9;

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_DAMROLL,ch->level/20,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_HITROLL,ch->level/20,0);

	affect_set(newobj,TO_OBJECT,sn,level,slvl,-1,APPLY_SAVES,-ch->level/10,0);

	obj_to_char(newobj,ch);
    wear_obj(ch,newobj,true,false);

	act("$n grasps $p as a shield.",ch,newobj,NULL,TO_ROOM);
	act("You grasp $p as your shield.",ch,newobj,NULL,TO_CHAR);
	extract_obj(oldobj);
}

void spell_shield_of_shadows(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    OBJ_DATA *wield, *shield;

	wield = get_eq_char(ch,WEAR_WIELD);

	if (calcHands(ch) > 1 || calcHands(ch) < 0)
    {
        send_to_char("You must free up one of your hands first, genius.\n\r",ch);
        return;
    }
	if (wield != NULL)
		if (IS_WEAPON_STAT(get_eq_char(ch,WEAR_WIELD),WPN_TWO_HANDS))
		{
			send_to_char("You must free up one of your hands first, genius.\n\r",ch);
			return;
		}

	if(get_eq_char(ch,WEAR_SHIELD) != NULL)
	{
		send_to_char("You're crazy, no two shields!\n\r",ch);
		return;
	}

	if (get_eq_char(ch,WEAR_SHIELD) != NULL)
	{
		/*OBJ_DATA *obj;
		obj = get_eq_char(ch,WEAR_MAGESIELD);
		extract_obj(obj);

		act("$p shimmers into existence and slowly becomes {dshadow{x.",ch,obj,NULL,TO_ROOM);
		act("$p shimmers and becomes {dshadow{x.",ch,obj,NULL,TO_CHAR);

		shield = create_object(get_obj_index(OBJ_VNUM_LIGHTNINGSHIELD),ch->level);
		shield->timer = 5;
		shield->value[0] = ch->level/10;
		shield->value[1] = ch->level/10;
		shield->value[2] = ch->level/10;
		shield->value[3] = ch->level/10;
        obj_to_char( shield, ch );
        equip_char(ch,shield,WEAR_SHIELD);*/
	}
	else
	{
		shield = create_object(get_obj_index(OBJ_VNUM_LIGHTNINGSHIELD),ch->level);
		shield->timer = 5;
		shield->value[0] = ch->level/10;
		shield->value[1] = ch->level/10;
		shield->value[2] = ch->level/10;
		shield->value[3] = ch->level/10;
        
		obj_to_char(shield,ch);
        equip_char(ch,shield,WEAR_SHIELD);

		act("$p engulfs $n's arm.",ch,shield,NULL,TO_ROOM);
		act("You draw upon the shadows about you and $p engulfs your arm.",ch,shield,NULL,TO_CHAR);
	}
}

void spell_chill_touch(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = number_range(level,level*1.2);

    if (!saves_spell(level,victim,DAM_COLD))
    {
		dam *= 1.5;
		act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
		send_to_char("You turn blue and shiver.\n\r",victim);
		affect_join(victim,TO_AFFECTS,sn,level,slvl,6,APPLY_STR,-1,0);
	}

    spell_damage(ch,victim,dam,sn,DAM_COLD,true);
    return;
}

void spell_bloodhound(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = number_range(level/2,level*1.5);

	act("You call a huge bloodhound and it tears at $N!",ch,NULL,victim,TO_CHAR);
	act("$n call a huge bloodhound and it tears at you!",ch,NULL,victim,TO_VICT);
	act("$n calls a huge bloodhound and it tears at $N!",ch,NULL,victim,TO_NOTVICT);
    if (!saves_spell(level,victim,DAM_NEGATIVE))
    {
		dam *= 3;
		act("$n begins bleeding profusely.",victim,NULL,NULL,TO_ROOM);
		send_to_char("Your blood begins spurting freely from your body.\n\r",victim);
		affect_join(victim,TO_AFFECTS,gsn_rupture,level,slvl,6,APPLY_STR,-1,AF_RUPTURE);
    }

    spell_damage(ch,victim,dam,sn,DAM_NEGATIVE,true);
    return;
}

void spell_ghoul_touch(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = number_range(level*.7,level * 1.5);

    if (!saves_spell(level,victim,DAM_DISEASE))
    {
		act("$n turns green and slows down.",victim,NULL,NULL,TO_ROOM);
		send_to_char("You turn green and your muscles stiffen.\n\r",victim);
		affect_join(victim,TO_AFFECTS,sn,level,slvl,6,APPLY_AGI,-3,0);
    }

    spell_damage(ch,victim,dam,sn,DAM_DISEASE,true);
}

void spell_dark_glare(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam;

	dam = number_range(level,level*2.5);

	if (!saves_spell(level,victim,DAM_MENTAL)){
		act("$n freezes.",victim,NULL,NULL,TO_ROOM);
		send_to_char("You feel your body lock up.\n\r",victim);
		affect_join(victim,TO_AFFECTS,sn,level,slvl,6,APPLY_AGI,-2,0);
		DAZE_STATE(victim,2 *PULSE_VIOLENCE);
		dam *= 1.5;
	}
	spell_damage(ch,victim,dam,sn,DAM_MENTAL,true);
}

void spell_blood_clot(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = number_range(level/2,level*1.5);

    if (!saves_spell(level*1.2,victim,DAM_NEGATIVE))
    {
		act("$n screams in agony.",victim,NULL,NULL,TO_ROOM);
		send_to_char("You feel an excruciating pressure in your veins!\n\r",victim);
		affect_join(victim,TO_AFFECTS,sn,level,slvl,2,APPLY_STR,-2,AF_PREVENTHEAL);
		DAZE_STATE(victim,2*PULSE_VIOLENCE);
		dam *= 1.5;
    }

    spell_damage(victim,victim,dam,sn,DAM_NEGATIVE,true);
    return;
}

void spell_shadowfiend(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = number_range(level,level*3);

	act("$n summons a dark shadow fiend and sends it screaming at $N!",ch,NULL,victim,TO_NOTVICT);
	act("$n summons a dark shadow fiend and sends it screaming right at YOU!",ch,NULL,victim,TO_VICT);
	act("You summon a dark shadow fiend and send it screaming at $N!",ch,NULL,victim,TO_CHAR);

    if (!saves_spell(level*1.2,victim,DAM_NEGATIVE))
    {
		affect_join(victim,TO_AFFECTS,sn,victim->level,slvl,victim->level/4,APPLY_INT,-2,AF_AGUE);

		send_to_char("Your mind burns a moment before returning to normal...?\n\r",victim);
		act("$n twitches violently for a moment.",victim,NULL,NULL,TO_ROOM);
		dam *= 1.5;
    }

    spell_damage(ch,victim,dam,sn,DAM_NEGATIVE,true);
    return;
}

void spell_claw_of_shade(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    spell_damage(ch,victim,dice(slvl * 3,slvl * 3) + level,sn,DAM_NEGATIVE,true);
    return;
}

void spell_blackstake(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = number_range(level,slvl * 2);

    spell_damage(ch,victim,dam,sn,DAM_PIERCE,true);
	if(slvl > 3){
		victim->hit -= dam/2;
		act("A dark, shadowy stake draws life from $n.",victim,NULL,NULL,TO_ROOM);
		act("You feel your life draining away.",victim,NULL,NULL,TO_CHAR);
		ch->hit += dam/2;
	}
}

void spell_unholygate(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *vch, *nvir;
	AFFECT_DATA af;
    DESCRIPTOR_DATA *d;

	if(is_safe(ch,ch)){
		send_to_char("You can't cast that here!",ch);
		return;
	}

    send_to_char("You create a massive gate to an unholy realm and evil shadows pour out of it!\n\r",ch);
    act("$n creates a massive gate to the shadow world and evil shadows pour out!",ch,NULL,NULL,TO_ROOM);

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir){
		nvir = vch->next_in_room;
		if (is_same_group(vch,ch) || vch == ch)
			continue;
		if (get_leader(vch) != ch && !saves_spell(level*2,vch,DAM_NEGATIVE)){
			send_to_char("An evil shadow strikes you!!\n\r",vch);
			if(!saves_spell(level + slvl,vch,DAM_NEGATIVE)){
				act("$n is struck by a shadowy grasp.",vch,NULL,NULL,TO_ROOM);
				send_to_char("You feel a shadow grasp on your soul.\n\r",vch);

				affect_set(vch,TO_AFFECTS,sn,level,slvl,6,APPLY_STR,-3,AF_CURSE);
			}
			spell_damage(ch,vch,level + dice(slvl,slvl * 2),sn,DAM_NEGATIVE,true);
		}
	}
	for ( d = descriptor_list; d != NULL; d = d->next ){
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;
		if (d->connected == CON_PLAYING && d->character != ch && d->character->in_room->area == ch->in_room->area && d->character->in_room != ch->in_room )
			send_to_char("There is a deep disturbance somewhere in the world.\n\r",d->character);
	}
    return;
}

void spell_awaken_blood(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level*2,victim,DAM_MENTAL) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)))
    {
		if (ch == victim)
			send_to_char("Your blood bubbles.\n\r",ch);
		else
		{
			send_to_char("You feel your blood bubble.\n\r",victim);
			act("$N groans in pain.",ch,NULL,victim,TO_CHAR);
		}
		return;
    }

    send_to_char("You feel a blinding pain as your blood surges in an effort to escape your body!\n\r",victim);
    act("$n screams in agony!", victim,NULL,NULL,TO_ROOM);
	spell_damage(ch,victim,number_range(level*.5,level*4),sn,DAM_MENTAL,true);

	if(!saves_spell(level,victim,DAM_MENTAL)){
		affect_join(victim,TO_AFFECTS,sn,victim->level,slvl,6,APPLY_STR,-2,AF_WEAKEN);
		send_to_char("You crumple weakly in pain!\n\r",victim);
		act("$n crumples in pain!", victim,NULL,NULL,TO_ROOM);
	}
}

void spell_bonestorm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj,*obj_next;
	int i,dam;
	bool found;

		act("$n fires a volley of glowing bones at $N!",ch,NULL,victim,TO_NOTVICT);
		act("$n fires a volley of glowing bones at you!",ch,NULL,victim,TO_VICT);
		act("You fire a volley of glowing bone at $N!",ch,NULL,victim,TO_CHAR);
	for(i=1; i < 4; i++)
	{
		found = false;
		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
			obj_next = obj->next_content;

			if (obj->item_type == ITEM_BONES && can_see_obj(ch,obj) && can_drop_obj(ch,obj) )
			{
				found = true;
				obj_from_char(obj);
				extract_obj(obj);
				break;
			}
		}

		if (!found)
		{
			send_to_char("Out of bones!\n\r",ch);
			return;
		}

		dam = number_range(level,level * 2);
		if ( !saves_spell(level,victim,DAM_PIERCE) )
			dam *= 2;
		spell_damage(ch,victim,dam,sn,DAM_PIERCE,true);
	}
    return;
}

void spell_extraction(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = number_range(level*.75,level);
	if ( !saves_spell(level*2,victim,DAM_PIERCE) )
	{
		act("You feel your life draining from your body.",ch,NULL,victim,TO_VICT);
		act("You drain $N of $S life energy.",ch,NULL,victim,TO_CHAR);
		ch->hit += dam;
		dam *= 2;
	}
	spell_damage(ch,victim,dam,sn,DAM_NEGATIVE,true);
    return;
}

void spell_putrid_explosion(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *vch, *nvir;
    DESCRIPTOR_DATA *d;
    OBJ_DATA *obj,*obj_next;
	int chance,dam;
	bool found = false;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if(obj->item_type == ITEM_CORPSE_NPC && !obj->contains)
		{
			found = true;
			break;
		}
	}

	if(!found)
	{
		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			if(obj->item_type == ITEM_CORPSE_NPC && !obj->contains)
			{
				found = true;
				break;
			}
		}
	}

    if (!found)
    {
		send_to_char("There are no NPC corpses in the room or your inventory.\n\r",ch);
		return;
    }

	chance = 35;
	chance += URANGE(0,get_curr_stat(ch,STAT_INT)/2,number_range(1,ch->level/4));

	if(number_percent() <= chance)
	{
		act("$p bulges from the mana fed into it and explodes!",ch,obj,NULL,TO_ROOM);
		act("$p bulges from the mana fed into it and explodes!",ch,obj,NULL,TO_CHAR);
		extract_obj(obj);
		dam = number_range(level,level*2);
		for ( vch = ch->in_room->people; vch != NULL; vch = nvir)
		{
			nvir = vch->next_in_room;
			if (is_same_group(vch,ch) || vch == ch)
				continue;
			if (saves_spell(level*2,vch,DAM_NEGATIVE))
				spell_damage(ch,vch,dam,sn,DAM_NEGATIVE,true);
			else
			{
				send_to_char("The exploding corpse hurts!!\n\r",vch);
				spell_damage(ch,vch,2*dam,sn,DAM_NEGATIVE,true);
			}
		}
		for ( d = descriptor_list; d; d = d->next )
			if ( d->character->in_room->area == ch->in_room->area && d->character->in_room != ch->in_room)
				send_to_char("The smell of rotting flesh flares your nostrils.\n\r",d->character);
		return;
	}
	else
	{
		if(number_percent() < 50)
		{
			act("$p bulges from the mana fed into it... and explodes in $n's face!",ch,obj,NULL,TO_ROOM);
			act("$p bulges from the mana fed into it... and explodes in your face!",ch,obj,NULL,TO_CHAR);
			extract_obj(obj);
			damage(ch,ch,number_range(level,level*2),sn,DAM_NEGATIVE,true);
		}
		else
			send_to_char("You fail!",ch);
		return;
	}
}

void spell_degeneration(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim,DAM_DISEASE) || (IS_NPC(victim) && victim->isact(AT_UNDEAD))){
		if (ch == victim)
			send_to_char("Your body stings a moment.\n\r",ch);
		else{
			send_to_char("Your body stings a moment.\n\r",victim);
			act("$N seems to be momentarily unaffected.",ch,NULL,victim,TO_CHAR);
		}
		return;
    }

	affect_join(victim,TO_AFFECTS,sn,victim->level,slvl,5,APPLY_STR,-3,AF_DEGENERATION);

    send_to_char("Your body suddenly screams in pain!\n\r",victim);
    act("$n screams loudly and falls over in pain!",victim,NULL,NULL,TO_ROOM);
}

void spell_agues_echo(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level*1.6,victim,DAM_MENTAL) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)))
    {
		if (ch == victim)
			send_to_char("Your mind tingles a bit.\n\r",ch);
		else
		{
			send_to_char("Your mind tingles a bit.\n\r",victim);
			act("$N seems to be momentarily unaffected.",ch,NULL,victim,TO_CHAR);
		}
		return;
    }

	affect_join(victim,TO_AFFECTS,sn,victim->level,slvl,victim->level/4,APPLY_INT,-2,AF_AGUE);

    send_to_char("Your mind burns a moment before returning to normal...?\n\r",victim);
    act("$n twitches violently for a moment.",victim,NULL,NULL,TO_ROOM);
}

void do_embalm(CHAR_DATA *ch,char *argument){
    OBJ_DATA *corpse;
    AFFECT_DATA af;

	if(!(corpse = get_obj_carry(ch,argument,ch))){
		ch->send("You aren't carrying that.\n\r");
		return;
	}
	if(get_skill(ch,gsn_embalm) < 1){
		ch->send("You don't know how to do that.\n\r");
		return;
	}
	if(corpse->item_type != ITEM_CORPSE_NPC){
		send_to_char("You may only cast this spell on an NPC corpse.\n\r",ch);
		return;
	}
	if(!roll_chance(ch,get_skill(ch,gsn_embalm))){
		ch->send("You fail.\n\r");
		WAIT_STATE(ch,skill_table[gsn_embalm].beats);
		return;
	}

	act("$n encases $p in an embalming magic.",ch,corpse,NULL,TO_ROOM);
	act("You encase $p in an embalming magic.",ch,corpse,NULL,TO_CHAR);

	affect_set(corpse,TO_OBJECT,gsn_embalm,ch->level,ch->getslvl(gsn_embalm),UMAX(1,ch->getslvl(gsn_embalm)) * 120,APPLY_NONE,0,ITM_EMBALM);
	WAIT_STATE(ch,skill_table[gsn_embalm].beats);
}

void spell_guarding_spirit(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if ( is_affected(ch,sn) )
	{
		if (victim == ch)
			send_to_char("You're already guarded.\n\r",ch);
		else
			act("$N is already guarded.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_AC,-40,AF_GUARDIAN);
    act("$n is wrapped in a guarding spirit.",victim,NULL,NULL,TO_ROOM);
    send_to_char("Your body is wrapped in a guarding spirit.\n\r",victim);
    return;
}

void do_graverob(CHAR_DATA *ch,char *argument)
{
	char arg[MSL],buf[MSL];
	int chance = 0, type = 0;
	OBJ_DATA *obj;

	one_argument(argument,arg);

	if ( arg[0] == '\0' )
    {
		send_to_char("Syntax: graverob <corpse/bones/ribcage>\n\r",ch);
		return;
    }

	if(!str_prefix(arg,"corpse"))
		type = 1;
	else if(!str_prefix(arg,"bones"))
		type = 2;
	else if(!str_prefix(arg,"ribcage"))
		type = 3;
	else
	{
		send_to_char("Syntax: graverob <corpse/bones/ribcage>\n\r",ch);
		return;
	}

	if(ch->move < 25)
	{
		send_to_char("You're too exhausted to rob graves.\n\r",ch);
		return;
	}

	if(ch->in_room->sector_type != SECT_GRAVEYARD)
	{
		send_to_char("You must be in a graveyard to graverob, duh.\n\r",ch);
		return;
	}

	chance = get_skill(ch,gsn_graverob) * .8;

	ch->move -= 25;
	WAIT_STATE(ch,skill_table[gsn_graverob].beats);
	if(number_percent() <= chance)
	{
		if(type==1)
		{
			obj = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC),0);
			obj->timer = 2;
			obj->level = ch->level;

			sprintf(buf,"a rotting corpse");
			free_string(obj->short_descr);
			obj->short_descr = str_dup(buf);
			sprintf(buf,"A nondescript corpse is here.");
			free_string(obj->description);
			obj->description = str_dup(buf);
			obj_to_char(obj,ch);
			send_to_char("You dig around and manage to dig up a minimally decomposed corpse.\n\r",ch);
			act("$n digs up a fresh corpse...",ch,NULL,NULL,TO_ROOM);
		}
		else if(type==2)
		{
			obj = create_object(get_obj_index(OBJ_VNUM_BONES),0);
			obj->timer = 100;
			obj->level = ch->level;
			obj_to_char(obj,ch);
			send_to_char("You dig around and manage to dig up a pile of bones.\n\r",ch);
			act("$n digs up a pile of bones...",ch,NULL,NULL,TO_ROOM);
		}
		else if(type == 3)
		{
			obj = create_object(get_obj_index(OBJ_VNUM_RIBS),0);
			obj->timer = 100;
			obj->level = ch->level;
			obj_to_char(obj,ch);
			send_to_char("You dig around and manage to dig up a ribcage.\n\r",ch);
			act("$n digs up a ribcage...",ch,NULL,NULL,TO_ROOM);
		}
		else
		{
			send_to_char("BUG! ALERT!",ch);
			return;
		}
		check_improve(ch,gsn_graverob,true,4);
		return;
	}
	else
	{
		send_to_char("You dig around and fail to find anything of use.\n\r",ch);
		act("$n digs around a bit before getting all out of breath and stuff.",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,gsn_graverob,false,2);
		return;
	}
}

void spell_nausea(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (saves_spell(level,victim,DAM_MENTAL) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)))
    {
		if (ch == victim)
			send_to_char("Your mind tingles a bit.\n\r",ch);
		else
		{
			send_to_char("Your mind tingles a bit.\n\r",victim);
			act("$N seems to be momentarily unaffected.",ch,NULL,victim,TO_CHAR);
		}
		return;
    }

	affect_set(victim,TO_AFFECTS,gsn_nausea,victim->level,slvl,5,APPLY_WIS,-2,AF_CONFUSION);

    send_to_char("You feel woozy.\n\r",victim);
    act("$n sways a moment.",victim,NULL,NULL,TO_ROOM);
}

void spell_asphyxiation(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if ( victim->isaff(AF_SLEEP) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)) )
		return;

	spell_damage(ch,victim,10,sn,DAM_MENTAL,true);
	if (!saves_spell(level,victim,DAM_MENTAL))
	{
		stop_fighting(victim,true);
		act("You feel an enormous pressure in your veins and pass out!",ch,NULL,victim,TO_VICT);
		act("$N stumbles a bit and passes out!",ch,NULL,victim,TO_NOTVICT);
		act("You call pressure to $N's brain vein and $E passes out!",ch,NULL,victim,TO_CHAR);

		affect_set(victim,TO_AFFECTS,gsn_sleep,level,slvl,1,APPLY_NONE,0,AF_SLEEP);
		victim->position = POS_SLEEPING;
		return;
	}
	act("You feel a slight pressure in your veins.",ch,NULL,victim,TO_VICT);
	act("You fail to put $N to sleep.",ch,NULL,victim,TO_CHAR);
}

void spell_darklegion(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
	CHAR_DATA *pet;
    AFFECT_DATA af;
	int i;

	if(is_affected(ch,sn))
	{
		send_to_char("You're too weary to do that!\n\r",ch);
		return;
	}
	act("$n summons an army from the pits of hell!",ch,NULL,NULL,TO_ROOM);
	act("You summon an army from the pits of hell!",ch,NULL,NULL,TO_CHAR);
	for(i=1; i < ch->level/10; i++)
	{
		pet	= create_mobile(get_mob_index(1));
		pet->level = ch->level;
		pet->max_hit = ch->level;
		pet->hit = pet->max_hit;
		pet->hitroll = 200;
		pet->setact(AT_AGGRESSIVE);
		//pet->setact(AT_AGGRESSIVE_MOB);

		free_string(pet->short_descr);
		pet->short_descr = str_dup("a nasty demon");

		free_string(pet->name);
		pet->name = str_dup("nasty demon");

		free_string(pet->long_descr);
		pet->long_descr = str_dup("A nasty demon marches about here, looking for a victim.\n\r");

		free_string(pet->description);
		pet->description = str_dup("This nasty demon marches about looking for something to kill.\n\r");
		char_to_room( pet, ch->in_room );
	}
	affect_set(ch,TO_AFFECTS,sn,level,slvl,24,APPLY_NONE,0,AF_WEARY);
}

void spell_summonspirit(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *pet;
	AFFECT_DATA af;
	int i,tries,thp;
	bool found = false;

	if(is_affected(ch,sn)){
		send_to_char("You're too weary to do that!\n\r",ch);
		return;
	}

	if(ch->pet != NULL){
		send_to_char("You already have a pet!\n\r",ch);
		return;
	}

	act("You attempt to summon a spirit.",ch,NULL,NULL,TO_CHAR);

	for(tries = 0; tries < MAX_NSPIRITS; ++tries){
		i = number_range(0,MAX_NSPIRITS-1);
		if(nspirit_table[i].name != NULL && nspirit_table[i].min_level <= ch->level);{
			found = true;
			break;
		}
	}
	if(!found || nspirit_table[i].name == NULL){
		send_to_char("You failed!\n\r",ch);
		return;
	}

	pet							= create_mobile(get_mob_index(1));
	pet->level					= ch->level;
	pet->pclass					= nspirit_table[i].pclass;
	pet->race					= race_lookup(nspirit_table[i].race);

	thp							= (slvl * 50 * nspirit_table[i].hpmult) + 1000;
	pet->max_hit				= number_range(thp * .75, thp * 1.25);
	pet->hit					= pet->max_hit;
	pet->hitroll				= ch->level * 1.5;

	pet->damage[DICE_NUMBER]	= pet->level / 4;
	pet->damage[DICE_TYPE]		= slvl + 2;
	pet->damage[DICE_BONUS]		= slvl * 5;
	pet->armor[AC_PIERCE]		= 0 - (pet->level *2);
	pet->armor[AC_BASH]			= 0 - (pet->level *2);
	pet->armor[AC_SLASH]		= 0 - (pet->level *2);
	pet->armor[AC_EXOTIC]		= 0 - (pet->level *3);

	pet->setmaxmana(dice((pet->level / 2)+1,pet->level/2) + (pet->level));
	pet->setmana(pet->getmaxmana());

	if(pet->short_descr != NULL)
		free_string(pet->short_descr);
	pet->short_descr			= str_dup(nspirit_table[i].short_descr);
	if(pet->name != NULL)
		free_string(pet->name);
	pet->name					= str_dup(nspirit_table[i].name);
	if(pet->long_descr != NULL)
		free_string(pet->long_descr);
	pet->long_descr				= str_dup(nspirit_table[i].long_descr);
	if(pet->description != NULL)
		free_string(pet->description);
	pet->description			= str_dup(nspirit_table[i].description);
	pet->setform(FRM_INSTANT_DECAY);
	pet->setform(FRM_SILENT);
	pet->setoff(OF_FAST);
	pet->setdef(DF_DODGE);

	init_familiar(ch,pet);

	//weary
	affect_set(ch,TO_AFFECTS,sn,level,slvl,12,APPLY_NONE,0,AF_WEARY);

	act("$N fades into existence and follows $n.",ch,NULL,pet,TO_ROOM);
	act("You summon $N as your servant.",ch,NULL,pet,TO_CHAR);
}

void spell_animate_corpse(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	OBJ_DATA *corpse = (OBJ_DATA*) vo;
	CHAR_DATA *pet;
	AFFECT_DATA af;
	int i,tries,thp;
	bool found = false;

	if(is_affected(ch,sn)){
		send_to_char("You're too weary to do that!\n\r",ch);
		return;
	}

	if(ch->in_room->sector_type != SECT_GRAVEYARD){
		send_to_char("You must be in a graveyard to mess with the dead!\n\r",ch);
		return;
	}

	if(corpse->item_type != ITEM_CORPSE_NPC || corpse->contains){
		send_to_char("You may only re-animate empty NPC corpses.\n\r",ch);
		return;
	}

	if(ch->pet != NULL){
		send_to_char("You already have a pet!\n\r",ch);
		return;
	}

	act("You attempt to re-animate $p.",ch,corpse,NULL,TO_CHAR);
	extract_obj(corpse);

	for(tries = 0; tries < MAX_NUNDEADS; ++tries){
		i = number_range(0,MAX_NUNDEADS-1);
		if(nundead_table[i].name != NULL && nundead_table[i].min_level <= ch->level);{
			found = true;
			break;
		}
	}
	if(!found || nundead_table[i].name == NULL){
		send_to_char("You failed!\n\r",ch);
		return;
	}

	pet							= create_mobile(get_mob_index(1));
	pet->level					= ch->level;
	pet->pclass					= nundead_table[i].pclass;
	pet->race					= race_lookup(nundead_table[i].race);

	thp							= (slvl * 50 * nundead_table[i].hpmult) + 1000;;
	pet->max_hit				= number_range(thp * .75, thp * 1.25);
	pet->hit					= pet->max_hit;
	pet->hitroll				= ch->level * 1.5;

	pet->damage[DICE_NUMBER]	= pet->level / 8;
	pet->damage[DICE_TYPE]		= slvl;
	pet->damage[DICE_BONUS]		= slvl * 2;
	pet->armor[AC_PIERCE]		= 0 - (pet->level *2);
	pet->armor[AC_BASH]			= 0 - (pet->level *2);
	pet->armor[AC_SLASH]		= 0 - (pet->level *2);
	pet->armor[AC_EXOTIC]		= 0 - (pet->level *3);

	pet->setmaxmana(dice((pet->level / 2)+1,pet->level/2) + (pet->level));
	pet->setmana(pet->getmaxmana());

	if(pet->short_descr != NULL)
		free_string(pet->short_descr);
	pet->short_descr			= str_dup(nundead_table[i].short_descr);
	if(pet->name != NULL)
		free_string(pet->name);
	pet->name					= str_dup(nundead_table[i].name);
	if(pet->long_descr != NULL)
		free_string(pet->long_descr);
	pet->long_descr				= str_dup(nundead_table[i].long_descr);
	if(pet->description != NULL)
		free_string(pet->description);
	pet->description			= str_dup(nundead_table[i].description);
	pet->setform(FRM_INSTANT_DECAY);

	init_familiar(ch,pet);
	pet->setdef(DF_ABSORB);

	//weary
	affect_set(ch,TO_AFFECTS,sn,level,slvl,35,APPLY_NONE,0,AF_WEARY);

	act("$N rises from the grave and follows $n.",ch,NULL,pet,TO_ROOM);
	act("You raise $N from the dead as your slave.",ch,NULL,pet,TO_CHAR);
}

void spell_calldemon(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *pet;
	AFFECT_DATA af;
	int i,tries,thp;
	bool found = false;

	if(is_affected(ch,sn)){
		send_to_char("You're too weary to do that!\n\r",ch);
		return;
	}

	if(ch->pet != NULL){
		send_to_char("You already have a pet!\n\r",ch);
		return;
	}

	act("You attempt to call a demon.",ch,NULL,NULL,TO_CHAR);

	for(tries = 0; tries < MAX_NDEMONS; ++tries){
		i = number_range(0,MAX_NDEMONS-1);
		if(ndemon_table[i].name != NULL && ndemon_table[i].min_level <= ch->level);{
			found = true;
			break;
		}
	}
	if(!found || ndemon_table[i].name == NULL){
		send_to_char("You failed!\n\r",ch);
		return;
	}

	pet							= create_mobile(get_mob_index(1));
	pet->level					= ch->level;
	pet->pclass					= ndemon_table[i].pclass;
	pet->race					= race_lookup(ndemon_table[i].race);

	thp							= (slvl * 50 * ndemon_table[i].hpmult) + 1000;
	pet->max_hit				= number_range(thp * .75, thp * 1.25);
	pet->hit					= pet->max_hit;
	pet->hitroll				= ch->level * 1.5;

	pet->damage[DICE_NUMBER]	= pet->level / 2;
	pet->damage[DICE_TYPE]		= slvl + 5;
	pet->damage[DICE_BONUS]		= slvl * 10;
	pet->armor[AC_PIERCE]		= 0 - (pet->level *2);
	pet->armor[AC_BASH]			= 0 - (pet->level *2);
	pet->armor[AC_SLASH]		= 0 - (pet->level *2);
	pet->armor[AC_EXOTIC]		= 0 - (pet->level *3);

	pet->setmaxmana(dice((pet->level / 2)+1,pet->level/2) + (pet->level));
	pet->setmana(pet->getmaxmana());

	free_string(pet->short_descr);
	pet->short_descr			= str_dup(ndemon_table[i].short_descr);
	free_string(pet->name);
	pet->name					= str_dup(ndemon_table[i].name);
	free_string(pet->long_descr);
	pet->long_descr				= str_dup(ndemon_table[i].long_descr);
	free_string(pet->description);
	pet->description			= str_dup(ndemon_table[i].description);

	pet->setform(FRM_INSTANT_DECAY);
	pet->setoff(OF_FAST);
	pet->setoff(OF_BERSERK);
	pet->setoff(OF_CRITICAL);
	pet->setoff(OF_SECONDATTACK);
	pet->setdef(DF_DODGE);
	pet->setdef(DF_FEINT);

	init_familiar(ch,pet);

	//weary
	affect_set(ch,TO_AFFECTS,sn,level,slvl,!IS_IMMORTAL(ch) ? af.duration = 172 : 0,APPLY_NONE,0,AF_WEARY);

	act("$N rises from the pits of hell to aide $n.",ch,NULL,pet,TO_ROOM);
	act("$N rises from the pits of hell to aide you as a minion.",ch,NULL,pet,TO_CHAR);
}

void spell_nether_storm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch,*vch_next;
	int n;

	act("Glowing black rain bubbles and rips from the ground into the sky!",ch,NULL,NULL,TO_ROOM);
	ch->send("You summon a nether storm the rise from the underworld!\n\r");

	for(n = 4;n>0;n--){
		for (vch = ch->in_room->people;vch;vch = vch_next){
			vch_next = vch->next_in_room;
			if (ch != vch && !vch->isaff(AF_CHARM)){
				if(saves_spell(level,ch,DAM_NEGATIVE))
					spell_damage(ch,vch,number_range(level/2,level),sn,DAM_NEGATIVE,true);
				else
					spell_damage(ch,vch,number_range(level * 1.5,ch->level),sn,DAM_NEGATIVE,true);
			}
		}
	}
}

void spell_insomnia(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    AFFECT_DATA af;

    if (victim->isaff(AF_SLEEP) || victim->isaff(AF_NARCOLEPSY) || saves_spell(level,victim,DAM_MENTAL) || (IS_NPC(victim) && victim->isact(AT_UNDEAD))){
		if (ch == victim)
			ch->send("Your mind stirs a moment.\n\r");
		else{
			victim->send("Your body stirs a moment.\n\r");
			act("$N seems to be momentarily unaffected.",ch,NULL,victim,TO_CHAR);
		}
		return;
    }

	affect_set(victim,TO_AFFECTS,sn,victim->level,slvl,5,APPLY_WIS,-3,AF_INSOMNIA);

    victim->send("Your mind stirs and rushes with a thousand thoughts!\n\r");
    act("$n looks VERY awake!",victim,NULL,NULL,TO_ROOM);
}

void spell_caustic_strike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	int hperc = ((victim->max_hit - victim->hit) * 100) / victim->max_hit,dam = 0;

	dam = number_range((victim->gettruemana() * (hperc / 4)) / 100, (victim->getmana() * (hperc / 10)) / 100);

	if (saves_spell(level,victim,DAM_ACID))
		dam /= 2;

	if (dam > 1 && victim->gettruemana() > dam){
		act("Your skin burns and you feel your energy burning off!",ch,NULL,victim,TO_VICT);
		act("$N's skin sizzles as whisps of energy seem to burn off $S body.",ch,NULL,victim,TO_NOTVICT);
		act("Your caustic strike causes $N's energy to sizzle and burn off.",ch,NULL,victim,TO_CHAR);
		victim->modtruemana(-dam);
	}

	dam = number_range((victim->gettrueantimana() * (hperc / 4)) / 100, (victim->getmana() * (hperc / 10)) / 100);

	if (saves_spell(level,victim,DAM_DISEASE))
		dam /= 2;

	if (dam > 1 && victim->gettrueantimana() > dam){
		act("Your skin burns and you feel your life force burning off!",ch,NULL,victim,TO_VICT);
		act("$N's skin sizzles as whisps of life force seem to burn off $S body.",ch,NULL,victim,TO_NOTVICT);
		act("Your caustic strike causes $N's life force to sizzle and burn off.",ch,NULL,victim,TO_CHAR);
		victim->modtrueantimana(-dam);
	}
}

void spell_narcolepsy(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
    AFFECT_DATA af;

	if (victim->isaff(AF_SLEEP) || victim->isaff(AF_INSOMNIA) || victim->isaff(AF_NARCOLEPSY) || (IS_NPC(victim) && victim->isact(AT_UNDEAD)) || saves_spell(level,victim,DAM_CHARM)){
		if (ch == victim)
			ch->send("Your mind gets sluggish... momentarily.\n\r");
		else{
			victim->send("Your mind momentarily feels sluggish.\n\r");
			act("$N seems unaffected... for now.",ch,NULL,victim,TO_CHAR);
		}
	}

	if (saves_spell(level,victim,DAM_MENTAL)){
		ch->send("You failed.\n\r");
		damage(ch,victim,0,sn,DAM_MENTAL,false);
		return;
	}

	victim->send("Your mind starts to slow down.\n\r");
	act("$n looks very sluggish in the head.",victim,NULL,NULL,TO_ROOM);

	affect_join(victim,TO_AFFECTS,gsn_narcolepsy,level,slvl,5,APPLY_WIS,-3,AF_NARCOLEPSY);
}

void spell_sinkhole(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;
	CHAR_DATA *rch,*rch_next;
    ROOM_INDEX_DATA *was_in_room,*scan_room;
	EXIT_DATA *pExit;
	int dam,bdam,door;
	bool found = false;

	bdam = number_range(ch->level * get_curr_stat(ch,STAT_FTH) / 25,ch->level);
	for (door = 0; door < 6 && !found; door++){
		scan_room = ch->in_room;
		if (pExit = scan_room->exit[door]){
			scan_room = pExit->u1.to_room;

			for(rch = scan_room->people;rch;rch = rch_next){
				rch_next = rch->next_in_room;
				if (rch != ch && !IS_SET(pExit->exit_info,EX_CLOSED) && canFight(ch,rch,false)){
					found = true;
					act("$n is seemingly sucked away!",rch,NULL,NULL,TO_ROOM);
					char_from_room(rch);
					char_to_room(rch,ch->in_room);
					act("$n is pulled in!",rch,NULL,NULL,TO_ROOM);
				}
			}
		}
	}
	if (!found)
		ch->send("There was no one in the surrounding rooms.\n\r");

	for(rch = ch->in_room->people;rch;rch = rch_next){
		rch_next = rch->next_in_room;

		if (!canFight(ch,rch,false) || rch == ch)
			continue;

		dam = bdam;
		if(saves_spell(level/5*ch->getslvl(sn),rch,DAM_NEGATIVE))
			dam /= 2;
		else{
			act("$n is trapped!",rch,NULL,NULL,TO_ROOM);
			rch->send("You are trapped!\n\r");
			affect_set(rch,TO_AFFECTS,gsn_snare,ch->level,slvl,0,APPLY_STR,-10,AF_SNARED);
		}
		spell_damage(ch,rch,dam,sn,DAM_NEGATIVE,true);
	}
	stop_fighting(ch,true);
}

void spell_death_cloak(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

	if (ch->in_room->israf(RAF_NEGATIVE)){
		ch->send("This room has been darkened already.\n\r");
		return;
	}
	act("You cloak the room with a deadly aura.",ch,NULL,NULL,TO_CHAR);
	act("$n cloaks the room in a deathly aura.",ch,NULL,NULL,TO_ROOM);

	affect_set(ch->in_room,TO_AFFECTS,sn,level,slvl,slvl * 2,APPLY_NONE,slvl,RAF_NEGATIVE);
}

void spell_nightmare(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
    AFFECT_DATA af;

	if(victim->isaff(AF_INSOMNIA) || victim->isaff(AF_NIGHTMARE) || (IS_NPC(victim) && victim->isact(AT_UNDEAD))){
		if (ch == victim)
			ch->send("Your mind gets scary... momentarily.\n\r");
		else{
			victim->send("Your mind momentarily feels scary.\n\r");
			act("$N seems unaffected... for now.",ch,NULL,victim,TO_CHAR);
		}
	}

	if(saves_spell(level,victim,DAM_MENTAL)){
		ch->send("You failed.\n\r");
		damage(ch,victim,0,sn,DAM_MENTAL,false);
		return;
	}

	victim->send("Your mind starts to get scary.\n\r");
	act("$n looks very scared in the head.",victim,NULL,NULL,TO_ROOM);

	affect_join(victim,TO_AFFECTS,gsn_nightmare,level,slvl,UMAX(1,slvl) * 10,APPLY_WIS,-slvl,AF_NIGHTMARE);
}

void spell_catalepsy(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
    AFFECT_DATA af;

	if(victim->isaff(AF_CATALEPSY)){
		if (ch == victim)
			ch->send("Your body twitches... momentarily.\n\r");
		else{
			victim->send("Your body feels scary.\n\r");
			act("$N seems unaffected... for now.",ch,NULL,victim,TO_CHAR);
		}
	}

	if(saves_spell(level,victim,DAM_MENTAL)){
		ch->send("You failed.\n\r");
		damage(ch,victim,0,sn,DAM_MENTAL,false);
		return;
	}

	victim->send("Your mind starts to get scary.\n\r");
	act("$n looks very scared in the head.",victim,NULL,NULL,TO_ROOM);

	affect_join(victim,TO_AFFECTS,gsn_catalepsy,level,slvl,UMAX(1,slvl)*10,APPLY_AGI,-slvl,AF_CATALEPSY);
}

void spell_extinguish(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch;
	OBJ_DATA *light;
	int total = ch->getslvl(sn);

	for(vch = ch->in_room->people;vch;vch = vch->next_in_room){
		if(total == 0)
			break;
		if(total != 5)
			total--;
		if(!(light = get_eq_char(ch,WEAR_LIGHT)))
			continue;
		if(light->value[2] == -1)
			continue;
		light->item_type = ITEM_TRASH;
		act("$p flickers and dims before going dark.",NULL,light,NULL,TO_ROOM);
	}
}

void spell_decay(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam;

	dam = number_range(level,level*1.2);

	if(!victim->isaff(AF_POISON) && !victim->isaff(AF_PLAGUE)){
		ch->send("They are not ill enough to rot their body.\n\r");
		return;
	}
	if(victim->isaff(AF_POISON))
		dam *= ch->getslvl(sn);
	if(victim->isaff(AF_PLAGUE))
		dam *= ch->getslvl(sn);

	if(victim->isaff(AF_POISON)){
		if(!saves_spell(25 * ch->getslvl(sn),victim,DAM_POISON)){
			act("$n's veins turn purple and $s face tinges gray.",victim,NULL,NULL,TO_ROOM);
			send_to_char("Your stomach churns and feel very sick.\n\r",victim);
			affect_join(victim,TO_AFFECTS,skill_lookup("poison"),level*slvl/5,slvl,UMAX(1,slvl)*2,APPLY_STR,-1,AF_POISON);
			spell_damage(ch,victim,dam*2,sn,DAM_POISON,true);
		}
		else
			spell_damage(ch,victim,dam,sn,DAM_POISON,true);
	}
	if(victim->isaff(AF_PLAGUE)){
		if(!saves_spell(level * ch->getslvl(sn),victim,DAM_POISON)){
			act("$n screams as $s plague sores boil and fester.",victim,NULL,NULL,TO_ROOM);
			send_to_char("Your plague sores boil and fester.\n\r",victim);
			affect_join(victim,TO_AFFECTS,gsn_plague,level * slvl / 5,slvl,UMAX(1,slvl)*2,APPLY_STR,-1,AF_POISON);
			spell_damage(ch,victim,dam * 2,sn,DAM_DISEASE,true);
		}
		else
			spell_damage(ch,victim,dam,sn,DAM_DISEASE,true);
	}
}

void spell_necrosis(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;

	if(saves_spell(level + (level * ch->getslvl(sn) / 5),victim,DAM_DISEASE)){
		victim->send("You feel your body sicken momentarily.\n\r");
		act("$n looks a little sick but returns to normal.",victim,NULL,NULL,TO_ROOM);
		return;
	}
	switch(number_range(1,5)){
	case 1:
		victim->send("Your hand withers and weakens!\n\r");
		act("$n's hand withers and weakens!",victim,NULL,NULL,TO_ROOM);
		break;
	case 2:
		victim->send("Your arm shrivels and turns purple!\n\r");
		act("$n's arm shrivels and turns purple!",victim,NULL,NULL,TO_ROOM);
		break;
	case 3:
		victim->send("Your leg shrivels and your veins bulge!\n\r");
		act("$n's leg shrivels and $s veins bulge!",victim,NULL,NULL,TO_ROOM);
		break;
	case 4:
		victim->send("Your vision blurs and dims!\n\r");
		act("$n's eyes shrivel and look like leather!",victim,NULL,NULL,TO_ROOM);
		break;
	case 5:
		victim->send("Your hand shrivels and turns purple!\n\r");
		act("$n's hand shrivels and turns purple!",victim,NULL,NULL,TO_ROOM);
		break;
	}
}

void crunch_introversion(CHAR_DATA *ch,int dam,bool receiving){
	int sn = gsn_introversion,skill = get_skill(ch,sn);

	if(receiving){
		check_improve(ch,sn,false,1);
		return;
	}
	check_improve(ch,sn,false,1);
}

void spell_ravage(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA*)vo;
	AFFECT_DATA af;

	if(saves_spell(level + (level*ch->getslvl(sn)/5),victim,DAM_NEGATIVE)){
		victim->send("You feel a negative energy momentarily surround you.\n\r");
		act("$n is momentarily surrounded by negative energy.",victim,NULL,NULL,TO_ROOM);
		return;
	}

	affect_join(victim,TO_AFFECTS,sn,level,slvl,UMAX(1,slvl),APPLY_WIS,-slvl,AF_RAVAGE);
	send_to_char("Your body is engulfed in negative energy and you are blinded by pain!\n\r",victim);
	act("$n is engulfed in negative energy and screams in pain!",victim,NULL,NULL,TO_ROOM);
}

void spell_defile(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
}
