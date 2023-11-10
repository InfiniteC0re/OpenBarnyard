#pragma once
#include "T2Allocator.h"

#ifdef GetObject
#undef GetObject
#endif

namespace Toshi {

	class T2GenericObjectPool : public T2Allocator
	{
	public:
		class Object
		{
		public:
			friend T2GenericObjectPool;

		public:
			void SetNext(Object* a_pObject) { m_pNext = a_pObject; }
			Object* GetNext() { return m_pNext; }
			const Object* GetNext() const { return m_pNext; }

		private:
			Object* m_pNext;
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
				pObject->m_pNext = pNext;
				pObject = pNext;
			}

			pObject->m_pNext = TNULL;
		}

		Object* GetObject()
		{
			Object* pNode = m_pHead;
			m_pHead = m_pHead->m_pNext;
			return pNode;
		}

		void ReturnObject(Object* a_pObject)
		{
			a_pObject->m_pNext = m_pHead;
			m_pHead = a_pObject;
		}

		TUINT GetNumUsedObjects() const
		{
			TUINT uiNumber = 0;
			for (auto it = m_pHead; it != TNULL; it = it->m_pNext, uiNumber++);

			return uiNumber;
		}

	protected:
		Object* m_pHead;
	};

	template <class T, TUINT MaxNumber = 2, TUINT ObjectSize = sizeof(T), TUINT Alignment = alignof(T)>
	class T2ObjectPool : protected T2GenericObjectPool
	{
	public:
		T2ObjectPool()
		{
			T2GenericObjectPool::Initialise(GetObjects(), MaxNumber, ObjectSize);
		}

		T* New()
		{
			TASSERT(TTRUE == CanAllocate(ObjectSize));
			return TSTATICCAST(T*, Malloc(ObjectSize));
		}

		void Delete(T* a_pObject)
		{
			TASSERT(TTRUE == IsAddressInPool(a_pObject));
			Free(a_pObject);
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

		virtual size_t GetSize() override
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