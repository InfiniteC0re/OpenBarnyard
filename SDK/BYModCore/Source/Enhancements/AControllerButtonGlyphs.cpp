#include "pch.h"
#include "AControllerButtonGlyphs.h"
#include "HookHelpers.h"

#include <BYardSDK/SDKHooks.h>
#include <Input/TInputDeviceController.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

extern const T2CommandLine* g_pCommandLine;

struct GlyphEntry
{
	TINT          iDoodad;
	const TWCHAR* wszGlyph;
};

static const GlyphEntry s_aGlyphs[] = {
	{ TInputDeviceController::BUTTON_CONTEXT_1, L"\x0F1A" },        // Triangle
	{ TInputDeviceController::BUTTON_CONTEXT_2, L"\x0F0D" },        // Circle
	{ TInputDeviceController::BUTTON_CONTEXT_3, L"\x0F0E" },        // Cross
	{ TInputDeviceController::BUTTON_CONTEXT_4, L"\x0F0F" },        // Square
	{ TInputDeviceController::BUTTON_SHOULDER_LEFT_1, L"\x0F08" },  // L1
	{ TInputDeviceController::BUTTON_SHOULDER_LEFT_2, L"\x0F11" },  // L2
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_1, L"\x0F09" }, // R1
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_2, L"\x0F10" }, // R2
	{ TInputDeviceController::VIRTSTK_DPAD, L"\x0F16" },            // D-pad
};

static const GlyphEntry s_aGlyphsGC[] = {
	{ TInputDeviceController::BUTTON_CONTEXT_1, L"\x0F0A" },        // Y
	{ TInputDeviceController::BUTTON_CONTEXT_2, L"\x0F0C" },        // B
	{ TInputDeviceController::BUTTON_CONTEXT_3, L"\x0F0B" },        // A
	{ TInputDeviceController::BUTTON_CONTEXT_4, L"\x0F1B" },        // X
	{ TInputDeviceController::BUTTON_SHOULDER_LEFT_1, L"\x0F19" },  // L
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_1, L"\x0F20" }, // R
	{ TInputDeviceController::BUTTON_SHOULDER_RIGHT_2, L"\x0F1C" }, // Z
	{ TInputDeviceController::VIRTSTK_DPAD, L"\x0F01" },            // D-pad
};

static const GlyphEntry* s_pGlyphs    = s_aGlyphs;
static TINT              s_iNumGlyphs = TARRAYSIZE( s_aGlyphs );

class AInputMapManager
{};

static void FillCustomNameMap( void* a_pInputMapManager )
{
	// T2Map<TInputDevice::Doodad, const TWCHAR*> m_oDoodadToCustomNameMap
	void* pCustomNameMap = (void*)( TUINTPTR( a_pInputMapManager ) + 0xD4 );

	for ( TINT i = 0; i < s_iNumGlyphs; i++ )
	{
		const GlyphEntry& rcEntry = s_pGlyphs[ i ];
		// T2Map::Insert, allocates the node with the map's (game's) allocator
		void*         pResult  = TNULL;
		TINT          iDoodad  = rcEntry.iDoodad;
		const TWCHAR* wszGlyph = rcEntry.wszGlyph;
		CALL_THIS( 0x00597460, void*, void*, pCustomNameMap, void**, &pResult, TINT*, &iDoodad, const TWCHAR**, &wszGlyph );
	}

	TINFO( "[glyphs] Added %d button icons to AInputMapManager\n", s_iNumGlyphs );
}

// The map belongs to the new instance, so filling it here never duplicates entries
MEMBER_HOOK( 0x0059c840, AInputMapManager, AInputMapManager_Constructor, AInputMapManager* )
{
	CallOriginal();
	FillCustomNameMap( this );

	return this;
}

void AControllerButtonGlyphs::Initialise()
{
	if ( !g_pCommandLine->HasParameter( "-ps2glyphs" ) )
	{
		s_pGlyphs    = s_aGlyphsGC;
		s_iNumGlyphs = TARRAYSIZE( s_aGlyphsGC );
	}

	InstallHook<AInputMapManager_Constructor>();
}
