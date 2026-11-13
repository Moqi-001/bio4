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
 * vitaGL private API header: the vgl* functions, memory pools and translator
 * configuration. For the full GL/EGL surface include <vitaGL.h> (compat
 * umbrella) or the individual <vitaGL/GLES/gl.h>, <vitaGL/GLES2/gl2.h> and
 * <vitaGL/EGL/egl.h> headers.
 */

#ifndef _VITAGL_VITAGL_H_
#define _VITAGL_VITAGL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <vitaGL/GLES/gl.h>

#include <vitashark.h>

#define VGL_YUV420P_NV12_BT601 0x18E70
#define VGL_YVU420P_NV21_BT601 0x18E71
#define VGL_YUV420P_NV12_BT709 0x18E72
#define VGL_YVU420P_NV21_BT709 0x18E73
#define VGL_YUV420P_BT601      0x18E74
#define VGL_YVU420P_BT601      0x18E75
#define VGL_YUV420P_BT709      0x18E76
#define VGL_YVU420P_BT709      0x18E77

// VGL_EXT_gpu_objects_array extension
// glColorPointer equivalent for legacy vgl* draw pipeline.
void vglColorPointer(GLint size, GLenum type, GLsizei stride, GLuint count, const GLvoid *pointer);

// Overloads the color object with a pre-GPU mapped memory block with a copy-less action for legacy vgl* draw pipeline.
void vglColorPointerMapped(GLenum type, const GLvoid *pointer);

// Performs a draw with the legacy vgl* draw pipeline.
void vglDrawObjects(GLenum mode, GLsizei count, GLboolean implicit_wvp);

// Sets up the index list to use for the subsequent legacy vgl* draw pipeline draw action.
void vglIndexPointer(GLenum type, GLsizei stride, GLuint count, const GLvoid *pointer);

// Overloads the index list with an internal progressive index buffer normally used for glDrawArrays call.
void vglIndexPointerDefault();

// Overloads the index list with a pre-GPU mapped memory block with a copy-less action for legacy vgl* draw pipeline.
void vglIndexPointerMapped(const GLvoid *pointer);

// glTexCoordPointer equivalent for legacy vgl* draw pipeline.
void vglTexCoordPointer(GLint size, GLenum type, GLsizei stride, GLuint count, const GLvoid *pointer);

// Overloads the texture coords object with a pre-GPU mapped memory block with a copy-less action for legacy vgl* draw pipeline.
void vglTexCoordPointerMapped(const GLvoid *pointer);

// glVertexPointer equivalent for legacy vgl* draw pipeline.
void vglVertexPointer(GLint size, GLenum type, GLsizei stride, GLuint count, const GLvoid *pointer);

// Overloads the vertex object with a pre-GPU mapped memory block with a copy-less action for legacy vgl* draw pipeline.
void vglVertexPointerMapped(GLint size, const GLvoid *pointer);

// VGL_EXT_gxp_shaders extension implementation
// glBindAttribLocation equivalent for legacy vgl* draw pipeline.
void vglBindAttribLocation(GLuint prog, GLuint index, const GLchar *name, const GLuint num, const GLenum type);

// glBindAttribLocation equivalent for legacy vgl* draw pipeline with packed attributes support.
GLint vglBindPackedAttribLocation(GLuint prog, const GLchar *name, const GLuint num, const GLenum type, GLuint offset, GLint stride);

// glVertexAttribPointer equivalent for legacy vgl* draw pipeline.
void vglVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint count, const GLvoid *pointer);

// Overloads a vertex attrib pointer with a pre-GPU mapped memory block with a copy-less action for legacy vgl* draw pipeline.
void vglVertexAttribPointerMapped(GLuint index, const GLvoid *pointer);

// Get the compiled shader binary of a given GL shader.
void vglGetShaderBinary(GLuint index, GLsizei bufSize, GLsizei *length, void *binary);

typedef enum {
    VGL_MEM_VRAM,      // CDRAM
    VGL_MEM_RAM,       // USER_RW RAM
    VGL_MEM_SLOW,      // PHYCONT_USER_RW RAM
    VGL_MEM_BUDGET,    // CDLG RAM
    VGL_MEM_EXTERNAL,  // newlib mem
    VGL_MEM_ALL
} vglMemType;

typedef enum {
    VGL_TYPE_NONE,      // No semantic
    VGL_TYPE_TEXCOORD,  // TEXCOORD#
    VGL_TYPE_COLOR,     // COLOR#
    VGL_TYPE_FOG,       // FOG
    VGL_TYPE_CLIP       // CLP#
} vglSemanticType;

typedef enum {
    VGL_MODE_SHADER_PAIR,  // Assumes glCompileShader is always called in couple of Vertex + Fragment shaders that will then be linked in the same program. Great results if this premise is respected.
    VGL_MODE_GLOBAL,       // Uses a global semantics pool for translating shaders. Less accuracy than VGL_MODE_SHADER_PAIR but has no premise that must be respected.
    VGL_MODE_POSTPONED     // Moves shaders compilation into glLinkProgram. Best results since will always have correct shader couples for the translation for accurate semantic bindings resolution.
} vglSemanticMode;

// vgl*
// Add a new global custom semantic binding for the GLSL translator.
void vglAddSemanticBinding(const GLchar *const *varying, GLint index, GLenum type);

// Add a new global custom semantic binding hint for the GLSL translator.
void vglAddSemanticBindingHint(const GLchar *const *varying, GLenum type);

// Alloc memory from vitaGL internal memory pools. Needs to be freed with vglFree.
void *vglAlloc(uint32_t size, vglMemType type);

// Alloc temporary memory from vitaGL internal scratch pool. Allocation is extremely fast if DISABLE_CIRCULAR_POOL is not used. Memory lifetime is guaranteed for 3 frames.
void *vglAllocFromScratch(size_t size);

// Overloads a buffer object with a pre-GPU mapped memory block with a copy-less action.
void vglBufferData(GLenum target, const GLvoid *data);

// calloc implementation for vitaGL internal memory pools.
void *vglCalloc(uint32_t nmember, uint32_t size);

// Alloc memory from vitaGL internal memory pools. If the memory pools exhausted, vitaGL will attempt to free enough memory to not fail this allocation. Needs to be freed with vglFree.
void *vglForceAlloc(uint32_t size);

// Frees a previously allocated memory block in the vitaGL internal memory pools.
void vglFree(void *addr);

// Get a GL function name given a function address.
char *vglGetFuncName(uint32_t func);

// Get the internal sceGxm texture descriptor of a GL texture.
SceGxmTexture *vglGetGxmTexture(GLenum target);

// Get a GL function address given a function name.
void *vglGetProcAddress(const char *name);

// Get the internal texture data pointer of a GL texture.
void *vglGetTexDataPointer(GLenum target);

// MSAA mode requested from vglInit. The zero value is VGL_MSAA_NONE, so a
// zero-initialised vglInitParams runs without multisampling; ask for
// VGL_MSAA_2X / VGL_MSAA_4X to enable it.
typedef enum {
    VGL_MSAA_NONE = 0,
    VGL_MSAA_2X,
    VGL_MSAA_4X,
} vglMsaaMode;

// Configuration for vglInit. Zero-initialise it and override only what you
// need; every field documents what its zero value means:
//
//   vglInitParams params = {0};
//   params.legacy_pool_size = 0x800000;
//   vglInit(&params);
//
// The pool thresholds are the amount of memory to RESERVE, not the amount the
// pool will get: the pool takes what is left of the free memory at init time.
typedef struct {
    // Display
    int         width;                 // 0 = 960
    int         height;                // 0 = 544
    vglMsaaMode msaa;                  // 0 = 4X
    int         display_buffer_count;  // 0 = 3

    // Fixed-function (immediate mode) vertex pool
    int legacy_pool_size;  // 0 = not allocated

    // Memory pools: free memory to leave untouched, per pool
    uint32_t ram_threshold;      // 0 = keep 16 MiB free
    uint32_t cdram_threshold;    // 0 = leave nothing free (pool takes all)
    uint32_t phycont_threshold;  // 0 = leave nothing free (pool takes all)
    uint32_t cdlg_threshold;     // 0 = no common-dialog pool

    // Internal circular pools
    uint32_t circular_pool_size;  // 0 = 32 MiB

    // Internal memory pools backed by cached memory instead of uncached
    GLboolean use_cached_mem;  // 0 = GL_FALSE

    // sceGxm ring buffers
    uint32_t param_buffer_size;          // 0 = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE
    uint32_t vdm_buffer_size;            // 0 = SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE
    uint32_t vertex_buffer_size;         // 0 = SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE
    uint32_t fragment_buffer_size;       // 0 = SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE
    uint32_t fragment_usse_buffer_size;  // 0 = SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE

    // Shader patcher
    uint32_t shader_patcher_buffer_size;         // 0 = 1 MiB
    uint32_t shader_patcher_vertex_usse_size;    // 0 = 1 MiB
    uint32_t shader_patcher_fragment_usse_size;  // 0 = 1 MiB

    // Garbage collector thread
    int gc_thread_priority;  // 0 = 0x10000100
    int gc_thread_affinity;  // 0 = main CPU mask
} vglInitParams;

// Initializes vitaGL. Pass NULL to initialize with all defaults.
// The fields that overlap with a vglSet*/vglSetup* setter take precedence over
// values configured through those setters.
GLboolean vglInit(const vglInitParams *params);

// Enables per-client-API validation (default: disabled for compatibility with
// Android games that call GLES1 entry points from GLES2 contexts).
void vglSetStrictAPI(GLboolean enable);

// Mark a memory block from a vitaGL internal memory pool to be deleted as soon as GPU finishes using it.
void vglLazyFree(void *addr);

// malloc implementation for vitaGL internal memory pools.
void *vglMalloc(uint32_t size);

// malloc_usable_size implementation for vitaGL internal memory pools.
size_t vglMallocUsableSize(void *ptr);

// memalign implementation for vitaGL internal memory pools.
void *vglMemalign(uint32_t alignment, uint32_t size);

// Gets the total amount of free memory in a given internal memory pool.
size_t vglMemFree(vglMemType type);

// Gets the total amount of free and used memory in a given internal memory pool.
size_t vglMemTotal(vglMemType type);

// Replaces original texture data pointer with a new one in a GL texture.
void vglOverloadTexDataPointer(GLenum target, void *data);

// Allows to override texture format of a GL texture. Requires ENABLE_UNPURE_TEXFORMATS.
void vglOverrideTexFormat(GLenum target);

// Variant of glReadPixels that uses GPU underneat to perform the readback. The passed data pointer must be GPU mapped (eg: heap memory).
void vglReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *data);

// realloc implementation for vitaGL internal memory pools.
void *vglRealloc(void *ptr, uint32_t size);

// Change the total memory to use for internal circular pools to use in vitaGL. Disabled with DISABLE_CIRCULAR_POOL. Default value: 32 * 1024 * 1024.
void vglSetCircularPoolSize(uint32_t size);

// Sets the number of buffers to use for the display swapchain. Default value: 3.
void vglSetDisplayBufferCount(int count);

// Setup a callback executed everytime a new frame is sent to the display. Useful to setup a CPU rendered overlay on-screen.
void vglSetDisplayCallback(void (*cb)(void *framebuf));

// Setup the fragment ring buffer size of sceGxm. Must be called before vglInit*. Default value: SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE.
void vglSetFragmentBufferSize(uint32_t size);

// Setup the parameter buffer size of sceGxm. Must be called before vglInit*. Default value: SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE.
void vglSetParamBufferSize(uint32_t size);

// Change the currently used semantics binding resolution mode for the GLSL translator. Default value: VGL_MODE_POSTPONED.
void vglSetSemanticBindingMode(GLenum mode);

// Change the lifetime for a texture to be considered cacheable. Requires ENABLE_TEXTURE_CACHE.
void vglSetTextureCacheFrequency(GLuint freq);

// Setup the fragment USSE ring buffer size of sceGxm. Must be called before vglInit*. Default value: SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE.
void vglSetUSSEBufferSize(uint32_t size);

// Setup the VDM ring buffer size of sceGxm. Must be called before vglInit*. Default value: SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE.
void vglSetVDMBufferSize(uint32_t size);

// Setup the circular pools size used for generic attribute values. There is a unique pool per vertex array object. Default values: Main: 256 * 1024, Auxiliary: 64 * 1024.
void vglSetVertexAttribPoolSize(uint32_t main_size, uint32_t aux_size);

// Setup the vertex ring buffer size of sceGxm. Must be called before vglInit*. Default value: SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE.
void vglSetVertexBufferSize(uint32_t size);

// Change the scenes per frame value to use for the display rendertarget. Default value: 1
void vglSetupDisplayRenderTarget(uint8_t size);

// Change the priority and affinity to use for the garbage collector thread. Must be called before vglInit*.
void vglSetupGarbageCollector(int priority, int affinity);

// Change what kind of vertex buffer objects are considered eligible for scratch memory usage in order to reduce allocation costs. Requires USE_SCRATCH_MEMORY.
void vglSetupScratchMemory(GLboolean scratch_for_dynamic, GLboolean scratch_for_stream);

// Setup the buffer sizes of the sceGxm shader patcher. Must be called before vglInit*. Default values: 1024 * 1024 each.
void vglSetupShaderPatcher(uint32_t buffer_mem_size, uint32_t vertex_usse_mem_size, uint32_t fragment_usse_mem_size);

// Change optimizations configuration for the runtime shader compiler.
void vglSetupRuntimeShaderCompiler(shark_opt opt_level, int32_t use_fastmath, int32_t use_fastprecision, int32_t use_fastint);

// Load a precompiled gxp binary to a given shader handle
void vglShaderGxpBinary(GLsizei count, const GLuint *handles, const void *binary, GLsizei length);

// Perform a display buffer swap. Equivalent of eglSwapBuffers but allows support with Common Dialog.
void vglSwapBuffers(GLboolean has_commondialog);

// Enqueue a request for display resolution change that will happen at next vglSwapBuffers call.
GLboolean vglSwapResolution(int width, int height);

// Loads the depth buffer of the currently bound renderbuffer into the currently bound GL texture.
void vglTexImageDepthBuffer(GLenum target);

// Makes vitaGL use cached memory instead of uncached memory for its internal memory pools. Must be called before vglInit*.
void vglUseCachedMem(GLboolean use);

// Makes the GLSL translator use low precision variables (eg: float -> half).
void vglUseLowPrecision(GLboolean val);

// Allows to swap between triple and double buffering. vitaGL by default uses triple buffering.
void vglUseTripleBuffering(GLboolean usage);

// Allows to set a preference on the kind of memory to use for the internal USSE buffers in sceGxm. By default vitaGL will not use VRAM memory.
void vglUseVramForUSSE(GLboolean usage);

// Allows vitaGL to use newlib memory once all internal mempools are exhausted. Default value: GL_TRUE.
void vglUseExtraMem(GLboolean usage);

// Simplified function to enable or disable V-Sync. For more fine granularity on the swap interval use eglSwapInterval.
void vglWaitVblankStart(GLboolean enable);

#ifdef __cplusplus
}
#endif

#endif  // _VITAGL_VITAGL_H_
