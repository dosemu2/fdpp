#include <stdint.h>

void *fdelf_open(const char *name);
void fdelf_reloc(void *arg, uint16_t seg);
void fdelf_close(void *arg);
void *fdelf_getsym(void *arg, const char *name);
int fdelf_getsymoff(void *arg, const char *name);
void *fdelf_getloadaddr(void *arg);
int fdelf_getloadoff(void *arg);
