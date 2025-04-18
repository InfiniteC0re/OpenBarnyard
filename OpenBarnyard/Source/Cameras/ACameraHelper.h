#pragma once
#include "ACamera.h"

class ACameraHelper : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACameraHelper, Toshi::TObject );

public:
	ACameraHelper();
	~ACameraHelper();

	virtual void  UpdateCurrentCamera();
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) = 0;
	virtual TBOOL Unk2( void* )                   = 0;
	virtual TBOOL Unk3( void*, void* )            = 0;
	virtual void  OnCreate()                      = 0;
	virtual void  OnDestroy()                     = 0;
	virtual void  OnAttach()                      = 0;
	virtual void  GetCameraRotation( Toshi::TVector4& a_rOutRotation ) const;
	virtual void  Unk7( void* );
	// ...
	virtual void   OnDetach();
	virtual TFLOAT GetFOV() const;

	ACamera* GetCamera() { return m_pCamera; }
	void     SetCamera( ACamera* a_pCamera ) { m_pCamera = a_pCamera; }

protected:
	// ...
	ACamera* m_pCamera;
};
