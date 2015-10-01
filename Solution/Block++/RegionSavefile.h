#pragma once

class Chunk;
class RegionSavefile
{
private:
	struct Record
	{
		uint32 Position;
		uint32 Size;
	};
	struct Header
	{
		Record	Records[REGION_SQRWIDTH];
	};
protected:
	Header			m_SavefileHeader;
	std::fstream	m_Savefile;
public:
	RegionSavefile(const VectorInt3& Position);
	~RegionSavefile();

	void Save(uint32 index, Chunk* chunk);
	void Load(uint32 index, Chunk* chunk);
};

