#include "ToshiPCH.h"
#include "TMaterial.h"
#include "TOrderTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TMaterial );

// $Barnyard: FUNCTION 006ccc60
TMaterial::TMaterial()
{
	m_Flags   = FLAGS_NONE;
	m_pShader = TNULL;
	m_iNumTex = 0;
	TUtil::MemClear( m_pTextures, sizeof( m_pTextures ) );
	m_pRegMaterial = TNULL;
	m_szName[ 0 ]  = '\0';
}

// $Barnyard: FUNCTION 006ccca0
// $Barnyard: FUNCTION 006ccdc0
TMaterial::~TMaterial()
{
}

// $Barnyard: FUNCTION 006cccc0
void TMaterial::OnDestroy()
{
	TASSERT( TTRUE == IsCreated() );
	m_Flags &= ~FLAGS_CREATED;
}

// $Barnyard: FUNCTION 006cccb0
TBOOL TMaterial::Create()
{
	TASSERT( TFALSE == IsCreated() );
	m_Flags |= FLAGS_CREATED;
	return TTRUE;
}

void TMaterial::PreRender()
{
}

void TMaterial::PostRender()
{
}

void TMaterial::Destroy()
{
	OnDestroy();
	delete this;
}

void TMaterial::SetShader( TShader* pShader )
{
	TASSERT( TNULL == m_pShader );
	m_pShader = pShader;
}

TShader* TMaterial::GetShader() const
{
	return m_pShader;
}

void TMaterial::SetTextureNum( TUINT32 a_iNumTextures )
{
	TASSERT( a_iNumTextures <= MAXTEXTURES );
	m_iNumTex = a_iNumTextures;
}

void TMaterial::SetTexture( TUINT32 a_iStage, TTexture* a_pTexture )
{
	TASSERT( a_iStage < MAXTEXTURES );
	m_pTextures[ a_iStage ] = a_pTexture;
}

TUINT32 TMaterial::GetTextureNum() const
{
	return m_iNumTex;
}

TTexture* TMaterial::GetTexture( TUINT32 a_iStage /*= 0*/ ) const
{
	return m_pTextures[ a_iStage ];
}

// $Barnyard: FUNCTION 006cccd0
void TMaterial::SetName( const TCHAR* a_szName )
{
	if ( a_szName == TNULL )
		m_szName[ 0 ] = '\0';

	TINT iNameLen = TStringManager::String8Length( a_szName );

	if ( NAMESIZELIMIT - 1 < iNameLen )
		TUtil::Log( "[ERROR] %s is to long, needs to be less then %d characters\n", a_szName, NAMESIZELIMIT );

	TStringManager::String8Copy( m_szName, a_szName, NAMESIZELIMIT );
}

// $Barnyard: FUNCTION 006ccd10
void TMaterial::SetFlags( Flags flag, TBOOL set /*= TFALSE*/ )
{
	if ( set )
	{
		if ( flag & ( FLAGS_NO_CULL | FLAGS_BLENDING | FLAGS_UNK3 ) )
		{
			m_Flags &= ~( FLAGS_NO_CULL | FLAGS_BLENDING | FLAGS_UNK3 );
		}

		m_Flags |= flag;
	}
	else
	{
		m_Flags &= ~flag;
	}
}

TBOOL TMaterial::IsCreated() const
{
	return m_Flags & FLAGS_CREATED;
}

TRenderPacket* TMaterial::AddRenderPacket( TMesh* a_pMesh )
{
	TVALIDPTR( a_pMesh );
	TVALIDPTR( m_pRegMaterial );

	return m_pRegMaterial->AddRenderPacket( a_pMesh );
}

void TMaterial::SetRegMaterial( TRegMaterial* pRegMaterial )
{
	m_pRegMaterial = pRegMaterial;
}

TRegMaterial* TMaterial::GetRegMaterial() const
{
	return m_pRegMaterial;
}

TOSHI_NAMESPACE_END
