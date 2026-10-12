#pragma once
#include <GUI/T2GUIElement.h>
#include <Render/TModel.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

class AAnimatableObject;

namespace Toshi
{
class TViewport;
class TCameraObject;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: GUI element that renders animatable objects into its own viewport,
// for 3D models on top of the GUI (the "!" of AAnimalPopulationManager, menus).
// vtable 0073a540: +0x00 dtor 00644b50, +0x04 Tick 00644b70, +0x0c Render 006455e0,
// +0x38 SetAlpha 00645420.
//-----------------------------------------------------------------------------
class AGUI2ModelRenderer
    : public Toshi::T2GUIElement
{
public:
	static constexpr TINT MAX_OBJECTS = 100;

	struct Object
	{
		AAnimatableObject*                      pObject;             // 0x00
		Toshi::TVector4                         vLightColour;        // 0x04
		Toshi::TVector4                         vAmbientColour;      // 0x14
		TBOOL                                   bVisible;            // 0x24
		Toshi::TModelInstance::t_CustomRenderCB fnRenderCallback;    // 0x28, the render callback of the model that was replaced
		void*                                   pRenderCallbackData; // 0x2c
	};

	struct ObjectList
	{
		ObjectList();

		// Removes the last object
		void RemoveLast();

		// Removes the object at a_iIndex
		void Remove( TINT a_iIndex );

		TINT   iNumObjects;             // 0x00
		Object aObjects[ MAX_OBJECTS ]; // 0x04
	};

public:
	AGUI2ModelRenderer();
	~AGUI2ModelRenderer();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;
	virtual void Render() override;
	virtual void SetAlpha( TFLOAT a_fAlpha ) override;

	// Creates a viewport of the same size and the same camera settings as a_pViewport and a_pCamera
	TBOOL CreateFromViewport( Toshi::TViewport* a_pViewport, Toshi::TCameraObject* a_pCamera, TBOOL a_bClearBackground );

	// Creates a viewport of the given size with a default camera
	TBOOL Create( TFLOAT a_fWidth, TFLOAT a_fHeight, TBOOL a_bClearBackground, TBOOL a_bCameraMode );

	// Adds a_pObject, it's rendered with the given light and ambient colours (white if TNULL)
	void AddObject( AAnimatableObject* a_pObject, const Toshi::TVector4* a_pLightColour, const Toshi::TVector4* a_pAmbientColour );

	// Removes a_pObject, returns its index or -1
	TINT RemoveObject( AAnimatableObject* a_pObject );

	// Removes every object
	void RemoveAllObjects();

	// Destroys every object
	void DestroyObjects();

	TINT FindObject( AAnimatableObject* a_pObject );

	// $Barnyard: FUNCTION 00644940
	AAnimatableObject* GetObject( TINT a_iIndex ) { return m_pObjects->aObjects[ a_iIndex ].pObject; }
	TINT               GetNumObjects() { return m_pObjects->iNumObjects; }

	void SetObjectVisible( TINT a_iIndex, TBOOL a_bVisible );
	void SetObjectOpacity( TINT a_iIndex, TFLOAT a_fOpacity );

	// $Barnyard: FUNCTION 006446a0
	Toshi::TCameraObject* GetCamera() { return m_pCamera; }
	Toshi::TViewport*     GetViewport() { return m_pViewport; }

	void SetCameraMatrix( const Toshi::TMatrix44& a_rcMatrix );
	void SetCameraPosition( const Toshi::TVector4& a_rcPosition );
	void SetLightDirection( const Toshi::TVector4& a_rcDirection );

	// Places the viewport where the element is on the screen
	void UpdateViewport();

	void  SetRender( TBOOL a_bRender ) { m_bRender = a_bRender; }
	TBOOL IsRender() const { return m_bRender; }

protected:
	// Renders the objects into the viewport
	void RenderObjects();

	static void RenderObjectCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );

protected:
	Toshi::TViewport*     m_pViewport;       // 0x54
	Toshi::TCameraObject* m_pCamera;         // 0x58
	ObjectList*           m_pObjects;        // 0x5c
	TBOOL                 m_bCreated;        // 0x60
	Toshi::TVector4       m_vLightDirection; // 0x64
	TBOOL                 m_bRender;         // 0x74
	TFLOAT                m_fViewportWidth;  // 0x78
	TFLOAT                m_fViewportHeight; // 0x7c
};

TSTATICASSERT( sizeof( AGUI2ModelRenderer::Object ) == 0x30 );
TSTATICASSERT( sizeof( AGUI2ModelRenderer::ObjectList ) == 0x12c4 );
TSTATICASSERT( sizeof( AGUI2ModelRenderer ) == 0x80 );
