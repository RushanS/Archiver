#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>


FILE *archive;


void pack(char *path, char *dname) {
	
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;

	if ((dir = opendir(path)) == NULL) {
		printf("cannot open directory\n");
		exit(0);	
	}
 	
	chdir(path);
	fwrite("{", 1, 1, archive);
	fwrite(dname, 1, 255, archive);

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
			continue;

		lstat(entry->d_name, &statbuf);
		
		if (S_ISDIR(statbuf.st_mode))  
			pack(entry->d_name, entry->d_name);	
		else {
			fwrite("f", 1, 1, archive);
			fwrite(&statbuf.st_size, sizeof(size_t), 1, archive);
			fwrite(&entry->d_name, 1, 255, archive);
			FILE *file = fopen(entry->d_name, "rb");
			char *buf = malloc(statbuf.st_size);
			fread(buf, 1, statbuf.st_size, file);
			fwrite(buf, 1, statbuf.st_size, archive);
			fclose(file);
		}
	}
	fwrite("}", 1, 1, archive);
	closedir(dir);
	chdir("..");
}


void unpack(char *path) {
	char name[255];
	char flag;
	size_t n;
	chdir(path);
	while ((n = fread(&flag, 1, 1, archive)) != 0) {
		if (flag == '{') {
			fread(name, 1, 255, archive);
			mkdir(name, S_IRUSR|S_IWUSR|S_IXUSR);
			chdir(name);
		}
		else if (flag == '}') {
			chdir("..");
		}
		else {
			size_t size;
			FILE *file;
			size_t nb;
			fread(&size, sizeof(size_t), 1, archive);
			fread(name, 1, 255, archive);
			file = fopen(name, "wb");
			char *buf = malloc(size);
			fread(buf, 1, size, archive);
			fwrite(buf, 1, size, file);
			fclose(file);
		}
	}
}


void main(int argc, char* argv[]) {
	if (argc == 3 && strcmp(argv[1], "pack") == 0) {
		char *name = strrchr(argv[2], '/') + 1;
		archive = fopen(name, "wb");
		pack(argv[2], name);
		fclose(archive);
	}
	else if (argc == 4 && strcmp(argv[1], "unpack") == 0) {
		archive = fopen(argv[2], "rb");
		unpack(argv[3]);
		fclose(archive);
	}
	else printf("wrong parameters!\n arch pack [directory to packaging]\narch unpack [archive file] [path for unpackaging]\n");
}

