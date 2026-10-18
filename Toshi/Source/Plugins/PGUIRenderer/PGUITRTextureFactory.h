#pragma once
#include <TGUI/TGUITextureFactory.h>
#include <Toshi/TArray.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TString8.h>
#include <Render/TTexture.h>
#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUIRenderer.h>

class PGUITRDisplayContext;

//-----------------------------------------------------------------------------
// Purpose: texture store of the TRender GUI renderer. A texture is either a
// named texture of the material library, found when the texture is validated,
// or a texture the factory created itself (unnamed, see GetTextureID with a
// texture and LoadTTexture). The IDs are the indices in the texture array,
// the slots of the destroyed textures are reused.
//
// NOTE: the original drew with the TSpriteMaterial of each texture, here it
// is a T2GUIMaterial of the T2GUI renderer of the display context.
//-----------------------------------------------------------------------------
class PGUITRTextureFactory : public Toshi::TGUITextureFactory
{
public:
	TDECLARE_CLASS( PGUITRTextureFactory, Toshi::TGUITextureFactory );

	class Texture
	{
	public:
		friend PGUITRTextureFactory;

	public:
		Texture( const Toshi::TPString8& a_rcName, PGUITRTextureFactory* a_pFactory, TBOOL a_bValidate );
		Texture( const Toshi::TPString8& a_rcName, PGUITRTextureFactory* a_pFactory, Toshi::T2GUIMaterial* a_pMaterial, TINT a_iWidth, TINT a_iHeight );
		~Texture();

		// Finds the texture of the material library (named textures only)
		void Validate();

		// Destroys the material and the texture the factory created (unnamed textures)
		void Invalidate();

		void SetMaterial( Toshi::T2GUIMaterial* a_pMaterial, TINT a_iWidth, TINT a_iHeight );

		void  SetFlag( TextureFlag a_eFlag, TBOOL a_bValue );
		TBOOL GetFlag( TextureFlag a_eFlag ) const;

		// $JPOG: FUNCTION PGUIRenderer.dll+00007b40
		const Toshi::TPString8& GetName() const { return m_strName; }

		// $JPOG: FUNCTION PGUIRenderer.dll+00005cb0
		Toshi::T2GUIMaterial* GetMaterial() const { return m_pMaterial; }

		// $JPOG: FUNCTION PGUIRenderer.dll+00005cd0
		TINT GetWidth() const { return m_iWidth; }

		// $JPOG: FUNCTION PGUIRenderer.dll+00005cc0
		TINT GetHeight() const { return m_iHeight; }

		// $JPOG: FUNCTION PGUIRenderer.dll+00005ca0
		TBOOL IsValidated() const { return m_pMaterial != TNULL; }

	private:
		Toshi::TPString8      m_strName;   // 0x00 empty for the textures the factory created
		PGUITRTextureFactory* m_pFactory;  // 0x04
		Toshi::T2GUIMaterial* m_pMaterial; // 0x08
		Toshi::TTexture*      m_pTexture;  // 0x0C created by the factory
		TINT                  m_iWidth;    // 0x10
		TINT                  m_iHeight;   // 0x14
		TUINT                 m_uiFlags;   // 0x18
		TBOOL                 m_bUnk;      // 0x1C
	};

	// A list of textures validated and invalidated together
	class TextureSet
	{
	public:
		struct Node : public Toshi::TNodeList<Node>::TNode
		{
			Node( Texture* a_pTexture )
			    : pTexture( a_pTexture )
			{}

			Texture* pTexture; // 0x0C
		};

	public:
		TextureSet( PGUITRTextureFactory* a_pFactory );
		~TextureSet();

		// Reserves the ID of the texture a_szName and adds the texture to the set
		void Add( const TCHAR* a_szName );

		void Validate();
		void Invalidate();

	private:
		Toshi::TNodeList<Node> m_oTextures; // 0x00
		PGUITRTextureFactory*  m_pFactory;  // 0x10
	};

public:
	PGUITRTextureFactory();
	~PGUITRTextureFactory();

	//-----------------------------------------------------------------------------
	// Toshi::TGUITextureFactory
	//-----------------------------------------------------------------------------
	virtual TINT16                  GetTextureID( const Toshi::TPString8& a_rcName ) override;
	virtual TINT16                  FindTextureID( const Toshi::TPString8& a_rcName ) override;
	virtual const Toshi::TPString8& GetTextureName( TINT16 a_iID ) const override;
	virtual TINT16                  GetTextureWidth( TINT16 a_iID ) const override;
	virtual TINT16                  GetTextureHeight( TINT16 a_iID ) const override;
	virtual TBOOL                   GetTextureFlag( TINT16 a_iID, TextureFlag a_eFlag ) const override;
	virtual void                    SetTextureFlag( TINT16 a_iID, TextureFlag a_eFlag, TBOOL a_bValue ) override;
	virtual TINT16                  GetFirstTextureID() const override;
	virtual TINT16                  GetNextTextureID( TINT16 a_iID ) const override;

	//-----------------------------------------------------------------------------
	// PGUITRTextureFactory
	//-----------------------------------------------------------------------------

	// Same as GetTextureID: gives an ID to the name without validating the texture
	virtual TINT16 ReserveTextureID( const Toshi::TPString8& a_rcName );

	virtual TBOOL IsTextureIDValid( TINT16 a_iID ) const;

	// Loads the PPM image a_rcName (P5 grey with alpha, P6 RGB or P7 RGBA) of the texture root folder
	virtual Toshi::TTexture* LoadTTexture( const Toshi::TPString8& a_rcName, TINT& a_rWidth, TINT& a_rHeight );

	void Create( PGUITRDisplayContext* a_pDisplayContext );

	// Validates all the textures (finds the named ones in the material library)
	void ResolveMaterials();

	// Gives an ID to a texture of the game (the material is destroyed with the texture, not the texture itself)
	TINT16 GetTextureID( Toshi::TTexture* a_pTexture );

	// Changes the texture of the texture a_iID (that the factory created), the previous texture is destroyed
	void ReplaceTexture( TINT16 a_iID, Toshi::TTexture* a_pTexture );

	void DestroyTexture( TINT16 a_iID );

	Texture* GetTexture( const Toshi::TPString8& a_rcName );

	Toshi::TTexture* ReadPPM( TUINT8* a_pData, TINT a_iSize, TINT& a_rWidth, TINT& a_rHeight );

	Toshi::T2GUIRenderer* GetShader() const;

	// $JPOG: FUNCTION PGUIRenderer.dll+00007b80
	Texture* GetTexture( TINT16 a_iID ) const { return m_aTextures[ a_iID ]; }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005d00
	TINT GetTextureCount() const { return m_aTextures.Size(); }

	// $JPOG: FUNCTION PGUIRenderer.dll+00005ce0
	void SetTextureRoot( const Toshi::TString8& a_rcTextureRoot ) { m_strTextureRoot = a_rcTextureRoot.GetString(); }

protected:
	TINT16 AllocateTextureID();

	void QuantiseRGBA16( TUINT8* a_pSrc, TINT a_iWidth, TINT a_iHeight, TINT a_iSrcPitch, TINT a_iSrcBytesPerPixel, TUINT8* a_pDst, TINT a_iDstWidth, TINT a_iDstHeight, TINT a_iDstPitch, TINT a_iDstBytesPerPixel, TBOOL a_bGrey, TBOOL a_bAlpha );
	TUINT8 QuantisePalette256( TUINT* a_pPalette, TUINT8* a_pSrc, TINT a_iWidth, TINT a_iHeight, TINT a_iSrcPitch, TUINT8* a_pDst, TINT a_iDstWidth, TINT a_iDstHeight, TINT a_iDstPitch, TINT a_iDstBytesPerPixel, TBOOL a_bGrey, TBOOL a_bAlpha );
	TUINT8 InsertColour( TUINT* a_pPalette, TUINT8& a_rNumColours, TUINT a_uiColour );
	TBOOL  TestRGBA16( TUINT8* a_pSrc, TINT a_iWidth, TINT a_iHeight, TINT a_iSrcPitch, TINT a_iSrcBytesPerPixel );

private:
	Toshi::TArray<Texture*> m_aTextures;       // 0x04
	Toshi::TString8         m_strTextureRoot;  // 0x14
	PGUITRDisplayContext*   m_pDisplayContext; // 0x1C
};
