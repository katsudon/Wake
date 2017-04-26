#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */

void quest_check_partcomplete(CHAR_DATA *ch,CHAR_DATA *mob){
}
void quest_check_allcomplete(CHAR_DATA *ch,CHAR_DATA *mob){
}

void quest_data :: show(CHAR_DATA *ch){
	printf_to_char(ch,"Vnum: %d\n\r",vnum);
}

