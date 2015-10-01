#include "stdafx.h"
#include "Light.h"

#include  <iomanip>

#define DAMPING 0.75f
FLOAT COLORMAPPING[] =
{
	0.0004f,
	pow(DAMPING, 14.0f),
	pow(DAMPING, 13.0f),
	pow(DAMPING, 12.0f),
	pow(DAMPING, 11.0f),
	pow(DAMPING, 10.0f),
	pow(DAMPING, 9.0f),
	pow(DAMPING, 8.0f),
	pow(DAMPING, 7.0f),
	pow(DAMPING, 6.0f),
	pow(DAMPING, 5.0f),
	pow(DAMPING, 4.0f),
	pow(DAMPING, 3.0f),
	pow(DAMPING, 2.0f),
	pow(DAMPING, 1.0f),
	pow(DAMPING, 0.0f),
};

UINT COLORMAPPINGI[] =
{
	1,
	UINT(pow(DAMPING, 14.0f) * 255),
	UINT(pow(DAMPING, 13.0f) * 255),
	UINT(pow(DAMPING, 12.0f) * 255),
	UINT(pow(DAMPING, 11.0f) * 255),
	UINT(pow(DAMPING, 10.0f) * 255),
	UINT(pow(DAMPING, 9.0f) * 255),
	UINT(pow(DAMPING, 8.0f) * 255),
	UINT(pow(DAMPING, 7.0f) * 255),
	UINT(pow(DAMPING, 6.0f) * 255),
	UINT(pow(DAMPING, 5.0f) * 255),
	UINT(pow(DAMPING, 4.0f) * 255),
	UINT(pow(DAMPING, 3.0f) * 255),
	UINT(pow(DAMPING, 2.0f) * 255),
	UINT(pow(DAMPING, 1.0f) * 255),
	UINT(pow(DAMPING, 0.0f) * 255),
};

Light Light::operator+=(const Light rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((rhs.rgbs & 0xF0F0) << 12) + (rhs.rgbs & 0x0F0F);

	v1 += v2;

	v1 = (v1 & 0x0F0F0F0F) | (((v1 & 0xF0F0F0F0) * 0xF) >> 4);

	rgbs = WORD(v1 + (v1 >> 12));
	return *this;
}

Light Light::operator-=(const Light rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((rhs.rgbs & 0xF0F0) << 12) + (rhs.rgbs & 0x0F0F);

	v1 -= ((((v1 - v2) >> 4) & (v1 ^ v2)) ^ v2);

	//v1 = (v1 & 0x0F0F0F0F) & (~((v1 & 0xF0F0F0F0) >> 4));

	rgbs = WORD(v1 + (v1 >> 12));
	return *this;
}

Light Light::operator|=(const Light rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((rhs.rgbs & 0xF0F0) << 12) + (rhs.rgbs & 0x0F0F);

	v1 = ((((v1 - v2) >> 4) & (v2 ^ v1)) ^ v1);

	rgbs = WORD(v1 + (v1 >> 12));
	return *this;
}

Light Light::operator&=(const Light rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((rhs.rgbs & 0xF0F0) << 12) + (rhs.rgbs & 0x0F0F);

	v1 = ((((v1 - v2) >> 4) & (v1 ^ v2)) ^ v2);

	rgbs = WORD(v1 + (v1 >> 12));
	return *this;
}

Light Light::operator+(const BYTE rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = (rhs * 0x1010101) & 0x0F0F0F0F;

	v1 += v2;

	v1 = (v1 & 0x0F0F0F0F) | (((v1 & 0xF0F0F0F0) * 0xF) >> 4);

	return WORD(v1 + (v1 >> 12));
}

Light Light::operator-(const BYTE rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = (rhs * 0x1010101) & 0x0F0F0F0F;

	v1 -= ((((v1 - v2) >> 4) & (v1 ^ v2)) ^ v2);

	return WORD(v1 + (v1 >> 12));
}

Light Light::operator+=(const BYTE rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = (rhs * 0x1010101) & 0x0F0F0F0F;

	v1 += v2;

	v1 = (v1 & 0x0F0F0F0F) | (((v1 & 0xF0F0F0F0) * 0xF) >> 4);

	rgbs = WORD(v1 + (v1 >> 12));
	return *this;
}

Light Light::operator-=(const BYTE rhs)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = (rhs * 0x1010101) & 0x0F0F0F0F;

	v1 -= ((((v1 - v2) >> 4) & (v1 ^ v2)) ^ v2);

	//v1 = (v1 & 0x0F0F0F0F) & (~((v1 & 0xF0F0F0F0) >> 4));

	rgbs = WORD(v1 + (v1 >> 12));
	return *this;
}

bool Light::operator==(const Light rhs) const
{
	return rgbs == rhs.rgbs;
}

bool Light::operator>=(const Light rhs) const
{
	return rgbs >= rhs.rgbs;
}

bool Light::operator>(const Light rhs) const
{
	return rgbs > rhs.rgbs;
}

bool Light::operator<(const Light rhs) const
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((rhs.rgbs & 0xF0F0) << 12) + (rhs.rgbs & 0x0F0F);

	return ((((v1 - v2) & 0xF0F0F0F0) * 0xF) == 0xF0F0F0F0);
}

DWORD Light::Color() const
{
	union
	{
		BYTE color[4];
		UINT icolor;
	};
	color[0] = COLORMAPPINGI[r];
	color[1] = COLORMAPPINGI[g];
	color[2] = COLORMAPPINGI[b];
	color[3] = COLORMAPPINGI[s];

	return icolor;
}

Vector4 Light::FColor() const
{
	return Vector4(COLORMAPPING[r], COLORMAPPING[g], COLORMAPPING[b], COLORMAPPING[s]);
}

DWORD Light::MergedColor(const Light l1, const Light l2, const Light l3) const
{
	DWORD v0 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v1 = ((l1.rgbs & 0xF0F0) << 12) + (l1.rgbs & 0x0F0F);
	DWORD v2 = ((l2.rgbs & 0xF0F0) << 12) + (l2.rgbs & 0x0F0F);
	DWORD v3 = ((l3.rgbs & 0xF0F0) << 12) + (l3.rgbs & 0x0F0F);

	DWORD sum = v0 + v1 + v2 + v3; // 0 - 60
	return sum * 4 + sum / 4; //0 - 255
}

void Light::MaxLight(const Light l1, const Light ldv)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((l1.rgbs & 0xF0F0) << 12) + (l1.rgbs & 0x0F0F);
	DWORD vd = ((ldv.rgbs & 0xF0F0) << 12) + (ldv.rgbs & 0x0F0F);

	v2 -= ((((v2 - vd) >> 4) & (v2 ^ vd)) ^ vd);

	v1 = ((((v1 - v2) >> 4) & (v2 ^ v1)) ^ v1);

	//v1 = (v1 & 0x0F0F0F0F) & (~((v1 & 0xF0F0F0F0) >> 4));

	rgbs = WORD(v1 + (v1 >> 12));
}

void Light::MaxLight(const Light l1, const Light l2, const Light ldv)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((l1.rgbs & 0xF0F0) << 12) + (l1.rgbs & 0x0F0F);
	DWORD v3 = ((l2.rgbs & 0xF0F0) << 12) + (l2.rgbs & 0x0F0F);
	DWORD vd = ((ldv.rgbs & 0xF0F0) << 12) + (ldv.rgbs & 0x0F0F);

	v2 -= ((((v2 - vd) >> 4) & (v2 ^ vd)) ^ vd);
	v3 -= ((((v3 - vd) >> 4) & (v3 ^ vd)) ^ vd);

	v1 = ((((v1 - v2) >> 4) & (v2 ^ v1)) ^ v1);
	v1 = ((((v1 - v3) >> 4) & (v3 ^ v1)) ^ v1);

	//v1 = (v1 & 0x0F0F0F0F) & (~((v1 & 0xF0F0F0F0) >> 4));

	rgbs = WORD(v1 + (v1 >> 12));
}

void Light::MaxLight(const Light l1, const Light l2, const Light l3, const Light ldv)
{
	DWORD v1 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD v2 = ((l1.rgbs & 0xF0F0) << 12) + (l1.rgbs & 0x0F0F);
	DWORD v3 = ((l2.rgbs & 0xF0F0) << 12) + (l2.rgbs & 0x0F0F);
	DWORD v4 = ((l3.rgbs & 0xF0F0) << 12) + (l3.rgbs & 0x0F0F);
	DWORD vd = ((ldv.rgbs & 0xF0F0) << 12) + (ldv.rgbs & 0x0F0F);

	v2 -= ((((v2 - vd) >> 4) & (v2 ^ vd)) ^ vd);
	v3 -= ((((v3 - vd) >> 4) & (v3 ^ vd)) ^ vd);
	v4 -= ((((v4 - vd) >> 4) & (v4 ^ vd)) ^ vd);

	v1 = ((((v1 - v2) >> 4) & (v2 ^ v1)) ^ v1);
	v3 = ((((v3 - v4) >> 4) & (v4 ^ v3)) ^ v3);
	v1 = ((((v1 - v3) >> 4) & (v3 ^ v1)) ^ v1);

	//v1 = (v1 & 0x0F0F0F0F) & (~((v1 & 0xF0F0F0F0) >> 4));

	rgbs = WORD(v1 + (v1 >> 12));
}

void Light::MaxLight(const Light l[6], const Light ldv)
{
	DWORD v0 = ((rgbs & 0xF0F0) << 12) + (rgbs & 0x0F0F);
	DWORD vd = ((ldv.rgbs & 0xF0F0) << 12) + (ldv.rgbs & 0x0F0F);
	DWORD v[6];
	for (int i = 0; i < 6; i++)
	{
		v[i] = ((l[i].rgbs & 0xF0F0) << 12) + (l[i].rgbs & 0x0F0F);
		v[i] -= ((((v[i] - vd) >> 4) & (v[i] ^ vd)) ^ vd);
	}

	v[0] = ((((v[0] - v[1]) >> 4) & (v[1] ^ v[0])) ^ v[0]);
	v[2] = ((((v[2] - v[3]) >> 4) & (v[3] ^ v[2])) ^ v[2]);
	v[4] = ((((v[4] - v[5]) >> 4) & (v[5] ^ v[4])) ^ v[4]);

	v[0] = ((((v[0] - v[2]) >> 4) & (v[2] ^ v[0])) ^ v[0]);
	v[4] = ((((v[4] -   v0) >> 4) & (v0   ^ v[4])) ^ v[4]);

	v0   = ((((v[0] - v[4]) >> 4) & (v[4] ^ v[0])) ^ v[0]);

	rgbs = WORD(v0 + (v0 >> 12));
}