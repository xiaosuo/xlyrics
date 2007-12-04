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
#include <glib.h>
#include <string.h>
#include "internal.h"
/* check the code and convert locale code to utf8 */
gchar* locale2utf8(const gchar* data)
{
	if(data == NULL)
		return NULL;
	if(g_utf8_validate(data, -1, NULL) == TRUE)
		return (gchar*)strdup(data);
	return g_locale_to_utf8(data, -1, NULL, NULL, NULL);
}

/* check the code and convert utf8 to locale code */
gchar * utf82locale(const gchar* data)
{
	if(data == NULL)
		return NULL;
	if(g_utf8_validate(data, -1, NULL) == FALSE)
		return (gchar*)strdup(data);
	return g_locale_from_utf8(data, -1, NULL, NULL, NULL);
}
