/**file test-fuzzer.c
 * from test-parse.c and test-mnote.c
 *
 * \brief Completely parse all files given on the command line.
 *
 * Copyright (C) 2007 Hans Ulrich Niedermann <gp@n-dimensional.de>
 * Copyright 2002 Lutz Mueller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "libexif/exif-data.h"
#include "libexif/exif-system.h"

/** Callback function handling an ExifEntry. */
void content_foreach_func(ExifEntry *entry, void *callback_data);
void content_foreach_func(ExifEntry *entry, void *UNUSED(callback_data))
{
	char buf[2001];

	/* ensure \0 */
	buf[sizeof(buf)-1] = 0;
	buf[sizeof(buf)-2] = 0;
	exif_entry_get_value(entry, buf, sizeof(buf)-1);
	printf("    Entry %p: %s (%s)\n"
		 "      Size, Comps: %d, %d\n"
		 "      Value: %s\n", 
		 entry,
		 exif_tag_get_name(entry->tag),
		 exif_format_get_name(entry->format),
		 entry->size,
		 (int)(entry->components),
		 exif_entry_get_value(entry, buf, sizeof(buf)-1));
	if (buf[sizeof(buf)-2] != 0) abort();
}


/** Callback function handling an ExifContent (corresponds 1:1 to an IFD). */
void data_foreach_func(ExifContent *content, void *callback_data);
void data_foreach_func(ExifContent *content, void *callback_data)
{
	printf("  Content %p: ifd=%d\n", content, exif_content_get_ifd(content));
	exif_content_foreach_entry(content, content_foreach_func, callback_data);
}
static int
test_exif_data (ExifData *d)
{
	unsigned int i, c;
	char v[1024], *p;
	ExifMnoteData *md;

	fprintf (stdout, "Byte order: %s\n",
		exif_byte_order_get_name (exif_data_get_byte_order (d)));

	md = exif_data_get_mnote_data (d);
	if (!md) {
		fprintf (stderr, "Could not parse maker note!\n");
		return 1;
	}

	exif_mnote_data_ref (md);
	exif_mnote_data_unref (md);

	c = exif_mnote_data_count (md);
	for (i = 0; i < c; i++) {
		const char *name = exif_mnote_data_get_name (md, i);
		if (!name) break;
		fprintf (stdout, "Dumping entry number %i...\n", i);
		fprintf (stdout, "  Name: '%s'\n",
				exif_mnote_data_get_name (md, i));
		fprintf (stdout, "  Title: '%s'\n",
				exif_mnote_data_get_title (md, i));
		fprintf (stdout, "  Description: '%s'\n",
				exif_mnote_data_get_description (md, i));
		p = exif_mnote_data_get_value (md, i, v, sizeof (v));
		if (p) fprintf (stdout, "  Value: '%s'\n", v);
	}

	return 0;
}



/** Run EXIF parsing test on the given file. */
static void test_parse(const char *filename, void *callback_data)
{
	ExifData *d;
	unsigned int buf_size;
	unsigned char *buf;

	d = exif_data_new_from_file(filename);
	exif_data_foreach_content(d, data_foreach_func, callback_data);
	test_exif_data (d);

	buf = NULL;
	exif_data_save_data (d, &buf, &buf_size);
	free (buf);

	exif_data_set_byte_order(d, EXIF_BYTE_ORDER_INTEL);

	buf = NULL;
	exif_data_save_data (d, &buf, &buf_size);
	free (buf);


	exif_data_unref(d);
}


/** Main program. */
int main(const int argc, const char *argv[])
{
	int i;
	void *callback_data = NULL;

	for (i=1; i<argc; i++) {
		test_parse(argv[i], callback_data);
	}
	return 0;
}
