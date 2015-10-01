#pragma once

#pragma pack(push,1)
struct Light
{
	union
	{
		struct
		{
			//* Sun Light
			uint16 s : 4;
			//* Blue Light
			uint16 b : 4;
			//* Green Light
			uint16 g : 4;
			//* Red Light
			uint16 r : 4;
		};
		uint16 rgbs;
	};

	Light(BYTE _r, BYTE _g, BYTE _b, BYTE _s) :
		rgbs((_r << 12) + (_g << 8) + (_b << 4) + _s)
	{}

	Light(BYTE _r, BYTE _g, BYTE _b) :
		rgbs((_r << 12) + (_g << 8) + (_b << 4))
	{}

	Light(BYTE _rgb, BYTE _s) :
		rgbs((_rgb * 0x1110) + _s)
	{}

	Light(uint16 _rgbs) :
		rgbs(_rgbs)
	{}

	Light() = default;

	Light operator += (const Light rhs);
	Light operator -= (const Light rhs);

	Light operator |= (const Light rhs);
	Light operator &= (const Light rhs);

	Light operator += (const BYTE rhs);
	Light operator -= (const BYTE rhs);

	Light operator + (const BYTE rhs);
	Light operator - (const BYTE rhs);

	bool operator == (const Light rhs) const; // EQUAL
	bool operator >= (const Light rhs) const; // ANY is bigger
	bool operator > (const Light rhs) const; // ANY is bigger
	bool operator < (const Light rhs) const; // ALL are smaller

	DWORD		Color() const;
	Vector4		FColor() const;
	DWORD		MergedColor(const Light l1, const Light l2, const Light l3) const;

	void MaxLight(const Light l1, const Light ldv);
	void MaxLight(const Light l1, const Light l2, const Light ldv);
	void MaxLight(const Light l1, const Light l2, const Light l3, const Light ldv);
	void MaxLight(const Light l[6], const Light ldv);
};
#pragma pack(pop)

namespace std
{
	template <typename charT, typename traitsT>
	basic_ostream<charT, traitsT> & operator << (basic_ostream<charT, traitsT> & strm, const Light & rhs)
	{
		return strm
			<< _L("0x")
			<< setfill(charT('0'))
			<< setw(4)
			<< hex
			<< rhs.rgbs;
	};
};


