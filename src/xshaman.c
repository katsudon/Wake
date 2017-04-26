#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

extern char *target_name;

void spell_spiritual_radiance( int sn, int level, CHAR_DATA *ch, void *vo, int target,int slvl)
{
    CHAR_DATA *vch, *nvir;
    AFFECT_DATA af;

    send_to_char( "Your inner spirit radiates outward in blinding brilliance!\n\r", ch );
    act( "$n's inner spirit radiates out in blinding brilliance!", ch, NULL, NULL, TO_ROOM );

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir)
	{
		nvir = vch->next_in_room;
		if (vch->isaff(AF_BLIND))
			continue;
		if ( vch != ch && !is_safe(ch,vch) && !saves_spell( ch->level*1.5,vch,DAM_HOLY))
		{
			act("You are struck by $n's spiritual radiance!",ch,NULL,vch,TO_VICT);
			spell_damage( ch, vch, vch->hit * .05, sn, DAM_HOLY,false);

			affect_set(vch,TO_AFFECTS,sn,level,slvl,vch->level/4+1,APPLY_HITROLL,-10,AF_BLIND);
			act("$N is blinded by $n's spiritual radiance!",ch,NULL,vch,TO_CHAR);
			send_to_char( "You are blinded!\n\r", vch );
			act("$N is blinded by $n's spiritual radiance!",ch,NULL,vch,TO_NOTVICT);
		}
	}
    return;
}

void spell_spirit_snow( int sn, int level, CHAR_DATA *ch, void *vo, int target,int slvl)
{
    CHAR_DATA *vch, *nvir;
    AFFECT_DATA af;

    send_to_char( "You call upon the spirit of snow and a blizzard swirls about you!\n\r", ch );
    act( "$n summons the spirit of snow and is engulfed in a blizzard!", ch, NULL, NULL, TO_ROOM );

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir)
	{
		nvir = vch->next_in_room;
		if ( vch != ch && !is_safe(ch,vch))
		{
			act("You are caught in $n's blizzard!",ch,NULL,vch,TO_VICT);
			spell_damage( ch, vch, vch->hit * .1, sn, DAM_COLD,false);

			if (!saves_spell( ch->level*1.5,vch,DAM_COLD)){
				affect_set(vch,TO_AFFECTS,sn,level,slvl,2,APPLY_AGI,-2,AF_SLOW);
				act("$N turns blue from the cold of the spirit of snow!",ch,NULL,vch,TO_CHAR);
				send_to_char( "Your limbs become stiff from the cold!\n\r", vch );
				act("$N turns blue from the cold of the spirit of snow!",ch,NULL,vch,TO_NOTVICT);
			}
		}
	}
    return;
}

void spell_spirit_storm( int sn, int level, CHAR_DATA *ch, void *vo, int target,int slvl)
{
    CHAR_DATA *vch, *nvir;
	int dam = 0;

    send_to_char( "You call upon the spirit of storms and a raging whirlwind of hail and lightning surrounds you!\n\r", ch );
    act( "$n summons the spirit of storms and is engulfed in a whirlwind of hail and lightning!", ch, NULL, NULL, TO_ROOM );

	dam = number_range(ch->level * .5, ch->level * .75);

	for ( vch = ch->in_room->people; vch != NULL; vch = nvir)
	{
		nvir = vch->next_in_room;
		if ( vch != ch && !is_safe(ch,vch))
		{
			act("You are caught in $n's storm!",ch,NULL,vch,TO_VICT);
			if (saves_spell( ch->level*1.5,vch,DAM_STORM))
				spell_damage( ch, vch, dam/2, sn, DAM_STORM,true);
			else
				spell_damage( ch, vch, dam, sn, DAM_STORM,true);
		}
	}
    return;
}

void spell_spirit_fire( int sn, int level, CHAR_DATA *ch, void *vo, int target,int slvl)
{
    CHAR_DATA *vch, *nvir;
    AFFECT_DATA af;

    send_to_char( "You call upon the spirit of fire and an inferno erupts from the ground about you!\n\r", ch );
    act( "$n summons the spirit of fire and is engulfed in an inferno!", ch, NULL, NULL, TO_ROOM );

	for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		nvir = vch->next_in_room;
		if ( vch != ch && !is_safe(ch,vch))
		{
			act("You are caught in $n's inferno!",ch,NULL,vch,TO_VICT);
			spell_damage( ch, vch, vch->hit * .1, sn, DAM_FIRE,false);

			if (!saves_spell( ch->level*1.5,vch,DAM_FIRE))
			{
				affect_set(vch,TO_AFFECTS,sn,level,slvl,2,APPLY_STR,-3,AF_WEAKEN);
				act("$N crumples in weakness from $S burns!",ch,NULL,vch,TO_CHAR);
				send_to_char( "You go numb as your flesh is seared!\n\r", vch );
				act("$N crumples in weakness from $S burns!",ch,NULL,vch,TO_NOTVICT);
			}
		}
	}
    return;
}

void spell_spirit_wind( int sn, int level, CHAR_DATA *ch, void *vo, int target,int slvl)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	EXIT_DATA *pexit;
	int found=0,door=0,attempt=0;

	if (ch->fighting == NULL)
	{
		send_to_char("The spirit of the wind cannot aid you out of combat.\n\r",ch);
		return;
	}
	
	if(victim == NULL)
		victim = ch;

	for ( attempt = 0; attempt < 10; attempt++ )
	{
		door = number_range(0,6);
		if ((pexit = victim->in_room->exit[door]) == 0
		||   pexit->u1.to_room == NULL
		||   IS_SET(pexit->exit_info, EX_CLOSED)
		||   number_range(0,victim->daze) != 0
		||  (IS_NPC(victim) && IS_SET(pexit->u1.to_room->room_flags,ROOM_NO_MOB)))
			continue;
		else
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		send_to_char("You failed!\n\r",ch);
		return;
	}

	if(victim == ch)
	{
		act( "You call upon the spirit of wind and send a powerful gust at yourself!", ch,NULL,victim,TO_CHAR );
		act( "$n summons the spirit of wind and sends a power gust of wind at $mself!", ch, NULL, victim, TO_ROOM );
		stop_fighting(ch,true);
		act("$n is hurled away by the spirit of wind!",ch,NULL,NULL,TO_ROOM);
		move_char( ch, door, false, false );
		send_to_char( "You are hurled away by the spirit of wind!\n\r", victim );
	}
	else
	{
		act( "You call upon the spirit of wind and send a powerful gust at $N!", ch,NULL,victim,TO_CHAR );
		if(!is_safe(ch,victim))
		{
			act( "$n summons the spirit of wind and sends a powerful gust of wind at $N!", ch, NULL, victim, TO_NOTVICT );
			act( "$n summons the spirit of wind and sends a powerful gust of wind at YOU!", ch, NULL, victim, TO_VICT );
			spell_damage( ch, victim, victim->hit * .1, sn, DAM_WIND,false);
			
			if (!saves_spell( ch->level * 2,victim,DAM_WIND))
			{
				stop_fighting(victim,true);
				act("$N is hurled away by the spirit of wind!",ch,NULL,victim,TO_ROOM);
				act("$N is hurled away by the spirit of wind!",ch,NULL,victim,TO_CHAR);
				move_char( victim, door, false, false );
				send_to_char( "You are hurled away by the spirit of wind!\n\r", victim );
			}
			else
			{
				send_to_char("You failed!\n\r",ch);
			}
		}
	}
    return;
}

void spell_malady(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
	int loc;

    victim = (CHAR_DATA *) vo;

    if ( saves_spell( ch->level*1.5, victim,DAM_DISEASE) )
    {
		act("$n seems to shiver a bit but regains control.",victim,NULL,NULL,TO_ROOM);
		send_to_char("You feel a momentary chill, but it passes.\n\r",victim);
		return;
    }


    send_to_char( "You feel a malady course through your veins.\n\r", victim );
    act("$n looks very very sick.",victim,NULL,NULL,TO_ROOM);

	switch(number_range(0,4))
	{
		case 0:
			act("$N whimpers as $S muscles seem to tense.",ch,NULL,victim,TO_CHAR);
			act("Your muscles begin to cramp and ache.",ch,NULL,victim,TO_VICT);
			act("$N whimpers as $S muscles seem to tense",ch,NULL,victim,TO_NOTVICT);
			loc  = APPLY_STR;
			break;
		case 1:
			act("$N claws at $S head in pain.",ch,NULL,victim,TO_CHAR);
			act("The blinding pain clouds your memories.",ch,NULL,victim,TO_VICT);
			act("$N claws at $S head in pain.",ch,NULL,victim,TO_NOTVICT);
			loc  = APPLY_WIS;
			break;
		case 2:
			act("$N's limbs seem to lock up and go into seizures.",ch,NULL,victim,TO_CHAR);
			act("Your joints seize up and lock up.",ch,NULL,victim,TO_VICT);
			act("$N's limbs seem to lock up and go into seizures.",ch,NULL,victim,TO_NOTVICT);
			loc  = APPLY_AGI;
			break;
		case 3:
			act("$N gets a blank stare on $S face and twitches.",ch,NULL,victim,TO_CHAR);
			act("Your mind goes blank as a mind blowing headache strikes you.",ch,NULL,victim,TO_VICT);
			act("$N gets a blank stare on $S face and twitches.",ch,NULL,victim,TO_NOTVICT);
			loc  = APPLY_INT;
			break;
		case 4:
			act("$N stumbles and $S muscles seem to atrophy.",ch,NULL,victim,TO_CHAR);
			act("Your muscles become sickeningly weak and atrophy.",ch,NULL,victim,TO_VICT);
			act("$N stumbles and $S muscles seem to atrophy.",ch,NULL,victim,TO_NOTVICT);
			loc  = APPLY_END;
			break;
	}
	affect_set(victim,TO_AFFECTS,sn,ch->level,slvl,3,loc,-3,AF_PLAGUE);
}

void spell_hawk_awareness( int sn, int level, CHAR_DATA *ch, void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if (victim->isaff(AF_DETECT_INVIS) )
    {
		if (victim == ch)
			send_to_char("You already have the awareness of the hawk's spirit.\n\r",ch);
		else
			act("$N is already imbued with the awareness of a hawk.",ch,NULL,victim,TO_CHAR);
		return;
    }

	affect_set(victim,TO_AFFECTS,sn,level,slvl,level,APPLY_NONE,0,AF_DETECT_INVIS);
    send_to_char( "Your eyes become more aware of spiritual energy.\n\r", victim );
    if ( ch != victim )
		send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_wolf_speed(int sn,int level,CHAR_DATA *ch, void *vo,int target,int slvl){
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
		if (victim == ch)
			send_to_char("The speed of the wolf already surges through your soul.\n\r",ch);
		else
			act("The wolf already surges through $N's body.",ch,NULL,victim,TO_CHAR);
		return;
    }

	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_AGI,level/10,0);

	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_AC,0 - level / 5,0);

    act( "The spirit of the wolf surges into $n's soul.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You feel the spirit of the wolf giving your soul great swiftness.\n\r", victim );
    return;
}

void spell_lion_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;

	if ( is_affected( ch, sn ) )
	{
		if (victim == ch)
			send_to_char("The strength of the lion already surges through your soul.\n\r",ch);
		else
			act("The lion already surges through $N's body.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_STR,level/20,0);

	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_HITROLL,level/10,0);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_DAMROLL,level/10,0);

    act( "The spirit of the lion surges into $n's soul.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You feel the spirit of the lion giving your soul great strength.\n\r", victim );
    return;
}
