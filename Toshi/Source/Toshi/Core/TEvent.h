#pragma once
#include "TDList.h"

namespace Toshi
{

	class TGenericEmitter;

	class TGenericListener : public TPriList<TGenericListener>::TNode
	{
	public:
		using t_Callback = TBOOL (*)(void*, void*, void*);
		friend class TGenericEmitter;

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

		void Connect(TGenericEmitter* emitter, void* caller, t_Callback callback, int unk2);

		void Disconnect()
		{
			Remove();
			m_pCaller = TNULL;
			m_pCallback = TNULL;
		}

	private:
		int m_Unk;
		void* m_pCaller;
		t_Callback m_pCallback;
	};

	class TGenericEmitter
	{
	public:
		TGenericEmitter()
		{
			//m_Listeners = TPriList<TGenericListener>();
			Create(TNULL);
		}

		TGenericEmitter(void* owner)
		{
			//m_Listeners = TPriList<TGenericListener>();
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

		void Destroy()
		{
			for (auto it = m_Listeners.Begin(); it != m_Listeners.End(); it++)
			{
				//it->Disconnect();
			}
			m_Owner = TNULL;
		}

	public:
		TPriList<TGenericListener> m_Listeners;
		void* m_Owner;
	};

	template <typename Owner, typename Data = Owner>
	class TEmitter : public TGenericEmitter
	{
	public:
		TEmitter() = default;
		TEmitter(Owner* owner) : TGenericEmitter(owner) {}

        void Throw(Data* pData) { TGenericEmitter::Throw(pData); }
        void Throw(Data&& data) { TGenericEmitter::Throw(&data); }

		void Create(Owner* owner) { TGenericEmitter::Create(owner); }

		void Destroy() { TGenericEmitter::Destroy(); }
	};
}