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
#pragma hdrstop
#include "../../idlib/precompiled.h"

/*
================================================================================================
Contains the Image implementation for OpenGL.
================================================================================================
*/

#include "../tr_local.h"

#ifdef GL_ES_VERSION_2_0

void SwapBytes_16bit( uint16 *dst, const uint16 *src, int len ) {
	int i = 0;
	// X86 might be useful, but SSE lacks an ability to swap bytes, only elements and doesn't support byte elements
#if defined( ID_QNX_ARM_NEON_INTRIN )
	for ( ; i + 16 <= len; i += 16 ) {
		uint8x16x2_t data = vld2q_u8( (uint8_t *)&src[i] );

		uint8x16_t tmp = data.val[0];
		data.val[0] = data.val[1];
		data.val[1] = tmp;

		vst2q_u8( (uint8_t *)&dst[i], data );
	}
#elif defined( ID_QNX_ARM_NEON_ASM )
	for ( ; i + 16 <= len; i += 16 ) {
		__asm__ __volatile__(
				"MOV r1, #2\n"
				"MLA r0, %[i], r1, %[s]\n"
				"MLA r1, %[i], r1, %[d]\n"
				"VLD2.8 {q0, q1}, [r0]\n"
				"VSWP q0, q1\n"
				"VST2.8 {q0, q1}, [r1]"
				: [d] "+r" (dst) : [i] "r" (i), [s] "r" (src) : "r0", "r1", "q0", "q1", "memory");
	}
#endif
	for ( ; i < len; i++ ) {
		uint16 s = src[i];
		dst[i] = ( ( s & 0xFF00 ) >> 8 ) | ( ( s & 0x00FF ) << 8 );
	}
}

#endif

/*
========================
idImage::SubImageUpload
========================
*/
void idImage::SubImageUpload( int mipLevel, int x, int y, int z, int width, int height, const void * pic, int pixelPitch ) const {
	assert( x >= 0 && y >= 0 && mipLevel >= 0 && width >= 0 && height >= 0 && mipLevel < opts.numLevels );

	int compressedSize = 0;

	if ( IsCompressed() ) {
		assert( !(x&3) && !(y&3) );

		// compressed size may be larger than the dimensions due to padding to quads
		int quadW = ( width + 3 ) & ~3;
		int quadH = ( height + 3 ) & ~3;
		compressedSize = quadW * quadH * BitsForFormat( opts.format ) / 8;

		int padW = ( opts.width + 3 ) & ~3;
		int padH = ( opts.height + 3 ) & ~3;
		(void)padH;
		(void)padW;
		assert( x + width <= padW && y + height <= padH );
		// upload the non-aligned value, OpenGL understands that there
		// will be padding
		if ( x + width > opts.width ) {
			width = opts.width - x;
		}
		if ( y + height > opts.height ) {
			height = opts.height - x;
		}
	} else {
		assert( x + width <= opts.width && y + height <= opts.height );
	}

	int target;
	int uploadTarget;
	if ( opts.textureType == TT_2D ) {
		target = GL_TEXTURE_2D;
		uploadTarget = GL_TEXTURE_2D;
	} else if ( opts.textureType == TT_CUBIC ) {
		target = GL_TEXTURE_CUBE_MAP_EXT;
		uploadTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + z;
	} else {
		assert( !"invalid opts.textureType" );
		target = GL_TEXTURE_2D;
		uploadTarget = GL_TEXTURE_2D;
	}

	qglBindTexture( target, texnum );

	if ( pixelPitch != 0 && glConfig.textureUnpackRowLengthAvaliable ) {
		qglPixelStorei( GL_UNPACK_ROW_LENGTH, pixelPitch );
	}
	if ( opts.format == FMT_RGB565 ) {
#ifndef GL_ES_VERSION_2_0
		qglPixelStorei( GL_UNPACK_SWAP_BYTES, GL_TRUE );
#else
		assert( pixelPitch == 0 ); //XXX

#if 1
		uint16 * dst = (uint16 *)_alloca16( width * height * sizeof( uint16 ) );
		SwapBytes_16bit( dst, (uint16 *)pic, width * height );
#else
		memcpy( dst, pic, width * height * sizeof( uint16 ) );
		idSwap::BigArray( dst, width * height );
#endif
		pic = dst;
#endif
	}
#ifdef DEBUG
	GL_CheckErrors();
#endif
	if ( IsCompressed() ) {
		qglCompressedTexSubImage2DARB( uploadTarget, mipLevel, x, y, width, height, internalFormat, compressedSize, pic );
	} else {

		// make sure the pixel store alignment is correct so that lower mips get created
		// properly for odd shaped textures - this fixes the mip mapping issues with
		// fonts
		int unpackAlignment = width * BitsForFormat( (textureFormat_t)opts.format ) / 8;
		if ( ( unpackAlignment & 3 ) == 0 ) {
			qglPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		} else {
			qglPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		}

		qglTexSubImage2D( uploadTarget, mipLevel, x, y, width, height, dataFormat, dataType, pic );
	}
#ifdef DEBUG
	GL_CheckErrors();
#endif
#ifndef GL_ES_VERSION_2_0
	if ( opts.format == FMT_RGB565 ) {
		qglPixelStorei( GL_UNPACK_SWAP_BYTES, GL_FALSE );
	}
#endif
	if ( pixelPitch != 0 && glConfig.textureUnpackRowLengthAvaliable ) {
		qglPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
	}
}

/*
========================
idImage::SetPixel
========================
*/
void idImage::SetPixel( int mipLevel, int x, int y, const void * data, int dataSize ) {
	SubImageUpload( mipLevel, x, y, 0, 1, 1, data );
}

/*
========================
idImage::SetTexParameters
========================
*/
void idImage::SetTexParameters() {
	int target = GL_TEXTURE_2D;
	switch ( opts.textureType ) {
		case TT_2D:
			target = GL_TEXTURE_2D;
			break;
		case TT_CUBIC:
			target = GL_TEXTURE_CUBE_MAP_EXT;
			break;
		default:
			idLib::FatalError( "%s: bad texture type %d", GetName(), opts.textureType );
			return;
	}

#if !defined( GL_ES_VERSION_2_0 ) || defined( GL_ES_VERSION_3_0 )
#ifdef GL_ES_VERSION_3_0
	if ( glConfig.textureSwizzleAvailable ) {
#endif
	// ALPHA, LUMINANCE, LUMINANCE_ALPHA, and INTENSITY have been removed
	// in OpenGL 3.2. In order to mimic those modes, we use the swizzle operators
#if defined( USE_CORE_PROFILE )
	if ( opts.colorFormat == CFM_GREEN_ALPHA ) {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	} else if ( opts.format == FMT_LUM8 ) {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_ONE );
	} else if ( opts.format == FMT_L8A8 ) {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	} else if ( opts.format == FMT_ALPHA ) {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	} else if ( opts.format == FMT_INT8 ) {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	} else {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_GREEN );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_BLUE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_ALPHA );
	}
#else
	if ( opts.colorFormat == CFM_GREEN_ALPHA ) {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	} else if ( opts.format == FMT_ALPHA ) {
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		qglTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	}
#endif
#ifdef GL_ES_VERSION_3_0
	} else {
#endif
#endif //!defined( GL_ES_VERSION_2_0 ) || defined( GL_ES_VERSION_3_0 )
#ifdef GL_ES_VERSION_2_0
#ifndef GL_ES_VERSION_3_0
	{
#endif
	//TODO: GLES 2.0 texture swizzling (possibly a shader modification)
	}
#endif

	switch( filter ) {
		case TF_DEFAULT:
			if ( r_useTrilinearFiltering.GetBool() ) {
				qglTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			} else {
				qglTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
			}
			qglTexParameterf( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			break;
		case TF_LINEAR:
			qglTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			qglTexParameterf( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			break;
		case TF_NEAREST:
			qglTexParameterf( target, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			qglTexParameterf( target, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			break;
		default:
			common->FatalError( "%s: bad texture filter %d", GetName(), filter );
	}

	if ( glConfig.anisotropicFilterAvailable ) {
		// only do aniso filtering on mip mapped images
		if ( filter == TF_DEFAULT ) {
			int aniso = r_maxAnisotropicFiltering.GetInteger();
			if ( aniso > glConfig.maxTextureAnisotropy ) {
				aniso = glConfig.maxTextureAnisotropy;
			}
			if ( aniso < 0 ) {
				aniso = 0;
			}
			qglTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso );
		} else {
			qglTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1 );
		}
	}
	if ( ( glConfig.textureLODBiasAvailable || glConfig.textureLODBiasShaderOnlyAvailable ) && ( usage != TD_FONT ) ) {
		// use a blurring LOD bias in combination with high anisotropy to fix our aliasing grate textures...
		if ( glConfig.textureLODBiasAvailable ) {
			qglTexParameterf(target, GL_TEXTURE_LOD_BIAS_EXT, r_lodBias.GetFloat() );
		} else {
			// while potentially problematic for fonts, this can be a uniform as opposed to per-texture since r_logBias is a global CVar
			idVec4 bias( r_lodBias.GetFloat() );
			renderProgManager.SetRenderParm( RENDERPARM_TEXBIAS, bias.ToFloatPtr() );
		}
	}

	// set the wrap/clamp modes
	switch( repeat ) {
		case TR_REPEAT:
			qglTexParameterf( target, GL_TEXTURE_WRAP_S, GL_REPEAT );
			qglTexParameterf( target, GL_TEXTURE_WRAP_T, GL_REPEAT );
			break;
		case TR_CLAMP_TO_ZERO: {
			if ( glConfig.clampToBorderAvailable ) {
				float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				qglTexParameterfv( target, GL_TEXTURE_BORDER_COLOR, color );
			}
			qglTexParameterf( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			qglTexParameterf( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			}
			break;
		case TR_CLAMP_TO_ZERO_ALPHA: {
			if ( glConfig.clampToBorderAvailable ) {
				float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				qglTexParameterfv( target, GL_TEXTURE_BORDER_COLOR, color );
			}
			qglTexParameterf( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			qglTexParameterf( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			}
			break;
		case TR_CLAMP:
			qglTexParameterf( target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			qglTexParameterf( target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			break;
		default:
			common->FatalError( "%s: bad texture repeat %d", GetName(), repeat );
	}
}

/*
========================
idImage::AllocImage

Every image will pass through this function. Allocates all the necessary MipMap levels for the
Image, but doesn't put anything in them.

This should not be done during normal game-play, if you can avoid it.
========================
*/
void idImage::AllocImage() {
	GL_CheckErrors();
	PurgeImage();

	switch ( opts.format ) {
	case FMT_RGBA8:
		internalFormat = GL_RGBA8;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_XRGB8:
		internalFormat = GL_RGB;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_RGB565:
#ifdef GL_ES_VERSION_3_0
		if ( glConfig.glVersion >= 3.0 ) {
			internalFormat = GL_RGB565;
		} else {
			internalFormat = GL_RGB;
		}
#else
		internalFormat = GL_RGB;
#endif
		dataFormat = GL_RGB;
		dataType = GL_UNSIGNED_SHORT_5_6_5;
		break;
	case FMT_ALPHA:
#if defined( USE_CORE_PROFILE )
		internalFormat = GL_R8;
		dataFormat = GL_RED;
#else
		internalFormat = GL_ALPHA8;
		dataFormat = GL_ALPHA;
#endif
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_L8A8:
#if defined( USE_CORE_PROFILE )
		internalFormat = GL_RG8;
		dataFormat = GL_RG;
#else
		internalFormat = GL_LUMINANCE8_ALPHA8;
		dataFormat = GL_LUMINANCE_ALPHA;
#endif
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_LUM8:
#if defined( USE_CORE_PROFILE )
		internalFormat = GL_R8;
		dataFormat = GL_RED;
#else
		internalFormat = GL_LUMINANCE8;
		dataFormat = GL_LUMINANCE;
#endif
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_INT8:
#if defined( USE_CORE_PROFILE )
		internalFormat = GL_R8;
		dataFormat = GL_RED;
#else
		internalFormat = GL_INTENSITY8;
		dataFormat = GL_LUMINANCE;
#endif
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_DXT1:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_DXT5:
		internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_DEPTH:
		internalFormat = GL_DEPTH_COMPONENT;
		dataFormat = GL_DEPTH_COMPONENT;
#ifndef GL_ES_VERSION_2_0
		dataType = GL_UNSIGNED_BYTE;
#else
		dataType = GL_UNSIGNED_INT;
#endif
		break;
	case FMT_X16:
#ifndef GL_ES_VERSION_2_0
		internalFormat = GL_INTENSITY16;
#else
		internalFormat = GL_LUMINANCE; // No code returns FMT_X16, so this will never occur
#endif
		dataFormat = GL_LUMINANCE;
		dataType = GL_UNSIGNED_SHORT;
		break;
	case FMT_Y16_X16:
#ifndef GL_ES_VERSION_2_0
		internalFormat = GL_LUMINANCE16_ALPHA16;
#else
		internalFormat = GL_LUMINANCE_ALPHA; // No code returns FMT_Y16_X16, so this will never occur
#endif
		dataFormat = GL_LUMINANCE_ALPHA;
		dataType = GL_UNSIGNED_SHORT;
		break;
#ifdef GL_ES_VERSION_2_0
	case FMT_ETC1:
		if ( glConfig.glVersion >= 3.0f ) {
			internalFormat = GL_COMPRESSED_RGB8_ETC2;
		} else {
			internalFormat = GL_ETC1_RGB8_OES;
		}
		dataFormat = GL_RGB;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_ETC2_PUNCH:
		internalFormat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case FMT_ETC2_ALPHA:
		internalFormat = GL_COMPRESSED_RGBA8_ETC2_EAC;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
#endif
	default:
		idLib::Error( "Unhandled image format %d in %s\n", opts.format, GetName() );
	}

	// if we don't have a rendering context, just return after we
	// have filled in the parms.  We must have the values set, or
	// an image match from a shader before OpenGL starts would miss
	// the generated texture
	if ( !R_IsInitialized() ) {
		return;
	}

	// generate the texture number
	qglGenTextures( 1, (GLuint *)&texnum );
	assert( texnum != TEXTURE_NOT_LOADED );

	//----------------------------------------------------
	// allocate all the mip levels with NULL data
	//----------------------------------------------------

	int numSides;
	int target;
	int uploadTarget;
	if ( opts.textureType == TT_2D ) {
		target = uploadTarget = GL_TEXTURE_2D;
		numSides = 1;
	} else if ( opts.textureType == TT_CUBIC ) {
		target = GL_TEXTURE_CUBE_MAP_EXT;
		uploadTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT;
		numSides = 6;
	} else {
		assert( !"opts.textureType" );
		target = uploadTarget = GL_TEXTURE_2D;
		numSides = 1;
	}

	qglBindTexture( target, texnum );

	for ( int side = 0; side < numSides; side++ ) {
		int w = opts.width;
		int h = opts.height;
		if ( opts.textureType == TT_CUBIC ) {
			h = w;
		}
		for ( int level = 0; level < opts.numLevels; level++ ) {

			// clear out any previous error
			GL_CheckErrors();

			if ( IsCompressed() ) {
				int compressedSize = ( ((w+3)/4) * ((h+3)/4) * int64( 16 ) * BitsForFormat( opts.format ) ) / 8;

#ifndef GL_ES_VERSION_2_0
				// Even though the OpenGL specification allows the 'data' pointer to be NULL, for some
				// drivers we actually need to upload data to get it to allocate the texture.
				// However, on 32-bit systems we may fail to allocate a large block of memory for large
				// textures. We handle this case by using HeapAlloc directly and allowing the allocation
				// to fail in which case we simply pass down NULL to glCompressedTexImage2D and hope for the best.
				// As of 2011-10-6 using NVIDIA hardware and drivers we have to allocate the memory with HeapAlloc
				// with the exact size otherwise large image allocation (for instance for physical page textures)
				// may fail on Vista 32-bit.
				void * data = HeapAlloc( GetProcessHeap(), 0, compressedSize );
				qglCompressedTexImage2DARB( uploadTarget+side, level, internalFormat, w, h, 0, compressedSize, data );
				if ( data != NULL ) {
					HeapFree( GetProcessHeap(), 0, data );
				}
#else
				qglCompressedTexImage2DARB( uploadTarget+side, level, internalFormat, w, h, 0, compressedSize, NULL );
#endif
			} else {
				// In OpenGL ES, glTexStorage2D could be used. But this causes the resulting texture to be immutable.
				// This causes issues if any function is used that could/does change dimensions is used, such as glCopyTexImage2D.
				qglTexImage2D( uploadTarget + side, level, internalFormat, w, h, 0, dataFormat, dataType, NULL );
			}

			GL_CheckErrors();

			w = Max( 1, w >> 1 );
			h = Max( 1, h >> 1 );
		}
	}

	if ( glConfig.textureMaxLevelAvailable ) {
		qglTexParameteri( target, GL_TEXTURE_MAX_LEVEL, opts.numLevels - 1 );
	}

	// see if we messed anything up
	GL_CheckErrors();

	SetTexParameters();

	GL_CheckErrors();
}

/*
========================
idImage::PurgeImage
========================
*/
void idImage::PurgeImage() {
	if ( texnum != TEXTURE_NOT_LOADED ) {
		qglDeleteTextures( 1, (GLuint *)&texnum );	// this should be the ONLY place it is ever called!
		texnum = TEXTURE_NOT_LOADED;
	}
	// clear all the current binding caches, so the next bind will do a real one
	for ( int i = 0 ; i < MAX_MULTITEXTURE_UNITS ; i++ ) {
		backEnd.glState.tmu[i].current2DMap = TEXTURE_NOT_LOADED;
		backEnd.glState.tmu[i].currentCubeMap = TEXTURE_NOT_LOADED;
	}
}

/*
========================
idImage::Resize
========================
*/
void idImage::Resize( int width, int height ) {
	if ( opts.width == width && opts.height == height ) {
		return;
	}
	opts.width = width;
	opts.height = height;
	AllocImage();
}

/*
========================
idImage::IsCompressed
========================
*/
bool idImage::IsCompressed() const {
#ifndef GL_ES_VERSION_2_0
	return ( opts.format == FMT_DXT1 || opts.format == FMT_DXT5 );
#else
	return ( opts.format == FMT_DXT1 || opts.format == FMT_DXT5 || opts.format == FMT_ETC1 || opts.format == FMT_ETC2_PUNCH || opts.format == FMT_ETC2_ALPHA );
#endif
}
