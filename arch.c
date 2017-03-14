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

void alerterror()
{
	printf("error !!!");
	exit(0);
}

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
		alerterror();
	if (write(archive, dname, NAME_SIZE) != NAME_SIZE)
		alerterror();
	while (entry = readdir(dir)) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		lstat(entry->d_name, &statbuf);

		if (S_ISDIR(statbuf.st_mode))
			pack(entry->d_name, entry->d_name);
		else {
			int file;
			char *buf;

			if (write(archive, "f", 1) != 1)
				alerterror();
			if (write(archive, &statbuf.st_size, sizeof(size_t)) != sizeof(size_t))
				alerterror();
			if (write(archive, &entry->d_name, NAME_SIZE) != NAME_SIZE)
				alerterror();
			file = open(entry->d_name, O_RDONLY);
			buf = malloc(statbuf.st_size);
			if (read(file, buf, statbuf.st_size) != statbuf.st_size)
				alerterror();
			if (write(archive, buf, statbuf.st_size) != statbuf.st_size)
				alerterror();
			close(file);
			free(buf);
		}
	}
	if (write(archive, "}", 1) != 1)
		alerterror();
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
			if (read(archive, name, NAME_SIZE) != NAME_SIZE)
				alerterror();
			mkdir(name, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
			chdir(name);
		} else if (flag == '}') {
			chdir("..");
		} else {
			size_t size;
			size_t nb;
			int file;
			char *buf;

			if (read(archive, &size, sizeof(size_t)) != sizeof(size_t))
				alerterror();
			if (read(archive, name, NAME_SIZE) != NAME_SIZE)
				alerterror();
			file = open(name, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
			buf = malloc(size);
			if (read(archive, buf, size) != size)
				alerterror();
			if (write(file, buf, size) != size)
				alerterror();
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
