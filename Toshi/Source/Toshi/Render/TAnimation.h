#pragma once
#include "Toshi/File/TTRB.h"
#include "Toshi2/T2DList.h"

#ifdef LoadLibrary
#undef LoadLibrary
#endif LoadLibrary

namespace Toshi {
	
	constexpr int TANIMATION_MAXBONES = 128;

	extern TMemoryHeap* s_pAnimMemHeap;

	struct TAnimVector
	{
		float m_fX;
		float m_fY;
		float m_fZ;
	};

	struct TAnimQuaternion
	{
		float m_fX;
		float m_fY;
		float m_fZ;
		float m_fW;
	};

	struct TAnimScale
	{
		float m_fScale;
	};

	struct TAnimationBone
	{
		unsigned short Key;
	};

	class TSkeletonInstance;

	class TAnimation : public TDList<TAnimation>::TNode
	{
	public:
		using Flags = uint8_t;
		enum Flags_ : Flags
		{
			Flags_None = 0,
			Flags_Active = BITFIELD(0),
			Flags_Managed = BITFIELD(1),
			Flags_Unknown2 = BITFIELD(2),
			Flags_UpdateStateOnRemove = BITFIELD(3),
		};

		using Mode = uint32_t;
		enum Mode_ : Mode
		{
			MODE_UNK1,
			MODE_UNK2,
			MODE_UNK3,
		};

	public:
		TAnimation()
		{
			m_eFlags = Flags_None;
		}

		TBOOL UpdateTime(float a_fDeltaTime);
		void RemoveAnimation(float a_fVal);

		float SetDestWeight(float a_fDestWeight, float a_fBlendInSpeed);

		void SetMode(Mode a_eMode) { m_eMode = a_eMode; }
		float SetSpeed(float a_fSpeed) { return std::exchange(m_fSpeed, a_fSpeed); }

		TBOOL IsActive() const { return m_eFlags & Flags_Active; }
		TBOOL IsManaged() const { return m_eFlags & Flags_Managed; }
		TBOOL IsUpdateStateOnRemove() const { return m_eFlags & Flags_UpdateStateOnRemove; }

		TSkeletonInstance* GetSkeletonInstance() const { return m_pSkeletonInstance; }
		unsigned short GetSequence() const { return m_iSeqID; }
		float GetSpeed() const { return m_fSpeed; }
		float GetSeqTime() const { return m_fSeqTime; }
		float GetTotalTime() const { return m_fTotalTime; }
		float GetWeight() const { return m_fWeight; }
		float GetDestWeight() const { return m_fDestWeight; }
		float GetBlendInSpeed() const { return m_fBlendInSpeed; }
		float GetBlendOutSpeed() const { return m_fBlendOutSpeed; }
		Flags GetFlags() const { return m_eFlags; }
		Mode GetMode() const { return m_eMode; }

		TAnimationBone* GetBones() { return TREINTERPRETCAST(TAnimationBone*, this + 1); }
		TAnimationBone* GetBone(int a_iIndex) { return &TREINTERPRETCAST(TAnimationBone*, this + 1)[a_iIndex]; }

	private:
		TSkeletonInstance* m_pSkeletonInstance;
		unsigned short m_iSeqID;
		Flags m_eFlags;
		int m_iUnk3;
		Mode m_eMode;
		float m_fSpeed;
		float m_fWeight;
		float m_fDestWeight;
		float m_fTotalTime;
		float m_fSeqTime;
		float m_fBlendInSpeed;
		float m_fBlendOutSpeed;
	};

	class TKeyframeLibrary : public TDList<TKeyframeLibrary>::TNode
	{
	public:
		friend class TKeyframeLibraryInstance;

		struct TRBHeader
		{
			const char* m_szName;
			TVector3 m_SomeVector;
			int m_iNumTranslations;
			int m_iNumQuaternions;
			int m_iNumScales;
			int m_iTranslationSize;
			int m_iQuaternionSize;
			int m_iScaleSize;
			TAnimVector* m_pTranslations;
			TAnimQuaternion* m_pQuaternions;
			TAnimScale* m_pScales;
		};

	public:
		static TKeyframeLibrary* Create(TRBHeader* a_pTRBHeader);

		void Delete();

		const char* GetName() const
		{
			return m_szName;
		}

	private:
		TKeyframeLibrary();

		void IncreaseRefCount()
		{
			m_iReferenceCount++;
		}

		void DecreaseRefCount()
		{
			m_iReferenceCount--;
		}

	private:
		const char* m_szName;
		TVector3 m_SomeVector;
		int m_iReferenceCount;
		int m_iNumTranslations;
		int m_iNumQuaternions;
		int m_iNumScales;
		int m_iTranslationSize;
		int m_iQuaternionSize;
		int m_iScaleSize;
		TAnimVector* m_pTranslations;
		TAnimQuaternion* m_pQuaternions;
		TAnimScale* m_pScales;
		TTRB* m_pTRB;
	};

	class TKeyframeLibraryInstance
	{
	public:
		TKeyframeLibraryInstance();

		TKeyframeLibrary* Create(TKeyframeLibrary* a_pLibrary);
		TKeyframeLibrary* CreateEx(TKeyframeLibrary* a_pLibrary, int a_iTKeyCount, int a_iQKeyCount, int a_iSKeyCount, int a_iTBaseIndex, int a_iQBaseIndex, int a_iSBaseIndex);

		void Destroy();

		void SetLibrary(TKeyframeLibrary* a_pLibrary) { m_pLibrary = a_pLibrary; }
		void SetTCount(int a_iTCount) { m_iTKeyCount = a_iTCount; }
		void SetQCount(int a_iQCount) { m_iQKeyCount = a_iQCount; }
		void SetSCount(int a_iSCount) { m_iSKeyCount = a_iSCount; }

		TKeyframeLibrary* GetLibrary() { return m_pLibrary; }
		int GetTCount() { return m_iTKeyCount; }
		int GetQCount() { return m_iQKeyCount; }
		int GetSCount() { return m_iSKeyCount; }

	private:
		TKeyframeLibrary* m_pLibrary;
		int m_iTBaseIndex;
		int m_iQBaseIndex;
		int m_iSBaseIndex;
		int m_iTKeyCount;
		int m_iQKeyCount;
		int m_iSKeyCount;
		TAnimVector* m_pTranslations;
		TAnimQuaternion* m_pQuaternions;
		TAnimScale* m_pScales;
	};

	class TKeyframeLibraryManager
	{
	public:
		TKeyframeLibraryManager() = default;
		~TKeyframeLibraryManager() = default;

		TKeyframeLibrary* GetLibrary(const char* a_szName);

		TKeyframeLibrary* LoadLibrary(TKeyframeLibrary::TRBHeader* a_pTRBHeader);
		TKeyframeLibrary* LoadLibraryFromTRB(TTRB* a_pTRB, const char* a_szSymbolName);
		void UnloadLibrary(TKeyframeLibrary* a_pLibrary);

	private:
		TDList<TKeyframeLibrary> m_List;
		int m_iNumLibraries = 0;
	};

}
