#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/TArray.h"

namespace Toshi {
	
	//-----------------------------------------------------------------------------
	// Use this to create shared buffer of type T.
	// Call TSharedBuffer<>::Create() specifying number of buffers to create.
	// Note: Max possible number of elements is BUFSIZE * m_iNumBuffers.
	//-----------------------------------------------------------------------------
	template <typename T, TINT BUFSIZE, TINT IDENTIFIER>
	class TSharedBuffer
	{
	public:
		class TNode
		{
		public:
			T& GetElement(TINT a_iIndex)
			{
				TASSERT(a_iIndex < BUFSIZE);
				return m_aElements[a_iIndex];
			}

			const T& GetElement(TINT a_iIndex) const
			{
				return GetElement(a_iIndex);
			}

			T& operator[](TINT a_iIndex)
			{
				return GetElement(a_iIndex);
			}

			const T& operator[](TINT a_iIndex) const
			{
				return GetElement(a_iIndex);
			}

		private:
			union
			{
				TUINT8 m_aMemoryBuffer[sizeof(T) * BUFSIZE];
				T m_aElements[BUFSIZE];
			};
		};

	public:
		//-----------------------------------------------------------------------------
		// Static definitions
		//-----------------------------------------------------------------------------

		static void CreateSharedBuffers(TINT a_iNumBuffers)
		{
			TASSERT(m_iNumBuffers == 0);
			TASSERT(m_pNodeArray == TNULL);

			m_iNumBuffers = a_iNumBuffers;
			
			if (a_iNumBuffers > m_oFreeList.SizeAllocated())
			{
				m_oFreeList.Resize(a_iNumBuffers);
			}

			m_oFreeList.SetSize(a_iNumBuffers);
			m_pNodeArray = TSTATICCAST(TNode*, TMalloc(sizeof(TNode) * m_iNumBuffers));

			for (TINT i = 0; i < m_iNumBuffers; i++)
			{
				m_oFreeList[i] = &m_pNodeArray[i];
			}

			TASSERT(m_oFreeList.Size() <= m_iNumBuffers);
			TASSERT((1 << TMath::IntLog2(BUFSIZE)) == BUFSIZE);

			m_iMainIndexShift = TMath::IntLog2(BUFSIZE);
			m_iSubIndexMask = (1 << TMath::IntLog2(BUFSIZE)) - 1;
		}

		static void DestroySharedBuffers()
		{
			TASSERT(m_oFreeList.Size() <= m_iNumBuffers);
			TASSERT(TFALSE == IsAnyBuffersUsed());

			m_oFreeList.Clear();

			TVALIDPTR(m_pNodeArray);
			TFree(m_pNodeArray);

			m_iNumBuffers = 0;
			m_pNodeArray = TNULL;
		}

		static TBOOL IsAnyBuffersUsed()
		{
			return m_iNumBuffers != m_oFreeList.Size();
		}

	private:
		inline static TNode* m_pNodeArray;
		inline static TINT m_iNumBuffers;
		inline static TINT m_iMainIndexShift;
		inline static TINT m_iSubIndexMask;
		inline static TArray<TNode*> m_oFreeList;

	public:
		//-----------------------------------------------------------------------------
		// Member definitions
		//-----------------------------------------------------------------------------

		TSharedBuffer() :
			m_oArray(4, 10)
		{
			TASSERT((1 << TMath::IntLog2(BUFSIZE)) == BUFSIZE);
		}

		~TSharedBuffer()
		{
			SetSize(0);
		}

		TUINT GetSize() const
		{
			return m_oArray.Size() << m_iMainIndexShift;
		}

		TBOOL SetSize(TINT a_iSize)
		{
			TASSERT(a_iSize >= 0);
			TASSERT(m_oFreeList.Size() <= m_iNumBuffers);
			TASSERT(TMath::IntLog2(BUFSIZE) == m_iMainIndexShift);

			TINT iDesiredNumNodes = a_iSize >> (m_iMainIndexShift % BUFSIZE);

			if (0 < (m_iSubIndexMask & a_iSize))
			{
				iDesiredNumNodes += 1;
			}

			TASSERT(iDesiredNumNodes >= 0);
			
			if (m_oArray.Size() != iDesiredNumNodes)
			{
				if (10 < iDesiredNumNodes) return TFALSE;

				if (iDesiredNumNodes < m_oArray.Size())
				{
					// Removing elements from m_oArray

					while (iDesiredNumNodes < m_oArray.Size())
					{
						// Remove the last element from array and store in the freelist
						m_oFreeList.Push(m_oArray.Pop());
					}

					TASSERT(m_oArray.Size() == iDesiredNumNodes);
					TASSERT(m_oFreeList.Size() <= m_iNumBuffers);
				}
				else
				{
					// Adding new elements to m_oArray

					if (m_oFreeList.Size() < 1) return TFALSE;

					while (m_oArray.Size() < iDesiredNumNodes)
					{
						if (m_oFreeList.Size() < 1) return TFALSE;

						TASSERT(m_oArray.Size() < m_oArray.SizeAllocated());

						m_oArray.Push(m_oFreeList.Pop());
					}

					TASSERT(m_oArray.Size() == iDesiredNumNodes);
					TASSERT(m_oFreeList.Size() <= m_iNumBuffers);
				}
			}

			return TTRUE;
		}

		T& At(TINT a_iIndex)
		{
			TASSERT(TMath::IntLog2(BUFSIZE) == m_iMainIndexShift);
			TASSERT(m_oFreeList.Size() <= m_iNumBuffers);
			TASSERT(a_iIndex < m_oArray.Size() * BUFSIZE);

			TINT iSubIndex = m_iSubIndexMask & a_iIndex;
			TINT iMainIndex = a_iIndex >> (m_iMainIndexShift % BUFSIZE);

			TASSERT(iMainIndex < m_oArray.Size());
			TASSERT(iSubIndex < BUFSIZE);

			return m_oArray[iMainIndex]->GetElement(iSubIndex);
		}

		const T& At(TINT a_iIndex) const
		{
			TASSERT(TMath::IntLog2(BUFSIZE) == m_iMainIndexShift);
			TASSERT(m_oFreeList.Size() <= m_iNumBuffers);
			TASSERT(a_iIndex < m_oArray.Size() * BUFSIZE);

			TINT iSubIndex = m_iSubIndexMask & a_iIndex;
			TINT iMainIndex = a_iIndex >> (m_iMainIndexShift % BUFSIZE);

			TASSERT(iMainIndex < m_oArray.Size());
			TASSERT(iSubIndex < BUFSIZE);

			return m_oArray[iMainIndex]->GetElement(iSubIndex);
		}

		T& operator[](TINT a_iIndex)
		{
			return At(a_iIndex);
		}

		const T& operator[](TINT a_iIndex) const
		{
			return At(a_iIndex);
		}

	private:
		TArray<TNode*> m_oArray;
	};

}