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
#include <string.h>
#include <glib.h>
#include "internal.h"
/* check the code and convert locale code to utf8 */
gchar* locale2utf8(const gchar* data)
{
	if(data == NULL)
		return NULL;
	if(g_utf8_validate(data, -1, NULL) == TRUE)
		return (gchar*)strdup(data);

	/* thanks to cyril <cyril42e@gmail.com>*/
	gchar *res = g_locale_to_utf8(data, -1, NULL, NULL, NULL);
	return res ? res : (gchar*)strdup(data);  
}
	
/* check the code and convert utf8 to locale code */
gchar * utf82locale(const gchar* data)
{
	if(data == NULL)
		return NULL;
	if(g_utf8_validate(data, -1, NULL) == FALSE)
		return (gchar*)strdup(data);

	/* thanks to cyril <cyril42e@gmail.com>*/
	gchar *res = g_locale_to_utf8(data, -1, NULL, NULL, NULL);
	return res ? res : (gchar*)strdup(data);  
}

gchar* gb23122utf8(const gchar *data)
{
	if(data == NULL)
		return NULL;
	if(g_utf8_validate(data, -1, NULL) == TRUE)
		return (gchar*)strdup(data);
	return g_convert(data, -1, "utf8", "gb2312", NULL, NULL, NULL);
}

gchar* locale2gb2312(const gchar *data)
{
	const gchar *charset;

	if(data == NULL)
		return NULL;
	g_get_charset(&charset);
	return g_convert(data, -1, "gb2312", charset, NULL, NULL, NULL);
}
