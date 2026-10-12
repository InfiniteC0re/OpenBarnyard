#include "pch.h"
#include "AAnimatableObjectType.h"
#include "AModelRepos.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0057ed30
AAnimatableObjectType::AAnimatableObjectType()
{
	m_pAModel = TNULL;
	m_pSkins  = TNULL;
}

// $Barnyard: FUNCTION 0057f2c0
// $Barnyard: FUNCTION 0057efe0
AAnimatableObjectType::~AAnimatableObjectType()
{
	Destroy();
}

TPSTRING8_DECLARE( Name );
TPSTRING8_DECLARE( Model );

// $Barnyard: FUNCTION 0057f450
TBOOL AAnimatableObjectType::CreateFromProperties( const PBProperties* a_pProperties, const Toshi::TPString8& a_rcName )
{
	TVALIDPTR( a_pProperties );
	m_strName = a_rcName;

	// Get anim object name
	TPString8 strObjName;
	a_pProperties->GetOptionalPropertyValue( strObjName, TPS8( Name ) );

	// Get anim object model name
	TPString8 strObjModel = strObjName;
	a_pProperties->GetOptionalPropertyValue( strObjModel, TPS8( Model ) );

	// Get full path to the model
	TString8 strModelPath;
	strModelPath.Format( "%s%s", "data\\models\\", strObjModel.GetString() );

	TPString8 strPooledModelPath = TPString8( strModelPath.GetString() );

	// Load model
	g_pModelRepos->LoadModel( strPooledModelPath, TNULL );

	m_pAModel = g_pModelRepos->GetModel( strPooledModelPath );
	TVALIDPTR( m_pAModel );

	TBOOL bLoadedAnimationSet = TFALSE;

	T2_FOREACH( ( *a_pProperties ), it )
	{
		if ( it->GetValue()->GetType() == PBPropertyValue::Type::Properties )
		{
			if ( !T2String8::Compare( it->GetName().GetString(), "NamedAnimationSet" ) )
			{
				LoadAnimationSet( it->GetValue()->GetProperties() );
				bLoadedAnimationSet = TTRUE;
			}
			else if ( !T2String8::Compare( it->GetName().GetString(), "AnimControllerType" ) )
			{
				SetAnimController( it->GetValue()->GetProperties() );
			}
			else if ( !T2String8::Compare( it->GetName().GetString(), "Skins" ) )
			{
				SetSkins( it->GetValue()->GetProperties() );
			}
		}
	}

	if ( !bLoadedAnimationSet )
		LoadAnimationSet( TNULL );

	return TTRUE;
}

// $Barnyard: FUNCTION 0057f2e0
TBOOL AAnimatableObjectType::Create( const Toshi::TPString8& a_rcName )
{
	m_strName = a_rcName;

	// Get full path to the model
	TString8 strModelPath;
	strModelPath.Format( "%s%s", "data\\models\\", m_strName.GetString() );

	TPString8 strPooledModelPath = TPString8( strModelPath.GetString() );

	// Load model
	g_pModelRepos->LoadModel( strPooledModelPath, TNULL );

	m_pAModel = g_pModelRepos->GetModel( strPooledModelPath );
	TVALIDPTR( m_pAModel );

	LoadAnimationSet( TNULL );

	return TTRUE;
}

void AAnimatableObjectType::OnAnimatableObjectCreated( AAnimatableObject* a_pAnimatableObject )
{
}

// $Barnyard: FUNCTION 0057eeb0
void AAnimatableObjectType::Destroy()
{
	m_vecAnimationSets.Clear();
	m_llAnimControllerTypes.DeleteAll();

	if ( m_pAModel )
	{
		g_pModelRepos->UnloadModel( m_pAModel->GetName(), TFALSE );
		m_pAModel = TNULL;
	}

	if ( m_pSkins )
	{
		delete m_pSkins;
		m_pSkins = TNULL;
	}
}

// $Barnyard: FUNCTION 0057ea10
AAnimatableObjectType::Skins::~Skins()
{
	for ( TINT i = 0; i < iNumSkins; i++ )
	{
		TFree( pszTextures[ i ] );
		TFree( pszSkinLights[ i ] );
	}

	if ( pszSkinLights ) TFree( pszSkinLights );
	if ( pszTextures ) TFree( pszTextures );
	if ( szMaterialName ) TFree( szMaterialName );
}

// $Barnyard: FUNCTION 0057ec00
TINT AAnimatableObjectType::GetNumSkins() const
{
	if ( m_pSkins )
		return m_pSkins->iNumSkins;

	return 0;
}

// $Barnyard: FUNCTION 0057ec10
const TCHAR* AAnimatableObjectType::GetMaterialName() const
{
	if ( m_pSkins )
		return m_pSkins->szMaterialName;

	return TNULL;
}

// $Barnyard: FUNCTION 0057ec20
const TCHAR* AAnimatableObjectType::GetSkinTexture( TINT a_iSkin ) const
{
	if ( m_pSkins && a_iSkin >= 0 && a_iSkin < m_pSkins->iNumSkins )
		return m_pSkins->pszTextures[ a_iSkin ];

	return TNULL;
}

// $Barnyard: FUNCTION 0057ec50
const TCHAR* AAnimatableObjectType::GetSkinLighting( TINT a_iSkin ) const
{
	if ( m_pSkins && a_iSkin >= 0 && a_iSkin < m_pSkins->iNumSkins )
		return m_pSkins->pszSkinLights[ a_iSkin ];

	return TNULL;
}

// $Barnyard: FUNCTION 0057ef40
ANamedAnimationSetRef AAnimatableObjectType::FindAnimationSet( const Toshi::TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_vecAnimationSets.Size(); i++ )
	{
		if ( m_vecAnimationSets[ i ]->GetName() == a_rcName )
		{
			return m_vecAnimationSets[ i ];
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0057f220
ANamedAnimationSetRef AAnimatableObjectType::FindAnimationSetOrDefault( const Toshi::TPString8& a_rcName ) const
{
	ANamedAnimationSetRef pAnimationSet = FindAnimationSet( a_rcName );

	if ( !pAnimationSet.Get() )
		pAnimationSet = m_vecAnimationSets[ 0 ];

	return pAnimationSet;
}

// $Barnyard: FUNCTION 0057efb0
ANamedAnimationSetRef AAnimatableObjectType::GetAnimationSet( TINT a_iIndex ) const
{
	TASSERT( a_iIndex < MAX_ANIMATION_SETS );

	if ( m_vecAnimationSets[ a_iIndex ].Get() )
		return m_vecAnimationSets[ a_iIndex ];

	return ANamedAnimationSetRef();
}

// $Barnyard: FUNCTION 0057ef10
ANamedAnimationSetRef AAnimatableObjectType::GetDefaultAnimationSet() const
{
	return GetAnimationSet( 0 );
}

TINT AAnimatableObjectType::GetNumAnimationSets() const
{
	return m_vecAnimationSets.Size();
}

AModelInstance* AAnimatableObjectType::Instantiate()
{
	return g_pModelRepos->InstantiateModel( GetModel() );
}

// $Barnyard: FUNCTION 0057f060
TBOOL AAnimatableObjectType::LoadAnimationSet( const PBProperties* a_pProperties )
{
	TVALIDPTR( m_pAModel );

	TSkeleton*            pSkeleton     = m_pAModel->GetSkeleton();
	ANamedAnimationSetRef pAnimationSet = TNULL;

	if ( a_pProperties )
	{
		// Get anim object name
		TPString8 strName;
		a_pProperties->GetOptionalPropertyValue( strName, TPS8( Name ) );

		// Check if this animation set is already created
		pAnimationSet = FindAnimationSet( strName ).Get();
	}

	if ( !pAnimationSet.Get() )
	{
		// Create new animation set
		pAnimationSet = new ( AMemory::GetMemBlock( AMemory::POOL_Misc ) ) ANamedAnimationSet();
		m_vecAnimationSets.PushBack( pAnimationSet );
	}

	TVALIDPTR( pAnimationSet );
	TVALIDPTR( pSkeleton );

	if ( a_pProperties )
		return pAnimationSet->CreateFromProperties( a_pProperties, pSkeleton );

	return TTRUE;
}

TPSTRING8_DECLARE( Type );

// $Barnyard: FUNCTION 0057eda0
TBOOL AAnimatableObjectType::SetAnimController( const PBProperties* a_pProperties )
{
	TVALIDPTR( a_pProperties );

	// Get controller type name
	TPString8 strType;
	a_pProperties->GetOptionalPropertyValue( strType, TPS8( Type ) );

	const TClass* pTypeClass = TClass::Find( strType, &TGetClass( AAnimControllerType ) );

	TASSERT( TNULL != pTypeClass );
	if ( pTypeClass == TNULL ) return TFALSE;

	// Create the actual controller type
	AAnimControllerType* pControllerType = TSTATICCAST( AAnimControllerType, pTypeClass->CreateObject() );
	TVALIDPTR( pControllerType );

	m_llAnimControllerTypes.PushBack( pControllerType );

	return pControllerType->Create( a_pProperties );
}

// $Barnyard: FUNCTION 0057ea70
TBOOL AAnimatableObjectType::SetSkins( const PBProperties* a_pProperties )
{
	TVALIDPTR( a_pProperties );

	const PBPropertyValue* pTexValue = a_pProperties->GetProperty( "Tex" );

	if ( pTexValue && pTexValue->GetType() == PBPropertyValue::Type::Array )
	{
		const PBPropertyValueArray* pTexArray = pTexValue->GetArray();

		// The first texture is the material name, the rest are pairs of skin texture and skin light
		TINT iNumSkins = ( TINT( pTexArray->GetSize() ) - 1 ) / 2;

		if ( iNumSkins < 1 )
			return TFALSE;

		m_pSkins            = new Skins();
		m_pSkins->iNumSkins = iNumSkins;

		const TCHAR* szMaterialName = pTexArray->GetValue( 0 )->GetString();
		m_pSkins->szMaterialName    = TSTATICCAST( TCHAR, TMalloc( TStringManager::String8Length( szMaterialName ) + 1 ) );
		TStringManager::String8Copy( m_pSkins->szMaterialName, szMaterialName );

		TINT iValueIndex        = 1;
		m_pSkins->pszTextures   = TSTATICCAST( TCHAR*, TMalloc( iNumSkins * sizeof( TCHAR* ) ) );
		m_pSkins->pszSkinLights = TSTATICCAST( TCHAR*, TMalloc( iNumSkins * sizeof( TCHAR* ) ) );

		for ( TINT i = 0; i < iNumSkins; i++, iValueIndex++ )
		{
			const TCHAR* szTexture     = pTexArray->GetValue( iValueIndex )->GetString();
			m_pSkins->pszTextures[ i ] = TSTATICCAST( TCHAR, TMalloc( TStringManager::String8Length( szTexture ) + 1 ) );
			TStringManager::String8Copy( m_pSkins->pszTextures[ i ], szTexture );
		}

		for ( TINT i = 0; i < iNumSkins; i++, iValueIndex++ )
		{
			const TCHAR* szSkinLight     = pTexArray->GetValue( iValueIndex )->GetString();
			m_pSkins->pszSkinLights[ i ] = TSTATICCAST( TCHAR, TMalloc( TStringManager::String8Length( szSkinLight ) + 1 ) );
			TStringManager::String8Copy( m_pSkins->pszSkinLights[ i ], szSkinLight );
		}
	}

	return TTRUE;
}
