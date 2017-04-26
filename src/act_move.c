#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

bool	canFight		( CHAR_DATA*,CHAR_DATA*,bool );
bool	IS_NATURE_BG	( CHAR_DATA* );
void	check_catnap	( CHAR_DATA* );
int		move_trek		( CHAR_DATA*,int );
bool	check_stamina	( CHAR_DATA* );
void check_frost_field ( CHAR_DATA* );
void check_alarm		(CHAR_DATA*);
char * const dir_name [] ={
    "north", "east", "south", "west", "up", "down"
};
char * const rdir_name [] ={
    "the north", "the east", "the south", "the west", "above", "below"
};
const sh_int rev_dir [] ={
    2, 3, 0, 1, 5, 4
};

/*
 * Local functions.
 */
int		find_door	( CHAR_DATA*,char* );
bool	has_key		( CHAR_DATA*,int );


void follow_char(CHAR_DATA *ch,int door,ROOM_INDEX_DATA *old_in_room){
	CHAR_DATA *fch,*fch_next;
	for (fch = old_in_room->people;fch;fch = fch_next){
		fch_next = fch->next_in_room;

		if(fch->master == ch && (fch->isaff(AF_CHARM) || fch == ch->mount) && fch->position < POS_STANDING)
			do_function(fch,&do_stand,"");

		if(fch->master == ch && IS_NPC(ch) && ch->isoff(OF_WAIT))
			continue;
		if(fch->isaff(AF_STALK) && fch->fighting && fch->fighting == ch)
			continue;

		if (fch->master == ch && fch->position == POS_STANDING){
			if (IS_SET(ch->in_room->room_flags,ROOM_LAW) && (IS_NPC(fch) && (fch->isact(AT_AGGRESSIVE) || fch->isact(AT_AGGRESSIVE_MOB)))){
				act("You can't bring $N into the city.",ch,NULL,fch,TO_CHAR);
				act("You aren't allowed in the city.",fch,NULL,NULL,TO_CHAR);
				continue;
			}

			act("You follow $N.",fch,NULL,ch,TO_CHAR);
			move_char(fch,door,true,true);
		}
	}
}

bool validate_movement(CHAR_DATA *ch,EXIT_DATA *pexit,int door,bool cSee,bool follow,ROOM_INDEX_DATA *old_in_room,ROOM_INDEX_DATA *to_room){
	if(follow && ch->fighting){
		ch->send("You're fighting!\n\r");
		return false;
	}
	if(ch->isaff(AF_SNARED) || ch->isaff(AF_ENTANGLED)){
		send_to_char("You're snared to the ground!\n\r", ch);
		return false;
	}
	if (!IS_NPC(ch) && (p_exit_trigger(ch,door,PRG_MPROG) || p_exit_trigger(ch,door,PRG_OPROG) || p_exit_trigger(ch,door,PRG_RPROG)))
		return false;
	if (IS_SET(pexit->exit_info,EX_CLOSED)
	&& (!ch->isaff(AF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS) /*|| number_range(1,6) > affect_find(ch->affected,gsn_phase)->modifier*/)
	&& !IS_TRUSTED(ch,IMMORTAL)){
		if(!IS_SET(pexit->exit_info,EX_QUIET)){
			if(!IS_NPC(ch) && IS_DRUNK(ch) && number_percent() < ch->pcdata->condition[COND_DRUNK]){
				ch->send("You forget how to operate a doorknob and try to walk through the closed door.\n\r");
				act("$n fumbles with the door and then proceeds to walk into the closed door!",ch,NULL,NULL,TO_ROOM);
			}
			else
				act("The $d is closed.",ch,NULL,pexit->keyword,TO_CHAR);
		}
		return false;
	}
	if (IS_SET(pexit->exit_info,EX_CLIFFTOP)){
		send_to_char("Your better judgement stops you.\n\r",ch);
		return false;
	}
	if (ch->isaff(AF_CHARM) && ch->master != NULL && old_in_room == ch->master->in_room){
		send_to_char("What?  And leave your beloved master?\n\r",ch);
		return false;
	}
	if (IS_SET(to_room->room_flags,ROOM_NEWBIES_ONLY) && ch->level > 9 && !IS_IMMORTAL(ch)){
		send_to_char("You are not a newbie.\n\r",ch);
		return false;
	}
	if (!is_room_owner(ch,to_room) && room_is_private(to_room)){
		send_to_char("That room is private right now.\n\r",ch);
		return false;
	}
	return true;
}

int crunch_move(CHAR_DATA *ch,ROOM_INDEX_DATA *old_in_room,ROOM_INDEX_DATA *to_room){
	AFFECT_DATA *af;
	int move = (sect_table[old_in_room->sector_type].mvdeduction + sect_table[to_room->sector_type].mvdeduction) / 2;

	if(check_durability(ch))
		move /= 2;

	//WAIT_STATE(ch,1); Maybe
	if (!MOUNTED(ch)){
		if(!IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] < 10)
			move *= 1.5;
		if(!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] < 10)
			move *= 2;

		if(ch->isaff(AF_ETHEREAL_ALACRITY) && (af = affect_find(ch->affected,AF_ETHEREAL_ALACRITY))){
			move /= af->slvl + 1;
			ch->modmana(-1);
		}
		if (ch->isaff(AF_SLOW))
			move *= 1.5;
		if (ch->isaff(AF_POISON))
			move *= 1.5;
		if (ch->isaff(AF_PLAGUE))
			move *= 1.5;

		if (ch->move < move){
			if((ch->hit * 100 / ch->max_hit) > 10 || !check_stamina(ch)){
				send_to_char("You are too exhausted.\n\r",ch);
				return -1;
			}
			else
				ch->send("You trudge on, ignoring your injuries.\n\r");
		}
		move = move_trek(ch,move);
		if(ch->isaff(AF_WINDWALK))
			move = 0;
		ch->move -= move;
	}
	else{
		if (MOUNTED(ch)->isaff(AF_FLYING) || MOUNTED(ch)->isaff(AF_HASTE))
			move /= 2;

		if (MOUNTED(ch)->isaff(AF_SLOW))
			move *= 1.5;
		if (MOUNTED(ch)->isaff(AF_POISON))
			move *= 1.5;
		if (MOUNTED(ch)->isaff(AF_PLAGUE))
			move *= 1.5;

		if (MOUNTED(ch)->move < move){
			send_to_char("Your mount is too exhausted.\n\r",ch);
			return -1;
		}
		if(MOUNTED(ch)->isaff(AF_WINDWALK))
			move = 0;
		MOUNTED(ch)->move -= move / 2; //Horses walk gooder than people
	}
	return move;
}

void print_leaving(CHAR_DATA *ch,int door){//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!CODE IN alertness and awareness and unsneaking
	char ch_name[100];

	sprintf(ch_name,"%s",IS_NPC(ch)? capitalize(ch->short_descr) : ch->morph ? ch->short_descr : ch->name);

	for(CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room){
		if(vch == ch || (ch->isaff(AF_PERFECTSNEAK) && !vch->isplr(PL_HOLYLIGHT)))
			 continue;
		if(!can_see_move(vch,ch)){//alertness or whatever
			if(vch->isaff(AF_ALERTNESS))
				vch->printf("You feel someone leave to the %s.\n\r",dir_name[door]);
		}
		else{//Visible?!
			if(ch->isaff(AF_SNEAK) || ch->isaff(AF_CAMOFLAGE)){
				if(ch->isaff(AF_FLYING))
					printf_to_char(vch,"%s floats stealthily %s.\n\r",ch_name,dir_name[door]);
				else
					printf_to_char(vch,"%s sneaks %s.\n\r",ch_name,dir_name[door]);
			}
			else if(ch->isaff(AF_PERFECTSNEAK)){
				if(ch->isaff(AF_FLYING))
					printf_to_char(vch,"%s floats stealthily in the {dshadows{x %s.\n\r",ch_name,dir_name[door]);
				else
					printf_to_char(vch,"%s sneaks in the {dshadows{x %s.\n\r",ch_name,dir_name[door]);
			}
			else{
				if(MOUNTED(ch)){
					if((MOUNTED(ch)->isaff(AF_FLYING)))
						vch->printf("%s soars %s, riding on %s.\n\r",ch_name,dir_name[door],MOUNTED(ch)->short_descr);
					else
						vch->printf("%s leaves %s, riding on %s.\n\r",ch_name,dir_name[door],MOUNTED(ch)->short_descr);
				}
				else{
					if(ch->isaff(AF_FLYING))
						vch->printf("%s soars %s.\n\r",ch_name,dir_name[door]);
					else
						vch->printf("%s leaves %s.\n\r",ch_name,dir_name[door]);
				}
			}
		}
	}
	if(is_affected(ch,gsn_desertcover) && ch->in_room->sector_type != SECT_DESERT){
		affect_strip(ch,gsn_desertcover);
		send_to_char("You become visible as you leave the sands.\n\r",ch);
	}
	else if (is_affected(ch,gsn_blend) && !IS_NATURE_BG(ch)){
		affect_strip(ch,gsn_blend);
		send_to_char("You become visible as you leave nature.\n\r",ch);
	}
}

void print_arriving(CHAR_DATA *ch,int door){
	char ch_name[100];

	sprintf(ch_name,"%s",IS_NPC(ch)? capitalize(ch->short_descr) : ch->morph ? ch->short_descr : ch->name);

	for(CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room){
		if(vch == ch || (ch->isaff(AF_PERFECTSNEAK) && !vch->isplr(PL_HOLYLIGHT)))
			 continue;
		if(!can_see(vch,ch)){//alertness or whatever
			if(vch->isaff(AF_ALERTNESS))
				vch->printf("You feel movement from %s.\n\r",rdir_name[rev_dir[door]]);
		}
		else{//Visible?!
			if(ch->isaff(AF_SNEAK) || ch->isaff(AF_CAMOFLAGE)){
				if(ch->isaff(AF_FLYING))
					printf_to_char(vch,"%s floats stealthily in from %s.\n\r",ch_name,rdir_name[rev_dir[door]]);
				else
					printf_to_char(vch,"%s sneaks in from %s.\n\r",ch_name,rdir_name[rev_dir[door]]);
			}
			else if(ch->isaff(AF_PERFECTSNEAK)){
				if(ch->isaff(AF_FLYING))
					printf_to_char(vch,"%s floats stealthily in from the {dshadows{x from %s.\n\r",ch_name,rdir_name[rev_dir[door]]);
				else
					printf_to_char(vch,"%s sneaks in from the {dshadows{x from %s.\n\r",ch_name,rdir_name[rev_dir[door]]);
			}
			else{
				if(MOUNTED(ch)){
					if((MOUNTED(ch)->isaff(AF_FLYING)))
						vch->printf("%s soars in from %s, on %s.\n\r",ch_name,rdir_name[rev_dir[door]],MOUNTED(ch)->short_descr);
					else
						vch->printf("%s has arrived from %s, riding on %s.\n\r",ch_name,rdir_name[rev_dir[door]],MOUNTED(ch)->short_descr);
				}
				else{
					if(ch->isaff(AF_FLYING))
						vch->printf("%s soars in from %s.\n\r",ch_name,rdir_name[rev_dir[door]]);
					else
						vch->printf("%s has arrived from %s.\n\r",ch_name,rdir_name[rev_dir[door]]);
				}
			}
		}
	}
}

void move_char(CHAR_DATA *ch,int door,bool follow,bool cSee){
	CHAR_DATA *fch, *fch_next;
	ROOM_INDEX_DATA *old_in_room, *to_room;
	EXIT_DATA *pexit;
	char buf[MSL];
	int iClass, iGuild, move;

	if (door < 0 || door > 5){
		bug("Do_move: bad door %d.",door);
		return;
	}
	if(!IS_NPC(ch) && IS_DRUNK(ch) && number_percent() < ch->pcdata->condition[COND_DRUNK]){
		ch->send("You stumble and forget which way you were going.\n\r");
		door = number_range(0,5);
	}

	old_in_room = ch->in_room;

	if (!(pexit = old_in_room->exit[door])){
		if(!IS_NPC(ch) && IS_DRUNK(ch)){
			if(number_percent() < ch->pcdata->condition[COND_DRUNK] / 3){
				ch->send("You drunkenly decide to get close and personal with the local walls... WHAM!!!\n\r");
				act("$n, in $s drunken stupor, decides to get friendly with the local walls in a painful faceplant!",ch,NULL,NULL,TO_ROOM);
			}
			else if(number_percent() < ch->pcdata->condition[COND_DRUNK] / 3){
				ch->send("You stumble about and drunkenly trip over yourself.\n\r");
				act("$n stumbles aimlessly before drunkenly tripping over $mself, falling down.",ch,NULL,NULL,TO_ROOM);
				ch->position = POS_RESTING;
				return;
			}
			else if(number_percent() < ch->pcdata->condition[COND_DRUNK] / 3){
				ch->send("You try to continue but gravity gets the last hurrah and you fall into a drunken slumber.\n\r");
				act("$n staggers a few steps before flopping over, snoring loudly.",ch,NULL,NULL,TO_ROOM);
				ch->position = POS_SLEEPING;
				return;
			}
		}
		else
			ch->send("Alas, you cannot go that way.\n\r");
		return;
	}

	if (MOUNTED(ch)){
		if (MOUNTED(ch)->position < POS_FIGHTING){
			send_to_char("Your mount must be standing.\n\r",ch);
			return;
		}
/*		if (!mount_success(ch,MOUNTED(ch),false)){
			send_to_char("Your mount subbornly refuses to go that way.\n\r",ch);
			return;
		}*/
	}
	if(IS_SET(pexit->exit_info,EX_RANDOM)){
		if(!(to_room = get_random_room_area(ch))){
			log_f("%s killed movement with random exits.",ch->name);
			return;
		}
	}

	if(!(to_room = pexit->u1.to_room)){
		send_to_char("Alas, you cannot go that way(DEAD ROOM!).\n\r",ch);
		return;
	}

	if(!validate_movement(ch,pexit,door,cSee,follow,old_in_room,to_room))
		return;

	if (!IS_NPC(ch)){
		if (old_in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR || to_room->sector_type == SECT_CLIFFSIDE){
			if (MOUNTED(ch)){
				if(!MOUNTED(ch)->isaff(AF_FLYING)){
					send_to_char("Your mount can't fly.\n\r",ch);
					return;
				}
			}
			else if (!ch->isaff(AF_FLYING) && !IS_IMMORTAL(ch)){
				send_to_char("You can't fly.\n\r",ch);
				return;
			}
		}

		if ((old_in_room->sector_type == SECT_WATER_NOSWIM || to_room->sector_type == SECT_WATER_NOSWIM)){
			if ((MOUNTED(ch) && !MOUNTED(ch)->isaff(AF_FLYING)) || !ch->isaff(AF_FLYING)){
				OBJ_DATA *obj;
				bool found;

				found = false;

				if (IS_IMMORTAL(ch))
					found = true;

				for ( obj = ch->carrying; obj && !found; obj = obj->next_content )
					if (obj->item_type == ITEM_BOAT)
						found = true;
				if (!found){
					send_to_char("You need a boat to go there.\n\r",ch);
					return;
				}
			}
		}

		if((move = crunch_move(ch,old_in_room,to_room)) < 0)
			return;
	}

	if (RIDDEN(ch)){
		CHAR_DATA *rch;
		rch = RIDDEN(ch);

/*		if (!mount_success(rch,ch,false)){
			act("Your mount escapes your control, and leaves $T.",rch,NULL,dir_name[door],TO_CHAR);
			if (RIDDEN(ch))
				ch = RIDDEN(ch);
		}*/
	}

	if(ch->isaff(AF_STALK) && follow)
		cSee = false;

	if (cSee)
		print_leaving(ch,door);
	if(ch->in_room->israf(RAF_QUAGMIRE)){
		ch->send("The quagmire in this room slows you down!\n\r");
		WAIT_STATE(ch,24);
	}

	if ((ch->mount != NULL && (!IS_NPC(ch) || !ch->isact(AT_MOUNT))) && ch->mount->in_room == ch->in_room) //Move the horsie
		move_char(ch->mount,door,true,false);

	if (old_in_room == to_room) /* no circular follows */
		return;

	strip_moveskill(ch);
	char_from_room(ch);
	char_to_room(ch,to_room);

	print_arriving(ch,door);


	do_function(ch,&do_look,"auto");
	check_catnap(ch);

	if(ch->isaff(AF_LETHARGY)) WAIT_STATE(ch,24);
	if(ch->in_room->israf(RAF_QUAGMIRE)){
		ch->send("The quagmire in this room slows you down!\n\r");
		WAIT_STATE(ch,24);
	}

	if (old_in_room == to_room) /* no circular follows */
		return;

	//aggro stuff
	for (fch = ch->in_room->people; fch != NULL; fch = fch->next_in_room){
		if (IS_NPC(fch) && fch->isact(AT_AGGRESSIVE) && ch->level < LEVEL_IMMORTAL && !IS_NPC(ch) && can_see(fch,ch))
			do_function(fch,&do_kill,ch->name);
		else if (IS_NPC(fch) && fch->isact(AT_AGGRESSIVE_MOB) && ch->level < LEVEL_IMMORTAL && IS_NPC(ch) && can_see(fch,ch)){
			CHAR_DATA *vch;
			bool isPCS = false;
			for (vch = fch->in_room->people; vch != NULL && !isPCS; vch = vch->next_in_room){
				if (!IS_NPC(vch))
					isPCS = true;
			}
			if (isPCS){
				if(ch->isaff(AF_AMIABLE) && number_percent() < 50)
					ch->send("Your friendliness saves you.\n\r");
				else
					do_function(fch,&do_kill,ch->name);
			}
		}
	}
follow_char(ch,door,old_in_room);

	/*  If someone is following the char, these triggers get activated for the followers before the char, but it's safer this way...*/
	if (IS_NPC(ch) && HAS_TRIGGER_MOB(ch,TRIG_ENTRY))
		p_percent_trigger(ch,NULL,NULL,NULL,NULL,NULL,TRIG_ENTRY);
	if (!IS_NPC(ch)){
		check_traps(ch,NULL);
		check_alarm(ch);
		p_greet_trigger(ch,PRG_MPROG);
		p_greet_trigger(ch,PRG_OPROG);
		p_greet_trigger(ch,PRG_RPROG);
	}
	if(ch->in_room && ch->in_room->israf(RAF_FROSTFIELD))
		check_frost_field(ch);
}

void do_north(CHAR_DATA *ch,char *argument){
	if(ch->fighting){
		ch->send("You're fighting!!!\n\r");
		return;
	}move_char(ch,DIR_NORTH,false,true);return;}
void do_east(CHAR_DATA *ch,char *argument){
	if(ch->fighting){
		ch->send("You're fighting!!!\n\r");
		return;
	}move_char(ch,DIR_EAST,false,true);return;}
void do_south(CHAR_DATA *ch,char *argument){
	if(ch->fighting){
		ch->send("You're fighting!!!\n\r");
		return;
	}move_char(ch,DIR_SOUTH,false,true);return;}
void do_west(CHAR_DATA *ch,char *argument){
	if(ch->fighting){
		ch->send("You're fighting!!!\n\r");
		return;
	}move_char(ch,DIR_WEST,false,true);return;}
void do_up(CHAR_DATA *ch,char *argument){
	if(ch->fighting){
		ch->send("You're fighting!!!\n\r");
		return;
	}move_char(ch,DIR_UP,false,true);return;}
void do_down(CHAR_DATA *ch,char *argument){
	if(ch->fighting){
		ch->send("You're fighting!!!\n\r");
		return;
	}move_char(ch,DIR_DOWN,false,true);return;}

int find_door(CHAR_DATA *ch,char *arg)
{
    EXIT_DATA *pexit;
    int door;

		 if (!str_cmp(arg,"n") || !str_cmp(arg,"north")) door = 0;
    else if (!str_cmp(arg,"e") || !str_cmp(arg,"east" )) door = 1;
    else if (!str_cmp(arg,"s") || !str_cmp(arg,"south")) door = 2;
    else if (!str_cmp(arg,"w") || !str_cmp(arg,"west") ) door = 3;
    else if (!str_cmp(arg,"u") || !str_cmp(arg,"up")   ) door = 4;
    else if (!str_cmp(arg,"d") || !str_cmp(arg,"down") ) door = 5;
    else
    {
		for ( door = 0; door <= 5; door++ )
			if ((pexit = ch->in_room->exit[door]) != NULL && IS_SET(pexit->exit_info,EX_ISDOOR) && pexit->keyword != NULL && is_name(arg,pexit->keyword))
				return door;
		//act("I see no $T here.",ch,NULL,arg,TO_CHAR);
		return -1;
    }

    if ( (pexit = ch->in_room->exit[door]) == NULL )
    {
		//act("I see no door $T here.",ch,NULL,arg,TO_CHAR);
		return -1;
    }

    if (!IS_SET(pexit->exit_info,EX_ISDOOR))
    {
		send_to_char("You can't do that.\n\r",ch);
		return -1;
    }

    return door;
}

void do_open(CHAR_DATA *ch,char *argument){
    char arg[MIL];
    OBJ_DATA *obj;
    int door;

    one_argument(argument,arg);

    if (arg[0] == '\0'){
		send_to_char("Open what?\n\r",ch);
		return;
    }

    if ((door = find_door(ch,arg)) >= 0)
    {
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit, *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info,EX_CLOSED)){
			send_to_char("It's already open.\n\r",ch);
			return;
		}
		if (IS_SET(pexit->exit_info,EX_LOCKED)){
			send_to_char("It's locked.\n\r",ch);
			return;
		}

		REMOVE_BIT(pexit->exit_info,EX_CLOSED);
		act("$n opens the $d.",ch,NULL,pexit->keyword,TO_ROOM);
		act("You open the $d.",ch,NULL,pexit->keyword,TO_CHAR);

		/* open the other side */
		if ((to_room = pexit->u1.to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL && pexit_rev->u1.to_room == ch->in_room){
			CHAR_DATA *rch;

			REMOVE_BIT(pexit_rev->exit_info,EX_CLOSED);
			for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
				act("The $d opens.",rch,NULL,pexit_rev->keyword,TO_CHAR);
		}
		return;
    }
	else if ((obj = get_obj_here(ch,NULL,arg)) != NULL)
    {
 		/* open portal */
		if (obj->item_type == ITEM_PORTAL)
		{
			if (!IS_SET(obj->value[1],EX_ISDOOR)){
				send_to_char("You can't do that.\n\r",ch);
				return;
			}

			if (!IS_SET(obj->value[1],EX_CLOSED)){
				send_to_char("It's already open.\n\r",ch);
				return;
			}

			if (IS_SET(obj->value[1],EX_LOCKED)){
				send_to_char("It's locked.\n\r",ch);
				return;
			}

			REMOVE_BIT(obj->value[1],EX_CLOSED);
			act("You open $p.",ch,obj,NULL,TO_CHAR);
			act("$n opens $p.",ch,obj,NULL,TO_ROOM);
			return;
 		}

		/* 'open object' */
		if (obj->item_type != ITEM_CONTAINER){
			send_to_char("That's not a container.\n\r",ch);
			return;
		}
		if (!IS_SET(obj->value[1],CONT_CLOSED)){
			send_to_char("It's already open.\n\r",ch);
			return;
		}
		if (!IS_SET(obj->value[1],CONT_CLOSEABLE)){
			send_to_char("You can't do that.\n\r",ch);
			return;
		}
		if (IS_SET(obj->value[1],CONT_LOCKED)){
			send_to_char("It's locked.\n\r",ch);
			return;
		}

		REMOVE_BIT(obj->value[1],CONT_CLOSED);
		act("You open $p.",ch,obj,NULL,TO_CHAR);
		act("$n opens $p.",ch,obj,NULL,TO_ROOM);
		return;
    }
	else
		ch->send("There's nothing like that here.\n\r");

    return;
}

void do_close(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	OBJ_DATA *obj;
	int door;

	one_argument(argument,arg);

	if(!arg[0]){
		ch->send("Close what?\n\r");
		return;
	}

	if((door = find_door(ch,arg)) >= 0){
		/* 'close door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit,*pexit_rev;

		pexit = ch->in_room->exit[door];
		if(IS_SET(pexit->exit_info,EX_CLOSED)){
			send_to_char("It's already closed.\n\r",ch);
			return;
		}

		SET_BIT(pexit->exit_info,EX_CLOSED);
		act("$n closes the $d.",ch,NULL,pexit->keyword,TO_ROOM);
		act("You close the $d.",ch,NULL,pexit->keyword,TO_CHAR);

		/* close the other side */
		if((to_room = pexit->u1.to_room) && (pexit_rev = to_room->exit[rev_dir[door]]) != 0 && pexit_rev->u1.to_room == ch->in_room){
			CHAR_DATA *rch;

			SET_BIT(pexit_rev->exit_info,EX_CLOSED);
			for(rch = to_room->people;rch;rch = rch->next_in_room)
				act("The $d closes.",rch,NULL,pexit_rev->keyword,TO_CHAR);
		}
		return;
	}
	else if((obj = get_obj_here(ch,NULL,arg))){
		if (obj->item_type == ITEM_PORTAL){//Portal junk
			if(!IS_SET(obj->value[1],EX_ISDOOR) || IS_SET(obj->value[1],EX_NOCLOSE)){
				send_to_char("You can't do that.\n\r",ch);
				return;
			}

			if(IS_SET(obj->value[1],EX_CLOSED)){
				send_to_char("It's already closed.\n\r",ch);
				return;
			}

			SET_BIT(obj->value[1],EX_CLOSED);
			act("You close $p.",ch,obj,NULL,TO_CHAR);
			act("$n closes $p.",ch,obj,NULL,TO_ROOM);
			return;
		}

		/* 'close object' */
		if(obj->item_type != ITEM_CONTAINER){
			send_to_char("That's not a container.\n\r",ch);
			return;
		}
		if(IS_SET(obj->value[1],CONT_CLOSED)){
			send_to_char("It's already closed.\n\r",ch);
			return;
		}
		if(!IS_SET(obj->value[1],CONT_CLOSEABLE)){
			send_to_char("You can't do that.\n\r",ch);
			return;
		}

		SET_BIT(obj->value[1],CONT_CLOSED);
		act("You close $p.",ch,obj,NULL,TO_CHAR);
		act("$n closes $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	else
		ch->send("There's nothing like that here.\n\r");
}

bool has_key(CHAR_DATA *ch,int key){
	OBJ_DATA *obj;

	for(obj = ch->carrying;obj;obj = obj->next_content)
		if(obj->pIndexData->vnum == key)
			return true;
	return false;
}

void do_lock(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	OBJ_DATA *obj;
	int door;

	one_argument(argument,arg);

	if (arg[0] == '\0'){
		send_to_char("Lock what?\n\r",ch);
		return;
	}
	if (MOUNTED(ch)){
		send_to_char("You can't reach the lock from your mount.\n\r",ch);
		return;
	}

	if ((door = find_door(ch,arg)) >= 0){
		/* 'lock door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit, *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info,EX_CLOSED)){
			send_to_char("It's not closed.\n\r",ch);
			return;
		}
		if (pexit->key < 0){
			send_to_char("It can't be locked.\n\r",ch);
			return;
		}
		if (!has_key(ch,pexit->key)){
			send_to_char("You lack the key.\n\r",ch);
			return;
		}
		if (IS_SET(pexit->exit_info,EX_LOCKED)){
			send_to_char("It's already locked.\n\r",ch);
			return;
		}

		SET_BIT(pexit->exit_info,EX_LOCKED);
		send_to_char("*Click*\n\r",ch);
		act("$n locks the $d.",ch,NULL,pexit->keyword,TO_ROOM);

		/* lock the other side */
		if ((to_room = pexit->u1.to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != 0 && pexit_rev->u1.to_room == ch->in_room)
			SET_BIT(pexit_rev->exit_info,EX_LOCKED);
		return;
	}
	else if ((obj = get_obj_here(ch,NULL,arg)) != NULL){/* portal stuff */
		if (obj->item_type == ITEM_PORTAL){
			if (!IS_SET(obj->value[1],EX_ISDOOR) || IS_SET(obj->value[1],EX_NOCLOSE)){
				send_to_char("You can't do that.\n\r",ch);
				return;
			}
			if (!IS_SET(obj->value[1],EX_CLOSED)){
				send_to_char("It's not closed.\n\r",ch);
 				return;
			}

			if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK)){
				send_to_char("It can't be locked.\n\r",ch);
				return;
			}

			if (!has_key(ch,obj->value[4])){
				send_to_char("You lack the key.\n\r",ch);
				return;
			}

			if (IS_SET(obj->value[1],EX_LOCKED)){
				send_to_char("It's already locked.\n\r",ch);
				return;
			}

			SET_BIT(obj->value[1],EX_LOCKED);
			act("You lock $p.",ch,obj,NULL,TO_CHAR);
			act("$n locks $p.",ch,obj,NULL,TO_ROOM);
			return;
		}

		/* 'lock object' */
		if (obj->item_type != ITEM_CONTAINER){
			send_to_char("That's not a container.\n\r",ch);
			return;
		}
		if (!IS_SET(obj->value[1],CONT_CLOSED)){
			send_to_char("It's not closed.\n\r",ch);
			return;
		}
		if (obj->value[2] < 0){
			send_to_char("It can't be locked.\n\r",ch);
			return;
		}
		if (!has_key(ch,obj->value[2])){
			send_to_char("You lack the key.\n\r",ch);
			return;
		}
		if (IS_SET(obj->value[1],CONT_LOCKED)){
			send_to_char("It's already locked.\n\r",ch);
			return;
		}

		SET_BIT(obj->value[1],CONT_LOCKED);
		act("You lock $p.",ch,obj,NULL,TO_CHAR);
		act("$n locks $p.",ch,obj,NULL,TO_ROOM);
		return;
	}
	else
		ch->send("There's nothing like that here.\n\r");
}

void do_unlock(CHAR_DATA *ch,char *argument)
{
    char arg[MIL];
    OBJ_DATA *obj;
    int door;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
		send_to_char("Unlock what?\n\r",ch);
		return;
    }
    if (MOUNTED(ch))
    {
        send_to_char("You can't reach the lock from your mount.\n\r",ch);
        return;
    }

    if ((door = find_door(ch,arg)) >= 0)
    {
		/* 'unlock door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit, *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info,EX_CLOSED))
		{
			send_to_char("It's not closed.\n\r",ch);
			return;
		}
		if (pexit->key < 0)
		{
			send_to_char("It can't be unlocked.\n\r",ch);
			return;
		}
		if (!has_key(ch,pexit->key))
		{
			send_to_char("You lack the key.\n\r",ch);
			return;
		}
		if (!IS_SET(pexit->exit_info,EX_LOCKED))
		{
			send_to_char("It's already unlocked.\n\r",ch);
			return;
		}

		REMOVE_BIT(pexit->exit_info,EX_LOCKED);
		send_to_char("*Click*\n\r",ch);
		act("$n unlocks the $d.",ch,NULL,pexit->keyword,TO_ROOM);

		/* unlock the other side */
		if ((to_room = pexit->u1.to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL && pexit_rev->u1.to_room == ch->in_room)
			REMOVE_BIT(pexit_rev->exit_info,EX_LOCKED);
    }
	else if ((obj = get_obj_here(ch,NULL,arg)) != NULL)
    {
 		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{
			if (!IS_SET(obj->value[1],EX_ISDOOR))
			{
				send_to_char("You can't do that.\n\r",ch);
				return;
			}

			if (!IS_SET(obj->value[1],EX_CLOSED))
			{
				send_to_char("It's not closed.\n\r",ch);
				return;
			}

			if (obj->value[4] < 0)
			{
				send_to_char("It can't be unlocked.\n\r",ch);
				return;
			}

			if (!has_key(ch,obj->value[4]))
			{
				send_to_char("You lack the key.\n\r",ch);
				return;
			}

			if (!IS_SET(obj->value[1],EX_LOCKED))
			{
				send_to_char("It's already unlocked.\n\r",ch);
				return;
			}

			REMOVE_BIT(obj->value[1],EX_LOCKED);
			act("You unlock $p.",ch,obj,NULL,TO_CHAR);
			act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
			return;
		}

		/* 'unlock object' */
		if (obj->item_type != ITEM_CONTAINER)
		{
			send_to_char("That's not a container.\n\r",ch);
			return;
		}
		if (!IS_SET(obj->value[1],CONT_CLOSED))
		{
			send_to_char("It's not closed.\n\r",ch);
			return;
		}
		if (obj->value[2] < 0)
		{
			send_to_char("It can't be unlocked.\n\r",ch);
			return;
		}
		if (!has_key(ch,obj->value[2]))
		{
			send_to_char("You lack the key.\n\r",ch);
			return;
		}
		if (!IS_SET(obj->value[1],CONT_LOCKED))
		{
			send_to_char("It's already unlocked.\n\r",ch);
			return;
		}

		REMOVE_BIT(obj->value[1],CONT_LOCKED);
		act("You unlock $p.",ch,obj,NULL,TO_CHAR);
		act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
		return;
    }
	else
		ch->send("There's nothing like that here.\n\r");

    return;
}

void do_stand(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj = NULL;

	if (ch->isaff(AF_ENTANGLED)){
		send_to_char("You can't fight entangled like that!\n\r", ch);
		return;
	}
	if (ch->isaff(AF_SLEEP) || ch->isaff(AF_SAP)){
		send_to_char("You can't wake up!\n\r",ch);
		return;
	}

	if (argument[0]){
		if (ch->position == POS_FIGHTING){
			if(ch->bashwait > 0){
				act("$n stands up.",ch,NULL,NULL,TO_ROOM);
				act("You stand up.",ch,NULL,NULL,TO_CHAR);
				ch->bashwait = 0;
				return;
			}
			else{
				send_to_char("Maybe you should finish fighting first?\n\r",ch);
				return;
			}
		}
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (!obj){
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
		if (obj->item_type != ITEM_FURNITURE || (!IS_SET(obj->value[2],STAND_AT) && !IS_SET(obj->value[2],STAND_ON) && !IS_SET(obj->value[2],STAND_IN))){
			send_to_char("You can't seem to find a place to stand.\n\r",ch);
			return;
		}
		if (ch->on != obj && count_users(obj) >= obj->value[0]){
			act_new("There's no room to stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
			return;
		}
		ch->position = POS_STANDING;
		ch->bashwait = 0;
 		ch->on = obj;
		if (IS_SET(obj->value[2],STAND_AT)){
			act("You stand at $p.",ch,obj,NULL,TO_CHAR);
			act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
		}
		else if (IS_SET(obj->value[2],STAND_ON)){
			act("You stand on $p.",ch,obj,NULL,TO_CHAR);
			act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
		}
		else{
			act("You stand in $p.",ch,obj,NULL,TO_CHAR);
			act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
		}
		if (HAS_TRIGGER_OBJ(obj,TRIG_SIT))
			p_percent_trigger(NULL,obj,NULL,ch,NULL,NULL,TRIG_SIT);
		return;
	}
	if (ch->bashwait > 0)
	{
		act("$n stands up.",ch,NULL,NULL,TO_ROOM);
		act("You stand up.",ch,NULL,NULL,TO_CHAR);
		ch->position = POS_STANDING;
		ch->bashwait = 0;
		return;
	}

	switch (ch->position){
		case POS_SLEEPING:
			if (!obj){
				send_to_char("You wake and stand up.\n\r",ch);
				act("$n wakes and stands up.",ch,NULL,NULL,TO_ROOM);
				ch->on = NULL;
			}
			else if (IS_SET(obj->value[2],STAND_AT)){
			   act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
			   act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],STAND_ON)){
				act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
				act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],STAND_IN)){
				act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
				act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
			}
			ch->position = POS_STANDING;
			do_function(ch,&do_look,"auto");
			break;
		case POS_RESTING:
		case POS_SITTING:
			if (!(obj = ch->on)){
				send_to_char("You stand up.\n\r",ch);
				act("$n stands up.",ch,NULL,NULL,TO_ROOM);
				ch->position = POS_STANDING;
				ch->on = NULL;
				if(ch->bashwait > 0)
					ch->bashwait=0;
				break;
			}
			else if (IS_SET(obj->value[2],STAND_AT)){
				act("You stand at $p.",ch,obj,NULL,TO_CHAR);
				act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],STAND_ON)){
				act("You stand on $p.",ch,obj,NULL,TO_CHAR);
				act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],STAND_IN)){
				act("You stand in $p.",ch,obj,NULL,TO_CHAR);
				act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
			}
			else{
				act("You stand up.",ch,obj,NULL,TO_CHAR);
				act("$n stands up.",ch,obj,NULL,TO_ROOM);
				ch->on = NULL;
			}
			ch->position = POS_STANDING;
			break;
		case POS_STANDING:
			if (!(obj = ch->on)){
				send_to_char("You are already standing.\n\r",ch);
				ch->on = NULL;
				if(ch->bashwait > 0)
					ch->bashwait=0;
				break;
			}
			ch->on = NULL;
			act("You step off $p.",ch,obj,NULL,TO_CHAR);
			act("$n steps off $p.",ch,obj,NULL,TO_ROOM);
			break;
		case POS_FIGHTING:
			if(ch->bashwait > 0){
				act("$n stands up.",ch,NULL,NULL,TO_ROOM);
				act("You stand up.",ch,NULL,NULL,TO_CHAR);
				ch->position = POS_STANDING;
				ch->bashwait=0;
			}
			else
				send_to_char("You are already fighting!\n\r",ch);
			break;
	}
	affect_strip(ch,gsn_catnap);
	ch->bashwait = 0;

	return;
}

void do_rest(CHAR_DATA *ch,char *argument){
    OBJ_DATA *obj = NULL;

    if (MOUNTED(ch))
    {
        send_to_char("You can't rest while mounted.\n\r",ch);
        return;
    }

	if (ch->isaff(AF_INSOMNIA)){
		ch->send("Your excited mind prevents you from getting comfortable.\n\r");
		return;
	}

    if (RIDDEN(ch))
    {
        send_to_char("You can't rest while being ridden.\n\r",ch);
        return;
    }


    if (ch->position == POS_FIGHTING)
    {
		send_to_char("You are already fighting!\n\r",ch);
		return;
    }

    /* okay, now that we know we can rest, find an object to rest on */
    if (argument[0] != '\0')
    {
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
    }
    else
		obj = ch->on;

    if (obj != NULL)
    {
        if (obj->item_type != ITEM_FURNITURE || (!IS_SET(obj->value[2],REST_ON) && !IS_SET(obj->value[2],REST_IN) && !IS_SET(obj->value[2],REST_AT) && !IS_SET(obj->value[2],REST_UNDER)))
    	{
			send_to_char("You can't rest on that.\n\r",ch);
			return;
    	}

        if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
        {
			act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
			return;
    	}
	
		ch->on = obj;
		if (HAS_TRIGGER_OBJ(obj,TRIG_SIT))
			p_percent_trigger(NULL,obj,NULL,ch,NULL,NULL,TRIG_SIT);
    }

    switch (ch->position)
    {
		case POS_SLEEPING:
			if (ch->isaff(AF_SLEEP))
			{
				send_to_char("You can't wake up!\n\r",ch);
				return;
			}
			if (ch->isaff(AF_SAP))
			{
				send_to_char("You're unconscious!\n\r",ch);
				return;
			}
			if (obj == NULL)
			{
				send_to_char("You wake up and start resting.\n\r",ch);
				act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_AT))
			{
				act_new("You wake up and rest at $p.",ch,obj,NULL,TO_CHAR,POS_SLEEPING);
				act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_ON))
			{
				act_new("You wake up and rest on $p.",ch,obj,NULL,TO_CHAR,POS_SLEEPING);
				act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_UNDER))
			{
				act_new("You wake up and rest under $p.",ch,obj,NULL,TO_CHAR,POS_SLEEPING);
				act("$n wakes up and rests under $p.",ch,obj,NULL,TO_ROOM);
			}
			else
			{
				act_new("You wake up and rest in $p.",ch,obj,NULL,TO_CHAR,POS_SLEEPING);
				act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
			}
			ch->position = POS_RESTING;
			break;
		case POS_RESTING:
			send_to_char("You are already resting.\n\r",ch);
			break;
		case POS_STANDING:
			if (obj == NULL)
			{
				send_to_char("You rest.\n\r",ch);
				act("$n sits down and rests.",ch,NULL,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_AT))
			{
				act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
				act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_ON))
			{
				act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
				act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_UNDER))
			{
				act("You sit under $p and rest.",ch,obj,NULL,TO_CHAR);
				act("$n sits under $p and rests.",ch,obj,NULL,TO_ROOM);
			}
			else
			{
				act("You rest in $p.",ch,obj,NULL,TO_CHAR);
				act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
			}
			ch->position = POS_RESTING;
			break;
		case POS_SITTING:
			if (obj == NULL)
			{
				send_to_char("You rest.\n\r",ch);
				act("$n rests.",ch,NULL,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_AT))
			{
				act("You rest at $p.",ch,obj,NULL,TO_CHAR);
				act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_ON))
			{
				act("You rest on $p.",ch,obj,NULL,TO_CHAR);
				act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],REST_UNDER))
			{
				act("You rest under $p.",ch,obj,NULL,TO_CHAR);
				act("$n rests under $p.",ch,obj,NULL,TO_ROOM);
			}
			else
			{
				act("You rest in $p.",ch,obj,NULL,TO_CHAR);
				act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
			}
			ch->position = POS_RESTING;
			break;
    }
    return;
}


void do_sit(CHAR_DATA *ch,char *argument)
{
    OBJ_DATA *obj = NULL;

    if (MOUNTED(ch))
    {
        send_to_char("You can't sit while mounted.\n\r",ch);
        return;
    }

    if (RIDDEN(ch))
    {
        send_to_char("You can't sit while being ridden.\n\r",ch);
        return;
    }

    if (ch->position == POS_FIGHTING)
    {
		send_to_char("Maybe you should finish this fight first?\n\r",ch);
		return;
    }

    /* okay, now that we know we can sit, find an object to sit on */
    if (argument[0] != '\0')
    {
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
			send_to_char("You don't see that here.\n\r",ch);
			return;
		}
    }
    else obj = ch->on;

    if (obj != NULL)
    {
		if (obj->item_type != ITEM_FURNITURE || (!IS_SET(obj->value[2],SIT_ON) && !IS_SET(obj->value[2],SIT_IN) && !IS_SET(obj->value[2],SIT_AT) && !IS_SET(obj->value[2],SIT_UNDER)))
		{
			send_to_char("You can't sit on that.\n\r",ch);
			return;
		}

		if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
		{
			act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
			return;
		}

		ch->on = obj;
		if ( HAS_TRIGGER_OBJ(obj,TRIG_SIT) )
			p_percent_trigger(NULL,obj,NULL,ch,NULL,NULL,TRIG_SIT);
    }
    switch (ch->position)
    {
		case POS_SLEEPING:
			if (ch->isaff(AF_SLEEP))
			{
				send_to_char("You can't wake up!\n\r",ch);
				return;
			}
			if (ch->isaff(AF_SAP))
			{
				send_to_char("You're unconscious!\n\r",ch);
				return;
			}
            if (obj == NULL)
            {
            	send_to_char("You wake and sit up.\n\r",ch);
            	act("$n wakes and sits up.",ch,NULL,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_AT))
            {
            	act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_ON))
            {
            	act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits on $p.",ch,obj,NULL,TO_ROOM);
            }
            else if (IS_SET(obj->value[2],SIT_UNDER))
            {
            	act_new("You wake and sit under $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits under $p.",ch,obj,NULL,TO_ROOM);
            }
            else
            {
            	act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
            	act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
            }

			ch->position = POS_SITTING;
			break;
		case POS_RESTING:
			if (obj == NULL)
				send_to_char("You stop resting.\n\r",ch);
			else if (IS_SET(obj->value[2],SIT_AT))
			{
				act("You sit at $p.",ch,obj,NULL,TO_CHAR);
				act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],SIT_UNDER))
			{
				act("You sit under $p.",ch,obj,NULL,TO_CHAR);
				act("$n sits under $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],SIT_ON))
			{
				act("You sit on $p.",ch,obj,NULL,TO_CHAR);
				act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
			}
			ch->position = POS_SITTING;
			break;
		case POS_SITTING:
			send_to_char("You are already sitting down.\n\r",ch);
			break;
		case POS_STANDING:
			if (obj == NULL)
    	    {
				send_to_char("You sit down.\n\r",ch);
    	        act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],SIT_AT))
			{
				act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
				act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],SIT_ON))
			{
				act("You sit on $p.",ch,obj,NULL,TO_CHAR);
				act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
			}
			else if (IS_SET(obj->value[2],SIT_UNDER))
			{
				act("You sit under $p.",ch,obj,NULL,TO_CHAR);
				act("$n sits under $p.",ch,obj,NULL,TO_ROOM);
			}
			else
			{
				act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
				act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
			}
    	    ch->position = POS_SITTING;
    	    break;
	}
    return;
}

void do_sleep(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj = NULL;

	if (MOUNTED(ch)){
		send_to_char("You can't sleep while mounted.\n\r",ch);
		return;
	}

	if (ch->isaff(AF_INSOMNIA)){
		ch->send("Your mind is too excited to let you sleep.\n\r");
		return;
	}

	if (RIDDEN(ch)){
		send_to_char("You can't sleep while being ridden.\n\r",ch);
		return;
	}

	switch (ch->position){
		case POS_SLEEPING:
			send_to_char("You are already sleeping.\n\r",ch);
			break;
		case POS_RESTING:
		case POS_SITTING:
		case POS_STANDING: 
			if (argument[0] == '\0' && ch->on == NULL){
				send_to_char("You go to sleep.\n\r",ch);
				act("$n goes to sleep.",ch,NULL,NULL,TO_ROOM);
				ch->position = POS_SLEEPING;
			}
			else  /* find an object and sleep on it */{
				if (argument[0] == '\0')
					obj = ch->on;
				else
	    			obj = get_obj_list(ch,argument,ch->in_room->contents);

				if (obj == NULL){
					send_to_char("You don't see that here.\n\r",ch);
					return;
				}
				if (obj->item_type != ITEM_FURNITURE || (!IS_SET(obj->value[2],SLEEP_ON) && !IS_SET(obj->value[2],SLEEP_IN) && !IS_SET(obj->value[2],SLEEP_AT) && !IS_SET(obj->value[2],SLEEP_UNDER))){
					send_to_char("You can't sleep on that!\n\r",ch);
					return;
				}

				if (ch->on != obj && count_users(obj) >= obj->value[0]){
					act_new("There is no room on $p for you.",ch,obj,NULL,TO_CHAR,POS_DEAD);
					return;
				}

				ch->on = obj;
				if (HAS_TRIGGER_OBJ(obj,TRIG_SIT))
					p_percent_trigger(NULL,obj,NULL,ch,NULL,NULL,TRIG_SIT);
				if (IS_SET(obj->value[2],SLEEP_AT)){
					act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
					act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
				}
				else if (IS_SET(obj->value[2],SLEEP_ON)){
					act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
					act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
				}
				else if (IS_SET(obj->value[2],SLEEP_UNDER)){
					act("You go to sleep under $p.",ch,obj,NULL,TO_CHAR);
					act("$n goes to sleep under $p.",ch,obj,NULL,TO_ROOM);
				}
				else{
					act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
					act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
				}
				ch->position = POS_SLEEPING;
			}
			break;
		case POS_FIGHTING:
			send_to_char("You are fighting!\n\r",ch);
			break;
	}
}

void do_wake(CHAR_DATA *ch,char *argument)
{
    char arg[MIL];
    CHAR_DATA *victim;

    one_argument(argument,arg);
	if (ch->isaff(AF_SLEEP)){
		ch->send("You're too sleepy..zzzzZZZZZzzzzzz\n\r");
		return;
	}
	if (ch->isaff(AF_SAP)){
		ch->send("You're too unconsiouszsgtnlsge..\n\r");
		return;
	}
    if (arg[0] == '\0'){
		do_function(ch,&do_stand,"");
		return;
	}

    if (!IS_AWAKE(ch)){
		if (!str_prefix(arg,"self") || !str_prefix(arg,ch->name)){
			do_function(ch,&do_stand,"");
			return;
		}
		else{
			send_to_char("You are asleep yourself!\n\r",ch);
			return;
		}
	}

    if (!(victim = get_char_room(ch,NULL,arg))){
		send_to_char("They aren't here.\n\r",ch);
		return;
	}
	if (victim->isaff(AF_SLEEP)){
		send_to_char("You can't wake them up!\n\r",ch);
		return;
	}
	if (victim->isaff(AF_SAP)){
		send_to_char("They're too unconscious!\n\r",ch);
		return;
	}

    if (IS_AWAKE(victim)){
		act("$N is already awake.",ch,NULL,victim,TO_CHAR);
		return;
	}

    if (victim->isaff(AF_SLEEP)){
		act("You can't wake $M!",ch,NULL,victim,TO_CHAR);
		return;
	}

    if (victim->isaff(AF_SAP)){
		act("You can't wake $M!",ch,NULL,victim,TO_CHAR);
		return;
	}

    act_new("$n wakes you.",ch,NULL,victim,TO_VICT,POS_SLEEPING);
    do_function(victim,&do_stand,"");
    return;
}

bool make_visible(CHAR_DATA *ch){
	bool found = false;

	if(is_affected(ch,gsn_desertcover)){
		ch->send("You stop hiding in the desert.\n\r");
		act("$n steps out of the sand.",ch,NULL,NULL,TO_ROOM);
		affect_strip(ch,gsn_desertcover);
		found = true;
	}
	if(is_affected(ch,gsn_blend)){
		ch->send("You stop blending.\n\r");
		act("$n steps out of nature.",ch,NULL,NULL,TO_ROOM);
		affect_strip(ch,gsn_blend);
		found = true;
	}
	if(is_affected(ch,gsn_invis)){
		ch->send("You fade into visibility.\n\r");
		act("$n fades into visibility.",ch,NULL,NULL,TO_ROOM);
	    affect_strip(ch,gsn_invis);
		found = true;
	}
	if(is_affected(ch,gsn_sneak)){
		ch->send("You stop sneaking around.\n\r");
		affect_strip(ch,gsn_sneak);
		found = true;
	}
	if(is_affected(ch,gsn_hide)){
		ch->send("You stop hiding.\n\r");
		act("$n steps out of the shadows.",ch,NULL,NULL,TO_ROOM);
		affect_strip(ch,gsn_hide);
		found = true;
	}
	if(is_affected(ch,gsn_mass_invis)){
		ch->send("You fade into visibility.\n\r");
		act("$n fades into visibility.",ch,NULL,NULL,TO_ROOM);
		affect_strip(ch,gsn_mass_invis);
		found = true;
	}
	if(is_affected(ch,gsn_penumbralveil)){
		ch->send("You step out of your shadowy cloak.\n\r");
		act("$n steps out of the shadows.",ch,NULL,NULL,TO_ROOM);
		affect_strip(ch,gsn_penumbralveil);
		found = true;
	}
	ch->remaff(AF_HIDE);
	ch->remaff(AF_PERFECTSNEAK);
	ch->remaff(AF_CAMOFLAGE);
    ch->remaff(AF_INVISIBLE);
	return found;
}

void do_visible(CHAR_DATA *ch,char *argument){
	if(!make_visible(ch)){
		ch->send("You are not hidden by any powers.\n\r");
		return;
	}
}

void do_recall(CHAR_DATA *ch,char *argument){
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

	if (IS_NPC(ch) && !ch->isact(AT_PET) && !ch->mount){
		send_to_char("Only players can recall.\n\r",ch);
		return;
	}
	if(ch->isplr(PL_ARENA)){
		send_to_char("You're in a duel.\n\r",ch);
		return;
	}
	if(ch->level > NEWBIE){
		ch->send("You are too high a level to do this.\n\r");
		return;
	}

	act("$n prays for transportation!",ch,0,0,TO_ROOM);

	if (!(location = get_room_index(hometowns[ch->hometown].recall))){
		send_to_char("You are completely lost.\n\r",ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags,ROOM_NO_RECALL) || ch->isaff(AF_CURSE)){
		send_to_char("You have been forsaken.\n\r",ch);
		return;
	}

	if ((victim = ch->fighting)){
		int lose,skill;

		skill = get_skill(ch,gsn_recall);

		if (number_percent() < 80 * skill / 100){
			check_improve(ch,gsn_recall,false,6);
			WAIT_STATE(ch,4);
			send_to_char("You failed!.\n\r",ch);
			return;
		}

		lose = (ch->desc != NULL) ? 25 : 50;
		gain_exp(ch,0 - lose);
		check_improve(ch,gsn_recall,true,4);
		printf_to_char(ch,"You recall from combat!  You lose %d exps.\n\r",lose);
		stop_fighting(ch,true);
	}

	ch->move *= .66;
	act("$n disappears.",ch,NULL,NULL,TO_ROOM);
	char_from_room(ch);
	char_to_room(ch,location);
	act("$n appears in the room.",ch,NULL,NULL,TO_ROOM);
	do_function(ch,&do_look,"auto");

	if (ch->pet != NULL)
		do_function(ch->pet,&do_recall,"");

	if (ch->mount != NULL)
		do_recall(ch->mount,"" );
}

void bank_deposit(CHAR_DATA *ch,CHAR_DATA *banker,char *argument){
	char buf[MSL], arg1 [MIL], arg2 [MIL];
	int ctype, amnt, tgold, tsilver;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	ctype = 0;

	if (arg1[0] == '\0' || arg2[0] == '\0'){
		send_to_char("How much of what do you wanna deposit?\n\r",ch);
		return;
	}

	if (!str_cmp(arg2,"gold"))
		ctype=1;
	if (!str_cmp(arg2,"silver"))
		ctype=2;

	if (ctype == 0){
		send_to_char("Syntax is: 'deposit # <{wsilver{x/{Ygold{x>'",ch);
		return;
	}

	amnt = atoi(arg1);

	if (ctype==1){
		if (amnt >= (ch->gold + 1)){
			sprintf(buf,"%s, you do not have {Y%d gold {xcoins.",ch->name,amnt);
			do_say(banker,buf);
			return;
		}

		ch->bankgold += amnt;
		ch->gold -= amnt;
		tgold = ch->bankgold;
		tsilver = ch->banksilver;
		sprintf(buf,"%s, your account now contains: {Y%d gold {Gcoins, and {w%d silver {Gcoins",ch->name,tgold,tsilver);
		do_say(banker,buf);
		sprintf(buf,"after depositing: {Y%d gold {Gcoins.",amnt);
		do_say(banker,buf);
		return;
	}
	else{
		if (amnt >= (ch->silver + 1)){
			sprintf(buf,"%s, you do not have {w%d silver {Gcoins.",ch->name,amnt);
			do_say(banker,buf);
			return;
		}

		ch->banksilver += amnt;
		ch->silver -= amnt;
		tgold = ch->bankgold;
		tsilver = ch->banksilver;
		sprintf(buf,"%s, your account now contains: {Y%d gold {Gcoins, and {w%d silver {Gcoins",ch->name,tgold,tsilver);
		do_say(banker,buf);
		sprintf(buf,"after depositing: {w%d silver {Gcoins.",amnt);
		do_say(banker,buf);
		return;
	}
}

void bank_withdraw(CHAR_DATA *ch,CHAR_DATA *banker,char *argument){
	char buf[MSL], arg1 [MIL], arg2 [MIL];
	int ctype, amnt, tgold, tsilver;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	ctype = 0;


	if (arg1[0] == '\0' || arg2[0] == '\0'){
		send_to_char("How much of what do you wanna deposit?\n\r",ch);
		return;
	}

	if (!str_cmp(arg2,"gold"))
		ctype=1;
	if (!str_cmp(arg2,"silver"))
		ctype=2;

	if(ctype == 0)
	{
		send_to_char("Syntax is: 'deposit # <{wsilver{x/{Ygold{x>'",ch);
		return;
	}

	amnt = atoi(arg1);

	if (ctype==1){
		if (amnt >= (ch->bankgold + 1)){
			sprintf(buf,"%s, you do not have {Y%d gold {xcoins.",ch->name,amnt);
			do_say(banker,buf);
			return;
		}

		ch->bankgold -= amnt;
		ch->gold += amnt;
		tgold = ch->bankgold;
		tsilver = ch->banksilver;
		sprintf(buf,"%s, your account now contains: {Y%d gold {Gcoins, and {w%d silver {Gcoins",ch->name,tgold,tsilver);
		do_say(banker,buf);
		sprintf(buf,"after withdrawing: {Y%d gold {Gcoins in the bank.",amnt);
		do_say(banker,buf);
		return;
	}
	else{
		if (amnt >= (ch->banksilver + 1)){
			sprintf(buf,"%s, you do not have {w%d silver {Gcoins in the bank.",ch->name,amnt);
			do_say(banker,buf);
			return;
		}

		ch->banksilver -= amnt;
		ch->silver += amnt;
		tgold = ch->bankgold;
		tsilver = ch->banksilver;
		sprintf(buf,"%s, your account now contains: {Y%d gold {Gcoins, and {w%d silver {Gcoins",ch->name,tgold,tsilver);
		do_say(banker,buf);
		sprintf(buf,"after withdrawing: {w%d silver {Gcoins.",amnt);
		do_say(banker,buf);
		return;
	}
}

void bank_balance(CHAR_DATA *ch,CHAR_DATA *banker,char *argument){
	int tgold = ch->bankgold;
	int tsilver = ch->banksilver;

	if (IS_NPC(ch))
		return;

	printf_to_char(ch,"You have {Y%d gold {xcoins and {w%d silver {xcoins in the bank.\n\r",tgold,tsilver);
	return;
}

void bank_exchange(CHAR_DATA *ch,CHAR_DATA *banker,char *argument){
	char arg[MIL],buf[MIL];
	int num = 0,stotal = 0,gtotal = 0;

	argument = one_argument(argument,arg);
	if(!argument[0] || !arg[0]){
		ch->send("Syntax: bank exchange <#> <silver/gold>\n\r");
		return;
	}
	if(!is_number(arg)){
		ch->send("That is not a number\n\r");
		return;
	}
	num = atoi(arg);
	if(!str_prefix(argument,"silver")){
		if(num < 1){
			ch->send("That is not a valid amount.\n\r");
			return;
		}
		if(num < 100){
			ch->send("You must exchange at least one hundred silver.\n\r");
		}
		if(ch->silver < num){
			ch->send("You don't have that much silver.\n\r");
			return;
		}
		while(num >= 100){
			num -= 100;
			stotal += 100;
			gtotal++;
			ch->silver -= 100;
			ch->gold++;
		}
		sprintf(buf,"For %d silver, here is %d gold.",stotal,gtotal);
		do_function(banker,&do_say,buf);
		return;
	}
	if(!str_prefix(argument,"gold")){
		if(num < 1){
			ch->send("That is not a valid amount.\n\r");
			return;
		}
		if(ch->gold < num){
			ch->send("You don't have that much gold.\n\r");
			return;
		}
		ch->gold -= num;
		ch->silver += num*100;
		sprintf(buf,"For %d gold, here is %d silver.",num,num*100);
		do_function(banker,&do_say,buf);
		return;
	}
	ch->send("You can only exchange silver or gold.\n\r");
}

void do_bank(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *banker;

	if (IS_NPC(ch))
		return;
	argument = one_argument(argument,arg);

	if (!IS_SET(ch->in_room->room_flags,ROOM_BANK)){
		send_to_char("But you are not in a bank.\n\r",ch);
		return;
	}

	banker = NULL;
	for ( banker = ch->in_room->people; banker; banker = banker->next_in_room )
		if (IS_NPC(banker) && banker->pIndexData->isact(AT_BANKER))
			break;

	if (!banker){
		send_to_char("The banker is currently not available.\n\r",ch);
		return;
	}
	if(!str_prefix(arg,"deposit")){
		bank_deposit(ch,banker,str_dup(argument));
		return;
	}
	if(!str_prefix(arg,"withdraw")){
		bank_withdraw(ch,banker,str_dup(argument));
		return;
	}
	if(!str_prefix(arg,"exchange")){
		bank_exchange(ch,banker,str_dup(argument));
		return;
	}
	if(!str_prefix(arg,"balance")){
		bank_balance(ch,banker,str_dup(argument));
		return;
	}
	ch->send("Syntax: bank <deposit/withdraw/exchange/balance>\n\r");
}

bool check_horsemanship(CHAR_DATA *ch)
{
    CHAR_DATA *mount;
	int chance;
	chance = get_skill(ch,gsn_riding);

    if((mount = MOUNTED(ch)) == NULL)
		return false;

	chance += get_skill(ch,gsn_horsemanship);
	chance /= 2;

	if (get_skill(ch,gsn_horsemanship) < 1 || !mount->isact(AT_WARHORSE))
	{
		send_to_char("Your horse bucks you off!\n\r",ch);
		act("$n falls flat on $s back as $s horse bucks $m off!",ch,NULL,NULL,TO_ROOM);
	    ch->mounted = false;
	    mount->mounted = false;
		ch->bashwait = 4;
		WAIT_STATE(ch, skill_table[gsn_bash].beats * .5);
		ch->position = POS_RESTING;
		return false;
	}

	if (number_percent() > chance)
	{
		send_to_char("Your horse becomes spooked and suddenly bucks you off!\n\r",ch);
		act("$n loses control of $s horse and gets bucked off!",ch,NULL,NULL,TO_ROOM);
	    ch->mounted = false;
	    mount->mounted = false;
		ch->bashwait = 4;
		WAIT_STATE(ch,skill_table[gsn_bash].beats * .5);
		ch->position = POS_RESTING;
		check_improve(ch,gsn_horsemanship,false,2);
		return false;
	}
	else
	{
		check_improve(ch,gsn_horsemanship,true,1);
		return true;
	}
}

void do_push_drag(CHAR_DATA *ch,char *argument,char *verb)
{
    char arg1[MIL], arg2[MIL];
    ROOM_INDEX_DATA *in_room, *to_room;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    OBJ_DATA *obj;
    int door;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    victim = get_char_room(ch, NULL,arg1);
    obj = get_obj_list(ch,arg1,ch->in_room->contents);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
		printf_to_char(ch,"%s whom or what where?\n\r",capitalize(verb));
		return;
    }

    if ((!victim || !can_see(ch,victim)) && (!obj || !can_see_obj(ch,obj)))
    {
		printf_to_char(ch,"%s whom or what where?\n\r",capitalize(verb));
        return;
    }

		 if (!str_cmp(arg2,"n") || !str_cmp(arg2,"north"))door = 0;
	else if (!str_cmp(arg2,"e") || !str_cmp(arg2,"east")) door = 1;
	else if (!str_cmp(arg2,"s") || !str_cmp(arg2,"south"))door = 2;
	else if (!str_cmp(arg2,"w") || !str_cmp(arg2,"west")) door = 3;
	else if (!str_cmp(arg2,"u") || !str_cmp(arg2,"up"))   door = 4;
	else if (!str_cmp(arg2,"d") || !str_cmp(arg2,"down")) door = 5;
	else
	{
		printf_to_char(ch,"Alas, you cannot %s in that direction.\n\r",verb);
		return;
	}

	if (obj)
	{
		in_room = obj->in_room;
		if ( (pexit = in_room->exit[door]) == NULL || (to_room = pexit->u1.to_room) == NULL || !can_see_room(ch,pexit->u1.to_room))
		{
			printf_to_char(ch,"Alas, you cannot %s in that direction.\n\r",verb);
			return;
		}

		if ( !IS_SET(obj->wear_flags, ITEM_TAKE) || !can_loot(ch,obj) )
		{
			send_to_char("It won't budge.\n\r",ch);
			return;
		}

		if ( IS_SET(pexit->exit_info,EX_CLOSED) || IS_SET(pexit->exit_info,EX_NOPASS) )
		{
			act("You cannot $t it through the $d.",ch,verb,pexit->keyword,TO_CHAR);
			act("$n decides to $t $P around!",ch,verb,obj,TO_ROOM);
			return;
		}

		act("You attempt to $T $p out of the room.",ch,obj,verb,TO_CHAR);
		act("$n is attempting to $T $p out of the room.",ch,obj,verb,TO_ROOM);

		if ( obj->weight >  (2 * can_carry_w (ch)) )
		{
			act("$p is too heavy to $T.\n\r",ch,obj,verb,TO_CHAR);
			act("$n attempts to $T $p, but it is too heavy.\n\r",ch,obj,verb,TO_ROOM);
			return;
		}
		if 	 ( !IS_IMMORTAL(ch)
		||   IS_SET(ch->in_room->room_flags, ROOM_SAFE)
		//||   IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
		//||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
		||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
		||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
		|| 	 (number_percent() > 75) )
		{
			send_to_char("It won't budge.\n\r",ch);
			return;
		}

		if ( ch->move > 10 )
		{
			ch->move -= 10;
			/*send_to_char( "You succeed!\n\r", ch );
			act( "$n succeeds!", ch, NULL, NULL, TO_ROOM );*/
			obj_from_room(obj);
			obj_to_room(obj,to_room);
			if (!str_cmp(verb,"drag"))
				move_char(ch,door,false,true);
		}
		else
		{
			printf_to_char(ch,"You are too tired to %s anything around!\n\r",verb);
		}
	}
	else
	{
		if ( ch == victim )
		{
			act("You $t yourself about the room and look very silly.",ch,verb,NULL,TO_CHAR);
			act("$n decides to be silly and $t $mself about the room.",ch,verb,NULL,TO_ROOM);
			return;
		}

		if(victim->fighting){
			ch->send("Let them finish their fight.\n\r");
			return;
		}
		if(!canFight(ch,victim,true))
			return;

		in_room = victim->in_room;
		if ( (pexit = in_room->exit[door]) == NULL || (to_room = pexit->u1.to_room) == NULL || !can_see_room(victim,pexit->u1.to_room))
		{
			printf_to_char(ch,"Alas, you cannot %s them that way.\n\r",verb);
			return;
		}

		if (IS_SET(pexit->exit_info,EX_CLOSED) && (!victim->isaff(AF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS)))
		{
			act("You try to $t them through the $d.",ch,verb,pexit->keyword,TO_CHAR);
			act("$n decides to $t you around!",ch,verb,victim,TO_VICT);
			act("$n decides to $t $N around!",ch,verb,victim,TO_NOTVICT);
			return;
		}

		act("You attempt to $t $N out of the room.",ch,verb,victim,TO_CHAR);
		act("$n is attempting to $t you out of the room!",ch,verb,victim,TO_VICT);
		act("$n is attempting to $t $N out of the room.",ch,verb,victim,TO_NOTVICT);

		if (IS_NPC(victim)
		&& (victim->isact(AT_IS_HEALER) || victim->isact(AT_IS_CHANGER) || victim->isact(AT_NOKILL) || victim->res[RS_SUMMON] < 1 || victim->pIndexData->pShop ))
		{
			send_to_char("That would not be a good idea.\n\r",ch);
			return;
		}

		if 	 (victim->in_room == NULL
		||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
		||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
		||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
		||	 (!str_cmp( verb, "drag" ) && victim->position >= POS_STANDING)
		||   (!str_cmp( verb, "push" ) && victim->position != POS_STANDING)
		|| 	 is_safe(ch,victim))
		{
			send_to_char("You can't move them.\n\r",ch);
			return;
		}
		if ( get_curr_stat(victim,STAT_STR) > get_curr_stat(ch,STAT_STR))
		{
			act("$n glares at $N and pushes back, slamming $M away!",victim,NULL,ch,TO_NOTVICT);
			act("You push back at $N, sending $M into another room.",victim,NULL,ch,TO_CHAR);
			act("$n glares at you and pushes back!",victim,NULL,ch,TO_VICT);
				move_char(ch,door,true,false);
			return;
		}
		if ( get_curr_stat(victim, STAT_STR) == get_curr_stat(ch, STAT_STR))
		{
			act("$n glares at $N and pushes back!",victim,NULL,ch,TO_NOTVICT);
			act("You push back at $N for $S failed attempts to move you.",victim,NULL,ch,TO_CHAR);
			act("$n glares at you and pushes back!",victim,NULL,ch,TO_VICT);
			return;
		}

		if ( ch->move > 5 )
		{
			act("$n pushes $N!",ch,NULL,victim,TO_NOTVICT);
			act("$n pushes you!",ch,NULL,victim,TO_VICT);
			act("You push $N!",ch,NULL,victim,TO_CHAR);
			ch->move -= 5;
			if (!str_cmp(verb,"drag"))
				move_char(ch,door,true,false);
			move_char(victim,door,false,false);
		}
		else
			printf_to_char(ch,"You are too tired to %s anybody around!\n\r",verb);
	}

	return;
}
               
void do_push(CHAR_DATA *ch,char *argument){
    do_push_drag(ch,argument,"push");
    return;
}

void do_drag(CHAR_DATA *ch,char *argument){
    do_push_drag(ch,argument,"drag");
    return;
}

void unSneak(CHAR_DATA *ch){
	affect_strip(ch,gsn_desertcover);
	affect_strip(ch,gsn_hide);
	affect_strip(ch,gsn_blend);
	ch->remaff(AF_HIDE);
	ch->remaff(AF_CAMOFLAGE);
}

void unSleep(CHAR_DATA *ch)
{
	affect_strip(ch,gsn_sap);
	affect_strip(ch,gsn_strangle);
	affect_strip(ch,gsn_sleep);
	ch->remaff(AF_SLEEP);
	ch->remaff(AF_SAP);
}

bool do_detecttraps(CHAR_DATA *ch)
{
	int chance = get_skill(ch,gsn_detecttraps) * .5;

	if (chance < 1)
		return false;

	chance += get_curr_stat(ch,STAT_WIS) / 4;

	if (number_percent() < chance){
		ch->send("Your spidey senses tingle!\n\r");
		return true;
	}
	return false;
}
