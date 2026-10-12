#pragma once
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

#include <Input/TInputDeviceMouse.h>

class AGUI2MouseCursor : public Toshi::T2GUIRectangle
{
public:
	enum Acceleration : TUINT32
	{
		Acceleration_Off,
		Acceleration_Slow,
		Acceleration_Fast,
		Acceleration_NUMOF,
	};

public:
	AGUI2MouseCursor();

	// $Barnyard: FUNCTION 006463f0
	// $Barnyard: FUNCTION 00646630
	~AGUI2MouseCursor() {}

	TBOOL Create( const TCHAR* a_szPointerUpTexture, const TCHAR* a_szPointerDownTexture, TFLOAT a_fScalar );
	void  Update();

	void MoveCursor( TFLOAT a_fDeltaX, TFLOAT a_fDeltaY );

	// Keeps the cursor within a_fRadius of (a_fX, a_fY)
	// $Barnyard: FUNCTION 006465d0
	void SetRadialField( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fRadius )
	{
		m_RadialFieldPos.x = a_fX;
		m_RadialFieldPos.y = a_fY;
		m_fRadialRadius    = a_fRadius;
	}

	// $Barnyard: FUNCTION 00646600
	void ClearRadialField()
	{
		m_fRadialRadius  = -1.0f;
		m_RadialFieldPos = Toshi::TVector2::VEC_ZERO;
	}

	void                   SetMousePos( const Toshi::TVector2& a_rcPos ) { m_MousePos = a_rcPos; }
	const Toshi::TVector2& GetCursorCenter() const { return m_CursorCenter; }
	const Toshi::TVector2& GetCursorPos() const { return m_CursorPos; }

private:
	Toshi::TVector2             m_MousePos;
	Toshi::TVector2             m_CursorCenter;
	Toshi::TVector2             m_CursorPos;
	Toshi::TInputDeviceMouse*   m_pMouseDevice;
	Toshi::T2GUITextureSection* m_pPointerUpSection;
	Toshi::T2GUITextureSection* m_pPointerDownSection;
	TFLOAT                      m_fCursorAccelerations[ Acceleration_NUMOF ];
	Acceleration                m_eAccelerationType;
	TBOOL                       m_bIsMouseDown;
	Toshi::TVector2             m_RadialFieldPos;
	TFLOAT                      m_fRadialRadius;
};
