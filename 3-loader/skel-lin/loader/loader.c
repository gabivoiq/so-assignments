/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "exec_parser.h"
#include "utils.h"

static so_exec_t *exec;
static struct sigaction action;
static struct sigaction old_action;
static int fd;
static int page_size;

typedef struct Page {
	uintptr_t addr_start;
	struct Page *next;
} Page;

static Page *mapped_pages;

int isEmpty(Page *head)
{
	return head == NULL;
}

void add_mapped_page_in_list(uintptr_t addr_start)
{
	Page *aux;
	Page *new;

	if (isEmpty(mapped_pages)) {
		mapped_pages = malloc(sizeof(Page));
		mapped_pages->addr_start = addr_start;
		mapped_pages->next = NULL;
		return;
	}

	new = malloc(sizeof(Page));

	aux = mapped_pages;
	while (aux->next != NULL)
		aux = aux->next;

	aux->next = new;
	new->next = NULL;
}

int search_mapped_page(uintptr_t addr_start)
{
	Page *aux = mapped_pages;

	while (aux != NULL) {
		if (aux->addr_start == addr_start)
			return 1;
		aux = aux->next;
	}
	return 0;
}

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	uintptr_t addr_segfault;
	int ret, i;
	char *mem;
	unsigned int mem_size, file_size, offset_page_to_map;

	addr_segfault = (uintptr_t) info->si_addr;

	/* search for mapped page */
	if (search_mapped_page(addr_segfault - addr_segfault % page_size)) {
		old_action.sa_sigaction(signum, info, context);
		return;
	}
	/* go through segments */
	for (i = 0; i < exec->segments_no; i++) {
		mem_size = exec->segments[i].mem_size;
		file_size = exec->segments[i].file_size;
		/* check if segfault address is contained in a segment */
		if (exec->segments[i].vaddr <= addr_segfault &&
			addr_segfault <
			exec->segments[i].vaddr + exec->segments[i].mem_size) {

			offset_page_to_map
			= (addr_segfault - exec->segments[i].vaddr) / page_size;

			/* map the correct page in virtual memory */
			mem = mmap((char *)(exec->segments[i].vaddr+
			page_size * offset_page_to_map),
			page_size,
			PROT_WRITE,
			MAP_ANONYMOUS | MAP_SHARED | MAP_FIXED,
			0,
			0);
			DIE(mem == MAP_FAILED, "mmap failed");

			add_mapped_page_in_list((uintptr_t) mem);

			ret = lseek(fd,
			exec->segments[i].offset
			+ offset_page_to_map * page_size, SEEK_SET);
			DIE(ret < 0, "lseek failed");

			if (file_size > offset_page_to_map * page_size) {
				/* case when segfault address is contained
				 * in Page 0....N (according to the schema
				 */
				if (addr_segfault >= exec->segments[i].vaddr
				+ file_size - file_size % page_size) {
					ret = read(fd, mem,
					file_size % page_size);
					DIE(ret < 0, "read failed");
					memset((void *) (exec->segments[i].vaddr
					+ page_size * offset_page_to_map
					+ file_size % page_size),
					0, page_size - file_size % page_size);
				} else {
					ret = read(fd, mem, page_size);
					DIE(ret < 0, "read failed");
				}
			} else {
				/* case when segfault address is contained
				 * in Page N+1....M (according to the schema
				 */
				if (addr_segfault >= exec->segments[i].vaddr
				+ mem_size - mem_size % page_size) {
					memset((void *) (exec->segments[i].vaddr
					+ page_size * offset_page_to_map), 0,
					addr_segfault % page_size);
				} else {
					memset((void *) (exec->segments[i].vaddr
					+ page_size * offset_page_to_map),
					0, page_size);
				}
			}
			/* correct protection after we write
			 * to the specific address
			 */
			ret = mprotect(mem, page_size, exec->segments[i].perm);
			DIE(ret == -1, "mprotect failed");
			return;
		}
	}
	/* default handler for segfault */
	old_action.sa_sigaction(signum, info, context);
}

int so_init_loader(void)
{
	/* initialize on-demand loader */

	int rc;

	action.sa_sigaction = segv_handler;
	sigemptyset(&action.sa_mask);
	sigaddset(&action.sa_mask, SIGSEGV);
	action.sa_flags = SA_SIGINFO;

	rc = sigaction(SIGSEGV, &action, &old_action);
	DIE(rc < 0, "sigaction failed");

	return 0;
}

int so_execute(char *path, char *argv[])
{
	page_size = getpagesize();
	fd = open(path, O_RDONLY);
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return 0;
}
