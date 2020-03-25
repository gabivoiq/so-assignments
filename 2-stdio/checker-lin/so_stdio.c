#include "so_stdio.h"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

typedef struct _so_file {
	int fd;
	int end_of_file;
	int buffer_offset;
	char buffer[BUFSIZE];
	int buffer_count;
	int write_syscall;
	int last_operation_read;
	int position_file;
	int err;
} SO_FILE;

SO_FILE *so_fopen(const char *pathname, const char *mode) {
	SO_FILE *so_file = calloc(1, sizeof(SO_FILE));
	if(so_file == NULL) {
		return NULL;
	}
	if (strcmp(mode, "r") == 0) {
		so_file->fd = open(pathname, O_RDONLY);
	} else if (strcmp(mode, "r+") == 0) {
		so_file->fd = open(pathname, O_RDWR);
	} else if (strcmp(mode, "w") == 0) {
		so_file->fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	} else if (strcmp(mode, "w+") == 0) {
		so_file->fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);
	} else if (strcmp(mode, "a") == 0) {
		so_file->fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND, 0644);
	} else if (strcmp(mode, "a+") == 0) {
		so_file->fd = open(pathname,  O_RDWR | O_CREAT| O_APPEND, 0644);
	} else {
		so_file->fd = -1;
	}

	if (so_file->fd < 0) {
		free(so_file);
		return NULL;
	}

	return so_file;
}

int so_fclose(SO_FILE *stream) {

	if(stream == NULL) {
		return SO_EOF;
	}
	if(so_fflush(stream) == SO_EOF) {
		stream->err = 1;
		return SO_EOF;
	}

	int val = close(stream->fd);
	if (val < 0) {
		stream->err = 1;
		return SO_EOF;
	}
	free(stream);

	return 0;
}

int so_fileno(SO_FILE *stream) {
	return stream->fd;
}

int so_fflush(SO_FILE *stream) {
	int ret = 0, totalBytes = 0;
	if (stream->write_syscall) {
		do {
			ret = write(stream->fd, stream->buffer + totalBytes, stream->buffer_count - totalBytes);
			if (ret < 0) {
				stream->err = 1;
				return SO_EOF;
			}
			totalBytes += ret;
		} while(totalBytes < stream->buffer_count);

		stream->buffer[0] = '\0';
		stream->buffer_count = 0;
		stream->buffer_offset = 0;
		stream->write_syscall = 0;
	}

	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence) {
	if(stream->buffer[0] != '\0') {
		if (stream->last_operation_read) {
			stream->buffer[0] = '\0';
			stream->buffer_count = 0;
			stream->buffer_offset = 0;
		} else {
			if(so_fflush(stream) == SO_EOF) {
				stream->err = 1;
				return SO_EOF;
			}
		}
	}

	stream->position_file = lseek(stream->fd, offset, whence);

	if(stream->position_file == -1) {
		stream->err = 1;
		return SO_EOF;
	}
	return 0;
}

long so_ftell(SO_FILE *stream) {
	return stream->position_file;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	int i = 0, j = 0, index = 0;

	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			int ret = so_fgetc(stream);
			if (ret == SO_EOF && !stream->end_of_file) {
				return 0;
			}
			if(stream->end_of_file) {
				return i;
			}
			((char *) ptr)[index++] = (char) ret;
		}
	}
	return nmemb;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream) {
	int i = 0, j = 0, index = 0, ret = 0;
	char c;

	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			c = ((char *) ptr)[index++];
			ret = so_fputc(c, stream);
			if (ret == SO_EOF) {
				stream->err = 1;
				return 0;
			}
		}
	}
	return nmemb;
}

int so_fgetc(SO_FILE *stream) {
	int bytesRead;
	unsigned char character;

	stream->write_syscall = 0;
	if (--stream->buffer_count <= 0) {
		bytesRead = read(stream->fd, stream->buffer, BUFSIZE);
		if (bytesRead < 0) {
			stream->err = 1;
			return SO_EOF;
		}
		if(!bytesRead) {
			stream->end_of_file = 1;
			return SO_EOF;
		}
		stream->last_operation_read = 1;
		stream->buffer_count = bytesRead;
		stream->buffer_offset = 0;
	}
	character = stream->buffer[stream->buffer_offset++];
	stream->position_file++;

	return (int) character;
}

int so_fputc(int c, SO_FILE *stream) {
	unsigned char character = c;
	int totalBytes = 0, bytesWrite = 0;

	stream->write_syscall = 1;
	if (stream->buffer_count > BUFSIZE - 1) {
		stream->buffer_count = 0;
		do {
			bytesWrite = write(stream->fd, stream->buffer + totalBytes, BUFSIZE - totalBytes);
			if (bytesWrite < 0) {
				stream->err = 1;
				return SO_EOF;
			}
			totalBytes += bytesWrite;
		} while (totalBytes < BUFSIZE);
		stream->last_operation_read = 0;
	}

	stream->buffer[stream->buffer_count] = character;
	stream->buffer[++stream->buffer_count] = '\0';
	stream->position_file++;

	return (int) character;
}

int so_feof(SO_FILE *stream) {
	return stream->end_of_file;
}

int so_ferror(SO_FILE *stream) {
	return stream->err;
}

SO_FILE *so_popen(const char *command, const char *type) {

}

int so_pclose(SO_FILE *stream) {

}