#pragma once
#include <Toshi/T2Vector.h>

class MODAPI ADisplayModes_Win : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ADisplayModes_Win, Toshi::TObject );

public:
	~ADisplayModes_Win();
	struct Mode
	{
		TUINT32 m_uiWidth;
		TUINT32 m_uiHeight;
		TUINT32 m_uiColourDepth;
	};

public:
	// $Barnyard: FUNCTION 00614c40
	ADisplayModes_Win() {}

	void Initialise();

	TBOOL DoesModeExist( const TUINT32& a_rWidth, const TUINT32& a_rHeight, const TUINT32& a_rColourDepth );

	TINT        GetNumModes() const { return m_Modes.Size(); }
	const Mode& GetMode( TINT a_iIndex ) const { return m_Modes[ a_iIndex ]; }

private:
	Toshi::T2Vector<Mode, 1024> m_Modes;
};
