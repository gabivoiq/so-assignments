#include "so_stdio.h"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

typedef struct _so_file {
	int fd;
	int offset;
	char buffer[BUFIZE];
	int buffer_count;
	int err;
} SO_FILE;

SO_FILE *so_fopen(const char *pathname, const char *mode) {
	SO_FILE *so_file = calloc(1, sizeof(SO_FILE));

	if (strcmp(mode, "r") == 0) {
		so_file->fd = open(pathname, O_RDONLY);
	} else if (strcmp(mode, "r+") == 0) {
		so_file->fd = open(pathname, O_RDWR);
	} else if (strcmp(mode, "w") == 0) {
		so_file->fd = open(pathname,O_WRONLY | O_CREAT | O_TRUNC, 0644);
	} else if (strcmp(mode, "w+") == 0) {
		so_file->fd = open(pathname,O_RDWR | O_CREAT | O_TRUNC, 0644);
	} else if (strcmp(mode, "a") == 0) {
		so_file->fd = open(pathname, O_APPEND | O_CREAT, 0644);
	} else if (strcmp(mode, "a+") == 0) {
		so_file->fd = open(pathname, O_APPEND | O_RDWR | O_CREAT, 0644);
	} else {
		so_file->fd = -1;
	}

	if(so_file->fd < 0) {
		free(so_file);
		return NULL;
	}

	return so_file;
}

int so_fclose(SO_FILE *stream) {
	int val = close(stream->fd);
	free(stream);
	if(val < 0) {
		return SO_EOF;
	}

	return 0;
}

int so_fileno(SO_FILE *stream) {
	return stream->fd;
}

int so_fflush(SO_FILE *stream) {

}

int so_fseek(SO_FILE *stream, long offset, int whence) {

}

long so_ftell(SO_FILE *stream) {

}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	int i = 0, j = 0, index = 0;
	for(i = 0; i < nmemb; i++) {
		for(j = 0; j < size; j++) {
			int ret = so_fgetc(stream);
			if(ret == SO_EOF) {
				return 0;
			}
			((char*)ptr)[index++] = (char) ret;
		}
	}
	return nmemb;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {

}

int so_fgetc(SO_FILE *stream) {
	int bytesRead;
	unsigned char character;

	if(--stream->buffer_count <= 0) {
		bytesRead = read(stream->fd, stream->buffer, BUFIZE);
		if (bytesRead < 0) {
			stream->err = 1;
			return SO_EOF;
		}
		stream->buffer_count = bytesRead;
		stream->offset = 0;
	}
	character = stream->buffer[stream->offset++];

	return (int) character;
}

int so_fputc(int c, SO_FILE *stream) {

}

int so_feof(SO_FILE *stream) {

}

int so_ferror(SO_FILE *stream) {

}

SO_FILE *so_popen(const char *command, const char *type) {

}

int so_pclose(SO_FILE *stream) {

}