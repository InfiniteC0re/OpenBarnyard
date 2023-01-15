#pragma once

class AGameState :
	public Toshi::TGenericClassDerived<AGameState, Toshi::TObject, "AGameState", TMAKEVERSION(1, 0), true>
{
public:
	AGameState()
	{

	}

	virtual ~AGameState()
	{

	}

	bool HandleMenuRequestToChangeState(uint32_t menuType)
	{

	}

private:

};