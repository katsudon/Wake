/*--------------------------------------------------------------------------
              .88b  d88. db    db d8888b.   .d888b. db   dD
              88'YbdP`88 88    88 88  `8D   VP  `8D 88 ,8P'
              88  88  88 88    88 88   88      odD' 88,8P
              88  88  88 88    88 88   88    .88'   88`8b
              88  88  88 88b  d88 88  .8D   j88.    88 `88.
              YP  YP  YP ~Y8888P' Y8888D'   888888D YP   YD
This material is copyrighted (c) 1999 - 2000 by Thomas J Whiting 
(twhiting@hawmps.2y.net). Usage of this material  means that you have read
and agree to all of the licenses in the ../licenses directory. None of these
licenses may ever be removed.
----------------------------------------------------------------------------
A LOT of time has gone into this code by a LOT of people. Not just on
this individual code, but on all of the codebases this even takes a piece
of. I hope that you find this code in some way useful and you decide to
contribute a small bit to it. There's still a lot of work yet to do.
---------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "interp.h"
#include "db.h"
#include "magic.h"
void lightning		( void );
void ice			( void );
void hail			( void );
void blizzard		( void );
void fog			( void );
void weather_update	( void );
extern char *target_name;

void arid_weather_handler();
void temperate_weather_handler();
void tropical_weather_handler();
void frigid_weather_handler();
void desolate_weather_handler();
void ocean_weather_handler();

/*
 * Update the weather.
 */
void check_penumbral(CHAR_DATA *ch){
	DESCRIPTOR_DATA *d;
	if(!ch){
		for(d = descriptor_list;d;d = d->next){
			CHAR_DATA *victim;

			victim = d->original ? d->original : d->character;

			if (d->connected == CON_PLAYING && victim->isaff(AF_PERFECTSNEAK)){
				affect_strip(victim,gsn_penumbralveil);
				victim->send("The daybreak ruins your perfect cover.\n\r");
			}
		}
	}
	else{
		if(ch->isaff(AF_PERFECTSNEAK)){
			affect_strip(ch,gsn_penumbralveil);
			ch->send("The daybreak ruins your perfect cover.\n\r");
		}
	}
}

void weather_update(void){
    char buf[MSL];
    DESCRIPTOR_DATA *d;

    buf[0] = '\0';

	if (++time_info.ticks > 11){
		time_info.ticks = 0;
		switch (++time_info.hour){
			case  5:
				weather_info.sunlight = SUN_RISE;
				strcat(buf,"The sun slowly rises above the eastern horizon.\n\r" );
				break;
			case  6:
				weather_info.sunlight = SUN_LIGHT;
				strcat(buf,"The day has begun.\n\r" );
				check_penumbral(NULL);
				break;
			case 12:
				strcat(buf,"The sun reaches its peak in the sky.\n\r" );
				break;
			case 19:
				weather_info.sunlight = SUN_SET;
				strcat(buf,"{wThe sun slowly disappears in the west.{x\n\r" );
				break;
			case 20:
				weather_info.sunlight = SUN_DARK;
				strcat(buf,"{dThe night has begun.{x\n\r");
				break;
			case 24:
				time_info.hour = 0;
				time_info.day++;
				break;
		}
	}

    if (time_info.day >= 35)
    {
		time_info.day = 0;
		time_info.month++;
    }

    if (time_info.month >= 12)
    {
		time_info.month = 0;
		time_info.year++;
    }

	if (time_info.ticks == 0)
	{
		arid_weather_handler();
		temperate_weather_handler();
		tropical_weather_handler();
		frigid_weather_handler();
		desolate_weather_handler();
		ocean_weather_handler();
	}

    if ( buf[0] != '\0' )
		for ( d = descriptor_list; d != NULL; d = d->next )
			if ( d->connected == CON_PLAYING && IS_OUTSIDE(d->character) && IS_AWAKE(d->character) )
				send_to_char( buf, d->character );

    return;
}

/*
New weather command. Reads the new weather stats as well as  tells you what time of day it is (morning, noon, night) Added so that players don't HAVE to have autoweather enabled*/
void do_weather( CHAR_DATA *ch, char *argument )
{
	char buf[MSL],buf2[MSL],*suf;
	int day,npercent;

	day     = time_info.day + 1;

		 if ( day > 4 && day <  20 ) suf = "th";
	else if ( day % 10 ==  1       ) suf = "st";
	else if ( day % 10 ==  2       ) suf = "nd";
	else if ( day % 10 ==  3       ) suf = "rd";
	else                             suf = "th";


	if ( !IS_OUTSIDE(ch) )
	{
		send_to_char( "You can't see the weather indoors.\n\r", ch );
		return;
	}
	send_to_char("{x\n\r",ch);

	switch (current_weather[ch->in_room->area->climate])
	{
		case SKY_RAINING:
			npercent = number_percent();
			if (npercent <= 15)			ch->send("{g[{YWeather{g] {CThere is a cold rain trickling down.{x\n\r");
			else if (npercent <= 30)	ch->send("{g[{YWeather{g] {CThere is a warm rain trickling down.{x\n\r");
			break;
		case SKY_CLOUDLESS:		send_to_char("{g[{YWeather{g] {CThe sky is beautiful, not a cloud around.{x\n\r",ch); break;
		case SKY_CLOUDY:		printf_to_char(ch,"{g[{YWeather{g] %s{x.\n\r", weather_info.change >= 0 ? "{CA warm breeze can be felt about" : "{CA cold breeze can be felt about" ); break;
		case SKY_WINDY:			send_to_char("{g[{YWeather{g] {CThere is a wind blowing through.{x\n\r",ch); break;
		case SKY_LIGHTNING:		send_to_char("{g[{YWeather{g] {CA {y Lightning{w storm is approaching {x\n\r",ch); break;
		case SKY_SNOWING:		send_to_char("{g[{YWeather{g] {CA light snow is falling.{x\n\r",ch); break;
		case SKY_BLIZZARD:		send_to_char("{g[{YWeather{g] {CThere is a blizzard about.{x\n\r",ch); break;
		case SKY_FOGGY:			send_to_char("{g[{YWeather{g] {CA misty haze covers the horizon.{x\n\r",ch); break;
		case SKY_HAILSTORM:		send_to_char("{g[{YWeather{g] {CGolfball like substances are falling from the sky.{x\n\r",ch); break;
		case SKY_THUNDERSTORM:		send_to_char("{g[{YWeather{g] {CThe skies thunder as a storm approaches.{x\n\r",ch); break;
		case SKY_SANDSTORM:		send_to_char("{g[{YWeather{g] {CSands blast about in a furious storm.{x\n\r",ch); break;
		case SKY_HEATWAVE:		send_to_char("{g[{YWeather{g] {CWaves of heat envelope everything.{x\n\r",ch); break;
		case SKY_FREEZE:		send_to_char("{g[{YWeather{g] {CThe air is misty with an intense cold.{x\n\r",ch); break;
		case SKY_ICESTORM:		send_to_char("{g[{YWeather{g] {CSheets of ice appear to be falling from the sky.{x\n\r",ch); break;
	}
	printf_to_char(ch,"{B[{W Time  {B] {RIt is {M%d{R o'clock {Y%s{x\n\r",(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,time_info.hour >= 12 ? "pm" : "am");
	return;
}

void show_weather(CHAR_DATA *ch)
{
	char buf[MSL];
	int npercent,climate = ch->in_room->area->climate;

	switch (current_weather[climate])
	{
		case SKY_RAINING:
			npercent = number_percent();
			if (npercent <= 15 && climate != CLIMATE_ARID)			ch->send("{g[{YWeather{g] {CThere is a cold rain trickling down.{x\n\r");
			else if (npercent <= 30  && climate != CLIMATE_FRIGID)	ch->send("{g[{YWeather{g] {CThere is a warm rain trickling down.{x\n\r");
			break;
		case SKY_CLOUDLESS:		send_to_char("{g[{YWeather{g] {CThe sky is beautiful, not a cloud around.{x\n\r",ch); break;
		case SKY_CLOUDY:		printf_to_char(ch,"{g[{YWeather{g] %s{x.\n\r", weather_info.change >= 0 ? "{CA warm breeze can be felt about" : "{CA cold breeze can be felt about" ); break;
		case SKY_WINDY:			send_to_char("{g[{YWeather{g] {CThere is a wind blowing through.{x\n\r",ch); break;
		case SKY_LIGHTNING:		send_to_char("{g[{YWeather{g] {CA {y Lightning{w storm is approaching {x\n\r",ch); break;
		case SKY_SNOWING:		send_to_char("{g[{YWeather{g] {CA light snow is falling.{x\n\r",ch); break;
		case SKY_BLIZZARD:		send_to_char("{g[{YWeather{g] {CThere is a blizzard about.{x\n\r",ch); break;
		case SKY_FOGGY:			send_to_char("{g[{YWeather{g] {CA misty haze covers the horizon.{x\n\r",ch); break;
		case SKY_HAILSTORM:		send_to_char("{g[{YWeather{g] {CGolfball like substances are falling from the sky.{x\n\r",ch); break;
		case SKY_THUNDERSTORM:	send_to_char("{g[{YWeather{g] {CThe skies thunder as a storm approaches.{x\n\r",ch); break;
		case SKY_SANDSTORM:		send_to_char("{g[{YWeather{g] {CSands blast about in a furious storm.{x\n\r",ch); break;
		case SKY_HEATWAVE:		send_to_char("{g[{YWeather{g] {CWaves of heat envelope everything.{x\n\r",ch); break;
		case SKY_FREEZE:		send_to_char("{g[{YWeather{g] {CThe air is misty with an intense cold.{x\n\r",ch); break;
		case SKY_ICESTORM:		send_to_char("{g[{YWeather{g] {CSheets of ice appear to be falling from the sky.{x\n\r",ch); break;
		default: send_to_char("{g[{YWeather{g] {CBUG!!!!!!!!!!!!!!! A descriptor does not exist for this condition  {x\n\r",ch); break;
	}
}

void lightning( void )
{
	DESCRIPTOR_DATA *d;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if (d->connected == CON_PLAYING && IS_OUTSIDE(d->character) && IS_AWAKE(d->character) && np_morethan(95) && current_weather[d->character->in_room->area->climate] == SKY_LIGHTNING)
		{
			send_to_char("{RYou see a brilliant surge of lightning racing towards you from the heavens and everything goes black!{x\n\r",d->character);
			act( "$n has been struck by lightning!", d->character, NULL, NULL,TO_ROOM);
			if (!d->character->res[RS_LIGHTNING] == 0)
			{
				if(d->character->fighting)
					stop_fighting(d->character,true);
				d->character->hit /= 2;
				d->character->position = POS_STUNNED;
				WAIT_STATE(d->character,40); 
			}
		}
	}
}
void blizzard( void )
{
	DESCRIPTOR_DATA *d;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING && IS_OUTSIDE( d->character ) && current_weather[d->character->in_room->area->climate] == SKY_BLIZZARD )
		{
			if ( number_range( 0, 2000 ) <= 1000  )
				send_to_char("{RThe sky before you is a mist of white blur. Perhaps you should find a safe place indoors.{x \n\r",d->character);
			else if ( number_range( 0, 2000 ) <= 1250  )
			{
				send_to_char("{RNot being able to see where you are going, you trip over a rock.{x \n\r",d->character );
				act( "$n falls face first into the oncoming drifts!", d->character, NULL, NULL,TO_ROOM);
				damage(d->character,d->character,UMIN(d->character->hit,15),TYPE_HIT,DAM_OTHER,true);
			}
			else if ( number_range( 0, 2000 ) <= 1500  )
			{
				send_to_char("{RYou've managed to slide yourself right into a tree. Good Job. Better hope you didn't break anything!{x \n\r",d->character);
				act( "$n has planted themselves headfirst into a tree. Encore!", d->character, NULL, NULL,TO_ROOM);
				damage(d->character,d->character,UMIN(d->character->hit,15),TYPE_HIT,DAM_OTHER,true);
			}
			else if ( number_range( 0, 2000 ) <= 1950  )
				send_to_char("{RThe sky before you is a mist of white blur. Perhaps you should find a safe place indoors.{x \n\r",d->character);
			else if ( number_range( 0, 2000 ) <= 2000  )
			{
				send_to_char("{RYour body twitches and your limbs start to freeze one by one.{x \n\r", d->character);
				act( "You watch as $n's limbs start to freeze !", d->character, NULL, NULL,TO_ROOM);
				damage(d->character,d->character,UMIN(d->character->hit,15),TYPE_HIT,DAM_OTHER,true);
			}
		}
	}
}

void ice(void)
{
	DESCRIPTOR_DATA *d;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING && IS_OUTSIDE( d->character ) && current_weather[d->character->in_room->area->climate] == SKY_ICESTORM )
		{
			if ( number_range( 0, 2000 ) <= 1000  )
				send_to_char("{RIt's starting to rain sheets of ice. Perhaps you should find a way inside{x \n\r",d->character);
			else if ( number_range( 0, 2000 ) <= 1250  )
			{
				send_to_char("{RThe ice around your feet firms up and causes you to fall  flat on your face{x \n\r",d->character );
				act( "$n falls face first into the ground!", d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 5;
			}
			else if ( number_range( 0, 2000 ) <= 1500  )
			{
				send_to_char("{RYou've managed to slide yourself right into a tree. Good Job. Better hope you didn't break anything!{x \n\r",d->character);
				act( "$n has planted themselves headfirst into a tree. Encore!", d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 15;
			}
			else if ( number_range( 0, 2000 ) <= 1950  )
				d->character->send("{RThe sky before you is a mist of white blur. Perhaps you should find a safe place indoors.{x \n\r");
			else if ( number_range( 0, 2000 ) <= 2000  )
			{
				d->character->send("{RYour body twitches and your limbs start to freeze one by one.{x \n\r");
				act( "You watch as $n's limbs start to freeze !", d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 30;
			}
		}
	}
}
void hail( void )
{
	DESCRIPTOR_DATA *d;
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
		if ( d->connected == CON_PLAYING && IS_OUTSIDE( d->character ) && current_weather[d->character->in_room->area->climate] == SKY_HAILSTORM )
		{
			if ( number_range( 0, 2000 ) <= 1000  )
				send_to_char("{RWas that a golfball or hail? It might be a good idea to  find yourself a way indoors quickly.{x \n\r",d->character);
			else if ( number_range( 0, 2000 ) <= 1250  )
			{
				send_to_char("{ROh MAN! You were just hit in the face by hail!{x\n\r",d->character );
				act( "You watch in ammusement as $n is hit in the face by a baby iceball  ", d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 5;
			}
			else if ( number_range( 0, 2000 ) <= 1950  )
				send_to_char("{RThe sky before you is a mist of white blur. Perhaps you should find a safe place indoors.{x \n\r",d->character);
			else if ( number_range( 0, 2000 ) <= 2000  )
			{
				send_to_char("{RYour body twitches and your limbs start to freeze one by one.{x \n\r", d->character);
				act( "You watch as $n's limbs start to freeze !", d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 30;
			}
		}
	}
}

void fog( void ){
	DESCRIPTOR_DATA *d;
	for(d = descriptor_list;d;d = d->next){
		if ( d->connected == CON_PLAYING && IS_OUTSIDE( d->character ) && current_weather[d->character->in_room->area->climate] == SKY_FOGGY )
		{
			/*if ( number_range( 0, 2000 ) <= 1000  )
				send_to_char("{RThe morning fog is as thick as pea soup. Perhaps you should find your way indoors.{x \n\r",d->character);
			else if ( number_range( 0, 2000 ) <= 1250  )
			{
				send_to_char("{RNot being able to see where you are going, you slip and fall into a hole.{x \n\r",d->character );
				act( "$n falls face first into a hole!", d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 5;
			}
			else if ( number_range( 0, 2000 ) <= 1500  )
			{
				send_to_char("{RYou've walked straight into a tree! Way to go!!{x \n\r",d->character);
				act( "$n has planted themselves headfirst into a tree. Encore!", d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 15;
			}
			else if ( number_range( 0, 2000 ) <= 1950  )
				send_to_char("{RThe morning fog is as thick as pea soup. Perhaps you should find your way indoors.{x \n\r",d->character);
			else if ( number_range( 0, 2000 ) <= 2000  )
			{
				send_to_char("{RNot being able to see where you're going, you fall straight into a river.{x \n\r", d->character);
				act( "$n pulls $m clothes out of the river after falling in!",d->character, NULL, NULL,TO_ROOM);
				d->character->hit -= 30;
			}*/
		}
	}
}

void do_wset( CHAR_DATA *ch, char *argument )
{
	char arg1 [MIL];
	argument = one_argument( argument, arg1 );
	if ( arg1[0] == '\0' )
	{
		ch->send("Syntax:\n\r"
				 "  set weather <condition> \n\r"
				 "  Condition can be :\n\r"
				 "   hail, fog, icestorm, blizzard, snowing,  rain\n\r"
				 "   lightning, thunderstorm, cloudless, cloudy  \n\r");
		return;
	}
	if(!str_cmp(arg1,"cloudless"))
	{
		send_to_char( "You wave your hands and in reverence to you, the clouds dissapear \n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_CLOUDLESS;
		act( "$n makes a strange movement with their hands and the clouds part.", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"fog"))
	{
		send_to_char( "You wave your hands and in reverence to you, a mist vapors the horizon \n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_FOGGY;
		act( "$n makes a strange movement with their hands and a mist vapors the horizon.", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"hail"))
	{
		send_to_char( "You wave your hands and in reverence to you, hailstones fall from the sky \n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_HAILSTORM;
		act( "$n makes a strange movement with their hands and hailstones fall from the sky.", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"icestorm"))
	{
		send_to_char( "You wave your hands and in reverence to you, it starts raining ice \n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_ICESTORM;
		act( "$n makes a strange movement with their hands and it starts raining ice.", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"blizzard"))
	{
		send_to_char( "You wave your hands and in reverence to you, snowflakes cover the horizon making it impossible to see.\n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_BLIZZARD;
		act( "$n makes a strange movement with their hands and snowflakes cover the horizon making it impossibile to see", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"snowing"))
	{
		send_to_char( "You wave your hands and in reverence to you, snowflakes  fall from the sky..\n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_SNOWING;
		act( "$n makes a strange movement with their hands and snowflakes fall from the sky", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"rain"))
	{
		send_to_char( "You wave your hands and in reverence to you, a warm rain starts to fall.\n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_RAINING;
		act( "$n makes a strange movement with their hands and a warm rain starts to fall", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"lightning"))
	{
		send_to_char( "You wave your hands and in reverence to you, lightning pierces the sky. \n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_LIGHTNING;
		act( "$n makes a strange movement with their hands and lightning pierces  the sky", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"thunderstorm"))
	{
		send_to_char( "You wave your hands and in reverence to you, The clouds clap in thunder.\n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_THUNDERSTORM;
		act( "$n makes a strange movement with their hands and the clouds clap in thunder", ch, NULL, NULL, TO_ROOM);
	}
	else if(!str_cmp(arg1,"cloudy"))
	{
		send_to_char( "You wave your hands and in reverence to you, clouds cover the horizon, threatening rain.\n\r", ch);
		current_weather[ch->in_room->area->climate] = SKY_CLOUDY;
		act( "$n makes a strange movement with their hands and clouds cover the horizon, threatening rain", ch, NULL, NULL, TO_ROOM);
	}
	/* okay, we haven't met any conditions so far. Did they forget? */
	else
	{
		send_to_char( "Syntax:\n\r",ch);
		send_to_char( "  set weather <condition> \n\r", ch);
		send_to_char( "  Condition can be :\n\r",ch);
		send_to_char("   hail, fog, icestorm, blizzard, snowing,  rain\n\r",ch);
		send_to_char("   lightning, thunderstorm, cloudless, cloudy  \n\r",ch);
		return;
	}
}

void writeout_weather(int wclimate,char *buf)
{
	DESCRIPTOR_DATA *d;
	if (buf[0] != '\0')
		for ( d = descriptor_list; d != NULL; d = d->next )
			if (d->connected == CON_PLAYING && IS_OUTSIDE(d->character) && d->character->in_room->area->climate == wclimate )
				act(buf,d->character,NULL,NULL,TO_CHAR);
}

void set_weather(int wclimate,int wtype,char *message)
{
	char buf[MSL];
	buf[0] = '\0';
	current_weather[wclimate] = wtype;
	sprintf(buf,"%s",message);
    if (buf[0]) writeout_weather(wclimate,buf);
}

void arid_weather_handler(){
	char buf[MSL];
	int climate = CLIMATE_ARID;

	buf[0] = '\0';
	/* Weather change. */
	switch (current_weather[climate]){
	default:
		log_string("Arid weather bug.");
		break;
	case SKY_CLOUDLESS:
		if (np_lessthan(10) &&  time_info.hour <= 6)							set_weather(climate,SKY_FOGGY,"A thick fog rolls over the landscape, obscuring the clear horizon.");
		else if (np_lessthan(15))												set_weather(climate,SKY_CLOUDY,"Shadows roam the ground as the clear sky above becomes dotted with clouds.");
		else if (np_lessthan(5))												set_weather(climate,SKY_RAINING,"The clear day quickly becomes dark and rainy as the thunderclouds roll in from the horizon.");
		else if (np_lessthan(10) && time_info.hour > 7 && time_info.hour < 19)	set_weather(climate,SKY_HEATWAVE,"A searing wind sucks the moisture from the air, leaving it hot and arid.");
		else if (np_lessthan(10) && time_info.hour < 6 && time_info.hour > 20)	set_weather(climate,SKY_FREEZE,"A biting wind chills the bones as the clear day becomes dangerously cold.");
		else if (np_lessthan(10))												set_weather(climate,SKY_SANDSTORM,"A strong wind stirs up massive amounts of sand, obscuring the clear sky with a stinging blanket of earth.");
		else if (np_lessthan(20))												set_weather(climate,SKY_WINDY,"A pleasant breeze sweeps across the ground, making the clear day even nicer.");
		else if (np_lessthan(15))												strcat(buf, "It is a nice day.");
		break;
	case SKY_HEATWAVE:
		if (np_lessthan(25))													set_weather(climate,SKY_CLOUDLESS,"The heat dissipates and the day becomes clear and pleasant.");
		else if (np_lessthan(25))												set_weather(climate,SKY_CLOUDY,"Clouds rolling in from the horizon provide some shade from the blazing heat of the sun.");
		else if (np_lessthan(5))												set_weather(climate,SKY_RAINING,"The heat of the day is abated as thunderclouds roll in, bringing cooling rain with them.");
		else if (np_lessthan(5) && time_info.hour < 7)							set_weather(climate,SKY_FOGGY,"The humidity in the air, made thick by the heat, quickly condenses into a foul fog.");
		else if (np_lessthan(35))												strcat(buf,"The heat of the atmosphere continues unabated.");
		break;
	case SKY_FREEZE:
		if (np_lessthan(25))													set_weather(climate,SKY_CLOUDLESS,"Warmth returns to the air, making the day clear and pleasant.");
		else if (np_lessthan(25))												set_weather(climate,SKY_CLOUDY,"The clouds appearing in the sky blot out the sky, removing what little warmth there was from the already freezing atmosphere.");
		else if (np_lessthan(5))												set_weather(climate,SKY_RAINING,"Thunderclouds roll in, pelting the earth with freezing rain.");
		else if (np_lessthan(5) && time_info.hour < 7)							set_weather(climate,SKY_FOGGY,"The cold air condenses the moisture in the air, blotting out the horizon with a thick fog.");
		else if (np_lessthan(35))												strcat(buf,"The freezing air feels as though the earth will never warm again.");
		break;
	case SKY_WINDY:
		if (np_lessthan(25))													set_weather(climate,SKY_CLOUDLESS,"The high wind sweeps the clouds from the sky and dies.");
		else if (np_lessthan(45))												set_weather(climate,SKY_SANDSTORM,"The rushing wind stirs up a dangerous sandstorm.");
		else if (np_lessthan(35))												strcat(buf,"The wind continues to batter the limbs of trees.");
		break;
	case SKY_SANDSTORM:
		if (np_lessthan(15))													set_weather(climate,SKY_THUNDERSTORM,"A thunderstorm rolls in, and the rain weighs down the driving sandstorm.");
		else if (np_lessthan(35))												set_weather(climate,SKY_WINDY,"The wind lessens to a light breeze, dropping the sand it carries to the ground.");
		else if (np_lessthan(30))												set_weather(climate,SKY_CLOUDLESS,"The sandstorm dies down, revealing a beautifully clear sky above.");
		else if (np_lessthan(5))												set_weather(climate,SKY_RAINING,"sandstorm to rain.");
		else if (np_lessthan(25))												set_weather(climate,SKY_SANDSTORM,"The wind dies momentarily, then rises again, stirring up more sand into a storm.");
		else if (np_lessthan(45))												strcat(buf,"The pelting sand, blown about by the wind, seems to show no indication of stopping.");
		break;
	case SKY_CLOUDY:
		if (np_lessthan(15))													set_weather(climate,SKY_THUNDERSTORM,"The clouds thicken and crash as a thunderstorm rears its head.");
		else if (np_lessthan(45))												set_weather(climate,SKY_CLOUDLESS,"The clouds meander across the horizon, leaving clear sky behind.");
		else if (np_lessthan(5))												set_weather(climate,SKY_RAINING,"The clouds release a light shower to moisten the earth.");
		else if (np_lessthan(35))												set_weather(climate,SKY_WINDY,"The clouds move quickly away as a steady wind picks up.");
		else if (np_lessthan(20))												set_weather(climate,SKY_SANDSTORM,"The clouds in the sky are blotted from view as the wind kicks up a violent sandstorm.");
		else if (np_lessthan(45))												strcat(buf,"Clouds continue to meander across the sky.");
		break;
	case SKY_RAINING:
		if (np_lessthan(15)){													set_weather(climate,SKY_LIGHTNING,"The rains pick up and become a thunderstorm, with lightning flashing across the sky.");lightning ();}
		else if (np_lessthan(10))												set_weather(climate,SKY_THUNDERSTORM,"The rains pick up and become a thunderstorm, with lightning flashing across the sky.");
		else if (np_lessthan(40))												set_weather(climate,SKY_CLOUDY,"The rain dissipates, leaving only clouds left in the sky.");
		else if (np_lessthan(45))												strcat(buf,"The rain continues to fall, threatening to flood the area.");
		break;
	case SKY_LIGHTNING:
		if (np_lessthan(25))													set_weather(climate,SKY_THUNDERSTORM,"The lightning comes to a stop, but the loud thunder continues to crackle in the sky.");
		else if (np_lessthan(5))												set_weather(climate,SKY_RAINING,"The electric storm breaks, releasing its rain onto the earth.");
		else if (np_lessthan(25))												set_weather(climate,SKY_CLOUDY,"The lightning ceases, leaving only clouds in the sky.");
		else if (np_lessthan(45))												strcat(buf,"The electric storm continues unabated.");
		break;
	case SKY_FOGGY:
		if (np_lessthan(25))													set_weather(climate,SKY_CLOUDY,"The fog seems to lift up, revealing the ground but dotting the sky with clouds.");
		else if (np_lessthan(55))												fog();
		break;
	case SKY_THUNDERSTORM:
		if (np_lessthan(5))														set_weather(climate,SKY_RAINING,"The thunder abates, and the clouds begin releasing rain upon the earth.");
		else if (np_lessthan(5))												set_weather(climate,SKY_CLOUDY,"The loud thunder subsides, leaving only silent, heavy clouds in the sky.");
		else if (np_lessthan(25))												set_weather(climate,SKY_LIGHTNING,"As the thunderstorm picks up in intensity, flashes of lightning start to lance through the sky.");
		else if (np_lessthan(35))												set_weather(climate,SKY_SANDSTORM,"The crackling thunderstorm dies down, but the rough winds create a raging sandstorm.");
		else if (np_lessthan(40))												strcat(buf,"thunder.");
		break;
	}
    if (buf[0]) writeout_weather(climate,buf);
}

void temperate_weather_handler()
{
	char buf[MSL];
	int climate = CLIMATE_TEMPERATE;

	buf[0] = '\0';
	/*
	 * Weather change.
	 */
	switch (current_weather[climate])
	{
	default:
		log_string("Temperate weather bug.");
		break;
	case SKY_CLOUDLESS:
		if (np_lessthan(30) &&  time_info.hour <= 6)							set_weather(climate,SKY_FOGGY,"Fog quickly fills the air, blotting out the clear skies.");
		else if (np_lessthan(25))												set_weather(climate,SKY_CLOUDY,"Some clouds start drifting by up in the clear sky.");
		else if (np_lessthan(10))												set_weather(climate,SKY_RAINING,"The clear skies are suddenly filled with grey clouds bearing rain.");
		else if (np_lessthan(5))												set_weather(climate,SKY_WINDY,"A strong wind picks up, shaking the clear sky.");
		else if (np_lessthan(35))												strcat(buf,"The sky is cloudless and perfect.");
		break;
	case SKY_WINDY:
		if (np_lessthan(25))													set_weather(climate,SKY_CLOUDLESS,"The winds seem to calm to a gentle breeze.");
		else if (np_lessthan(35))												strcat(buf,"The world seems to breath deeply as a wind blows through.");
		break;
	case SKY_CLOUDY:
		if (np_lessthan(10))													set_weather(climate,SKY_THUNDERSTORM,"The clouds darken, and a fierce thunder starts to rumble in the sky.");
		else if (np_lessthan(35))												set_weather(climate,SKY_CLOUDLESS,"The clouds slowly dissipate, leaving the sky clear.");
		else if (np_lessthan(15))												set_weather(climate,SKY_RAINING,"The clouds thicken and darken, and rain starts to fall to the ground.");
		else if (np_lessthan(25))												set_weather(climate,SKY_WINDY,"The clouds are pushed away by a strong wind.");
		else if (np_lessthan(45))												strcat(buf,"Clouds silently roam the sky.");
		break;
	case SKY_RAINING:
		if (np_lessthan(15)){													set_weather(climate,SKY_LIGHTNING,"The rain lightens, and a fierce lightning storm arcs through the sky.");lightning ();}
		else if (np_lessthan(10))												set_weather(climate,SKY_THUNDERSTORM,"The loud clap of thunder fills the sky as the rain slowly abates.");
		else if (np_lessthan(40))												set_weather(climate,SKY_CLOUDY,"The rain lightens and then stops, leaving only thick clouds in the sky.");
		else if (np_lessthan(4))												set_weather(climate,SKY_SNOWING,"As the air grows colder, the rain slowly becomes snowflakes.");
		else if (np_lessthan(45))												strcat(buf,"rain.");
		break;
	case SKY_LIGHTNING:
		if (np_lessthan(25))													set_weather(climate,SKY_THUNDERSTORM,"lightning to thunder.");
		else if (np_lessthan(15))												set_weather(climate,SKY_RAINING,"lightning to rain.");
		else if (np_lessthan(25))												set_weather(climate,SKY_CLOUDY,"lightning to clouds.");
		else if (np_lessthan(45))												strcat(buf,"lightning.");
		break;
	case SKY_FOGGY:
		if (np_lessthan(25) || time_info.hour > 6)								set_weather(climate,SKY_CLOUDY,"fog to clouds.");
		else if (np_lessthan(55))												fog();
		break;
	case SKY_SNOWING:
		if (np_lessthan(45))													set_weather(climate,SKY_CLOUDY,"snow to clouds.");
		else if (np_lessthan(25))												set_weather(climate,SKY_FOGGY,"snow to fog.");
		break;
	case SKY_THUNDERSTORM:
		if (np_lessthan(5))														set_weather(climate,SKY_RAINING,"thunder to rain.");
		else if (np_lessthan(15))												set_weather(climate,SKY_CLOUDY,"thunder to clouds.");
		else if (np_lessthan(25))												set_weather(climate,SKY_LIGHTNING,"thunder to lightning.");
		else if (np_lessthan(40))												strcat(buf,"thunder.");
		break;
	}
    if (buf[0]) writeout_weather(climate,buf);
}

void tropical_weather_handler()
{
	char buf[MSL];
	int climate = CLIMATE_TROPICAL;

	buf[0] = '\0';
	/*
	 * Weather change.
	 */
	switch (current_weather[climate])
	{
	default:
		log_string("Tropical weather bug.");
		break;
	case SKY_CLOUDLESS:
		if (np_lessthan(10) &&  time_info.hour <= 6)							set_weather(climate,SKY_FOGGY,"nocloud to The fog slowly rolls in.");
		else if (np_lessthan(25))												set_weather(climate,SKY_CLOUDY,"nocloud to Some clouds start drifting by up in the sky.");
		else if (np_lessthan(10))												set_weather(climate,SKY_RAINING,"nocloud to It begins raining lightly.");
		else if (np_lessthan(5))												set_weather(climate,SKY_WINDY,"nocloud to wind.");
		else if (np_lessthan(55))												set_weather(climate,SKY_RAINING,"nocloud to rain.");
		else if (np_lessthan(35))												strcat(buf,"no clouds!");
		break;
	case SKY_WINDY:
		if (np_lessthan(25))													set_weather(climate,SKY_CLOUDLESS,"wind to The clouds dissipateness.");
		else if (np_lessthan(35))												set_weather(climate,SKY_RAINING,"wind to rain.");
		else if (np_lessthan(35))												strcat(buf,"The world seems to breath deeply as the wind blows through.");
		break;
	case SKY_CLOUDY:
		if (np_lessthan(10))													set_weather(climate,SKY_THUNDERSTORM,"cloud to Thunder claps about like a madman.");
		else if (np_lessthan(35))												set_weather(climate,SKY_CLOUDLESS,"cloud to The clouds dissipateness.");
		else if (np_lessthan(65))												set_weather(climate,SKY_RAINING,"cloud to rain.");
		else if (np_lessthan(25))												set_weather(climate,SKY_WINDY,"cloud to wind.");
		else if (np_lessthan(45))												strcat(buf,"clouds.");
		break;
	case SKY_RAINING:
		if (np_lessthan(25)){													set_weather(climate,SKY_LIGHTNING,"rain to Lightning flashes its goods.");lightning ();}
		else if (np_lessthan(20))												set_weather(climate,SKY_THUNDERSTORM,"rain to Thunder go boom.");
		else if (np_lessthan(10))												set_weather(climate,SKY_CLOUDY,"Rain stops but clouds stay.");
		else if (np_lessthan(45))												set_weather(climate,SKY_RAINING,"Rain.");
		break;
	case SKY_LIGHTNING:
		if (np_lessthan(25))													set_weather(climate,SKY_THUNDERSTORM,"lightning to thunder.");
		else if (np_lessthan(35))												set_weather(climate,SKY_RAINING,"lightning to rain.");
		else if (np_lessthan(25))												set_weather(climate,SKY_CLOUDY,"lightning to clouds.");
		else if (np_lessthan(45))												set_weather(climate,SKY_LIGHTNING,"Lightning.");
		break;
	case SKY_FOGGY:
		if (np_lessthan(25))													set_weather(climate,SKY_CLOUDY,"fog to clouds.");
		else if (np_lessthan(65))												set_weather(climate,SKY_RAINING,"fog to rain.");
		else if (np_lessthan(55))												fog();
		break;
	case SKY_THUNDERSTORM:
		if (np_lessthan(45))														set_weather(climate,SKY_RAINING,"thunder to rain.");
		else if (np_lessthan(15))												set_weather(climate,SKY_CLOUDY,"thunder to clouds.");
		else if (np_lessthan(25))												set_weather(climate,SKY_LIGHTNING,"thunder to lightning.");
		else if (np_lessthan(40))												strcat(buf,"thunder.");
		break;
	}
    if (buf[0]) writeout_weather(climate,buf);
}

void frigid_weather_handler()
{
	char buf[MSL];
	int climate = CLIMATE_FRIGID,n = number_percent();

	buf[0] = '\0';
	/*
	 * Weather change.
	 */
	switch (current_weather[climate])
	{
	default:
		log_string("Frigid weather bug.");
		break;
	case SKY_CLOUDLESS:
			 if (n < 10 &&  time_info.hour <= 6)		set_weather(climate,SKY_FOGGY,"A cold fog calmly crawls into the area.");
		else if (n < 25)								set_weather(climate,SKY_CLOUDY,"Thin layered clouds begin to form and spread across the sky.");
		else if (n < 35)								set_weather(climate,SKY_RAINING,"Light rain begins to fall from the cloudless sky.");
		else if (n < 40)								set_weather(climate,SKY_WINDY,"The light air is quickly forms into a cold strong breeze.");
		else if (n < 60)								set_weather(climate,SKY_SNOWING,"A cold breeze flows through as light snow begins to fall.");
		else if (n < 85)								strcat(buf,"no clouds!");
		break;
	case SKY_WINDY:
			 if (n < 20)								set_weather(climate,SKY_CLOUDLESS,	"The wind dies down and the clouds dissolve, leaving the sky calm and cloudless.");
		else if (n < 40)								set_weather(climate,SKY_RAINING,	"wind to rain.");
		else if (n < 60)								set_weather(climate,SKY_CLOUDY,		"wind to cloud.");
		else if (n < 85)								set_weather(climate,SKY_SNOWING,	"Rain to snow.");
		else if (n < 99)								strcat(buf,"The freezing wind howls as it blows through.");
		break;
	case SKY_CLOUDY:
			 if (n < 10)								set_weather(climate,SKY_THUNDERSTORM,"cloud to Thunder claps about like a madman.");
		else if (n < 25)								set_weather(climate,SKY_CLOUDLESS,	"cloud to The clouds dissipateness.");
		else if (n < 35)								set_weather(climate,SKY_RAINING,	"cloud to rain.");
		else if (n < 75)								set_weather(climate,SKY_SNOWING,	"cloud to snow.");
		else if (n < 85)								set_weather(climate,SKY_WINDY,		"cloud to wind.");
		else if (n < 99)								strcat(buf,"clouds.");
		break;
	case SKY_RAINING:
			 if (n < 25){								set_weather(climate,SKY_LIGHTNING,	"rain to Lightning flashes its goods.");lightning ();}
		else if (n < 45)								set_weather(climate,SKY_RAINING,	"Rain.");
		else if (n < 60)								set_weather(climate,SKY_FOGGY,		"Rain to snow.");
		else if (n < 20)								set_weather(climate,SKY_THUNDERSTORM,"rain to Thunder go boom.");
		else if (n < 10)								set_weather(climate,SKY_CLOUDY,		"Rain stops but clouds stay.");
		else if (n < 60)								set_weather(climate,SKY_SNOWING,	"Rain to snow.");
		break;
	case SKY_LIGHTNING:
			 if (n < 25)								set_weather(climate,SKY_THUNDERSTORM,"lightning to thunder.");
		else if (n < 35)								set_weather(climate,SKY_RAINING,	"lightning to rain.");
		else if (n < 55)								set_weather(climate,SKY_CLOUDY,		"lightning to clouds.");
		else if (n < 80)								set_weather(climate,SKY_SNOWING,	"lightning to snow.");
		else if (n < 99)								set_weather(climate,SKY_LIGHTNING,	"Lightning.");
		break;
	case SKY_FOGGY:
			 if (n < 25)								set_weather(climate,SKY_CLOUDY,		"fog to clouds.");
		else if (n < 45)								set_weather(climate,SKY_RAINING,	"fog to rain.");
		else if (n < 70)								set_weather(climate,SKY_SNOWING,	"Rain to snow.");
		else if (n < 99)								fog();
		break;
	case SKY_THUNDERSTORM:
			 if (n < 15)								set_weather(climate,SKY_RAINING,	"thunder to rain.");
		else if (n < 35)								set_weather(climate,SKY_CLOUDY,		"thunder to clouds.");
		else if (n < 55)								set_weather(climate,SKY_LIGHTNING,	"thunder to lightning.");
		else if (n < 80)								set_weather(climate,SKY_SNOWING,	"Rain to snow.");
		else if (n < 99)								strcat(buf,"thunder.");
		break;
	case SKY_SNOWING:
			 if (n <  5)								set_weather(climate,SKY_RAINING,	"snow to rain.");
		else if (n < 15)								set_weather(climate,SKY_CLOUDY,		"snow to clouds.");
		else if (n < 20)								set_weather(climate,SKY_LIGHTNING,	"snow to lightning.");
		else if (n < 35)								set_weather(climate,SKY_BLIZZARD,	"snow to blizzard.");
		else if (n < 40)								set_weather(climate,SKY_HAILSTORM,	"snow to hail.");
		else if (n < 50)								set_weather(climate,SKY_FREEZE,		"snow to freeze.");
		else if (n < 60)								set_weather(climate,SKY_ICESTORM,	"snow to sleet.");
		else if (n < 70)								set_weather(climate,SKY_FOGGY,		"snow to foggy.");
		else if (n < 99)								strcat(buf,"snow.");
		break;
	case SKY_BLIZZARD:
			 if (n <  5)								set_weather(climate,SKY_RAINING,	"blizzard to rain.");
		else if (n < 15)								set_weather(climate,SKY_CLOUDY,		"blizzard to clouds.");
		else if (n < 20)								set_weather(climate,SKY_LIGHTNING,	"blizzard to lightning.");
		else if (n < 50)								set_weather(climate,SKY_SNOWING,	"blizzard to snow.");
		else if (n < 55)								set_weather(climate,SKY_HAILSTORM,	"blizzard to hail.");
		else if (n < 65)								set_weather(climate,SKY_FREEZE,		"blizzard to freeze.");
		else if (n < 75)								set_weather(climate,SKY_ICESTORM,	"blizzard to icestorm.");
		else if (n < 80)								set_weather(climate,SKY_FOGGY,		"blizzard to fog.");
		else if (n < 99)								strcat(buf,"blizzard.");
		break;
	case SKY_HAILSTORM:
			 if (n <  5)								set_weather(climate,SKY_RAINING,	"hail to rain.");
		else if (n < 30)								set_weather(climate,SKY_CLOUDY,		"hail to clouds.");
		else if (n < 35)								set_weather(climate,SKY_LIGHTNING,	"hail to lightning.");
		else if (n < 40)								set_weather(climate,SKY_FOGGY,		"hail to fog.");
		else if (n < 50)								set_weather(climate,SKY_SNOWING,	"hail to snow.");
		else if (n < 60)								set_weather(climate,SKY_BLIZZARD,	"hail to blizzard.");
		else if (n < 70)								set_weather(climate,SKY_FREEZE,		"hail to freeze.");
		else if (n < 80)								set_weather(climate,SKY_ICESTORM,	"hail to sleet.");
		else if (n < 99)								strcat(buf,"hail.");
		break;
	case SKY_FREEZE:
			 if (n <  5)								set_weather(climate,SKY_RAINING,	"freeze to rain.");
		else if (n < 15)								set_weather(climate,SKY_CLOUDY,		"freeze to clouds.");
		else if (n < 20)								set_weather(climate,SKY_LIGHTNING,	"freeze to lightning.");
		else if (n < 40)								set_weather(climate,SKY_FOGGY,		"freeze to fog.");
		else if (n < 55)								set_weather(climate,SKY_SNOWING,	"freeze to snow.");
		else if (n < 70)								set_weather(climate,SKY_BLIZZARD,	"freeze to blizzard.");
		else if (n < 75)								set_weather(climate,SKY_HAILSTORM,	"freeze to hail.");
		else if (n < 85)								set_weather(climate,SKY_ICESTORM,	"freeze to sleet.");
		else if (n < 99)								strcat(buf,"frozen shit.");
		break;
	case SKY_ICESTORM:
			 if (n <  5)								set_weather(climate,SKY_RAINING,	"icestorm to rain.");
		else if (n < 15)								set_weather(climate,SKY_CLOUDY,		"icestorm to clouds.");
		else if (n < 20)								set_weather(climate,SKY_LIGHTNING,	"icestorm to lightning.");
		else if (n < 30)								set_weather(climate,SKY_FOGGY,		"icestorm to fog.");
		else if (n < 60)								set_weather(climate,SKY_SNOWING,	"icestorm to snow.");
		else if (n < 70)								set_weather(climate,SKY_BLIZZARD,	"icestorm to blizzard.");
		else if (n < 75)								set_weather(climate,SKY_HAILSTORM,	"icestorm to hail.");
		else if (n < 85)								set_weather(climate,SKY_FREEZE,		"icestorm to freeze.");
		else if (n < 99)								strcat(buf,"sleet.");
		break;
	}
    if (buf[0]) writeout_weather(climate,buf);
}

void desolate_weather_handler()
{
	char buf[MSL];
	int climate = CLIMATE_DESOLATE;

	buf[0] = '\0';
	/*
	 * Weather change.
	 */
	if (np_lessthan(40))
		strcat(buf,"Eerie wind.");
    if (buf[0]) writeout_weather(climate,buf);
}

void ocean_weather_handler()
{
	char buf[MSL];
	int climate = CLIMATE_OCEAN,n = number_percent();

	buf[0] = '\0';
	/*
	 * Weather change.
	 */
	switch (current_weather[climate])
	{
	default:
		log_string("Ocean weather bug.");
		break;
	case SKY_CLOUDLESS:
			 if (n < 10 &&  time_info.hour <= 6)		set_weather(climate,SKY_FOGGY,"nocloud to The fog slowly rolls in.");
		else if (n < 25)								set_weather(climate,SKY_CLOUDY,"nocloud to Some clouds start drifting by up in the sky.");
		else if (n < 35)								set_weather(climate,SKY_RAINING,"nocloud to It begins raining lightly.");
		else if (n < 40)								set_weather(climate,SKY_WINDY,"nocloud to wind.");
		else if (n < 85)								strcat(buf,"no clouds!");
		break;
	case SKY_WINDY:
			 if (n < 20)								set_weather(climate,SKY_CLOUDLESS,	"wind to The clouds dissipateness.");
		else if (n < 40)								set_weather(climate,SKY_RAINING,	"wind to rain.");
		else if (n < 60)								set_weather(climate,SKY_CLOUDY,		"wind to cloud.");
		else if (n < 95)								strcat(buf,"The cool wind buffets everything in its path.");
		break;
	case SKY_CLOUDY:
			 if (n < 10)								set_weather(climate,SKY_THUNDERSTORM,"cloud to Thunder claps about like a madman.");
		else if (n < 25)								set_weather(climate,SKY_CLOUDLESS,	"cloud to The clouds dissipateness.");
		else if (n < 65)								set_weather(climate,SKY_RAINING,	"cloud to rain.");
		else if (n < 85)								set_weather(climate,SKY_WINDY,		"cloud to wind.");
		else if (n < 95)								strcat(buf,"clouds.");
		break;
	case SKY_RAINING:
			 if (n < 25){								set_weather(climate,SKY_LIGHTNING,	"rain to Lightning flashes its goods.");lightning ();}
		else if (n < 45)								set_weather(climate,SKY_RAINING,	"Rain.");
		else if (n < 50)								set_weather(climate,SKY_FOGGY,		"Rain to snow.");
		else if (n < 65)								set_weather(climate,SKY_THUNDERSTORM,"rain to Thunder go boom.");
		else if (n < 75)								set_weather(climate,SKY_CLOUDY,		"Rain stops but clouds stay.");
		else											strcat(buf,"rain.");
		break;
	case SKY_LIGHTNING:
			 if (n < 25)								set_weather(climate,SKY_THUNDERSTORM,"lightning to thunder.");
		else if (n < 35)								set_weather(climate,SKY_RAINING,	"lightning to rain.");
		else if (n < 55)								set_weather(climate,SKY_CLOUDY,		"lightning to clouds.");
		else if (n < 99)								set_weather(climate,SKY_LIGHTNING,	"Lightning.");
		break;
	case SKY_FOGGY:
			 if (n < 25)								set_weather(climate,SKY_CLOUDY,		"fog to clouds.");
		else if (n < 45)								set_weather(climate,SKY_RAINING,	"fog to rain.");
		else if (n < 75)								fog();
		break;
	case SKY_THUNDERSTORM:
			 if (n < 25)								set_weather(climate,SKY_RAINING,	"thunder to rain.");
		else if (n < 30)								set_weather(climate,SKY_CLOUDY,		"thunder to clouds.");
		else if (n < 55)								set_weather(climate,SKY_LIGHTNING,	"thunder to lightning.");
		else if (n < 85)								strcat(buf,"thunder.");
		break;
	case SKY_FREEZE:
			 if (n < 15)								set_weather(climate,SKY_RAINING,	"freeze to rain.");
		else if (n < 25)								set_weather(climate,SKY_CLOUDY,		"freeze to clouds.");
		else if (n < 30)								set_weather(climate,SKY_LIGHTNING,	"freeze to lightning.");
		else if (n < 40)								set_weather(climate,SKY_FOGGY,		"freeze to fog.");
		else if (n < 60)								set_weather(climate,SKY_WINDY,		"freeze to wind.");
		else if (n < 80)								strcat(buf,"frozen shit.");
		break;
	}
    if (buf[0]) writeout_weather(climate,buf);
}
