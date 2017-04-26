#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	( int  );
extern	int	malloc_verify	( void );
#endif

/*
 * Socket and TCP/IP stuff.
 */
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };

int	close			args( ( int fd ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	select			args( ( int width, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout ) );
int	socket			args( ( int domain, int type, int protocol ) );
void check_multiplay ( DESCRIPTOR_DATA* );
void d_insert				( DESCRIPTOR_DATA* );

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;				/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;							/* Next descriptor in loop	*/
FILE *				fpReserve;						/* Reserved file handle		*/
bool				god;							/* All new chars are gods!	*/
bool				merc_down;						/* Shutdown			*/
char				str_boot_time[MAX_INPUT_LENGTH];
time_t				current_time;					/* time of this pulse */	
bool				MOBtrigger = true;				/* act() switch                 */


void	game_loop_unix			( int );
void	nanny					( DESCRIPTOR_DATA*,char* );
int		init_socket				( int );
int		main					( int,char** );
void	init_descriptor			( int );
bool	read_from_descriptor	( DESCRIPTOR_DATA* );
bool	write_to_descriptor		( int,char*,int );
bool    check_illegal_name		( char* );
bool	process_output			( DESCRIPTOR_DATA*,bool );
void	read_from_buffer		( DESCRIPTOR_DATA* );
void	stop_idling				( CHAR_DATA* );
void    bust_a_prompt			( CHAR_DATA* );
void	write_max_con			( );
void	check_max_con			( );
void	save_who				( );

/* Needs to be global because of do_copyover */
int port, control;


int main(int argc,char **argv){
	struct timeval now_time;
	bool fCopyOver = false;
#ifdef IMC
   int imcsocket = -1;
#endif

	/*
	 * Memory debugging if needed.
	 */
	#if defined(MALLOC_DEBUG)
		malloc_debug(2);
	#endif

	// Init time.
	gettimeofday(&now_time,NULL);
	current_time = (time_t) now_time.tv_sec;
	strcpy( str_boot_time, ctime(&current_time) );

	// Reserve one channel for our use.
	if ( ( fpReserve = fopen(NULL_FILE,"r") ) == NULL ){
		perror(NULL_FILE);
		exit(1);
	}

	// Get the port number.
	port = 4000;
	if (argc > 1){
		if (!is_number(argv[1])){
			fprintf(stderr,"Usage: %s [port #]\n",argv[0]);
			exit(1);
		}
		else if ((port = atoi(argv[1])) <= 1024){
			fprintf(stderr,"Port number must be above 1024.\n");
			exit(1);
		}
		
		/* Are we recovering from a copyover? */
		if (argv[2] && argv[2][0]){
			fCopyOver = true;
			control = atoi(argv[3]);
#ifdef IMC
	   imcsocket = atoi( argv[4] );
#endif
 		}
 		else
 			fCopyOver = false;
	}

	// Run the game.
	if (!fCopyOver)
		control = init_socket(port);

	boot_db();

	if(port == 1337){
		mud.coder = true;
		sprintf(log_buf,"nashMUD locked and loaded on CODER port %d.",port);
	}
	else{
		mud.coder = false;
		sprintf(log_buf,"nashMUD locked and loaded on port %d.",port);
	}
	log_string(log_buf);
#ifdef IMC
   /* Initialize and connect to IMC2 */
   imc_startup( FALSE, imcsocket, fCopyOver );
#endif
	if (fCopyOver)
		copyover_recover();

	game_loop_unix(control);

	close (control);
#ifdef IMC
   imc_shutdown( FALSE );
#endif

	// That's all, folks.
	log_string("Normal termination of game.");
	exit(0);
	return 0;
}

int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket(AF_INET,SOCK_STREAM,0) ) < 0 )
    {
		perror("Init_socket: socket");
		exit(1);
    }

    if ( setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&x, sizeof(x)) < 0 )
    {
		perror("Init_socket: SO_REUSEADDR");
		close(fd);
		exit(1);
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
		struct	linger	ld;

		ld.l_onoff  = 1;
		ld.l_linger = 1000;

		if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
		(char *) &ld, sizeof(ld) ) < 0 )
		{
			perror( "Init_socket: SO_DONTLINGER" );
			close(fd);
			exit( 1 );
		}
    }
#endif

    sa = sa_zero;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if ( bind(fd,(struct sockaddr *) &sa,sizeof(sa)) < 0 )
    {
		perror("Init socket: bind");
		close(fd);
		exit(1);
    }


    if ( listen(fd,3) < 0 )
    {
		perror("Init socket: listen");
		close(fd);
		exit(1);
    }

    return fd;
}

void game_loop_unix( int control ){
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		DESCRIPTOR_DATA *d;
		int maxdesc;

	#if defined(MALLOC_DEBUG)
		if ( malloc_verify( ) != 1 )
			abort( );
	#endif

		/*
		 * Poll all active descriptors.
		 */
		FD_ZERO(&in_set);
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);
		FD_SET(control,&in_set);
		maxdesc	= control;
		for ( d = descriptor_list; d; d = d->next )
		{
			maxdesc = UMAX(maxdesc,d->descriptor);
			FD_SET(d->descriptor,&in_set);
			FD_SET(d->descriptor,&out_set);
			FD_SET(d->descriptor,&exc_set);
		}

		if ( select(maxdesc + 1,&in_set,&out_set,&exc_set,&null_time) < 0 )
		{
			perror("Game_loop: select: poll");
			exit(1);
		}

		/*
		 * New connection?
		 */
		if ( FD_ISSET(control,&in_set) )
			init_descriptor(control);

		/*
		 * Kick out the freaky folks.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next = d->next;   
			if ( FD_ISSET(d->descriptor,&exc_set) )
			{
				FD_CLR(d->descriptor,&in_set);
				FD_CLR(d->descriptor,&out_set);
				if ( d->character && d->connected == CON_PLAYING)
					cql_save_char(d->character);//nash how often does this happen?
				d->outtop = 0;
				close_socket(d);
			}
		}

		/*
		 * Process input.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next	= d->next;
			d->fcommand	= false;

			if(FD_ISSET(d->descriptor,&in_set)){
				if(d->character)
					d->character->timer = 0;
				if(!read_from_descriptor(d)){
					FD_CLR(d->descriptor,&out_set);
					if(d->character && d->connected == CON_PLAYING)
						cql_save_char(d->character);
					d->outtop = 0;
					close_socket(d);
					continue;
				}
			}
			CHAR_DATA *ch = d->character;

			if(ch && ch->daze > 0)
				--ch->daze;

			if(ch && ch->wait > 0){
				--ch->wait;
				if(number_percent() < get_curr_stat(ch,STAT_AGI))
					--ch->wait;
				if(ch->wait == 0 && ch->iscomm(CM_DEBUG))
					ch->send("WAIT OVER\n\r");
				continue;
			}

			if (ch && ch->bashwait > 0){
				ch->bashwait--;
				if(ch->bashwait == 0){
					act("$n stands up.",ch,NULL,NULL,TO_ROOM);
					act("You stand up.",ch,NULL,NULL,TO_CHAR);
					ch->position = POS_STANDING;
				}
			}

			read_from_buffer( d );
			if ( d->incomm[0] != '\0' )
			{
				d->fcommand	= true;
				stop_idling(d->character);

				if ( d->showstr_point )
					show_string(d,d->incomm);
				else
					if ( d->pString )
						string_add(d->character,d->incomm);
					else
						switch ( d->connected )
						{
							case CON_PLAYING:
								if ( !run_olc_editor(d) )
    								substitute_alias(d,d->incomm);
								break;
							default:
								nanny(d,d->incomm);
								break;
						}
				d->incomm[0]	= '\0';
			}
		}


#ifdef IMC
	imc_loop();
#endif

		/* Autonomous game motion. */
		update_handler( );

		/*
		 * Output.
		 */
		for ( d = descriptor_list; d != NULL; d = d_next )
		{
			d_next = d->next;

			if ( ( d->fcommand || d->outtop > 0 ) && FD_ISSET(d->descriptor,&out_set) )
			{
				if ( !process_output(d,true) )
				{
					if ( d->character != NULL && d->connected == CON_PLAYING)
						cql_save_char(d->character);
					d->outtop = 0;
					close_socket(d);
				}
			}
		}

		/*
		 * Synchronize to a clock.
		 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		 * Careful here of signed versus unsigned arithmetic.
		 */
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;

			gettimeofday( &now_time, NULL );
			usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
				+ 1000000 / PULSE_PER_SECOND;
			secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
			while ( usecDelta < 0 )
			{
				usecDelta += 1000000;
				secDelta  -= 1;
			}

			while ( usecDelta >= 1000000 )
			{
				usecDelta -= 1000000;
				secDelta  += 1;
			}

			if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
			{
				struct timeval stall_time;

				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec  = secDelta;
				if ( select(0,NULL,NULL,NULL,&stall_time) < 0 )
				{
					perror("Game_loop: select: stall");
					exit(1);
				}
			}
		}

		gettimeofday(&last_time,NULL);
		current_time = (time_t) last_time.tv_sec;
    }

    return;
}

void init_descriptor(int control){
	char buf[MSL];
	DESCRIPTOR_DATA *dnew;
	struct sockaddr_in sock;
	struct hostent *from;
	int desc,size;

	size = sizeof(sock);
	getsockname( control, (struct sockaddr *) &sock, (socklen_t *)&size );
	if((desc = accept(control,(struct sockaddr*)&sock,(socklen_t*)&size)) < 0){
		perror("New_descriptor: accept");
		return;
	}

	#if !defined(FNDELAY)
		#define FNDELAY O_NDELAY
	#endif

	if(fcntl(desc,F_SETFL,FNDELAY) == -1){
		perror("New_descriptor: fcntl: FNDELAY");
		return;
	}

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor(); /* new_descriptor now also allocates things */
	dnew->descriptor = desc;

	size = sizeof(sock);
	if(getpeername(desc,(struct sockaddr*)&sock,(socklen_t*)&size) < 0){
		perror("New_descriptor: getpeername");
		dnew->host = str_dup("(unknown)");
	}
	else{
		/*
		 * Would be nice to use inet_ntoa here but it takes a struct arg,
		 * which ain't very compatible between gcc and system libraries.
		 */
		int addr;

		addr = ntohl(sock.sin_addr.s_addr);
		sprintf(buf, "%d.%d.%d.%d",(addr >> 24) & 0xFF,(addr >> 16) & 0xFF,(addr >>  8) & 0xFF,(addr) & 0xFF );
		sprintf(log_buf,"Sock.sinaddr:  %s",buf);
		log_string( log_buf );
		from = gethostbyaddr((char *) &sock.sin_addr,sizeof(sock.sin_addr),AF_INET);
		dnew->host = str_dup(from ? from->h_name : buf);
	}

	/*
	 * Swiftest: I added the following to ban sites.  I don't
	 * endorse banning of sites, but Copper has few descriptors now
	 * and some people from certain sites keep abusing access by
	 * using automated 'autodialers' and leaving connections hanging.
	 *
	 * Furey: added suffix check by request of Nickel of HiddenWorlds.
	 */
	if(check_ban(dnew->host,BAN_ALL)){
		write_to_descriptor(desc,"Your site has been banned from this mud.\n\r",0);
		close(desc);
		free_descriptor(dnew);
		return;
	}
	/*
	 * Init descriptor data.
	 */
	dnew->next			= descriptor_list;
	descriptor_list		= dnew;

	/*
	 * Send the greeting.
	 */

	dnew->connected = CON_GET_NAME;
	dnew->ansi = true;
	{
		extern char * help_greeting;
		if(help_greeting[0] == '.' )
			send_to_desc(help_greeting+1,dnew);
		else
			send_to_desc(help_greeting,dnew);
		if(mud.coder)
			send_to_desc("\n\r{xWelcome to the {RC{Go{Bd{Ye{Cr {MP{Ro{Gr{Bt{R!{G!{B!{x\n\rWhat is your name?\n\r",dnew);
	}
}

void close_socket( DESCRIPTOR_DATA *dclose ){
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
		process_output(dclose,false);

    if ( dclose->snoop_by != NULL )
		write_to_buffer(dclose->snoop_by,"Your victim has left the game.\n\r",0);

    {
		DESCRIPTOR_DATA *d;

		for ( d = descriptor_list; d != NULL; d = d->next )
			if ( d->snoop_by == dclose )
				d->snoop_by = NULL;
    }

    if ((ch = dclose->character)){
		sprintf(log_buf,"Closing link to %s.",ch->name);
		log_string(log_buf);
		/* cut down on wiznet spam when rebooting */
		/* If ch is writing note or playing, just lose link otherwise clear char */
		if ((dclose->connected == CON_PLAYING && !merc_down) || ((dclose->connected >= CON_NOTE_TO) && (dclose->connected <= CON_NOTE_FINISH))){
			ch->remplr(PL_FORGING);
			act("{REvil internet trolls come and drag $n away!",ch,NULL,NULL,TO_ROOM);
			save_who();
			mud.t_con--;
			wiznet("Net death has claimed $N.",ch,NULL,WZ_LINKS,0,0);
			ch->desc = NULL;
		}
		else
			free_char(dclose->original ? dclose->original : dclose->character);
    }

    if ( d_next == dclose )
		d_next = d_next->next;

    if ( dclose == descriptor_list )
		descriptor_list = descriptor_list->next;
    else
    {
		DESCRIPTOR_DATA *d;

		for ( d = descriptor_list; d && d->next != dclose; d = d->next )
			;
		if ( d != NULL )
			d->next = dclose->next;
		else
			bug("Close_socket: dclose not found.",0);
    }

    close(dclose->descriptor);
    free_descriptor(dclose);
    return;
}

bool read_from_descriptor(DESCRIPTOR_DATA *d){
	int iStart;

	/* Hold horses if pending command already. */
	if(d->incomm[0])
		return true;

	/* Check for overflow. */
	iStart = strlen_color(d->inbuf);
	if(iStart >= sizeof(d->inbuf) - 10){
		sprintf(log_buf,"%s input overflow!",d->host);
		log_string(log_buf);
		write_to_descriptor(d->descriptor,"\n\r*** PUT A LID ON IT!!! ***\n\r",0);
		return false;
	}

	/* Snarf input. */
	for (;;){
		int nRead;

		nRead = read(d->descriptor,d->inbuf + iStart,sizeof(d->inbuf) - 10 - iStart);
		if (nRead > 0){
			iStart += nRead;
			if(d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r')
				break;
		}
		else if(nRead == 0){
			log_string("EOF encountered on read.");
			return false;
		}
		else if(errno == EWOULDBLOCK)
			break;
		else{
			perror("Read_from_descriptor");
			return false;
		}
	}
	d->inbuf[iStart] = '\0';
	return true;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(DESCRIPTOR_DATA *d){
	int i, j, k;

	/*
	 * Hold horses if pending command already.
	 */
	if(d->incomm[0])
		return;

	/*
	 * Look for at least one new line.
	 */
	for (i = 0;d->inbuf[i] != '\n' && d->inbuf[i] != '\r';i++)
		if(d->inbuf[i] == '\0')
			return;

	/*
	 * Canonical input processing.
	 */
	for(i = 0,k = 0;d->inbuf[i] != '\n' && d->inbuf[i] != '\r';i++){
		if(k >= MAX_INPUT_LENGTH - 2){
			write_to_descriptor(d->descriptor,"Line too long.\n\r",0);

			/* skip the rest of the line */
			for(;d->inbuf[i];i++)
				if(d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
					break;
			d->inbuf[i]   = '\n';
			d->inbuf[i+1] = '\0';
			break;
		}

		if(d->inbuf[i] == '\b' && k > 0)
			--k;
		else if(isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
			d->incomm[k++] = d->inbuf[i];
	}
	/*
	 * Finish off the line.
	 */
	if(k == 0)
		d->incomm[k++] = ' ';
	d->incomm[k] = '\0';

	if(k > 1 || d->incomm[0] == '!'){
		if ( (d->incomm[0] != '!' && strcmp(d->incomm,d->inlast))
		||(!str_prefix(d->incomm,"west") 
		|| !str_prefix(d->incomm,"north")
		|| !str_prefix(d->incomm,"east")
		|| !str_prefix(d->incomm,"south")
		|| !str_prefix(d->incomm,"down")
		|| !str_prefix(d->incomm,"up")
		|| !str_prefix(d->incomm,"note")))
			d->repeat = 0;
		else{
			if(++d->repeat >= 25 && d->character && d->connected == CON_PLAYING){
				sprintf(log_buf,"%s input spamming!",d->host);
				log_string(log_buf);
				wiznet("$N needs to shut the hell up!",d->character,NULL,WZ_SPAM,0,get_trust(d->character));

				if(d->incomm[0] == '!')
					wiznet(d->inlast,d->character,NULL,WZ_SPAM,0,get_trust(d->character));
				else
					wiznet(d->incomm,d->character,NULL,WZ_SPAM,0,get_trust(d->character));

				write_to_descriptor(d->descriptor,"\n\r*** PUT A LID ON IT!!! ***\n\r",0);
				d->repeat = 0;
				strcpy(d->incomm,"quit");
			}
		}
	}
	/*
	 * Do '!' substitution.
	 */
	if(d->incomm[0] == '!')
		strcpy(d->incomm,d->inlast);
	else
		strcpy(d->inlast,d->incomm);
	/*
	 * Shift the input buffer.
	 */
	while(d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		i++;
	for(j = 0;(d->inbuf[j] = d->inbuf[i+j]);j++)
		;
}

/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt ){
    extern bool merc_down;
	CHAR_DATA *ch,*victim;
	int percent;
	char wound[100],*pbuff,buf[MSL],buffer[MSL*2],woundsm[MSL],outp[MSL];

    /*
     * Bust a prompt.
     */
    if ( !merc_down )
	{
		if ( d->showstr_point )
		{
			sprintf(buf,"\n\r                            {R[{wHit {dReturn {wto continue{R]{x\n\r");
			pbuff	= buffer;
			colorconv( pbuff, buf, d->character );
			write_to_buffer( d, buffer, 0);
		}
		else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
			write_to_buffer( d, "> ", 2 );
		else if ( fPrompt && d->connected == CON_PLAYING )
		{
			ch = d->character;

			/* battle prompt */
			if ((victim = ch->fighting) != NULL){
					if (victim->max_hit > 0)	percent = victim->hit * 100 / victim->max_hit;
					else						percent = -1;

					if (percent >= 100)
					{
						sprintf(wound,"is in {Gexcellent {xcondition.");
						sprintf(woundsm,"{GExcellent{x");
					}
					else if (percent >= 90)
					{
						sprintf(wound,"has a {gfew scratches{x.");
						sprintf(woundsm,"{gScratches{x");
					}
					else if (percent >= 75)
					{
						sprintf(wound,"has some {gsmall wounds and bruises{x.");
						sprintf(woundsm,"{gSmallWounds{x");
					}
					else if (percent >= 50)
					{
						sprintf(wound,"has {Yquite a few wounds{x.");
						sprintf(woundsm,"{YFewWounds{x");
					}
					else if (percent >= 30)
					{
						sprintf(wound,"has some {ybig nasty wounds{x.");
						sprintf(woundsm,"{yNastyWounds{x");
					}
					else if (percent >= 15)
					{
						sprintf(wound,"looks {Rpretty hurt{x.");
						sprintf(woundsm,"{RPrettyHurt{x");
					}
					else if (percent >= 0)
					{
						sprintf(wound,"is in {rawful {xcondition.");
						sprintf(woundsm,"{rAwful{x");
					}
					else
					{
						sprintf(wound,"is {Rb{rl{Re{re{Rd{ri{Rn{rg {xto {ddeath{x.");
						sprintf(woundsm,"{rD{Ry{ri{Rn{rg{x");
					}

					switch(ch->battleprompt){
						default:
						case PROMPT_DEFAULT:
							sprintf(outp,"< {c%d{whp {g%d{wmp {m%d{wmv | {x%s %s {w>{x",ch->hit,ch->getmana(),ch->move,PERS(victim,ch),wound);
							break;
						case PROMPT_BRIEF:
							sprintf(outp,"< {c%d{whp {g%d{wmp {m%d{wmv | {x%s{d : %s{w >{x",ch->hit,ch->getmana(),ch->move,PERS(victim,ch),woundsm);
							break;
						case PROMPT_FULL:
							sprintf(outp,"{w< %s %s {w>\n\r< {c%d{w/{C%d{whp {g%d{w/{G%d{wmp {m%d{w/{M%d{wmv {Y%d{wtnl >{x",PERS(victim,ch),wound,ch->hit,ch->max_hit,ch->getmana(),ch->getmaxmana(),ch->move,ch->max_move,IS_NPC(ch) ? 0 : exp_per_level(ch) - ch->exp);
							break;
					}
					//sprintf(buf,"%s %s \n\r", IS_NPC(victim) ? victim->short_descr : victim->name,wound);
					pbuff	= buffer;
					strcat(outp,"\n\r");
					colorconv(pbuff, outp, d->character );
				if (!ch->iscomm(CM_COMPACT) )
					write_to_buffer( d, "\n\r", 2 );
					write_to_buffer( d, buffer, 0);
			}
			else{
				ch = d->original ? d->original : d->character;
				if (!ch->iscomm(CM_COMPACT) && ch->iscomm(CM_PROMPT))
					write_to_buffer( d, "\n\r", 2 );

				if(ch->iscomm(CM_PROMPT) && !ch->spellsn)
					bust_a_prompt(d->character);
			}

			ch = d->original ? d->original : d->character;

			if (ch->iscomm(CM_TELNET_GA))
				write_to_buffer(d,go_ahead_str,0);
		}
	}
    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
		return true;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
		if (d->character != NULL)
			write_to_buffer( d->snoop_by, d->character->name,0);
		write_to_buffer( d->snoop_by, " > ", 2 );
		write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
		d->outtop = 0;
		return false;
    }
    else
    {
		d->outtop = 0;
		return true;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt(CHAR_DATA *ch){
    EXIT_DATA *pexit;
    const char *str,*i,*dir_name[] = {"N","E","S","W","U","D"};
    char *point,*pbuff,buffer[MSL*2],doors[MIL],buf[MSL],buf2[MSL];
    bool found;
    int door;
 
    point = buf;
    str = ch->prompt;
    if( !str || str[0] == '\0'){
        strcpy(buf,"{W[{C%h{W/{c%H {G%m{W/{g%M {w%v{xmv {Y%X{Wtnl][{y%e{W]{x");
	    free_string(ch->prompt);
		ch->prompt = str_dup(buf);
		return;
    }

	if (ch->iscomm(CM_AFK)){
		if(!IS_NPC(ch) && ch->pcdata->afk[0])	ch->send("<{YAFK{x>");
		else									ch->send("<{yAFK{x> ");
		return;
	}

	if(mud.coder)
		ch->send("{x[{RC{Go{Bd{Ye{Cr{MP{Ro{Gr{Bt{x]");
	while( *str != '\0' ){
		if( *str != '%' ){
			*point++ = *str++;
			continue;
		}
		++str;
		switch( *str ){
			default :
				i = " ";
				break;
			case 'e':
				found = false;
				doors[0] = '\0';
				for (door = 0; door < 6; door++){
					if ((pexit=ch->in_room->exit[door]) != NULL && pexit ->u1.to_room != NULL &&  (can_see_room(ch,pexit->u1.to_room) || ((ch->isaff(AF_INFRARED) || ch->isaff(AF_DARK_VISION)) && !ch->isaff(AF_BLIND)))){
						if(!IS_SET(pexit->exit_info,EX_CLOSED)){
							if (!IS_SET(pexit->exit_info,EX_HIDDEN) && !IS_SET(pexit->exit_info,EX_SUPERINVIS)){
								found = true;
								strcat(doors,dir_name[door]);
							}
							else{
								if (ch->isaff(AF_DETECT_HIDDEN) && !IS_SET(pexit->exit_info,EX_SUPERINVIS)){
									found = true;
									strcat(doors,"[");
									strcat(doors,dir_name[door]);
									strcat(doors,"]");
								}
							}
						}
						else
						{
							if (!IS_SET(pexit->exit_info,EX_HIDDEN) && !IS_SET(pexit->exit_info,EX_SDOOR) && !IS_SET(pexit->exit_info,EX_SUPERINVIS))
							{
								found = true;
								strcat(doors,"(");
								strcat(doors,dir_name[door]);
								strcat(doors,")");
							}
							else
							{
								if (ch->isaff(AF_DETECT_HIDDEN) && !IS_SET(pexit->exit_info,EX_SUPERINVIS))
								{
									found = true;
									strcat(doors,"[(");
									strcat(doors,dir_name[door]);
									strcat(doors,")]");
								}
							}
						}
					}
				}
				if (!found)
	 				strcat(doors,"none");
				sprintf(buf2,"%s",doors);
				i = buf2;
				break;
 			case 'c' :
				sprintf(buf2,"%s","\n\r");
				i = buf2;
				break;
 			case 't' :
				sprintf(buf2,"%s",weather_info.sunlight == SUN_DARK ? "{dN" : "{YD");
				i = buf2;
				break;
			case 'h' :
				sprintf( buf2, "%d", ch->hit );
				i = buf2;
				break;
			case 'H' :
				sprintf( buf2, "%d", ch->max_hit );
				i = buf2;
				break;
			case 'm' :
				sprintf( buf2, "%d", ch->getmana());
				i = buf2;
				break;
			case 'M' :
				sprintf( buf2, "%d", ch->getmaxmana());
				i = buf2;
				break;
			case 'v' :
				sprintf( buf2, "%d", ch->move );
				i = buf2;
				break;
			case 'V' :
				sprintf( buf2, "%d", ch->max_move );
				i = buf2;
				break;
			case 'x' :
				sprintf( buf2, "%d", ch->exp );
				i = buf2;
				break;
			case 'X' :
				sprintf(buf2, "%d", IS_NPC(ch) ? 0 : ch->level < LEVEL_HERO ? exp_per_level(ch) - ch->exp : 0);
				i = buf2;
				break;
			case 'W':
				//get_carry_weight(ch) / 10, can_carry_w(ch) / 10;
				sprintf( buf2, "%d", can_carry_w(ch) / 10);
				i = buf2;
				break;
			case 'w' :
				sprintf( buf2, "%ld", get_carry_weight(ch) / 10);
				i = buf2;
				break;
			case 'g' :
				sprintf( buf2, "%d", ch->gold);
				i = buf2;
				break;
			case 's' :
				sprintf( buf2, "%d", ch->silver);
				i = buf2;
				break;
			case 'a' :
				if( ch->level > 9 )
					sprintf( buf2, "%d", ch->alignment );
				else
					sprintf( buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral" );
				i = buf2;
				break;
			case 'r' :
				if( ch->in_room != NULL )
				sprintf( buf2, "%s", ((!IS_NPC(ch) && ch->isplr(PL_HOLYLIGHT)) || (!ch->isaff(AF_BLIND) && !room_is_dark( ch->in_room )))	? ch->in_room->name : "darkness");
				else
					sprintf( buf2, " " );
				i = buf2;
				break;
			case 'R' :
				if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
					sprintf( buf2, "%d", ch->in_room->vnum );
				else
					sprintf( buf2, " " );
				i = buf2;
				break;
			case 'z' :
				if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
					sprintf( buf2, "%s", ch->in_room->area->name );
				else
					sprintf( buf2, " " );
				i = buf2;
				break;
			case '%' :
				sprintf( buf2, "%%" );
				i = buf2;
				break;
			case 'o' :
				sprintf( buf2, "%s", olc_ed_name(ch) );
				i = buf2;
				break;
			case 'O' :
				sprintf( buf2, "%s", olc_ed_vnum(ch) );
				i = buf2;
				break;
		}
		++str;
		while( (*point = *i) != '\0' )
			++point, ++i;
	}
	*point	= '\0';
	strcat(buf,"\n\r");
	pbuff	= buffer;
	colorconv(pbuff,buf,ch);
	write_to_buffer(ch->desc,buffer,0);

	if(ch->prefix[0])
		write_to_buffer(ch->desc,ch->prefix,0);
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
		length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
		d->outbuf[0]	= '\n';
		d->outbuf[1]	= '\r';
		d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
		char *outbuf;

        if (d->outsize >= 32000)
		{
			bug("Buffer overflow. Closing.\n\r",0);
			close_socket(d);
			return;
 		}
		outbuf = (char *)alloc_mem( 2 * d->outsize );
		strncpy(outbuf,d->outbuf,d->outtop);
		free_mem(d->outbuf,d->outsize);
		d->outbuf = outbuf;
		d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor(int desc,char *txt,int length){
	int iStart,nWrite,nBlock;

	if(length <= 0)
		length = strlen_color(txt);

	for(iStart = 0;iStart < length;iStart += nWrite){
		nBlock = UMIN(length - iStart,4096);
		if((nWrite = write(desc,txt + iStart,nBlock)) < 0){
			perror("Write_to_descriptor");
			return false;
		}
	}
	return true;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    int clan;

    /*
     * Reserved words.
     */
    if (is_exact_name(name,"all auto immortal self someone something the you loner none"))
		return false;

	/*
	 * BadNames!
	*/
	if (check_illegal_name( name ))
		return false;

    /*
     * Length restrictions.
     */
     
    if ( strlen_color(name) <  2 )
		return false;

    if ( strlen_color(name) > 12 )
		return false;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = false,cleancaps = false;
 	int total_caps = 0;

	fIll = true;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
			return false;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
			if (adjcaps)
				cleancaps = true;
			total_caps++;
			adjcaps = true;
	    }
	    else
			adjcaps = false;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
			fIll = false;
	}

	if ( fIll )
	    return false;

	if (cleancaps || (total_caps > (strlen_color(name)) / 2 && strlen_color(name) < 3))
	    return false;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    /*{ NASH blocks this for now, it's a dumbass idea.
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return false;
	    }
	}
    }*/

    return true;
}
bool check_parse_lname(char *name){
    int clan;

    // Reserved words.
    if (is_exact_name(name,"all auto immortal self someone something the you loner none"))
		return false;

	// BadNames!
	if (check_illegal_name( name ))
		return false;

    // Length restrictions.
     
    if ( strlen_color(name) <  2 )
		return false;

    if ( strlen_color(name) > 15 )
		return false;

    // Alphanumerics only. Lock out IllIll twits.
    {
	char *pc;
	bool fIll,adjcaps = false,cleancaps = false;
 	int total_caps = 0;

	fIll = true;
	for ( pc = name; *pc != '\0'; pc++ ){
	    //if ( !isalpha(*pc) )
		//	return false;

	    if ( isupper(*pc)){// ugly anti-caps hack
			if (adjcaps)
				cleancaps = true;
			total_caps++;
			adjcaps = true;
	    }
	    else
			adjcaps = false;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
			fIll = false;
	}

	if (fIll)
	    return false;

	if (cleancaps || (total_caps > (strlen_color(name)) / 2 && strlen_color(name) < 3))
	    return false;
    }

    return true;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(DESCRIPTOR_DATA *d,char *name,bool fConn){
	CHAR_DATA *ch;

	for(ch = char_list;ch;ch = ch->next){
		if(!IS_NPC(ch) && (!fConn || !ch->desc) && !str_cmp(d->character->name,ch->name)){
			if(!fConn){
				free_string( d->character->pcdata->pwd );
				d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
			}
			else{
				free_char(d->character);
				d->character = ch;
				ch->desc	 = d;
				ch->timer	 = 0;
				send_to_char( "{GReconnecting. Type replay to see missed tells.{x\n\r",ch);
				act("{GEvil internet trolls think $n tastes bad and throw $m back.{x",ch,NULL,NULL,TO_ROOM);

				sprintf(log_buf,"%s@%s reconnected.",ch->name,d->host);
				check_multiplay(d);
				log_string(log_buf);

				d_insert(ch->desc);
				mud.t_con++;
				check_max_con();

				wiznet("$N becomes one with $S link.", ch,NULL,WZ_LINKS,0,0);
				d->connected = CON_PLAYING;
				/* Inform the character of a note in progress and the possbility
				 * of continuation!
				 */
				if (ch->pcdata->in_progress)
					send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r", ch);
			}
			return true;
		}
	}
	return false;
}



// Check if already playing.
bool check_playing(DESCRIPTOR_DATA *d,char *name){
	DESCRIPTOR_DATA *dold;

	for ( dold = descriptor_list; dold; dold = dold->next ){
		if (dold != d && dold->character != NULL && dold->connected != CON_GET_NAME && dold->connected != CON_GET_OLD_PASSWORD && !str_cmp(name,dold->original ? dold->original->name : dold->character->name)){
			write_to_buffer(d,"That character is already playing.\n\r",0);
			send_to_desc("Do you wish to connect anyway ({GYes{x/{RNo{x)?",d);
			d->connected = CON_BREAK_CONNECT;
			return true;
		}
	}
	return false;
}



void stop_idling(CHAR_DATA *ch)
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
		return;

    ch->timer = 0;
    char_from_room(ch);
    char_to_room(ch,ch->was_in_room);
    ch->was_in_room	= NULL;
    act("$n has returned from {dTh{we {xV{wo{did{x.",ch,NULL,NULL,TO_ROOM);
	ch->send("You return from {dTh{we {xV{wo{did{x.\n\r");
    return;
}



/*
 * Write to one char.
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

/*
 * Write to one char, new color version, by Lope.
 */
void send_to_desc( const char *txt, DESCRIPTOR_DATA *d )
{
	const	char 	*point;
	char 	*point2;
	char 	buf[ MSL*4 ];
	int	skip = 0;

	buf[0] = '\0';
	point2 = buf;
	if( txt && d )
	{
		if( d->ansi == true )
		{
			for( point = txt ; *point ; point++ )
			{
				if( *point == '{' )
				{
					point++;
					skip = color( *point, NULL, point2 );
					while( skip-- > 0 )
					++point2;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}			
			*point2 = '\0';
			write_to_buffer( d, buf, point2 - buf );
		}
		else
		{
			for( point = txt ; *point ; point++ )
			{
				if( *point == '{' )
				{
					point++;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}
			*point2 = '\0';
			write_to_buffer( d, buf, point2 - buf );
		}
	}
	return;
}
/*
 * Write to one char, new color version, by Lope.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
	const char *point;
	char *point2;
	char buf[ MSL*4 ];
	int	skip = 0;

	buf[0] = '\0';
	point2 = buf;
	if(txt && ch->desc)
	{
		if (ch->isplr(PL_COLOR))
		{
			for( point = txt ; *point ; point++ )
			{
				if( *point == '{' )
				{
					point++;
					skip = color( *point, ch, point2 );
					while( skip-- > 0 )
						++point2;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}			
			*point2 = '\0';
			write_to_buffer( ch->desc, buf, point2 - buf );
		}
		else
		{
			for( point = txt ; *point ; point++ )
			{
				if( *point == '{' )
				{
					point++;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}
			*point2 = '\0';
			write_to_buffer( ch->desc, buf, point2 - buf );
		}
	}
	return;
}
/*
 * Test for sending stuff to a char
 */
void char_data :: send(const char *fmt, ...){
	const char *point;
	char *point2;
	char buf[ MSL*4 ];
	char buf2[ MSL*4 ];
	int	skip = 0;
        va_list args;
        va_start (args, fmt);
        vsprintf (buf2, fmt, args);
        va_end (args);
	char *txt = buf2;

	buf[0] = '\0';
	point2 = buf;
	if(txt && desc){
		if (desc->character->isplr(PL_COLOR)){
			for( point = txt ; *point ; point++){
				if( *point == '{' ){
					point++;
					skip = color( *point,desc->character, point2 );
					while( skip-- > 0 )
						++point2;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}			
			*point2 = '\0';
			write_to_buffer(desc, buf, point2 - buf );
		}
		else{
			for( point = txt ; *point ; point++ ){
				if( *point == '{' ){
					point++;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}
			*point2 = '\0';
			write_to_buffer(desc, buf, point2 - buf );
		}
	}
}
void char_data :: printf(char *fmt,...){
	char buf[MSL];
	va_list args;
	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	va_end(args);
	
	send_to_char(buf,this);
}

/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL)
	return;

    if (ch->lines == 0 )
    {
	send_to_char(txt,ch);
	return;
    }
	
    ch->desc->showstr_head = (char *)alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
}

/*
 * Page to one char, new color version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch ){
    const	char	*point;
    		char	*point2;
    		char	buf[ MSL * 6 ];
		int	skip = 0;

    buf[0] = '\0';
    point2 = buf;
    if(txt && ch->desc){
	    if(ch->isplr(PL_COLOR)){
			for( point = txt ; *point ; point++ ){
				if( *point == '{' )
				{
					point++;
					skip = color( *point, ch, point2 );
					while( skip-- > 0 )
						++point2;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}			
			*point2 = '\0';
			ch->desc->showstr_head  =(char*) alloc_mem( strlen_color( buf ) + 1 );
			strcpy( ch->desc->showstr_head, buf );
			ch->desc->showstr_point = ch->desc->showstr_head;
			show_string( ch->desc, "" );
	    }
	    else
	    {
			for( point = txt ; *point ; point++ )
				{
				if( *point == '{' )
				{
					point++;
					continue;
				}
				*point2 = *point;
				*++point2 = '\0';
			}
			*point2 = '\0';
			ch->desc->showstr_head  = (char*)alloc_mem( strlen_color( buf ) + 1 );
			strcpy( ch->desc->showstr_head, buf );
			ch->desc->showstr_point = ch->desc->showstr_head;
			show_string( ch->desc, "" );
	    }
	}
    return;
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MSL];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
		if (d->showstr_head)
		{
			free_mem(d->showstr_head,strlen_color(d->showstr_head));
			d->showstr_head = 0;
		}
		d->showstr_point  = 0;
		return;
    }

    if (d->character)
		show_lines = d->character->lines;
    else
		show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
		if (((*scan = *d->showstr_point) == '\n' || *scan == '\r') && (toggle = -toggle) < 0)
			lines++;
		else if (!*scan || (show_lines > 0 && lines >= show_lines))
		{
			*scan = '\0';
			write_to_buffer(d,buffer,strlen_color(buffer));
			for (chk = d->showstr_point; isspace(*chk); chk++);
			{
				if (!*chk)
				{
					if (d->showstr_head)
        			{
            			free_mem(d->showstr_head,strlen_color(d->showstr_head));
            			d->showstr_head = 0;
        			}
        			d->showstr_point  = 0;
    			}
			}
			return;
		}
    }
    return;
}

void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act_new(const char *format,CHAR_DATA *ch,const void *arg1,const void *arg2,int type,int min_pos){
    CHAR_DATA *to,*vch = (CHAR_DATA*)arg2;
    OBJ_DATA *obj1 = (OBJ_DATA*)arg1,*obj2 = (OBJ_DATA*)arg2;
	ROOM_INDEX_DATA *foyerroom,*pArenaView,*viewerroom;
    const char *str;
    char *i2 = NULL,*i = NULL,*point,*pbuff,buffer[MSL*2],buf[MSL],fname[MIL],buf2[MSL],fixed[MSL];
    bool fColor = false,in_arena = false;
 
    // Discard null and zero-length messages.
	if(!format || !*format)
		return;

    // discard null rooms and chars
	if(!ch || !ch->in_room)
		return;

	if(!IS_NPC(ch) && ch->isplr(PL_ARENA) && (type == TO_ROOM || type == TO_NOTVICT)){
		foyerroom = get_room_index(ch->foyervnum);
		if(!foyerroom)
			return;
		viewerroom = get_room_index(foyerroom->arenaviewvn);
		if(!viewerroom)
			return;

		in_arena = true;
		pArenaView = viewerroom;
	}

	if(in_arena && pArenaView->people && (type == TO_ROOM || type == TO_NOTVICT)){
		to = pArenaView->people;
		for(;to;to = to->next_in_room){
			if((!IS_NPC(to) && to->desc == NULL) || (IS_NPC(to)) || to->position < min_pos)
				continue;
			point   = buf;
			str     = format;
			while( *str ){
				if( *str != '$' && *str != '{' ){
					*point++ = *str++;
					continue;
				}

				i = NULL;
				switch( *str ){
					case '$':
						fColor = true;
						++str;
						i = " <@@@> ";
						if ( !arg2 && *str >= 'A' && *str <= 'Z' && *str != 'G' ){
							bug( "Act: missing arg2 for code %d.", *str );
							i = " <@@@> ";
						}
						else{
							switch ( *str ){
								default:
									bug( "Act: bad code %d.", *str );
									i = " <@@@> ";
									break;
								case 't':
									i = (char *) arg1;
									break;
								case 'T':
									i = (char *) arg2;
									break;
								case 'n':
									i = (char*)PERS( ch,  to  );
									break;
								case 'N':
									i = (char*)PERS( vch, to  );
									break;
								case 'e':
									i = sex_table[SEX(ch)].eword;
									break;
								case 'E':
									i = sex_table[SEX(vch)].eword;
									break;
								case 'm':
									i = sex_table[SEX(ch)].mword;
									break;
								case 'M':
									i = sex_table[SEX(vch)].mword;
									break;
								case 'o' :
									sprintf( buf2, "%s", olc_ed_name(ch) );
									i = buf2;
									break;
								case 'O' :
									sprintf( buf2, "%s", olc_ed_vnum(ch) );
									i = buf2;
									break;
								case 's':
									i = sex_table[SEX(ch)].sword;
									break;
								case 'S':
									i = sex_table[SEX(vch)].sword;
									break;
								case 'p':
									i = (char*)(can_see_obj( to, obj1 ) ? obj1->short_descr : "something");
									break;
								case 'P':
									i = (char*)(can_see_obj( to, obj2 ) ? obj2->short_descr : "something");
									break;
								case 'd':
									if ( !arg2 || ((char *) arg2)[0] == '\0' ){
										i = "door";
									}
									else{
										one_argument( (char *) arg2, fname );
										i = fname;
									}
									break;
								case 'G':
									if ( ch->alignment < 0 ){
										i = "Tribunal";
									}
									else{
										i = "Galactus";
									}
									break;
							}
						}
						break;
					case '{':
						fColor = false;
						++str;
						i = NULL;
						if(to->isact(PL_COLOR)){
							/*i = */color( *str, to,i );
						}
						break;
					default:
						fColor = false;
						*point++ = *str++;
						break;
				}

				++str;
				if( fColor && i ){
					fixed[0] = '\0';
					i2 = fixed;

					if(to->isact(PL_COLOR ) ){
						for( i2 = fixed ; *i ; i++ ){
							if( *i == '{' ){
								i++;
								/*strcat( fixed, */color( *i, to,/**/fixed ) /*)*/;
								for( i2 = fixed ; *i2 ; i2++ )
									;
								continue;
							}
							*i2 = *i;
							*++i2 = '\0';
						}
						*i2 = '\0';
						i = &fixed[0];
					}
					else{
						for( i2 = fixed ; *i ; i++ ){
							if( *i == '{' ){
								i++;
								if( *i != '{' ){
									continue;
								}
							}
							*i2 = *i;
							*++i2 = '\0';
						}
						*i2 = '\0';
						i = &fixed[0];
					}
				}
				if( i ){
					while( ( *point = *i ) != '\0' ){
						++point;
						++i;
					}
				}
			}

			*point++        = '\n';
			*point++        = '\r';
			*point          = '\0';
			buf[0]          = UPPER( buf[0] );
			if (to->desc && (to->desc->connected == CON_PLAYING))
				write_to_buffer( to->desc, buf, point - buf );
		} /* end of for-loop */
	} /* end of if check for arena viewers */


	to = ch->in_room->people;

	if(type == TO_VICT){
		if(!vch){
			bug("Act: null vch with TO_VICT.",0);
			return;
		}

		if(!vch->in_room)
			return;

		to = vch->in_room->people;
	}

	for(;to;to = to->next_in_room){
		if ((!IS_NPC(to) && to->desc == NULL ) || (IS_NPC(to) && !HAS_TRIGGER_MOB(to,TRIG_ACT) && to->desc == NULL) || to->position < min_pos)
			continue;
		if(!to->desc || to->position < min_pos)
			continue;
		if((type == TO_CHAR) && to != ch)
			continue;
		if(type == TO_VICT && (to != vch || to == ch))
			continue;
		if(type == TO_ROOM && to == ch)
			continue;
		if(type == TO_NOTVICT && (to == ch || to == vch))
			continue;

		point   = buf;
		str     = format;
		while(*str != '\0'){
			if(*str != '$'){
				*point++ = *str++;
				continue;
			}
			fColor = true;
			++str;
			i = " <@@@> ";
			if( !arg2 && *str >= 'A' && *str <= 'Z' ){
				bug( "Act: missing arg2 for code %d.", *str );
				i = " <@@@> ";
			}
			else{
				switch ( *str ){
					default: bug( "Act: bad code %d.", *str ); i = " <@@@> ";				break;
					case 't': i = (char *) arg1;											break;
					case 'T': i = (char *) arg2;											break;
					case 'n': i = (char *)PERS(ch,to);											break;
					case 'N': i = (char *)PERS(vch,to);											break;
					case 'e': i = sex_table[SEX(ch)].eword;						break;
					case 'E': i = sex_table[SEX(vch)].eword;						break;
					case 'm': i = sex_table[SEX(ch)].mword;						break;
					case 'M': i = sex_table[SEX(vch)].mword;						break;
					case 's': i = sex_table[SEX(ch)].sword;						break;
					case 'S': i = sex_table[SEX(vch)].sword;						break;
					case 'p': i = can_see_obj(to,obj1) ? obj1->short_descr : (char*)"something";	break;
					case 'P': i = can_see_obj(to,obj2) ? obj2->short_descr : (char*)"something";break;
					case 'd':
						if (arg2 == NULL || ((char *) arg2)[0] == '\0')
							i = "door";
						else{
							one_argument((char *) arg2,fname);
							i = fname;
						}
						break;
				}
			}
			++str;
			while ((*point = *i) != '\0')
				++point, ++i;
		}

		*point++ = '\n';
		*point++ = '\r';
		*point   = '\0';
		buf[0]   = UPPER(buf[0]);
		pbuff	 = buffer;
		colorconv(pbuff,buf,to);
		capitalize(buffer);
		if (to->desc
		&& (to->desc->connected == CON_PLAYING
		|| to->desc->connected == CON_NOTE_TO
		|| to->desc->connected == CON_NOTE_SUBJECT
		|| to->desc->connected == CON_NOTE_EXPIRE
		|| to->desc->connected == CON_NOTE_TEXT
		|| to->desc->connected == CON_NOTE_FINISH))
			write_to_buffer( to->desc, buffer, 0 );
		else// if (HAS_TRIGGER_MOB(to,TRIG_ACT))
			global_message(0,MAX_LEVEL,"ROAR\n\r",0);
//			p_act_trigger( buf, to, NULL, NULL, ch, NULL, NULL, TRIG_ACT );
	}

	if ( type == TO_ROOM || type == TO_NOTVICT ){
		OBJ_DATA *obj, *obj_next;
		CHAR_DATA *tch, *tch_next;

		point   = buf;
		str     = format;
		while( *str != '\0' )
			*point++ = *str++;
		*point   = '\0';
 
		for( obj = ch->in_room->contents; obj; obj = obj_next ){
			obj_next = obj->next_content;
			if ( HAS_TRIGGER_OBJ(obj,TRIG_ACT) )
				p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
		}

		for( tch = ch; tch; tch = tch_next ){
			tch_next = tch->next_in_room;

			for ( obj = tch->carrying; obj; obj = obj_next ){
				obj_next = obj->next_content;
				if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
					p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
			}
		}

		if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_ACT ) )
			p_act_trigger( buf, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_ACT );
	}
}

void log_f (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);
	
	log_string (buf);
}

int color( char type, CHAR_DATA *ch, char *string )
{
    char	code[ 20 ];
    char	*p = '\0';

    if( ch  && IS_NPC( ch ) ) 
		return( 0 );

    switch( type )
    {
		default:
			sprintf( code, CLEAR );
			break;
		case 'x':
			sprintf( code, CLEAR );
			break;
		case 'b':
			sprintf( code, C_BLUE );
			break;
		case 'c':
			sprintf( code, C_CYAN );
			break;
		case 'g':
			sprintf( code, C_GREEN );
			break;
		case 'd':
			sprintf( code, CB_D_GREY );
			break;
		case 'm':
			sprintf( code, C_MAGENTA );
			break;
		case 'r':
			sprintf( code, C_RED );
			break;
		case 'w':
			sprintf( code, C_WHITE );
			break;
		case 'y':
			sprintf( code, C_YELLOW );
			break;
		case 'B':
			sprintf( code, C_B_BLUE );
			break;
		case 'C':
			sprintf( code, C_B_CYAN );
			break;
		case 'G':
			sprintf( code, C_B_GREEN );
			break;
		case 'M':
			sprintf( code, C_B_MAGENTA );
			break;
		case 'R':
			sprintf( code, C_B_RED );
			break;
		case 'W':
			sprintf( code, C_B_WHITE );
			break;
		case 'Y':
			sprintf( code, C_B_YELLOW );
			break;
		case 'D':
			sprintf( code, C_D_GREY );
			break;
		case '*':
			sprintf( code, "%c", 007 );
			break;
		case '/':
			sprintf( code, "%c", 012 );
			break;
		case '{':
			sprintf( code, "{"  );
			break;
    }

    p = code;
    while( *p != '\0' )
    {
		*string = *p++;
		*++string = '\0';
    }

    return strlen(code);
}

void colorconv(char *buffer,const char *txt,CHAR_DATA *ch){
    const char *point;
	int	skip = 0;

	if(ch->desc && txt){
		if (ch->isplr(PL_COLOR)){
			for(point = txt;*point;point++){
				if(*point == '{'){
					point++;
					skip = color(*point,ch,buffer);
					while(skip-- > 0)
						++buffer;
					continue;
				}
				*buffer = *point;
				*++buffer = '\0';
			}			
			*buffer = '\0';
		}
		else{
			for(point = txt;*point;point++){
				if(*point == '{'){
					point++;
					continue;
				}
				*buffer = *point;
				*++buffer = '\0';
			}
			*buffer = '\0';
		}
    }
    return;
}

void printf_to_char (CHAR_DATA *ch, char *fmt, ...){
	char buf [MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);
	
	send_to_char (buf, ch);
}

void bugf (char * fmt, ...)
{
	char buf [2*MSL];
	va_list args;
	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	bug (buf, 0);
}

extern int  find_door       args( ( CHAR_DATA *ch, char *arg ) );

void do_knock(CHAR_DATA *ch, char *argument)
{
	/* Constructs taken from do_open().  */
	int door;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument,arg);

	if (arg[0] == '\0')
	{
		send_to_char("Knock on what?\n\r",ch);
		return;
	}

	if ( ( door = find_door( ch, arg ) ) >= 0 )
	{
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];

		act( "$n knocks on the $d.", ch, NULL, pexit->keyword, TO_ROOM);
		act( "You knock on the $d.", ch, NULL, pexit->keyword, TO_CHAR);

		/* Notify the other side.  */
		if ( ( to_room = pexit->u1.to_room ) != NULL && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL && pexit_rev->u1.to_room == ch->in_room )
		{
			CHAR_DATA *rch;
			for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
				act( "You hear someone knocking.", rch, NULL, pexit_rev->keyword, TO_CHAR);
		}
	}

	return;
}

bool check_illegal_name( char *name )
{
    char strsave[MIL], nameread[MSL];
    FILE *fp;
    fclose( fpReserve );
    sprintf( strsave, "%s", ILLEGAL_NAME_FILE );
    if ( (fp = fopen( strsave, "r" ) ) != NULL )
    {
        for ( ; ; )
		{
			fscanf (fp, " %s", nameread);
			if ( !str_cmp( nameread, "END" ) )
				break;
			else if (is_name(name,nameread))
				return true;
		}
	}
	else
		fp = fopen( NULL_FILE, "r" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return false;
}

void d_insert(DESCRIPTOR_DATA *dnew){/*
	DESCRIPTOR_DATA *d_last,*d_next,*d;

	if(!descriptor_list->next || !dnew)
		return;

	if(dnew != descriptor_list){
		for(d = descriptor_list;d && d->next != dnew;d = d->next)
			;
		if(d)
			d->next = dnew->next;
		}
	}
	else
		descriptor_list = dnew->next;

	if(!descriptor_list->next){
		if(dnew->character->level > descriptor_list->character->level){
			dnew->next = descriptor_list;
			descriptor_list = dnew;
		}
		else
			descriptor_list->next = dnew;
		return;
	}
	for(d = descriptor_list;d && d->character->level > dnew->character->level;d=d->next)
		;
	if(d){
		dnew->next = d->next;
		d->next = dnew;
	}*/
}
