/*
 * Copyright (c) 2005, Ben Supnik and Chris Serio.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _XOGLUtils_h_
#define _XOGLUtils_h_

#include "SystemGL.h"
#if IBM
#include <windows.h>
#include <GL/glext.h>
#elif LIN
#define GLX_GLXEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#include <GL/glx.h>
#elif APL
#include <OpenGL/glext.h>
#endif

#if LIN
#define GLX_GLXEXT_PROTOTYPES
#endif

// Open GL Extension Defintion
#if APL
#define APIENTRY
#endif
#if APL || LIN
#define GL_ARRAY_BUFFER_ARB               0x8892
#define GL_ARRAY_BUFFER_BINDING_ARB       0x8894
#endif

typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC )			(GLenum, GLuint);
typedef void (APIENTRY * PFNGLACTIVETEXTUREARBPROC)			(GLenum);
typedef void (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC )	(GLenum);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC )		(GLenum, GLfloat, GLfloat);
typedef void (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC)		(GLenum, const GLfloat *);

#if IBM
extern PFNGLBINDBUFFERARBPROC			glBindBufferARB;
extern PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC		glMultiTexCoord2fvARB;
extern PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB;
extern PFNGLGENERATEMIPMAPPROC          glGenerateMipmap;
#endif

#ifdef DEBUG_GL
#ifdef IBM
extern PFNGLDEBUGMESSAGECONTROLPROC		glDebugMessageControl;
extern PFNGLDEBUGMESSAGEINSERTPROC		glDebugMessageInsert;
extern PFNGLDEBUGMESSAGECALLBACKPROC	glDebugMessageCallback;
extern PFNGLGETDEBUGMESSAGELOGPROC		glGetDebugMessageLog;
extern PFNGLPUSHDEBUGGROUPPROC			glPushDebugGroup;
extern PFNGLPOPDEBUGGROUPPROC			glPopDebugGroup;
extern PFNGLOBJECTLABELPROC				glObjectLabel;
extern PFNGLGETOBJECTLABELPROC			glGetObjectLabel;
extern PFNGLOBJECTPTRLABELPROC			glObjectPtrLabel;
extern PFNGLGETOBJECTPTRLABELPROC		glGetObjectPtrLabel;
#endif

extern bool								xpmp_ogl_can_debug;
#define OGLDEBUG(x)						if(xpmp_ogl_can_debug) x

enum GLDebug_Group {
	XPMP_DBG_TexLoad = 1,
	XPMP_DBG_TexLoad_CreateTex,
	XPMP_DBG_TexLoad_GenMips,
	XPMP_DBG_TexLoad_ConfigFiltering,
	XPMP_DBG_TexLoad_ConfigWrap,
	XPMP_DBG_TexLoad_Done,
	XPMP_DBG_OBJRender
};

#else /* #ifdef DEBUG_GL */

#define OGLDEBUG(X)

#endif /* #ifdef DEBUG_GL */


bool	OGL_UtilsInit();

#ifdef __cplusplus

#include <string>
bool	OGL_HasExtension(const std::string &inExtensionName);

#endif

#endif
