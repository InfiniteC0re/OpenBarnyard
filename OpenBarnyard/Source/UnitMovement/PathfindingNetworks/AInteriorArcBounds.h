#pragma once
#include <Math/TVector2.h>

// Corner of an area and the edge going from it to the next corner
struct AInteriorArcBounds
{
	const Toshi::TVector2& GetPoint() const { return *TREINTERPRETCAST( const Toshi::TVector2*, &m_fX ); }
	const Toshi::TVector2& GetNormal() const { return *TREINTERPRETCAST( const Toshi::TVector2*, &m_fNormalX ); }
	const Toshi::TVector2& GetCornerNormal() const { return *TREINTERPRETCAST( const Toshi::TVector2*, &m_fCornerNormalX ); }

	TFLOAT m_fX;
	TFLOAT m_fZ;
	TFLOAT m_fNormalX; // normal of the edge (points inside of the area)
	TFLOAT m_fNormalZ;
	TFLOAT m_fCornerNormalX; // normal of the corner
	TFLOAT m_fCornerNormalZ;
	TINT   m_iNeighborArea; // area on the other side of the edge, -1 if it's an outer edge
};
