/**
 * @file
 * @brief Test seams
 * @author James Humphrey (humphreyj@somnisoft.com)
 *
 * This software has been placed into the public domain using CC0.
 */
#ifndef HEAD_TEST_SEAMS_H
#define HEAD_TEST_SEAMS_H

#include "test.h"

/*
 * Redefine these functions to internal test seams.
 */
#undef fclose
#undef ferror
#undef fwrite
#undef getline
#undef printf
#undef putchar

/**
 * Inject a test seam to replace fclose().
 */
#define fclose test_seam_fclose

/**
 * Inject a test seam to replace ferror().
 */
#define ferror test_seam_ferror

/**
 * Inject a test seam to replace fwrite().
 */
#define fwrite test_seam_fwrite

/**
 * Inject a test seam to replace getline().
 */
#define getline test_seam_getline

/**
 * Inject a test seam to replace printf().
 */
#define printf test_seam_printf

/**
 * Inject a test seam to replace putchar().
 */
#define putchar test_seam_putchar

#endif /* HEAD_TEST_SEAMS_H */

