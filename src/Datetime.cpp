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
#include <sstream>
#include <iomanip>
#include <cassert>
#include <stdlib.h>
#include <shared.h>
#include <format.h>
#include <unicode.h>
#include <utf8.h>

static std::vector <std::string> dayNames {
  "sunday",
  "monday",
  "tuesday",
  "wednesday",
  "thursday",
  "friday",
  "saturday"};

static std::vector <std::string> monthNames {
  "january",
  "february",
  "march",
  "april",
  "may",
  "june",
  "july",
  "august",
  "september",
  "october",
  "november",
  "december"};

int Datetime::weekstart = 1; // Monday, per ISO-8601.
int Datetime::minimumMatchLength = 3;
bool Datetime::isoEnabled = true;
bool Datetime::lookForwards = true;

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
Datetime::Datetime (const time_t t)
{
  clear ();
  _date = t;
}

////////////////////////////////////////////////////////////////////////////////
Datetime::Datetime (const int y, const int m, const int d)
{
  // Protect against arguments being passed in the wrong order.
  assert (y >= 1969 && y < 2100);
  assert (m >= 1 && m <= 12);
  assert (d >= 1 && d <= 31);

  clear ();

  // Error if not valid.
  struct tm t {};
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = d;
  t.tm_mon   = m - 1;
  t.tm_year  = y - 1900;

  _date = mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
Datetime::Datetime (const int y,  const int m,  const int d,
                    const int hr, const int mi, const int se)
{
  // Protect against arguments being passed in the wrong order.
  assert (y >= 1969 && y < 2100);
  assert (m >= 1 && m <= 12);
  assert (d >= 1 && d <= 31);
  assert (hr >= 0 && hr <= 24);
  assert (mi >= 0 && mi < 60);
  assert (se >= 0 && se < 60);

  clear ();

  // Error if not valid.
  struct tm t {};
  t.tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t.tm_mday  = d;
  t.tm_mon   = m - 1;
  t.tm_year  = y - 1900;
  t.tm_hour  = hr;
  t.tm_min   = mi;
  t.tm_sec   = se;

  _date = mktime (&t);
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
    pig.skipN (static_cast <int> (i));

  // Parse epoch first, as it's the most common scenario.
  if (parse_epoch (pig))
  {
    // ::validate and ::resolve are not needed in this case.
    start = pig.cursor ();
    return true;
  }

  else if (parse_formatted (pig, format))
  {
    // Check the values and determine time_t.
    if (validate ())
    {
      start = pig.cursor ();
      resolve ();
      return true;
    }
  }

  // Allow parse_date_time and parse_date_time_ext regardless of
  // Datetime::isoEnabled setting, because these formats are relied upon by
  // the 'import' command, JSON parser and hook system.
  else if (parse_date_time_ext   (pig) || // Strictest first.
           parse_date_time       (pig) ||
           (Datetime::isoEnabled &&
            (parse_date_ext      (pig) ||
             parse_date          (pig) ||
             parse_time_utc_ext  (pig) ||
             parse_time_utc      (pig) ||
             parse_time_off_ext  (pig) ||
             parse_time_off      (pig) ||
             parse_time_ext      (pig) ||
             parse_time          (pig)))) // Time last, as it is the most permissive.
  {
    // Check the values and determine time_t.
    if (validate ())
    {
      start = pig.cursor ();
      resolve ();
      return true;
    }
  }

  else if (parse_named (pig))
  {
    // ::validate and ::resolve are not needed in this case.
    start = pig.cursor ();
    return true;
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
bool Datetime::parse_formatted (Pig& pig, const std::string& format)
{
  // Short-circuit on missing format.
  if (format == "")
    return false;

  auto checkpoint = pig.cursor ();

  int month  {-1};   // So we can check later.
  int day    {-1};
  int year   {-1};
  int hour   {-1};
  int minute {-1};
  int second {-1};

  // For parsing, unused.
  int wday   {-1};
  int week   {-1};

  for (unsigned int f = 0; f < format.length (); ++f)
  {
    switch (format[f])
    {
    case 'm':
      if (pig.getDigit (month))
      {
        if (month == 0)
          pig.getDigit (month);

        if (month == 1)
          if (pig.getDigit (month))
            month += 10;
      }
      else
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'M':
      if (! pig.getDigit2 (month))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'd':
      if (pig.getDigit (day))
      {
        if (day == 0)
          pig.getDigit (day);

        if (day == 1 || day == 2 || day == 3)
        {
          int tens = day;
          if (pig.getDigit (day))
            day += 10 * tens;
        }
      }
      else
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'D':
      if (! pig.getDigit2 (day))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'y':
      if (! pig.getDigit2 (year))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      year += 2000;
      break;

    case 'Y':
      if (! pig.getDigit4 (year))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'h':
      if (pig.getDigit (hour))
      {
        if (hour == 0)
          pig.getDigit (hour);

        if (hour == 1 || hour == 2)
        {
          int tens = hour;
          if (pig.getDigit (hour))
            hour += 10 * tens;
        }
      }
      else
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'H':
      if (! pig.getDigit2 (hour))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'n':
      if (pig.getDigit (minute))
      {
        if (minute == 0)
          pig.getDigit (minute);

        if (minute < 6)
        {
          int tens = minute;
          if (pig.getDigit (minute))
            minute += 10 * tens;
        }
      }
      else
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'N':
      if (! pig.getDigit2 (minute))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 's':
      if (pig.getDigit (second))
      {
        if (second == 0)
          pig.getDigit (second);

        if (second < 6)
        {
          int tens = second;
          if (pig.getDigit (second))
            second += 10 * tens;
        }
      }
      else
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'S':
      if (! pig.getDigit2 (second))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'v':
      if (pig.getDigit (week))
      {
        if (week == 0)
          pig.getDigit (week);

        if (week < 6)
        {
          int tens = week;
          if (pig.getDigit (week))
            week += 10 * tens;
        }
      }
      else
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'V':
      if (! pig.getDigit2 (week))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;

    case 'a':
      wday = Datetime::dayOfWeek (pig.str ().substr (0, 3));
      if (wday == -1)
      {
        pig.restoreTo (checkpoint);
        return false;
      }

      pig.skipN (3);
      break;

    case 'A':
      {
        std::string dayName;
        if (pig.getUntil (format[f + 1], dayName))
        {
          wday = Datetime::dayOfWeek (dayName);
          if (wday == -1)
          {
            pig.restoreTo (checkpoint);
            return false;
          }
        }
      }
      break;

    case 'b':
      month = Datetime::monthOfYear (pig.str ().substr (0, 3));
      if (month == -1)
      {
        pig.restoreTo (checkpoint);
        return false;
      }

      pig.skipN (3);
      break;

    case 'B':
      {
        std::string monthName;
        if (pig.getUntil (format[f + 1], monthName))
        {
          month = Datetime::monthOfYear (monthName);
          if (month == -1)
          {
            pig.restoreTo (checkpoint);
            return false;
          }
        }
      }
      break;

    default:
      if (! pig.skip (format[f]))
      {
        pig.restoreTo (checkpoint);
        return false;
      }
      break;
    }
  }

  // It is possible that the format='Y-M-D', and the input is Y-M-DTH:N:SZ, and
  // this should not be considered a match.
  if (! pig.eos () && ! unicodeWhitespace (pig.peek ()))
  {
    pig.restoreTo (checkpoint);
    return false;
  }

  // Missing values are filled in from the current date.
  if (year == -1)
  {
    Datetime now;
    year = now.year ();
    if (month == -1)
    {
      month = now.month ();
      if (day == -1)
      {
        day = now.day ();
        if (hour == -1)
        {
          hour = now.hour ();
          if (minute == -1)
          {
            minute = now.minute ();
            if (second == -1)
              second = now.second ();
          }
        }
      }
    }
  }

  // Any remaining undefined values are assigned defaults.
  if (month  == -1) month  = 1;
  if (day    == -1) day    = 1;
  if (hour   == -1) hour   = 0;
  if (minute == -1) minute = 0;
  if (second == -1) second = 0;

  _year    = year;
  _month   = month;
  _day     = day;
  _seconds = (hour * 3600) + (minute * 60) + second;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Note how these are all single words:
//   <day>
//   <month>
//   Nth
//   socy, eocy
//   socq, eocq
//   socm, eocm
//   som, eom
//   soq, eoq
//   soy, eoy
//   socw, eocw
//   sow, eow
//   soww, eoww
//   sod, eod
//   yesterday
//   today
//   now
//   tomorrow
//   later          = midnight, Jan 18th, 2038.
//   someday        = midnight, Jan 18th, 2038.
//   easter
//   eastermonday
//   ascension
//   pentecost
//   goodfriday
//   midsommar      = midnight, 1st Saturday after 20th June
//   midsommarafton = midnight, 1st Friday after 19th June
//   juhannus       = midnight, 1st Friday after 19th June
//
bool Datetime::parse_named (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::string token;
  if (pig.getUntilWS (token))
  {
    if (initializeNow            (token) ||
        initializeToday          (token) ||
        initializeSod            (token) ||
        initializeEod            (token) ||
        initializeTomorrow       (token) ||
        initializeYesterday      (token) ||
        initializeDayName        (token) ||
        initializeMonthName      (token) ||
        initializeLater          (token) ||
        initializeEoy            (token) ||
        initializeSocy           (token) ||
        initializeSoy            (token) ||
        initializeEoq            (token) ||
        initializeSocq           (token) ||
        initializeSoq            (token) ||
        initializeSocm           (token) ||
        initializeSom            (token) ||
        initializeEom            (token) ||
        initializeSocw           (token) ||
        initializeEow            (token) ||
        initializeSow            (token) ||
        initializeEoww           (token) ||
        initializeOrdinal        (token) ||
        initializeEaster         (token) ||
        initializeMidsommar      (token) ||
        initializeMidsommarafton (token) ||
        initializeInformalTime   (token))
    {
      return true;
    }
  }

/////

  pig.restoreTo (checkpoint);

  // obtain input tokens.
  std::vector <std::string> tokens;
  while (pig.getUntilWS (token))
  {
    tokens.push_back (token);
    if (! pig.skipWS ())
      break;
  }

  if (initializeFoo (tokens))
  {
    return true;
  }

/////

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Valid epoch values are unsigned integers after 1980-01-01T00:00:00Z. This
// restriction means that '12' will not be identified as an epoch date.
bool Datetime::parse_epoch (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int epoch {};
  if (pig.getDigits (epoch) &&
      pig.eos ()            &&
      epoch >= 315532800)
  {
    _date = static_cast <time_t> (epoch);
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// date-ext 'T' time-ext 'Z'
// date-ext 'T' time-ext offset-ext
// date-ext 'T' time-ext
bool Datetime::parse_date_time_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_date_ext (pig) &&
      pig.skip ('T')       &&
      (parse_time_utc_ext (pig) ||
       parse_time_off_ext (pig) ||
       parse_time_ext     (pig) ))
  {
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// YYYY-MM-DD
// YYYY-MM
// YYYY-DDD
// YYYY-Www-D
// YYYY-Www
bool Datetime::parse_date_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int year {};
  if (parse_year (pig, year) &&
      pig.skip ('-'))
  {
    auto checkpointYear = pig.cursor ();

    int month {};
    int day {};
    int julian {};

    if (pig.skip ('W') &&
        parse_week (pig, _week))
    {
      if (pig.skip ('-') &&
          pig.getDigit (_weekday))
      {
        // What is happening here - must be something to do?
      }

      if (! unicodeLatinDigit (pig.peek ()))
      {
        _year = year;
        return true;
      }
    }
    else
      pig.restoreTo (checkpointYear);

    if (parse_month (pig, month) &&
        pig.skip ('-')           &&
        parse_day (pig, day)     &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      _year = year;
      _month = month;
      _day = day;
      return true;
    }
    else
      pig.restoreTo (checkpointYear);

    if (parse_julian (pig, julian) &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      _year = year;
      _julian = julian;
      return true;
    }
    else
      pig.restoreTo (checkpointYear);

    if (parse_month (pig, month) &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      _year = year;
      _month = month;
      _day = 1;
      return true;
    }
    else
      pig.restoreTo (checkpointYear);
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// ±hh[:mm]
bool Datetime::parse_off_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int sign = pig.peek ();
  if (sign == '+' || sign == '-')
  {
    pig.skipN (1);

    int hour {0};
    int minute {0};

    if (parse_off_hour (pig, hour) &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      if (pig.skip (':'))
      {
        if (! parse_off_minute (pig, minute))
        {
          pig.restoreTo (checkpoint);
          return false;
        }
      }

      _offset = (hour * 3600) + (minute * 60);
      if (sign == '-')
        _offset = - _offset;

      if (! unicodeLatinDigit (pig.peek ()))
        return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// hh:mm[:ss]
bool Datetime::parse_time_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int hour {};
  int minute {};
  if (parse_hour (pig, hour) &&
      pig.skip (':')         &&
      parse_minute (pig, minute))
  {
    if (pig.skip (':'))
    {
      int second {};
      if (parse_second (pig, second) &&
          ! unicodeLatinDigit (pig.peek ()))
      {
        _seconds = (hour * 3600) + (minute * 60) + second;
        return true;
      }

      pig.restoreTo (checkpoint);
      return false;
    }

    auto following = pig.peek ();
    if (! unicodeLatinDigit (following) &&
        following != 'A'                &&
        following != 'a'                &&
        following != 'P'                &&
        following != 'p')
    {
      _seconds = (hour * 3600) + (minute * 60);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time-ext 'Z'
bool Datetime::parse_time_utc_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_time_ext (pig) &&
      pig.skip ('Z')       &&
      ! unicodeLatinDigit (pig.peek ()))
  {
    _utc = true;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time-ext off-ext
bool Datetime::parse_time_off_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_time_ext (pig) &&
      parse_off_ext (pig))
  {
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// YYYYMMDDTHHMMSSZ
// YYYYMMDDTHHMMSS
bool Datetime::parse_date_time (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_date (pig) &&
      pig.skip ('T')   &&
      (parse_time_utc (pig) ||
       parse_time_off (pig) ||
       parse_time     (pig)))
  {
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// YYYYWww
// YYYYDDD
// YYYYMMDD
// YYYYMM
bool Datetime::parse_date (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int year {};
  int month {};
  int julian {};
  int week {};
  int weekday {};
  int day {};
  if (parse_year (pig, year))
  {
    auto checkpointYear = pig.cursor ();

    if (pig.skip ('W') &&
        parse_week (pig, week))
    {
      if (pig.getDigit (weekday))
        _weekday = weekday;

      if (! unicodeLatinDigit (pig.peek ()))
      {
        _year = year;
        _week = week;
        return true;
      }
    }
    else
      pig.restoreTo (checkpointYear);

    if (parse_julian (pig, julian) &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      _year = year;
      _julian = julian;
      return true;
    }
    else
      pig.restoreTo (checkpointYear);

    if (parse_month (pig, month))
    {
      if (parse_day (pig, day))
      {
        if (! unicodeLatinDigit (pig.peek ()))
        {
          _year = year;
          _month = month;
          _day = day;
          return true;
        }
      }
      else
      {
        if (! unicodeLatinDigit (pig.peek ()))
        {
          _year = year;
          _month = month;
          _day = 1;
          return true;
        }
      }
    }
    else
      pig.restoreTo (checkpointYear);
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <time> Z
bool Datetime::parse_time_utc (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_time (pig) &&
      pig.skip ('Z'))
  {
    _utc = true;
    if (! unicodeLatinDigit (pig.peek ()))
      return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <time> <off>
bool Datetime::parse_time_off (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_time (pig) &&
      parse_off (pig))
  {
    if (! unicodeLatinDigit (pig.peek ()))
      return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// hhmmss
// hhmm
bool Datetime::parse_time (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int hour {};
  int minute {};
  int second {};
  if (parse_hour (pig, hour) &&
      parse_minute (pig, minute))
  {
    parse_second (pig, second);
    if (! unicodeLatinDigit (pig.peek ()))
    {
      _seconds = (hour * 3600) + (minute * 60) + second;
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// ±hhmm
// ±hh
bool Datetime::parse_off (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int sign = pig.peek ();
  if (sign == '+' || sign == '-')
  {
    pig.skipN (1);

    int hour {};
    if (parse_off_hour (pig, hour))
    {
      int minute {};
      parse_off_minute (pig, minute);
      if (! unicodeLatinDigit (pig.peek ()))
      {
        _offset = (hour * 3600) + (minute * 60);
        if (sign == '-')
          _offset = - _offset;

        return true;
      }
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_year (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int year;
  if (pig.getDigit4 (year) &&
      year > 1969)
  {
    value = year;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_month (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int month;
  if (pig.getDigit2 (month) &&
      month > 0             &&
      month <= 12)
  {
    value = month;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_week (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int week;
  if (pig.getDigit2 (week) &&
      week > 0             &&
      week <= 53)
  {
    value = week;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_julian (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int julian;
  if (pig.getDigit3 (julian) &&
      julian > 0             &&
      julian <= 366)
  {
    value = julian;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_day (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int day;
  if (pig.getDigit2 (day) &&
      day > 0             &&
      day <= 31)
  {
    value = day;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_weekday (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int weekday;
  if (pig.getDigit (weekday) &&
      weekday >= 1           &&
      weekday <= 7)
  {
    value = weekday;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_hour (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int hour;
  if (pig.getDigit2 (hour) &&
      hour >= 0            &&
      hour < 24)
  {
    value = hour;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_minute (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int minute;
  if (pig.getDigit2 (minute) &&
      minute >= 0            &&
      minute < 60)
  {
    value = minute;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_second (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int second;
  if (pig.getDigit2 (second) &&
      second >= 0            &&
      second < 60)
  {
    value = second;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_off_hour (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int hour;
  if (pig.getDigit2 (hour) &&
      hour >= 0            &&
      hour <= 12)
  {
    value = hour;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_off_minute (Pig& pig, int& value)
{
  auto checkpoint = pig.cursor ();

  int minute;
  if (pig.getDigit2 (minute) &&
      minute >= 0            &&
      minute < 60)
  {
    value = minute;
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeNow (const std::string& token)
{
  if (token == "now")
  {
    _date = time (nullptr);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeToday (const std::string& token)
{
  if (token == "today")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSod (const std::string& token)
{
  if (token == "sod")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    if (Datetime::lookForwards)
      t->tm_mday++;

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);

    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeEod (const std::string& token)
{
  if (token == "eod")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_mday++;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeTomorrow (const std::string& token)
{
  if (closeEnough ("tomorrow", token, Datetime::minimumMatchLength))
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_mday++;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeYesterday (const std::string& token)
{
  if (closeEnough ("yesterday", token, Datetime::minimumMatchLength))
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    t->tm_mday -= 1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeDayName (const std::string& token)
{
  auto day = dayOfWeek (token);
  if (day != -1)
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    if (Datetime::lookForwards)
    {
      if (t->tm_wday >= day)
        t->tm_mday += day - t->tm_wday + 7;
      else
        t->tm_mday += day - t->tm_wday;
    }
    else
    {
      if (t->tm_wday >= day)
        t->tm_mday += day - t->tm_wday;
      else
        t->tm_mday += day - t->tm_wday - 7;
    }

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeMonthName (const std::string& token)
{
  auto month = monthOfYear (token);
  if (month != -1)
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    if (Datetime::lookForwards)
    {
      if (t->tm_mon >= month - 1)
        t->tm_year++;
    }

    t->tm_mon = month - 1;
    t->tm_mday = 1;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeLater (const std::string& token)
{
  if (token == "later" ||
      token == "someday")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_year = 138;
    t->tm_mon = 0;
    t->tm_mday = 18;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeEoy (const std::string& token)
{
  if (token == "eoy" ||
      token == "eocy")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon = 0;
    t->tm_mday = 1;
    t->tm_year++;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSocy (const std::string& token)
{
  if (token == "socy")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon = 0;
    t->tm_mday = 1;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSoy (const std::string& token)
{
  if (token == "soy")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon = 0;
    t->tm_mday = 1;

    if (Datetime::lookForwards)
      t->tm_year++;

    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeEoq (const std::string& token)
{
  if (token == "eoq" ||
      token == "eocq")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon += 3 - (t->tm_mon % 3);
    if (t->tm_mon > 11)
    {
      t->tm_mon -= 12;
      ++t->tm_year;
    }

    t->tm_mday = 1;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSocq (const std::string& token)
{
  if (token == "socq")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mon -= t->tm_mon % 3;
    t->tm_mday = 1;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSoq (const std::string& token)
{
  if (token == "soq")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    if (Datetime::lookForwards)
    {
      t->tm_mon += 3 - (t->tm_mon % 3);
      if (t->tm_mon > 11)
      {
        t->tm_mon -= 12;
        ++t->tm_year;
      }
    }
    else
    {
      t->tm_mon -= t->tm_mon % 3;
    }

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mday = 1;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSocm (const std::string& token)
{
  if (token == "socm")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_mday = 1;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSom (const std::string& token)
{
  if (token == "som")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = t->tm_min = t->tm_sec = 0;

    if (Datetime::lookForwards)
    {
      t->tm_mon++;
      if (t->tm_mon == 12)
      {
        t->tm_year++;
        t->tm_mon = 0;
      }
    }

    t->tm_mday = 1;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeEom (const std::string& token)
{
  if (token == "eom" ||
      token == "eocm")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_hour = 24;
    t->tm_min = t->tm_sec = 0;
    t->tm_mday = daysInMonth (t->tm_year + 1900, t->tm_mon + 1);
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSocw (const std::string& token)
{
  if (token == "socw")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);
    t->tm_hour = t->tm_min = t->tm_sec = 0;

    int extra = (t->tm_wday + 6) % 7;
    t->tm_mday -= extra;

    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeEow (const std::string& token)
{
  if (token == "eow" ||
      token == "eocw")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    t->tm_mday += 8 - t->tm_wday;
    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;

  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeSow (const std::string& token)
{
  if (token == "sow" || token == "soww")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    if (Datetime::lookForwards)
      t->tm_mday += 8 - t->tm_wday;
    else
      t->tm_mday -= (6 + t->tm_wday) % 7;

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeEoww (const std::string& token)
{
  if (token == "eoww")
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    if (Datetime::lookForwards)
      t->tm_mday += 6 - t->tm_wday;
    else
      t->tm_mday -= (t->tm_wday + 1) % 7;

    t->tm_hour = t->tm_min = t->tm_sec = 0;
    t->tm_isdst = -1;
    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeOrdinal (const std::string& token)
{
  if (
      (
       token.length () == 3                  &&
       unicodeLatinDigit (token[0])          &&
       ((token[1] == 's' && token[2] == 't') ||
        (token[1] == 'n' && token[2] == 'd') ||
        (token[1] == 'r' && token[2] == 'd') ||
        (token[1] == 't' && token[2] == 'h'))
      )
      ||
      (
       token.length () == 4                  &&
       unicodeLatinDigit (token[0])          &&
       unicodeLatinDigit (token[1])          &&
       ((token[2] == 's' && token[3] == 't') ||
        (token[2] == 'n' && token[3] == 'd') ||
        (token[2] == 'r' && token[3] == 'd') ||
        (token[2] == 't' && token[3] == 'h'))
      )
     )
  {
    int number;
    std::string ordinal;

    if (unicodeLatinDigit (token[1]))
    {
      number = strtol (token.substr (0, 2).c_str (), nullptr, 10);
      ordinal = lowerCase (token.substr (2));
    }
    else
    {
      number = strtol (token.substr (0, 1).c_str (), nullptr, 10);
      ordinal = lowerCase (token.substr (1));
    }

    // Sanity check.
    if (number <= 31)
    {
      // Make sure the digits and suffix agree.
      int remainder1 = number % 10;
      int remainder2 = number % 100;
      if ((remainder2 != 11 && remainder1 == 1 && ordinal == "st") ||
          (remainder2 != 12 && remainder1 == 2 && ordinal == "nd") ||
          (remainder2 != 13 && remainder1 == 3 && ordinal == "rd") ||
          ((remainder2 == 11 ||
            remainder2 == 12 ||
            remainder2 == 13 ||
            remainder1 == 0 ||
            remainder1 > 3) && ordinal == "th"))
      {
        time_t now = time (nullptr);
        struct tm* t = localtime (&now);

        int y = t->tm_year + 1900;
        int m = t->tm_mon + 1;
        int d = t->tm_mday;

        // If it is this month.
        if (! Datetime::lookForwards ||
            (d < number &&
             number <= daysInMonth (y, m)))
        {
          t->tm_hour = t->tm_min = t->tm_sec = 0;
          t->tm_mon  = m - 1;
          t->tm_mday = number;
          t->tm_year = y - 1900;
          t->tm_isdst = -1;
          _date = mktime (t);
        }
        else
        {
          if (++m > 12)
          {
            m = 1;
            y++;
          }

          t->tm_hour = t->tm_min = t->tm_sec = 0;
          t->tm_mon  = m - 1;
          t->tm_mday = number;
          t->tm_year = y - 1900;
          t->tm_isdst = -1;
          _date = mktime (t);
        }

        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeEaster (const std::string& token)
{
  if (closeEnough ("easter",       token, Datetime::minimumMatchLength) ||
      closeEnough ("eastermonday", token, Datetime::minimumMatchLength) ||
      closeEnough ("ascension",    token, Datetime::minimumMatchLength) ||
      closeEnough ("pentecost",    token, Datetime::minimumMatchLength) ||
      closeEnough ("goodfriday",   token, Datetime::minimumMatchLength))
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);

    easter (t);
    _date = mktime (t);

    // If the result is earlier this year, then recalc for next year.
    if (Datetime::lookForwards && _date < now)
    {
      t = localtime (&now);
      t->tm_year++;
      easter (t);
    }

         if (closeEnough ("goodfriday",   token, Datetime::minimumMatchLength)) t->tm_mday -= 2;

    // DO NOT REMOVE THIS USELESS-LOOKING LINE.
    // It is here to capture an exact match for 'easter', to prevent 'easter'
    // being a partial match for 'eastermonday'.
    else if (closeEnough ("easter",       token, Datetime::minimumMatchLength)) ;
    else if (closeEnough ("eastermonday", token, Datetime::minimumMatchLength)) t->tm_mday += 1;
    else if (closeEnough ("ascension",    token, Datetime::minimumMatchLength)) t->tm_mday += 39;
    else if (closeEnough ("pentecost",    token, Datetime::minimumMatchLength)) t->tm_mday += 49;

    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeMidsommar (const std::string& token)
{
  if (closeEnough ("midsommar", token, Datetime::minimumMatchLength))
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);
    midsommar (t);
    _date = mktime (t);

    // If the result is earlier this year, then recalc for next year.
    if (Datetime::lookForwards &&
        _date < now)
    {
      t = localtime (&now);
      t->tm_year++;
      midsommar (t);
    }

    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::initializeMidsommarafton (const std::string& token)
{
  if (closeEnough ("midsommarafton", token, Datetime::minimumMatchLength) ||
      closeEnough ("juhannus",       token, Datetime::minimumMatchLength))
  {
    time_t now = time (nullptr);
    struct tm* t = localtime (&now);
    midsommarafton (t);
    _date = mktime (t);

    // If the result is earlier this year, then recalc for next year.
    if (Datetime::lookForwards &&
        _date < now)
    {
      t = localtime (&now);
      t->tm_year++;
      midsommarafton (t);
    }

    _date = mktime (t);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// 8am
// 8a
// 8:30am
// 8:30a
// 8:30
//
// \d+ [ : \d{2} ] [ am | a | pm | p ]
//
bool Datetime::initializeInformalTime (const std::string& token)
{
  Pig pig (token);

  int digit = 0;
  bool needDesignator = true;  // Require am/pm.
  bool haveDesignator = false;  // Require am/pm.
  if (pig.getDigit (digit))
  {
    int hours = digit;
    if (pig.getDigit (digit))
      hours = 10 * hours + digit;

    int minutes = 0;
    int seconds = 0;
    if (pig.skip (':') &&
        pig.getDigit2 (minutes))
    {
      if (pig.skip (':') &&
          pig.getDigits (seconds))
      {
        // NOP
      }

      needDesignator = false;
    }

    if (pig.skipLiteral ("am") ||
        pig.skipLiteral ("a"))
    {
      haveDesignator = true;
      if (hours == 12)
        hours = 0;
    }

    else if (pig.skipLiteral ("pm") ||
             pig.skipLiteral ("p"))
    {
      // Note: '12pm is an exception:
      //  12am = 0h
      //  11am = 11h + 12h
      //  12pm = 12h
      //  1pm  = 1h + 12h
      if (hours != 12)
        hours += 12;

      haveDesignator = true;
    }

    if (haveDesignator || ! needDesignator)
    {
      // Midnight today + hours:minutes:seconds.
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      int now_seconds  = (t->tm_hour * 3600) + (t->tm_min * 60) + t->tm_sec;
      int calc_seconds = (hours      * 3600) + (minutes   * 60) + seconds;

      if (Datetime::lookForwards)
      {
        if (calc_seconds < now_seconds)
          ++t->tm_mday;
      }
      else
      {
        if (calc_seconds > now_seconds)
          --t->tm_mday;
      }

      t->tm_hour = hours;
      t->tm_min = minutes;
      t->tm_sec = seconds;
      t->tm_isdst = -1;
      _date = mktime (t);

      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Datetime::easter (struct tm* t) const
{
  int Y = t->tm_year + 1900;
  int a = Y % 19;
  int b = Y / 100;
  int c = Y % 100;
  int d = b / 4;
  int e = b % 4;
  int f = (b + 8) / 25;
  int g = (b - f + 1) / 3;
  int h = (19 * a + b - d - g + 15) % 30;
  int i = c / 4;
  int k = c % 4;
  int L = (32 + 2 * e + 2 * i - h - k) % 7;
  int m = (a + 11 * h + 22 * L) / 451;
  int month = (h + L - 7 * m + 114) / 31;
  int day = ((h + L - 7 * m + 114) % 31) + 1;

  t->tm_isdst = -1;   // Requests that mktime determine summer time effect.
  t->tm_mday  = day;
  t->tm_mon   = month - 1;
  t->tm_year  = Y - 1900;
  t->tm_isdst = -1;
  t->tm_hour = t->tm_min = t->tm_sec = 0;
}

////////////////////////////////////////////////////////////////////////////////
void Datetime::midsommar (struct tm* t) const
{
  t->tm_mon = 5;                          // June.
  t->tm_mday = 20;                        // Saturday after 20th.
  t->tm_hour = t->tm_min = t->tm_sec = 0; // Midnight.
  t->tm_isdst = -1;                       // Probably DST, but check.

  time_t then = mktime (t);               // Obtain the weekday of June 20th.
  struct tm* mid = localtime (&then);
  t->tm_mday += 6 - mid->tm_wday;         // How many days after 20th.
}

////////////////////////////////////////////////////////////////////////////////
void Datetime::midsommarafton (struct tm* t) const
{
  t->tm_mon = 5;                          // June.
  t->tm_mday = 19;                        // Saturday after 20th.
  t->tm_hour = t->tm_min = t->tm_sec = 0; // Midnight.
  t->tm_isdst = -1;                       // Probably DST, but check.

  time_t then = mktime (t);               // Obtain the weekday of June 19th.
  struct tm* mid = localtime (&then);
  t->tm_mday += 5 - mid->tm_wday;         // How many days after 19th.
}

////////////////////////////////////////////////////////////////////////////////
// Suggested date expressions:
//   {ordinal} {day} in|of {month}
//   last|past|next|this {day}
//   last|past|next|this {month}
//   last|past|next|this week
//   last|past|next|this month
//   last|past|next|this weekend
//   last|past|next|this year
//   {day} last|past|next|this week
//   {day} [at] {time}
//   {time} {day}
//
// Candidates:
//   <dayname> <time>
//   <time>
//   tue 9am
//   Friday before easter
//   3 days before eom
//   in the morning
//   am|pm
//   4pm
//   noon
//   midnight
//   tomorrow in one year
//   in two weeks
//   2 weeks from now
//   2 weeks ago tuesday
//   thursday in 2 weeks
//   last day next month
//   10 days from today
//   thursday before last weekend in may
//   friday last full week in may
//   3rd wednesday this month
//   3 weeks after 2nd tuesday next month
//   100 days from the beginning of the month
//   10 days after last monday
//   sunday in the evening
//   in 6 hours
//   6 in the morning
//   kl 18
//   feb 11
//   11 feb
//   2011-02-08
//   11/19/2011
//   next business day
//   new moon
//   full moon
//   in 28 days
//   3rd quarter
//   week 23
//   {number} {unit}
//   - {number} {unit}
//   {ordinal} {unit} in {larger-unit}
//   end of day tomorrow
//   end of {day}
//   by {day}
//   first thing {day}
//
bool Datetime::initializeFoo (const std::vector <std::string>&)
{

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Validation via simple range checking.
bool Datetime::validate ()
{
  // _year;
  if ((_year    && (_year    <   1900 || _year    >                                  2200)) ||
      (_month   && (_month   <      1 || _month   >                                    12)) ||
      (_week    && (_week    <      1 || _week    >                                    53)) ||
      (_weekday && (_weekday <      0 || _weekday >                                     6)) ||
      (_julian  && (_julian  <      1 || _julian  >          Datetime::daysInYear (_year))) ||
      (_day     && (_day     <      1 || _day     > Datetime::daysInMonth (_year, _month))) ||
      (_seconds && (_seconds <      1 || _seconds >                                 86400)) ||
      (_offset  && (_offset  < -86400 || _offset  >                                 86400)))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// int tm_sec;       seconds (0 - 60)
// int tm_min;       minutes (0 - 59)
// int tm_hour;      hours (0 - 23)
// int tm_mday;      day of month (1 - 31)
// int tm_mon;       month of year (0 - 11)
// int tm_year;      year - 1900
// int tm_wday;      day of week (Sunday = 0)
// int tm_yday;      day of year (0 - 365)
// int tm_isdst;     is summer time in effect?
// char *tm_zone;    abbreviation of timezone name
// long tm_gmtoff;   offset from UTC in seconds
void Datetime::resolve ()
{
  // Don't touch the original values.
  int year    = _year;
  int month   = _month;
  int week    = _week;
  int weekday = _weekday;
  int julian  = _julian;
  int day     = _day;
  int seconds = _seconds;
  int offset  = _offset;
  bool utc    = _utc;

  // Get current time.
  time_t now = time (nullptr);

  // A UTC offset needs to be accommodated.  Once the offset is subtracted,
  // only local and UTC times remain.
  if (offset)
  {
    seconds -= offset;
    now -= offset;
    utc = true;
  }

  // Get 'now' in the relevant location.
  struct tm* t_now = utc ? gmtime (&now) : localtime (&now);

  int seconds_now = (t_now->tm_hour * 3600) +
                    (t_now->tm_min  *   60) +
                     t_now->tm_sec;

  // Project forward one day if the specified seconds are earlier in the day
  // than the current seconds.
  // TODO This does not cover the inverse case of subtracting 86400.
  if (Datetime::lookForwards &&
      year    == 0           &&
      month   == 0           &&
      day     == 0           &&
      week    == 0           &&
      weekday == 0           &&
      seconds < seconds_now)
  {
    seconds += 86400;
  }

  // Convert week + weekday --> julian.
  if (week)
  {
    julian = (week * 7) + weekday - dayOfWeek (year, 1, 4) - 3;
  }

  // Provide default values for year, month, day.
  else
  {
    // Default values for year, month, day:
    //
    // y   m   d  -->  y   m   d
    // y   m   -  -->  y   m   1
    // y   -   -  -->  y   1   1
    // -   -   -  -->  now now now
    //
    if (year == 0)
    {
      year  = t_now->tm_year + 1900;
      month = t_now->tm_mon + 1;
      day   = t_now->tm_mday;
    }
    else
    {
      if (month == 0)
      {
        month = 1;
        day   = 1;
      }
      else if (day == 0)
        day = 1;
    }
  }

  if (julian)
  {
    month = 1;
    day = julian;
  }

  struct tm t {};
  t.tm_isdst = -1;  // Requests that mktime/gmtime determine summer time effect.
  t.tm_year = year - 1900;
  t.tm_mon = month - 1;
  t.tm_mday = day;

  if (seconds > 86400)
  {
    int days = seconds / 86400;
    t.tm_mday += days;
    seconds %= 86400;
  }

  t.tm_hour = seconds / 3600;
  t.tm_min = (seconds % 3600) / 60;
  t.tm_sec = seconds % 60;

  _date = utc ? timegm (&t) : mktime (&t);
}

////////////////////////////////////////////////////////////////////////////////
time_t Datetime::toEpoch () const
{
  return _date;
}

////////////////////////////////////////////////////////////////////////////////
std::string Datetime::toEpochString () const
{
  std::stringstream epoch;
  epoch << _date;
  return epoch.str ();
}

////////////////////////////////////////////////////////////////////////////////
// 19980119T070000Z =  YYYYMMDDThhmmssZ
std::string Datetime::toISO () const
{
  struct tm* t = gmtime (&_date);

  std::stringstream iso;
  iso << std::setw (4) << std::setfill ('0') << t->tm_year + 1900
      << std::setw (2) << std::setfill ('0') << t->tm_mon + 1
      << std::setw (2) << std::setfill ('0') << t->tm_mday
      << "T"
      << std::setw (2) << std::setfill ('0') << t->tm_hour
      << std::setw (2) << std::setfill ('0') << t->tm_min
      << std::setw (2) << std::setfill ('0') << t->tm_sec
      << "Z";

  return iso.str ();
}

////////////////////////////////////////////////////////////////////////////////
// 1998-01-19T07:00:00 =  YYYY-MM-DDThh:mm:ss
std::string Datetime::toISOLocalExtended () const
{
  struct tm* t = localtime (&_date);

  std::stringstream iso;
  iso << std::setw (4) << std::setfill ('0') << t->tm_year + 1900
      << "-"
      << std::setw (2) << std::setfill ('0') << t->tm_mon + 1
      << "-"
      << std::setw (2) << std::setfill ('0') << t->tm_mday
      << "T"
      << std::setw (2) << std::setfill ('0') << t->tm_hour
      << ":"
      << std::setw (2) << std::setfill ('0') << t->tm_min
      << ":"
      << std::setw (2) << std::setfill ('0') << t->tm_sec;

  return iso.str ();
}

////////////////////////////////////////////////////////////////////////////////
double Datetime::toJulian () const
{
  return (_date / 86400.0) + 2440587.5;
}

////////////////////////////////////////////////////////////////////////////////
void Datetime::toYMD (int& y, int& m, int& d) const
{
  struct tm* t = localtime (&_date);

  m = t->tm_mon + 1;
  d = t->tm_mday;
  y = t->tm_year + 1900;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Datetime::toString (const std::string& format) const
{
  std::stringstream formatted;
  for (unsigned int i = 0; i < format.length (); ++i)
  {
    int c = format[i];
    switch (c)
    {
    case 'm': formatted                                        << month ();               break;
    case 'M': formatted << std::setw (2) << std::setfill ('0') << month ();               break;
    case 'd': formatted                                        << day ();                 break;
    case 'D': formatted << std::setw (2) << std::setfill ('0') << day ();                 break;
    case 'y': formatted << std::setw (2) << std::setfill ('0') << (year () % 100);        break;
    case 'Y': formatted                                        << year ();                break;
    case 'a': formatted                                        << Datetime::dayNameShort (dayOfWeek ()); break;
    case 'A': formatted                                        << Datetime::dayName (dayOfWeek ());      break;
    case 'b': formatted                                        << Datetime::monthNameShort (month ());   break;
    case 'B': formatted                                        << Datetime::monthName (month ());        break;
    case 'v': formatted                                        << week ();                break;
    case 'V': formatted << std::setw (2) << std::setfill ('0') << week ();                break;
    case 'h': formatted                                        << hour ();                break;
    case 'H': formatted << std::setw (2) << std::setfill ('0') << hour ();                break;
    case 'n': formatted                                        << minute ();              break;
    case 'N': formatted << std::setw (2) << std::setfill ('0') << minute ();              break;
    case 's': formatted                                        << second ();              break;
    case 'S': formatted << std::setw (2) << std::setfill ('0') << second ();              break;
    case 'j': formatted                                        << dayOfYear ();           break;
    case 'J': formatted << std::setw (3) << std::setfill ('0') << dayOfYear ();           break;
    case 'w': formatted                                        << dayOfWeek ();           break;
    default:  formatted                                        << static_cast <char> (c); break;
    }
  }

  return formatted.str ();
}

////////////////////////////////////////////////////////////////////////////////
Datetime Datetime::startOfDay () const
{
  return Datetime (year (), month (), day ());
}

////////////////////////////////////////////////////////////////////////////////
Datetime Datetime::startOfWeek () const
{
  Datetime sow (_date);
  sow -= (dayOfWeek () * 86400);
  return Datetime (sow.year (), sow.month (), sow.day ());
}

////////////////////////////////////////////////////////////////////////////////
Datetime Datetime::startOfMonth () const
{
  return Datetime (year (), month (), 1);
}

////////////////////////////////////////////////////////////////////////////////
Datetime Datetime::startOfYear () const
{
  return Datetime (year (), 1, 1);
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::valid (const std::string& input, const std::string& format)
{
  try
  {
    Datetime test (input, format);
  }

  catch (...)
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::valid (
  const int y, const int m, const int d,
  const int hr, const int mi, const int se)
{
  if (hr < 0 || hr > 24)
    return false;

  if (mi < 0 || mi > 59)
    return false;

  if (se < 0 || se > 59)
    return false;

  if (hr == 24 &&
      (mi != 0 ||
       se != 0))
    return false;

  return Datetime::valid (y, m, d);
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::valid (const int y, const int m, const int d)
{
  // Check that the year is valid.
  if (y < 0)
    return false;

  // Check that the month is valid.
  if (m < 1 || m > 12)
    return false;

  // Finally check that the days fall within the acceptable range for this
  // month, and whether or not this is a leap year.
  if (d < 1 || d > Datetime::daysInMonth (y, m))
    return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Julian
bool Datetime::valid (const int y, const int d)
{
  // Check that the year is valid.
  if (y < 0)
    return false;

  if (d < 1 || d > Datetime::daysInYear (y))
    return false;

  return true;
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
int Datetime::daysInMonth (int year, int month)
{
  // Protect against arguments being passed in the wrong order.
  assert (year >= 1969 && year < 2100);
  assert (month >= 1 && month <= 31);

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
// Static
std::string Datetime::monthName (int month)
{
  assert (month > 0);
  assert (month <= 12);
  return upperCaseFirst (monthNames[month - 1]);
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string Datetime::monthNameShort (int month)
{
  assert (month > 0);
  assert (month <= 12);
  return upperCaseFirst (monthNames[month - 1]).substr (0, 3);
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string Datetime::dayName (int dow)
{
  assert (dow >= 0);
  assert (dow <= 6);
  return upperCaseFirst (dayNames[dow]);
}

////////////////////////////////////////////////////////////////////////////////
// Static
std::string Datetime::dayNameShort (int dow)
{
  assert (dow >= 0);
  assert (dow <= 6);
  return upperCaseFirst (dayNames[dow]).substr (0, 3);
}

////////////////////////////////////////////////////////////////////////////////
// Static
int Datetime::dayOfWeek (const std::string& input)
{
  if (Datetime::minimumMatchLength== 0)
    Datetime::minimumMatchLength = 3;

  for (unsigned int i = 0; i < dayNames.size (); ++i)
    if (closeEnough (dayNames[i], input, Datetime::minimumMatchLength))
       return i;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Using Zeller's Congruence.
// Static
int Datetime::dayOfWeek (int year, int month, int day)
{
  int adj = (14 - month) / 12;
  int m = month + 12 * adj - 2;
  int y = year - adj;
  return (day + (13 * m - 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
}

////////////////////////////////////////////////////////////////////////////////
// Static
int Datetime::monthOfYear (const std::string& input)
{
  if (Datetime::minimumMatchLength== 0)
    Datetime::minimumMatchLength = 3;

  for (unsigned int i = 0; i < monthNames.size (); ++i)
    if (closeEnough (monthNames[i], input, Datetime::minimumMatchLength))
       return i + 1;

  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Static
int Datetime::length (const std::string& format)
{
  int len = 0;
  for (auto& i : format)
  {
    switch (i)
    {
    case 'm':
    case 'M':
    case 'd':
    case 'D':
    case 'y':
    case 'v':
    case 'V':
    case 'h':
    case 'H':
    case 'n':
    case 'N':
    case 's':
    case 'S': len += 2;  break;
    case 'b':
    case 'j':
    case 'J':
    case 'a': len += 3;  break;
    case 'Y': len += 4;  break;
    case 'A':
    case 'B': len += 10; break;

    // Calculate the width, don't assume a single character width.
    default:  len += mk_wcwidth (i); break;
    }
  }

  return len;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::month () const
{
  struct tm* t = localtime (&_date);
  return t->tm_mon + 1;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::week () const
{
  struct tm* t = localtime (&_date);

  char weekStr[3];
  if (Datetime::weekstart == 0)
    strftime (weekStr, sizeof (weekStr), "%U", t);
  else if (Datetime::weekstart == 1)
    strftime (weekStr, sizeof (weekStr), "%V", t);
  else
    throw std::string ("The week may only start on a Sunday or Monday.");

  int weekNumber = strtol (weekStr, nullptr, 10);
  if (weekstart == 0)
    weekNumber += 1;

  return weekNumber;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::day () const
{
  struct tm* t = localtime (&_date);
  return t->tm_mday;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::year () const
{
  struct tm* t = localtime (&_date);
  return t->tm_year + 1900;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::dayOfWeek () const
{
  struct tm* t = localtime (&_date);
  return t->tm_wday;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::dayOfYear () const
{
  struct tm* t = localtime (&_date);
  return t->tm_yday + 1;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::hour () const
{
  struct tm* t = localtime (&_date);
  return t->tm_hour;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::minute () const
{
  struct tm* t = localtime (&_date);
  return t->tm_min;
}

////////////////////////////////////////////////////////////////////////////////
int Datetime::second () const
{
  struct tm* t = localtime (&_date);
  return t->tm_sec;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::operator== (const Datetime& rhs) const
{
  return rhs._date == _date;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::operator!= (const Datetime& rhs) const
{
  return rhs._date != _date;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::operator< (const Datetime& rhs) const
{
  return _date < rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::operator> (const Datetime& rhs) const
{
  return _date > rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::operator<= (const Datetime& rhs) const
{
  return _date <= rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::operator>= (const Datetime& rhs) const
{
  return _date >= rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameHour (const Datetime& rhs) const
{
  return year ()  == rhs.year ()  &&
         month () == rhs.month () &&
         day ()   == rhs.day ()   &&
         hour ()  == rhs.hour ();
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameDay (const Datetime& rhs) const
{
  return year ()  == rhs.year ()  &&
         month () == rhs.month () &&
         day ()   == rhs.day ();
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameWeek (const Datetime& rhs) const
{
  return year () == rhs.year () &&
         week () == rhs.week ();
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameMonth (const Datetime& rhs) const
{
  return year ()  == rhs.year () &&
         month () == rhs.month ();
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameQuarter (const Datetime& rhs) const
{
  return year () == rhs.year () &&
         ((month () - 1) / 3) == ((rhs.month () - 1) / 3);
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameYear (const Datetime& rhs) const
{
  return year () == rhs.year ();
}

////////////////////////////////////////////////////////////////////////////////
Datetime Datetime::operator+ (const int delta)
{
  return Datetime (_date + delta);
}

////////////////////////////////////////////////////////////////////////////////
Datetime Datetime::operator- (const int delta)
{
  return Datetime (_date - delta);
}

////////////////////////////////////////////////////////////////////////////////
Datetime& Datetime::operator+= (const int delta)
{
  _date += (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
Datetime& Datetime::operator-= (const int delta)
{
  _date -= (time_t) delta;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
time_t Datetime::operator- (const Datetime& rhs)
{
  return _date - rhs._date;
}

////////////////////////////////////////////////////////////////////////////////
// Prefix decrement by one day.
void Datetime::operator-- ()
{
  Datetime yesterday = startOfDay () - 1;
  yesterday = Datetime (yesterday.year (),
                        yesterday.month (),
                        yesterday.day (),
                        hour (),
                        minute (),
                        second ());
  _date = yesterday._date;
}

////////////////////////////////////////////////////////////////////////////////
// Postfix decrement by one day.
void Datetime::operator-- (int)
{
  Datetime yesterday = startOfDay () - 1;
  yesterday = Datetime (yesterday.year (),
                        yesterday.month (),
                        yesterday.day (),
                        hour (),
                        minute (),
                        second ());
  _date = yesterday._date;
}

////////////////////////////////////////////////////////////////////////////////
// Prefix increment by one day.
void Datetime::operator++ ()
{
  Datetime tomorrow = (startOfDay () + 90001).startOfDay ();
  tomorrow = Datetime (tomorrow.year (),
                       tomorrow.month (),
                       tomorrow.day (),
                       hour (),
                       minute (),
                       second ());
  _date = tomorrow._date;
}

////////////////////////////////////////////////////////////////////////////////
// Postfix increment by one day.
void Datetime::operator++ (int)
{
  Datetime tomorrow = (startOfDay () + 90001).startOfDay ();
  tomorrow = Datetime (tomorrow.year (),
                       tomorrow.month (),
                       tomorrow.day (),
                       hour (),
                       minute (),
                       second ());
  _date = tomorrow._date;
}

////////////////////////////////////////////////////////////////////////////////
/*
std::string Datetime::dump () const
{
  std::stringstream s;
  s << "Datetime"
    << " y"   << _year
    << " m"   << _month
    << " w"   << _week
    << " wd"  << _weekday
    << " j"   << _julian
    << " d"   << _day
    << " s"   << _seconds
    << " off" << _offset
    << " utc" << _utc
    << " ="   << _date
    << "  "   << (_date ? toISO () : "");

  return s.str ();
}
*/

////////////////////////////////////////////////////////////////////////////////
