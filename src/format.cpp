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
#include <format.h>
#include <utf8.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <strings.h>
#include <unistd.h>
#include <sys/select.h>
#include <cerrno>
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
