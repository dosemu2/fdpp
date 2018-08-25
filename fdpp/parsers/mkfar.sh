mkfar() {
    sed -e ':loop' \
	-e "s/\([^(, ]\+\) $1 *\*/$2(\1)/" \
	-e "s/struct $2(/$2(struct /" \
	-e "s/const $2(/$2(const /" \
	-e "s/CONST $2(/$2(CONST /" \
	-e "s/unsigned $2(/$2(unsigned /" \
	-e 't loop' $3
}

mkfar FAR __FAR $1
