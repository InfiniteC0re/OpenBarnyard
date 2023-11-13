#pragma once
#include "AGUI2Context.h"
#include "AGUI2Element.h"
#include "AGUI2Renderer.h"
#include "AGUI2TextBox.h"
#include "AGUI2MouseCursor.h"
#include "AGUI2TextureSection.h"

#include <Toshi/Core/TTask.h>

class AGUI2 : 
	public Toshi::TGenericClassDerived<AGUI2, Toshi::TTask, "AGUI2", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::TSingleton<AGUI2>
{
public:
	AGUI2();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;

	void GetDimensions(TFLOAT& a_rWidth, TFLOAT& a_rHeight);
	void CreateContext();

	TBOOL IsCursorVisible() const
	{
		return m_bShowMouseCursor;
	}

public:
	static AGUI2Element* GetRootElement() { return GetSingletonSafe()->m_pRootElement; }
	static AGUI2Context* GetContext() { return ms_pCurrentContext; }
	static AGUI2Renderer* GetRenderer();

private:
	static void MainPostRenderCallback();

private:
	inline static AGUI2Context* ms_pCurrentContext = TNULL;

private:
	AGUI2Element* m_pRootElement;
	AGUI2MouseCursor m_oMouseCursor;
	TBOOL m_bShowMouseCursor;
	AGUI2TextBox m_oFPS;
	AGUI2TextBox m_oMemStats;
	AGUI2TextBox m_oPlayerInfo;
	AGUI2TextBox m_oTexturesInfo;
	wchar_t m_wszMemStats[320];
	wchar_t m_wszFPS[64];
	wchar_t m_wszPlayerInfo[128];
	wchar_t m_wszTexturesInfo[64];
	TBOOL m_bShowMemStatsInfo;
	TBOOL m_bShowFPSInfo;
	TBOOL m_bShowPlayerInfo;
	TBOOL m_bShowTexturesInfo;
};
