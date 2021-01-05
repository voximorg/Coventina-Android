#pragma once

#ifdef __ANDROID__
#define USE_GLES 1
#endif

#if USE_GLES == 1
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif
