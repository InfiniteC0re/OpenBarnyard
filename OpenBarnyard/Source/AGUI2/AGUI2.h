#pragma once
#include "AGUI2Context.h"
#include "AGUI2Element.h"
#include "AGUI2Renderer.h"
#include "AGUI2TextureSection.h"

#include <Toshi/Core/TTask.h>

class AGUI2 : 
	public Toshi::TGenericClassDerived<AGUI2, Toshi::TTask, "AGUI2", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AGUI2>
{
public:
	AGUI2();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(float a_fDeltaTime) override;

	void CreateContext();

public:
	static AGUI2TextureSection* GetTextureSection(const char* a_szSectionName);
	static AGUI2Context* GetContext() { return ms_pCurrentContext; }
	static AGUI2Renderer* GetRenderer();

private:
	static void MainPostRenderCallback();

private:
	inline static AGUI2Context* ms_pCurrentContext = TNULL;

private:
	AGUI2Element* m_pRootElement;
	TBOOL m_bShowMouseCursor;
	// ...
	TBOOL m_bShowMemStatsInfo;
	TBOOL m_bShowFPSInfo;
	TBOOL m_bShowPlayerInfo;
	TBOOL m_bShowTexturesInfo;
};
