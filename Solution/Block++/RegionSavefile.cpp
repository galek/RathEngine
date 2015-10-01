#include "stdafx.h"
#include "RegionSavefile.h"
#include "Chunk.h"

#include "WorldGenerator.h"

#include <FileIO.h>
#include <zlib.h>

RegionSavefile::RegionSavefile(const VectorInt3& Position)
{
	WCHAR filename[_MAX_PATH];
	size_t hash = std::hash<XMINT3>()(Position);
	swprintf_s(filename, _MAX_PATH, L"Data\\Region%llX.rgn", hash);

	if (Rath::FileExists(filename))
	{
		m_Savefile.open(filename, std::fstream::binary | std::fstream::in | std::fstream::out);
		m_Savefile.read((char*)&m_SavefileHeader, sizeof(Header));
		m_Savefile.seekp(0, std::ios::end);
	}
	else
	{
		m_Savefile.open(filename, std::fstream::binary | std::fstream::out);
		memset(&m_SavefileHeader, 0, sizeof(Header));
		m_Savefile.write((char*)&m_SavefileHeader, sizeof(Header));
	}
}


RegionSavefile::~RegionSavefile()
{
	m_Savefile.close();
}

void RegionSavefile::Save(uint32 index, Chunk* chunk)
{
	if (chunk != nullptr)
	{
		auto writeChunk = [&](void)
		{
			m_Savefile.seekp(0, std::ios::end);

			uint32 offset = (uint32)m_Savefile.tellp();

			m_Savefile << (*chunk);

			uint32 size = (uint32)m_Savefile.tellp() - offset;

			m_SavefileHeader.Records[index].Position = offset;
			m_SavefileHeader.Records[index].Size = size;

			m_Savefile.seekp(sizeof(Record) * index, std::ios::beg);
			m_Savefile.write((const char*)&m_SavefileHeader.Records[index], sizeof(Record));
		};

		if (m_Savefile.eof()) // no Entries are following
		{
			writeChunk();
		}
		else if (m_SavefileHeader.Records[index].Size == 0)
		{
			writeChunk();
		}
		else //if (mChanged[index])
		{
			//mSavefile.seekp(offset);
			//mSavefile.seekg(offset + mSavefileHeader.Records[index].Size + sizeof(uint32));

			//std::copy(
			//	std::istream_iterator<unsigned char>(mSavefile),
			//	std::istream_iterator<unsigned char>(),
			//	std::ostream_iterator<unsigned char>(mSavefile));

			//for (size_t j = index + 1; j < REGION_SQRWIDTH; j++)
			//{
			//	mSavefileHeader.Records[j].Position -= mSavefileHeader.Records[index].Size;
			//}

			//writeChunk();
		}
	}
}

void RegionSavefile::Load(uint32 index, Chunk* chunk)
{
	if (chunk != nullptr)
	{
		if (m_SavefileHeader.Records[index].Size > 0)
		{
			uint32 offset = m_SavefileHeader.Records[index].Position;

			m_Savefile.seekg(offset, std::ios::beg);

			m_Savefile >> (*chunk);
		}
		else
		{
			WorldGenerator::GenerateChunk(chunk);
		}
	}
}