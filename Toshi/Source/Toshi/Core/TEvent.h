#pragma once

namespace Toshi
{
	class TGenericEmitter
	{
	public:
		TGenericEmitter(void* owner) { m_Unk1 = this; m_Unk2 = this; Create(owner); }

		void Create(void* owner) { m_Owner = owner; }
		//void Destroy() { }

	private:
		void* m_Unk1;
		void* m_Unk2;
		void* m_Owner;
	};

	class TGenericListener
	{
	public:
		//void Connect(TGenericEmitter* )
	};
}