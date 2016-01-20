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

#include <cmake.h>
#include <text.h>
#include <utf8.h>
#include <sstream>
#include <iomanip>
#include <strings.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
void wrapText (
  std::vector <std::string>& lines,
  const std::string& text,
  const int width,
  bool hyphenate)
{
  std::string line;
  unsigned int offset = 0;
  while (extractLine (line, text, width, hyphenate, offset))
    lines.push_back (line);
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> split (const std::string& input, const char delimiter)
{
  std::vector <std::string> results;
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + 1;
  }

  if (input.length ())
    results.push_back (input.substr (start));

  return results;
}

////////////////////////////////////////////////////////////////////////////////
std::string trim (const std::string& input)
{
  auto start = input.find_first_not_of (" \t\n\r\f");
  auto end   = input.find_last_not_of  (" \t\n\r\f");

  if (start == std::string::npos)
    return "";

  if (end == std::string::npos)
    return input.substr (start);

  return input.substr (start, end - start + 1);
}

////////////////////////////////////////////////////////////////////////////////
int longestWord (const std::string& input)
{
  int longest = 0;
  int length = 0;
  std::string::size_type i = 0;
  int character;

  while ((character = utf8_next_char (input, i)))
  {
    if (character == ' ')
    {
      if (length > longest)
        longest = length;

      length = 0;
    }
    else
      length += mk_wcwidth (character);
  }

  if (length > longest)
    longest = length;

  return longest;
}

////////////////////////////////////////////////////////////////////////////////
int longestLine (const std::string& input)
{
  int longest = 0;
  int length = 0;
  std::string::size_type i = 0;
  int character;

  while ((character = utf8_next_char (input, i)))
  {
    if (character == '\n')
    {
      if (length > longest)
        longest = length;

      length = 0;
    }
    else
      length += mk_wcwidth (character);
  }

  if (length > longest)
    longest = length;

  return longest;
}

////////////////////////////////////////////////////////////////////////////////
// Walk the input text looking for a break point.  A break point is one of:
//   - EOS
//   - \n
//   - last space before 'length' characters
//   - last punctuation (, ; . :) before 'length' characters, even if not
//     followed by a space
//   - first 'length' characters
//
// text       "one two three\n  four"
// bytes       0123456789012 3456789
// characters  1234567890a23 4567890
//
// leading_ws
// ws             ^   ^       ^^
// punct
// break                     ^
bool extractLine (
  std::string& line,
  const std::string& text,
  int width,
  bool hyphenate,
  unsigned int& offset)
{
  // Terminate processing.
  // Note: bytes vs bytes.
  if (offset >= text.length ())
    return false;

  std::string::size_type last_last_bytes = offset;
  std::string::size_type last_bytes = offset;
  std::string::size_type bytes = offset;
  unsigned int last_ws = 0;
  int character;
  int char_width = 0;
  int line_width = 0;
  while (1)
  {
    last_last_bytes = last_bytes;
    last_bytes = bytes;
    character = utf8_next_char (text, bytes);

    if (character == 0 ||
        character == '\n')
    {
      line = text.substr (offset, last_bytes - offset);
      offset = bytes;
      break;
    }
    else if (character == ' ')
      last_ws = last_bytes;

    char_width = mk_wcwidth (character);
    if (line_width + char_width > width)
    {
      int last_last_character = text[last_last_bytes];
      int last_character = text[last_bytes];

      // [case 1] one| two --> last_last != 32, last == 32, ws == 0
      if (last_last_character != ' ' &&
          last_character      == ' ')
      {
        line = text.substr (offset, last_bytes - offset);
        offset = last_bytes + 1;
        break;
      }

      // [case 2] one |two --> last_last == 32, last != 32, ws != 0
      else if (last_last_character == ' ' &&
               last_character      != ' ' &&
               last_ws             != 0)
      {
        line = text.substr (offset, last_bytes - offset - 1);
        offset = last_bytes;
        break;
      }

      else if (last_last_character != ' ' &&
               last_character      != ' ')
      {
        // [case 3] one t|wo --> last_last != 32, last != 32, ws != 0
        if (last_ws != 0)
        {
          line = text.substr (offset, last_ws - offset);
          offset = last_ws + 1;
          break;
        }
        // [case 4] on|e two --> last_last != 32, last != 32, ws == 0
        else
        {
          if (hyphenate)
          {
            line = text.substr (offset, last_bytes - offset - 1) + "-";
            offset = last_last_bytes;
          }
          else
          {
            line = text.substr (offset, last_bytes - offset);
            offset = last_bytes;
          }
        }

        break;
      }
    }

    line_width += char_width;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool compare (
  const std::string& left,
  const std::string& right,
  bool sensitive /*= true*/)
{
  // Use strcasecmp if required.
  if (!sensitive)
    return strcasecmp (left.c_str (), right.c_str ()) == 0 ? true : false;

  // Otherwise, just use std::string::operator==.
  return left == right;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (std::string& value)
{
  return value;
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (const char* value)
{
  std::string s (value);
  return s;
}

////////////////////////////////////////////////////////////////////////////////
const std::string formatHex (int value)
{
  std::stringstream s;
  s.setf (std::ios::hex, std::ios::basefield);
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (float value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  if (0 < value && value < 1)
  {
    // For value close to zero, width - 2 (2 accounts for the first zero and
    // the dot) is the number of digits after zero that are significant
    double factor = 1;
    for (int i = 2; i < width; i++)
      factor *= 10;
    value = roundf (value * factor) / factor;
  }
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (double value, int width, int precision)
{
  std::stringstream s;
  s.width (width);
  s.precision (precision);
  if (0 < value && value < 1)
  {
    // For value close to zero, width - 2 (2 accounts for the first zero and
    // the dot) is the number of digits after zero that are significant
    double factor = 1;
    for (int i = 2; i < width; i++)
      factor *= 10;
    value = round (value * factor) / factor;
  }
  s << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
const std::string format (double value)
{
  std::stringstream s;
  s << std::fixed << value;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
void replace_positional (
  std::string& fmt,
  const std::string& from,
  const std::string& to)
{
  std::string::size_type pos = 0;
  while ((pos = fmt.find (from, pos)) != std::string::npos)
  {
    fmt.replace (pos, from.length (), to);
    pos += to.length ();
  }
}

////////////////////////////////////////////////////////////////////////////////
std::string leftJustify (const int input, const int width)
{
  std::stringstream s;
  s << input;
  std::string output = s.str ();
  return output + std::string (width - output.length (), ' ');
}

////////////////////////////////////////////////////////////////////////////////
std::string leftJustify (const std::string& input, const int width)
{
  return input + std::string (width - utf8_text_width (input), ' ');
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustifyZero (const int input, const int width)
{
  std::stringstream s;
  s << std::setw (width) << std::setfill ('0') << input;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustify (const int input, const int width)
{
  std::stringstream s;
  s << std::setw (width) << std::setfill (' ') << input;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string rightJustify (const std::string& input, const int width)
{
  unsigned int len = utf8_text_width (input);
  return (((unsigned int) width > len)
           ? std::string (width - len, ' ')
           : "")
         + input;
}

////////////////////////////////////////////////////////////////////////////////
const char *json_encode[] = {
  "\x00", "\x01", "\x02", "\x03", "\x04", "\x05", "\x06", "\x07",
   "\\b",  "\\t",  "\\n", "\x0b",  "\\f",  "\\r", "\x0e", "\x0f",
  "\x10", "\x11", "\x12", "\x13", "\x14", "\x15", "\x16", "\x17",
  "\x18", "\x19", "\x1a", "\x1b", "\x1c", "\x1d", "\x1e", "\x1f",
  "\x20", "\x21", "\\\"", "\x23", "\x24", "\x25", "\x26", "\x27",
  "\x28", "\x29", "\x2a", "\x2b", "\x2c", "\x2d", "\x2e",  "\\/",
  "\x30", "\x31", "\x32", "\x33", "\x34", "\x35", "\x36", "\x37",
  "\x38", "\x39", "\x3a", "\x3b", "\x3c", "\x3d", "\x3e", "\x3f",
  "\x40", "\x41", "\x42", "\x43", "\x44", "\x45", "\x46", "\x47",
  "\x48", "\x49", "\x4a", "\x4b", "\x4c", "\x4d", "\x4e", "\x4f",
  "\x50", "\x51", "\x52", "\x53", "\x54", "\x55", "\x56", "\x57",
  "\x58", "\x59", "\x5a", "\x5b", "\\\\", "\x5d", "\x5e", "\x5f",
  "\x60", "\x61", "\x62", "\x63", "\x64", "\x65", "\x66", "\x67",
  "\x68", "\x69", "\x6a", "\x6b", "\x6c", "\x6d", "\x6e", "\x6f",
  "\x70", "\x71", "\x72", "\x73", "\x74", "\x75", "\x76", "\x77",
  "\x78", "\x79", "\x7a", "\x7b", "\x7c", "\x7d", "\x7e", "\x7f",
  "\x80", "\x81", "\x82", "\x83", "\x84", "\x85", "\x86", "\x87",
  "\x88", "\x89", "\x8a", "\x8b", "\x8c", "\x8d", "\x8e", "\x8f",
  "\x90", "\x91", "\x92", "\x93", "\x94", "\x95", "\x96", "\x97",
  "\x98", "\x99", "\x9a", "\x9b", "\x9c", "\x9d", "\x9e", "\x9f",
  "\xa0", "\xa1", "\xa2", "\xa3", "\xa4", "\xa5", "\xa6", "\xa7",
  "\xa8", "\xa9", "\xaa", "\xab", "\xac", "\xad", "\xae", "\xaf",
  "\xb0", "\xb1", "\xb2", "\xb3", "\xb4", "\xb5", "\xb6", "\xb7",
  "\xb8", "\xb9", "\xba", "\xbb", "\xbc", "\xbd", "\xbe", "\xbf",
  "\xc0", "\xc1", "\xc2", "\xc3", "\xc4", "\xc5", "\xc6", "\xc7",
  "\xc8", "\xc9", "\xca", "\xcb", "\xcc", "\xcd", "\xce", "\xcf",
  "\xd0", "\xd1", "\xd2", "\xd3", "\xd4", "\xd5", "\xd6", "\xd7",
  "\xd8", "\xd9", "\xda", "\xdb", "\xdc", "\xdd", "\xde", "\xdf",
  "\xe0", "\xe1", "\xe2", "\xe3", "\xe4", "\xe5", "\xe6", "\xe7",
  "\xe8", "\xe9", "\xea", "\xeb", "\xec", "\xed", "\xee", "\xef",
  "\xf0", "\xf1", "\xf2", "\xf3", "\xf4", "\xf5", "\xf6", "\xf7",
  "\xf8", "\xf9", "\xfa", "\xfb", "\xfc", "\xfd", "\xfe", "\xff"
};

std::string jsonEncode (const std::string& input)
{
  std::string output;
  output.reserve ((input.size () * 6) / 5);  // 20% increase.

  auto last = input.begin ();
  for (auto i = input.begin (); i != input.end (); ++i)
  {
    switch (*i)
    {
    // Simple translations.
    case  '"':
    case '\\':
    case  '/':
    case '\b':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
      output.append (last, i);
      output += json_encode[(unsigned char)(*i)];
      last = i + 1;

    // Default NOP.
    }
  }

  output.append (last, input.end ());

  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string jsonDecode (const std::string& input)
{
  std::string output;
  output.reserve (input.size ());  // Same size.

  size_t pos = 0;

  while (pos < input.length ())
  {
    if (input[pos] == '\\')
    {
      ++pos;
      switch (input[pos])
      {
        // Simple translations.
        case '"':  output += '"';  break;
        case '\\': output += '\\'; break;
        case '/':  output += '/';  break;
        case 'b':  output += '\b'; break;
        case 'f':  output += '\f'; break;
        case 'n':  output += '\n'; break;
        case 'r':  output += '\r'; break;
        case 't':  output += '\t'; break;

        // Compose a UTF8 unicode character.
        case 'u':
          output += utf8_character (utf8_codepoint (input.substr (++pos)));
          pos += 3;
          break;

        // If it is an unrecognized sequence, do nothing.
        default:
          output += '\\';
          output += input[pos];
          break;
      }
      ++pos;
    }
    else
    {
      size_t next_backslash = input.find ('\\', pos);
      output.append (input, pos, next_backslash - pos);
      pos = next_backslash;
    }
  }

  return output;
}

////////////////////////////////////////////////////////////////////////////////
