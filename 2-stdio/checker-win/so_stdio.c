#include "so_stdio.h"
#include <string.h>
#include <windows.h>

typedef struct _so_file {
	HANDLE fd;
	int end_of_file;
	int buffer_offset;
	char buffer[BUFSIZE];
	int buffer_count;
	int write_syscall;
	int last_operation_read;
	long position_file;
	int err;
} SO_FILE;

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	SO_FILE *so_file = calloc(1, sizeof(SO_FILE));

	if (so_file == NULL)
		return NULL;

	if (strcmp(mode, "r") == 0) {
		so_file->fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	} else if (strcmp(mode, "r+") == 0) {
		so_file->fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	} else if (strcmp(mode, "w") == 0) {
		so_file->fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	} else if (strcmp(mode, "w+") == 0) {
		so_file->fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	} else if (strcmp(mode, "a") == 0) {
		so_file->fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (so_file->fd != INVALID_HANDLE_VALUE)
			so_file->position_file = SetFilePointer(so_file->fd, 0,
				NULL, FILE_END);

	} else if (strcmp(mode, "a+") == 0) {
		so_file->fd = CreateFile(pathname,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (so_file->fd != INVALID_HANDLE_VALUE)
			so_file->position_file = SetFilePointer(so_file->fd, 0,
				NULL, FILE_END);

	} else {
		so_file->fd = INVALID_HANDLE_VALUE;
	}

	if (so_file->fd == INVALID_HANDLE_VALUE) {
		free(so_file);
		return NULL;
	}

	return so_file;
}

int so_fclose(SO_FILE *stream)
{
	BOOL ret;

	if (so_fflush(stream) == SO_EOF) {
		free(stream);
		return SO_EOF;
	}

	ret = CloseHandle(stream->fd);
	if (ret == FALSE) {
		free(stream);
		return SO_EOF;
	}

	free(stream);

	return 0;
}

HANDLE so_fileno(SO_FILE *stream)
{
	return stream->fd;
}

int so_fflush(SO_FILE *stream)
{
	BOOL ret;
	DWORD bytesWritten = 0;
	int totalBytes = 0;

	if (stream->write_syscall) {
		do {
			ret = WriteFile(stream->fd,
				stream->buffer + totalBytes,
				stream->buffer_count - totalBytes,
				&bytesWritten,
				NULL);
			if (ret == FALSE) {
				stream->err = 1;
				return SO_EOF;
			}
			totalBytes += bytesWritten;
		} while (totalBytes < stream->buffer_count);

		stream->buffer[0] = '\0';
		stream->buffer_count = 0;
		stream->buffer_offset = 0;
		stream->write_syscall = 0;
	}

	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
	if (stream->buffer[0] != '\0') {
		if (stream->last_operation_read) {
			stream->buffer[0] = '\0';
			stream->buffer_count = 0;
			stream->buffer_offset = 0;
		} else {
			if (so_fflush(stream) == SO_EOF) {
				stream->err = 1;
				return SO_EOF;
			}
		}
	}

	stream->position_file = SetFilePointer(stream->fd, offset,
		NULL, whence);

	if (stream->position_file == INVALID_SET_FILE_POINTER) {
		stream->err = 1;
		return SO_EOF;
	}
	return 0;
}

long so_ftell(SO_FILE *stream)
{
	if(stream == NULL)
		return SO_EOF;
	return stream->position_file;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	size_t i = 0, j = 0;
	int index = 0, ret = 0;

	for (i = 0; i < nmemb; i++)
		for (j = 0; j < size; j++) {
			ret = so_fgetc(stream);
			if (ret == SO_EOF && !stream->end_of_file)
				return 0;
			if (stream->end_of_file)
				return i;
			((char *) ptr)[index++] = (char) ret;
		}
	return nmemb;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
	size_t i = 0, j = 0, index = 0;
	int ret = 0;
	char c;

	for (i = 0; i < nmemb; i++)
		for (j = 0; j < size; j++) {
			c = ((char *) ptr)[index++];
			ret = so_fputc(c, stream);
			if (ret == SO_EOF) {
				stream->err = 1;
				return 0;
			}
		}
	return nmemb;
}

int so_fgetc(SO_FILE *stream)
{
	DWORD bytesRead;
	BOOL ret;
	unsigned char character;

	stream->write_syscall = 0;
	if (--stream->buffer_count <= 0) {
		ret = ReadFile(stream->fd, stream->buffer,
			BUFSIZE,
			&bytesRead,
			NULL);
		if (ret == FALSE) {
			stream->err = 1;
			return SO_EOF;
		}
		if (ret && !bytesRead) {
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

int so_fputc(int c, SO_FILE *stream)
{
	unsigned char character = (unsigned char) c;
	DWORD totalBytes = 0;
	DWORD bytesWrite = 0;
	BOOL ret;

	stream->write_syscall = 1;
	if (stream->buffer_count > BUFSIZE - 1) {
		stream->buffer_count = 0;
		do {
			ret = WriteFile(stream->fd,
				stream->buffer + totalBytes,
				BUFSIZE - totalBytes,
				&bytesWrite,
				NULL);
			if (ret == FALSE) {
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

int so_feof(SO_FILE *stream)
{
	return stream->end_of_file;
}

int so_ferror(SO_FILE *stream)
{
	return stream->err;
}

SO_FILE *so_popen(const char *command, const char *type)
{
	return NULL;
}

int so_pclose(SO_FILE *stream)
{
	return 0;
}
