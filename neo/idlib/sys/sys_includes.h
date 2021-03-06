/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014 Vincent Simonetti

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef SYS_INCLUDES_H
#define SYS_INCLUDES_H

// Include the various platform specific header files (windows.h, etc)

/*
================================================================================================

	Windows

================================================================================================
*/


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// prevent auto literal to string conversion

#ifdef ID_WIN32
#ifndef _D3SDK
#ifndef GAME_DLL

#define WINVER				0x501

#include <winsock2.h>
#include <mmsystem.h>
#include <mmreg.h>

#define DIRECTINPUT_VERSION  0x0800			// was 0x0700 with the old mssdk
#define DIRECTSOUND_VERSION  0x0800

#include <dsound.h>
#include <dinput.h>

#endif /* !GAME_DLL */
#endif /* !_D3SDK */

#include <intrin.h>			// needed for intrinsics like _mm_setzero_si28

#pragma warning(disable : 4100)				// unreferenced formal parameter
#pragma warning(disable : 4127)				// conditional expression is constant
#pragma warning(disable : 4244)				// conversion to smaller type, possible loss of data
#pragma warning(disable : 4714)				// function marked as __forceinline not inlined
#pragma warning(disable : 4996)				// unsafe string operations

#include <malloc.h>							// no malloc.h on mac or unix
#include <windows.h>						// for qgl.h
#undef FindText								// fix namespace pollution
#endif

/*
================================================================================================

	QNX

================================================================================================
*/

#ifdef ID_QNX

#include <stddef.h>
#include <stdint.h>

#include <semaphore.h>
#include <pthread.h>

#include <alloca.h>
#include <assert.h>
#include <wctype.h>

#include <unistd.h>

#ifdef ID_QNX_X86
#include <x86intrin.h>
#endif

#include <slog2.h>

//Only needed for FlushCacheLine
#include <sys/mman.h>
#include <sys/cache.h>

#ifdef ID_QNX_ARM_NEON
#include <arm_neon.h>
#endif

//This does some setup for versions (shouldn't really be here, but we don't want to include files anywhere else)
#include <bps/bps.h>
#if BPS_VERSION >= 3001002 //Since we will get a compile error if we try to use bbndk.h before it was added
#include <bbndk.h>
#else
#define BBNDK_VERSION_CURRENT_MAJOR 10
#define BBNDK_VERSION_CURRENT_MINOR 0
#define BBNDK_VERSION_CURRENT_PATCH 9 //First public version of BB10 was 10.0.9
#define BBNDK_VERSION_ENCODE(major, minor, patch) (((major)*1000000)+((minor)*1000)+(patch))
#define BBNDK_VERSION_CURRENT BBNDK_VERSION_ENCODE(BBNDK_VERSION_CURRENT_MAJOR,BBNDK_VERSION_CURRENT_MINOR,BBNDK_VERSION_CURRENT_PATCH)
#define BBNDK_VERSION_AT_LEAST(major, minor, patch) (BBNDK_VERSION_CURRENT >= BBNDK_VERSION_ENCODE(major, minor, patch))
#endif

#if BBNDK_VERSION_AT_LEAST(10, 2, 0)
#define ID_OPENGL_ES_3 0 //XXX make sure versions determine what this actually is
#else
#define ID_OPENGL_ES_2
#endif

#endif

/*
================================================================================================

	Common Include Files

================================================================================================
*/

#if !defined( _DEBUG ) && !defined( NDEBUG )
	// don't generate asserts
	#define NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>
#include <typeinfo>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <memory>

//-----------------------------------------------------

// Hacked stuff we may want to consider implementing later
class idScopedGlobalHeap {
};

#endif // SYS_INCLUDES_H
