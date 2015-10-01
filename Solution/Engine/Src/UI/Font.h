#pragma once
#include "UIElement.h"

#include <unordered_map>

namespace Rath
{
	class Font
	{
	protected:
#pragma pack(push)
#pragma pack(1)
		struct infoBlock
		{
			unsigned short fontSize;
			char           reserved : 4;
			char           bold : 1;
			char           italic : 1;
			char           unicode : 1;
			char           smooth : 1;
			unsigned char  charSet;
			unsigned short stretchH;
			char           aa;
			unsigned char  paddingUp;
			unsigned char  paddingRight;
			unsigned char  paddingDown;
			unsigned char  paddingLeft;
			unsigned char  spacingHoriz;
			unsigned char  spacingVert;
			unsigned char  outline;
			char           fontName[1];
		} mInfo;
		struct commonBlock
		{
			unsigned short lineHeight;
			unsigned short base;
			unsigned short scaleW;
			unsigned short scaleH;
			unsigned short pages;
			unsigned char  packed : 1;
			unsigned char  reserved : 7;
			unsigned char  alphaChnl;
			unsigned char  redChnl;
			unsigned char  greenChnl;
			unsigned char  blueChnl;
		} mCommon;
		struct charBlock
		{
			DWORD id;
			WORD x;
			WORD y;
			WORD width;
			WORD height;
			short xoffset;
			short yoffset;
			short xadvance;
			char  page;
			char  channel;
		};
		struct kerningBlock
		{
			DWORD first;
			DWORD second;
			short amount;
		};
#pragma pack(pop)
		char* mTextureName;
		std::unordered_map<DWORD, charBlock> mChars;
		std::unordered_map<UINT64, kerningBlock> mKernings;
	public:
		Font(const char* Font);
		~Font();
		char* GetTextureName() { return mTextureName; };
	};

	class FontElement : public Font, public UIElement
	{
	protected:
		UINT mBackBufferWidth;
		UINT mBackBufferHeight;

		void PutChar(const WCHAR wchar, XMFLOAT2& position, FLOAT size, FLOAT depth, const FLOAT* color);
	public:
		FontElement(LPCSTR font);

		void WindowSizeChanged(uint32 width, uint32 height) { mBackBufferWidth = width; mBackBufferHeight = height; };

		void PrintText(LPCWCHAR text, const XMFLOAT2& position, FLOAT size, const FLOAT* color, FLOAT depth);
		void PrintText(LPCWCHAR text, const XMINT2& position, UINT size, const FLOAT* color, FLOAT depth);
	};
}