/*
 *  A sequence of test procedures for this JBIG implementation
 * 
 *  Run this test sequence after each modification on the JBIG library.
 *
 *  Markus Kuhn -- http://www.cl.cam.ac.uk/~mgk25/
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#include "jbig85.h"

#define TESTBUF_SIZE 400000L
#define TESTPIC_SIZE 477995L

#define FAILED "F\bFA\bAI\bIL\bLE\bED\bD"
#define PASSED "PASSED"

unsigned char *testbuf;
unsigned char *testpic;

long testbuf_len;


static void *checkedmalloc(size_t n)
{
  void *p;
  
  if ((p = calloc(1, n)) == NULL) {
    fprintf(stderr, "Sorry, not enough memory available!\n");
    exit(1);
  }
  
  return p;
}

static void testbuf_write(int v, void *dummy)
{
  if (testbuf_len < TESTBUF_SIZE)
    testbuf[testbuf_len++] = v;
  (void) dummy;
  return;
}


static void testbuf_writel(unsigned char *start, size_t len, void *dummy)
{
  if (testbuf_len < TESTBUF_SIZE) {
    if (testbuf_len + len < TESTBUF_SIZE)
      memcpy(testbuf + testbuf_len, start, len);
    else
      memcpy(testbuf + testbuf_len, start, TESTBUF_SIZE - testbuf_len);
  }
  testbuf_len += len;

#ifdef DEBUG
  {
    unsigned char *p;
    unsigned sum = 0;
    
    for (p = start; p - start < (ptrdiff_t) len; sum = (sum ^ *p++) << 1);
    printf("  testbuf_writel: %4lu bytes, checksum %04x\n",
	   (unsigned long) len, sum & 0xffff);
  }
#endif

  (void) dummy;
  return;
}


static int line_out(const struct jbg85_dec_state *s,
		    unsigned char *start, size_t len,
		    unsigned long y, void *bitmap)
{
  assert(jbg85_dec_validwidth(s));
  assert(len == (jbg85_dec_getwidth(s) >> 3) + !!(jbg85_dec_getwidth(s) & 7));
  assert(y < jbg85_dec_getheight(s));
  memcpy((unsigned char *) bitmap + len * y, start, len);
  return 0;
}


/*
 * Store the artificial test image defined in T.82, clause 7.2.1 at
 * pic. The image requires 477995 bytes of memory, is 1960 x 1951 pixels
 * large and has one plane.
 */ 
static void testimage(unsigned char *pic)
{
  unsigned long i, j, sum;
  unsigned int prsg, repeat[8];
  unsigned char *p;
  
  memset(pic, 0, TESTPIC_SIZE);
  p = pic;
  prsg = 1;
  for (j = 0; j < 1951; j++)
    for (i = 0; i < 1960; i++) {
      if (j >= 192) {
	if (j < 1023 || ((i >> 3) & 3) == 0) {
	  sum = (prsg & 1) + ((prsg >> 2) & 1) + ((prsg >> 11) & 1) +
	    ((prsg >> 15) & 1);
	  prsg = (prsg << 1) + (sum & 1);
	  if ((prsg & 3) == 0) {
	    *p |= 1 << (7 - (i & 7));
	    repeat[i & 7] = 1;
	  } else {
	    repeat[i & 7] = 0;
	  }
	} else {
	  if (repeat[i & 7])
	    *p |= 1 << (7 - (i & 7));
	}
      }
      if ((i & 7) == 7) ++p;
    }

  /* verify test image */
  sum = 0;
  for (i = 0; i < TESTPIC_SIZE; i++)
    for (j = 0; j < 8; j++)
      sum += (pic[i] >> j) & 1;
  if (sum != 861965L)
    printf("WARNING: Artificial test image has %lu (not 861965) "
	   "foreground pixels!\n", sum);

  return;
}
  

/*
 * Perform a full test cycle with one set of parameters. Encode an image
 * and compare the length of the result with correct_length. Then decode
 * the image again both in one single chunk or byte by byte and compare
 * the results with the original input image.
 */
static int test_cycle(unsigned char *orig_image, int width, int height,
		      int options, unsigned long l0, int mx,
		      long correct_length, const char *test_id)
{
  struct jbg85_enc_state sje;
  struct jbg85_dec_state sjd;
  int trouble = 0;
  long l;
  size_t plane_size, buffer_len;
  int i, result;
  unsigned char *image, *buffer;
  size_t bpl;
  size_t cnt;

  bpl = (width + 7) / 8;
  plane_size = bpl * height;
  image = (unsigned char *) checkedmalloc(plane_size);
  memcpy(image, orig_image, plane_size);
  
  printf("\nTest-85 %s.1: Encoding ...\n", test_id);
  testbuf_len = 0;
  jbg85_enc_init(&sje, width, height, testbuf_writel, NULL);
  jbg85_enc_options(&sje, options, l0, mx);
  for (i = 0; i < height; i++)
    jbg85_enc_lineout(&sje,
		      image + i * bpl,
		      image + (i-1) * bpl,
		      image + (i-2) * bpl);
  free(image);
  printf("Encoded BIE has %6ld bytes: ", testbuf_len);
  if (correct_length >= 0)
    if (testbuf_len == correct_length)
      puts(PASSED);
    else {
      trouble++;
      printf(FAILED ", correct would have been %ld\n", correct_length);
    }
  else
    puts("");
  
#if 1
  buffer_len = ((width >> 3) + !!(width & 7)) * 3;
  buffer = (unsigned char *) checkedmalloc(buffer_len);
  image = (unsigned char *) checkedmalloc(plane_size);
  printf("Test-85 %s.2: Decoding whole chunk ...\n", test_id);
  jbg85_dec_init(&sjd, buffer, buffer_len, line_out, image);
  result = jbg85_dec_in(&sjd, testbuf, testbuf_len, &cnt);
  if (result != JBG_EOK) {
    printf("Decoder complained with return value 0x%02x: "
	   FAILED "\nCause: '%s'\n", result, jbg85_strerror(result));
    printf("%ld bytes of BIE read, %lu lines decoded.\n",
	   (long) cnt, sjd.y);
    trouble++;
  } else {
    printf("Image comparison: ");
    result = 1;
    if (memcmp(orig_image, image, plane_size)) {
      result = 0;
      trouble++;
      printf(FAILED);
    }
    if (result)
      puts(PASSED);
  }
  free(image);

  image = (unsigned char *) checkedmalloc(plane_size);
  printf("Test-85 %s.3: Decoding with single-byte feed ...\n", test_id);
  jbg85_dec_init(&sjd, buffer, buffer_len, line_out, image);
  result = JBG_EAGAIN;
  for (l = 0; l < testbuf_len; l++) {
    result = jbg85_dec_in(&sjd, testbuf + l, 1, NULL);
    if (l < testbuf_len - 1 && result != JBG_EAGAIN) {
      printf("Decoder complained with return value 0x%02x at byte %ld: "
	     FAILED "\nCause: '%s'\n", result, l, jbg85_strerror(result));
      trouble++;
      break;
    }
  }
  if (l == testbuf_len) {
    if (result != JBG_EOK) {
      printf("Decoder complained with return value 0x%02x at final byte: "
	     FAILED "\nCause: '%s'\n", result, jbg85_strerror(result));
      trouble++;
    } else {
      printf("Image comparison: ");
      result = 1;
      if (memcmp(orig_image, image, plane_size)) {
	result = 0;
	trouble++;
	printf(FAILED);
      }
      if (result)
	puts(PASSED);
    }
  }
  free(image);

#endif
  puts("");
  
  return trouble != 0;
}


int main(int argc, char **argv)
{
  int trouble, problems = 0;
  struct jbg_arenc_state *se;
  struct jbg_ardec_state *sd;
  long i;
  int pix;
  unsigned char *pp;

  int t82pix[16] = {
    0x05e0, 0x0000, 0x8b00, 0x01c4, 0x1700, 0x0034, 0x7fff, 0x1a3f,
    0x951b, 0x05d8, 0x1d17, 0xe770, 0x0000, 0x0000, 0x0656, 0x0e6a
  };
  int t82cx[16] = {
    0x0fe0, 0x0000, 0x0f00, 0x00f0, 0xff00, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  };
  unsigned char t82sde[32] = {
    0x69, 0x89, 0x99, 0x5c, 0x32, 0xea, 0xfa, 0xa0,
    0xd5, 0xff, 0x00, 0x52, 0x7f, 0xff, 0x00, 0xff,
    0x00, 0xff, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x3f,
    0xff, 0x00, 0x2d, 0x20, 0x82, 0x91, 0xff, 0x02
  };

  printf("\nAutomatic JBIG Compatibility Test Suite\n"
	 "---------------------------------------\n\n"
	 "JBIG-KIT Version " JBG85_VERSION " (T.85 version)"
	 " -- This test may take a few minutes.\n\n\n");

  /* allocate test buffer memory */
  testbuf = (unsigned char *) checkedmalloc(TESTBUF_SIZE);
  testpic = (unsigned char *) checkedmalloc(TESTPIC_SIZE);
  se = (struct jbg_arenc_state *) checkedmalloc(sizeof(struct jbg_arenc_state));
  sd = (struct jbg_ardec_state *) checkedmalloc(sizeof(struct jbg_ardec_state));

  /* only supported command line option:
   * output file name for exporting test image */
  if (argc > 1) {
    FILE *f;
    
    puts("Generating test image ...");
    testimage(testpic);
    printf("Storing in '%s' ...\n", argv[1]);
    
    /* write out test image as PBM file */
    f = fopen(argv[1], "wb");
    if (!f) abort();
    fprintf(f, "P4\n");
#if 0
    fprintf(f, "# Test image as defined in ITU-T T.82, clause 7.2.1\n");
#endif
    fprintf(f, "1960 1951\n");
    fwrite(testpic, 1, TESTPIC_SIZE, f);
    fclose(f);
    exit(0);
  }

#if 1
  puts("1) Arithmetic encoder test sequence from ITU-T T.82, clause 7.1\n"
       "---------------------------------------------------------------\n");
  arith_encode_init(se, 0);
  testbuf_len = 0;
  se->byte_out = testbuf_write;
  for (i = 0; i < 16 * 16; i++)
    arith_encode(se, (t82cx[i >> 4] >> ((15 - i) & 15)) & 1,
		 (t82pix[i >> 4] >> ((15 - i) & 15)) & 1);
  arith_encode_flush(se);
  printf("result of encoder:\n  ");
  for (i = 0; i < testbuf_len && i < TESTBUF_SIZE; i++)
    printf("%02x", testbuf[i]);
  printf("\nexpected result:\n  ");
  for (i = 0; i < 30; i++)
    printf("%02x", t82sde[i]);
  printf("\n\nTest 1: ");
  if (testbuf_len != 30 || memcmp(testbuf, t82sde, 30)) {
    problems++;
    printf(FAILED);
  } else
    printf(PASSED);
  printf("\n\n");


  puts("2) Arithmetic decoder test sequence from ITU-T T.82, clause 7.1\n"
       "---------------------------------------------------------------\n");
  printf("Test 2.1: Decoding whole chunk ...\n");
  arith_decode_init(sd, 0);
  sd->pscd_ptr = t82sde;
  sd->pscd_end = t82sde + 32;
  trouble = 0;
  for (i = 0; i < 16 * 16 && !trouble; i++) {
    pix = arith_decode(sd, (t82cx[i >> 4] >> ((15 - i) & 15)) & 1);
    if (pix < 0) {
      printf("Problem at pixel %ld, byte %ld.\n\n",
	     i+1, (long) (sd->pscd_ptr - sd->pscd_end));
      trouble++;
      break;
    }
    if (pix != ((t82pix[i >> 4] >> ((15 - i) & 15)) & 1)) {
      printf("Wrong PIX answer (%d) at pixel %ld.\n\n", pix, i+1);
      trouble++;
      break;
    }
  }
  if (!trouble && sd->pscd_ptr != sd->pscd_end - 2) {
    printf("%ld bytes left after decoder finished.\n\n",
	   (long) (sd->pscd_end - sd->pscd_ptr - 2));
    trouble++;
  }
  printf("Test result: ");
  if (trouble) {
    problems++;
    puts(FAILED);
  } else
    puts(PASSED);
  printf("\n");

  printf("Test 2.2: Decoding with single byte feed ...\n");
  arith_decode_init(sd, 0);
  pp = t82sde;
  sd->pscd_ptr = pp;
  sd->pscd_end = pp + 1;
  trouble = 0;
  for (i = 0; i < 16 * 16 && !trouble; i++) {
    pix = arith_decode(sd, (t82cx[i >> 4] >> ((15 - i) & 15)) & 1);
    while (pix < 0 && sd->pscd_end < t82sde + 32) {
      pp++;
      if (sd->pscd_ptr != pp - 1)
	sd->pscd_ptr = pp;
      sd->pscd_end = pp + 1;
      pix = arith_decode(sd, (t82cx[i >> 4] >> ((15 - i) & 15)) & 1);
    }
    if (pix < 0) {
      printf("Problem at pixel %ld, byte %ld.\n\n",
	     i+1, (long) (sd->pscd_ptr - sd->pscd_end));
      trouble++;
      break;
    }
    if (pix != ((t82pix[i >> 4] >> ((15 - i) & 15)) & 1)) {
      printf("Wrong PIX answer (%d) at pixel %ld.\n\n", pix, i+1);
      trouble++;
      break;
    }
  }
  if (!trouble && sd->pscd_ptr != sd->pscd_end - 2) {
    printf("%ld bytes left after decoder finished.\n\n",
	   (long) (sd->pscd_end - sd->pscd_ptr - 2));
    trouble++;
  }
  printf("Test result: ");
  if (trouble) {
    problems++;
    puts(FAILED);
  } else
    puts(PASSED);
  printf("\n");
  
  puts("3) Parametric algorithm test sequence from ITU-T T.82, clause 7.2\n"
       "-----------------------------------------------------------------\n");
  puts("Generating test image ...");
  testimage(testpic);
  putchar('\n');

  puts("Test-85 3.1: TPBON=0, Mx=0, LRLTWO=0, L0=1951, 0 layers");
  problems += test_cycle(testpic, 1960, 1951, 0,
			 1951, 0, 317384L, "3.1");
  puts("Test-85 3.2: TPBON=0, Mx=0, LRLTWO=1, L0=1951, 0 layers");
  problems += test_cycle(testpic, 1960, 1951, JBG_LRLTWO,
			 1951, 0, 317132L, "3.2");
  puts("Test-85 3.3: TPBON=1, Mx=8, LRLTWO=0, L0=128, 0 layers");
  problems += test_cycle(testpic, 1960, 1951, JBG_TPBON,
			 128, 8, 253653L, "3.3");
#endif

#if 0
  puts("4) Same T.82 tests with SDRST instead of SDNORM\n"
       "-----------------------------------------------\n");

  puts("Test-85 4.0: TPBON=1, Mx=8, LRLTWO=0, L0=128, 0 layers");
  problems += test_cycle(&pp, 1960, 1951, JBG_SDRST | JBG_TPBON,
			 128, 8, -1, "4.0");

  puts("Test-85 4.1: TPBON=0, Mx=0, LRLTWO=0, L0=1951, 0 layers");
  problems += test_cycle(&pp, 1960, 1951, JBG_SDRST,
			 1951, 0, -1, "4.1");
  puts("Test-85 4.2: TPBON=0, Mx=0, LRLTWO=1, L0=1951, 0 layers");
  problems += test_cycle(&pp, 1960, 1951, JBG_LRLTWO | JBG_SDRST,
			 1951, 0, -1, "4.2");
  puts("Test-85 4.3: TPBON=1, Mx=8, LRLTWO=0, L0=128, 0 layers");
  problems += test_cycle(&pp, 1960, 1951, JBG_TPBON | JBG_SDRST,
			 128, 8, -1, "4.3");
#endif

  printf("\nTest result summary: the T.85 library has %s the test suite.\n\n",
	 problems ? FAILED : PASSED);
  if (problems)
    puts("This is bad. If you cannot identify the problem yourself, please "
	 "send\nthis output plus a detailed description of your "
	 "compile environment\n(OS, compiler, version, options, etc.) to "
	 "Markus Kuhn\n<http://www.cl.cam.ac.uk/~mgk25/>.");
  else
    puts("Congratulations, everything is fine.\n");

  return problems != 0;
}
