#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <vitaGL.h>

// ==================== libGLESv1_CM.so (OpenGL ES 1.x Common Profile) ====================
static so_symbol libGLESv1_CM_symbols[] = {
    // ===== OpenGL ES 1.x Core Functions =====

    // Vertex Arrays
    SO_SYMBOL_FUNC("glVertexPointer", &glVertexPointer),
    SO_SYMBOL_FUNC("glNormalPointer", &glNormalPointer),
    SO_SYMBOL_FUNC("glColorPointer", &glColorPointer),
    SO_SYMBOL_FUNC("glTexCoordPointer", &glTexCoordPointer),
    SO_SYMBOL_FUNC("glEnableClientState", &glEnableClientState),
    SO_SYMBOL_FUNC("glDisableClientState", &glDisableClientState),
    SO_SYMBOL_FUNC("glArrayElement", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawArrays", &glDrawArrays),
    SO_SYMBOL_FUNC("glDrawElements", &glDrawElements),

    // Immediate Mode (float)
    SO_SYMBOL_FUNC("glVertex2f", &glVertex2f),
    SO_SYMBOL_FUNC("glVertex3f", &glVertex3f),
    SO_SYMBOL_FUNC("glNormal3f", &glNormal3f),
    SO_SYMBOL_FUNC("glColor4f", &glColor4f),
    SO_SYMBOL_FUNC("glColor4ub", &glColor4ub),
    SO_SYMBOL_FUNC("glTexCoord2f", &glTexCoord2f),

    // Immediate Mode (fixed-point)
    SO_SYMBOL_FUNC("glVertex2x", &so_ret_0),
    SO_SYMBOL_FUNC("glVertex3x", &so_ret_0),
    SO_SYMBOL_FUNC("glNormal3x", &glNormal3x),
    SO_SYMBOL_FUNC("glColor4x", &glColor4x),
    SO_SYMBOL_FUNC("glTexCoord2x", &so_ret_0),

    // Matrix Operations
    SO_SYMBOL_FUNC("glMatrixMode", &glMatrixMode),
    SO_SYMBOL_FUNC("glLoadIdentity", &glLoadIdentity),
    SO_SYMBOL_FUNC("glLoadMatrixf", &glLoadMatrixf),
    SO_SYMBOL_FUNC("glLoadMatrixx", &glLoadMatrixx),
    SO_SYMBOL_FUNC("glMultMatrixf", &glMultMatrixf),
    SO_SYMBOL_FUNC("glMultMatrixx", &glMultMatrixx),
    SO_SYMBOL_FUNC("glPushMatrix", &glPushMatrix),
    SO_SYMBOL_FUNC("glPopMatrix", &glPopMatrix),
    SO_SYMBOL_FUNC("glRotatef", &glRotatef),
    SO_SYMBOL_FUNC("glRotatex", &glRotatex),
    SO_SYMBOL_FUNC("glScalef", &glScalef),
    SO_SYMBOL_FUNC("glScalex", &glScalex),
    SO_SYMBOL_FUNC("glTranslatef", &glTranslatef),
    SO_SYMBOL_FUNC("glTranslatex", &glTranslatex),
    SO_SYMBOL_FUNC("glOrthof", &glOrthof),
    SO_SYMBOL_FUNC("glOrthox", &glOrthox),
    SO_SYMBOL_FUNC("glFrustumf", &glFrustumf),
    SO_SYMBOL_FUNC("glFrustumx", &glFrustumx),

    // Multi-Texture
    SO_SYMBOL_FUNC("glMultiTexCoord4f", &so_ret_0),
    SO_SYMBOL_FUNC("glMultiTexCoord4x", &so_ret_0),

    // Clip Planes
    SO_SYMBOL_FUNC("glClipPlanef", &glClipPlanef),
    SO_SYMBOL_FUNC("glClipPlanex", &glClipPlanex),

    // Lighting
    SO_SYMBOL_FUNC("glLightf", &so_ret_0),
    SO_SYMBOL_FUNC("glLightx", &so_ret_0),
    SO_SYMBOL_FUNC("glLightfv", &glLightfv),
    SO_SYMBOL_FUNC("glLightxv", &glLightxv),
    SO_SYMBOL_FUNC("glLightModelf", &so_ret_0),
    SO_SYMBOL_FUNC("glLightModelx", &so_ret_0),
    SO_SYMBOL_FUNC("glLightModelfv", &glLightModelfv),
    SO_SYMBOL_FUNC("glLightModelxv", &glLightModelxv),
    SO_SYMBOL_FUNC("glMaterialf", &glMaterialf),
    SO_SYMBOL_FUNC("glMaterialx", &glMaterialx),
    SO_SYMBOL_FUNC("glMaterialfv", &glMaterialfv),
    SO_SYMBOL_FUNC("glMaterialxv", &glMaterialxv),
    SO_SYMBOL_FUNC("glColorMaterial", &so_ret_0),

    // Fog
    SO_SYMBOL_FUNC("glFogf", &glFogf),
    SO_SYMBOL_FUNC("glFogx", &glFogx),
    SO_SYMBOL_FUNC("glFogfv", &glFogfv),
    SO_SYMBOL_FUNC("glFogxv", &glFogxv),

    // Alpha Test and Blending
    SO_SYMBOL_FUNC("glAlphaFunc", &glAlphaFunc),
    SO_SYMBOL_FUNC("glAlphaFuncx", &glAlphaFuncx),
    SO_SYMBOL_FUNC("glBlendFunc", &glBlendFunc),

    // Depth and Stencil
    SO_SYMBOL_FUNC("glDepthFunc", &glDepthFunc),
    SO_SYMBOL_FUNC("glDepthMask", &glDepthMask),
    SO_SYMBOL_FUNC("glDepthRangef", &glDepthRangef),
    SO_SYMBOL_FUNC("glDepthRangex", &glDepthRangex),
    SO_SYMBOL_FUNC("glStencilFunc", &glStencilFunc),
    SO_SYMBOL_FUNC("glStencilMask", &glStencilMask),
    SO_SYMBOL_FUNC("glStencilOp", &glStencilOp),

    // Clear
    SO_SYMBOL_FUNC("glClear", &glClear),
    SO_SYMBOL_FUNC("glClearColor", &glClearColor),
    SO_SYMBOL_FUNC("glClearColorx", &glClearColorx),
    SO_SYMBOL_FUNC("glClearDepthf", &glClearDepthf),
    SO_SYMBOL_FUNC("glClearDepthx", &glClearDepthx),
    SO_SYMBOL_FUNC("glClearStencil", &glClearStencil),

    // Color Mask
    SO_SYMBOL_FUNC("glColorMask", &glColorMask),

    // Face Culling
    SO_SYMBOL_FUNC("glCullFace", &glCullFace),
    SO_SYMBOL_FUNC("glFrontFace", &glFrontFace),

    // Scissor and Viewport
    SO_SYMBOL_FUNC("glScissor", &glScissor),
    SO_SYMBOL_FUNC("glViewport", &glViewport),

    // Shading
    SO_SYMBOL_FUNC("glShadeModel", &glShadeModel),

    // Texturing
    SO_SYMBOL_FUNC("glActiveTexture", &glActiveTexture),
    SO_SYMBOL_FUNC("glClientActiveTexture", &glClientActiveTexture),
    SO_SYMBOL_FUNC("glBindTexture", &glBindTexture),
    SO_SYMBOL_FUNC("glDeleteTextures", &glDeleteTextures),
    SO_SYMBOL_FUNC("glGenTextures", &glGenTextures),
    SO_SYMBOL_FUNC("glIsTexture", &glIsTexture),
    SO_SYMBOL_FUNC("glTexImage2D", &glTexImage2D),
    SO_SYMBOL_FUNC("glTexSubImage2D", &glTexSubImage2D),
    SO_SYMBOL_FUNC("glCopyTexImage2D", &glCopyTexImage2D),
    SO_SYMBOL_FUNC("glCopyTexSubImage2D", &glCopyTexSubImage2D),
    SO_SYMBOL_FUNC("glCompressedTexImage2D", &glCompressedTexImage2D),
    SO_SYMBOL_FUNC("glCompressedTexSubImage2D", &so_ret_0),
    SO_SYMBOL_FUNC("glTexEnvf", &glTexEnvf),
    SO_SYMBOL_FUNC("glTexEnvi", &glTexEnvi),
    SO_SYMBOL_FUNC("glTexEnvx", &glTexEnvx),
    SO_SYMBOL_FUNC("glTexEnvfv", &glTexEnvfv),
    SO_SYMBOL_FUNC("glTexEnviv", &so_ret_0),
    SO_SYMBOL_FUNC("glTexEnvxv", &glTexEnvxv),
    SO_SYMBOL_FUNC("glTexParameterf", &glTexParameterf),
    SO_SYMBOL_FUNC("glTexParameterx", &glTexParameterx),
    SO_SYMBOL_FUNC("glTexParameteri", &glTexParameteri),
    SO_SYMBOL_FUNC("glTexParameterfv", &so_ret_0),
    SO_SYMBOL_FUNC("glTexParameterxv", &so_ret_0),
    SO_SYMBOL_FUNC("glTexParameteriv", &glTexParameteriv),

    // Point and Line
    SO_SYMBOL_FUNC("glPointSize", &glPointSize),
    SO_SYMBOL_FUNC("glPointSizex", &glPointSizex),
    SO_SYMBOL_FUNC("glLineWidth", &glLineWidth),
    SO_SYMBOL_FUNC("glLineWidthx", &glLineWidthx),

    // Point Parameters
    SO_SYMBOL_FUNC("glPointParameterf", &so_ret_0),
    SO_SYMBOL_FUNC("glPointParameterfv", &so_ret_0),
    SO_SYMBOL_FUNC("glPointParameterx", &so_ret_0),
    SO_SYMBOL_FUNC("glPointParameterxv", &so_ret_0),

    // Polygon Offset
    SO_SYMBOL_FUNC("glPolygonOffset", &glPolygonOffset),
    SO_SYMBOL_FUNC("glPolygonOffsetx", &glPolygonOffsetx),

    // Buffer Objects (VBO)
    SO_SYMBOL_FUNC("glBindBuffer", &glBindBuffer),
    SO_SYMBOL_FUNC("glBufferData", &glBufferData),
    SO_SYMBOL_FUNC("glBufferSubData", &glBufferSubData),
    SO_SYMBOL_FUNC("glDeleteBuffers", &glDeleteBuffers),
    SO_SYMBOL_FUNC("glGenBuffers", &glGenBuffers),
    SO_SYMBOL_FUNC("glGetBufferParameteriv", &glGetBufferParameteriv),
    SO_SYMBOL_FUNC("glIsBuffer", &so_ret_0),

    // State Management
    SO_SYMBOL_FUNC("glEnable", &glEnable),
    SO_SYMBOL_FUNC("glDisable", &glDisable),
    SO_SYMBOL_FUNC("glIsEnabled", &glIsEnabled),
    SO_SYMBOL_FUNC("glFlush", &glFlush),
    SO_SYMBOL_FUNC("glFinish", &glFinish),
    SO_SYMBOL_FUNC("glHint", &glHint),
    SO_SYMBOL_FUNC("glPixelStorei", &glPixelStorei),
    SO_SYMBOL_FUNC("glLogicOp", &so_ret_0),

    // Queries
    SO_SYMBOL_FUNC("glGetError", &glGetError),
    SO_SYMBOL_FUNC("glGetString", &glGetString),
    SO_SYMBOL_FUNC("glGetFloatv", &glGetFloatv),
    SO_SYMBOL_FUNC("glGetIntegerv", &glGetIntegerv),
    SO_SYMBOL_FUNC("glGetBooleanv", &glGetBooleanv),
    SO_SYMBOL_FUNC("glGetFixedv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetPointerv", &glGetPointerv),

    // Texture Queries
    SO_SYMBOL_FUNC("glGetTexEnvfv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexEnviv", &glGetTexEnviv),
    SO_SYMBOL_FUNC("glGetTexEnvxv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexParameterfv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexParameteriv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexParameterxv", &so_ret_0),

    // Lighting Queries
    SO_SYMBOL_FUNC("glGetLightfv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetLightxv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetMaterialfv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetMaterialxv", &so_ret_0),

    // Clip Plane Queries
    SO_SYMBOL_FUNC("glGetClipPlanef", &so_ret_0),
    SO_SYMBOL_FUNC("glGetClipPlanex", &so_ret_0),

    // Sample Coverage
    SO_SYMBOL_FUNC("glSampleCoverage", &so_ret_0),
    SO_SYMBOL_FUNC("glSampleCoveragex", &so_ret_0),

    // Read Pixels
    SO_SYMBOL_FUNC("glReadPixels", &glReadPixels),

    // ===== OpenGL ES 1.x OES Extensions =====

    // OES draw texture
    SO_SYMBOL_FUNC("glDrawTexfOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawTexfvOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawTexiOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawTexivOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawTexsOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawTexsvOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawTexxOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDrawTexxvOES", &so_ret_0),

    // OES matrix palette
    SO_SYMBOL_FUNC("glCurrentPaletteMatrixOES", &so_ret_0),
    SO_SYMBOL_FUNC("glLoadPaletteFromModelViewMatrixOES", &so_ret_0),
    SO_SYMBOL_FUNC("glMatrixIndexPointerOES", &so_ret_0),
    SO_SYMBOL_FUNC("glWeightPointerOES", &so_ret_0),

    // OES point size array
    SO_SYMBOL_FUNC("glPointSizePointerOES", &so_ret_0),

    // OES query matrix
    SO_SYMBOL_FUNC("glQueryMatrixxOES", &so_ret_0),

    // OES texture cube map
    SO_SYMBOL_FUNC("glTexGenfOES", &so_ret_0),
    SO_SYMBOL_FUNC("glTexGenfvOES", &so_ret_0),
    SO_SYMBOL_FUNC("glTexGeniOES", &so_ret_0),
    SO_SYMBOL_FUNC("glTexGenivOES", &so_ret_0),
    SO_SYMBOL_FUNC("glTexGenxOES", &so_ret_0),
    SO_SYMBOL_FUNC("glTexGenxvOES", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexGenfvOES", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexGenivOES", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexGenxvOES", &so_ret_0),

    // OES framebuffer object
    SO_SYMBOL_FUNC("glBindFramebufferOES", &glBindFramebuffer),
    SO_SYMBOL_FUNC("glBindRenderbufferOES", &glBindRenderbuffer),
    SO_SYMBOL_FUNC("glCheckFramebufferStatusOES", &glCheckFramebufferStatus),
    SO_SYMBOL_FUNC("glDeleteFramebuffersOES", &glDeleteFramebuffers),
    SO_SYMBOL_FUNC("glDeleteRenderbuffersOES", &glDeleteRenderbuffers),
    SO_SYMBOL_FUNC("glFramebufferRenderbufferOES", &glFramebufferRenderbuffer),
    SO_SYMBOL_FUNC("glFramebufferTexture2DOES", &glFramebufferTexture2D),
    SO_SYMBOL_FUNC("glGenFramebuffersOES", &glGenFramebuffers),
    SO_SYMBOL_FUNC("glGenRenderbuffersOES", &glGenRenderbuffers),
    SO_SYMBOL_FUNC("glGenerateMipmapOES", &glGenerateMipmap),
    SO_SYMBOL_FUNC("glGetFramebufferAttachmentParameterivOES", &glGetFramebufferAttachmentParameteriv),
    SO_SYMBOL_FUNC("glGetRenderbufferParameterivOES", &so_ret_0),
    SO_SYMBOL_FUNC("glIsFramebufferOES", &glIsFramebuffer),
    SO_SYMBOL_FUNC("glIsRenderbufferOES", &glIsRenderbuffer),
    SO_SYMBOL_FUNC("glRenderbufferStorageOES", &glRenderbufferStorage),

    // OES blend equation separate
    SO_SYMBOL_FUNC("glBlendEquationOES", &glBlendEquation),
    SO_SYMBOL_FUNC("glBlendEquationSeparateOES", &glBlendEquationSeparate),

    // OES blend func separate
    SO_SYMBOL_FUNC("glBlendFuncSeparateOES", &glBlendFuncSeparate),

    // OES EGL image
    SO_SYMBOL_FUNC("glEGLImageTargetTexture2DOES", &so_ret_0),
    SO_SYMBOL_FUNC("glEGLImageTargetRenderbufferStorageOES", &so_ret_0),

    // OES mapbuffer
    SO_SYMBOL_FUNC("glMapBufferOES", &glMapBuffer),
    SO_SYMBOL_FUNC("glUnmapBufferOES", &glUnmapBuffer),
    SO_SYMBOL_FUNC("glGetBufferPointervOES", &so_ret_0),

    // OES compressed paletted texture
    SO_SYMBOL_FUNC("glCompressedTexImage2D", &glCompressedTexImage2D),
    SO_SYMBOL_FUNC("glCompressedTexSubImage2D", &so_ret_0),

    // OES vertex array object
    SO_SYMBOL_FUNC("glBindVertexArrayOES", &so_ret_0),
    SO_SYMBOL_FUNC("glDeleteVertexArraysOES", &so_ret_0),
    SO_SYMBOL_FUNC("glGenVertexArraysOES", &so_ret_0),
    SO_SYMBOL_FUNC("glIsVertexArrayOES", &so_ret_0),
};

so_library so_library_GLESv1_CM = {
    .soname = "libGLESv1_CM.so",
    .symbols = libGLESv1_CM_symbols,
    .num_symbols = sizeof(libGLESv1_CM_symbols) / sizeof(libGLESv1_CM_symbols[0]),
    .prepare     = NULL,
};
