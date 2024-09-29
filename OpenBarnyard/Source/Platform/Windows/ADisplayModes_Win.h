#pragma once
#include <Toshi/T2Vector.h>

class ADisplayModes_Win : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ADisplayModes_Win, Toshi::TObject );

public:
	struct Mode
	{
		TUINT32 m_uiWidth;
		TUINT32 m_uiHeight;
		TUINT32 m_uiColourDepth;
	};

public:
	ADisplayModes_Win() = default;

	void Initialise();

	TBOOL IsModeExists( const TUINT32& a_rWidth, const TUINT32& a_rHeight, const TUINT32& a_rColourDepth );

private:
	Toshi::T2Vector<Mode, 1024> m_Modes;
};
