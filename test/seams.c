/**
 * @file
 * @brief Test seams
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "test.h"

/**
 * Error counter for @ref test_seam_fclose.
 */
int g_test_seam_err_ctr_fclose = -1;

/**
 * Error counter for @ref test_seam_ferror.
 */
int g_test_seam_err_ctr_ferror = -1;

/**
 * Error counter for @ref test_seam_fwrite.
 */
int g_test_seam_err_ctr_fwrite = -1;

/**
 * Error counter for @ref test_seam_getline.
 */
int g_test_seam_err_ctr_getline = -1;

/**
 * Error counter for @ref test_seam_printf.
 */
int g_test_seam_err_ctr_printf = -1;

/**
 * Error counter for @ref test_seam_putchar.
 */
int g_test_seam_err_ctr_putchar = -1;

/**
 * Decrement an error counter until it reaches -1.
 *
 * After a counter reaches -1, it will return a true response. This gets
 * used by the test suite to denote when to cause a function to fail. For
 * example, the unit test might need to cause the malloc() function to fail
 * after calling it a third time. In that case, the counter should initially
 * get set to 2 and will get decremented every time this function gets called.
 *
 * @param[in,out] err_ctr Error counter to decrement.
 * @retval        true    The counter has reached -1.
 * @retval        false   The counter has been decremented, but did not reach
 *                        -1 yet.
 */
static bool
test_seam_dec_err_ctr(int *const err_ctr){
  bool reached_end;

  reached_end = false;
  if(*err_ctr >= 0){
    *err_ctr -= 1;
    if(*err_ctr < 0){
      reached_end = true;
    }
  }
  return reached_end;
}

/**
 * Control when fclose() fails.
 *
 * @param[in,out] stream File pointer.
 * @retval        0      Successfully closed file.
 * @retval        EOF    Failed to close file.
 */
int
test_seam_fclose(FILE *stream){
  int rc;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_fclose)){
    rc = EOF;
    errno = EBADF;
  }
  else{
    rc = fclose(stream);
  }
  return rc;
}

/**
 * Control when ferror() fails.
 *
 * @param[in] stream File pointer.
 * @retval    0      File error indicator not set.
 * @retval    !0     File error indicator set.
 */
int
test_seam_ferror(FILE *stream){
  int rc;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_ferror)){
    rc = 1;
  }
  else{
    rc = ferror(stream);
  }
  return rc;
}

/**
 * Control when fwrite() fails.
 *
 * @param[in]     ptr    Data to write to @p stream.
 * @param[in]     size   Size of each element in @p nitems.
 * @param[in]     nitems Number of items to write.
 * @param[in,out] stream File pointer to write to.
 * @return               Number of @p items written to @p stream.
 */
size_t
test_seam_fwrite(const void *ptr,
                 size_t size,
                 size_t nitems,
                 FILE *stream){
  size_t nwrite;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_fwrite)){
    nwrite = 0;
    errno = EPIPE;
  }
  else{
    nwrite = fwrite(ptr, size, nitems, stream);
  }
  return nwrite;
}

/**
 * Control when getline() fails.
 *
 * @param[in,out] lineptr Line buffer that may get reallocated.
 * @param[in,out] n       Number of bytes allocated in @p lineptr.
 * @param[in,out] stream  File pointer to read from.
 * @retval        >=0     Number of bytes read.
 * @retval        -1      Error occurred.
 */
ssize_t
test_seam_getline(char **lineptr,
                  size_t *n,
                  FILE *stream){
  ssize_t linelen;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_getline)){
    linelen = -1;
    errno = ENOMEM;
  }
  else{
    linelen = getline(lineptr, n, stream);
  }
  return linelen;
}

/**
 * Control when printf() fails.
 *
 * @param[in] format Format argument list to print.
 * @retval    >=0    Number of bytes written.
 * @retval    <0     Output error.
 */
int
test_seam_printf(const char *format, ...){
  int bytes_written;
  va_list ap;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_printf)){
    bytes_written = -1;
    errno = ENOMEM;
  }
  else{
    va_start(ap, format);
    bytes_written = vprintf(format, ap);
    va_end(ap);
  }
  return bytes_written;
}

/**
 * Control when putchar() fails.
 *
 * @param[in] c   Byte to write.
 * @retval    c   Successfully wrote @p c.
 * @retval    EOF Error occurred.
 */
int
test_seam_putchar(int c){
  int byte_written;

  if(test_seam_dec_err_ctr(&g_test_seam_err_ctr_putchar)){
    byte_written = EOF;
    errno = ENOMEM;
  }
  else{
    byte_written = putchar(c);
  }
  return byte_written;
}

