/**
 * @file TThread.h
 * Threading system for the Toshi engine
 * 
 * This file provides threading functionality including:
 * - Thread creation and management
 * - Thread synchronization primitives
 * - Platform-specific thread abstraction
 * - Thread safety utilities
 */

#pragma once

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/TThread_Win.h"
#endif // TOSHI_SKU_WINDOWS
