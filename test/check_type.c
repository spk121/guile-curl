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

void test_free_handle_string_postfields(void)
{
  extern SCM cl_CURLOPT_POSTFIELDS;
  SCM handle = cl_easy_init();
  SCM str = scm_from_locale_string("abcdefghijklmnopqrstuvwxyz");
  SCM ret = cl_easy_setopt(handle, scm_variable_ref(cl_CURLOPT_POSTFIELDS), str, SCM_BOOL_F);
  gc_free_handle(handle);
}

#if SCM_MAJOR_VERSION != 1
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

int main()
{
  int ret;
  scm_init_guile();

  ret = test_scm_is_handle();
  printf("test scm is handle: %d\n", ret);
  if (!ret)
    return 1;
  printf("test scm is not handle: %d\n", ret);
  if (!ret)
    return 1;
  printf("test that frees of a handle with string-based postfields doesn't segfault\n");
  test_free_handle_string_postfields();

#if SCM_MAJOR_VERSION != 1
  printf("test that frees of a handle with bytevector-based postfields doesn't segfault\n");
  test_free_handle_bytevector_postfields();
#endif

  return 0;
}

