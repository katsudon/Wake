#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "db.h"

char *printName(CHAR_DATA *wch, bool iswhois);

void save_who(){
	FILE *fp;
	CHAR_DATA *wch;
	char buf[MSL],arg[MSL];
	    char strsave[MIL];
    DESCRIPTOR_DATA *d;
    int wlevel, nMatch, fBCheck,guildlist[MAX_GUILD];
	bool group = false;

	fclose(fpReserve);
	sprintf(strsave,"%sWHO",PLAYER_DIR);
	if(!(fp = fopen(TEMP_FILE,"w"))){
		bug("Save_who: fopen",0);
		perror(strsave);
	}
	else{
		fprintf(fp,"\n\r           {rWake{x\n\r");

		nMatch = 0;
		buf[0] = '\0';

		for(int i = 0;i<MAX_GUILD;i++)
			guildlist[i] = 0;
		for (d = descriptor_list;d;d = d->next){
			if (d->connected != CON_PLAYING)
				continue;
			wch = d->original ? d->original : d->character;
			if(guilds[wch->guild].hidden){
				guildlist[guilds[wch->guild].hidden] = 1;// && (ch->guild == guilds[wch->guild].mask && ch->rank != 0))
				continue;
			}
			guildlist[wch->guild] = 1;
		}

		for(int i = 0;i<MAX_GUILD;i++){
			if(guildlist[i] == 0)
				continue;
			fprintf(fp,"\n\r%s\n\r",guilds[i].who_name);

			for(int j = MAX_LEVEL+1;j > 0;j--){
				for (d = descriptor_list; d != NULL; d = d->next){
					if(d->connected != CON_PLAYING)
						continue;
					wch = (d->original != NULL) ? d->original : d->character;
					if(wch->level != j)
						continue;
					if(wch->guild != i){
						if(!guilds[wch->guild].hidden || guilds[wch->guild].hidden != i)
							continue;
					}
					fprintf(fp,"%s",printName(wch,false));
					nMatch++;
				}
			}
			
		}
		fprintf(fp,"\n\rPlayers found: {R%d{x, Most today: {G%d{x, Most ever: {G%d{x\n\r",nMatch,mud.d_con,mud.max_con);
		if (double_exp)
			fprintf(fp,"Doubles are currently {Gactive{x.\n\r");
		}

	fclose(fp);
	rename(TEMP_FILE,strsave);
	fpReserve = fopen( NULL_FILE, "r" );
}
