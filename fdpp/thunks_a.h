struct athunk {
    const char *name;
    struct far_s *ptr;
#define THUNKF_SHORT 1
#define THUNKF_DEEP 2
    unsigned flags;
};

struct cthunk {
    const char *name;
    int num;
    struct far_s *ptr;
};

extern struct athunk asm_thunks[];
extern const int num_athunks;
extern struct cthunk asm_cthunks[];
extern const int num_cthunks;
extern far_t asm_tab[];
