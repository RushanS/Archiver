#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

void arch(char *path, char *subdir) {
	
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;

	if ((dir = opendir(path)) == NULL) {
		printf("cannot open directory\n");
		exit(0);	
	}
 	
	chdir(path);
	printf("START %s\n", subdir);

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
			continue;
		lstat(entry->d_name, &statbuf);
		
		if (S_ISDIR(statbuf.st_mode))  {
			arch(entry->d_name, entry->d_name);
		}
		else printf("%s\n", entry->d_name);
	}
	closedir(dir);
	chdir("..");
	printf("END\n");
}


void main() {
	char path[1000];
	printf("directory to archiving: ");
	scanf("%s", path);
	arch(path, strrchr(path, '/') + 1);
	printf("----end----\n");
}

