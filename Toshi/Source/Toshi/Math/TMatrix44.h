#pragma once
namespace Toshi
{
	class TMatrix44
	{
	public:
		TFloat
			a, b, c, d,
			e, f, g, h,
			i, j, k, l,
			m, n, o, p;

	public:
		TMatrix44(
			TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
			TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24,
			TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34,
			TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44
		)
		{
			a = a_f11;
			b = a_f12;
			c = a_f13;
			d = a_f14;
			e = a_f21;
			f = a_f22;
			g = a_f23;
			h = a_f24;
			i = a_f31;
			j = a_f32;
			k = a_f33;
			l = a_f34;
			m = a_f41;
			n = a_f42;
			o = a_f43;
			p = a_f44;
		}

		TMatrix44(const TMatrix44& a_rMatrix);
		TMatrix44() { }

		void Identity();

		void Set(
			TFloat a_f11, TFloat a_f12, TFloat a_f13, TFloat a_f14,
			TFloat a_f21, TFloat a_f22, TFloat a_f23, TFloat a_f24,
			TFloat a_f31, TFloat a_f32, TFloat a_f33, TFloat a_f34,
			TFloat a_f41, TFloat a_f42, TFloat a_f43, TFloat a_f44
		);

	private:
		constexpr static float s_Identity[16] = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};
	};
}
