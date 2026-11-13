#ifndef __glplatform_h_
#define __glplatform_h_

/* $Revision: 10601 $ on $Date:: 2010-03-04 22:15:27 -0800 #$ */

/*
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

/* Platform-specific definitions for OpenGL ES 1.X gl.h (vitaGL variant:
 * the Khronos baseline with the khrplatform.h layer folded away, the base
 * types are defined directly in gl.h and the call decorations are empty).
 * Derived from the Khronos header, original license retained below. */

#include <stdint.h>

#ifndef GL_API
#define GL_API
#endif

#ifndef GL_APIENTRY
#define GL_APIENTRY
#endif

/* Normally provided by glext.h, which vitaGL does not ship: kept here so
 * that application code declaring its own PFNGL*PROC typedefs compiles. */
#ifndef GL_APIENTRYP
#define GL_APIENTRYP GL_APIENTRY *
#endif

#endif /* __glplatform_h_ */
