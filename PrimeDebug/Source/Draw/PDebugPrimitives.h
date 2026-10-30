#pragma once
#include "PDebugRenderer.h"

#ifdef PRIMEDEBUG_DEV
#  include <Math/TVector4.h>
#  include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Purpose: Prime's debug primitives (geDebugPrimitives): lines on the screen
// and in the world, triangles, and shapes queued for a frame: boxes,
// spheres, cylinders, capsules (solid and/or wire, with or without the depth
// test), arrows and pyramids (kept for a number of frames). Render draws the
// world ones, Flush2D the screen lines.
// A shape's matrix is its frame in the world (TOSHI's row vectors: the
// translation in the last row); cylinders and capsules are along its y,
// arrows and pyramids along its x.
// NOTE: Prime keeps one mesh per kind in vertex and index buffers and draws
// each shape with its matrix in a shader; here the meshes are kept as arrays
// and each shape is transformed into lines and triangles for
// PDebugRenderer. Colours are 0xAARRGGBB; a colour with alpha 0 isn't drawn.
//-----------------------------------------------------------------------------
class PDebugPrimitives
{
public:
	// In a shape's wire colour: draw it with the depth test
	static constexpr TUINT32 DEPTH_FLAG = 0x10000;

	// How many of each kind can be queued in a frame
	struct Settings
	{
		TINT  iMaxLines;     // 0x00
		TINT  iMaxBoxes;     // 0x04
		TINT  iMaxSpheres;   // 0x08
		TINT  iMaxArrows;    // 0x0C
		TINT  iMaxPyramids;  // 0x10
		TINT  iMaxTriangles; // 0x14, each also has room for its three edges in the lines
		TINT  iMaxCylinders; // 0x18
		TINT  iMaxCapsules;  // 0x1C
		TBOOL bLineDepth;    // 0x20, the depth test for 3D lines

		Settings();
	};

	struct Box
	{
		Toshi::TMatrix44 oMatrix;      // 0x00
		Toshi::TVector4  vScale;       // 0x40
		TUINT32          uiColour;     // 0x50, not drawn with alpha 0
		TUINT32          uiWireColour; // 0x54, | DEPTH_FLAG

		Box();
		Box( const Box& a_rOther );
		Box& operator=( const Box& a_rOther ) = default;

		void SetDepth( TBOOL a_bDepth );
	};

	struct Sphere
	{
		Toshi::TMatrix44 oMatrix;      // 0x00
		TFLOAT           fRadius;      // 0x40
		TUINT32          uiColour;     // 0x44
		TUINT32          uiWireColour; // 0x48

		Sphere() = default;
		Sphere( const Sphere& a_rOther );
		Sphere& operator=( const Sphere& a_rOther ) = default;

		void SetDepth( TBOOL a_bDepth );
	};

	// Kept for a number of frames
	struct Arrow
	{
		Toshi::TMatrix44 oMatrix;  // 0x00, x along the arrow
		TFLOAT           fLength;  // 0x40
		TUINT32          uiColour; // 0x44
		TUINT32          uiFrames; // 0x48

		Arrow() = default;
		Arrow( const Arrow& a_rOther );
		Arrow& operator=( const Arrow& a_rOther );
	};

	struct Pyramid
	{
		Toshi::TMatrix44 oMatrix;  // 0x00, the apex at the origin, the base along x
		TFLOAT           fLength;  // 0x40
		TFLOAT           fWidth;   // 0x44
		TUINT32          uiColour; // 0x48
		TUINT32          uiFrames; // 0x4C

		Pyramid() = default;
		Pyramid( const Pyramid& a_rOther );
		Pyramid& operator=( const Pyramid& a_rOther );
	};

	struct Cylinder
	{
		Toshi::TMatrix44 oMatrix;      // 0x00, along y
		TFLOAT           fRadius;      // 0x40
		TFLOAT           fHeight;      // 0x44
		TUINT32          uiColour;     // 0x48
		TUINT32          uiWireColour; // 0x4C

		Cylinder() = default;
		Cylinder( const Cylinder& a_rOther );
		Cylinder& operator=( const Cylinder& a_rOther ) = default;

		void SetDepth( TBOOL a_bDepth );
	};

	struct Capsule
	{
		Toshi::TMatrix44 oMatrix;      // 0x00, along y
		TFLOAT           fRadius;      // 0x40
		TFLOAT           fLength;      // 0x44, of the cylinder between the half spheres
		TUINT32          uiColour;     // 0x48
		TUINT32          uiWireColour; // 0x4C

		Capsule() = default;
		Capsule( const Capsule& a_rOther );
		Capsule& operator=( const Capsule& a_rOther ) = default;

		void SetDepth( TBOOL a_bDepth );
	};

public:
	static void  Startup( const Settings* a_pSettings, TINT a_iSphereSlices, TINT a_iSphereStacks, TINT a_iCylinderSlices, TINT a_iCapsuleSlices );
	static void  Shutdown();
	static TBOOL IsStarted() { return ms_bStarted; }

	static void SetLineDepth( TBOOL a_bDepth );
	static void SetAlpha( TUINT32 a_uiAlpha );

	// On screen, in fractions of it
	static void   AddLine2D( TFLOAT a_fX0, TFLOAT a_fY0, TFLOAT a_fX1, TFLOAT a_fY1, TUINT32 a_uiColour );
	static void   AddLine2D( TFLOAT a_fX0, TFLOAT a_fY0, TFLOAT a_fX1, TFLOAT a_fY1, TUINT32 a_uiColour0, TUINT32 a_uiColour1 );
	static void   AddLinePixels( TINT a_iX0, TINT a_iY0, TINT a_iX1, TINT a_iY1, TUINT32 a_uiColour );
	static void   AddLinePixels( TINT a_iX0, TINT a_iY0, TINT a_iX1, TINT a_iY1, TUINT32 a_uiColour0, TUINT32 a_uiColour1 );
	static void   AddEllipse2D( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fRadiusX, TFLOAT a_fRadiusY, TUINT32 a_uiColour );
	static TFLOAT PixelsToX( TINT a_iX );
	static TFLOAT PixelsToY( TINT a_iY );
	static void   Flush2D();

	// In the world
	static void AddLine( const Toshi::TVector4* a_pFrom, const Toshi::TVector4* a_pTo, TUINT32 a_uiColour );
	static void AddTriangle( const Toshi::TVector4* a_pA, const Toshi::TVector4* a_pB, const Toshi::TVector4* a_pC, TUINT32 a_uiColour, TUINT32 a_uiEdgeColour );
	static void AddBox( const Toshi::TMatrix44* a_pMatrix, const Toshi::TVector4* a_pSize, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth );
	static void AddSphere( const Toshi::TMatrix44* a_pMatrix, TFLOAT a_fRadius, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth );
	static void AddCylinder( const Toshi::TMatrix44* a_pMatrix, TFLOAT a_fRadius, TFLOAT a_fHeight, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth );
	static void AddCapsule( const Toshi::TMatrix44* a_pMatrix, TFLOAT a_fRadius, TFLOAT a_fLength, TUINT32 a_uiColour, TUINT32 a_uiWireColour, TBOOL a_bDepth );
	static void AddArrow( const Toshi::TMatrix44* a_pMatrix, TFLOAT a_fLength, TUINT32 a_uiColour, TUINT32 a_uiFrames );
	static void AddArrow( const Toshi::TVector4* a_pPosition, const Toshi::TVector4* a_pDirection, TFLOAT a_fLength, TUINT32 a_uiColour, TUINT32 a_uiFrames );
	static void AddPyramid( const Toshi::TMatrix44* a_pMatrix, TFLOAT a_fLength, TFLOAT a_fWidth, TUINT32 a_uiColour, TUINT32 a_uiFrames );
	static void AddPyramid( const Toshi::TVector4* a_pPosition, const Toshi::TVector4* a_pDirection, TFLOAT a_fLength, TFLOAT a_fWidth, TUINT32 a_uiColour, TUINT32 a_uiFrames );
	static void Render();

	// TTRUE if anything is queued for Render
	static TBOOL HasWorldPrimitives();

public:
	// A mesh of positions (x, y, z), its solid indices (a strip or a list) and its wire indices (pairs)
	struct Mesh
	{
		TFLOAT*  pVertices;
		TINT     iNumVertices;
		TUINT16* pSolid;
		TINT     iNumSolid;
		TUINT16* pWire;
		TINT     iNumWire;
	};

private:
	struct Lines
	{
		TUINT         uiCount; // 0x00, of vertices
		TUINT         uiMax;   // 0x04
		PDebugVertex* pVertices;
		TBOOL         bDepth;
	};

	template <class T>
	struct Shapes
	{
		TUINT uiCount;
		TUINT uiMax;
		T*    pShapes;
		Mesh  oMesh;
	};

	static void MakeFrame( Toshi::TMatrix44* a_pOut, const Toshi::TVector4* a_pPosition, const Toshi::TVector4* a_pDirection );
	static void GetSphereSizes( TINT* a_pNumVertices, TINT* a_pNumSolid, TINT* a_pNumWire, TINT a_iSlices, TINT a_iStacks, TBOOL a_bHalf );
	static void BuildSphere( TFLOAT* a_pVertices, TINT* a_pNumVertices, TUINT16* a_pSolid, TINT* a_pNumSolid, TUINT16* a_pWire, TINT* a_pNumWire, TINT a_iSlices, TINT a_iStacks, TBOOL a_bHalf );
	static void BuildSphereMesh( Mesh& a_rMesh, TINT a_iSlices, TINT a_iStacks, TBOOL a_bHalf );
	static void BuildTube( TINT a_iSlices, Mesh& a_rMesh );
	static void FreeMesh( Mesh& a_rMesh );

	// Transforms a mesh by a matrix and a scale and hands its solid (a strip or a list) or wire part to the renderer
	static void DrawMesh( const Mesh& a_rcMesh, const Toshi::TMatrix44& a_rcMatrix, const Toshi::TVector4& a_rcScale, TUINT32 a_uiColour, TBOOL a_bWire, TBOOL a_bStrip, TBOOL a_bDepth );

private:
	static TBOOL            ms_bStarted;
	static Lines            ms_aLines[ 2 ]; // on screen, in the world
	static Shapes<Box>      ms_oBoxes;
	static Shapes<Sphere>   ms_oSpheres;
	static Shapes<Arrow>    ms_oArrows;
	static Shapes<Pyramid>  ms_oPyramids;
	static Shapes<Cylinder> ms_oCylinders;
	static Shapes<Capsule>  ms_oCapsules;
	static Mesh             ms_oCapsuleCap; // the half sphere at each end
	static PDebugVertex*    ms_pTriangles;  // three a triangle
	static TUINT            ms_uiNumTriangles;
	static TUINT            ms_uiMaxTriangles;
	static TUINT32          ms_uiAlpha;

	// Set once a kind is full, cleared by the next one that fits
	static TBOOL ms_bLinesFull;
	static TBOOL ms_bLines2DFull;
	static TBOOL ms_bCylindersFull;
	static TBOOL ms_bCapsulesFull;
	static TBOOL ms_bTrianglesFull;
	static TBOOL ms_bSpheresFull;
	static TBOOL ms_bArrowsFull;
	static TBOOL ms_bPyramidsFull;
	static TBOOL ms_bBoxesFull;
};

#endif // PRIMEDEBUG_DEV
