#pragma once
#ifdef BARNYARD_LATEST

#  include "XURData.h"

#  include <Toshi/TSingleton.h>
#  include <Toshi/T2Map.h>
#  include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIShapeCache : Toshi::TSingleton<AGUI2XUIShapeCache>
// Triangulated outlines of XUI figures, shared between figures with the same path and size.
//-----------------------------------------------------------------------------
class AGUI2XUIShapeCache
    : public Toshi::TSingleton<AGUI2XUIShapeCache>
{
public:
	// What a figure looks a shape up with (it's allocated by the figure)
	struct ShapeKey
	{
		XURXUIFigureData::Point* pPoints;     // 0x0
		TUINT32                  uiNumPoints; // 0x4
		Toshi::TVector2          vecSize;     // 0x8
	};

	struct ShapeKeyComparator
	{
		TINT operator()( const ShapeKey* a_pA, const ShapeKey* a_pB ) const;
	};

	class Shape
	{
	public:
		Shape();
		~Shape();

		TBOOL BuildPoints( XURXUIFigureData::Point* a_pPoints, TUINT32 a_uiNumPoints, const Toshi::TVector2& a_rcSize );
		TBOOL Triangulate( XURXUIFigureData::Point* a_pPoints, TUINT32 a_uiNumPoints );
		TBOOL BuildStroke( Toshi::TVector2** a_ppOutVertices, const Toshi::TVector2& a_rcScale, TFLOAT a_fWidth );

	private:
		TBOOL IsEar( TINT a_iPrev, TINT a_iCurrent, TINT a_iNext );

	public:
		Toshi::TVector2  m_vecMax;             // 0x00
		Toshi::TVector2* m_pVertices;          // 0x08 the outline, curves are split into segments
		Toshi::TVector2* m_pControlPoints;     // 0x0C three per figure point
		TINT*            m_pIndices;           // 0x10 triangle list
		TINT             m_iRefCount;          // 0x14
		TUINT32          m_uiNumControlPoints; // 0x18
		TUINT32          m_uiNumVertices;      // 0x1C
		TUINT32          m_uiNumIndices;       // 0x20
		TUINT32          m_uiNumTriangles;     // 0x24
	};

public:
	AGUI2XUIShapeCache();
	virtual ~AGUI2XUIShapeCache();

	Shape* GetShape( ShapeKey* a_pKey );
	void   ReleaseShape( Shape* a_pShape );

	static void EvaluateBezier( Toshi::TVector2& a_rOutPoint, const Toshi::TVector2* a_pPoints, TINT a_iDegree, TFLOAT a_fT );

public:
	// Number of segments a curved part of a figure is split into
	inline static TINT ms_iCurveSegments = 5;

private:
	Toshi::T2Map<ShapeKey*, Shape*, ShapeKeyComparator> m_mapShapes; // 0x04
};

#endif // BARNYARD_LATEST
