#pragma once
#include "AWorld.h"

class AWorldVis
{
public:
	struct Model
	{
		TINT iIndex;
	};

public:
	AWorldVis();
	~AWorldVis();

	void Create(World* a_pWorld);

	void Reset();

public:
	World* m_pWorld;
	// ...
	Model* m_pModels;
	TINT m_iSomeNum1;
	TINT m_iSomeNum2;
	// ...
	void* m_fnSomeCallback;
};