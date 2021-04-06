////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Paul Beckingham, Federico Hernandez.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_FORMAT
#define INCLUDED_FORMAT

#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

const std::string format (std::string&);
const std::string format (const char*);
const std::string formatHex (int);
const std::string format (float, int, int);
const std::string format (double, int, int);
const std::string format (double);

void replace_positional (std::string&, const std::string&, const std::string&);

template<typename T>
const std::string format (T value)
{
    std::stringstream s;
    s << value;
    return s.str ();
}

template<typename T>
const std::string format (int fmt_num, const std::string& fmt, T arg)
{
    std::string output = fmt;
    replace_positional (output, '{' + format (fmt_num) + '}', format (arg));
    return output;
}

template<typename T, typename... Args>
const std::string format (int fmt_num, const std::string& fmt, T arg, Args... args)
{
    const std::string fmt_replaced (format (fmt_num, fmt, arg));
    return format (fmt_num+1, fmt_replaced, args...);
}

template<typename... Args>
const std::string format (const std::string& fmt, Args... args)
{
    return format (1, fmt, args...);
}

std::string leftJustify (const int, const int);
std::string leftJustify (const std::string&, const int);
std::string rightJustifyZero (const int, const int);
std::string rightJustify (const int, const int);
std::string rightJustify (const std::string&, const int);

std::string commify (const std::string&);
std::string formatBytes (size_t);
std::string formatTime (time_t);
std::string printable (const std::string&);
std::string printable (char);

std::string obfuscateText (const std::string&);

#endif
