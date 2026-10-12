#pragma once
#ifdef BARNYARD_LATEST

#  include "XURData.h"

#  include <Toshi/T2DList.h>
#  include <Math/TVector2.h>

class AGUI2XUIElement;
class AGUI2XUIResource;

// Keyframe times of XUR files are frames at 60 fps
constexpr TFLOAT XUI_FRAME_TIME = 1.0f / 60.0f;

//-----------------------------------------------------------------------------
// Plays the keyframes of one XURXUITimelineData on one child element.
// NOTE: there's no RTTI for the timeline classes, the names follow TXUITimeline
// and TXUITimelineControl of newer TOSHI.
//-----------------------------------------------------------------------------
class AGUI2XUITimeline
    : public Toshi::T2DList<AGUI2XUITimeline>::Node
{
public:
	AGUI2XUITimeline();
	~AGUI2XUITimeline();

	TBOOL Create( AGUI2XUIResource& a_rResource, XURXUITimelineData* a_pData, AGUI2XUIElement* a_pOwner );
	void  SetTime( TBOOL a_bClamp, TFLOAT a_fTime );
	void  Update( TFLOAT a_fDeltaTime );
	void  Reset();

	const TWCHAR*    GetElementId() const { return m_wszElementId; }
	AGUI2XUIElement* GetElement() const { return m_pElement; }
	void             SetElement( AGUI2XUIElement* a_pElement ) { m_pElement = a_pElement; }
	TFLOAT           GetDuration() const { return m_fDuration; }

private:
	void   SetEase( TINT a_iEaseIn, TINT a_iEaseOut, TUINT a_uiEaseScale );
	TFLOAT GetEaseScalar( TFLOAT a_fT );

private:
	const TWCHAR*       m_wszElementId;     // 0x08
	AGUI2XUIElement*    m_pElement;         // 0x0C
	TFLOAT              m_fDuration;        // 0x10
	TFLOAT              m_fTime;            // 0x14
	TINT                m_iKeyframe;        // 0x18
	AGUI2XUIResource*   m_pResource;        // 0x1C
	XURXUITimelineData* m_pData;            // 0x20
	Toshi::TVector2     m_aEaseCurve[ 4 ];  // 0x24 control points of the ease bezier
	TBOOL               m_bKeyframeChanged; // 0x44
};

//-----------------------------------------------------------------------------
// The named frames and timelines of a canvas, group or scene
//-----------------------------------------------------------------------------
class AGUI2XUITimelineControl
{
public:
	// The standard named frames of XUI, see GetStdNamedFrame
	enum STDFRAME
	{
		STDFRAME_NORMAL,
		STDFRAME_ENDNORMAL,
		STDFRAME_NORMALDISABLE,
		STDFRAME_ENDNORMALDISABLE,
		STDFRAME_FOCUS,
		STDFRAME_ENDFOCUS,
		STDFRAME_FOCUSLOOP,
		STDFRAME_FOCUSDISABLE,
		STDFRAME_ENDFOCUSDISABLE,
		STDFRAME_INACTIVEFOCUSLOOP,
		STDFRAME_INITFOCUS,
		STDFRAME_ENDINITFOCUS,
		STDFRAME_INITFOCUSDISABLE,
		STDFRAME_ENDINITFOCUSDISABLE,
		STDFRAME_KILLFOCUS,
		STDFRAME_PRESS,
		STDFRAME_ENDPRESS,
		STDFRAME_NUMOF = 56
	};

	class NamedFrame
	    : public Toshi::T2DList<NamedFrame>::Node
	{
	public:
		TINT          m_iStdFrame; // 0x08 STDFRAME or -1
		const TWCHAR* m_wszName;   // 0x0C
		TUINT32       m_uiTime;    // 0x10
		TUINT32       m_eCommand;  // 0x14 XURXUINamedFrameData::COMMAND
		const TWCHAR* m_wszTarget; // 0x18
	};

public:
	AGUI2XUITimelineControl();
	~AGUI2XUITimelineControl();

	TBOOL Create( AGUI2XUIElement* a_pOwner, AGUI2XUIResource& a_rResource, XURXUIObjectData* a_pData );

	NamedFrame*       FindNamedFrame( const TWCHAR* a_wszName );
	NamedFrame*       FindNextNamedFrame( NamedFrame* a_pFrame );
	AGUI2XUITimeline* FindTimeline( const TWCHAR* a_wszElementId );

	void Reset();
	void SetTime( TBOOL a_bClamp, TFLOAT a_fTime );
	void Tick( TFLOAT a_fDeltaTime );

	AGUI2XUIElement* GetOwner() const { return m_pOwner; }
	TUINT            GetNumNamedFrames() const { return m_llNamedFrames.Size(); }
	TFLOAT           GetTime() const { return m_fTime; }

	TINT GetStdNamedFrame( const TWCHAR* a_wszName );

private:
	TBOOL CreateNamedFrames( AGUI2XUIResource& a_rResource, XURXUIObjectData* a_pData );
	TBOOL CreateTimelines( AGUI2XUIResource& a_rResource, XURXUIObjectData* a_pData );

private:
	AGUI2XUIElement*                 m_pOwner;        // 0x00
	Toshi::T2DList<NamedFrame>       m_llNamedFrames; // 0x04
	Toshi::T2DList<AGUI2XUITimeline> m_llTimelines;   // 0x0C
	TFLOAT                           m_fTime;         // 0x14
	TFLOAT                           m_fDuration;     // 0x18
};

//-----------------------------------------------------------------------------
// Plays the named frames of a timeline control (PlayAnimation, PlayTimelineRange)
// NOTE: there's no RTTI and no matching class in newer TOSHI, the name is made up.
//-----------------------------------------------------------------------------
class AGUI2XUITimelinePlayer
{
public:
	AGUI2XUITimelinePlayer( AGUI2XUITimelineControl* a_pTimelineControl );
	~AGUI2XUITimelinePlayer();

	void Tick( TFLOAT a_fDeltaTime );
	void PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd );
	void PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame );
	void Stop();
	void StopAndReset();

	TFLOAT GetStartTime() const { return m_fStartTime; }
	TFLOAT GetEndTime() const { return m_fEndTime; }

private:
	AGUI2XUITimelineControl::NamedFrame* m_pFrame;           // 0x00
	TFLOAT                               m_fStartTime;       // 0x04
	AGUI2XUITimelineControl::NamedFrame* m_pNextFrame;       // 0x08
	TFLOAT                               m_fTime;            // 0x0C
	TFLOAT                               m_fNextFrameTime;   // 0x10
	AGUI2XUITimelineControl*             m_pTimelineControl; // 0x14
	AGUI2XUITimelineControl::NamedFrame* m_pEndFrame;        // 0x18
	TFLOAT                               m_fEndTime;         // 0x1C
	TFLOAT                               m_fRangeStart;      // 0x20
	TFLOAT                               m_fRangeEnd;        // 0x24
	TBOOL                                m_bPlaying;         // 0x28
};

#endif // BARNYARD_LATEST
