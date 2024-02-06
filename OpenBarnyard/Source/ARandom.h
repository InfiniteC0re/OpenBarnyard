#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Math/TRandom.h"

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