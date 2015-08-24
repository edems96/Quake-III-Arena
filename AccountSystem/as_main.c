#include "as_local.h"
#include "as_public.h"
#include <stdio.h>

int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {

	switch (command) {
		case AS_INIT: AS_Init(); return 0;
		case AS_LOGIN: AS_Login(arg0, arg1); return 0;
	}

	return -1;
}

void AS_Init() {
	FILE * f;
	f = fopen("test.txt", "a+");
	fprintf(f, "hello as\n");
	fclose(f);
}

void AS_Login(const char * name, const char * pass) {

}