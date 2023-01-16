#pragma once

namespace Toshi
{
	class TMaterial :
		public TGenericClassDerived<TMaterial, TObject, "TMaterial", TMAKEVERSION(1, 0), false>
	{
	public:
		static constexpr size_t NAMESIZELIMIT = 31;

		typedef uint32_t State;
		enum State_ : State
		{
			State_NULL = 0,
			State_Unk1 = BITFIELD(0),
			State_Unk2 = BITFIELD(1),
			State_Unk3 = BITFIELD(2),
			State_Unk4 = BITFIELD(3),
			State_Created = BITFIELD(4),
		};

	public:
		TMaterial()
		{
			m_State = State_NULL;
			m_Unk = TNULL;
		}

		virtual void OnDestroy()
		{
			TASSERT(TTRUE == IsCreated());
			m_State &= ~State_Created;
			m_Unk = TNULL;
		}

		virtual bool Create()
		{
			TASSERT(TFALSE == IsCreated());
			m_State |= State_Created;
			return true;
		}

		virtual void PreRender()
		{

		}

		virtual void PostRender()
		{

		}

		void SetName(const char* name)
		{
			if (name == TNULL)
			{
				m_Name[0] = '\0';
			}
			else
			{
				T2String8::Copy(m_Name, name, NAMESIZELIMIT);
			}
		}

		void SetFlag(State flag, bool set = false)
		{
			if (set)
			{
				if (flag & (State_Unk1 | State_Unk2 | State_Unk3))
				{
					m_State &= ~(State_Unk1 | State_Unk2 | State_Unk3);
				}

				m_State |= flag;
			}
			else
			{
				m_State &= ~flag;
			}
		}

		bool IsCreated() const
		{
			return m_State & State_Created;
		}

	private:
		State m_State;                  // 0x04
		char m_Name[NAMESIZELIMIT + 1]; // 0x2C
		void* m_Unk;                    // 0x4C
	};
}