#ifdef __cplusplus
extern "C" {
#endif

void *elf_open(const char *name);
void elf_close(void *arg);
void *elf_getsym(void *arg, const char *name);

#ifdef __cplusplus
}
#endif
