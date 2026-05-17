#pragma once
#include "Sound/ASound.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIPolygon.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>
#include <Math/TMathInline.h>

class AGUI2Timer : public Toshi::T2GUIElement
{
public:
	AGUI2Timer();
	~AGUI2Timer();

	void Create( TFLOAT a_fWidth, TFLOAT a_fHeight );
	void SetDuration( TFLOAT a_fDuration );
	void SetTime( TFLOAT a_fTime );
	void StopCue();
	void SetPaused( TBOOL a_bPaused );
	void Reset();

	TBOOL IsTimerWarning() const;
	TBOOL IsTimerCritical() const;
	TBOOL IsTimerFull() const;
	TBOOL IsTickComplete() const;

	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) OVERRIDE;
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;
	virtual void SetVisible( TBOOL a_bVisible );

private:
	enum TimerCue : TINT
	{
		TimerCue_None     = -1,
		TimerCue_Critical = soundbank::ui::UI_TIME02,
		TimerCue_Warning  = soundbank::ui::UI_TIME01,
		TimerCue_Full     = soundbank::ui::UI_TIMES_UP,
	};

	static constexpr TFLOAT TIMER_WARNING_RATIO      = 0.2f;
	static constexpr TFLOAT TIMER_CRITICAL_RATIO     = 0.1f;
	static constexpr TFLOAT TIMER_TICK_COMPLETE_TIME = 1.0f;
	static constexpr TFLOAT TIMER_TICK_ROTATION      = Toshi::TMath::DegToRad( 5.0f );
	static constexpr TFLOAT TIMER_POLYGON_SCALE      = 1.7f;

	void SetClockColour( TUINT32 a_uiColour, TFLOAT a_fAlpha );
	void ToggleTickRotation();
	void UpdateClockPolygons();
	void UpdatePolygonVertices( Toshi::T2GUIPolygon& a_rPolygon, TFLOAT a_fWidth, TFLOAT a_fHeight );

private:
	Toshi::T2GUIElement        m_oRootElement;
	Toshi::T2GUIRectangle      m_oBacking;
	Toshi::T2GUIPolygon        m_oClockPolygon1;
	Toshi::T2GUIPolygon        m_oClockPolygon2;
	Toshi::T2GUITextureSection* m_pClockTextureSection;
	TFLOAT                     m_fDuration;
	TFLOAT                     m_fTextureWidth;
	TFLOAT                     m_fTextureHeight;
	TimerCue                   m_eTimerCue;
	TINT                       m_iCueIndex;
	TFLOAT                     m_fTimeRatio;
	TFLOAT                     m_fTickAccumulator;
	TFLOAT                     m_fTickElapsed;
	TFLOAT                     m_fTickRotation;
	TFLOAT                     m_fTickInterval;
	TBOOL                      m_bPaused;
};
