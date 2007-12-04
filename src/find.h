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


/*find file in path return the found file*/
/*the return var must freed*/
extern char *find_file_in_dir(char *path, char *file, char *file_full, int not_abs );
/* the return var must be freed*/
extern char *find_lyrics_from_cache(char *playfile);
/* add a new item to cache*/
extern void add_item_to_cache(char *playfile,char *lyricsfile);
