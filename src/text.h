////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_TEXT
#define INCLUDED_TEXT

#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

// text.cpp, Non-UTF-8 aware.
void wrapText (std::vector <std::string>&, const std::string&, const int, bool);
int longestWord (const std::string&);
int longestLine (const std::string&);
bool extractLine (std::string&, const std::string&, int, bool, unsigned int&);
std::vector <std::string> split (const std::string&, const char);
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
    replace_positional (output, "{" + format (fmt_num) + "}", format (arg));
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

// List operations.
template <class T> void listDiff (
  const T& left, const T& right, T& leftOnly, T& rightOnly)
{
  leftOnly.clear ();
  for (auto& l : left)
    if (std::find (right.begin (), right.end (), l) == right.end ())
      leftOnly.push_back (l);

  rightOnly.clear ();
  for (auto& r : right)
    if (std::find (left.begin (), left.end (), r) == left.end ())
      rightOnly.push_back (r);
}

#endif
////////////////////////////////////////////////////////////////////////////////
