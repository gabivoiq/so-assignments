CC = gcc
CFLAGS = -Wall -Wextra -fPIC

build: libso_stdio.so

libso_stdio.so: libso_stdio.so.o
		$(CC) -shared libso_stdio.so.o -o libso_stdio.so

libso_stdio.so.o: so_stdio.c
		$(CC) $(CFLAGS) -c -o libso_stdio.so.o so_stdio.c

clean:
		rm libso_stdio.so *.o
