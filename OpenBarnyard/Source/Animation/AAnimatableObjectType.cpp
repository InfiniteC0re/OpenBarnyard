#include "pch.h"
#include "AAnimatableObjectType.h"
#include "Render/AModelRepos.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0057ed30
AAnimatableObjectType::AAnimatableObjectType()
{
	m_pModel = TNULL;
	m_pUnk   = TNULL;
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

	TPString8 strPooledModelPath = strModelPath.GetString();

	// Load model
	AModelRepos* pModelRepos = AModelRepos::GetSingleton();
	pModelRepos->LoadModel( strPooledModelPath, TNULL );

	m_pModel = pModelRepos->GetModel( strPooledModelPath );
	TVALIDPTR( m_pModel );

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
				SetAnimControllerType( it->GetValue()->GetProperties() );
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

	TPString8 strPooledModelPath = strModelPath.GetString();

	// Load model
	AModelRepos* pModelRepos = AModelRepos::GetSingleton();
	pModelRepos->LoadModel( strPooledModelPath, TNULL );

	m_pModel = pModelRepos->GetModel( strPooledModelPath );
	TVALIDPTR( m_pModel );

	LoadAnimationSet( TNULL );

	return TTRUE;
}

// $Barnyard: FUNCTION 0057eeb0
void AAnimatableObjectType::Destroy()
{
	m_vecAnimationSets.Clear();
	m_UnkList.DeleteAll();

	if ( m_pModel )
	{
		AModelRepos::GetSingleton()->UnloadModel( m_pModel->GetName(), TFALSE );
		m_pModel = TNULL;
	}

	if ( m_pUnk )
	{
		TASSERT( !"Destroy m_pUnk" );
		delete m_pUnk;
		m_pUnk = TNULL;
	}
}

TBOOL AAnimatableObjectType::LoadAnimationSet( const PBProperties* a_pProperties )
{
	TVALIDPTR( a_pProperties );
	TASSERT( !"Not implemented" );

	return TTRUE;
}

TBOOL AAnimatableObjectType::SetAnimControllerType( const PBProperties* a_pProperties )
{
	TVALIDPTR( a_pProperties );
	TASSERT( !"Not implemented" );

	return TTRUE;
}

TBOOL AAnimatableObjectType::SetSkins( const PBProperties* a_pProperties )
{
	TVALIDPTR( a_pProperties );
	TASSERT( !"Not implemented" );

	return TTRUE;
}
