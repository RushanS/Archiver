all:
	gcc arch.c -o arch -fsanitize="address"
