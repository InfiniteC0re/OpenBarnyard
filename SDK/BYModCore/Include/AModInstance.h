#pragma once
#include <Toshi2/T2DList.h>

class AModInstance :
	public Toshi::T2DList<AModInstance>::Node
{
public:
	AModInstance() = default;
	
	virtual ~AModInstance()
	{
		OnUnload();
	}

	virtual TBOOL OnLoad() = 0;
	virtual void OnUnload() { }
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) { return TTRUE; }
	virtual void OnImGuiRender() = 0;
	virtual TBOOL HasSettingsUI() { return TFALSE; }
	virtual const char* GetName() = 0;
};
