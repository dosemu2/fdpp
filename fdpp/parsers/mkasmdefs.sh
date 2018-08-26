grep "__ASM(" $1 | grep -v '^/' | \
	sed 's/.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >$2
grep "__ASM_ARR(" $1 | grep -v '^/' | \
	sed 's/.\+(.\+, \(.\+\), .\+).*/#define \1 __ASYM(__\1)/' >>$2
grep "__ASM_ARRI(" $1 | grep -v '^/' | \
	sed 's/.\+(.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$2
grep "__ASM_ARRI_F(" $1 | grep -v '^/' | \
	sed 's/.\+(.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$2
grep "__ASM_FAR(" $1 | grep -v '^/' | \
	sed 's/.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$2
grep "__ASM_NEAR(" $1 | grep -v '^/' | \
	sed 's/.\+, \(.\+\)).*/#define \1 __ASYM(__\1)/' >>$2
grep "__ASM_FUNC(" $1 | grep -v '^/' | \
	sed 's/.\+(\(.\+\)).*/#define \1 __ASYM(__\1)/' >>$2
