#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "merc.h"
#include "olc.h"
#include "string.h"

char *string_linedel( char *, int );
char *string_lineadd( char *, char *, int );
char *numlineas( char * );
void do_spellcheck(CHAR_DATA*,char*);
void string_spellcheck(CHAR_DATA*,char*,bool);

/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit(CHAR_DATA *ch,char **pString)
{
    send_to_char("-========- Entering EDIT Mode -=========-\n\r",ch);
    send_to_char("    Type .h on a new line for help\n\r",ch);
    send_to_char(" Terminate with a ~ or @ on a blank line.\n\r",ch);
    send_to_char("-=======================================-\n\r",ch);

    if (*pString == NULL)
        *pString = str_dup("");
    else
        **pString = '\0';

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append(CHAR_DATA *ch,char **pString){
    send_to_char("-=======- Entering APPEND Mode -========-\n\r",ch);
    send_to_char("    Type .h on a new line for help\n\r",ch);
    send_to_char(" Terminate with a ~ or @ on a blank line.\n\r",ch);
    send_to_char("-=======================================-\n\r",ch);

    if (*pString == NULL)
        *pString = str_dup("");
    send_to_char( numlineas(*pString), ch );

/* numlineas entrega el string con \n\r */
/*  if ( *(*pString + strlen_color( *pString ) - 1) != '\r' )
	send_to_char( "\n\r", ch ); */

    ch->desc->pString = pString;

    return;
}

/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_replace(char * orig,char * old,char * ne)
{
    char xbuf[MSL];
    int i;

    xbuf[0] = '\0';
    strcpy(xbuf,orig);
    if (strstr(orig,old) != NULL)
    {
        i = strlen_color(orig) - strlen_color(strstr(orig,old));
        xbuf[i] = '\0';
        strcat(xbuf,ne);
        strcat(xbuf,&orig[i+strlen_color(old)]);
        free_string(orig);
    }

    return str_dup(xbuf);
}

/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add(CHAR_DATA *ch,char *argument)
{
    char buf[MSL],doop[MIL];

    /*
     * Thanks to James Seng
     */
    smash_tilde(argument);

    if (*argument == '.')
    {
        char arg1[MIL], arg2[MIL], arg3[MIL], tmparg3[MIL];
		char *buf;

        argument = one_argument(argument,arg1);
		buf = str_dup(argument);
        argument = first_arg(argument,arg2,false);
		strcpy(tmparg3,argument);
        argument = first_arg(argument,arg3,false);

        if (!str_cmp(arg1,".c") || !str_cmp(arg1,".clear"))
        {
            send_to_char("String cleared.\n\r",ch);
			free_string(*ch->desc->pString);
			*ch->desc->pString = str_dup("");
            return;
        }

        if (!str_cmp(arg1,".s") || !str_cmp(arg1,".show"))
        {
            send_to_char("String so far:\n\r",ch);
            send_to_char(numlineas(*ch->desc->pString),ch);
            return;
        }

        if (!str_cmp(arg1,".r") || !str_cmp(arg1,".replace"))
        {
            if (arg2[0] == '\0')
            {
                send_to_char("usage:  .r \"old string\" \"new string\"\n\r",ch);
                return;
            }

            *ch->desc->pString = string_replace(*ch->desc->pString,arg2,arg3);
            printf_to_char(ch,"'%s' replaced with '%s'.\n\r",arg2,arg3);
            return;
        }

        if (!str_cmp(arg1,".f") || !str_cmp(arg1,".format"))
        {
            *ch->desc->pString = format_string(*ch->desc->pString);
            send_to_char("String formatted.\n\r",ch);
            return;
        }
        
		if (!str_cmp(arg1,".ld") || !str_cmp(arg1,".linedelete"))
		{
			*ch->desc->pString = string_linedel(*ch->desc->pString,atoi(arg2));
			send_to_char("Line removed.\n\r",ch);
			return;
		}

		if (!str_cmp(arg1,".li") || !str_cmp(arg1,".lineinsert"))
		{
			*ch->desc->pString = string_lineadd(*ch->desc->pString,tmparg3,atoi(arg2));
			send_to_char("Line inserted.\n\r",ch);
			return;
		}

		if (!str_cmp(arg1,".lr") || !str_cmp(arg1,".linereplace"))
		{
			*ch->desc->pString = string_linedel( *ch->desc->pString, atoi(arg2) );
			*ch->desc->pString = string_lineadd( *ch->desc->pString, tmparg3, atoi(arg2) );
			send_to_char( "Line replaced.\n\r", ch );
			return;
		}

		if (!str_cmp(arg1,".^"))
		{
			one_argument(buf,doop);
			if(str_prefix(doop,"gossip") && str_prefix(doop,"say") && str_prefix(doop,"chat") && str_prefix(doop,"immtalk")
				&& str_prefix(doop,":") && str_prefix(doop,"gossip") && str_prefix(doop,";") && str_prefix(doop,".") && str_prefix(doop,"gecho")){
				ch->send("You can only use this command for speech.\n\r");
			}
			else
				interpret(ch,buf);
			return;
		}
		if (!str_cmp(arg1,".spellcheck"))
		{
			if (arg2[0])
				do_spellcheck(ch,arg2);
			else
				string_spellcheck(ch,*ch->desc->pString,false);
			return;
		}
        if (!str_cmp(arg1,".h") || !str_cmp(arg1,".help"))
        {
            send_to_char("Sedit help (commands on blank line):   \n\r",ch);
            send_to_char(".r 'old' 'new'   - replace a substring \n\r",ch);
            send_to_char("                   (requires '', \"\") \n\r",ch);
            send_to_char(".h               - get help (this info)\n\r",ch);
            send_to_char(".s               - show string so far  \n\r",ch);
            send_to_char(".f               - (word wrap) string  \n\r",ch);
            send_to_char(".c               - clear string so far \n\r",ch);
            send_to_char(".ld <num>        - remove line number <num>\n\r",ch);
            send_to_char(".li <num> <str>  - insert <str> in line <num>\n\r",ch);
		    send_to_char(".lr <num> <str>  - replace line <num> with <str>\n\r",ch);
			send_to_char(".spellcheck      - spellcheck the string\r\n",ch);
			send_to_char(".spellcheck <w>  - spellcheck a word\r\n",ch);
            send_to_char("@                - end string          \n\r",ch);
            return;
        }

        send_to_char("SEdit:  Invalid dot command.\n\r",ch);
        return;
    }

    if (*argument == '~' || *argument == '@')
    {
		if (ch->desc->editor == ED_MPCODE) /* para los mobprogs */
		{
			MOB_INDEX_DATA *mob;
			int hash;
			PROG_LIST *mpl;
			PROG_CODE *mpc;

			EDIT_MPCODE(ch, mpc);

			if (mpc != NULL)
				for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
					for ( mob = mob_index_hash[hash]; mob; mob = mob->next )
						for ( mpl = mob->mprogs; mpl; mpl = mpl->next )
							if (mpl->vnum == mpc->vnum)
							{
								printf_to_char(ch,"Updating mob %d.\n\r",mob->vnum);
								mpl->code = mpc->code;
							}
		}
		if (ch->desc->editor == ED_OPCODE) /* for the objprogs */
		{
			OBJ_INDEX_DATA *obj;
			int hash;
			PROG_LIST *opl;
			PROG_CODE *opc;

			EDIT_OPCODE(ch, opc);

			if (opc != NULL)
				for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
					for ( obj = obj_index_hash[hash]; obj; obj = obj->next )
						for ( opl = obj->oprogs; opl; opl = opl->next )
							if ( opl->vnum == opc->vnum )
							{
								printf_to_char(ch,"Fixing object %d.\n\r",obj->vnum);
								opl->code = opc->code;
							}
		}

		if (ch->desc->editor == ED_RPCODE) /* for the roomprogs */
		{
			ROOM_INDEX_DATA *room;
			int hash;
			PROG_LIST *rpl;
			PROG_CODE *rpc;

			EDIT_RPCODE(ch, rpc);

			if (rpc != NULL)
				for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
					for ( room = room_index_hash[hash]; room; room = room->next )
						for ( rpl = room->rprogs; rpl; rpl = rpl->next )
							if (rpl->vnum == rpc->vnum)
							{
								printf_to_char(ch,"Fixing room %d.\n\r",room->vnum);
								rpl->code = rpc->code;
							}
		}
        ch->desc->pString = NULL;
        return;
    }

    strcpy( buf, *ch->desc->pString );

    /*  Truncate strings to MSL.*/
    if (strlen_color(buf) + strlen_color(argument) >= (MSL - 4))
    {
        send_to_char("String too long, last line skipped.\n\r",ch);

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /* Ensure no tilde's inside string.*/
    smash_tilde(argument);

    strcat(buf,argument);
    strcat(buf,"\n\r");
    free_string(*ch->desc->pString);
    *ch->desc->pString = str_dup(buf);
    return;
}

char *format_string(char *oldstring /*, bool fSpace */)
{
	char xbuf[MSL], xbuf2[MSL], *rdesc;
	int i=0;
	bool cap = true;

	xbuf[0] = xbuf2[0]=0;

	i = 0;

	for (rdesc = oldstring; *rdesc; rdesc++)
	{
		if (*rdesc == '\n')
		{
			if (xbuf[i-1] != ' ')
			{
				xbuf[i] = ' ';
				i++;
			}
		}
		else if (*rdesc == '\r')
			;
		else if (*rdesc == ' ')
		{
			if (xbuf[i-1] != ' ')
			{
				xbuf[i] = ' ';
				i++;
			}
		}
		else if (*rdesc == ')')
		{
			if (xbuf[i-1] == ' ' && xbuf[i-2] == ' ' && (xbuf[i-3] == '.' || xbuf[i-3] == '?' || xbuf[i-3] == '!'))
			{
				xbuf[i-2] = *rdesc;
				xbuf[i-1] = ' ';
				xbuf[i] = ' ';
				i++;
			}
			else
			{
				xbuf[i]=*rdesc;
				i++;
			}
		}
		else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')
		{
			if (xbuf[i-1] == ' ' && xbuf[i-2] == ' ' && (xbuf[i-3] == '.' || xbuf[i-3] == '?' || xbuf[i-3] == '!'))
			{
				xbuf[i-2] = *rdesc;
				if (*(rdesc+1) != '\"')
				{
					xbuf[i-1] = ' ';
					xbuf[i] = ' ';
					i++;
				}
				else
				{
					xbuf[i-1] = '\"';
					xbuf[i] = ' ';
					xbuf[i+1] = ' ';
					i += 2;
					rdesc++;
				}
			}
			else
			{
				xbuf[i] =* rdesc;
				if (*(rdesc+1) != '\"')
				{
					xbuf[i+1] = ' ';
					xbuf[i+2] = ' ';
					i += 3;
				}
				else
				{
					xbuf[i+1] = '\"';
					xbuf[i+2] = ' ';
					xbuf[i+3] = ' ';
					i += 4;
					rdesc++;
				}
			}
			cap = true;
		}
		else
		{
			xbuf[i] =* rdesc;
			if (cap)
			{
				cap = false;
				xbuf[i] = UPPER(xbuf[i]);
			}
			i++;
		}
	}
	xbuf[i] = 0;
	strcpy(xbuf2,xbuf);

	rdesc=xbuf2;

	xbuf[0] = 0;

	for ( ; ; )
	{
		for (i = 0; i < 77; i++)
			if (!*(rdesc+i))
				break;

		if (i < 77)
			break;

		for (i = (xbuf[0]?76:73) ; i; i--)
			if (*(rdesc+i)==' ')
				break;

		if (i)
		{
			*(rdesc+i)=0;
			strcat(xbuf,rdesc);
			strcat(xbuf,"\n\r");
			rdesc += i+1;
			while (*rdesc == ' ')
				rdesc++;
		}
		else
		{
			bug("format_string: No spaces", 0);
			*(rdesc+75) = 0;
			strcat(xbuf,rdesc);
			strcat(xbuf,"-\n\r");
			rdesc += 76;
		}
	}
	while (*(rdesc+i) && (*(rdesc+i)==' ' || *(rdesc+i)=='\n' || *(rdesc+i)=='\r'))
		i--;
	*(rdesc+i+1)=0;
	strcat(xbuf,rdesc);
	if (xbuf[strlen_color(xbuf)-2] != '\n')
		strcat(xbuf,"\n\r");
	free_string(oldstring);

	return (str_dup(xbuf));
}

char *first_arg(char *argument,char *arg_first,bool fCase)
{
	char cEnd;

	while ( *argument == ' ' )
		argument++;

	cEnd = ' ';
	if ( *argument == '\'' || *argument == '"' || *argument == '%' || *argument == '(' )
	{
		if (*argument == '(')
		{
			cEnd = ')';
			argument++;
		}
		else
			cEnd = *argument++;
	}

	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		if (fCase)
			*arg_first = LOWER(*argument);
		else
			*arg_first = *argument;
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while ( *argument == ' ' )
		argument++;

	return argument;
}

/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad(char * argument)
{
    char buf[MSL], *s;

    s = argument;

    while ( *s == ' ' )
        s++;

    strcpy(buf,s);
    s = buf;

    if (*s != '\0')
    {
        while (*s != '\0')
            s++;
        s--;

        while(*s == ' ')
            s--;
        s++;
        *s = '\0';
    }

    free_string(argument);
    return str_dup(buf);
}

/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper(char * argument)
{
    char *s;

    s = argument;

    while (*s != '\0')
    {
        if (*s != ' ')
        {
            *s = UPPER(*s);
            while (*s != ' ' && *s != '\0')
                s++;
        }
        else
            s++;
    }

    return argument;
}

char *string_linedel(char *string,int line)
{
	char *strtmp = string, buf[MSL];
	int cnt = 1, tmp = 0;

	buf[0] = '\0';

	for ( ; *strtmp != '\0'; strtmp++ )
	{
		if (cnt != line)
			buf[tmp++] = *strtmp;

		if (*strtmp == '\n')
		{
			if (*(strtmp + 1) == '\r')
			{
				if (cnt != line)
					buf[tmp++] = *(++strtmp);
				else
					++strtmp;
			}

			cnt++;
		}
	}

	buf[tmp] = '\0';

	free_string(string);
	return str_dup(buf);
}

char *string_lineadd(char *string,char *newstr,int line)
{
	char buf[MSL], *strtmp = string;
	int cnt = 1, tmp = 0;
	bool done = false;

	buf[0] = '\0';

	for ( ; *strtmp != '\0' || (!done && cnt == line); strtmp++ )
	{
		if (cnt == line && !done)
		{
			strcat(buf,newstr);
			strcat(buf,"\n\r");
			tmp += strlen_color(newstr) + 2;
			cnt++;
			done = true;
		}

		buf[tmp++] = *strtmp;

		if (done && *strtmp == '\0')
			break;

		if (*strtmp == '\n')
		{
			if (*(strtmp + 1) == '\r')
				buf[tmp++] = *(++strtmp);

			cnt++;
		}

		buf[tmp] = '\0';
	}

	free_string(string);
	return str_dup(buf);
}

/* buf queda con la linea sin \n\r */
char *getline(char *str,char *buf)
{
	int tmp = 0;
	bool found = false;

	while (*str)
	{
		if (*str == '\n')
		{
			found = true;
			break;
		}

		buf[tmp++] = *(str++);
	}

	if (found)
	{
		if (*(str + 1) == '\r')
			str += 2;
		else
			str += 1;
	} /* para que quedemos en el inicio de la prox linea */

	buf[tmp] = '\0';

	return str;
}

char *numlineas(char *string)
{
	int cnt = 1;
	static char buf[MSL*2];
	char buf2[MSL], tmpb[MSL];

	buf[0] = '\0';

	while (*string){
		string = getline(string,tmpb);
		sprintf(buf2,"%2d. %s\n\r",cnt++,tmpb);
		strcat(buf,buf2);
	}

	return buf;
}

bool s_prefix(string stra,string strb){
	int lena = stra.length(),lenb = strb.length();
	string bufa,bufb;

	bufa = lena < lenb || lena == lenb ? stra : stra.substr(0,lenb);
	bufb = lena > lenb || lena == lenb ? strb : strb.substr(0,lena);

	if (bufa == bufb)
		return true;
	return false;
}

bool s_exact(string stra,string strb){
	if (stra.length() !=  strb.length())
		return false;
	if (stra == strb)
		return true;
	return false;
}

char *strip_color(char *buf){
}

//Davion's String Stuff!

String::String(char *base):
string(0),
pos(0),
len(0)
{	string = new char[strlen(base)+1];
	pos = string;
	sprintf(string, "%s", base);
	len = strlen(string);
}

String::String():
string(0),
pos(0),
len(0)
{
}
void String::Set(const char *buf)
{	if(len != 0 )
	{	delete [] string;
		string = NULL;
		len = 0;
	}
	string = new char[strlen(buf)+1];
	pos = string;
	sprintf(string, "%s", buf);
	len = strlen(string);
}
void String::Set(String &buf)
{	if(len != 0 )
	{	delete [] string;
		string = NULL;
		len = 0;
	}
	string = new char[strlen(*buf)+1];
	pos = string;
	sprintf(string, "%s", *buf);
	len = strlen(string);
}

void String::Setf(const char *txt, ...)
{	char buf [10000];
	va_list args;
	va_start (args, txt);
	vsprintf (buf, txt, args);
	va_end (args);
	Set(buf);
}

void String::Empty()
{	if(len == 0 )
		return;
	delete [] string;
	string = NULL;
	len = 0;
	pos = NULL;
}

bool String::IsEmpty()
{	if(!len)
		return true;
	return false;
}
int String::GetLen() {	return len; }
bool String::Compare( String &buf )
{	if(!strcasecmp(*(*this), *buf) )
		return true;
	return false;
}
bool String::Compare( const char *buf)
{	if(!strcasecmp(*(*this), buf) )
		return true;
	return false;
}
int String::Compare( const char *buf[], int max)
{	for( int i = 0; i < max ; ++i )
	{	if(Compare(buf[i]) )
			return i;
	}
	return -1;
}

bool String::preCompare( const char *buf )
{	if(!strncasecmp(buf, *(*this), strlen(buf) ) )
		return true;
	return false;
}

bool String::preCompare( String &buf )
{	if( !strncasecmp(*buf, *(*this), strlen(*buf) ) )
		return true;
	return false;
}

void String::Read(char delim, String &rhs)
{	while(*pos != '\0' && *pos != delim && *pos != '\r' && *pos != '\n' )
	{	rhs += *pos;
		++pos;
	}
	if(*pos == '\n' || *pos == '\r' )
	{	while(*pos == '\n' || *pos == '\r')
			++pos;
	}
	if(*pos != '\0')
		++pos;	//Remove delimiter
}

void String::GetLine(String &rhs)
{	rhs.Empty();
	while(*pos != '\0')
	{	if(*pos == '\r' || *pos == '\n')
		{	if(*(pos+1) == '\r' || *(pos+1) == '\n')
				pos+= 2;
			break;
		}
		rhs += *pos++;
	}
	return;
}

void String::GetArg(String &rhs)
{	char delim;
	if(*pos == '\'')
		delim = '\'';
	else if( *pos == '"' )
		delim = '"';
	else
		delim = ' ';
	Read(delim, rhs);
}
void String::Copy(String &rhs)
{	Empty();
	Set(rhs.string);
}

void String::StripSpace()
{	while(isspace(*pos) ) ++pos;
	return;
}

void String::DiceArg(String &rhs)
{	GetArg(rhs);
	const int poo = strlen(pos)+1;
	char buf[poo];
	sprintf(buf, "%s", pos);
	Set(buf);
}

bool String::ValidName()
{	char *tracker = pos;
	while(*tracker != '\0')
	{	if(isspace(*tracker) ) return false;
		if(!isalpha(*tracker) ) return false;
		tracker++;
	}
	return true;
}

//Overloaded Operators
char * String::operator*()
{	return pos;
}
//Root of all evil! :D
String &String::operator+=(const char *add)
{	if(len == 0)
	{	Set(add);
		return *this;
	}
	char buf[len+strlen(add)+10];
	sprintf(buf, "%s%s", pos, add);
	delete [] string;
	len = 0;
	string = NULL;
	string = new char[strlen(buf)+10];
	pos = string;
	sprintf(string, "%s", buf);
	len = strlen(string);
	return *this;
}

String &String::operator+=(int i)
{	char buf[12];
	sprintf(buf, "%d", i);
	*this += buf;
	return *this;
}
String &String::operator+=(double i)
{	char buf[12];
	sprintf(buf, "%.2f", i);
	*this += buf;
	return *this;
}
String &String::operator+=(String &add)
{	*this += *add;
	return *this;
}
String &String::operator+=(char c)
{	char buf[2];
	sprintf(buf, "%c", c);
	*this += buf;
	return *this;
}
String &String::operator<<(int i)
{	*this += i;
	return *this;
}
String &String::operator<<(double i)
{	*this += i;
	return *this;
}
String &String::operator<<(const char *add)
{	*this += add;
	return *this;
}

String &String::operator<<(String &add)
{	*this += add;
	return *this;
}

char String::operator[](int i)
{	if(i > len)
		return '\0';
	if(!pos) return '\0';
	return pos[i];
}

String::~String()
{	if( len != 0 )
	{	delete [] string;
		string = NULL;
	}

	len = 0;
}
String::String( const String & rhs )
{       string = new char[rhs.len+1];
	sprintf(string, "%s", rhs.string);
	pos = string;
	len = rhs.len;
}

char* string_pad(char *string, int nWidth, bool Right_Align)
{
char buf[MSL];
char buf2[MSL];
int x = 0;
int count = 0;

if(string == NULL)
return "";

for( x = 0 ; ; x++ )
{
if( string[x] == '\0')
{
break;
}
else if( string[x] == '{')
{
x++;
if(string[x] == '{')
{
count++;
}
}
else
{
count++;
}
}
strcpy(buf2,"");

for( x = 0; x <= nWidth - count; x++)
strcat(buf2," ");

if(Right_Align)
{
strcpy(buf,buf2);
strcat(buf,string);
}
else
{
strcpy(buf,string);
strcat(buf,buf2);
}
return str_dup(buf);
}
