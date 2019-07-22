/**
 * @file
 * @brief Test suite
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"

/**
 * Generate the output of the head command to this file.
 */
#define PATH_TMP_FILE "build/test-head.txt"

/**
 * Number of arguments in @ref g_argv.
 */
static int
g_argc;

/**
 * Argument list to pass to utility.
 */
static char **
g_argv;

/**
 * Call @ref head_main with the given arguments.
 *
 * @param[in] nlines             Number of initial lines to write.
 * @param[in] stdin_bytes        Transmit these bytes to STDIN of child
 *                               process.
 * @param[in] stdin_bytes_len    Number of bytes in @p stdin_bytes.
 * @param[in] expect_ref_file    Reference file containing expected head
 *                               output.
 * @param[in] expect_exit_status Expected exit status code.
 * @param[in] file_list          List of files to print.
 */
static void
test_head_main(const char *const nlines,
               const char *const stdin_bytes,
               const size_t stdin_bytes_len,
               const char *const expect_ref_file,
               const int expect_exit_status,
               const char *const file_list, ...){
  va_list ap;
  const char *file;
  ssize_t bytes_written;
  int exit_status;
  pid_t pid;
  FILE *new_stdout;
  int status;
  int cmp_exit_status;
  int pipe_stdin[2];
  char cmp_cmd[1000];

  g_argc = 1;
  if(nlines){
    strcpy(g_argv[g_argc++], "-n");
    strcpy(g_argv[g_argc++], nlines);
  }

  va_start(ap, file_list);
  for(file = file_list; file; file = va_arg(ap, const char *const)){
    strcpy(g_argv[g_argc++], file);
  }
  va_end(ap);

  errno = 0;
  assert(remove(PATH_TMP_FILE) == 0 || errno == ENOENT);

  if(stdin_bytes){
    assert(pipe(pipe_stdin) == 0);
  }

  pid = fork();
  assert(pid >= 0);
  if(pid == 0){
    if(stdin_bytes){
      assert(close(pipe_stdin[1]) == 0);
      assert(dup2(pipe_stdin[0], STDIN_FILENO) >= 0);
      assert(close(pipe_stdin[0]) == 0);
    }
    new_stdout = freopen(PATH_TMP_FILE, "w", stdout);
    assert(new_stdout);
    exit_status = head_main(g_argc, g_argv);
    exit(exit_status);
  }
  if(stdin_bytes){
    assert(close(pipe_stdin[0]) == 0);
    bytes_written = write(pipe_stdin[1], stdin_bytes, stdin_bytes_len);
    assert(bytes_written >= 0 && (size_t)bytes_written == stdin_bytes_len);
    assert(close(pipe_stdin[1]) == 0);
  }
  assert(waitpid(pid, &status, 0) == pid);
  assert(WIFEXITED(status));
  assert(WEXITSTATUS(status) == expect_exit_status);
  if(expect_ref_file){
    sprintf(cmp_cmd, "cmp '%s' '%s'", PATH_TMP_FILE, expect_ref_file);
    cmp_exit_status = system(cmp_cmd);
    assert(cmp_exit_status == 0);
  }
}

/**
 * Test different failure scenarios.
 */
static void
test_all_errors(void){
  int i;

  /* Invalid argument. */
  test_head_main(NULL, NULL, 0, NULL, EXIT_FAILURE, "-z", NULL);

  /* Blank nlines. */
  test_head_main("", NULL, 0, NULL, EXIT_FAILURE, NULL);

  /* Invalid nlines character. */
  test_head_main("9f", NULL, 0, NULL, EXIT_FAILURE, NULL);

  /* nlines too large (assume unsigned 64 bit). */
  test_head_main("18446744073709551616", NULL, 0, NULL, EXIT_FAILURE, NULL);

  /* File does not exist. */
  test_head_main(NULL, NULL, 0, NULL, EXIT_FAILURE, "/noexist.txt", NULL);

  /* Failed to getline. */
  g_test_seam_err_ctr_getline = 0;
  test_head_main(NULL, NULL, 0, NULL, EXIT_FAILURE, "README.md", NULL);
  g_test_seam_err_ctr_getline = -1;

  /* Failed to fwrite file. */
  g_test_seam_err_ctr_fwrite = 0;
  test_head_main(NULL, NULL, 0, NULL, EXIT_FAILURE, "README.md", NULL);
  g_test_seam_err_ctr_fwrite = -1;

  /* File error indicator set. */
  for(i = 0; i < 2; i++){
    g_test_seam_err_ctr_ferror = i;
    test_head_main(NULL, NULL, 0, NULL, EXIT_FAILURE, "README.md", NULL);
    g_test_seam_err_ctr_ferror = -1;
  }

  /* Failed to fclose file. */
  g_test_seam_err_ctr_fclose = 0;
  test_head_main(NULL, NULL, 0, NULL, EXIT_FAILURE, "README.md", NULL);
  g_test_seam_err_ctr_fclose = -1;

  /* Failed to print banner. */
  g_test_seam_err_ctr_printf = 0;
  test_head_main(NULL,
                 NULL,
                 0,
                 NULL,
                 EXIT_FAILURE,
                 "README.md",
                 "README.md",
                 NULL);
  g_test_seam_err_ctr_printf = -1;

  /* Failed to print newline before banner. */
  g_test_seam_err_ctr_putchar = 0;
  test_head_main(NULL,
                 NULL,
                 0,
                 NULL,
                 EXIT_FAILURE,
                 "README.md",
                 "README.md",
                 NULL);
  g_test_seam_err_ctr_putchar = -1;

  /* First file does not exist. */
  test_head_main(NULL,
                 NULL,
                 0,
                 "test/files/comb-noexist-1.txt",
                 EXIT_FAILURE,
                 "/noexist.txt",
                 "test/files/1.txt",
                 NULL);

  /* Second file does not exist. */
  test_head_main(NULL,
                 NULL,
                 0,
                 "test/files/comb-1-noexist.txt",
                 EXIT_FAILURE,
                 "test/files/1.txt",
                 "/noexist.txt",
                 NULL);
}

/**
 * Run test cases where we send data through STDIN.
 */
static void
test_all_stdin(void){
  const char *const stdin_bytes =
  "1: line 1\n"
  "2: line 2\n"
  "3: line 3\n"
  "4: line 4\n"
  "5: line 5\n";
  size_t stdin_bytes_len;

  stdin_bytes_len = strlen(stdin_bytes);

  /* Use STDIN. */
  test_head_main(NULL,
                 stdin_bytes,
                 stdin_bytes_len,
                 "test/files/5.txt",
                 EXIT_SUCCESS,
                 NULL);

  /* Fail to read from STDIN. */
  g_test_seam_err_ctr_getline = 0;
  test_head_main(NULL,
                 stdin_bytes,
                 stdin_bytes_len,
                 NULL,
                 EXIT_FAILURE,
                 NULL);
  g_test_seam_err_ctr_getline = -1;

}

/**
 * Run all test cases for the head utility.
 */
static void
test_all(void){
  test_head_main(NULL,
                 NULL,
                 0,
                 "test/files/10.txt",
                 EXIT_SUCCESS,
                 "test/files/10.txt",
                 NULL);

  test_head_main(NULL,
                 NULL,
                 0,
                 "test/files/comb-5-1.txt",
                 EXIT_SUCCESS,
                 "test/files/5.txt",
                 "test/files/1.txt",
                 NULL);

  test_head_main(NULL,
                 NULL,
                 0,
                 "test/files/comb-1-10-1.txt",
                 EXIT_SUCCESS,
                 "test/files/1.txt",
                 "test/files/10.txt",
                 "test/files/1.txt",
                 NULL);

  /* Print out 5 out of 10 lines in the input file. */
  test_head_main("5",
                 NULL,
                 0,
                 "test/files/5.txt",
                 EXIT_SUCCESS,
                 "test/files/10.txt",
                 NULL);

  /* Print 0 lines from the input file. */
  test_head_main("0",
                 NULL,
                 0,
                 "test/files/0.txt",
                 EXIT_SUCCESS,
                 "test/files/10.txt",
                 NULL);

  /* Print only 10 lines with higer nlimit. */
  test_head_main("100",
                 NULL,
                 0,
                 "test/files/10.txt",
                 EXIT_SUCCESS,
                 "test/files/10.txt",
                 NULL);

  /* Combine two 5-line outputs from 10 line file. */
  test_head_main("5",
                 NULL,
                 0,
                 "test/files/comb-10-10_5.txt",
                 EXIT_SUCCESS,
                 "test/files/10.txt",
                 "test/files/10.txt",
                 NULL);

  /* 98 lines from random file. */
  test_head_main("98",
                 NULL,
                 0,
                 "build/test-rand.txt.98",
                 EXIT_SUCCESS,
                 "build/test-rand.txt",
                 NULL);

  /* Maximum allowed nlines value (assume unsigned 64 bit). */
  test_head_main("18446744073709551615",
                 NULL,
                 0,
                 "build/test-rand.txt",
                 EXIT_SUCCESS,
                 "build/test-rand.txt",
                 NULL);

  /* File without ending newline. */
  test_head_main(NULL,
                 NULL,
                 0,
                 "test/files/5-no-eol.txt",
                 EXIT_SUCCESS,
                 "test/files/5-no-eol.txt",
                 NULL);

  test_all_stdin();
  test_all_errors();
}

/**
 * Test head utility.
 *
 * Usage: test
 *
 * @retval 0 All tests passed.
 */
int
main(void){
  const size_t MAX_ARGS = 20;
  const size_t MAX_ARG_LEN = 255;
  size_t i;

  g_argv = malloc(MAX_ARGS * sizeof(g_argv));
  assert(g_argv);
  for(i = 0; i < MAX_ARGS; i++){
    g_argv[i] = malloc(MAX_ARG_LEN * sizeof(*g_argv));
    assert(g_argv[i]);
  }
  g_argc = 0;
  strcpy(g_argv[g_argc++], "head");
  test_all();
  for(i = 0; i < MAX_ARGS; i++){
    free(g_argv[i]);
  }
  free(g_argv);
  return 0;
}

