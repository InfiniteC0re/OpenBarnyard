#pragma once

namespace Toshi
{
	// TClassProps definitions
	typedef class TObject* (*t_CreateTObject)();
	typedef class TObject* (*t_CreateTObjectInPlace)(void*);
	typedef void  (*t_InitializeStatic)();
	typedef void  (*t_UninitializeStatic)();

	// RecurseTree definitions
	typedef bool  (*t_RecurceTreeCheck)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseBeginCb)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseEndCb)(class TClass*, void*);

	class TClass
	{
	public:
		TClass(const char* name, TClass* parent, uint32_t version, size_t size, t_CreateTObject fCreate, t_CreateTObjectInPlace m_CreateInPlace, t_InitializeStatic fInit, t_UninitializeStatic fUninit);
		~TClass();

		constexpr TClass(const char* name, uint32_t version, t_CreateTObject fCreate, t_CreateTObjectInPlace fCreateInPlace, uint32_t size)
		{
			m_Name = name;
			m_Create = fCreate;
			m_CreateInPlace = fCreateInPlace;
			m_Initialize = nullptr;
			m_Uninitialize = nullptr;
			m_Parent = nullptr;
			m_Previous = nullptr;
			m_LastAttached = nullptr;
			m_Version = 0;
			m_Size = size;
			m_Unk = 0;
			m_Initialized = false;
		}

		void Initialize();
		void RecurseTree(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom);
		void RecurseTree2(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom);
		class TObject* CreateObject();

		inline bool IsExactly(TClass* toCompare) const { return this == toCompare; }
		inline bool IsInitialized() const { return m_Initialized; }
		inline TClass* GetParent() { return static_cast<TClass*>(m_Parent); }
		inline const char* GetName() const { return m_Name; }
		inline uint32_t GetVersion() const { return m_Version; }
		inline uint16_t GetVersionMajor() const { return m_Version >> 16; }
		inline uint16_t GetVersionMinor() const { return m_Version & 0xFFFF; }

		// todo: move this function away from this class
		static bool TryInitialize(TClass* tClass);
		
		// Looks for a class in parent
		static TClass* FindRecurse(const char* const& name, TClass* parent, bool hasPrevious);

	public:
		inline bool operator==(const TClass* other) const { return this == other; }
	
	private:
		const char* m_Name = 0;                                 // 0x00
		t_CreateTObject m_Create = 0;                           // 0x04
		t_CreateTObjectInPlace m_CreateInPlace = 0;             // 0x08
		t_InitializeStatic m_Initialize = 0;                    // 0x0C
		t_UninitializeStatic m_Uninitialize = 0;                // 0x10
		TClass* m_Parent = 0;                                   // 0x14
		TClass* m_Previous = 0;                                 // 0x18
		TClass* m_LastAttached;                                 // 0x1C
		uint32_t m_Version = 0;                                 // 0x20
		size_t m_Size = 0;                                      // 0x24
		uint32_t m_Unk = 0;                                     // 0x28
		bool m_Initialized = 0;                                 // 0x2C
	};
}
