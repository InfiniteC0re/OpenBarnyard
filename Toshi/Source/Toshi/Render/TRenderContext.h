#pragma once

#include "Toshi/Math/TMatrix44.h"

namespace Toshi
{
	class TRenderContext
	{

		enum FLAG : uint8_t
		{
			FLAG_DIRTY = 1,
			FLAG_FOG
		};

		struct PROJECTIONPARAMS
		{
			float m_fCentreX;	// 0x0
			float m_fCentreY;	// 0x0
			float m_fProjX;		// 0x8
			float m_fProjY;		// 0xC
		};

		uint8_t m_eFlags;						// 0x8
		static PROJECTIONPARAMS m_sProjParams;	// 0x30
		TMatrix44 m_mModelViewMatrix;			// 0x40
		TMatrix44 m_mWorldViewMatrix;			// 0x80

	protected:
		inline void SetDirty(bool enable) { enable ? m_eFlags |= FLAG_DIRTY : m_eFlags &= ~FLAG_DIRTY; }
		inline void SetFlag(FLAG flag, bool enable) { enable ? m_eFlags |= flag : m_eFlags &= ~flag; }

		inline void EnableFog(bool enable) { enable ? m_eFlags |= FLAG_FOG : m_eFlags &= ~FLAG_FOG; }

		inline bool IsFogEnabled() const { return (m_eFlags & FLAG_FOG) != 0; }
		inline bool IsDirty() const { return (m_eFlags & FLAG_DIRTY) != 0; }
	public:

		TRenderContext() {}
		TRenderContext(TRenderInterface&) {}

		virtual void SetModelViewMatrix(const TMatrix44& a_rMatrix);
		virtual void SetWorldViewMatrix(const TMatrix44& a_rMatrix);
	};
	
	class TRenderContextRev : public TRenderContext
	{
	public:
		TRenderContextRev() : TRenderContext() {}
		TRenderContextRev(TRenderInterface& renderer) : TRenderContext(renderer) {}

	};
}