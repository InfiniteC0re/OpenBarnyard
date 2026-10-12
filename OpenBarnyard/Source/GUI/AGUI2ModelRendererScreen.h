#pragma once
#include "AGUI2ModelRenderer.h"

#include <Toshi/TTask.h>

//-----------------------------------------------------------------------------
// Purpose: AGUI2ModelRenderer covering the whole screen that renders objects
// with the view (camera and light) they were seen with when they were added,
// so they can be drawn on top of the GUI (the sim creates it, ASSeqUI uses it).
// vtable 0073a5c8: +0x00 dtor 00646340, +0x0c Render 00646090.
// Singleton at DAT_007b50ac.
//-----------------------------------------------------------------------------
class AGUI2ModelRendererScreen
    : public AGUI2ModelRenderer
{
public:
	static constexpr TINT MAX_VIEWS = 100;

	struct View
	{
		Toshi::TMatrix44 mWorldView;      // 0x00
		Toshi::TMatrix44 mCamera;         // 0x40
		Toshi::TMatrix44 mLightDirection; // 0x80
		TFLOAT           fFOV;            // 0xc0
	};

	//-----------------------------------------------------------------------------
	// Purpose: keeps the screen above the other GUI elements.
	// vtable 0073a58c.
	//-----------------------------------------------------------------------------
	class UpdateTask
	    : public Toshi::TTask
	{
	public:
		UpdateTask() = default;
		~UpdateTask();

		virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	};

public:
	AGUI2ModelRendererScreen();
	~AGUI2ModelRendererScreen();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;

	// Adds a_pObject that is rendered the way the main camera sees it now
	void AddObject( AAnimatableObject* a_pObject );

	// Adds a_pObject that is rendered with the given view
	void AddObject( AAnimatableObject* a_pObject, const Toshi::TMatrix44& a_rcWorldView, const Toshi::TMatrix44& a_rcCamera, const Toshi::TMatrix44& a_rcLightDirection, TFLOAT a_fFOV );

	void RemoveObject( AAnimatableObject* a_pObject );

	// Returns the transform a_pObject has to have in the world to look like it does on the screen now
	void GetObjectWorldTransform( AAnimatableObject* a_pObject, Toshi::TMatrix44& a_rOutTransform );

	// Starts or stops the update task
	void SetActive( TBOOL a_bActive );

	void SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }

	static AGUI2ModelRendererScreen* GetSingleton() { return ms_pSingleton; }

private:
	// Adds a_pObject without changing its render callbacks
	void AddObjectWithoutColours( AAnimatableObject* a_pObject );

	void RemoveView( TINT a_iIndex );
	void RenderObjects();

	static void RenderObjectCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );

public:
	inline static AGUI2ModelRendererScreen* ms_pSingleton; // 0x007b50ac

private:
	TINT       m_iNumViews;           // 0x80
	View       m_aViews[ MAX_VIEWS ]; // 0x84
	TBOOL      m_bEnabled;            // 0x4d14
	UpdateTask m_oUpdateTask;         // 0x4d18
};

TSTATICASSERT( sizeof( AGUI2ModelRendererScreen::View ) == 0xc4 );
TSTATICASSERT( sizeof( AGUI2ModelRendererScreen ) == 0x4d38 );
