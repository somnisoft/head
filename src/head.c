/**
 * @file
 * @brief head utility
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef TEST
/**
 * Declare some functions with extern linkage, allowing the test suite to call
 * those functions.
 */
# define LINKAGE extern
# include "../test/seams.h"
#else /* !(TEST) */
/**
 * Define all functions as static when not testing.
 */
# define LINKAGE static
#endif /* TEST */

/**
 * Default number of lines to print if -n argument unspecified.
 */
#define HEAD_DEFAULT_LINES (10)

/**
 * Head utility context.
 */
struct head{
  /**
   * Exit status set to one of the following values.
   *   - EXIT_SUCCESS
   *   - EXIT_FAILURE
   */
  int status_code;

  /**
   * Padding for alignment.
   */
  char pad[4];

  /**
   * Number of initial lines to write in each file.
   *
   * Corresponds to the (-n) argument.
   */
  uintmax_t nlines;

  /**
   * getline() line buffer.
   */
  char *line;

  /**
   * Number of bytes allocated in @ref line.
   */
  size_t linesize;
};

/**
 * Print an error message to STDERR and set an error status code.
 *
 * @param[in,out] head      See @ref head.
 * @param[in]     errno_msg Include a standard message describing errno.
 * @param[in]     fmt       Format string used by vwarn.
 */
static void
head_warn(struct head *const head,
          const bool errno_msg,
          const char *const fmt, ...){
  va_list ap;

  head->status_code = EXIT_FAILURE;
  va_start(ap, fmt);
  if(errno_msg){
    vwarn(fmt, ap);
  }
  else{
    vwarnx(fmt, ap);
  }
  va_end(ap);
}

/**
 * Print head lines from file pointer.
 *
 * @param[in,out] head See @ref head.
 * @param[in,out] fp   File pointer to read from.
 */
static void
head_fp(struct head *const head,
        FILE *fp){
  uintmax_t i;
  ssize_t linelen;
  size_t nmemb;

  for(i = 0; i < head->nlines; i++){
    errno = 0;
    linelen = getline(&head->line, &head->linesize, fp);
    if(linelen < 0){
      if(errno != 0){
        head_warn(head, true, "getline");
      }
      break;
    }
    /* linelen >= 0 */
    nmemb = (size_t)linelen;
    if(fwrite(head->line, sizeof(*head->line), nmemb, stdout) != nmemb){
      head_warn(head, true, "fwrite: %s", &head->line);
      break;
    }
  }
  if(ferror(fp) || ferror(stdout)){
    head_warn(head, true, "ferror: file error indicator set");
  }
}

/**
 * Open a file path and call @ref head_fp.
 *
 * @param[in,out] head See @ref head.
 * @param[in]     path File path.
 */
static void
head_path(struct head *const head,
          const char *const path){
  FILE *fp;

  fp = fopen(path, "r");
  if(fp == NULL){
    head_warn(head, true, "fopen: %s", path);
  }
  else{
    head_fp(head, fp);
    if(fclose(fp) != 0){
      head_warn(head, true, "fclose: %s", path);
    }
  }
}

/**
 * Parse number of lines to print.
 *
 * Corresponds to the (-n) argument.
 *
 * @param[in,out] head See @ref head.
 * @param[in]     s    String to parse.
 */
static void
head_parse_nlines(struct head *const head,
                  const char *const s){
  char *ep;

  errno = 0;
  head->nlines = strtoumax(s, &ep, 10);
  if(s[0] == '\0' || *ep != '\0'){
    head_warn(head, false, "not a number: %s", s);
  }
  else if(head->nlines == UINTMAX_MAX && errno == ERANGE){
    head_warn(head, true, "out of range: %s", s);
  }
}

/**
 * Main entry point for head program.
 *
 * Usage:
 * head [-n number] [file...]
 *
 * @param[in]     argc         Number of arguments in @p argv.
 * @param[in,out] argv         Argument list.
 * @retval        EXIT_SUCCESS Successful.
 * @retval        EXIT_FAILURE Error occurred.
 */
LINKAGE int
head_main(int argc,
          char *argv[]){
  int c;
  int i;
  struct head head;

  memset(&head, 0, sizeof(head));
  head.nlines = HEAD_DEFAULT_LINES;
  while((c = getopt(argc, argv, "n:")) != -1){
    switch(c){
      case 'n':
        head_parse_nlines(&head, optarg);
        break;
      default:
        head.status_code = EXIT_FAILURE;
        break;
    }
  }
  argc -= optind;
  argv += optind;

  if(head.status_code == 0){
    if(argc < 1){
      head_fp(&head, stdin);
    }
    else{
      for(i = 0; i < argc && head.status_code == 0; i++){
        if(argc > 1){
          if(i > 0){
            if(putchar('\n') != '\n'){
              head_warn(&head, true, "putchar: <NL>");
            }
          }
          if(printf("==> %s <==\n", argv[i]) < 0){
            head_warn(&head, true, "printf: file header");
          }
        }
        head_path(&head, argv[i]);
      }
    }
    free(head.line);
  }
  return head.status_code;
}

#ifndef TEST
/**
 * Main program entry point.
 *
 * @param[in]     argc See @ref head_main.
 * @param[in,out] argv See @ref head_main.
 * @return             See @ref head_main.
 */
int
main(int argc,
     char *argv[]){
  return head_main(argc, argv);
}
#endif /* TEST */

