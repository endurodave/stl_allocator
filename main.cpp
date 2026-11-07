#include <new>
#include <iostream>
#include <cassert>
#include <cstdlib>
#if WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include "xlist.h"
#include "xmap.h"
#include "xqueue.h"
#include "xset.h"
#include "xsstream.h"
#include "xstring.h"
#include "xallocator.h"

#define ASSERT() assert(false)

// @see https://github.com/endurodave/stl_allocator
// David Lafreniere

// On VisualStudio, to disable the debug heap for faster performance when using
// the debugger use this option:
// Debugging > Environment _NO_DEBUG_HEAP=1

using namespace std;

static int MAX_BENCHMARK = 10000;

typedef void (*TestFunc)();
void ListGlobalHeapTest();
void MapGlobalHeapTest();
void StringGlobalHeapTest();
void ListFixedBlockTest();
void MapFixedBlockTest();
void StringFixedBlockTest();
void Benchmark(const char* name, TestFunc testFunc);

static void out_of_memory()
{
	// new-handler called when memory runs out. Allocate() retries after it
	// returns, as operator new does, so it must free memory, throw or abort.
	std::cerr << "out of memory" << std::endl;
	std::abort();
}

int AllocatorTests();

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(void)
{
	std::set_new_handler(out_of_memory);

	xlist<int> myList;
	myList.push_back(123);
	
	xmap<char, int> myMap;
	myMap['a'] = 10;

	xqueue<int> myQueue;
	myQueue.push(123);

	xset<xstring> mySet;
	mySet.insert("hello");
	mySet.insert("world");

	xstringstream myStringStream;
	myStringStream << "hello world " << 2016 << ends;

	xwstringstream myWStringStream;
	myWStringStream << L"hello world " << 2016 << ends;

	xstring myString("hello world");

	Benchmark("std::list Global Heap (Run 1)", ListGlobalHeapTest);
	Benchmark("std::list Global Heap (Run 2)", ListGlobalHeapTest);
	Benchmark("std::list Global Heap (Run 3)", ListGlobalHeapTest);

	Benchmark("xlist Fixed Block (Run 1)", ListFixedBlockTest);
	Benchmark("xlist Fixed Block (Run 2)", ListFixedBlockTest);
	Benchmark("xlist Fixed Block (Run 3)", ListFixedBlockTest);

	Benchmark("std::map Global Heap (Run 1)", MapGlobalHeapTest);
	Benchmark("std::map Global Heap (Run 2)", MapGlobalHeapTest);
	Benchmark("std::map Global Heap (Run 3)", MapGlobalHeapTest);

	Benchmark("xmap Fixed Block (Run 1)", MapFixedBlockTest);
	Benchmark("xmap Fixed Block (Run 2)", MapFixedBlockTest);
	Benchmark("xmap Fixed Block (Run 3)", MapFixedBlockTest);

	Benchmark("std::string Global Heap (Run 1)", StringGlobalHeapTest);
	Benchmark("std::string Global Heap (Run 2)", StringGlobalHeapTest);
	Benchmark("std::string Global Heap (Run 3)", StringGlobalHeapTest);

	Benchmark("xstring Fixed Block (Run 1)", StringFixedBlockTest);
	Benchmark("xstring Fixed Block (Run 2)", StringFixedBlockTest);
	Benchmark("xstring Fixed Block (Run 3)", StringFixedBlockTest);

	xalloc_stats();

	// Unit tests (tests/AllocatorTests.cpp), last: one fills the xallocator
	// allocator table. Non-zero exit on failure.
	return AllocatorTests() == 0 ? 0 : 1;
}

//------------------------------------------------------------------------------
// MapGlobalHeapTest
//------------------------------------------------------------------------------
void MapGlobalHeapTest()
{
	map<int, char> myMap;
	for (int i=0; i<MAX_BENCHMARK; i++)
		myMap[i] = 'a';
	myMap.clear();
}

//------------------------------------------------------------------------------
// MapFixedBlockTest
//------------------------------------------------------------------------------
void MapFixedBlockTest()
{
	xmap<int, char> myMap;
	for (int i=0; i<MAX_BENCHMARK; i++)
		myMap[i] = 'a';
	myMap.clear();
}

//------------------------------------------------------------------------------
// ListGlobalHeapTest
//------------------------------------------------------------------------------
void ListGlobalHeapTest()
{
	list<int> myList;
	for (int i=0; i<MAX_BENCHMARK; i++)
		myList.push_back(123);
	myList.clear();
}

//------------------------------------------------------------------------------
// ListFixedBlockTest
//------------------------------------------------------------------------------
void ListFixedBlockTest()
{
	xlist<int> myList;
	for (int i=0; i<MAX_BENCHMARK; i++)
		myList.push_back(123);
	myList.clear();
}

//------------------------------------------------------------------------------
// StringGlobalHeapTest
//------------------------------------------------------------------------------
void StringGlobalHeapTest()
{
	list<string> myList;
	for (int i=0; i<MAX_BENCHMARK; i++)
	{
		string myString("benchmark");
		myString += "benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test "
			"benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test";
		myList.push_back(myString);
	}
	myList.clear();
}

//------------------------------------------------------------------------------
// StringFixedBlockTest
//------------------------------------------------------------------------------
void StringFixedBlockTest()
{
	xlist<xstring> myList;
	for (int i=0; i<MAX_BENCHMARK; i++)
	{
		xstring myString("benchmark");
		myString += "benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test "
			"benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test benchmark test";
		myList.push_back(myString);
	}
	myList.clear();
}

//------------------------------------------------------------------------------
// Benchmark
//------------------------------------------------------------------------------
void Benchmark(const char* name, TestFunc testFunc)
{
#if WIN32
	LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds = {0};
	LARGE_INTEGER Frequency;

	SetProcessPriorityBoost(GetCurrentProcess(), true);

	QueryPerformanceFrequency(&Frequency); 
	QueryPerformanceCounter(&StartingTime);

	// Call test function
	testFunc();

	QueryPerformanceCounter(&EndingTime);

	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	std::cout << name << " Elapsed time: " << ElapsedMicroseconds.QuadPart << std::endl;

	SetProcessPriorityBoost(GetCurrentProcess(), false);
#endif
}


