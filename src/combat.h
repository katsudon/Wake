#ifndef _COMBAT_H_
#define _COMBAT_H_

int		melee_hit				( CHAR_DATA*,CHAR_DATA*,int,bool );
int		check_brawler			( CHAR_DATA*,int );
void	check_assist			( CHAR_DATA*,CHAR_DATA* );
bool	check_damagereduction	( CHAR_DATA*,CHAR_DATA*,int );
bool	check_poverty			( CHAR_DATA* );
void	check_readied			( CHAR_DATA* );
int		get_attackspeed			( CHAR_DATA*,bool );
void	shield_combat			( CHAR_DATA*,CHAR_DATA*,bool );
int		kindred_spirits			( CHAR_DATA*,CHAR_DATA*,int );
void	check_sonic_impact		( CHAR_DATA*,CHAR_DATA*,bool );
void	get_new_killer			( CHAR_DATA*,CHAR_DATA* );
int		dual_nerf				( CHAR_DATA*,int,bool );
int		check_improved_accuracy ( CHAR_DATA*,int );
int		heavy_hand				( CHAR_DATA*,int );
void	check_fuse				( CHAR_DATA*,CHAR_DATA*,int );
void	end_duel				( CHAR_DATA*,CHAR_DATA* );
bool	check_blind_fighting	( CHAR_DATA*,CHAR_DATA* );
bool	check_tierlevel			( CHAR_DATA* );
bool	check_stamina			( CHAR_DATA* );
void	check_energy_shield		( CHAR_DATA*,CHAR_DATA* );
int		check_furor				( CHAR_DATA* );
void	set_fighting			( CHAR_DATA*,CHAR_DATA* );
void	dam_message				( CHAR_DATA*,CHAR_DATA*,int,int,bool );
void	do_emote				( CHAR_DATA*,char * );
void	mob_hit					( CHAR_DATA*,CHAR_DATA*,int,bool );
int		xp_compute				( CHAR_DATA*,CHAR_DATA*,int,int,int );
void	message_stuff			( CHAR_DATA*,CHAR_DATA*,CHAR_DATA*,bool,int,char,const char *,const char *,int );
void	one_hit					( CHAR_DATA*,CHAR_DATA*,int,bool,bool );
void	group_gain				( CHAR_DATA*,CHAR_DATA* );
void	raw_kill				( CHAR_DATA*,CHAR_DATA* );
void	snake_bite				( CHAR_DATA*,CHAR_DATA* );
bool	check_animal_magnetism	( CHAR_DATA*,CHAR_DATA* );
bool	check_true_belief		( CHAR_DATA* );
#endif
