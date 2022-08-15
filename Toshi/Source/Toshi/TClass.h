#pragma once
#include <cstdint>

namespace Toshi
{
	typedef void* (*t_CreateTObject)();
	typedef void* (*t_CreateTObjectAtPlace)();
	typedef void  (*t_RegisterScriptingAPI)();

	class TClass
	{
	public:
		const char* m_pcClassName; //0
		t_CreateTObject m_pCreateTObject; //0x4
		t_CreateTObjectAtPlace m_pCreateTObjectAtPlace; //0x8
		t_RegisterScriptingAPI m_pRegisterScriptingAPI; //0xC
		void* m_pFunc; //0x10

		TClass* m_parent; //0x14
		TClass* m_attached; //0x18
		TClass* m_tclass; //0x1C
		size_t m_size; //0x20
		uint32_t m_version; // 0x24
		void* m_unk4; // 0x28
		bool m_bInitialised; // 0x2C

		TClass(const char* name, TClass* parent, t_CreateTObject create, t_CreateTObjectAtPlace createAtPlace, t_RegisterScriptingAPI scripting, uint16_t verMajor, uint16_t verMinor, size_t size);

		inline const char* GetName() const { return m_pcClassName; }
		bool AttachClassToParent();
		bool IsAttached() const;
	};
}

