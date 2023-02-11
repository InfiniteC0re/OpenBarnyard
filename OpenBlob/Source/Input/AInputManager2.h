#pragma once
#include "Input/AInputMap.h"

#include <Toshi/Core/TStack.h>

class AInputManager2 : public Toshi::TSingleton<AInputManager2>
{
public:

	Toshi::TStack<AInputMap::INPUTCONTEXT, 10> m_inputContextStack; // 0x1A0

	void SetContext(AInputMap::INPUTCONTEXT a_eInputContext)
	{
		m_inputContextStack.Push(a_eInputContext);
	}

	AInputManager2();
	~AInputManager2();
};