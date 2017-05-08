/*
 *  pbmtojbg85 - Portable Bitmap to JBIG converter (T.85 version)
 *
 *  Markus Kuhn - http://www.cl.cam.ac.uk/~mgk25/jbigkit/
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "jbig85.h"


char *progname;                  /* global pointer to argv[0] */


/*
 * Print usage message and abort
 */
static void usage(void)
{
  fprintf(stderr,
     "PBMtoJBIG converter " JBG85_VERSION " (T.85 version) --\n"
     "creates bi-level image entity (BIE) as output file\n\n"
     "usage: %s [<options>] [<input-file> | -  [<output-file>]]\n\n"
     "options:\n\n", progname);
  fprintf(stderr,
     "  -s number\theight of a stripe\n");
  fprintf(stderr,
     "  -m number\tmaximum adaptive template pixel horizontal offset (default 8)\n"
     "  -p number\toptions byte value: add TPBON=8, LRLTWO=64\n"
     "\t\t(default 8 = TPBON)\n");
  fprintf(stderr,
     "  -C string\tadd the provided string as a comment marker segment\n");
  fprintf(stderr,
     "  -Y yi yr\tannounce in header initially the larger image height yi\n"
     "\t\tand then announce after line yr has been encoded the real height\n"
     "\t\tusing NEWLEN marker (for testing NEWLEN and VLENGTH=1 function)\n\n");
  exit(1);
}


/*
 * malloc() with exception handler
 */
void *checkedmalloc(size_t n)
{
  void *p;
  
  if ((p = malloc(n)) == NULL) {
    fprintf(stderr, "Sorry, not enough memory available!\n");
    exit(1);
  }
  
  return p;
}


/* 
 * Read an ASCII integer number from file f and skip any PBM
 * comments which are encountered.
 */
static unsigned long getint(FILE *f)
{
  int c;
  unsigned long i;

  while ((c = getc(f)) != EOF && !isdigit(c))
    if (c == '#')
      while ((c = getc(f)) != EOF && !(c == 13 || c == 10)) ;
  if (c != EOF) {
    ungetc(c, f);
    fscanf(f, "%lu", &i);
  }

  return i;
}


/*
 * Callback procedure which is used by JBIG encoder to deliver the
 * encoded data. It simply sends the bytes to the output file.
 */
static void data_out(unsigned char *start, size_t len, void *file)
{
  fwrite(start, len, 1, (FILE *) file);
  return;
}


int main (int argc, char **argv)
{
  FILE *fin = stdin, *fout = stdout;
  const char *fnin = NULL, *fnout = NULL;
  int i, j, c;
  int all_args = 0, files = 0;
  unsigned long x, y;
  unsigned long width, height;
  size_t bpl;
  char type;
  unsigned char *p, *lines, *next_line;
  unsigned char *prev_line = NULL, *prevprev_line = NULL;
  struct jbg85_enc_state s;
  int mx = -1;
  unsigned long l0 = 0, yi = 0, yr = 0;
  char *comment = NULL;
  int options = JBG_TPBON;

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
	  case 0 :
	    if (files++) usage();
	    break;
	  case 'Y':
	    if (i+2 >= argc) usage();
	    j = -1;
	    yi = atol(argv[++i]);
	    yr = atol(argv[++i]);
	    break;
	  case 'p':
	    if (++i >= argc) usage();
	    j = -1;
	    options = atoi(argv[i]);
	    break;
	  case 's':
	    if (++i >= argc) usage();
	    j = -1;
	    l0 = atol(argv[i]);
	    break;
	  case 'm':
	    if (++i >= argc) usage();
	    j = -1;
	    mx = atoi(argv[i]);
	    break;
	  case 'C':
	    if (++i >= argc) usage();
	    j = -1;
	    comment = argv[i];
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
  
  /* open input file */
  if (fnin) {
    fin = fopen(fnin, "rb");
    if (!fin) {
      fprintf(stderr, "Can't open input file '%s", fnin);
      perror("'");
      exit(1);
    }
  } else
    fnin  = "<stdin>";

  /* read PBM header */
  while ((c = getc(fin)) != EOF && (isspace(c) || c == '#'))
    if (c == '#')
      while ((c = getc(fin)) != EOF && !(c == 13 || c == 10)) ;
  type = getc(fin);
  if (c != 'P' || (type != '1' && type != '4')) {
    fprintf(stderr, "Input file '%s' does not look like a PBM file!\n", fnin);
    exit(1);
  }
  width = getint(fin);
  height = getint(fin);
  fgetc(fin);    /* skip line feed */

  /* Test for valid parameters */
  if (width < 1 || height < 1) {
    fprintf(stderr, "Image dimensions must be positive!\n");
    exit(1);
  }

  /* allocate buffer for a single image line */
  bpl = (width >> 3) + !!(width & 7);     /* bytes per line */
  lines = (unsigned char *) checkedmalloc(bpl * 3);
  
  /* open output file */
  if (fnout) {
    fout = fopen(fnout, "wb");
    if (!fout) {
      fprintf(stderr, "Can't open input file '%s", fnout);
      perror("'");
      exit(1);
    }
  } else
    fnout = "<stdout>";

  /* initialize parameter struct for JBIG encoder*/
  jbg85_enc_init(&s, width, yi ? yi : height, data_out, fout);

  /* Specify a few other options (each is ignored if negative) */
  if (yi)
    options |= JBG_VLENGTH;
  if (comment) {
    s.comment_len = strlen(comment);
    s.comment = (unsigned char *) comment;
  }
  jbg85_enc_options(&s, options, l0, mx);

  for (y = 0; y < height; y++) {

    /* Use a 3-line ring buffer, because the encoder requires that the two
     * previously supplied lines are still in memory when the next line is
     * processed. */
    next_line = lines + (y%3)*bpl;

    switch (type) {
    case '1':
      /* PBM text format */
      p = next_line;
      for (x = 0; x <= ((width-1) | 7); x++) {
	*p <<= 1;
	if (x < width)
	  *p |= getint(fin) & 1;
	if ((x & 7) == 7)
	  ++p;
      }
      break;
    case '4':
      /* PBM raw binary format */
      fread(next_line, bpl, 1, fin);
      break;
    default:
      fprintf(stderr, "Unsupported PBM type P%c!\n", type);
      exit(1);
    }
    if (ferror(fin)) {
      fprintf(stderr, "Problem while reading input file '%s", fnin);
      perror("'");
      exit(1);
    }
    if (feof(fin)) {
      fprintf(stderr, "Unexpected end of input file '%s'!\n", fnin);
      exit(1);
    }

    /* JBIG compress another line and write out result via callback */
    jbg85_enc_lineout(&s, next_line, prev_line, prevprev_line);
    prevprev_line = prev_line;
    prev_line = next_line;

    /* adjust final image height via NEWLEN */
    if (yi && y == yr)
      jbg85_enc_newlen(&s, height);
  }

  /* check for file errors and close fout */
  if (ferror(fout) || fclose(fout)) {
    fprintf(stderr, "Problem while writing output file '%s", fnout);
    perror("'");
    exit(1);
  }

  return 0;
}
