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

#ifndef INCLUDED_SHARED
#define INCLUDED_SHARED

#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

// shared.cpp, Non-UTF-8 aware.
void wrapText (std::vector <std::string>&, const std::string&, const int, bool);
int longestWord (const std::string&);
int longestLine (const std::string&);
bool extractLine (std::string&, const std::string&, int, bool, unsigned int&, char = '.');
std::vector <std::string> split (const std::string&, const char);
std::vector <std::string> split (const std::string&);
std::string join (const std::string&, const std::vector<int>&);
std::string join (const std::string&, const std::vector<std::string>&);
std::string str_replace (const std::string&, const std::string&, const std::string&);
std::string trim (const std::string&, const std::string& edible = " \t\n\f\r");
std::string ltrim (const std::string&, const std::string& edible = " \t\n\f\r");
std::string rtrim (const std::string&, const std::string& edible = " \t\n\f\r");
bool compare (const std::string&, const std::string&, bool sensitive = true);
bool closeEnough (const std::string&, const std::string&, unsigned int minLength = 0);
int matchLength (const std::string&, const std::string&);
std::string::size_type find (const std::string&, const std::string&, bool sensitive = true);
std::string::size_type find (const std::string&, const std::string&, std::string::size_type, bool sensitive = true);

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

template <class T> T listIntersect (
  const T& left, const T& right)
{
  T intersection;
  for (auto& l : left)
    if (std::find (right.begin (), right.end (), l) != right.end ())
      intersection.push_back (l);

  return intersection;
}

std::string lowerCase (const std::string&);
std::string upperCase (const std::string&);
std::string upperCaseFirst (const std::string&);

int autoComplete (const std::string&, const std::vector<std::string>&, std::vector<std::string>&, int minimum = 1);
bool confirm (const std::string&);

int execute (const std::string&, const std::vector <std::string>&, const std::string&, std::string&);
std::string osName ();
std::string cppCompliance ();

// ip.cpp
bool isIPv4Address (const std::string&, std::string&, int&);
bool isIPv6Address (const std::string&, std::string&, int&);

#endif
