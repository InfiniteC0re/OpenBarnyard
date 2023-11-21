#pragma once
#include "TDList.h"

namespace Toshi {

	class TGenericEmitter;

	class TGenericListener : public TPriList<TGenericListener>::TNode
	{
	public:
		using t_Callback = TBOOL (*)(void* a_pCaller, void* a_pOwner, void* a_pData);
		friend class TGenericEmitter;

	public:
		TGenericListener() = default;
		~TGenericListener()
		{
			Disconnect();
		}

		void Connect(TGenericEmitter& a_rEmitter, void* a_pCaller, t_Callback a_pCallback, int a_iPriority);
		void Disconnect();

	private:
		void Execute(void* pOwner, void* pData)
		{
			m_pCallback(m_pCaller, pOwner, pData);
		}

	private:
		void* m_pCaller = TNULL;
		t_Callback m_pCallback = TNULL;
	};

	class TGenericEmitter
	{
	public:
		TGenericEmitter()
		{
			Create(TNULL);
		}

		TGenericEmitter(void* a_pOwner)
		{
			Create(a_pOwner);
		}

		void Throw(void* a_pData)
		{
			auto pListener = m_Listeners.Begin();

			while (pListener != m_Listeners.End())
			{
				auto pNextListener = pListener->Next();
				pListener->Execute(m_pOwner, a_pData);
				pListener = pNextListener;
			}
		}

		void Create(void* a_pOwner)
		{
			m_pOwner = a_pOwner;
		}

		void Destroy()
		{
			for (auto it = m_Listeners.Begin(); it != m_Listeners.End(); it++)
			{
				//it->Disconnect();
			}

			m_pOwner = TNULL;
		}

		TPriList<TGenericListener>& GetListeners() { return m_Listeners; }
		const TPriList<TGenericListener>& GetListeners() const { return m_Listeners; }

	public:
		TPriList<TGenericListener> m_Listeners;
		void* m_pOwner;
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

	template <typename Owner, typename Data, typename Caller>
	class TListener : public TGenericListener
	{
	public:
		using t_CallerCallback = TBOOL(*)(Caller* a_pCaller, Owner* a_pOwner, Data* a_pData);

	public:
		void Connect(TGenericEmitter& a_rEmitter, Caller* a_pCaller, t_CallerCallback a_pCallback, int a_iPriority)
		{
			TGenericListener::Connect(
				a_rEmitter,
				a_pCaller,
				TREINTERPRETCAST(t_Callback, a_pCallback),
				a_iPriority
			);
		}
	};
}
