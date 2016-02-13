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
#include <Datetime.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////
Datetime::Datetime ()
{
  clear ();
  _date = time (nullptr);
}

////////////////////////////////////////////////////////////////////////////////
Datetime::Datetime (const std::string& input, const std::string& format)
{
  clear ();
  std::string::size_type start = 0;
  if (! parse (input, start, format))
    throw ::format ("'{1}' is not a valid date in the '{2}' format.", input, format);
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse (
  const std::string& input,
  std::string::size_type& start,
  const std::string& format)
{
  auto i = start;
  Pig pig (input);
  if (i)
    pig.skip (i);

  // Parse epoch first, as it's the most common scenario.
  if (parse_epoch (pig))
  {
    // ::validate and ::resolve are not needed in this case.
    start = pig.cursor ();
    return true;
  }

  else if (parse_formatted (pig, format))
  {
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Datetime::clear ()
{
  _year    = 0;
  _month   = 0;
  _week    = 0;
  _weekday = 0;
  _julian  = 0;
  _day     = 0;
  _seconds = 0;
  _offset  = 0;
  _utc     = false;
  _date    = 0;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_formatted (Pig& n, const std::string& format)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Valid epoch values are unsigned integers after 1980-01-01T00:00:00Z. This
// restriction means that '12' will not be identified as an epoch date.
bool Datetime::parse_epoch (Pig& pig)
{
  pig.save ();

  int epoch;
  if (pig.getDigits (epoch) &&
      pig.eos ()            &&
      epoch >= 315532800)
  {
    _date = static_cast <time_t> (epoch);
    return true;
  }

  pig.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
time_t Datetime::toEpoch () const
{
  return _date;
}

////////////////////////////////////////////////////////////////////////////////
// Static
bool Datetime::leapYear (int year)
{
  return ((! (year % 4)) && (year % 100)) ||
         ! (year % 400);
}

////////////////////////////////////////////////////////////////////////////////
// Static
int Datetime::daysInMonth (int month, int year)
{
  static int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (month == 2 && Datetime::leapYear (year))
    return 29;

  return days[month - 1];
}

////////////////////////////////////////////////////////////////////////////////
// Static
int Datetime::daysInYear (int year)
{
  return Datetime::leapYear (year) ? 366 : 365;
}

////////////////////////////////////////////////////////////////////////////////
