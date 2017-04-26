#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "lookup.h"
#include "recycle.h"

/*
 * Local functions.
 */
bool saves_dispel		( int,int,int );
bool check_poverty		( CHAR_DATA* );
int check_purity		( CHAR_DATA*,int );
void check_reverence	( CHAR_DATA*,int );
void raw_kill			( CHAR_DATA*,CHAR_DATA* );
int crunch_devotion	( CHAR_DATA*,CHAR_DATA*,int );

void heal_them(CHAR_DATA *ch,CHAR_DATA *victim,int sn,int heal){
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"healBase %d\n\r",heal);
	if (victim->position == POS_MORTAL){
		ch->send("They are too far gone for that.\n\r");
		return;
	}

	if(check_poverty(ch))
		heal *= 1.5;
	if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"healPov %d\n\r",heal);

	if (ch != victim){
		act("Your prayers are answered and $N is enveloped in a white glow.",ch,NULL,victim,TO_CHAR);
		act("$n's prayers briefly envelope $N in a white glow.",ch,NULL,victim,TO_NOTVICT);
		act("$n's prayers bring warmth to your body.",ch,NULL,victim,TO_VICT);
	}
	else{
		act("Your prayers are answered and warmth fills your body.",ch,NULL,NULL,TO_CHAR);
		act("$n's prayers are answered and $e glows briefly.",ch,NULL,NULL,TO_ROOM);
	}
	heal = UMAX(1,heal);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"healMax %d\n\r",heal);

	//heal += heal * get_curr_stat(ch,STAT_FTH) / (STAT_MAX);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"healFaith %d\n\r",heal);

	heal = crunch_devotion(ch,victim,heal);
	heal = check_piety(ch,heal,PIETY_DAMAGE);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"healPiety %d\n\r",heal);

	heal = check_purity(ch,heal);if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"healPurity %d\n\r",heal);

	if (victim->hit < victim->max_hit)
		damage(ch,victim,-1 * heal,sn,DAM_HOLY,true);

	if(sn != gsn_reverence)
		check_reverence(ch,heal);
	update_pos(victim,NULL);
}

void spell_cure_disease(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim,gsn_plague)){
		if (victim == ch)
			send_to_char("You aren't ill.\n\r",ch);
		else
			act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level,victim,gsn_plague)){
		send_to_char("Your sores vanish.\n\r",victim);
		act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
	}
	else
		send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_poison(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
 
    if ( !is_affected(victim,gsn_poison) )
    {
		if (victim == ch)
			send_to_char("You aren't poisoned.\n\r",ch);
		else
			act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
		return;
    }
 
    if (check_dispel(level,victim,gsn_poison))
    {
        send_to_char("A warm feeling runs through your body.\n\r",victim);
        act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    }
    else
        send_to_char("Spell failed.\n\r",ch);
}

void spell_prayer(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	AFFECT_DATA af;

	if (ch->in_room->israf(RAF_REGENERATION)){
		ch->send("This room is already healies.\n\r");
		return;
	}
	if(ch->in_room->sector_type == SECT_NEUTRALTEMPLE || ch->in_room->sector_type == SECT_EVILTEMPLE){
		ch->send("This room is not fit for the blessings of your god.\n\r");
		return;
	}
	act("You offer a prayer and the room seems to glow with a gentle warmth.",ch,NULL,NULL,TO_CHAR);
	act("$n offers a prayer and the room seems to glow with a gentle warmth.",ch,NULL,NULL,TO_ROOM);
    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 6;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = RAF_REGENERATION;
    affect_to_room(ch->in_room,&af);
}

void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim,gsn_blindness)){
		if (victim == ch)
			ch->send("You aren't blind.\n\r");
		else
			act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level,victim,gsn_blindness)){
		victim->send("Your vision returns!\n\r");
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
	}
	else
		ch->send("Spell failed.\n\r");
}

void spell_holy_word(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch;

	for(vch = ch->in_room->people;vch;vch = vch->next_in_room){
		if(!is_same_group(ch,vch) || is_affected(vch,sn) || vch->alignment < 0)
			continue;
		affect_set(ch,TO_AFFECTS,sn,level,slvl,UMAX(1,ch->getslvl(sn)) * 5,APPLY_SAVES,-1,0);
		affect_set(ch,TO_AFFECTS,sn,level,slvl,UMAX(1,ch->getslvl(sn)) * 5,APPLY_AC,-1*(ch->getslvl(sn)*8),0);

		act("You are protected by a holy aura.",vch,NULL,NULL,TO_CHAR);
		act("$n is protected with a holy aura.",vch,NULL,NULL,TO_ROOM);
	}
}

void check_divinity(CHAR_DATA *ch,int heal){
	CHAR_DATA *vch;
	int sn = gsn_divinity,skill = get_skill(ch,sn) / 20,count = 0;

	if(skill < 1)
		return;

	skill += get_curr_stat(ch,STAT_FTH) + ch->getslvl(sn);

	if(number_percent() < skill){
		ch->send("Your divinity spreads your healing touch to your allies.\n\r");
		act("$n's divinity spreads $s healing touch to $s allies.\n\r",ch,NULL,NULL,TO_ROOM);
		for(vch = ch->in_room->people;vch;vch = vch->next_in_room)
			if(is_same_group(ch,vch))
				count++;
		if(count < 2)
			return;
		heal += heal / 10 * ch->getslvl(sn);
		heal /= count;
		for(vch = ch->in_room->people;vch;vch = vch->next_in_room){
			if(vch == ch || !is_same_group(ch,vch))
				continue;
			heal_them(ch,vch,sn,heal);
		}
		check_improve(ch,sn,true,1);
		return;
	}
	check_improve(ch,sn,false,1);
}

void spell_ray_of_truth(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,align;

	if(IS_EVIL(ch)){
		victim = ch;
		send_to_char("The energy explodes inside you!\n\r",ch);
	}

	if(victim != ch){
		act("$n raises $s hand, and a blinding ray of light shoots forth!",ch,NULL,NULL,TO_ROOM);
		send_to_char("You raise your hand and a blinding ray of light shoots forth!\n\r",ch);
	}

	if(IS_GOOD(victim)){
		act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
		send_to_char("The light seems powerless to affect you.\n\r",victim);
		return;
	}

	dam = dice(level * ch->getslvl(sn),10);
	if(saves_spell(level,victim,DAM_HOLY))
		dam /= 2;

	if(IS_EVIL(victim))
		dam *= 2;

	spell_damage(ch,victim,dam,sn,DAM_HOLY,true);
	spell_blindness(gsn_blindness,3 * level / 4,ch,(void *) victim,TARGET_CHAR,slvl);
}

void spell_lift_curse(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    bool found = false;

    /* do object cases first */
    if (target == TARGET_OBJ)
    {
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj,ITM_NODROP) || IS_OBJ_STAT(obj,ITM_NOREMOVE))
		{
			if (!IS_OBJ_STAT(obj,ITM_NOUNCURSE) && !saves_dispel(level + 2,obj->level,0))
			{
				REMOVE_BIT(obj->extra_flags,ITM_NODROP);
				REMOVE_BIT(obj->extra_flags,ITM_NOREMOVE);
				act("$p glows red.",ch,obj,NULL,TO_ALL);
				return;
			}

			act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
			return;
		}
		act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR);
		return;
    }

    /* characters */
    victim = (CHAR_DATA *) vo;

	if (check_dispel(level,victim,gsn_curse))
	{
		send_to_char("You feel better.\n\r",victim);
		act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
	}

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
        if ((IS_OBJ_STAT(obj,ITM_NODROP) || IS_OBJ_STAT(obj,ITM_NOREMOVE)) &&  !IS_OBJ_STAT(obj,ITM_NOUNCURSE))
        {   /* attempt to remove curse */
            if (!saves_dispel(level,obj->level,0))
            {
                found = true;
                REMOVE_BIT(obj->extra_flags,ITM_NODROP);
                REMOVE_BIT(obj->extra_flags,ITM_NOREMOVE);
                act("Your $p glows red.",victim,obj,NULL,TO_CHAR);
                act("$n's $p glows red.",victim,obj,NULL,TO_ROOM);
            }
         }
    }
}

void spell_sanctuary(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if ( victim->isaff(AF_SANCTUARY) )
	{
		if (victim == ch)
			send_to_char("You are already in sanctuary.\n\r",ch);
		else
			act("$N is already in sanctuary.",ch,NULL,victim,TO_CHAR);
		return;
	}
	if(victim != ch && ch->getslvl(sn) != 5){
		ch->send("You cannot cast this on others.\n\r");
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level/6,APPLY_NONE,0,AF_SANCTUARY);
	act("$n is surrounded by a white aura.",victim,NULL,NULL,TO_ROOM);
	send_to_char("You are surrounded by a white aura.\n\r",victim );
}

void spell_calm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch;
	AFFECT_DATA af;
	int mlevel = 0,count = 0,high_level = 0,chance;

	for(vch = ch->in_room->people;vch;vch = vch->next_in_room){
		if(vch->position == POS_FIGHTING){
			count++;
			if(IS_NPC(vch))
			  mlevel += vch->level;
			else
			  mlevel += vch->level/2;
			high_level = UMAX(high_level,vch->level);
		}
	}

	chance = 4 * level - high_level + 2 * count;

	if (IS_IMMORTAL(ch)) /* always works */
	  mlevel = 0;

	if (number_range(0,chance) >= mlevel){  /* hard to stop large fights */
		for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room){
			if (IS_NPC(vch) && (vch->res[RS_LIGHT] == 0 || vch->isact(AT_UNDEAD)))
				return;

			if (vch->isaff(AF_CALM) || vch->isaff(AF_BERSERK) || is_affected(vch,skill_lookup("frenzy")))
				return;
			
			send_to_char("A wave of calm passes over you.\n\r",vch);

			if (vch->fighting || vch->position == POS_FIGHTING)
				stop_fighting(vch,false);

			affect_set(vch,TO_AFFECTS,sn,level,slvl,4,APPLY_HITROLL,-5,AF_CALM);

			affect_set(vch,TO_AFFECTS,sn,level,slvl,4,APPLY_DAMROLL,-5,AF_CALM);
		}
	}
}

void spell_harm(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    spell_damage(ch,(CHAR_DATA*)vo,dice(slvl + 4,slvl * 3) + (level * 1.5),sn,DAM_HARM,true);
    return;
}

void spell_frenzy(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim,sn) || victim->isaff(AF_BERSERK)){
		if (victim == ch)
			send_to_char("You are already in a frenzy.\n\r",ch);
		else
			act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (is_affected(victim,skill_lookup("calm"))){
		if (victim == ch)
			send_to_char("Why don't you just relax for a while?\n\r",ch);
		else
			act("$N doesn't look like $e wants to fight anymore.", ch,NULL,victim,TO_CHAR);
		return;
	}

	if ((IS_GOOD(ch) && !IS_GOOD(victim)) || (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) || (IS_EVIL(ch) && !IS_EVIL(victim))){
		act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level/3,APPLY_HITROLL,level/6,0);

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level/3,APPLY_DAMROLL,level/6,0);

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level/3,APPLY_AC,10 * (level / 12),0);

	victim->send("You are filled with holy wrath!\n\r");
	act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

void spell_heal_minor(int sn, int level,CHAR_DATA *ch, void *vo, int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(ch->level/2,2);
	heal += heal * ch->getslvl(sn) / 5;
	heal_them(ch,victim,sn,heal);//170 or 1.75
}

void spell_heal_light(int sn, int level,CHAR_DATA *ch, void *vo, int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(ch->level,2);
	heal += heal * ch->getslvl(sn) / 5;
	heal_them(ch,victim,sn,heal);//225 or 1.3333333
}

void spell_heal_serious(int sn, int level,CHAR_DATA *ch, void *vo, int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(ch->level,3);
	heal += heal * ch->getslvl(sn) / 5;
	heal_them(ch,victim,sn,heal);//255 or 1.18
}

void spell_heal_critical(int sn, int level, CHAR_DATA *ch,void *vo, int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(ch->level,4);
	heal += heal * ch->getslvl(sn) / 5;
	heal_them(ch,victim,sn,heal);//300
}

void do_holytouch(CHAR_DATA *ch,char *argument)
{
/*	char arg[MSL];
	CHAR_DATA *victim;
	AFFECT_DATA af;

	one_argument(argument,arg);

	if (ch->isaff(AF_HOLYTOUCHWEARY))
	{
		ch->send("You're too weary to do this.\n\r");
		return;
	}

	if (!arg[0])
		victim = ch;
	else if ((victim = get_char_room(ch,NULL,arg)) == NULL)
	{
		ch->send("They're... not here.\n\r");
		return;
	}

	if (number_percent() < get_skill(ch,gsn_holytouch) *.7)
	{
		if (victim == ch)
		{
			ch->send("You place your hand above your heart and begin glowing as a holy warmth passes through your body.\n\r");
			act("$n places his hand above his heart and glows lightly.",ch,NULL,NULL,TO_ROOM);
		}
		else
		{
			act("You place your hand above $N's heart and glow lightly as you heal $M.",ch,NULL,NULL,TO_CHAR);
			act("$n places his hand above your heart and glows lightly and you feel a healing warmth.",ch,NULL,NULL,TO_VICT);
			act("$n places his hand above $N's heart and glows lightly.",ch,NULL,NULL,TO_NOTVICT);
		}
		heal_them(ch,victim,gsn_holytouch,UMIN(victim->max_hit - victim->hit,400));
		affect_set(ch,TO_AFFECTS,gsn_holytouch,ch->level,slvl,ch->level*10,APPLY_NONE,0,AF_WEARY);
	}
	else
	{
		if (victim == ch)
		{
			ch->send("You fail and stuff.\n\r");
			act("$n fails and stuff.",ch,NULL,NULL,TO_ROOM);
		}
		else
		{
			act("You fail $N.",ch,NULL,NULL,TO_CHAR);
			act("$n fails you.",ch,NULL,NULL,TO_VICT);
			act("$n fails $N.",ch,NULL,NULL,TO_NOTVICT);
		}
	}*/
}

void do_divinetouch(CHAR_DATA *ch,char *argument){
	char arg[MSL];
	CHAR_DATA *victim;
	AFFECT_DATA af;

	one_argument(argument,arg);

	/*if (ch->isaff(AF_DIVINETOUCHWEARY))
	{
		ch->send("You're too weary to do this.\n\r");
		return;
	}*/

	if (!arg[0])
		victim = ch;
	else if ((victim = get_char_room(ch,NULL,arg)) == NULL){
		ch->send("They're... not here.\n\r");
		return;
	}

	if (number_percent() < get_skill(ch,gsn_divinetouch) *.7){
		check_improve(ch,gsn_divinetouch,true,2);
		if (victim == ch){
			ch->send("You place your hand above your heart and begin glowing as a holy warmth passes through your body.\n\r");
			act("$n places his hand above his heart and glows lightly.",ch,NULL,NULL,TO_ROOM);
		}
		else{
			act("You place your hand above $N's heart and glow lightly as you heal $M.",ch,NULL,NULL,TO_CHAR);
			act("$n places his hand above your heart and glows lightly and you feel a healing warmth.",ch,NULL,NULL,TO_VICT);
			act("$n places his hand above $N's heart and glows lightly.",ch,NULL,NULL,TO_NOTVICT);
		}
		heal_them(ch,victim,gsn_divinetouch,UMIN(victim->max_hit - victim->hit,100));
		affect_strip(victim,gsn_poison);
		affect_set(ch,TO_AFFECTS,gsn_divinetouch,ch->level,ch->getslvl(gsn_divinetouch),ch->level*10,APPLY_NONE,0,AF_WEARY);
	}
	else{
		check_improve(ch,gsn_divinetouch,false,3);
		if (victim == ch){
			ch->send("You miserably fail to whip out your divine touch.\n\r");
			act("$n miserably fails to whip out $s divine touch.",ch,NULL,NULL,TO_ROOM);
		}
		else{
			act("You mess up $N",ch,NULL,NULL,TO_CHAR);
			act("$n messes you up.",ch,NULL,NULL,TO_VICT);
			act("$n messes up $N.",ch,NULL,NULL,TO_NOTVICT);
		}
	}
}

void spell_focus(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(victim->isaff(AF_MANAREGEN)){
		if (victim == ch)
			ch->send("You already are regenerating mana!\n\r");
		else
			ch->send("They already are regenerating mana.\n\r");
		return;
	}

	if (victim->isaff(AF_CONFUSION)){
		purge_affect(level,victim,TO_AFFECTS,AF_CONFUSION);
	}
	else if (victim->isaff(AF_SLOWCAST)){
		purge_affect(level,victim,TO_AFFECTS,AF_SLOWCAST);
	}
	else{
		act("$n seems to shimmer briefly.",victim,NULL,NULL,TO_ROOM);
		victim->send("You seem to shimmer briefly.\n\r");
		affect_set(victim,TO_AFFECTS,sn,ch->level,slvl,ch->level/10,APPLY_WIS,1,AF_MANAREGEN);
	}
}

void spell_recovery(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(victim->isaff(AF_REGENERATION)){
		if (victim == ch)
			ch->send("You already are regenerating mana!\n\r");
		else
			ch->send("They already are regenerating mana.\n\r");
		return;
	}

	if (victim->isaff(AF_DEGENERATION)){
		purge_affect(level,victim,TO_AFFECTS,AF_DEGENERATION);
	}
	else if (victim->isaff(AF_PREVENTHEAL)){
		purge_affect(level,victim,TO_AFFECTS,AF_PREVENTHEAL);
	}
	else{
		act("$n seems to glow briefly.",victim,NULL,NULL,TO_ROOM);
		victim->send("You seem to glow briefly before a warm feeling fills your body.\n\r");
		affect_set(victim,TO_AFFECTS,sn,ch->level,slvl,UMAX(1,ch->getslvl(sn)),APPLY_END,1,AF_REGENERATION);
	}
}

void spell_resuscitate(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;

	if(victim->death_timer < 1){
		ch->send("They seem to be in no danger of death.\n\r");
		return;
	}

	victim->position = POS_RESTING;
	victim->hit = 1;
	victim->death_timer = 0;
	act("$n's wounds heal and $e seems to no longer be dying.",victim,NULL,NULL,TO_ROOM);
	victim->send("Your life has been saved.\n\r");
	damage(ch,victim,-1 * (victim->max_hit/20),sn,DAM_HOLY,true);
}

void spell_revive(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;

	if(victim->position > POS_SLEEPING){
		ch->send("They are not under unnatural means of slumber.\n\r");
		return;
	}
	if(victim->position <= POS_MORTAL || victim->death_timer > 0){
		ch->send("They are too far gone now.\n\r");
		return;
	}

	victim->position = POS_RESTING;
	affect_strip(victim,gsn_sap);
	affect_strip(victim,gsn_strangle);
	act("$n seems to to be much more awake.",victim,NULL,NULL,TO_ROOM);
	victim->send("You've been revived.\n\r");
	victim->hit = UMAX(victim->hit,1);
}

void spell_purify(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim;
    OBJ_DATA *obj;

	if (target == TARGET_OBJ){
		obj = (OBJ_DATA *) vo;	

		if (obj->item_type != ITEM_DRINK_CON && obj->item_type != ITEM_FOOD){
			act("$p is not food or a drink.",ch,obj,NULL,TO_CHAR);
			return;
		}
		if (!obj->value[3]){
			act("$p is not poisoned.",ch,obj,NULL,TO_CHAR);
			return;
		}

		if(number_percent() < ch->level * .75){
			act("You purify $p.",ch,obj,NULL,TO_CHAR);
			act("$n purifies $p of any poisons.",ch,obj,NULL,TO_ROOM);
			obj->value[3] = false;
		}
		else
			ch->send("You fail.\n\r");
		return;
	}

	victim = (CHAR_DATA *) vo;

	if(!victim->isaff(AF_AGUE) && !victim->isaff(AF_CURSE)){
		if (ch == victim)
			ch->send("You are pure enough.\n\r");
		else
			ch->send("They are pure enough.\n\r");
		return;
	}

	purge_affect(level,victim,TO_AFFECTS,AF_AGUE);
	purge_affect(level,victim,TO_AFFECTS,AF_CURSE);
}

void spell_cleanse(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if(!victim->isaff(AF_FAERIE_FIRE) && !victim->isaff(AF_DEAF) && !victim->isaff(AF_CHARM)){
		if (ch == victim)
			ch->send("You are clean enough.\n\r");
		else
			ch->send("They are clean enough.\n\r");
		return;
	}

	act("$n is cleansed.",victim,NULL,NULL,TO_ROOM);
	victim->send("You are cleansed.\n\r");
	purge_affect(level,victim,TO_AFFECTS,AF_FAERIE_FIRE);
	purge_affect(level,victim,TO_AFFECTS,AF_DEAF);
	purge_affect(level,victim,TO_AFFECTS,AF_CHARM);
}

void spell_soothe(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if(!victim->isaff(AF_RUPTURE) && !victim->isaff(AF_WEAKEN)){
		if (ch == victim)
			ch->send("You are soothed enough.\n\r");
		else
			ch->send("They are soothed enough.\n\r");
		return;
	}

	if (ch == victim){
		act("$n attempts to soothe $mself.",victim,NULL,NULL,TO_ROOM);
		victim->send("You attempt to soothe your body.\n\r");
	}
	else{
		act("You attempt to soothe $N's body.",ch,NULL,victim,TO_CHAR);
		act("$n attempts to soothe your body.",ch,NULL,victim,TO_VICT);
		act("$n attempts to soothe $N's body.",ch,NULL,victim,TO_NOTVICT);
	}
	purge_affect(level,victim,TO_AFFECTS,AF_RUPTURE);
	purge_affect(level,victim,TO_AFFECTS,AF_WEAKEN);
}

void spell_renew(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim == ch){
		ch->send("You cannot renew your own mana reserves.\n\r");
		return;
	}
	act("$n is momentarily enveloped in a golden glow.",victim,NULL,NULL,TO_ROOM);
	victim->send("You are evenloped in a golden glow for a moment.\n\r");
	heal_them(ch,victim,sn,ch->level * 1.5);
	act("$n looks more energetic.",victim,NULL,NULL,TO_ROOM);
	victim->send("You feel your energy rushing back.\n\r");
	victim->setmana(UMIN(victim->getmaxmana(),victim->getmana() + (ch->level / 2)));
	act("$n looks less tired.",victim,NULL,NULL,TO_ROOM);
	victim->send("You feel less tired.\n\r");
	victim->move = UMIN(victim->max_move,victim->move + (ch->level / 2));
}

void spell_restoration(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *gch;
    int heal_num, refresh_num;

    heal_num = 125+(25 * ch->pcdata->skill_level[sn]);
    refresh_num = skill_lookup("refresh");

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ){
		if ((IS_NPC(ch) && IS_NPC(gch)) || (!IS_NPC(ch) && !IS_NPC(gch))){
			heal_them(ch,gch,sn,heal_num);
			spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR,slvl);
		}
    }
}

bool check_poverty(CHAR_DATA *ch)
{
	int chance;

	if(IS_NPC(ch) || (chance = get_skill(ch,gsn_poverty)) < 1)
		return false;

	chance /= 2;

	if(ch->gold + ch->bankgold >= 1000 || ch->silver + ch->banksilver > 500 || ch->carry_number > 60)
		return false;

	if(number_percent() <= chance)
	{
		send_to_char("The gods reward you for your faith to immaterialism.\n\r",ch);
		check_improve(ch,gsn_poverty,true,8);
		return true;
	}
	else
		check_improve(ch,gsn_poverty,false,8);
	return false;
}

int check_piety(CHAR_DATA *ch,int n,int type){
	int sn = gsn_piety,chance = get_skill(ch,sn) * .9;

	if (chance < 1){
		switch(type){
		case PIETY_AC:
			return 0;
		case PIETY_MANA:
		case PIETY_DAMAGE:
			return n;
		}
	}

	if (number_percent() < chance){
		if(type != PIETY_AC)
			ch->send("Your piety to your god imbues your spell.\n\r");
		check_improve(ch,sn,true,1);
		if (type == PIETY_AC)
			return get_curr_stat(ch,STAT_FTH) + (ch->getslvl(sn) * 5);
		else if (type == PIETY_MANA)
			return n -= n * (ch->getslvl(sn) * 10) / 100;
		else if (type == PIETY_DAMAGE)
			return n += ((n * get_curr_stat(ch,STAT_FTH) / (STAT_MAX / 2)) * ch->getslvl(sn) / 5);
	}
	check_improve(ch,sn,false,1);
	if (type == PIETY_AC)
		return 0;
	else
		return n;
}

int check_purity(CHAR_DATA *ch,int heal){
	int sn = gsn_purity,skill = get_skill(ch,sn) / 3;
	if(skill < 1)
		return heal;

	skill += get_curr_stat(ch,STAT_FTH) / 2;
	if(roll_chance(ch,skill)){
		ch->send("Your faith is a benefit to your powers.\n\r");
		heal += heal / 100 * ch->getslvl(sn) * 2;
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
	return heal;
}

void spell_turn_undead(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,chance;

	act("$n points a hand at $N, engulfing them in a white light!",ch,NULL,victim,TO_NOTVICT);
	act("$n points a hand at you and you are engulfed in a white light!",ch,NULL,victim,TO_VICT);
	act("You point your hand at $N, engulfing them in divine judgement!",ch,NULL,victim,TO_CHAR);

	if(!victim->isact(AT_UNDEAD) && !victim->isform(FRM_UNDEAD)){
		ch->send("They're not undead!\n\r");
		return;
	}

	chance = get_curr_stat(ch,STAT_FTH) + (ch->getslvl(sn)*5);

	if(roll_chance(ch,chance)){
		dam = level + 5 * ((get_curr_stat(ch,STAT_INT) * 4) + get_curr_stat(ch,STAT_WIS)) / 5;

		act("$N crumples to the ground, turning into a lifeless corpse!",ch,NULL,victim,TO_NOTVICT);
		act("$n turns you back to your lifeless form!",ch,NULL,victim,TO_VICT);
		act("You turn $N, sending $s soul back to the grave!",ch,NULL,victim,TO_CHAR);
		raw_kill(ch,victim);
		return;
	}
	spell_damage(ch,victim,ch->level * ch->getslvl(sn),sn,DAM_HOLY,true);
}

void spell_consecrate(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	act("$n points a hand at $N, and a white shockwave knocks $M off $S feet!",ch,NULL,victim,TO_NOTVICT);
	act("$n points a hand at you and a white shockwave knockes you off your feet!",ch,NULL,victim,TO_VICT);
	act("You point your hand at $N, knocking $M off $S feet with a holy shockwave!",ch,NULL,victim,TO_CHAR);

	dam = level + 5 * ((get_curr_stat(ch,STAT_FTH) * 4) + number_range(1,get_curr_stat(ch,STAT_WIS)));

	if(victim->isact(AT_UNDEAD) || victim->isform(FRM_UNDEAD) || victim->alignment < 0){
		dam *= 2;
	}
	if(!saves_spell(level,victim,DAM_HOLY))//NASHNEEDSTOREVIEWSAVERULES
		dam *= 1.5;

	spell_damage(ch,victim,ch->level * ch->getslvl(sn),sn,DAM_HOLY,true);
}

void spell_sacrifice(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	int count = 0,heal = 100;
	if(ch->hit < 101){
		ch->send("You are too weak to sacrifice yourself.\n\r");
		return;
	}
	for(CHAR_DATA *vch = ch->in_room->people;vch;vch = vch->next_in_room)
		if(is_same_group(vch,ch) && ch != vch)
			count++;
	if(count < 1){
		ch->send("There is no one else to sacrifice yourself for.\n\r");
		return;
	}
	ch->hit -= 100;
	heal += heal / 10 * slvl;
	for(CHAR_DATA *vch = ch->in_room->people;vch;vch = vch->next_in_room)
		if(is_same_group(vch,ch) && ch != vch)
			heal_them(ch,vch,sn,heal);
}

void spell_guiding_light(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *vch = NULL;
	int energy,dam;

	act("$n raises $s hand and a divine light shoots out from $s finger!",ch,NULL,NULL,TO_ROOM);
	act("You call forth a guiding light!",ch,NULL,NULL,TO_CHAR);

	energy = ch->getslvl(sn)*2;

	dam = ch->level + (ch->getslvl(sn) * ch->level/10);
	dam += number_range(get_curr_stat(ch,STAT_FTH),get_curr_stat(ch,STAT_FTH)*2);

	for(;energy > 0;energy--){
		for(int tries = 0;tries < 10;tries++){
			vch = get_random_char(ch,NULL,NULL);
			if(vch == ch || is_same_group(vch,ch) || !canFight(ch,vch,false))
				continue;
		}
		if(!vch)
			continue;
		if(vch == ch || is_same_group(vch,ch) || !canFight(ch,vch,false)){
			ch->send("Your light fizzles.\n\r");
			continue;
		}
		if(!saves_spell(level,vch,DAM_HOLY))
			spell_damage(ch,vch,dam*2,sn,DAM_HOLY,true);
		else
			spell_damage(ch,vch,dam,sn,DAM_HOLY,true);
	}
}

void check_reverence(CHAR_DATA *ch,int dam){
	CHAR_DATA *vch;
	int sn = gsn_reverence,skill = get_skill(ch,gsn_reverence);
	if(IS_NPC(ch) || skill <1)
		return;

	skill /= 5;

	skill += get_curr_stat(ch,STAT_FTH);

	if(roll_chance(ch,skill)){
		ch->send("You glow white as your faith spreads your healing touch to your allies.\n\r");
		act("$n glows a holy white.",ch,NULL,NULL,TO_ROOM);
		for(vch = ch->in_room->people;vch;vch = vch->next_in_room){
			if(ch == vch || !is_same_group(ch,vch))
				continue;
			vch->send("You feel warmth over your body.\n\r");
			heal_them(ch,vch,sn,dam * (ch->getslvl(sn)) / 4);
		}
		check_improve(ch,sn,true,1);
	}
	else
		check_improve(ch,sn,false,1);
}

bool check_true_belief(CHAR_DATA *ch){
	AFFECT_DATA *af = new_affect();
	int sn = gsn_true_belief,skill = get_skill(ch,sn);

	if(skill < 1 || is_affected(ch,sn) || (IS_NPC(ch) && !ch->isdef(DF_TRUEBELIEF)))
		return false;

	ch->send("As blackness creeps at your vision, a shaft of golden light encases your body.\n\r");
	ch->send("{YYour true belief in a higher power saves you from death!{x\n\r");
	act("$n is engulfed in a golden light and is saved from death!",ch,NULL,NULL,TO_ROOM);
	affect_set(ch,TO_AFFECTS,sn,ch->level,ch->getslvl(gsn_true_belief),120 - ch->getslvl(gsn_true_belief) * 10,APPLY_NONE,0,AF_WEARY);
	check_improve(ch,sn,true,1);
	return true;
}
