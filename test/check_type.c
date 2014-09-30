/* Test driver for type functions for guile-curl

   Copyright (c) 2014 Michael L. Gran

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
  return (ret == SCM_BOOL_T);
}

int test_not_equalp_handle(void)
{
  SCM handle1 = cl_easy_init();
  SCM handle2 = cl_easy_init();
  SCM ret = equalp_handle(handle1, handle2);
  return (ret == SCM_BOOL_F);
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

#if (SCM_MAJOR_VERSION != 1)
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
#endif

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
  printf("test that cl_is_handle_p returns #t when passed a handle: %d\n", ret);
  return scm_is_true(ret);
}

int test_cl_is_handle_p__true (void)
{
  SCM handle = SCM_BOOL_T;
  SCM ret = cl_is_handle_p(handle);
  printf("test that cl_is_handle_p returns #f when not passed a handle: %d\n", ret);
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


int main()
{
  int ret;
  scm_init_guile();
  cl_init_const();

  ret = test_scm_is_handle();
  printf("test that a handle is a handle: %d\n", ret);
  if (!ret)
    return 1;

  ret = test_scm_is_not_handle();
  printf("test that a bool is not handle: %d\n", ret);
  if (!ret)
    return 1;

  ret = test_equalp_handle();
  printf("test that a handle equals itself: %d\n", ret);
  if (!ret)
    return 1;

  ret = test_not_equalp_handle();
  printf("test that a handle does not equal another handle: %d\n", ret);
  if (!ret)
    return 1;

  ret = test_not_free_handle();
  printf("test that an unfreed handle has non-NULL SMOB data: %d\n", ret);
  if (!ret)
    return 1;

  ret = test_free_handle();
  printf("test that a freed handle has NULL SMOB data: %d\n", ret);
  if (!ret)
    return 1;

  printf("test that double frees of handles don't segfault\n");
  test_double_free_handle();

  printf("test that frees of a handle with string-based postfields doesn't segfault\n");
  test_free_handle_string_postfields();

#if SCM_MAJOR_VERSION != 1
  printf("test that frees of a handle with bytevector-based postfields doesn't segfault\n");
  test_free_handle_bytevector_postfields();
#endif

#define TEST_SLIST(x) \
  extern SCM x; \
  printf ("test that frees of a handle with a " #x " slist doesn't segfault\n"); \
  test_free_handle_slist_option(x); \
  printf ("test that frees of a handle with an empty " #x " slist don't segfault\n"); \
  test_free_handle_empty_slist_option(x)

  TEST_SLIST(cl_CURLOPT_HTTPHEADER);
  TEST_SLIST(cl_CURLOPT_HTTP200ALIASES);
  TEST_SLIST(cl_CURLOPT_MAIL_RCPT);
  TEST_SLIST(cl_CURLOPT_QUOTE);
  TEST_SLIST(cl_CURLOPT_POSTQUOTE);
  TEST_SLIST(cl_CURLOPT_PREQUOTE);
  TEST_SLIST(cl_CURLOPT_RESOLVE);
  TEST_SLIST(cl_CURLOPT_TELNETOPTIONS);

  ret = test_cl_is_handle_p__handle ();
  if (!ret)
    return 1;

  ret = test_cl_is_handle_p__true ();
  if (!ret)
    return 1;

  ret = test_can_convert_to_slist__slist ();
  if (!ret)
    return 1;

  ret = test_can_convert_to_slist__empty_list ();
  if (!ret)
    return 1;

  ret = test_can_convert_to_slist__list_of_integers ();
  if (!ret)
    return 1;

  ret = test_scm_convert_to_slist__slist ();
  if (!ret)
    return 1;

  ret = test_scm_convert_to_slist__empty_list ();
  if (!ret)
    return 1;

  ret = test_scm_convert_to_slist__list_of_integers ();
  if (!ret)
    return 1;

  return 0;
}
