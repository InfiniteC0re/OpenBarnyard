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

/* Toshi Header Files */
#include "Toshi/Typedefs.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Core/STL.h"
#include "Toshi/Core/Core.h"
#include "Toshi/Core/TObject.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Core/THPTimer.h"
#include "Toshi/Core/TTask.h"
#include "Toshi/Math/Math.h"
#include "Toshi/Utils/TLog.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Utils/TUtil.h"
#include "Toshi/Utils/TMutex.h"
#include "Toshi/Utils/TMutexLock.h"
#include "Toshi/Strings/TStringManager.h"

/* Toshi 2.0 Header Files */
#include "Toshi2/T2Mutex.h"
#include "Toshi2/T2MutexLock.h"

/* Platform specific headers */
#ifdef TOSHI_PLATFORM_WINDOWS
	#include <Windows.h>
#endif