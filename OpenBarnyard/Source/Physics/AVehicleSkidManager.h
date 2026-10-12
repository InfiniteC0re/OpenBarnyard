#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

class ASysMesh;
class ASysMaterial;

namespace Toshi
{
class TTexture;
}

//-----------------------------------------------------------------------------
// Purpose: Tyre tracks left on the ground by the vehicles (ABikeSteer).
// Owns a pool of quads (one ASysMesh each) textured with
// Particles\tyretrack.tga. A vehicle asks for a track with GetFreeTrack (the
// oldest one is recycled) and fills its TyreTrack. Every track fades out by
// being hidden once its age reaches its lifetime.
// TClass 0x007cd044, vtable 0x0073afb4, size 0x34, singleton 0x007cd040.
// Created in AGameSystemManager::OnCreate with 200 tracks.
//-----------------------------------------------------------------------------
class AVehicleSkidManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AVehicleSkidManager>
{
public:
	TDECLARE_CLASS( AVehicleSkidManager, Toshi::TTask );

	//-----------------------------------------------------------------------------
	// A single segment of a tyre track (0x50 bytes). The quad is built around
	// vPosition: fWidth wide along the side axis (cross of vNormal and vDirection)
	// and fLength long along vDirection, lifted by 0.05 along VEC_NEGY (up).
	// To use one: fill vPosition, vNormal, vDirection, fWidth, fLength, uiColour
	// and fUVOffset of the track returned by GetFreeTrack (it resets fAge and
	// fUVOffset to 0, so the track is visible until fAge reaches fLifetime).
	//-----------------------------------------------------------------------------
	struct TyreTrack
	{
		TyreTrack();
		~TyreTrack();

		// Returns TFALSE if the track is outside the frustum of the particle system
		TBOOL IsVisible() const;

		// Fills the mesh with the quad of the track and renders it
		void Render();

		ASysMesh*       pMesh;      // 0x00
		TINT            iUnk4;      // 0x04, never used
		Toshi::TVector4 vPosition;  // 0x08, start of the track (w copied to the matrix)
		Toshi::TVector4 vNormal;    // 0x18, normal of the ground (VEC_NEGY by default)
		Toshi::TVector4 vDirection; // 0x28, normalised direction of the track (VEC_POSX by default)
		TFLOAT          fWidth;     // 0x38
		TFLOAT          fLength;    // 0x3c, also the radius used for the visibility test
		TUINT32         uiColour;   // 0x40, ARGB colour of the vertices
		TFLOAT          fUVOffset;  // 0x44, V coordinate at the end of the track (V at the start = fUVOffset + fLength / fWidth)
		TFLOAT          fAge;       // 0x48, the track is hidden while >= fLifetime
		TFLOAT          fLifetime;  // 0x4c, 60 seconds
	};

public:
	AVehicleSkidManager();
	~AVehicleSkidManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE { return TTRUE; }     // +0x18, RETURNS_TRUE (0046cde0)
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x1c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Creates the material and a_iNumTracks tracks
	void Create( TINT a_iNumTracks );

	// Returns the oldest track (reset to age 0) or TNULL if the material or the texture isn't loaded
	TyreTrack* GetFreeTrack();

	// Hides all the tracks
	void HideAllTracks();

	// Finds the texture of the tracks (AAssetLoader::CreateAssetsFromLibrary)
	void LoadTyreTrackTexture();

	// Hides all the tracks and forgets the texture (AAssetLoader, when unloading the level)
	void ReleaseTyreTrackTexture();

	// Renders the visible tracks (ASimState::OnRender)
	void Render();

private:
	// Stores the track with the highest age in m_pOldestTrack
	void UpdateOldestTrack();

private:
	ASysMaterial*    m_pMaterial;    // 0x20
	Toshi::TTexture* m_pTexture;     // 0x24
	TyreTrack*       m_pTracks;      // 0x28
	TyreTrack*       m_pOldestTrack; // 0x2c, the next track returned by GetFreeTrack
	TINT             m_iNumTracks;   // 0x30
};

TSTATICASSERT( sizeof( AVehicleSkidManager::TyreTrack ) == 0x50 );
