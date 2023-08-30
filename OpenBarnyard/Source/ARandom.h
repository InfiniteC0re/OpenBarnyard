#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Math/TRandom.h"

class ARandom : Toshi::TObject, public Toshi::TSingleton<ARandom>
{
public:
	ARandom()
	{

	}

	Toshi::TRandom m_oRandom;
};