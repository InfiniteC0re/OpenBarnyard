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
	m_pUnk    = TNULL;
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
	AModelRepos* pModelRepos = AModelRepos::GetSingleton();
	pModelRepos->LoadModel( strPooledModelPath, TNULL );

	m_pAModel = pModelRepos->GetModel( strPooledModelPath );
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
	AModelRepos* pModelRepos = AModelRepos::GetSingleton();
	pModelRepos->LoadModel( strPooledModelPath, TNULL );

	m_pAModel = pModelRepos->GetModel( strPooledModelPath );
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
		AModelRepos::GetSingleton()->UnloadModel( m_pAModel->GetName(), TFALSE );
		m_pAModel = TNULL;
	}

	if ( m_pUnk )
	{
		TASSERT( !"Destroy m_pUnk" );
		delete m_pUnk;
		m_pUnk = TNULL;
	}
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
	return AModelRepos::GetSingleton()->InstantiateModel( GetModel() );
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

	TClass* pTypeClass = TClass::Find( strType, &TGetClass( AAnimControllerType ) );

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
	TASSERT( !"Not implemented" );

	return TTRUE;
}
