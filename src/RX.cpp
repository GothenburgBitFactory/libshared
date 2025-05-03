////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 - 2017, 2019 - 2021, 2023, Gothenburg Bit Factory.
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

#include <RX.h>

#include <cstring>
#include <regex>
#include <string>

////////////////////////////////////////////////////////////////////////////////
RX::RX () = default;

////////////////////////////////////////////////////////////////////////////////
RX::RX (
  const std::string& pattern,
  bool case_sensitive /* = true */)
: _compiled (false)
, _pattern (pattern)
, _case_sensitive (case_sensitive)
{
  compile ();
}

////////////////////////////////////////////////////////////////////////////////
RX::RX (const RX& other)
{
  _compiled       = false;
  _pattern        = other._pattern;
  _case_sensitive = other._case_sensitive;
}

////////////////////////////////////////////////////////////////////////////////
RX::~RX() = default;

////////////////////////////////////////////////////////////////////////////////
RX& RX::operator= (const RX& other)
{
  _compiled       = false;
  _pattern        = other._pattern;
  _case_sensitive = other._case_sensitive;

  return *this;
}

////////////////////////////////////////////////////////////////////////////////
void RX::compile ()
{
  if (! _compiled)
  {
    std::regex::flag_type flags = std::regex_constants::ECMAScript;
    if (!_case_sensitive) {
      flags |= std::regex_constants::icase;
    }
    try {
      _regex.assign(_pattern, flags);
      _compiled = true;
    } catch (const std::regex_error& e) {
      throw std::runtime_error(e.what());
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool RX::match (const std::string& in)
{
  if (! _compiled)
    compile ();

  return std::regex_search(in, _regex);
}

////////////////////////////////////////////////////////////////////////////////
bool RX::match (
  std::vector<std::string>& matches,
  const std::string& in)
{
  if (! _compiled)
    compile ();

  auto matches_begin = std::sregex_iterator(in.begin(), in.end(), _regex);
  auto matches_end = std::sregex_iterator();
  for (std::sregex_iterator it = matches_begin; it != matches_end; ++it) {
      matches.push_back(it->str());
  }

  return !matches.empty();
}

////////////////////////////////////////////////////////////////////////////////
bool RX::match (
  std::vector <int>& start,
  std::vector <int>& end,
  const std::string& in)
{
  if (! _compiled)
    compile ();

  auto matches_begin = std::sregex_iterator(in.begin(), in.end(), _regex);
  auto matches_end = std::sregex_iterator();
  for (std::sregex_iterator it = matches_begin; it != matches_end; ++it) {
    start.push_back(it->position());
    end.push_back(it->position() + it->length());
  }

  return !start.empty ();
}

////////////////////////////////////////////////////////////////////////////////
