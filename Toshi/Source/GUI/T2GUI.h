#pragma once
#include "T2GUIElement.h"
#include "T2GUITextureSection.h"
#include "T2GUIRenderer.h"
#include "T2GUIContext.h"

#include <Toshi/TTask.h>

TOSHI_NAMESPACE_START

class T2GUI
{
public:
	static T2GUIContext*  GetContext() { return ms_pCurrentContext; }
	static T2GUIRenderer* GetRenderer() { return ms_pCurrentContext->GetRenderer(); }

	// $Barnyard: FUNCTION 006c47c0
	static T2GUIContext* CreateContext()
	{
		ms_pCurrentContext = new T2GUIContext;
		return ms_pCurrentContext;
	}

private:
	inline static T2GUIContext* ms_pCurrentContext = TNULL;
};

TOSHI_NAMESPACE_END
