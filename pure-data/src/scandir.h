#include <dirent.h>

int scandir (const char *directory_name,
             struct dirent ***array_pointer,
             int (*select_function) (const struct dirent *),
             /* This is what the linux man page says */
             int (*compare_function) (const struct dirent**, const struct dirent**)
	);

int alphasort(const void *dirent1, const void *dirent2);
