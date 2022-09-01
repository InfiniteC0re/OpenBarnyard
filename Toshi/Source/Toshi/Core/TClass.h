#pragma once

namespace Toshi
{
	// TClassProps definitions
	typedef void* (*t_CreateTObject)();
	typedef void* (*t_CreateTObjectInPlace)(void*);
	typedef void  (*t_InitializeStatic)();
	typedef void  (*t_UninitializeStatic)();

	// RecurseTree definitions
	typedef bool  (*t_RecurceTreeCheck)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseBeginCb)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseEndCb)(class TClass*, void*);

	// This is a separate struct because we want to do compile-time initialization
	struct TClassProps
	{
		const char* m_Name = 0;                                 // 0x00
		t_CreateTObject m_Create = 0;                           // 0x04
		t_CreateTObjectInPlace m_CreateInPlace = 0;             // 0x08
		t_InitializeStatic m_Initialize = 0;                    // 0x0C
		t_UninitializeStatic m_Uninitialize = 0;                // 0x10
		TClassProps* m_Parent = 0;                              // 0x14
		TClassProps* m_Previous = 0;                            // 0x18
		TClassProps* m_LastAttached = 0;                        // 0x1C
		uint32_t m_Version = 0;                                 // 0x20
		size_t m_Size = 0;                                      // 0x24
		uint32_t m_Unk = 0;                                     // 0x28
		bool m_Initialized = 0;                                 // 0x2C
	};

	class TClass : public TClassProps
	{
	public:
		TClass(const char* name, TClassProps* parent, uint32_t version, size_t size, t_CreateTObject fCreate, t_CreateTObjectInPlace m_CreateInPlace, t_InitializeStatic fInit, t_UninitializeStatic fUninit);
		~TClass();

		void Initialize();
		void RecurseTree(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom);
		void RecurseTree2(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom);

		inline bool IsInitialized() { return m_Initialized; }
		inline const char* GetName() const { return m_Name; }
		inline uint32_t GetVersion() const { return m_Version; }
		inline uint16_t GetVersionMajor() const { return m_Version >> 16; }
		inline uint16_t GetVersionMinor() const { return m_Version & 0xFFFF; }
		inline TClassProps& GetPropsRef() { return *this; }
		inline TClassProps* GetProps() { return this; }

		// todo: move this function away from this class
		static bool TryInitialize(TClass* tClass);
	};

	// TClass should be equal to TClassProps
	static_assert(sizeof(TClass) == sizeof(TClassProps));

	// Helper cast function
	static inline TClass* TClassFromProps(TClassProps& props)
	{
		return static_cast<TClass*>(&props);
	}
}

