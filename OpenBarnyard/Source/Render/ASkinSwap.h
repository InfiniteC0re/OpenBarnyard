#pragma once
#include "Animation/AAnimatableObject.h"
#include "Render/ASkinShader/ASkinMaterial.h"

#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// NOTE: invented name, the original one is unknown.
// Renders an AAnimatableObject with one of its type's alternative skin textures:
// creates an ASkinMaterial named "swap" and, while the swap is active, hooks the
// model instance's custom render callback to temporarily replace the skin material
// of the first LOD's meshes with it.
// Used by Cow Tipping elements (Enemy +4, Cow +0) and 0049fd30.
// ctor 0060df80, dtor 0060e390, size 0x14.
//-----------------------------------------------------------------------------
class ASkinSwap
{
public:
	ASkinSwap();
	~ASkinSwap();

	// Sets the object whose skin is swapped and the index of the skin to use
	void SetObject( AAnimatableObject* a_pObject, TINT a_iSkin );

	// Restores the original render callback and destroys the swap material
	void Reset();

private:
	// NOTE: __stdcall in retail (RET 8); uses the engine's callback type here
	static void RenderCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );

public:
	TINT                                    m_iSkin;             // 0x00
	AAnimatableObject*                      m_pObject;           // 0x04
	Toshi::TModelInstance::t_CustomRenderCB m_fnOrigRenderCb;    // 0x08
	ASkinMaterial*                          m_pSwapMaterial;     // 0x0C
	ASkinMaterial*                          m_pOriginalMaterial; // 0x10
};

TSTATICASSERT( sizeof( ASkinSwap ) == 0x14 );
