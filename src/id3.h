/*
 * id3 include stuff
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define VERSION_SIZE 8

typedef struct {
	char version[VERSION_SIZE]; //It will never be this large!
	char *title;
	char *artist;
	char *album;
	char *year;
	char *comment;
	char *track;
	char *genre;
	char *buffer;
	char *ptr;
	size_t length;
	size_t size;
	size_t tag_length;
} ID3;

#define GENRE_MAX 148
#define ID3_INIT_SIZE 8192

#define isframeid(a) (isupper(a) || isdigit(a))
#define HUGE_STRING_LEN 8192
#define WATCHPOINT printf("WATCHPOINT %s %d\n", __FILE__, __LINE__);

extern unsigned char *blob;
extern size_t filesize;

ID3 * create_ID3();
int destroy_ID3(ID3 *blob);
int getID3ByFile(ID3 *info, const unsigned char *filename);
int getID3ByBlob(ID3 *info, const unsigned char *blob, size_t blob_length);

#ifdef NDEBUG 
#define debug() fprintf(stderr, "%d: %d\n", __LINE__, rc);
#else
#define debug()
#endif
