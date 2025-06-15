/**
 * @file Toshi.h
 * Main header file for the Toshi game engine
 * 
 * This file serves as the primary include point for the Toshi engine,
 * bringing together core systems including:
 * - Memory management
 * - Platform abstraction
 * - Debug utilities
 * - Math functionality
 * - String management
 * - Object system
 */

#pragma once

#include "Toshi/Defines.h"
#include "Toshi/T2UniqueValue.h"

#include "Toshi/Typedefs.h"
#include "Toshi/Platform.h"
#include "Toshi/Profiler.h"
#include "Toshi/Endianness.h"
#include "Toshi/Helpers.h"
#include "Toshi/Verify.h"

#include "Toshi/TDebug.h"
#include "Toshi/TUtil.h"
#include "Toshi/TSingleton.h"
#include "Toshi/TObject.h"
#include "Toshi/T2Iterator.h"

#include "Core/TMemory.h"

#include "Toshi/TStringManager.h"

#include "Math/TMathInline.h"
#include "Math/TMath.h"
