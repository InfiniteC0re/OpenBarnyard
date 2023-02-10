#pragma once

#include "TGenericDList.h"

namespace Toshi
{
	template <class T>
	class TGenericEmitter : public TPriList<T>::TNode
	{
	public:
		TGenericEmitter() = default;
		TGenericEmitter(void* owner) 
		{
			m_List.m_Root.m_Next = this;
			m_List.m_Root.m_Prev = this;
			Create(owner); 
		}

		TGenericEmitter(const TGenericEmitter& cpy)
		{
			m_List.m_Root.m_Next = cpy.m_List.m_Root.m_Next;
			m_List.m_Root.m_Prev = cpy.m_List.m_Root.m_Prev;
			Create(cpy.m_Owner);
		}

		void Throw(void* owner)
		{
			auto pTVar2 = m_List.m_Root.m_Next;
			do
			{
				if (pTVar2 == this) return;
				pTVar2 = m_List.m_Root.m_Next;
			} while (true);
		}
		void Create(void* owner) { m_Owner = owner; }
		//void Destroy() { }

		TGenericEmitter& operator=(const TGenericEmitter& cpy)
		{
			TGenericEmitter(cpy);
		}

	private:
		TPriList<T> m_List;
		void* m_Owner;
	};

	//class TGenericListener
	//{
	//public:

	//	using t_Func = bool (*)(void*, void*, void*);

	//protected:
	//	//void Connect(TGenericEmitter* emitter, void* unk1, t_Func func, int unk2);
	//};
}