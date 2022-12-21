#include <assert.h> // assert
#include <stdlib.h> // atof
#include <stdio.h>  // printf, FILE, etc



////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform definitions

#if _WIN32
#	define PLATFORM_WINDOWS 1
#elif __linux__
#	define PLATFORM_LINUX 1
#elif __APPLE__
#	define PLATFORM_APPLE 1
#else
#	error "Unsupported platform"
#endif

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// Base types and definitions

typedef char i8;
typedef short int i16;
typedef int i32;
typedef int i32;
typedef long long int i64;
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef float f32;
typedef double f64;
typedef	unsigned char byte;

#define internal static

#define KB(x) (1024ul * x)
#define MB(x) (1024ul * KB(x))
#define GB(x) (1024ul * MB(x))
#define TB(x) (1024ul * GB(x))

#define ASSERT(expression) assert(expression)
#define INVALID_CODE_PATH() ASSERT(0 && "Invalid code path")
#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))
#define LOG(channel, fmt, ...) printf(fmt, ##__VA_ARGS__)



////////////////////////////////////////////////////////////////////////////////////////////////////
// Strings

struct String
{
	const char* str;
	u32 size;
};

String MakeString(const char *str, u32 size)
{
	String string = { str, size };
	return string;
}

u32 StrLen(char *str)
{
	u32 len = 0;
	while (*str++) ++len;
	return len;
}

void StrCopy(char *dst, const String& src_string)
{
	u32 size = src_string.size;
	const char *src = src_string.str;
	while (size-- > 0) *dst++ = *src++;
	dst[src_string.size] = '\0';
}

void StrCopy(char *dst, const char *src)
{
	while (*src) *dst++ = *src++;
	*dst = 0;
}

void StrCat(char *dst, const char *src)
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

f32 StrToFloat(const String &s)
{
	char buf[256] = {};
	ASSERT(s.size + 1 < ARRAY_COUNT(buf));
	StrCopy(buf, s);
	f32 number = atof(buf);
	return number;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory

#if PLATFORM_LINUX || PLATFORM_APPLE

#include<sys/mman.h>
#include<sys/stat.h>

void* AllocateVirtualMemory(u32 size)
{
	void* baseAddress = 0;
	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_PRIVATE | MAP_ANONYMOUS;
	int fd = -1;
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

void MemSet(void *ptr, u32 size)
{
	byte *bytePtr = (byte*)ptr;
	while (size-- > 0) *bytePtr++ = 0;
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

void ResetArena(Arena &arena)
{
	// This tells the OS we don't need these pages
	//madvise(arena.base, arena.size, MADV_DONTNEED);
	arena.used = 0;
}

void PrintArenaUsage(Arena &arena)
{
	printf("Memory Arena Usage:\n");
	printf("- size: %u B / %u kB\n", arena.size, arena.size/1024);
	printf("- used: %u B / %u kB\n", arena.used, arena.used/1024);
}

#define ZeroStruct( pointer ) MemSet(pointer, sizeof(*pointer) )
#define PushStruct( arena, struct_type ) (struct_type*)PushSize(arena, sizeof(struct_type))
#define PushArray( arena, type, count ) (type*)PushSize(arena, sizeof(type) * count)



////////////////////////////////////////////////////////////////////////////////////////////////////
// Files

struct FileOnMemory
{
	byte *data;
	u32 size;
};

u32 GetFileSize(const char *filename)
{
	u32 size = 0;
	FILE *f = fopen(filename, "rb");
	if ( f )
	{
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fclose(f);
	}
	return size;
}

u32 ReadEntireFile(const char *filename, void *bytes, u32 bytesSize)
{
	u32 bytesRead = 0;
	FILE *f = fopen(filename, "rb");
	if ( f )
	{
		bytesRead = fread(bytes, sizeof(unsigned char), bytesSize, f);
		ASSERT(bytesRead <= bytesSize);
		((byte*)bytes)[bytesRead] = 0; // TODO: Revisit this, it's dangerous to write beyond bytesSize
		fclose(f);
	}
	return bytesRead;
}

FileOnMemory *PushFile( Arena& arena, const char *filename )
{
	FileOnMemory *file = 0;

	u32 size = GetFileSize( filename );
	if ( size > 0 )
	{
		Arena backupArena = arena;

		byte *data = PushArray( arena, byte, size + 1 ); // +1 for final zero put by ReadEntireFile
		u32 bytesRead = ReadEntireFile( filename, data, size );
		if ( bytesRead == size )
		{
			file = PushStruct( arena, FileOnMemory );
			file->data = data;
			file->size = size;
		}
		else
		{
			arena = backupArena;
		}
	}

	return file;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Math

inline u32 Min( u32 a, u32 b ) { return a < b ? a : b; }
inline u32 Max( u32 a, u32 b ) { return a > b ? a : b; }
inline u32 Clamp( u32 v, u32 min, u32 max ) { return Min( Max( v, min ), max ); }

