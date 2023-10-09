#pragma once
#include <Toshi2/T2Vector.h>

class ADisplayModes_Win : 
	public Toshi::TGenericClassDerived<ADisplayModes_Win, Toshi::TObject, "ADisplayModes_Win", TMAKEVERSION(1, 0), TTRUE>
{
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

	TBOOL IsModeExists(const TUINT32& a_rWidth, const TUINT32& a_rHeight, const TUINT32& a_rColourDepth);

private:
	Toshi::T2Vector<Mode, 1024> m_Modes;
};
