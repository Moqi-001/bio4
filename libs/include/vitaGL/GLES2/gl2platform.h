#ifndef __gl2platform_h_
#define __gl2platform_h_

/* $Revision: 10602 $ on $Date:: 2010-03-04 22:35:34 -0800 #$ */

/*
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

/* Platform-specific definitions for OpenGL ES 2.X gl2.h (vitaGL variant:
 * the Khronos baseline with the khrplatform.h layer folded away, the base
 * types are defined directly in gl2.h and the call decorations are empty).
 * Derived from the Khronos header, original license retained below. */

#include <stdint.h>

#ifndef GL_APICALL
#define GL_APICALL
#endif

#ifndef GL_APIENTRY
#define GL_APIENTRY
#endif

/* Normally provided by gl2ext.h, which vitaGL does not ship: kept here so
 * that application code declaring its own PFNGL*PROC typedefs compiles. */
#ifndef GL_APIENTRYP
#define GL_APIENTRYP GL_APIENTRY *
#endif

#endif /* __gl2platform_h_ */
