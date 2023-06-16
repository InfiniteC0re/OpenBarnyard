#include "ToshiPCH.h"

void Toshi::TGenericListener::Connect(TGenericEmitter* emitter, void* caller, t_Callback callback, int unk2)
{
	TASSERT(IsLinked() == TFALSE);
	m_pCaller = caller;
	m_pCallback = callback;
	m_Unk = unk2;
	emitter->m_Listeners.Insert(this);
}
