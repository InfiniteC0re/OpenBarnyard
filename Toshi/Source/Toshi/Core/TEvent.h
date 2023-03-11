#pragma once
#include "TDList.h"

namespace Toshi
{

	class TGenericListener;

	class TGenericEmitter
	{
	public:
		TGenericEmitter()
		{
			m_Listeners = TPriList<TGenericListener>();
			Create(TNULL);
		}

		TGenericEmitter(void* owner)
		{
			m_Listeners = TPriList<TGenericListener>();
			Create(owner);
		}

		TGenericEmitter(const TGenericEmitter& cpy)
		{
			m_Listeners.m_Root = cpy.m_Listeners.m_Root;
			Create(cpy.m_Owner);
		}

		void Throw(void* pData)
		{
			/*for (auto it = m_Listeners.Begin(); it != m_Listeners.End(); it++)
			{
				it->Execute(m_Owner, pData);
			}*/
		}

		void Create(void* owner)
		{
			m_Owner = owner;
		}

		//void Destroy() { }

	public:
		TPriList<TGenericListener> m_Listeners;
		void* m_Owner;
	};

	class TGenericListener : public TPriList<TGenericListener>::TNode
	{
	public:
		using t_Callback = bool (*)(void*, void*, void*);
		
	public:
		TGenericListener() : TNode()
		{
			m_pCaller = TNULL;
			m_pCallback = TNULL;
			m_Unk = 0;
		}

		void Execute(void* pOwner, void* pData)
		{
			m_pCallback(m_pCaller, pOwner, pData);
		}

	protected:
		void Connect(TGenericEmitter* emitter, void* caller, t_Callback callback, int unk2)
		{
			TASSERT(IsLinked() == TFALSE);
			m_pCaller = caller;
			m_pCallback = callback;
			m_Unk = unk2;
			emitter->m_Listeners.Insert(this);
		}

		void Disconnect()
		{
			m_Prev->m_Next = m_Next;
			m_Next->m_Prev = m_Prev;
			m_Next = this;
			m_Prev = this;
			m_pCaller = TNULL;
			m_pCallback = TNULL;
		}

	private:
		int m_Unk;
		void* m_pCaller;
		t_Callback m_pCallback;
	};



	template <class T, class U = void*>
	class TEmitter : public TGenericEmitter
	{
	public:
		TEmitter() = default;
		TEmitter(T* owner) : TGenericEmitter(owner) {}
	};
}