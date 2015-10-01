#pragma once
#include <iostream>
#include <map>

#define UUID(x) \
uint32 Save(FILE*); \
uint32 Load(FILE*); \
static const uint32 UUID = x; \
uint32 getUUID() { return UUID; };

class ISaveable
{
public:
	virtual uint32 Save(FILE * file) = 0;
	virtual uint32 Load(FILE * file) = 0;

	/*
	UUID needs to be redefined and unique
	*/
	static const uint32 UUID = 0;
	/*
	getUUID needs to be overwritten
	*/
	virtual uint32 getUUID() { return UUID; };
	/*
	Create-method, takes the File to read and returns
	a new Object created out of the data.
	*/
	template<class T, class ClassT>
	static ClassT * Load(FILE * file)
	{
		T * obj = new T();
		obj->Load(file);
		return obj;
	};
};

template<class ClassT>
class Factory 
{
public:
	typedef ClassT * (*DerivedClassCreatorFn)(FILE*);
private:
	std::map<uint32, DerivedClassCreatorFn>	m_mapClassCreators;
	Factory() {};
public:
	Factory(Factory const&) = delete;
	void operator = (Factory const&) = delete;

	static Factory& Instance() 
	{
		static Factory		instance;
		return				instance;
	};

	void RegisterClassCreator(uint32 id, DerivedClassCreatorFn creatorFn)
	{
		m_mapClassCreators.emplace(id, creatorFn);
	};

	template<class T>
	void RegisterClassCreator() 
	{
#if defined(_PROFILE) | defined(_DEBUG)
		printf("Registered %s with UUID: %d\n", typeid(T).name(), T::UUID);
#endif
		m_mapClassCreators.emplace(T::UUID, ISaveable::Load<T, ClassT>);
	};

	void UnregisterClassCreator(uint32 id)
	{
		m_mapClassCreators.erase(id);
	};

	ClassT * Load(FILE * file)
	{
		uint32 id = 0;
		fread((char*)&id, sizeof(char), sizeof(id), file);
		if (id == 0) return nullptr;
		auto it = m_mapClassCreators.find(id);
		if (it == m_mapClassCreators.end()) 
		{
#if defined(_PROFILE) | defined(_DEBUG)
			printf("Failed to find class with UUID: %d\n", id);
#endif
			return nullptr;
		}
		else 
		{
			return it->second(file);
		}
	};

	void Save(ClassT * tclass, FILE * file)
	{
		if (tclass) 
		{
			uint32 id = tclass->getUUID();
			fwrite((const char*)&id, sizeof(char), sizeof(id), file);
			tclass->Save(file);
		}
		else 
		{
			uint32 id = 0;
			fwrite((const char*)&id, sizeof(char), sizeof(id), file);
		}
	};
};