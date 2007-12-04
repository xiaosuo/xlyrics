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

#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<stdlib.h>
#include<glib.h>
#include<stdio.h>

#include"find.h"
#include"id3.h"

/* get the content after " - "*/
/* the return var must be freed*/
char *extract_title(const char *s)
{
	char *c = NULL, *ss;

	ss = (char*)strdup(s);
	if((c = strrchr(ss, '-')) && *(c+1) == ' ' && *(c+2))
	{
		int len = strlen(c+2);
		strncpy(ss, c+2, len);
		*(ss + len) = '\0';
	}
	return ss;
}
/* remove the .ext(.mp3) from the file */
/* the return var must be freed*/
char *remove_ext(const char *file) 
{
	char *pos, *ret;

	ret = (char*)strdup(file);
	if((pos = strrchr(ret, '.')))
		*pos = '\0';

	return ret;
}

/* See comment before dofuzzystrcmp() above. */
int fuzzystrcmp(const char *s1, const char *s2, int not_abs)
{
	int res;
	char *ps1, *ps2, *tar;

	ps1 = remove_ext(s1);
	ps2 = remove_ext(s2);
	if(not_abs == 0)
	{ 
		if(strcmp(ps1, ps2) == 0)
			res = 0;
		else
			res = -1;
	}
	else 
	{
		tar = ps1;
		ps1 = extract_title(ps1);
		free(tar);
		tar = ps2;
		ps2 = extract_title(ps2);
		free(tar);
		if(strcmp(ps1, ps2) == 0)
			res = 0;
		else
			res = -1;
	}

	free(ps1);
	free(ps2);
	return res;
}

int id3tagcmp(const char *d_name, ID3 *info)
{
	int retval = -1;
	char *name, *tmp;

	if(!info || !info->title) return -1;
	tmp = name = remove_ext(d_name);
	name = extract_title(name);
	free(tmp);
	if ( info->title && strncmp(info->title, name, strlen(name)) == 0 ){
		retval = 0;
	}
	free(name);

	return retval;
}

char *join_dir_and_file(const char *dir, const char *file)
{
	static char buf[512];

	if(dir[strlen(dir)-1] == '/')
		snprintf(buf, 512, "%s%s", dir, file);
	else
		snprintf(buf, 512, "%s/%s", dir, file);
	return buf;
}

/*the return var must be freed */
char *find_file_in_dir(char *path, char *file, char *file_full, int not_abs )
{
	char *ext[] = {".txt", ".lrc"};
	DIR *dir;
	ID3 *id3;

	/* create the ID3 info */
	id3 = create_ID3();
	if (getID3ByFile(id3, file_full) != 0){
		munmap(blob, filesize);
		destroy_ID3(id3);
		id3 = NULL;
	}

	dir = opendir(path);
	if(dir) /* the directory is exists and opend */
	{
		struct dirent *dirent;

		while((dirent = readdir(dir)))
		{
			char  *e, *p;/* e is ext, p is new_path*/
			int ext_num = 0;
			struct stat info;

			if(dirent->d_name[0] == '.') /* ., ..*/
				continue;

			p = join_dir_and_file(path, dirent->d_name);
			stat(p, &info);
			/* If it is a direcotory, dive int it. */
			if(S_ISDIR(info.st_mode)) {
				char *res;
				char *child_path = (char*)strdup(p);

				res = find_file_in_dir(child_path, file, file_full, not_abs);
				free(child_path);

				if(res)
				{
					closedir(dir); 
					if(id3){
						munmap(blob, filesize);
						destroy_ID3(id3);
					}
					return res;
				}
				continue;
			}

			/* Do not even consider loading too large (>100kB) files. */
			if(info.st_size > 100*1024)
				continue;

			/* Does it have any extention? */
			if(!(e = strrchr(dirent->d_name, '.')))
				continue;

			while(ext_num < 2) {
				if(!strcasecmp(e, ext[ext_num])) {
					if(fuzzystrcmp(dirent->d_name, file, not_abs) == 0 
							|| id3tagcmp(dirent->d_name, id3) == 0)
					{
						char *res = (char*)strdup(join_dir_and_file(path, dirent->d_name));

						closedir(dir);
						if(id3){
							munmap(blob, filesize);
							destroy_ID3(id3);
						}
						return(res);
					}
					break;
				}
				ext_num++;
			}
		}
		closedir(dir);
	}
	if(id3){
		munmap(blob, filesize);
		destroy_ID3(id3);
	}

	return(NULL);
}

/* the return var must be freed*/
char *find_lyrics_from_cache(char *playfile)
{
	FILE *cache_file;
	FILE *test;
	gchar full_name[255];
	gchar *in_index;
	gchar *in_value;
	gchar *iter;
	gchar buffer[512];
	long read_pos = 0;

	if( playfile == NULL)
		return NULL;

	snprintf(full_name, 255, "%s/.lyrics_cache", g_get_home_dir());

	if((cache_file = fopen( full_name, "r")) == NULL)
		return NULL;

	while(1)
	{
		if(!fgets(buffer, 512, cache_file))
		{
			fclose(cache_file);
			return NULL;
		}

		buffer[511] = 0;
		iter = buffer + strlen(buffer) - 1;
		*iter = 0;

		if((iter = strstr(buffer, " = ")))
		{
			*iter = 0;
			in_index = buffer;
			in_value = iter+3;
		}
		else
		{
			in_index = buffer;
			in_value = buffer;
		}

		if(!strcmp(in_index, playfile))
		{
			read_pos = ftell(cache_file);
			fclose(cache_file);
			test = fopen(in_value, "r");
			if(test == NULL)
			{/* if the file can not open 
			  *or not exist delete it from the file*/
				FILE *old_cache;
				FILE *new_cache;
				gchar tmp_file[255];	

				snprintf(tmp_file, 255, "%s.tmp", full_name);
				old_cache = fopen(full_name, "r");
				new_cache = fopen(tmp_file, "w+");
				while(fgets(buffer, 512, old_cache))
				{
					if(ftell(old_cache) == read_pos)
						continue;
					fprintf(new_cache, "%s", buffer);
				}

				fclose(old_cache);
				fclose(new_cache);
				rename(tmp_file, full_name);
				return NULL;
			}
			else
			{
				fclose(test);
				return (char*)strdup(in_value);
			}
		}
	}
}
/* add a new item to cache*/
void add_item_to_cache(char *playfile, char *lyricsfile)
{
	FILE *cache_file;
	gchar full_name[255];

	if( playfile == NULL || lyricsfile == NULL)
		return;

	snprintf(full_name, 255, "%s/.lyrics_cache", g_get_home_dir());

	if((cache_file = fopen( full_name, "a+")) == NULL)
		return;

	fprintf(cache_file, "%s = %s\n", playfile, lyricsfile);

	fclose(cache_file);

}
