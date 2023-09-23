#pragma once

#define TGetClass(X) X::GetClassStatic()

namespace Toshi
{
	// TClassProps definitions
	typedef class TObject* (*t_CreateTObject)();
	typedef class TObject* (*t_CreateTObjectInPlace)(void*);
	typedef void  (*t_InitializeStatic)();
	typedef void  (*t_UninitializeStatic)();

	// RecurseTree definitions
	typedef TBOOL (*t_RecurceTreeCheck)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseBeginCb)(class TClass*, void*);
	typedef void  (*t_RecurceTreeBaseEndCb)(class TClass*, void*);

	class TClass
	{
	public:
		TClass(const char* a_szName, TClass* a_pParent, TUINT32 a_uiVersion, TUINT32 a_uiSize, t_CreateTObject a_fCreate, t_CreateTObjectInPlace a_fCreateInPlace, t_InitializeStatic a_fInit, t_UninitializeStatic a_fUninit, TUINT32 a_uiAlignment);

		constexpr TClass(const char* a_szName, uint32_t a_uiVersion, t_CreateTObject a_fCreate, t_CreateTObjectInPlace a_fCreateInPlace, uint32_t a_uiSize, TUINT32 a_uiAlignment) :
			m_Name(a_szName),
			m_Create(a_fCreate),
			m_CreateInPlace(a_fCreateInPlace),
			m_Initialize(TNULL),
			m_Uninitialize(TNULL),
			m_Parent(TNULL),
			m_Previous(TNULL),
			m_LastAttached(TNULL),
			m_Version(a_uiVersion),
			m_Size(a_uiSize),
			m_Alignment(a_uiAlignment),
			m_Initialized(TFALSE)
		{ }

		~TClass();

		void Initialize();
		void RecurseTree(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom);
		void RecurseTree2(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom);
		class TObject* CreateObject();

		TBOOL IsA(TClass* pClass);
		TBOOL IsExactly(TClass* toCompare) const { return this == toCompare; }
		TBOOL IsInitialized() const { return m_Initialized; }
		TClass* GetParent() { return static_cast<TClass*>(m_Parent); }
		const char* GetName() const { return m_Name; }
		TUINT32 GetAlignment() const { return m_Alignment; }
		TUINT32 GetSize() const { return m_Size; }
		TUINT32 GetVersion() const { return m_Version; }
		TUINT16 GetVersionMajor() const { return m_Version >> 16; }
		TUINT16 GetVersionMinor() const { return m_Version & 0xFFFF; }

		// todo: move this function away from this class
		static TBOOL TryInitialize(TClass* tClass);
		
		// Looks for a class in parent
		static const TClass* FindRecurse(const char* const& name, const TClass* parent, TBOOL hasPrevious);
		static const TClass* Find(const char* name, const TClass* parent);

	public:
		inline TBOOL operator==(const TClass* other) const { return this == other; }
	
	protected:
		const char* m_Name = 0;                     // 0x00
		t_CreateTObject m_Create = 0;               // 0x04
		t_CreateTObjectInPlace m_CreateInPlace = 0; // 0x08
		t_InitializeStatic m_Initialize = 0;        // 0x0C
		t_UninitializeStatic m_Uninitialize = 0;    // 0x10
		TClass* m_Parent = 0;                       // 0x14
		TClass* m_Previous = 0;                     // 0x18
		TClass* m_LastAttached;                     // 0x1C
		TUINT32 m_Version = 0;                      // 0x20
		TUINT32 m_Size = 0;                         // 0x24
		TUINT32 m_Alignment = 0;                    // 0x28
		TBOOL m_Initialized = 0;                    // 0x2C
	};
}
