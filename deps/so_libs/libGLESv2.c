#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <vitaGL.h>

// ==================== libGLESv2.so (OpenGL ES 2.0) ====================
static so_symbol libGLESv2_symbols[] = {
    // ===== OpenGL ES 2.0 Core Functions =====

    // Shader and Program
    SO_SYMBOL_FUNC("glAttachShader", &glAttachShader),
    SO_SYMBOL_FUNC("glBindAttribLocation", &glBindAttribLocation),
    SO_SYMBOL_FUNC("glCompileShader", &glCompileShader),
    SO_SYMBOL_FUNC("glCreateProgram", &glCreateProgram),
    SO_SYMBOL_FUNC("glCreateShader", &glCreateShader),
    SO_SYMBOL_FUNC("glDeleteProgram", &glDeleteProgram),
    SO_SYMBOL_FUNC("glDeleteShader", &glDeleteShader),
    SO_SYMBOL_FUNC("glDetachShader", &so_ret_0),
    SO_SYMBOL_FUNC("glGetActiveAttrib", &glGetActiveAttrib),
    SO_SYMBOL_FUNC("glGetActiveUniform", &glGetActiveUniform),
    SO_SYMBOL_FUNC("glGetAttachedShaders", &glGetAttachedShaders),
    SO_SYMBOL_FUNC("glGetAttribLocation", &glGetAttribLocation),
    SO_SYMBOL_FUNC("glGetProgramiv", &glGetProgramiv),
    SO_SYMBOL_FUNC("glGetProgramInfoLog", &glGetProgramInfoLog),
    SO_SYMBOL_FUNC("glGetShaderiv", &glGetShaderiv),
    SO_SYMBOL_FUNC("glGetShaderInfoLog", &glGetShaderInfoLog),
    SO_SYMBOL_FUNC("glGetShaderPrecisionFormat", &so_ret_0),
    SO_SYMBOL_FUNC("glGetShaderSource", &glGetShaderSource),
    SO_SYMBOL_FUNC("glGetUniformLocation", &glGetUniformLocation),
    SO_SYMBOL_FUNC("glIsProgram", &glIsProgram),
    SO_SYMBOL_FUNC("glIsShader", &so_ret_0),
    SO_SYMBOL_FUNC("glLinkProgram", &glLinkProgram),
    SO_SYMBOL_FUNC("glReleaseShaderCompiler", &glReleaseShaderCompiler),
    SO_SYMBOL_FUNC("glShaderBinary", &glShaderBinary),
    SO_SYMBOL_FUNC("glShaderSource", &glShaderSource),
    SO_SYMBOL_FUNC("glUseProgram", &glUseProgram),
    SO_SYMBOL_FUNC("glValidateProgram", &so_ret_0),

    // Vertex Attributes
    SO_SYMBOL_FUNC("glDisableVertexAttribArray", &glDisableVertexAttribArray),
    SO_SYMBOL_FUNC("glEnableVertexAttribArray", &glEnableVertexAttribArray),
    SO_SYMBOL_FUNC("glGetVertexAttribfv", &glGetVertexAttribfv),
    SO_SYMBOL_FUNC("glGetVertexAttribiv", &glGetVertexAttribiv),
    SO_SYMBOL_FUNC("glGetVertexAttribPointerv", &glGetVertexAttribPointerv),
    SO_SYMBOL_FUNC("glVertexAttrib1f", &glVertexAttrib1f),
    SO_SYMBOL_FUNC("glVertexAttrib1fv", &glVertexAttrib1fv),
    SO_SYMBOL_FUNC("glVertexAttrib2f", &glVertexAttrib2f),
    SO_SYMBOL_FUNC("glVertexAttrib2fv", &glVertexAttrib2fv),
    SO_SYMBOL_FUNC("glVertexAttrib3f", &glVertexAttrib3f),
    SO_SYMBOL_FUNC("glVertexAttrib3fv", &glVertexAttrib3fv),
    SO_SYMBOL_FUNC("glVertexAttrib4f", &glVertexAttrib4f),
    SO_SYMBOL_FUNC("glVertexAttrib4fv", &glVertexAttrib4fv),
    SO_SYMBOL_FUNC("glVertexAttribPointer", &glVertexAttribPointer),

    // Uniforms
    SO_SYMBOL_FUNC("glUniform1f", &glUniform1f),
    SO_SYMBOL_FUNC("glUniform1fv", &glUniform1fv),
    SO_SYMBOL_FUNC("glUniform1i", &glUniform1i),
    SO_SYMBOL_FUNC("glUniform1iv", &glUniform1iv),
    SO_SYMBOL_FUNC("glUniform2f", &glUniform2f),
    SO_SYMBOL_FUNC("glUniform2fv", &glUniform2fv),
    SO_SYMBOL_FUNC("glUniform2i", &glUniform2i),
    SO_SYMBOL_FUNC("glUniform2iv", &glUniform2iv),
    SO_SYMBOL_FUNC("glUniform3f", &glUniform3f),
    SO_SYMBOL_FUNC("glUniform3fv", &glUniform3fv),
    SO_SYMBOL_FUNC("glUniform3i", &glUniform3i),
    SO_SYMBOL_FUNC("glUniform3iv", &glUniform3iv),
    SO_SYMBOL_FUNC("glUniform4f", &glUniform4f),
    SO_SYMBOL_FUNC("glUniform4fv", &glUniform4fv),
    SO_SYMBOL_FUNC("glUniform4i", &glUniform4i),
    SO_SYMBOL_FUNC("glUniform4iv", &glUniform4iv),
    SO_SYMBOL_FUNC("glUniformMatrix2fv", &glUniformMatrix2fv),
    SO_SYMBOL_FUNC("glUniformMatrix3fv", &glUniformMatrix3fv),
    SO_SYMBOL_FUNC("glUniformMatrix4fv", &glUniformMatrix4fv),

    // Uniform Queries
    SO_SYMBOL_FUNC("glGetUniformfv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetUniformiv", &so_ret_0),

    // Buffer Objects
    SO_SYMBOL_FUNC("glBindBuffer", &glBindBuffer),
    SO_SYMBOL_FUNC("glBufferData", &glBufferData),
    SO_SYMBOL_FUNC("glBufferSubData", &glBufferSubData),
    SO_SYMBOL_FUNC("glDeleteBuffers", &glDeleteBuffers),
    SO_SYMBOL_FUNC("glGenBuffers", &glGenBuffers),
    SO_SYMBOL_FUNC("glGetBufferParameteriv", &glGetBufferParameteriv),
    SO_SYMBOL_FUNC("glIsBuffer", &so_ret_0),

    // Framebuffer Objects
    SO_SYMBOL_FUNC("glBindFramebuffer", &glBindFramebuffer),
    SO_SYMBOL_FUNC("glBindRenderbuffer", &glBindRenderbuffer),
    SO_SYMBOL_FUNC("glCheckFramebufferStatus", &glCheckFramebufferStatus),
    SO_SYMBOL_FUNC("glDeleteFramebuffers", &glDeleteFramebuffers),
    SO_SYMBOL_FUNC("glDeleteRenderbuffers", &glDeleteRenderbuffers),
    SO_SYMBOL_FUNC("glFramebufferRenderbuffer", &glFramebufferRenderbuffer),
    SO_SYMBOL_FUNC("glFramebufferTexture2D", &glFramebufferTexture2D),
    SO_SYMBOL_FUNC("glGenFramebuffers", &glGenFramebuffers),
    SO_SYMBOL_FUNC("glGenRenderbuffers", &glGenRenderbuffers),
    SO_SYMBOL_FUNC("glGenerateMipmap", &glGenerateMipmap),
    SO_SYMBOL_FUNC("glGetFramebufferAttachmentParameteriv", &glGetFramebufferAttachmentParameteriv),
    SO_SYMBOL_FUNC("glGetRenderbufferParameteriv", &so_ret_0),
    SO_SYMBOL_FUNC("glIsFramebuffer", &glIsFramebuffer),
    SO_SYMBOL_FUNC("glIsRenderbuffer", &glIsRenderbuffer),
    SO_SYMBOL_FUNC("glRenderbufferStorage", &glRenderbufferStorage),

    // Textures
    SO_SYMBOL_FUNC("glActiveTexture", &glActiveTexture),
    SO_SYMBOL_FUNC("glBindTexture", &glBindTexture),
    SO_SYMBOL_FUNC("glCompressedTexImage2D", &glCompressedTexImage2D),
    SO_SYMBOL_FUNC("glCompressedTexSubImage2D", &so_ret_0),
    SO_SYMBOL_FUNC("glCopyTexImage2D", &glCopyTexImage2D),
    SO_SYMBOL_FUNC("glCopyTexSubImage2D", &glCopyTexSubImage2D),
    SO_SYMBOL_FUNC("glDeleteTextures", &glDeleteTextures),
    SO_SYMBOL_FUNC("glGenTextures", &glGenTextures),
    SO_SYMBOL_FUNC("glGetTexParameterfv", &so_ret_0),
    SO_SYMBOL_FUNC("glGetTexParameteriv", &so_ret_0),
    SO_SYMBOL_FUNC("glIsTexture", &glIsTexture),
    SO_SYMBOL_FUNC("glPixelStorei", &glPixelStorei),
    SO_SYMBOL_FUNC("glTexImage2D", &glTexImage2D),
    SO_SYMBOL_FUNC("glTexParameterf", &glTexParameterf),
    SO_SYMBOL_FUNC("glTexParameterfv", &so_ret_0),
    SO_SYMBOL_FUNC("glTexParameteri", &glTexParameteri),
    SO_SYMBOL_FUNC("glTexParameteriv", &glTexParameteriv),
    SO_SYMBOL_FUNC("glTexSubImage2D", &glTexSubImage2D),

    // Drawing
    SO_SYMBOL_FUNC("glDrawArrays", &glDrawArrays),
    SO_SYMBOL_FUNC("glDrawElements", &glDrawElements),

    // Blending
    SO_SYMBOL_FUNC("glBlendColor", &so_ret_0),
    SO_SYMBOL_FUNC("glBlendEquation", &glBlendEquation),
    SO_SYMBOL_FUNC("glBlendEquationSeparate", &glBlendEquationSeparate),
    SO_SYMBOL_FUNC("glBlendFunc", &glBlendFunc),
    SO_SYMBOL_FUNC("glBlendFuncSeparate", &glBlendFuncSeparate),

    // Depth and Stencil
    SO_SYMBOL_FUNC("glDepthFunc", &glDepthFunc),
    SO_SYMBOL_FUNC("glDepthMask", &glDepthMask),
    SO_SYMBOL_FUNC("glDepthRangef", &glDepthRangef),
    SO_SYMBOL_FUNC("glStencilFunc", &glStencilFunc),
    SO_SYMBOL_FUNC("glStencilFuncSeparate", &glStencilFuncSeparate),
    SO_SYMBOL_FUNC("glStencilMask", &glStencilMask),
    SO_SYMBOL_FUNC("glStencilMaskSeparate", &glStencilMaskSeparate),
    SO_SYMBOL_FUNC("glStencilOp", &glStencilOp),
    SO_SYMBOL_FUNC("glStencilOpSeparate", &glStencilOpSeparate),

    // Clear and Color
    SO_SYMBOL_FUNC("glClear", &glClear),
    SO_SYMBOL_FUNC("glClearColor", &glClearColor),
    SO_SYMBOL_FUNC("glClearDepthf", &glClearDepthf),
    SO_SYMBOL_FUNC("glClearStencil", &glClearStencil),
    SO_SYMBOL_FUNC("glColorMask", &glColorMask),

    // Face Culling
    SO_SYMBOL_FUNC("glCullFace", &glCullFace),
    SO_SYMBOL_FUNC("glFrontFace", &glFrontFace),

    // Scissor and Viewport
    SO_SYMBOL_FUNC("glScissor", &glScissor),
    SO_SYMBOL_FUNC("glViewport", &glViewport),

    // State Management
    SO_SYMBOL_FUNC("glDisable", &glDisable),
    SO_SYMBOL_FUNC("glEnable", &glEnable),
    SO_SYMBOL_FUNC("glFinish", &glFinish),
    SO_SYMBOL_FUNC("glFlush", &glFlush),
    SO_SYMBOL_FUNC("glHint", &glHint),

    // Queries
    SO_SYMBOL_FUNC("glGetError", &glGetError),
    SO_SYMBOL_FUNC("glGetString", &glGetString),
    SO_SYMBOL_FUNC("glGetBooleanv", &glGetBooleanv),
    SO_SYMBOL_FUNC("glGetFloatv", &glGetFloatv),
    SO_SYMBOL_FUNC("glGetIntegerv", &glGetIntegerv),
    SO_SYMBOL_FUNC("glIsEnabled", &glIsEnabled),

    // Line Width and Polygon Offset
    SO_SYMBOL_FUNC("glLineWidth", &glLineWidth),
    SO_SYMBOL_FUNC("glPolygonOffset", &glPolygonOffset),

    // Read Pixels and Sample Coverage
    SO_SYMBOL_FUNC("glReadPixels", &glReadPixels),
    SO_SYMBOL_FUNC("glSampleCoverage", &so_ret_0),

    // ===== OpenGL ES 2.0 OES Extensions =====

    // OES mapbuffer
    SO_SYMBOL_FUNC("glMapBufferOES", &glMapBuffer),
    SO_SYMBOL_FUNC("glUnmapBufferOES", &glUnmapBuffer),
    SO_SYMBOL_FUNC("glGetBufferPointervOES", &so_ret_0),

    // OES EGL image
    SO_SYMBOL_FUNC("glEGLImageTargetTexture2DOES", &so_ret_0),
    SO_SYMBOL_FUNC("glEGLImageTargetRenderbufferStorageOES", &so_ret_0),

    // OES vertex array object
    SO_SYMBOL_FUNC("glBindVertexArrayOES", &glBindVertexArray),
    SO_SYMBOL_FUNC("glDeleteVertexArraysOES", &glDeleteVertexArrays),
    SO_SYMBOL_FUNC("glGenVertexArraysOES", &glGenVertexArrays),
    SO_SYMBOL_FUNC("glIsVertexArrayOES", &so_ret_0),

    // OES get program binary
    SO_SYMBOL_FUNC("glGetProgramBinaryOES", &glGetProgramBinary),
    SO_SYMBOL_FUNC("glProgramBinaryOES", &glProgramBinary),

    // EXT discard framebuffer
    SO_SYMBOL_FUNC("glDiscardFramebufferEXT", &so_ret_0),

    // EXT multi draw arrays
    SO_SYMBOL_FUNC("glMultiDrawArraysEXT", &glMultiDrawArrays),
    SO_SYMBOL_FUNC("glMultiDrawElementsEXT", &so_ret_0),
};

so_library so_library_GLESv2 = {
    .soname      = "libGLESv2.so",
    .symbols     = libGLESv2_symbols,
    .num_symbols = sizeof(libGLESv2_symbols) / sizeof(libGLESv2_symbols[0]),
    .prepare     = NULL,
};
