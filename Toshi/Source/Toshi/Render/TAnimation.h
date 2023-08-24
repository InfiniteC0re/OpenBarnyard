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

	class TSkeletonInstance;

#pragma pack(push, 1)
	class TAnimation : public TDList<TAnimation>::TNode
	{
	public:
		TAnimation()
		{
			m_iUnk2 = 0;
		}

		TBOOL UpdateTime(float a_fDeltaTime);

		TSkeletonInstance* GetSkeletonInstance() const
		{
			return m_pSkeletonInstance;
		}

		unsigned short GetSequenceID() const
		{
			return m_iSeqID;
		}

	public: // FIXME: Make members private when most are figured out
		TSkeletonInstance* m_pSkeletonInstance;
		unsigned short m_iSeqID;
		short m_iUnk2;
		int m_iUnk3;
		int m_eFlags;
		float m_fAnimSpeedMultiplier;
		float m_fUnk4;
		float m_fTotalPlayingTime;
		float m_fPlaybackTime;
		float m_fUnk6;
		float m_fUnk7;
	};
#pragma pack(pop)

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
