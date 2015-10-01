#include "pch.h"
#include "Font.h"
#include "AssetLibrary.h"

#include "FileIO.h"

namespace Rath
{
	Font::Font(const char* Font) :
		mTextureName(nullptr)
	{
		char magicword[3];
		char version;
		char check;
		int size;

		CHAR buffer[_MAX_PATH];
		sprintf_s(buffer, _MAX_PATH, "%s%s", FONTFOLDER, Font);
		FileContainer file;
		file.open(buffer);
		file.read(magicword, 3);
		file.read(version);

		// --- INFOBLOCK --- //
		file.read(check);
		file.read(&size, 4);
		file.read(&mInfo, sizeof(infoBlock));
		size -= sizeof(infoBlock);
		char* fontname = (char*)malloc(size);
		file.read(fontname, size);
		delete fontname;
		// --- INFOBLOCK --- //

		// --- COMMONBLOCK --- //
		file.read(check);
		file.read(&size, 4);
		file.read(&mCommon, sizeof(commonBlock));
		// --- COMMONBLOCK --- //

		// --- PAGEBLOCK --- //
		file.read(check);
		file.read(&size, 4);
		size /= mCommon.pages;
		char* pagename = (char*)malloc(size);
		for (int n = 0; n < mCommon.pages; n++)
		{
			file.read(pagename, size);
		}
		mTextureName = pagename;
		// --- PAGEBLOCK --- //

		// --- GLYPHBLOCK --- //
		file.read(check);
		file.read(&size, 4);
		size /= sizeof(charBlock);
		for (int n = 0; n < size; n++)
		{
			charBlock cb;
			file.read(&cb, sizeof(charBlock));
			mChars.insert(std::make_pair(cb.id, cb));
		}
		// --- GLYPHBLOCK --- //

		// --- KERNINGBLOCK --- //
		file.read(check);
		file.read(&size, 4);
		size /= sizeof(kerningBlock);
		for (int n = 0; n < size; n++)
		{
			kerningBlock kb;
			file.read(&kb, sizeof(kerningBlock));
			mKernings.insert(std::make_pair(UINT64(kb.first) + (UINT64(kb.second) << 32), kb));
		}
		// --- KERNINGBLOCK --- //

		file.close();
	}

	Font::~Font()
	{
		SAFE_DELETE(mTextureName);
	}

	void FontElement::PutChar(const WCHAR wchar, XMFLOAT2& position, FLOAT size, FLOAT depth, const FLOAT* color)
	{
		auto it = mChars.find(DWORD(wchar));
		if (it != mChars.end())
		{
			XMFLOAT4 texture, quad;

			texture.x = float(it->second.x) / mCommon.scaleW;
			texture.y = float(it->second.y) / mCommon.scaleH;
			texture.z = float(it->second.x + it->second.width) / mCommon.scaleW;
			texture.w = float(it->second.y + it->second.height) / mCommon.scaleH;

			quad.x = position.x + float(it->second.xoffset) * size;
			quad.y = position.y + float(it->second.yoffset) * size;
			quad.z = quad.x + float(it->second.width) * size;
			quad.w = quad.y + float(it->second.height) * size;

			quad.x /= mBackBufferWidth;
			quad.y /= mBackBufferHeight;
			quad.z /= mBackBufferWidth;
			quad.w /= mBackBufferHeight;

			position.x += float(it->second.xadvance) * size;

			PushQuad(quad, depth, texture, color);
		}
	}

	void FontElement::PrintText(LPCWCHAR text, const XMFLOAT2& position, FLOAT size, const FLOAT* color, FLOAT depth)
	{
		size_t index = 0;
		WCHAR wchar = text[index];
		XMFLOAT2 textPosition = XMFLOAT2(position.x * mBackBufferWidth, position.y * mBackBufferHeight);
		while (wchar != 0)
		{
			if (wchar == L'\n')
			{
				textPosition.y += ((float)mCommon.lineHeight / mBackBufferHeight) * size;
				textPosition.x = position.x * mBackBufferWidth;
			}
			else
			{
				PutChar(wchar, textPosition, size, depth, color);
			}
			wchar = text[++index];
		}
	}

	void FontElement::PrintText(LPCWCHAR text, const XMINT2& position, UINT size, const FLOAT* color, FLOAT depth)
	{
		size_t index = 0;
		WCHAR wchar = text[index];
		XMFLOAT2 textPosition = XMFLOAT2((float)position.x, (float)position.y);
		FLOAT fsize = float(size) / mInfo.fontSize;
		while (wchar != 0)
		{
			if (wchar == L'\n')
			{
				textPosition.y += ((float)mCommon.lineHeight / mBackBufferHeight) * fsize;
				textPosition.x = (float)position.x;
			}
			else
			{
				PutChar(wchar, textPosition, fsize, depth, color);
			}
			wchar = text[++index];
		}
	}

	FontElement::FontElement(LPCSTR font) :
		Font(font),
		mBackBufferWidth(800),
		mBackBufferHeight(600)
	{
		m_pTexture = AssetLibrary::GetTexture(mTextureName);
	}
}