/*
 *  jbgtopbm - JBIG to Portable Bitmap converter
 *
 *  Markus Kuhn - http://www.cl.cam.ac.uk/~mgk25/jbigkit/
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "jbig.h"

char *progname;                  /* global pointer to argv[0] */


/*
 * Print usage message and abort
 */
static void usage(void)
{
  fprintf(stderr, "JBIGtoPBM converter " JBG_VERSION " -- "
     "reads a bi-level image entity (BIE) as input file\n\n"
     "usage: %s [<options>] [<input-file> | -  [<output-file>]]\n\n"
     "options:\n\n", progname);
  fprintf(stderr,
     "  -x number\tif possible decode only up to a resolution layer not\n"
     "\t\twider than the given number of pixels\n"
     "  -y number\tif possible decode only up to a resolution layer not\n"
     "\t\thigher than the given number of pixels\n"
     "  -m\t\tdecode a progressive sequence of multiple concatenated BIEs\n"
     "  -b\t\tuse binary code for multiple bit planes (default: Gray code)\n"
     "  -d\t\tdiagnose single BIE, print header, list marker sequences\n"
     "  -p number\tdecode only one single bit plane (0 = first plane)\n\n");
  exit(1);
}


/*
 * Call-back routine for merged image output
 */
void write_it(unsigned char *data, size_t len, void *file)
{
  fwrite(data, len, 1, (FILE *) file);
}

/*
 * Remalloc a buffer and append a file f into its content.
 * If *buflen == 0, then malloc a buffer first.
 */
void read_file(unsigned char **buf, size_t *buflen, size_t *len, FILE *f)
{
  if (*buflen == 0) {
    *buflen = 4000;
    *len = 0;
    *buf = (unsigned char *) malloc(*buflen);
    if (!*buf) {
      fprintf(stderr, "Sorry, not enough memory available!\n");
      exit(1);
    }
  }
  do {
    *len += fread(*buf + *len, 1, *buflen - *len, f);
    if (*len == *buflen) {
      *buflen *= 2;
      *buf = (unsigned char *) realloc(*buf, *buflen);
      if (!*buf) {
	fprintf(stderr, "Sorry, not enough memory available!\n");
	exit(1);
      }
    }
    if (ferror(f)) {
      perror("Problem while reading input file");
      exit(1);
    }
  } while (!feof(f));
  *buflen = *len;
  *buf = (unsigned char *) realloc(*buf, *buflen);
  if (!*buf) {
    fprintf(stderr, "Oops, realloc failed when shrinking buffer!\n");
    exit(1);
  }
  
  return;
}


/* marker codes */
#define MARKER_STUFF    0x00
#define MARKER_SDNORM   0x02
#define MARKER_SDRST    0x03
#define MARKER_ABORT    0x04
#define MARKER_NEWLEN   0x05
#define MARKER_ATMOVE   0x06
#define MARKER_COMMENT  0x07
#define MARKER_ESC      0xff


/*
 * Output (prefix of) a short byte sequence in hexadecimal
 * for diagnostic purposes
 */
void fprint_bytes(FILE *f, unsigned char *p, size_t len, int width)
{
  size_t i;
  size_t max = width / 3;
  if (len > max)
    max -= 7;
  for (i = 0; i < len && i < max; i++)
    fprintf(f, "%02x ", p[i]);
  if (len > i)
    fprintf(f, "... %lu bytes total", (unsigned long) len);
  fprintf(f, "\n");
}

/*
 * Read BIE and output human readable description of content
 */
void diagnose_bie(FILE *fin)
{
  unsigned char *bie, *p, *pnext;
  size_t buflen = 0, len;
  unsigned long xd, yd, l0;
  int dl, d;
  FILE *f = stdout;
  extern unsigned char *jbg_next_pscdms(unsigned char *p, size_t len);
  extern unsigned long jbg_stripes(unsigned long l0, unsigned long yd,
				   unsigned long d);
  unsigned long stripes;
  int layers, planes;
  unsigned long sdes, sde = 0;
  
  /* read BIH */
  read_file(&bie, &buflen, &len, fin);
  if (len < 20) {
    fprintf(f, "Error: Input file is %lu < 20 bytes long and therefore "
	    "does not contain an intact BIE header!\n", (unsigned long) len);
    return;
  }

  /* parse BIH */
  fprintf(f, "BIH:\n\n  DL = %d\n  D  = %d\n  P  = %d\n"
	  "  -  = %d\n  XD = %lu\n  YD = %lu\n  L0 = %lu\n  MX = %d\n"
	  "  MY = %d\n",
	  dl = bie[0], d = bie[1], planes = bie[2], bie[3],
	  xd = ((unsigned long)bie[ 4] << 24) | ((unsigned long)bie[ 5] << 16)|
	  ((unsigned long) bie[ 6] <<  8) | ((unsigned long) bie[ 7]),
	  yd = ((unsigned long)bie[ 8] << 24) | ((unsigned long)bie[ 9] << 16)|
	  ((unsigned long) bie[10] <<  8) | ((unsigned long) bie[11]),
	  l0 = ((unsigned long)bie[12] << 24) | ((unsigned long)bie[13] << 16)|
	  ((unsigned long) bie[14] <<  8) | ((unsigned long) bie[15]),
	  bie[16], bie[17]);
  fprintf(f, "  order   = %d %s%s%s%s%s\n", bie[18],
	  bie[18] & JBG_HITOLO ? " HITOLO" : "",
	  bie[18] & JBG_SEQ ? " SEQ" : "",
	  bie[18] & JBG_ILEAVE ? " ILEAVE" : "",
	  bie[18] & JBG_SMID ? " SMID" : "",
	  bie[18] & 0xf0 ? " other" : "");
  fprintf(f, "  options = %d %s%s%s%s%s%s%s%s\n", bie[19],
	  bie[19] & JBG_LRLTWO ? " LRLTWO" : "",
	  bie[19] & JBG_VLENGTH ? " VLENGTH" : "",
	  bie[19] & JBG_TPDON ? " TPDON" : "",
	  bie[19] & JBG_TPBON ? " TPBON" : "",
	  bie[19] & JBG_DPON ? " DPON" : "",
	  bie[19] & JBG_DPPRIV ? " DPPRIV" : "",
	  bie[19] & JBG_DPLAST ? " DPLAST" : "",
	  bie[19] & 0x80 ? " other" : "");
  stripes = jbg_stripes(l0, yd, d);
  layers = d - dl + 1;
  fprintf(f, "\n  %lu stripes, %d layers, %d planes => ",
	  stripes, layers, planes);
  if ((ULONG_MAX / layers) / planes >= stripes) {
    sdes = stripes * layers * planes;
    fprintf(f, "%lu SDEs\n\n", sdes);
  } else {
    /* handle integer overflow */
    fprintf(f, ">%lu SDEs!\n", ULONG_MAX);
    return;
  }

  /* parse BID */
  fprintf(f, "BID:\n\n");
  p = bie + 20; /* skip BIH */
  if ((bie[19] & (JBG_DPON | JBG_DPPRIV | JBG_DPLAST))
      == (JBG_DPON | JBG_DPPRIV))
    p += 1728;  /* skip DPTABLE */
  if (p > bie + len) {
    fprintf(f, "Error: Input file is %lu < 20+1728 bytes long and therefore "
	    "does not contain an intact BIE header with DPTABLE!\n",
	    (unsigned long) len);
    return;
  }
  while (p != bie + len) {
    if (p > bie + len - 2) {
      fprintf(f, "%06lx: Error: single byte 0x%02x left\n",
	      (long) (p - bie), *p);
      return;
    }
    pnext = jbg_next_pscdms(p, len - (p - bie));
    if (p[0] != MARKER_ESC || p[1] == MARKER_STUFF) {
      fprintf(f, "%06lx: PSCD: ", (long) (p - bie));
      fprint_bytes(f, p, pnext ? (size_t) (pnext - p) : len - (p - bie), 60);
      if (!pnext) {
	fprintf(f, "Error: PSCD not terminated by SDNORM or SDRST marker\n");
	return;
      }
    } else
      switch (p[1]) {
      case MARKER_SDNORM:
      case MARKER_SDRST:
	fprintf(f, "%06lx: ESC %s, ending SDE #%lu", (long) (p - bie),
		(p[1] == MARKER_SDNORM) ? "SDNORM" : "SDRST", ++sde);
	if (sde == sdes)
	  fprintf(f, " (final SDE)");
	else if (sde == sdes + 1)
	  fprintf(f, " (first surplus SDE, VLENGTH = %d)",
		  (bie[19] & JBG_VLENGTH) > 0);
	fprintf(f, "\n");
	break;
      case MARKER_ABORT:
	fprintf(f, "%06lx: ESC ABORT\n", (long) (p - bie));
	break;
      case MARKER_NEWLEN:
	fprintf(f, "%06lx: ESC NEWLEN ", (long) (p - bie));
	if (p + 5 < bie + len) {
	  fprintf(f, "YD = %lu\n",
		  yd = (((long) p[2] << 24) | ((long) p[3] << 16) |
			((long) p[4] <<  8) |  (long) p[5]));
	  stripes = jbg_stripes(l0, yd, d);
	  fprintf(f, "        %lu stripes, %d layers, %d planes => ",
		  stripes, layers, planes);
	  if ((ULONG_MAX / layers) / planes >= stripes) {
	    sdes = stripes * layers * planes;
	    fprintf(f, "%lu SDEs\n", sdes);
	  } else {
	    /* handle integer overflow */
	    fprintf(f, ">%lu SDEs!\n", ULONG_MAX);
	    return;
	  }
	} else
	  fprintf(f, "unexpected EOF\n");
	break;
      case MARKER_ATMOVE:
	fprintf(f, "%06lx: ESC ATMOVE ", (long) (p - bie));
	if (p + 7 < bie + len)
	  fprintf(f, "YAT = %lu, tX = %d, tY = %d\n",
		  (((long) p[2] << 24) | ((long) p[3] << 16) |
		   ((long) p[4] <<  8) |  (long) p[5]), p[6], p[7]);
	else
	  fprintf(f, "unexpected EOF\n");
	break;
      case MARKER_COMMENT:
	fprintf(f, "%06lx: ESC COMMENT ", (long) (p - bie));
	if (p + 5 < bie + len)
	  fprintf(f, "LC = %lu\n",
		  (((long) p[2] << 24) | ((long) p[3] << 16) |
		   ((long) p[4] <<  8) |  (long) p[5]));
	else
	  fprintf(f, "unexpected EOF\n");
	break;
      default:
	fprintf(f, "%06lx: ESC 0x%02x\n", (long) (p - bie), p[1]);
      }
    if (!pnext) {
      fprintf(f, "Error encountered!\n");
      return;
    }
    p = pnext;
  }

  free(bie);

  return;
}


int main (int argc, char **argv)
{
  FILE *fin = stdin, *fout = stdout;
  const char *fnin = NULL, *fnout = NULL;
  int i, j, result;
  int all_args = 0, files = 0;
  struct jbg_dec_state s;
  unsigned char *buffer, *p;
  size_t buflen, len, cnt;
  size_t bytes_read = 0;
  unsigned long xmax = 4294967295UL, ymax = 4294967295UL, max;
  int plane = -1, use_graycode = 1, diagnose = 0, multi = 0;

  buflen = 8000;
  buffer = (unsigned char *) malloc(buflen);
  if (!buffer) {
    printf("Sorry, not enough memory available!\n");
    exit(1);
  }

  /* parse command line arguments */
  progname = argv[0];
  for (i = 1; i < argc; i++) {
    if (!all_args && argv[i][0] == '-')
      if (argv[i][1] == 0) {
	if (files++) usage();
      } else
	for (j = 1; j > 0 && argv[i][j]; j++)
	  switch(argv[i][j]) {
	  case '-' :
	    all_args = 1;
	    break;
	  case 'b':
	    use_graycode = 0;
	    break;
	  case 'm':
	    multi = 1;
	    break;
	  case 'd':
	    diagnose = 1;
	    break;
	  case 'x':
	    if (++i >= argc) usage();
	    xmax = atol(argv[i]);
	    j = -1;
	    break;
	  case 'y':
	    if (++i >= argc) usage();
	    ymax = atol(argv[i]);
	    j = -1;
	    break;
	  case 'p':
	    if (++i >= argc) usage();
	    plane = atoi(argv[i]);
	    j = -1;
	    break;
	  default:
	    usage();
	  }
    else
      switch (files++) {
      case 0: fnin  = argv[i]; break;
      case 1: fnout = argv[i]; break;
      default:
	usage();
      }
  }

  if (fnin) {
    fin = fopen(fnin, "rb");
    if (!fin) {
      fprintf(stderr, "Can't open input file '%s", fnin);
      perror("'");
      exit(1);
    }
  } else
    fnin  = "<stdin>";
  if (diagnose) {
    diagnose_bie(fin);
    exit(0);
  }
  if (fnout) {
    fout = fopen(fnout, "wb");
    if (!fout) {
      fprintf(stderr, "Can't open input file '%s", fnout);
      perror("'");
      exit(1);
    }
  } else
    fnout = "<stdout>";

  /* send input file to decoder */
  jbg_dec_init(&s);
  jbg_dec_maxsize(&s, xmax, ymax);
  /* read BIH first to check VLENGTH */
  len = fread(buffer, 1, 20, fin);
  if (len < 20) {
    fprintf(stderr, "Input file '%s' (%lu bytes) must be at least "
	    "20 bytes long\n", fnin, (unsigned long) len);
    if (fout != stdout) {
      fclose(fout);
      remove(fnout);
    }
    exit(1);
  }
  if (buffer[19] & JBG_VLENGTH) {
    /* VLENGTH = 1 => we might encounter a NEWLEN, therefore read entire
     * input file into memory and run two passes over it */
    read_file(&buffer, &buflen, &len, fin);
    /* scan for NEWLEN marker segments and update BIE header accordingly */
    result = jbg_newlen(buffer, len);
    /* feed data to decoder */
    if (result == JBG_EOK) {
      p = (unsigned char *) buffer;
      result = JBG_EAGAIN;
      while (len > 0 &&
	     (result == JBG_EAGAIN || (result == JBG_EOK && multi))) {
	result = jbg_dec_in(&s, p, len, &cnt);
	p += cnt;
	len -= cnt;
	bytes_read += cnt;
      }
    }
  } else {
    /* VLENGTH = 0 => we can simply pass the input file directly to decoder */
    result = JBG_EAGAIN;
    do {
      cnt = 0;
      p = (unsigned char *) buffer;
      while (len > 0 &&
	     (result == JBG_EAGAIN || (result == JBG_EOK && multi))) {
	result = jbg_dec_in(&s, p, len, &cnt);
	p += cnt;
	len -= cnt;
	bytes_read += cnt;
      }
      if (!(result == JBG_EAGAIN || (result == JBG_EOK && multi)))
	break;
      len = fread(buffer, 1, buflen, fin);
    } while (len > 0);
    if (ferror(fin)) {
      fprintf(stderr, "Problem while reading input file '%s", fnin);
      perror("'");
      if (fout != stdout) {
	fclose(fout);
	remove(fnout);
      }
      exit(1);
    }
  }
  if (result != JBG_EOK && result != JBG_EOK_INTR) {
    fprintf(stderr, "Problem with input file '%s': %s\n"
            "(error code 0x%02x, %lu = 0x%04lx BIE bytes processed)\n",
 	    fnin, jbg_strerror(result), result,
	    (unsigned long) bytes_read, (unsigned long) bytes_read);
    if (fout != stdout) {
      fclose(fout);
      remove(fnout);
    }
    exit(1);
  }
  if (plane >= 0 && jbg_dec_getplanes(&s) <= plane) {
    fprintf(stderr, "Image has only %d planes!\n", jbg_dec_getplanes(&s));
    if (fout != stdout) {
      fclose(fout);
      remove(fnout);
    }
    exit(1);
  }

  if (jbg_dec_getplanes(&s) == 1 || plane >= 0) {
    /* write PBM output file */
    fprintf(fout, "P4\n%10lu\n%10lu\n", jbg_dec_getwidth(&s),
	    jbg_dec_getheight(&s));
    fwrite(jbg_dec_getimage(&s, plane < 0 ? 0 : plane), 1,
	   jbg_dec_getsize(&s), fout);
  } else {
    /* write PGM output file */
    if ((size_t) jbg_dec_getplanes(&s) > sizeof(unsigned long) * 8) {
      fprintf(stderr, "Image has too many planes (%d)!\n",
	      jbg_dec_getplanes(&s));
      if (fout != stdout) {
	fclose(fout);
	remove(fnout);
      }
      exit(1);
    }
    max = 0;
    for (i = jbg_dec_getplanes(&s); i > 0; i--)
      max = (max << 1) | 1;
    fprintf(fout, "P5\n%10lu\n%10lu\n%lu\n", jbg_dec_getwidth(&s),
	    jbg_dec_getheight(&s), max);
    jbg_dec_merge_planes(&s, use_graycode, write_it, fout);
  }

  /* check for file errors and close fout */
  if (ferror(fout) || fclose(fout)) {
    fprintf(stderr, "Problem while writing output file '%s", fnout);
    perror("'");
    exit(1);
  }

  jbg_dec_free(&s);

  return 0;
}
