/*
 *  jbgtopbm85 - JBIG to Portable Bitmap converter (T.85 version)
 *
 *  Markus Kuhn - http://www.cl.cam.ac.uk/~mgk25/jbigkit/
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "jbig85.h"

char *progname;                  /* global pointer to argv[0] */
unsigned long y_0;
fpos_t ypos;
int ypos_error = 1;
unsigned long ymax = 0;

/*
 * Print usage message and abort
 */
static void usage(void)
{
  fprintf(stderr, "JBIGtoPBM converter " JBG85_VERSION " (T.85 version) --\n"
	  "reads a bi-level image entity (BIE) as input file\n\n"
	  "usage: %s [<input-file> | -  [<output-file>]]\n\n", progname);
  fprintf(stderr, "options:\n\n"
	  "  -x number\tmaximum number of pixels per line for which memory\n"
	  "\t\tis allocated (default: 8192)\n"
          "  -y number\tinterrupt decoder after this number of lines\n"
	  "  -B number\tinput buffer size\n\n");
  exit(1);
}


/*
 * Call-back routine for merged image output
 */
int line_out(const struct jbg85_dec_state *s,
	     unsigned char *start, size_t len, unsigned long y, void *file)
{
  if (y == 0) {
    /* prefix first line with PBM header */
    fprintf((FILE *) file, "P4\n");
    fprintf((FILE *) file, "%10lu\n", jbg85_dec_getwidth(s));
    /* store file position of height, so we can update it after NEWLEN */
    y_0 = jbg85_dec_getheight(s);
    ypos_error = fgetpos((FILE *) file, &ypos);
    fprintf((FILE *) file, "%10lu\n", y_0); /* pad number to 10 bytes */
  }
  fwrite(start, len, 1, (FILE *) file);
  return y == ymax - 1;
}


int main (int argc, char **argv)
{
  FILE *fin = stdin, *fout = stdout;
  const char *fnin = NULL, *fnout = NULL;
  int i, j, result;
  int all_args = 0, files = 0;
  struct jbg85_dec_state s;
  unsigned char *inbuf, *outbuf;
  size_t inbuflen = 8192, outbuflen, len, cnt, cnt2;
  unsigned long xmax = 8192;
  size_t bytes_read = 0;

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
          case 'x':
            if (++i >= argc) usage();
            j = -1;
            xmax = atol(argv[i]);
            break;
          case 'y':
            if (++i >= argc) usage();
            j = -1;
            ymax = atol(argv[i]);
            break;
          case 'B':
            if (++i >= argc) usage();
            j = -1;
            inbuflen = atol(argv[i]);
	    if (inbuflen < 1) usage();
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

  inbuf = (unsigned char *) malloc(inbuflen);
  outbuflen = ((xmax >> 3) + !!(xmax & 7)) * 3;
  outbuf = (unsigned char *) malloc(outbuflen);
  if (!inbuf || !outbuf) {
    printf("Sorry, not enough memory available!\n");
    exit(1);
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
  jbg85_dec_init(&s, outbuf, outbuflen, line_out, fout);
  result = JBG_EAGAIN;
  while ((len = fread(inbuf, 1, inbuflen, fin))) {
    result = jbg85_dec_in(&s, inbuf, len, &cnt);
    bytes_read += cnt;
    while (result == JBG_EOK_INTR) {
      /* demonstrate decoder interrupt at given line number */
      printf("Decoding interrupted after %lu lines and %lu BIE bytes "
	     "... continuing ...\n", s.y, (unsigned long) bytes_read);
      /* and now continue decoding */
      result = jbg85_dec_in(&s, inbuf + cnt, len - cnt, &cnt2);
      bytes_read += cnt2;
      cnt += cnt2;
    }
    if (result != JBG_EAGAIN)
      break;
  }
  if (ferror(fin)) {
    fprintf(stderr, "Problem while reading input file '%s", fnin);
    perror("'");
    if (fout != stdout) {
      fclose(fout);
      remove(fnout);
    }
    exit(1);
  }
  if (result == JBG_EAGAIN || result == JBG_EOK_INTR) {
    /* signal end-of-BIE explicitely */
    result = jbg85_dec_end(&s);
    while (result == JBG_EOK_INTR) {
      /* demonstrate decoder interrupt at given line number */
      printf("Decoding interrupted after %lu lines and %lu BIE bytes "
	     "... continuing ...\n", s.y, (unsigned long) bytes_read);
      result = jbg85_dec_end(&s);
    }
  }
  if (result != JBG_EOK) {
    fprintf(stderr, "Problem with input file '%s': %s\n"
            "(error code 0x%02x, %lu = 0x%04lx BIE bytes "
	    "and %lu pixel rows processed)\n",
	    fnin, jbg85_strerror(result), result,
	    (unsigned long) bytes_read, (unsigned long) bytes_read, s.y);
    if (fout != stdout) {
      fclose(fout);
      /*remove(fnout);*/
    }
    exit(1);
  }

  /* do we have to update the image height in the PBM header? */
  if (!ypos_error && y_0 != jbg85_dec_getheight(&s)) {
    if (fsetpos(fout, &ypos) == 0) {
      fprintf(fout, "%10lu", jbg85_dec_getheight(&s)); /* pad to 10 bytes */
    } else {
      fprintf(stderr, "Problem while updating height in output file '%s",
	      fnout);
      perror("'");
      exit(1);
    }
  }

  /* check for file errors and close fout */
  if (ferror(fout) || fclose(fout)) {
    fprintf(stderr, "Problem while writing output file '%s", fnout);
    perror("'");
    exit(1);
  }

  return 0;
}
