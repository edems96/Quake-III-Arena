#include "../code/client/client.h"
#include "../AccountSystem/as_public.h"

vm_t *asvm;
asImport_t as;

void *VM_ArgPtr(int intValue);
#define	VMA(x) VM_ArgPtr(args[x])

int AS_SystemCalls( int *args ) {

	Com_Printf("AS_SystemCalls\n");
	switch( args[0] ) {
		//case as.AS_PRINTF: return 0;
		//case COM_PRINTF: Com_Printf("%s", VMA(0)); return 0;
	}

	return 0;
}


void AS_InitVM() {
	asvm = VM_Create("as", AS_SystemCalls);

	if (!asvm)
		Com_Error(ERR_DROP, "VM_Create on accountsystem failed");
	
	Com_Printf("created VM\n");
	VM_Call(asvm, AS_INIT);
}