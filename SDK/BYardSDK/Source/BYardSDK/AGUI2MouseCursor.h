#pragma once
#include "AGUI2Rectangle.h"
#include "AGUI2TextureSection.h"

#include <Input/TInputDeviceMouse.h>

class AGUI2MouseCursor : public AGUI2Rectangle
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

	TBOOL Create( const char* a_szPointerUpTexture, const char* a_szPointerDownTexture, TFLOAT a_fScalar );
	void  Update();

	void MoveCursor( TFLOAT a_fDeltaX, TFLOAT a_fDeltaY );

public:
	Toshi::TVector2           m_MousePos;
	Toshi::TVector2           m_CursorCenter;
	Toshi::TVector2           m_CursorPos;
	Toshi::TInputDeviceMouse* m_pMouseDevice;
	AGUI2TextureSection*      m_pPointerUpSection;
	AGUI2TextureSection*      m_pPointerDownSection;
	TFLOAT                    m_fCursorAccelerations[ Acceleration_NUMOF ];
	Acceleration              m_eAccelerationType;
	TBOOL                     m_bIsMouseDown;
	Toshi::TVector2           m_RadialFieldPos;
	TFLOAT                    m_fRadialRadius;
};
