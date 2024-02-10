#pragma once

namespace Toshi
{
    class BTECCompressor
    {
    public:
        class Node
        {
        public:
            friend BTECCompressor;

        public:
            Node() = default;
            /*{
                m_Ptr1 = this;
                m_Ptr2 = this;
            }*/

            void Reset()
            {
                m_Ptr1 = this;
                m_Ptr2 = this;
            }

        private:
            Node* m_Ptr1;
            Node* m_Ptr2;
            Node* m_Unk1;
            Node* m_Unk2;
            TCHAR* m_Unk3;
            void* m_Unk4;
        };

        using Bucket = Node**;

    public:
        BTECCompressor();
        ~BTECCompressor();

        void Initialize(TCHAR* buffer, size_t bufferSize, TINT maxoffset, TINT unk);
        void FUN_0068ae40(size_t dataSize);
        TBOOL FUN_0068af10(TCHAR* buffer, size_t bufferSize, TCHAR*& offset, size_t& dataSize);
        void AllocSubstring(TCHAR* buffer);

        static TBOOL FUN_0068b300(TCHAR* buffer, Bucket nodeBucket, size_t bufferSize, Node*& out1, Node*& out2);

        static TUINT32 HashData(TCHAR* data, size_t size)
        {
            TUINT32 hash = 0;
            while (size != 0)
            {
                hash = (TUINT32)*data ^ hash >> 0x1B ^ hash << 5;
                size -= 1;
                data += 1;
            }

            return hash;
        }

    private:
        TCHAR* m_Buffer1;
        TUINT32 m_MaxOffset;
        TCHAR* m_DataEnd;
        TCHAR* m_Buffer2;
        TCHAR* m_Buffer3;
        Node* m_SomeArray1;
        Node m_Root1;
        Node m_Root2;
        TCHAR* m_Offsets[256];
        Bucket* m_Buckets;
        TUINT32 m_BucketCount;
    };
}