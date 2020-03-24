build: so-cpp

so-cpp: so-cpp.o hashmap.o
	gcc so-cpp.o hashmap.o -o so-cpp

hashmap.o: hashmap.c
	gcc -Wall -Wextra -g -c -o hashmap.o hashmap.c

so-cpp.o: so-tema1.c
	gcc -Wall -Wextra -g -c -o so-cpp.o so-tema1.c

clean:
	rm so-cpp so-cpp.o hashmap.o