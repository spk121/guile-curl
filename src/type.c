#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <libguile.h>
#include <curl/curl.h>
#include "type.h"

static scm_t_bits handle_tag;

SCM equalp_handle (SCM x1, SCM x2);
size_t gc_free_handle (SCM x);
int print_handle (SCM x, SCM port, scm_print_state *pstate);

/* handle -- In C, a CURL *.  In Scheme, a smob that contains the pointer */


int 
_scm_is_handle (SCM x)
{
  if (SCM_SMOB_PREDICATE (handle_tag, x))
    {
      if ((CURL *) SCM_SMOB_DATA (x) != (CURL *) NULL)
	return 1;
      else 
	return 0;
    }
  else
    return 0;
}

CURL *
_scm_to_handle (SCM x)
{
  return (CURL *) SCM_SMOB_DATA (x);
}

SCM
_scm_from_handle (CURL *x)
{
  SCM s_handle;

  SCM_CRITICAL_SECTION_START;

  assert (x != NULL);

  SCM_NEWSMOB (s_handle, handle_tag, x);

  assert (x == (CURL *) SCM_SMOB_DATA (s_handle));

  if (0) 
    {
      fprintf (stderr, "Making <#handle> smob from CURL * %p\n", x);
    }
  SCM_CRITICAL_SECTION_END;
  return (s_handle);
}

// Handles are equal if they point to the same C structure
SCM
equalp_handle (SCM x1, SCM x2)
{
  SCM ret;
  CURL *handle1;
  CURL *handle2;

  SCM_CRITICAL_SECTION_START;
  handle1 = (CURL *) SCM_SMOB_DATA (x1);
  handle2 = (CURL *) SCM_SMOB_DATA (x2);

  if ((handle1 == NULL) || (handle2 == NULL))
    ret = SCM_BOOL_F;
  else if ((handle1 != handle2))
    ret = SCM_BOOL_F;
  else
    ret = SCM_BOOL_T;
  SCM_CRITICAL_SECTION_END;
}

size_t
gc_free_handle (SCM handle)
{
  SCM_ASSERT (SCM_SMOB_PREDICATE (handle_tag, handle), handle, SCM_ARG1, "free-handle");

  SCM_CRITICAL_SECTION_START;

  CURL *m = _scm_to_handle (handle);

  assert (m != NULL);

  if (m != (CURL *) NULL)
    {
      curl_easy_cleanup (m);
      m = (CURL *) NULL;
    }
  SCM_CRITICAL_SECTION_END;

  return 0;
}

int
#ifdef __GNUC__
print_handle (SCM x, SCM port, scm_print_state *pstate __attribute__ ((unused)))
#else
print_handle (SCM x, SCM port, scm_print_state *pstate)
#endif
{
  CURL *frm = (CURL *) SCM_SMOB_DATA (x);
  char *str;

  scm_puts ("#<handle ", port);

  if (frm == (CURL *) NULL)
    {
      scm_puts ("(freed)", port);
    }
  else
    {
      if (asprintf (&str, "%p", frm) < 0)
	scm_puts ("???", port);
      else
	scm_puts (str, port);
    }

  scm_puts (">", port);
  
  // non-zero means success 
  return 1;
}

SCM
cl_is_handle_p (SCM x)
{
  return scm_from_bool (_scm_is_handle (x));
}

static scm_t_bits slist_tag;

SCM equalp_slist (SCM x1, SCM x2);
size_t gc_free_slist (SCM x);
int print_slist (SCM x, SCM port, scm_print_state *pstate);


/* slist -- In C, a struct curl_slist *.
   In Scheme, a smob that contains the pointer */

int 
_scm_is_slist (SCM x)
{
  if (SCM_SMOB_PREDICATE (slist_tag, x))
    return 1;
  return 0;
}

struct curl_slist *
_scm_to_slist (SCM x)
{
  return (struct curl_slist *) SCM_SMOB_DATA (x);
}

SCM
_scm_from_slist (struct curl_slist *x)
{
  SCM s_slist;

  SCM_CRITICAL_SECTION_START;

  assert (x != NULL);

  SCM_NEWSMOB (s_slist, slist_tag, x);

  assert (x == (struct curl_slist *) SCM_SMOB_DATA (s_slist));

  if (0) 
    {
      fprintf (stderr, "Making <#slist> smob from struct curl_slist * %p\n", x);
    }
  SCM_CRITICAL_SECTION_END;
  return (s_slist);
}

// Slists are equal if they point to the same C structure
SCM
equalp_slist (SCM x1, SCM x2)
{
  SCM ret;
  struct curl_slist *slist1;
  struct curl_slist *slist2;

  SCM_CRITICAL_SECTION_START;
  slist1 = (struct curl_slist *) SCM_SMOB_DATA (x1);
  slist2 = (struct curl_slist *) SCM_SMOB_DATA (x2);

  if ((slist1 == NULL) || (slist2 == NULL))
    ret = SCM_BOOL_F;
  else if ((slist1 != slist2))
    ret = SCM_BOOL_F;
  else
    ret = SCM_BOOL_T;
  SCM_CRITICAL_SECTION_END;
}

size_t
gc_free_slist (SCM slist)
{
  SCM_ASSERT (SCM_SMOB_PREDICATE (slist_tag, slist), slist, SCM_ARG1, "free-slist");

  SCM_CRITICAL_SECTION_START;

  struct curl_slist *m = _scm_to_slist (slist);

  assert (m != NULL);

  if (m != (struct curl_slist *) NULL)
    {
      curl_slist_free_all (m);
      m = (struct curl_slist *) NULL;
    }
  SCM_CRITICAL_SECTION_END;

  return 0;
}

int
print_slist (SCM x, SCM port, scm_print_state *pstate)
{
  struct curl_slist *frm = (struct curl_slist *) SCM_SMOB_DATA (x);
  char *str;

  scm_puts ("#<slist ", port);

  if (frm == (struct curl_slist *) NULL)
    {
      scm_puts ("(freed)", port);
    }
  else
    {
      if (asprintf (&str, "%p", frm) < 0)
	scm_puts ("???", port);
      else
	scm_puts (str, port);
    }

  scm_puts (">", port);
  
  // non-zero means success 
  return 1;
}

SCM
cl_is_slist_p (SCM x)
{
  return scm_from_bool (_scm_is_slist (x));
}

SCM
cl_list_to_curl_list (SCM x)
{
  int i, n;
  SCM elt;
  char *str;
  struct curl_slist *slist = NULL;

  SCM_ASSERT (scm_is_true (scm_list_p (x)), x, SCM_ARG2, "list->curl-list");
  n = scm_to_int (scm_length (x));
  if (n == 0)
    return _scm_from_slist (slist);
  for (i = 0; i < n; i ++)
    {
      elt = scm_list_ref (x, scm_from_int (i));
      if (!scm_is_string (elt))
	scm_wrong_type_arg ("list->curl-list", SCM_ARG1, x);
      str = scm_to_locale_string (elt);
      slist = curl_slist_append (slist, str);
    }
  return _scm_from_slist (slist);
}


void 
cl_init_type ()
{
  handle_tag = scm_make_smob_type ("handle", sizeof (CURL *));
  scm_set_smob_free (handle_tag, gc_free_handle);
  scm_set_smob_print (handle_tag, print_handle);
  scm_set_smob_equalp (handle_tag, equalp_handle);
  scm_c_define_gsubr ("handle?", 1, 0, 0, cl_is_handle_p);

  slist_tag = scm_make_smob_type ("slist", sizeof (struct curl_slist *));
  scm_set_smob_free (slist_tag, gc_free_slist);
  scm_set_smob_print (slist_tag, print_slist);
  scm_set_smob_equalp (slist_tag, equalp_slist);
  scm_c_define_gsubr ("slist?", 1, 0, 0, cl_is_slist_p);
  scm_c_define_gsubr ("list->curl-list", 1, 0, 0, cl_list_to_curl_list);

}

