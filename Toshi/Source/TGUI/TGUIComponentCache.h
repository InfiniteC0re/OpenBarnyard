#pragma once
#include "TGUIRectangle.h"

TOSHI_NAMESPACE_START

class TGUIComponent;

//-----------------------------------------------------------------------------
// Purpose: cache of a widget for the components of its style: the bounding
// box of the widget and data the components compute when they paint it (the
// dimensions of their elements, formatted texts...), kept until the cache is
// invalidated. The components destroy their data (TGUIComponent::DestroyCache).
//-----------------------------------------------------------------------------
class TGUIComponentCache
{
public:
	// Data of a component
	struct Data
	{
		Data*                pNext;      // 0x0
		Data*                pPrev;      // 0x4
		const TGUIComponent* pComponent; // 0x8
		void*                pData;      // 0xC
	};

public:
	TGUIComponentCache();
	TGUIComponentCache( const TGUIComponentCache& a_rcOther );
	~TGUIComponentCache();

	// Copies the bounding box (the data isn't copied, the cache is invalidated)
	TGUIComponentCache& operator=( const TGUIComponentCache& a_rcOther );

	// Destroys the data of the components
	void Invalidate();

	void* GetData( const TGUIComponent* a_pComponent ) const;

	// Sets the data of a component (its previous data is destroyed)
	void SetData( const TGUIComponent* a_pComponent, void* a_pData );

	// $JPOG: FUNCTION TGuiInterface.dll+00006b10
	const TGUIRectangle& GetBBox() const { return m_oBBox; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006b00
	TGUIRectangle& ModifyBBox( const TGUIComponent* a_pComponent ) { return m_oBBox; }

	void SetBBox( const TGUIRectangle& a_rcBBox, const TGUIComponent* a_pComponent );

	static TINT GetNumCaches() { return ms_iNumCaches; }

protected:
	Data* FindData( const TGUIComponent* a_pComponent ) const;

private:
	// NOTE: the name is unknown
	static TINT ms_iNumCaches;

private:
	TGUIRectangle m_oBBox; // 0x00
	Data          m_oRoot; // 0x10 only pNext and pPrev are used
};

TOSHI_NAMESPACE_END
