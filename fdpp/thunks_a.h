struct athunk {
    struct far_s *ptr;
#define THUNKF_SHORT 1
#define THUNKF_DEEP 2
    unsigned flags;
};

extern struct athunk asm_thunks[];
extern const int num_athunks;
