#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "XURData.h"
#  include "AGUI2XUIResource.h"
#  include "AGUI2XUIElement.h"
#  include "AGUI2XUIControl.h"

#  include <Math/TVector2.h>
#  include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Values of the two marker bytes used in the timeline property lists
static constexpr TUINT8 XURTIMELINE_PROP_FULL  = 0x83;
static constexpr TUINT8 XURTIMELINE_PROP_SHORT = 0x02;

//-----------------------------------------------------------------------------
// XURHeader
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80410948
XURHeader::XURHeader()
{
	m_pSections     = TNULL;
	m_uiFileID      = 0;
	m_uiVersion     = 0;
	m_usFlags       = 0;
	m_uiFileSize    = 0;
	m_usFlagsHigh   = 0;
	m_usNumSections = 0;
}

// $BarnyardWiiJP: FUNCTION 80410978
XURHeader::~XURHeader()
{
	delete[] m_pSections;
}

//-----------------------------------------------------------------------------
// XURXUINamedFrameData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 804109dc
XURXUINamedFrameData::XURXUINamedFrameData()
{
	m_uiTime   = 0;
	m_usName   = 0;
	m_usTarget = 0;
	m_eCommand = COMMAND_PLAY;
}

// $BarnyardWiiJP: FUNCTION 80414db0
XURXUINamedFrameData::~XURXUINamedFrameData()
{
}

//-----------------------------------------------------------------------------
// XURXUIKeyframeData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80410a00
XURXUIKeyframeData::XURXUIKeyframeData()
{
	m_pValues         = TNULL;
	m_uiTime          = 0;
	m_ucInterpolation = INTERPOLATION_LINEAR;
	m_cEaseIn         = 0;
	// NOTE: the original stores the halfword 50 at 0xE (big-endian), so it is EaseOut = 0 and EaseScale = 50
	m_cEaseOut   = 0;
	m_cEaseScale = 50;
}

// $BarnyardWiiJP: FUNCTION 80410a30
XURXUIKeyframeData::~XURXUIKeyframeData()
{
	delete[] m_pValues;
}

//-----------------------------------------------------------------------------
// XURXUITimelineData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80410a94
XURXUITimelineData::XURXUITimelineData()
{
	m_pOwnerData     = TNULL;
	m_pElementData   = TNULL;
	m_pKeyframes     = TNULL;
	m_pProps         = TNULL;
	m_uiNumProps     = 0;
	m_uiNumKeyframes = 0;
	m_usElementId    = 0;
}

// $BarnyardWiiJP: FUNCTION 80410ac4
XURXUITimelineData::~XURXUITimelineData()
{
	delete[] m_pKeyframes;
	delete[] m_pProps;
}

// $BarnyardWiiJP: FUNCTION 80410b44
TBOOL XURXUITimelineData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	m_usElementId = XURReader::ReadUInt16( a_rpData );
	m_uiNumProps  = XURReader::ReadUInt32( a_rpData );
	m_pProps      = new XURXUITimelineProp[ m_uiNumProps ];

	m_pElementData = m_pOwnerData->FindChildElementData( m_usElementId );

	for ( TUINT32 i = 0; i < m_uiNumProps; i++ )
	{
		TUINT8 ucType = *a_rpData;

		if ( ucType == XURTIMELINE_PROP_FULL )
		{
			a_rpData++;
			m_pProps[ i ].uiProp  = XURReader::ReadUInt32( a_rpData );
			m_pProps[ i ].uiIndex = XURReader::ReadUInt32( a_rpData );
		}
		else if ( ucType == XURTIMELINE_PROP_SHORT )
		{
			m_pProps[ i ].uiProp = XURReader::ReadUInt32( a_rpData );
		}
		else
		{
			// Only three bytes are stored, the lowest one is zero
			m_pProps[ i ].uiProp = 0;
			TUtil::MemCopy( &m_pProps[ i ].uiProp, a_rpData, 3 );
			m_pProps[ i ].uiProp = XURReader::PeekUInt32( TREINTERPRETCAST( TUINT8*, &m_pProps[ i ].uiProp ) );
			a_rpData += 3;
		}

		m_pElementData->ValidateTimelineProp( m_pProps[ i ].uiProp );
	}

	m_uiNumKeyframes = XURReader::ReadUInt32( a_rpData );
	m_pKeyframes     = new XURXUIKeyframeData[ m_uiNumKeyframes ];

	for ( TUINT32 i = 0; i < m_uiNumKeyframes; i++ )
	{
		XURXUIKeyframeData& rKeyframe = m_pKeyframes[ i ];

		rKeyframe.m_uiTime          = XURReader::ReadUInt32( a_rpData );
		rKeyframe.m_ucInterpolation = XURReader::ReadUInt8( a_rpData );
		rKeyframe.m_cEaseIn         = XURReader::ReadUInt8( a_rpData );
		rKeyframe.m_cEaseOut        = XURReader::ReadUInt8( a_rpData );
		rKeyframe.m_cEaseScale      = XURReader::ReadUInt8( a_rpData );
		rKeyframe.m_pValues         = new XURXUIKeyframeData::Value[ m_uiNumProps ];

		for ( TUINT32 k = 0; k < m_uiNumProps; k++ )
		{
			if ( *a_rpData == XURTIMELINE_PROP_SHORT )
				a_rpData++;

			TUINT32 uiSize   = m_pElementData->GetTimelinePropSize( m_pProps[ k ].uiProp );
			TBOOL   bIsFloat = m_pElementData->IsFloatPropType( m_pProps[ k ].uiProp );

			if ( uiSize == 3 ) continue;

			if ( uiSize == 4 )
			{
				if ( bIsFloat == TTRUE )
					rKeyframe.m_pValues[ k ].fValue = XURReader::ReadFloat( a_rpData );
				else
					rKeyframe.m_pValues[ k ].uiValue = XURReader::ReadUInt32( a_rpData );
			}
			else if ( uiSize == 1 )
			{
				rKeyframe.m_pValues[ k ].uiValue = XURReader::ReadUInt8( a_rpData );
			}
			else if ( uiSize == 2 )
			{
				rKeyframe.m_pValues[ k ].uiValue = XURReader::ReadUInt16( a_rpData );
			}
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIObjectData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 8041105c
XURXUIObjectData::XURXUIObjectData()
{
	m_apChildren       = TNULL;
	m_pNamedFrames     = TNULL;
	m_pTimelines       = TNULL;
	m_uiNumChildren    = 0;
	m_uiNumNamedFrames = 0;
	m_uiNumTimelines   = 0;
	m_usPropMasks      = 0;
	m_usClassIndex     = 0;
	m_pClass           = TNULL;
}

// $BarnyardWiiJP: FUNCTION 80411094
XURXUIObjectData::~XURXUIObjectData()
{
	for ( TUINT32 i = 0; i < m_uiNumChildren; i++ )
	{
		if ( m_apChildren[ i ] )
			delete m_apChildren[ i ];
	}

	delete[] m_apChildren;
	delete[] m_pNamedFrames;
	delete[] m_pTimelines;
}

// $BarnyardWiiJP: FUNCTION 8041116c
TBOOL XURXUIObjectData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	m_usPropMasks = XURReader::ReadUInt16( a_rpData );
	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80411194
TBOOL XURXUIObjectData::LoadChildren( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	m_uiNumChildren = XURReader::ReadUInt32( a_rpData );
	m_apChildren    = new XURXUIObjectData*[ m_uiNumChildren ];

	for ( TUINT32 i = 0; i < m_uiNumChildren; i++ )
	{
		TUINT16 usClassIndex = XURReader::ReadUInt16( a_rpData );

		m_apChildren[ i ]                 = AGUI2XUIResource::CreateObjectData( a_rResource, usClassIndex );
		m_apChildren[ i ]->m_usClassIndex = usClassIndex;

		XURXUIObjectData::OBJECTFLAGS ucFlags = XURReader::ReadUInt8( a_rpData );
		m_apChildren[ i ]->Load( a_rResource, a_rpData );

		if ( ucFlags & XURXUIObjectData::OBJECTFLAGS_CHILDREN )
			m_apChildren[ i ]->LoadChildren( a_rResource, a_rpData );

		if ( ucFlags & XURXUIObjectData::OBJECTFLAGS_NAMED_FRAMES )
		{
			m_apChildren[ i ]->LoadNamedFrames( a_rResource, a_rpData );

			if ( ucFlags & XURXUIObjectData::OBJECTFLAGS_CHILDREN )
				m_apChildren[ i ]->LoadTimelines( a_rResource, a_rpData );
		}
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804112fc
TBOOL XURXUIObjectData::LoadNamedFrames( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	m_uiNumNamedFrames = XURReader::ReadUInt32( a_rpData );

	if ( m_uiNumNamedFrames != 0 )
	{
		m_pNamedFrames = new XURXUINamedFrameData[ m_uiNumNamedFrames ];

		for ( TUINT32 i = 0; i < m_uiNumNamedFrames; i++ )
		{
			m_pNamedFrames[ i ].m_usName   = XURReader::ReadUInt16( a_rpData );
			m_pNamedFrames[ i ].m_uiTime   = XURReader::ReadUInt32( a_rpData );
			m_pNamedFrames[ i ].m_eCommand = XURReader::ReadUInt8( a_rpData );
			m_pNamedFrames[ i ].m_usTarget = XURReader::ReadUInt16( a_rpData );
		}
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80411474
TBOOL XURXUIObjectData::LoadTimelines( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	m_uiNumTimelines = XURReader::ReadUInt32( a_rpData );

	if ( m_uiNumTimelines != 0 )
	{
		m_pTimelines = new XURXUITimelineData[ m_uiNumTimelines ];

		for ( TUINT32 i = 0; i < m_uiNumTimelines; i++ )
		{
			m_pTimelines[ i ].m_pOwnerData = this;
			m_pTimelines[ i ].Load( a_rResource, a_rpData );
		}
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80411574
XURXUIElementData* XURXUIObjectData::FindChildElementData( TUINT16 a_usId )
{
	for ( TUINT32 i = 0; i < m_uiNumChildren; i++ )
	{
		XURXUIElementData* pChild = TSTATICCAST( XURXUIElementData, m_apChildren[ i ] );

		if ( pChild->m_usId == a_usId )
			return pChild;
	}

	return TNULL;
}

//-----------------------------------------------------------------------------
// XURXUIElementData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 804115b4
XURXUIElementData::XURXUIElementData()
{
	m_fWidth                    = 60.0f;
	m_fHeight                   = 30.0f;
	m_fOpacity                  = 1.0f;
	m_iPosition                 = -1;
	m_iScale                    = -1;
	m_iRotation                 = -1;
	m_iPivot                    = -1;
	m_uiAnchor                  = 0;
	m_uiBlendMode               = 0;
	m_uiColorWriteFlags         = 0;
	m_usId                      = 0;
	m_bShow                     = TTRUE;
	m_bDisableTimelineRecursion = TFALSE;
}

// $BarnyardWiiJP: FUNCTION 80414d44
XURXUIElementData::~XURXUIElementData()
{
}

// $BarnyardWiiJP: FUNCTION 8041163c
TBOOL XURXUIElementData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIObjectData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK usMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		usMask = XURReader::ReadUInt16( a_rpData );

	if ( usMask & PROPMASK_ID ) m_usId = XURReader::ReadUInt16( a_rpData );
	if ( usMask & PROPMASK_WIDTH ) m_fWidth = XURReader::ReadFloat( a_rpData );
	if ( usMask & PROPMASK_HEIGHT ) m_fHeight = XURReader::ReadFloat( a_rpData );
	if ( usMask & PROPMASK_POSITION ) m_iPosition = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_SCALE ) m_iScale = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_ROTATION ) m_iRotation = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_OPACITY ) m_fOpacity = XURReader::ReadFloat( a_rpData );
	if ( usMask & PROPMASK_ANCHOR ) m_uiAnchor = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_PIVOT ) m_iPivot = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_SHOW ) m_bShow = XURReader::ReadUInt8( a_rpData );
	if ( usMask & PROPMASK_BLEND_MODE ) m_uiBlendMode = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_DISABLE_TIMELINE_RECURSION ) m_bDisableTimelineRecursion = XURReader::ReadUInt8( a_rpData );
	// NOTE: 0x1000 (DesignTime) is never stored
	if ( usMask & PROPMASK_COLOR_WRITE_FLAGS ) m_uiColorWriteFlags = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80411994
TBOOL XURXUIElementData::ValidateTimelineProp( TUINT32 a_uiProp )
{
	return a_uiProp == XURPROP_ELEMENT_WIDTH ||
	    a_uiProp == XURPROP_ELEMENT_HEIGHT ||
	    a_uiProp == XURPROP_ELEMENT_POSITION ||
	    a_uiProp == XURPROP_ELEMENT_SCALE ||
	    a_uiProp == XURPROP_ELEMENT_ROTATION ||
	    a_uiProp == XURPROP_ELEMENT_OPACITY ||
	    a_uiProp == XURPROP_ELEMENT_ANCHOR ||
	    a_uiProp == XURPROP_ELEMENT_PIVOT ||
	    a_uiProp == XURPROP_ELEMENT_SHOW ||
	    a_uiProp == XURPROP_ELEMENT_BLENDMODE ||
	    a_uiProp == XURPROP_ELEMENT_DISABLETIMELINERECURSION ||
	    a_uiProp == XURPROP_ELEMENT_BRUSHFLAGS ||
	    a_uiProp == XURPROP_ELEMENT_COLORWRITEFLAGS;
}

// $BarnyardWiiJP: FUNCTION 80411a10
TUINT32 XURXUIElementData::TranslateTimelineProp( const TCHAR* a_szName )
{
	if ( TStringManager::String8Compare( a_szName, "Width" ) == 0 ) return XURPROP_ELEMENT_WIDTH;
	if ( TStringManager::String8Compare( a_szName, "Height" ) == 0 ) return XURPROP_ELEMENT_HEIGHT;
	if ( TStringManager::String8Compare( a_szName, "Position" ) == 0 ) return XURPROP_ELEMENT_POSITION;
	if ( TStringManager::String8Compare( a_szName, "Scale" ) == 0 ) return XURPROP_ELEMENT_SCALE;
	if ( TStringManager::String8Compare( a_szName, "Rotation" ) == 0 ) return XURPROP_ELEMENT_ROTATION;
	if ( TStringManager::String8Compare( a_szName, "Opacity" ) == 0 ) return XURPROP_ELEMENT_OPACITY;
	if ( TStringManager::String8Compare( a_szName, "Anchor" ) == 0 ) return XURPROP_ELEMENT_ANCHOR;
	if ( TStringManager::String8Compare( a_szName, "Pivot" ) == 0 ) return XURPROP_ELEMENT_PIVOT;
	if ( TStringManager::String8Compare( a_szName, "Show" ) == 0 ) return XURPROP_ELEMENT_SHOW;
	if ( TStringManager::String8Compare( a_szName, "BlendMode" ) == 0 ) return XURPROP_ELEMENT_BLENDMODE;
	if ( TStringManager::String8Compare( a_szName, "DisableTimelineRecursion" ) == 0 ) return XURPROP_ELEMENT_DISABLETIMELINERECURSION;
	if ( TStringManager::String8Compare( a_szName, "BrushFlags" ) == 0 ) return XURPROP_ELEMENT_BRUSHFLAGS;

	return ( TStringManager::String8Compare( a_szName, "ColorWriteFlags" ) == 0 ) ? XURPROP_ELEMENT_COLORWRITEFLAGS : 0;
}

// $BarnyardWiiJP: FUNCTION 80411c20
TUINT32 XURXUIElementData::GetTimelinePropSize( TUINT32 a_uiProp )
{
	if ( a_uiProp == XURPROP_ELEMENT_DISABLETIMELINERECURSION || a_uiProp == XURPROP_ELEMENT_SHOW )
		return 1;

	return 4;
}

// $BarnyardWiiJP: FUNCTION 80411c44
TBOOL XURXUIElementData::IsFloatPropType( TUINT32 a_uiProp )
{
	return a_uiProp == XURPROP_ELEMENT_WIDTH || a_uiProp == XURPROP_ELEMENT_HEIGHT || a_uiProp == XURPROP_ELEMENT_OPACITY;
}

//-----------------------------------------------------------------------------
// XURXUIControlData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80411c70
XURXUIControlData::XURXUIControlData()
{
	m_usClassOverride   = 0;
	m_usVisual          = 0;
	m_usText            = 0;
	m_usImagePath       = 0;
	m_bEnabled          = TTRUE;
	m_bUnfocussedInput  = TFALSE;
	m_aNav[ NAV_UP ]    = 0;
	m_aNav[ NAV_DOWN ]  = 0;
	m_aNav[ NAV_LEFT ]  = 0;
	m_aNav[ NAV_RIGHT ] = 0;
}

// $BarnyardWiiJP: FUNCTION 80414e00
XURXUIControlData::~XURXUIControlData()
{
}

// $BarnyardWiiJP: FUNCTION 80411cdc
TBOOL XURXUIControlData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK usMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		usMask = XURReader::ReadUInt16( a_rpData );

	if ( usMask & PROPMASK_CLASS_OVERRIDE ) m_usClassOverride = XURReader::ReadUInt16( a_rpData );
	if ( usMask & PROPMASK_VISUAL ) m_usVisual = XURReader::ReadUInt16( a_rpData );
	if ( usMask & PROPMASK_ENABLED ) m_bEnabled = XURReader::ReadUInt8( a_rpData );
	if ( usMask & PROPMASK_UNFOCUSSED_INPUT ) m_bUnfocussedInput = XURReader::ReadUInt8( a_rpData );
	if ( usMask & PROPMASK_NAV_UP ) m_aNav[ NAV_UP ] = XURReader::ReadUInt16( a_rpData );
	if ( usMask & PROPMASK_NAV_DOWN ) m_aNav[ NAV_DOWN ] = XURReader::ReadUInt16( a_rpData );
	if ( usMask & PROPMASK_NAV_LEFT ) m_aNav[ NAV_LEFT ] = XURReader::ReadUInt16( a_rpData );
	if ( usMask & PROPMASK_NAV_RIGHT ) m_aNav[ NAV_RIGHT ] = XURReader::ReadUInt16( a_rpData );

	// NOTE: the original reads both of these as halfwords but only skips one byte for each
	if ( usMask & PROPMASK_TEXT )
	{
		m_usText = XURReader::PeekUInt16( a_rpData );
		a_rpData += 1;
	}

	if ( usMask & PROPMASK_IMAGE_PATH )
	{
		m_usImagePath = XURReader::PeekUInt16( a_rpData );
		a_rpData += 1;
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80411edc
TBOOL XURXUIControlData::ValidateTimelineProp( TUINT32 a_uiProp )
{
	return a_uiProp == XURPROP_CONTROL_ELEMENT_WIDTH ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_HEIGHT ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_POSITION ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_SCALE ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_ROTATION ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_OPACITY ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_ANCHOR ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_PIVOT ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_SHOW ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_BLENDMODE ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_DISABLETIMELINERECURSION ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_BRUSHFLAGS ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_COLORWRITEFLAGS ||
	    a_uiProp == XURPROP_CONTROL_VISUAL ||
	    a_uiProp == XURPROP_CONTROL_ENABLED ||
	    a_uiProp == XURPROP_CONTROL_UNFOCUSSEDINPUT ||
	    a_uiProp == XURPROP_CONTROL_NAVLEFT ||
	    a_uiProp == XURPROP_CONTROL_NAVRIGHT ||
	    a_uiProp == XURPROP_CONTROL_NAVUP ||
	    a_uiProp == XURPROP_CONTROL_NAVDOWN ||
	    a_uiProp == XURPROP_CONTROL_TEXT ||
	    a_uiProp == 0x01010900 ||
	    a_uiProp == XURPROP_CONTROL_IMAGEPATH ||
	    XURXUIElementData::ValidateTimelineProp( a_uiProp );
}

// $BarnyardWiiJP: FUNCTION 80411fac
TUINT32 XURXUIControlData::TranslateTimelineProp( const TCHAR* a_szName )
{
	if ( TStringManager::String8Compare( a_szName, "Width" ) == 0 ) return XURPROP_CONTROL_ELEMENT_WIDTH;
	if ( TStringManager::String8Compare( a_szName, "Height" ) == 0 ) return XURPROP_CONTROL_ELEMENT_HEIGHT;
	if ( TStringManager::String8Compare( a_szName, "Position" ) == 0 ) return XURPROP_CONTROL_ELEMENT_POSITION;
	if ( TStringManager::String8Compare( a_szName, "Scale" ) == 0 ) return XURPROP_CONTROL_ELEMENT_SCALE;
	if ( TStringManager::String8Compare( a_szName, "Rotation" ) == 0 ) return XURPROP_CONTROL_ELEMENT_ROTATION;
	if ( TStringManager::String8Compare( a_szName, "Opacity" ) == 0 ) return XURPROP_CONTROL_ELEMENT_OPACITY;
	if ( TStringManager::String8Compare( a_szName, "Anchor" ) == 0 ) return XURPROP_CONTROL_ELEMENT_ANCHOR;
	if ( TStringManager::String8Compare( a_szName, "Pivot" ) == 0 ) return XURPROP_CONTROL_ELEMENT_PIVOT;
	if ( TStringManager::String8Compare( a_szName, "Show" ) == 0 ) return XURPROP_CONTROL_ELEMENT_SHOW;
	if ( TStringManager::String8Compare( a_szName, "BlendMode" ) == 0 ) return XURPROP_CONTROL_ELEMENT_BLENDMODE;
	if ( TStringManager::String8Compare( a_szName, "DisableTimelineRecursion" ) == 0 ) return XURPROP_CONTROL_ELEMENT_DISABLETIMELINERECURSION;
	if ( TStringManager::String8Compare( a_szName, "BrushFlags" ) == 0 ) return XURPROP_CONTROL_ELEMENT_BRUSHFLAGS;
	if ( TStringManager::String8Compare( a_szName, "ColorWriteFlags" ) == 0 ) return XURPROP_CONTROL_ELEMENT_COLORWRITEFLAGS;
	if ( TStringManager::String8Compare( a_szName, "Visual" ) == 0 ) return XURPROP_CONTROL_VISUAL;
	if ( TStringManager::String8Compare( a_szName, "Enabled" ) == 0 ) return XURPROP_CONTROL_ENABLED;
	if ( TStringManager::String8Compare( a_szName, "UnfocussedInput" ) == 0 ) return XURPROP_CONTROL_UNFOCUSSEDINPUT;
	if ( TStringManager::String8Compare( a_szName, "NavLeft" ) == 0 ) return XURPROP_CONTROL_NAVLEFT;
	if ( TStringManager::String8Compare( a_szName, "NavRight" ) == 0 ) return XURPROP_CONTROL_NAVRIGHT;
	if ( TStringManager::String8Compare( a_szName, "NavUp" ) == 0 ) return XURPROP_CONTROL_NAVUP;
	if ( TStringManager::String8Compare( a_szName, "NavDown" ) == 0 ) return XURPROP_CONTROL_NAVDOWN;
	if ( TStringManager::String8Compare( a_szName, "Text" ) == 0 ) return XURPROP_CONTROL_TEXT;
	if ( TStringManager::String8Compare( a_szName, "ImagePath" ) == 0 ) return XURPROP_CONTROL_IMAGEPATH;

	return XURXUIElementData::TranslateTimelineProp( a_szName );
}

// $BarnyardWiiJP: FUNCTION 80412314
TUINT32 XURXUIControlData::GetTimelinePropSize( TUINT32 a_uiProp )
{
	if ( a_uiProp == XURPROP_CONTROL_IMAGEPATH )
		return 2;

	return XURXUIElementData::GetTimelinePropSize( a_uiProp );
}

// $BarnyardWiiJP: FUNCTION 80412330
TBOOL XURXUIControlData::IsFloatPropType( TUINT32 a_uiProp )
{
	return a_uiProp == XURPROP_CONTROL_ELEMENT_WIDTH ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_HEIGHT ||
	    a_uiProp == XURPROP_CONTROL_ELEMENT_OPACITY ||
	    XURXUIElementData::IsFloatPropType( a_uiProp );
}

//-----------------------------------------------------------------------------
// XURXUISceneData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 8041235c
XURXUISceneData::XURXUISceneData()
{
	m_uiUnk1          = 0;
	m_usDefaultFocus  = 0;
	m_bIgnorePresses  = TFALSE;
	m_usTransFrom     = 0;
	m_usTransTo       = 0;
	m_usTransBackFrom = 0;
	m_usTransBackTo   = 0;
	m_pClass          = TClass::Find( "AGUI2XUIScene", &TGetClass( AGUI2XUIControl ) );
}

// $BarnyardWiiJP: FUNCTION 80414fdc
XURXUISceneData::~XURXUISceneData()
{
}

// $BarnyardWiiJP: FUNCTION 804123d0
TBOOL XURXUISceneData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIControlData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_DEFAULT_FOCUS ) m_usDefaultFocus = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_TRANS_FROM ) m_usTransFrom = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_TRANS_TO ) m_usTransTo = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_TRANS_BACK_FROM ) m_usTransBackFrom = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_TRANS_BACK_TO ) m_usTransBackTo = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_UNK5 ) m_uiUnk1 = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_IGNORE_PRESSES ) m_bIgnorePresses = XURReader::ReadUInt8( a_rpData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041256c
TBOOL XURXUISceneData::ValidateTimelineProp( TUINT32 a_uiProp )
{
	// NOTE: 0x01010500 is in the original list, probably meant to be 0x01000500
	return a_uiProp == XURPROP_SCENE_TRANSFROM ||
	    a_uiProp == XURPROP_SCENE_TRANSTO ||
	    a_uiProp == XURPROP_SCENE_TRANSBACKFROM ||
	    a_uiProp == XURPROP_SCENE_TRANSBACKTO ||
	    a_uiProp == 0x01010500 ||
	    a_uiProp == XURPROP_SCENE_IGNOREPRESSES ||
	    XURXUIControlData::ValidateTimelineProp( a_uiProp );
}

// $BarnyardWiiJP: FUNCTION 804125b4
TUINT32 XURXUISceneData::TranslateTimelineProp( const TCHAR* a_szName )
{
	if ( TStringManager::String8Compare( a_szName, "TransFrom" ) == 0 ) return XURPROP_SCENE_TRANSFROM;
	if ( TStringManager::String8Compare( a_szName, "TransTo" ) == 0 ) return XURPROP_SCENE_TRANSTO;
	if ( TStringManager::String8Compare( a_szName, "TransBackFrom" ) == 0 ) return XURPROP_SCENE_TRANSBACKFROM;
	if ( TStringManager::String8Compare( a_szName, "TransBackTo" ) == 0 ) return XURPROP_SCENE_TRANSBACKTO;
	if ( TStringManager::String8Compare( a_szName, "IgnorePresses" ) == 0 ) return XURPROP_SCENE_IGNOREPRESSES;

	return XURXUIControlData::TranslateTimelineProp( a_szName );
}

// $BarnyardWiiJP: FUNCTION 804126b8
TUINT32 XURXUISceneData::GetTimelinePropSize( TUINT32 a_uiProp )
{
	if ( a_uiProp == XURPROP_SCENE_IGNOREPRESSES )
		return 1;

	return XURXUIControlData::GetTimelinePropSize( a_uiProp );
}

//-----------------------------------------------------------------------------
// XURXUICanvasData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 804126d4
XURXUICanvasData::XURXUICanvasData()
{
	m_pClass = TClass::Find( "AGUI2XUICanvas", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80414ce8
XURXUICanvasData::~XURXUICanvasData()
{
}

// $BarnyardWiiJP: FUNCTION 80412728
TBOOL XURXUICanvasData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );
	a_rpData += 1;

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIFigureData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80412a1c
XURXUIFigureData::XURXUIFigureData()
{
	// NOTE: m_bClosed isn't initialised by the original
	m_fPathWidth  = TVector2::VEC_ZERO.x;
	m_fPathHeight = TVector2::VEC_ZERO.y;
	m_uiNumPoints = 0;
	m_pStroke     = TNULL;
	m_pFill       = TNULL;
	m_pPoints     = TNULL;
	m_pClass      = TClass::Find( "AGUI2XUIFigure", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80412a94
XURXUIFigureData::~XURXUIFigureData()
{
	if ( m_pStroke )
		delete m_pStroke;

	if ( m_pFill )
		delete m_pFill;

	delete[] m_pPoints;
}

// $BarnyardWiiJP: FUNCTION 80412b5c
TBOOL XURXUIFigureData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	// NOTE: unlike the other objects, the mask is always stored here
	PROPMASK ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_STROKE )
	{
		m_pStroke = new XURXUIStrokeData();
		m_pStroke->Load( a_rResource, a_rpData );
	}

	if ( ucMask & PROPMASK_FILL )
	{
		m_pFill = new XURXUIFillData();
		m_pFill->Load( a_rResource, a_rpData );
	}

	if ( ucMask & PROPMASK_CLOSED )
		m_bClosed = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_PATH )
	{
		// The points are stored in the custom data section
		TUINT8* pPath = a_rResource.GetCustData() + XURReader::ReadUInt32( a_rpData );

		m_fPathWidth  = XURReader::PeekFloat( pPath + 4 );
		m_fPathHeight = XURReader::PeekFloat( pPath + 8 );
		m_uiNumPoints = XURReader::PeekUInt32( pPath + 12 );

		TUINT8* pPoint = pPath + 16;

		if ( m_uiNumPoints != 0 )
		{
			m_pPoints = new Point[ m_uiNumPoints ];

			for ( TUINT32 i = 0; i < m_uiNumPoints; i++ )
			{
				m_pPoints[ i ].fX      = XURReader::ReadFloat( pPoint );
				m_pPoints[ i ].fY      = XURReader::ReadFloat( pPoint );
				m_pPoints[ i ].fCtrl1X = XURReader::ReadFloat( pPoint );
				m_pPoints[ i ].fCtrl1Y = XURReader::ReadFloat( pPoint );
				m_pPoints[ i ].fCtrl2X = XURReader::ReadFloat( pPoint );
				m_pPoints[ i ].fCtrl2Y = XURReader::ReadFloat( pPoint );
			}

			for ( TUINT32 i = 0; i < m_uiNumPoints; i++ )
			{
				TUINT32 uiPrev = i - 1;
				if ( TINT( i - 1 ) < 0 )
					uiPrev += m_uiNumPoints;

				Point& rPoint = m_pPoints[ i ];
				Point& rPrev  = m_pPoints[ uiPrev ];

				TFLOAT fCtrlX = rPoint.fCtrl1X - rPoint.fX;
				TFLOAT fCtrlY = rPoint.fCtrl1Y - rPoint.fY;

				// The segment is curved when either of the control points differs from the point
				if ( 0.0f != fCtrlX * fCtrlX + fCtrlY * fCtrlY )
				{
					rPoint.bIsCurved = TTRUE;
				}
				else
				{
					TFLOAT fPrevX = rPoint.fX - rPrev.fCtrl2X;
					TFLOAT fPrevY = rPoint.fY - rPrev.fCtrl2Y;

					rPoint.bIsCurved = 0.0f != fPrevX * fPrevX + fPrevY * fPrevY;
				}
			}
		}
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 80412f74
TBOOL XURXUIFigureData::ValidateTimelineProp( TUINT32 a_uiProp )
{
	return a_uiProp == XURPROP_FIGURE_CLOSED ||
	    a_uiProp == XURPROP_FIGURE_STROKE_STROKEWIDTH ||
	    a_uiProp == XURPROP_FIGURE_STROKE_STROKECOLOR ||
	    a_uiProp == XURPROP_FIGURE_FILL_FILLTYPE ||
	    a_uiProp == XURPROP_FIGURE_FILL_FILLCOLOR ||
	    a_uiProp == 0x02000102 ||
	    a_uiProp == 0x02000103 ||
	    a_uiProp == XURPROP_FIGURE_FILL_TRANSLATION ||
	    a_uiProp == XURPROP_FIGURE_FILL_SCALE ||
	    a_uiProp == 0x02000106 ||
	    a_uiProp == XURPROP_FIGURE_FILL_WRAPX ||
	    a_uiProp == XURPROP_FIGURE_FILL_WRAPY ||
	    ( a_uiProp - XURPROP_FIGURE_FILL_GRADIENT_STOPCOLOR ) <= 1 ||
	    XURXUIElementData::ValidateTimelineProp( a_uiProp );
}

// $BarnyardWiiJP: FUNCTION 80412ffc
TUINT32 XURXUIFigureData::TranslateTimelineProp( const TCHAR* a_szName )
{
	if ( TStringManager::String8Compare( a_szName, "Closed" ) == 0 ) return XURPROP_FIGURE_CLOSED;
	if ( TStringManager::String8Compare( a_szName, "Stroke.StrokeWidth" ) == 0 ) return XURPROP_FIGURE_STROKE_STROKEWIDTH;
	if ( TStringManager::String8Compare( a_szName, "Stroke.StrokeColor" ) == 0 ) return XURPROP_FIGURE_STROKE_STROKECOLOR;
	if ( TStringManager::String8Compare( a_szName, "Fill.FillType" ) == 0 ) return XURPROP_FIGURE_FILL_FILLTYPE;
	if ( TStringManager::String8Compare( a_szName, "Fill.FillColor" ) == 0 ) return XURPROP_FIGURE_FILL_FILLCOLOR;
	if ( TStringManager::String8Compare( a_szName, "Fill.Translation" ) == 0 ) return XURPROP_FIGURE_FILL_TRANSLATION;
	if ( TStringManager::String8Compare( a_szName, "Fill.Scale" ) == 0 ) return XURPROP_FIGURE_FILL_SCALE;
	if ( TStringManager::String8Compare( a_szName, "Fill.WrapX" ) == 0 ) return XURPROP_FIGURE_FILL_WRAPX;
	if ( TStringManager::String8Compare( a_szName, "Fill.WrapY" ) == 0 ) return XURPROP_FIGURE_FILL_WRAPY;
	if ( TStringManager::String8Compare( a_szName, "Fill.BrushFlags" ) == 0 ) return XURPROP_FIGURE_FILL_BRUSHFLAGS;
	if ( TStringManager::String8Compare( a_szName, "Fill.Gradient.StopPos" ) == 0 ) return XURPROP_FIGURE_FILL_GRADIENT_STOPPOS;
	if ( TStringManager::String8Compare( a_szName, "Fill.Gradient.StopColor" ) == 0 ) return XURPROP_FIGURE_FILL_GRADIENT_STOPCOLOR;

	return XURXUIElementData::TranslateTimelineProp( a_szName );
}

// $BarnyardWiiJP: FUNCTION 804131f8
TUINT32 XURXUIFigureData::GetTimelinePropSize( TUINT32 a_uiProp )
{
	if ( a_uiProp == XURPROP_FIGURE_CLOSED || a_uiProp == XURPROP_ELEMENT_DISABLETIMELINERECURSION || a_uiProp == XURPROP_ELEMENT_SHOW )
		return 1;

	if ( ( a_uiProp - XURPROP_FIGURE_FILL_WRAPX ) > 1 )
		return XURXUIElementData::GetTimelinePropSize( a_uiProp );

	return 2;
}

// $BarnyardWiiJP: FUNCTION 80413240
TBOOL XURXUIFigureData::IsFloatPropType( TUINT32 a_uiProp )
{
	return a_uiProp == XURPROP_FIGURE_FILL_GRADIENT_STOPPOS || XURXUIElementData::IsFloatPropType( a_uiProp );
}

//-----------------------------------------------------------------------------
// XURXUIFillData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 8041325c
XURXUIFillData::XURXUIFillData()
{
	m_uiFillType        = 1;
	m_uiFillColor       = 0xFF0F0F80;
	m_usTextureFileName = 0;
	m_pGradient         = TNULL;
	m_iTranslation      = -1;
	m_iScale            = -1;
	m_fRotation         = 0.0f;
	m_uiWrapX           = 0;
	m_uiWrapY           = 0;
}

// $BarnyardWiiJP: FUNCTION 804132d0
XURXUIFillData::~XURXUIFillData()
{
	if ( m_pGradient )
		delete m_pGradient;
}

// $BarnyardWiiJP: FUNCTION 8041335c
TBOOL XURXUIFillData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIObjectData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK usMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		usMask = XURReader::ReadUInt16( a_rpData );

	if ( usMask & PROPMASK_FILL_TYPE ) m_uiFillType = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_FILL_COLOR ) m_uiFillColor = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_TEXTURE_FILE_NAME ) m_usTextureFileName = XURReader::ReadUInt16( a_rpData );

	if ( usMask & PROPMASK_GRADIENT )
	{
		m_pGradient = new XURXUIGradientData();
		m_pGradient->Load( a_rResource, a_rpData );
	}

	if ( usMask & PROPMASK_TRANSLATION ) m_iTranslation = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_SCALE ) m_iScale = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_ROTATION ) m_fRotation = XURReader::ReadFloat( a_rpData );
	if ( usMask & PROPMASK_WRAP_X ) m_uiWrapX = XURReader::ReadUInt32( a_rpData );
	if ( usMask & PROPMASK_WRAP_Y ) m_uiWrapY = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIStrokeData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80413608
XURXUIStrokeData::XURXUIStrokeData()
{
	m_fStrokeWidth  = 0.0f;
	m_uiStrokeColor = 0xFF0F0F80;
}

// $BarnyardWiiJP: FUNCTION 80413654
XURXUIStrokeData::~XURXUIStrokeData()
{
}

// $BarnyardWiiJP: FUNCTION 804136ac
TBOOL XURXUIStrokeData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIObjectData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_STROKE_WIDTH ) m_fStrokeWidth = XURReader::ReadFloat( a_rpData );
	if ( ucMask & PROPMASK_STROKE_COLOR ) m_uiStrokeColor = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIGradientData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 804137a4
XURXUIGradientData::XURXUIGradientData()
{
	m_bRadial     = TFALSE;
	m_uiNumStops  = 0;
	m_pStopColors = TNULL;
	m_pStopPos    = TNULL;
}

// $BarnyardWiiJP: FUNCTION 804137f4
XURXUIGradientData::~XURXUIGradientData()
{
	delete[] m_pStopColors;
	delete[] m_pStopPos;
}

// $BarnyardWiiJP: FUNCTION 8041386c
TBOOL XURXUIGradientData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIObjectData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_RADIAL ) m_bRadial = XURReader::ReadUInt8( a_rpData );
	if ( ucMask & PROPMASK_NUM_STOPS ) m_uiNumStops = XURReader::ReadUInt32( a_rpData );

	if ( ucMask & PROPMASK_STOP_COLORS )
	{
		TUINT8 ucNumColors = XURReader::ReadUInt8( a_rpData );
		m_pStopColors      = new TUINT32[ ucNumColors ];

		for ( TUINT8 i = 0; i < ucNumColors; i++ )
			m_pStopColors[ i ] = XURReader::ReadUInt32( a_rpData );
	}

	if ( ucMask & PROPMASK_STOP_POS )
	{
		TUINT8 ucNumPos = XURReader::ReadUInt8( a_rpData );
		m_pStopPos      = new TFLOAT[ ucNumPos ];

		for ( TUINT8 i = 0; i < ucNumPos; i++ )
			m_pStopPos[ i ] = XURReader::ReadFloat( a_rpData );
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIVisualData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80413e90
XURXUIVisualData::XURXUIVisualData()
{
	m_pClass = TClass::Find( "AGUI2XUIVisual", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80414f70
XURXUIVisualData::~XURXUIVisualData()
{
}

// $BarnyardWiiJP: FUNCTION 80413ee4
TBOOL XURXUIVisualData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );
	a_rpData += 1;

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIImageData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80413f20
XURXUIImageData::XURXUIImageData()
{
	m_usImagePath  = 0;
	m_uiSizeMode   = 0;
	m_uiBrushFlags = 0;
	m_pClass       = TClass::Find( "AGUI2XUIImage", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80413f84
XURXUIImageData::~XURXUIImageData()
{
}

// $BarnyardWiiJP: FUNCTION 80413fe0
TBOOL XURXUIImageData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_SIZE_MODE ) m_uiSizeMode = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_IMAGE_PATH ) m_usImagePath = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_BRUSH_FLAGS ) m_uiBrushFlags = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 804140f8
TBOOL XURXUIImageData::ValidateTimelineProp( TUINT32 a_uiProp )
{
	return a_uiProp == XURPROP_IMAGE_IMAGEPATH ||
	    a_uiProp == XURPROP_IMAGE_SIZEMODE ||
	    a_uiProp == XURPROP_IMAGE_BRUSHFLAGS ||
	    XURXUIElementData::ValidateTimelineProp( a_uiProp );
}

// $BarnyardWiiJP: FUNCTION 80414124
TUINT32 XURXUIImageData::TranslateTimelineProp( const TCHAR* a_szName )
{
	if ( TStringManager::String8Compare( a_szName, "ImagePath" ) == 0 ) return XURPROP_IMAGE_IMAGEPATH;
	if ( TStringManager::String8Compare( a_szName, "SizeMode" ) == 0 ) return XURPROP_IMAGE_SIZEMODE;
	if ( TStringManager::String8Compare( a_szName, "BrushFlags" ) == 0 ) return XURPROP_IMAGE_BRUSHFLAGS;

	return XURXUIElementData::TranslateTimelineProp( a_szName );
}

// $BarnyardWiiJP: FUNCTION 804141d8
TUINT32 XURXUIImageData::GetTimelinePropSize( TUINT32 a_uiProp )
{
	if ( a_uiProp == XURPROP_IMAGE_IMAGEPATH )
		return 2;

	return XURXUIElementData::GetTimelinePropSize( a_uiProp );
}

//-----------------------------------------------------------------------------
// XURXUITextData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 804141f4
XURXUITextData::XURXUITextData()
{
	m_usText              = 0;
	m_uiTextColor         = 0xFF000000;
	m_uiDropShadowColor   = 0x80000000;
	m_uiPointSize         = 14;
	m_usFont              = 0;
	m_uiTextStyle         = 0x100;
	m_uiLineSpacingAdjust = 0;
	m_pClass              = TClass::Find( "AGUI2XUIText", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80414278
XURXUITextData::~XURXUITextData()
{
}

// $BarnyardWiiJP: FUNCTION 804142d4
TBOOL XURXUITextData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_TEXT ) m_usText = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_TEXT_COLOR ) m_uiTextColor = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_DROP_SHADOW_COLOR ) m_uiDropShadowColor = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_POINT_SIZE ) m_uiPointSize = TUINT32( XURReader::ReadFloat( a_rpData ) );
	if ( ucMask & PROPMASK_FONT ) m_usFont = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_TEXT_STYLE ) m_uiTextStyle = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_LINE_SPACING_ADJUST ) m_uiLineSpacingAdjust = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIButtonData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 804144e8
XURXUIButtonData::XURXUIButtonData()
{
	m_uiUnk1 = 0;
	m_pClass = TClass::Find( "AGUI2XUIButton", &TGetClass( AGUI2XUIControl ) );
}

// $BarnyardWiiJP: FUNCTION 80414544
XURXUIButtonData::~XURXUIButtonData()
{
}

// $BarnyardWiiJP: FUNCTION 804145a4
TBOOL XURXUIButtonData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIControlData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_UNK0 ) m_uiUnk1 = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIGroupData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80414658
XURXUIGroupData::XURXUIGroupData()
{
	m_pClass = TClass::Find( "AGUI2XUIGroup", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80414f14
XURXUIGroupData::~XURXUIGroupData()
{
}

// $BarnyardWiiJP: FUNCTION 804146ac
TBOOL XURXUIGroupData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );
	a_rpData += 1;

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUIImagePresenterData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 804146e8
XURXUIImagePresenterData::XURXUIImagePresenterData()
{
	m_uiDataAssociation = 0;
	m_uiSizeMode        = 0;
	m_uiBrushFlags      = 0;
	m_pClass            = TClass::Find( "AGUI2XUIImagePresenter", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80414eb8
XURXUIImagePresenterData::~XURXUIImagePresenterData()
{
}

// $BarnyardWiiJP: FUNCTION 8041474c
TBOOL XURXUIImagePresenterData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_SIZE_MODE ) m_uiSizeMode = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_DATA_ASSOCIATION ) m_uiDataAssociation = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_BRUSH_FLAGS ) m_uiBrushFlags = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUITextPresenterData
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 80414888
XURXUITextPresenterData::XURXUITextPresenterData()
{
	m_uiDataAssociation   = 0;
	m_uiTextColor         = 0xFF000000;
	m_uiDropShadowColor   = 0x80000000;
	m_uiPointSize         = 14;
	m_usFont              = 0;
	m_uiTextStyle         = 0x100;
	m_uiLineSpacingAdjust = 0;
	m_pClass              = TClass::Find( "AGUI2XUITextPresenter", &TGetClass( AGUI2XUIElement ) );
}

// $BarnyardWiiJP: FUNCTION 80414e5c
XURXUITextPresenterData::~XURXUITextPresenterData()
{
}

// $BarnyardWiiJP: FUNCTION 8041490c
TBOOL XURXUITextPresenterData::Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData )
{
	XURXUIElementData::Load( a_rResource, a_rpData );

	if ( XURReader::ReadUInt8( a_rpData ) == 0 )
		return TTRUE;

	PROPMASK ucMask = PROPMASK_NONE;
	if ( m_usPropMasks != 0 )
		ucMask = XURReader::ReadUInt8( a_rpData );

	if ( ucMask & PROPMASK_TEXT_COLOR ) m_uiTextColor = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_DROP_SHADOW_COLOR ) m_uiDropShadowColor = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_POINT_SIZE ) m_uiPointSize = TUINT32( XURReader::ReadFloat( a_rpData ) );
	if ( ucMask & PROPMASK_FONT ) m_usFont = XURReader::ReadUInt16( a_rpData );
	if ( ucMask & PROPMASK_TEXT_STYLE ) m_uiTextStyle = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_LINE_SPACING_ADJUST ) m_uiLineSpacingAdjust = XURReader::ReadUInt32( a_rpData );
	if ( ucMask & PROPMASK_DATA_ASSOCIATION ) m_uiDataAssociation = XURReader::ReadUInt32( a_rpData );

	return TTRUE;
}

#endif // BARNYARD_LATEST
