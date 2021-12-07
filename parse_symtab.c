#ifdef __linux__
//for memmem on Linux
#define _GNU_SOURCE 1
#endif //__linux__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if 0
#define DPRINT(fmt, args...) do {} while (0)
#else
#define DPRINT(fmt, args...) fprintf(stderr, fmt, ##args)
#endif

#define CHECK(cond) do { \
	if (!(cond)) { \
		fprintf(stderr, "%s:%d: failed to check '%s'\n", \
				__func__, __LINE__, #cond); \
		goto err; \
	} \
} while (0)

struct sym_ref {
	uint64_t va;
	uint32_t pad;
	uint32_t str_offset;
} __attribute__((packed));

#define FIRST_SYM "__code_start"
#define LAST_SYM "__exidx_start"

int parse_symtab_mem(void *mem, size_t size)
{
	char *umem = (char*)mem;
	void *str_start = memmem(mem, size, FIRST_SYM, sizeof(FIRST_SYM));
	CHECK(str_start != NULL);

	void *str_end = memmem(mem, size, LAST_SYM, sizeof(LAST_SYM));
	CHECK(str_end != NULL);

	char *ustr_start = (char*)str_start;
	char *ustr_end = (char*)str_end;
	ustr_end += sizeof(LAST_SYM);

	CHECK(ustr_start - umem >= sizeof(struct sym_ref));
	struct sym_ref *sym_ref = ((struct sym_ref*)ustr_start) - 1;

	char *str_ptr = ustr_start;
	while (str_ptr < ustr_end)
	{
		char *in_str_ptr = str_ptr;
		while (*in_str_ptr++) {}

		//save symbol name and offset
		char *sym_name = str_ptr;
		size_t offset = str_ptr - ustr_start;
		DPRINT("string '%s' offset=%08zx\n",
				sym_name, offset);
		str_ptr = in_str_ptr;

		//rewind sym_ref to the entry with the matching or smaller offset.
		//this will not get quadratic because all entries are sorted.
		//we rewind once and then only increment the pointer
		while (sym_ref->str_offset > offset) {
			CHECK(((char*)sym_ref) - umem >= sizeof(struct sym_ref));
			sym_ref--;
		}

		struct sym_ref *lref = sym_ref;
		while (lref->str_offset != offset && ((char*)lref < ustr_start))
		{
			DPRINT("\tlref++: lref->str_offset=%08x offset=%08zx\n",
					lref->str_offset, offset);
			lref++;
		}

		//still not found?
		if (lref->str_offset != offset) {
			DPRINT("\t no ref found for func '%s'\n", sym_name);
			continue;
		}

		printf("%16zx %s\n", lref->va, sym_name);

		//assuming everything's sorted, it should be safe to update
		//the sym_ref head to current to avoid quadratic scan
		sym_ref = lref;
	}

	return 0;
err:
	return -1;
}

int parse_symtab(char *fname) {
	int fd = -1;
	struct stat s = {};
	size_t size = 0;
	void *mem = NULL;

	CHECK((fd = open(fname, O_RDONLY)) >= 0);
	CHECK(fstat(fd, &s) >= 0);
	size = s.st_size;

	mem = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	CHECK(mem != MAP_FAILED);

	return parse_symtab_mem(mem, size);
err:
	if (fd >= 0) {
		close(fd);
	}
	return -1;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		return -1;
	}
	char *fname = argv[1];
	return parse_symtab(fname);
}
