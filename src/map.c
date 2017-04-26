/************************************************************************/
/* mlkesl@stthomas.edu	=====>	Ascii Automapper utility		*/
/* Let me know if you use this. Give a newbie some credit,		*/
/* at least I'm not asking how to add classes...			*/
/* Also, if you fix something could ya send me mail about, thanks	*/
/* PLEASE mail me if you use this ot like it, that way I will keep it up*/
/************************************************************************/
/* MapArea -> 	when given a room, ch, x, and y,...			*/
/*	   	this function will fill in values of map as it should 	*/
/* ShowMap -> 	will simply spit out the contents of map array		*/
/*		Would look much nicer if you built your own areas	*/
/*		without all of the overlapping stock Rom has		*/
/* do_map  ->	core function, takes map size as argument		*/
/************************************************************************/
/* To install:: 							*/
/*	remove all occurances of "u1." (or union your exits)		*/
/*	add do_map prototypes to interp.c and merc.h (or interp.h)	*/
/*	customize the first switch with your own sectors		*/
/*	remove the color codes or change to suit your own color coding	*/
/* Other stuff::							*/
/*	make a skill, call from do_move (only if ch is not in city etc) */
/*	allow players to actually make ITEM_MAP objects			*/
/* 	change your areas to make them more suited to map code! :)	*/
/************************************************************************/ 	
#include <string.h>
#include <stdlib.h>
#include <stdio.h> 
#include <time.h> 
#include "merc.h"

#define MAX_MAP 90
#define MAX_MAP_DIR 4

char *map[MAX_MAP][MAX_MAP];
int offsets[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1} };

void MapArea 
(ROOM_INDEX_DATA *room, CHAR_DATA *ch, int x, int y, int min, int max)
{
ROOM_INDEX_DATA *prospect_room;
CHAR_DATA *rch;
EXIT_DATA *pexit;
int door;

/* marks the room as visited */
switch (room->sector_type)
{
case SECT_INSIDE:			map[x][y]="{W%{x";		break;
case SECT_CITY:				map[x][y]="{W#{x";		break;
case SECT_FIELD:			map[x][y]="{G\"{x";		break;
case SECT_FOREST:			map[x][y]="{g@{x";		break;
case SECT_HILLS:			map[x][y]="{G^{x";		break;
case SECT_MOUNTAIN:			map[x][y]="{y^{x";		break;
case SECT_WATER_SWIM:		map[x][y]="{B~{x";		break;
case SECT_WATER_NOSWIM:		map[x][y]="{b~{x";		break;
case SECT_UNDERWATER:		map[x][y]="{bX{x";		break;
case SECT_AIR:				map[x][y]="{C:{x";		break;
case SECT_DESERT		:	map[x][y]="{Y={x";		break;
case SECT_VOLCANO:			map[x][y]="{r1{x";		break;
case SECT_RAINFOREST:		map[x][y]="{g2{x";		break;
case SECT_SWAMP:			map[x][y]="{c3{x";		break;
case SECT_RIVER:			map[x][y]="{B4{x";		break;
case SECT_TUNNEL:			map[x][y]="{d5{x";		break;
case SECT_FORESTCITY:		map[x][y]="{g6{x";		break;
case SECT_MOUNTAINCITY:		map[x][y]="{y7{x";		break;
case SECT_DESERTCITY:		map[x][y]="{y8{x";		break;
case SECT_GRAVEYARD:		map[x][y]="{d9{x";		break;
case SECT_ROAD:				map[x][y]="{d0{x";		break;
case SECT_DIRTROAD:			map[x][y]="{yA{x";		break;
case SECT_CITYROAD:			map[x][y]="{wB{x";		break;
case SECT_DIRTPATH:			map[x][y]="{yC{x";		break;
case SECT_GOODTEMPLE:		map[x][y]="{wD{x";		break;
case SECT_NEUTRALTEMPLE:	map[x][y]="{xE{x";		break;
case SECT_EVILTEMPLE:		map[x][y]="{dF{x";		break;
case SECT_CLIFFSIDE:		map[x][y]="{yG{x";		break;
case SECT_ICETUNNEL:		map[x][y]="{CH{x";		break;
case SECT_BOARDWALK:		map[x][y]="{YI{x";		break;
case SECT_BEACH:			map[x][y]="{YJ{x";		break;
case SECT_SEWER:			map[x][y]="{dK{x";		break;
default: 					map[x][y]="{yo{x";
}
   for (rch=room->people; rch != NULL; rch=rch->next_in_room)
   {
	if (!IS_NPC(rch))
	map[x][y]="{b!{x";
   }

    for ( door = 0; door < MAX_MAP_DIR; door++ ) 
    {
	if (
             (pexit = room->exit[door]) != NULL
	     &&   pexit->u1.to_room != NULL 
	     &&   can_see_room(ch,pexit->u1.to_room)  /* optional */
	     &&   !IS_SET(pexit->exit_info, EX_CLOSED)
           )
        { /* if exit there */

	prospect_room = pexit->u1.to_room;

        if ( prospect_room->exit[rev_dir[door]] &&
	 prospect_room->exit[rev_dir[door]]->u1.to_room!=room)
		{ /* if not two way */
		if ((prospect_room->sector_type==SECT_CITY)
		||  (prospect_room->sector_type==SECT_INSIDE))
			map[x][y]="{W@{x";
		else
			map[x][y]="{D?{x";
		return;
		} /* end two way */

        if ((x<=min)||(y<=min)||(x>=max)||(y>=max)) return;
        if (map[x+offsets[door][0]][y+offsets[door][1]]==NULL) {
                MapArea (pexit->u1.to_room,ch,
                    x+offsets[door][0], y+offsets[door][1],min,max);
        }

	} /* end if exit there */
    }
return;
}

void ShowMap( CHAR_DATA *ch, int min, int max)
{
int x,y;

    for (x = min; x < max; ++x) 
    {
         for (y = min; y < max; ++y)
         {
	   if (map[x][y]==NULL) send_to_char(" ",ch);		
	   else 		send_to_char(map[x][y],ch); 	
         }
      send_to_char("\n\r",ch); 
    }   

return;
}

void do_map2( CHAR_DATA *ch, char *argument )
{
int size,center,x,y,min,max;
char arg1[10];

   one_argument( argument, arg1 );
   size = atoi (arg1);

size=URANGE(14,size,90);
center=MAX_MAP/2;

min = MAX_MAP/2-size/2;
max = MAX_MAP/2+size/2;

for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
                  map[x][y]=NULL;

/* starts the mapping with the center room */
MapArea(ch->in_room, ch, center, center, min, max); 

/* marks the center, where ch is */
map[center][center]="{R*{x";
ShowMap (ch, min, max); 

return;

}

typedef struct detail_map_holder
{	int x,y;
	ROOM_INDEX_DATA ***map;
	char **details;
} DetailMapHolder;

typedef struct room_holder
{	ROOM_INDEX_DATA *pRoom;
	int x,y,dir;
	struct room_holder *next, *from;
} RoomHolder;

typedef struct path_holder
{	ROOM_INDEX_DATA *pRoom;
	int to, from;
	struct path_holder *next;
} PathHolder;

bool has_shop(ROOM_INDEX_DATA *pRoom )
{	CHAR_DATA *keeper;
	for ( keeper = pRoom->people; keeper; keeper = keeper->next_in_room )
	{	if ( IS_NPC(keeper) && keeper->pIndexData->pShop != NULL )
			return true;
	}
	return false;
}
PathHolder * on_path(PathHolder *path, ROOM_INDEX_DATA *pRoom)
{	PathHolder *pPath;

	for(pPath = path ; pPath ; pPath = pPath->next )
	{	if(pRoom == pPath->pRoom)
			return pPath;
	}
	return NULL;
}

char *get_str(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, int level, PathHolder *path )
{	static char buf[4];
	PathHolder *pPath;
	sprintf(buf, "...");
	pPath = on_path(path, pRoom);
	
	switch( level )
	{	case 0:
			if(!pRoom->exit[DIR_WEST]  || !pRoom->exit[DIR_NORTH] || IS_SET(pRoom->exit[DIR_NORTH]->exit_info, EX_CLOSED)  )
				buf[0] = '#';
			else
				buf[0] = '.';
			if(!pRoom->exit[DIR_NORTH] )
				buf[1] = '#';
			else
			{	if(IS_SET(pRoom->exit[DIR_NORTH]->exit_info, EX_CLOSED) )
					buf[1] = '_';
				if( pPath && (pPath->to == DIR_NORTH || pPath->from == DIR_NORTH) )
					buf[1] = 'N';
			}
			if(!pRoom->exit[DIR_EAST] || !pRoom->exit[DIR_NORTH] || IS_SET(pRoom->exit[DIR_NORTH]->exit_info, EX_CLOSED)  )
				buf[2] = '#';
			else
				buf[0] = '.';
			if(pRoom->exit[DIR_UP])
			{	buf[2] = '^';
				if( pPath && pPath->to == DIR_UP)
					buf[2] = 'U';
			}
			break;
		case 1:
			if(!pRoom->exit[DIR_WEST])
				buf[0] = '#';
			else
			{	if(IS_SET(pRoom->exit[DIR_WEST]->exit_info, EX_CLOSED) )
					buf[0] = '|';
				if( pPath && (pPath->to == DIR_WEST || pPath->from == DIR_WEST ) )
					buf[0] = 'W';
			}
			if(has_shop(pRoom) )
				buf[1] = '$';
			if(pPath)
			{	if(!pPath->next) //End of the path
					buf[1] = 'X';
				else
				{	if(pPath->to == DIR_UP || pPath->to == DIR_DOWN )
						buf[1] = '/';
					else if( pPath->to == DIR_NORTH || pPath->to == DIR_SOUTH )
						buf[1] = '|';
					else if( pPath->to == DIR_EAST || pPath->to == DIR_WEST )
						buf[1] = '-';
				}
			}
			if(ch->in_room == pRoom )
				buf[1] = '@';

			if(!pRoom->exit[DIR_EAST] )
				buf[2] = '#';
			else
			{	if(IS_SET(pRoom->exit[DIR_EAST]->exit_info, EX_CLOSED) )
					buf[2] = '|';
				if( pPath && (pPath->to == DIR_EAST || pPath->from == DIR_EAST ) )
					buf[2] = 'E';

			}
			break;
		case 2:
			if(!pRoom->exit[DIR_WEST] || !pRoom->exit[DIR_SOUTH] || IS_SET(pRoom->exit[DIR_SOUTH]->exit_info, EX_CLOSED) )
				buf[0] = '#';
			else
				buf[0] = '.';
			if(pRoom->exit[DIR_DOWN] )
			{	buf[0] = 'v';
				if( pPath && pPath->to == DIR_DOWN )
					buf[0] = 'D';
			}
			if(!pRoom->exit[DIR_SOUTH] )
				buf[1] = '#';
			else
			{	if(IS_SET(pRoom->exit[DIR_SOUTH]->exit_info, EX_CLOSED) )
					buf[1] = '_';
				if(pPath && (pPath->to == DIR_SOUTH || pPath->from == DIR_SOUTH) )
					buf[1] = 'S';
			}
			if(!pRoom->exit[DIR_EAST] || !pRoom->exit[DIR_SOUTH] || IS_SET(pRoom->exit[DIR_SOUTH]->exit_info, EX_CLOSED) )
				buf[2] = '#';
			else
				buf[0] = '.';
			break;
		default: return "???";
	};

	return buf;
}
char * get_description_line(int length, char *descr, char *ptr )
{	bool reached = false;

	while ( 1 )
	{	if(*descr == '\0' || reached )
			break;
		if(*descr == '\n' || *descr == '\r' )
		{	if(*descr == '\n') *ptr++ = ' ';
			descr++;
			length++;
			if(length >= 80 )
				break;
			continue;
		}
		*ptr++ = *descr++;
		length++;
		if(length >= 70)
			if(*descr == ' ')
			{	descr++;
				reached = true;
			}
	}
	*ptr = '\0';
	return descr;
}
char *getline( char *str, char *buf );


PathHolder * GetPath(CHAR_DATA *ch, ROOM_INDEX_DATA *start, ROOM_INDEX_DATA *end, int max_steps);
void DetailMapBuild(CHAR_DATA *ch, int x, int y, DetailMapHolder *Map, ROOM_INDEX_DATA *pRoom );
void GenerateDetailMap(CHAR_DATA *ch, char *descr)
{	int x,y, i_y,i;
	DetailMapHolder *DetailMap;
	ROOM_INDEX_DATA *pRoom, *point;
	char buf[MSL], tmp[MSL], line[MSL], *ptr;
	int line_map = 0, line_on = 0;
	PathHolder *path = NULL, *p_next;

	buf[0] = '\0';

	if(IS_NPC(ch) )
		return;


	x = 3;//ch->pcdata->detail_map_x;
	y = 3;//ch->pcdata->detail_map_y;

	pRoom = ch->in_room;

	DetailMap = (DetailMapHolder*)calloc(sizeof(*DetailMap), 1);
	DetailMap->map = (ROOM_INDEX_DATA***)calloc(sizeof(*DetailMap->map), y);
	for(i_y = 0; i_y < y ; ++i_y)
		DetailMap->map[i_y] = (ROOM_INDEX_DATA**)calloc(sizeof(*DetailMap->map[i_y] ), x);
	DetailMap->x = x;
	DetailMap->y = y;
	
	DetailMapBuild(ch, x/2, y/2, DetailMap, pRoom);
	if( (point = ch->pcdata->map_point ) )
		path = GetPath(ch, pRoom, point, 20);

	DetailMap->details = (char**)calloc(sizeof(*DetailMap->details), y * 3);
	for(i_y = 0; i_y < y * 3; ++i_y )
		DetailMap->details[i_y] = (char*)calloc(sizeof(*DetailMap->details[i_y]), x * 3 );
	for(y = 0; y < DetailMap->y * 3; ++y)
	{	for(x = 0; x < DetailMap->x ; ++x )
		{	if( ( pRoom = DetailMap->map[y/3][x])  == NULL )//|| !explored_vnum(ch, pRoom->vnum) )
			{	strcat(buf, "   ");
				continue;
			}
			else
			{	char buf2[4];
				sprintf(buf2, "%s", get_str(ch, pRoom, y % 3, path) );
				switch(buf2[0])
				{	case '#': strcat(buf, "{d#{x"); break;
				case '1':
					strcat(buf,"{d1{x");break;
				case '2':
					strcat(buf,"{d2{x");break;
				case '3':
					strcat(buf,"{d3{x");break;
				case '4':
					strcat(buf,"{d4{x");break;
				case '5':
					strcat(buf,"{d5{x");break;
				case '6':
					strcat(buf,"{d6{x");break;
				case '7':
					strcat(buf,"{d7{x");break;
				case '8':
					strcat(buf,"{d8{x");break;
					case '.': strcat(buf, "{w {x"); break;
					case '|': strcat(buf, "{D|{x"); break;
					case 'v': strcat(buf, "{Wv{x"); break;
					case 'W': strcat(buf, "{R-{x"); break;
					case 'D': strcat(buf, "{Rv{x"); break;
					default: sprintf(tmp, "{R%c{x", buf[0]); strcat(buf,tmp); break;
				}
				switch(buf2[1])
				{	case '#': strcat(buf, "{d#{x"); break;
				case '1':
					strcat(buf,"{d1{x");break;
				case '2':
					strcat(buf,"{d2{x");break;
				case '3':
					strcat(buf,"{d3{x");break;
				case '4':
					strcat(buf,"{d4{x");break;
				case '5':
					strcat(buf,"{d5{x");break;
				case '6':
					strcat(buf,"{d6{x");break;
				case '7':
					strcat(buf,"{d7{x");break;
				case '8':
					strcat(buf,"{d8{x");break;
					case '.': strcat(buf, "{w {x"); break;
					case '@': strcat(buf, "{R@{x"); break;
					case '$': strcat(buf, "{Y${x"); break;
					case '_': strcat(buf, "{D_{x"); break;
					case '/': strcat(buf, "{R/{x"); break;
					case 'X': strcat(buf, "{RX{x"); break;
					case '-': strcat(buf, "{R-{x"); break;
					case '|': strcat(buf, "{R|{x"); break;
					case 'S': case 'N': strcat(buf, "{R|{x"); break;
					default: sprintf(tmp, "{R%c{x", buf[1]); strcat(buf,tmp); break;
				}
				switch(buf2[2])
				{	case '#': strcat(buf, "{d#{x"); break;
				case '1':
					strcat(buf,"{d1{x");break;
				case '2':
					strcat(buf,"{d2{x");break;
				case '3':
					strcat(buf,"{d3{x");break;
				case '4':
					strcat(buf,"{d4{x");break;
				case '5':
					strcat(buf,"{d5{x");break;
				case '6':
					strcat(buf,"{d6{x");break;
				case '7':
					strcat(buf,"{d7{x");break;
				case '8':
					strcat(buf,"{d8{x");break;
					case '.': strcat(buf, "{w {x"); break;
					case '|': strcat(buf, "{D|{x"); break;
					case '^': strcat(buf, "{M^{x"); break;
					case 'E': strcat(buf, "{R-{x"); break;
					case 'U': strcat(buf, "{R^{x"); break;
					default: sprintf(tmp, "{R%c{x", buf[2]); strcat(buf,tmp); break;
				}
			}
		}
		strcat(buf, "\n\r");
	}
	ptr = descr;
	line_map += (DetailMap->y * 3)+2; //plus 2 for % border;
	ptr = buf;
	while( 1 )
	{	line_on++;
		
		if(line_on <= line_map)
		{	if(line_on == 1 || line_on == line_map)
			{	for(i = 0; i < (DetailMap->x * 3) +2; ++i)
					send_to_char("{d-{x",ch);//{[room_name]%{x",ch);
			}
			else
			{	ptr = getline(ptr, line);
				printf_to_char(ch, "{d|{x%s{d|{x",line);//{[room_name]%%{x%s{[room_name]%%{x", line);
			}
		}

		if( *descr != '\0' )
		{	int len = DetailMap->y*3+3;
			if(line_on > line_map )
				len = 0;
			descr = get_description_line(len, descr, line);
			printf_to_char(ch, "  %s%s", sect_table[ch->in_room->sector_type].color,line);
		}
		if( *descr == '\0' && line_on > line_map )
			break;
		else
			send_to_char("\n\r",ch);
	}
	send_to_char("\n\r",ch);
 
	for(i_y = 0; i_y < DetailMap->y ; ++i_y )
		free(DetailMap->map[i_y]);
	free(DetailMap->map);
	for(i_y = 0; i_y < DetailMap->y * 3; ++i_y )
		free(DetailMap->details[i_y]);
	free(DetailMap->details);

	free(DetailMap);
	if(path)
		for(;path ; path = p_next)
		{	p_next = path->next;
			free(path);
		}
}
			 

bool if_mapped(RoomHolder *first, int vnum)
{	RoomHolder *i;
	for(i = first ; i != NULL ; i = i->next )
	{	if(i->pRoom->vnum == vnum)
			return true;
	}
	return false;
}






//This uses BFS like DetailMapBuild but it searches up and down, so the path exists if it goes up or down.
//The walkto pathfinding algorithm in act_move.c on the NW uses recurssion and simply shoots off and maps one direction
//at a time. Using BFS means I'll get the closest location first. No need to store multiple paths and leave the
//system open to infinite loops.
//Going to recycle RoomHolder so we don't have bunch of struct defs. x will be used for steps
PathHolder * GetPath(CHAR_DATA *ch, ROOM_INDEX_DATA *start, ROOM_INDEX_DATA *end, int max_steps)
{	RoomHolder *black_list, *white_list, *i_r, *i_next = NULL, *hold, *i_last;
	EXIT_DATA *pExit;
	ROOM_INDEX_DATA *to_room;
	int dir;
	PathHolder *path_list = NULL, *pPath;

	bool found = false; //If we found the room, no need to map more, just throw everything allocated to the white_list

	white_list = NULL;
	black_list = (RoomHolder*)calloc(sizeof(*black_list), 1);
	black_list->pRoom = start;
	i_last = black_list;

	for(i_r = black_list ; i_r ; i_r = i_next)
	{	if(i_r->pRoom == end )
			break;
		if(i_r->x + 1 <= max_steps && !found )
		for(dir = 0; dir < MAX_DIR ; ++dir)
		{	if( !(pExit = i_r->pRoom->exit[dir] ) )
				continue;
			if( !(to_room = pExit->u1.to_room )
			|| if_mapped(white_list, to_room->vnum )
			|| if_mapped(black_list, to_room->vnum )
			|| to_room == i_r->pRoom )
				continue;
			hold = (RoomHolder*)calloc(sizeof(*hold), 1);
			hold->x = i_r->x+1;
			hold->pRoom = to_room;
			hold->from = i_r;
			hold->dir = rev_dir[dir];
			i_last->next = hold;
			i_last = hold;
		}
		i_next = i_r->next;
		i_r->next = white_list;
		white_list = i_r;
		if(i_r == black_list )
			black_list = i_next;
	}

	if(i_r)//Path was found. Lets build it!
	{	int temp_to = -1;
		for( hold = i_r ; hold ; hold = hold->from )
		{	pPath = (PathHolder*)calloc(1, sizeof(*pPath) );
			pPath->pRoom = hold->pRoom;
			pPath->from = hold->dir;
			pPath->to = temp_to;
			temp_to = rev_dir[pPath->from];
			pPath->next = path_list;
			path_list = pPath;
		}
		path_list->from = -1; //Starting point.
		i_next = NULL;
		for( ; i_r ; i_r = i_next)
		{	i_next = i_r->next;
			i_r->next = white_list;
			white_list = i_r;
		}
	}
	//Free what we've allocated
	for( i_r = white_list ; i_r ; i_r = i_last )
	{	i_last = i_r->next;
		free(i_r);
	}
	return path_list;
}

void DetailMapBuild(CHAR_DATA *ch, int x, int y, DetailMapHolder *Map, ROOM_INDEX_DATA *pRoom )
{	EXIT_DATA *pExit;
	RoomHolder *black_list, *white_list, *i_r,  *i_last, *hold, *i_next;
	ROOM_INDEX_DATA *to_room;
	int dir;

	white_list = NULL;
	black_list = (RoomHolder*)calloc(sizeof(*black_list), 1);
	black_list->x = x;
	black_list->y = y;
	black_list->pRoom = pRoom;
	i_last = black_list;
	
	for(i_r = black_list ; i_r ; i_r = i_next)
	{	Map->map[i_r->y][i_r->x] = i_r->pRoom;	
		

		for( dir = 0 ; dir < MAX_MAP_DIR ; ++dir )
		{	int new_x, new_y;

			if( !(pExit = i_r->pRoom->exit[dir] ) )
				continue;
			if( !(to_room = pExit->u1.to_room )
			|| if_mapped(white_list, to_room->vnum )
			|| if_mapped(black_list, to_room->vnum )
			|| to_room == i_r->pRoom )
				continue;
			new_x = i_r->x + offsets[dir][1];
			new_y = i_r->y + offsets[dir][0];
			if(new_x >= Map->x || new_y >= Map->y || new_x < 0 || new_y < 0 ||  Map->map[new_y][new_x] != 0 )
				continue;
			
			hold = (RoomHolder*)calloc(sizeof(*hold), 1);
			hold->x = new_x;
			hold->y = new_y;
			hold->pRoom = to_room;
			i_last->next = hold;
			i_last = hold;
		}
		i_next = i_r->next;
		i_r->next = white_list;
		white_list = i_r;
		if(i_r == black_list )
			black_list = i_next;
	}

	for( i_r = white_list ; i_r ; i_r = i_last )
	{	i_last = i_r->next;
		free(i_r);
	}

}


void do_test(CHAR_DATA *ch, char *argument )
{
	ROOM_INDEX_DATA *start = ch->in_room;
	RoomHolder *black_list, *white_list, *i_r, *i_next = NULL, *hold, *i_last;
	EXIT_DATA *pExit;
	ROOM_INDEX_DATA *to_room;
	AREA_DATA *pArea;
	char buf[MSL*2], tmp[MSL];;
	int dir;
	int count = 1;

	bool found = false; //If we found the room, no need to map more, just throw everything allocated to the white_list

	white_list = NULL;
	black_list = (RoomHolder*)calloc(sizeof(*black_list), 1);
	black_list->pRoom = start;
	i_last = black_list;

	for(i_r = black_list ; i_r ; i_r = i_next)
	{	for(dir = 0; dir < MAX_DIR ; ++dir)
		{	if( !(pExit = i_r->pRoom->exit[dir] ) )
				continue;
			if( !(to_room = pExit->u1.to_room )
			|| if_mapped(white_list, to_room->vnum )
			|| if_mapped(black_list, to_room->vnum )
			|| to_room == i_r->pRoom )
				continue;
			hold = (RoomHolder*)calloc(sizeof(*hold), 1);
			hold->x = i_r->x+1;
			hold->pRoom = to_room;
			hold->from = i_r;
			hold->dir = rev_dir[dir];
			i_last->next = hold;
			i_last = hold;
			++count;
		}
		i_next = i_r->next;
		i_r->next = white_list;
		white_list = i_r;
		if(i_r == black_list )
			black_list = i_next;
	}

	printf_to_char(ch, "There are %d rooms connected to you!\r\nChecking integrety...\r\n", count);
	for(count = 0, i_r = white_list ; i_r ; i_r = i_r->next, ++count );
	printf_to_char(ch, "%d items mapped\r\n", count);
	
	for(count = 0, i_r = white_list ; i_r ; i_r = i_r->next )
	{	if( if_mapped(i_r->next, i_r->pRoom->vnum ) )
			count++;

		for(pArea = area_first ; pArea ; pArea = pArea->next )
		{	sprintf(tmp, "[%d]", pArea->vnum);
			if(strstr(buf, tmp) )
				continue;
			if(pArea->min_vnum < i_r->pRoom->vnum  && pArea->max_vnum > i_r->pRoom->vnum )
				strcat(buf, tmp);
		}

	}

	for(pArea = area_first ; pArea ; pArea = pArea->next )
	{	sprintf(tmp, "[%d]", pArea->vnum );
		if(!strstr(buf, tmp ) )
			printf_to_char(ch, "%s(%d-%d)\r\n", pArea->name, pArea->min_vnum, pArea->max_vnum );
	}


	printf_to_char(ch, "Possibility of %d duplicates.\r\n",count);
	
	return;
}

void do_mapmax(CHAR_DATA *ch, char *argument )
{	int x,y;
	char arg[MIL], arg2[MIL];
	
	argument = one_argument(argument,arg);
	one_argument(argument, arg2);

	if(arg[0] == '\0' || arg2[0] == '\0' )
	{	//printf_to_char(ch, "Syntax: maxmap <max width> <max height>\r\nYour current width is %d and your height is %d\r\n", ch->pcdata->detail_map_x, ch->pcdata->detail_map_y );
		return;
	}

	if(!is_number(arg) || !is_number(arg2) )
	{	send_to_char("They have to be numbers.\r\n",ch);
		return;
	}
	x = atoi(arg);
	y = atoi(arg2);
	if(x < 1 || y < 1 || x > 100 || y > 100 )
	{	send_to_char("Not possible.\r\n",ch);
		return;
	}
	//ch->pcdata->detail_map_x = x;
	//ch->pcdata->detail_map_y = y;
	printf_to_char(ch, "Your map is now %d wide and %d in height.\r\n", x, y);
	return;
}


