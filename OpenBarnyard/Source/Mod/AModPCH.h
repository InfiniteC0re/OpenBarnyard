#pragma once

//-----------------------------------------------------------------------------
// Include this first in a mod's precompiled header. It brings in the game's own
// precompiled header, so game headers see the includes they're written against,
// and the mod API.
//-----------------------------------------------------------------------------
#if !defined( MODAPI_MOD )
#  error "Mod/AModPCH.h is meant for mods, see Mods/ModProject.lua"
#endif

#include "../pch.h"

#include "AModInstance.h"
#include "AModEvents.h"
#include "AModHooks.h"
#include "AModLoader.h"
