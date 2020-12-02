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
#include "SystemGL.h"
#include "XOGLUtils.h"
#include <set>
#include <string>

/*
 * This was moved from xplanemp1 as we still need it for XSB3, but not
 * for xplanemp2.  It remains subject to it's original license per the above.
 *
 * -- Chris Collins - 2020/02/29
*/

// This had to be renamed because on Linux, namespaces appear to be shared between ourselves and XPlane
// so i would end up overwritting XPlanes function pointer!

#if IBM
PFNGLBINDBUFFERARBPROC			glBindBufferARB			 = NULL;
PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB		 = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB	 = NULL;
PFNGLMULTITEXCOORD2FVARBPROC	glMultiTexCoord2fvARB	 = NULL;
PFNGLGENERATEMIPMAPPROC			glGenerateMipmap		 = NULL;
PFNGLGENVERTEXARRAYSPROC 		glGenVertexArrays   	 = NULL;
PFNGLDELETEVERTEXARRAYSPROC		glDeleteVertexArrays	 = NULL;
#endif

#ifdef DEBUG
#if IBM
PFNGLDEBUGMESSAGECONTROLPROC	glDebugMessageControl = NULL;
PFNGLDEBUGMESSAGEINSERTPROC		glDebugMessageInsert = NULL;
PFNGLDEBUGMESSAGECALLBACKPROC	glDebugMessageCallback = NULL;
PFNGLGETDEBUGMESSAGELOGPROC		glGetDebugMessageLog = NULL;
PFNGLPUSHDEBUGGROUPPROC			glPushDebugGroup = NULL;
PFNGLPOPDEBUGGROUPPROC			glPopDebugGroup = NULL;
PFNGLOBJECTLABELPROC			glObjectLabel = NULL;
PFNGLGETOBJECTLABELPROC			glGetObjectLabel = NULL;
PFNGLOBJECTPTRLABELPROC			glObjectPtrLabel = NULL;
PFNGLGETOBJECTPTRLABELPROC		glGetObjectPtrLabel = NULL;
#endif

bool							xpmp_ogl_can_debug = false;

#endif


#if LIN

#define		wglGetProcAddress(x)		glXGetProcAddressARB((GLubyte*) (x))

#endif

/**************************************************
			   Utilities Initialization
***************************************************/

std::set<std::string>	xpmp_glExtensions;

static void 
OGL_EnumerateExtensions()
{
	const GLubyte *	glExtAll = glGetString(GL_EXTENSIONS);
	std::string 	allExtensions(reinterpret_cast<const char *>(glExtAll));

	std::string::size_type offs;
	while ((offs = allExtensions.find(" ")) != std::string::npos) {
		xpmp_glExtensions.insert(allExtensions.substr(0, offs));
		allExtensions = allExtensions.substr(offs+1);
	}
}

bool	OGL_UtilsInit()
{
	static bool firstTime = true;
	if(firstTime)
	{
		// enumerate our extensions.
		OGL_EnumerateExtensions();

		// Initialize all OGL Function Pointers
#if IBM		
		glBindBufferARB			 = (PFNGLBINDBUFFERARBPROC)			 wglGetProcAddress("glBindBufferARB"		);
		glActiveTextureARB 		 = (PFNGLACTIVETEXTUREARBPROC)		 wglGetProcAddress("glActiveTextureARB"		 );
		glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) wglGetProcAddress("glClientActiveTextureARB");
		glMultiTexCoord2fARB	 = (PFNGLMULTITEXCOORD2FARBPROC )	 wglGetProcAddress("glMultiTexCoord2fARB"    );
		glMultiTexCoord2fvARB	 = (PFNGLMULTITEXCOORD2FVARBPROC )	 wglGetProcAddress("glMultiTexCoord2fvARB"   );
		glGenerateMipmap		 = (PFNGLGENERATEMIPMAPPROC)		 wglGetProcAddress("glGenerateMipmap"		 );
		glGenVertexArrays   	 = (PFNGLGENVERTEXARRAYSPROC)		 wglGetProcAddress("glGenVertexArrays");
		glDeleteVertexArrays	 = (PFNGLDELETEVERTEXARRAYSPROC)	 wglGetProcAddress("glDeleteVertexArrays");
#endif
#ifdef DEBUG_GL
		if (OGL_HasExtension("GL_KHR_debug")) {
#if IBM
			glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)	wglGetProcAddress("glDebugMessageControl");
			glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC)	wglGetProcAddress("glDebugMessageInsert");
			glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
			glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC)	wglGetProcAddress("glGetDebugMessageLog");
			glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC)			wglGetProcAddress("glPushDebugGroup");
			glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC)				wglGetProcAddress("glPopDebugGroup");
			glObjectLabel = (PFNGLOBJECTLABELPROC)					wglGetProcAddress("glObjectLabel");
			glGetObjectLabel = (PFNGLGETOBJECTLABELPROC)			wglGetProcAddress("glGetObjectLabel");
			glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC)			wglGetProcAddress("glObjectPtrLabel");
			glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC)		wglGetProcAddress("glGetObjectPtrLabel");
#endif
			xpmp_ogl_can_debug = true;
		}
#endif
		firstTime = false;
	}

#if IBM
	// Make sure everything got initialized
	if(glBindBufferARB &&
			glActiveTextureARB &&
			glClientActiveTextureARB &&
			glMultiTexCoord2fARB &&
			glMultiTexCoord2fvARB &&
			glGenerateMipmap)
	{
		return true;
	}
	else
		return false;
#else
	return true;
#endif

}

bool	OGL_HasExtension(const std::string &inExtensionName) 
{
	return (xpmp_glExtensions.count(inExtensionName) > 0);
}
