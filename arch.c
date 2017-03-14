#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define NAME_SIZE 255

int archive;


void pack(char *path, char *dname)
{
	struct dirent *entry;
	struct stat statbuf;
	DIR *dir = opendir(path);

	if (dir == NULL) {
		printf("cannot open directory\n");
		exit(0);
	}
	chdir(path);
	if (write(archive, "{", 1) <= 0)
		printf("error");
	if (write(archive, dname, NAME_SIZE))
		printf("error");
	while (entry = readdir(dir)) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		lstat(entry->d_name, &statbuf);

		if (S_ISDIR(statbuf.st_mode))
			pack(entry->d_name, entry->d_name);
		else {
			int file;
			char *buf;

			write(archive, "f", 1);
			write(archive, &statbuf.st_size, sizeof(size_t));
			write(archive, &entry->d_name, NAME_SIZE);
			file = open(entry->d_name, O_RDONLY);
			buf = malloc(statbuf.st_size);
			read(file, buf, statbuf.st_size);
			write(archive, buf, statbuf.st_size);
			close(file);
			free(buf);
		}
	}
	write(archive, "}", 1);
	closedir(dir);
	chdir("..");
}

void unpack(char *path)
{
	char name[NAME_SIZE];
	char flag;
	size_t n;

	chdir(path);
	while ((n = read(archive, &flag, 1)) != 0) {
		if (flag == '{') {
			read(archive, name, NAME_SIZE);
			mkdir(name, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
			chdir(name);
		} else if (flag == '}') {
			chdir("..");
		} else {
			size_t size;
			size_t nb;
			int file;
			char *buf;

			read(archive, &size, sizeof(size_t));
			read(archive, name, NAME_SIZE);
			file = open(name, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
			buf = malloc(size);
			read(archive, buf, size);
			write(file, buf, size);
			close(file);
			free(buf);
		}
	}
}

void main(int argc, char *argv[])
{
	if (argc == 3 && strcmp(argv[1], "pack") == 0) {
		char *name = strrchr(argv[2], '/') + 1;
		char filename[NAME_SIZE];

		strncat(filename, name, NAME_SIZE - 5);
		strncat(filename, ".arch", 5);
		archive = open(filename, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
		pack(argv[2], name);
		close(archive);
	} else if (argc == 4 && strcmp(argv[1], "unpack") == 0) {
		archive = open(argv[2], O_RDONLY);
		unpack(argv[3]);
		close(archive);
	} else {
		printf("wrong parameters!\n");
		printf("arch pack [directory to packaging]\n");
		printf("arch unpack [archive file] [path for unpackaging]\n");
	}
}
