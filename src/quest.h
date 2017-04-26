enum{//input types
	QIN_ITEMGIVE = 0,
	QIN_MONEYGIVE,
	QIN_KILL,
	QIN_SPEECH,
	QIN_ITEMGET,
	QIN_ITEMQUAFF,
	QIN_ITEMBRANDISH,
	QIN_SPELLCAST,
	QIN_ITEMEAT,
	QIN_ITEMDRINK,
	QIN_ITEMOPEN,
	QIN_ITEMSIT,
	QIN_ARRIVE,//Good WITH QIN_KILL
	MAX_QINTYPE
};
enum{
	QPRIZE_XP = 0,
	QPRIZE_GOLD,
	QPRIZE_CREDITS,
	QPRIZE_ITEM,
	QPRIZE_SKILL,//uh
	MAX_QPRIZE
};
struct qpart_data{
	int qtype;
	int iinput;
	char *cinput;
	char *startmsg,*endmsg;
	int input_type;
	bool isstart,isfinish;
};
struct quest_data{
	int vnum;
	qpart_data qparts[25];//is this enough?
	int reward_type;
	int reward_num;
	QUEST_DATA *next;

	void show();
};
