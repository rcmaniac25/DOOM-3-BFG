/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Vincent Simonetti

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

#pragma hdrstop
#include "../../idlib/precompiled.h"

#include "qnx_local.h"
#include <sys/syspage.h>
#include "../../renderer/tr_local.h"
#include <EGL/eglext.h>

idCVar r_useGLES3( "r_useGLES3", "0", CVAR_INTEGER, "0 = OpenGL ES 3.0 if available, 1 = OpenGL ES 2.0, 2 = OpenGL 3.0", 0, 2 );

/*
========================
GLimp_glReadBuffer
========================
*/
void GLimp_glReadBuffer( GLenum mode ) {
	//TODO
}

/*
========================
GLimp_glDrawBuffer
========================
*/
void GLimp_glDrawBuffer( GLenum mode ) {
	//TODO
}

//XXX Functions that might need replacing-glDrawBuffers, gl functions that read and draw

//
// function declaration
//
bool QGL_Init( const char *dllname, const EGLFunctionReplacements_t & replacements );
void QGL_Shutdown();

/*
========================
GLimp_TestSwapBuffers
========================
*/
void GLimp_TestSwapBuffers( const idCmdArgs &args ) {
	idLib::Printf( "GLimp_TimeSwapBuffers\n" );
	static const int MAX_FRAMES = 5;
	uint64	timestamps[MAX_FRAMES];
	qglDisable( GL_SCISSOR_TEST );

	for ( int swapInterval = 2 ; swapInterval >= 0 ; swapInterval-- ) {
		qeglSwapInterval( qnx.eglDisplay, swapInterval );
		for ( int i = 0 ; i < MAX_FRAMES ; i++ ) {
			if ( i & 1 ) {
				qglClearColor( 0, 1, 0, 1 );
			} else {
				qglClearColor( 1, 0, 0, 1 );
			}
			qglClear( GL_COLOR_BUFFER_BIT );
			qeglSwapBuffers( qnx.eglDisplay, qnx.eglSurface );
			qglFinish();
			timestamps[i] = Sys_Microseconds();
		}

		idLib::Printf( "\nswapinterval %i\n", swapInterval );
		for ( int i = 1 ; i < MAX_FRAMES ; i++ ) {
			idLib::Printf( "%i microseconds\n", (int)(timestamps[i] - timestamps[i-1]) );
		}
	}
}

/*
========================
GLimp_SetGamma

The renderer calls this when the user adjusts r_gamma or r_brightness
========================
*/
void GLimp_SetGamma( unsigned short red[256], unsigned short green[256], unsigned short blue[256] ) {
	// XXX
	// The proper thing to do would be to change screen gamma and brightness by inverse calculation of gamma values... but when OpenGL is in use, those values seem to get
	// ignored by QNX. So it's not worth doing the calculation.

	// Note: Below is calculation and ranges (actual code is in RenderSystem_init's R_SetColorMappings). If the calculation is ever implemented and done,
	// and an error occurs, state which function failed (win32 uses SetDeviceGammaRamp, so the error is "common->Printf( "WARNING: SetDeviceGammaRamp failed.\n" );").

	/*
	gammaTable = 0xFFFF * ((j/255)^(1/gamma) + 0.5)

	j_min=0-128
	j_max=0-512

	gamma_min=0.5
	gamma_max=3

	gammaTable_gmin_jmin=0-0x4081
	gammaTable_gmin_jmax=0-0x4080D => 0-0xFFFF

	gammaTable_gmax_jmin=0-0xCB74
	gammaTable_gmax_jmax=0-0x142F6 => 0-0xFFFF
	 */
}

/*
====================
PrintDisplayMode
====================
*/
static void PrintDisplayMode( screen_display_mode_t & mode ) {
	//common->Printf( "          display index: %i\n", mode.index );
	common->Printf( "          width       : %i\n", mode.width );
	common->Printf( "          height      : %i\n", mode.height );
	common->Printf( "          refresh     : %i\n", mode.refresh );
	common->Printf( "          interlaced  : 0x%x\n", mode.interlaced ); //XXX Can this be broken down to what interlaced mode it's in?
	common->Printf( "          aspect_ratio: %ix%i\n", mode.aspect_ratio[0], mode.aspect_ratio[1] );
	common->Printf( "          flags       : 0x%x\n", mode.flags ); //XXX Can this be broken down to what the flags are?
}

/*
====================
PrintDisplayType
====================
*/
static void PrintDisplayType( int type ) {
	const char * dispType;
	switch ( type ) {
	case SCREEN_DISPLAY_TYPE_INTERNAL:			dispType = "Internal"; break;
	case SCREEN_DISPLAY_TYPE_COMPOSITE:			dispType = "Composite"; break;
	case SCREEN_DISPLAY_TYPE_SVIDEO:			dispType = "S-Video"; break;
	case SCREEN_DISPLAY_TYPE_COMPONENT_YPbPr:	dispType = "Component (YPbPr)"; break;
	case SCREEN_DISPLAY_TYPE_COMPONENT_RGB:		dispType = "Component (RGB)"; break;
	case SCREEN_DISPLAY_TYPE_COMPONENT_RGBHV:	dispType = "Component (RGBHV)"; break;
	case SCREEN_DISPLAY_TYPE_DVI:				dispType = "DVI"; break;
	case SCREEN_DISPLAY_TYPE_HDMI:				dispType = "HDMI"; break;
	case SCREEN_DISPLAY_TYPE_DISPLAYPORT:		dispType = "DisplayPort"; break;
	case SCREEN_DISPLAY_TYPE_OTHER:				dispType = "Other"; break;
	default:									dispType = "Unknown"; break;
	}
	common->Printf( "      Type              : %s\n", dispType );
}

/*
====================
PrintDisplayType
====================
*/
static void PrintDisplayTechnology( int tech ) {
	const char * dispTech;
	switch ( tech ) {
	case SCREEN_DISPLAY_TECHNOLOGY_CRT:		dispTech = "CRT"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_LCD:		dispTech = "LCD"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_PLASMA:	dispTech = "Plasma"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_LED:		dispTech = "LED"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_OLED:	dispTech = "OLED"; break;
	case SCREEN_DISPLAY_TECHNOLOGY_UNKNOWN:
	default:								dispTech = "Unknown"; break;
	}
	common->Printf( "      Technology        : %s\n", dispTech );
}

/*
====================
PrintDisplayTransparency
====================
*/
static void PrintDisplayTransparency( int trans ) {
	const char * transTech;
	switch ( trans ) {
	case SCREEN_TRANSPARENCY_SOURCE:		transTech = "Source"; break;
	case SCREEN_TRANSPARENCY_TEST:			transTech = "Test"; break;
	case SCREEN_TRANSPARENCY_SOURCE_COLOR:	transTech = "Source Color"; break;
	case SCREEN_TRANSPARENCY_SOURCE_OVER:	transTech = "Source Over"; break;
	case SCREEN_TRANSPARENCY_NONE:			transTech = "None"; break;
	case SCREEN_TRANSPARENCY_DISCARD:		transTech = "Discard"; break;
	default:								transTech = "Unknown"; break;
	}
	common->Printf( "      Transparency      : %s\n", transTech );
}

/*
====================
PrintDisplayPowerMode
====================
*/
static void PrintDisplayPowerMode( int power ) {
	const char * powerTech;
	switch ( power ) {
	case SCREEN_POWER_MODE_OFF:			powerTech = "Off"; break;
	case SCREEN_POWER_MODE_SUSPEND:		powerTech = "Suspend"; break;
	case SCREEN_POWER_MODE_LIMITED_USE:	powerTech = "Limited Use"; break;
	case SCREEN_POWER_MODE_ON:			powerTech = "On"; break;
	default:							powerTech = "Unknown"; break;
	}
	common->Printf( "      Power Mode        : %s\n", powerTech );
}

/*
====================
DumpAllDisplayDevices
====================
*/
void DumpAllDisplayDevices() {
	common->Printf( "\n" );

	int displayCount = 1;
	if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
		common->Printf( "ERROR:  screen_get_context_property_iv(..., SCREEN_PROPERTY_DISPLAY_COUNT, ...) failed!\n\n" );
		return;
	}

	screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
	if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
		Mem_Free( displayList );
		common->Printf( "ERROR:  screen_get_context_property_pv(..., SCREEN_PROPERTY_DISPLAYS, ...) failed!\n\n" );
		return;
	}

	screen_display_t display;
	screen_display_mode_t * modes;
	char buffer[1024];
	int vals[2];
	int64 longVal;
	for ( int displayNum = 0; displayNum < displayCount; displayNum++ ) {
		display = displayList[displayNum];

		// General display stats
		common->Printf( "display device: %i\n", displayNum );
		screen_get_display_property_cv( display, SCREEN_PROPERTY_ID_STRING, sizeof(buffer) - 1, buffer );
		common->Printf( "  ID     : %s\n", buffer );
		screen_get_display_property_cv( display, SCREEN_PROPERTY_VENDOR, sizeof(buffer) - 1, buffer );
		common->Printf( "  Vendor : %s\n", buffer );
		screen_get_display_property_cv( display, SCREEN_PROPERTY_PRODUCT, sizeof(buffer) - 1, buffer );
		common->Printf( "  Product: %s\n", buffer );

		// Detailed display stats
		screen_get_display_property_iv( display, SCREEN_PROPERTY_TYPE, vals );
		PrintDisplayType( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_TECHNOLOGY, vals );
		PrintDisplayTechnology( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_ATTACHED, vals );
		common->Printf( "      Attached          : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_DETACHABLE, vals );
		common->Printf( "      Detachable        : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_ROTATION, vals );
		common->Printf( "      Rotation          : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_DPI, vals );
		common->Printf( "      DPI               : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_NATIVE_RESOLUTION, vals );
		common->Printf( "      Native Resolution : %ix%i\n", vals[0], vals[1] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_PHYSICAL_SIZE, vals );
		common->Printf( "      Physical Size     : %ix%i\n", vals[0], vals[1] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_SIZE, vals );
		common->Printf( "      Size              : %ix%i\n", vals[0], vals[1] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_TRANSPARENCY, vals );
		PrintDisplayTransparency( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_GAMMA, vals );
		common->Printf( "      Gamma             : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_INTENSITY, vals );
		common->Printf( "      Intensity         : %i\n", vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_IDLE_STATE, vals );
		common->Printf( "      Idle              : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_llv( display, SCREEN_PROPERTY_IDLE_TIMEOUT, &longVal );
		common->Printf( "      Idle Timeout      : %lld\n", longVal );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_KEEP_AWAKES, vals );
		common->Printf( "      Keep Awake        : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_MIRROR_MODE, vals );
		common->Printf( "      Mirror Mode       : %s\n", ( vals[0] ? "True" : "False" ) );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_POWER_MODE, vals );
		PrintDisplayPowerMode( vals[0] );
		screen_get_display_property_iv( display, SCREEN_PROPERTY_PROTECTION_ENABLE, vals );
		common->Printf( "      Protection Enabled: %s\n", ( vals[0] ? "True" : "False" ) );

		// Modes
		screen_get_display_property_iv( display, SCREEN_PROPERTY_MODE_COUNT, vals );
		modes = ( screen_display_mode_t* )Mem_Alloc( sizeof( screen_display_mode_t ) * vals[0], TAG_TEMP );
		if ( screen_get_display_modes( display, vals[0], modes ) == 0 ) {
			for ( int mode = 0; mode < vals[0]; mode++ ) {
				screen_display_mode_t & displayMode = modes[mode];

				if ( displayMode.refresh < 30 ) {
					continue;
				}
				if ( displayMode.height < 720 ) {
					continue;
				}

				common->Printf( "          -------------------\n" );
				common->Printf( "          modeNum     : %i\n", mode );
				PrintDisplayMode( displayMode );
			}
		}
		Mem_Free( ( void* )modes );
	}

	Mem_Free( ( void* )displayList );

	common->Printf( "\n" );
}

class idSort_VidMode : public idSort_Quick< vidMode_t, idSort_VidMode > {
public:
	int Compare( const vidMode_t & a, const vidMode_t & b ) const {
		int wd = a.width - b.width;
		int hd = a.height - b.height;
		int fd = a.displayHz - b.displayHz;
		return ( hd != 0 ) ? hd : ( wd != 0 ) ? wd : fd;
	}
};

/*
====================
R_GetModeListForDisplay
====================
*/
bool R_GetModeListForDisplay( const int requestedDisplayNum, idList<vidMode_t> & modeList ) {
	modeList.Clear();

	bool	verbose = false;

	int displayCount = 1;
	if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
		return false;
	}

	if ( requestedDisplayNum >= displayCount ) {
		return false;
	}

	screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
	if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
		Mem_Free( displayList );
		return false;
	}

	screen_display_t display;
	char buffer[1024];
	int vals[2];
	screen_display_mode_t * modes;
	for ( int displayNum = requestedDisplayNum; ; displayNum++ ) {
		if ( displayNum >= displayCount ) {
			Mem_Free( displayList );
			return false;
		}

		display = displayList[displayNum];

		// Only get displays that are still attached
		if ( screen_get_display_property_iv( display, SCREEN_PROPERTY_ATTACHED, vals ) != 0 || !vals[0] ) {
			continue;
		}

		if ( verbose ) {
			common->Printf( "display device: %i\n", displayNum );
			screen_get_display_property_cv( display, SCREEN_PROPERTY_ID_STRING, sizeof(buffer) - 1, buffer );
			common->Printf( "  ID     : %s\n", buffer );
			screen_get_display_property_cv( display, SCREEN_PROPERTY_VENDOR, sizeof(buffer) - 1, buffer );
			common->Printf( "  Vendor : %s\n", buffer );
			screen_get_display_property_cv( display, SCREEN_PROPERTY_PRODUCT, sizeof(buffer) - 1, buffer );
			common->Printf( "  Product: %s\n", buffer );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_TYPE, vals );
			PrintDisplayType( vals[0] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_TECHNOLOGY, vals );
			PrintDisplayTechnology( vals[0] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_ROTATION, vals );
			common->Printf( "      Rotation          : %i\n", vals[0] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_DPI, vals );
			common->Printf( "      DPI               : %i\n", vals[0] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_NATIVE_RESOLUTION, vals );
			common->Printf( "      Native Resolution : %ix%i\n", vals[0], vals[1] );
			screen_get_display_property_iv( display, SCREEN_PROPERTY_PHYSICAL_SIZE, vals );
			common->Printf( "      Physical Size     : %ix%i\n", vals[0], vals[1] );
		}

		screen_get_display_property_iv( display, SCREEN_PROPERTY_MODE_COUNT, vals );
		modes = ( screen_display_mode_t* )Mem_Alloc( sizeof( screen_display_mode_t ) * vals[0], TAG_TEMP );
		if ( screen_get_display_modes( display, vals[0], modes ) == 0 ) {
			for ( int mode = 0; mode < vals[0]; mode++ ) {
				screen_display_mode_t & displayMode = modes[mode];

				// The internal display lists it's refresh rate at 59...
				if ( displayMode.refresh != 59 || displayMode.refresh != 60 || displayMode.refresh != 120 ) {
					continue;
				}
				if ( displayMode.height < 720 ) {
					continue;
				}

				if ( verbose ) {
					common->Printf( "          -------------------\n" );
					common->Printf( "          modeNum     : %i\n", mode );
					PrintDisplayMode( displayMode );
				}
				vidMode_t mode;
				//XXX Is rotation swapping needed for width/height
				mode.width = displayMode.width;
				mode.height = displayMode.height;
				mode.displayHz = displayMode.refresh;
				modeList.AddUnique( mode );
			}
		}
		Mem_Free( ( void* )modes );

		if ( modeList.Num() > 0 ) {
			break;
		}
	}

	Mem_Free( ( void* )displayList );

	if ( modeList.Num() > 0 ) {

		// sort with lowest resolution first
		modeList.SortWithTemplate( idSort_VidMode() );

		return true;
	}

	return false;
}

/*
=================
R_UpdateFramebuffers
=================
*/
void R_UpdateFramebuffers() {
	//TODO: change framebuffer formats (glConfig.sRGBFramebufferAvailable and r_useSRGB) and/or multisampling (r_multiSamples)
}

/*
=================
R_UpdateGLVersion
=================
*/
void R_UpdateGLESVersion() {
	//TODO: replace functions based on "actual" opengl version (glConfig.glVersion)
}

/*
=================
EGL_CheckExtension
=================
*/
bool EGL_CheckExtension( char *name ) {
	if ( !strstr( glConfig.egl_extensions_string, name ) ) {
		common->Printf( "X..%s not found\n", name );
		return false;
	}

	common->Printf( "...using %s\n", name );
	return true;
}

/*
===================
QNXGLimp_CreateWindow
===================
*/
bool QNXGLimp_CreateWindow( int x, int y, int width, int height, int fullScreen ) {

	const int screenFormat = SCREEN_FORMAT_RGBA8888;
	const int screenIdleMode = SCREEN_IDLE_MODE_KEEP_AWAKE;

#ifdef ID_QNX_X86
	int screenUsage = 0;
#else
	int screenUsage = SCREEN_USAGE_DISPLAY; // Physical device copy directly into physical display
#endif
#if defined(GL_ES_VERSION_3_0) && BBNDK_VERSION_AT_LEAST(10, 2, 0)
	screenUsage |= ( ( r_useGLES3.GetInteger() != 1 ) ? SCREEN_USAGE_OPENGL_ES3 : SCREEN_USAGE_OPENGL_ES2 );
#else
	screenUsage |= SCREEN_USAGE_OPENGL_ES2;
#endif

	if ( screen_create_context( &qnx.screenCtx, 0 ) != 0 ) {
		common->Printf( "Could not create screen context\n" );
		return false;
	}

	if ( screen_create_window( &qnx.screenWin, qnx.screenCtx ) != 0 ) {
		common->Printf( "Could not create screen window\n" );
		return false;
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_FORMAT, &screenFormat ) != 0 ) {
		common->Printf( "Could not set window format\n" );
		return false;
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_USAGE, &screenUsage ) != 0 ) {
#if defined(GL_ES_VERSION_3_0) && BBNDK_VERSION_AT_LEAST(10, 2, 0)
		// Try to fallback to GLES 2.0 for usage expectations
#ifdef ID_QNX_X86
		screenUsage = SCREEN_USAGE_OPENGL_ES2;
#else
		screenUsage = SCREEN_USAGE_DISPLAY | SCREEN_USAGE_OPENGL_ES2;
#endif
		if ( r_useGLES3.GetInteger() != 0 || screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_USAGE, &screenUsage ) != 0 ) {
			common->Printf( "Could not set window usage\n" );
			return false;
		}
#else
		common->Printf( "Could not set window usage\n" );
		return false;
#endif // defined(GL_ES_VERSION_3_0) && BBNDK_VERSION_AT_LEAST(10, 2, 0)
	}

	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_IDLE_MODE, &screenIdleMode ) != 0 ) {
		common->Printf( "Could not set window idle mode\n" );
		return false;
	}

	if ( r_debugContext.GetBool() ) {
		// Setup debug info (just a readout)
		int debugFlags = SCREEN_DEBUG_GRAPH_FPS;
		if ( r_debugContext.GetInteger() >= 2 ) {
			debugFlags |= SCREEN_DEBUG_GRAPH_CPU_TIME | SCREEN_DEBUG_GRAPH_GPU_TIME;
		}
		if ( r_debugContext.GetInteger() >= 3 ) {
			debugFlags = SCREEN_DEBUG_STATISTICS; // Note: We replace the value since it's not supposed to be a flag
		}
		if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_DEBUG, &debugFlags ) != 0 ) {
			common->Printf( "Could not set window debug flags\n" );
		}
	}

	const int position[2] = {x, y};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_POSITION, position ) != 0 ) {
		common->Printf( "Could not set window position\n" );
		return false;
	}

	const int size[2] = {width, height};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_BUFFER_SIZE, size ) != 0 ) {
		common->Printf( "Could not set window size\n" );
		return false;
	}

	// Change display if determined
	screen_display_t display;
	if ( fullScreen > 1 ) {
		int displayCount = 1;
		if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
			common->Printf( "Could not get display count\n" );
			return false;
		}

		if ( ( fullScreen - 1 ) >= displayCount ) {
			common->Printf( "fullScreen index does not correlate to a display\n" );
			return false;
		}

		screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
		if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
			Mem_Free( displayList );
			common->Printf( "Could not get displays\n" );
			return false;
		}
		display = displayList[ fullScreen - 1 ];
		Mem_Free( ( void* )displayList );

		if ( screen_set_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) != 0 ) {
			common->Printf( "Could not set window display\n" );
			return false;
		}
	}

	if ( screen_get_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) == 0 ) {
		screen_get_display_property_iv( display, SCREEN_PROPERTY_ID, &qnx.screenDisplayID );
	}

	if ( screen_create_window_buffers( qnx.screenWin, 2 ) != 0 ) {
		common->Printf( "Could not set window usage\n" );
		return false;
	}

	return true;
}

/*
===================
QNXGLimp_CreateEGLConfig
===================
*/
bool QNXGLimp_CreateEGLConfig( int multisamples, bool gles3bit ) {
	EGLint eglConfigAttrs[] =
	{
#ifdef GLES_MULTISAMPLE_EGL
		EGL_SAMPLE_BUFFERS,     ( ( multisamples >= 1 ) ? 1 : 0 ),
		EGL_SAMPLES,            multisamples,
#endif
		EGL_RED_SIZE,           8,
		EGL_GREEN_SIZE,         8,
		EGL_BLUE_SIZE,          8,
		EGL_ALPHA_SIZE,         8,
		EGL_DEPTH_SIZE,         24,
		EGL_STENCIL_SIZE,       8,
		EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
#ifdef EGL_OPENGL_ES3_BIT_KHR
		EGL_RENDERABLE_TYPE,    ( gles3bit ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT ),
#else
		EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
#endif
		EGL_NONE
	};

	EGLint eglConfigCount;
	bool success = true;

	if ( qeglChooseConfig( qnx.eglDisplay, eglConfigAttrs, &qnx.eglConfig, 1, &eglConfigCount ) != EGL_TRUE || eglConfigCount == 0 ) {
		success = false;
#if 0 // While useful, if the specified number of samples isn't supported then we want it to re-pick a config instead of saying one config while having another set
		while (multisamples) {
			// Try lowering the MSAA sample count until we find a supported config
			multisamples /= 2;
			eglConfigAttrs[1] = ( ( multisamples >= 1 ) ? 1 : 0 );
			eglConfigAttrs[3] = multisamples;

			if ( qeglChooseConfig( qnx.eglDisplay, eglConfigAttrs, &qnx.eglConfig, 1, &eglConfigCount ) == EGL_TRUE && eglConfigCount > 0 ) {
				success = true;
				break;
			}
		}
#endif
	}

	return success;
}

/*
===================
QNXGLimp_CreateEGLSurface
===================
*/
bool QNXGLimp_CreateEGLSurface( EGLNativeWindowType window, const int multisamples, bool createContextAvaliable ) {
	common->Printf( "Initializing OpenGL driver\n" );

	EGLint eglContextAttrs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION,		0,
		EGL_NONE,						EGL_NONE,
		EGL_NONE
	};
#ifdef GL_ES_VERSION_3_0
	eglContextAttrs[1] = 3;
	if ( r_useGLES3.GetInteger() == 1 ) {
		eglContextAttrs[1] = 2;
	}
#else
	eglContextAttrs[1] = 2;
	if ( r_useGLES3.GetInteger() == 2 ) {
		// Compiled with OpenGL ES 2.0, 3.0 is not available
		return false;
	}
#endif

#if defined( EGL_CONTEXT_FLAGS_KHR ) && defined( EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR )
	if ( r_debugContext.GetBool() && createContextAvaliable ) {
		eglContextAttrs[2] = EGL_CONTEXT_FLAGS_KHR;
		eglContextAttrs[3] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
	}
#endif

	if ( qeglInitialize( qnx.eglDisplay, NULL, NULL ) != EGL_TRUE ) {
		common->Printf( "...^3Could not initialize EGL display^0\n");
		return false;
	}

	if ( !QNXGLimp_CreateEGLConfig( multisamples, createContextAvaliable && r_useGLES3.GetInteger() != 1 ) ) {
		if ( r_useGLES3.GetInteger() != 0 || !QNXGLimp_CreateEGLConfig( multisamples, false ) ) {
			common->Printf( "...^3Could not determine EGL config^0\n");
			return false;
		}
	}

	for ( int config = 0; config < 2; config++ ) {
		qnx.eglContext = qeglCreateContext( qnx.eglDisplay, qnx.eglConfig, EGL_NO_CONTEXT, eglContextAttrs );
		if ( qnx.eglContext == EGL_NO_CONTEXT ) {
			// If we already tried to create the context or we only want a specific version of GLES, then fail
			if ( config == 1 || r_useGLES3.GetInteger() != 0 ) {
				common->Printf( "...^3Could not create EGL context^0\n");
				return false;
			}
			eglContextAttrs[1] = 2; // Switch to OpenGL ES 2.0
			continue;
		}
		break;
	}

	qnx.eglSurface = qeglCreateWindowSurface( qnx.eglDisplay, qnx.eglConfig, window, NULL );
	if ( qnx.eglSurface == EGL_NO_SURFACE ) {
		common->Printf( "...^3Could not create EGL window surface^0\n");
		return false;
	}

	common->Printf( "...making context current: " );
	if ( qeglMakeCurrent( qnx.eglDisplay, qnx.eglSurface, qnx.eglSurface, qnx.eglContext ) != EGL_TRUE ) {
		common->Printf( "^3failed^0\n");
		return false;
	}
	common->Printf( "succeeded\n" );

	return true;
}

/*
===================
GLimp_Init

This is the platform specific OpenGL initialization function.  It
is responsible for loading OpenGL, initializing it,
creating a window of the appropriate size, doing
fullscreen manipulations, etc.  Its overall responsibility is
to make sure that a functional OpenGL subsystem is operating
when it returns to the ref.

If there is any failure, the renderer will revert back to safe
parameters and try again.
===================
*/
bool GLimp_Init( glimpParms_t parms ) {
	const char	*driverName;

	// Some early checks to make sure supported params exist
	if ( parms.x != 0 || parms.y != 0 || parms.fullScreen <= 0 ) {
		return false;
	}

	cmdSystem->AddCommand( "testSwapBuffers", GLimp_TestSwapBuffers, CMD_FL_SYSTEM, "Times swapbuffer options" );

	common->Printf( "Initializing OpenGL subsystem with multisamples:%i stereo:%i\n",
			parms.multiSamples, parms.stereo );

	// this will load the dll and set all our qgl* function pointers,
	// but doesn't create a window

	const EGLFunctionReplacements_t replacements = {
		GLimp_glReadBuffer,	// glReadBufferImpl
		GLimp_glDrawBuffer	// glDrawBufferImpl
	};

	// r_glDriver is only intended for using instrumented OpenGL
	// so-s.  Normal users should never have to use it, and it is
	// not archived.
	driverName = r_glDriver.GetString()[0] ? r_glDriver.GetString() : "libGLESv2.so";
	if ( !QGL_Init( driverName, replacements ) ) {
		common->Printf( "^3GLimp_Init() could not load r_glDriver \"%s\"^0\n", driverName );
		return false;
	}

	qnx.eglDisplay = qeglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( qnx.eglDisplay == EGL_NO_DISPLAY ) {
		common->Printf( "...^3Could not get EGL display^0\n");
		return false;
	}

	glConfig.egl_extensions_string = qeglQueryString( qnx.eglDisplay, EGL_EXTENSIONS );

	if ( !QNXGLimp_CreateWindow( parms.x, parms.y, parms.width, parms.height, parms.fullScreen ) ) {
		GLimp_Shutdown();
		return false;
	}

	if ( !QNXGLimp_CreateEGLSurface( qnx.screenWin, parms.multiSamples, EGL_CheckExtension( "EGL_KHR_create_context" ) ) ) {
		GLimp_Shutdown();
		return false;
	}

	r_swapInterval.SetModified();	// force a set next frame
	glConfig.swapControlTearAvailable = false;
	glConfig.stereoPixelFormatAvailable = true;

	qeglGetConfigAttrib( qnx.eglDisplay, qnx.eglConfig, EGL_BUFFER_SIZE, &glConfig.colorBits );
	qeglGetConfigAttrib( qnx.eglDisplay, qnx.eglConfig, EGL_DEPTH_SIZE, &glConfig.depthBits );
	qeglGetConfigAttrib( qnx.eglDisplay, qnx.eglConfig, EGL_STENCIL_SIZE, &glConfig.stencilBits );

	//XXX Is rotation swapping needed for width/height and physicalScreenWidth?
	glConfig.isFullscreen = parms.fullScreen;
	glConfig.isStereoPixelFormat = parms.stereo;
	glConfig.nativeScreenWidth = parms.width;
	glConfig.nativeScreenHeight = parms.height;
	glConfig.multisamples = parms.multiSamples;

	glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted
									// should side-by-side stereo modes be consider aspect 0.5?

	screen_display_t display;
	glConfig.physicalScreenWidthInCentimeters = 100.0f;
	if ( screen_get_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) == 0 ) {
		int size[2];
		if ( screen_get_display_property_iv( display, SCREEN_PROPERTY_PHYSICAL_SIZE, size ) == 0 ) {
			glConfig.physicalScreenWidthInCentimeters = size[0] * 0.1f; //MM to CM
		}
	}

	// check logging
	GLimp_EnableLogging( ( r_logFile.GetInteger() != 0 ) );

	return true;
}

/*
===================
GLimp_SetScreenParms

Sets up the screen based on passed parms..
===================
*/
bool GLimp_SetScreenParms( glimpParms_t parms ) {
	// Some early checks to make sure supported params exist
	if ( parms.x != 0 || parms.y != 0 || parms.fullScreen <= 0 ) {
		return false;
	}

	//XXX If x,y,width, height have changed then change window properties
	//XXX Need to destroy and recreate EGL surface if size/position are changing if window position and size change (reset interval too)

	const int position[2] = {parms.x, parms.y};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_POSITION, position ) != 0 ) {
		common->Printf( "Could not set window position\n" );
		return false;
	}

	const int size[2] = {parms.width, parms.height};
	if ( screen_set_window_property_iv( qnx.screenWin, SCREEN_PROPERTY_BUFFER_SIZE, size ) != 0 ) {
		common->Printf( "Could not set window size\n" );
		return false;
	}
	//XXX Is rotation swapping needed for width/height?
	glConfig.nativeScreenWidth = parms.width;
	glConfig.nativeScreenHeight = parms.height;

	r_swapInterval.SetModified();
	//XXX END changing window properties

	// Get displays
	int displayCount = 1;
	if ( screen_get_context_property_iv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAY_COUNT, &displayCount ) != 0 ) {
		common->Printf( "Could not get display count\n" );
		return false;
	}

	if ( ( parms.fullScreen - 1 ) >= displayCount ) {
		common->Printf( "fullScreen index does not correlate to a display\n" );
		return false;
	}

	screen_display_t* displayList = ( screen_display_t* )Mem_Alloc( sizeof( screen_display_t ) * displayCount, TAG_TEMP );
	if ( screen_get_context_property_pv( qnx.screenCtx, SCREEN_PROPERTY_DISPLAYS, ( void** )displayList ) != 0 ) {
		Mem_Free( displayList );
		common->Printf( "Could not get displays\n" );
		return false;
	}
	screen_display_t display = displayList[ parms.fullScreen - 1 ];
	Mem_Free( ( void* )displayList );

	int displayId;
	screen_get_display_property_iv( display, SCREEN_PROPERTY_ID, &displayId );
	if ( displayId != qnx.screenDisplayID && screen_set_window_property_pv( qnx.screenWin, SCREEN_PROPERTY_DISPLAY, ( void** )&display ) != 0 ) {
		common->Printf( "Could not set window display\n" );
		return false;
	}
	qnx.screenDisplayID = displayId;

	glConfig.isFullscreen = parms.fullScreen;
	glConfig.pixelAspect = 1.0f;	// FIXME: some monitor modes may be distorted

	return true;
}

/*
===================
GLimp_Shutdown

This routine does all OS specific shutdown procedures for the OpenGL
subsystem.
===================
*/
void GLimp_Shutdown() {
	const char *success[] = { "failed", "success" };
	int retVal;

	common->Printf( "Shutting down OpenGL subsystem\n" );

	qnx.eglConfig = NULL;

	if ( qnx.eglDisplay != EGL_NO_DISPLAY ) {
		retVal = qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		common->Printf( "...qeglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT ): %s\n", success[retVal] );
	}

	if ( qnx.eglSurface != EGL_NO_SURFACE ) {
		retVal = qeglDestroySurface( qnx.eglDisplay, qnx.eglSurface );
		common->Printf( "...destroying surface: %s\n", success[retVal] );
		qnx.eglSurface = EGL_NO_SURFACE;
	}

	if ( qnx.eglContext != EGL_NO_CONTEXT ) {
		retVal = qeglDestroyContext( qnx.eglDisplay, qnx.eglContext );
		common->Printf( "...destroying context: %s\n", success[retVal] );
		qnx.eglContext = EGL_NO_CONTEXT;
	}

	if ( qnx.eglDisplay != EGL_NO_DISPLAY ) {
		retVal = qeglTerminate( qnx.eglDisplay );
		common->Printf( "...terminating display: %s\n", success[retVal] );
		qnx.eglDisplay = EGL_NO_DISPLAY;
	}

	if ( qnx.screenWin ) {
		retVal = screen_destroy_window( qnx.screenWin ) + 1;
		common->Printf( "...destroying window: %s\n", success[retVal] );
		qnx.screenWin = NULL;
	}

	if ( qnx.screenCtx ) {
		retVal = screen_destroy_context( qnx.screenCtx ) + 1;
		common->Printf( "...destroying window context: %s\n", success[retVal] );
		qnx.screenCtx = NULL;
	}

	if ( qnx.renderThread ) {
		common->Printf( "...closing smp thread\n" );
		Sys_DestroyThread( qnx.renderThread );
		qnx.renderThread = NULL;
	}

	if ( qnx.renderCommandsEvent ) {
		common->Printf( "...destroying smp signals\n" );
		Sys_SignalDestroy( qnx.renderCommandsEvent );
		Sys_SignalDestroy( qnx.renderCompletedEvent );
		Sys_SignalDestroy( qnx.renderActiveEvent );
		qnx.renderCommandsEvent = NULL;
		qnx.renderCompletedEvent = NULL;
		qnx.renderActiveEvent = NULL;
	}

	// shutdown QGL subsystem
	QGL_Shutdown();
}

/*
=====================
GLimp_SwapBuffers
=====================
*/
void GLimp_SwapBuffers() {
	//TODO: flush backbuffer to default framebuffer, then reset to prior buffer

	if ( r_swapInterval.IsModified() ) {
		r_swapInterval.ClearModified();

		int interval = 0;
		if ( r_swapInterval.GetInteger() != 0 ) {
			interval = 1;
		}

		qeglSwapInterval( qnx.eglDisplay, interval );
	}

	qeglSwapBuffers( qnx.eglDisplay, qnx.eglSurface );
}

/*
===========================================================

SMP acceleration

===========================================================
*/

/*
===================
GLimp_ActivateContext
===================
*/
void GLimp_ActivateContext() {
	qeglMakeCurrent( qnx.eglDisplay, qnx.eglSurface, qnx.eglSurface, qnx.eglContext );
}

/*
===================
GLimp_DeactivateContext
===================
*/
void GLimp_DeactivateContext() {
	qglFinish();
	qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
}

/*
===================
GLimp_RenderThreadWrapper
===================
*/
static unsigned int GLimp_RenderThreadWrapper( void* args ) {
	void (*renderThread)() = ( void (*)() )args;

	renderThread();

	// unbind the context before we die
	return qeglMakeCurrent( qnx.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
}

/*
=======================
GLimp_SpawnRenderThread

Returns false if the system only has a single processor
=======================
*/
bool GLimp_SpawnRenderThread( void (*function)() ) {
	// check number of processors
	if ( _syspage_ptr->num_cpu < 2 ) {
		return false;
	}

	//What if the values already exist? The Windows version doesn't do anything (though it doesn't seem to cleanup it's events either...)

	// create the IPC elements
	Sys_SignalCreate( qnx.renderCommandsEvent, true );
	Sys_SignalCreate( qnx.renderCompletedEvent, true );
	Sys_SignalCreate( qnx.renderActiveEvent, true );

	qnx.renderThread = Sys_CreateThread( GLimp_RenderThreadWrapper, ( void* )function, THREAD_ABOVE_NORMAL, "qnx_renderThread", CORE_ANY );

	return true;
}


//#define	DEBUG_PRINTS

/*
===================
GLimp_BackEndSleep
===================
*/
void *GLimp_BackEndSleep() {
	void	*data;

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "-->GLimp_BackEndSleep\n" );
#endif
	Sys_SignalClear( qnx.renderActiveEvent );

	// after this, the front end can exit GLimp_FrontEndSleep
	Sys_SignalRaise( qnx.renderCompletedEvent );

	Sys_SignalWait( qnx.renderCommandsEvent, idSysSignal::WAIT_INFINITE );

	Sys_SignalClear( qnx.renderCompletedEvent );
	Sys_SignalClear( qnx.renderCommandsEvent );

	data = qnx.smpData;

	// after this, the main thread can exit GLimp_WakeRenderer
	Sys_SignalRaise( qnx.renderActiveEvent );

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "<--GLimp_BackEndSleep\n" );
#endif
	return data;
}

/*
===================
GLimp_FrontEndSleep
===================
*/
void GLimp_FrontEndSleep() {
#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "-->GLimp_FrontEndSleep\n" );
#endif
	Sys_SignalWait( qnx.renderCompletedEvent, idSysSignal::WAIT_INFINITE );

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "<--GLimp_FrontEndSleep\n" );
#endif
}

volatile bool	renderThreadActive;

// We can use Sys_SignalWait, but it will simply return false if something goes wrong
int Sys_SignalWait_ErrorReturn( signalHandle_t & handle, int timeout );

/*
===================
GLimp_WakeBackEnd
===================
*/
void GLimp_WakeBackEnd( void *data ) {
	int		r;

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "-->GLimp_WakeBackEnd\n" );
#endif

	qnx.smpData = data;

	if ( renderThreadActive ) {
		common->FatalError( "GLimp_WakeBackEnd: already active" );
	}

	r = Sys_SignalWait_ErrorReturn( qnx.renderActiveEvent, 0 );
	if ( r == EOK ) {
		common->FatalError( "GLimp_WakeBackEnd: already signaled" );
	}

	r = Sys_SignalWait_ErrorReturn( qnx.renderCommandsEvent, 0 );
	if ( r == EOK ) {
		common->FatalError( "GLimp_WakeBackEnd: commands already signaled" );
	}

	// after this, the renderer can continue through GLimp_RendererSleep
	Sys_SignalRaise( qnx.renderCommandsEvent );

	r = Sys_SignalWait_ErrorReturn( qnx.renderActiveEvent, 5000 );

	if ( r == ETIMEDOUT ) {
		common->FatalError( "GLimp_WakeBackEnd: WAIT_TIMEOUT" );
	}

#ifdef DEBUG_PRINTS
	slog2c( NULL, SLOG_CODE, SLOG2_DEBUG2, "<--GLimp_WakeBackEnd\n" );
#endif
}

/*
===================
GLimp_ExtensionPointer

Returns a function pointer for an OpenGL extension entry point
===================
*/
GLExtension_t GLimp_ExtensionPointer( const char *name ) {
	void	(*proc)();

	proc = (GLExtension_t)qeglGetProcAddress( name );

	if ( !proc ) {
		common->Printf( "Couldn't find proc address for: %s\n", name );
	}

	return proc;
}
