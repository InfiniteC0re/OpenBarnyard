#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"
#  include "AGUI2XUIShapeCache.h"

#  include <GUI/T2GUIMaterial.h>

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIFigure : AGUI2XUIElement
// A filled (solid colour, texture or gradient) and stroked path.
//-----------------------------------------------------------------------------
class AGUI2XUIFigure
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUIFigure, AGUI2XUIElement );

	using DIRTY = TUINT32;
	enum DIRTY_ : DIRTY
	{
		DIRTY_NONE     = 0,
		DIRTY_UNK1     = BITFLAG( 0 ),
		DIRTY_GRADIENT = BITFLAG( 1 ),
		DIRTY_STROKE   = BITFLAG( 2 ),
		DIRTY_UVS      = BITFLAG( 3 ),
	};

	// Values of the XUI FillType property
	enum FILLTYPE : TUINT32
	{
		FILLTYPE_NONE,
		FILLTYPE_SOLID,
		FILLTYPE_LINEARGRADIENT,
		FILLTYPE_RADIALGRADIENT,
		FILLTYPE_TEXTURE,
	};

public:
	AGUI2XUIFigure();
	~AGUI2XUIFigure();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void PreRender() override;
	virtual void Render() override;
	virtual void PostRender() override;
	virtual void SetWidth( TFLOAT a_fWidth ) override;
	virtual void SetHeight( TFLOAT a_fHeight ) override;
	virtual void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;
	virtual void  UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT ) override;
	virtual void  UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT ) override;

	void SetGradientStopPos( TUINT32 a_uiIndex, TFLOAT a_fPos );
	void SetGradientStopColour( TUINT32 a_uiIndex, TUINT32 a_uiColour );

	static TUINT32 LerpColour( TFLOAT a_fT, TUINT32 a_uiFrom, TUINT32 a_uiTo );

	// Debug switches of the original
	inline static TBOOL ms_bDrawOutlines      = TFALSE;
	inline static TBOOL ms_bDumpGradientImage = TFALSE;

private:
	TBOOL SetupFill( AGUI2XUIResource& a_rResource, XURXUIFillData* a_pFill );
	TBOOL SetGradient( XURXUIGradientData* a_pGradient, TBOOL a_bRadial );
	void  GenerateGradient( TBOOL a_bRadial );
	TBOOL UpdateUVs();

private:
	TBOOL                         m_bClosed;             // 0x98
	DIRTY                         m_uiDirtyFlags;        // 0x9C
	Toshi::TVector2*              m_pUVs;                // 0xA0
	Toshi::T2GUIMaterial*         m_pMaterial;           // 0xA4
	Toshi::TTexture*              m_pGradientTexture;    // 0xA8
	TFLOAT                        m_fStrokeWidth;        // 0xAC
	TUINT32                       m_uiStrokeColour;      // 0xB0
	Toshi::T2GUITransform         m_oFillTransform;      // 0xB4
	TUINT32                       m_uiNumGradientStops;  // 0xCC
	TUINT32*                      m_pGradientColours;    // 0xD0
	TFLOAT*                       m_pGradientPositions;  // 0xD4
	TBOOL                         m_bRadialGradient;     // 0xD8
	Toshi::TVector2               m_vecOriginalSize;     // 0xDC
	Toshi::TVector2               m_vecScale;            // 0xE4
	Toshi::T2GUITransform         m_oUnkTransform;       // 0xEC
	AGUI2XUIShapeCache::ShapeKey* m_pShapeKey;           // 0x104
	AGUI2XUIShapeCache::Shape*    m_pShape;              // 0x108
	Toshi::TVector2*              m_pStrokeVertices;     // 0x10C
	TUINT32                       m_uiNumStrokeVertices; // 0x110
	TUINT32*                      m_pGradientPixels;     // 0x114
};

#endif // BARNYARD_LATEST
