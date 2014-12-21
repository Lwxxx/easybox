/*
 * cscope index tool
 *
 * Copyright (C) 2014 by Li Wenxiang <lwx169@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>


#define CS_ARG_LEN 32
#define CS_CMD_LEN 128

#define show_progress(fmt, ...) do {			\
	if (__cs_idx_verbose) {						\
		printf(fmt, ##__VA_ARGS__);				\
	}											\
} while (0)

#define set_verbose() __cs_idx_verbose = 1

static int __cs_idx_verbose = 0;


static void show_usage(void)
{
	char usage_str[] = "Usage:\n\n"
		"\tcscope-indexer [ -v ] [-f database_file ] [-i list_file ] [ -l ] [ -r ]\n\n"
		"where:\n\n"
		"\t-f database_file\n"
		"\t\tSpecifies the cscope database file (default: cscope.out).\n\n"
		"\t-i list_file\n"
		"\t\tSpecifies the name of the file into which the list of files\n"
		"\t\tto index is placed (default: cscope.files).\n\n"
		"\t-l\n"
		"\t\tSuppress the generation/updating of the cscope database\n"
		"\t\tfile.  Only a list of files is generated.\n\n"
		"\t-r\n"
		"\t\tRecurse into subdirectories to locate files to index.\n"
		"\t\tWithout this option, only the current directory is\n"
		"\t\tsearched.\n\n"
		"\t-v\n"
		"\t\tBe verbose.  Output simple progress messages.\n\n"
		"\t-h\n"
		"\t\tShow these help informations.\n\n";

	printf("%s", usage_str);
	exit(0);
}

// check suffix: .c, .h, .cc, .cxx, .hxx, .cpp, .hpp
static int is_c_source(char* filename)
{
	int name_len    = strlen(filename);
	int find_suffix = 0;
	char suffix[5]  = {0};
	int i = 0, j = 0;

	for (i = name_len - 1, j = 0; i >= 0 && j < 4; --i, ++j) {
		if ('.' == filename[i]) {
			find_suffix = 1;
			break;
		}
		suffix[j] = filename[i];
	}

	if (0 == find_suffix) {
		return 0;
	}

	if (0 == strcmp("c", suffix) || 0 == strcmp("h", suffix) ||
		0 == strcmp("cc", suffix) || 0 == strcmp("xxc", suffix) ||
		0 == strcmp("xxh", suffix) || 0 == strcmp("ppc", suffix) ||
		0 == strcmp("pph", suffix)) {
		return 1;
	}

	return 0;
}


void dir_traversal(char* path, int recursve, void (*callback)(char*))
{
	struct dirent* entry = NULL;
	DIR* cur_dir         = NULL;
	char file_path[PATH_MAX] = {0};

	if (NULL != (cur_dir = opendir(path))) {
		while (NULL != (entry = readdir(cur_dir))) {

			if (1 == strlen(path) && '.' == path[0]) {
				sprintf(file_path, "%s", entry->d_name);
			} else {
				sprintf(file_path, "%s/%s", path, entry->d_name);
			}

			if ('.' == entry->d_name[0]) {
				continue;
			} else if (DT_REG == entry->d_type) {
				callback(file_path);
			} else if (recursve && DT_DIR == entry->d_type) {
				dir_traversal(file_path, recursve, callback);
			} else {
				continue;
			}
		}
		closedir(cur_dir);
	}
}


int main(int argc, char* argv[])
{
	int arg_val       = 0;
	int opt_list_only = 0;
	int opt_recurse   = 0;
	FILE* list_fp     = NULL;

	char list_file[CS_ARG_LEN]  = "cscope.files";
	char db_file[CS_ARG_LEN]    = "cscope.out";
	char cscope_cmd[CS_CMD_LEN] = {0};

	/* gcc feature: statement expression and nested function, may cause
	 * build errors when use other compiler */
	void (*add_file_to_list)(char* filename) =
	({
		void __add_file(char* filename) {
			if (list_fp) {
				if (is_c_source(filename)) {
					fprintf(list_fp, "%s\n", filename);
				}
			}
		}
		__add_file;
	});

	// parse command line options
	while (EOF != (arg_val = getopt(argc, argv, "f:i:lrvh"))) {
		switch (arg_val) {
		case 'f':
			memset(db_file, 0, CS_ARG_LEN);
			strncpy(db_file, optarg, CS_ARG_LEN);
			break;
		case 'i':
			memset(list_file, 0, CS_ARG_LEN);
			strncpy(list_file, optarg, CS_ARG_LEN);
			break;
		case 'l':
			opt_list_only = 1;
			break;
		case 'r':
			opt_recurse = 1;
			break;
		case 'v':
			set_verbose();
			break;
		case 'h':
			show_usage();
			break;
		default:
			break;
		}
	}

	// create a list of all c source files.
	show_progress("Creating list of files to index ...\n");
	list_fp = fopen(list_file, "w");
	if (NULL == list_fp) {
		perror("open list file failed!\n");
		goto failed_out;
	}
	dir_traversal(".", opt_recurse, add_file_to_list);
	fclose(list_fp);

	show_progress("Creating list of files to index ... done\n");
	if (1 == opt_list_only) {
		return 0;
	}

	// create cscope database by file list
	show_progress("Indexing files ...\n");
	sprintf(cscope_cmd, "cscope -b -i %s -f %s", list_file, db_file);
	if (0 != system(cscope_cmd)) {
		show_progress("Indexing files ... failed\n");
	} else {
		show_progress("Indexing files ... done\n");
	}

	return 0;

failed_out:
	if (list_fp) {
		fclose(list_fp);
	}

	return -1;
}
