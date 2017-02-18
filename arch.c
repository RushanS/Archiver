#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>


void main() {
	
	char path[100];
	DIR *dir;
	struct dirent *entry;

	printf("directory to archiving: ");
	scanf("%s", path);
	
	if ((dir = opendir(path)) == NULL) {
		printf("cannot open directory\n");
		exit(0);	
	}
 	
	while ((entry = readdir(dir)) != NULL) {
		printf("%s\n", entry->d_name);
	}

	printf("----end----\n"); 
}

