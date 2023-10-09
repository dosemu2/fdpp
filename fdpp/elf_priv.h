void *elf_open(const char *name);
void elf_reloc(void *arg, uint16_t seg);
void elf_close(void *arg);
void *elf_getsym(void *arg, const char *name);
int elf_getsymoff(void *arg, const char *name);
