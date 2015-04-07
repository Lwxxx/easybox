#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include "eb_global.h"
#include "component_funcs.conf"

#define COMPENENT_NAME_MAX 32

struct component
{
	char  name[COMPENENT_NAME_MAX];
	void* (*run)(void* arg);
	int   port;
};

extern struct component* find_component_by_name(char* name);

#endif // __COMPONENT_H__
