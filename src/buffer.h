/*** buffer.h ****/

#define BUFFER_DEBUG

typedef struct buffer_type BUFFER2;

struct buffer_type
{
	char *data; /* The data */
	
	int len;	/* The current len of the buffer */
	int size;	/* The allocated size of data */
	
	bool overflowed; /* Is the buffer overflowed? */
};

#ifdef BUFFER_DEBUG /* Debugged version */

#define buffer_new(size)           __buffer_new (size, __FILE__, __LINE__)
#define buffer_strcat(buffer,text) __buffer_strcat (buffer, text, __FILE__, __LINE__)

BUFFER2 * __buffer_new (int size, const char *file, unsigned line);
void __buffer_strcat (BUFFER2 *buffer, const char *text, const char *file, unsigned line);

#else  /* not debugged version */

#define buffer_new(size)           __buffer_new (size)
#define buffer_strcat(buffer,text) __buffer_strcat (buffer,text)

BUFFER2 * __buffer_new (int size);
void __buffer_strcat (BUFFER2 *buffer, const char *text);

#endif


void buffer_free (BUFFER2 *buffer);
void buffer_clear (BUFFER2 *buffer);
int find_mem_size (int min_size);
int bprintf (BUFFER2 *buffer, char *fmt, ...);




