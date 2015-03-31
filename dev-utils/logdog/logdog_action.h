#ifndef __LOGDOG_ACTION_H__
#define __LOGDOG_ACTION_H__

#define LDG_MATCH     0
#define LDG_NOT_MATCH 1

#define OVECCOUNT     30

int  logdog_match(char* line, char* pattern);
void logdog_do(char* cmd);

#endif // __LOGDOG_ACTION_H__
