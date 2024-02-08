#include "ToshiPCH.h"
#include "TNativeFile_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

namespace Toshi {

	TBOOL TFileManager::Create()
	{
		CreateCommon();
		auto fileManager = TFileManager::GetSingletonSafe();

		CHAR currentDir[0x200];
		DWORD dirLength = GetCurrentDirectoryA(sizeof(currentDir), currentDir);
		TASSERT(dirLength > 0, "The directory's length is 0");

		(new TNativeFileSystem("local"))->SetPrefix(currentDir);
		(new TNativeFileSystem("abs"))->SetPrefix("");
		(new TNativeFileSystem("null"))->SetPrefix("");

		fileManager->SetSystemPath("local");

		return TTRUE;
	}

}