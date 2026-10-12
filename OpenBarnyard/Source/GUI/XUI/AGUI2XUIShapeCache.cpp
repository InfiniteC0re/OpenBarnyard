#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIShapeCache.h"

#  include <Math/TMath.h>
#  include <Math/TMathInline.h>
#  include <Toshi/T2DList.h>
#  include <ToshiTools/T2DynamicVector.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $BarnyardWiiJP: FUNCTION 8041cabc
// NOTE: the original compares the fields with >= one after another, which isn't a strict ordering
TINT AGUI2XUIShapeCache::ShapeKeyComparator::operator()( const ShapeKey* a_pA, const ShapeKey* a_pB ) const
{
	if ( TUINTPTR( a_pA->pPoints ) >= TUINTPTR( a_pB->pPoints ) &&
	     a_pA->uiNumPoints >= a_pB->uiNumPoints &&
	     a_pA->vecSize.x >= a_pB->vecSize.x &&
	     a_pA->vecSize.y >= a_pB->vecSize.y )
	{
		TBOOL bGreater = TUINTPTR( a_pA->pPoints ) > TUINTPTR( a_pB->pPoints ) ||
		    a_pA->uiNumPoints > a_pB->uiNumPoints ||
		    a_pA->vecSize.x > a_pB->vecSize.x ||
		    a_pA->vecSize.y > a_pB->vecSize.y;

		return bGreater ? 1 : 0;
	}

	return -1;
}

// $BarnyardWiiJP: FUNCTION 8041b974
AGUI2XUIShapeCache::AGUI2XUIShapeCache()
{
}

// $BarnyardWiiJP: FUNCTION 8041b9bc
AGUI2XUIShapeCache::~AGUI2XUIShapeCache()
{
}

// $BarnyardWiiJP: FUNCTION 8041ba28
AGUI2XUIShapeCache::Shape* AGUI2XUIShapeCache::GetShape( ShapeKey* a_pKey )
{
	auto   it     = m_mapShapes.Find( a_pKey );
	Shape* pShape = TNULL;

	if ( it == m_mapShapes.End() )
	{
		pShape = new Shape();
		pShape->BuildPoints( a_pKey->pPoints, a_pKey->uiNumPoints, a_pKey->vecSize );
		pShape->Triangulate( a_pKey->pPoints, a_pKey->uiNumPoints );
		m_mapShapes.Insert( a_pKey, pShape );
		pShape->m_iRefCount++;
	}
	else
	{
		pShape = it->GetSecond();
		pShape->m_iRefCount++;
	}

	return pShape;
}

// $BarnyardWiiJP: FUNCTION 8041bb18
void AGUI2XUIShapeCache::ReleaseShape( Shape* a_pShape )
{
	if ( --a_pShape->m_iRefCount != 0 )
		return;

	for ( auto it = m_mapShapes.Begin(); it != m_mapShapes.End(); )
	{
		auto itNext = it;
		itNext++;

		if ( it->GetSecond() == a_pShape )
		{
			m_mapShapes.Remove( it );
			delete a_pShape;
		}

		it = itNext;
	}
}

// $BarnyardWiiJP: FUNCTION 8041b7c0
void AGUI2XUIShapeCache::EvaluateBezier( TVector2& a_rOutPoint, const TVector2* a_pPoints, TINT a_iDegree, TFLOAT a_fT )
{
	if ( a_fT == 0.0f )
	{
		a_rOutPoint = a_pPoints[ 0 ];
		return;
	}

	// NOTE: the original always returns the fourth point here
	if ( a_fT == 1.0f )
	{
		a_rOutPoint = a_pPoints[ 3 ];
		return;
	}

	TVector2 vecResult( 0.0f, 0.0f );
	TFLOAT   fPowT        = 1.0f;
	TFLOAT   fPowInverseT = TMath::Pow( 1.0f - a_fT, TFLOAT( a_iDegree ) );

	for ( TINT i = 0; i <= a_iDegree; i++ )
	{
		TFLOAT fWeight = fPowT * fPowInverseT;
		fPowInverseT /= 1.0f - a_fT;
		fPowT *= a_fT;

		// Binomial coefficient
		TINT iN = a_iDegree;
		TINT iK = i;
		TINT iL = a_iDegree - i;

		for ( TINT k = a_iDegree; k >= 1; k-- )
		{
			fWeight *= TFLOAT( iN-- );

			if ( iK > 1 )
				fWeight /= TFLOAT( iK-- );

			if ( iL > 1 )
				fWeight /= TFLOAT( iL-- );
		}

		vecResult.x += fWeight * a_pPoints[ i ].x;
		vecResult.y += fWeight * a_pPoints[ i ].y;
	}

	a_rOutPoint = vecResult;
}

//-----------------------------------------------------------------------------
// AGUI2XUIShapeCache::Shape
//-----------------------------------------------------------------------------

// $BarnyardWiiJP: FUNCTION 8041bbc4
AGUI2XUIShapeCache::Shape::Shape()
{
	m_vecMax             = TVector2::VEC_ZERO;
	m_pVertices          = TNULL;
	m_pControlPoints     = TNULL;
	m_pIndices           = TNULL;
	m_iRefCount          = 0;
	m_uiNumControlPoints = 0;
	m_uiNumVertices      = 0;
	m_uiNumIndices       = 0;
	m_uiNumTriangles     = 0;
}

// $BarnyardWiiJP: FUNCTION 8041bbfc
AGUI2XUIShapeCache::Shape::~Shape()
{
	delete[] m_pVertices;
	delete[] m_pControlPoints;
	delete[] m_pIndices;
}

// $BarnyardWiiJP: FUNCTION 8041bc7c
TBOOL AGUI2XUIShapeCache::Shape::BuildPoints( XURXUIFigureData::Point* a_pPoints, TUINT32 a_uiNumPoints, const TVector2& a_rcSize )
{
	m_uiNumControlPoints = a_uiNumPoints * 3;
	m_pControlPoints     = new TVector2[ m_uiNumControlPoints ];

	for ( TUINT32 i = 0; i < a_uiNumPoints; i++ )
	{
		XURXUIFigureData::Point& rPoint = a_pPoints[ i ];

		m_vecMax.x = TMath::Max( m_vecMax.x, rPoint.fX );
		m_vecMax.y = TMath::Max( m_vecMax.y, rPoint.fY );
		m_vecMax.x = TMath::Max( m_vecMax.x, rPoint.fCtrl1X );
		m_vecMax.y = TMath::Max( m_vecMax.y, rPoint.fCtrl1Y );
		m_vecMax.x = TMath::Max( m_vecMax.x, rPoint.fCtrl2X );
		m_vecMax.y = TMath::Max( m_vecMax.y, rPoint.fCtrl2Y );
	}

	// NOTE: this scales the points of the figure data itself
	TFLOAT fScaleX = a_rcSize.x / m_vecMax.x;
	TFLOAT fScaleY = a_rcSize.y / m_vecMax.y;

	for ( TUINT32 i = 0; i < a_uiNumPoints; i++ )
	{
		XURXUIFigureData::Point& rPoint = a_pPoints[ i ];

		rPoint.fX *= fScaleX;
		rPoint.fY *= fScaleY;
		rPoint.fCtrl1X *= fScaleX;
		rPoint.fCtrl1Y *= fScaleY;
		rPoint.fCtrl2X *= fScaleX;
		rPoint.fCtrl2Y *= fScaleY;
	}

	// The shape is centered
	TFLOAT fOffsetX = a_rcSize.x * -0.5f;
	TFLOAT fOffsetY = a_rcSize.y * -0.5f;

	for ( TUINT32 i = 0; i < a_uiNumPoints; i++ )
	{
		XURXUIFigureData::Point& rPoint = a_pPoints[ i ];

		m_pControlPoints[ i * 3 + 0 ] = TVector2( fOffsetX + rPoint.fX, fOffsetY + rPoint.fY );
		m_pControlPoints[ i * 3 + 1 ] = TVector2( fOffsetX + rPoint.fCtrl1X, fOffsetY + rPoint.fCtrl1Y );
		m_pControlPoints[ i * 3 + 2 ] = TVector2( fOffsetX + rPoint.fCtrl2X, fOffsetY + rPoint.fCtrl2Y );
	}

	return TTRUE;
}

// Node of the vertex list the triangulation clips ears from
struct XUIEarClipNode
    : public T2DList<XUIEarClipNode>::Node
{
	TINT iIndex; // 0x8
};

// $BarnyardWiiJP: FUNCTION 8041c1dc
TBOOL AGUI2XUIShapeCache::Shape::Triangulate( XURXUIFigureData::Point* a_pPoints, TUINT32 a_uiNumPoints )
{
	// The four control points of a segment, wrapping around
	auto GetSegment = [ this ]( TUINT32 a_uiFirst, TVector2* a_pOutPoints ) {
		for ( TUINT32 k = 0; k < 4; k++ )
		{
			TUINT32 uiIndex = a_uiFirst + k;

			if ( uiIndex >= m_uiNumControlPoints )
				uiIndex = 0;

			a_pOutPoints[ k ] = m_pControlPoints[ uiIndex ];
		}
	};

	TVector2 aSegment[ 4 ];

	// Count the vertices: one for a straight segment, ms_iCurveSegments for a curved one
	m_uiNumVertices = 0;

	for ( TUINT32 i = 0, uiFirst = 0; i < a_uiNumPoints; i++, uiFirst += 3 )
	{
		GetSegment( uiFirst, aSegment );

		if ( aSegment[ 0 ] == aSegment[ 1 ] && aSegment[ 2 ] == aSegment[ 3 ] )
			m_uiNumVertices++;
		else
			m_uiNumVertices += ms_iCurveSegments;
	}

	m_pVertices = new TVector2[ m_uiNumVertices ];

	TINT iVertex = 0;

	for ( TUINT32 i = 0, uiFirst = 0; i < a_uiNumPoints; i++, uiFirst += 3 )
	{
		GetSegment( uiFirst, aSegment );

		if ( aSegment[ 0 ] == aSegment[ 1 ] && aSegment[ 2 ] == aSegment[ 3 ] )
		{
			m_pVertices[ iVertex++ ] = aSegment[ 0 ];
		}
		else
		{
			TFLOAT fT    = 0.0f;
			TFLOAT fStep = 1.0f / TFLOAT( ms_iCurveSegments );

			for ( TINT k = 0; k < ms_iCurveSegments; k++ )
			{
				TVector2 vecPoint;
				EvaluateBezier( vecPoint, aSegment, 3, fT );
				fT += fStep;

				m_pVertices[ iVertex++ ] = vecPoint;
			}
		}
	}

	// Ear clipping
	T2DynamicVector<TINT>   vecIndices( GetGlobalAllocator(), 64, 64 );
	XUIEarClipNode*         pNodes = new XUIEarClipNode[ m_uiNumVertices ];
	T2DList<XUIEarClipNode> llNodes;

	for ( TUINT32 k = 0; k < m_uiNumVertices; k++ )
	{
		pNodes[ k ].iIndex = k;
		llNodes.PushBack( &pNodes[ k ] );
	}

	auto it = llNodes.Begin();
	it++;

	while ( !llNodes.IsEmpty() )
	{
		auto itPrev = it.Prev();
		if ( itPrev == llNodes.End() )
			itPrev = itPrev.Prev();

		auto itNext = it.Next();
		if ( itNext == llNodes.End() )
			itNext = llNodes.Begin();

		TINT iPrev    = itPrev->iIndex;
		TINT iCurrent = it->iIndex;
		TINT iNext    = itNext->iIndex;

		if ( IsEar( iPrev, iCurrent, iNext ) == TTRUE )
		{
			vecIndices.PushBack( iPrev );
			vecIndices.PushBack( iCurrent );
			vecIndices.PushBack( iNext );

			if ( llNodes.Size() == 3 )
			{
				llNodes.Clear();
			}
			else
			{
				XUIEarClipNode* pNode = it.Get();
				it++;
				pNode->Remove();
			}
		}
		else
		{
			it++;
		}

		if ( it == llNodes.End() )
			it = llNodes.Begin();
	}

	delete[] pNodes;

	m_uiNumIndices   = vecIndices.Size();
	m_uiNumTriangles = m_uiNumIndices / 3;
	m_pIndices       = new TINT[ m_uiNumIndices ];
	TUtil::MemCopy( m_pIndices, vecIndices.Data(), vecIndices.Size() * sizeof( TINT ) );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041b628
TBOOL AGUI2XUIShapeCache::Shape::IsEar( TINT a_iPrev, TINT a_iCurrent, TINT a_iNext )
{
	TVector2 vecToPrev( m_pVertices[ a_iPrev ].x - m_pVertices[ a_iCurrent ].x, m_pVertices[ a_iPrev ].y - m_pVertices[ a_iCurrent ].y );
	vecToPrev.Normalize();

	TVector2 vecToNext( m_pVertices[ a_iNext ].x - m_pVertices[ a_iCurrent ].x, m_pVertices[ a_iNext ].y - m_pVertices[ a_iCurrent ].y );
	vecToNext.Normalize();

	TFLOAT fDot = vecToNext.x * vecToPrev.x + vecToNext.y * vecToPrev.y;
	TMath::Clip( fDot, -1.0f, 1.0f );

	TFLOAT fAngle = TMath::ACos( fDot );

	if ( vecToPrev.x * vecToNext.y - vecToPrev.y * vecToNext.x > 0.0f )
		fAngle += TMath::PI;

	if ( fAngle >= TMath::PI )
		return TFALSE;

	// No other vertex can be inside of the ear
	for ( TUINT32 i = 0; i < m_uiNumVertices; i++ )
	{
		if ( i != a_iPrev && i != a_iCurrent && i != a_iNext &&
		     TMath::IsPointInTriangle2D( m_pVertices[ a_iPrev ], m_pVertices[ a_iCurrent ], m_pVertices[ a_iNext ], m_pVertices[ i ] ) == TTRUE )
			return TFALSE;
	}

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041c7c4
TBOOL AGUI2XUIShapeCache::Shape::BuildStroke( TVector2** a_ppOutVertices, const TVector2& a_rcScale, TFLOAT a_fWidth )
{
	TINT     iNumVertices = m_uiNumVertices;
	TINT     iOut         = 0;
	TVector2 vecFirstNormal;

	for ( TINT i = 0; i < iNumVertices; i++ )
	{
		TINT iPrev = i - 1;

		if ( iPrev >= 0 )
		{
			if ( iPrev > iNumVertices - 1 )
				iPrev -= iNumVertices;
		}
		else
		{
			iPrev += iNumVertices;
		}

		TINT iNext = i + 1;

		if ( iNext >= 0 )
		{
			if ( iNext > iNumVertices - 1 )
				iNext -= iNumVertices;
		}
		else
		{
			iNext += iNumVertices;
		}

		// Normals of the two edges
		TVector2 vecNormal1( m_pVertices[ i ].y - m_pVertices[ iPrev ].y, -( m_pVertices[ i ].x - m_pVertices[ iPrev ].x ) );
		vecNormal1.Normalize();

		TVector2 vecNormal2( m_pVertices[ iNext ].y - m_pVertices[ i ].y, -( m_pVertices[ iNext ].x - m_pVertices[ i ].x ) );
		vecNormal2.Normalize();

		TVector2 vecNormal( ( vecNormal1.x + vecNormal2.x ) * 0.5f, ( vecNormal1.y + vecNormal2.y ) * 0.5f );
		vecNormal.Normalize();

		if ( i == 0 )
			vecFirstNormal = vecNormal;

		( *a_ppOutVertices )[ iOut++ ] = TVector2( m_pVertices[ i ].x * a_rcScale.x, m_pVertices[ i ].y * a_rcScale.y );
		( *a_ppOutVertices )[ iOut++ ] = TVector2( m_pVertices[ i ].x * a_rcScale.x + vecNormal.x * a_fWidth, m_pVertices[ i ].y * a_rcScale.y + vecNormal.y * a_fWidth );
	}

	// Close the strip
	( *a_ppOutVertices )[ iOut++ ] = TVector2( m_pVertices[ 0 ].x * a_rcScale.x, m_pVertices[ 0 ].y * a_rcScale.y );
	( *a_ppOutVertices )[ iOut++ ] = TVector2( m_pVertices[ 0 ].x * a_rcScale.x + vecFirstNormal.x * a_fWidth, m_pVertices[ 0 ].y * a_rcScale.y + vecFirstNormal.y * a_fWidth );

	return TTRUE;
}

#endif // BARNYARD_LATEST
