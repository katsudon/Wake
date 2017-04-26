#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */
bool				remove_obj			( CHAR_DATA*,int,bool );
void				wear_obj			( CHAR_DATA*,OBJ_DATA*,bool,bool );
CHAR_DATA *			find_keeper			( CHAR_DATA* );
int					get_cost			( CHAR_DATA*,OBJ_DATA*,bool );
void				obj_to_keeper		( OBJ_DATA*,CHAR_DATA* );
OBJ_DATA *			get_obj_keeper		( CHAR_DATA*,CHAR_DATA*,char* );
bool				check_dual_bare		( CHAR_DATA*,OBJ_DATA* );
bool				check_dual_blade	( CHAR_DATA*,OBJ_DATA*,OBJ_DATA* );
EXTRA_DESCR_DATA	*new_extra_descr	( void );
int					calcHands			( CHAR_DATA* );
int					get_attackspeed		( CHAR_DATA*,bool );
void				do_buy_pet			( CHAR_DATA*,char* );
void				save_char_locker	( CHAR_DATA* );
char *flag_string			( const struct flag_type*,int );

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
{
    CHAR_DATA *owner, *wch;

    if (IS_IMMORTAL(ch))
		return true;

    if (!obj->owner || obj->owner == NULL)
		return true;

    owner = NULL;
    for ( wch = char_list; wch != NULL ; wch = wch->next )
        if (!str_cmp(wch->name,obj->owner))
            owner = wch;

    if (owner == NULL)
		return true;

    if (!str_cmp(ch->name,owner->name))
		return true;

    if (!IS_NPC(owner) && owner->isact(PL_CANLOOT))
		return true;

    if (is_same_group(ch,owner))
		return true;

    return false;
}

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool csee){
	CHAR_DATA *gch;
	char buffer[100];

	if (!CAN_WEAR(obj,ITEM_TAKE) && ch->level < HEADIMM){
		send_to_char("You can't take that.\n\r",ch);
		return;
	}

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)){
		act("$d: you can't carry that many items.",ch,NULL,obj->name,TO_CHAR);
		return;
	}

	if ((!obj->in_obj || obj->in_obj->carried_by != ch) && (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch))){
		act("$d: you can't carry that much weight.",ch,NULL,obj->name,TO_CHAR);
		return;
	}

	if (!can_loot(ch,obj)){
		act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
		return;
	}

	if (obj->in_room != NULL)
		for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
			if (gch->on == obj){
				act("$N appears to be using $p.",ch,obj,gch,TO_CHAR);
				return;
			}

	if (container != NULL){
		if (container->pIndexData->vnum == OBJ_VNUM_PIT && !CAN_WEAR(container,ITEM_TAKE) && !IS_OBJ_STAT(obj,ITM_HAD_TIMER))
			obj->timer = 0;
		act("You get $p from $P.",ch,obj,container,TO_CHAR);
		act("$n gets $p from $P.",ch,obj,container,TO_ROOM);
		REMOVE_BIT(obj->extra_flags,ITM_HAD_TIMER);
		obj_from_obj(obj);
	}
	else{
		if(csee){
			act("You get $p.",ch,obj,container,TO_CHAR);
			act("$n gets $p.",ch,obj,container,TO_ROOM);
		}
		obj_from_room(obj);
	}

	if (obj->item_type == ITEM_MONEY){
		ch->silver += obj->value[0];
		ch->gold += obj->value[1];
		if (ch->isplr(PL_AUTOSPLIT)){
			if (obj->value[0] > 0 || obj->value[1] > 0){
				sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
				do_function(ch,&do_split,buffer);	
			}
		}
		extract_obj(obj);
	}
	else{
		obj_to_char(obj,ch);
		if (HAS_TRIGGER_OBJ(obj,TRIG_GET))
			p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_GET);
		if (HAS_TRIGGER_ROOM(ch->in_room,TRIG_GET))
			p_give_trigger(NULL,NULL,ch->in_room,ch,obj,TRIG_GET);
	}
}

void do_get(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	OBJ_DATA *obj,*obj_next,*container;
	bool found;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if (!str_cmp(arg2,"from"))
		argument = one_argument(argument,arg2);

	if (!arg1[0]){
		send_to_char("Get what?\n\r",ch);
		return;
	}

	if (!arg2[0]){
		if (str_cmp(arg1,"all") && str_prefix("all.",arg1)){
			if (!(obj = get_obj_list(ch,arg1,ch->in_room->contents))){
				send_to_char("You don't see that here.\n\r",ch);
				return;
			}
			get_obj(ch,obj,NULL,true);
		}
		else{
			found = false;
			for (obj = ch->in_room->contents;obj != NULL;obj = obj_next){
				obj_next = obj->next_content;
				if ((!arg1[3] || is_name(&arg1[4],obj->name)) && can_see_obj(ch,obj)){
					found = true;
					get_obj(ch,obj,NULL,true);
				}
			}

			if (!found){
				if (!arg1[3])
					send_to_char("You see nothing here.\n\r",ch);
				else
					act("You see no $T here.",ch,NULL,&arg1[4],TO_CHAR);
			}
		}
	}
	else{
		if (!str_cmp(arg2,"all") || !str_prefix("all.",arg2)){
			send_to_char("You can't do that.\n\r",ch);
			return;
		}

		if (!(container = get_obj_here(ch,NULL,arg2))){
			act("I see no $T here.",ch,NULL,arg2,TO_CHAR);
			return;
		}

		switch (container->item_type){
			default:
				send_to_char("That's not a container.\n\r",ch);
				return;
			case ITEM_CONTAINER:
			case ITEM_LOCKER:
			case ITEM_CORPSE_NPC:
				break;
			case ITEM_FORGE:
				if(!str_cmp(arg1,"all") || !str_prefix("all.",arg1))
					ch->send("Don't be so hasty!\n\r");
				else{
					if (!(obj = get_obj_list(ch,arg1,container->contains)))
						act("I see nothing like that in the $T.",ch,NULL,arg2,TO_CHAR);
					else
					get_obj(ch,obj,container,true);
				}
				return;
			case ITEM_CORPSE_PC:
				if (!can_loot(ch,container)){
					send_to_char("You can't do that.\n\r",ch);
					return;
				}
				break;
		}

		if (IS_SET(container->value[1],CONT_CLOSED)){
			act("The $d is closed.",ch,NULL,container->name,TO_CHAR);
			return;
		}

		if (str_cmp(arg1,"all") && str_prefix("all.",arg1)){
			if (!(obj = get_obj_list(ch,arg1,container->contains))){
				act("I see nothing like that in the $T.",ch,NULL,arg2,TO_CHAR);
				return;
			}
			get_obj(ch,obj,container,true);
			if(container->item_type == ITEM_LOCKER)
				save_char_locker(ch);
		}
		else{
			found = false;
			for (obj = container->contains;obj != NULL;obj = obj_next){
				obj_next = obj->next_content;
				if ((!arg1[3] || is_name(&arg1[4],obj->name)) && can_see_obj(ch,obj)){
					found = true;
					if(container->item_type == ITEM_LOCKER && container->value[0] && !IS_IMMORTAL(ch)){
						send_to_char("Don't be so greedy!\n\r",ch);
						return;
					}
					get_obj(ch,obj,container,true);
				}
			}

			if (!found){
				if (!arg1[3])
					act("You see nothing in $p.",ch,container,NULL,TO_CHAR);
				else
					act("You see nothing like that in the $p.",ch,container,NULL,TO_CHAR);
			}
			else if(container->item_type == ITEM_LOCKER)
				save_char_locker(ch);
		}
	}
}

void put_forge(CHAR_DATA *ch,OBJ_DATA *forge, OBJ_DATA *obj){
	if (obj == forge){
		send_to_char("You can't fold it into itself.\n\r",ch);
		return;
	}

	if (!can_drop_obj(ch,obj)){
		send_to_char("You can't let go of it.\n\r",ch);
		return;
	}

	if (get_obj_weight(obj) + get_true_weight(forge,false) > forge->value[2] * 10 || get_obj_weight(obj) > forge->value[2] * 10){
		send_to_char("It won't fit.\n\r",ch);
		return;
	}

	obj_from_char(obj);
	obj_to_obj(obj,forge);

	act("$n puts $p in $P.",ch,obj,forge,TO_ROOM);
	act("You put $p in $P.",ch,obj,forge,TO_CHAR);
	return;
}

void do_put(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL];
	OBJ_DATA *container,*obj,*obj_next;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if(!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
		argument = one_argument(argument,arg2);

	if(!arg1[0] || !arg2[0]){
		send_to_char( "Put what in what?\n\r", ch );
		return;
	}

	if(!str_cmp(arg2,"all") || !str_prefix("all.",arg2)){
		send_to_char("You can't do that.\n\r",ch);
		return;
	}

	if(!(container = get_obj_here(ch,NULL,arg2))){
		act("I see no $T here.",ch,NULL,arg2,TO_CHAR);
		return;
	}

	if(container->item_type == ITEM_FORGE){
		if(!str_cmp(arg1,"all") || !str_prefix("all.",arg1)){
			ch->send("Don't be so hasty!\n\r");
			return;
		}
		if(!(obj = get_obj_carry(ch,arg1,ch))){
			send_to_char("You do not have that item.\n\r",ch);
			return;
		}
		put_forge(ch,container,obj);
		return;
	}

	if(container->item_type != ITEM_CONTAINER && container->item_type != ITEM_LOCKER){
		send_to_char("That's not a container.\n\r",ch);
		return;
	}

	if(IS_SET(container->value[1],CONT_CLOSED)){
		act("The $d is closed.",ch,NULL,container->name,TO_CHAR);
		return;
	}

	if(str_cmp(arg1,"all") && str_prefix("all.",arg1)){
		if ((obj = get_obj_carry(ch,arg1,ch)) == NULL){
			send_to_char("You do not have that item.\n\r",ch);
			return;
		}

		if (obj == container){
			send_to_char("You can't fold it into itself.\n\r",ch);
			return;
		}

		if (!can_drop_obj(ch,obj)){
			send_to_char("You can't let go of it.\n\r",ch);
			return;
		}

		if (WEIGHT_MULT(obj) != 100){
		   send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
			return;
		}

		if(container->item_type != ITEM_LOCKER
		&& (get_obj_weight(obj) + get_true_weight(container,true) > (container->value[0] * 10) || get_obj_weight(obj) > (container->value[0] * 10))){
			send_to_char("It will not fit.\n\r",ch);
			return;
		}
		if(container->cont_count >= container->value[3]){
			ch->send("It won't fit.\n\r");
			return;
		}

		obj_from_char(obj);
		obj_to_obj(obj,container);

		if (IS_SET(container->value[1],CONT_PUT_ON)){
			act("$n puts $p on $P.",ch,obj,container,TO_ROOM);
			act("You put $p on $P.",ch,obj,container,TO_CHAR);
		}
		else{
			act("$n puts $p in $P.",ch,obj,container,TO_ROOM);
			act("You put $p in $P.",ch,obj,container,TO_CHAR);
		}
		if(container->item_type != ITEM_LOCKER){
			if (HAS_TRIGGER_OBJ(obj,TRIG_PUT))
				p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_PUT);
			}
		else
			save_char_locker(ch);
	}
	else{
		for ( obj = ch->carrying; obj != NULL; obj = obj_next ){
			obj_next = obj->next_content;

			if ((arg1[3] == '\0' || is_name(&arg1[4],obj->name))
			&&   can_see_obj(ch,obj)
			&&   WEIGHT_MULT(obj) == 100
			&&   obj->wear_loc == WEAR_NONE
			&&   obj != container
			&&   can_drop_obj(ch,obj)
			&&   get_obj_weight(obj) + get_true_weight(container,true) <= (container->value[0] * 10) 
			&&   get_obj_weight(obj) < (container->value[3] * 10)
			&&   container->cont_count < container->value[3]){
	    		if (container->pIndexData->vnum == OBJ_VNUM_PIT	&&  !CAN_WEAR(obj, ITEM_TAKE)){
	    			if (obj->timer)
						SET_BIT(obj->extra_flags,ITM_HAD_TIMER);
	    			else
	    	    		obj->timer = number_range(100,200);
				}
				obj_from_char(obj);
				obj_to_obj(obj,container);

        		if (IS_SET(container->value[1],CONT_PUT_ON)){
            		act("$n puts $p on $P.",ch,obj,container,TO_ROOM);
            		act("You put $p on $P.",ch,obj,container,TO_CHAR);
        		}
				else{
					act("$n puts $p in $P.", ch, obj, container,TO_ROOM);
					act("You put $p in $P.", ch, obj, container,TO_CHAR);
				}
				if(container->item_type != ITEM_LOCKER){
					if (HAS_TRIGGER_OBJ(obj,TRIG_PUT))
						p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_PUT);
				}
			}
		}
		if(container->item_type == ITEM_LOCKER)
			save_char_locker(ch);
	}
}

void do_drop(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	OBJ_DATA *obj, *obj_next;
	bool found;

	argument = one_argument(argument,arg);

	if (arg[0] == '\0'){
		send_to_char("Drop what?\n\r",ch);
		return;
	}

	if (is_number(arg)){
		int amount, gold = 0, silver = 0;

		amount   = atoi(arg);
		argument = one_argument(argument,arg);
		if ( amount <= 0 || ( str_cmp(arg,"coins") && str_cmp(arg,"coin") && str_cmp(arg,"gold") && str_cmp(arg,"silver") ) ){
			send_to_char("Sorry, you can't do that.\n\r",ch);
			return;
		}

		if ( !str_cmp(arg,"coins") || !str_cmp(arg,"coin") || !str_cmp(arg,"silver")){
			if (ch->silver < amount){
				send_to_char("You don't have that much silver.\n\r",ch);
				return;
			}

			ch->silver -= amount;
			silver = amount;
		}
		else{
			if (ch->gold < amount){
				send_to_char("You don't have that much gold.\n\r",ch);
				return;
			}
			ch->gold -= amount;
  			gold = amount;
		}

		for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ){
			obj_next = obj->next_content;

			switch (obj->pIndexData->vnum){
				case OBJ_VNUM_SILVER_ONE:
					silver += 1;
					extract_obj(obj);
					break;
				case OBJ_VNUM_GOLD_ONE:
					gold += 1;
					extract_obj( obj );
					break;
				case OBJ_VNUM_SILVER_SOME:
					silver += obj->value[0];
					extract_obj(obj);
					break;
				case OBJ_VNUM_GOLD_SOME:
					gold += obj->value[1];
					extract_obj( obj );
					break;
				case OBJ_VNUM_COINS:
					silver += obj->value[0];
					gold += obj->value[1];
					extract_obj(obj);
					break;
			}
		}

		obj_to_room(create_money(gold,silver),ch->in_room);
		act("$n drops some coins.",ch,NULL,NULL,TO_ROOM);
		send_to_char("OK.\n\r",ch);
		return;
	}

	if (str_cmp(arg,"all") && str_prefix("all.",arg)){
		if ( (obj = get_obj_carry(ch,arg,ch)) == NULL ){
			send_to_char("You do not have that item.\n\r",ch);
			return;
		}

		if (!can_drop_obj(ch,obj)){
			send_to_char("You can't let go of it.\n\r",ch);
			return;
		}

		obj_from_char(obj);
		obj_to_room(obj,ch->in_room);
		act("$n drops $p.",ch,obj,NULL,TO_ROOM);
		act("You drop $p.",ch,obj,NULL,TO_CHAR);
		if (HAS_TRIGGER_OBJ(obj,TRIG_DROP))
			p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_DROP);
		if ( HAS_TRIGGER_ROOM(ch->in_room,TRIG_DROP))
			p_give_trigger(NULL,NULL,ch->in_room,ch,obj,TRIG_DROP);

		if (obj && IS_OBJ_STAT(obj,ITM_MELT_DROP)){
			act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
			act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
			extract_obj(obj);
		}
	}
	else{
		found = false;
		for ( obj = ch->carrying; obj != NULL; obj = obj_next ){
			obj_next = obj->next_content;

			if ((arg[3] == '\0' || is_name(&arg[4],obj->name)) && can_see_obj(ch,obj) && obj->wear_loc == WEAR_NONE && can_drop_obj(ch,obj)){
				found = true;
				obj_from_char(obj);
				obj_to_room(obj,ch->in_room);
				act("$n drops $p.",ch,obj,NULL,TO_ROOM);
				act("You drop $p.",ch,obj,NULL,TO_CHAR);
        		if (HAS_TRIGGER_OBJ(obj,TRIG_DROP))
					p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_DROP);
				if (HAS_TRIGGER_ROOM(ch->in_room,TRIG_DROP))
					p_give_trigger(NULL,NULL,ch->in_room,ch,obj,TRIG_DROP);

				if (obj && IS_OBJ_STAT(obj,ITM_MELT_DROP)){
					act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
					act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
					extract_obj(obj);
				}
			}
		}

		if (!found){
			if (arg[3] == '\0')
				act("You are not carrying anything.",ch,NULL,arg,TO_CHAR);
			else
				act("You are not carrying any $T.",ch,NULL,&arg[4],TO_CHAR);
		}
	}
}

void do_give(CHAR_DATA *ch,char *argument){
	char arg1[MIL],arg2[MIL],arg3[MIL],buf[MSL];
	int count = 0;
	CHAR_DATA *victim;
	OBJ_DATA *obj,*obj_next,*obj2,*obj2_next;

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0'){
		send_to_char("Give what to whom?\n\r",ch);
		return;
	}

	if (is_number(arg1)){
		int amount;
		bool silver;

		amount   = atoi(arg1);
		if (amount <= 0 || (str_cmp(arg2,"coins") && str_cmp(arg2,"coin") && str_cmp(arg2,"gold") && str_cmp(arg2,"silver"))){
			send_to_char("Sorry, you can't do that.\n\r",ch);
			return;
		}
		silver = str_cmp(arg2,"gold");
		argument = one_argument(argument,arg2);
		if (arg2[0] == '\0'){
			send_to_char("Give what to whom?\n\r",ch);
			return;
		}

		if ((victim = get_char_room(ch,NULL,arg2)) == NULL){
			send_to_char("They aren't here.\n\r",ch);
			return;
		}

		if ((!silver && ch->gold < amount) || (silver && ch->silver < amount)){
			send_to_char("You haven't got that much.\n\r",ch);
			return;
		}

		if (silver){
			ch->silver		-= amount;
			victim->silver 	+= amount;
		}
		else{
			ch->gold		-= amount;
			victim->gold	+= amount;
		}

		sprintf(buf,"$n gives you %d %s.",amount,silver ? "silver" : "gold");
		act(buf,ch,NULL,victim,TO_VICT);
		act("$n gives $N some coins.", ch,NULL,victim,TO_NOTVICT);
		sprintf(buf,"You give $N %d %s.",amount,silver ? "silver" : "gold");
		act(buf,ch,NULL,victim,TO_CHAR);

		//Bribe trigger
		if (IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_BRIBE))
			p_bribe_trigger(victim,ch,silver ? amount : amount * 100);
		if ((IS_NPC(victim) && victim->isact(AT_IS_CHANGER)) || (IS_NPC(victim) && victim->isact(AT_BANKER))){
			int change = (silver ? 95 * amount / 100 / 100 : 95 * amount);

			if (!silver && change > victim->silver)
	    		victim->silver += change;
			if (silver && change > victim->gold)
				victim->gold += change;
			if (change < 1 && can_see(victim,ch)){
				act("$n tells you 'I'm sorry, you did not give me enough to change.'",victim,NULL,ch,TO_VICT);
				ch->reply = victim;
				sprintf(buf,"%d %s %s",amount,silver ? "silver" : "gold",ch->name);
				do_function(victim,&do_give,buf);
			}
			else if (can_see(victim,ch)){
				sprintf(buf,"%d %s %s",change,silver ? "gold" : "silver",ch->name);
				do_function(victim,&do_give,buf);
				if (silver){
					sprintf(buf,"%d silver %s",(95 * amount / 100 - change * 100),ch->name);
					do_function(victim,&do_give,buf);
				}
				act("$n tells you 'Thank you, come again.'",victim,NULL,ch,TO_VICT);
				ch->reply = victim;
			}
		}
		return;
	}
	if (str_cmp(arg1,"all") && str_prefix("all.",arg1)){
		if ((obj = get_obj_carry(ch,arg1,ch)) == NULL){
			send_to_char("You do not have that item.\n\r",ch);
			return;
		}
		if (obj->wear_loc != WEAR_NONE){
			send_to_char("You must remove it first.\n\r",ch);
			return;
		}
		if ((victim = get_char_room(ch,NULL,arg2)) == NULL){
			send_to_char("They aren't here.\n\r",ch);
			return;
		}
		if (IS_NPC(victim) && victim->pIndexData->pShop != NULL){
			act("$N tells you 'Sorry, you'll have to sell that.'",ch,NULL,victim,TO_CHAR);
			ch->reply = victim;
			return;
		}
		if (!can_drop_obj(ch,obj)){
			send_to_char("You can't let go of it.\n\r",ch);
			return;
		}
		if (victim->carry_number + get_obj_number(obj) > can_carry_n(victim)){
			act("$N has $S hands full.",ch,NULL,victim,TO_CHAR);
			return;
		}
		if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w(victim)){
			act("$N can't carry that much weight.",ch,NULL,victim,TO_CHAR);
			return;
		}
		if (!can_see_obj(victim,obj)){
			act("$N can't see it.",ch,NULL,victim,TO_CHAR);
			return;
		}

		obj_from_char(obj);
		obj_to_char(obj,victim);
		MOBtrigger = false;
		act("$n gives $p to $N.",ch,obj,victim,TO_NOTVICT);
		act("$n gives you $p.",ch,obj,victim,TO_VICT);
		act("You give $p to $N.",ch,obj,victim,TO_CHAR);
		MOBtrigger = true;
		if (HAS_TRIGGER_OBJ(obj,TRIG_GIVE))
			p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_GIVE);
		if (HAS_TRIGGER_ROOM(ch->in_room,TRIG_GIVE))
			p_give_trigger(NULL,NULL,ch->in_room,ch,obj,TRIG_GIVE);

		if (IS_NPC(victim) && HAS_TRIGGER_MOB(victim,TRIG_GIVE))
			p_give_trigger(victim,NULL,NULL,ch,obj,TRIG_GIVE);
	}
	else{
		bool found = false;
		if(!(victim = get_char_room(ch,NULL,arg2))){
			ch->send("They aren't here.\n\r");
			return;
		}
		for(obj = ch->carrying; obj != NULL; obj = obj_next){
			obj_next = obj->next_content;
			if ((arg1[3] == '\0' || is_name(&arg1[4],obj->name)) && can_see_obj(ch,obj) && obj->wear_loc == WEAR_NONE && can_drop_obj(ch,obj)){
				found = true;
				break;
			}
		}
		if(!found){
			if(arg1[3] == '\0')
				act("You are not carrying anything.",ch,NULL,NULL,TO_CHAR);
			else
				act("You are not carrying any $T.",ch,NULL,&arg1[4],TO_CHAR);
		}
		else{
			for(obj2 = ch->carrying;obj2 != NULL;obj2 = obj2_next){
				obj2_next = obj2->next_content;
				if(!str_cmp(obj->name,obj2->name) && can_see_obj(ch,obj2) && obj2->wear_loc == WEAR_NONE && can_drop_obj(ch,obj2) && obj != obj2){
					obj_from_char(obj2);
					obj_to_char(obj2,victim);
					count++;
				}
			}
			obj_from_char(obj);
			obj_to_char(obj,victim);
			if(count == 0){
				act("$n gives $p to $N.",ch,obj,victim,TO_NOTVICT);
				act("$n gives $p to you.",ch,obj,victim,TO_VICT);
				act("You give $p to $N.",ch,obj,victim,TO_CHAR);
			}
			else{
				sprintf(buf,"$n gives $p (%d) to $N.",count+1);
				act(buf,ch,obj,victim,TO_NOTVICT);
				sprintf(buf,"$n gives $p (%d) to you.",count+1);
				act(buf,ch,obj,victim,TO_VICT);
				sprintf(buf,"You give $p (%d) to $N.",count+1);
				act(buf,ch,obj,victim,TO_CHAR);
			}
    		if (HAS_TRIGGER_OBJ(obj,TRIG_GIVE))
				p_give_trigger(NULL,obj,NULL,ch,obj,TRIG_GIVE);
			if (HAS_TRIGGER_ROOM(ch->in_room,TRIG_GIVE))
				p_give_trigger(NULL,NULL,ch->in_room,ch,obj,TRIG_GIVE);
		}
	}
}

void do_envenom(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj;
	AFFECT_DATA af;
	int percent,skill = get_skill(ch,gsn_envenom);

	if (argument[0] == '\0'){
		send_to_char("Envenom what item?\n\r",ch);
		return;
	}

	if(!(obj = get_obj_carry(ch,argument,ch))){
		send_to_char("You don't have that item.\n\r",ch);
		return;
	}

	if (skill < 1){
		send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
		return;
	}

	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON){
		if (IS_OBJ_STAT(obj,ITM_BURN_PROOF)|| IS_OBJ_STAT(obj,ITM_SHOCK_PROOF)){
			act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
			return;
		}

		if (number_percent() < skill){
			act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
			act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
			if (!obj->value[3]){
				obj->value[3] = 1;
				check_improve(ch,gsn_envenom,true,4);
			}
			WAIT_STATE(ch,skill_table[gsn_envenom].beats);
			return;
		}

		act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
		if (!obj->value[3])
			check_improve(ch,gsn_envenom,false,4);
		WAIT_STATE(ch,skill_table[gsn_envenom].beats);
		return;
	}

	if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_NEEDLE){
		if (IS_WEAPON_STAT(obj,WPN_FLAMING)
		||  IS_WEAPON_STAT(obj,WPN_FROST)
		||  IS_WEAPON_STAT(obj,WPN_VAMPIRIC)
		||  IS_WEAPON_STAT(obj,WPN_POWERDRAIN)
		||  IS_WEAPON_STAT(obj,WPN_SHARP)
		||  IS_WEAPON_STAT(obj,WPN_VORPAL)
		||  IS_WEAPON_STAT(obj,WPN_SERRATED)
		||  IS_WEAPON_STAT(obj,WPN_HOLY)
		||  IS_WEAPON_STAT(obj,WPN_DEMONIC)
		||  IS_WEAPON_STAT(obj,WPN_SHOCKING)
		|| IS_OBJ_STAT(obj,ITM_BURN_PROOF)|| IS_OBJ_STAT(obj,ITM_SHOCK_PROOF)){
			act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
			return;
		}

		if (obj->value[3] < 0 || attack_table[obj->value[3]].damage == DAM_BASH){
			send_to_char("You can only envenom edged weapons.\n\r",ch);
			return;
		}

		if (IS_WEAPON_STAT(obj,WPN_POISON) || IS_WEAPON_STAT(obj,WPN_PESTILENCE)){
			act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
			return;
		}

		percent = number_percent();
		if (percent < skill){
			af.where     = TO_WEAPON;
			af.type      = gsn_poison;
			af.level     = ch->level * percent / 100;
			af.duration  = ch->level/2 * percent / 100;
			af.location  = 0;
			af.modifier  = 0;
			af.bitvector = WPN_POISON;
			affect_to_obj(obj,&af);

			act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
			act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
			check_improve(ch,gsn_envenom,true,3);
			WAIT_STATE(ch,skill_table[gsn_envenom].beats);
			return;
		}
		else{
			act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
			check_improve(ch,gsn_envenom,false,3);
			WAIT_STATE(ch,skill_table[gsn_envenom].beats);
			return;
		}
	}

	act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
}

void do_ready(CHAR_DATA *ch,char *argument)
{
    OBJ_DATA *obj;
    AFFECT_DATA af;
    int percent,skill;

    if (argument[0] == '\0')
    {
		send_to_char("Ready what?\n\r",ch);
		return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== NULL)
    {
		send_to_char("You don't have that weapon.\n\r",ch);
		return;
    }

    if ((skill = get_skill(ch,gsn_ready)) < 1)
    {
		send_to_char("You poke yourself as you try to ready a weapon!\n\r",ch);
		return;
    }

    if (obj->item_type != ITEM_WEAPON)
    {
		send_to_char("What, you wanna slap the enemy with a sandwich?\n\r",ch);
		return;
    }

	percent = number_percent();
	if (percent < skill)
	{
 
		af.where     = TO_WEAPON;
		af.type      = gsn_ready;
		af.level     = ch->level * percent / 100;
		af.duration  = ch->level/2 * percent / 100;
		af.location  = 0;
		af.modifier  = 0;
		af.bitvector = WPN_READIED;
		affect_to_obj(obj,&af);

		act("$n readies $p as $s secondary wield.",ch,obj,NULL,TO_ROOM);
		act("You ready $p.",ch,obj,NULL,TO_CHAR);
		check_improve(ch,gsn_ready,true,3);
		WAIT_STATE(ch,skill_table[gsn_ready].beats);
		return;
    }
	else
	{
	    act("You fumble and improperly ready $p.",ch,obj,NULL,TO_CHAR);
	    check_improve(ch,gsn_ready,false,3);
	    WAIT_STATE(ch,skill_table[gsn_ready].beats);
	    return;
    }
 
    act("You can't ready $p.",ch,obj,NULL,TO_CHAR);
    return;
}
void do_fill(CHAR_DATA *ch,char *argument)
{
    char arg[MIL], buf[MSL];
    OBJ_DATA *obj, *fountain;
    bool found;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
		send_to_char("Fill what?\n\r",ch);
		return;
    }

    if ((obj = get_obj_carry(ch,arg,ch)) == NULL)
    {
		send_to_char("You do not have that item.\n\r",ch);
		return;
    }

    if (obj->item_type != ITEM_DRINK_CON)
    {
		send_to_char("You can't fill that.\n\r",ch);
		return;
    }

    if (obj->value[1] >= obj->value[0])
    {
		send_to_char("Your container is full.\n\r",ch);
		return;
    }

    found = false;
    for ( fountain = ch->in_room->contents; fountain != NULL && !found; fountain = fountain->next_content )
		if (fountain->item_type == ITEM_FOUNTAIN)
		{
			found = true;
			break;
		}

    if (!found)
    {
		if (ch->in_room->sector_type != SECT_RIVER)
			send_to_char("There is no fountain here!\n\r",ch);
		else
		{
			act("You fill $p with water from the river.",ch,obj,NULL,TO_CHAR);
			act("$n fills $p with water from the river.",ch,obj,NULL,TO_ROOM);
			if (number_percent() < 10)
				obj->value[3] = 1;
			obj->value[2] = LIQ_WATER;
			obj->value[1] = obj->value[0];
		}
		return;
    }

    if (obj->value[1] != 0 && obj->value[2] != fountain->value[2])
    {
		send_to_char("There is already another liquid in it.\n\r",ch);
		return;
    }

    sprintf(buf,"You fill $p with %s from $P.",liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_CHAR);
    sprintf(buf,"$n fills $p with %s from $P.",liq_table[fountain->value[2]].liq_name);
    act(buf,ch,obj,fountain,TO_ROOM);
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];
    return;
}

void do_pour(CHAR_DATA *ch,char *argument)
{
    char arg[MSL],buf[MSL];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument(argument,arg);
    
    if (arg[0] == '\0' || argument[0] == '\0')
    {
		send_to_char("Pour what into what?\n\r",ch);
		return;
    }
    

    if ((out = get_obj_carry(ch,arg,ch)) == NULL)
    {
		send_to_char("You don't have that item.\n\r",ch);
		return;
    }

    if (out->item_type != ITEM_DRINK_CON)
    {
		send_to_char("That's not a drink container.\n\r",ch);
		return;
    }

    if (!str_cmp(argument,"out"))
    {
		if (out->value[1] == 0)
		{
			send_to_char("It's already empty.\n\r",ch);
			return;
		}

		out->value[1] = 0;
		out->value[3] = 0;
		sprintf(buf,"You invert $p, spilling %s all over the ground.", liq_table[out->value[2]].liq_name);
		act(buf,ch,out,NULL,TO_CHAR);
		sprintf(buf,"$n inverts $p, spilling %s all over the ground.", liq_table[out->value[2]].liq_name);
		act(buf,ch,out,NULL,TO_ROOM);
		return;
    }

    if ((in = get_obj_here(ch, NULL,argument)) == NULL)
    {
		vch = get_char_room(ch, NULL, argument);

		if (vch == NULL)
		{
			send_to_char("Pour into what?\n\r",ch);
			return;
		}

		in = (get_eq_char(vch,WEAR_HOLD_R) == NULL ? get_eq_char(vch,WEAR_HOLD_L) : get_eq_char(vch,WEAR_HOLD_R));

		if (in == NULL)
		{
			send_to_char("They aren't holding anything.",ch);
 			return;
		}
    }

    if (in->item_type != ITEM_DRINK_CON)
    {
		send_to_char("You can only pour into other drink containers.\n\r",ch);
		return;
    }
    
    if (in == out)
    {
		send_to_char("You cannot change the laws of physics!\n\r",ch);
		return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2])
    {
		send_to_char("They don't hold the same liquid.\n\r",ch);
		return;
    }

    if (out->value[1] == 0)
    {
		act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
		return;
    }

    if (in->value[1] >= in->value[0])
    {
		act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
		return;
    }

    amount = UMIN(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];
    
    if (vch == NULL)
    {
    	sprintf(buf,"You pour %s from $p into $P.", liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_CHAR);
    	sprintf(buf,"$n pours %s from $p into $P.", liq_table[out->value[2]].liq_name);
    	act(buf,ch,out,in,TO_ROOM);
    }
    else
    {
        sprintf(buf,"You pour some %s for $N.", liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_CHAR);
		sprintf(buf,"$n pours you some %s.", liq_table[out->value[2]].liq_name);
		act(buf,ch,NULL,vch,TO_VICT);
        sprintf(buf,"$n pours some %s for $N.", liq_table[out->value[2]].liq_name);
        act(buf,ch,NULL,vch,TO_NOTVICT);
    }
}

void do_drink(CHAR_DATA *ch,char *argument){
    char arg[MIL];
    OBJ_DATA *obj;
	AFFECT_DATA af;
    int amount,liquid = LIQ_WATER;

    one_argument(argument,arg);

    if (!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->condition[COND_FULL] > 40){
		send_to_char("You're too full to drink more.\n\r",ch);
		return;
    }

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] >= 100){
		send_to_char("You fail to reach your mouth.  *Hic*\n\r",ch);
		return;
    }

    if (!arg[0]){
		for (obj = ch->in_room->contents;obj;obj = obj->next_content)
			if (obj->item_type == ITEM_FOUNTAIN)
				break;

		if (!obj){
			send_to_char("Drink what?\n\r",ch);
			return;
		}
		if ((liquid = obj->value[2]) < 0){
			bug("Do_drink: bad liquid number %d.",liquid);
			liquid = obj->value[2] = 0;
		}
		amount = liq_table[liquid].liq_affect[4];
    }
    else{
		if (!str_cmp(arg,"river")){
			if (ch->in_room->sector_type != SECT_RIVER)
				send_to_char("You may only drink from rivers.\n\r",ch);
			else{
				send_to_char("You lean over and take a drink from the river.\n\r",ch);
				act("$n sticks $s face in the river and takes a drink of water.",ch,NULL,NULL,TO_ROOM);
				amount = liq_table[liquid].liq_affect[4];
				if (number_percent() < 10){
					send_to_char("You drink something gross from the water!\n\r",ch);
					act("$n gags and sputters!",ch,NULL,NULL,TO_ROOM);
					affect_join(ch,TO_AFFECTS,gsn_poison,ch->level,1,number_range(1,ch->level/4+2),APPLY_STR,-1,AF_POISON);
				}
			}
		}
		else if (!(obj = get_obj_here(ch,NULL,arg))){
			send_to_char("You can't find it.\n\r",ch);
			return;
		}
		else{
			switch (obj->item_type){
				default:
					send_to_char("You can't drink from that.\n\r",ch);
					return;
				case ITEM_FOUNTAIN:
					if ((liquid = obj->value[2]) < 0){
						bug("Do_drink: bad liquid number %d.",liquid);
						liquid = obj->value[2] = 0;
					}
					amount = liq_table[liquid].liq_affect[4];
					break;
				case ITEM_DRINK_CON:
					if (obj->value[1] < 1){
						send_to_char("It is already empty.\n\r",ch);
						return;
					}
					if ((liquid = obj->value[2])  < 0){
						bug("Do_drink: bad liquid number %d.",liquid);
						liquid = obj->value[2] = 0;
					}
					amount = liq_table[liquid].liq_affect[4];
					amount = UMIN(amount, obj->value[1]);
					break;
			}
		}
	}
	if(obj){
		act("$n drinks $T from $p.",ch,obj,liq_table[liquid].liq_name,TO_ROOM);
		act("You drink $T from $p.",ch,obj,liq_table[liquid].liq_name,TO_CHAR);
	}

	if (ch->israce("dwarf"))
		gain_condition(ch,COND_DRUNK,liq_table[liquid].liq_affect[COND_DRUNK]/2);
	else
		gain_condition(ch,COND_DRUNK,liq_table[liquid].liq_affect[COND_DRUNK]);

	gain_condition(ch,COND_FULL,  liq_table[liquid].liq_affect[COND_FULL]);
	gain_condition(ch,COND_THIRST,amount * liq_table[liquid].liq_affect[COND_THIRST]);
	gain_condition(ch,COND_HUNGER,amount + liq_table[liquid].liq_affect[COND_HUNGER]);

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
		send_to_char("You feel drunk.\n\r",ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )
		send_to_char("You are full.\n\r",ch);
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
		send_to_char("Your thirst is quenched.\n\r",ch);
	
    if(obj->value[3]){
		act("$n chokes and gags.",ch,NULL,NULL,TO_ROOM);
		send_to_char("You choke and gag.\n\r",ch);
		affect_join(ch,TO_AFFECTS,gsn_poison,number_fuzzy(amount),1,3*amount,APPLY_NONE,0,AF_POISON);
    }
	
    if (obj && obj->value[0] > 0)
        obj->value[1]--;
    return;
}

void do_eat(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	OBJ_DATA *obj;

	one_argument(argument,arg);
	if (!arg[0]){
		send_to_char("Eat what?\n\r",ch);
		return;
	}

	if (!(obj = get_obj_carry(ch,arg,ch))){
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}

	if (!IS_IMMORTAL(ch)){
		if (obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL){
			send_to_char("That's not edible.\n\r",ch);
			return;
		}

		if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40){
			send_to_char("You are too full to eat more.\n\r",ch);
			return;
		}
	}

	act("$n eats $p.",ch,obj,NULL,TO_ROOM);
	act("You eat $p.",ch,obj,NULL,TO_CHAR);

	switch (obj->item_type){
		case ITEM_FOOD:
			if (!IS_NPC(ch)){
				gain_condition(ch,COND_FULL,obj->value[1]);
				gain_condition(ch,COND_HUNGER,obj->value[0]);
				if (ch->pcdata->condition[COND_HUNGER] > 0)
					send_to_char("You are no longer hungry.\n\r",ch);
				if (ch->pcdata->condition[COND_FULL] > 40)
					send_to_char("You are full.\n\r",ch);
			}
			if (obj->value[3] != 0){
				/* The food was poisoned! */
				AFFECT_DATA af;

				act("$n chokes and gags.",ch,NULL,NULL,TO_ROOM);
				send_to_char("You choke and gag.\n\r",ch);

				affect_join(ch,TO_AFFECTS,gsn_poison,number_fuzzy(obj->value[0]),1,2 * obj->value[0],APPLY_NONE,0,AF_POISON);
			}
			break;
		case ITEM_PILL:
			obj_cast_spell(obj->value[1], obj->value[0], ch, ch, NULL );
			obj_cast_spell(obj->value[2], obj->value[0], ch, ch, NULL );
			obj_cast_spell(obj->value[3], obj->value[0], ch, ch, NULL );
			break;
	}
	extract_obj(obj);
}

bool remove_obj(CHAR_DATA *ch,int iWear,bool fReplace){
    OBJ_DATA *obj;

    if ((obj = get_eq_char(ch,iWear)) == NULL)
		return true;

    if (!fReplace)
		return false;

    if (IS_SET(obj->extra_flags,ITM_NOREMOVE)){
		act("You can't remove $p.",ch,obj,NULL,TO_CHAR);
		return false;
    }

    unequip_char(ch,obj);
    act("$n stops using $p.",ch,obj,NULL,TO_ROOM);
    act("You stop using $p.",ch,obj,NULL,TO_CHAR);
    return true;
}

void wear_obj(CHAR_DATA *ch,OBJ_DATA *obj,bool fReplace,bool canseethis){
	if (ch->level < obj->level){
		printf_to_char(ch,"You must be level %d to use this object.\n\r",obj->level);
		if (canseethis == true)
			act("$n tries to use $p, but is too inexperienced.",ch,obj,NULL,TO_ROOM);
		return;
	}

	if (obj->item_type == ITEM_LIGHT){
		if (CAN_WEAR(obj,ITEM_WEAR_FLOAT_LIGHT)){
			if (!remove_obj(ch,WEAR_FLOAT_LIGHT,fReplace))
				return;
			if (canseethis == true){
				act("$n releases $p to float beside $m.",ch,obj,NULL,TO_ROOM);
				act("You release $p to float beside you.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_FLOAT_LIGHT);
			return;
		}
		if (!remove_obj(ch,WEAR_LIGHT,fReplace))
			return;
		if (calcHands(ch) > 1){
			if(get_eq_char(ch,WEAR_SECONDARY) && remove_obj(ch,WEAR_SECONDARY,fReplace)){
			}
			else if(get_eq_char(ch,WEAR_SHIELD) && remove_obj(ch,WEAR_SHIELD,fReplace)){
			}
			else if(get_eq_char(ch,WEAR_HOLD_L) && remove_obj(ch,WEAR_HOLD_L,fReplace)){
			}
			else if(get_eq_char(ch,WEAR_HOLD_R) && remove_obj(ch,WEAR_HOLD_R,fReplace)){
			}
			else if(get_eq_char(ch,WEAR_WIELD) && remove_obj(ch,WEAR_WIELD,fReplace)){
			}
			else{
				send_to_char("You need to free up a hand first.\n\r",ch);
				return;
			}
		}
		if (canseethis == true){
			act("$n lights $p and holds it.",ch,obj,NULL,TO_ROOM);
			act("You light $p and hold it.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_LIGHT);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_FINGER)){
		if (get_eq_char(ch,WEAR_FINGER_L) != NULL && get_eq_char(ch,WEAR_FINGER_R) != NULL && !remove_obj(ch,WEAR_FINGER_L,fReplace) && !remove_obj(ch,WEAR_FINGER_R,fReplace))
			return;
		if (get_eq_char(ch,WEAR_FINGER_L) == NULL){
			if (canseethis == true){
				act("$n wears $p on $s left finger.",ch,obj,NULL,TO_ROOM);
				act("You wear $p on your left finger.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_FINGER_L);
			return;
		}
		if (get_eq_char(ch,WEAR_FINGER_R) == NULL){
			if (canseethis == true){
				act("$n wears $p on $s right finger.",ch,obj,NULL,TO_ROOM);
				act("You wear $p on your right finger.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_FINGER_R);
			return;
		}
		send_to_char("You already wear two rings.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_EAR)){
		if (get_eq_char(ch,WEAR_EAR_L) != NULL && get_eq_char(ch,WEAR_EAR_R) != NULL && !remove_obj(ch,WEAR_EAR_L,fReplace) && !remove_obj(ch,WEAR_EAR_R,fReplace))
			return;
		if (get_eq_char(ch,WEAR_EAR_L) == NULL){
			if (canseethis == true){
				act("$n wears $p in $s left ear.",ch,obj,NULL,TO_ROOM);
				act("You wear $p in your left ear.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_EAR_L);
			return;
		}
		if (get_eq_char(ch,WEAR_EAR_R) == NULL){
			if (canseethis == true){
				act("$n wears $p in $s right ear.",ch,obj,NULL,TO_ROOM);
				act("You wear $p in your right ear.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_EAR_R);
			return;
		}
		send_to_char("You already wear two earrings.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_NECK)){
		if (get_eq_char(ch,WEAR_NECK_1) != NULL && get_eq_char(ch,WEAR_NECK_2) != NULL && !remove_obj(ch,WEAR_NECK_1,fReplace) && !remove_obj(ch,WEAR_NECK_2,fReplace))
			return;
		if (get_eq_char(ch,WEAR_NECK_1) == NULL){
			if (canseethis == true){
				act("$n wears $p around $s neck.",ch,obj,NULL,TO_ROOM);
				act("You wear $p around your neck.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_NECK_1);
			return;
		}
		if (get_eq_char(ch,WEAR_NECK_2) == NULL){
			if (canseethis == true){
				act("$n wears $p around $s neck.",ch,obj,NULL,TO_ROOM);
				act("You wear $p around your neck.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_NECK_2);
			return;
		}
		send_to_char("You already wear two neck items.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_ELBOW)){
		if ( get_eq_char(ch,WEAR_ELBOW_L) != NULL && get_eq_char(ch,WEAR_ELBOW_R) != NULL && !remove_obj(ch,WEAR_ELBOW_L,fReplace) && !remove_obj(ch,WEAR_ELBOW_R,fReplace))
			return;
		if (get_eq_char(ch,WEAR_ELBOW_L) == NULL){
			if (canseethis == true){
				act("$n straps $p on $s elbow.",ch,obj,NULL,TO_ROOM);
				act("You strap $p on your elbow.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_ELBOW_L);
			return;
		}
		if (get_eq_char(ch,WEAR_ELBOW_R) == NULL){
			if (canseethis == true){
				act("$n straps $p on $s elbow.",ch,obj,NULL,TO_ROOM);
				act("You strap $p on your elbow.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_ELBOW_R);
			return;
		}
		send_to_char("You already wear two elbow items.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_SHIN)){
		if (get_eq_char(ch,WEAR_SHIN_L) != NULL && get_eq_char(ch,WEAR_SHIN_R) != NULL && !remove_obj(ch,WEAR_SHIN_L,fReplace) && !remove_obj(ch,WEAR_SHIN_R,fReplace))
			return;
		if (get_eq_char(ch,WEAR_SHIN_L) == NULL){
			if (canseethis == true){
				act("$n straps $p on $s shin.",ch,obj,NULL,TO_ROOM);
				act("You strap $p on your shin.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_SHIN_L);
			return;
		}
		if (get_eq_char(ch,WEAR_SHIN_R) == NULL){
			if (canseethis == true){
				act("$n straps $p on $s shin.",ch,obj,NULL,TO_ROOM);
				act("You strap $p on your shin.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_SHIN_R);
			return;
		}
		send_to_char("You already wear two shin items.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_KNEE)){
		if (get_eq_char(ch,WEAR_KNEE_L) != NULL && get_eq_char(ch,WEAR_KNEE_R) != NULL && !remove_obj(ch,WEAR_KNEE_L,fReplace) && !remove_obj(ch,WEAR_KNEE_R,fReplace))
			return;
		if (get_eq_char(ch,WEAR_KNEE_L) == NULL){
			if (canseethis == true){
				act("$n straps $p on $s left knee.",ch,obj,NULL,TO_ROOM);
				act("You strap $p on your left knee.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_KNEE_L);
			return;
		}
		if (get_eq_char(ch,WEAR_KNEE_R) == NULL){
			if (canseethis == true){
				act("$n straps $p on $s right knee.",ch,obj,NULL,TO_ROOM);
				act("You strap $p on your right knee.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_KNEE_R);
			return;
		}
		send_to_char("You already wear two knee items.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_TORSO)){
		if (!remove_obj(ch,WEAR_TORSO,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p on $s torso.",ch,obj,NULL,TO_ROOM);
			act("You wear $p on your torso.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_TORSO);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_HEAD)){
		if (!remove_obj(ch,WEAR_HEAD,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p on $s head.",ch,obj,NULL,TO_ROOM);
			act("You wear $p on your head.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_HEAD);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_LEGS)){
		if (!remove_obj(ch,WEAR_LEGS,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p on $s legs.",ch,obj,NULL,TO_ROOM);
			act("You wear $p on your legs.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_LEGS);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_SHOULDER)){
		if (!remove_obj(ch,WEAR_SHOULDER,fReplace))
			return;
		if (canseethis == true){
			act("$n straps $p on $s shoulder.",ch,obj,NULL,TO_ROOM);
			act("You strap $p on your shoulder.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_SHOULDER);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_QUIVER)){
		if (!remove_obj(ch,WEAR_QUIVER,fReplace))
			return;
		if (canseethis == true){
			act("$n straps $p on $s shoulder.",ch,obj,NULL,TO_ROOM);
			act("You strap $p on your shoulder.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_QUIVER);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_FEET)){
		if (!remove_obj(ch,WEAR_FEET,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p on $s feet.",ch,obj,NULL,TO_ROOM);
			act("You wear $p on your feet.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_FEET);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_HANDS)){
		if (!remove_obj(ch,WEAR_HANDS,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p on $s hands.",ch,obj,NULL,TO_ROOM);
			act("You wear $p on your hands.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_HANDS);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_TATTOO)){
		if (canseethis == true){
			act("$n gets $p etched onto $s flesh.",ch,obj,NULL,TO_ROOM);
			act("You have $p tattooed onto your body.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_TATTOO);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_FACE)){
		if (!remove_obj(ch,WEAR_FACE,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p on $s face.",ch,obj,NULL,TO_ROOM);
			act("You wear $p on your face.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_FACE);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_PENDANT)){
		if (!remove_obj(ch,WEAR_PENDANT,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p as a pendant.",ch,obj,NULL,TO_ROOM);
			act("You wear $p as a pendant.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_PENDANT);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_ARMS)){
		if (!remove_obj(ch,WEAR_ARMS,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p on $s arms.",ch,obj,NULL,TO_ROOM);
			act("You wear $p on your arms.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_ARMS);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_ABOUT)){
		if (!remove_obj(ch,WEAR_ABOUT,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p about $s torso.",ch,obj,NULL,TO_ROOM);
			act("You wear $p about your torso.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_ABOUT);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_WAIST)){
		if (!remove_obj(ch,WEAR_WAIST,fReplace))
			return;
		if (canseethis == true){
			act("$n wears $p about $s waist.",ch,obj,NULL,TO_ROOM);
			act("You wear $p about your waist.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_WAIST);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_WRIST)){
		if (get_eq_char(ch,WEAR_WRIST_L) != NULL
		&&   get_eq_char(ch,WEAR_WRIST_R) != NULL
		&&   !remove_obj(ch,WEAR_WRIST_L, fReplace)
		&&   !remove_obj(ch,WEAR_WRIST_R, fReplace))
			return;

		if (get_eq_char(ch,WEAR_WRIST_L) == NULL){
			if (canseethis == true){
				act("$n wears $p around $s left wrist.",ch,obj,NULL,TO_ROOM);
				act("You wear $p around your left wrist.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_WRIST_L);
			return;
		}

		if (get_eq_char(ch,WEAR_WRIST_R) == NULL){
			if (canseethis == true){
				act("$n wears $p around $s right wrist.", ch,obj,NULL,TO_ROOM);
				act("You wear $p around your right wrist.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_WRIST_R);
			return;
		}
		send_to_char("You already wear two wrist items.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_ANKLE)){
		if ( get_eq_char(ch,WEAR_ANKLE_L) != NULL
		&&   get_eq_char(ch,WEAR_ANKLE_R) != NULL
		&&   !remove_obj(ch,WEAR_ANKLE_L,fReplace)
		&&   !remove_obj(ch,WEAR_ANKLE_R,fReplace))
			return;

		if (get_eq_char(ch,WEAR_ANKLE_L) == NULL){
			if (canseethis == true){
				act("$n wears $p around $s left ankle.",ch,obj,NULL,TO_ROOM);
				act("You wear $p around your left ankle.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_ANKLE_L);
			return;
		}
		if (get_eq_char(ch,WEAR_ANKLE_R) == NULL){
			if (canseethis==true){
				act("$n wears $p around $s right ankle.",ch,obj,NULL,TO_ROOM);
				act("You wear $p around your right ankle.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_ANKLE_R);
			return;
		}
		send_to_char("You already have two ankle items on.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_SHIELD)){
		if (!remove_obj(ch,WEAR_SHIELD,fReplace))
		   return;
		if (calcHands(ch) > 1){
			send_to_char("You need to free up a hand first.\n\r",ch);
			return;
		}
		if (obj->item_type != ITEM_SHIELD){
			ch->send("You can only use shields as shields!... and here I was using a soda can.\n\r");
			return;
		}

		if (get_eq_char(ch,WEAR_SECONDARY) != NULL && get_eq_char(ch,WEAR_SECONDARY)->item_type != ITEM_SHIELD){
			send_to_char("You cannot use a shield while using two weapons.\n\r",ch);
			return;
		}
		if (canseethis == true)
		{
			act("$n wears $p as a shield.",ch,obj,NULL,TO_ROOM);
			act("You wear $p as a shield.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_SHIELD);
		return;
	}

	if ( CAN_WEAR(obj,ITEM_WIELD)){
		int sn,skill;

		if (!remove_obj(ch,WEAR_WIELD,fReplace))
			return;

		if (!IS_NPC(ch) && get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield * 10)){
			send_to_char("It is too heavy for you to wield.\n\r",ch);
			return;
		}

		if (IS_WEAPON_STAT(obj,WPN_TWO_HANDS)){
			if (!IS_NPC(ch) && ch->size < SIZE_LARGE && calcHands(ch) > 0){
				send_to_char("You need two hands free for that weapon.\n\r",ch);
				return;
			}
		}
		else if (calcHands(ch) > 1){
			send_to_char("You need to free up a hand first.\n\r",ch);
			return;
		}

		if (canseethis == true){
			char buf[10];
			sprintf(buf,"%s",ch->lefty ? "left" : "right");
			act("$n wields $p in $s $d hand.",ch,obj,buf,TO_ROOM);
			act("You wield $p in your $d hand.",ch,obj,buf,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_WIELD);

		sn = get_weapon_sn(ch,false);

		if (sn == gsn_combatives)
		   return;

		skill = get_weapon_skill(ch,sn);

		if (skill >= 100)		act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
		else if (skill > 85)	act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
		else if (skill > 70)	act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
		else if (skill > 50)	act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
		else if (skill > 25)	act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
		else if (skill > 1)		act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
		else					act("You don't even know which end is up on $p.",ch,obj,NULL,TO_CHAR);
		return;
	}

	if (CAN_WEAR(obj,ITEM_HOLD)){
		if (calcHands(ch) > 1){
			send_to_char("You need to free up a hand first.\n\r",ch);
			return;
		}

		if ( get_eq_char(ch,WEAR_HOLD_L) != NULL && get_eq_char(ch,WEAR_HOLD_R) != NULL && !remove_obj(ch,WEAR_HOLD_L,fReplace) && !remove_obj(ch,WEAR_HOLD_R,fReplace))
			return;

		if (get_eq_char(ch,WEAR_HOLD_L) == NULL){
			if (canseethis == true){
				act("$n holds $p in $s hand.",ch,obj,NULL,TO_ROOM);
				act("You hold $p in your hand.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_HOLD_L);
			return;
		}

		if (get_eq_char(ch,WEAR_HOLD_R) == NULL){
			if (canseethis == true){
				act("$n holds $p in $s hand.",ch,obj,NULL,TO_ROOM);
				act("You hold $p in your hand.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_HOLD_R);
			return;
		}
		send_to_char("You already hold two items.\n\r",ch);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_FLOAT)){
		if (!remove_obj(ch,WEAR_FLOAT,fReplace))
			return;
		if (canseethis==true){
			act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
			act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_FLOAT);
		return;
	}

	if (fReplace)
		send_to_char("You can't wear, wield, or hold that.\n\r",ch);
}

void newwear_obj(CHAR_DATA *ch,OBJ_DATA *obj,bool fReplace,bool canseethis){
	if (ch->level < obj->level)
	{
		printf_to_char(ch,"You must be level %d to use this object.\n\r",obj->level);
		if (canseethis == true)
			act("$n tries to use $p, but is too inexperienced.",ch,obj,NULL,TO_ROOM);
		return;
	}

	if (obj->item_type == ITEM_LIGHT)
	{
		if (!remove_obj(ch,WEAR_LIGHT,fReplace))
			return;

		if (calcHands(ch) > 1)
		{
			send_to_char("You need to free up a hand first.\n\r",ch);
			return;
		}

		if (canseethis == true)
		{
			act("$n lights $p and holds it.",ch,obj,NULL,TO_ROOM);
			act("You light $p and hold it.",ch,obj,NULL,TO_CHAR);
		}
		equip_char(ch,obj,WEAR_LIGHT);
		return;
	}

	if (CAN_WEAR(obj,ITEM_WEAR_FINGER))
	{
		if (get_eq_char(ch,WEAR_FINGER_L) != NULL && get_eq_char(ch,WEAR_FINGER_R) != NULL && !remove_obj(ch,WEAR_FINGER_L,fReplace) && !remove_obj(ch,WEAR_FINGER_R,fReplace))
			return;

		if (get_eq_char(ch,WEAR_FINGER_L) == NULL)
		{
			if (canseethis == true)
			{
				act("$n wears $p on $s left finger.",ch,obj,NULL,TO_ROOM);
				act("You wear $p on your left finger.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_FINGER_L);
			return;
		}

		if (get_eq_char(ch,WEAR_FINGER_R) == NULL)
		{
			if (canseethis == true)
			{
				act("$n wears $p on $s right finger.",ch,obj,NULL,TO_ROOM);
				act("You wear $p on your right finger.",ch,obj,NULL,TO_CHAR);
			}
			equip_char(ch,obj,WEAR_FINGER_R);
			return;
		}
		send_to_char("You already wear two rings.\n\r",ch);
		return;
	}

	if (fReplace)
		send_to_char("You can't wear, wield, or hold that.\n\r",ch);
	return;
	}

void do_wear(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	OBJ_DATA *obj,*dual,*obj_next;
	int cHands;

	one_argument(argument,arg);

	if(!arg[0]){
		send_to_char("Wear, wield, or hold what?\n\r",ch);
		return;
	}
	if(ch->morph){
		ch->send("It wouldn't fit!\n\r");
		return;
	}

	if(!str_cmp(arg,"all")){
		for(obj = ch->carrying;obj;obj = obj_next){
			obj_next = obj->next_content;
			if (obj->wear_loc != WEAR_NONE)
				continue;
			if (!check_armor(ch,obj)){
				printf_to_char(ch,"You fumble and fail to properly wear %s.\n\r",obj->short_descr);
				continue;
			}
			if (obj->item_type == ITEM_WEAPON){
				if (get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10)){
					printf_to_char(ch,"You aren't strong enough to wield %s.\n\r",obj->short_descr);
					continue;
				}
				dual = get_eq_char(ch,WEAR_SECONDARY);
				if (dual != NULL){
					if (!weapon_table[obj->value[0]].candual){
						send_to_char("You can't wield a secondary weapon with that primary.\n\r",ch);
						continue;
					}
					else if (!check_dual_blade(ch,obj,dual)){
						send_to_char("You can't wield that secondary weapon with that primary.\n\r",ch);
						continue;
					}
				}
			}
			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch,obj))
				wear_obj(ch,obj,false,true);
		}
		return;
	}
	else{
		if(!(obj = get_obj_carry(ch,arg,ch))){
			send_to_char("You do not have that item.\n\r",ch);
			return;
		}
		if(!check_armor(ch,obj)){
			printf_to_char(ch,"You fumble and fail to properly wear %s.\n\r",obj->short_descr);
			return;
		}
		if (is_excluded(ch,obj)){
			send_to_char("You're excluded from using that item.\n\r",ch);
			return;
		}
		if(obj->item_type == ITEM_ARROW){
			obj->value[0] = obj->level / 2;
			obj->value[1] = ch->level / 10;
		}
		if ((obj->item_type != ITEM_LIGHT && obj->wear_flags == ITEM_HOLD) || obj->wear_flags == ITEM_WEAR_SHIELD){
			cHands = calcHands(ch);
			if(cHands < 0 || (obj->item_type == ITEM_WEAPON && IS_WEAPON_STAT(obj,WPN_TWO_HANDS) && cHands > 0) || cHands > 1){
				send_to_char("You have to free up a hand first.\n\r",ch);
				return;
			}
		}
		if (obj->item_type == ITEM_WEAPON){
			if (get_obj_weight(obj) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10)){
				printf_to_char(ch,"You aren't strong enough to wield %s.\n\r",obj->short_descr);
				return;
			}
			dual = get_eq_char(ch,WEAR_SECONDARY);
			if (dual != NULL){
				if (!weapon_table[obj->value[0]].candual){
					send_to_char("You can't wield a secondary weapon with that primary.\n\r",ch);
					return;
				}
				else if (!check_dual_blade(ch,obj,dual)){
					send_to_char("You can't wield that secondary weapon with that primary.\n\r",ch);
					return;
				}
			}
		}
		wear_obj(ch,obj,true,true);
	}
}

void do_remove(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	OBJ_DATA *obj;
	int iWear;

	one_argument(argument,arg);

	if (arg[0] == '\0'){
		send_to_char("Remove what?\n\r",ch);
		return;
	}
	if(ch->isaff(AF_FORTIFY)){
		ch->send("You are too fortified!\n\r");
		return;
	}

	if (!str_cmp(arg,"all")){
		for (iWear = 0;iWear < MAX_WEAR; iWear++){
			if ((obj = get_eq_char(ch,iWear)) == NULL)
				continue;

			if (can_see_obj(ch,obj)){
				if (obj->wear_loc == WEAR_TATTOO)
					send_to_char("Tattoos are permanent, sucker.\n\r",ch);
				else if (obj->wear_loc == WEAR_LODGE_RIB || obj->wear_loc == WEAR_LODGE_LEG || obj->wear_loc == WEAR_LODGE_ARM)
					send_to_char("It's embedded in your body.. try dislodging it.\n\r",ch);
				else
					remove_obj(ch,obj->wear_loc,true);
			}
			else
				send_to_char("You can't see something you wear.\n\r",ch);
		}
	}
	else{
		if ((obj = get_obj_wear(ch,arg,true)) == NULL){
			send_to_char("You do not have that item.\n\r",ch);
			return;
		}

		if (can_see_obj(ch,obj)){
			if (obj->wear_loc == WEAR_TATTOO)
				send_to_char("Tattoos are permanent, sucker.\n\r",ch);
			else if (obj->wear_loc == WEAR_LODGE_RIB || obj->wear_loc == WEAR_LODGE_LEG || obj->wear_loc == WEAR_LODGE_ARM)
				send_to_char("It's embedded in your body.. try dislodging it.\n\r",ch);
			else
				remove_obj(ch,obj->wear_loc,true);
		}
	}
}

void do_sacrifice(CHAR_DATA *ch,char *argument){
	char arg[MIL],buffer[100];
	CHAR_DATA *gch;
	OBJ_DATA *obj;
	int silver,members;

	one_argument(argument,arg);

	if(!arg[0] || !str_cmp(arg,ch->name)){
		act("$n offers $mself to $s god, who graciously declines.",ch,NULL,NULL,TO_ROOM);
		printf_to_char(ch,"%s appreciates your offer and may accept it later.\n\r",god_table[ch->god].name);
		return;
	}

	obj = get_obj_list(ch,arg,ch->in_room->contents);
	if(!obj){
		for(obj = ch->carrying;obj;obj = obj->next_content){
			if (is_name(arg,obj->name) && can_see_obj(ch,obj) && obj->wear_loc == WEAR_NONE && can_drop_obj(ch,obj))
				break;
		}
		if(!obj){
			send_to_char("You can't find it.\n\r",ch);
			return;
		}
	}

	if(obj->item_type == ITEM_CORPSE_PC)
		if (obj->contains){
			printf_to_char(ch,"%s wouldn't like that.\n\r",god_table[ch->god].name);
			return;
		}

	if (!CAN_WEAR(obj,ITEM_TAKE) || CAN_WEAR(obj,ITEM_NO_SAC)){
		act("$p is not an acceptable sacrifice.",ch,obj,0,TO_CHAR);
		return;
	}

	if(obj->in_room)
		for(gch = obj->in_room->people;gch;gch = gch->next_in_room)
			if(gch->on == obj){
				act("$N appears to be using $p.", ch,obj,gch,TO_CHAR);
				return;
			}
		
	silver = UMAX(1,obj->level * 3);

	if(obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
		silver = UMIN(silver,obj->cost);

	if(silver == 1)
		printf_to_char(ch,"%s gives you one silver coin for your sacrifice.\n\r","Boon");//god_table[ch->god].name);
	else
		printf_to_char(ch,"%s gives you %d silver coins for your sacrifice.\n\r","Boon",silver);//god_table[ch->god].name,silver);

	ch->silver += silver;

	if(ch->isplr(PL_AUTOSPLIT)){
		if(silver > 1){
			sprintf(buffer,"%d",silver);
			do_function(ch, &do_split, buffer);	
		}
	}

	act("$n sacrifices $p to $s god.",ch,obj,NULL,TO_ROOM);
	wiznet("$N sends up $p as a burnt offering.",ch,obj,WZ_SACCING,0,0);
	extract_obj(obj);
}

void do_quaff(CHAR_DATA *ch,char *argument){
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument(argument,arg);

    if (arg[0] == '\0'){
		send_to_char("Quaff what?\n\r",ch);
		return;
    }

    if ((obj = get_obj_carry(ch,arg,ch)) == NULL)
    {
		send_to_char("You do not have that potion.\n\r",ch);
		return;
    }

    if (obj->item_type != ITEM_POTION)
    {
		send_to_char("You can quaff only potions.\n\r",ch);
		return;
    }

    if (ch->level < obj->level)
    {
		send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
		return;
    }

    act("$n quaffs $p.",ch,obj,NULL,TO_ROOM);
    act("You quaff $p.",ch,obj,NULL,TO_CHAR);

    obj_cast_spell(obj->value[1],obj->value[0],ch,ch,NULL );
    obj_cast_spell(obj->value[2],obj->value[0],ch,ch,NULL );
    obj_cast_spell(obj->value[3],obj->value[0],ch,ch,NULL );

    extract_obj(obj);
    return;
}

void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if ((scroll = get_obj_carry(ch,arg1,ch)) == NULL)
    {
		send_to_char("You do not have that scroll.\n\r",ch);
		return;
    }

    if (scroll->item_type != ITEM_SCROLL)
    {
		send_to_char("You can recite only scrolls.\n\r",ch);
		return;
    }

    if (ch->level < scroll->level)
    {
		send_to_char("This scroll is too complex for you to comprehend.\n\r",ch);
		return;
    }

    obj = NULL;
    if (arg2[0] == '\0')
		victim = ch;
    else if ((victim = get_char_room(ch,NULL,arg2)) == NULL && (obj = get_obj_here(ch,NULL,arg2)) == NULL)
	{
		send_to_char("You can't find it.\n\r",ch);
		return;
	}

    act("$n recites $p.",ch,scroll,NULL,TO_ROOM);
    act("You recite $p.",ch,scroll,NULL,TO_CHAR);

    if (number_percent() >= 20 + get_skill(ch,gsn_scrolls) * 4/5)
    {
		send_to_char("You mispronounce a syllable.\n\r",ch);
		check_improve(ch,gsn_scrolls,false,2);
    }

    else
    {
    	obj_cast_spell(scroll->value[1],scroll->value[0],ch,victim,obj);
    	obj_cast_spell(scroll->value[2],scroll->value[0],ch,victim,obj);
    	obj_cast_spell(scroll->value[3],scroll->value[0],ch,victim,obj);
		check_improve(ch,gsn_scrolls,true,2);
    }

    extract_obj(scroll);
    return;
}

void do_brandish(CHAR_DATA *ch,char *argument){
	return;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ((staff = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R))) == NULL){
		send_to_char("You hold nothing in your hand.\n\r",ch);
		return;
    }

    if (staff->item_type != ITEM_STAFF)
    {
		send_to_char("You can brandish only with a staff.\n\r",ch);
		return;
    }

    if ((sn = staff->value[3]) < 0 || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
    {
		bug("Do_brandish: bad sn %d.",sn);
		return;
    }

    WAIT_STATE(ch,2 * PULSE_VIOLENCE);

    if (staff->value[2] > 0)
    {
		act("$n brandishes $p.",ch,staff,NULL,TO_ROOM);
		act("You brandish $p.",ch,staff,NULL,TO_CHAR);
		if ( ch->level < staff->level || number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5)
 		{
			act("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
			act("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
			check_improve(ch,gsn_wands,false,2);
		}
		else
			for ( vch = ch->in_room->people; vch; vch = vch_next )
			{
				vch_next = vch->next_in_room;

				switch ( skill_table[sn].target )
				{
					default:
						bug("Do_brandish: bad target for sn %d.",sn);
						return;
					case TAR_IGNORE:
						if ( vch != ch )
							continue;
						break;
					case TAR_CHAR_OFFENSIVE:
						if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
							continue;
						break;
					case TAR_CHAR_DEFENSIVE:
						if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
							continue;
						break;
					case TAR_CHAR_SELF:
						if ( vch != ch )
							continue;
						break;
				}
				obj_cast_spell(staff->value[3],staff->value[0],ch,vch,NULL);
				check_improve(ch,gsn_wands,true,2);
			}
    }
    if (--staff->value[2] <= 0)
    {
		act("$n's $p blazes bright and is gone.",ch,staff,NULL,TO_ROOM);
		act("Your $p blazes bright and is gone.",ch,staff,NULL,TO_CHAR);
		extract_obj(staff);
    }
    return;
}

void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument(argument,arg);
    if (arg[0] == '\0' && ch->fighting == NULL)
    {
		send_to_char("Zap whom or what?\n\r",ch);
		return;
    }

    if ((wand = (get_eq_char(ch,WEAR_HOLD_R) == NULL ? get_eq_char(ch,WEAR_HOLD_L) : get_eq_char(ch,WEAR_HOLD_R))) == NULL)
    {
		send_to_char("You hold nothing in your hand.\n\r",ch);
		return;
    }

    if (wand->item_type != ITEM_WAND)
    {
		send_to_char("You can zap only with a wand.\n\r",ch);
		return;
    }

    obj = NULL;
    if (arg[0] == '\0')
    {
		if (ch->fighting != NULL)
			victim = ch->fighting;
		else
		{
			send_to_char("Zap whom or what?\n\r",ch);
			return;
		}
    }
    else
    {
		if ((victim = get_char_room(ch,NULL,arg)) == NULL && (obj = get_obj_here(ch,NULL,arg)) == NULL)
		{
			send_to_char("You can't find it.\n\r",ch);
			return;
		}
    }

    WAIT_STATE(ch,2 * PULSE_VIOLENCE);

    if (wand->value[2] > 0)
    {
		if (victim != NULL)
		{
			act("$n zaps $N with $p.",ch,wand,victim,TO_NOTVICT);
			act("You zap $N with $p.",ch,wand,victim,TO_CHAR);
			act("$n zaps you with $p.",ch,wand,victim,TO_VICT);
		}
		else
		{
			act("$n zaps $P with $p.",ch,wand,obj,TO_ROOM);
			act("You zap $P with $p.",ch,wand,obj,TO_CHAR);
		}

 		if (ch->level < wand->level || number_percent() >= 20 + get_skill(ch,gsn_wands) * 4/5) 
		{
			act("Your efforts with $p produce only smoke and sparks.",ch,wand,NULL,TO_CHAR);
			act( "$n's efforts with $p produce only smoke and sparks.",ch,wand,NULL,TO_ROOM);
			check_improve(ch,gsn_wands,false,2);
		}
		else
		{
			obj_cast_spell(wand->value[3],wand->value[0],ch,victim,obj);
			check_improve(ch,gsn_wands,true,2);
		}
    }
    if (--wand->value[2] <= 0)
    {
		act("$n's $p explodes into fragments.",ch,wand,NULL,TO_ROOM);
		act("Your $p explodes into fragments.",ch,wand,NULL,TO_CHAR);
		extract_obj(wand);
    }
    return;
}

CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
	char buf[MSL];
	CHAR_DATA *keeper;
	SHOP_DATA *pShop;

	pShop = NULL;
	for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
		if (IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
			break;

	if (pShop == NULL)
	{
		send_to_char("You can't do that here.\n\r",ch);
		return NULL;
	}

	if (!IS_NPC(ch) && ch->isplr(PL_KILLER))
	{
		do_function(keeper,&do_say,"Killers are not welcome!");
		sprintf(buf,"%s the KILLER is over here!\n\r",ch->name);
		do_function(keeper,&do_yell,buf);
		return NULL;
	}

	if (!IS_NPC(ch) && ch->isplr(PL_THIEF))
	{
		do_function(keeper,&do_say,"Thieves are not welcome!");
		sprintf(buf,"%s the THIEF is over here!\n\r",ch->name);
		do_function(keeper,&do_yell,buf);
		return NULL;
	}
	if (time_info.hour < pShop->open_hour && time_info.hour > pShop->close_hour)
	{
		do_function(keeper,&do_say,"Sorry, I am closed. Come back later.");
		return NULL;
	}

	if (!can_see(keeper,ch))
	{
		do_function(keeper,&do_say,"I don't trade with folks I can't see.");
		return NULL;
	}
	return keeper;
}

void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
	OBJ_DATA *t_obj, *t_obj_next;

	for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
	{
		t_obj_next = t_obj->next_content;

		if (obj->pIndexData == t_obj->pIndexData && !str_cmp(obj->short_descr,t_obj->short_descr))
		{
			if (IS_OBJ_STAT(t_obj,ITM_INVENTORY))
			{
				extract_obj(obj);
				return;
			}
			obj->cost = t_obj->cost;
			break;
		}
	}

	if (t_obj == NULL)
	{
		obj->next_content = ch->carrying;
		ch->carrying = obj;
	}
	else
	{
		obj->next_content = t_obj->next_content;
		t_obj->next_content = obj;
	}

	obj->carried_by  = ch;
	obj->in_room     = NULL;
	obj->in_obj      = NULL;
	ch->carry_number += get_obj_number(obj);
	ch->carry_weight += get_obj_weight(obj);
}

OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, char *argument ){
	char arg[MIL];
	OBJ_DATA *obj;
	int number,count;

	number = number_argument(argument,arg);
	count  = 0;
	for(obj = keeper->carrying;obj;obj = obj->next_content){
		if(obj->wear_loc == WEAR_NONE && can_see_obj(keeper,obj) && can_see_obj(ch,obj) && is_name(arg,obj->name)){
			if(++count == number)
				return obj;
			while (obj->next_content != NULL && obj->pIndexData == obj->next_content->pIndexData && !str_cmp(obj->short_descr,obj->next_content->short_descr))
				obj = obj->next_content;
		}
	}

	return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ){
    SHOP_DATA *pShop;
    int cost;

    if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
		return 0;

    if (fBuy)
		cost = obj->cost * pShop->profit_buy  / 100;
    else
    {
		OBJ_DATA *obj2;
		int itype;

		cost = 0;
		for ( itype = 0; itype < MAX_TRADE; itype++ )
			if (obj->item_type == pShop->buy_type[itype])
			{
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}

		if (!IS_OBJ_STAT(obj,ITM_SELL_EXTRACT))
			for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
	    		if (obj->pIndexData == obj2->pIndexData &&   !str_cmp(obj->short_descr,obj2->short_descr))
				{
	 				if (IS_OBJ_STAT(obj2,ITM_INVENTORY))
						cost /= 2;
					else
						cost = cost * 3 / 4;
				}
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
		if (obj->value[1] == 0)
			cost /= 4;
		else
			cost = cost * obj->value[2] / obj->value[1];
    }
    return cost;
}

char *display_cost(int cost,bool named){
	static char buf[100];
	int silver = 0, gold = 0;

	if (cost > 100){
		gold = cost / 100;
		silver = cost - (gold * 100);
	}
	else{
		gold = 0;
		silver = cost;
	}

	if(named)
		sprintf(buf,"{Y%d{x gold and {w%2d{x silver",gold,silver);
	else
		sprintf(buf,"{Y%3d {w%2d",gold,silver);
	return buf;
}

void do_buy(CHAR_DATA *ch,char *argument){
	char buf[MSL];
	int cost,roll;

	if(!argument[0]){
		ch->send("Buy what?\n\r");
		return;
	}
	if(IS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP)){
		do_buy_mount(ch,argument);
		return;
	}

	if(IS_SET(ch->in_room->room_flags,ROOM_PET_SHOP)){
		do_buy_pet(ch,argument);
		return;
	}
	else{
		CHAR_DATA *keeper;
		OBJ_DATA *obj,*t_obj;
		char arg[MIL];
		int number, count = 1;
		OBJ_DATA *tattoo = get_eq_char(ch,WEAR_TATTOO);

		if (!(keeper = find_keeper(ch)))
			return;

		number	= mult_argument(argument,arg);
		obj		= get_obj_keeper(ch,keeper,arg);
		cost	= get_cost(keeper,obj,true);

		if (number < 1 || number > 99){
			act("$n tells you 'Get real!",keeper,NULL,ch,TO_VICT);
			return;
		}

		if (cost <= 0 || !can_see_obj(ch,obj)){
			act("$n tells you 'I don't sell that -- try 'list''.",keeper,NULL,ch,TO_VICT);
			ch->reply = keeper;
			return;
		}

		if (!IS_OBJ_STAT(obj,ITM_INVENTORY)){
			for (t_obj = obj->next_content;	count < number && t_obj != NULL; t_obj = t_obj->next_content)
	    		if (t_obj->pIndexData == obj->pIndexData &&  !str_cmp(t_obj->short_descr,obj->short_descr))
					count++;
	    		else
					break;

			if (count < number){
	    		act("$n tells you 'I don't have that many in stock.",keeper,NULL,ch,TO_VICT);
	    		ch->reply = keeper;
	    		return;
			}
		}

		if ((ch->silver + ch->gold * 100) < cost * number){
			if (number > 1)
				act("$n tells you 'You can't afford to buy that many.",keeper,obj,ch,TO_VICT);
			else
	    		act("$n tells you 'You can't afford to buy $p'.",keeper,obj,ch,TO_VICT);
			ch->reply = keeper;
			return;
		}

		if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch)){
			send_to_char("You can't carry that many items.\n\r",ch);
			return;
		}

		if (ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch)){
			send_to_char("You can't carry that much weight.\n\r",ch);
			return;
		}

		roll = number_percent();
		if (!IS_OBJ_STAT(obj,ITM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle)){
			cost -= obj->cost / (7 - ch->getslvl(gsn_haggle)) * roll / 100;
			act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
			check_improve(ch,gsn_haggle,true,4);
		}

		if (number > 1){
			sprintf(buf,"$n buys $p[%d].",number);
			act(buf,ch,obj,NULL,TO_ROOM);
			sprintf(buf,"You buy $p[%d] for %s.",number,display_cost(cost*number,true));
			act(buf,ch,obj,NULL,TO_CHAR);
		}
		else{
			act("$n buys $p.",ch,obj,NULL,TO_ROOM);
			sprintf(buf,"You buy $p for %s.",display_cost(cost,true));
			act(buf,ch,obj,NULL,TO_CHAR);
		}
		deduct_cost(ch,cost * number);
		keeper->gold += cost * number/100;
		keeper->silver += cost * number - (cost * number/100) * 100;

		for (count = 0; count < number; count++){
			if (IS_SET(obj->extra_flags,ITM_INVENTORY))
	    		t_obj = create_object(obj->pIndexData,obj->level);
			else{
				t_obj = obj;
				obj = obj->next_content;
	    		obj_from_char(t_obj);
			}

			if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITM_HAD_TIMER))
	    		t_obj->timer = 0;
			REMOVE_BIT(t_obj->extra_flags,ITM_HAD_TIMER);
			obj_to_char(t_obj,ch);
			if (CAN_WEAR(t_obj,ITEM_WEAR_TATTOO)){
				if (tattoo != NULL) {
					act("$n gets $p tattooed to $s skin, over an older one.",ch,obj,NULL,TO_ROOM);
					act("You have $p tattooed over your old one.",ch,obj,NULL,TO_CHAR);
					obj_from_char(tattoo);
					extract_obj(tattoo);
					equip_char(ch, t_obj, WEAR_TATTOO);
				}
				else{
					act("$n gets $p tattooed to $s skin.",ch,obj,NULL,TO_ROOM);
					act("You have $p tattooed to your flesh.",ch,obj,NULL,TO_CHAR);
					equip_char(ch,t_obj,WEAR_TATTOO);
				}
			}
			if (cost < t_obj->cost)
	    		t_obj->cost = cost;
		}
	}
}

void do_list(CHAR_DATA *ch,char *argument){
	if(IS_SET(ch->in_room->room_flags,ROOM_PET_SHOP) || IS_SET(ch->in_room->room_flags,ROOM_MOUNT_SHOP)){
		ROOM_INDEX_DATA *pRoomIndexNext = NULL;
		CHAR_DATA *pet;
		bool found;

		if(!(pRoomIndexNext = get_room_index(ch->in_room->vnum + 1))){
			bug("Do_list: bad pet shop at vnum %d.",ch->in_room->vnum);
			send_to_char("You can't do that here.\n\r",ch);
			return;
		}

		found = false;
		for(pet = pRoomIndexNext->people;pet;pet = pet->next_in_room){
			if(pet->isact(AT_PET) || pet->isact(AT_MOUNT)){
				if(!found){
					found = true;
					if(pet->isact(AT_PET))
						send_to_char("Pets for sale:\n\r",ch);
					else if(pet->isact(AT_MOUNT))
						send_to_char("Mounts for sale:\n\r",ch);
				}
				printf_to_char(ch,"[%2d] %8d - %s\n\r",pet->level,pet->level * pet->level * 10,pet->short_descr);
			}
		}
		if(!found){
			if(IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP))
				send_to_char("Sorry, we're out of pets right now.\n\r",ch);
			else
				send_to_char("Sorry, we're out of mounts right now.\n\r",ch);
			return;
		}
		return;
	}
	else{
		CHAR_DATA *keeper;
		OBJ_DATA *obj;
		int cost,count;
		bool found;
		char arg[MAX_INPUT_LENGTH];

		if(!(keeper = find_keeper(ch)))
			return;
		one_argument(argument,arg);

		found = false;
	/*	printf_to_char(ch,"Items %s will buy and sell:\n\r",keeper->short_descr);
		for(int i=0;i < MAX_TRADE;i++)
			if (keeper->pIndexData->pShop->buy_type[i] != 0)
					printf_to_char(ch,"  -%s\n\r",flag_string(type_flags,keeper->pIndexData->pShop->buy_type[i]));*/
		ch->send("\n\rItems for sale:\n\r");
		for(obj = keeper->carrying;obj;obj = obj->next_content){
			if(obj->wear_loc == WEAR_NONE && can_see_obj(ch,obj) && (cost = get_cost(keeper,obj,true)) > 0 && ( arg[0] == '\0' || is_name(arg,obj->name))){
				if(!found){
					found = true;
					send_to_char("[Lvl  Price   Qty] Item\n\r",ch);
				}

				char buf[40];
				if(obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_SHIELD)
					sprintf(buf," (%s)",armortype_bit_name(obj->armortype_flags));
				else if(obj->item_type == ITEM_WEAPON)
					sprintf(buf," (%s)",weapon_name(obj->value[0]));
				else
					sprintf(buf,"");

				if(IS_OBJ_STAT(obj,ITM_INVENTORY))
					printf_to_char(ch,"[{C%3d {w%-12s {r--{x ] %30.30s%s\n\r",obj->level,display_cost(cost,false),obj->short_descr,buf);
				else{
					count = 1;

					while (obj->next_content != NULL && obj->pIndexData == obj->next_content->pIndexData && !str_cmp(obj->short_descr,obj->next_content->short_descr)){
						obj = obj->next_content;
						count++;
					}
					printf_to_char(ch,"[{C%3d {w%s {R%2d{x ] %s\n\r",obj->level,display_cost(cost,false),count,obj->short_descr);
				}
			}
		}
		if(!found)
			send_to_char("You can't buy anything here.\n\r",ch);
		return;
	}
}

void do_sell(CHAR_DATA *ch,char *argument){
    char buf[MSL];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost,roll;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
		send_to_char("Sell what?\n\r",ch);
		return;
    }

    if ((keeper = find_keeper(ch)) == NULL)
		return;

    if ((obj = get_obj_carry(ch,arg,ch)) == NULL)
    {
		send_to_char("You don't have that item.\n\r",ch);
		return;
    }

    if (!can_drop_obj(ch,obj))
    {
		send_to_char("You can't let go of it.\n\r",ch);
		return;
    }

    if (!can_see_obj(keeper,obj))
    {
		act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
		return;
    }

    if ((cost = get_cost(keeper,obj,false)) <= 0)
    {
		if(ch->iscomm(CM_DEBUG))printf_to_char(ch,"%d ",get_cost(keeper,obj,false));
		act("$n says to you, '{G$p isn't worth the dirt under my shoe.{x'",keeper,obj,ch,TO_VICT);
		return;
    }
    if (cost > (keeper->silver + 100 * keeper->gold))
    {
		act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",keeper,obj,ch,TO_VICT);
		return;
    }

    act("$n sells $p.",ch,obj,NULL,TO_ROOM);

    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITM_SELL_EXTRACT) && roll < get_skill(ch,gsn_haggle))
    {
        send_to_char("You haggle with the shopkeeper.\n\r",ch);
        cost += obj->cost / (7 - ch->getslvl(gsn_haggle)) * roll / 100;
        cost = UMIN(cost,95 * get_cost(keeper,obj,true) / 100);
		cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
        check_improve(ch,gsn_haggle,true,4);
    }
    sprintf(buf,"You sell $p for %s.",display_cost(cost,true));
    act(buf,ch,obj,NULL,TO_CHAR);
    ch->gold   += cost/100;
    ch->silver += cost - (cost/100) * 100;
    deduct_cost(keeper,cost);
    if (keeper->gold < 0)
		keeper->gold = 0;
    if (keeper->silver < 0)
		keeper->silver = 0;

    if (obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITM_SELL_EXTRACT))
		extract_obj(obj);
    else
    {
		obj_from_char(obj);
		if (obj->timer)
			SET_BIT(obj->extra_flags,ITM_HAD_TIMER);
		else
			obj->timer = number_range(50,100);
		obj_to_keeper(obj,keeper);
    }
    return;
}

void do_value( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
		send_to_char("Value what?\n\r",ch);
		return;
    }

    if ((keeper = find_keeper(ch)) == NULL)
		return;

    if ((obj = get_obj_carry(ch,arg,ch)) == NULL)
    {
		act("$n tells you 'You don't have that item'.",keeper,NULL,ch,TO_VICT);
		ch->reply = keeper;
		return;
    }

    if (!can_see_obj(keeper,obj))
    {
        act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
        return;
    }

    if (!can_drop_obj(ch,obj))
    {
		send_to_char("You can't let go of it.\n\r",ch);
		return;
    }

    if ((cost = get_cost(keeper,obj,false)) <= 0)
    {
		printf_to_char(ch,"%d ",get_cost(keeper,obj,false));
		act("$n says to you, '{G$p isn't worth the dirt under my shoe.{x'",keeper,obj,ch,TO_VICT);
		return;
    }

    sprintf(buf,"$n tells you 'I'll give you %d silver and %d gold coins for $p'.",cost - (cost/100) * 100,cost/100);
    act(buf,keeper,obj,ch,TO_VICT);
    ch->reply = keeper;
    return;
}

void do_dual_wield(CHAR_DATA *ch, char *argument){
	OBJ_DATA *wield,*obj;
	int canWield=0;
	bool shield=false;

	if (!argument[0]){
		send_to_char("Wear which weapon in your off-hand?\n\r",ch);
		return;
	}

	obj = get_obj_carry(ch,argument,ch);

	if (!obj){
		send_to_char("You have no such thing.\n\r",ch);
		return;
	}

	if(get_skill(ch,gsn_dual_wield) < 1){
		ch->send("You aren't ambidextrous enough to do that.\n\r");
		return;
	}

	if (obj->item_type != ITEM_WEAPON){
		if (obj->item_type != ITEM_SHIELD){
			send_to_char("You can only dual wield weapons or shields.\n\r",ch);
			return;
		}
		else
			shield = true;
	}

	if (calcHands(ch) > 1 && !get_eq_char(ch,WEAR_SECONDARY)){
		send_to_char("You must free up a hand first.\n\r",ch);
		return;
	}

	if (ch->level < obj->level){
		printf_to_char(ch,"You must be level %d to use this object.\n\r",obj->level);
		act("$n tries to use $p, but is too inexperienced.",ch,obj,NULL,TO_ROOM);
		return;
	}

	if (!shield){
		if ((wield = get_eq_char(ch,WEAR_WIELD)) == NULL){
			send_to_char("No wield.\n\r",ch);
			if(!check_dual_bare(ch,obj))
				canWield = 1;
			else{
				if (!weapon_table[obj->value[0]].candual)
					canWield = 2;
				else
					canWield = 3;
			}
		}
		else{
			if (!weapon_table[wield->value[0]].candual)
				canWield = 2;
			else if (check_dual_blade(ch,wield,obj))
				canWield = 3;
		}
		if (canWield == 1 || canWield == 2 || canWield == 0){
			send_to_char("You can't do that.\n\r",ch);
			return;
		}
	}
	/* at last - the char uses the weapon */
	if (!remove_obj(ch,WEAR_SECONDARY,true)){
		ch->send("You can't remove your secondary.\n\r");
		return;
	}

	act("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM);
	act("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
	equip_char(ch,obj,WEAR_SECONDARY);
	ch->dcounter = get_attackspeed(ch,true);
	return;
}

void do_grasp(CHAR_DATA *ch, char *argument)
{
	int chance;
	char arg[MSL];
	AFFECT_DATA af;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	one_argument(argument,arg);

	if((chance = get_skill(ch,gsn_grasp)) * .9 < 1)
		return;

	if(arg[0] != '\0')
	{
		wield = get_obj_wear(ch,arg,true);
		if(wield->item_type != ITEM_WEAPON)
		{
			send_to_char("You can only grasp weapons.\n\r",ch);
			return;
		}
	}

	if(wield == NULL)
	{
		send_to_char("You don't have that.\n\r",ch);
		return;
	}

	if(calcHands(ch) > 1)
	{
		send_to_char("Your hands are tied up right now..\n\r",ch);
		return;
	}

	if(IS_SET(wield->extra_flags,ITM_GRASPED))
	{
		send_to_char("You've already got this weapon tightly grasped in both hands!\n\r",ch);
		return;
	}

	if(number_percent() <= chance)
	{
		act("You grasp $p with both of your hands.",ch,wield,NULL,TO_CHAR);
		act("$n grasps $p with $s two hands.",ch,wield,NULL,TO_ROOM);
		af.where     = TO_OBJECT;
		af.type      = gsn_grasp;
		af.level     = ch->level;
		af.duration  = 7;
		af.location  = 0;
		af.modifier  = 0;
		af.bitvector = ITM_GRASPED;
		affect_to_obj(wield,&af);
	}
	else
		send_to_char("Your second hand flies past your weapon's hilt!\n\r",ch);
	return;
}

void do_grip(CHAR_DATA *ch, char *argument){
	int chance;
	char arg[MSL];
	AFFECT_DATA af;
	OBJ_DATA *wield = get_eq_char(ch,WEAR_WIELD);
	one_argument(argument,arg);

	if((chance = get_skill(ch,gsn_grip)) * .8 < 1){
		send_to_char("Your hand just doesn't have the power!\n\r",ch);
		return;
	}

	if(arg[0] != '\0'){
		if(!(wield = get_obj_wear(ch,arg,true))){
			ch->send("You don't have that.\n\r");
			return;
		}
		if(wield->item_type != ITEM_WEAPON){
			send_to_char("You can only grip weapons.\n\r",ch);
			return;
		}
	}

	if(!wield){
		send_to_char("You don't have that.\n\r",ch);
		return;
	}

	if(IS_SET(wield->extra_flags,ITM_GRIPPED)){
		send_to_char("It's already gripped.\n\r",ch);
		return;
	}

	if(number_percent() <= chance){
		act("You grip $p tightly.",ch,wield,NULL,TO_CHAR);
		act("$n grips $p tightly.",ch,wield,NULL,TO_ROOM);
		af.where     = TO_OBJECT;
		af.type      = gsn_grip;
		af.level     = ch->level;
		af.duration  = 5;
		af.location  = 0;
		af.modifier  = 0;
		af.bitvector = ITM_GRIPPED;
		affect_to_obj(wield,&af);
	}
	else
		send_to_char("Your hand just doesn't have the power!\n\r",ch);
	return;
}

void do_scribe(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
    EXTRA_DESCR_DATA *ed;
	char arg[MSL];
	argument = one_argument(argument,arg);

	if(arg[0] == '\0')
	{
		send_to_char("Syntax: scribe <parchment>\n\r",ch);
		return;
	}

	if((obj = get_obj_carry(ch,arg,ch)) == NULL)
	{
		send_to_char("You don't have that.\n\r",ch);
		return;
	}
	
	if(obj->item_type != ITEM_PARCHMENT)
	{
		send_to_char("You may only write on parchment.\n\r",ch);
		return;
	}

	argument = one_argument(argument,arg);
	if (argument == NULL)
	{
	    send_to_char("Syntax: oset <object> ed <keyword> <string>\n\r",ch);
	    return;
	}

	for ( ed = obj->extra_descr; ed; ed = ed->next )
		if (is_name("message",ed->keyword))
			break;

	if (!ed)
	{
		ed					= new_extra_descr();
		ed->keyword			= str_dup("message");
		ed->next			= obj->extra_descr;
		obj->extra_descr	= ed;
	}

	act("$n begins writing on $p.",ch,obj,NULL,TO_ROOM);
	act("You begin writing on $p.",ch,obj,NULL,TO_CHAR);
	string_append(ch,&ed->description);
	return;
}

void do_hurl(CHAR_DATA *ch,char *argument)
{
	extern char * const dir_name[];
	char arg[MIL],arg2[MIL];
	OBJ_DATA *obj = NULL;
	CHAR_DATA *victim = NULL;
	ROOM_INDEX_DATA *scan_room = NULL;
	EXIT_DATA *pExit;
	int door;
	bool found = false;

	argument = one_argument(argument,arg);
	one_argument(argument,arg2);

	if ((obj = get_obj_carry(ch,arg,ch)) == NULL)
	{
		send_to_char("Hurl what?\n\r",ch);
		return;
	}

	if ((victim = get_char_room(ch,NULL,arg2)) == NULL)
	{
		for (int door = 0; door < 6 && !found; door++)
		{
			scan_room = ch->in_room;
			if (!found && ((pExit = scan_room->exit[door]) != NULL))
			{
				scan_room = pExit->u1.to_room;
				CHAR_DATA *rch;

				if ((victim = get_char_room(NULL,scan_room,arg2)) != NULL && victim != ch && !IS_SET(pExit->exit_info,EX_CLOSED) && can_see(ch,victim))
					found = true;
			}
		}
	}
	else
		found = true;

	if (!found)
	{
		send_to_char("Who?\n\r",ch);
		return;
	}

	if (victim->in_room == ch->in_room)
	{
		act("You hurl $p at $N and it bounces off $S head. BOINK!",ch,obj,victim,TO_CHAR);
		act("$n hurls $p at $N and it bounces off $S head. BOINK!",ch,obj,victim,TO_NOTVICT);
		act("$n hurls $p at you and it bounces off your head. BOINK!",ch,obj,victim,TO_VICT);
		obj_from_char(obj);
		obj_to_room(obj,ch->in_room);
	}
	else
	{
		act("You hurl $p at $N.",ch,obj,victim,TO_CHAR);
		act("$n hurls $p $T.",ch,obj,dir_name[door],TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj,victim->in_room);
		act("$p flies in and bounces off your head. BOINK!",NULL,obj,victim,TO_VICT);
		act("$p flies in and bounces off $N's head. BOINK!",NULL,obj,victim,TO_ROOM);
	}
	return;
}

void do_donate(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj,*d_box;
	ROOM_INDEX_DATA *pRoom,*from_room;

	if(!argument[0]){
		ch->send("Donate what?\n\r");
		return;
	}

	if(!(obj = get_obj_carry(ch,argument,ch))){
		ch->send("You are not carrying any such item.\n\r");
		return;
	}
	if (!can_drop_obj(ch,obj)){
		ch->send("You can't let go of it.\n\r");
		return;
	}

	if(!(pRoom = get_room_index(hometowns[ch->hometown].donor))){
		ch->send("Your hometown is broken...\n\r");
		log_f("%s has a crappy hometown %d",ch->name,ch->hometown);
		return;
	}
	for(d_box = pRoom->contents;d_box;d_box = d_box->next_content){
		if(d_box->item_type == ITEM_LOCKER)
			break;
	}
	if(!d_box || d_box->item_type != ITEM_LOCKER){
		ch->send("There is no donor box for your hometown?\n\r");
		log_f("%s has no donor box in hometown %d",ch->name,ch->hometown);
		return;
	}
	if(d_box->cont_count >= d_box->value[3]){
		ch->send("The donor box is full.\n\r");
		return;
	}
	obj_from_char(obj);
	obj_to_obj(obj,d_box);
	act("$n donates $p.",ch,obj,NULL,TO_ROOM);
	act("You donate $p to your hometown.",ch,obj,NULL,TO_CHAR);
	from_room = ch->in_room;
	char_from_room(ch);
	char_to_room(ch,pRoom);
	save_char_locker(ch);
	act("$p shimmers briefly.",ch,d_box,NULL,TO_ROOM);
	char_from_room(ch);
	char_to_room(ch,from_room);
}
