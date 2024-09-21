#include "pch.h"
#include "AModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AModelInstance );

// $Barnyard: FUNCTION 0060fef0
AModel::AModel( const TPString8& a_rName, TTRB* a_pTRB ) :
    m_Name( a_rName ),
    m_uiID( ms_uiNumCreated ),
    m_pModelPtr( TNULL ),
    m_Vec1( TVector3::VEC_ZERO ),
    m_Vec2( TVector3::VEC_ZERO )
{
	m_pModelPtr = AModel::Create( a_rName, a_pTRB );

	ms_uiNumCreated += 1;
}

// $Barnyard: FUNCTION 0060fc90
// $Barnyard: FUNCTION 0060fed0
AModel::~AModel()
{
	if ( m_pModelPtr )
	{
		delete m_pModelPtr;
	}
}

// $Barnyard: FUNCTION 0060f9f0
void AModel::Update( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();

	for ( TUINT i = 0; i < m_vecInstanceRefs.Size(); i++ )
	{
		auto pInstance = m_vecInstanceRefs[ i ].Get();

		if ( pInstance->IsUpdatingSkeleton() )
		{
			pInstance->GetSceneObject()->Update( a_fDeltaTime );
		}
	}
}

// $Barnyard: FUNCTION 0060fb10
void AModel::Render( TUINT8 a_uiFlags )
{
	for ( TUINT i = 0; i < m_vecInstanceRefs.Size(); i++ )
	{
		auto& pModelInstance = m_vecInstanceRefs[ i ];
		auto  eFlags         = pModelInstance->m_eFlags;

		if ( ( ( eFlags >> 4 & 1 ) == a_uiFlags && ( eFlags & 2 ) != 0 ) && ( eFlags & 1 ) != 0 )
		{
			auto pSceneObject = pModelInstance->GetSceneObject();
			auto pModel       = pSceneObject->GetInstance()->GetModel();

			auto& transform      = pSceneObject->GetTransform();
			auto& transformScale = transform.GetScale();
			auto& lod            = pModel->GetLOD( 0 );

			TFLOAT fRadiusScale = TMath::Max( TMath::Max( transformScale.x, transformScale.y ), transformScale.z );
			TFLOAT fRadius      = lod.BoundingSphere.GetRadius() * fRadiusScale;

			Toshi::TMatrix44 transformMatrix;
			transform.GetLocalMatrixImp( transformMatrix );

			Toshi::TVector3 boundingPos;
			Toshi::TMatrix44::TransformVector( boundingPos, transformMatrix, lod.BoundingSphere.GetOrigin() );
			pSceneObject->Render( pModelInstance->GetClipFlags(), boundingPos );
		}
	}
}

// $Barnyard: FUNCTION 0060fd20
AModelInstanceRef* AModel::CreateInstance( AModelInstanceRef& a_rOutRef )
{
	// TODO: use this name in debug?
	TString8 instanceName;
	GenerateInstanceName( instanceName, m_Name );

	AModelInstanceRef modelInstanceRef = new AModelInstance(
	    this,
	    m_pModelPtr->CreateSceneObject(),
	    TFALSE );

	m_vecInstanceRefs.PushBack( modelInstanceRef );
	a_rOutRef = modelInstanceRef;

	return &a_rOutRef;
}

TSIZE AModel::GetNumInstances() const
{
	return m_vecInstanceRefs.Size();
}

// $Barnyard: FUNCTION 0060f8c0
TModelPtr* AModel::Create( const TPString8& a_rFilePath, TTRB* a_pTRB )
{
	// TODO: use this name in debug?
	TString8 modelName;
	GetNameFromPath( a_rFilePath, modelName );

	auto pModelPtr = new TModelPtr();

	if ( pModelPtr )
	{
		TString8 filepath = a_rFilePath.GetString8();
		filepath.MakeLower();

		if ( filepath.Find( ".trb" ) < 0 &&
		     filepath.Find( ".trz" ) < 0 )
		{
			filepath += ".trb";
		}

		pModelPtr->Create( filepath, a_pTRB );
		TFIXME( "Call FUN_0060dcc0" );
	}

	return pModelPtr;
}

// $Barnyard: FUNCTION 0060f6e0
void AModel::GetNameFromPath( const TPString8& a_FilePath, TString8& a_rName )
{
	TString8 name = a_FilePath.GetString8();

	if ( name.Length() <= 8 )
	{
		a_rName = name;
	}
	else
	{
		TINT iPos1 = 0;
		TINT iPos2;

		do {
			iPos2 = iPos1;
			iPos1 = name.Find( '\\', iPos1 + 1 );
		} while ( -1 < iPos1 );

		do {
			iPos1 = iPos2;
			iPos2 = name.Find( '/', iPos1 + 1 );
		} while ( -1 < iPos2 );

		if ( iPos1 < 0 || name.Length() - 1 <= iPos1 ) iPos1 = 0;
		iPos1 += 1;
		iPos2 = name.Find( '_', iPos1 );
		if ( iPos2 < 0 ) iPos2 = name.Length() - 1;

		TUINT iSize = iPos2 - iPos1;

		if ( iSize < 1 )
		{
			iPos1 = 0;
			iSize = 8;
		}
		else if ( 8 < iSize )
		{
			iSize = 8;
		}

		a_rName.Copy( name.GetString( iPos1 ), iSize );
	}
}

// $Barnyard: FUNCTION 0060f7f0
TString8* AModel::GenerateInstanceName( TString8& a_rOutName, const TPString8& a_FilePath )
{
	static TUINT ms_uiDebugObjectIndex = 0;

	TString8 name;
	GetNameFromPath( a_FilePath, name );

	TString8 debugName;
	debugName.Format( "%s%i", name.GetString(), ms_uiDebugObjectIndex % 1000 );
	ms_uiDebugObjectIndex += 1;

	a_rOutName = debugName;
	return &a_rOutName;
}

// $Barnyard: FUNCTION 00611280
AModelInstance::AModelInstance( AModel* a_pModel, TSceneObject* a_pSceneObject, TBOOL a_bEnableSkeletonUpdate ) :
    m_ChangeEmitter( this )
{
	TFIXME( "Initialise some unknown members" );

	m_Unknown1[ 0 ] = 1.0f;
	m_Unknown1[ 1 ] = 1.0f;
	m_Unknown1[ 2 ] = 1.0f;
	m_Unknown1[ 3 ] = 1.0f;
	m_pModel        = a_pModel;
	m_uiClipFlags   = 0x3F;
	m_pSceneObject  = a_pSceneObject;
	m_eFlags        = 0b00001001;

	SetSkeletonUpdating( a_bEnableSkeletonUpdate );
	m_pSceneObject->GetInstance()->SetCustomRenderMethod( RenderInstanceCallback, this );
}

// $Barnyard: FUNCTION 00610820
AModelInstance::AModelInstance() :
    m_ChangeEmitter( this )
{
	TFIXME( "Initialise some unknown members" );

	m_Unknown1[ 0 ] = 1.0f;
	m_Unknown1[ 1 ] = 1.0f;
	m_Unknown1[ 2 ] = 1.0f;
	m_Unknown1[ 3 ] = 1.0f;
	m_pModel        = TNULL;
	m_pSceneObject  = TNULL;
	m_uiClipFlags   = 0x3F;
	m_eFlags        = 0b00011000;
}

void AModelInstance::RenderInstanceCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 00610cd0
void AModelInstance::SetSkeletonUpdating( TBOOL a_bUpdating )
{
	if ( a_bUpdating )
	{
		if ( !IsUpdatingSkeleton() )
		{
			m_pSceneObject->EnableSkeletonUpdate();
			m_ChangeEmitter.Throw( ChangeEvent_EnabledSkeletonUpdate );
		}

		m_eFlags |= Flags_UpdatingSkeleton;
	}
	else
	{
		if ( IsUpdatingSkeleton() )
		{
			m_pSceneObject->DisableSkeletonUpdate();
			m_ChangeEmitter.Throw( ChangeEvent_DisabledSkeletonUpdate );
		}

		m_eFlags &= ~Flags_UpdatingSkeleton;
	}
}
