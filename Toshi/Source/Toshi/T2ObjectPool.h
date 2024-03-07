#pragma once
#include "T2Allocator.h"

#ifdef GetObject
#undef GetObject
#endif

namespace Toshi {

	class T2GenericObjectPool : protected T2Allocator
	{
	protected:
		struct Object
		{
			Object* pNextObject;
		};

	public:
		virtual ~T2GenericObjectPool() = default;

	protected:
		virtual void* Malloc(size_t a_uiSize, size_t a_uiAlignment) override
		{
			return TNULL;
		}

		virtual void* Malloc(size_t a_uiSize) override
		{
			return GetObject();
		}

		virtual void Free(void* a_pPtr) override
		{
			ReturnObject(TSTATICCAST(Object, a_pPtr));
		}

		virtual TBOOL CanAllocate(size_t size) override
		{
			return m_pHead != TNULL;
		}

		virtual void* TryMalloc(size_t size, size_t alignment) override
		{
			return TNULL;
		}

		virtual void* TryMalloc(size_t size) override
		{
			if (m_pHead != TNULL)
			{
				return GetObject();
			}

			return TNULL;
		}

	protected:
		void Initialise(Object* a_pObjects, TUINT a_uiMaxNumber, TUINT a_uiObjectSize)
		{
			TASSERT(a_uiMaxNumber > 1);
			m_pHead = a_pObjects;
			Object* pObject = a_pObjects;

			for (TUINT i = a_uiMaxNumber - 1; i != 0; i--)
			{
				Object* pNext = TREINTERPRETCAST(Object*, TREINTERPRETCAST(uintptr_t, pObject) + a_uiObjectSize);
				pObject->pNextObject = pNext;
				pObject = pNext;
			}

			pObject->pNextObject = TNULL;
		}

		Object* GetObject()
		{
			Object* pNode = m_pHead;
			m_pHead = m_pHead->pNextObject;
			return pNode;
		}

		void ReturnObject(Object* a_pObject)
		{
			a_pObject->pNextObject = m_pHead;
			m_pHead = a_pObject;
		}

		TUINT GetNumFreeObjects() const
		{
			TUINT uiNumber = 0;
			for (auto it = m_pHead; it != TNULL; it = it->pNextObject, uiNumber++);

			return uiNumber;
		}

	protected:
		Object* m_pHead;
	};

	template <class T, TUINT MaxNumber, TUINT ObjectSize = sizeof(T), TUINT Alignment = alignof(T)>
	class T2ObjectPool :
		protected T2GenericObjectPool
	{
	public:
		TSTATICASSERT(MaxNumber >= 2);
		TSTATICASSERT(sizeof(T) >= sizeof(T2GenericObjectPool::Object));

	public:
		T2ObjectPool()
		{
			T2GenericObjectPool::Initialise(
				TREINTERPRETCAST(T2GenericObjectPool::Object*, GetObjects()),
				MaxNumber,
				ObjectSize
			);
		}

		template<class... Args>
		T* NewObject(Args&& ...args)
		{
			TASSERT(TTRUE == CanAllocate(ObjectSize));
			T* pValue = new (Malloc(ObjectSize)) T(std::forward<Args>(args)...);
			return pValue;
		}

		T* AllocateObject()
		{
			TASSERT(TTRUE == CanAllocate(ObjectSize));
			return TSTATICCAST(T, Malloc(ObjectSize));
		}

		void FreeObject(T* a_pObject)
		{
			TASSERT(TTRUE == IsAddressInPool(a_pObject));
			Free(a_pObject);
		}

		void DeleteObject(T* a_pObject)
		{
			TASSERT(TTRUE == IsAddressInPool(a_pObject));
			a_pObject->~T();
			Free(a_pObject);
		}

		virtual size_t GetUsedSize() override
		{
			return (MaxNumber - GetNumFreeObjects()) * ObjectSize;
		}

		virtual size_t GetCapacity() override
		{
			return MaxNumber * ObjectSize;
		}

		virtual TBOOL IsAddressInPool(const void* a_pAddress)
		{
			return
				(TREINTERPRETCAST(uintptr_t, this) + sizeof(T2GenericObjectPool)) <= TREINTERPRETCAST(uintptr_t, a_pAddress) &&
				TREINTERPRETCAST(uintptr_t, a_pAddress) < (TREINTERPRETCAST(uintptr_t, this) + sizeof(T2GenericObjectPool) + (MaxNumber * ObjectSize));
		}

	private:
		T* GetObjects()
		{
			return TREINTERPRETCAST(T*, m_aObjects);
		}

		const T* GetObjects() const
		{
			return TREINTERPRETCAST(const T*, m_aObjects);
		}

	private:
		TUINT8 m_aObjects[MaxNumber * ObjectSize];
	};

	template <class TClassType>
	class T2DerivedDynamicObjectPool :
		protected T2GenericObjectPool
	{
	public:
		T2DerivedDynamicObjectPool(T2Allocator* a_pAllocator, TINT a_iMaxNumber)
		{
			TVALIDPTR(a_pAllocator);
			m_iMaxNumber = a_iMaxNumber;
			m_pAllocator = a_pAllocator;
			
			TUINT32 uiClassSize = TClassType::GetClassStatic()->GetMaxSizeOfDerivedClasses();
			TUINT32 uiClassAlignment = TClassType::GetClassStatic()->GetMaxAlignmentOfDerivedClasses();
			m_uiObjectSize = (uiClassSize - 1) + uiClassAlignment & ~(uiClassAlignment - 1U);

			TASSERT(m_uiObjectSize >= sizeof(TClassType) && uiClassAlignment > 0);

			if (m_iMaxNumber < 1)
			{
				m_pData = TNULL;
			}
			else
			{
				m_pData = m_pAllocator->Malloc(m_iMaxNumber * m_uiObjectSize, uiClassAlignment);
			}

			TVALIDPTR(m_pData);
			T2GenericObjectPool::Initialise(
				TREINTERPRETCAST(T2GenericObjectPool::Object*, m_pData),
				m_iMaxNumber,
				m_uiObjectSize
			);
		}

		virtual ~T2DerivedDynamicObjectPool()
		{
			m_pAllocator->Free(m_pData);
		}

		template<class... Args>
		TClassType* NewObject(Args&& ...args)
		{
			TASSERT(TTRUE == CanAllocate(m_uiObjectSize));
			TClassType* pValue = new (Malloc(m_uiObjectSize)) TClassType(std::forward<Args>(args)...);
			return pValue;
		}

		template<class T, class... Args>
		T* NewDerivedObject(Args&& ...args)
		{
			TASSERT(TTRUE == T::GetClassStatic()->IsA(TClassType::GetClassStatic()));
			TASSERT(TTRUE == CanAllocate(m_uiObjectSize));
			T* pValue = new (Malloc(m_uiObjectSize)) T(std::forward<Args>(args)...);
			return pValue;
		}

		TClassType* AllocateObject()
		{
			TASSERT(TTRUE == CanAllocate(m_uiObjectSize));
			return TSTATICCAST(TClassType, Malloc(m_uiObjectSize));
		}

		void FreeObject(TClassType* a_pObject)
		{
			TASSERT(TTRUE == IsAddressInPool(a_pObject));
			Free(a_pObject);
		}

		void DeleteObject(TClassType* a_pObject)
		{
			TASSERT(TTRUE == IsAddressInPool(a_pObject));
			a_pObject->~TClassType();
			Free(a_pObject);
		}

		virtual size_t GetUsedSize() override
		{
			return (m_iMaxNumber - GetNumFreeObjects()) * m_uiObjectSize;
		}

		virtual size_t GetCapacity() override
		{
			return m_iMaxNumber * m_uiObjectSize;
		}

		virtual TBOOL IsAddressInPool(const void* a_pAddress)
		{
			return
				TREINTERPRETCAST(uintptr_t, m_pData) <= TREINTERPRETCAST(uintptr_t, a_pAddress) &&
				TREINTERPRETCAST(uintptr_t, a_pAddress) < TREINTERPRETCAST(uintptr_t, m_pData) + (m_iMaxNumber * m_uiObjectSize);
		}

	private:
		T2Allocator* m_pAllocator;
		TINT m_iMaxNumber;
		TUINT m_uiObjectSize;
		void* m_pData;
	};

}