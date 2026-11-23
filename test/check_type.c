/* Test driver for type functions for guile-curl

   Copyright (c) 2014, 2021 Michael L. Gran

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../src/type.h"
#include "../src/func.h"
#include "../src/const.h"

int test_scm_is_handle(void)
{
  SCM handle = cl_easy_init();
  int ret = _scm_is_handle(handle);
  cl_easy_cleanup(handle);
  return (ret == 1);
}

int test_scm_is_not_handle(void)
{
  SCM handle = SCM_BOOL_T;
  int ret = _scm_is_handle(handle);
  return (ret == 0);
}

int test_equalp_handle(void)
{
  SCM handle = cl_easy_init();
  SCM ret = equalp_handle(handle, handle);
  return (scm_is_true (ret));
}

int test_not_equalp_handle(void)
{
  SCM handle1 = cl_easy_init();
  SCM handle2 = cl_easy_init();
  SCM ret = equalp_handle(handle1, handle2);
  return (scm_is_false (ret));
}

int test_not_free_handle(void)
{
  SCM handle = cl_easy_init();
  handle_post_t *x = _scm_to_handle(handle);
  int ret = (x != NULL);
  gc_free_handle(handle);
  return ret;
}

int test_free_handle(void)
{
  SCM handle = cl_easy_init();
  gc_free_handle(handle);
  handle_post_t *x = _scm_to_handle(handle);
  int ret = (x == NULL);
  return ret;
}

void test_double_free_handle(void)
{
  SCM handle = cl_easy_init();
  gc_free_handle(handle);
  gc_free_handle(handle);
  gc_free_handle(handle);
}

void test_free_handle_string_postfields(void)
{
  extern SCM cl_CURLOPT_POSTFIELDS;
  SCM handle = cl_easy_init();
  SCM str = scm_from_locale_string("abcdefghijklmnopqrstuvwxyz");
  SCM ret = cl_easy_setopt(handle, scm_variable_ref(cl_CURLOPT_POSTFIELDS), str, SCM_BOOL_F);
  gc_free_handle(handle);
}

void test_free_handle_bytevector_postfields(void)
{
  extern SCM cl_CURLOPT_POSTFIELDS;
  SCM handle = cl_easy_init();
  SCM bv = scm_c_make_bytevector (26);

  for (int i = 0; i <= 25; i ++)
    SCM_BYTEVECTOR_CONTENTS(bv)[i] = i + 'a';

  SCM ret = cl_easy_setopt(handle, scm_variable_ref(cl_CURLOPT_POSTFIELDS), bv, SCM_BOOL_F);
  gc_free_handle(handle);
}

void test_free_handle_slist_option (SCM option)
{
  SCM handle = cl_easy_init();
  SCM str_list = scm_list_2(scm_from_locale_string("foo"), scm_from_locale_string("bar"));
  SCM ret = cl_easy_setopt(handle, scm_variable_ref(option), str_list, SCM_BOOL_F);
  gc_free_handle(handle);
}

void test_free_handle_empty_slist_option (SCM option)
{
  SCM handle = cl_easy_init();
  SCM str_list = scm_list_n(SCM_UNDEFINED);
  SCM ret = cl_easy_setopt(handle, scm_variable_ref(option), str_list, SCM_BOOL_F);
  gc_free_handle(handle);
}

int test_cl_is_handle_p__handle (void)
{
  SCM handle = cl_easy_init();
  SCM ret = cl_is_handle_p(handle);
  printf("test that cl_is_handle_p returns #t when passed a handle: %d\n", scm_is_true(ret));
  return scm_is_true(ret);
}

int test_cl_is_handle_p__true (void)
{
  SCM handle = SCM_BOOL_T;
  SCM ret = cl_is_handle_p(handle);
  printf("test that cl_is_handle_p returns #f when not passed a handle: %d\n", scm_is_false(ret));
  return scm_is_false(ret);
}

int test_can_convert_to_slist__slist (void)
{
  SCM str_list = scm_list_2(scm_from_locale_string("foo"), scm_from_locale_string("bar"));
  int ret = _scm_can_convert_to_slist (str_list);
  printf("test that _scm_can_convert_to_slist returns 1 when passed a list of strings: %d\n", ret == 1);
  return ret == 1;
}

int test_can_convert_to_slist__empty_list (void)
{
  SCM str_list = scm_list_n(SCM_UNDEFINED);
  int ret = _scm_can_convert_to_slist (str_list);
  printf("test that _scm_can_convert_to_slist returns 1 when passed an empty list: %d\n", ret == 1);
  return ret == 1;
}

int test_can_convert_to_slist__list_of_integers (void)
{
  SCM str_list = scm_list_2(scm_from_int(1), scm_from_int(2));
  int ret = _scm_can_convert_to_slist (str_list);
  printf("test that _scm_can_convert_to_slist returns 0 when passed a list of integers: %d\n", ret == 0);
  return ret == 0;
}

int test_scm_convert_to_slist__slist (void)
{
  SCM str_list = scm_list_2(scm_from_locale_string("foo"), scm_from_locale_string("bar"));
  struct curl_slist *ret = _scm_convert_to_slist (str_list);
  printf("test that _scm_convert_to_slist returns an slist when passed a list of strings: %d\n", ret != NULL);
  return ret != NULL;
}

int test_scm_convert_to_slist__empty_list (void)
{
  SCM str_list = scm_list_n(SCM_UNDEFINED);
  struct curl_slist *ret = _scm_convert_to_slist (str_list);
  printf("test that _scm_convert_to_slist returns NULL when passed an empty list: %d\n", ret == NULL);
  return ret == NULL;
}

int test_scm_convert_to_slist__list_of_integers (void)
{
  SCM str_list = scm_list_2(scm_from_int(1), scm_from_int(2));
  struct curl_slist *ret = _scm_convert_to_slist (str_list);
  printf("test that _scm_convert_to_slist returns NULL when passed a list of integers: %d\n", ret == NULL);
  return ret == NULL;
}


/* Test driver.

   Each test runs under a catch, so that a Scheme error raised by the
   code under test is reported (and the remaining tests still run)
   instead of making Guile abort() with no explanation.  Output is
   unbuffered, so that everything printed before a crash still reaches
   the test log.  */

void cl_init (void);

typedef int (*test_fn) (SCM arg);

struct test_case
{
  const char *name;
  test_fn fn;
  SCM arg;
};

static SCM
run_test_body (void *data)
{
  struct test_case *t = data;
  return scm_from_bool (t->fn (t->arg));
}

static SCM
report_exception (void *data, SCM key, SCM args)
{
  struct test_case *t = data;
  char *str = scm_to_locale_string
    (scm_object_to_string (scm_cons (key, args), SCM_UNDEFINED));
  printf ("FAIL: %s: uncaught Scheme exception: %s\n", t->name, str);
  free (str);
  return SCM_BOOL_F;
}

/* Returns 1 if the test passed.  */
static int
run_test (struct test_case *t)
{
  SCM ret;

  printf ("running: %s\n", t->name);
  ret = scm_c_catch (SCM_BOOL_T,
                     run_test_body, t,
                     report_exception, t,
                     NULL, NULL);
  if (scm_is_true (ret))
    {
      printf ("PASS: %s\n", t->name);
      return 1;
    }
  printf ("FAIL: %s\n", t->name);
  return 0;
}

static SCM
init_body (void *data)
{
  (void) data;
  cl_init ();
  return SCM_BOOL_T;
}

static SCM
init_handler (void *data, SCM key, SCM args)
{
  struct test_case t = { "initializing guile-curl", NULL, SCM_BOOL_F };
  (void) data;
  return report_exception (&t, key, args);
}

/* Adapters for tests that take no argument or return nothing.  */
#define TEST0(f) static int f ## _0 (SCM arg) { (void) arg; return f (); }
#define TESTV(f) static int f ## _v (SCM arg) { (void) arg; f (); return 1; }
TEST0 (test_scm_is_handle)
TEST0 (test_scm_is_not_handle)
TEST0 (test_equalp_handle)
TEST0 (test_not_equalp_handle)
TEST0 (test_not_free_handle)
TEST0 (test_free_handle)
TESTV (test_double_free_handle)
TESTV (test_free_handle_string_postfields)
TESTV (test_free_handle_bytevector_postfields)
TEST0 (test_cl_is_handle_p__handle)
TEST0 (test_cl_is_handle_p__true)
TEST0 (test_can_convert_to_slist__slist)
TEST0 (test_can_convert_to_slist__empty_list)
TEST0 (test_can_convert_to_slist__list_of_integers)
TEST0 (test_scm_convert_to_slist__slist)
TEST0 (test_scm_convert_to_slist__empty_list)
TEST0 (test_scm_convert_to_slist__list_of_integers)

static int
test_slist_option (SCM option)
{
  test_free_handle_slist_option (option);
  return 1;
}

static int
test_empty_slist_option (SCM option)
{
  test_free_handle_empty_slist_option (option);
  return 1;
}

int main(int argc, char **argv)
{
  extern SCM cl_CURLOPT_HTTPHEADER, cl_CURLOPT_HTTP200ALIASES,
    cl_CURLOPT_MAIL_RCPT, cl_CURLOPT_QUOTE, cl_CURLOPT_POSTQUOTE,
    cl_CURLOPT_PREQUOTE, cl_CURLOPT_RESOLVE, cl_CURLOPT_TELNETOPTIONS,
    cl_CURLOPT_CONNECT_TO;
  int failures = 0;

  (void) argc;
  (void) argv;
  setvbuf (stdout, NULL, _IONBF, 0);
  setvbuf (stderr, NULL, _IONBF, 0);

  printf ("initializing Guile\n");
  scm_init_guile ();

  /* The same initialization as loading the (curl) module: libcurl,
     the handle type, the constants and the procedures.  */
  printf ("initializing guile-curl\n");
  if (scm_is_false (scm_c_catch (SCM_BOOL_T, init_body, NULL,
                                 init_handler, NULL, NULL, NULL)))
    return 1;

  {
    struct test_case tests[] = {
      { "a handle is a handle", test_scm_is_handle_0, SCM_BOOL_F },
      { "a bool is not a handle", test_scm_is_not_handle_0, SCM_BOOL_F },
      { "a handle equals itself", test_equalp_handle_0, SCM_BOOL_F },
      { "a handle does not equal another handle",
        test_not_equalp_handle_0, SCM_BOOL_F },
      { "an unfreed handle has non-NULL SMOB data",
        test_not_free_handle_0, SCM_BOOL_F },
      { "a freed handle has NULL SMOB data", test_free_handle_0, SCM_BOOL_F },
      { "double frees of handles don't segfault",
        test_double_free_handle_v, SCM_BOOL_F },
      { "freeing a handle with string postfields doesn't segfault",
        test_free_handle_string_postfields_v, SCM_BOOL_F },
      { "freeing a handle with bytevector postfields doesn't segfault",
        test_free_handle_bytevector_postfields_v, SCM_BOOL_F },
#define SLIST_TESTS(x)                                                  \
      { "freeing a handle with a " #x " slist doesn't segfault",        \
        test_slist_option, x },                                         \
      { "freeing a handle with an empty " #x " slist doesn't segfault", \
        test_empty_slist_option, x }
      SLIST_TESTS (cl_CURLOPT_HTTPHEADER),
      SLIST_TESTS (cl_CURLOPT_HTTP200ALIASES),
      SLIST_TESTS (cl_CURLOPT_MAIL_RCPT),
      SLIST_TESTS (cl_CURLOPT_QUOTE),
      SLIST_TESTS (cl_CURLOPT_POSTQUOTE),
      SLIST_TESTS (cl_CURLOPT_PREQUOTE),
      SLIST_TESTS (cl_CURLOPT_RESOLVE),
      SLIST_TESTS (cl_CURLOPT_TELNETOPTIONS),
      SLIST_TESTS (cl_CURLOPT_CONNECT_TO),
      { "cl_is_handle_p returns #t for a handle",
        test_cl_is_handle_p__handle_0, SCM_BOOL_F },
      { "cl_is_handle_p returns #f for #t",
        test_cl_is_handle_p__true_0, SCM_BOOL_F },
      { "_scm_can_convert_to_slist accepts a list of strings",
        test_can_convert_to_slist__slist_0, SCM_BOOL_F },
      { "_scm_can_convert_to_slist accepts an empty list",
        test_can_convert_to_slist__empty_list_0, SCM_BOOL_F },
      { "_scm_can_convert_to_slist rejects a list of integers",
        test_can_convert_to_slist__list_of_integers_0, SCM_BOOL_F },
      { "_scm_convert_to_slist converts a list of strings",
        test_scm_convert_to_slist__slist_0, SCM_BOOL_F },
      { "_scm_convert_to_slist returns NULL for an empty list",
        test_scm_convert_to_slist__empty_list_0, SCM_BOOL_F },
      { "_scm_convert_to_slist returns NULL for a list of integers",
        test_scm_convert_to_slist__list_of_integers_0, SCM_BOOL_F },
    };
    size_t n = sizeof tests / sizeof tests[0];

    for (size_t i = 0; i < n; i ++)
      if (!run_test (&tests[i]))
        failures ++;
    printf ("%zu tests, %d failed\n", n, failures);
  }

  return failures ? 1 : 0;
}
