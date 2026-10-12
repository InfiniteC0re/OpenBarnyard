#include "pch.h"
#include "AAnimatableObjectManager.h"
#include "AAnimatableObjectTypeBunch.h"
#include "AModelRepos.h"
#include "ALoadScreen.h"
#include "FX/AParticleEffectManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AAnimatableObjectManager );

// T2ObjectPool<AToshiAnimationRef, 100> methods (vtable 0x00727ef0):
// $Barnyard: FUNCTION 005822f0
// $Barnyard: FUNCTION 00582310
// $Barnyard: FUNCTION 00582330
// $Barnyard: FUNCTION 00582350
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
AAnimatableObject* AAnimatableObjectManager::CreateAnimatableObject( AAnimatableObjectType* a_pObjectType, void* a_pOwner, TUINT a_eFlags )
{
	AAnimatableObject* pObject = new AAnimatableObject();

	m_llAnimatableObjects.PushBack( pObject );
	pObject->Create( a_pObjectType, a_pOwner, a_eFlags );

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

	// NOTE: there might be no ModelTypes file (the barn items, "bn_saloon_bar_add"), the type is then created from the model of the same name
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

	// NOTE: no error here, callers use FindType to check whether a type is loaded (e.g. 'sim_golf_flag' in
	// AGameLoader::DestroyLevel, 'exclamation' in AAnimalPopulationManager::SetMarkersEnabled), like retail
	return TNULL;
}

#ifdef BARNYARD_COMMUNITY_PATCH
TBOOL AAnimatableObjectManager::IsModelUsedByType( const AModel* a_pModel )
{
	T2_FOREACH( m_llObjectTypes, it )
	{
		if ( it->GetModel() == a_pModel )
			return TTRUE;
	}

	return TFALSE;
}
#endif

// $Barnyard: FUNCTION 0057e060
void AAnimatableObjectManager::AttachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent, TINT8 a_iBone, TINT8 a_iParentBone, const TVector4& a_rcTranslation, const TQuaternion& a_rcRotation, TFLOAT a_fScale )
{
	a_pObject->m_oAttachmentInfo.pParentObject = a_pParent;
	a_pObject->m_oAttachmentInfo.translation   = a_rcTranslation;
	a_pObject->m_oAttachmentInfo.quaternion    = a_rcRotation;

	a_pObject->GetModelInstance()->SetScale( TVector4( a_fScale, a_fScale, a_fScale, 1.0f ) );

	a_pObject->m_oAttachmentInfo.iBone       = a_iBone;
	a_pObject->m_oAttachmentInfo.iParentBone = a_iParentBone;
	a_pParent->m_llAttachedObjects.PushFront( a_pObject );

	RegisterAnimatableObject( a_pObject );
}

// $Barnyard: FUNCTION 0057e730
void AAnimatableObjectManager::AttachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent, const TPString8& a_rcBone, const TPString8& a_rcParentBone, const TVector4& a_rcTranslation, const TQuaternion& a_rcRotation, TFLOAT a_fScale )
{
	TINT iBone       = ( a_rcBone.IsEmpty() ) ? -1 : a_pObject->GetModelInstance()->GetBoneIndex( a_rcBone );
	TINT iParentBone = a_pParent->GetModelInstance()->GetBoneID( a_rcParentBone );

	AttachObject( a_pObject, a_pParent, TINT8( iBone ), TINT8( iParentBone ), a_rcTranslation, a_rcRotation, a_fScale );
}

// $Barnyard: FUNCTION 0057daf0
void AAnimatableObjectManager::RegisterAnimatableObject( AAnimatableObject* a_pObject )
{
	a_pObject->Remove();
	m_llAnimatableObjects.PushBack( a_pObject );

	T2_FOREACH( a_pObject->m_llAttachedObjects, it )
	{
		RegisterAnimatableObject( it );
	}
}

// $Barnyard: FUNCTION 0057d8e0
AAnimatableObject* AAnimatableObjectManager::GetAttachedObject( AAnimatableObject* a_pObject, TINT a_iBone, TINT a_iIndex )
{
	if ( a_iBone > -1 )
	{
		TINT iIndex = 0;

		T2_FOREACH( a_pObject->m_llAttachedObjects, it )
		{
			if ( it->m_oAttachmentInfo.iParentBone == a_iBone )
			{
				if ( iIndex == a_iIndex )
					return it;

				iIndex++;
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057d950
AAnimatableObject* AAnimatableObjectManager::GetAttachedObjectOfType( AAnimatableObject* a_pObject, TINT a_iBone, const TPString8& a_rcTypeName, TINT a_iIndex )
{
	if ( a_iBone > -1 )
	{
		TINT iIndex = 0;

		T2_FOREACH( a_pObject->m_llAttachedObjects, it )
		{
			if ( it->m_pObjectType && it->m_oAttachmentInfo.iParentBone == a_iBone && it->m_pObjectType->GetName() == a_rcTypeName )
			{
				if ( iIndex == a_iIndex )
					return it;

				iIndex++;
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057e1b0
AAnimatableObject* AAnimatableObjectManager::GetAttachedObjectOfType( AAnimatableObject* a_pObject, const TPString8& a_rcBone, const TPString8& a_rcTypeName, TINT a_iIndex )
{
	TINT iBone = a_pObject->GetModelInstance()->GetBoneIndex( a_rcBone );
	return GetAttachedObjectOfType( a_pObject, iBone, a_rcTypeName, a_iIndex );
}

// $Barnyard: FUNCTION 0057e350
AAnimatableObject* AAnimatableObjectManager::GetAttachedObject( AAnimatableObject* a_pObject, const TPString8& a_rcBone, TINT a_iIndex )
{
	TINT iBone = a_pObject->GetModelInstance()->GetBoneID( a_rcBone );
	return GetAttachedObject( a_pObject, iBone, a_iIndex );
}

// $Barnyard: FUNCTION 0057e120
AAnimatableObject* AAnimatableObjectManager::GetAttachedObjectByBoneName( AAnimatableObject* a_pObject, const TPString8& a_rcBone, TINT a_iIndex )
{
	TINT iBone = a_pObject->GetModelInstance()->GetBoneIndex( a_rcBone );
	return GetAttachedObject( a_pObject, iBone, a_iIndex );
}

// $Barnyard: FUNCTION 0057d9d0
TINT AAnimatableObjectManager::GetNumAttachedObjects( AAnimatableObject* a_pObject, TINT a_iBone )
{
	TINT iNumAttached = 0;

	if ( a_iBone > -1 )
	{
		T2_FOREACH( a_pObject->m_llAttachedObjects, it )
		{
			if ( it->m_oAttachmentInfo.iParentBone == a_iBone )
				iNumAttached++;
		}
	}

	return iNumAttached;
}

// $Barnyard: FUNCTION 0057e240
TINT AAnimatableObjectManager::GetNumAttachedObjects( AAnimatableObject* a_pObject, const TPString8& a_rcBone )
{
	TINT iBone = a_pObject->GetModelInstance()->GetBoneIndex( a_rcBone );
	return GetNumAttachedObjects( a_pObject, iBone );
}

// $Barnyard: FUNCTION 0057da40
TINT AAnimatableObjectManager::GetNumAttachedObjectsRecursive( AAnimatableObject* a_pObject, TINT a_iBone )
{
	TINT iNumAttached = 0;

	if ( a_iBone > -1 )
	{
		T2_FOREACH( a_pObject->m_llAttachedObjects, it )
		{
			if ( it->m_oAttachmentInfo.iParentBone == a_iBone )
				iNumAttached += GetNumAttachedObjectsRecursive( it, 0 ) + 1;
		}
	}

	return iNumAttached;
}

// $Barnyard: FUNCTION 0057e2c0
TINT AAnimatableObjectManager::GetNumAttachedObjectsRecursive( AAnimatableObject* a_pObject, const TPString8& a_rcBone )
{
	TINT iBone = a_pObject->GetModelInstance()->GetBoneIndex( a_rcBone );
	return GetNumAttachedObjectsRecursive( a_pObject, iBone );
}

// $Barnyard: FUNCTION 0057dac0
void AAnimatableObjectManager::DetachObject( AAnimatableObject* a_pObject, AAnimatableObject* a_pParent )
{
	TASSERT( a_pObject->m_oAttachmentInfo.pParentObject == a_pParent );
	a_pObject->m_oAttachmentInfo.pParentObject = TNULL;
	a_pParent->m_llAttachedObjects.Erase( a_pObject );
}

// $Barnyard: FUNCTION 0057dd40
void AAnimatableObjectManager::DestroyType( const TCHAR* a_szName )
{
	DestroyType( TPString8( a_szName ) );
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

// $Barnyard: FUNCTION 0057dcb0
void AAnimatableObjectManager::DestroyAnimatableObjectsOfType( AAnimatableObjectType* a_pType )
{
	for ( auto it = m_llAnimatableObjects.Begin(); it != m_llAnimatableObjects.End(); )
	{
		AAnimatableObject* pObject = it;
		++it;

		if ( pObject->m_pObjectType == a_pType && m_llAnimatableObjects.Exists( pObject ) )
		{
			DestroyAttachedObjects( pObject );

			if ( pObject->m_oAttachmentInfo.pParentObject )
				DetachObject( pObject, pObject->m_oAttachmentInfo.pParentObject );

			pObject->Remove();
			delete pObject;
		}
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

// $Barnyard: FUNCTION 0057d370
TINT AAnimatableObjectManager::GetAttachmentBone( AAnimatableObject* a_pObject )
{
	return a_pObject->m_oAttachmentInfo.iBone;
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

// $Barnyard: FUNCTION 0057dea0
void AAnimatableObjectManager::UpdateAttachedObjects( TFLOAT a_fDeltaTime )
{
	if ( !IsActive() )
		return;

	T2_FOREACH( m_llAnimatableObjects, it )
	{
		AToshiAnimationInterface&       rAnimInterface = it->GetToshiAnimInterface();
		AToshiAnimationInterface::FLAGS uiAnimFlags    = rAnimInterface.GetFlags();

		// All animations are static: the skeleton state doesn't change, so don't recalculate it
		if ( HASANYFLAG( uiAnimFlags, AToshiAnimationInterface::FLAGS_ALL_STATIC ) && rAnimInterface.GetTime() > 1.0f && !HASANYFLAG( uiAnimFlags, AToshiAnimationInterface::FLAGS_ALWAYS_UPDATE ) )
			it->GetModelInstance()->GetSkeletonInstance()->UpdateStateFrame();

		if ( !it->m_llAttachedObjects.IsEmpty() )
		{
			// Bones of this object are needed to place the attached objects
			if ( it->GetModelInstance()->GetSceneObject()->ShouldAnimateThisFrame() )
			{
				TSkeletonInstance* pSkeletonInstance = it->GetModelInstance()->GetSkeletonInstance();
				pSkeletonInstance->UpdateTime( a_fDeltaTime );
				pSkeletonInstance->UpdateState( TTRUE ); // NOTE: retail UpdateState has no frame check
			}
		}

		if ( it->m_oAttachmentInfo.pParentObject )
			it->UpdateAttachmentTransform();
		else if ( it->m_oAttachmentInfo.pLocator )
			it->UpdateLocatorTransform();
	}

	AParticleEffectManager::GetSingleton()->OnUpdate( a_fDeltaTime );
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
