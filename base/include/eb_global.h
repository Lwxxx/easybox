#ifndef __EASYBOX_GLOBAL_H__
#define __EASYBOX_GLOBAL_H__

#define GLOBAL
#define LOCAL  static
#define IMPORT extern

#ifdef PATH_MAX
#define EB_PATH_MAX PATH_MAX
#else
#define EB_PATH_MAX 256
#endif

#endif // __EASYBOX_GLOBAL_H__
