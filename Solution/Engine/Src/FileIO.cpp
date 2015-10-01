#include "pch.h"
#include "FileIO.h"
#include "Debug/Exceptions.h"

namespace Rath
{

	// Returns true if a file exits
	bool FileExists(const wchar* filePath)
	{
		if (filePath == NULL)
			return false;

		DWORD fileAttr = GetFileAttributes(filePath);
		if (fileAttr == INVALID_FILE_ATTRIBUTES)
			return false;

		return true;
	}

	// Retursn true if a directory exists
	bool DirectoryExists(const wchar* dirPath)
	{
		if (dirPath == NULL)
			return false;

		DWORD fileAttr = GetFileAttributes(dirPath);
		return (fileAttr != INVALID_FILE_ATTRIBUTES && (fileAttr & FILE_ATTRIBUTE_DIRECTORY));
	}


	// Returns the directory containing a file
	std::wstring GetDirectoryFromFilePath(const wchar* filePath_)
	{
		Assert_(filePath_);

		std::wstring filePath(filePath_);
		size_t idx = filePath.rfind(L'\\');
		if (idx != std::wstring::npos)
			return filePath.substr(0, idx + 1);
		else
			return std::wstring(L"");
	}

	// Returns the name of the file given the path (extension included)
	std::wstring GetFileName(const wchar* filePath_)
	{
		Assert_(filePath_);

		std::wstring filePath(filePath_);
		size_t idx = filePath.rfind(L'\\');
		if (idx != std::wstring::npos && idx < filePath.length() - 1)
			return filePath.substr(idx + 1);
		else
			return filePath;
	}

	// Returns the name of the file given the path, minus the extension
	std::wstring GetFileNameWithoutExtension(const wchar* filePath)
	{
		std::wstring fileName = GetFileName(filePath);
		return GetFilePathWithoutExtension(fileName.c_str());
	}

	// Returns the given file path, minus the extension
	std::wstring GetFilePathWithoutExtension(const wchar* filePath_)
	{
		Assert_(filePath_);

		std::wstring filePath(filePath_);
		size_t idx = filePath.rfind(L'.');
		if (idx != std::wstring::npos)
			return filePath.substr(0, idx);
		else
			return std::wstring(L"");
	}

	// Returns the extension of the file path
	std::wstring GetFileExtension(const wchar* filePath_)
	{
		Assert_(filePath_);

		std::wstring filePath(filePath_);
		size_t idx = filePath.rfind(L'.');
		if (idx != std::wstring::npos)
			return filePath.substr(idx + 1, filePath.length() - idx - 1);
		else
			return std::wstring(L"");
	}

	// Gets the last written timestamp of the file
	uint64 GetFileTimestamp(const wchar* filePath)
	{
		Assert_(filePath);

		WIN32_FILE_ATTRIBUTE_DATA attributes;
		Win32Call(GetFileAttributesEx(filePath, GetFileExInfoStandard, &attributes));
		return attributes.ftLastWriteTime.dwLowDateTime | (uint64(attributes.ftLastWriteTime.dwHighDateTime) << 32);
	}

	// Returns the contents of a file as a string
	std::string ReadFileAsString(const wchar* filePath)
	{
		File file(filePath, File::OpenRead);
		size_t fileSize = file.Size();

		std::string fileContents;
		fileContents.resize(size_t(fileSize), 0);
		file.Read(fileSize, &fileContents[0]);

		return fileContents;
	}

	// Writes the contents of a string to a file
	void WriteStringAsFile(const wchar* filePath, const std::string& data)
	{
		File file(filePath, File::OpenWrite);
		file.Write(data.length(), data.c_str());
	}

	// == File ========================================================================================

	File::File() : fileHandle(INVALID_HANDLE_VALUE), openMode(OpenRead)
	{
	}

	File::File(const wchar* filePath, OpenMode openMode) : fileHandle(INVALID_HANDLE_VALUE),
		openMode(OpenRead)
	{
		Open(filePath, openMode);
	}

	File::~File()
	{
		Close();
		Assert_(fileHandle == INVALID_HANDLE_VALUE);
	}

	void File::Open(const wchar* filePath, OpenMode openMode_)
	{
		Assert_(fileHandle == INVALID_HANDLE_VALUE);
		openMode = openMode_;

		if (openMode == OpenRead)
		{
			Assert_(FileExists(filePath));

			// Open the file
			fileHandle = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fileHandle == INVALID_HANDLE_VALUE)
				Win32Call(false);
		}
		else
		{
			// If the exists, delete it
			if (FileExists(filePath))
				Win32Call(DeleteFile(filePath));

			// Create the file
			fileHandle = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fileHandle == INVALID_HANDLE_VALUE)
				Win32Call(false);
		}
	}

	void File::Close()
	{
		if (fileHandle == INVALID_HANDLE_VALUE)
			return;

		// Close the file
		Win32Call(CloseHandle(fileHandle));

		fileHandle = INVALID_HANDLE_VALUE;
	}

	void File::Read(size_t size, void* data) const
	{
		Assert_(fileHandle != INVALID_HANDLE_VALUE);
		Assert_(openMode == OpenRead);

		DWORD bytesRead = 0;
		Win32Call(ReadFile(fileHandle, data, static_cast<DWORD>(size), &bytesRead, NULL));
	}

	void File::Write(size_t size, const void* data) const
	{
		Assert_(fileHandle != INVALID_HANDLE_VALUE);
		Assert_(openMode == OpenWrite);

		DWORD bytesWritten = 0;
		Win32Call(WriteFile(fileHandle, data, static_cast<DWORD>(size), &bytesWritten, NULL));
	}

	size_t File::Size() const
	{
		Assert_(fileHandle != INVALID_HANDLE_VALUE);

		LARGE_INTEGER fileSize;
		Win32Call(GetFileSizeEx(fileHandle, &fileSize));

		return (size_t)fileSize.QuadPart;
	}


#include <zlib.h>

	void FileContainer::open(const char* filename)
	{
		FILE * pFile;
		fopen_s(&pFile, filename, "ab+");
		if (pFile != nullptr)
		{
			fclose(pFile);
		}
		auto file = std::filebuf::open(filename, std::ios::in | std::ios::out | std::ios::binary);
	}

	void FileContainer::close()
	{
		std::filebuf::close();
	}

	void FileContainer::write(const void* ptr, unsigned int size)
	{
		sputn((const char*)ptr, size);
	}

	void FileContainer::write(const std::string& str)
	{
		UINT size = (UINT)str.size();
		sputn((const char*)&size, sizeof(size));
		sputn(str.c_str(), size);
	}

	void FileContainer::writeCompressed(const void* ptr, unsigned int size)
	{
		uLongf CompressedSpace = size * 2;
		Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);
		compress(CompressedSpaceBuffer, &CompressedSpace, (Bytef*)ptr, size);
		sputn((const char*)&CompressedSpace, sizeof(CompressedSpace));
		sputn((const char*)CompressedSpaceBuffer, (unsigned int)CompressedSpace);
		free(CompressedSpaceBuffer);
	}

	void FileContainer::write(char character)
	{
		sputc(character);
	}

	void FileContainer::read(void* ptr, unsigned int size)
	{
		sgetn((char*)ptr, size);
	}

	void FileContainer::read(std::string& str)
	{
		UINT size = 0;
		sgetn((char*)&size, sizeof(size));
		str.resize(size);
		sgetn(&str[0], size);
	}

	void FileContainer::readCompressed(void* ptr, unsigned int size)
	{
		uLongf CompressedSpace = 0;
		sgetn((char*)&CompressedSpace, sizeof(CompressedSpace));
		Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);
		sgetn((char*)CompressedSpaceBuffer, CompressedSpace);
		uLongf UncompressedSize = size;
		uncompress((Bytef*)ptr, &UncompressedSize, CompressedSpaceBuffer, CompressedSpace);
		free(CompressedSpaceBuffer);
	}

	void FileContainer::read(char& character)
	{
		if (sgetc() != EOF)
		{
			character = sbumpc();
		}
		else
		{
			// Error EOF
		}
	}

	template<typename T> std::streamsize write_compressed(std::ostream& stream, const T& value)
	{
		uLong CompressedSpace = sizeof(T) * 2;
		Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);
		compress(CompressedSpaceBuffer, &CompressedSpace, (Bytef*)&value, sizeof(T));
		stream.write((const char*)&CompressedSpace, sizeof(CompressedSpace));
		stream.write((const char*)CompressedSpaceBuffer, (unsigned int)CompressedSpace);
		free(CompressedSpaceBuffer);
		return (std::streamsize)CompressedSpace + sizeof(CompressedSpace);
	};

	template<typename T> std::streamsize read_compressed(std::istream& stream, const T& value)
	{
		uLong CompressedSpace = 0;
		stream.read((char*)&CompressedSpace, sizeof(CompressedSpace));
		Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);
		stream.read((char*)CompressedSpaceBuffer, CompressedSpace);
		uLong UncompressedSize = sizeof(T);
		uncompress((Bytef*)&value, &UncompressedSize, CompressedSpaceBuffer, CompressedSpace);
		free(CompressedSpaceBuffer);
		return (std::streamsize)CompressedSpace + sizeof(CompressedSpace);
	}

	std::streamsize write_compressed(std::ostream& stream, const char* s, std::streamsize n)
	{
		uLong CompressedSpace = (uLong)n * 2;
		Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);
		compress(CompressedSpaceBuffer, &CompressedSpace, (Bytef*)s, (uLong)n);
		stream.write((const char*)&CompressedSpace, sizeof(CompressedSpace));
		stream.write((const char*)CompressedSpaceBuffer, (unsigned int)CompressedSpace);
		free(CompressedSpaceBuffer); 
		return (std::streamsize)CompressedSpace + sizeof(CompressedSpace);
	};

	std::streamsize read_compressed(std::istream& stream, const char* s, std::streamsize n)
	{
		uLong CompressedSpace = 0;
		stream.read((char*)&CompressedSpace, sizeof(CompressedSpace));
		Bytef* CompressedSpaceBuffer = (Bytef*)malloc(CompressedSpace);
		stream.read((char*)CompressedSpaceBuffer, CompressedSpace);
		uLong UncompressedSize = (uLong)n;
		uncompress((Bytef*)s, &UncompressedSize, CompressedSpaceBuffer, CompressedSpace);
		free(CompressedSpaceBuffer);
		return (std::streamsize)CompressedSpace + sizeof(CompressedSpace);
	}
}