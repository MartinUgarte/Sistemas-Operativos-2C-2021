#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define CI_FLAG "-i"


static int
contains_substr_cs(const char *str, const char *substr)
{
	return strstr(str, substr) != NULL;
}

static int
contains_substr_ci(const char *str, const char *substr)
{
	return strcasestr(str, substr) != NULL;
}

static void
print_path(char *path, char *entry_name)
{
	strcmp(path, "") == 0 ? printf("%s\n", entry_name)
	                      : printf("%s/%s\n", path, entry_name);
}

static void
find_str(DIR *dir,
         char *str,
         char *path,
         int (*contains_substr)(const char *, const char *))
{
	struct dirent *entry;
	char new_path[PATH_MAX];
	memset(new_path, 0, PATH_MAX);
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG &&
		    (*contains_substr)(entry->d_name, str))
			print_path(path, entry->d_name);

		if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
		    strcmp(entry->d_name, "..") != 0) {
			if ((*contains_substr)(entry->d_name, str)) {
				print_path(path, entry->d_name);
			}
			int actual_fd = dirfd(dir);
			int next_fd =
			        openat(actual_fd, entry->d_name, O_DIRECTORY);
			DIR *next_dir = fdopendir(next_fd);
			strcat(new_path, path);
			if (strcmp(path, "") != 0)
				strcat(new_path, "/");
			strcat(new_path, entry->d_name);
			find_str(next_dir, str, new_path, contains_substr);
		}
	}
	closedir(dir);
	return;
}

int
main(int argc, char *argv[])
{
	if (argc <= 1 || argc > 3) {
		fprintf(stderr, "Se necesita al menos un argumento y opcional -l para case-insensitive\n");
		return 0;
	}
	char *str = argv[1];
	int mode = 0;
	if (strcmp(str, CI_FLAG) == 0) {
		mode = 1;
		str = argv[2];
	}

	DIR *dir = opendir(".");
	if (dir == NULL) {
		perror("Error opendir");
		return -1;
	}

	int (*contains_substr)(const char *, const char *);
	contains_substr = (mode) ? contains_substr_ci : contains_substr_cs;

	char path[PATH_MAX];
	memset(path, 0, PATH_MAX);

	find_str(dir, str, path, contains_substr);

	return 0;
}