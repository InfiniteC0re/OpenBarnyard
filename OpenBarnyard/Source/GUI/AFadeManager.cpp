#include "pch.h"
#include "AFadeManager.h"
#include "Tasks/ARootTask.h"
#include "GUI/AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS( AFadeManager );

TBOOL AFadeManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !ARootTask::GetSingleton()->IsPaused() )
	{
		for ( auto it = m_ActiveFades.Begin(); it != m_ActiveFades.End(); it++ )
		{
			it->Update( a_fDeltaTime );
		}

		for ( auto it = m_ActiveFades.Begin(); it != m_ActiveFades.End(); )
		{
			auto pNext = it->Next();

			if ( it->IsStillFading( TTRUE ) )
			{
				AFade::Color color;
				it->GetCurrentColor( color );
				it->GetRectangleElement().SetColour( color.GetTColor32() );
			}
			else
			{
				m_ActiveFades.Remove( it );
				m_Fades.DeleteObject( it );
			}

			it = pNext;
		}

		return TTRUE;
	}

	return TFALSE;
}

void AFadeManager::StopAllFades()
{
	while ( TTRUE )
	{
		auto it = m_ActiveFades.Begin();
		if ( it == m_ActiveFades.End() ) break;

		m_ActiveFades.Remove( it );
		m_Fades.DeleteObject( it );
	}
}

AFade* AFadeManager::StartFade( const AFade::Color& a_rFadeFrom, const AFade::Color& a_rFadeTo, TFLOAT a_fFadeTime )
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	if ( m_Fades.GetUsedSize() == m_Fades.GetCapacity() )
	{
		auto pFade = m_ActiveFades.Begin();

		if ( pFade != m_ActiveFades.End() )
		{
			m_ActiveFades.Remove( pFade );
			m_Fades.DeleteObject( pFade );
		}
	}

	TASSERT( m_Fades.GetUsedSize() != m_Fades.GetCapacity() );

	auto pFade = m_Fades.NewObject();
	m_ActiveFades.Push( pFade );

	pFade->SetFadeTime( a_fFadeTime );
	pFade->SetFadeFromColor( a_rFadeFrom );
	pFade->SetFadeToColor( a_rFadeTo );

	auto& rRectangle = pFade->GetRectangleElement();
	rRectangle.Create( fWidth, fHeight );
	rRectangle.SetAttachment( AGUI2Element::Anchor_TopCenter, AGUI2Element::Pivot_TopCenter );
	rRectangle.SetColour( a_rFadeFrom.GetTColor32() );
	rRectangle.SetInFront();

	rRectangle.Unlink();
	AGUI2::GetRootElement()->AddChildTail( rRectangle );
	rRectangle.Show();

	return pFade;
}

TBOOL AFadeManager::HasAnyFadesInProgress( TBOOL a_bThrowFadeOver )
{
	for ( auto it = m_ActiveFades.Begin(); it != m_ActiveFades.End(); it++ )
	{
		if ( it->IsStillFading( a_bThrowFadeOver ) )
		{
			return TTRUE;
		}
	}

	return TFALSE;
}
