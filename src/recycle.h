#ifndef _RECYCLE_H_
#define _RECYCLE_H_
/* externs */
extern char str_empty[1];
extern int mobile_count;

/* stuff for providing a crash-proof buffer */

#define MAX_BUF		16384
#define MAX_BUF_LIST 	10
#define BASE_BUF 	1024

/* valid states */
#define BUFFER_SAFE	0
#define BUFFER_OVERFLOW	1
#define BUFFER_FREED 	2

/* ban data recycling */
BAN_DATA	*new_ban ( void );
void	free_ban ( BAN_DATA* );

/* descriptor recycling */
DESCRIPTOR_DATA	*new_descriptor ( void );
void	free_descriptor ( DESCRIPTOR_DATA* );

/* extra descr recycling */
EXTRA_DESCR_DATA	*new_extra_descr ( void );
void	free_extra_descr ( EXTRA_DESCR_DATA* );

/* affect recycling */
AFFECT_DATA	*new_affect ( void );
void	free_affect ( AFFECT_DATA* );

/* object recycling */
OBJ_DATA	*new_obj ( void );
void	free_obj ( OBJ_DATA* );

/* character recyling */
CHAR_DATA	*new_char ( void );
void	free_char ( CHAR_DATA* );
PC_DATA	*new_pcdata ( void );
void	free_pcdata ( PC_DATA* );


/* mob id and memory procedures */
long 		get_pc_id		( void );
long		get_mob_id		( void );
MEM_DATA	*new_mem_data	( void );
void		free_mem_data	( MEM_DATA* );
MEM_DATA	*find_memory	( MEM_DATA*,long );

/* buffer procedures */
BUFFER	*new_buf		( void );
BUFFER  *new_buf_size	( int );
void	free_buf		( BUFFER* );
bool	add_buf			( BUFFER*,char* );
void	clear_buf		( BUFFER* );
char	*buf_string		( BUFFER* );

HELP_AREA *	new_had		( void );
HELP_DATA *	new_help	( void );
void		free_help	( HELP_DATA* );

void free_sleep_data(SLEEP_DATA *sd);
SLEEP_DATA *new_sleep_data(void);
#endif

