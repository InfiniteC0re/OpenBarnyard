#pragma once
//-----------------------------------------------------------------------------
// XUR (binary XUI resource) data structures of the Wii build.
// The Wii RTTI names them without a namespace (XURHeader, XURXUIObjectData, ...).
//
// NOTE: the XUR payload is big-endian on every platform (only the TRB container
// that holds it is converted). The retail loaders read every field with explicit
// byte shifts, so they work on a little-endian PC too. See XURReader below.
//-----------------------------------------------------------------------------
#ifdef BARNYARD_LATEST

#  include <Toshi/TClass.h>

class AGUI2XUIResource;
class XURXUIElementData;

//-----------------------------------------------------------------------------
// Big-endian readers, these match the byte-by-byte reads of the original
//-----------------------------------------------------------------------------
namespace XURReader
{
inline TUINT8 ReadUInt8( TUINT8*& a_rpData )
{
	return *a_rpData++;
}

inline TUINT16 ReadUInt16( TUINT8*& a_rpData )
{
	TUINT16 uiValue = TUINT16( ( a_rpData[ 0 ] << 8 ) | a_rpData[ 1 ] );
	a_rpData += 2;
	return uiValue;
}

inline TUINT32 ReadUInt32( TUINT8*& a_rpData )
{
	TUINT32 uiValue = ( TUINT32( a_rpData[ 0 ] ) << 24 ) | ( TUINT32( a_rpData[ 1 ] ) << 16 ) | ( TUINT32( a_rpData[ 2 ] ) << 8 ) | TUINT32( a_rpData[ 3 ] );
	a_rpData += 4;
	return uiValue;
}

inline TFLOAT ReadFloat( TUINT8*& a_rpData )
{
	TUINT32 uiValue = ReadUInt32( a_rpData );
	return *TREINTERPRETCAST( TFLOAT*, &uiValue );
}

// Reads without moving the pointer
inline TUINT16 PeekUInt16( const TUINT8* a_pData )
{
	return TUINT16( ( a_pData[ 0 ] << 8 ) | a_pData[ 1 ] );
}

inline TUINT32 PeekUInt32( const TUINT8* a_pData )
{
	return ( TUINT32( a_pData[ 0 ] ) << 24 ) | ( TUINT32( a_pData[ 1 ] ) << 16 ) | ( TUINT32( a_pData[ 2 ] ) << 8 ) | TUINT32( a_pData[ 3 ] );
}

inline TFLOAT PeekFloat( const TUINT8* a_pData )
{
	TUINT32 uiValue = PeekUInt32( a_pData );
	return *TREINTERPRETCAST( TFLOAT*, &uiValue );
}
} // namespace XURReader

//-----------------------------------------------------------------------------
// Timeline property ids: 0xCCLLPP00 style constants returned by TranslateTimelineProp.
// NOTE: names come from the property strings each TranslateTimelineProp compares against.
//-----------------------------------------------------------------------------
enum XURPROP : TUINT32
{
	// XURXUIElementData
	XURPROP_ELEMENT_WIDTH                    = 0x01010100,
	XURPROP_ELEMENT_HEIGHT                   = 0x01010200,
	XURPROP_ELEMENT_POSITION                 = 0x01010300,
	XURPROP_ELEMENT_SCALE                    = 0x01010400,
	XURPROP_ELEMENT_ROTATION                 = 0x01010500,
	XURPROP_ELEMENT_OPACITY                  = 0x01010600,
	XURPROP_ELEMENT_ANCHOR                   = 0x01010700,
	XURPROP_ELEMENT_PIVOT                    = 0x01010800,
	XURPROP_ELEMENT_SHOW                     = 0x01010900,
	XURPROP_ELEMENT_BLENDMODE                = 0x01010A00,
	XURPROP_ELEMENT_DISABLETIMELINERECURSION = 0x01010B00,
	XURPROP_ELEMENT_BRUSHFLAGS               = 0x01010C00,
	XURPROP_ELEMENT_COLORWRITEFLAGS          = 0x01010D00,

	// XURXUIControlData: the element properties are moved to 0x0102xx00
	XURPROP_CONTROL_ELEMENT_WIDTH                    = 0x01020100,
	XURPROP_CONTROL_ELEMENT_HEIGHT                   = 0x01020200,
	XURPROP_CONTROL_ELEMENT_POSITION                 = 0x01020300,
	XURPROP_CONTROL_ELEMENT_SCALE                    = 0x01020400,
	XURPROP_CONTROL_ELEMENT_ROTATION                 = 0x01020500,
	XURPROP_CONTROL_ELEMENT_OPACITY                  = 0x01020600,
	XURPROP_CONTROL_ELEMENT_ANCHOR                   = 0x01020700,
	XURPROP_CONTROL_ELEMENT_PIVOT                    = 0x01020800,
	XURPROP_CONTROL_ELEMENT_SHOW                     = 0x01020900,
	XURPROP_CONTROL_ELEMENT_BLENDMODE                = 0x01020A00,
	XURPROP_CONTROL_ELEMENT_DISABLETIMELINERECURSION = 0x01020B00,
	XURPROP_CONTROL_ELEMENT_BRUSHFLAGS               = 0x01020C00,
	XURPROP_CONTROL_ELEMENT_COLORWRITEFLAGS          = 0x01020D00,
	// ...and the control's own ones use the element range
	XURPROP_CONTROL_VISUAL          = 0x01010100,
	XURPROP_CONTROL_ENABLED         = 0x01010200,
	XURPROP_CONTROL_UNFOCUSSEDINPUT = 0x01010300,
	XURPROP_CONTROL_NAVLEFT         = 0x01010400,
	XURPROP_CONTROL_NAVRIGHT        = 0x01010500,
	XURPROP_CONTROL_NAVUP           = 0x01010600,
	XURPROP_CONTROL_NAVDOWN         = 0x01010700,
	XURPROP_CONTROL_TEXT            = 0x01010800,
	XURPROP_CONTROL_IMAGEPATH       = 0x01010A00,

	// XURXUISceneData
	XURPROP_SCENE_TRANSFROM     = 0x01000100,
	XURPROP_SCENE_TRANSTO       = 0x01000200,
	XURPROP_SCENE_TRANSBACKFROM = 0x01000300,
	XURPROP_SCENE_TRANSBACKTO   = 0x01000400,
	XURPROP_SCENE_IGNOREPRESSES = 0x01000600,

	// XURXUIFigureData
	XURPROP_FIGURE_CLOSED                  = 0x01000200,
	XURPROP_FIGURE_STROKE_STROKEWIDTH      = 0x02000000,
	XURPROP_FIGURE_STROKE_STROKECOLOR      = 0x02000001,
	XURPROP_FIGURE_FILL_FILLTYPE           = 0x02000100,
	XURPROP_FIGURE_FILL_FILLCOLOR          = 0x02000101,
	XURPROP_FIGURE_FILL_TRANSLATION        = 0x02000104,
	XURPROP_FIGURE_FILL_SCALE              = 0x02000105,
	XURPROP_FIGURE_FILL_WRAPX              = 0x02000107,
	XURPROP_FIGURE_FILL_WRAPY              = 0x02000108,
	XURPROP_FIGURE_FILL_BRUSHFLAGS         = 0x02000109,
	XURPROP_FIGURE_FILL_GRADIENT_STOPCOLOR = 0x00010302,
	XURPROP_FIGURE_FILL_GRADIENT_STOPPOS   = 0x00010303,

	// XURXUIImageData
	XURPROP_IMAGE_SIZEMODE   = 0x01000000,
	XURPROP_IMAGE_IMAGEPATH  = 0x01000100,
	XURPROP_IMAGE_BRUSHFLAGS = 0x00000000,
};

//-----------------------------------------------------------------------------
// Header of a XUR file
//-----------------------------------------------------------------------------
struct XURSection
{
	TUINT32 uiName;   // 0x0 ('STRN', 'VECT', 'QUAT', 'CUST' or 'DATA')
	TUINT32 uiOffset; // 0x4 from the start of the file
	TUINT32 uiSize;   // 0x8
};

TSTATICASSERT( sizeof( XURSection ) == 0xC );

constexpr TUINT32 XURSECTION_STRN = 0x5354524E;
constexpr TUINT32 XURSECTION_VECT = 0x56454354;
constexpr TUINT32 XURSECTION_QUAT = 0x51554154;
constexpr TUINT32 XURSECTION_CUST = 0x43555354;
constexpr TUINT32 XURSECTION_DATA = 0x44415441;

class XURHeader
{
public:
	XURHeader();
	virtual ~XURHeader();

public:
	XURSection* m_pSections;     // 0x04
	TUINT32     m_uiFileID;      // 0x08 'XUIB'
	TUINT32     m_uiVersion;     // 0x0C
	TUINT16     m_usFlags;       // 0x10 low half of the flags
	TUINT32     m_uiFileSize;    // 0x14
	TUINT16     m_usFlagsHigh;   // 0x18 high half of the flags
	TUINT16     m_usNumSections; // 0x1A
};

#  ifndef TOSHI_SKU_WINDOWS
TSTATICASSERT( sizeof( XURHeader ) == 0x1C );
#  endif

//-----------------------------------------------------------------------------
// Named frames and timelines
//-----------------------------------------------------------------------------
class XURXUINamedFrameData
{
public:
	enum COMMAND : TUINT8
	{
		COMMAND_PLAY,
		COMMAND_STOP,
		COMMAND_GOTO,
		COMMAND_GOTOANDPLAY,
		COMMAND_GOTOANDSTOP,
	};

public:
	XURXUINamedFrameData();
	virtual ~XURXUINamedFrameData();

public:
	TUINT32 m_uiTime;   // 0x4
	TUINT16 m_usName;   // 0x8 string index
	TUINT16 m_usTarget; // 0xA string index of the goto target
	TUINT8  m_eCommand; // 0xC
};

class XURXUIKeyframeData
{
public:
	XURXUIKeyframeData();
	virtual ~XURXUIKeyframeData();

public:
	union Value
	{
		TUINT32 uiValue;
		TFLOAT  fValue;
	};

	// How the values change towards this keyframe
	using INTERPOLATION = TUINT8;
	enum INTERPOLATION_ : INTERPOLATION
	{
		INTERPOLATION_LINEAR = 0,
		INTERPOLATION_NONE   = 1, // jumps to the values at the end
		INTERPOLATION_EASE   = 2, // m_cEaseIn, m_cEaseOut, m_cEaseScale
	};

	Value*        m_pValues;         // 0x4 one per timeline property
	TUINT32       m_uiTime;          // 0x8
	INTERPOLATION m_ucInterpolation; // 0xC
	TINT8         m_cEaseIn;         // 0xD
	TINT8         m_cEaseOut;        // 0xE
	TINT8         m_cEaseScale;      // 0xF
};

struct XURXUITimelineProp
{
	XURXUITimelineProp()
	{
		uiProp  = 0;
		uiIndex = -1;
	}

	TUINT32 uiProp;  // 0x0
	TUINT32 uiIndex; // 0x4
};

class XURXUITimelineData
{
public:
	XURXUITimelineData();
	virtual ~XURXUITimelineData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData );

public:
	class XURXUIObjectData* m_pOwnerData;     // 0x04
	XURXUIElementData*      m_pElementData;   // 0x08 the child the timeline animates
	XURXUIKeyframeData*     m_pKeyframes;     // 0x0C
	XURXUITimelineProp*     m_pProps;         // 0x10
	TUINT32                 m_uiNumProps;     // 0x14
	TUINT32                 m_uiNumKeyframes; // 0x18
	TUINT16                 m_usElementId;    // 0x1C string index
};

//-----------------------------------------------------------------------------
// Objects
// Wii vtable: +0x08 dtor, +0x0C Load, +0x10 ValidateTimelineProp, +0x14 TranslateTimelineProp,
//             +0x18 GetTimelinePropSize, +0x1C IsFloatPropType
//-----------------------------------------------------------------------------
class XURXUIObjectData
{
public:
	// Flags stored before each object
	using OBJECTFLAGS = TUINT8;
	enum OBJECTFLAGS_ : OBJECTFLAGS
	{
		OBJECTFLAGS_NONE         = 0,
		OBJECTFLAGS_CHILDREN     = BITFLAG( 1 ), // LoadChildren (and LoadTimelines with OBJECTFLAGS_NAMED_FRAMES)
		OBJECTFLAGS_NAMED_FRAMES = BITFLAG( 2 ), // LoadNamedFrames
	};

public:
	XURXUIObjectData();
	virtual ~XURXUIObjectData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData );

	// $BarnyardWiiJP: FUNCTION 80414df0
	virtual TBOOL ValidateTimelineProp( TUINT32 a_uiProp ) { return TFALSE; }

	// $BarnyardWiiJP: FUNCTION 80414df8
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) { return 0; }

	// $BarnyardWiiJP: FUNCTION 80414fd4
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) { return 0; }

	// $BarnyardWiiJP: FUNCTION 80414fcc
	virtual TBOOL IsFloatPropType( TUINT32 a_uiProp ) { return TFALSE; }

	TBOOL              LoadChildren( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData );
	TBOOL              LoadNamedFrames( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData );
	TBOOL              LoadTimelines( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData );
	XURXUIElementData* FindChildElementData( TUINT16 a_usId );

	XURXUIObjectData*     GetChild( TUINT32 a_uiIndex ) const { return m_apChildren[ a_uiIndex ]; }
	TUINT32               GetNumChildren() const { return m_uiNumChildren; }
	const Toshi::TClass*  GetUIClass() const { return m_pClass; }
	TUINT16               GetClassIndex() const { return m_usClassIndex; }
	XURXUINamedFrameData* GetNamedFrames() const { return m_pNamedFrames; }
	TUINT32               GetNumNamedFrames() const { return m_uiNumNamedFrames; }
	XURXUITimelineData*   GetTimelines() const { return m_pTimelines; }
	TUINT32               GetNumTimelines() const { return m_uiNumTimelines; }

public:
	XURXUIObjectData**    m_apChildren;       // 0x04
	XURXUINamedFrameData* m_pNamedFrames;     // 0x08
	XURXUITimelineData*   m_pTimelines;       // 0x0C
	TUINT32               m_uiNumChildren;    // 0x10
	TUINT32               m_uiNumNamedFrames; // 0x14
	TUINT32               m_uiNumTimelines;   // 0x18
	// NOTE: the name is unknown; when it's zero the objects have no property masks
	TUINT16              m_usPropMasks;  // 0x1C
	TUINT16              m_usClassIndex; // 0x1E string index of the XUI class name
	const Toshi::TClass* m_pClass;       // 0x20 the AGUI2XUI* class to create
};

class XURXUIElementData : public XURXUIObjectData
{
public:
	using PROPMASK = TUINT16;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE                       = 0,
		PROPMASK_ID                         = BITFLAG( 0 ),
		PROPMASK_WIDTH                      = BITFLAG( 1 ),
		PROPMASK_HEIGHT                     = BITFLAG( 2 ),
		PROPMASK_POSITION                   = BITFLAG( 3 ),
		PROPMASK_SCALE                      = BITFLAG( 4 ),
		PROPMASK_ROTATION                   = BITFLAG( 5 ),
		PROPMASK_OPACITY                    = BITFLAG( 6 ),
		PROPMASK_ANCHOR                     = BITFLAG( 7 ),
		PROPMASK_PIVOT                      = BITFLAG( 8 ),
		PROPMASK_SHOW                       = BITFLAG( 9 ),
		PROPMASK_BLEND_MODE                 = BITFLAG( 10 ),
		PROPMASK_DISABLE_TIMELINE_RECURSION = BITFLAG( 11 ),
		PROPMASK_DESIGN_TIME                = BITFLAG( 12 ), // never stored
		PROPMASK_COLOR_WRITE_FLAGS          = BITFLAG( 13 ),
	};

public:
	XURXUIElementData();
	~XURXUIElementData();

	virtual TBOOL   Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiProp ) override;
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override;
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override;
	virtual TBOOL   IsFloatPropType( TUINT32 a_uiProp ) override;

public:
	TFLOAT  m_fWidth;                    // 0x24
	TFLOAT  m_fHeight;                   // 0x28
	TFLOAT  m_fOpacity;                  // 0x2C
	TINT    m_iPosition;                 // 0x30 vector index
	TINT    m_iScale;                    // 0x34 vector index
	TINT    m_iRotation;                 // 0x38 quaternion index
	TINT    m_iPivot;                    // 0x3C vector index
	TUINT32 m_uiAnchor;                  // 0x40
	TUINT32 m_uiBlendMode;               // 0x44
	TUINT32 m_uiColorWriteFlags;         // 0x48
	TUINT16 m_usId;                      // 0x4C string index
	TBOOL   m_bShow;                     // 0x4E
	TBOOL   m_bDisableTimelineRecursion; // 0x4F
};

class XURXUIControlData : public XURXUIElementData
{
public:
	using PROPMASK = TUINT16;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE             = 0,
		PROPMASK_CLASS_OVERRIDE   = BITFLAG( 0 ),
		PROPMASK_VISUAL           = BITFLAG( 1 ),
		PROPMASK_ENABLED          = BITFLAG( 2 ),
		PROPMASK_UNFOCUSSED_INPUT = BITFLAG( 3 ),
		PROPMASK_NAV_LEFT         = BITFLAG( 4 ),
		PROPMASK_NAV_RIGHT        = BITFLAG( 5 ),
		PROPMASK_NAV_UP           = BITFLAG( 6 ),
		PROPMASK_NAV_DOWN         = BITFLAG( 7 ),
		PROPMASK_TEXT             = BITFLAG( 8 ),
		PROPMASK_IMAGE_PATH       = BITFLAG( 9 ),
	};

	enum NAV
	{
		NAV_UP,
		NAV_DOWN,
		NAV_LEFT,
		NAV_RIGHT,
		NAV_NUMOF
	};

public:
	XURXUIControlData();
	~XURXUIControlData();

	virtual TBOOL   Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiProp ) override;
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override;
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override;
	virtual TBOOL   IsFloatPropType( TUINT32 a_uiProp ) override;

public:
	TUINT16 m_usClassOverride;   // 0x50 string index
	TUINT16 m_usVisual;          // 0x52 string index
	TUINT16 m_aNav[ NAV_NUMOF ]; // 0x54 string indices
	TUINT16 m_usText;            // 0x5C string index
	TUINT16 m_usImagePath;       // 0x5E string index
	TBOOL   m_bEnabled;          // 0x60
	TBOOL   m_bUnfocussedInput;  // 0x61
};

class XURXUISceneData : public XURXUIControlData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE            = 0,
		PROPMASK_DEFAULT_FOCUS   = BITFLAG( 0 ),
		PROPMASK_TRANS_FROM      = BITFLAG( 1 ),
		PROPMASK_TRANS_TO        = BITFLAG( 2 ),
		PROPMASK_TRANS_BACK_FROM = BITFLAG( 3 ),
		PROPMASK_TRANS_BACK_TO   = BITFLAG( 4 ),
		PROPMASK_UNK5            = BITFLAG( 5 ),
		PROPMASK_IGNORE_PRESSES  = BITFLAG( 6 ),
	};

public:
	XURXUISceneData();
	~XURXUISceneData();

	virtual TBOOL   Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiProp ) override;
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override;
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override;

public:
	// NOTE: name unknown (the property after TransBackTo)
	TUINT32 m_uiUnk1;          // 0x64
	TUINT16 m_usDefaultFocus;  // 0x68 string index
	TUINT16 m_usTransFrom;     // 0x6A string index
	TUINT16 m_usTransTo;       // 0x6C string index
	TUINT16 m_usTransBackFrom; // 0x6E string index
	TUINT16 m_usTransBackTo;   // 0x70 string index
	TBOOL   m_bIgnorePresses;  // 0x72
};

class XURXUICanvasData : public XURXUIElementData
{
public:
	XURXUICanvasData();
	~XURXUICanvasData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
};

class XURXUIVisualData : public XURXUIElementData
{
public:
	XURXUIVisualData();
	~XURXUIVisualData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
};

class XURXUIGroupData : public XURXUIElementData
{
public:
	XURXUIGroupData();
	~XURXUIGroupData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
};

class XURXUIStrokeData : public XURXUIObjectData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE         = 0,
		PROPMASK_STROKE_WIDTH = BITFLAG( 0 ),
		PROPMASK_STROKE_COLOR = BITFLAG( 1 ),
	};

public:
	XURXUIStrokeData();
	~XURXUIStrokeData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;

public:
	TFLOAT  m_fStrokeWidth;  // 0x24
	TUINT32 m_uiStrokeColor; // 0x28
};

class XURXUIGradientData : public XURXUIObjectData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE        = 0,
		PROPMASK_RADIAL      = BITFLAG( 0 ),
		PROPMASK_NUM_STOPS   = BITFLAG( 1 ),
		PROPMASK_STOP_COLORS = BITFLAG( 2 ),
		PROPMASK_STOP_POS    = BITFLAG( 3 ),
	};

public:
	XURXUIGradientData();
	~XURXUIGradientData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;

public:
	TBOOL    m_bRadial;     // 0x24
	TUINT32  m_uiNumStops;  // 0x28
	TUINT32* m_pStopColors; // 0x2C
	TFLOAT*  m_pStopPos;    // 0x30
};

class XURXUIFillData : public XURXUIObjectData
{
public:
	using PROPMASK = TUINT16;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE              = 0,
		PROPMASK_FILL_TYPE         = BITFLAG( 0 ),
		PROPMASK_FILL_COLOR        = BITFLAG( 1 ),
		PROPMASK_TEXTURE_FILE_NAME = BITFLAG( 2 ),
		PROPMASK_GRADIENT          = BITFLAG( 3 ),
		PROPMASK_TRANSLATION       = BITFLAG( 4 ),
		PROPMASK_SCALE             = BITFLAG( 5 ),
		PROPMASK_ROTATION          = BITFLAG( 6 ),
		PROPMASK_WRAP_X            = BITFLAG( 7 ),
		PROPMASK_WRAP_Y            = BITFLAG( 8 ),
	};

public:
	XURXUIFillData();
	~XURXUIFillData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;

public:
	TUINT32             m_uiFillType;        // 0x24
	TUINT32             m_uiFillColor;       // 0x28
	TUINT16             m_usTextureFileName; // 0x2C string index
	XURXUIGradientData* m_pGradient;         // 0x30
	TINT                m_iTranslation;      // 0x34 vector index
	TINT                m_iScale;            // 0x38 vector index
	TFLOAT              m_fRotation;         // 0x3C
	TUINT32             m_uiWrapX;           // 0x40
	TUINT32             m_uiWrapY;           // 0x44
};

class XURXUIFigureData : public XURXUIElementData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE   = 0,
		PROPMASK_STROKE = BITFLAG( 0 ),
		PROPMASK_FILL   = BITFLAG( 1 ),
		PROPMASK_CLOSED = BITFLAG( 2 ),
		PROPMASK_PATH   = BITFLAG( 3 ), // the points are in the custom data section
	};

public:
	struct Point
	{
		TFLOAT fX;        // 0x00
		TFLOAT fY;        // 0x04
		TFLOAT fCtrl1X;   // 0x08
		TFLOAT fCtrl1Y;   // 0x0C
		TFLOAT fCtrl2X;   // 0x10
		TFLOAT fCtrl2Y;   // 0x14
		TBOOL  bIsCurved; // 0x18
	};

public:
	XURXUIFigureData();
	~XURXUIFigureData();

	virtual TBOOL   Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiProp ) override;
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override;
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override;
	virtual TBOOL   IsFloatPropType( TUINT32 a_uiProp ) override;

public:
	TBOOL             m_bClosed;     // 0x50
	TFLOAT            m_fPathWidth;  // 0x54
	TFLOAT            m_fPathHeight; // 0x58
	TUINT32           m_uiNumPoints; // 0x5C
	XURXUIStrokeData* m_pStroke;     // 0x60
	XURXUIFillData*   m_pFill;       // 0x64
	Point*            m_pPoints;     // 0x68
};

class XURXUIImageData : public XURXUIElementData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE        = 0,
		PROPMASK_SIZE_MODE   = BITFLAG( 0 ),
		PROPMASK_IMAGE_PATH  = BITFLAG( 1 ),
		PROPMASK_BRUSH_FLAGS = BITFLAG( 2 ),
	};

public:
	XURXUIImageData();
	~XURXUIImageData();

	virtual TBOOL   Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;
	virtual TBOOL   ValidateTimelineProp( TUINT32 a_uiProp ) override;
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override;
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override;

public:
	TUINT16 m_usImagePath;  // 0x50 string index
	TUINT32 m_uiSizeMode;   // 0x54
	TUINT32 m_uiBrushFlags; // 0x58
};

class XURXUIImagePresenterData : public XURXUIElementData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE             = 0,
		PROPMASK_SIZE_MODE        = BITFLAG( 0 ),
		PROPMASK_DATA_ASSOCIATION = BITFLAG( 1 ),
		PROPMASK_BRUSH_FLAGS      = BITFLAG( 2 ),
	};

public:
	XURXUIImagePresenterData();
	~XURXUIImagePresenterData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;

	// $BarnyardWiiJP: FUNCTION 80414878
	virtual TBOOL ValidateTimelineProp( TUINT32 a_uiProp ) override { return TFALSE; }

	// $BarnyardWiiJP: FUNCTION 80414880
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override { return XURXUIElementData::TranslateTimelineProp( a_szName ); }

	// $BarnyardWiiJP: FUNCTION 80414884
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override { return XURXUIElementData::GetTimelinePropSize( a_uiProp ); }

public:
	TUINT32 m_uiDataAssociation; // 0x50
	TUINT32 m_uiSizeMode;        // 0x54
	TUINT32 m_uiBrushFlags;      // 0x58
};

class XURXUITextData : public XURXUIElementData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE                = 0,
		PROPMASK_TEXT                = BITFLAG( 0 ),
		PROPMASK_TEXT_COLOR          = BITFLAG( 1 ),
		PROPMASK_DROP_SHADOW_COLOR   = BITFLAG( 2 ),
		PROPMASK_POINT_SIZE          = BITFLAG( 3 ),
		PROPMASK_FONT                = BITFLAG( 4 ),
		PROPMASK_TEXT_STYLE          = BITFLAG( 5 ),
		PROPMASK_LINE_SPACING_ADJUST = BITFLAG( 6 ),
	};

public:
	XURXUITextData();
	~XURXUITextData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;

	// $BarnyardWiiJP: FUNCTION 804144dc
	virtual TBOOL ValidateTimelineProp( TUINT32 a_uiProp ) override { return XURXUIElementData::ValidateTimelineProp( a_uiProp ); }

	// $BarnyardWiiJP: FUNCTION 804144e0
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override { return XURXUIElementData::TranslateTimelineProp( a_szName ); }

	// $BarnyardWiiJP: FUNCTION 804144e4
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override { return XURXUIElementData::GetTimelinePropSize( a_uiProp ); }

public:
	TUINT16 m_usText;              // 0x50 string index
	TUINT32 m_uiTextColor;         // 0x54
	TUINT32 m_uiDropShadowColor;   // 0x58
	TUINT32 m_uiPointSize;         // 0x5C
	TUINT16 m_usFont;              // 0x60 string index
	TUINT32 m_uiTextStyle;         // 0x64
	TUINT32 m_uiLineSpacingAdjust; // 0x68
};

class XURXUITextPresenterData : public XURXUIElementData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE                = 0,
		PROPMASK_TEXT_COLOR          = BITFLAG( 0 ),
		PROPMASK_DROP_SHADOW_COLOR   = BITFLAG( 1 ),
		PROPMASK_POINT_SIZE          = BITFLAG( 2 ),
		PROPMASK_FONT                = BITFLAG( 3 ),
		PROPMASK_TEXT_STYLE          = BITFLAG( 4 ),
		PROPMASK_LINE_SPACING_ADJUST = BITFLAG( 5 ),
		PROPMASK_DATA_ASSOCIATION    = BITFLAG( 6 ),
	};

public:
	XURXUITextPresenterData();
	~XURXUITextPresenterData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;

	// $BarnyardWiiJP: FUNCTION 80414b28
	virtual TBOOL ValidateTimelineProp( TUINT32 a_uiProp ) override { return TFALSE; }

	// $BarnyardWiiJP: FUNCTION 80414b30
	virtual TUINT32 TranslateTimelineProp( const TCHAR* a_szName ) override { return XURXUIElementData::TranslateTimelineProp( a_szName ); }

	// $BarnyardWiiJP: FUNCTION 80414b34
	virtual TUINT32 GetTimelinePropSize( TUINT32 a_uiProp ) override { return XURXUIElementData::GetTimelinePropSize( a_uiProp ); }

public:
	TUINT32 m_uiDataAssociation;   // 0x50
	TUINT32 m_uiTextColor;         // 0x54
	TUINT32 m_uiDropShadowColor;   // 0x58
	TUINT32 m_uiPointSize;         // 0x5C
	TUINT16 m_usFont;              // 0x60 string index
	TUINT32 m_uiTextStyle;         // 0x64
	TUINT32 m_uiLineSpacingAdjust; // 0x68
};

class XURXUIButtonData : public XURXUIControlData
{
public:
	using PROPMASK = TUINT8;
	enum PROPMASK_ : PROPMASK
	{
		PROPMASK_NONE = 0,
		PROPMASK_UNK0 = BITFLAG( 0 ),
	};

public:
	XURXUIButtonData();
	~XURXUIButtonData();

	virtual TBOOL Load( AGUI2XUIResource& a_rResource, TUINT8*& a_rpData ) override;

public:
	// NOTE: name unknown (the only XuiButton property)
	TUINT32 m_uiUnk1; // 0x64
};

#endif // BARNYARD_LATEST
