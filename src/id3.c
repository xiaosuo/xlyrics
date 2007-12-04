/*
 **  id3.c
 **  $Id: id3.c,v 1.2 2006/05/05 12:08:57 xiaosuo Exp $
 **  This is something I really did not want to write.
 **
 **  It was pretty annoying that I had to write this
 **  but I could not find one single ID3 library
 **  that had a BSD license and was written in C.
 */ 

#include "id3.h"
#include <string.h>
#include <wchar.h>
unsigned char *blob;
size_t filesize;

const char *id3_genres[GENRE_MAX] =
{
	"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop",
	"Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "R&B", "Rap", "Reggae",
	"Rock", "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack",
	"Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion", "Trance", "Classical",
	"Instrumental", "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise", "Alt",
	"Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop",
	"Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic",
	"Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult",
	"Gangsta Rap", "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American",
	"Cabaret", "New Wave", "Psychedelic", "Rave", "Showtunes", "Trailer", "Lo-Fi", "Tribal",
	"Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock", "Folk",
	"Folk/Rock", "National Folk", "Swing", "Fast-Fusion", "Bebob", "Latin", "Revival",
	"Celtic", "Bluegrass", "Avantgarde", "Gothic Rock", "Progressive Rock",
	"Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band", "Chorus", "Easy Listening",
	"Acoustic", "Humour", "Speech", "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony",
	"Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango",
	"Samba", "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
	"Punk Rock", "Drum Solo", "A Cappella", "Euro-House", "Dance Hall", "Goa",
	"Drum & Bass", "Club-House", "Hardcore", "Terror", "Indie", "BritPop", "Negerpunk",
	"Polsk Punk", "Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal", "Crossover",
	"Contemporary Christian", "Christian Rock", "Merengue", "Salsa", "Thrash Metal",
	"Anime", "JPop", "Synthpop"
};

void print_char (const unsigned char *foo, int length)
{
	int x=0;

	printf("String %d\n", length);
	for (x; x < length; x++ ) {
		printf("%d: %c:%d\n", x, foo[x], foo[x]);
	}
	putchar('\n');
}

const char * genre_string(int genre)
{
	if(genre < GENRE_MAX && genre > -1)
		return id3_genres[genre];

	return NULL;
}

void clean_string(unsigned char *string, int length)
{
	unsigned char *ptr = string;

	for(; (ptr - string) < length && *ptr; ptr++); 
	if(ptr - string == length){
		*(char*)(string) = '\0';
		return;
	}
	size_t len = length - (ptr - string) - 1;
	memmove(string, ptr+1, len);
	*(char *)(string + len) = '\0';
}

char * add_tag(ID3 *info, const unsigned char *tag, size_t length)
{
	char *begin = info->ptr;

	if (length > info->size) {
		if ( length > info->tag_length  ) {
			info->tag_length = length + 1;
			info->buffer = realloc(info->buffer, info->tag_length);
			info->ptr = info->buffer + info->length;
			info->size = info->tag_length;
		} else {
			//This should never, ever happen
			return NULL;
		}
	}
	memcpy(info->ptr, tag, length);	
	clean_string(info->ptr, length);
	info->length += length + 1;
	info->ptr += length;
	*info->ptr++ = '\0';

	// Now return the position of the last string
	return begin;
}

size_t id3_size(const unsigned char* buffer)
{
	size_t size  = 0;
	size =  (buffer[3] & 0x7f) + 
		((buffer[2] & 0x7f) << 7) +
		((buffer[1] & 0x7f) << 14) +
		((buffer[0] & 0x7f) << 21);

	return size;
}

size_t id3_size2(const unsigned char * buffer)
{
	size_t size  = 0;
	size =  (buffer[2] & 0x7f) + 
		((buffer[1] & 0x7f) << 7) +
		((buffer[0] & 0x7f) << 14);

	return size;
}

size_t get_framesize(const char *buffer)
{
	return ((buffer[6] << 8) + (buffer[7]));
}

int get_id3v1_tag (ID3 *info, const unsigned char *blob, size_t blob_length)
{
	const unsigned char *ptr_buffer = blob;
	const char *genre = NULL;

	ptr_buffer += (blob_length - 128);

	if(!strncmp(ptr_buffer, "TAG", 3)) {
		/* Paranoid, not all systems are made equally */
		ptr_buffer +=3;

		info->title = add_tag(info, ptr_buffer, 30);
		ptr_buffer +=30;

		info->artist = add_tag(info, ptr_buffer, 30);
		ptr_buffer +=30;

		info->album = add_tag(info, ptr_buffer, 30);
		ptr_buffer +=30;

		info->year = add_tag(info, ptr_buffer, 4);
		ptr_buffer +=4;

		info->comment = add_tag(info, ptr_buffer, 30);
		ptr_buffer +=30;

		genre = genre_string(ptr_buffer[0]);
		if (genre)
			info->genre = add_tag(info, genre, strlen(genre));

		return 0;
	}

	return 1;
}

int id_2_2(ID3 *info, const unsigned char *blob)
{
	const unsigned char *ptr = blob;

#ifdef NDEBUG
	printf("BEGIN %d : %d \n", info->tag_length, (ptr - blob));
#endif
	while (info->tag_length > (ptr - blob)) {
		size_t size = id3_size2(ptr+3);
		if(size <= 0) break;

		if (size + (ptr - blob) > info->tag_length) 
			break;
		/*
		printf("TAG %s\n", ptr);
		printf("String %s\n", ptr + 6 + 1);
		printf("Size %d\n", size);
		printf("OFF_SET %d \n ", off_set);
		*/
		if(!strncmp("TP1", ptr, 3)){
			info->artist = add_tag(info, ptr + 6, size);
		} else if (!strncmp("TT2", ptr, 3)) {
			info->title = add_tag(info, ptr + 6, size);
		} else if (!strncmp("TAL", ptr, 3)) {
			info->album = add_tag(info, ptr + 6, size);
		} else if (!strncmp("TRK", ptr, 3)) {
			info->track = add_tag(info, ptr + 6, size);
		} else if (!strncmp("TYE", ptr, 3)) {
			info->year = add_tag(info, ptr + 6, size);
		} else if (!strncmp("COM", ptr, 3)) {
			info->comment = add_tag(info, ptr + 6, size);
		} else if (!strncmp("TCO", ptr, 3)) {
			info->genre = add_tag(info, ptr + 6, size);
		}
		ptr += size + 6;
	}

	return 0;
}

int id_2_3(ID3 *info, const char *blob)
{
	//size_t processedsize = 0;
	const char *ptr = blob;

#ifdef NDEBUG
	printf("BEGIN %d : %d \n", info->tag_length, (ptr - blob));
#endif
	while (info->tag_length > (ptr - blob)) {
		size_t size = get_framesize(ptr);
		if(size <= 0) break;

		if (size + (ptr - blob) > info->tag_length) 
			break;

#ifdef NDEBUG
			printf("SIZE %d : %0.4s : %s \n", size, ptr, ptr + 11);
#endif
		if (!strncmp("TPE1",ptr,4)) {
			info->artist = add_tag(info, ptr +10, size);
		} else if (!strncmp("TIT2",ptr,4)) {
			info->title = add_tag(info, ptr +10, size);
		} else if (!strncmp("TALB",ptr,4)) {
			info->album = add_tag(info, ptr +10, size);
		} else if (!strncmp("TRCK",ptr,4)) {
			//info->track = add_tag(info, ptr +10, size);
		} else if (!strncmp("TYER",ptr,4)) {
			info->year = add_tag(info, ptr +10, size);
		} else if (!strncmp("COMM",ptr,4)) {
			// Was originally 14
			info->comment = add_tag(info, ptr +10, size);
		} else if (!strncmp("TCON",ptr,4)) {
		}
		ptr += 10 + size;
		//processedsize += 10 + size;
	}

	return 0;
}

int process_extended_header(ID3 *info, const char *blob)
{
	unsigned long CRC = 0;
	unsigned long paddingsize = 0;

	/*Shortcut. The extended header size will be either 6 or 10 bytes. 
	  If it's ten bytes, it means that there's CRC data (though we check
	  the flag anyway). I'm gonna save it, though I'll be damned if I 
	  know what to do with it.*/
	if ((blob[3] == 0x0A) && (blob[4])) {
		CRC = (blob[10] << 24) + (blob[11] << 16) +
			(blob[12] << 8) + (blob[13]);
	}

	paddingsize = (blob[6] << 24) + (blob[7] << 16) +
		(blob[8] << 8) + (blob[9]);

	/*subtract the size of the padding from the size of the tag */
	info->tag_length -= paddingsize;

	/*continue decoding the frames */
	return id_2_3(info, blob);
}

int get_id3v2_tag (ID3 *info, const unsigned char *blob, size_t blob_length)
{
	int unsynchronized = 0;
	int hasExtendedHeader = 0;
	int experimental = 0;
	const char *ptr_buffer = blob;

	if(!strncmp(ptr_buffer, "ID3", 3)) {
		info->tag_length = id3_size(ptr_buffer+6);
#ifdef NDEBUG
		printf("TAG SIZE %d\n", info->tag_length);
#endif
		snprintf(info->version, VERSION_SIZE, "%d.%d", blob[3], blob[4]);

		if ((blob[5] & 0x40) >> 6) {
			hasExtendedHeader = 1;
		}
		if ((blob[5] & 0x80) >> 7) {
			unsynchronized = 1;
		}

#ifdef UNUSED
		/*Present, but not very useful*/
		if ((blob[5] & 0x20) >> 5) {
			experimental = 1;
		}

		if (unsynchronized) {
			register int i,j; /*index*/
			/*Replace every instance of '0xFF 0x00'
			  with '0xFF'*/
			for(i=0; i < tagsize; i++) {
				if (buffer[i] == 0xFF && buffer[i+1] == 0x00) {
					for(j=i+1; i < tagsize; i++) {
						buffer[j] = buffer[j+1];
					}
				}
			}
		}
#endif

		/* Move past the above */
		ptr_buffer += 10;
		/*If the tag has an extended header, parse it*/
		if (hasExtendedHeader) {
			return process_extended_header(info, blob);
		} else if (blob[3] == 2) {
			return id_2_2(info, ptr_buffer);
		} else if (blob[3] == 3) {
			return id_2_3(info, ptr_buffer);
		}

	}

	return 1;
}

int getID3ByFile(ID3 *info, const unsigned char *filename)
{
	int rc;
	struct stat buf;
	int fd = -1;

	if(stat(filename, &buf)) {
		return -1;
	}

	if((fd = open(filename, O_RDONLY)) ==  -1) {
		return -1;
	}

	lseek(fd, 0, SEEK_SET);
	filesize = buf.st_size;
	blob = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if((blob ==(void *)-1)) {
		fprintf(stderr, "Errno %s(%d)\n",strerror(errno), errno);
		return -1;
	}

	close(fd);
	rc = getID3ByBlob(info, blob, filesize);
	return rc;
}

int getID3ByBlob(ID3 *info, const unsigned char *blob, size_t blob_length)
{
	int rc = -1;

	// Sometimes people encode both headers, so we test for both

	/*	if ((blob_length > 128) && !get_id3v1_tag(info, blob, blob_length)) {
		sprintf(info->version, "1");
		rc = 0;
		goto out;
		}
		*/

	if (!get_id3v2_tag(info, blob, blob_length))
		rc = 0;

out:
	return rc;
}

int destroy_ID3(ID3 *info)
{
	free(info->buffer);
	free(info);

	return 0; //Eventually we should do more here
}

ID3 * create_ID3(void)
{
	//At the moment ID3 is just a structure
	ID3 *info = (ID3 *)malloc(sizeof(ID3));
	if (info)
		memset(info, 0, sizeof(ID3));
	else
		return NULL;

	info->buffer = (char *)malloc(ID3_INIT_SIZE);
	info->size = ID3_INIT_SIZE;
	info->ptr = info->buffer;

	return info;
}
