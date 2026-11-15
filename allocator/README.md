# allocator

Copies of [Allocator](https://github.com/endurodave/Allocator) 1.2.0 and [xallocator](https://github.com/endurodave/xallocator) 1.3.0 (their `src/` folders), used by this repository's CMake build.

The PlatformIO and Arduino packages don't include them: they depend on xallocator (which depends on Allocator) instead, so a project can use all three libraries together without duplicate symbols.
