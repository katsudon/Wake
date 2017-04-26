#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "lookup.h"

/*
 * Local functions.
 */

extern char *target_name;

void spell_magic_missile(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int i,dam,tdam;

	if(ch->getslvl(sn) > 2){
		act("$n fires a volley of magic missiles at $N!",ch,NULL,victim,TO_NOTVICT);
		act("$n fires a volley of magic missiles at you!",ch,NULL,victim,TO_VICT);
		act("You fire a volley of magic missiles at $N!",ch,NULL,victim,TO_CHAR);
	}
	else{
		act("$n fires a magic missile at $N!",ch,NULL,victim,TO_NOTVICT);
		act("$n fires a magic missile at you!",ch,NULL,victim,TO_VICT);
		act("You fire a magic missile at $N!",ch,NULL,victim,TO_CHAR);
	}
	dam = pow(25,(1.0+((level - 1) *0.002)));//(level * 20 * ch->getslvl(sn))/ 100;

	for(i=1; i <= ch->getslvl(sn); i++){
		tdam = dam / i;
		if (saves_spell(level,victim,DAM_ENERGY))
			tdam /= 2;
		spell_damage(ch,victim,tdam,sn,DAM_ENERGY,true);
	}
}

void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target,int slvl){
    char buf1[MSL];
    char buf2[MSL];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument(target_name,speaker);

    sprintf(buf1,"%s says '%s'.\n\r",speaker,target_name);
    sprintf(buf2,"Someone makes %s say '%s'.\n\r",speaker,target_name);
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
		if (!is_exact_name(speaker,vch->name) && IS_AWAKE(vch))
			send_to_char(saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1,vch);
    return;
}
