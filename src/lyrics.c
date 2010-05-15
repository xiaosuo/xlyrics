/*  XLyrics by xiaosuo <xiaosuo1@eyou.com>
 *  Homepage gentux.blogchina.com
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> /* the function isdigit*/

#include "lyrics.h"

static struct Song *
init_song(void) /*init the song struct*/
{
    struct Song *song;

    song = malloc(sizeof(struct Song));

    song->head  = NULL;
    return (song);
}

static void  /*add a new line to Song lyrics */
addline(struct Song *song, struct LyricsLine *node)
{
	struct LyricsLine *tar = NULL;
    node->next = NULL;

    if (song->head) 
    {
		for(tar = song->head; tar->next != NULL; tar = tar->next)
			if(node->line_time < tar->next->line_time)
			{
				node->next = tar->next;
				tar->next = node;
				break;
			}
		if(tar->next == NULL)
			tar->next = node;
    }
    else /*if the Song is empty*/
		song->head = node;
}

static char * /*get the line content*/
get_line_body(char *line)
{
    int a = 0;

    while (line[a] != '\0')
    {
	if (line[a] == ']')
	{
	    if (line[a + 1] != '[')
			return (&line[a + 1]);/*return the content after the ] ,but not ][*/
	}
	a++;
    }
	return NULL;
}

struct LyricsLine *  /*get the lyrics line from the buffer ,the buffer begin with "[" end with '\0'*/
get_lyrics_line(char *buffer, struct Song *song, char synced_lyrics)
{
    struct LyricsLine *newll = NULL;
    int current_pos = 0;
    float minutes, seconds;

	while(1)
	{
		if (synced_lyrics && buffer[current_pos] != '[') /* not the begin */
	   		return NULL;

		newll = calloc(1, sizeof(struct LyricsLine));
		if (!newll)
		    return (NULL);
	
		if (synced_lyrics)
		{
		sscanf(&buffer[current_pos], "[%f:%f]", &minutes, &seconds);
		newll->line_time = minutes * 60 + seconds;
		newll->buffer = strdup(get_line_body(buffer));
		addline(song, newll);
	
		while (buffer[current_pos] != ']')
			current_pos ++;
		current_pos ++;
		} else
		{
			newll->line_time = 0;
			newll->buffer = strdup(buffer);
			addline(song, newll);
			break;
		}
	}

    return (newll);
}

struct Song *
read_lyrics_file(char *filename)
{
    struct LyricsLine *line;
    struct Song *song;
    FILE *file;
    char buffer[255];
    char *x;
	int line_n=0;
	char synced_lyrics = strcmp(strrchr(filename,'.'), ".txt");

    file = fopen(filename, "r");
    if (!file)
	return (NULL);

    song = init_song();

    while (fgets(buffer, sizeof(buffer), file))
    {
		if ((x = strchr(buffer, '\r')))/*if the file is windows format */
		    *x = '\0';
		if ((x = strchr(buffer, '\n')))
			*x = '\0';

		for(x = buffer; isblank(*x); x++) ;
	
		if (!synced_lyrics || ((x[0] == '[') && (isdigit(x[1]))))
		{
		    get_lyrics_line(x, song, synced_lyrics);
		}
		else if ((x[0] == '[') && !(isdigit(x[1])))
		{
		    if ((x[1] == 'a') && (x[2] == 'r'))	// artist
			song->artist = strdup(&x[4]);
		    else if ((x[1] == 't') && (x[2] == 'i'))	// title
			song->title = strdup(&x[4]);
		    else if ((x[1] == 'a') && (x[2] == 'l'))	// al
			song->language = strdup(&x[4]);
		    else if ((x[1] == 'b') && (x[2] == 'y'))	// author
			song->author = strdup(&x[4]);
		}
    }
	/* set the line unmber needed by list.c*/
	for(line=song->head; line!=NULL; line=line->next)
		line->line_number=line_n++;	
	fclose(file);
    return (song);
}
void lyrics_cleanup(struct Song* song)
{
	struct LyricsLine *line;

	if(song == NULL)
		return;

	for(line=song->head; line!=NULL; line=song->head)
	{
		song->head = line->next;
		free(line);
	}
	free(song);
}

/*
static char *
show_lyrics(struct Song *song, int time)
{
    struct LyricsLine *line;

    line = song->head;
    while (line)
	{
		if (line->line_time == time)
	   		 return (line->buffer);
		line = line->next;
    }
}

   int main(void)
   {
   struct Song *song;
   int a = 0;

   song = read_lyrics_file("./test_lyrics");
   for( a=0; a<360; a++)
   	show_lyrics(song, a);
   return 0;
   }
   */
