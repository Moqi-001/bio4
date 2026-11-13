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
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Compatibility umbrella header: aggregates everything the former single
 * vitaGL.h provided. New code can also include the individual headers
 * directly (<vitaGL/GLES/gl.h>, <vitaGL/GLES2/gl2.h>, <vitaGL/GLES/glcompat.h>, <vitaGL/EGL/egl.h>
 * or <vitaGL/vitaGL.h> for the vgl* private API).
 */

#ifndef _VITAGL_H_
#define _VITAGL_H_

#include <vitaGL/GLES/gl.h>
#include <vitaGL/GLES2/gl2.h>
#include <vitaGL/GLES/glcompat.h>
#include <vitaGL/EGL/egl.h>
/* Extension entry points are exposed unconditionally, matching the rest of
 * the vitaGL headers (no need for apps to define EGL_EGLEXT_PROTOTYPES). */
#define EGL_EGLEXT_PROTOTYPES
#include <vitaGL/EGL/eglext.h>
#include <vitaGL/vitaGL.h>

#endif  // _VITAGL_H_
