#pragma once
#include "TGUITextureSection.h"

#include <Toshi/TArray.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: animated texture section ("[textureanimations]" blocks): a list
// of frames (rectangles of the texture) shown for m_fFrequency seconds each.
// Frame 0 is the texture section itself, the frames of the list start at 1.
//-----------------------------------------------------------------------------
class TGUITextureAnimation : public TGUITextureSection
{
public:
	TDECLARE_CLASS( TGUITextureAnimation, TGUITextureSection );

	struct Frame
	{
		TUINT16 uiX;      // 0x0
		TUINT16 uiY;      // 0x2
		TUINT16 uiWidth;  // 0x4
		TUINT16 uiHeight; // 0x6
	};

public:
	TGUITextureAnimation();
	TGUITextureAnimation( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iNumFrames, TFLOAT a_fFrequency, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iRotation, TBOOL a_bWrap, TBOOL a_bMirrorX, TBOOL a_bMirrorY );
	TGUITextureAnimation( const TGUITextureAnimation& a_rcOther );
	~TGUITextureAnimation();

	virtual TGUITextureAnimation* Clone() const override;
	virtual void                  GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TFLOAT a_fTime ) const override;
	virtual void                  GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TINT a_iFrame ) const override;
	virtual void                  SetBBox( TUINT16 a_uiX, TUINT16 a_uiY, TUINT16 a_uiWidth, TUINT16 a_uiHeight, TINT a_iFrame ) override;

	// a_iFrame starts at 1
	void InsertFrame( TINT a_iFrame, Frame& a_rFrame );
	void RemoveFrame( TINT a_iFrame );

	// $JPOG: FUNCTION TGuiInterface.dll+0001a530
	Frame& GetFrame( TINT a_iFrame ) { return m_aFrames[ a_iFrame - 1 ]; }

	// $JPOG: FUNCTION TGuiInterface.dll+0001ab50
	TArray<Frame>& GetFrames() { return m_aFrames; }

	// $JPOG: FUNCTION TGuiInterface.dll+0001ab60
	TFLOAT GetFrequency() const { return m_fFrequency; }

	// $JPOG: FUNCTION TGuiInterface.dll+0001ab40
	void SetFrequency( TFLOAT a_fFrequency ) { m_fFrequency = a_fFrequency; }

private:
	TFLOAT        m_fFrequency; // 0x24 duration of a frame
	TArray<Frame> m_aFrames;    // 0x28
};

TOSHI_NAMESPACE_END
