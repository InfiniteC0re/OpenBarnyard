#pragma once

#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>
#include <utility>

#include "Toshi/Typedefs.h"
#include "Toshi/Core/STL.h"
#include "Toshi/Core/Core.h"
#include "Toshi/Core/TObject.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Core/TTask.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Math/TMathInline.h"
#include "Toshi/Math/TVector3.h"
#include "Toshi/Math/TVector4.h"
#include "Toshi/Math/TQuaternion.h"
#include "Toshi/Utils/TLog.h"
#include "Toshi/Utils/TSingleton.h"

#ifdef TOSHI_PLATFORM_WINDOWS
	#include <Windows.h>
#endif