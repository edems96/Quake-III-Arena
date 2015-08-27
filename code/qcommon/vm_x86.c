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
// vm_x86.c -- load time compiler and execution environment for x86

#include "vm_local.h"

#ifdef __FreeBSD__ // rb0101023
#include <sys/types.h>
#endif

#ifndef _WIN32
#include <sys/mman.h> // for PROT_ stuff
#endif

// TTimo: initialised the statics, this fixes a crash when entering a compiled VM 
static	byte	*buf = NULL;
static	byte	*jused = NULL;
static	int		compiledOfs = 0;
static	byte	*code = NULL;
static	int		pc = 0;

static	int		*instructionPointers = NULL;

#define FTOL_PTR

#ifdef _WIN32

#else // _WIN32

#if defined( FTOL_PTR )
// bk001213 - BEWARE: does not work! UI menu etc. broken - stack!
// bk001119 - added: int gftol( float x ) { return (int)x; }

int qftol( void );     // bk001213 - label, see unix/ftol.nasm
int qftol027F( void ); // bk001215 - fixed FPU control variants
int qftol037F( void );
int qftol0E7F( void ); // bk010102 - fixed bogus bits (duh)
int qftol0F7F( void );


static	int		ftolPtr = (int)qftol0F7F;
#endif // FTOL_PTR

void doAsmCall( void );
static	int		asmCallPtr = (int)doAsmCall;
#endif // !_WIN32


static	int		callMask = 0; // bk001213 - init

static	int	instruction, pass;
static	int	lastConst = 0;
static	int	oc0, oc1, pop0, pop1;