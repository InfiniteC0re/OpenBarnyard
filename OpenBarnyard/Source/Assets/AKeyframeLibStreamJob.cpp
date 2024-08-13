#include "pch.h"
#include "AKeyframeLibStreamJob.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AKeyframeLibStreamJob::AKeyframeLibStreamJob() :
	TFileStreamJob( TNULL )
{
	m_pLibrary	  = TNULL;
	m_szName[ 0 ] = '\0';
}

void AKeyframeLibStreamJob::Process()
{
	m_pLibrary	  = TNULL;
	auto pLibrary = TRenderInterface::GetSingleton()->GetKeyframeLibraryManager().LoadLibrary( m_szName );
	m_szName[ 0 ] = '\0';
	m_pLibrary	  = pLibrary;
}

void AKeyframeLibStreamJob::Init( const TCHAR* a_szFileName )
{
	TStringManager::String8Copy(
		m_szName,
		a_szFileName,
		TMath::Min( TStringManager::String8Length( a_szFileName ), sizeof( m_szName ) ) );
}
