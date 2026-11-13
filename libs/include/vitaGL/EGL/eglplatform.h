#ifndef __eglplatform_h_
#define __eglplatform_h_

/* $Revision: 12306 $ on $Date: 2010-08-25 09:51:28 -0700 (Wed, 25 Aug 2010) $ */

/*
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

/* Platform-specific types and definitions for egl.h (vitaGL variant: the
 * Khronos baseline with the khrplatform.h layer folded away, the native
 * window-system types are plain void pointers on Vita and the call
 * decorations expand to nothing). Derived from the Khronos header, original
 * license retained above. */

#include <stdint.h>

/* Macros used in EGL function prototype declarations.
 *
 * EGL functions should be prototyped as:
 *
 * EGLAPI return-type EGLAPIENTRY eglFunction(arguments);
 * typedef return-type (EGLAPIENTRYP PFNEGLFUNCTIONPROC) (arguments);
 */

#ifndef EGLAPI
#define EGLAPI
#endif

#ifndef EGLAPIENTRY
#define EGLAPIENTRY
#endif
#define EGLAPIENTRYP EGLAPIENTRY*

/* Native window-system types. Vita has no X11/Win32 window system: the EGL
 * surface is always the console framebuffer, so all native handles are void
 * pointers. */
typedef void *EGLNativeDisplayType;
typedef void *EGLNativePixmapType;
typedef void *EGLNativeWindowType;

/* EGL 1.2 types, renamed for consistency in EGL 1.3 */
typedef EGLNativeDisplayType NativeDisplayType;
typedef EGLNativePixmapType  NativePixmapType;
typedef EGLNativeWindowType  NativeWindowType;


/* Define EGLint. This must be a signed integral type large enough to contain
 * all legal attribute names and values passed into and out of EGL, whether
 * their type is boolean, bitmask, enumerant (symbolic constant), integer,
 * handle, or other.  While in general a 32-bit integer will suffice, if
 * handles are 64 bit types, then EGLint should be defined as a signed 64-bit
 * integer type.
 */
typedef int32_t EGLint;

#endif /* __eglplatform_h */
