#pragma once
#include "T2Allocator.h"

#ifdef GetObject
#undef GetObject
#endif

namespace Toshi {

	class T2GenericObjectPool
	{
	protected:
		struct Object
		{
			Object* pNextObject;
		};

	public:
		virtual ~T2GenericObjectPool() = default;

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

		TUINT GetNumUsedObjects() const
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
		protected T2GenericObjectPool,
		protected T2Allocator
	{
	public:
		static_assert(MaxNumber >= 2);
		static_assert(sizeof(T) >= sizeof(T2GenericObjectPool::Object));

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
			return TSTATICCAST(T*, Malloc(ObjectSize));
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
			return (MaxNumber - GetNumUsedObjects()) * ObjectSize;
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
			ReturnObject(TSTATICCAST(Object*, a_pPtr));
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

}