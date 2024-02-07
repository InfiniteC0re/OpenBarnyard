#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Math/TRandom.h>

class ARandom :
	public Toshi::TObject,
	public Toshi::TSingleton<ARandom>
{
public:
	TDECLARE_CLASS(Toshi::TObject);

public:
	ARandom()
	{
		
	}

	Toshi::TRandom m_oRandom;
};