#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "eb_global.h"
#include "component.h"

LOCAL struct component component_list[] =
{
   #include "component.conf"
};

LOCAL int component_count = sizeof(component_list)/sizeof(struct component);

GLOBAL struct component* find_component_by_name(char* name)
{
	int i = 0;

	if (NULL == name)
	{
		return NULL;
	}

	for (i = 0; i < component_count; ++i)
	{
		if (0 == strncmp(component_list[i].name, name, COMPENENT_NAME_MAX))
		{
			return &(component_list[i]);
		}
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int ret = 0;
	pthread_t* thread_id = NULL;
	struct component* component = NULL;

	thread_id = (pthread_t*)malloc(component_count * sizeof(pthread_t));
	if (NULL == thread_id)
	{
		fprintf(stderr, "allocate pthread memory failed\n");
		return -1;
	}

	for (i = 0; i < component_count; ++i)
	{
		component = &(component_list[i]);
		ret = pthread_create(&(thread_id[i]), NULL, component->run, NULL);
		if (0 != ret)
		{
			fprintf(stderr, "start component %s failed\n", component->name);
		}
	}

	for (i = 0; i < component_count; ++i)
	{
		pthread_join(thread_id[i], NULL);
	}

	if (NULL != thread_id)
	{
		free(thread_id);
	}
	return 0;
}
