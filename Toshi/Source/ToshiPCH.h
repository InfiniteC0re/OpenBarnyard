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
#include "Toshi/Core/Core.h"
#include "Toshi/Utils/TLog.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Core/TColor.h"
#include "Toshi/Core/TObject.h"
#include "Toshi/Math/TMathInline.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Utils/TUtil.h"
#include "Toshi/Thread/TMutexLock.h"
#include "Toshi/Strings/TStringManager.h"

/* Toshi 2.0 Header Files */
#include "Toshi2/Thread/T2MutexLock.h"

/* Platform specific headers */
#ifdef TOSHI_SKU_WINDOWS
	#include <Windows.h>
#endif