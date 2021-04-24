////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 - 2021, Paul Beckingham, Federico Hernandez.
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
#include <Pig.h>
#include <shared.h>
#include <unicode.h>
#include <utf8.h>
#include <algorithm>
#include <sstream>
#include <cinttypes>
#include <cstdlib>

////////////////////////////////////////////////////////////////////////////////
Pig::Pig (const std::string& text)
: _text {std::make_shared <std::string> (text)}
{
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::skip (int c)
{
  if ((*_text)[_cursor] == c)
  {
    ++_cursor;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::skipN (const int quantity)
{
  auto save = _cursor;

  auto count = 0;
  while (count++ < quantity)
  {
    if (! utf8_next_char (*_text, _cursor))
    {
      _cursor = save;
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::skipWS ()
{
  auto save = _cursor;

  int c;
  auto prev = _cursor;
  while ((c = utf8_next_char (*_text, _cursor)))
  {
    if (! unicodeWhitespace (c))
    {
      _cursor = prev;
      break;
    }
    prev = _cursor;
  }

  return _cursor > save;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::skipLiteral (const std::string& literal)
{
  if (_text->find (literal, _cursor) == _cursor)
  {
    _cursor += literal.length ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::skipPartial (const std::string& reference, std::string& result,
                       bool ignore_case)
{
  // Walk the common substring.
  auto pos = 0;
  while (reference[pos] &&
         (*_text)[_cursor + pos] &&
         ((reference[pos] == (*_text)[_cursor + pos] && !ignore_case) ||
          (reference[pos] == tolower((*_text)[_cursor + pos]) && ignore_case)))
    ++pos;

  if (pos > 0)
  {
    result = _text->substr (_cursor, pos);
    _cursor += pos;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// ASCII-only version of getUntil. Has better performance characteristics at
// the cost of not considering utf-8 characters correctly. Use with caution.
bool Pig::getUntilAscii (char end, std::string& result)
{
  auto save = _cursor;
  auto found = _text->find (end, _cursor + 1);

  if (found == std::string::npos)
  {
    found = _text->size ();
    result = _text->substr (_cursor, found - _cursor);
    _cursor = found;
    return true;
  }

  result = _text->substr (_cursor, found - _cursor);
  _cursor = _cursor + result.size();

  return _cursor > save;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getUntil (int end, std::string& result)
{
  auto save = _cursor;

  int c;
  auto prev = _cursor;
  while ((c = utf8_next_char (*_text, _cursor)))
  {
    if (c == end)
    {
      _cursor = prev;
      result = _text->substr (save, _cursor - save);
      return true;
    }

    else if (eos ())
    {
      result = _text->substr (save, _cursor - save);
      return true;
    }

    prev = _cursor;
  }

  return _cursor > save;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getUntilWS (std::string& result)
{
  auto save = _cursor;

  int c;
  auto prev = _cursor;
  while ((c = utf8_next_char (*_text, _cursor)))
  {
    if (unicodeWhitespace (c))
    {
      _cursor = prev;
      result = _text->substr (save, _cursor - save);
      return true;
    }

    // Note: This test must follow the above unicodeWhitespace(c) test because
    //       it is testing the value of 'c', and eos() is testing _cursor,
    //       which has already been advanced.
    else if (eos ())
    {
      result = _text->substr (save, _cursor - save);
      return true;
    }

    prev = _cursor;
  }

  return _cursor > save;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getCharacter (int& result)
{
  int c = (*_text)[_cursor];
  if (c)
  {
    result = c;
    ++_cursor;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getDigit (int& result)
{
  int c = (*_text)[_cursor];
  if (c &&
      unicodeLatinDigit (c))
  {
    result = c - '0';
    ++_cursor;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getDigit2 (int& result)
{
  if (unicodeLatinDigit ((*_text)[_cursor + 0]))
  {
    if (unicodeLatinDigit ((*_text)[_cursor + 1]))
    {
      result = strtoimax (_text->substr (_cursor, 2).c_str (), NULL, 10);
      _cursor += 2;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getDigit3 (int& result)
{
  if (unicodeLatinDigit ((*_text)[_cursor + 0]))
  {
    if (unicodeLatinDigit ((*_text)[_cursor + 1]))
    {
      if (unicodeLatinDigit ((*_text)[_cursor + 2]))
      {
        result = strtoimax (_text->substr (_cursor, 3).c_str (), NULL, 10);
        _cursor += 3;
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getDigit4 (int& result)
{
  if (unicodeLatinDigit ((*_text)[_cursor + 0]))
  {
    if (unicodeLatinDigit ((*_text)[_cursor + 1]))
    {
      if (unicodeLatinDigit ((*_text)[_cursor + 2]))
      {
        if (unicodeLatinDigit ((*_text)[_cursor + 3]))
        {
          result = strtoimax (_text->substr (_cursor, 4).c_str (), NULL, 10);
          _cursor += 4;
          return true;
        }
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getDigits (int& result)
{
    long long temp = 0;
    bool success = false;

    success = getDigits (temp);
    if (success)
      result = temp;

    return success;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getDigits (long long& result)
{
  auto save = _cursor;

  int c;
  auto prev = _cursor;
  while ((c = utf8_next_char (*_text, _cursor)))
  {
    if (! unicodeLatinDigit (c))
    {
      _cursor = prev;
      break;
    }

    prev = _cursor;
  }

  if (_cursor > save)
  {
    result = strtoimax (_text->substr (save, _cursor - save).c_str (), NULL, 10);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getHexDigit (int& result)
{
  int c = (*_text)[_cursor];
  if (c &&
      unicodeHexDigit (c))
  {
    if (c >= '0' && c <= '9')
    {
      result = c - '0';
      ++_cursor;
      return true;
    }
    else if (c >= 'A' && c <= 'F')
    {
      result = c - 'A' + 10;
      ++_cursor;
      return true;
    }
    else if (c >= 'a' && c <= 'f')
    {
      result = c - 'a' + 10;
      ++_cursor;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// number:
//   int frac? exp?
//
// int:
//   (-|+)? digit+
//
// frac:
//   . digit+
//
// exp:
//   e digit+
//
// e:
//   e|E (+|-)?
//
bool Pig::getNumber (std::string& result)
{
  auto i = _cursor;

  // [+-]?
  if ((*_text)[i] &&
      ((*_text)[i] == '-' ||
       (*_text)[i] == '+'))
    ++i;

  // digit+
  if ((*_text)[i] &&
      unicodeLatinDigit ((*_text)[i]))
  {
    ++i;

    while ((*_text)[i] && unicodeLatinDigit ((*_text)[i]))
      ++i;

    // ( . digit+ )?
    if ((*_text)[i] && (*_text)[i] == '.')
    {
      ++i;

      while ((*_text)[i] && unicodeLatinDigit ((*_text)[i]))
        ++i;
    }

    // ( [eE] [+-]? digit+ )?
    if ((*_text)[i] &&
        ((*_text)[i] == 'e' ||
         (*_text)[i] == 'E'))
    {
      ++i;

      if ((*_text)[i] &&
          ((*_text)[i] == '+' ||
           (*_text)[i] == '-'))
        ++i;

      if ((*_text)[i] && unicodeLatinDigit ((*_text)[i]))
      {
        ++i;

        while ((*_text)[i] && unicodeLatinDigit ((*_text)[i]))
          ++i;

        result = _text->substr (_cursor, i - _cursor);
        _cursor = i;
        return true;
      }

      return false;
    }

    result = _text->substr (_cursor, i - _cursor);
    _cursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getNumber (double& result)
{
  std::string s;
  if (getNumber (s))
  {
    result = std::strtod (s.c_str (), NULL);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// [ + | - ] \d+ [ . [ \d+ ]]
bool Pig::getDecimal (std::string& result)
{
  auto i = _cursor;

  // [+-]?
  if ((*_text)[i] &&
      ((*_text)[i] == '-' ||
       (*_text)[i] == '+'))
    ++i;

  // digit+
  if ((*_text)[i] && unicodeLatinDigit ((*_text)[i]))
  {
    ++i;

    while ((*_text)[i] && unicodeLatinDigit ((*_text)[i]))
      ++i;

    // ( . digit+ )?
    if ((*_text)[i] && (*_text)[i] == '.')
    {
      ++i;

      while ((*_text)[i] && unicodeLatinDigit ((*_text)[i]))
        ++i;
    }

    result = _text->substr (_cursor, i - _cursor);
    _cursor = i;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getDecimal (double& result)
{
  std::string s;
  if (getDecimal (s))
  {
    result = std::strtod (s.c_str (), NULL);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Gets quote content:      "foobar" -> foobar      (for c = '"')
// Handles escaped quotes:  "foo\"bar" -> foo\"bar  (for c = '"')
// Returns false if first character is not c, or if there is no closing c.
// Does not modify content between quotes.
bool Pig::getQuoted (int quote, std::string& result)
{
  if (! (*_text)[_cursor] ||
      (*_text)[_cursor] != quote)
    return false;

  auto start = _cursor + utf8_sequence (quote);
  auto i = start;

  while ((*_text)[i])
  {
    i = _text->find (quote, i);
    if (i == std::string::npos)
      return false;  // Unclosed quote.  Short cut, not definitive.

    if (i == start)
    {
      // Empty quote
      _cursor += 2 * utf8_sequence (quote);  // Skip both quote chars
      result = "";
      return true;
    }

    if ((*_text)[i - 1] == '\\')
    {
      // Check for escaped backslashes.  Backtracking like this is not very
      // efficient, but is only done in extreme corner cases.

      auto j = i - 2;  // Start one character further left
      bool is_escaped_quote = true;
      while (j >= start && (*_text)[j] == '\\')
      {
        // Toggle flag for each further backslash encountered.
        is_escaped_quote = is_escaped_quote ? false : true;
        --j;
      }

      if (is_escaped_quote)
      {
        // Keep searching
        ++i;
        continue;
      }
    }

    // None of the above applied, we must have found the closing quote char.
    result.assign (*_text, start, i - start);
    _cursor = i + utf8_sequence (quote);  // Skip closing quote char
    return true;
  }

  // This should never be reached.  We could throw here instead.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Assumes that the options are sorted by decreasing length, so that if the
// options contain 'fourteen' and 'four', the stream is first matched against
// the longer entry.
bool Pig::getOneOf (
  const std::vector <std::string>& options,
  std::string& found)
{
  for (const auto& option : options)
  {
    if (skipLiteral (option))
    {
      found = option;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getHMS (int& hours, int& minutes, int& seconds)
{
  auto save = _cursor;

  if ((getDigit2 (hours) || getDigit (hours)) &&
      skip (':')                              &&
      getDigit2 (minutes))
  {
    seconds = 0;
    if (skip (':') &&
        ! getDigit2 (seconds))
      return false;

    return true;
  }

  _cursor = save;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::getRemainder (std::string& result)
{
  if ((*_text)[_cursor])
  {
    result = _text->substr (_cursor);
    _cursor += result.length ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Pig::eos () const
{
  return (*_text)[_cursor] == '\0';
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
int Pig::peek () const
{
  return (*_text)[_cursor];
}

////////////////////////////////////////////////////////////////////////////////
// Peeks ahead - does not move cursor.
std::string Pig::peek (const int quantity) const
{
  std::string::size_type adjusted = std::min (static_cast <std::string::size_type> (quantity), _text->length () - _cursor);
  if ((*_text)[_cursor])
    return _text->substr (_cursor, adjusted);

  return "";
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type Pig::cursor () const
{
  return _cursor;
}

////////////////////////////////////////////////////////////////////////////////
// Note: never called internally, otherwise the client cannot rely on iṫ.
std::string::size_type Pig::save ()
{
  return _saved = _cursor;
}

////////////////////////////////////////////////////////////////////////////////
// Note: never called internally, otherwise the client cannot rely on iṫ.
std::string::size_type Pig::restore ()
{
  return _cursor = _saved;
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type Pig::restoreTo (std::string::size_type previous)
{
  return _cursor = previous;
}

////////////////////////////////////////////////////////////////////////////////
std::string Pig::substr (
  std::string::size_type start,
  std::string::size_type end) const
{
  return _text->substr (start, end - start);
}

////////////////////////////////////////////////////////////////////////////////
std::string Pig::str () const
{
  return _text->substr (_cursor);
}

////////////////////////////////////////////////////////////////////////////////
// Show the text, with the matched part in white on green, and the unmatched
// part white on red, followed by the index equivalent.
std::string Pig::dump () const
{
  std::stringstream out;
  if (_cursor)
    out << "[37;42m"
        << _text->substr (0, _cursor)
        << "[0m";

  out << "[37;41m"
      << _text->substr (_cursor)
      << "[0m "
      << _cursor
      << '/'
      << _text->length ();

  return str_replace (out.str (), "\n", "\\n");
}

////////////////////////////////////////////////////////////////////////////////
