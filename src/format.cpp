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
#include <format.h>
#include <utf8.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <csignal>
#include <cmath>

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
  return output + std::string (std::max<int> (width - output.length (), 0), ' ');
}

////////////////////////////////////////////////////////////////////////////////
std::string leftJustify (const std::string& input, const int width)
{
  return input + std::string (std::max<int> (width - utf8_text_width (input), 0), ' ');
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
  return std::string (std::max<int> (width - utf8_text_width (input), 0), ' ') + input;
}

////////////////////////////////////////////////////////////////////////////////
std::string commify (const std::string& data)
{
  // First scan for decimal point and end of digits.
  int decimalPoint = -1;
  int end          = -1;

  int i;
  for (int i = 0; i < (int) data.length (); ++i)
  {
    if (isdigit (data[i]))
      end = i;

    if (data[i] == '.')
      decimalPoint = i;
  }

  std::string result;
  if (decimalPoint != -1)
  {
    // In reverse order, transfer all digits up to, and including the decimal
    // point.
    for (i = (int) data.length () - 1; i >= decimalPoint; --i)
      result += data[i];

    int consecutiveDigits = 0;
    for (; i >= 0; --i)
    {
      if (isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && isdigit (data[i - 1]))
        {
          result += ',';
          consecutiveDigits = 0;
        }
      }
      else
        result += data[i];
    }
  }
  else
  {
    // In reverse order, transfer all digits up to, but not including the last
    // digit.
    for (i = (int) data.length () - 1; i > end; --i)
      result += data[i];

    int consecutiveDigits = 0;
    for (; i >= 0; --i)
    {
      if (isdigit (data[i]))
      {
        result += data[i];

        if (++consecutiveDigits == 3 && i && isdigit (data[i - 1]))
        {
          result += ',';
          consecutiveDigits = 0;
        }
      }
      else
        result += data[i];
    }
  }

  // reverse result into data.
  std::string done;
  for (int i = (int) result.length () - 1; i >= 0; --i)
    done += result[i];

  return done;
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in bytes to a more readable format.
std::string formatBytes (size_t bytes)
{
  char formatted[24];

       if (bytes >=  995000000) snprintf (formatted, sizeof(formatted), "%.1f GiB", bytes / 1000000000.0);
  else if (bytes >=     995000) snprintf (formatted, sizeof(formatted), "%.1f MiB", bytes /    1000000.0);
  else if (bytes >=        995) snprintf (formatted, sizeof(formatted), "%.1f KiB", bytes /       1000.0);
  else                          snprintf (formatted, sizeof(formatted), "%d B",     (int)bytes);

  return commify (formatted);
}

////////////////////////////////////////////////////////////////////////////////
// Convert a quantity in seconds to a more readable format.
std::string formatTime (time_t seconds)
{
  char formatted[24];
  float days = (float) seconds / 86400.0;

       if (seconds >= 86400 * 365) snprintf (formatted, sizeof(formatted), "%.1f y", (days / 365.0));
  else if (seconds >= 86400 * 84)  snprintf (formatted, sizeof(formatted), "%1d mo", (int) (days / 30));
  else if (seconds >= 86400 * 13)  snprintf (formatted, sizeof(formatted), "%d wk",  (int) (float) (days / 7.0));
  else if (seconds >= 86400)       snprintf (formatted, sizeof(formatted), "%d d",   (int) days);
  else if (seconds >= 3600)        snprintf (formatted, sizeof(formatted), "%d h",   (int) (seconds / 3600));
  else if (seconds >= 60)          snprintf (formatted, sizeof(formatted), "%d m",   (int) (seconds / 60));
  else if (seconds >= 1)           snprintf (formatted, sizeof(formatted), "%d s",   (int) seconds);
#ifdef HAVE_STRLCPY
  else                             strlcpy (formatted, "-", sizeof(formatted));
#else
  else                             strcpy (formatted, "-");
#endif /* HAVE_STRLCPY */

  return std::string (formatted);
}

////////////////////////////////////////////////////////////////////////////////
std::string printable (const std::string& input)
{
  // Sanitize 'message'.
  std::string sanitized = input;
  std::string::size_type bad;
  while ((bad = sanitized.find ('\r')) != std::string::npos)
    sanitized.replace (bad, 1, "\\r");

  while ((bad = sanitized.find ('\n')) != std::string::npos)
    sanitized.replace (bad, 1, "\\n");

  while ((bad = sanitized.find ('\f')) != std::string::npos)
    sanitized.replace (bad, 1, "\\f");

  while ((bad = sanitized.find ('\t')) != std::string::npos)
    sanitized.replace (bad, 1, "\\t");

  while ((bad = sanitized.find ('\v')) != std::string::npos)
    sanitized.replace (bad, 1, "\\v");

  return sanitized;
}

////////////////////////////////////////////////////////////////////////////////
std::string printable (char input)
{
  // Sanitize 'message'.
  char stringized[2] = {0};
  stringized[0] = input;

  std::string sanitized = stringized;
  switch (input)
  {
  case '\r': sanitized = "\\r"; break;
  case '\n': sanitized = "\\n"; break;
  case '\f': sanitized = "\\f"; break;
  case '\t': sanitized = "\\t"; break;
  case '\v': sanitized = "\\v"; break;
  default:   sanitized = input; break;
  }

  return sanitized;
}

////////////////////////////////////////////////////////////////////////////////
// Iterate over the input, converting text to 'x'.
// Does not modify color codes.
std::string obfuscateText (const std::string& input)
{
  std::stringstream output;
  std::string::size_type i = 0;
  int character;
  bool inside = false;

  while ((character = utf8_next_char (input, i)))
  {
    if (inside)
    {
      output << (char) character;

      if (character == 'm')
        inside = false;
    }
    else
    {
      if (character == 033)
        inside = true;

      if (inside || character == ' ')
        output << (char) character;
      else
        output << 'x';
    }
  }

  return output.str ();
}

////////////////////////////////////////////////////////////////////////////////
