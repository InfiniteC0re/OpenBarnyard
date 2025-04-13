#pragma once

class AHashedObject
{
public:
	AHashedObject();
	virtual ~AHashedObject();

	virtual void  Unknown1() = 0;
	virtual void  Unknown2() = 0;
	virtual TBOOL Unknown3();
	virtual void  SetUnk2( void* a_pUnk );
	virtual void* GetUnk2();

private:
	void* m_Unk1;
	void* m_Unk2;
};
