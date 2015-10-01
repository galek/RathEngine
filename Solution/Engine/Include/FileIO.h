#pragma once

namespace Rath
{
	// Utility functions
	bool FileExists(const wchar* filePath);
	bool DirectoryExists(const wchar* dirPath);
	std::wstring GetDirectoryFromFilePath(const wchar* filePath);
	std::wstring GetFileName(const wchar* filePath);
	std::wstring GetFileNameWithoutExtension(const wchar* filePath);
	std::wstring GetFilePathWithoutExtension(const wchar* filePath);
	std::wstring GetFileExtension(const wchar* filePath);
	uint64 GetFileTimestamp(const wchar* filePath);

	std::string ReadFileAsString(const wchar* filePath);
	void WriteStringAsFile(const wchar* filePath, const std::string& data);

	class File
	{

	public:

		enum OpenMode
		{
			OpenRead = 0,
			OpenWrite = 1,
		};

	private:

		HANDLE fileHandle;
		OpenMode openMode;

	public:

		// Lifetime
		File();
		File(const wchar* filePath, OpenMode openMode);
		~File();

		// Explicit Open and close
		void Open(const wchar* filePath, OpenMode openMode);
		void Close();

		// I/O
		void Read(size_t size, void* data) const;
		void Write(size_t size, const void* data) const;

		template<typename T> void Read(T& data) const;
		template<typename T> void Write(const T& data) const;

		// Accessors
		size_t Size() const;
	};

	// == File ========================================================================================

	template<typename T> void File::Read(T& data) const
	{
		Read(sizeof(T), &data);
	}

	template<typename T> void File::Write(const T& data) const
	{
		Write(sizeof(T), &data);
	}

	// Templated helper functions

	// Reads a POD type from a file
	template<typename T> void ReadFromFile(const wchar* fileName, T& val)
	{
		File file(fileName, File::OpenRead);
		file.Read(val);
	}

	// Writes a POD type to a file
	template<typename T> void WriteToFile(const wchar* fileName, const T& val)
	{
		File file(fileName, File::OpenWrite);
		file.Write(val);
	}

	class FileContainer : private std::filebuf
	{
	protected:

	public:
		void open(const char* filename);
		void close();

		void write(const void* ptr, unsigned int size);
		void write(const std::string& str);
		void writeCompressed(const void* ptr, unsigned int size);
		void write(char character);

		void read(void* ptr, unsigned int size);
		void read(std::string& str);
		void readCompressed(void* ptr, unsigned int size);
		void read(char& character);
	};

	template<typename T> std::streamsize write_compressed(std::ostream& stream, const T& value);
	template<typename T> std::streamsize read_compressed(std::istream& stream, const T& value);

	std::streamsize write_compressed(std::ostream& stream, const char* s, std::streamsize n);
	std::streamsize read_compressed(std::istream& stream, const char* s, std::streamsize n);
}