#ifndef _STRING_H_
#define _STRING_H_
#include <iostream>

class String
{	private:
		char *string;
		char *pos;
		int len;
		String operator=(String &rhs);
	public:
		String(char *base);
		String();
		void Set(const char *buf);
		void Set(String &buf);
		void Setf(const char *txt, ...);
		void Empty();
		bool IsEmpty();
		int GetLen();
		bool Compare( String &buf );
		bool Compare( const char *buf);
		int Compare( const char *buf[], int max);
		bool preCompare( String &buf );
		bool preCompare( const char *buf);
		void Read(char delim, String &rhs);
		void GetLine(String &rhs);
		void GetArg(String &rhs);
		void Copy(String &rhs);
		void StripSpace();
		void DiceArg(String &rhs);
		bool ValidName();
		char * operator*();
		String &operator+=(const char *add);
		String &operator+=(int i);
		String &operator+=(String &add);
		String &operator+=(char c);
		String &operator+=(double i);
		String &operator<<(int i);
		String &operator<<(const char *add);
		String &operator<<(double i);
		String &operator<<(String &add);
		
		char operator[](int i);

		~String();
		String( const String & rhs );

};
#endif

