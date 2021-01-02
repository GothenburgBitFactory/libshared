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
#include <JSON.h>
#include <utf8.h>
#include <sstream>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>

////////////////////////////////////////////////////////////////////////////////
bool json::SAX::parse (const std::string& input, SAX::Sink& sink)
{
  sink.eventDocStart ();
  std::string::size_type cursor = 0;
  ignoreWhitespace (input, cursor);
  if (isObject (input, cursor, sink) ||
      isArray  (input, cursor, sink))
  {
    ignoreWhitespace (input, cursor);
    if (cursor < input.length ())
      error ("Error: extra characters found at position ", cursor);

    sink.eventDocEnd ();
    return true;
  }

  error ("Error: Missing '{' or '[' at position ", cursor);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Complete Unicode whitespace list.
//
// http://en.wikipedia.org/wiki/Whitespace_character
// Updated 2015-09-13
void json::SAX::ignoreWhitespace (const std::string& input, std::string::size_type& cursor)
{
  int c = input[cursor];
  while (c == 0x0020 ||   // space Common  Separator, space
         c == 0x0009 ||   // Common  Other, control  HT, Horizontal Tab
         c == 0x000A ||   // Common  Other, control  LF, Line feed
         c == 0x000B ||   // Common  Other, control  VT, Vertical Tab
         c == 0x000C ||   // Common  Other, control  FF, Form feed
         c == 0x000D ||   // Common  Other, control  CR, Carriage return
         c == 0x0085 ||   // Common  Other, control  NEL, Next line
         c == 0x00A0 ||   // no-break space  Common  Separator, space
         c == 0x1680 ||   // ogham space mark  Ogham Separator, space
         c == 0x180E ||   // mongolian vowel separator Mongolian Separator, space
         c == 0x2000 ||   // en quad Common  Separator, space
         c == 0x2001 ||   // em quad Common  Separator, space
         c == 0x2002 ||   // en space  Common  Separator, space
         c == 0x2003 ||   // em space  Common  Separator, space
         c == 0x2004 ||   // three-per-em space  Common  Separator, space
         c == 0x2005 ||   // four-per-em space Common  Separator, space
         c == 0x2006 ||   // six-per-em space  Common  Separator, space
         c == 0x2007 ||   // figure space  Common  Separator, space
         c == 0x2008 ||   // punctuation space Common  Separator, space
         c == 0x2009 ||   // thin space  Common  Separator, space
         c == 0x200A ||   // hair space  Common  Separator, space
         c == 0x200B ||   // zero width space
         c == 0x200C ||   // zero width non-joiner
         c == 0x200D ||   // zero width joiner
         c == 0x2028 ||   // line separator  Common  Separator, line
         c == 0x2029 ||   // paragraph separator Common  Separator, paragraph
         c == 0x202F ||   // narrow no-break space Common  Separator, space
         c == 0x205F ||   // medium mathematical space Common  Separator, space
         c == 0x2060 ||   // word joiner
         c == 0x3000)     // ideographic space Common  Separator, space
  {
    c = input[++cursor];
  }
}

////////////////////////////////////////////////////////////////////////////////
// object := '{' [<pair> [, <pair> ...]] '}'
bool json::SAX::isObject (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);
  auto backup = cursor;

  if (isLiteral (input, '{', cursor))
  {
    sink.eventObjectStart ();
    int counter = 0;

    if (isPair (input, cursor, sink))
    {
      ++counter;
      while (isLiteral (input, ',', cursor) &&
             isPair    (input, cursor, sink))
      {
        ++counter;
      }
    }

    ignoreWhitespace (input, cursor);
    if (isLiteral (input, '}', cursor))
    {
      sink.eventObjectEnd (counter);
      return true;
    }
    else
      error ("Error: Missing '}' at position ", cursor);
  }

  cursor = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// array := '[' [<value> [, <value> ...]] ']'
bool json::SAX::isArray (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);
  auto backup = cursor;

  if (isLiteral (input, '[', cursor))
  {
    sink.eventArrayStart ();
    int counter = 0;

    if (isValue (input, cursor, sink))
    {
      ++counter;
      while (isLiteral (input, ',', cursor) &&
             isValue   (input,      cursor, sink))
      {
        ++counter;
      }
    }

    ignoreWhitespace (input, cursor);
    if (isLiteral (input, ']', cursor))
    {
      sink.eventArrayEnd (counter);
      return true;
    }
    else
      error ("Error: Missing ']' at position ", cursor);
  }

  cursor = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// pair := <string> ':' <value>
bool json::SAX::isPair (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);
  auto backup = cursor;

  if (isKey (input, cursor, sink))
  {
    if (isLiteral (input, ':', cursor))
    {
      if (isValue (input, cursor, sink))
        return true;

      error ("Error: Missing value at position ", cursor);
    }
    else
      error ("Error: Missing ':' at position ", cursor);
  }

  cursor = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// value  := <string>
//         | <number>
//         | <object>
//         | <array>
//         | 'true'
//         | 'false'
//         | 'null'
bool json::SAX::isValue (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);

  return isString (input, cursor, sink) ||
         isNumber (input, cursor, sink) ||
         isObject (input, cursor, sink) ||
         isArray  (input, cursor, sink) ||
         isBool   (input, cursor, sink) ||
         isNull   (input, cursor, sink);
}

////////////////////////////////////////////////////////////////////////////////
bool json::SAX::isKey (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);

  std::string value;
  if (isStringValue (input, cursor, value))
  {
    sink.eventName (value);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool json::SAX::isString (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);

  std::string value;
  if (isStringValue (input, cursor, value))
  {
    sink.eventValueString (value);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// string := '"' [<chars> ...] '"'
// chars  := <unicode>
//         | '\"'
//         | '\\'
//         | '\/'
//         | '\b'
//         | '\f'
//         | '\n'
//         | '\r'
//         | '\t'
//         | \uXXXX
bool json::SAX::isStringValue (const std::string& input, std::string::size_type& cursor, std::string& value)
{
  auto backup = cursor;

  if (isLiteral (input, '"', cursor))
  {
    std::string word;
    int c;
    while ((c = input[cursor]))
    {
      // EOS.
      if (c == '"')
      {
        ++cursor;
        value = word;
        return true;
      }

      // Unicode \uXXXX codepoint.
      else if (input[cursor + 0] == '\\'  &&
               input[cursor + 1] == 'u'   &&
               isHexDigit (input[cursor + 2]) &&
               isHexDigit (input[cursor + 3]) &&
               isHexDigit (input[cursor + 4]) &&
               isHexDigit (input[cursor + 5]))
      {
        word += utf8_character (
                  hexToInt (
                    input[cursor + 2],
                    input[cursor + 3],
                    input[cursor + 4],
                    input[cursor + 5]));
        cursor += 6;
      }

      // An escaped thing.
      else if (c == '\\')
      {
        c = input[++cursor];
        switch (c)
        {
        case '"':  word += (char) 0x22; ++cursor; break;
        case '\'': word += (char) 0x27; ++cursor; break;
        case '\\': word += (char) 0x5C; ++cursor; break;
        case 'b':  word += (char) 0x08; ++cursor; break;
        case 'f':  word += (char) 0x0C; ++cursor; break;
        case 'n':  word += (char) 0x0A; ++cursor; break;
        case 'r':  word += (char) 0x0D; ++cursor; break;
        case 't':  word += (char) 0x09; ++cursor; break;
        case 'v':  word += (char) 0x0B; ++cursor; break;

        // This pass-through default case means that anything can be escaped
        // harmlessly. In particular 'quote' is included, if it not one of the
        // above characters.
        default:   word += (char) c;    ++cursor; break;
        }
      }

      // Ordinary character.
      else
      {
        word += (char) c;
        ++cursor;
      }
    }

    error ("Error: Missing '\"' at position ", cursor);
  }

  cursor = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// number := <int> [<frac>] [<exp>]
bool json::SAX::isNumber (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);
  auto backup = cursor;

  std::string integerPart;
  if (isInt (input, cursor, integerPart))
  {
    std::string fractionalPart;
    isFrac (input, cursor, fractionalPart);

    std::string exponentPart;
    isExp (input, cursor, exponentPart);

    // Does it fit in a long?
    std::string combined = integerPart + fractionalPart + exponentPart;
    char* end;
    long longValue = strtol (combined.c_str (), &end, 10);
    if (! *end && errno != ERANGE)
    {
      sink.eventValueInt (longValue);
      return true;
    }

    // Does it fit in an unsigned long?
    unsigned long ulongValue = strtoul (combined.c_str (), &end, 10);
    if (! *end && errno != ERANGE)
    {
      sink.eventValueUint (ulongValue);
      return true;
    }

    // If the above fail, allow this one to be capped at imax.
    double doubleValue = strtod (combined.c_str (), &end);
    if (! *end)
    {
      sink.eventValueDouble (doubleValue);
      return true;
    }
  }

  cursor = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// int := ['-'] <digits>
bool json::SAX::isInt (const std::string& input, std::string::size_type& cursor, std::string& value)
{
  auto backup = cursor;

  isLiteral (input, '-', cursor);
  if (isDigits (input, cursor))
  {
    value = input.substr (backup, cursor - backup);
    return true;
  }

  // No restore necessary.
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// frac := '.' <digits>
bool json::SAX::isFrac (const std::string& input, std::string::size_type& cursor, std::string& value)
{
  auto backup = cursor;

  if (isLiteral (input, '.', cursor) &&
      isDigits  (input,      cursor))
  {
    value = input.substr (backup, cursor - backup);
    return true;
  }

  cursor = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// digits := <digit> [<digit> ...]
bool json::SAX::isDigits (const std::string& input, std::string::size_type& cursor)
{
  int c = input[cursor];
  if (isDecDigit (c))
  {
    c = input[++cursor];

    while (isDecDigit (c))
      c = input[++cursor];

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// digit := 0x30 ('0') .. 0x39 ('9')
bool json::SAX::isDecDigit (int c)
{
  return c >= 0x30 && c <= 0x39;
}

////////////////////////////////////////////////////////////////////////////////
// hex := 0x30 ('0') .. 0x39 ('9')
bool json::SAX::isHexDigit (int c)
{
  return (c >= 0x30 && c <= 0x39) ||
         (c >= 0x61 && c <= 0x66) ||
         (c >= 0x41 && c <= 0x46);
}

////////////////////////////////////////////////////////////////////////////////
// exp := <e> <digits>
bool json::SAX::isExp (const std::string& input, std::string::size_type& cursor, std::string& value)
{
  auto backup = cursor;

  if (isE      (input, cursor) &&
      isDigits (input, cursor))
  {
    value = input.substr (backup, cursor - backup);
    return true;
  }

  cursor = backup;
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// e := e
//    | e+
//    | e-
//    | E
//    | E+
//    | E-
bool json::SAX::isE (const std::string& input, std::string::size_type& cursor)
{
  int c = input[cursor];
  if (c == 'e' ||
      c == 'E')
  {
    c = input[++cursor];

    if (c == '+' ||
        c == '-')
    {
      ++cursor;
    }

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool json::SAX::isBool (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);

  if (input[cursor + 0] == 't' &&
      input[cursor + 1] == 'r' &&
      input[cursor + 2] == 'u' &&
      input[cursor + 3] == 'e')
  {
    cursor += 4;
    sink.eventValueBool (true);
    return true;
  }
  else if (input[cursor + 0] == 'f' &&
           input[cursor + 1] == 'a' &&
           input[cursor + 2] == 'l' &&
           input[cursor + 3] == 's' &&
           input[cursor + 4] == 'e')
  {
    cursor += 5;
    sink.eventValueBool (false);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool json::SAX::isNull (const std::string& input, std::string::size_type& cursor, SAX::Sink& sink)
{
  ignoreWhitespace (input, cursor);

  if (input[cursor + 0] == 'n' &&
      input[cursor + 1] == 'u' &&
      input[cursor + 2] == 'l' &&
      input[cursor + 3] == 'l')
  {
    cursor += 4;
    sink.eventValueNull ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool json::SAX::isLiteral (const std::string& input, char literal, std::string::size_type& cursor)
{
  ignoreWhitespace (input, cursor);

  if (input[cursor] == literal)
  {
    ++cursor;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Converts '0'     -> 0
//          '9'     -> 9
//          'a'/'A' -> 10
//          'f'/'F' -> 15
int json::SAX::hexToInt (int c)
{
       if (c >= 0x30 && c <= 0x39) return (c - 0x30);
  else if (c >= 0x41 && c <= 0x46) return (c - 0x41 + 10);
  else                             return (c - 0x61 + 10);
}

////////////////////////////////////////////////////////////////////////////////
int json::SAX::hexToInt (int c0, int c1, int c2, int c3)
{
  return (hexToInt (c0) << 12) +
         (hexToInt (c1) << 8)  +
         (hexToInt (c2) << 4)  +
          hexToInt (c3);
}

////////////////////////////////////////////////////////////////////////////////
void json::SAX::error (const std::string& message, std::string::size_type cursor)
{
  std::stringstream error;
  error << message << cursor;
  throw error.str ();
}

////////////////////////////////////////////////////////////////////////////////

