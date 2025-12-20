#pragma once
#include "T2AtomicMutex.h"

#define T2MUTEX_LOCK_SCOPE( mutex ) TANONYMOUSVAR( Toshi::T2MutexLock )( mutex )

TOSHI_NAMESPACE_START

using T2Mutex = T2AtomicMutex;
using T2MutexLock = T2AtomicMutexLock;

TOSHI_NAMESPACE_END
