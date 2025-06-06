#pragma once

TOSHI_NAMESPACE_START

typedef TUINT8 SYSRESOURCE;
enum SYSRESOURCE_ : SYSRESOURCE
{
	SYSRESOURCE_VFactories,
	SYSRESOURCE_VFSYSVNDUV1,
	SYSRESOURCE_VFSKIN,
	SYSRESOURCE_4,
	SYSRESOURCE_5,
	SYSRESOURCE_6,
	SYSRESOURCE_VFWORLD,
	SYSRESOURCE_8,
	SYSRESOURCE_9,
	SYSRESOURCE_IFactories,
	SYSRESOURCE_IFSYS,
	SYSRESOURCE_12,
	SYSRESOURCE_TEXTUREFACTORY = 27,
	SYSRESOURCE_NUMOF
};

/**
 * Texture coordinate types for addressing modes
 */
enum TEXCOORD
{
	TEXCOORD_UV = -1, // UV coordinates
	TEXCOORD_U,       // U coordinate
	TEXCOORD_V,       // V coordinate
	TEXCOORD_NUMOF    // Number of coordinate types
};

/**
 * Texture addressing modes
 */
enum ADDRESSINGMODE
{
	ADDRESSINGMODE_WRAP,
	ADDRESSINGMODE_MIRROR,
	ADDRESSINGMODE_CLAMP,
	ADDRESSINGMODE_BORDER,
	ADDRESSINGMODE_NUMOF
};

TOSHI_NAMESPACE_END
