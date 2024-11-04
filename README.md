# A Custom STL std::allocator Replacement Improves Performance
Protect against heap fragmentation faults and improve execution speed with a fixed block alternative to STL std::allocator.

# Table of Contents

- [A Custom STL std::allocator Replacement Improves Performance](#a-custom-stl-stdallocator-replacement-improves-performance)
- [Table of Contents](#table-of-contents)
- [Preface](#preface)
- [Introduction](#introduction)
- [std::allocator](#stdallocator)
- [xallocator    ](#xallocator-)
- [stl\_allocator](#stl_allocator)
  - ["x" Containers](#x-containers)
  - ["x" Strings](#x-strings)
  - ["x" Streams](#x-streams)
- [Benchmarking](#benchmarking)
- [Reference Articles](#reference-articles)
- [Benefits](#benefits)


# Preface

Originally published on CodeProject at: <a href="https://www.codeproject.com/Articles/1089905/A-Custom-STL-std-allocator-Replacement-Improves-Pe"><strong>A Custom STL std::allocator Replacement Improves Performance</strong></a>

<p><a href="https://www.cmake.org/">CMake</a>&nbsp;is used to create the build files. CMake is free and open-source software. Windows, Linux and other toolchains are supported. See the <strong>CMakeLists.txt </strong>file for more information.</p>

# Introduction

<p>This is my third and final article concerning fixed block memory allocators here on Code Project. This time, we&#39;ll create an alternate C++ Standard Library <code>std::allocator</code> memory manager using the groundwork laid by the first two articles.&nbsp;</p>

<p>The Standard Template Library (STL) is powerful C++ software library including container and iterator support. The problem with using the library for mission-critical or time-critical projects isn&#39;t with STL itself &ndash; the library is very robust. Instead, it&#39;s with the unrestricted use of the global heap. The standard STL allocator uses the heap extensively during operation. This is a problem on resource constrained embedded systems. Embedded devices can be expected to operate for months or years where a fault caused by a fragmented heap must be prevented. Fortunately, STL provides a means to replace <code>std::allocator</code> with one of our own design.&nbsp;</p>

<p>Fixed block memory allocators are a common technique to provide dynamic-like operation yet the memory is provided from memory pools where the blocks doled are of a fixed size. Unlike the global heap, which is expected to universally operate on blocks of any size, a fixed block allocator can be tailored for a narrowly defined purpose. Fixed block allocators can also offer consistent execution times whereas the global heap cannot offer such guarantees.&nbsp;</p>

<p>This article describes an STL-compatible allocator implementation that relies upon a fixed block allocator for dispensing and reclaiming memory. The new allocator prevents faults caused by a fragmented heap and offer consistent allocation/deallocation execution times.&nbsp;</p>

# std::allocator

<p>The STL <code>std::allocator</code> class provides the default memory allocation and deallocation strategy. If you examine code for a container class, such as <code>std::list</code>, you&#39;ll see the default <code>std::allocator</code> template argument. In this case, <code>allocator&lt;_Ty&gt;</code> is template class handling allocation duties for <code>_Ty</code> objects.</p>

<pre lang="C++">
template&lt;class _Ty, class _Ax = allocator&lt;_Ty&gt; &gt;
class list : public _List_val&lt;_Ty, _Ax&gt;
{
    // ...
}</pre>

<p>Since the template parameter <code>_Ax</code> defaults to <code>allocator&lt;_Ty</code>&gt; you can create a list object without manually specifying an allocator. Declaring <code>myList </code>as shown below creates an allocator class for allocating/deallocating <code>int </code>values.&nbsp;</p>

<pre lang="C++">
std::list&lt;int&gt; myList;</pre>

<p>The STL containers rely upon dynamic memory for elements and nodes. An element is the size of an inserted object. In this case, <code>sizeof(int)</code> is the memory required to store one list element. A node is an internal structure required to bind the elements together. For <code>std::list</code>, it is a doubly-linked list storing, at a minimum, pointers to the next and previous nodes.&nbsp;</p>

<p>Of course, the element size varies depending on the objects stored. However, the node size varies too depending on the container used. A <code>std::list</code> node may have a different size than a <code>std::map</code> node. Because of this, a STL-allocator must be able to handle different sized memory block requests.&nbsp;</p>

<p>An STL-allocator must adhere to specific interface requirements. This isn&#39;t an article on the how&#39;s and why&#39;s of the <code>std::allocator</code> API &ndash; there are many online references that explain this better than I. Instead, I&#39;ll focus on where to place the memory allocation/deallocation calls within an existing STL-allocator class interface and provide new &quot;x&quot; versions of all common containers to simplify usage.&nbsp;</p>

# xallocator&nbsp;&nbsp; &nbsp;

<p>Most of the heavy lifting for the new fixed block STL allocator comes from the underlying <code>xallocator</code> as described within my article &quot;<a href="https://github.com/endurodave/xallocator"><strong>Replace malloc/free with a Fast Fixed Block Memory Allocator</strong></a>&quot;. As the title states, this module replaces <code>malloc</code>/<code>free </code>with new fixed block <code>xmalloc</code>/<code>xfree</code> versions.&nbsp;</p>

<p>To the user, these replacement functions operate in the same way as the standard CRT versions except for the fixed block feature. In short, <code>xallocator</code> has two modes of operation: <em>static pools</em>, where all memory is obtained from pre-declared static memory, or <em>heap blocks</em>, where blocks are obtained from the global heap but recycled for later use when freed. See the aforementioned article for implementation details.&nbsp;</p>

# stl_allocator

<p>The class <code>stl_allocator </code>is the fixed block STL-compatible implementation. This class is used as an alternative to <code>std::allocator</code>.&nbsp;</p>

```cpp
#ifndef _STL_ALLOCATOR_H
#define _STL_ALLOCATOR_H

// @see https://github.com/endurodave/stl_allocator
// David Lafreniere

#include "xallocator.h"
#include <memory>  // For std::allocator and std::allocator_traits

// Forward declaration for stl_allocator<void>
template <typename T>
class stl_allocator;

// Specialization for `void`, but we no longer need to define `pointer` and `const_pointer`
template <>
class stl_allocator<void> 
{
public:
	typedef void value_type;

	template <class U>
	struct rebind { typedef stl_allocator<U> other; };
};

// Define the custom stl_allocator inheriting from std::allocator
template <typename T>
class stl_allocator : public std::allocator<T> 
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	// Default constructor
	stl_allocator() {}

	// Copy constructor
	template <class U>
	stl_allocator(const stl_allocator<U>&) {}

	// Rebind struct
	template <class U>
	struct rebind { typedef stl_allocator<U> other; };

	// Override allocate method to use custom allocation function
	pointer allocate(size_type n, typename std::allocator_traits<stl_allocator<void>>::const_pointer hint = 0) 
	{
		return static_cast<pointer>(xmalloc(n * sizeof(T)));
	}

	// Override deallocate method to use custom deallocation function
	void deallocate(pointer p, size_type n) 
	{
		xfree(p);
	}

	// You can inherit other methods like construct and destroy from std::allocator
};

// Comparison operators for compatibility
template <typename T, typename U>
inline bool operator==(const stl_allocator<T>&, const stl_allocator<U>) { return true; }

template <typename T, typename U>
inline bool operator!=(const stl_allocator<T>&, const stl_allocator<U>) { return false; }

#endif 
```

<p>The code is really just a standard <code>std::allocator</code> interface. There are many examples online. The source attached to this article has been used on many different compilers (GCC, Keil, VisualStudio). The thing we&#39;re interested in is where to tap into the interface with our own memory manager. The methods of interest are:</p>

<ul>
	<li><code>allocate()</code></li>
	<li><code>deallocate()</code></li>
</ul>

<p><code>allocate()</code> allocates <code>n</code> number of instances of object type <code>T</code>. <code>xmalloc()</code> is used to obtained memory from the fixed block memory pool as opposed to the global heap.</p>

<pre lang="C++">
pointer allocate(size_type n, stl_allocator&lt;void&gt;::const_pointer hint = 0)
{
    return static_cast&lt;pointer&gt;(xmalloc(n*sizeof(T)));
}</pre>

<p><code>deallocate()</code> deallocates a memory block previously allocated using <code>allocate()</code>. A simple call to <code>xfree()</code> routes the request to our memory manager.&nbsp;</p>

<pre lang="C++">
void deallocate(pointer p, size_type n)
{
    xfree(p);
}</pre>

<p>Really, that&#39;s all there is to it once you have an fixed block memory manager. <code>xallocator</code> is designed to handle any size memory request. Therefore, no matter storage size called for by the C++ Standard Library, elements or nodes, <code>xmalloc</code>/<code>xfree</code> processes the memory request.</p>

<p>Of course, <code>stl_allocator</code> is a template class. Notice that the fixed block allocation duties are delegated to non-template functions <code>xmalloc()</code> and <code>xfree()</code>. This makes the instantiated code for each instance as small as possible.&nbsp;</p>

## &quot;x&quot; Containers

<p>The following STL container classes use fixed sized memory blocks that never change in size while the container is being utilized. The number of heap element/node blocks goes up and down, but block sizes are constant for a given container instantiation.</p>

<ul>
	<li><code>std::list</code></li>
	<li><code>std::map</code></li>
	<li><code>std::multipmap</code></li>
	<li><code>std::set</code></li>
	<li><code>std::multiset</code></li>
	<li><code>std::queue</code></li>
</ul>

<p>To make using <code>stl_allocator</code> a bit easier, new classes were created for many of the standard container types. Each new container just inherits from the standard library counterpart and is pre-pended with &quot;<code>x</code>&quot;.&nbsp;</p>

<ul>
	<li><code>xlist</code></li>
	<li><code>xmap</code></li>
	<li><code>xmultimap</code></li>
	<li><code>xset</code></li>
	<li><code>xmultiset</code></li>
	<li><code>xqueue</code></li>
</ul>

<p>The following code shows the complete <code>xlist</code> implementation. Notice <code>xlist</code> just inherits from <code>std::list</code>, but the key difference is the <code>_Ax</code> template argument defaults to <code>stl_allocator</code> and not <code>std::allocator</code>.&nbsp;</p>

```cpp
#ifndef _XLIST_H
#define _XLIST_H

#include "stl_allocator.h"
#include <list>

// xlist uses a fix-block memory allocator
template <typename T, typename Alloc = stl_allocator<T>>
using xlist = std::list<T, Alloc>;

#endif
```

<p>Each of the &ldquo;<code>x</code>&rdquo; versions of the STL container is used just like the <code>std</code> version except all allocations are handled by <code>stl_allocator</code>. For instance:</p>

<pre lang="C++">
#include  &quot;xlist.h&quot;

xlist&lt;xstring&gt; myList;
myList.push_back(&quot;hello world&quot;);
</pre>

<p>The following container types use variable sized memory blocks from the heap that expand or contract in size during operation.&nbsp;</p>

<ul>
	<li><code>std::string</code></li>
	<li><code>std::vector</code></li>
</ul>

<p>A corresponding <code>xvector</code> wasn&#39;t implemented because vectors require a contiguous memory region whereas the other container types are node based. A fixed block allocator works well with equally sized blocks, but not so well if a <code>vector</code> continually expands to a larger and larger single block. While <code>stl_allocator</code> would work with a <code>vector</code>, it could be misused and cause runtime problems with the fixed block memory manager.&nbsp;</p>

<p>A <code>std::string</code> also varies its requested memory size during execution, but typically, strings aren&#39;t used in an unbounded fashion. Meaning, in most cases you&#39;re not trying to store a 100K-byte string, whereas a <code>vector</code> you might actually do that. Therefore, <code>xstring</code>&#39;s are available as explained next.&nbsp;</p>

## &quot;x&quot; Strings

<p>New <code>x</code>-version of the standard and wide versions of the <code>string</code> classes are created.</p>

<ul>
	<li><code>xstring</code></li>
	<li><code>xwstring</code></li>
</ul>

<p>Here, we just <code>typedef</code> a new x-version using <code>stl_allocator</code> as the default allocator of <code>char</code> and <code>wchar_t</code> types:</p>

<pre lang="C++">
#ifndef _XSTRING_H
#define _XSTRING_H

#include &quot;stl_allocator.h&quot;
#include &lt;string&gt;

typedef std::basic_string&lt;char, std::char_traits&lt;char&gt;, stl_allocator&lt;char&gt; &gt; xstring;
typedef std::basic_string&lt;wchar_t, std::char_traits&lt;wchar_t&gt;, stl_allocator&lt;wchar_t&gt; &gt; xwstring;

#endif </pre>

<p lang="c++">Usage of an <code>xstring</code> is just like any other <code>std::string</code>.&nbsp;</p>

<pre lang="C++">
#include &quot;xstring.h&quot;

xwstring s = L&quot;This is &quot;;
s += L&quot;my string.&quot;;</pre>

## &quot;x&quot; Streams

<p>The <code>iostreams</code> C++ Standard Library offers powerful and easy-to-use string formatting by way of the <code>stream</code> classes.&nbsp;</p>

<ul>
	<li><code>std::stringstream</code></li>
	<li><code>std::ostringstream</code></li>
	<li><code>std::wstringstream</code></li>
	<li><code>std::wostringstream</code></li>
</ul>

<p>Like the container classes, <code>iostreams</code> makes use of the custom STL allocator instead of the global heap using these new definitions.</p>

<ul>
	<li><code>xstringstream</code></li>
	<li><code>xostringstream</code></li>
	<li><code>xwstringstream</code></li>
	<li><code>xwostringstream</code></li>
</ul>

<p>Again, just <code>typedef</code> new x-versions with the default <code>stl_allocator</code> template arguments makes it easy.</p>

<pre lang="C++">
#ifndef _XSSTREAM_H
#define _XSSTREAM_H

#include &quot;stl_allocator.h&quot;
#include &lt;sstream&gt;

typedef std::basic_stringstream&lt;char, std::char_traits&lt;char&gt;, 
stl_allocator&lt;char&gt; &gt; xstringstream;
typedef std::basic_ostringstream&lt;char, std::char_traits&lt;char&gt;, 
stl_allocator&lt;char&gt; &gt; xostringstream;

typedef std::basic_stringstream&lt;wchar_t, std::char_traits&lt;wchar_t&gt;, 
stl_allocator&lt;wchar_t&gt; &gt; xwstringstream;
typedef std::basic_ostringstream&lt;wchar_t, std::char_traits&lt;wchar_t&gt;, 
stl_allocator&lt;wchar_t&gt; &gt; xwostringstream;

#endif</pre>

<p>Now, using an <code>xstringstream</code> is a snap.</p>

<pre lang="C++">
xstringstream myStringStream;
myStringStream &lt;&lt; &quot;hello world &quot; &lt;&lt; 2016 &lt;&lt; ends;
</pre>

# Benchmarking

<p>In my previous allocator articles, simple tests show the fixed block allocator is faster than the global heap. Now let&#39;s check the <code>stl_allocator</code> enabled containers to see they compare against <code>std::allocator</code> on a Windows PC.&nbsp;</p>

<p>All tests were built with Visual Studio 2008 and maximum speed compiler optimizations enabled. The <code>xallocator</code> is running in heap blocks mode where blocks are initially obtained from the heap but recycled during deallocations. The debug heap is also disabled by setting the <strong>Debugging &gt; Environment _NO_DEBUG_HEAP=1</strong>. The debug heap is considerably slower because of the extra safety checks.&nbsp;</p>

<p>The benchmark tests are simplistic and stress insertion/removal of 10000 elements. Each test is run three times. The insert/remove is where the STL library relies upon dynamic storage and that&#39;s the focus of these tests, not the underlying algorithmic performance.&nbsp;</p>

<p>The code snippet below is the <code>std::list</code> test. The other container tests are similarly basic.&nbsp;</p>

<pre lang="C++">
list&lt;int&gt; myList;
for (int i=0; i&lt;MAX_BENCHMARK; i++)
    myList.push_back(123);
myList.clear();</pre>

<p>The performance difference between <code>std::allocator</code> and <code>stl_allocator</code> running in heap block mode is shown below:&nbsp;<br />
&nbsp;</p>

<table class="ArticleTable">
	<thead>
		<tr>
			<td>Container</td>
			<td>Mode</td>
			<td>Run</td>
			<td>Benchmark Time (mS)</td>
		</tr>
	</thead>
	<tbody>
		<tr>
			<td><code>std::list</code></td>
			<td>Global Heap</td>
			<td>1</td>
			<td>60</td>
		</tr>
		<tr>
			<td><code>std::list</code></td>
			<td>Global Heap</td>
			<td>2</td>
			<td>32</td>
		</tr>
		<tr>
			<td><code>std::list</code></td>
			<td>Global Heap</td>
			<td>3</td>
			<td>23</td>
		</tr>
		<tr>
			<td><code>xlist</code></td>
			<td>Heap Blocks</td>
			<td>1</td>
			<td>19</td>
		</tr>
		<tr>
			<td><code>xlist</code></td>
			<td>Heap Blocks</td>
			<td>2</td>
			<td>11</td>
		</tr>
		<tr>
			<td><code>xlist</code></td>
			<td>Heap Blocks</td>
			<td>3</td>
			<td>11</td>
		</tr>
		<tr>
			<td><code>std::map</code></td>
			<td>Global Heap</td>
			<td>1</td>
			<td>37</td>
		</tr>
		<tr>
			<td><code>std::map</code></td>
			<td>Global Heap</td>
			<td>2</td>
			<td>34</td>
		</tr>
		<tr>
			<td><code>std::map</code></td>
			<td>Global Heap</td>
			<td>3</td>
			<td>41</td>
		</tr>
		<tr>
			<td><code>xmap</code></td>
			<td>Heap Blocks</td>
			<td>1</td>
			<td>38</td>
		</tr>
		<tr>
			<td><code>xmap</code></td>
			<td>Heap Blocks</td>
			<td>2</td>
			<td>25</td>
		</tr>
		<tr>
			<td><code>xmap</code></td>
			<td>Heap Blocks</td>
			<td>3</td>
			<td>25</td>
		</tr>
		<tr>
			<td><code>std::string</code></td>
			<td>Global Heap</td>
			<td>1</td>
			<td>171</td>
		</tr>
		<tr>
			<td><code>std::string</code></td>
			<td>Global Heap</td>
			<td>2</td>
			<td>146</td>
		</tr>
		<tr>
			<td><code>std::string</code></td>
			<td>Global Heap</td>
			<td>3</td>
			<td>95</td>
		</tr>
		<tr>
			<td><code>xstring</code></td>
			<td>Heap Blocks</td>
			<td>1</td>
			<td>57</td>
		</tr>
		<tr>
			<td><code>xstring</code></td>
			<td>Heap Blocks</td>
			<td>2</td>
			<td>36</td>
		</tr>
		<tr>
			<td><code>xstring</code></td>
			<td>Heap Blocks</td>
			<td>3</td>
			<td>40</td>
		</tr>
	</tbody>
</table>

<p>The test results shows that, for this benchmark, the <code>stl_allocator</code> enabled versions are approximately 2 to 3 times faster than <code>std::allocator</code>. Now this doesn&#39;t mean that STL is now magically overall twice as fast. Again, this benchmark is just concentrating on insertion and removal performance. The underlying container algorithmic performance hasn&#39;t changed. Therefore, the overall improvement seen will vary depending on how often your application inserts/removes from containers.&nbsp;</p>

<p>The <code>stl_allocator</code> operates in fixed time if <code>xallocator</code> is used in static pool mode. If <code>xallocator</code> heap blocks mode is used, the execution time is constant once the free list is populated with blocks obtained from the heap. Notice that the <code>xlist</code> benchmark above has an initial execution time of 19mS, yet the subsequent executions are 11mS each. On the first run, <code>xallocator</code> had to obtain fresh blocks from the global heap using <code>operator new</code>. On runs 2 and 3, the blocks already existed within the <code>xallocator</code> free-list so the heap isn&#39;t relied upon making successive runs much faster.&nbsp;</p>

<p>The global heap has unpredictable execution time performance when the heap fragments. Since <code>xallocator</code> only allocates blocks and recycles them for later use the execution time is more predictable and consistent.&nbsp;</p>

<p>Game developers go to great lengths to solve heap fragmentation and its myriad of associated problems. The Electronic Arts Standard Template Library (EASTL) white paper goes into detail about the problems with STL and specifically the <code>std::allocator</code>. Paul states in the&nbsp;<em>EASTL -- Electronic Arts Standard Template Library</em> document:</p>

<blockquote class="quote">
<div class="op">Quote:</div>

<p>Among game developers the most fundamental weakness is the std allocator design, and it is this weakness that was the largest contributing factor to the creation of EASTL.</p>
</blockquote>

<p>While I don&#39;t write code for games, I can see how the lag associated with fragmentation and erratic allocation/deallocation times would effect game play.&nbsp;</p>

# Reference Articles

<ul>
	<li><strong><a href="http://www.codeproject.com/Articles/1084801/Replace-malloc-free-with-a-Fast-Fixed-Block-Memory">Replace malloc/free with a Fast Fixed Block Memory Allocator</a></strong> by David Lafreniere</li>
	<li><a href="http://www.codeproject.com/Articles/1083210/An-Efficient-Cplusplus-Fixed-Block-Memory-Allocato"><strong>An Efficient C++ Fixed Block Memory Allocator</strong></a> by David Lafreniere</li>
	<li><a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2271.html"><strong>EASTL -- Electronic Arts Standard Template Library</strong></a>&nbsp; by Paul Pedriana</li>
</ul>

# Benefits

<p>STL is an extremely useful C++ library. Too often through, for medical devices I can&#39;t use it due to the possibility of a heap fragmentation memory fault. This leads to rolling your own custom container classes for each project instead of using an existing, well-tested library.&nbsp;</p>

<p>My goal with <code>stl_allocator</code> was eliminating memory faults; however, the fixed block allocator does offer faster and more consistent execution times whereas the heap does not. The heap implementation and level of fragmentation leads to unpredictable execution times. Depending on your application, this may or may not be an issue.</p>

<p>As shown in this article, simply employing an STL-compatible fixed block allocator opens up the possibility of using C++ Standard Template Library features on a project that otherwise may not have been possible.&nbsp;</p>



