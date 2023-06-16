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
            char* m_Unk3;
            void* m_Unk4;
        };

        using Bucket = Node**;

    public:
        BTECCompressor();
        ~BTECCompressor();

        void Initialize(char* buffer, size_t bufferSize, int maxoffset, int unk);
        void FUN_0068ae40(size_t dataSize);
        TBOOL FUN_0068af10(char* buffer, size_t bufferSize, char*& offset, size_t& dataSize);
        void AllocSubstring(char* buffer);

        static TBOOL FUN_0068b300(char* buffer, Bucket nodeBucket, size_t bufferSize, Node*& out1, Node*& out2);

        static uint32_t HashData(char* data, size_t size)
        {
            uint32_t hash = 0;
            while (size != 0)
            {
                hash = (uint32_t)*data ^ hash >> 0x1B ^ hash << 5;
                size -= 1;
                data += 1;
            }

            return hash;
        }

    private:
        char* m_Buffer1;
        uint32_t m_MaxOffset;
        char* m_DataEnd;
        char* m_Buffer2;
        char* m_Buffer3;
        Node* m_SomeArray1;
        Node m_Root1;
        Node m_Root2;
        char* m_Offsets[256];
        Bucket* m_Buckets;
        uint32_t m_BucketCount;
    };
}