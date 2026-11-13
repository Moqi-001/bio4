/*
 * This file is part of vitaGL
 * Copyright 2017, 2018, 2019, 2020 Rinnegatamante
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * glcompat.h:
 * Entry points that are NOT part of the OpenGL ES 1.1 / 2.0 cores, kept so
 * that code written against other GL dialects keeps compiling: desktop-GL
 * immediate mode and matrix calls, display lists, the GLU helpers, the
 * DSA-style entry points, a few GLES3/ANGLE entry points and vitaGL's
 * explicit matrix API. Cross-generation by nature (most of it is desktop
 * GL 1.x flavored, but it also carries newer GLES entry points).
 *
 * Best-effort only: declarations match the historical vitaGL signatures and
 * are not covered by the GLES conformance goal. The supported contract is
 * GLES/gl.h and GLES2/gl2.h; standard GLES users never need this file.
 */

#ifndef VITAGL_GLES_COMPAT_H
#define VITAGL_GLES_COMPAT_H

#include <vitaGL/GLES/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Desktop-GL types that GLES does not define */
typedef double GLdouble;
typedef double GLclampd;

// ===== Constants not in GLES =====

#define GL_INVALID_INDEX                                 0xFFFFFFFF

#define GL_QUADS                                        0x0007
#define GL_QUAD_STRIP                                   0x0008
#define GL_POLYGON                                      0x0009
#define GL_POLYGON_MODE                                 0x0B40
#define GL_DOUBLEBUFFER                                 0x0C32
#define GL_UNPACK_ROW_LENGTH                            0x0CF2
#define GL_TEXTURE_1D                                   0x0DE0
#define GL_COMPILE                                      0x1300
#define GL_COMPILE_AND_EXECUTE                          0x1301
#define GL_HALF_FLOAT                                   0x140B
#define GL_COLOR_INDEX                                  0x1900
#define GL_RED                                          0x1903
#define GL_GREEN                                        0x1904
#define GL_BLUE                                         0x1905
#define GL_POINT                                        0x1B00
#define GL_LINE                                         0x1B01
#define GL_FILL                                         0x1B02
#define GL_CLAMP                                        0x2900
#define GL_POLYGON_OFFSET_POINT                         0x2A01
#define GL_POLYGON_OFFSET_LINE                          0x2A02
#define GL_V2F                                          0x2A20
#define GL_V3F                                          0x2A21
#define GL_C4UB_V2F                                     0x2A22
#define GL_C4UB_V3F                                     0x2A23
#define GL_C3F_V3F                                      0x2A24
#define GL_T2F_V3F                                      0x2A27
#define GL_T4F_V4F                                      0x2A28
#define GL_T2F_C4UB_V3F                                 0x2A29
#define GL_T2F_C3F_V3F                                  0x2A2A
#define GL_CLIP_PLANE6                                  0x3006
#define GL_ABGR_EXT                                     0x8000
#define GL_MIN                                          0x8007
#define GL_MAX                                          0x8008
#define GL_UNSIGNED_INT_8_8_8_8                         0x8035
#define GL_INTENSITY                                    0x8049
#define GL_RGB8                                         0x8051
#define GL_RGBA8                                        0x8058
#define GL_COLOR_TABLE                                  0x80D0
#define GL_BGR                                          0x80E0
#define GL_BGRA                                         0x80E1
#define GL_COLOR_INDEX8_EXT                             0x80E5
#define GL_MAX_ELEMENTS_VERTICES                        0x80E8
#define GL_MAX_ELEMENTS_INDICES                         0x80E9
#define GL_PHONG_WIN                                    0x80EA
#define GL_DEPTH_COMPONENT24                            0x81A6
#define GL_DEPTH_COMPONENT32                            0x81A7
#define GL_DEPTH_STENCIL_ATTACHMENT                     0x821A
#define GL_RG                                           0x8227
#define GL_R8                                           0x8229
#define GL_UNSIGNED_SHORT_1_5_5_5_REV                   0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV                     0x8367
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                0x83F3
#define GL_TEXTURE_COMPRESSION_HINT                     0x84EF
#define GL_TEXTURE_MAX_ANISOTROPY_EXT                   0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT               0x84FF
#define GL_TEXTURE_LOD_BIAS                             0x8501
#define GL_MIRROR_CLAMP_EXT                             0x8742
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD              0x87EE
#define GL_RGBA16F                                      0x881A
#define GL_POINT_SPRITE                                 0x8861
#define GL_MAX_TEXTURE_COORDS                           0x8871
#define GL_READ_ONLY                                    0x88B8
#define GL_WRITE_ONLY                                   0x88B9
#define GL_READ_WRITE                                   0x88BA
#define GL_STREAM_READ                                  0x88E1
#define GL_STREAM_COPY                                  0x88E2
#define GL_STATIC_READ                                  0x88E5
#define GL_STATIC_COPY                                  0x88E6
#define GL_DYNAMIC_READ                                 0x88E9
#define GL_DYNAMIC_COPY                                 0x88EA
#define GL_DEPTH24_STENCIL8                             0x88F0
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG              0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG              0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG             0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG             0x8C03
#define GL_SRGB                                         0x8C40
#define GL_SRGB8                                        0x8C41
#define GL_SRGB_ALPHA                                   0x8C42
#define GL_SRGB8_ALPHA8                                 0x8C43
#define GL_SLUMINANCE_ALPHA                             0x8C44
#define GL_SLUMINANCE8_ALPHA8                           0x8C45
#define GL_SLUMINANCE                                   0x8C46
#define GL_SLUMINANCE8                                  0x8C47
#define GL_COMPRESSED_SRGB                              0x8C48
#define GL_COMPRESSED_SRGB_ALPHA                        0x8C49
#define GL_COMPRESSED_SRGB_S3TC_DXT1                    0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1              0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3              0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5              0x8C4F
#define GL_ATC_RGB_AMD                                  0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                  0x8C93
#define GL_READ_FRAMEBUFFER                             0x8CA8
#define GL_DRAW_FRAMEBUFFER                             0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING                     0x8CAA
#define GL_MAX_COLOR_ATTACHMENTS                        0x8CDF
#define GL_DEPTH_COMPONENT32F                           0x8DAB
#define GL_DEPTH32F_STENCIL8                            0x8DAC
#define GL_FRAMEBUFFER_SRGB                             0x8DB9
#define GL_COMPRESSED_RED_RGTC1                         0x8DBB
#define GL_COMPRESSED_RG_RGTC2                          0x8DBD
#define GL_HALF_FLOAT_OES                               0x8D61
#define GL_ETC1_RGB8_OES                                0x8D64
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX         0x9047
#define GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX   0x9048
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG             0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG             0x9138
#define GL_COMPRESSED_RGBA8_ETC2_EAC                    0x9278
#define GL_MAX_TEXTURE_LOD_BIAS                         31
#define GL_POINT_SIZE_RANGE                             0x0B12
#define GL_POINT_BIT                                    0x00000002
#define GL_LINE_BIT                                     0x00000004
#define GL_POLYGON_BIT                                  0x00000008
#define GL_LIGHTING_BIT                                 0x00000040
#define GL_FOG_BIT                                      0x00000080
#define GL_VIEWPORT_BIT                                 0x00000800
#define GL_TRANSFORM_BIT                                0x00001000
#define GL_ENABLE_BIT                                   0x00002000
#define GL_HINT_BIT                                     0x00008000
#define GL_SCISSOR_BIT                                  0x00080000
#define GL_ALL_ATTRIB_BITS                              0xFFFFFFFF
#define GL_MAP_READ_BIT                                 0x0001
#define GL_MAP_WRITE_BIT                                0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT                     0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT                    0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT                       0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT                       0x0020
#define GL_DRAW_FRAMEBUFFER_BINDING                     GL_FRAMEBUFFER_BINDING
#define GL_MAJOR_VERSION                      0x821B
#define GL_MINOR_VERSION                      0x821C
#define GL_NUM_EXTENSIONS                     0x821D
#define GL_QUERY_TARGET                       0x82EA
#define GL_PROGRAM_ERROR_POSITION_ARB         0x864B
#define GL_PROGRAM_BINARY_LENGTH              0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS         0x87FE
#define GL_QUERY_RESULT                       0x8866
#define GL_QUERY_RESULT_AVAILABLE             0x8867
#define GL_CG_VERTEX_SHADER_EXT               0x890E
#define GL_CG_FRAGMENT_SHADER_EXT             0x890F
#define GL_SAMPLES_PASSED                     0x8914
#define GL_SAMPLER_BINDING                    0x8919
#define GL_UNIFORM_BUFFER                     0x8A11
#define GL_MAX_VARYING_FLOATS                 0x8B4B
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS    0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS      0x8B4A
#define GL_ANY_SAMPLES_PASSED                 0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE    0x8D6A
#define GL_QUERY_RESULT_NO_WAIT               0x9194

// ===== Entry points not in GLES =====

void           glBegin(GLenum mode);
void           glBeginQuery(GLenum target, GLuint id);
void           glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
void           glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
void           glBindSampler(GLuint unit, GLuint smp);
void           glBindVertexArray(GLuint array);
void           glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void           glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void           glCallList(GLuint list);
void           glCallLists(GLsizei n, GLenum type, const void *lists);
GLenum         glCheckNamedFramebufferStatus(GLuint target, GLenum dummy);
void           glClearDepth(GLdouble depth);
void           glClipPlane(GLenum plane, const GLdouble *equation);
void           glColor3f(GLfloat red, GLfloat green, GLfloat blue);
void           glColor3fv(const GLfloat *v);
void           glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
void           glColor3ubv(const GLubyte *v);
void           glColor3us(GLushort red, GLushort green, GLushort blue);
void           glColor3usv(const GLushort *c);
void           glColor4fv(const GLfloat *v);
void           glColor4ubv(const GLubyte *v);
void           glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
void           glColor4usv(const GLushort *c);
void           glColorMaterial(GLenum face, GLenum mode);
void           glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *data);
void           glCompressedTextureImage2D(GLuint texture, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
void           glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void           glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void           glCopyTextureImage1D(GLuint texture, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void           glCopyTextureImage2D(GLuint texture, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void           glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void           glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void           glCreateBuffers(GLsizei n, GLuint *buffers);
void           glCreateFramebuffers(GLsizei n, GLuint *framebuffers);
void           glCreateTextures(GLenum target, GLsizei n, GLuint *textures);
void           glDeleteLists(GLuint list, GLsizei range);
void           glDeleteQueries(GLsizei n, const GLuint *ids);
void           glDeleteSamplers(GLsizei n, const GLuint *smp);
void           glDeleteVertexArrays(GLsizei n, const GLuint *gl_arrays);
void           glDepthRange(GLdouble nearVal, GLdouble farVal);
void           glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
void           glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *gl_indices, GLint baseVertex);
void           glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *gl_indices, GLsizei primcount);
void           glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
void           glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, void *indices, GLint basevertex);
void           glEnd(void);
void           glEndList(void);
void           glEndQuery(GLenum target);
void           glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
void           glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length);
void           glFogi(GLenum pname, const GLint param);
void           glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
void           glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);
void           glGenerateTextureMipmap(GLuint target);
void           glGenQueries(GLsizei n, GLuint *ids);
GLuint         glGenLists(GLsizei range);
void           glGenSamplers(GLsizei n, GLuint *smps);
void           glGenVertexArrays(GLsizei n, GLuint *res);
void           glGetDoublev(GLenum pname, GLdouble *data);
void           glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint *params);
void           glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params);
const GLubyte *glGetStringi(GLenum name, GLuint index);
void           glInterleavedArrays(GLenum format, GLsizei stride, const void *pointer);
void           glLighti(GLenum light, GLenum pname, GLint param);
void           glLightModeli(GLenum pname, GLint param);
void           glListBase(GLuint base);
void           glLoadMatrixd(const GLdouble *m);
void           glLoadTransposeMatrixf(const GLfloat *m);
void           glLoadTransposeMatrixx(const GLfixed *m);
void          *glMapBuffer(GLenum target, GLbitfield access);
void          *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
void          *glMapNamedBuffer(GLuint buffer, GLenum access);
void          *glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);
void           glMateriali(GLenum face, GLenum pname, GLint param);
void           glMatrixFrustum(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void           glMatrixLoadd(GLenum mode, const GLdouble *matrix);
void           glMatrixLoadf(GLenum mode, const GLfloat *matrix);
void           glMatrixLoadIdentity(GLenum mode);
void           glMatrixMultd(GLenum mode, const GLdouble *matrix);
void           glMatrixMultf(GLenum mode, const GLfloat *matrix);
void           glMatrixOrtho(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void           glMatrixPop(GLenum matrixMode);
void           glMatrixPush(GLenum matrixMode);
void           glMatrixRotated(GLenum matrixMode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void           glMatrixRotatef(GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void           glMatrixScaled(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
void           glMatrixScalef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
void           glMatrixTranslated(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
void           glMatrixTranslatef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
void           glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
void           glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
void           glMultiTexCoord2fv(GLenum target, GLfloat *f);
void           glMultiTexCoord2i(GLenum target, GLint s, GLint t);
void           glMultMatrixd(const GLdouble *m);
void           glMultTransposeMatrixd(const GLdouble *m);
void           glMultTransposeMatrixf(const GLfloat *m);
void           glMultTransposeMatrixx(const GLfixed *m);
void           glNamedBufferData(GLuint buffer, GLsizei size, const void *data, GLenum usage);
void           glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data);
void           glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void           glNamedFramebufferTexture(GLuint target, GLenum attachment, GLuint texture, GLint level);
void           glNamedFramebufferTexture2D(GLuint target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void           glNamedRenderbufferStorage(GLuint target, GLenum internalformat, GLsizei width, GLsizei height);
void           glNewList(GLuint list, GLenum mode);
void           glNormal3fv(const GLfloat *v);
void           glNormal3s(GLshort x, GLshort y, GLshort z);
void           glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);
void           glPointParameteri(GLenum pname, GLint param);
void           glPointParameteriv(GLenum pname, const GLint *params);
void           glPolygonMode(GLenum face, GLenum mode);
void           glPopAttrib(void);
void           glPopGroupMarker(void);
void           glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
void           glPushAttrib(GLbitfield mask);
void           glPushGroupMarker(GLsizei length, const GLchar *marker);
void           glReadBuffer(GLenum mode);
void           glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void           glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
void           glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void           glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
void           glSamplerParameteri(GLuint target, GLenum pname, GLint param);
void           glScaled(GLdouble x, GLdouble y, GLdouble z);
void           glTexCoord2f(GLfloat s, GLfloat t);
void           glTexCoord2fv(GLfloat *f);
void           glTexCoord2i(GLint s, GLint t);
void           glTexCoord2s(GLshort s, GLshort t);
void           glTexImage1D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *data);
void           glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void           glTextureImage1D(GLuint texture, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *data);
void           glTextureImage2D(GLuint texture, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void           glTextureParameterf(GLuint texture, GLenum pname, GLfloat param);
void           glTextureParameteri(GLuint texture, GLenum pname, GLint param);
void           glTextureParameteriv(GLuint texture, GLenum pname, GLint *param);
void           glTextureParameterx(GLuint texture, GLenum pname, GLfixed param);
void           glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void           glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void           glTranslated(GLdouble x, GLdouble y, GLdouble z);
GLboolean      glUnmapBuffer(GLenum target);
GLboolean      glUnmapNamedBuffer(GLuint buffer);
void           glVertex2d(GLdouble x, GLdouble y);
void           glVertex2f(GLfloat x, GLfloat y);
void           glVertex2i(GLint x, GLint y);
void           glVertex2dv(const GLdouble *v);
void           glVertex2fv(const GLfloat *v);
void           glVertex3d(GLdouble x, GLdouble y, GLdouble z);
void           glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void           glVertex3i(GLint x, GLint y, GLint z);
void           glVertex3dv(const GLdouble *v);
void           glVertex3fv(const GLfloat *v);
void           gluBuild2DMipmaps(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data);
void           gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ);
void           gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
GLint          gluScaleImage(GLenum format, GLsizei wIn, GLsizei hIn, GLenum typeIn, const void *dataIn, GLsizei wOut, GLsizei hOut, GLenum typeOut, void *dataOut);
void      glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
GLuint    glGetUniformBlockIndex(GLuint prog, const GLchar *uniformBlockName);
void      glProgramUniform1f(GLuint program, GLint location, GLfloat v0);
void      glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
void      glProgramUniform1i(GLuint program, GLint location, GLint v0);
void      glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value);
void      glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1);
void      glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
void      glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1);
void      glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value);
void      glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void      glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
void      glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
void      glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value);
void      glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void      glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
void      glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
void      glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value);
void      glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void      glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void      glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void      glUniformBlockBinding(GLuint prog, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
void      glVertexAttribDivisor(GLuint index, GLuint divisor);

#ifdef __cplusplus
}
#endif

#endif  // VITAGL_GLES_COMPAT_H
