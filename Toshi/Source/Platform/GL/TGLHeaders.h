#pragma once

#ifdef TOSHI_SKU_WASM
#  include <GLES3/gl3.h>
#  include <GLES2/gl2ext.h>
#else
#  define GLEW_STATIC
#  include <GL/glew.h>
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#  define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#  define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#  define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
