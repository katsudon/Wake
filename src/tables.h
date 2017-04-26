#ifndef _TABLES_H_
#define _TABLES_H_

/* game tables */
extern 	const   struct	chan_type       chan_table[];

struct chan_type{
	char *chan_pre;
	char *chan_spre;
	char *chan_emote;
	char *chan_name;
	char *chan_say;
	int bit;
	bool canemote;
};
extern	const	struct	morph_type	morph_table[MAX_MORPH * 5];

extern	const	struct	part_type	part_table[];
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type	sex_table[];
extern	const	struct	size_type	size_table[];

/* flag tables */
extern	const	struct	flag_type	act_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	flag_type	climate_flags[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	off_flags[];
extern	const	struct	flag_type	def_flags[];
extern	const	struct	flag_type	form_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	pick_table[];
extern	const	struct	flag_type	exclude_flags[];
extern	const	struct	flag_type	armortype_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	portal_flags[];
extern	const	struct	flag_type	room_flags[];
extern	const	struct	flag_type	exit_flags[];
extern 	const	struct  flag_type	mprog_flags[];
extern	const	struct	flag_type	area_flags[];
extern	const	struct	flag_type	sector_flags[];
extern	const	struct	flag_type	door_resets[];
extern	const	struct	flag_type	wear_loc_strings[];
extern	const	struct	flag_type	wear_loc_flags[];
extern	const	struct	flag_type	res_flags[];
extern	const	struct	flag_type	dam_flags[];
extern	const	struct	flag_type	type_flags[];
extern	const	struct	flag_type	apply_flags[];
extern	const	struct	flag_type	stat_flags[];
extern	const	struct	flag_type	abbrev_stat_flags[];
extern	const	struct	flag_type	stat_mini[];
extern	const	struct	flag_type	sex_flags[];
extern	const	struct	flag_type	furniture_flags[];
extern	const	struct	flag_type	weapon_class[];
extern	const	struct	flag_type	trap_class[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	weapon_type2[];
extern	const	struct	flag_type	trap_type[];
extern	const	struct	flag_type	apply_types[];
extern	const	struct	flag_type	size_flags[];
extern	const	struct	flag_type	position_flags[];
extern	const	struct	flag_type	ac_type[];
extern	const	struct	flag_type	area_groups[];
extern	const	struct	bit_type	bitvector_type[];
extern	const	struct	flag_type	oprog_flags[];
extern	const	struct	flag_type	rprog_flags[];
extern	const	struct	flag_type	arrow_flags[];
extern	const	struct	sect_type	sect_table[];
extern	const	struct	excl_type	excl_table[];
extern 	const	struct  material_type	material_flags[];

#endif
