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
#include <common.h>
#include <format.h>
#include <unicode.h>

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
Datetime::Datetime (const int m, const int d, const int y)
{
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
Datetime::Datetime (const int m,  const int d,  const int y,
                    const int hr, const int mi, const int se)
{
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
  else if (parse_date_time     (pig)   || // Strictest first.
           parse_date_time_ext (pig)   /*||
           (Datetime::isoEnabled &&
            (parse_date_ext      (pig) ||
             parse_time_utc_ext  (pig) ||
             parse_time_off_ext  (pig) ||
             parse_time_ext      (pig)))*/) // Time last, as it is the most permissive.
  {
    // Check the values and determine time_t.
    if (validate ())
    {
      start = pig.cursor ();
      resolve ();
      return true;
    }
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
bool Datetime::parse_date_time (Pig& pig)
{
  pig.save ();
  int year, month, day, hour, minute, second;
  if (pig.getDigit4 (year)   &&
      pig.getDigit2 (month)  && month &&
      pig.getDigit2 (day)    && day   &&
      pig.skip      ('T')    &&
      pig.getDigit2 (hour)   &&
      pig.getDigit2 (minute) && minute < 60 &&
      pig.getDigit2 (second) && second < 60)
  {
    if (pig.skip ('Z'))
      _utc = true;

    _year    = year;
    _month   = month;
    _day     = day;
    _seconds = (((hour * 60) + minute) * 60) + second;

    return true;
  }

  _year    = 0;
  _month   = 0;
  _day     = 0;
  _seconds = 0;

  pig.restore ();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::parse_date_time_ext (Pig& pig)
{
  pig.save ();

  pig.restore ();
  return false;
}

/*
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
      (_day     && (_day     <      1 || _day     > Datetime::daysInMonth (_month, _year))) ||
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
  if (year    == 0 &&
      month   == 0 &&
      day     == 0 &&
      week    == 0 &&
      weekday == 0 &&
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
int Datetime::month () const
{
  struct tm* t = localtime (&_date);
  return t->tm_mon + 1;
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
  return this->year ()  == rhs.year ()  &&
         this->month () == rhs.month () &&
         this->day ()   == rhs.day ()   &&
         this->hour ()  == rhs.hour ();
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameDay (const Datetime& rhs) const
{
  return this->year ()  == rhs.year ()  &&
         this->month () == rhs.month () &&
         this->day ()   == rhs.day ();
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameMonth (const Datetime& rhs) const
{
  return this->year ()  == rhs.year () &&
         this->month () == rhs.month ();
}

////////////////////////////////////////////////////////////////////////////////
bool Datetime::sameYear (const Datetime& rhs) const
{
  return this->year () == rhs.year ();
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
