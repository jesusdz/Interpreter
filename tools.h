/*
 * tools.h
 * Author: Jesus Diaz Garcia
 *
 * Single file library with several utilities among the following:
 * - Platform identification
 * - Assertions, debugging, errors, logging
 * - Aliases for sized types
 * - Intrinsics
 * - Strings
 * - Hashing
 * - Memory allocators
 * - File reading
 * - Mathematics
 * - Clock / timing
 * - Window creation
 * - Input handling (mouse and keyboard)
 */

#ifndef TOOLS_H
#define TOOLS_H

////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform definitions

#if _WIN32
#	define PLATFORM_WINDOWS 1
#elif __ANDROID__
#	define PLATFORM_ANDROID 1
#elif __linux__
#	define PLATFORM_LINUX 1
#elif __APPLE__
#	define PLATFORM_APPLE 1
#else
#	error "Unsupported platform"
#endif


#if PLATFORM_WINDOWS || PLATFORM_LINUX
#define USE_IMGUI 1
#endif


#if PLATFORM_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WindowsX.h>
#elif PLATFORM_LINUX || PLATFORM_ANDROID
#include <time.h>     // TODO: Find out if this header belongs to the C runtime library...
#include <sys/stat.h> // stat
#include <fcntl.h>    // open
#include <unistd.h>   // read, close
#include <string.h>   // strerror_r
#include <errno.h>    // errno
#include <sys/mman.h> // mmap
#endif

#if PLATFORM_ANDROID
#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#endif


#include <stdio.h>  // printf
#include <math.h>
// TODO: Remove C runtime library includes. But first...
// TODO: Remove calls to printf.



////////////////////////////////////////////////////////////////////////////////////////////////////
// Useful defines

#define internal static

#define KB(x) (1024ul * x)
#define MB(x) (1024ul * KB(x))
#define GB(x) (1024ul * MB(x))
#define TB(x) (1024ul * GB(x))

#if PLATFORM_ANDROID
#define Debug ANDROID_LOG_DEBUG
#define Info ANDROID_LOG_INFO
#define Warning ANDROID_LOG_WARN
#define Error ANDROID_LOG_ERROR
#define LOG(channel, fmt, ...) ((void)__android_log_print(channel, "tools", fmt, ##__VA_ARGS__))
#else
#define LOG(channel, fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

#if PLATFORM_ANDROID
#define QUIT_ABNORMALLY() __builtin_trap();
#else
#define QUIT_ABNORMALLY() *((int*)0) = 0;
#endif

#define ASSERT(expression) if ( !(expression) ) { \
		LOG(Error, "%s", "Assertion failed: ASSERT( " #expression " )\n" ); \
		QUIT_ABNORMALLY(); \
	}
#define INVALID_CODE_PATH() ASSERT(0 && "Invalid code path")
#define INVALID_CODE_PATH_MSG(message) ASSERT(0 && message)
#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#define CT_ASSERT3(expression, number) static int ct_assert_##number[(expression) ? 1 : 0]
#define CT_ASSERT2(expression, number) CT_ASSERT3(expression, number)
#define CT_ASSERT(expression) CT_ASSERT2(expression, __COUNTER__)



////////////////////////////////////////////////////////////////////////////////////////////////////
// Sized types

typedef char i8;
typedef short int i16;
typedef int i32;
typedef long long int i64;
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef float f32;
typedef double f64;
typedef	unsigned char byte;

CT_ASSERT(sizeof(i8) == 1);
CT_ASSERT(sizeof(i16) == 2);
CT_ASSERT(sizeof(i32) == 4);
CT_ASSERT(sizeof(i64) == 8);
CT_ASSERT(sizeof(u8) == 1);
CT_ASSERT(sizeof(u16) == 2);
CT_ASSERT(sizeof(u32) == 4);
CT_ASSERT(sizeof(u64) == 8);
CT_ASSERT(sizeof(f32) == 4);
CT_ASSERT(sizeof(f64) == 8);
CT_ASSERT(sizeof(byte) == 1);

#define U8_MAX 255
#define U16_MAX 65535
#define U32_MAX 4294967295



////////////////////////////////////////////////////////////////////////////////////////////////////
// System helpers

#if PLATFORM_WINDOWS

void Win32ReportError()
{
	DWORD errorCode = ::GetLastError();
	if ( errorCode != ERROR_SUCCESS )
	{
		LPVOID messageBuffer;
		FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				errorCode,
				MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
				(LPTSTR)&messageBuffer,
				0, NULL );

		LOG(Error, "Error: %s\n", (char*)messageBuffer);
		LocalFree( messageBuffer );
	}
}

#elif PLATFORM_LINUX || PLATFORM_ANDROID

void LinuxReportError(const char *context)
{
	char buffer[1024] = {};
	const char *errorString = strerror_r(errno, buffer, ARRAY_COUNT(buffer));
	LOG(Error, "Error (%s): %s\n", context, errorString);
}

#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// Intrinsics

#define TOOLS_USE_INTRINSICS 1

#if TOOLS_USE_INTRINSICS
#if PLATFORM_WINDOWS
// Count trailing zeros
u32 CTZ(u32 bitMask)
{
	ASSERT(bitMask != 0);
	unsigned long firstBitSetIndex;
	_BitScanForward(&firstBitSetIndex, bitMask);
	return firstBitSetIndex;
}

// Count leading zeros
u32 CLZ(u32 bitMask)
{
	ASSERT(bitMask != 0);
	unsigned long lastBitSetIndex;
	_BitScanReverse(&lastBitSetIndex, bitMask);
	return 31 - lastBitSetIndex;
}
#else // #if PLATFORM_WINDOWS
// Count trailing zeros
u32 CTZ(u32 bitMask)
{
	ASSERT(bitMask != 0);
	u32 count = __builtin_ctz(bitMask);
	return count;
}

// Count leading zeros
u32 CLZ(u32 bitMask)
{
	ASSERT(bitMask != 0);
	u32 count = __builtin_clz(bitMask);
	return count;
}
#endif // #else // #if PLATFORM_WINDOWS
#else // #if TOOLS_USE_INTRINSICS
// Count trailing zeros
u32 CTZ(u32 bitMask)
{
	ASSERT(bitMask != 0);
	u32 count = 0;
	while ((bitMask & 1) == 0) {
		bitMask >>= 1;
		++count;
	}
	return count;
}

// Count leading zeros
u32 CLZ(u32 bitMask)
{
	// TODO: This has not been tested
	ASSERT(bitMask != 0);
	u32 count = 0;
	while ((bitMask & 0x80000000) == 0) {
		bitMask <<= 1;
		++count;
	}
	return count;
}
#endif // #else // #if TOOLS_USE_INTRINSICS



////////////////////////////////////////////////////////////////////////////////////////////////////
// Strings

struct String
{
	const char* str;
	u32 size;
};

u32 StrLen(const char *str)
{
	u32 len = 0;
	while (*str++) ++len;
	return len;
}

String MakeString(const char *str, u32 size)
{
	String string = { str, size };
	return string;
}

String MakeString(const char *str)
{
	u32 size = StrLen(str);
	String string = { str, size };
	return string;
}

void StrCopy(char *dst, const String& src_string)
{
	u32 size = src_string.size;
	const char *src = src_string.str;
	while (size-- > 0) *dst++ = *src++;
	*dst = '\0';
}

void StrCopy(char *dst, const char *src)
{
	while (*src) *dst++ = *src++;
	*dst = 0;
}

void StrCopyN(char *dst, const char *src, u32 size)
{
	while (*src && size-- > 0) *dst++ = *src++;
	*dst = 0;
}

void StrCat(char *dst, const char *src)
{
	while (*dst) ++dst;
	StrCopy(dst, src);
}

void StrCat(char *dst, const String &src)
{
	while (*dst) ++dst;
	StrCopy(dst, src);
}

bool StrEq(const String &s1, const String &s2)
{
	if ( s1.size != s2.size ) return false;

	const char *str1 = s1.str;
	const char *str2 = s2.str;
	const char *str2end = s2.str + s2.size;

	while ( *str1 == *str2 && str2 != str2end )
	{
		str1++;
		str2++;
	}

	return str2 == str2end;
}

bool StrEq(const String &s11, const char *s2)
{
	const char *s1 = s11.str;
	u32 count = s11.size;
	while ( count > 0 && *s1 == *s2 )
	{
		s1++;
		s2++;
		count--;
	}
	return count == 0 && *s2 == 0;
}

bool StrEq(const char *s1, const char *s2)
{
	while ( *s1 == *s2 && *s1 )
	{
		s1++;
		s2++;
	}
	return *s1 == *s2;
}

bool StrEqN(const char *s1, const char *s2, u32 n)
{
	while ( *s1 == *s2 && *s1 && n > 0 )
	{
		s1++;
		s2++;
		n--;
	}
	return n == 0 || *s1 == *s2;
}

// Searches str2 in str1 and returns a pointer to the last matching character '\0'.
// If the beginning of str1 does not completely match str2, then NULL is returned.
const char *StrConsume( const char *str1, const char *str2 )
{
	while ( *str2 && *str1++ == *str2++ );
	// If str does not point to the last char, there's no match so NULL is returned.
	// Otherwise the 'advanced' str1 pointer is returned.
	return *str2 ? NULL: str1;
}


bool StrToBool(const char *str, u32 len = U32_MAX)
{
	const bool value = *str == '1' || StrEqN(str, "true", len);
	return value;
}

bool StrToBool(const String &s)
{
	const bool value = StrToBool(s.str, s.size);
	return value;
}

bool StrToChar(const char *str, u32 len = U32_MAX)
{
	return len > 0 ? *str : '?';
}

bool StrToChar(const String &s)
{
	const char value = StrToChar(s.str, s.size);
	return value;
}

i32 StrToInt(const char *str, u32 len = U32_MAX)
{
	i32 integer = 0;

	// scan sign
	bool negative = false;
	if (*str == '-') {
		negative = true;
		str++;
		len--;
	}

	// scan integer part
	while (*str >= '0' && *str <= '9' && len > 0) {
		integer = (integer << 3) + (integer << 1); // x10
		integer += *str++ - '0';
		len--;
	}

	const i32 result = negative ? -integer : integer;
	return result;
}

i32 StrToInt(const String &s)
{
	const i32 number = StrToInt(s.str, s.size);
	return number;
}

u32 StrToUnsignedInt(const char *str, u32 len = U32_MAX)
{
	u32 integer = 0;

	// scan integer part
	while (*str >= '0' && *str <= '9' && len > 0) {
		integer = (integer << 3) + (integer << 1); // x10
		integer += *str++ - '0';
		len--;
	}

	return integer;
}

u32 StrToUnsignedInt(const String &s)
{
	const u32 number = StrToUnsignedInt(s.str, s.size);
	return number;
}

f32 StrToFloat(const char *str, u32 len = U32_MAX)
{
	i32 integer = 0;

	// scan sign
	i32 sign = 1.0f;
	if (*str == '-') {
		sign = -1.0f;
		str++;
		len--;
	}

	// scan integer part
	while (*str >= '0' && *str <= '9' && len > 0) {
		integer = (integer << 3) + (integer << 1); // x10
		integer += *str++ - '0';
		len--;
	}

	switch (*str++) {
		case '.': break;
		default: return integer;
	}
	len--;

	// scan decimal part
	u32 tenPower = 1;
	while (*str >= '0' && *str <= '9' && len > 0) {
		tenPower = (tenPower << 3) + (tenPower << 1);
		integer = (integer << 3) + (integer << 1); // x10
		integer += *str++ - '0';
		len--;
	}

	const float value = sign * integer / (f32)tenPower;
	return value;
}

f32 StrToFloat(const String &s)
{
	const f32 number = StrToFloat(s.str, s.size);
	return number;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Hashing

#define TOOLS_HASH_FNV_SEED 16777619

u32 HashFNV(const void *data, u32 size, u32 prime = TOOLS_HASH_FNV_SEED)
{
	u32 hash = 0;
	const unsigned char *str = (const unsigned char *)data;
	for (u32 i = 0; i < size; ++i) {
		hash = (hash * prime) ^ str[i];
	}
	return hash;
}

u32 HashStringFNV(const char *data, u32 prime = TOOLS_HASH_FNV_SEED)
{
	u32 hash = 0;
	const unsigned char *str = (const unsigned char *)data;
	while (*str) {
		hash = (hash * prime) ^ *str++;
	}
	return hash;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory

#if PLATFORM_LINUX || PLATFORM_ANDROID || PLATFORM_APPLE

void* AllocateVirtualMemory(u32 size)
{
	void* baseAddress = 0;
	i32 prot = PROT_READ | PROT_WRITE;
	i32 flags = MAP_PRIVATE | MAP_ANONYMOUS;
	i32 fd = -1;
	off_t offset = 0;
	void *allocatedMemory = mmap(baseAddress, size, prot, flags, fd, offset);
	ASSERT( allocatedMemory != MAP_FAILED && "Failed to allocate memory." );
	return allocatedMemory;
}

#elif PLATFORM_WINDOWS

void* AllocateVirtualMemory(u32 size)
{
	void *data = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	return data;
}

#endif

void MemSet(void *ptr, u32 size, byte value)
{
	byte *bytePtr = (byte*)ptr;
	while (size-- > 0) *bytePtr++ = value;
}

void MemCopy(void *dst, const void *src, u32 size)
{
	const byte *pSrc = (byte*) src;
	const byte *pEnd = pSrc + size;
	byte *pDst = (byte*) dst;
	while (pSrc != pEnd) *pDst++ = *pSrc++;
}

i32 MemCompare(const void *a, const void *b, u32 size)
{
	const byte *pA = (const byte*) a;
	const byte *pB = (const byte*) b;
	const byte *pEnd = pA + size;
	while (pA != pEnd) {
		const byte valueA = *pA++;
		const byte valueB = *pB++;
		if (valueA != valueB) {
			return valueA - valueB;
		}
	}
	return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Arena

struct Arena
{
	byte* base;
	u32 used;
	u32 size;
};

Arena MakeArena(byte* base, u32 size)
{
	ASSERT(base != NULL && "MakeArena needs a non-null base pointer.");
	ASSERT(size > 0 && "MakeArena needs a greater-than-zero size.");
	Arena arena = {};
	arena.base = base;
	arena.size = size;
	arena.used = 0;
	return arena;
}

Arena MakeSubArena(Arena &arena, u32 size)
{
	ASSERT(arena.used + size <= arena.size && "MakeSubArena of bounds of the memory arena.");
	Arena subarena = {};
	subarena.base = arena.base + arena.used;
	subarena.size = size;
	subarena.used = 0;
	return subarena;
}

Arena MakeSubArena(Arena &arena)
{
	u32 remainingSize = arena.size - arena.used;
	Arena subarena = MakeSubArena(arena, remainingSize);
	return subarena;
}

byte* PushSize(Arena &arena, u32 size)
{
	ASSERT(arena.used + size <= arena.size && "PushSize of bounds of the memory arena.");
	byte* head = arena.base + arena.used;
	arena.used += size;
	return head;
}

byte* PushZeroSize(Arena &arena, u32 size)
{
	byte *bytes = PushSize(arena, size);
	MemSet(bytes, size, 0);
	return bytes;
}

char *PushStringN(Arena &arena, const char *str, u32 len)
{
	char *bytes = (char*)PushSize(arena, len+1);
	MemCopy(bytes, str, len);
	bytes[len] = 0;
	return bytes;
}

void ResetArena(Arena &arena)
{
	// This tells the OS we don't need these pages
	//madvise(arena.base, arena.size, MADV_DONTNEED);
	arena.used = 0;
}

void PrintArenaUsage(Arena &arena)
{
	LOG(Info, "Memory Arena Usage:\n");
	LOG(Info, "- size: %u B / %u kB\n", arena.size, arena.size/1024);
	LOG(Info, "- used: %u B / %u kB\n", arena.used, arena.used/1024);
}

#define ZeroStruct( pointer ) MemSet(pointer, sizeof(*pointer), 0)
#define PushStruct( arena, struct_type ) (struct_type*)PushSize(arena, sizeof(struct_type))
#define PushArray( arena, type, count ) (type*)PushSize(arena, sizeof(type) * count)
#define PushZeroStruct( arena, struct_type ) (struct_type*)PushZeroSize(arena, sizeof(struct_type))
#define PushZeroArray( arena, type, count ) (type*)PushZeroSize(arena, sizeof(type) * count)



////////////////////////////////////////////////////////////////////////////////////////////////////
// String interning

struct StringIntern
{
	char *str;
	u32 hash;
};

struct StringInterningNode
{
	StringIntern stringIntern;
	StringInterningNode *next;
};

struct StringInterningTable
{
	StringInterningNode* bins[1024];
};

struct StringInterning
{
	Arena *arena;
	StringInterningTable *table;
};

StringInterning StringInterningCreate(Arena *arena)
{
	StringInterning interning = {
		.arena = arena,
		.table = PushZeroStruct(*arena, StringInterningTable),
	};
	return interning;
}

const char *MakeStringIntern(StringInterning *context, const char *str, u32 len)
{
	ASSERT(context && context->table);
	const u32 hash = HashFNV(str, len);
	const u32 index = hash % ARRAY_COUNT(context->table->bins);

	StringInterningNode *next = NULL;

	if (context->table->bins[index])
	{
		StringInterningNode *node = context->table->bins[index];
		while (node)
		{
			if (node->stringIntern.hash == hash) // same hash?
			{
				if (StrEq(node->stringIntern.str, str)) // same string?
				{
					return node->stringIntern.str; // found!
				}
			}
			node = node->next; // collision! keep searching...
		}
		next = context->table->bins[index];
	}

	// No coincidence found, insert a new node
	StringInterningNode *node = PushZeroStruct(*(context->arena), StringInterningNode);
	node->stringIntern.str = PushStringN(*(context->arena), str, len);
	node->stringIntern.hash = hash;
	node->next = next;
	context->table->bins[index] = node;
	return node->stringIntern.str;
}

const char *MakeStringIntern(StringInterning *context, const char *str)
{
	const u32 len = StrLen(str);
	const char *internStr = MakeStringIntern(context, str, len);
	return internStr;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Files

struct DataChunk
{
	union {
		byte *bytes;
		const char *chars;
	};
	u64 size;
};

bool GetFileSize(const char *filename, u64 &size)
{
	bool ok = true;
#if PLATFORM_WINDOWS
	WIN32_FILE_ATTRIBUTE_DATA Data;
	if( GetFileAttributesExA(filename, GetFileExInfoStandard, &Data) )
	{
		size = (u64)Data.nFileSizeLow;
		size |= (u64)Data.nFileSizeHigh << 32;
	}
	else
	{
		Win32ReportError();
		ok = false;
	}
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	struct stat attrib;
	if ( stat(filename, &attrib) == 0 )
	{
		size = attrib.st_size;
	}
	else
	{
		LinuxReportError("stat");
		ok = false;
	}
#endif
	return ok;
}

bool ReadEntireFile(const char *filename, void *buffer, u64 bytesToRead)
{
	bool ok = false;
#if PLATFORM_WINDOWS
	HANDLE file = CreateFileA( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL );
	if ( file == INVALID_HANDLE_VALUE  )
	{
		Win32ReportError();
	}
	else
	{
		DWORD bytesRead = 0;
		ok = ReadFile( file, buffer, bytesToRead, &bytesRead, NULL );
		if ( !ok && bytesToRead == bytesRead )
		{
			Win32ReportError();
		}
		CloseHandle( file );
	}
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	int fd = open(filename, O_RDONLY);
	if ( fd == -1 )
	{
		LinuxReportError("open");
	}
	else
	{
		while ( bytesToRead > 0 )
		{
			u32 bytesRead = read(fd, buffer, bytesToRead);
			if ( bytesRead > 0 )
			{
				bytesToRead -= bytesRead;
				buffer = (byte*)buffer + bytesRead;
			}
			else
			{
				LinuxReportError("read");
				break;
			}
		}
		ok = (bytesToRead == 0);
		close(fd);
	}
#endif
	return ok;
}

DataChunk *PushFile( Arena& arena, const char *filename )
{
	DataChunk *file = 0;

	u64 fileSize;
	if ( GetFileSize( filename, fileSize ) && fileSize > 0 )
	{
		Arena backupArena = arena;
		byte *fileData = PushArray( arena, byte, fileSize + 1 );
		if ( ReadEntireFile( filename, fileData, fileSize ) )
		{
			fileData[fileSize] = 0; // final zero
			file = PushStruct( arena, DataChunk );
			file->bytes = fileData;
			file->size = fileSize;
		}
		else
		{
			// TODO: Log error here?
			arena = backupArena;
		}
	}

	return file;
}

bool GetFileLastWriteTimestamp(const char* filename, u64 &ts)
{
	bool ok = true;

#if PLATFORM_WINDOWS
	union Filetime2u64 {
		FILETIME filetime;
		u64      u64time;
	} conversor;

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if( GetFileAttributesExA(filename, GetFileExInfoStandard, &Data) )
	{
		conversor.filetime = Data.ftLastWriteTime;
		ts = conversor.u64time;
	}
	else
	{
		Win32ReportError();
		ok = false;
	}
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	struct stat attrib;
	if ( stat(filename, &attrib) == 0 )
	{
		ts = attrib.st_mtime;
	}
	else
	{
		LinuxReportError("stat");
		ok = false;
	}
#endif

	return ok;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// File paths

#define MAX_PATH_LENGTH 512

struct FilePath
{
	char str[MAX_PATH_LENGTH];
};

FilePath MakePath(const char *relativePath)
{
	FilePath path = {};
#if PLATFORM_ANDROID
	// TODO: Don't hardcode this path here and get it from Android API.
	StrCat(path.str, "/sdcard/Android/data/com.tools.game/files/");
#endif
	StrCat(path.str, relativePath);
	return path;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Math

struct uint2
{
	union { u32 x, r; };
	union { u32 y, g; };
};

struct float2
{
	union { float x, r; };
	union { float y, g; };
};

struct float3
{
	union { float x, r; };
	union { float y, g; };
	union { float z, b; };
};

struct float4
{
	union
	{
		f32 vec[4];
		struct
		{
			union { float x, r; };
			union { float y, g; };
			union { float z, b; };
			union { float w, a; };
		};
		float3 xyz;
	};
};

struct float3x3
{
	union
	{
		f32 mat[3][3];
		struct
		{
			f32 m00, m01, m02,
				m10, m11, m12,
				m20, m21, m22;
		};
	};
};

struct float4x4
{
	union
	{
		f32 mat[4][4];
		struct
		{
			f32 m00, m01, m02, m03,
				m10, m11, m12, m13,
				m20, m21, m22, m23,
				m30, m31, m32, m33;
		};
	};
};

static constexpr f32 Pi = 3.14159265358979323846f;
static constexpr f32 ToRadians = Pi / 180.0f;
static constexpr f32 ToDegrees = 180.0f / Pi;

float3 Float3(float value)
{
	const float3 res = {value, value, value};
	return res;
}

float3 Float3(float x, float y, float z)
{
	const float3 res = {x, y, z};
	return res;
}

float4 Float4(float3 xyz, f32 w)
{
	const float4 res = { xyz.x, xyz.y, xyz.z, w };
	return res;
}

float3x3 Float3x3(const float4x4 &m)
{
	const float3x3 res = {
		m.m00, m.m01, m.m02,
		m.m10, m.m11, m.m12,
		m.m20, m.m21, m.m22,
	};
	return res;
}

float4x4 Float4x4(const float3x3 &m)
{
	const float4x4 res = {
		m.m00, m.m01, m.m02, 0.0f,
		m.m10, m.m11, m.m12, 0.0f,
		m.m20, m.m21, m.m22, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	return res;
}

f32 Sin(f32 value)
{
	const f32 res = ::sinf(value);
	return res;
}

f32 Cos(f32 value)
{
	const f32 res = ::cosf(value);
	return res;
}

f32 Tan(f32 value)
{
	const f32 res = ::tanf(value);
	return res;
}

f32 Sqrt(f32 value)
{
	const f32 res = ::sqrtf(value);
	return res;
}

float3 FromTo(const float3 &a, const float3 &b)
{
	const float3 res = { b.x - a.x, b.y - a.y, b.z - a.z };
	return res;
}

float3 Add(const float3 &a, const float3 &b)
{
	const float3 res = { b.x + a.x, b.y + a.y, b.z + a.z };
	return res;
}

float3 Sub(const float3 &a, const float3 &b)
{
	const float3 res = { a.x - b.x, a.y - b.y, a.z - b.z };
	return res;
}

float3 Mul(const float3 &a, f32 b)
{
	const float3 res = { a.x * b, a.y * b, a.z * b };
	return res;
}

float4 Mul(const float4x4 &a, const float4 &b)
{
#define USE_ROWS_TIMES_COLUMNS_MATRIX_MULTIPLICATION 1
#if USE_ROWS_TIMES_COLUMNS_MATRIX_MULTIPLICATION
	const float4 res = {
		a.m00*b.x + a.m10*b.y + a.m20*b.z + a.m30*b.w,
		a.m01*b.x + a.m11*b.y + a.m21*b.z + a.m31*b.w,
		a.m02*b.x + a.m12*b.y + a.m22*b.z + a.m32*b.w,
		a.m03*b.x + a.m13*b.y + a.m23*b.z + a.m33*b.w,
	};
	return res;
#else
#	error "Missing implementation"
#endif
}

float3 MulVector(const float4x4 &a, const float3 &b)
{
	const float3 res = Mul(a, Float4(b, 0.0)).xyz;
	return res;
}

float3 MulPoint(const float4x4 &a, const float3 &b)
{
	const float3 res = Mul(a, Float4(b, 1.0)).xyz;
	return res;
}

float4x4 Mul(const float4x4 &a, const float4x4 &b)
{
#define USE_ROWS_TIMES_COLUMNS_MATRIX_MULTIPLICATION 1
#if USE_ROWS_TIMES_COLUMNS_MATRIX_MULTIPLICATION
	const float4x4 res = {
		a.m00*b.m00 + a.m10*b.m01 + a.m20*b.m02 + a.m30*b.m03,
		a.m01*b.m00 + a.m11*b.m01 + a.m21*b.m02 + a.m31*b.m03,
		a.m02*b.m00 + a.m12*b.m01 + a.m22*b.m02 + a.m32*b.m03,
		a.m03*b.m00 + a.m13*b.m01 + a.m23*b.m02 + a.m33*b.m03,

		a.m00*b.m10 + a.m10*b.m11 + a.m20*b.m12 + a.m30*b.m13,
		a.m01*b.m10 + a.m11*b.m11 + a.m21*b.m12 + a.m31*b.m13,
		a.m02*b.m10 + a.m12*b.m11 + a.m22*b.m12 + a.m32*b.m13,
		a.m03*b.m10 + a.m13*b.m11 + a.m23*b.m12 + a.m33*b.m13,

		a.m00*b.m20 + a.m10*b.m21 + a.m20*b.m22 + a.m30*b.m23,
		a.m01*b.m20 + a.m11*b.m21 + a.m21*b.m22 + a.m31*b.m23,
		a.m02*b.m20 + a.m12*b.m21 + a.m22*b.m22 + a.m32*b.m23,
		a.m03*b.m20 + a.m13*b.m21 + a.m23*b.m22 + a.m33*b.m23,

		a.m00*b.m30 + a.m10*b.m31 + a.m20*b.m32 + a.m30*b.m33,
		a.m01*b.m30 + a.m11*b.m31 + a.m21*b.m32 + a.m31*b.m33,
		a.m02*b.m30 + a.m12*b.m31 + a.m22*b.m32 + a.m32*b.m33,
		a.m03*b.m30 + a.m13*b.m31 + a.m23*b.m32 + a.m33*b.m33,
	};
#else // columns times rows
	const float4x4 res = {
		a.m00*b.m00 + a.m01*b.m10 + a.m02*b.m20 + a.m03*b.m30,
		a.m00*b.m01 + a.m01*b.m11 + a.m02*b.m21 + a.m03*b.m31,
		a.m00*b.m02 + a.m01*b.m12 + a.m02*b.m22 + a.m03*b.m32,
		a.m00*b.m03 + a.m01*b.m13 + a.m02*b.m23 + a.m03*b.m33,

		a.m10*b.m00 + a.m11*b.m10 + a.m12*b.m20 + a.m13*b.m30,
		a.m10*b.m01 + a.m11*b.m11 + a.m12*b.m21 + a.m13*b.m31,
		a.m10*b.m02 + a.m11*b.m12 + a.m12*b.m22 + a.m13*b.m32,
		a.m10*b.m03 + a.m11*b.m13 + a.m12*b.m23 + a.m13*b.m33,

		a.m20*b.m00 + a.m21*b.m10 + a.m22*b.m20 + a.m23*b.m30,
		a.m20*b.m01 + a.m21*b.m11 + a.m22*b.m21 + a.m23*b.m31,
		a.m20*b.m02 + a.m21*b.m12 + a.m22*b.m22 + a.m23*b.m32,
		a.m20*b.m03 + a.m21*b.m13 + a.m22*b.m23 + a.m23*b.m33,

		a.m30*b.m00 + a.m31*b.m10 + a.m32*b.m20 + a.m33*b.m30,
		a.m30*b.m01 + a.m31*b.m11 + a.m32*b.m21 + a.m33*b.m31,
		a.m30*b.m02 + a.m31*b.m12 + a.m32*b.m22 + a.m33*b.m32,
		a.m30*b.m03 + a.m31*b.m13 + a.m32*b.m23 + a.m33*b.m33,
	};
#endif
	return res;
}

float3 Negate(const float3 &v)
{
	const float3 res = { -v.x, -v.y, -v.z };
	return res;
}

f32 Dot(const float3 &a, const float3 &b)
{
	const f32 res = a.x * b.x + a.y * b.y + a.z * b.z;
	return res;
}

f32 Dot(const float4 &a, const float4 &b)
{
	const f32 res = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	return res;
}

f32 Length2(const float3 &v)
{
	const f32 res = Dot(v, v);
	return res;
}

f32 Length(const float3 &v)
{
	const f32 res = Sqrt(Length2(v));
	return res;
}

bool IsZero(const float3 &v)
{
	return Length2(v) == 0.0f;
}

float3 Normalize(const float3 &v)
{
	const f32 invLen = 1.0f / Length(v);
	const float3 res = {v.x * invLen, v.y * invLen, v.z * invLen};
	return res;
}

float3 NormalizeIfNotZero(const float3 &v)
{
	const float3 res = Length2(v) > 0.0f ? Normalize(v) : v;
	return res;
}

float3 Cross(const float3 &u, const float3 &v)
{
	const float3 res = {
		u.y * v.z - u.z * v.y, // x
		u.z * v.x - u.x * v.z, // y
		u.x * v.y - u.y * v.x  // z
	};
	return res;
}

float4x4 Eye()
{
	float4x4 mat = {};
	mat.m00 = mat.m11 = mat.m22 = mat.m33 = 1.0f;
	return mat;
}

float4x4 Translate(const float3 &t)
{
	const float4x4 mat = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		t.x, t.y, t.z, 1,
	};
	return mat;
}

float4x4 Rotate(const float3 &axis, f32 degrees)
{
	// ASSERT( IsNormalized(axis) );

	const f32 radians = degrees * ToRadians;
	const f32 c = Cos(radians);
	const f32 s = Sin(radians);
	const f32 t = 1.0f - c;

	const f32 m00 = c + axis.x*axis.x*t;
	const f32 m11 = c + axis.y*axis.y*t;
	const f32 m22 = c + axis.z*axis.z*t;

	const f32 tmp1 = axis.x*axis.y*t;
	const f32 tmp2 = axis.z*s;
	const f32 m10 = tmp1 + tmp2;
	const f32 m01 = tmp1 - tmp2;

	const f32 tmp3 = axis.x*axis.z*t;
	const f32 tmp4 = axis.y*s;
	const f32 m20 = tmp3 - tmp4;
	const f32 m02 = tmp3 + tmp4;

	const f32 tmp5 = axis.y*axis.z*t;
	const f32 tmp6 = axis.x*s;
	const f32 m21 = tmp5 + tmp6;
	const f32 m12 = tmp5 - tmp6;

	const float4x4 mat = {
		m00, m01, m02, 0,
		m10, m11, m12, 0,
		m20, m21, m22, 0,
		0,     0,   0, 1,
	};
	return mat;
}

float4x4 Scale(const float3 &s)
{
	const float4x4 mat = {
		s.x, 0, 0, 0,
		0, s.y, 0, 0,
		0, 0, s.y, 0,
		0, 0, 0, 1,
	};
	return mat;
}

float4x4 LookAt(const float3 &vrp, const float3 &obs, const float3 &up)
{
	const float3 forward = Normalize(FromTo(vrp, obs));
	const float3 right = Normalize(Cross(up, forward));
	const float3 newUp = Cross(forward, right);
	const f32 tx = Dot(Negate(right) ,obs);
	const f32 ty = Dot(Negate(newUp) ,obs);
	const f32 tz = Dot(Negate(forward) ,obs);
	const float4x4 mat = {
		right.x, newUp.x, forward.x, 0,
		right.y, newUp.y, forward.y, 0,
		right.z, newUp.z, forward.z, 0,
		tx, ty, tz, 1,
	};
	return mat;
}

//#define USE_DEPTH_ZERO_TO_ONE 0 // For OpenGL
#define USE_DEPTH_ZERO_TO_ONE 1 // For DX12, Vulkan

//#define USE_REVERSE_Z 0
#define USE_REVERSE_Z 1

float4x4 Perspective(float fov, float aspect, float Near, float Far)
{
#if USE_REVERSE_Z
#if USE_DEPTH_ZERO_TO_ONE
	float tmp = Near;
	Near = Far;
	Far = tmp;
#else
#error "Check reverse-z for OpenGL implementations"
#endif
#endif

	const f32 yScale = 1.0f / Tan(fov * ToRadians / 2.0f);
	const f32 xScale = yScale / aspect;
	const f32 NearMinusFar = Near - Far;

	float4x4 mat = {};
	mat.m00 = xScale;
	mat.m11 = yScale;
#if USE_DEPTH_ZERO_TO_ONE
	mat.m22 = Far / NearMinusFar;
	mat.m32 = Far * Near / NearMinusFar;
#else
	mat.m22 = (Far + Near) / NearMinusFar;
	mat.m32 = 2.0f * Far * Near / NearMinusFar;
#endif
	mat.m23 = -1;

	return mat;
}

float4x4 Orthogonal(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f)
{
#if USE_REVERSE_Z
#if USE_DEPTH_ZERO_TO_ONE
	float tmp = n;
	n = f;
	f = tmp;
#else
#error "Check reverse-z for OpenGL implementations"
#endif
#endif

	float4x4 mat = {};
	const f32 RminusL = r - l;
	const f32 TminusB = t - b;
	const f32 FminusN = f - n;

	mat.m00 = 2.0f / RminusL;
	mat.m30 = -(r + l)/RminusL;
	mat.m11 = 2.0f / TminusB;
	mat.m31 = -(t + b)/TminusB;
#if USE_DEPTH_ZERO_TO_ONE
	mat.m22 = -1.0f / FminusN;
	mat.m32 = - n /FminusN;
#else
	mat.m22 = -2.0f / FminusN;
	mat.m32 = -(f + n)/FminusN;
#endif
	mat.m33 = 1.0f;

	return mat;
}

float3x3 Transpose(const float3x3 &m)
{
	const float3x3 res = {
		m.m00, m.m10, m.m20,
		m.m01, m.m11, m.m21,
		m.m02, m.m12, m.m22,
	};
	return res;
}

float4x4 Transpose(const float4x4 &m)
{
	const float4x4 res = {
		m.m00, m.m10, m.m20, m.m30,
		m.m01, m.m11, m.m21, m.m31,
		m.m02, m.m12, m.m22, m.m32,
		m.m03, m.m13, m.m23, m.m33,
	};
	return res;
}

i32 Floor(f32 value)
{
	const f32 res = ::floorf(value);
	return static_cast<i32>(res);
}

f32 Log2(f32 value)
{
	const f32 res = ::log2f(value);
	return res;
}



#define INSTANTIATE_MATH_OPS_FOR_TYPE( Type ) \
Type Min( Type a, Type b ) { return a < b ? a : b; } \
Type Max( Type a, Type b ) { return a > b ? a : b; } \
Type Clamp( Type v, Type min, Type max ) { return Min( Max( v, min ), max ); }

INSTANTIATE_MATH_OPS_FOR_TYPE( float )
INSTANTIATE_MATH_OPS_FOR_TYPE( i32 )
INSTANTIATE_MATH_OPS_FOR_TYPE( u32 )



////////////////////////////////////////////////////////////////////////////////////////////////////
// Alignment functions

bool IsPowerOfTwo(u32 value)
{
	// First value in the below expression is for the case when value is 0
	return value && (!(value & (value - 1)));
}

u32 AlignUp(u32 value, u32 alignment)
{
	ASSERT( IsPowerOfTwo(alignment) );
	const u32 alignedValue =  ((value - 1) & ~(alignment - 1)) + alignment;
	return alignedValue;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Time

struct Clock
{
#if PLATFORM_WINDOWS
	LARGE_INTEGER counter;
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	timespec timeSpec;
#else
#	error "Missing implementation"
#endif
};

#define SECONDS_FROM_NANOSECONDS( ns ) ((float)(ns) / 1000000000.0f)

#if PLATFORM_WINDOWS
internal LONGLONG Win32GetPerformanceCounterFrequency()
{
	LARGE_INTEGER PerfCountFrequencyResult;
	BOOL res = QueryPerformanceFrequency(&PerfCountFrequencyResult);
	ASSERT(res && "QueryPerformanceFrequency() failed!");
	return PerfCountFrequencyResult.QuadPart;
}

// TODO: Investigate... is there any problem in computing this at static init time?
internal LONGLONG gPerformanceCounterFrequency = Win32GetPerformanceCounterFrequency();
#endif

Clock GetClock()
{
	Clock clock;
#if PLATFORM_WINDOWS
	QueryPerformanceCounter(&clock.counter);
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	clock_gettime(CLOCK_MONOTONIC, &clock.timeSpec);
#endif
	return clock;
}

float GetSecondsElapsed(Clock start, Clock end)
{
	f32 elapsedSeconds;
#if PLATFORM_WINDOWS
	ASSERT( start.counter.QuadPart <= end.counter.QuadPart );
	elapsedSeconds = (
			(float)(end.counter.QuadPart - start.counter.QuadPart) /
			(float)gPerformanceCounterFrequency );
	return elapsedSeconds;
#elif PLATFORM_LINUX || PLATFORM_ANDROID
	ASSERT( start.timeSpec.tv_sec < end.timeSpec.tv_sec || (
				start.timeSpec.tv_sec == end.timeSpec.tv_sec &&
				start.timeSpec.tv_nsec <= end.timeSpec.tv_nsec ) );
	if ( start.timeSpec.tv_sec == end.timeSpec.tv_sec )
	{
		elapsedSeconds = SECONDS_FROM_NANOSECONDS( end.timeSpec.tv_nsec - start.timeSpec.tv_nsec );
	}
	else
	{
		elapsedSeconds = end.timeSpec.tv_sec - start.timeSpec.tv_sec - 1;
		elapsedSeconds += 1.0f - SECONDS_FROM_NANOSECONDS( start.timeSpec.tv_nsec );
		elapsedSeconds += SECONDS_FROM_NANOSECONDS( end.timeSpec.tv_nsec );
	}
#endif
	return elapsedSeconds;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Window and input

#if defined(TOOLS_WINDOW)

#if PLATFORM_LINUX
#	define USE_XCB 1
#elif PLATFORM_ANDROID
#	define USE_ANDROID 1
#elif PLATFORM_WINDOWS
#	define USE_WINAPI 1
#endif


#if USE_XCB
#	include <xcb/xcb.h>
#	include <stdlib.h> // free

void XcbReportError( int xcbErrorCode, const char *context )
{
	static const char *xcbErrorMessages[] = {
		"NO_ERROR",
		"XCB_CONN_ERROR",                   // 1
		"XCB_CONN_CLOSED_EXT_NOTSUPPORTED", // 2
		"XCB_CONN_CLOSED_MEM_INSUFFICIENT", // 3
		"XCB_CONN_CLOSED_REQ_LEN_EXCEED",   // 4
		"XCB_CONN_CLOSED_PARSE_ERR",        // 5
		"XCB_CONN_CLOSED_INVALID_SCREEN",   // 6
		"XCB_CONN_CLOSED_FDPASSING_FAILED", // 7
	};
	LOG(Error, "Xcb error (%s): %s\n", context, xcbErrorMessages[xcbErrorCode]);
}

void XcbReportGenericError( xcb_connection_t *conn, xcb_generic_error_t *err, const char *context )
{
	// TODO: Find a better way to report XCB generic errors
	LOG(Error, "Xcb generic error (%s)\n", context);
}

#endif



enum Key
{
	KEY_NULL,
	KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN,
	KEY_ESCAPE,
	KEY_SPACE,
	KEY_RETURN,
	KEY_TAB,
	KEY_CONTROL,
	KEY_SHIFT,
	KEY_ALT,
	KEY_0, KEY_1, KEY_2,
	KEY_3, KEY_4, KEY_5,
	KEY_6, KEY_7, KEY_8, KEY_9,
	KEY_A, KEY_B, KEY_C, KEY_D,
	KEY_E, KEY_F, KEY_G, KEY_H,
	KEY_I, KEY_J, KEY_K, KEY_L,
	KEY_M, KEY_N, KEY_O, KEY_P,
	KEY_Q, KEY_R, KEY_S, KEY_T,
	KEY_U, KEY_V, KEY_W, KEY_X,
	KEY_Y, KEY_Z,
	KEY_COUNT,
};

enum MouseButton
{
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_COUNT,
};

enum KeyState
{
	KEY_STATE_IDLE,
	KEY_STATE_PRESS,
	KEY_STATE_PRESSED,
	KEY_STATE_RELEASE,
};

enum ButtonState
{
	BUTTON_STATE_IDLE,
	BUTTON_STATE_PRESS,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_RELEASE,
};

enum TouchState
{
	TOUCH_STATE_IDLE,
	TOUCH_STATE_PRESS,
	TOUCH_STATE_PRESSED,
	TOUCH_STATE_RELEASE,
};

struct Keyboard
{
	KeyState keys[KEY_COUNT];
};

struct Mouse
{
	u32 x, y;
	i32 dx, dy;
	ButtonState buttons[MOUSE_BUTTON_COUNT];
};

struct Touch
{
	f32 x0, y0;
	f32 x, y;
	f32 dx, dy;
	TouchState state;
};

bool KeyPress(const Keyboard &keyboard, Key key)
{
	ASSERT(key < KEY_COUNT);
	return keyboard.keys[key] == KEY_STATE_PRESS;
}

bool KeyPressed(const Keyboard &keyboard, Key key)
{
	ASSERT(key < KEY_COUNT);
	return keyboard.keys[key] == KEY_STATE_PRESSED;
}

bool KeyRelease(const Keyboard &keyboard, Key key)
{
	ASSERT(key < KEY_COUNT);
	return keyboard.keys[key] == KEY_STATE_RELEASE;
}

bool MouseMoved(const Mouse &mouse)
{
	return mouse.dx != 0 || mouse.dy != 0;
}

bool MouseButtonPress(const Mouse &mouse, MouseButton button)
{
	ASSERT(button < MOUSE_BUTTON_COUNT);
	return mouse.buttons[button] == BUTTON_STATE_PRESS;
}

bool MouseButtonPressed(const Mouse &mouse, MouseButton button)
{
	ASSERT(button < MOUSE_BUTTON_COUNT);
	return mouse.buttons[button] == BUTTON_STATE_PRESSED;
}

bool MouseButtonRelease(const Mouse &mouse, MouseButton button)
{
	ASSERT(button < MOUSE_BUTTON_COUNT);
	return mouse.buttons[button] == BUTTON_STATE_RELEASE;
}

bool MouseButtonChanged(const Mouse &mouse, MouseButton button)
{
	return MouseButtonPress(mouse, button) || MouseButtonRelease(mouse, button);
}

bool MouseChanged(const Mouse &mouse)
{
	return MouseMoved(mouse)
		|| MouseButtonChanged(mouse, MOUSE_BUTTON_LEFT)
		|| MouseButtonChanged(mouse, MOUSE_BUTTON_RIGHT)
		|| MouseButtonChanged(mouse, MOUSE_BUTTON_MIDDLE);
}

enum WindowFlags
{
	WindowFlags_WasCreated  = 1 << 0,
	WindowFlags_WillDestroy = 1 << 1,
	WindowFlags_WasResized  = 1 << 2,
	WindowFlags_Exit        = 1 << 3,
};

struct Window
{
#if USE_XCB
	xcb_connection_t *connection;
	xcb_window_t window;
	xcb_atom_t closeAtom;
#elif USE_ANDROID
	ANativeWindow *nativeWindow;
#elif USE_WINAPI
	HINSTANCE hInstance;
	HWND hWnd;
#endif
	u32 width;
	u32 height;
	u32 flags;

	Keyboard keyboard;
	Mouse mouse;
	Touch touches[2];
};

struct PlatformConfig
{
#if PLATFORM_ANDROID
	struct android_app *androidApp;
#endif // PLATFORM_ANDROID
};

struct Platform
{
	// To be configured by the client app

	u32 globalMemorySize = MB(64);
	u32 frameMemorySize = MB(16);
	u32 stringMemorySize = KB(16);

	bool (*InitCallback)(Platform &);
	void (*UpdateCallback)(Platform &);
	void (*CleanupCallback)(Platform &);
	bool (*WindowInitCallback)(Platform &);
	void (*WindowCleanupCallback)(Platform &);

	void *userData;

#if PLATFORM_ANDROID
	struct android_app *androidApp;
#endif // PLATFORM_ANDROID

	// Platform components

	Arena globalArena;
	Arena frameArena;
	Arena stringArena;
	StringInterning stringInterning;
	Window window;
	f32 deltaSeconds;
};



#if USE_XCB && 0
void PrintModifiers(uint32_t mask)
{
	const char **mod, *mods[] = {
		"Shift", "Lock", "Ctrl", "Alt",
		"Mod2", "Mod3", "Mod4", "Mod5",
		"Button1", "Button2", "Button3", "Button4", "Button5"
	};
	LOG(Info, "Modifier mask: ");
	for (mod = mods ; mask; mask >>= 1, mod++)
		if (mask & 1)
			LOG(Info, *mod);
	putchar ('\n');
}
#endif



#if USE_XCB

#include "xcb_key_mappings.h"

#if USE_IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API bool ImGui_ImplXcb_HandleInputEvent(xcb_generic_event_t *event);
#endif

void XcbWindowProc(Window &window, xcb_generic_event_t *event)
{
#if USE_IMGUI
	if (ImGui_ImplXcb_HandleInputEvent(event))
		return;
#endif

	u32 eventType = event->response_type & ~0x80;

	switch ( eventType )
	{
		case XCB_KEY_PRESS:
		case XCB_KEY_RELEASE:
			{
				// NOTE: xcb_key_release_event_t is an alias of xcb_key_press_event_t
				xcb_key_press_event_t *ev = (xcb_key_press_event_t *)event;
				u32 keyCode = ev->detail;
				ASSERT( keyCode < ARRAY_COUNT(XcbKeyMappings) );
				u32 mapping = XcbKeyMappings[ keyCode ];
				ASSERT( mapping < KEY_COUNT );
				KeyState state = eventType == XCB_KEY_PRESS ? KEY_STATE_PRESS : KEY_STATE_RELEASE;
				window.keyboard.keys[ mapping ] = state;
				break;
			}

		case XCB_BUTTON_PRESS:
		case XCB_BUTTON_RELEASE:
			{
				// NOTE: xcb_button_release_event_t is an alias of xcb_button_press_event_t
				xcb_button_press_event_t *ev = (xcb_button_press_event_t *)event;
				ButtonState state = eventType == XCB_BUTTON_PRESS ? BUTTON_STATE_PRESS : BUTTON_STATE_RELEASE;
				switch (ev->detail) {
					case 1: window.mouse.buttons[ MOUSE_BUTTON_LEFT ] = state; break;
					case 2: window.mouse.buttons[ MOUSE_BUTTON_MIDDLE ] = state; break;
					case 3: window.mouse.buttons[ MOUSE_BUTTON_RIGHT ] = state; break;
					//case 4: // wheel up
					//case 5: // wheel down
					default:;
				}
				break;
			}

		case XCB_MOTION_NOTIFY:
			{
				xcb_motion_notify_event_t *ev = (xcb_motion_notify_event_t *)event;
				window.mouse.dx = static_cast<i32>(ev->event_x) - static_cast<i32>(window.mouse.x);
				window.mouse.dy = static_cast<i32>(ev->event_y) - static_cast<i32>(window.mouse.y);
				window.mouse.x = ev->event_x;
				window.mouse.y = ev->event_y;
				break;
			}

		case XCB_ENTER_NOTIFY:
			{
				xcb_enter_notify_event_t *ev = (xcb_enter_notify_event_t *)event;
				//LOG(Info, "Mouse entered window %ld, at coordinates (%d,%d)\n",
				//		ev->event, ev->event_x, ev->event_y);
				break;
			}

		case XCB_LEAVE_NOTIFY:
			{
				xcb_leave_notify_event_t *ev = (xcb_leave_notify_event_t *)event;
				//LOG(Info, "Mouse left window %ld, at coordinates (%d,%d)\n",
				//		ev->event, ev->event_x, ev->event_y);
				break;
			}

		case XCB_CONFIGURE_NOTIFY:
			{
				const xcb_configure_notify_event_t *ev = (const xcb_configure_notify_event_t *)event;
				if ( window.width != ev->width || window.height != ev->height )
				{
					window.width = ev->width;
					window.height = ev->height;
					window.flags |= WindowFlags_WasResized;
				}
				break;
			}

		case XCB_CLIENT_MESSAGE:
			{
				const xcb_client_message_event_t *ev = (const xcb_client_message_event_t *)event;
				if ( ev->data.data32[0] == window.closeAtom )
				{
					window.flags |= WindowFlags_WillDestroy;
					window.flags |= WindowFlags_Exit;
				}
				break;
			}

		default:
			/* Unknown event type, ignore it */
			LOG(Info, "Unknown window event: %d\n", event->response_type);
			break;
	}

#if USE_IMGUI
	const Mouse &mouse = window.mouse;

	ImGuiIO& io = ImGui::GetIO();
	if (MouseChanged(mouse))
	{
		if (MouseMoved(mouse))
		{
			io.AddMousePosEvent(mouse.x, mouse.y);
		}
		int button = -1;
		bool press = true;
		if (MouseButtonPress(mouse, MOUSE_BUTTON_LEFT)) { button = 0; press = true; }
		if (MouseButtonPress(mouse, MOUSE_BUTTON_RIGHT)) { button = 1; press = true; }
		if (MouseButtonPress(mouse, MOUSE_BUTTON_MIDDLE)) { button = 2; press = true; }
		if (MouseButtonRelease(mouse, MOUSE_BUTTON_LEFT)) { button = 0; press = false; }
		if (MouseButtonRelease(mouse, MOUSE_BUTTON_RIGHT)) { button = 1; press = false; }
		if (MouseButtonRelease(mouse, MOUSE_BUTTON_MIDDLE)) { button = 2; press = false; }
		if ( button != -1 )
		{
			io.AddMouseButtonEvent(button, press);
			io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
		}
	}

	// TODO: When we detect the mouse leves the window
	// io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);

	// Clear mouse/keyboard events if handled by ImGui
	if ( io.WantCaptureMouse ) window.mouse = {};
	if ( io.WantCaptureKeyboard ) window.keyboard = {};
#endif // USE_IMGUI
}

#elif USE_ANDROID

/**
 * Process the next main command.
 * enum NativeAppGlueAppCmd {
 *   UNUSED_APP_CMD_INPUT_CHANGED = 0
 *   APP_CMD_INIT_WINDOW = 1
 *   APP_CMD_TERM_WINDOW = 2
 *   APP_CMD_WINDOW_RESIZED = 3
 *   APP_CMD_WINDOW_REDRAW_NEEDED = 4
 *   APP_CMD_CONTENT_RECT_CHANGED = 5
 *   APP_CMD_GAINED_FOCUS = 6
 *   APP_CMD_LOST_FOCUS = 7
 *   APP_CMD_CONFIG_CHANGED = 8
 *   APP_CMD_LOW_MEMORY = 9
 *   APP_CMD_START = 10
 *   APP_CMD_RESUME = 11
 *   APP_CMD_SAVE_STATE = 12
 *   APP_CMD_PAUSE = 13
 *   APP_CMD_STOP = 14
 *   APP_CMD_DESTROY = 15
 *   APP_CMD_WINDOW_INSETS_CHANGED = 16
 * }
 */
void AndroidHandleAppCommand(struct android_app *app, int32_t cmd)
{
	Platform *platform = (Platform*)app->userData;

	switch (cmd)
	{
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			ASSERT(app->window != NULL);
			if (app->window && app->window != platform->window.nativeWindow)
			{
				platform->window.nativeWindow = app->window;
				platform->window.flags |= WindowFlags_WasCreated;
			}
			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			platform->window =  {};
			platform->window.flags |= WindowFlags_WillDestroy;
			break;
		case APP_CMD_WINDOW_RESIZED:
			{
				int32_t newWidth = ANativeWindow_getWidth(app->window);
				int32_t newHeight = ANativeWindow_getHeight(app->window);
				if ( newWidth != platform->window.width || newHeight != platform->window.height )
				{
					platform->window.width = newWidth;
					platform->window.height = newHeight;
					platform->window.flags |= WindowFlags_WasResized;
				}
			}
			break;
		//case APP_CMD_WINDOW_REDRAW_NEEDED: break;
		//case APP_CMD_CONTENT_RECT_CHANGED: break;
		//case APP_CMD_GAINED_FOCUS: break;
		//case APP_CMD_LOST_FOCUS: break;
		//case APP_CMD_CONFIG_CHANGED: break;
		//case APP_CMD_LOW_MEMORY: break;
		//case APP_CMD_START: break;
		//case APP_CMD_RESUME: break;
		//case APP_CMD_SAVE_STATE: break;
		case APP_CMD_PAUSE:
			{
				// Gets activated at APP_CMD_INIT_WINDOW
			}
			break;
		//case APP_CMD_STOP: break;
		//case APP_CMD_DESTROY: break;
		//case APP_CMD_WINDOW_INSETS_CHANGED: break;
		default:
			//LOG( Info, "UNKNOWN ANDROID COMMAND: %d\n", cmd);
			break;
	}
	//LOG( Info, "ANDROID APP COMMAND: %d\n", cmd);
}

int32_t AndroidHandleInputEvent(struct android_app *app, AInputEvent *event)
{
	Platform *platform = (Platform*)app->userData;

	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		const int32_t actionAndPointer = AMotionEvent_getAction( event );
		const uint32_t action = actionAndPointer & AMOTION_EVENT_ACTION_MASK;
		const uint32_t pointerIndex = (actionAndPointer & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		const uint32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);
		const uint32_t pointerCount = AMotionEvent_getPointerCount(event);
		const float x = AMotionEvent_getX(event, pointerIndex);
		const float y = AMotionEvent_getY(event, pointerIndex);

		if (pointerId < ARRAY_COUNT(platform->window.touches))
		{
			Touch *touches = platform->window.touches;

			switch( action )
			{
				case AMOTION_EVENT_ACTION_DOWN:
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
					{
						touches[pointerId].state = TOUCH_STATE_PRESS;
						touches[pointerId].x0 = x;
						touches[pointerId].y0 = y;
						touches[pointerId].x = x;
						touches[pointerId].y = y;
					}
					break;
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_POINTER_UP:
					{
						touches[pointerId].state = TOUCH_STATE_RELEASE;
						touches[pointerId].x = x;
						touches[pointerId].y = y;
					}
					break;
				case AMOTION_EVENT_ACTION_MOVE:
					// On move ements, we are meant to handle all pointers in the gesture
					for (u32 pointerIndex = 0; pointerIndex < pointerCount; ++pointerIndex)
					{
						const float x = AMotionEvent_getX(event, pointerIndex);
						const float y = AMotionEvent_getY(event, pointerIndex);
						const uint32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);
						touches[pointerId].dx = x - touches[pointerId].x;
						touches[pointerId].dy = y - touches[pointerId].y;
						touches[pointerId].x = x;
						touches[pointerId].y = y;
					}
					break;
			}
		}
		return 1;
	}
	return 0;
}

#elif USE_WINAPI

#include "win32_key_mappings.h"

#if USE_IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

LRESULT CALLBACK Win32WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool processMouseEvents = true;
	bool processKeyboardEvents = true;
#if USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}
	if (ImGui::GetCurrentContext() != nullptr)
	{
		const ImGuiIO& io = ImGui::GetIO();
		processKeyboardEvents = !io.WantCaptureKeyboard;
		processMouseEvents = !io.WantCaptureMouse;
	}
#endif

	Window *window = (Window*)GetPropA(hWnd, "WindowPtr");


	switch (uMsg)
	{
		case WM_KEYDOWN:
		case WM_KEYUP:
			if ( processKeyboardEvents )
			{
				ASSERT(window);
				WPARAM keyCode = wParam;
				ASSERT( keyCode < ARRAY_COUNT(Win32KeyMappings) );
				u32 mapping = Win32KeyMappings[ keyCode ];
				ASSERT( mapping < KEY_COUNT );
				KeyState state = uMsg == WM_KEYDOWN ? KEY_STATE_PRESS : KEY_STATE_RELEASE;
				window->keyboard.keys[ mapping ] = state;
			}
			break;

		case WM_SYSCHAR:
			// If this message is not handled the default window procedure will
			// play a system notification sound when Alt+Enter is pressed.
			break;

		case WM_LBUTTONDOWN:
			if ( processMouseEvents )
			{
				//int xPos = GET_X_LPARAM(lParam);
				//int yPos = GET_Y_LPARAM(lParam);
				ASSERT(window);
				window->mouse.buttons[MOUSE_BUTTON_LEFT] = BUTTON_STATE_PRESS;
			}
			break;
		case WM_LBUTTONUP:
			if ( processMouseEvents )
			{
				ASSERT(window);
				window->mouse.buttons[MOUSE_BUTTON_LEFT] = BUTTON_STATE_RELEASE;
			}
			break;
		case WM_RBUTTONDOWN:
			if ( processMouseEvents )
			{
				ASSERT(window);
				window->mouse.buttons[MOUSE_BUTTON_RIGHT] = BUTTON_STATE_PRESS;
			}
			break;
		case WM_RBUTTONUP:
			if ( processMouseEvents )
			{
				ASSERT(window);
				window->mouse.buttons[MOUSE_BUTTON_RIGHT] = BUTTON_STATE_RELEASE;
			}
			break;
		case WM_MBUTTONDOWN:
			if ( processMouseEvents )
			{
				ASSERT(window);
				window->mouse.buttons[MOUSE_BUTTON_MIDDLE] = BUTTON_STATE_PRESS;
			}
			break;
		case WM_MBUTTONUP:
			if ( processMouseEvents )
			{
				ASSERT(window);
				window->mouse.buttons[MOUSE_BUTTON_MIDDLE] = BUTTON_STATE_RELEASE;
			}
			break;

		case WM_MOUSEMOVE:
			if ( processMouseEvents )
			{
				ASSERT(window);
				i32 xPos = GET_X_LPARAM(lParam);
				i32 yPos = GET_Y_LPARAM(lParam);
				window->mouse.dx = xPos - window->mouse.x;
				window->mouse.dy = yPos - window->mouse.y;
				window->mouse.x = xPos;
				window->mouse.y = yPos;
				//LOG( Info, "Mouse at position (%d, %d)\n", xPos, yPos );
			}
			break;

		case WM_MOUSEHOVER:
		case WM_MOUSELEAVE:
			{
				// These events are disabled by default. See documentation if needed:
				// https://learn.microsoft.com/en-us/windows/win32/learnwin32/other-mouse-operations
				//LOG( Info, "Mouse %s the window\n", uMsg == WM_MOUSEHOVER ? "entered" : "left" );
				break;
			}

		case WM_SIZE:
			{
				ASSERT(window);
				i32 width = LOWORD(lParam);
				i32 height = HIWORD(lParam);
				if ( window->width != width || window->height != height )
				{
					window->width = Max(width, 0);
					window->height = Max(height, 0);
					window->flags |= WindowFlags_WasResized;
				}
				break;
			}

		case WM_CLOSE:
			{
				// If we want to show a dialog to ask the user for confirmation before
				// closing the window, it should be done here. Zero should be returned
				// to indicate that we handled this message.
				// Otherwise, calling DefWindowProc will internally call DestroyWindow
				// and will internally send the WM_DESTROY message.
				DestroyWindow(hWnd);
				break;
			}

		case WM_DESTROY:
			{
				// This inserts a WM_QUIT message in the queue, which will in turn cause
				// GetMessage to return zero. We will exit the main loop when that happens.
				// On the other hand, PeekMessage has to handle WM_QUIT messages explicitly.
				window->flags |= WindowFlags_WillDestroy;
				PostQuitMessage(0);
				break;
			}

		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

#endif



bool InitializeWindow(
		Window &window,
		u32 width = 640,
		u32 height = 480,
		const char *title = "Example window"
		)
{
	ZeroStruct(&window);
	window.width = width;
	window.height = height;

#if USE_XCB

	// Connect to the X server
	xcb_connection_t *xcbConnection = xcb_connect(NULL, NULL);

	int xcbConnError = xcb_connection_has_error(xcbConnection);
	if ( xcbConnError > 0 )
	{
		XcbReportError(xcbConnError, "xcb_connect");
		xcb_disconnect(xcbConnection);
		return false;
	}

	// Get the first screen
	const xcb_setup_t *setup = xcb_get_setup(xcbConnection);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
	xcb_screen_t * screen = iter.data;

	// Configure events to capture
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2] = {
		screen->black_pixel,
		XCB_EVENT_MASK_KEY_PRESS       | XCB_EVENT_MASK_KEY_RELEASE    |
		XCB_EVENT_MASK_BUTTON_PRESS    | XCB_EVENT_MASK_BUTTON_RELEASE |
		XCB_EVENT_MASK_POINTER_MOTION  |
		XCB_EVENT_MASK_ENTER_WINDOW    | XCB_EVENT_MASK_LEAVE_WINDOW   |
		XCB_EVENT_MASK_STRUCTURE_NOTIFY
	};

	// Create a window
	xcb_window_t xcbWindow = xcb_generate_id(xcbConnection);
	xcb_void_cookie_t createWindowCookie = xcb_create_window_checked(
		xcbConnection,                 // xcb connection
		XCB_COPY_FROM_PARENT,          // depth
		xcbWindow,                     // window id
		screen->root,                  // parent window
		0, 0,                          // x, y
		width, height,                 // width, height
		0,                             // bnorder_width
		XCB_WINDOW_CLASS_INPUT_OUTPUT, // class
		screen->root_visual,           // visual
		mask, values);                 // value_mask, value_list

	xcb_generic_error_t *createWindowError = xcb_request_check(xcbConnection, createWindowCookie);
	if ( createWindowError )
	{
		XcbReportGenericError(xcbConnection, createWindowError, "xcb_create_window_checked");
		xcb_destroy_window(xcbConnection, xcbWindow);
		xcb_disconnect(xcbConnection);
		return false;
	}

	// Handle close event
	// TODO: Handle xcb_intern_atom errors
	xcb_intern_atom_cookie_t protocolCookie = // handle error
		xcb_intern_atom_unchecked( xcbConnection, 1, 12, "WM_PROTOCOLS");
	xcb_intern_atom_reply_t *protocolReply =
		xcb_intern_atom_reply( xcbConnection, protocolCookie, 0);
	xcb_intern_atom_cookie_t closeCookie = // handle error
		xcb_intern_atom_unchecked( xcbConnection, 0, 16, "WM_DELETE_WINDOW");
	xcb_intern_atom_reply_t *closeReply =
		xcb_intern_atom_reply( xcbConnection, closeCookie, 0);
	u8 dataFormat = 32;
	u32 dataLength = 1;
	void *data = &closeReply->atom;
	xcb_change_property( // handle error
			xcbConnection,
			XCB_PROP_MODE_REPLACE,
			xcbWindow,
			protocolReply->atom, XCB_ATOM_ATOM,
			dataFormat, dataLength, data);
	xcb_atom_t closeAtom = closeReply->atom;
	free(protocolReply);
	free(closeReply);

	// Map the window to the screen
	xcb_void_cookie_t mapWindowCookie = xcb_map_window_checked(xcbConnection, xcbWindow);
	xcb_generic_error_t *mapWindowError = xcb_request_check(xcbConnection, mapWindowCookie);
	if ( mapWindowError )
	{
		XcbReportGenericError(xcbConnection, mapWindowError, "xcb_map_window_checked");
		xcb_destroy_window(xcbConnection, xcbWindow);
		xcb_disconnect(xcbConnection);
		return false;
	}

	// Flush the commands before continuing
	xcb_flush(xcbConnection);

	// Get window info at this point
	xcb_get_geometry_cookie_t cookie= xcb_get_geometry( xcbConnection, xcbWindow ); // handle error
	xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply( xcbConnection, cookie, NULL ); // handle error

	window.connection = xcbConnection;
	window.window = xcbWindow;
	window.closeAtom = closeAtom;
	window.width = reply->width;
	window.height = reply->height;

	window.flags = WindowFlags_WasCreated;

#endif

#if USE_WINAPI

	// Register the window class.
	const char CLASS_NAME[]  = "Sample Window Class";
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASS wc = {};
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = Win32WindowProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	ATOM classAtom = RegisterClassA(&wc);

	if (classAtom == 0)
	{
		Win32ReportError();
		return false;
	}

	// Given the desired client window size, get the full size
	RECT windowRect = { 0, 0, (int)width, (int)height };
	AdjustWindowRect( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );
	int fullWidth = windowRect.right - windowRect.left;
	int fullHeight = windowRect.bottom - windowRect.top;

	HWND hWnd = CreateWindowExA(
			0,                              // Optional window styles.
			CLASS_NAME,                     // Window class
			title,                          // Window text
			WS_OVERLAPPEDWINDOW,            // Window style
			CW_USEDEFAULT, CW_USEDEFAULT,   // Position
			fullWidth, fullHeight,          // Size
			NULL,                           // Parent window
			NULL,                           // Menu
			hInstance,                      // Instance handle
			NULL                            // Additional application data
			);

	if (hWnd == NULL)
	{
		Win32ReportError();
		return false;
	}

	if ( !SetPropA(hWnd, "WindowPtr", &window) )
	{
		Win32ReportError();
		return false;
	}

	ShowWindow(hWnd, SW_SHOW);

	window.hInstance = hInstance;
	window.hWnd = hWnd;

	window.flags = WindowFlags_WasCreated;

#endif

	return true;
}


void CleanupWindow(Window &window)
{
#if USE_XCB
	xcb_destroy_window(window.connection, window.window);
	xcb_disconnect(window.connection);
#elif USE_WINAPI
	DestroyWindow(window.hWnd);
#endif
}


void PlatformUpdateEventLoop(Platform &platform)
{
	Window &window = platform.window;

	// Transition key states
	for ( u32 i = 0; i < KEY_COUNT; ++i ) {
		if ( window.keyboard.keys[i] == KEY_STATE_PRESS ) {
			window.keyboard.keys[i] = KEY_STATE_PRESSED;
		} else if ( window.keyboard.keys[i] == KEY_STATE_RELEASE ) {
			window.keyboard.keys[i] = KEY_STATE_IDLE;
		}
	}

	// Transition mouse button states
	for ( u32 i = 0; i < MOUSE_BUTTON_COUNT; ++i ) {
		if ( window.mouse.buttons[i] == BUTTON_STATE_PRESS ) {
			window.mouse.buttons[i] = BUTTON_STATE_PRESSED;
		} else if ( window.mouse.buttons[i] == BUTTON_STATE_RELEASE ) {
			window.mouse.buttons[i] = BUTTON_STATE_IDLE;
		}
	}

	window.mouse.dx = 0.0f;
	window.mouse.dy = 0.0f;

	// Transition touch states
	for ( u32 i = 0; i < ARRAY_COUNT(window.touches); ++i ) {
		if ( window.touches[i].state == TOUCH_STATE_PRESS ) {
			window.touches[i].state = TOUCH_STATE_PRESSED;
		} else if ( window.touches[i].state == TOUCH_STATE_RELEASE ) {
			window.touches[i].state = TOUCH_STATE_IDLE;
		}
		window.touches[i].dx = 0.0f;
		window.touches[i].dy = 0.0f;
	}

#if USE_XCB

	xcb_generic_event_t *event;
	while ( (event = xcb_poll_for_event(window.connection)) != 0 )
	{
		XcbWindowProc(window, event);
		free(event);
	}

#elif USE_ANDROID

	// Read all pending events.
	int ident;
	int events;
	struct android_poll_source* source;

	const int kWaitForever = -1;
	const int kDontWait = 0;
	while ((ident=ALooper_pollAll(kDontWait, NULL, &events, (void**)&source)) >= 0)
	{
		// Process this event.
		if (source != NULL)
		{
			source->process(platform.androidApp, source);
		}

		// Check if we are exiting.
		if (platform.androidApp->destroyRequested != 0)
		{
			LOG(Info, "androidApp->destroyRequesteds\n");
			window.flags |= WindowFlags_Exit;
		}
	}

#elif USE_WINAPI

	MSG msg = { };
	while ( PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		if ( LOWORD( msg.message ) == WM_QUIT )
		{
			window.flags |= WindowFlags_Exit;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#endif
}

bool PlatformRun(Platform &platform)
{
	ASSERT( platform.globalMemorySize > 0 );
	ASSERT( platform.frameMemorySize > 0 );
	ASSERT( platform.InitCallback );
	ASSERT( platform.UpdateCallback );
	ASSERT( platform.CleanupCallback );
	ASSERT( platform.WindowInitCallback );
	ASSERT( platform.WindowCleanupCallback );

	if ( !InitializeWindow(platform.window) )
	{
		return false;
	}

	byte *baseMemory = (byte*)AllocateVirtualMemory(platform.globalMemorySize);
	platform.globalArena = MakeArena(baseMemory, platform.globalMemorySize);

	byte *frameMemory = (byte*)AllocateVirtualMemory(platform.frameMemorySize);
	platform.frameArena = MakeArena(frameMemory, platform.frameMemorySize);

	byte *stringMemory = (byte*)AllocateVirtualMemory(platform.stringMemorySize);
	platform.stringArena = MakeArena(stringMemory, platform.stringMemorySize);
	platform.stringInterning = StringInterningCreate(&platform.stringArena);

#if PLATFORM_ANDROID
	ASSERT( platform.androidApp );
	platform.androidApp = platform.androidApp;
	platform.androidApp->onAppCmd = AndroidHandleAppCommand;
	platform.androidApp->onInputEvent = AndroidHandleInputEvent;
	platform.androidApp->userData = &platform;
#endif // PLATFORM_ANDROID

	if ( !platform.InitCallback(platform) )
	{
#if PLATFORM_ANDROID
		ANativeActivity_finish(platform.androidApp->activity);
#endif // PLATFORM_ANDROID
		return false;
	}

	Clock lastFrameClock = GetClock();

	bool windowInitialized = false;

	while ( 1 )
	{
		ResetArena(platform.frameArena);

		const Clock currentFrameClock = GetClock();
		platform.deltaSeconds = GetSecondsElapsed(lastFrameClock, currentFrameClock);
		lastFrameClock = currentFrameClock;

		PlatformUpdateEventLoop(platform);

		if ( platform.window.flags & WindowFlags_Exit )
		{
			break;
		}
		if ( platform.window.flags & WindowFlags_WasCreated )
		{
			platform.WindowInitCallback(platform);
			windowInitialized = true;
		}
		if ( platform.window.flags & WindowFlags_WillDestroy )
		{
			platform.WindowCleanupCallback(platform);
			CleanupWindow(platform.window);
			windowInitialized = false;
		}

		if ( windowInitialized )
		{
			platform.UpdateCallback(platform);
		}

		platform.window.flags = 0;
	}

	platform.CleanupCallback(platform);

	PrintArenaUsage(platform.globalArena);

	return false;
}

#endif // #if defined(TOOLS_WINDOW)

#endif // #ifndef TOOLS_H

