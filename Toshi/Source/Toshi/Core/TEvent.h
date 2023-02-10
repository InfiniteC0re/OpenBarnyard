#pragma once

#include "TGenericDList.h"

namespace Toshi
{

	class TGenericListener
	{
	public:

		using t_Func = bool (*)(void*, void*, void*);
		void* m_pUnk;
		void* m_pCaller;
		t_Func* m_pCallback;

	protected:
		//void Connect(TGenericEmitter* emitter, void* unk1, t_Func func, int unk2);
	};

	template <class T>
	class TGenericEmitter : public TPriList<T>::TNode
	{
	public:
		TGenericEmitter() = default;
		TGenericEmitter(void* owner) 
		{
			m_Listeners.m_Root.m_Next = this;
			m_Listeners.m_Root.m_Prev = this;
			Create(owner); 
		}

		TGenericEmitter(const TGenericEmitter& cpy)
		{
			m_Listeners.m_Root.m_Next = cpy.m_Listeners.m_Root.m_Next;
			m_Listeners.m_Root.m_Prev = cpy.m_Listeners.m_Root.m_Prev;
			Create(cpy.m_Owner);
		}

		void Throw(void* owner)
		{
			auto curListener = m_Listeners.m_Root.m_Next->As<TGenericListener>();
			do
			{
				if (curListener == this) return;
				curListener->m_pCallback(curListener->m_pCaller, m_Owner, owner);
				curListener = m_Listeners.m_Root.m_Next->As<TGenericListener>();
			} while (true);
		}
		void Create(void* owner) { m_Owner = owner; }
		//void Destroy() { }

		TGenericEmitter& operator=(const TGenericEmitter& cpy)
		{
			TGenericEmitter(cpy);
		}

	private:
		TPriList<TGenericListener> m_Listeners;
		void* m_Owner;
	};


}