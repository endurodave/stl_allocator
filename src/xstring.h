#ifndef _XSTRING_H
#define _XSTRING_H

#include "stl_allocator.h"
#include <string>

// std::string::traits_type, not std::char_traits<char>: GCC with libc++ 17+
// warns on naming std::char_traits<char> (its primary template is deprecated).
typedef std::basic_string<char, std::string::traits_type, stl_allocator<char> > xstring;
typedef std::basic_string<wchar_t, std::wstring::traits_type, stl_allocator<wchar_t> > xwstring;

#endif 

