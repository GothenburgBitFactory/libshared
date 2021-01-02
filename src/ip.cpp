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

#include <cmake.h>
#include <shared.h>
#include <cctype>

static bool isPort (const std::string&, unsigned int&);
static bool isChar (const std::string&, char, unsigned int&);
static bool isEOS (const std::string&, unsigned int&);
static bool isIPv4Block (const std::string&, unsigned int&);
static bool isIPv4BlockSet (const std::string&, unsigned int&);
static bool isIPv6Block (const std::string&, unsigned int&);
static bool isIPv6BlockSet (const std::string&, unsigned int&);

////////////////////////////////////////////////////////////////////////////////
static bool isPort (const std::string& input, unsigned int& c)
{
  auto start = c;
  while (std::isdigit (input[c]))
    ++c;

  return c - start > 0 &&
         c - start < 6;
}

////////////////////////////////////////////////////////////////////////////////
static bool isChar (const std::string& input, char character, unsigned int& c)
{
  if (input[c] == character)
  {
    ++c;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool isEOS (const std::string& input, unsigned int& c)
{
  return c >= input.length ();
}

////////////////////////////////////////////////////////////////////////////////
static bool isIPv4Block (const std::string& input, unsigned int& c)
{
  auto start = c;
  while (std::isdigit (input[c]))
    ++c;

  if (c - start > 0 &&
      c - start < 4)
  {
    auto byte = std::stoi (input.substr (start, c - start));
    if (byte < 256)
      return true;
  }

  c = start;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool isIPv4BlockSet (const std::string& input, unsigned int& c)
{
  auto start = c;

  if (isIPv4Block (input,      c) &&
      isChar      (input, '.', c) &&
      isIPv4Block (input,      c) &&
      isChar      (input, '.', c) &&
      isIPv4Block (input,      c) &&
      isChar      (input, '.', c) &&
      isIPv4Block (input,      c))
  {
    return true;
  }

  c = start;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
static bool isIPv6Block (const std::string& input, unsigned int& c)
{
  auto start = c;
  while (std::isxdigit (input[c]))
    ++c;

  if (c - start > 0 &&
      c - start < 5)
  {
    return true;
  }

  c = start;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// at least one non-empty block, or '::'
// at least two colons
// 8 or less blocks
static bool isIPv6BlockSet (const std::string& input, unsigned int& c)
{
  auto start = c;
  int count_colons {0};
  int count_blocks {0};

  while (1)
  {
    if (isEOS (input, c))
    {
      if (c == start)
        return false;

      break;
    }

    else if (isChar (input, ':', c))
      ++count_colons;

    else if (isIPv4BlockSet (input, c))
      ++count_blocks;

    else if (isIPv6Block (input, c))
      ++count_blocks;

    else if (isChar (input, '.', c) ||
             isChar (input, ']', c))
    {
      --c;
      break;
    }

    else
      break;
  }

  if (count_colons >= 2 &&
      count_colons <= 7 &&
      ((count_blocks == 0 && input.substr (start, c) == "::") || count_blocks >= 1) &&
      count_blocks <= 8)
  {
    return true;
  }

  c = start;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <address>:<port>
// <address>
bool isIPv4Address (const std::string& input, std::string& address, int& port)
{
  unsigned int c = 0;
  if (isIPv4BlockSet (input, c))
  {
    auto colon = c;
    if (isChar (input, ':', c))
      if (! isPort (input, c))
        return false;

    if (isEOS (input, c))
    {
      address = input.substr (0, std::min (c, colon));
      port = 0;
      if (! isEOS (input, colon))
        port = std::stoi (input.substr (colon + 1));

      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
//  [<address>]:<port>
//  <address>
bool isIPv6Address (const std::string& input, std::string& address, int& port)
{
  unsigned int c = 0;

  if (isChar         (input, '[', c) &&
      isIPv6BlockSet (input,      c) &&
      isChar         (input, ']', c))
  {
    auto colon = c;
    if (isChar         (input, ':', c) &&
        isPort         (input,      c) &&
        isEOS          (input,      c))
    {
      address = input.substr (1, colon - 2);
      port = std::stoi (input.substr (colon + 1));
      return true;
    }
  }

  c = 0;
  if (isIPv6BlockSet (input, c) &&
      isEOS          (input, c))
  {
    address = input;
    port = 0;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
