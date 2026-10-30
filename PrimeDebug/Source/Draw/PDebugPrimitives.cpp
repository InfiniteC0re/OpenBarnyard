#include "PrimeDebugPCH.h"
#include "PDebugPrimitives.h"

#ifdef PRIMEDEBUG_DEV
#  include "Util/PArray.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#  define PDEBUGPRIMITIVES_TWO_PI 6.2831855f
#  define PDEBUGPRIMITIVES_PI     3.1415927f

TBOOL                                                PDebugPrimitives::ms_bStarted;
PDebugPrimitives::Lines                              PDebugPrimitives::ms_aLines[ 2 ];
PDebugPrimitives::Shapes<PDebugPrimitives::Box>      PDebugPrimitives::ms_oBoxes;
PDebugPrimitives::Shapes<PDebugPrimitives::Sphere>   PDebugPrimitives::ms_oSpheres;
PDebugPrimitives::Shapes<PDebugPrimitives::Arrow>    PDebugPrimitives::ms_oArrows;
PDebugPrimitives::Shapes<PDebugPrimitives::Pyramid>  PDebugPrimitives::ms_oPyramids;
PDebugPrimitives::Shapes<PDebugPrimitives::Cylinder> PDebugPrimitives::ms_oCylinders;
PDebugPrimitives::Shapes<PDebugPrimitives::Capsule>  PDebugPrimitives::ms_oCapsules;
PDebugPrimitives::Mesh                               PDebugPrimitives::ms_oCapsuleCap;
PDebugVertex*                                        PDebugPrimitives::ms_pTriangles;
TUINT                                                PDebugPrimitives::ms_uiNumTriangles;
TUINT                                                PDebugPrimitives::ms_uiMaxTriangles;
TUINT32                                              PDebugPrimitives::ms_uiAlpha;
TBOOL                                                PDebugPrimitives::ms_bLinesFull;
TBOOL                                                PDebugPrimitives::ms_bLines2DFull;
TBOOL                                                PDebugPrimitives::ms_bCylindersFull;
TBOOL                                                PDebugPrimitives::ms_bCapsulesFull;
TBOOL                                                PDebugPrimitives::ms_bTrianglesFull;
TBOOL                                                PDebugPrimitives::ms_bSpheresFull;
TBOOL                                                PDebugPrimitives::ms_bArrowsFull;
TBOOL                                                PDebugPrimitives::ms_bPyramidsFull;
TBOOL                                                PDebugPrimitives::ms_bBoxesFull;

static const TFLOAT s_afBoxVertices[ 8 * 3 ] = {
	-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
};

static const TUINT16 s_auiBoxSolid[ 36 ] = { 0, 3, 1, 0, 2, 3, 4, 5, 7, 4, 7, 6, 0, 4, 2, 2, 4, 6, 1, 3, 5, 5, 3, 7, 2, 6, 3, 6, 7, 3, 1, 5, 4, 1, 4, 0 };
static const TUINT16 s_auiBoxWire[ 24 ]  = { 0, 1, 1, 3, 3, 2, 2, 0, 4, 5, 5, 7, 7, 6, 6, 4, 3, 7, 6, 2, 1, 5, 4, 0 };

static const TFLOAT  s_afArrowVertices[ 6 * 3 ] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.6f, 0.3f, 0.0f, 0.6f, -0.3f, 0.0f, 0.6f, 0.0f, 0.3f, 0.6f, 0.0f, -0.3f };
static const TUINT16 s_auiArrowLines[ 10 ]      = { 0, 1, 1, 2, 1, 3, 1, 4, 1, 5 };

static const TFLOAT  s_afPyramidVertices[ 5 * 3 ] = { 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
static const TUINT16 s_auiPyramidSolid[ 18 ]      = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 3, 2, 1, 3, 1, 4 };

// The vertices handed to the renderer
static PArray<PDebugVertex> s_oScratch;

// $deBlob2: FUNCTION 008001ac
PDebugPrimitives::Settings::Settings()
{
	iMaxLines     = 0x800;
	iMaxBoxes     = 0x100;
	iMaxSpheres   = 0x100;
	iMaxTriangles = 0x400;
	iMaxCylinders = 0x100;
	iMaxArrows    = 0x80;
	iMaxPyramids  = 0x80;
	iMaxCapsules  = 0x40;
	bLineDepth    = TTRUE;
}

PDebugPrimitives::Box::Box()
{
	vScale.x = 0.0f;
	vScale.y = 0.0f;
	vScale.z = 0.0f;
	vScale.w = 0.0f;
}

// $deBlob2: FUNCTION 00800211
PDebugPrimitives::Box::Box( const Box& a_rOther )
{
	oMatrix      = a_rOther.oMatrix;
	vScale.x     = a_rOther.vScale.x;
	vScale.y     = a_rOther.vScale.y;
	vScale.z     = a_rOther.vScale.z;
	vScale.w     = a_rOther.vScale.w;
	uiColour     = a_rOther.uiColour;
	uiWireColour = a_rOther.uiWireColour;
}

// $deBlob2: FUNCTION 00803bd4
void PDebugPrimitives::Box::SetDepth( TBOOL a_bDepth )
{
	if ( !a_bDepth )
		uiWireColour &= ~DEPTH_FLAG;
	else
		uiWireColour |= DEPTH_FLAG;
}

// $deBlob2: FUNCTION 00800323
PDebugPrimitives::Sphere::Sphere( const Sphere& a_rOther )
{
	oMatrix      = a_rOther.oMatrix;
	fRadius      = a_rOther.fRadius;
	uiColour     = a_rOther.uiColour;
	uiWireColour = a_rOther.uiWireColour;
}

// $deBlob2: FUNCTION 00803c2b
void PDebugPrimitives::Sphere::SetDepth( TBOOL a_bDepth )
{
	if ( !a_bDepth )
		uiWireColour &= ~DEPTH_FLAG;
	else
		uiWireColour |= DEPTH_FLAG;
}

// $deBlob2: FUNCTION 008001e1
PDebugPrimitives::Arrow::Arrow( const Arrow& a_rOther )
{
	oMatrix  = a_rOther.oMatrix;
	fLength  = a_rOther.fLength;
	uiColour = a_rOther.uiColour;
	uiFrames = a_rOther.uiFrames;
}

// $deBlob2: FUNCTION 00800375
PDebugPrimitives::Arrow& PDebugPrimitives::Arrow::operator=( const Arrow& a_rOther )
{
	oMatrix  = a_rOther.oMatrix;
	fLength  = a_rOther.fLength;
	uiColour = a_rOther.uiColour;
	uiFrames = a_rOther.uiFrames;
	return *this;
}

// $deBlob2: FUNCTION 008002b7
PDebugPrimitives::Pyramid::Pyramid( const Pyramid& a_rOther )
{
	oMatrix  = a_rOther.oMatrix;
	fLength  = a_rOther.fLength;
	fWidth   = a_rOther.fWidth;
	uiColour = a_rOther.uiColour;
	uiFrames = a_rOther.uiFrames;
}

// $deBlob2: FUNCTION 008003a2
PDebugPrimitives::Pyramid& PDebugPrimitives::Pyramid::operator=( const Pyramid& a_rOther )
{
	oMatrix  = a_rOther.oMatrix;
	fLength  = a_rOther.fLength;
	fWidth   = a_rOther.fWidth;
	uiColour = a_rOther.uiColour;
	uiFrames = a_rOther.uiFrames;
	return *this;
}

// $deBlob2: FUNCTION 008002ed
PDebugPrimitives::Cylinder::Cylinder( const Cylinder& a_rOther )
{
	oMatrix      = a_rOther.oMatrix;
	fRadius      = a_rOther.fRadius;
	fHeight      = a_rOther.fHeight;
	uiColour     = a_rOther.uiColour;
	uiWireColour = a_rOther.uiWireColour;
}

// $deBlob2: FUNCTION 00803c0e
void PDebugPrimitives::Cylinder::SetDepth( TBOOL a_bDepth )
{
	if ( !a_bDepth )
		uiWireColour &= ~DEPTH_FLAG;
	else
		uiWireColour |= DEPTH_FLAG;
}

// $deBlob2: FUNCTION 00800281
PDebugPrimitives::Capsule::Capsule( const Capsule& a_rOther )
{
	oMatrix      = a_rOther.oMatrix;
	fRadius      = a_rOther.fRadius;
	fLength      = a_rOther.fLength;
	uiColour     = a_rOther.uiColour;
	uiWireColour = a_rOther.uiWireColour;
}

// $deBlob2: FUNCTION 00803bf1
void PDebugPrimitives::Capsule::SetDepth( TBOOL a_bDepth )
{
	if ( !a_bDepth )
		uiWireColour &= ~DEPTH_FLAG;
	else
		uiWireColour |= DEPTH_FLAG;
}

//-----------------------------------------------------------------------------
// Adding
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 008003d5
void PDebugPrimitives::AddArrow( const TMatrix44* a_pMatrix, TFLOAT a_fLength, TUINT32 a_uiColour, TUINT32 a_uiFrames )
{
	if ( ms_oArrows.uiCount < ms_oArrows.uiMax )
	{
		ms_bArrowsFull   = TFALSE;
		Arrow* pArrow    = &ms_oArrows.pShapes[ ms_oArrows.uiCount ];
		pArrow->oMatrix  = *a_pMatrix;
		pArrow->fLength  = a_fLength;
		pArrow->uiColour = a_uiColour;
		pArrow->uiFrames = a_uiFrames;
		ms_oArrows.uiCount++;
		return;
	}

	if ( !ms_bArrowsFull )
		ms_bArrowsFull = TTRUE;
}

// The frame of a direction: a_pDirection along x, the others made with a nearly fixed axis
void PDebugPrimitives::MakeFrame( TMatrix44* a_pOut, const TVector4* a_pPosition, const TVector4* a_pDirection )
{
	const TVector4* d = a_pDirection;

	TFLOAT fY = d->z * 0.1f - d->x * 0.0f;
	TFLOAT fX = d->y * 0.0f - d->z;
	TFLOAT fZ = d->x - d->y * 0.1f;

	TFLOAT   fInv = 1.0f / TFLOAT( sqrt( double( fX * fX + fY * fY + fZ * fZ ) ) );
	TVector4 vUp;
	vUp.y = fY * fInv;
	vUp.z = fZ * fInv;
	vUp.x = fX * fInv;
	vUp.w = fInv * 0.0f;

	TFLOAT fSX = d->z * vUp.y - d->y * vUp.z;
	TFLOAT fSY = d->x * vUp.z - d->z * vUp.x;
	TFLOAT fSZ = d->y * vUp.x - d->x * vUp.y;

	TFLOAT   fInvSide = 1.0f / TFLOAT( sqrt( double( fSX * fSX + fSY * fSY + fSZ * fSZ ) ) );
	TVector4 vSide;
	vSide.x = fSX * fInvSide;
	vSide.z = fSZ * fInvSide;
	vSide.y = fSY * fInvSide;
	vSide.w = fInvSide * 0.0f;

	// Prime's columns are TOSHI's rows
	a_pOut->m_f11 = a_pDirection->x;
	a_pOut->m_f12 = a_pDirection->y;
	a_pOut->m_f13 = a_pDirection->z;
	a_pOut->m_f14 = a_pDirection->w;
	a_pOut->m_f21 = vSide.x;
	a_pOut->m_f22 = vSide.y;
	a_pOut->m_f23 = vSide.z;
	a_pOut->m_f24 = vSide.w;
	a_pOut->m_f31 = vUp.x;
	a_pOut->m_f32 = vUp.y;
	a_pOut->m_f33 = vUp.z;
	a_pOut->m_f34 = vUp.w;
	a_pOut->m_f41 = a_pPosition->x;
	a_pOut->m_f42 = a_pPosition->y;
	a_pOut->m_f43 = a_pPosition->z;
	a_pOut->m_f44 = a_pPosition->w;
}

// $deBlob2: FUNCTION 0080045a
void PDebugPrimitives::AddArrow( const TVector4* a_pPosition, const TVector4* a_pDirection, TFLOAT a_fLength, TUINT32 a_uiColour, TUINT32 a_uiFrames )
{
	TMatrix44 oMatrix;
	MakeFrame( &oMatrix, a_pPosition, a_pDirection );
	AddArrow( &oMatrix, a_fLength, a_uiColour, a_uiFrames );
}

// $deBlob2: FUNCTION 00800949
void PDebugPrimitives::AddCapsule( const TMatrix44* a_pMatrix, TFLOAT a_fRadius, TFLOAT a_fLength, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth )
{
	if ( ms_oCapsules.uiCount < ms_oCapsules.uiMax )
	{
		ms_bCapsulesFull       = TFALSE;
		Capsule* pCapsule      = &ms_oCapsules.pShapes[ ms_oCapsules.uiCount ];
		pCapsule->oMatrix      = *a_pMatrix;
		pCapsule->fRadius      = a_fRadius;
		pCapsule->fLength      = a_fLength;
		pCapsule->uiColour     = a_uiColour;
		pCapsule->uiWireColour = a_uiWireColour;
		pCapsule->SetDepth( a_bDepth );
		ms_oCapsules.uiCount++;
		return;
	}

	if ( !ms_bCapsulesFull )
		ms_bCapsulesFull = TTRUE;
}

// $deBlob2: FUNCTION 008009ff
void PDebugPrimitives::AddEllipse2D( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fRadiusX, TFLOAT a_fRadiusY, TUINT32 a_uiColour )
{
	TFLOAT fRadius = a_fRadiusY;

	if ( a_fRadiusY < a_fRadiusX )
		fRadius = a_fRadiusX;

	TINT iSegments = 0x40;

	if ( TINT( fRadius * 128.0f ) < 0x40 )
		iSegments = TINT( fRadius * 128.0f );

	TINT iCount = 12;

	if ( 12 < iSegments )
		iCount = iSegments;

	TFLOAT fAngle = 0.0f;
	TFLOAT fPrevX = a_fX + a_fRadiusX;
	TFLOAT fPrevY = a_fY;

	for ( TINT i = iCount + 1; i != 0; i-- )
	{
		TFLOAT fX = TFLOAT( cos( double( fAngle ) ) ) * a_fRadiusX + a_fX;
		TFLOAT fY = a_fY - TFLOAT( sin( double( fAngle ) ) ) * a_fRadiusY;
		AddLine2D( fPrevX, fPrevY, fX, fY, a_uiColour );
		fAngle += PDEBUGPRIMITIVES_TWO_PI / TFLOAT( iCount );
		fPrevY = fY;
		fPrevX = fX;
	}
}

// $deBlob2: FUNCTION 00800b1c
void PDebugPrimitives::AddPyramid( const TMatrix44* a_pMatrix, TFLOAT a_fLength, TFLOAT a_fWidth, TUINT32 a_uiColour, TUINT32 a_uiFrames )
{
	if ( ms_oPyramids.uiCount < ms_oPyramids.uiMax )
	{
		ms_bPyramidsFull   = TFALSE;
		Pyramid* pPyramid  = &ms_oPyramids.pShapes[ ms_oPyramids.uiCount ];
		pPyramid->oMatrix  = *a_pMatrix;
		pPyramid->fLength  = a_fLength;
		pPyramid->fWidth   = a_fWidth;
		pPyramid->uiColour = a_uiColour;
		pPyramid->uiFrames = a_uiFrames;
		ms_oPyramids.uiCount++;
		return;
	}

	if ( !ms_bPyramidsFull )
		ms_bPyramidsFull = TTRUE;
}

// $deBlob2: FUNCTION 00800bb8
void PDebugPrimitives::AddPyramid( const TVector4* a_pPosition, const TVector4* a_pDirection, TFLOAT a_fLength, TFLOAT a_fWidth, TUINT32 a_uiColour, TUINT32 a_uiFrames )
{
	TMatrix44 oMatrix;
	MakeFrame( &oMatrix, a_pPosition, a_pDirection );
	AddPyramid( &oMatrix, a_fLength, a_fWidth, a_uiColour, a_uiFrames );
}

// $deBlob2: FUNCTION 00800dbc
void PDebugPrimitives::AddCylinder( const TMatrix44* a_pMatrix, TFLOAT a_fRadius, TFLOAT a_fHeight, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth )
{
	if ( ms_oCylinders.uiCount < ms_oCylinders.uiMax )
	{
		ms_bCylindersFull       = TFALSE;
		Cylinder* pCylinder     = &ms_oCylinders.pShapes[ ms_oCylinders.uiCount ];
		pCylinder->oMatrix      = *a_pMatrix;
		pCylinder->fRadius      = a_fRadius;
		pCylinder->fHeight      = a_fHeight;
		pCylinder->uiColour     = a_uiColour;
		pCylinder->uiWireColour = a_uiWireColour;
		pCylinder->SetDepth( a_bDepth );
		ms_oCylinders.uiCount++;
		return;
	}

	if ( !ms_bCylindersFull )
		ms_bCylindersFull = TTRUE;
}

// $deBlob2: FUNCTION 00800e72
void PDebugPrimitives::AddLine2D( TFLOAT a_fX0, TFLOAT a_fY0, TFLOAT a_fX1, TFLOAT a_fY1, TUINT32 a_uiColour )
{
	AddLine2D( a_fX0, a_fY0, a_fX1, a_fY1, a_uiColour, a_uiColour );
}

// NOTE: Prime keeps the vertices in clip space (x * 2 - 1, 1 - y * 2); here they stay fractions of the screen
// $deBlob2: FUNCTION 00800eb3
void PDebugPrimitives::AddLine2D( TFLOAT a_fX0, TFLOAT a_fY0, TFLOAT a_fX1, TFLOAT a_fY1, TUINT32 a_uiColour0, TUINT32 a_uiColour1 )
{
	Lines& rLines = ms_aLines[ 0 ];

	if ( rLines.uiCount < rLines.uiMax - 1 )
	{
		ms_bLines2DFull     = TFALSE;
		PDebugVertex* pFrom = &rLines.pVertices[ rLines.uiCount ];
		pFrom->fX           = a_fX0;
		pFrom->fY           = a_fY0;
		pFrom->fZ           = 0.0f;
		pFrom->uiColour     = a_uiColour0;
		rLines.uiCount++;

		PDebugVertex* pTo = &rLines.pVertices[ rLines.uiCount ];
		pTo->fX           = a_fX1;
		pTo->fY           = a_fY1;
		pTo->fZ           = 0.0f;
		pTo->uiColour     = a_uiColour1;
		rLines.uiCount++;
		return;
	}

	if ( !ms_bLines2DFull )
		ms_bLines2DFull = TTRUE;
}

// $deBlob2: FUNCTION 00800fae
void PDebugPrimitives::AddLinePixels( TINT a_iX0, TINT a_iY0, TINT a_iX1, TINT a_iY1, TUINT32 a_uiColour )
{
	AddLinePixels( a_iX0, a_iY0, a_iX1, a_iY1, a_uiColour, a_uiColour );
}

// In pixels of the screen
// $deBlob2: FUNCTION 00800fcd
void PDebugPrimitives::AddLinePixels( TINT a_iX0, TINT a_iY0, TINT a_iX1, TINT a_iY1, TUINT32 a_uiColour0, TUINT32 a_uiColour1 )
{
	AddLine2D( PixelsToX( a_iX0 ), PixelsToY( a_iY0 ), PixelsToX( a_iX1 ), PixelsToY( a_iY1 ), a_uiColour0, a_uiColour1 );
}

// $deBlob2: FUNCTION 0080103f
void PDebugPrimitives::AddLine( const TVector4* a_pFrom, const TVector4* a_pTo, TUINT32 a_uiColour )
{
	Lines& rLines = ms_aLines[ 1 ];

	if ( rLines.uiCount < rLines.uiMax - 1 )
	{
		ms_bLinesFull       = TFALSE;
		PDebugVertex* pFrom = &rLines.pVertices[ rLines.uiCount ];
		pFrom->fX           = a_pFrom->x;
		pFrom->fY           = a_pFrom->y;
		pFrom->fZ           = a_pFrom->z;
		pFrom->uiColour     = a_uiColour;
		rLines.uiCount++;

		PDebugVertex* pTo = &rLines.pVertices[ rLines.uiCount ];
		pTo->fX           = a_pTo->x;
		pTo->fY           = a_pTo->y;
		pTo->fZ           = a_pTo->z;
		pTo->uiColour     = a_uiColour;
		rLines.uiCount++;
		return;
	}

	if ( !ms_bLinesFull )
		ms_bLinesFull = TTRUE;
}

// $deBlob2: FUNCTION 008010f9
void PDebugPrimitives::AddSphere( const TMatrix44* a_pMatrix, TFLOAT a_fRadius, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth )
{
	if ( ms_oSpheres.uiCount < ms_oSpheres.uiMax )
	{
		ms_bSpheresFull       = TFALSE;
		Sphere* pSphere       = &ms_oSpheres.pShapes[ ms_oSpheres.uiCount ];
		pSphere->oMatrix      = *a_pMatrix;
		pSphere->fRadius      = a_fRadius;
		pSphere->uiColour     = a_uiColour;
		pSphere->uiWireColour = a_uiWireColour;
		pSphere->SetDepth( a_bDepth );
		ms_oSpheres.uiCount++;
		return;
	}

	if ( !ms_bSpheresFull )
		ms_bSpheresFull = TTRUE;
}

// NOTE: Prime has the box queue and its drawing but no function to add one in de Blob 2; this one is made like the
// other shapes' (a_pSize: the size along x, y and z)
void PDebugPrimitives::AddBox( const TMatrix44* a_pMatrix, const TVector4* a_pSize, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth )
{
	if ( ms_oBoxes.uiCount < ms_oBoxes.uiMax )
	{
		ms_bBoxesFull      = TFALSE;
		Box* pBox          = &ms_oBoxes.pShapes[ ms_oBoxes.uiCount ];
		pBox->oMatrix      = *a_pMatrix;
		pBox->vScale       = TVector4( a_pSize->x, a_pSize->y, a_pSize->z, 1.0f );
		pBox->uiColour     = a_uiColour;
		pBox->uiWireColour = a_uiWireColour;
		pBox->SetDepth( a_bDepth );
		ms_oBoxes.uiCount++;
		return;
	}

	if ( !ms_bBoxesFull )
		ms_bBoxesFull = TTRUE;
}

// The edges are added as lines when their colour isn't transparent
// $deBlob2: FUNCTION 008011ff
void PDebugPrimitives::AddTriangle( const TVector4* a_pA, const TVector4* a_pB, const TVector4* a_pC, TUINT32 a_uiColour, TUINT32 a_uiEdgeColour )
{
	if ( ms_uiNumTriangles < ms_uiMaxTriangles )
	{
		ms_bTrianglesFull     = TFALSE;
		PDebugVertex* pVertex = &ms_pTriangles[ ms_uiNumTriangles * 3 ];
		pVertex[ 0 ].fX       = a_pA->x;
		pVertex[ 0 ].fY       = a_pA->y;
		pVertex[ 0 ].fZ       = a_pA->z;
		pVertex[ 0 ].uiColour = a_uiColour;
		pVertex[ 1 ].fX       = a_pB->x;
		pVertex[ 1 ].fY       = a_pB->y;
		pVertex[ 1 ].uiColour = a_uiColour;
		pVertex[ 1 ].fZ       = a_pB->z;
		pVertex[ 2 ].fX       = a_pC->x;
		pVertex[ 2 ].fY       = a_pC->y;
		pVertex[ 2 ].uiColour = a_uiColour;
		pVertex[ 2 ].fZ       = a_pC->z;

		if ( a_uiEdgeColour & 0xFF000000 )
		{
			AddLine( a_pA, a_pB, a_uiEdgeColour );
			AddLine( a_pB, a_pC, a_uiEdgeColour );
			AddLine( a_pC, a_pA, a_uiEdgeColour );
		}

		ms_uiNumTriangles++;
		return;
	}

	if ( !ms_bTrianglesFull )
		ms_bTrianglesFull = TTRUE;
}

// $deBlob2: FUNCTION 0080398c
TFLOAT PDebugPrimitives::PixelsToX( TINT a_iX )
{
	PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer();
	return pRenderer ? TFLOAT( a_iX ) / TFLOAT( pRenderer->GetScreenWidth() ) : 0.0f;
}

// $deBlob2: FUNCTION 008039b2
TFLOAT PDebugPrimitives::PixelsToY( TINT a_iY )
{
	PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer();
	return pRenderer ? TFLOAT( a_iY ) / TFLOAT( pRenderer->GetScreenHeight() ) : 0.0f;
}

// $deBlob2: FUNCTION 00803c48
void PDebugPrimitives::SetLineDepth( TBOOL a_bDepth )
{
	ms_aLines[ 1 ].bDepth = a_bDepth;
}

// The alpha of the blend factor the shapes are drawn with.
// NOTE: in Prime the blend factor is only the alpha of what's written to the render target, the shapes are blended
// by their own alpha; it's kept but not used here
// $deBlob2: FUNCTION 00803c55
void PDebugPrimitives::SetAlpha( TUINT32 a_uiAlpha )
{
	ms_uiAlpha = a_uiAlpha;
}

TBOOL PDebugPrimitives::HasWorldPrimitives()
{
	return ms_bStarted && ( ms_uiNumTriangles || ms_aLines[ 1 ].uiCount || ms_oBoxes.uiCount || ms_oSpheres.uiCount || ms_oCylinders.uiCount || ms_oCapsules.uiCount || ms_oArrows.uiCount || ms_oPyramids.uiCount );
}

//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 008019f8
void PDebugPrimitives::Flush2D()
{
	Lines& rLines = ms_aLines[ 0 ];

	if ( rLines.uiCount )
	{
		if ( PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer() )
			pRenderer->DrawLines2D( rLines.pVertices, TINT( rLines.uiCount ) );

		rLines.uiCount = 0;
	}
}

void PDebugPrimitives::DrawMesh( const Mesh& a_rcMesh, const TMatrix44& a_rcMatrix, const TVector4& a_rcScale, TUINT32 a_uiColour, TBOOL a_bWire, TBOOL a_bStrip, TBOOL a_bDepth )
{
	PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer();

	if ( !pRenderer )
		return;

	const TUINT16* pIndices    = a_bWire ? a_rcMesh.pWire : a_rcMesh.pSolid;
	TINT           iNumIndices = a_bWire ? a_rcMesh.iNumWire : a_rcMesh.iNumSolid;

	s_oScratch.SetSize( 0 );

	auto AddVertex = [ & ]( TUINT16 a_uiIndex ) {
		const TFLOAT* p = &a_rcMesh.pVertices[ a_uiIndex * 3 ];
		TFLOAT        x = p[ 0 ] * a_rcScale.x;
		TFLOAT        y = p[ 1 ] * a_rcScale.y;
		TFLOAT        z = p[ 2 ] * a_rcScale.z;

		PDebugVertex oVertex;
		oVertex.fX       = x * a_rcMatrix.m_f11 + y * a_rcMatrix.m_f21 + z * a_rcMatrix.m_f31 + a_rcMatrix.m_f41;
		oVertex.fY       = x * a_rcMatrix.m_f12 + y * a_rcMatrix.m_f22 + z * a_rcMatrix.m_f32 + a_rcMatrix.m_f42;
		oVertex.fZ       = x * a_rcMatrix.m_f13 + y * a_rcMatrix.m_f23 + z * a_rcMatrix.m_f33 + a_rcMatrix.m_f43;
		oVertex.uiColour = a_uiColour;
		s_oScratch.PushBack( oVertex );
	};

	if ( a_bWire )
	{
		for ( TINT i = 0; i + 1 < iNumIndices; i += 2 )
		{
			AddVertex( pIndices[ i ] );
			AddVertex( pIndices[ i + 1 ] );
		}

		pRenderer->DrawLines3D( s_oScratch.Data(), s_oScratch.Size(), a_bDepth, TTRUE );
	}
	else
	{
		if ( a_bStrip )
		{
			// The triangles of the strip, the degenerate ones (that join its parts) left out
			for ( TINT i = 0; i + 2 < iNumIndices; i++ )
			{
				TUINT16 a = pIndices[ i ];
				TUINT16 b = pIndices[ i + 1 ];
				TUINT16 c = pIndices[ i + 2 ];

				if ( a == b || b == c || a == c )
					continue;

				AddVertex( a );
				AddVertex( b );
				AddVertex( c );
			}
		}
		else
		{
			for ( TINT i = 0; i + 2 < iNumIndices; i += 3 )
			{
				AddVertex( pIndices[ i ] );
				AddVertex( pIndices[ i + 1 ] );
				AddVertex( pIndices[ i + 2 ] );
			}
		}

		pRenderer->DrawTriangles3D( s_oScratch.Data(), s_oScratch.Size(), a_bDepth );
	}
}

// A translation along the matrix's own axes
static TMatrix44 PDebugPrimitives_Translate( const TMatrix44& a_rcMatrix, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ )
{
	TMatrix44 oResult = a_rcMatrix;
	oResult.m_f41 += a_fX * a_rcMatrix.m_f11 + a_fY * a_rcMatrix.m_f21 + a_fZ * a_rcMatrix.m_f31;
	oResult.m_f42 += a_fX * a_rcMatrix.m_f12 + a_fY * a_rcMatrix.m_f22 + a_fZ * a_rcMatrix.m_f32;
	oResult.m_f43 += a_fX * a_rcMatrix.m_f13 + a_fY * a_rcMatrix.m_f23 + a_fZ * a_rcMatrix.m_f33;
	return oResult;
}

// $deBlob2: FUNCTION 00801b07
// $deBlob2: FUNCTION 00801b1b
void PDebugPrimitives::Render()
{
	PDebugRenderer* pRenderer = PDebugRenderer::GetRenderer();

	if ( !ms_bStarted || !pRenderer )
		return;

	TBOOL bDepth = TTRUE;

	if ( ms_uiNumTriangles )
	{
		pRenderer->DrawTriangles3D( ms_pTriangles, TINT( ms_uiNumTriangles * 3 ), bDepth );
		ms_uiNumTriangles = 0;
	}

	Lines& rLines = ms_aLines[ 1 ];

	if ( rLines.uiCount )
	{
		pRenderer->DrawLines3D( rLines.pVertices, TINT( rLines.uiCount ), rLines.bDepth, TFALSE );
		bDepth         = rLines.bDepth;
		rLines.uiCount = 0;
	}

	if ( ms_oBoxes.uiCount )
	{
		for ( TUINT i = 0; i < ms_oBoxes.uiCount; i++ )
		{
			Box oBox( ms_oBoxes.pShapes[ i ] );
			bDepth = ( oBox.uiWireColour >> 16 ) & 1;

			if ( oBox.uiColour & 0xFF000000 )
				DrawMesh( ms_oBoxes.oMesh, oBox.oMatrix, oBox.vScale, oBox.uiColour, TFALSE, TFALSE, bDepth );

			if ( oBox.uiWireColour & 0xFF000000 )
				DrawMesh( ms_oBoxes.oMesh, oBox.oMatrix, oBox.vScale, oBox.uiWireColour & ~DEPTH_FLAG, TTRUE, TFALSE, bDepth );
		}

		bDepth            = TTRUE;
		ms_oBoxes.uiCount = 0;
	}

	if ( ms_oCylinders.uiCount )
	{
		for ( TUINT i = 0; i < ms_oCylinders.uiCount; i++ )
		{
			Cylinder oCylinder( ms_oCylinders.pShapes[ i ] );
			bDepth = ( oCylinder.uiWireColour >> 16 ) & 1;

			TVector4 vScale( oCylinder.fRadius, oCylinder.fHeight, oCylinder.fRadius, 1.0f );

			if ( oCylinder.uiColour & 0xFF000000 )
				DrawMesh( ms_oCylinders.oMesh, oCylinder.oMatrix, vScale, oCylinder.uiColour, TFALSE, TTRUE, bDepth );

			if ( oCylinder.uiWireColour & 0xFF000000 )
				DrawMesh( ms_oCylinders.oMesh, oCylinder.oMatrix, vScale, oCylinder.uiWireColour & ~DEPTH_FLAG, TTRUE, TTRUE, bDepth );
		}

		bDepth                = TTRUE;
		ms_oCylinders.uiCount = 0;
	}

	if ( ms_oCapsules.uiCount )
	{
		for ( TUINT i = 0; i < ms_oCapsules.uiCount; i++ )
		{
			Capsule oCapsule( ms_oCapsules.pShapes[ i ] );
			bDepth = ( oCapsule.uiWireColour >> 16 ) & 1;

			TFLOAT fRadius = oCapsule.fRadius;
			TFLOAT fLength = oCapsule.fLength;

			// The body, the top half sphere (turned over) and the bottom one
			TVector4  vBodyScale( fRadius, fLength, fRadius, 1.0f );
			TVector4  vTopScale( fRadius, -fRadius, fRadius, 1.0f );
			TVector4  vCapScale( fRadius, fRadius, fRadius, 1.0f );
			TMatrix44 oTop    = PDebugPrimitives_Translate( oCapsule.oMatrix, 0.0f, fLength * 0.5f, 0.0f );
			TMatrix44 oBottom = PDebugPrimitives_Translate( oTop, 0.0f, -fLength, 0.0f );

			for ( TINT iWire = 0; iWire < 2; iWire++ )
			{
				TUINT32 uiColour = iWire ? ( oCapsule.uiWireColour & ~DEPTH_FLAG ) : oCapsule.uiColour;

				if ( !( uiColour & 0xFF000000 ) )
					continue;

				DrawMesh( ms_oCapsules.oMesh, oCapsule.oMatrix, vBodyScale, uiColour, iWire, TTRUE, bDepth );
				DrawMesh( ms_oCapsuleCap, oTop, vTopScale, uiColour, iWire, TTRUE, bDepth );
				DrawMesh( ms_oCapsuleCap, oBottom, vCapScale, uiColour, iWire, TTRUE, bDepth );
			}
		}

		bDepth               = TTRUE;
		ms_oCapsules.uiCount = 0;
	}

	// NOTE: the depth test is left as the last sphere had it
	if ( ms_oSpheres.uiCount )
	{
		for ( TUINT i = 0; i < ms_oSpheres.uiCount; i++ )
		{
			Sphere oSphere( ms_oSpheres.pShapes[ i ] );
			bDepth = ( oSphere.uiWireColour >> 16 ) & 1;

			TVector4 vScale( oSphere.fRadius, oSphere.fRadius, oSphere.fRadius, 1.0f );

			if ( oSphere.uiColour & 0xFF000000 )
				DrawMesh( ms_oSpheres.oMesh, oSphere.oMatrix, vScale, oSphere.uiColour, TFALSE, TTRUE, bDepth );

			if ( oSphere.uiWireColour & 0xFF000000 )
				DrawMesh( ms_oSpheres.oMesh, oSphere.oMatrix, vScale, oSphere.uiWireColour & ~DEPTH_FLAG, TTRUE, TTRUE, bDepth );
		}

		ms_oSpheres.uiCount = 0;
	}

	// Arrows and pyramids stay for their number of frames: the ones left are moved down
	TUINT uiKept = 0;

	if ( ms_oArrows.uiCount )
	{
		bDepth = TFALSE;
		uiKept = 0;

		for ( TUINT i = 0; i < ms_oArrows.uiCount; i++ )
		{
			Arrow oArrow( ms_oArrows.pShapes[ i ] );

			Mesh oMesh;
			oMesh.pVertices    = const_cast<TFLOAT*>( s_afArrowVertices );
			oMesh.iNumVertices = 6;
			oMesh.pSolid       = TNULL;
			oMesh.iNumSolid    = 0;
			oMesh.pWire        = const_cast<TUINT16*>( s_auiArrowLines );
			oMesh.iNumWire     = 10;
			DrawMesh( oMesh, oArrow.oMatrix, TVector4( oArrow.fLength, oArrow.fLength, oArrow.fLength, 1.0f ), oArrow.uiColour, TTRUE, TFALSE, bDepth );

			if ( --oArrow.uiFrames != 0 )
				ms_oArrows.pShapes[ uiKept++ ] = oArrow;
		}
	}

	ms_oArrows.uiCount = uiKept;
	uiKept             = 0;

	if ( ms_oPyramids.uiCount )
	{
		uiKept = 0;

		for ( TUINT i = 0; i < ms_oPyramids.uiCount; i++ )
		{
			Pyramid oPyramid( ms_oPyramids.pShapes[ i ] );

			Mesh oMesh;
			oMesh.pVertices    = const_cast<TFLOAT*>( s_afPyramidVertices );
			oMesh.iNumVertices = 5;
			oMesh.pSolid       = const_cast<TUINT16*>( s_auiPyramidSolid );
			oMesh.iNumSolid    = 18;
			oMesh.pWire        = TNULL;
			oMesh.iNumWire     = 0;
			DrawMesh( oMesh, oPyramid.oMatrix, TVector4( oPyramid.fLength, oPyramid.fWidth, oPyramid.fWidth, 1.0f ), oPyramid.uiColour, TFALSE, TFALSE, bDepth );

			if ( --oPyramid.uiFrames != 0 )
				ms_oPyramids.pShapes[ uiKept++ ] = oPyramid;
		}
	}

	ms_oPyramids.uiCount = uiKept;
}

//-----------------------------------------------------------------------------
// Meshes
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00803e8b
void PDebugPrimitives::GetSphereSizes( TINT* a_pNumVertices, TINT* a_pNumSolid, TINT* a_pNumWire, TINT a_iSlices, TINT a_iStacks, TBOOL a_bHalf )
{
	*a_pNumVertices = a_iSlices * a_iStacks + 2;

	TINT iSolid  = ( ( a_iStacks + 4 ) * a_iSlices + a_iStacks * 2 ) * 2 - 4;
	*a_pNumSolid = iSolid;

	if ( a_bHalf )
		*a_pNumSolid = iSolid + a_iSlices * -5;

	TINT iWire  = ( a_iStacks * 2 + 1 ) * a_iSlices * 2;
	*a_pNumWire = iWire;

	if ( a_bHalf )
		*a_pNumWire = iWire + a_iSlices * -2;
}

// Rings of a_iSlices points between the poles at -1 (bottom) and 1 (top); half: the bottom half and no top pole. The
// solid one is a strip.
// $deBlob2: FUNCTION 008012dc
void PDebugPrimitives::BuildSphere( TFLOAT* a_pVertices, TINT* a_pNumVertices, TUINT16* a_pSolid, TINT* a_pNumSolid, TUINT16* a_pWire, TINT* a_pNumWire, TINT a_iSlices, TINT a_iStacks, TBOOL a_bHalf )
{
	TINT iNumVertices = 2;
	a_pVertices[ 0 ]  = 0.0f;
	a_pVertices[ 2 ]  = 0.0f;
	a_pVertices[ 1 ]  = -1.0f;
	a_pVertices[ 3 ]  = 0.0f;
	a_pVertices[ 5 ]  = 0.0f;
	a_pVertices[ 4 ]  = 1.0f;

	TFLOAT fStep = PDEBUGPRIMITIVES_PI / TFLOAT( a_iStacks + 1 );

	if ( a_bHalf )
		fStep = ( TFLOAT( ceil( double( TFLOAT( a_iStacks + 1 ) * 0.5f ) ) ) / TFLOAT( a_iStacks + 1 ) ) * fStep;

	TFLOAT* pVertex = a_pVertices + 6;

	for ( TINT i = 0; i < a_iStacks; )
	{
		i++;
		TFLOAT fSin  = TFLOAT( sin( double( TFLOAT( i ) * fStep ) ) );
		TFLOAT fCos  = TFLOAT( cos( double( TFLOAT( i ) * fStep ) ) );
		TFLOAT fRing = fCos * 0.0f - fSin * -1.0f;
		TFLOAT fY    = fSin * 0.0f - fCos * 1.0f;

		if ( a_iSlices > 0 )
		{
			iNumVertices += a_iSlices;

			for ( TINT j = 0; j < a_iSlices; )
			{
				j++;
				TFLOAT fAngle = TFLOAT( j ) * ( PDEBUGPRIMITIVES_TWO_PI / TFLOAT( a_iSlices ) );
				TFLOAT fS     = TFLOAT( sin( double( fAngle ) ) );
				TFLOAT fC     = TFLOAT( cos( double( fAngle ) ) );
				pVertex[ 0 ]  = fRing * fC + fS * 0.0f;
				pVertex[ 1 ]  = fY;
				pVertex[ 2 ]  = fC * 0.0f - fRing * fS;
				pVertex += 3;
			}
		}
	}

	*a_pNumVertices = iNumVertices;

	TINT iNumSolid = ( ( a_iStacks + 4 ) * a_iSlices + a_iStacks * 2 ) * 2 - 4;

	if ( a_bHalf )
		iNumSolid += a_iSlices * -5;

	// The bottom cap, then the top one, then the bands between the rings
	TINT n = 0;

	for ( TINT i = 0; i < a_iSlices; i++ )
	{
		TUINT16 uiNext  = TUINT16( ( i + 1 ) % a_iSlices + 2 );
		TUINT16 uiFirst = 0;
		TUINT16 uiLast  = uiNext;

		if ( i & 1 )
		{
			uiLast  = 0;
			uiFirst = uiNext;
		}

		a_pSolid[ n + 0 ] = uiFirst;
		a_pSolid[ n + 1 ] = uiFirst;
		a_pSolid[ n + 2 ] = TUINT16( i + 2 );
		a_pSolid[ n + 3 ] = uiLast;
		a_pSolid[ n + 4 ] = uiLast;
		n += 5;
	}

	TUINT16 uiSlices = TUINT16( a_iSlices );

	if ( !a_bHalf )
	{
		TUINT16 uiBase   = TUINT16( ( a_iStacks - 1 ) * uiSlices + 2 );
		TUINT16 uiParity = TUINT16( n ) & 1;

		for ( TINT i = 0; i < a_iSlices; i++ )
		{
			TUINT16 uiNext = TUINT16( ( i + 1 ) % a_iSlices + uiBase );
			TUINT16 uiFirst, uiLast;

			if ( TINT( TINT16( uiParity ) ) == ( i & 1 ) )
			{
				uiLast  = 1;
				uiFirst = uiNext;
			}
			else
			{
				uiFirst = 1;
				uiLast  = uiNext;
			}

			a_pSolid[ n + 0 ] = uiFirst;
			a_pSolid[ n + 1 ] = uiFirst;
			a_pSolid[ n + 2 ] = TUINT16( i + uiBase );
			a_pSolid[ n + 3 ] = uiLast;
			a_pSolid[ n + 4 ] = uiLast;
			n += 5;
		}
	}

	TINT    iBands = a_iStacks - 1;
	TUINT16 uiRing = 2;

	for ( TINT k = iBands; k > 0; k-- )
	{
		a_pSolid[ n++ ] = uiRing;

		for ( TINT j = 0; j < a_iSlices; j++ )
		{
			a_pSolid[ n + 0 ] = TUINT16( uiRing + j );
			a_pSolid[ n + 1 ] = TUINT16( uiRing + uiSlices + j );
			n += 2;
		}

		a_pSolid[ n + 0 ] = uiRing;
		a_pSolid[ n + 1 ] = TUINT16( uiRing + uiSlices );
		a_pSolid[ n + 2 ] = TUINT16( uiRing + uiSlices );
		n += 3;
		uiRing = TUINT16( uiRing + a_iSlices );
	}

	*a_pNumSolid = iNumSolid;

	TINT iNumWire = ( a_iStacks * 2 + 1 ) * a_iSlices * 2;

	if ( a_bHalf )
		iNumWire += a_iSlices * -2;

	// The rings, the lines between them, the lines to the poles
	n      = 0;
	uiRing = 2;

	for ( TINT k = a_iStacks; k > 0; k-- )
	{
		for ( TINT j = 0; j < a_iSlices; )
		{
			a_pWire[ n + 0 ] = TUINT16( uiRing + j );
			j++;
			a_pWire[ n + 1 ] = TUINT16( j % a_iSlices + uiRing );
			n += 2;
		}

		uiRing = TUINT16( uiRing + a_iSlices );
	}

	uiRing = 2;

	for ( TINT k = iBands; k > 0; k-- )
	{
		for ( TINT j = 0; j < a_iSlices; j++ )
		{
			a_pWire[ n + 0 ] = TUINT16( uiRing + j );
			a_pWire[ n + 1 ] = TUINT16( uiRing + uiSlices + j );
			n += 2;
		}

		uiRing = TUINT16( uiRing + a_iSlices );
	}

	for ( TINT j = 0; j < a_iSlices; j++ )
	{
		a_pWire[ n + 0 ] = 0;
		a_pWire[ n + 1 ] = TUINT16( j + 2 );
		n += 2;
	}

	if ( !a_bHalf )
	{
		for ( TINT j = 0; j < a_iSlices; j++ )
		{
			a_pWire[ n + 0 ] = 1;
			a_pWire[ n + 1 ] = TUINT16( TUINT16( iBands ) * uiSlices + 2 + j );
			n += 2;
		}
	}

	*a_pNumWire = iNumWire;
}

void PDebugPrimitives::BuildSphereMesh( Mesh& a_rMesh, TINT a_iSlices, TINT a_iStacks, TBOOL a_bHalf )
{
	GetSphereSizes( &a_rMesh.iNumVertices, &a_rMesh.iNumSolid, &a_rMesh.iNumWire, a_iSlices, a_iStacks, a_bHalf );
	a_rMesh.pSolid    = new TUINT16[ a_rMesh.iNumSolid ];
	a_rMesh.pWire     = new TUINT16[ a_rMesh.iNumWire ];
	a_rMesh.pVertices = new TFLOAT[ a_rMesh.iNumVertices * 3 ];
	BuildSphere( a_rMesh.pVertices, &a_rMesh.iNumVertices, a_rMesh.pSolid, &a_rMesh.iNumSolid, a_rMesh.pWire, &a_rMesh.iNumWire, a_iSlices, a_iStacks, a_bHalf );
}

// A cylinder of height 1 along y: a ring at each end with its centre first. The solid one is a strip.
void PDebugPrimitives::BuildTube( TINT a_iSlices, Mesh& a_rMesh )
{
	TINT    iNumVertices = a_iSlices * 2 + 2;
	TINT    iRing        = iNumVertices / 2;
	TFLOAT* pVertices    = new TFLOAT[ iNumVertices * 3 ];
	TFLOAT* pTop         = pVertices;
	TFLOAT* pBottom      = pVertices + iRing * 3;
	TFLOAT  fStep        = PDEBUGPRIMITIVES_TWO_PI / TFLOAT( a_iSlices );

	pTop[ 0 ]    = 0.0f;
	pTop[ 2 ]    = 0.0f;
	pTop[ 1 ]    = 0.5f;
	pBottom[ 0 ] = 0.0f;
	pBottom[ 2 ] = 0.0f;
	pBottom[ 1 ] = -0.5f;

	for ( TINT i = 0; i < a_iSlices; )
	{
		i++;
		TFLOAT fAngle = TFLOAT( i ) * fStep;
		TFLOAT fSin   = TFLOAT( sin( double( fAngle ) ) );
		TFLOAT fCos   = TFLOAT( cos( double( fAngle ) ) );
		TFLOAT fZ     = ( fCos * -1.0f - fSin * 0.0f ) + 0.0f;
		TFLOAT fX     = ( fCos * 0.0f - fSin * 1.0f ) + 0.0f;

		pTop[ i * 3 + 1 ]    = 0.5f;
		pTop[ i * 3 + 2 ]    = fZ;
		pTop[ i * 3 + 0 ]    = fX;
		pBottom[ i * 3 + 0 ] = fX;
		pBottom[ i * 3 + 1 ] = -0.5f;
		pBottom[ i * 3 + 2 ] = fZ;
	}

	a_rMesh.pVertices    = pVertices;
	a_rMesh.iNumVertices = iNumVertices;

	// Pairs of quads around the side, each starting and ending at the top centre
	TUINT    uiNumSolid = ( ( a_iSlices % 2 ) ? 6 : 0 ) + ( a_iSlices / 2 ) * 12;
	TUINT16* pSolid     = new TUINT16[ uiNumSolid ];
	TUINT16* pIndex     = pSolid;
	TFLOAT   fPairs     = TFLOAT( ceil( double( TFLOAT( a_iSlices ) * 0.5f ) ) );

	if ( fPairs > 0.0f )
	{
		TINT    iFirst = TINT( fPairs );
		TINT    iNext  = TINT( fPairs ) + 1;
		TINT    iPair  = 0;
		TUINT16 uiRing = TUINT16( iRing );

		do
		{
			TINT iA     = ( iFirst + 1 - TINT( fPairs ) ) % a_iSlices;
			pIndex[ 0 ] = 0;
			pIndex[ 1 ] = TUINT16( iPair + 1 );
			pIndex[ 2 ] = TUINT16( iA + 1 );
			pIndex[ 3 ] = TUINT16( uiRing + 1 + iPair );
			pIndex[ 4 ] = TUINT16( uiRing + 1 + iA );
			pIndex[ 5 ] = uiRing;

			if ( iFirst >= a_iSlices )
				break;

			TINT iB      = iNext % a_iSlices;
			pIndex[ 6 ]  = uiRing;
			pIndex[ 7 ]  = TUINT16( uiRing + 1 + iB );
			pIndex[ 8 ]  = TUINT16( uiRing + 1 + iFirst );
			pIndex[ 9 ]  = TUINT16( iB + 1 );
			pIndex[ 10 ] = TUINT16( iFirst + 1 );
			pIndex[ 11 ] = 0;
			pIndex += 12;

			iPair++;
			iFirst++;
			iNext++;
		} while ( TFLOAT( iPair ) < fPairs );
	}

	a_rMesh.pSolid    = pSolid;
	a_rMesh.iNumSolid = TINT( uiNumSolid );

	// The rings and the lines between them
	TINT     iNumWire = a_iSlices * 6;
	TUINT16* pWire    = new TUINT16[ iNumWire ];
	pIndex            = pWire;

	for ( TINT i = 1; i <= a_iSlices; i++ )
	{
		TUINT16 uiNext = TUINT16( i % a_iSlices );
		TUINT16 uiDown = TUINT16( a_iSlices + 1 + i );
		pIndex[ 0 ]    = TUINT16( i );
		pIndex[ 1 ]    = TUINT16( uiNext + 1 );
		pIndex[ 2 ]    = TUINT16( i );
		pIndex[ 3 ]    = uiDown;
		pIndex[ 4 ]    = uiDown;
		pIndex[ 5 ]    = TUINT16( a_iSlices + 2 + uiNext );
		pIndex += 6;
	}

	a_rMesh.pWire    = pWire;
	a_rMesh.iNumWire = iNumWire;
}

void PDebugPrimitives::FreeMesh( Mesh& a_rMesh )
{
	delete[] a_rMesh.pVertices;
	delete[] a_rMesh.pSolid;
	delete[] a_rMesh.pWire;
	a_rMesh.pVertices    = TNULL;
	a_rMesh.pSolid       = TNULL;
	a_rMesh.pWire        = TNULL;
	a_rMesh.iNumVertices = 0;
	a_rMesh.iNumSolid    = 0;
	a_rMesh.iNumWire     = 0;
}

//-----------------------------------------------------------------------------
// Startup
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 00803ee5
// $deBlob2: FUNCTION 00803820
// $deBlob2: FUNCTION 008033ec
// $deBlob2: FUNCTION 00802e7f
void PDebugPrimitives::Startup( const Settings* a_pSettings, TINT a_iSphereSlices, TINT a_iSphereStacks, TINT a_iCylinderSlices, TINT a_iCapsuleSlices )
{
	ms_uiAlpha = 0xC0;

	for ( TINT i = 0; i < 2; i++ )
	{
		Lines& rLines    = ms_aLines[ i ];
		rLines.uiCount   = 0;
		rLines.uiMax     = a_pSettings->iMaxTriangles * 6 + a_pSettings->iMaxLines;
		rLines.bDepth    = ( i == 1 ) ? a_pSettings->bLineDepth : TFALSE;
		rLines.pVertices = new PDebugVertex[ rLines.uiMax ];
	}

	ms_oBoxes.uiMax   = a_pSettings->iMaxBoxes;
	ms_oBoxes.uiCount = 0;
	ms_oBoxes.pShapes = new Box[ a_pSettings->iMaxBoxes ];

	ms_oBoxes.oMesh.iNumVertices = 8;
	ms_oBoxes.oMesh.pVertices    = new TFLOAT[ 8 * 3 ];
	ms_oBoxes.oMesh.iNumSolid    = 36;
	ms_oBoxes.oMesh.pSolid       = new TUINT16[ 36 ];
	ms_oBoxes.oMesh.iNumWire     = 24;
	ms_oBoxes.oMesh.pWire        = new TUINT16[ 24 ];
	memcpy( ms_oBoxes.oMesh.pVertices, s_afBoxVertices, sizeof( s_afBoxVertices ) );
	memcpy( ms_oBoxes.oMesh.pSolid, s_auiBoxSolid, sizeof( s_auiBoxSolid ) );
	memcpy( ms_oBoxes.oMesh.pWire, s_auiBoxWire, sizeof( s_auiBoxWire ) );

	ms_uiMaxTriangles = a_pSettings->iMaxTriangles;
	ms_uiNumTriangles = 0;
	ms_pTriangles     = new PDebugVertex[ a_pSettings->iMaxTriangles * 3 ];

	// The spheres (geDebugPrimitives_CreateSpheres)
	ms_oSpheres.uiCount = 0;
	ms_oSpheres.uiMax   = a_pSettings->iMaxSpheres;
	ms_oSpheres.pShapes = new Sphere[ a_pSettings->iMaxSpheres ];
	BuildSphereMesh( ms_oSpheres.oMesh, a_iSphereSlices, a_iSphereStacks, TFALSE );

	// The cylinders (geDebugPrimitives_CreateCylinders)
	ms_oCylinders.uiCount = 0;
	ms_oCylinders.uiMax   = a_pSettings->iMaxCylinders;
	ms_oCylinders.pShapes = new Cylinder[ a_pSettings->iMaxCylinders ];
	BuildTube( a_iCylinderSlices, ms_oCylinders.oMesh );

	// The capsules: a tube and a half sphere of 6 stacks (geDebugPrimitives_CreateCapsules)
	ms_oCapsules.uiCount = 0;
	ms_oCapsules.uiMax   = a_pSettings->iMaxCapsules;
	ms_oCapsules.pShapes = new Capsule[ a_pSettings->iMaxCapsules ];
	BuildSphereMesh( ms_oCapsuleCap, a_iCapsuleSlices, 6, TTRUE );
	BuildTube( a_iCapsuleSlices, ms_oCapsules.oMesh );

	ms_oArrows.uiCount = 0;
	ms_oArrows.uiMax   = a_pSettings->iMaxArrows;
	ms_oArrows.pShapes = new Arrow[ a_pSettings->iMaxArrows ];

	ms_oPyramids.uiCount = 0;
	ms_oPyramids.uiMax   = a_pSettings->iMaxPyramids;
	ms_oPyramids.pShapes = new Pyramid[ a_pSettings->iMaxPyramids ];

	ms_bStarted = TTRUE;
}

// $deBlob2: FUNCTION 00803c62
void PDebugPrimitives::Shutdown()
{
	if ( !ms_bStarted )
		return;

	ms_bStarted = TFALSE;

	delete[] ms_oArrows.pShapes;
	ms_oArrows.pShapes = TNULL;
	ms_oArrows.uiCount = 0;

	delete[] ms_oPyramids.pShapes;
	ms_oPyramids.pShapes = TNULL;
	ms_oPyramids.uiCount = 0;

	FreeMesh( ms_oSpheres.oMesh );
	delete[] ms_oSpheres.pShapes;
	ms_oSpheres.pShapes = TNULL;
	ms_oSpheres.uiCount = 0;

	FreeMesh( ms_oBoxes.oMesh );
	delete[] ms_oBoxes.pShapes;
	ms_oBoxes.pShapes = TNULL;
	ms_oBoxes.uiCount = 0;

	FreeMesh( ms_oCylinders.oMesh );
	delete[] ms_oCylinders.pShapes;
	ms_oCylinders.pShapes = TNULL;
	ms_oCylinders.uiCount = 0;

	FreeMesh( ms_oCapsules.oMesh );
	FreeMesh( ms_oCapsuleCap );
	delete[] ms_oCapsules.pShapes;
	ms_oCapsules.pShapes = TNULL;
	ms_oCapsules.uiCount = 0;

	for ( TINT i = 0; i < 2; i++ )
	{
		delete[] ms_aLines[ i ].pVertices;
		ms_aLines[ i ].pVertices = TNULL;
		ms_aLines[ i ].uiCount   = 0;
	}

	delete[] ms_pTriangles;
	ms_pTriangles     = TNULL;
	ms_uiNumTriangles = 0;

	s_oScratch.Free();
}

#endif // PRIMEDEBUG_DEV
