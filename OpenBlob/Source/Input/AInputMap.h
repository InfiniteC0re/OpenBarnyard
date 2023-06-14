#pragma once
#include "Toshi2/T2RedBlackTree.h"
#include "Toshi2/T2Map.h"
#include "Toshi2/T2Vector.h"

class AInputMap
{
public:
	enum INPUTCONTEXT
	{
		UNK,
		UNK2,
		UNK3,
		UNK4
	};

	enum ActionButton
	{

	};

	typedef Toshi::TComparator<int> ButtonMapComparator;

	AInputMap()
	{

	}

	Toshi::T2Map<ActionButton, Toshi::T2Vector<int, 4>, ButtonMapComparator> m_ActionButtonMap; // 0x0
};

