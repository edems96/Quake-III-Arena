/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// vm.c -- virtual machine

/*


intermix code and data
symbol table

a dll has one imported function: VM_SystemCall
and one exported function: Perform


*/

#include "vm_local.h"

#define	MAX_VM		4
vm_t	vmTable[MAX_VM];
vm_t	*currentVM = NULL;

void VM_Init( void ) {
	Com_Memset( vmTable, 0, sizeof( vmTable ) );
}

/*
============
VM_DllSyscall

Dlls will call this directly

 rcg010206 The horror; the horror.

  The syscall mechanism relies on stack manipulation to get it's args.
   This is likely due to C's inability to pass "..." parameters to
   a function in one clean chunk. On PowerPC Linux, these parameters
   are not necessarily passed on the stack, so while (&arg[0] == arg)
   is true, (&arg[1] == 2nd function parameter) is not necessarily
   accurate, as arg's value might have been stored to the stack or
   other piece of scratch memory to give it a valid address, but the
   next parameter might still be sitting in a register.

  Quake's syscall system also assumes that the stack grows downward,
   and that any needed types can be squeezed, safely, into a signed int.

  This hack below copies all needed values for an argument to a
   array in memory, so that Quake can get the correct values. This can
   also be used on systems where the stack grows upwards, as the
   presumably standard and safe stdargs.h macros are used.

  As for having enough space in a signed int for your datatypes, well,
   it might be better to wait for DOOM 3 before you start porting.  :)

  The original code, while probably still inherently dangerous, seems
   to work well enough for the platforms it already works on. Rather
   than add the performance hit for those platforms, the original code
   is still in use there.

  For speed, we just grab 15 arguments, and don't worry about exactly
   how many the syscall actually needs; the extra is thrown away.
 
============
*/
int QDECL VM_DllSyscall( int arg, ... ) {
#if ((defined __linux__) && (defined __powerpc__))
  // rcg010206 - see commentary above
  int args[16];
  int i;
  va_list ap;
  
  args[0] = arg;
  
  va_start(ap, arg);
  for (i = 1; i < sizeof (args) / sizeof (args[i]); i++)
    args[i] = va_arg(ap, int);
  va_end(ap);
  
  return currentVM->systemCall( args );
#else // original id code
	return currentVM->systemCall( &arg );
#endif
}

/*
=================
VM_Restart

Reload the data, but leave everything else in place
This allows a server to do a map_restart without changing memory allocation
=================
*/
vm_t *VM_Restart( vm_t *vm ) {
	char		name[MAX_QPATH];
	int			(*systemCall)( int *parms );
		
	systemCall = vm->systemCall;	
	Q_strncpyz( name, vm->name, sizeof( name ) );

	VM_Free( vm );

	vm = VM_Create(name, systemCall);
	return vm;
}

/*
================
VM_Create

If image ends in .qvm it will be interpreted, otherwise
it will attempt to load as a system dll
================
*/

vm_t *VM_Create(const char *module, int (*systemCalls)(int *)) {
	vm_t	*vm;
	int		i;

	if ( !module || !module[0] || !systemCalls ) {
		Com_Error( ERR_FATAL, "VM_Create: bad parms" );
	}

	//remaining = Hunk_MemoryRemaining();

	// see if we already have the VM
	for ( i = 0 ; i < MAX_VM ; i++ ) {
		if (!Q_stricmp(vmTable[i].name, module)) {
			vm = &vmTable[i];
			return vm;
		}
	}

	// find a free vm
	for ( i = 0 ; i < MAX_VM ; i++ ) {
		if ( !vmTable[i].name[0] ) {
			break;
		}
	}

	if ( i == MAX_VM ) {
		Com_Error( ERR_FATAL, "VM_Create: no free vm_t" );
	}

	vm = &vmTable[i];

	Q_strncpyz( vm->name, module, sizeof( vm->name ) );
	vm->systemCall = systemCalls;

	Com_Printf( "Loading dll file '%s'.\n", vm->name );
	vm->dllHandle = Sys_LoadDll( module, vm->fqpath , &vm->entryPoint, VM_DllSyscall );

	if ( vm->dllHandle ) {
		return vm;
	}

	Com_Printf( "Failed to load dll!\n" );
	return NULL;
}

/*
==============
VM_Free
==============
*/
void VM_Free(vm_t *vm) {
	Sys_UnloadDll( vm->dllHandle );
	Com_Memset( vm, 0, sizeof( *vm ) );
	currentVM = NULL;
}

void VM_Clear(void) {
	for (int i = 0; i < MAX_VM; i++) {
		if ( vmTable[i].dllHandle ) {
			Sys_UnloadDll( vmTable[i].dllHandle );
		}

		Com_Memset( &vmTable[i], 0, sizeof( vm_t ) );
	}

	currentVM = NULL;
}

/*
==============
VM_Call
==============
*/
int	QDECL VM_Call( vm_t *vm, int callnum, ... ) {
	vm_t	*oldVM;
	int i;
	int args[16];
	va_list ap;

	if ( !vm ) {
		Com_Error( ERR_FATAL, "VM_Call with NULL vm" );
	}
	
	oldVM = currentVM;
	currentVM = vm;

	va_start(ap, callnum);
	for (i = 0; i < sizeof (args) / sizeof (args[i]); i++) {
		args[i] = va_arg(ap, int);
	}
	va_end(ap);

	int r = vm->entryPoint(callnum,  args[0],  args[1],  args[2], args[3],
                       args[4],  args[5],  args[6], args[7],
                       args[8],  args[9], args[10], args[11],
                       args[12], args[13], args[14], args[15]);

	if ( oldVM != NULL )
		currentVM = oldVM;

	return r;
}

void *VM_ArgPtr(int intValue) {
	return !intValue ? NULL : (void *)intValue;
}
