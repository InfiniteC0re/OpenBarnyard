#pragma once
#include <Toshi/TArray.h>
#include <Toshi/T2SortedVector.h>
#include <Toshi/TClass.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TObject.h>
#include <Toshi/TPString8.h>

class AFunction
{
public:
	using Callback_t = void ( * )();

public:
	virtual ~AFunction();
	virtual AFunction* Clone() const = 0;

	const Toshi::TPString8* GetName() const { return m_pNameLower; }
	Callback_t              GetCallback() const { return m_fnCallback; }
	void*                   GetUserData() const { return m_pUserData; }

protected:
	Toshi::TPString8* m_pNameLower; // 0x04
	Callback_t        m_fnCallback; // 0x08
	void*             m_pUserData;  // 0x0C
};

class AFunctionTable
    : public Toshi::TObject
    , public Toshi::TNodeList<AFunctionTable>::TNode
{
public:
	struct AFunctionSortResult
	{
		TINT operator()( AFunction* const& a_rLeft, AFunction* const& a_rRight ) const
		{
			const Toshi::TPooledString8* pLeft  = a_rLeft->GetName()->GetPooledString();
			const Toshi::TPooledString8* pRight = a_rRight->GetName()->GetPooledString();

			if ( pRight < pLeft )
				return 1;

			return -( pLeft < pRight );
		}
	};

	using FunctionContainer = Toshi::TArray<AFunction*>;
	using FunctionList      = Toshi::T2SortedVector<AFunction*, FunctionContainer, AFunctionSortResult>;

public:
	AFunctionTable( Toshi::TClass* a_pClass, AFunctionTable* a_pParentTable );
	~AFunctionTable() override;

	void Register( void* a_fnFallback );
	void Unregister();
	void AddFunction( AFunction* a_pFunction );

	Toshi::TClass*          GetFunctionClass() const { return m_pClass; }
	AFunctionTable*         GetParentTable() const { return m_pParentTable; }
	const Toshi::TPString8& GetClassNameLower() const { return m_strClassNameLower; }
	FunctionList&           GetFunctions() { return m_aFunctions; }
	const FunctionList&     GetFunctions() const { return m_aFunctions; }
	void*                   GetFallback() const { return m_fnFallback; }

	static Toshi::TNodeList<AFunctionTable>& GetRegisteredTables();

private:
	Toshi::TClass*   m_pClass;            // 0x10
	AFunctionTable*  m_pParentTable;      // 0x14
	Toshi::TPString8 m_strClassNameLower; // 0x18
	FunctionList     m_aFunctions;        // 0x1C
	void*            m_fnFallback;        // 0x2C
};
