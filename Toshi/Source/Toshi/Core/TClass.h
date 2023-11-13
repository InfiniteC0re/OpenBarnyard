#pragma once

#define TGetClass(X) X::s_Class

namespace Toshi
{
	// TClassProps definitions
	typedef class TObject* (*t_CreateTObject)();
	typedef class TObject* (*t_CreateTObjectInPlace)(void*);
	typedef void  (*t_InitializeStatic)();
	typedef void  (*t_UninitializeStatic)();

	// RecurseTree definitions
	typedef TBOOL  (*t_RecurceTreeCheck)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseBeginCb)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseEndCb)(class TClass*, void*);

	class TClass
	{
	public:
		// FIXME: Make correct order of arguments
		TClass(const char* name, TClass* parent, TUINT32 version, TUINT32 size, TUINT32 alignment, t_CreateTObject fCreate, t_CreateTObjectInPlace m_CreateInPlace, t_InitializeStatic fInit, t_UninitializeStatic fUninit);
		~TClass();

		constexpr TClass(const char* name, TUINT32 version, t_CreateTObject fCreate, t_CreateTObjectInPlace fCreateInPlace, TUINT32 size)
		{
			m_Name = name;
			m_Create = fCreate;
			m_CreateInPlace = fCreateInPlace;
			m_Initialize = TNULL;
			m_Uninitialize = TNULL;
			m_Parent = TNULL;
			m_Previous = TNULL;
			m_LastAttached = TNULL;
			m_Version = 0;
			m_Size = size;
			m_Alignment = 0;
			m_Initialized = TFALSE;
		}

		void Initialize();
		void RecurseTree(t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, t_RecurceTreeCheck fCheck, void* custom);
		void RecurseTree2(t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, t_RecurceTreeCheck fCheck, void* custom);
		class TObject* CreateObject() const;

		TBOOL IsA(TClass* pClass);
		TBOOL IsExactly(TClass* toCompare) const { return this == toCompare; }
		TBOOL IsInitialized() const { return m_Initialized; }
		TClass* GetParent() { return static_cast<TClass*>(m_Parent); }
		const char* GetName() const { return m_Name; }
		TUINT32 GetVersion() const { return m_Version; }
		TUINT16 GetVersionMajor() const { return m_Version >> 16; }
		TUINT16 GetVersionMinor() const { return m_Version & 0xFFFF; }
		TUINT32 GetMaxSizeOfDerivedClasses();
		TUINT32 GetMaxAlignmentOfDerivedClasses();

		// todo: move this function away from this class
		static TBOOL TryInitialize(TClass* tClass);
		
		// Looks for a class in parent
		static TClass* FindRecurse(const char* const& name, TClass* parent, TBOOL hasPrevious);
		static TClass* Find(const char* name, TClass* parent = TNULL);

	public:
		inline TBOOL operator==(const TClass* other) const { return this == other; }
	
	private:
		const char* m_Name = 0;                                 // 0x00
		t_CreateTObject m_Create = 0;                           // 0x04
		t_CreateTObjectInPlace m_CreateInPlace = 0;             // 0x08
		t_InitializeStatic m_Initialize = 0;                    // 0x0C
		t_UninitializeStatic m_Uninitialize = 0;                // 0x10
		TClass* m_Parent = 0;                                   // 0x14
		TClass* m_Previous = 0;                                 // 0x18
		TClass* m_LastAttached;                                 // 0x1C
		TUINT32 m_Version = 0;                                  // 0x20
		TUINT32 m_Size = 0;                                     // 0x24
		TUINT32 m_Alignment = 0;                                // 0x28
		TBOOL m_Initialized = 0;                                // 0x2C
	};
}
