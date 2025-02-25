#pragma once
#include "Math/TMatrix44.h"
#include "Render/TMaterial.h"
#include "Render/TMesh.h"
#include "Render/TAnimation.h"

TOSHI_NAMESPACE_START

class TRenderPacket
{
public:
	TRenderPacket();
	~TRenderPacket();

	TRenderPacket* GetNextPacket() { return m_pNextPacket; }
	void           SetNextPacket( TRenderPacket* a_pNextPacket ) { m_pNextPacket = a_pNextPacket; }

	TMesh* GetMesh() const { return m_pMesh; }
	void   SetMesh( TMesh* a_pMesh ) { m_pMesh = a_pMesh; }

	TSkeletonInstance* GetSkeletonInstance() const { return m_pSkeletonInstance; }
	void               SetSkeletonInstance( TSkeletonInstance* a_pSkeletonInstance ) { m_pSkeletonInstance = a_pSkeletonInstance; }

	TMaterial* GetMaterial() const { return m_pMaterial; }
	void       SetMaterial( TMaterial* pMaterial ) { m_pMaterial = pMaterial; }

	TMatrix44& GetModelViewMatrix() { return m_oModelView; }
	void       SetModelViewMatrix( TMatrix44& a_rMatrix44 ) { m_oModelView = a_rMatrix44; }

	void            SetAmbientColour( const TVector3& a_rColour ) { m_oAmbientColour = a_rColour; }
	const TVector3& GetAmbientColour() const { return m_oAmbientColour; }

	void            SetLightColour( const TVector3& a_rColour ) { m_oLightColour = a_rColour; }
	const TVector3& GetLightColour() const { return m_oLightColour; }

	void            SetLightDirection( const TVector3& a_rDirection ) { m_oLightDirection = a_rDirection; }
	const TVector3& GetLightDirection() const { return m_oLightDirection; }

	TUINT8 GetShadeCoeff() const { return m_ui8ShadeCoeff; }
	void   SetShadeCoeff( TUINT8 val ) { m_ui8ShadeCoeff = val; }

	TFLOAT GetAlpha() const { return m_fAlpha; }
	void   SetAlpha( TFLOAT val ) { m_fAlpha = val; }

#ifndef TOSHI_SDK
private:
#endif
	TRenderPacket*     m_pNextPacket;
	void*              m_pUnk;
	TMesh*             m_pMesh;
	TMatrix44          m_oModelView;
	TSkeletonInstance* m_pSkeletonInstance;
	TMaterial*         m_pMaterial;
	TVector3           m_oAmbientColour;
	TVector3           m_oLightColour;
	TVector3           m_oLightDirection;
	TUINT8             m_ui8Unk1;
	TUINT8             m_ui8ShadeCoeff;
	TFLOAT             m_fAlpha;
};

TOSHI_NAMESPACE_END
