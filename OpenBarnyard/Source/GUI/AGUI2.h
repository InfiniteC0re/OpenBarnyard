#pragma once
#include "AGUI2Context.h"
#include "AGUI2Element.h"
#include "AGUI2Renderer.h"
#include "AGUI2TextBox.h"
#include "AGUI2MouseCursor.h"
#include "AGUI2TextureSection.h"

#include <Toshi/TTask.h>

class AGUI2 : public Toshi::TTask
    , public Toshi::TSingleton<AGUI2>
{
public:
	TDECLARE_CLASS( AGUI2, Toshi::TTask );

public:
	AGUI2();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight );
	void CreateContext();

	TBOOL IsCursorVisible() const { return m_bShowMouseCursor; }
	void  SetCursorVisible( TBOOL a_bVisible ) { m_bShowMouseCursor = a_bVisible; }

public:
	static AGUI2MouseCursor& GetMouseCursor() { return GetSingletonSafe()->m_oMouseCursor; }
	static AGUI2Element*     GetRootElement() { return GetSingletonSafe()->m_pRootElement; }
	static AGUI2Context*     GetContext() { return ms_pCurrentContext; }
	static AGUI2Renderer*    GetRenderer();

private:
	static void MainPostRenderCallback();

private:
	inline static AGUI2Context* ms_pCurrentContext = TNULL;

private:
	AGUI2Element*    m_pRootElement;
	AGUI2MouseCursor m_oMouseCursor;
	TBOOL            m_bShowMouseCursor;
	AGUI2TextBox     m_oFPS;
	AGUI2TextBox     m_oMemStats;
	AGUI2TextBox     m_oPlayerInfo;
	AGUI2TextBox     m_oTexturesInfo;
	TWCHAR           m_wszMemStats[ 320 ];
	TWCHAR           m_wszFPS[ 64 ];
	TWCHAR           m_wszPlayerInfo[ 128 ];
	TWCHAR           m_wszTexturesInfo[ 64 ];
	TBOOL            m_bShowMemStatsInfo;
	TBOOL            m_bShowFPSInfo;
	TBOOL            m_bShowPlayerInfo;
	TBOOL            m_bShowTexturesInfo;
};

TSINGLETON_DECLARE_ALIAS( AGUI2, GUI2 );
