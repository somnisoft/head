/**
 * @file
 * @brief Test suite
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */
#ifndef HEAD_TEST_H
#define HEAD_TEST_H

#include <stdio.h>

int
head_main(int argc,
          char *argv[]);

int
test_seam_fclose(FILE *stream);

int
test_seam_ferror(FILE *stream);

size_t
test_seam_fwrite(const void *ptr,
                 size_t size,
                 size_t nitems,
                 FILE *stream);

ssize_t
test_seam_getline(char **lineptr,
                  size_t *n,
                  FILE *stream);

int
test_seam_printf(const char *format, ...);

int
test_seam_putchar(int c);

extern int g_test_seam_err_ctr_fclose;
extern int g_test_seam_err_ctr_ferror;
extern int g_test_seam_err_ctr_fwrite;
extern int g_test_seam_err_ctr_getline;
extern int g_test_seam_err_ctr_printf;
extern int g_test_seam_err_ctr_putchar;

#endif /* HEAD_TEST_H */

