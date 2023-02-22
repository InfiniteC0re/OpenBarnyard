#pragma once
#include "Toshi/Math/Math.h"

#include <cstdint>

struct BezierPoint
{
	Toshi::TVector2 Point;
	Toshi::TVector2 Ctrl1;
	Toshi::TVector2 Ctrl2;
};

enum XUI_ELEMENT_PROPERTY_TYPE
{
	XUI_EPT_EMPTY,
	XUI_EPT_BOOL,
	XUI_EPT_INTEGER,
	XUI_EPT_UNSIGNED,
	XUI_EPT_FLOAT,
	XUI_EPT_STRING,
	XUI_EPT_COLOR,
	XUI_EPT_VECTOR,
	XUI_EPT_QUATERNION,
	XUI_EPT_OBJECT,
	XUI_EPT_CUSTOM,
	XUI_EPT_SHORT32,
	XUI_EPT_USHORT32,
	XUI_EPT_UINT8
};

typedef bool XUIEPTBool;
typedef int32_t XUIEPTInteger;
typedef uint32_t XUIEPTUnsigned;
typedef float XUIEPTFloat;
typedef uint16_t XUIEPTString;
typedef uint32_t XUIEPTColor;
typedef uint16_t XUIEPTVector;
typedef uint16_t XUIEPTQuaternion;
typedef uint16_t XUIEPTObject;
typedef uint16_t XUIEPTCustom;
typedef int16_t XUIEPTShort32;
typedef uint16_t XUIEPTUShort32;
typedef uint32_t XUIEPTUColor;
typedef uint8_t XUIEPTUInt8;

// property flags
#define XUI_ELEMENT_PROPF_NONE      0x0000       // no flags specified
#define XUI_ELEMENT_PROPF_INDEXED   0x0001       // the property is indexed
#define XUI_ELEMENT_PROPF_HIDDEN    0x0002       // the property is hidden in the design time environment
#define XUI_ELEMENT_PROPF_LOCALIZE  0x0004       // the property is localizable
#define XUI_ELEMENT_PROPF_NOANIM    0x0008       // the property must not be animated
#define XUI_ELEMENT_PROPF_FILEPATH  0x0010       // the property represents a file path
#define XUI_ELEMENT_PROPF_EVENT     0x0020       // treat as an event (set on keyframes only)
#define XUI_ELEMENT_PROPF_DESIGN    0x0040       // design time only property