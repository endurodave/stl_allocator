# stl_allocator
A custom STL `std::allocator` replacement to improve performance and prevent heap fragmentation.

## Introduction
`stl_allocator` is a fast, fixed-block memory allocator designed as a replacement for the standard C++ `std::allocator`. It is particularly suited for resource-constrained or mission-critical systems where heap fragmentation must be avoided and deterministic execution time is required. By providing memory from pre-allocated pools or recycled heap blocks, it offers a robust alternative to the global heap.

## Key Features
- **Deterministic Performance:** Fixed-block allocation offers consistent execution times compared to the global heap.
- **Fragmentation Protection:** Prevents faults caused by heap fragmentation in long-running applications.
- **Drop-in "x" Containers:** Includes pre-defined versions of common STL containers (e.g., `xlist`, `xmap`, `xset`, `xstring`, `xstringstream`) that use the fixed-block allocator by default.
- **C & C++ Support:** Provides both a C++ `stl_allocator` class and a C-style `xmalloc`/`xfree` interface.
- **Modern C++ Support:** Includes `xmake_shared` for pool-aware shared pointers and `xnew`/`xdelete` for manual object lifecycle management.
- **Flexible Modes:** Supports both static memory pools (no heap usage) and heap-recycled blocks.
- **Easy Integration:** Simple CMake-based build system and header-rich implementation.
- **Benchmark Proven:** Includes a benchmarking suite demonstrating significant performance improvements for container insertions/removals.

## Example Usage
Using the allocator is as simple as using the provided "x" container types:

```cpp
#include "xlist.h"
#include "xstring.h"

// Use xlist and xstring just like std::list and std::string
// but with fixed-block memory allocation.
xlist<xstring> myList;
myList.push_back("hello world");
```

For custom classes, use the `XALLOCATOR` macro to enable fixed-block allocation:

```cpp
#include "xallocator.h"

class MyClass {
    XALLOCATOR
public:
    int data;
};

// Now 'new MyClass' uses the fixed-block allocator
MyClass* obj = new MyClass();
delete obj;
```

### Modern C++ Features
`xmake_shared` provides a pool-aware alternative to `std::make_shared`:

```cpp
#include "xmake_shared.h"

// Allocates both the object and the control block from the fixed-block pool
auto sp = xmake_shared<int>(123);
```

`xnew` and `xdelete` provide pool-aware manual allocation for types without the `XALLOCATOR` macro:

```cpp
#include "xnew.h"

// Allocate and construct in the pool
MyClass* p = xnew<MyClass>(args...);

// Destroy and deallocate
xdelete(p);
```

## Getting Started
CMake is used to create the project build files:

1. Clone the repository.
2. From the repository root, run: `cmake -B build .`
3. Build the project: `cmake --build build --config Release`
4. Run the benchmarks and tests: `.\build\Release\stl_allocator_app.exe`

# Technical Details

## std::allocator
The STL `std::allocator` class provides the default memory allocation and deallocation strategy. Most STL containers (like `std::list` or `std::map`) use this allocator for both elements and internal nodes. Because these containers rely heavily on dynamic memory, the default allocator can cause heap fragmentation over time. `stl_allocator` solves this by routing allocations to a fixed-block memory manager.

## xallocator
The underlying engine is `xallocator`, a C-style fixed-block allocator. It has two modes:
1. **Static Pools:** Memory is obtained from pre-declared static arrays (zero heap usage).
2. **Heap Blocks:** Blocks are obtained from the global heap as needed but recycled into free-lists once freed.

## stl_allocator Implementation
`stl_allocator<T>` is a standard-compliant C++ allocator that wraps `xmalloc` and `xfree`.

```cpp
template <typename T>
class stl_allocator : public std::allocator<T> 
{
public:
	pointer allocate(size_type n, ...) {
		return static_cast<pointer>(xmalloc(n * sizeof(T)));
	}
	void deallocate(pointer p, size_type n) {
		xfree(p);
	}
};
```

### "x" Containers
Pre-defined templates simplify the use of `stl_allocator` with standard containers. These "x" versions are drop-in replacements for their `std` counterparts:

| Category | Supported Types |
| :--- | :--- |
| **Node-based** | `xlist`, `xmap`, `xmultimap`, `xset`, `xmultiset`, `xqueue` |
| **Strings** | `xstring`, `xwstring` |
| **Streams** | `xstringstream`, `xostringstream`, `xwstringstream`, `xwostringstream` |

> **Note:** `xvector` is not provided by default because vectors require contiguous memory blocks of varying sizes, which can be inefficient for a fixed-block allocator if the vector grows frequently.

## Benchmarking
Benchmarks demonstrate that `stl_allocator` is typically 2-3x faster than the global heap for container operations.

| Container | Mode | Run | Benchmark Time (mS) |
| :--- | :--- | :--- | :--- |
| `std::list` | Global Heap | 1 | 60 |
| `xlist` | Heap Blocks | 1 | 19 |
| `xlist` | Heap Blocks | 2 | 11 |

Initial runs in "Heap Blocks" mode may be slower as blocks are first fetched from the heap, but subsequent runs are significantly faster as blocks are recycled.

# Portable C/C++ Libraries
Explore [portable-c-cpp-libs](https://github.com/endurodave/portable-c-cpp-libs) for reusable C/C++ components, including state machines, callbacks, threading, memory management, and more.

## References
- [Allocator](https://github.com/endurodave/Allocator) - C++ fixed-block memory allocator.
- [xallocator](https://github.com/endurodave/xallocator) - C fixed-block memory allocator.

## License
Licensed under the MIT License. See [LICENSE](LICENSE) for details.
