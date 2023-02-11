#pragma once
#include "TDList.h"

namespace Toshi
{
	class TGenericListener : public TPriList<TGenericListener>::TNode
	{
	public:
		using t_Callback = bool (*)(void*, void*, void*);
		
	public:
		TGenericListener() = default;

		void Execute(void* pOwner, void* pData)
		{
			m_pCallback(m_pCaller, pOwner, pData);
		}

	protected:
		//void Connect(TGenericEmitter* emitter, void* unk1, t_Func func, int unk2);

	private:
		void* m_pUnk;
		void* m_pCaller;
		t_Callback m_pCallback;
	};

	class TGenericEmitter
	{
	public:
		TGenericEmitter() = default;

		TGenericEmitter(void* owner) 
		{
			Create(owner); 
		}

		TGenericEmitter(const TGenericEmitter& cpy)
		{
			m_Listeners.m_Root = cpy.m_Listeners.m_Root;
			Create(cpy.m_Owner);
		}

		void Throw(void* pData)
		{
			for (auto it = m_Listeners.Begin(); it != m_Listeners.End(); it++)
			{
				it->Execute(m_Owner, pData);
			}
		}

		void Create(void* owner)
		{
			m_Owner = owner;
		}

		//void Destroy() { }

	private:
		TPriList<TGenericListener> m_Listeners;
		void* m_Owner;
	};
}