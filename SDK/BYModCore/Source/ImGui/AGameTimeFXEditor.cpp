#include "pch.h"
#include "AGameTimeFXEditor.h"

#include "AImGui.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGameTimeFXEditor::AGameTimeFXEditor()
    : m_ppSettings( TNULL )
    , m_iNumSettings( 0 )
    , m_bClosed( TFALSE )
{
	TUINTPTR pFXManager = *TREINTERPRETCAST( TUINTPTR*, 0x00783d08 );

	if ( pFXManager )
	{
		m_ppSettings   = *TREINTERPRETCAST( Settings_TRBHeader***, pFXManager + 0x794 );
		m_iNumSettings = *TREINTERPRETCAST( TINT*, pFXManager + 0x79C );

		m_bClosed = TTRUE;
	}
}

AGameTimeFXEditor::~AGameTimeFXEditor()
{
}

TBOOL AGameTimeFXEditor::Render()
{
	if ( !m_ppSettings || m_bClosed ) return !m_bClosed;

	ImGui::Begin( "AGameTimeFX Editor", &m_bClosed );
	{
		if ( ImGui::BeginTabBar( "MyTabBar" ) )
		{
			if ( ImGui::BeginTabItem( "FX" ) )
			{
				for ( TINT i = 0; i < m_iNumSettings; i++ )
				{
					if ( ImGui::CollapsingHeader( m_ppSettings[ i ]->m_pchName ) )
					{

#define FIELD1( name )    \
	ImGui::Text( #name ); \
	ImGui::InputFloat( #name, &( m_ppSettings[ i ]->name ) )

#define FIELD4( name )    \
	ImGui::Text( #name ); \
	ImGui::InputFloat4( #name, ( (float*)&m_ppSettings[ i ]->name ) )

						FIELD4( m_vecLightDir );
						FIELD4( field1_0x10 );
						FIELD4( m_oShadowColour );
						FIELD4( m_oAmbientColour );
						FIELD4( m_vecFogColor );
						FIELD4( field5_0x50 );
						FIELD4( field6_0x60 );
						FIELD4( field7_0x70 );
						FIELD4( field8_0x80 );
						FIELD4( field9_0x90 );
						FIELD4( field10_0xa0 );
						FIELD4( field11_0xb0 );
						FIELD4( field12_0xc0 );
						FIELD4( field13_0xd0 );
						FIELD4( field14_0xe0 );
						FIELD4( field15_0xf0 );
						FIELD4( field16_0x100 );
						FIELD1( field17_0x110 );
						FIELD1( field19_0x118 );
						FIELD1( m_fFogStart );
						FIELD1( m_fFogEnd );
						FIELD1( field22_0x124 );
						FIELD1( m_fShadowAmbientProgress );
						FIELD1( m_flInstanceShading );
						FIELD1( field25_0x130 );
						FIELD1( field26_0x134 );
						FIELD1( field27_0x138 );
						FIELD1( field28_0x13c );
						FIELD1( m_fStarsOpacity );
						FIELD1( field30_0x144 );
						FIELD1( field31_0x148 );
						FIELD1( field32_0x14c );
						FIELD1( m_fDarkeningFactor );
						FIELD1( m_fLightIntensity );
						FIELD1( field35_0x158 );
						FIELD1( field36_0x15c );
						FIELD1( field37_0x160 );
						FIELD1( field38_0x164 );
						FIELD1( field39_0x168 );
						FIELD1( field40_0x16c );
						FIELD1( field41_0x170 );
						FIELD1( field42_0x174 );
					}
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	return !m_bClosed;
}
