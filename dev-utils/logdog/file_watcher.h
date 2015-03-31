#ifndef __FILE_WATCHER__H
#define __FILE_WATCHER__H

#define LDG_CHECK_INTERVAL 1
#define LDG_LINE_MAX       256

void file_watch(char* filename, void (*callback)(char* line));

#endif // __FILE_WATCHER__H
