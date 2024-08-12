#include "ToshiPCH.h"
#include "T2TestingFramework.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// Using native malloc/free instead of TOSHI allocator so that
// we can be sure nothing goes wrong with the framework.

static void* Malloc( TSIZE a_uiSize )
{
    return malloc( a_uiSize );
}

static void Free( void* a_pMem )
{
    free( a_pMem );
}

template < class T, typename... Args >
static T* New( Args&&... args )
{
    return new ( malloc( sizeof( T ) ) ) T( std::forward( args )... );
}

T2TestingFramework* g_pTestingFramework;

T2TestingFramework::TestAutoReg::TestAutoReg( const TCHAR* a_pchTestName, const TCHAR* a_pchCategoryName, TestMethod_t a_fnMethod )
{
    if ( !g_pTestingFramework )
        g_pTestingFramework = New< T2TestingFramework >();

    g_pTestingFramework->RegisterTest(
        a_pchTestName,
        g_pTestingFramework->RegisterCategory( a_pchCategoryName ),
        a_fnMethod );
}

T2TestingFramework::Category* T2TestingFramework::FindCategory( const TCHAR* a_pchName )
{
    Category* pCategory = m_pHeadCategory;

    while ( pCategory )
    {
        if ( !strcmp( pCategory->pchName, a_pchName ) )
            return pCategory;

        pCategory = pCategory->pNext;
    }

    return TNULL;
}

T2TestingFramework::Category* T2TestingFramework::RegisterCategory( const TCHAR* a_pchName )
{
    Category* pCategory = FindCategory( a_pchName );

    if ( !pCategory )
    {
        pCategory          = New< Category >();
        pCategory->pchName = a_pchName;
        pCategory->pNext   = m_pHeadCategory;

        m_pHeadCategory = pCategory;
    }

    return pCategory;
}

T2TestingFramework::Test* T2TestingFramework::RegisterTest( const TCHAR* a_pchTestName, Category* a_pCategory, TestMethod_t a_fnMethod )
{
    if ( !a_pCategory )
        return TNULL;

    Test* pTest      = New< Test >();
    pTest->pchName   = a_pchTestName;
    pTest->pCategory = a_pCategory;
    pTest->fnMethod  = a_fnMethod;
    pTest->pNext     = a_pCategory->pHeadTest;

    a_pCategory->pHeadTest = pTest;

    return pTest;
}

T2TestingFramework::FailCallback_t    g_fnFailCallback;
T2TestingFramework::SuccessCallback_t g_fnSuccessCallback;

void T2TestingFramework::SignalRequirementResult( Test* a_pTest, const TCHAR* a_pchFileName, TINT a_iLineNum, const TCHAR* a_pchStatement, TBOOL a_bFailed )
{
    if ( !a_pTest )
        return;

    Check* pCheck        = New< Check >();
    pCheck->pchFileName  = a_pchFileName;
    pCheck->pchStatement = a_pchStatement;
    pCheck->iLineNum     = a_iLineNum;
    pCheck->bFailed      = a_bFailed;
    pCheck->pNext        = a_pTest->pHeadCheck;

    a_pTest->pHeadCheck = pCheck;

    if ( a_bFailed )
    {
        a_pTest->iNumFails += 1;

        if ( g_fnFailCallback )
            g_fnFailCallback( a_pTest, pCheck );
    }
    else if ( g_fnSuccessCallback )
    {
        g_fnSuccessCallback( a_pTest, pCheck );
    }
}

TINT T2TestingFramework::RunTests( FailCallback_t a_fnFailCallback /* = TNULL*/, SuccessCallback_t a_fnSuccessCallback /* = TNULL */ )
{
    TINT      iNumFails = 0;
    Category* pCategory = m_pHeadCategory;

    // Update callbacks
    g_fnFailCallback    = a_fnFailCallback;
    g_fnSuccessCallback = a_fnSuccessCallback;

    while ( pCategory )
    {
        Test* pTest = pCategory->pHeadTest;

        while ( pTest )
        {
            if ( !pTest->bWasExecuted )
            {
                // Run the actual test
                m_pCurrentTest = pTest;

                pTest->fnMethod();
                pTest->bWasExecuted = TTRUE;

                // Count number of fails
                iNumFails += pTest->iNumFails;
                pCategory->iNumTotalFails += pTest->iNumFails;
            }

            m_pCurrentTest = TNULL;
            pTest          = pTest->pNext;
        }

        pCategory = pCategory->pNext;
    }

    // Reset callbacks
    g_fnFailCallback    = TNULL;
    g_fnSuccessCallback = TNULL;

    return iNumFails;
}

TOSHI_NAMESPACE_END
