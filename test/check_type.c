#include "../src/type.h"
#include "../src/func.h"

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
  return 0;
}

