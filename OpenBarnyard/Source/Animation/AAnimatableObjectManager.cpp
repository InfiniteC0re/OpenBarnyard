#include "pch.h"
#include "AAnimatableObjectManager.h"
#include "AAnimatableObjectTypeBunch.h"
#include "AModelRepos.h"
#include "ALoadScreen.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AAnimatableObjectManager );

Toshi::T2ObjectPool<AToshiAnimationRef, 100> g_oAnimationRefPool;

static constexpr const TCHAR* MODELTYPES_DIR = "data\\modeltypes\\";

// $Barnyard: FUNCTION 0057d5f0
// $Barnyard: FUNCTION 0057d560
AAnimatableObjectManager::AAnimatableObjectManager()
    : m_pAnimSoundBPsTRB( TNULL )
    , m_pSoundBreakpoints( TNULL )
{
}

// $Barnyard: FUNCTION 0057db80
// $Barnyard: FUNCTION 0057d680
AAnimatableObjectManager::~AAnimatableObjectManager()
{
	if ( m_pAnimSoundBPsTRB )
	{
		delete m_pAnimSoundBPsTRB;
		m_pAnimSoundBPsTRB  = TNULL;
		m_pSoundBreakpoints = TNULL;
	}

	m_llObjectTypes.DeleteAll();
	m_llAnimatableObjects.DeleteAll();
}

// $Barnyard: FUNCTION 0057d710
AAnimatableObject* AAnimatableObjectManager::CreateAnimatableObject( AAnimatableObjectType* a_pObjectType, void* a_Unk1, TUINT a_eFlags )
{
	AAnimatableObject* pObject = new AAnimatableObject();

	m_llAnimatableObjects.PushBack( pObject );
	pObject->Create( a_pObjectType, a_Unk1, a_eFlags );

	return pObject;
}

// $Barnyard: FUNCTION 0057e8c0
void AAnimatableObjectManager::LoadTypesFromLibrary( const TCHAR* a_szLibName )
{
	TString8 strLibPath;
	strLibPath.Format( "%slibs\\%s.trb", MODELTYPES_DIR, a_szLibName );

	TTRB   oTRB;
	TUINT8 eLoadRes = oTRB.Load( strLibPath );
	TASSERT( eLoadRes == TTRB::ERROR_OK );

	if ( eLoadRes == TTRB::ERROR_OK )
	{
		const PBProperties* pProperties = PBProperties::LoadFromTRB( oTRB );
		oTRB.DeleteSymbolTable();

		// Skip file if the first type's name is starting with _ (why?)
		if ( pProperties->Begin()->GetName().GetString()[ 0 ] != '_' )
		{
			TINT iLoadScreenUpdateCounter = 0;

			T2_FOREACH( ( *pProperties ), it )
			{
				LoadAnimObjType( it->GetName().GetString(), it->GetValue()->GetProperties(), TFALSE );

				// Update load screen only each N type
				if ( ++iLoadScreenUpdateCounter > 5 )
				{
					iLoadScreenUpdateCounter = 0;
					g_oLoadScreen.Update();
				}
			}
		}

		oTRB.Close();
	}

	g_oLoadScreen.Update();
}

// $Barnyard: FUNCTION 0057dba0
void AAnimatableObjectManager::UnloadTypesFromProperties( const PBPropertyValue* a_pArray )
{
	auto pArray = a_pArray->GetArray();

	for ( TUINT i = 0; i < pArray->GetSize(); i++ )
	{
		DestroyType( TPString8( pArray->GetValue( i )->GetString() ) );
	}
}

// $Barnyard: FUNCTION 0057e550
void AAnimatableObjectManager::LoadAnimObjType( const TPString8& a_rcName, const PBProperties* a_pProperties, TBOOL a_bNoBreakpoints )
{
	TString8 strAnimObjPath;
	strAnimObjPath.Format( "%s%s.trb", MODELTYPES_DIR, a_rcName.GetString() );

	TTRB oTRB;

	// If no properties is specified, load from file
	if ( !a_pProperties && oTRB.Load( strAnimObjPath ) == TTRB::ERROR_OK )
	{
		a_pProperties = PBProperties::LoadFromTRB( oTRB );
		oTRB.DeleteSymbolTable();
	}

	TVALIDPTR( a_pProperties );
	const PBProperties* pTypeProperties = TNULL;

	AAnimatableObjectType* pObjectType = TNULL;

	if ( a_pProperties )
	{
		// Get AnimatableObjectTypeBunch or AnimatableObjectType

		if ( a_pProperties->GetOptionalPropertyValue( pTypeProperties, "AnimatableObjectTypeBunch" ) )
			pObjectType = new AAnimatableObjectTypeBunch();
		else
			a_pProperties->GetOptionalPropertyValue( pTypeProperties, "AnimatableObjectType" );
	}

	if ( !pObjectType )
		pObjectType = new AAnimatableObjectType();

	TVALIDPTR( pObjectType );

	m_llObjectTypes.PushBack( pObjectType );

	TBOOL bCreated = ( pTypeProperties ) ?
	    pObjectType->CreateFromProperties( pTypeProperties, a_rcName ) :
	    pObjectType->Create( a_rcName );

	if ( bCreated && !a_bNoBreakpoints )
		AttachSoundBreakpoints( pObjectType );

	oTRB.Close();
}

// $Barnyard: FUNCTION 0057e840
void AAnimatableObjectManager::LoadAnimObjType( const TCHAR* a_szName, const PBProperties* a_pProperties, TBOOL a_bNoBreakpoints )
{
	LoadAnimObjType( TPString8( a_szName ), a_pProperties, a_bNoBreakpoints );
}

// $Barnyard: FUNCTION 0057d820
AAnimatableObjectType* AAnimatableObjectManager::FindType( const Toshi::TPString8& a_rcName )
{
	T2_FOREACH( m_llObjectTypes, it )
	{
		if ( it->GetName() == a_rcName )
			return it;
	}

	TERROR( "AAnimatableObjectManager: unable to find animatable type '%s'\n", a_rcName.GetString() );
	return TNULL;
}

// $Barnyard: FUNCTION 0057dac0
void AAnimatableObjectManager::DetachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent )
{
	TASSERT( a_pObject->m_oAttachmentInfo.pParentObject == a_pParent );
	a_pObject->m_oAttachmentInfo.pParentObject = TNULL;
	a_pParent->m_llAttachedObjects.Erase( a_pObject );
}

// $Barnyard: FUNCTION 0057d870
void AAnimatableObjectManager::DestroyType( const Toshi::TPString8& a_rcName )
{
	AAnimatableObjectType* pType = FindType( a_rcName );

	if ( pType )
	{
		pType->Remove();
		delete pType;

		g_pModelRepos->UnloadUnusedModels();
	}
}

// $Barnyard: FUNCTION 0057dc60
void AAnimatableObjectManager::DestroyAnimatableObject( AAnimatableObject* a_pAnimatableObject )
{
	if ( !a_pAnimatableObject )
		return;

	if ( m_llAnimatableObjects.Exists( a_pAnimatableObject ) )
	{
		// Destroy all attached objects
		DestroyAttachedObjects( a_pAnimatableObject );

		// Detach from a parent
		if ( a_pAnimatableObject->m_oAttachmentInfo.pParentObject )
			DetachObject( a_pAnimatableObject, a_pAnimatableObject->m_oAttachmentInfo.pParentObject );

		a_pAnimatableObject->Remove();
		delete a_pAnimatableObject;
	}
}

// $Barnyard: FUNCTION 0057d7c0
void AAnimatableObjectManager::DestroyAttachedObjects( AAnimatableObject* a_pAnimatableObject )
{
	if ( !a_pAnimatableObject )
		return;

	while ( a_pAnimatableObject->m_llAttachedObjects.IsLinked() )
	{
		AAnimatableObject* pAttachedObject = a_pAnimatableObject->m_llAttachedObjects.Begin();
		DestroyAnimatableObject( pAttachedObject );
	}
}

// $Barnyard: FUNCTION 0057d390
TBOOL AAnimatableObjectManager::LoadAnimSoundBreakpoints( const TCHAR* a_szFilePath )
{
	TASSERT( m_pAnimSoundBPsTRB == TNULL );

	if ( a_szFilePath )
	{
		m_pAnimSoundBPsTRB = new TTRB();

		if ( m_pAnimSoundBPsTRB->Load( a_szFilePath ) == TTRB::ERROR_OK )
		{
			m_pSoundBreakpoints = m_pAnimSoundBPsTRB->CastSymbol<ASoundBreakpointsTable>( "Main" );
			TVALIDPTR( m_pSoundBreakpoints );
		}
		else
		{
			TASSERT( !"Unable to load info about sound breakpoints" );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0057d420
TINT AAnimatableObjectManager::FindNumAnimationSoundBreakpoints( const Toshi::TPString8& a_rcModelName, ANamedAnimation* a_pAnimation )
{
	TVALIDPTR( m_pSoundBreakpoints );

	TINT iCount = 0;

	for ( TUINT i = 0; i < m_pSoundBreakpoints->uiCount; i++ )
	{
		if ( !T2String8::CompareNoCase( m_pSoundBreakpoints->pBreakpoints[ i ].pszModelName, a_rcModelName ) &&
		     !T2String8::CompareNoCase( m_pSoundBreakpoints->pBreakpoints[ i ].pszAnimationName, a_pAnimation->GetExportedName() ) )
		{
			iCount += 1;
		}
	}

	return iCount;
}

// $Barnyard: FUNCTION 0057ddc0
TBOOL AAnimatableObjectManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	T2_FOREACH( m_llAnimatableObjects, it )
	{
		// Find out whether the animation of this object needs to be updated or not
		TBOOL bUpdateAnim = it->IsAnimated();

		if ( bUpdateAnim )
		{
			AModelInstanceRef modelInstance = it->GetModelInstance();

			// Also, don't update animations when skeleton is not updating and no animations are queued
			if ( !modelInstance->IsUpdatingSkeleton() && it->GetQueuedAnimations().Size() <= 0 )
				bUpdateAnim = TFALSE;
		}

		if ( bUpdateAnim )
			it->Update( a_fDeltaTime );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0057e3e0
void AAnimatableObjectManager::AttachSoundBreakpoints( AAnimatableObjectType* a_pObjectType )
{
	if ( m_pSoundBreakpoints )
	{
		// Extract model name from the path
		const TCHAR* pszModelName       = a_pObjectType->GetModel()->GetName().GetString();
		TINT         iModelNameStartPos = T2String8::Length( pszModelName );

		while ( iModelNameStartPos > 0 && ( pszModelName[ iModelNameStartPos - 1 ] != '\\' && pszModelName[ iModelNameStartPos - 1 ] != '/' ) )
		{
			iModelNameStartPos--;
		}

		TCHAR szModelName[ 512 ];
		T2String8::Copy( szModelName, pszModelName + iModelNameStartPos );

		TSkeleton* pSkeleton = a_pObjectType->GetModel()->GetSkeleton();

		for ( TUINT i = 0; i < m_pSoundBreakpoints->uiCount; i++ )
		{
			ASoundBreakpoint* pSoundBreakpoint = &m_pSoundBreakpoints->pBreakpoints[ i ];

			if ( !T2String8::CompareNoCase( szModelName, pSoundBreakpoint->pszModelName ) )
			{
				for ( TINT k = 0; k < a_pObjectType->GetNumAnimationSets(); k++ )
				{
					ANamedAnimationSetRef pAnimationSet = a_pObjectType->GetAnimationSet( k );

					pAnimationSet->AddSoundBreakpoint( pSoundBreakpoint, pSkeleton );
				}
			}
		}
	}
}
