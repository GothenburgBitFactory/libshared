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
#include <Datetime.h>
#include <algorithm>
#include <iostream>
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
bool Datetime::isoEnabled            = true;
bool Datetime::standaloneDateEnabled = true;
bool Datetime::standaloneTimeEnabled = true;

// When true,
// - HH:MM:SS is assumed to be today if the time > now, otherwise it is assumed to be tomorrow.
// - day name is converted to date relative to now
// - 1st, 2nd, 3rd, ... is converted to date relative to now
// When false,
// - HH:MM:SS is always today.
// - day name is always converted to date before now
// - 1st, 2nd, 3rd, ... is always converted to date before now
bool Datetime::timeRelative = true;

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
  assert (y >= 1969 && y <= 9999);
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
  assert (y >= 1969 && y <= 9999);
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

  auto checkpoint = pig.cursor ();

  // Parse epoch first, as it's the most common scenario.
  if (parse_epoch (pig))
  {
    // ::validate and ::resolve are not needed in this case.
    start = pig.cursor ();
    return true;
  }

  if (parse_formatted (pig, format))
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
  if (parse_date_time_ext   (pig) || // Strictest first.
      parse_date_time       (pig) ||
      (Datetime::isoEnabled &&
       (                                    parse_date_ext      (pig)  ||
        (Datetime::standaloneDateEnabled && parse_date          (pig)) ||
                                            parse_time_utc_ext  (pig)  ||
                                            parse_time_utc      (pig)  ||
                                            parse_time_off_ext  (pig)  ||
                                            parse_time_ext      (pig)  ||
        (Datetime::standaloneTimeEnabled && parse_time          (pig)) || // Time last, as it is the most permissive.
        (Datetime::standaloneTimeEnabled && parse_time_off      (pig))
       )
      )
     )
  {
    // Check the values and determine time_t.
    if (validate ())
    {
      start = pig.cursor ();
      resolve ();
      return true;
    }
  }

  pig.restoreTo (checkpoint);

  if (parse_named (pig))
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
// Note how these are all single words.
//
// Examples and descriptions, assuming now == 2017-03-05T12:34:56.
//
//                  Example              Notes
//                  -------------------  ------------------
//   now            2017-03-05T12:34:56  Unaffected
//   yesterday      2017-03-04T00:00:00  Unaffected
//   today          2017-03-05T00:00:00  Unaffected
//   tomorrow       2017-03-06T00:00:00  Unaffected
//   <ordinal> 12th 2017-03-12T00:00:00
//   <day> monday   2017-03-06T00:00:00
//   <month> april  2017-04-01T00:00:00
//   later          9999-12-30T00:00:00  Unaffected
//   someday        9999-12-30T00:00:00  Unaffected
//   sopd           2017-03-04T00:00:00  Unaffected
//   sod            2017-03-05T00:00:00  Unaffected
//   sond           2017-03-06T00:00:00  Unaffected
//   eopd           2017-03-04T23:59:59  Unaffected
//   eod            2017-03-05T23:59:59  Unaffected
//   eond           2017-03-06T23:59:59  Unaffected
//   sopw           2017-02-26T00:00:00  Unaffected
//   sow            2017-03-05T00:00:00  Unaffected
//   sonw           2017-03-12T00:00:00  Unaffected
//   eopw           2017-02-26T23:59:59  Unaffected
//   eow            2017-03-05T23:59:59  Unaffected
//   eonw           2017-03-12T23:59:59  Unaffected
//   sopww          2017-02-27T00:00:00  Unaffected
//   soww           2017-03-06T00:00:00
//   sonww          2017-03-06T00:00:00  Unaffected
//   eopww          2017-03-03T23:59:59  Unaffected
//   eoww           2017-03-10T23:59:59
//   eonww          2017-03-17T23:59:59  Unaffected
//   sopm           2017-02-01T00:00:00  Unaffected
//   som            2017-03-01T00:00:00  Unaffected
//   sonm           2017-04-01T00:00:00  Unaffected
//   eopm           2017-02-28T23:59:59  Unaffected
//   eom            2017-03-31T23:59:59  Unaffected
//   eonm           2017-04-30T23:59:59  Unaffected
//   sopq           2017-10-01T00:00:00  Unaffected
//   soq            2017-01-01T00:00:00  Unaffected
//   sonq           2017-04-01T00:00:00  Unaffected
//   eopq           2016-12-31T23:59:59  Unaffected
//   eoq            2017-03-31T23:59:59  Unaffected
//   eonq           2017-06-30T23:59:59  Unaffected
//   sopy           2016-01-01T00:00:00  Unaffected
//   soy            2017-01-01T00:00:00  Unaffected
//   sony           2018-01-01T00:00:00  Unaffected
//   eopy           2016-12-31T23:59:59  Unaffected
//   eoy            2017-12-31T23:59:59  Unaffected
//   eony           2018-12-31T23:59:59  Unaffected
//   easter         2017-04-16T00:00:00
//   eastermonday   2017-04-16T00:00:00
//   ascension      2017-05-25T00:00:00
//   pentecost      2017-06-04T00:00:00
//   goodfriday     2017-04-14T00:00:00
//   midsommar      2017-06-24T00:00:00  midnight, 1st Saturday after 20th June
//   midsommarafton 2017-06-23T00:00:00  midnight, 1st Friday after 19th June
//   juhannus       2017-06-23T00:00:00  midnight, 1st Friday after 19th June
//
bool Datetime::parse_named (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  // Experimental handling of date phrases, such as "first monday in march".
  // Note that this requires that phrases are deliminted by EOS or WS.
  std::string token;
  std::vector <std::string> tokens;
  while (pig.getUntilWS (token))
  {
    tokens.push_back (token);
    if (! pig.skipWS ())
      break;
  }

/*
  // This group contains "1st monday ..." which must be processed before
  // initializeOrdinal below.
  if (initializeNthDayInMonth (tokens))
  {
    return true;
  }
*/

  // Restoration necessary because of the tokenization.
  pig.restoreTo (checkpoint);

  if (initializeNow            (pig) ||
      initializeYesterday      (pig) ||
      initializeToday          (pig) ||
      initializeTomorrow       (pig) ||
      initializeOrdinal        (pig) ||
      initializeDayName        (pig) ||
      initializeMonthName      (pig) ||
      initializeLater          (pig) ||
      initializeSopd           (pig) ||
      initializeSod            (pig) ||
      initializeSond           (pig) ||
      initializeEopd           (pig) ||
      initializeEod            (pig) ||
      initializeEond           (pig) ||
      initializeSopw           (pig) ||
      initializeSow            (pig) ||
      initializeSonw           (pig) ||
      initializeEopw           (pig) ||
      initializeEow            (pig) ||
      initializeEonw           (pig) ||
      initializeSopww          (pig) ||  // Must appear after sopw
      initializeSonww          (pig) ||  // Must appear after sonw
      initializeSoww           (pig) ||  // Must appear after sow
      initializeEopww          (pig) ||  // Must appear after eopw
      initializeEonww          (pig) ||  // Must appear after eonw
      initializeEoww           (pig) ||  // Must appear after eow
      initializeSopm           (pig) ||
      initializeSom            (pig) ||
      initializeSonm           (pig) ||
      initializeEopm           (pig) ||
      initializeEom            (pig) ||
      initializeEonm           (pig) ||
      initializeSopq           (pig) ||
      initializeSoq            (pig) ||
      initializeSonq           (pig) ||
      initializeEopq           (pig) ||
      initializeEoq            (pig) ||
      initializeEonq           (pig) ||
      initializeSopy           (pig) ||
      initializeSoy            (pig) ||
      initializeSony           (pig) ||
      initializeEopy           (pig) ||
      initializeEoy            (pig) ||
      initializeEony           (pig) ||
      initializeEaster         (pig) ||
      initializeMidsommar      (pig) ||
      initializeMidsommarafton (pig) ||
      initializeInformalTime   (pig))
  {
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Valid epoch values are unsigned integers after 1980-01-01T00:00:00Z. This
// restriction means that '12' will not be identified as an epoch date.
bool Datetime::parse_epoch (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  long long epoch {};
  if (pig.getDigits (epoch)             &&
      ! unicodeLatinAlpha (pig.peek ()) &&
      epoch >= 315532800                &&
      epoch < 253402293599 )  // 9999-12-31, 23:59:59 AoE
  {
    _date = static_cast <time_t> (epoch);
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// date_ext 'T' time_utc_ext 'Z'
// date_ext 'T' time_off_ext
// date_ext 'T' time_ext
bool Datetime::parse_date_time_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_date_ext (pig) &&
      pig.skip ('T')       &&
      (parse_time_utc_ext (pig) ||
       parse_time_off_ext (pig) ||
       parse_time_ext     (pig)))
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

    pig.restoreTo (checkpointYear);

    if (parse_julian (pig, julian) &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      _year = year;
      _julian = julian;
      return true;
    }

    pig.restoreTo (checkpointYear);

    if (parse_month (pig, month) &&
        pig.peek () != '-'       &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      _year = year;
      _month = month;
      _day = 1;
      return true;
    }
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

    if (parse_off_hour (pig, hour))
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
bool Datetime::parse_time_ext (Pig& pig, bool terminated)
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
          ! unicodeLatinDigit (pig.peek ()) &&
          (! terminated || (pig.peek () != '-' && pig.peek () != '+')))
      {
        _seconds = (hour * 3600) + (minute * 60) + second;
        return true;
      }

      pig.restoreTo (checkpoint);
      return false;
    }

    auto following = pig.peek ();
    if (! unicodeLatinDigit (following)    &&
        (! terminated || (following != '+' && following != '-')) &&
        following != 'A'                   &&
        following != 'a'                   &&
        following != 'P'                   &&
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

  if (parse_time_ext (pig, false) &&
      pig.skip ('Z'))
  {
    if (! unicodeLatinDigit (pig.peek ()))
    {
      _utc = true;
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time-ext off-ext
bool Datetime::parse_time_off_ext (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_time_ext (pig, false) &&
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

    pig.restoreTo (checkpointYear);

    if (parse_julian (pig, julian) &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      _year = year;
      _julian = julian;
      return true;
    }

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
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <time> Z
bool Datetime::parse_time_utc (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_time (pig, false) &&
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
// <time><off>
bool Datetime::parse_time_off (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (parse_time (pig, false) &&
      parse_off (pig))
  {
    auto terminator = pig.peek ();
    if (terminator != '-' && ! unicodeLatinDigit (terminator))
    {
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// hhmmss
// hhmm
bool Datetime::parse_time (Pig& pig, bool terminated)
{
  auto checkpoint = pig.cursor ();

  int hour {};
  int minute {};
  if (parse_hour (pig, hour) &&
      parse_minute (pig, minute))
  {
    int second {};
    parse_second (pig, second);

    auto terminator = pig.peek ();
    if (! terminated ||
        (! unicodeLatinDigit (terminator) && terminator != '-' && terminator != '+'))
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
// now [ !<alpha> && !<digit> ]
bool Datetime::initializeNow (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("now"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      _date = time (nullptr);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// yesterday/abbrev  [ !<alpha> && !<digit> ]
bool Datetime::initializeYesterday (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::string token;
  if (pig.skipPartial ("yesterday", token) &&
      token.length () >= static_cast <std::string::size_type> (Datetime::minimumMatchLength))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      t->tm_mday -= 1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// today/abbrev  [ !<alpha> && !<digit> ]
bool Datetime::initializeToday (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::string token;
  if (pig.skipPartial ("today", token) &&
      token.length () >= static_cast <std::string::size_type> (Datetime::minimumMatchLength))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      _date = mktime (t);

      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// tomcorrow/abbrev  [ !<alpha> && !<digit> ]
bool Datetime::initializeTomorrow (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::string token;
  if (pig.skipPartial ("tomorrow", token) &&
      token.length () >= static_cast <std::string::size_type> (Datetime::minimumMatchLength))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday++;
      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// <digit>+ [ "st" | "nd" | "rd" | "th" ] [ !<alpha> && !<digit> ]
bool Datetime::initializeOrdinal (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  long long number = 0;
  if (pig.getDigits (number) &&
      number > 0             &&
      number <= 31)
  {
    int character1;
    int character2;
    if (pig.getCharacter (character1)     &&
        pig.getCharacter (character2)     &&
        ! unicodeLatinAlpha (pig.peek ()) &&
        ! unicodeLatinDigit (pig.peek ()))
    {
      int remainder1 = number % 10;
      int remainder2 = number % 100;
      if ((remainder2 != 11 && remainder1 == 1 && character1 == 's' && character2 == 't') ||
          (remainder2 != 12 && remainder1 == 2 && character1 == 'n' && character2 == 'd') ||
          (remainder2 != 13 && remainder1 == 3 && character1 == 'r' && character2 == 'd') ||
          ((remainder2 == 11 ||
            remainder2 == 12 ||
            remainder2 == 13 ||
            remainder1 == 0 ||
            remainder1 > 3) && character1 == 't' && character2 == 'h'))
      {
        time_t now = time (nullptr);
        struct tm* t = localtime (&now);

        int y = t->tm_year + 1900;
        int m = t->tm_mon + 1;
        int d = t->tm_mday;

        if (timeRelative && (1 <= number && number <= d))
        {
          if (++m > 12)
          {
            m = 1;
            y++;
          }
        }
        else if (!timeRelative && (d < number && number <= daysInMonth (y, m)))
        {
          if (--m < 1)
          {
            m = 12;
            y--;
          }
        }

        t->tm_hour = t->tm_min = t->tm_sec = 0;
        t->tm_mon  = m - 1;
        t->tm_mday = number;
        t->tm_year = y - 1900;
        t->tm_isdst = -1;

        _date = mktime (t);

        return true;
      }
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sunday/abbrev [ !<alpha> && !<digit> && !: && != ]
bool Datetime::initializeDayName (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::string token;
  for (int day = 0; day <= 7; ++day)   // Deliberate <= so that 'sunday' is either 0 or 7.
  {
    if (pig.skipPartial (dayNames[day % 7], token, true) &&
        token.length () >= static_cast <std::string::size_type> (Datetime::minimumMatchLength))
    {
      auto following = pig.peek ();
      if (! unicodeLatinAlpha (following) &&
          ! unicodeLatinDigit (following) &&
          following != ':' &&
          following != '=')
      {
        time_t now = time (nullptr);
        struct tm* t = localtime (&now);

        if (t->tm_wday >= day)
        {
          t->tm_mday += day - t->tm_wday + (timeRelative ? 7 : 0);
        }
        else
        {
          t->tm_mday += day - t->tm_wday - (timeRelative ? 0 : 7);
        }

        t->tm_hour = t->tm_min = t->tm_sec = 0;
        t->tm_isdst = -1;
        _date = mktime (t);
        return true;
      }
    }

    pig.restoreTo (checkpoint);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// january/abbrev [ !<alpha> && !<digit> && !: && != ]
bool Datetime::initializeMonthName (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::string token;
  for (int month = 0; month < 12; ++month)
  {
    if (pig.skipPartial (monthNames[month], token, true) &&
        token.length () >= static_cast <std::string::size_type> (Datetime::minimumMatchLength))
    {
      auto following = pig.peek ();
      if (! unicodeLatinAlpha (following) &&
          ! unicodeLatinDigit (following) &&
          following != ':' &&
          following != '=')
      {
        time_t now = time (nullptr);
        struct tm* t = localtime (&now);

        if (t->tm_mon >= month && timeRelative)
        {
          t->tm_year++;
        }

        t->tm_mon = month;
        t->tm_mday = 1;
        t->tm_hour = t->tm_min = t->tm_sec = 0;
        t->tm_isdst = -1;
        _date = mktime (t);
        return true;
      }
    }

    pig.restoreTo (checkpoint);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// later/abbrev  [ !<alpha> && !<digit> ]
// someday/abbrev  [ !<alpha> && !<digit> ]
bool Datetime::initializeLater (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::string token;
  if ((pig.skipPartial ("later", token) &&
      token.length () >= static_cast <std::string::size_type> (Datetime::minimumMatchLength))

      ||

     (pig.skipPartial ("someday", token) &&
      token.length () >= static_cast <std::string::size_type> (std::max (Datetime::minimumMatchLength, 4))))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_year = 8099;  // Year 9999
      t->tm_mon = 11;
      t->tm_mday = 30;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sopd [ !<alpha> && !<digit> ]
bool Datetime::initializeSopd (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sopd"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      t->tm_mday -= 1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sod [ !<alpha> && !<digit> ]
bool Datetime::initializeSod (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sod"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sond [ !<alpha> && !<digit> ]
bool Datetime::initializeSond (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sond"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday++;
      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eopd [ !<alpha> && !<digit> ]
bool Datetime::initializeEopd (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eopd"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eod [ !<alpha> && !<digit> ]
bool Datetime::initializeEod (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eod"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday++;
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eond [ !<alpha> && !<digit> ]
bool Datetime::initializeEond (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eond"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday += 2;
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sopw [ !<alpha> && !<digit> ]
bool Datetime::initializeSopw (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sopw"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);
      t->tm_hour = t->tm_min = t->tm_sec = 0;

      int extra = (t->tm_wday + 6) % 7;
      t->tm_mday -= extra;
      t->tm_mday -= 7;

      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sow [ !<alpha> && !<digit> ]
bool Datetime::initializeSow (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sow"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
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
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sonw [ !<alpha> && !<digit> ]
bool Datetime::initializeSonw (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sonw"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);
      t->tm_hour = t->tm_min = t->tm_sec = 0;

      int extra = (t->tm_wday + 6) % 7;
      t->tm_mday -= extra;
      t->tm_mday += 7;

      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eopw [ !<alpha> && !<digit> ]
bool Datetime::initializeEopw (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eopw"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;

      int extra = (t->tm_wday + 6) % 7;
      t->tm_mday -= extra;

      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eow [ !<alpha> && !<digit> ]
bool Datetime::initializeEow (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eow"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;

      int extra = (t->tm_wday + 6) % 7;
      t->tm_mday -= extra;
      t->tm_mday += 7;

      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eonw [ !<alpha> && !<digit> ]
bool Datetime::initializeEonw (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eonw"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;

      int extra = (t->tm_wday + 6) % 7;
      t->tm_mday += 14 - extra;

      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sopww [ !<alpha> && !<digit> ]
bool Datetime::initializeSopww (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sopww"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday += -6 - t->tm_wday;
      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// soww [ !<alpha> && !<digit> ]
bool Datetime::initializeSoww (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("soww"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday += 1 - t->tm_wday;
      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sonww [ !<alpha> && !<digit> ]
bool Datetime::initializeSonww (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sonww"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday += 8 - t->tm_wday;
      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eopww [ !<alpha> && !<digit> ]
bool Datetime::initializeEopww (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eopww"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday -= (t->tm_wday + 1) % 7;
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eoww [ !<alpha> && !<digit> ]
bool Datetime::initializeEoww (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eoww"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday += 6 - t->tm_wday;
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eonww [ !<alpha> && !<digit> ]
bool Datetime::initializeEonww (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eonww"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mday += 13 - t->tm_wday;
      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sopm [ !<alpha> && !<digit> ]
bool Datetime::initializeSopm (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sopm"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;

      if (t->tm_mon == 0)
      {
        t->tm_year--;
        t->tm_mon = 11;
      }
      else
        t->tm_mon--;

      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// som [ !<alpha> && !<digit> ]
bool Datetime::initializeSom (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("som"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sonm [ !<alpha> && !<digit> ]
bool Datetime::initializeSonm (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sonm"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;

      t->tm_mon++;
      if (t->tm_mon > 11)
      {
        t->tm_year++;
        t->tm_mon = 0;
      }

      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eopm [ !<alpha> && !<digit> ]
bool Datetime::initializeEopm (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eopm"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eom [ !<alpha> && !<digit> ]
bool Datetime::initializeEom (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eom"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;

      t->tm_mon++;
      if (t->tm_mon > 11)
      {
        t->tm_year++;
        t->tm_mon = 0;
      }

      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eonm [ !<alpha> && !<digit> ]
bool Datetime::initializeEonm (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eonm"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mday = 1;
      t->tm_mon += 2;
      if (t->tm_mon > 11)
      {
        t->tm_year++;
        t->tm_mon -= 12;
      }

      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sopq [ !<alpha> && !<digit> ]
bool Datetime::initializeSopq (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sopq"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mon -= t->tm_mon % 3;
      t->tm_mon -= 3;
      if (t->tm_mon < 0)
      {
        t->tm_mon += 12;
        t->tm_year--;
      }

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// soq [ !<alpha> && !<digit> ]
bool Datetime::initializeSoq (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("soq"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
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
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sonq [ !<alpha> && !<digit> ]
bool Datetime::initializeSonq (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sonq"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mon += 3 - (t->tm_mon % 3);
      if (t->tm_mon > 11)
      {
        t->tm_mon -= 12;
        ++t->tm_year;
      }

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eopq [ !<alpha> && !<digit> ]
bool Datetime::initializeEopq (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eopq"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mon -= t->tm_mon % 3;
      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eoq [ !<alpha> && !<digit> ]
bool Datetime::initializeEoq (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eoq"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_mon += 3 - (t->tm_mon % 3);
      if (t->tm_mon > 11)
      {
        t->tm_mon -= 12;
        ++t->tm_year;
      }

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eonq [ !<alpha> && !<digit> ]
bool Datetime::initializeEonq (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eonq"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mon += 6 - (t->tm_mon % 3);
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
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sopy [ !<alpha> && !<digit> ]
bool Datetime::initializeSopy (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sopy"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = t->tm_sec = 0;
      t->tm_mon = 0;
      t->tm_mday = 1;
      t->tm_year--;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// soy [ !<alpha> && !<digit> ]
bool Datetime::initializeSoy (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("soy"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
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
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// sony [ !<alpha> && !<digit> ]
bool Datetime::initializeSony (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("sony"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
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
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eopy [ !<alpha> && !<digit> ]
bool Datetime::initializeEopy (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eopy"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mon = 0;
      t->tm_mday = 1;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eoy [ !<alpha> && !<digit> ]
bool Datetime::initializeEoy (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eoy"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mon = 0;
      t->tm_mday = 1;
      t->tm_year++;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// eony [ !<alpha> && !<digit> ]
bool Datetime::initializeEony (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("eony"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      t->tm_hour = t->tm_min = 0;
      t->tm_sec = -1;
      t->tm_mon = 0;
      t->tm_mday = 1;
      t->tm_year += 2;
      t->tm_isdst = -1;
      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// easter       [ !<alpha> && !<digit> ]
// eastermonday [ !<alpha> && !<digit> ]
// ascension    [ !<alpha> && !<digit> ]
// pentecost    [ !<alpha> && !<digit> ]
// goodfriday   [ !<alpha> && !<digit> ]
bool Datetime::initializeEaster (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  std::vector <std::string> holidays = {"eastermonday", "easter", "ascension", "pentecost", "goodfriday"};
  std::vector <int>         offsets  = {             1,        0,          39,          49,           -2};

  std::string token;
  for (int holiday = 0; holiday < 5; ++holiday)
  {
   if (pig.skipLiteral (holidays[holiday]) &&
       ! unicodeLatinAlpha (pig.peek ()) &&
       ! unicodeLatinDigit (pig.peek ()))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      easter (t);
      _date = mktime (t);

      // If the result is earlier this year, then recalc for next year.
      if (_date < now)
      {
        t = localtime (&now);
        t->tm_year++;
        easter (t);
      }

      // Adjust according to holiday-specific offsets.
      t->tm_mday += offsets[holiday];

      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// midsommar [ !<alpha> && !<digit> ]
bool Datetime::initializeMidsommar (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("midsommar"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);
      midsommar (t);
      _date = mktime (t);

      // If the result is earlier this year, then recalc for next year.
      if (_date < now)
      {
        t = localtime (&now);
        t->tm_year++;
        midsommar (t);
      }

      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// midsommarafton [ !<alpha> && !<digit> ]
// juhannus [ !<alpha> && !<digit> ]
bool Datetime::initializeMidsommarafton (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skipLiteral ("midsommarafton") ||
      pig.skipLiteral ("juhannus"))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);
      midsommarafton (t);
      _date = mktime (t);

      // If the result is earlier this year, then recalc for next year.
      if (_date < now)
      {
        t = localtime (&now);
        t->tm_year++;
        midsommarafton (t);
      }

      _date = mktime (t);
      return true;
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// 8am
// 8a
// 8:30am
// 8:30a
// 8:30
// 8:30:00
//
// \d+ [ : \d{2} ] [ am | a | pm | p ] [ !<alpha> && !<digit> && !: && !+ && !- ]
//
bool Datetime::initializeInformalTime (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  int digit = 0;
  bool needDesignator = true;   // Require am/pm.
  bool haveDesignator = false;  // Provided am/pm.
  if (pig.getDigit (digit))
  {
    int hours = digit;
    if (pig.getDigit (digit))
      hours = 10 * hours + digit;

    int minutes = 0;
    long long seconds = 0;
    if (pig.skip (':'))
    {
      if (! pig.getDigit2 (minutes))
      {
        pig.restoreTo (checkpoint);
        return false;
      }

      if (pig.skip (':'))
      {
        if (! pig.getDigits (seconds))
        {
          pig.restoreTo (checkpoint);
          return false;
        }
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

    // Informal time needs to be terminated.
    auto following = pig.peek ();
    if (unicodeLatinAlpha (following) ||
        unicodeLatinDigit (following) ||
        following == ':'              ||
        following == '-'              ||
        following == '+')
    {
      pig.restoreTo (checkpoint);
      return false;
    }

    if (haveDesignator || ! needDesignator)
    {
      // Midnight today + hours:minutes:seconds.
      time_t now = time (nullptr);
      struct tm* t = localtime (&now);

      int now_seconds  = (t->tm_hour * 3600) + (t->tm_min * 60) + t->tm_sec;
      int calc_seconds = (hours      * 3600) + (minutes   * 60) + seconds;

      if (Datetime::timeRelative &&
          calc_seconds < now_seconds)
        ++t->tm_mday;

      // Basic validation.
      if (hours   >= 0 && hours   < 24 &&
          minutes >= 0 && minutes < 60 &&
          seconds >= 0 && seconds < 60)
      {
        t->tm_hour = hours;
        t->tm_min = minutes;
        t->tm_sec = seconds;
        t->tm_isdst = -1;
        _date = mktime (t);

        return true;
      }
    }
  }

  pig.restoreTo (checkpoint);
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

////////////////////////////////////////////////////////////////////////////////
// <ordinal> <weekday> in|of <month>
bool Datetime::initializeNthDayInMonth (const std::vector <std::string>& tokens)
{
  if (tokens.size () == 4)
  {
    int ordinal {0};
    if (isOrdinal (tokens[0], ordinal))
    {
      auto day = Datetime::dayOfWeek (tokens[1]);
      if (day != -1)
      {
        if (tokens[2] == "in" ||
            tokens[2] == "of")
        {
          auto month = Datetime::monthOfYear (tokens[3]);
          if (month != -1)
          {
            std::cout << "# ordinal=" << ordinal << " day=" << day << " in month=" << month << '\n';

            // TODO Assume 1st of the month
            // TODO Assume current year
            // TODO Determine the day
            // TODO Project forwards/backwards, to the desired day
            // TODO Add ((ordinal - 1) * 7) days

            return true;
          }
        }
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::isOrdinal (const std::string& token, int& ordinal)
{
  Pig p (token);
  long long number;
  std::string suffix;
  if (p.getDigits (number) &&
      p.getRemainder (suffix))
  {
    if ((number >= 11 && number <= 13 && suffix == "th") ||
        (number % 10 == 1             && suffix == "st") ||
        (number % 10 == 2             && suffix == "nd") ||
        (number % 10 == 3             && suffix == "rd") ||
        (                                suffix == "th"))
    {
      ordinal = number;
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Validation via simple range checking.
bool Datetime::validate ()
{
  // _year;
  if ((_year    && (_year    <   1900 || _year    >                                  9999)) ||
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
  // than the current seconds. Overridden by the ::timeRelative setting.
  if (Datetime::timeRelative &&
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
  return format ("{1}", _date);
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
      << 'T'
      << std::setw (2) << std::setfill ('0') << t->tm_hour
      << std::setw (2) << std::setfill ('0') << t->tm_min
      << std::setw (2) << std::setfill ('0') << t->tm_sec
      << 'Z';

  return iso.str ();
}

////////////////////////////////////////////////////////////////////////////////
// 1998-01-19T07:00:00 =  YYYY-MM-DDThh:mm:ss
std::string Datetime::toISOLocalExtended () const
{
  struct tm* t = localtime (&_date);

  std::stringstream iso;
  iso << std::setw (4) << std::setfill ('0') << t->tm_year + 1900
      << '-'
      << std::setw (2) << std::setfill ('0') << t->tm_mon + 1
      << '-'
      << std::setw (2) << std::setfill ('0') << t->tm_mday
      << 'T'
      << std::setw (2) << std::setfill ('0') << t->tm_hour
      << ':'
      << std::setw (2) << std::setfill ('0') << t->tm_min
      << ':'
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
  assert (year >= 1969 && year <= 9999);
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
