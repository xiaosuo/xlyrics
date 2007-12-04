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



struct LyricsLine
{
    struct LyricsLine *next;

    char *buffer;
    int line_time;
    int line_number;
};

struct Song
{
    struct LyricsLine *head;

    char *author;
    char *artist;
    char *title;
    char *language;
};

struct Song *read_lyrics_file(char *filename);
void lyrics_cleanup(struct Song* song);
