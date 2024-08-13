#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Math/TRandom.h>

class ARandom :
	public Toshi::TObject,
	public Toshi::TSingleton<ARandom>
{
public:
	TDECLARE_CLASS( ARandom, Toshi::TObject );

public:
	ARandom()
	{
	}

	// TODO: Add interface for TRandom

	Toshi::TRandom m_oRandom;
};
