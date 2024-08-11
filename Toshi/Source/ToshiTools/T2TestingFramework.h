#pragma once

#define T2_TEST1( TEST_NAME, CATEGORY_NAME, TEST_ID ) \
	namespace JustMakingSureTheMacroIsCalledOutsideOfAnyMethod { }\
	void CONCATTOKEN( s_T2TestingFrameworkTest_, TEST_ID )();\
	static Toshi::T2TestingFramework::TestAutoReg CONCATTOKEN( s_T2TestingFrameworkAutoReg_, TEST_ID ) ( TEST_NAME, CATEGORY_NAME, CONCATTOKEN( s_T2TestingFrameworkTest_, TEST_ID ) );\
	void CONCATTOKEN( s_T2TestingFrameworkTest_, TEST_ID )()

#define T2_TEST( TEST_NAME, CATEGORY_NAME ) T2_TEST1( TEST_NAME, CATEGORY_NAME, __COUNTER__ )

#define T2_TEST_REQUIRE( STATEMENT )\
	{\
		volatile TBOOL __bCheckResult = ( STATEMENT );\
		g_pTestingFramework->SignalRequirementResult( g_pTestingFramework->GetCurrentTest(), __FILE__, __LINE__, #STATEMENT, !__bCheckResult );\
	}

TOSHI_NAMESPACE_START

class TOSHI_API T2TestingFramework
{
public:
	using TestMethod_t = void( * )( );

	class TestAutoReg
	{
	public:
		TestAutoReg( const TCHAR* a_pchTestName, const TCHAR* a_pchCategoryName, TestMethod_t a_fnMethod );
	};

	struct Test;

	struct Category
	{
		Category* pNext = TNULL;
		const TCHAR* pchName = TNULL;
		Test* pHeadTest = TNULL;
		TINT iNumTotalFails = 0;
	};

	struct Check
	{
		Check* pNext = TNULL;
		const TCHAR* pchFileName = TNULL;
		const TCHAR* pchStatement = TNULL;
		TINT iLineNum : 31 = -1;
		TBOOL bFailed : 1 = TFALSE;
	};

	struct Test
	{
		Test* pNext = TNULL;
		const TCHAR* pchName = TNULL;
		Category* pCategory = TNULL;
		TestMethod_t fnMethod = TNULL;
		TINT bWasExecuted : 1 = TFALSE;
		TINT iNumFails : 31 = 0;
		Check* pHeadCheck = TNULL;
	};

	using FailCallback_t = void( * )( Test* a_pTest, Check* a_pCheck );
	using SuccessCallback_t = void( * )( Test* a_pTest, Check* a_pCheck );

public:
	// constructors/destructor
	T2TestingFramework() = default;
	~T2TestingFramework() = default;

	// Returns pointer to a category if it exists or TNULL if it doesn't
	Category* FindCategory( const TCHAR* a_pchName );

	// Registers a category if it doesn't exist and returns it
	Category* RegisterCategory( const TCHAR* a_pchName );

	// Registers a test in a category and returns it
	Test* RegisterTest( const TCHAR* a_pchTestName, Category* a_pCategory, TestMethod_t a_fnMethod );

	// Runs all incomplete tests and returns total number of fails
	TINT RunTests( FailCallback_t a_fnFailCallback = TNULL, SuccessCallback_t a_fnSuccessCallback = TNULL );

	// Saves info about results of a T2_TEST_REQUIRE call
	void SignalRequirementResult( Test* a_pTest, const TCHAR* a_pchFileName, TINT a_iLineNum, const TCHAR* a_pchStatement, TBOOL a_bFailed );

	// Returns the currently running test or TNULL if no test is run
	Test* GetCurrentTest() const { return m_pCurrentTest; }

private:
	Category* m_pHeadCategory = TNULL;
	Test* m_pCurrentTest = TNULL;
};

extern T2TestingFramework* g_pTestingFramework;

TOSHI_NAMESPACE_END
