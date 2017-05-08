/*
 *  pbmtojbg - Portable Bitmap to JBIG converter
 *
 *  Markus Kuhn - http://www.cl.cam.ac.uk/~mgk25/jbigkit/
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "jbig.h"


char *progname;                  /* global pointer to argv[0] */
unsigned long total_length = 0;  /* used for determining output file length */


/*
 * Print usage message and abort
 */
static void usage(void)
{
  fprintf(stderr,
     "PBMtoJBIG converter " JBG_VERSION " -- "
     "creates bi-level image entity (BIE) as output file\n\n"
     "usage: %s [<options>] [<input-file> | -  [<output-file>]]\n\n"
     "options:\n\n", progname);
  fprintf(stderr,
     "  -q\t\tsequential coding, no differential layers (like -d 0)\n"
     "  -x number\tmaximum width of lowest resolution layer (default 640)\n"
     "  -y number\tmaximum height of lowest resolution layer (default 480)\n"
     "  -l number\tlowest layer written to output file (default 0)\n"
     "  -h number\thighest layer written to output file (default max)\n"
     "  -b\t\tuse binary code for multiple bitplanes (default: Gray code)\n"
     "  -d number\ttotal number of differential layers (overrides -x and -y)\n"
     "  -s number\theight of a stripe in layer 0\n");
  fprintf(stderr,
     "  -m number\tmaximum adaptive template pixel horizontal offset (default 8)\n"
     "  -t number\tencode only that many most significant planes\n"
     "  -o number\torder byte value: add 1=SMID, 2=ILEAVE, 4=SEQ, 8=HITOLO\n"
     "\t\t(default 3 = ILEAVE+SMID)\n"
     "  -p number\toptions byte value: add DPON=4, TPBON=8, TPDON=16, LRLTWO=64\n"
     "\t\t(default 28 = DPON+TPBON+TPDON)\n");
  fprintf(stderr,
     "  -C string\tadd the provided string as a comment marker segment\n"
     "  -c\t\tdelay adaptive template changes to first line of next stripe\n"
	  "\t\t(only provided for a conformance test)\n"
     "  -r\t\tterminate each stripe with SDRST marker\n"
	  "\t\t(only intended for decoder testing)\n" );
  fprintf(stderr,
     "  -Y number\tannounce in header initially this larger image height\n"
     "\t\t(only for generating test files with NEWLEN and VLENGTH=1)\n"
     "  -f\t\tchose encoding options for T.85 fax profile complianance\n"
     "  -v\t\tverbose output\n\n");
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
  total_length += len;
  return;
}


int main (int argc, char **argv)
{
  FILE *fin = stdin, *fout = stdout;
  const char *fnin = NULL, *fnout = NULL;
  int i, j, c;
  int all_args = 0, files = 0;
  unsigned long x, y;
  unsigned long width, height, max, v;
  unsigned long bpl;
  int bpp, planes, encode_planes = -1;
  size_t bitmap_size;
  char type;
  unsigned char **bitmap, *p, *image;
  struct jbg_enc_state s;
  int verbose = 0, delay_at = 0, reset = 0, use_graycode = 1;
  long mwidth = 640, mheight = 480;
  int dl = -1, dh = -1, d = -1, mx = -1;
  unsigned long l0 = 0, y1 = 0;
  char *comment = NULL;
  int options = JBG_TPDON | JBG_TPBON | JBG_DPON;
  int order = JBG_ILEAVE | JBG_SMID;

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
	  case 'v':
	    verbose = 1;
	    break;
	  case 'b':
	    use_graycode = 0;
	    break;
	  case 'c':
	    delay_at = 1;
	    break;
	  case 'r':
	    reset = 1;
	    break;
	  case 'f':
	    d = 0;
	    order = 0;
            options = 8;
	    l0 = 128;
	    encode_planes = 1;
	    mx = 127;
	    break;
	  case 'x':
	    if (++i >= argc) usage();
	    j = -1;
	    mwidth = atol(argv[i]);
	    break;
	  case 'y':
	    if (++i >= argc) usage();
	    j = -1;
	    mheight = atol(argv[i]);
	    break;
	  case 'Y':
	    if (++i >= argc) usage();
	    j = -1;
	    y1 = atol(argv[i]);
	    break;
	  case 'o':
	    if (++i >= argc) usage();
	    j = -1;
	    order = atoi(argv[i]);
	    break;
	  case 'p':
	    if (++i >= argc) usage();
	    j = -1;
	    options = atoi(argv[i]);
	    break;
	  case 'l':
	    if (++i >= argc) usage();
	    j = -1;
	    dl = atoi(argv[i]);
	    break;
	  case 'h':
	    if (++i >= argc) usage();
	    j = -1;
	    dh = atoi(argv[i]);
	    break;
	  case 'q':
	    d = 0;
	    break;
	  case 'd':
	    if (++i >= argc) usage();
	    j = -1;
	    d = atoi(argv[i]);
	    break;
	  case 's':
	    if (++i >= argc) usage();
	    j = -1;
	    l0 = atol(argv[i]);
	    break;
	  case 't':
	    if (++i >= argc) usage();
	    j = -1;
	    encode_planes = atoi(argv[i]);
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

  /* read PBM header */
  while ((c = getc(fin)) != EOF && (isspace(c) || c == '#'))
    if (c == '#')
      while ((c = getc(fin)) != EOF && !(c == 13 || c == 10)) ;
  if (c != 'P') {
    fprintf(stderr, "Input file '%s' does not look like a PBM file!\n", fnin);
    exit(1);
  }
  type = getc(fin);
  width = getint(fin);
  height = getint(fin);
  if (type == '2' || type == '5' ||
      type == '3' || type == '6')
    max = getint(fin);
  else
    max = 1;
  for (planes = 0, v = max; v; planes++, v >>= 1) ;
  bpp = (planes + 7) / 8;
  if (encode_planes < 0 || encode_planes > planes)
    encode_planes = planes;
  fgetc(fin);    /* skip line feed */

  /* read PBM image data */
  bpl = (width + 7) / 8;     /* bytes per line */
  bitmap_size = bpl * (size_t) height;
  bitmap = (unsigned char **) checkedmalloc(sizeof(unsigned char *) *
					    encode_planes);
  for (i = 0; i < encode_planes; i++)
    bitmap[i] = (unsigned char *) checkedmalloc(bitmap_size);
  switch (type) {
  case '1':
    /* PBM text format */
    p = bitmap[0];
    for (y = 0; y < height; y++)
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
    fread(bitmap[0], bitmap_size, 1, fin);
    break;
  case '2':
  case '5':
    /* PGM */
    image = (unsigned char *) checkedmalloc(width * height * bpp);
    if (type == '2') {
      for (x = 0; x < width * height; x++) {
	v = getint(fin);
	for (j = 0; j < bpp; j++)
	  image[x * bpp + (bpp - 1) - j] = v >> (j * 8);
      }
    } else
      fread(image, width * height, bpp, fin);
    jbg_split_planes(width, height, planes, encode_planes, image, bitmap,
		     use_graycode);
    free(image);
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

  /* Test for valid parameters */
  if (width < 1 || height < 1) {
    fprintf(stderr, "Image dimensions must be positive!\n");
    exit(1);
  }
  if (encode_planes < 1 || encode_planes > 255) {
    fprintf(stderr, "Number of planes must be in range 1-255!\n");
    exit(1);
  }

  /* Test the final byte in each image line for correct zero padding */
  if ((width & 7) && type == '4') {
    for (y = 0; y < height; y++)
      if (bitmap[0][y * bpl + bpl - 1] & ((1 << (8 - (width & 7))) - 1)) {
	fprintf(stderr, "Warning: No zero padding in last byte (0x%02x) of "
		"line %lu!\n", bitmap[0][y * bpl + bpl - 1], y + 1);
	break;
      }
  }

  /* Apply JBIG algorithm and write BIE to output file */

  /* initialize parameter struct for JBIG encoder*/
  jbg_enc_init(&s, width, height, encode_planes, bitmap, data_out, fout);

  /* Select number of resolution layers either directly or based
   * on a given maximum size for the lowest resolution layer */
   if (d >= 0)
    jbg_enc_layers(&s, d);
  else
    jbg_enc_lrlmax(&s, mwidth, mheight);

  /* Specify a few other options (each is ignored if negative) */
  if (delay_at)
    options |= JBG_DELAY_AT;
  if (reset)
    options |= JBG_SDRST;
  if (comment) {
    s.comment_len = strlen(comment);
    s.comment = (unsigned char *) comment;
  }
  if (y1)
    s.yd1 = y1;
  jbg_enc_lrange(&s, dl, dh);
  jbg_enc_options(&s, order, options, l0, mx, -1);

  /* now encode everything and send it to data_out() */
  jbg_enc_out(&s);

  /* give encoder a chance to free its temporary data structures */
  jbg_enc_free(&s);

  /* check for file errors and close fout */
  if (ferror(fout) || fclose(fout)) {
    fprintf(stderr, "Problem while writing output file '%s", fnout);
    perror("'");
    exit(1);
  }

  /* In case the user wants to know all the gory details ... */
  if (verbose) {
    fprintf(stderr, "Information about the created JBIG bi-level image entity "
	    "(BIE):\n\n");
    fprintf(stderr, "              input image size: %lu x %lu pixel\n",
	    s.xd, s.yd);
    fprintf(stderr, "                    bit planes: %d\n", s.planes);
    if (s.planes > 1)
      fprintf(stderr, "                      encoding: %s code, MSB first\n",
	      use_graycode ? "Gray" : "binary");
    fprintf(stderr, "                       stripes: %lu\n", s.stripes);
    fprintf(stderr, "   lines per stripe in layer 0: %lu\n", s.l0);
    fprintf(stderr, "  total number of diff. layers: %d\n", s.d);
    fprintf(stderr, "           lowest layer in BIE: %d\n", s.dl);
    fprintf(stderr, "          highest layer in BIE: %d\n", s.dh);
    fprintf(stderr, "             lowest layer size: %lu x %lu pixel\n",
	    jbg_ceil_half(s.xd, s.d - s.dl), jbg_ceil_half(s.yd, s.d - s.dl));
    fprintf(stderr, "            highest layer size: %lu x %lu pixel\n",
	    jbg_ceil_half(s.xd, s.d - s.dh), jbg_ceil_half(s.yd, s.d - s.dh));
    fprintf(stderr, "                   option bits:%s%s%s%s%s%s%s\n",
	    s.options & JBG_LRLTWO  ? " LRLTWO" : "",
	    s.options & JBG_VLENGTH ? " VLENGTH" : "",
	    s.options & JBG_TPDON   ? " TPDON" : "",
	    s.options & JBG_TPBON   ? " TPBON" : "",
	    s.options & JBG_DPON    ? " DPON" : "",
	    s.options & JBG_DPPRIV  ? " DPPRIV" : "",
	    s.options & JBG_DPLAST  ? " DPLAST" : "");
    fprintf(stderr, "                    order bits:%s%s%s%s\n",
	    s.order & JBG_HITOLO ? " HITOLO" : "",
	    s.order & JBG_SEQ    ? " SEQ" : "",
	    s.order & JBG_ILEAVE ? " ILEAVE" : "",
	    s.order & JBG_SMID   ? " SMID" : "");
    fprintf(stderr, "           AT maximum x-offset: %d\n"
	    "           AT maximum y-offset: %d\n", s.mx, s.my);
    fprintf(stderr, "         length of output file: %lu byte\n\n",
	    total_length);
  }

  return 0;
}
