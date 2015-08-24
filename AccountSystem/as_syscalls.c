#include "as_local.h"

#ifdef Q3_VM
#error "Do not use in VM build"
#endif

static int (QDECL *syscall)(int arg, ...) = (int (QDECL *)(int, ...)) - 1;

void dllEntry(int (QDECL *syscallptr)(int arg, ...)) {
	syscall = syscallptr;
}