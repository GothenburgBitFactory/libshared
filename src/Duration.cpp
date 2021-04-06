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
#include <Duration.h>
#include <unicode.h>
#include <sstream>
#include <iomanip>
#include <vector>

bool Duration::standaloneSecondsEnabled = true;

#define DAY    86400
#define HOUR    3600
#define MINUTE    60
#define SECOND     1

static struct
{
  std::string unit;
  int seconds;
  bool standalone;
} durations[] =
{
  // These are sorted by first character, then length, so that Pig::getOneOf
  // returns a maximal match.
  {"annual",     365 * DAY,    true },
  {"biannual",   730 * DAY,    true },
  {"bimonthly",   61 * DAY,    true },
  {"biweekly",    14 * DAY,    true },
  {"biyearly",   730 * DAY,    true },
  {"daily",        1 * DAY,    true },
  {"days",         1 * DAY,    false},
  {"day",          1 * DAY,    true },
  {"d",            1 * DAY,    false},
  {"fortnight",   14 * DAY,    true },
  {"hours",        1 * HOUR,   false},
  {"hour",         1 * HOUR,   true },
  {"hrs",          1 * HOUR,   false},
  {"hr",           1 * HOUR,   true },
  {"h",            1 * HOUR,   false},
  {"minutes",      1 * MINUTE, false},
  {"minute",       1 * MINUTE, true },
  {"mins",         1 * MINUTE, false},
  {"min",          1 * MINUTE, true },
  {"monthly",     30 * DAY,    true },
  {"months",      30 * DAY,    false},
  {"month",       30 * DAY,    true },
  {"mnths",       30 * DAY,    false},
  {"mths",        30 * DAY,    false},
  {"mth",         30 * DAY,    true },
  {"mos",         30 * DAY,    false},
  {"mo",          30 * DAY,    true },
  {"m",           30 * DAY,    false},
  {"quarterly",   91 * DAY,    true },
  {"quarters",    91 * DAY,    false},
  {"quarter",     91 * DAY,    true },
  {"qrtrs",       91 * DAY,    false},
  {"qrtr",        91 * DAY,    true },
  {"qtrs",        91 * DAY,    false},
  {"qtr",         91 * DAY,    true },
  {"q",           91 * DAY,    false},
  {"semiannual", 183 * DAY,    true },
  {"sennight",    14 * DAY,    false},
  {"seconds",      1 * SECOND, false},
  {"second",       1 * SECOND, true },
  {"secs",         1 * SECOND, false},
  {"sec",          1 * SECOND, true },
  {"s",            1 * SECOND, false},
  {"weekdays",     1 * DAY,    true },
  {"weekly",       7 * DAY,    true },
  {"weeks",        7 * DAY,    false},
  {"week",         7 * DAY,    true },
  {"wks",          7 * DAY,    false},
  {"wk",           7 * DAY,    true },
  {"w",            7 * DAY,    false},
  {"yearly",     365 * DAY,    true },
  {"years",      365 * DAY,    false},
  {"year",       365 * DAY,    true },
  {"yrs",        365 * DAY,    false},
  {"yr",         365 * DAY,    true },
  {"y",          365 * DAY,    false},
};

#define NUM_DURATIONS (sizeof (durations) / sizeof (durations[0]))

////////////////////////////////////////////////////////////////////////////////
Duration::Duration ()
{
  clear ();
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (const std::string& input)
{
  clear ();
  std::string::size_type idx = 0;
  parse (input, idx);
}

////////////////////////////////////////////////////////////////////////////////
Duration::Duration (time_t input)
{
  clear ();
  _period = input;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator< (const Duration& other)
{
  return _period < other._period;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator> (const Duration& other)
{
  return _period > other._period;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator<= (const Duration& other)
{
  return _period <= other._period;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::operator>= (const Duration& other)
{
  return _period >= other._period;
}

////////////////////////////////////////////////////////////////////////////////
std::string Duration::toString () const
{
  std::stringstream s;
  s << _period;
  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
time_t Duration::toTime_t () const
{
  return _period;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::parse (const std::string& input, std::string::size_type& start)
{
  auto i = start;
  Pig pig (input);
  if (i)
    pig.skipN (static_cast <int> (i));

  if (Duration::standaloneSecondsEnabled && parse_seconds (pig))
  {
    // ::resolve is not needed in this case.
    start = pig.cursor ();
    return true;
  }

  else if (parse_designated (pig) ||
           parse_weeks (pig)      ||
           parse_units (pig))
  {
    start = pig.cursor ();
    resolve ();
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::parse_seconds (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  long long epoch {};
  if (pig.getDigits (epoch)             &&
      ! unicodeLatinAlpha (pig.peek ()) &&
      (epoch == 0 ||
       epoch > 60))
  {
    _period = static_cast <time_t> (epoch);
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// 'P' [nn 'Y'] [nn 'M'] [nn 'D'] ['T' [nn 'H'] [nn 'M'] [nn 'S']]
bool Duration::parse_designated (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skip ('P') &&
      ! pig.eos ())
  {
    long long value;
    pig.save ();
    if (pig.getDigits (value) && pig.skip ('Y'))
      _year = value;
    else
      pig.restore ();

    pig.save ();
    if (pig.getDigits (value) && pig.skip ('M'))
      _month = value;
    else
      pig.restore ();

    pig.save ();
    if (pig.getDigits (value) && pig.skip ('D'))
      _day = value;
    else
      pig.restore ();

    if (pig.skip ('T') &&
        ! pig.eos ())
    {
      pig.save ();
      if (pig.getDigits (value) && pig.skip ('H'))
        _hours = value;
      else
        pig.restore ();

      pig.save ();
      if (pig.getDigits (value) && pig.skip ('M'))
        _minutes = value;
      else
        pig.restore ();

      pig.save ();
      if (pig.getDigits (value) && pig.skip ('S'))
        _seconds = value;
      else
        pig.restore ();
    }

    auto following = pig.peek ();
    if (pig.cursor () - checkpoint >= 3   &&
        ! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
      return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// 'P' [nn 'W']
bool Duration::parse_weeks (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  if (pig.skip ('P') &&
      ! pig.eos ())
  {
    long long value;
    pig.save ();
    if (pig.getDigits (value) && pig.skip ('W'))
      _weeks = value;
    else
      pig.restore ();

    auto following = pig.peek ();
    if (pig.cursor () - checkpoint >= 3   &&
        ! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
      return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Duration::parse_units (Pig& pig)
{
  auto checkpoint = pig.cursor ();

  // Static and so preserved between calls.
  static std::vector <std::string> units;
  if (units.size () == 0)
    for (unsigned int i = 0; i < NUM_DURATIONS; i++)
      units.push_back (durations[i].unit);

  double number;
  std::string unit;
  if (pig.getOneOf (units, unit))
  {
    auto following = pig.peek ();
    if (! unicodeLatinAlpha (following) &&
        ! unicodeLatinDigit (following))
    {
      for (unsigned int i = 0; i < NUM_DURATIONS; i++)
      {
        if (durations[i].unit == unit &&
            durations[i].standalone)
        {
          _period = static_cast <time_t> (durations[i].seconds);
          return true;
        }
      }
    }
    else
      pig.restoreTo (checkpoint);
  }

  else if (pig.getDecimal (number))
  {
    pig.skipWS ();
    if (pig.getOneOf (units, unit))
    {
      // The "d" unit is a special case, because it is the only one that can
      // legitimately occur at the beginning of a UUID, and be followed by an
      // operator:
      //
      //   1111111d-0000-0000-0000-000000000000
      //
      // Because Lexer::isDuration is higher precedence than Lexer::isUUID,
      // the above UUID looks like:
      //
      //   <1111111d> <-> ...
      //   duration   op  ...
      //
      // So as a special case, durations, with units of "d" are rejected if the
      // quantity exceeds 10000.
      //
      if (unit == "d" && number > 10000.0)
      {
        pig.restoreTo (checkpoint);
        return false;
      }

      auto following = pig.peek ();
      if (! unicodeLatinAlpha (following) &&
          ! unicodeLatinDigit (following))
      {
        // Linear lookup - should instead be logarithmic.
        double seconds = 1;
        for (unsigned int i = 0; i < NUM_DURATIONS; i++)
        {
          if (durations[i].unit == unit)
          {
            seconds = durations[i].seconds;
            _period = static_cast <time_t> (number * static_cast <double> (seconds));
            return true;
          }
        }
      }
    }
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Duration::clear ()
{
  _year    = 0;
  _month   = 0;
  _weeks   = 0;
  _day     = 0;
  _hours   = 0;
  _minutes = 0;
  _seconds = 0;
  _period  = 0;
}

////////////////////////////////////////////////////////////////////////////////
const std::string Duration::format () const
{
  if (_period)
  {
    time_t t = _period;
    int seconds = t % 60; t /= 60;
    int minutes = t % 60; t /= 60;
    int hours   = t % 24; t /= 24;
    int days    = t;

    std::stringstream s;
    if (days)
      s << days << "d ";

    s << hours
      << ':'
      << std::setw (2) << std::setfill ('0') << minutes
      << ':'
      << std::setw (2) << std::setfill ('0') << seconds;

    return s.str ();
  }
  else
  {
    return "0:00:00";
  }
}

////////////////////////////////////////////////////////////////////////////////
const std::string Duration::formatHours () const
{
  if (_period)
  {
    time_t t = _period;
    int seconds = t % 60; t /= 60;
    int minutes = t % 60; t /= 60;
    int hours   = t;

    std::stringstream s;
    s << hours
      << ':'
      << std::setw (2) << std::setfill ('0') << minutes
      << ':'
      << std::setw (2) << std::setfill ('0') << seconds;

    return s.str ();
  }
  else
  {
    return "0:00:00";
  }
}

////////////////////////////////////////////////////////////////////////////////
const std::string Duration::formatISO () const
{
  if (_period)
  {
    time_t t = _period;
    int seconds = t % 60; t /= 60;
    int minutes = t % 60; t /= 60;
    int hours   = t % 24; t /= 24;
    int days    = t;

    std::stringstream s;
    s << 'P';
    if (days)   s << days   << 'D';

    if (hours || minutes || seconds)
    {
      s << 'T';
      if (hours)   s << hours   << 'H';
      if (minutes) s << minutes << 'M';
      if (seconds) s << seconds << 'S';
    }

    return s.str ();
  }
  else
  {
    return "PT0S";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Range      Representation
// ---------  ---------------------
// >= 365d    {n.n}y
// >= 90d     {n}mo
// >= 14d     {n}w
// >= 1d      {n}d
// >= 1h      {n}h
// >= 1min    {n}min
//            {n}s
//
const std::string Duration::formatVague (bool padding) const
{
  float days = (float) _period / 86400.0;

  std::stringstream formatted;
       if (_period >= 86400 * 365) formatted << std::fixed << std::setprecision (1) << (days / 365) << (padding ? "y  " : "y");
  else if (_period >= 86400 * 90)  formatted << static_cast <int> (days / 30)       << (padding ? "mo " : "mo");
  else if (_period >= 86400 * 14)  formatted << static_cast <int> (days / 7)        << (padding ? "w  " : "w");
  else if (_period >= 86400)       formatted << static_cast <int> (days)            << (padding ? "d  " : "d");
  else if (_period >= 3600)        formatted << static_cast <int> (_period / 3600)  << (padding ? "h  " : "h");
  else if (_period >= 60)          formatted << static_cast <int> (_period / 60)    << "min";  // Longest suffix - no padding
  else if (_period >= 1)           formatted << static_cast <int> (_period)         << (padding ? "s  " : "s");

  return formatted.str ();
}

////////////////////////////////////////////////////////////////////////////////
int Duration::days () const
{
  return _period / 86400;
}

////////////////////////////////////////////////////////////////////////////////
int Duration::hours () const
{
  return _period / 3600;
}

////////////////////////////////////////////////////////////////////////////////
int Duration::minutes () const
{
  return _period / 60;
}

////////////////////////////////////////////////////////////////////////////////
time_t Duration::seconds () const
{
  return _period;
}

////////////////////////////////////////////////////////////////////////////////
// Allow un-normalized values.
void Duration::resolve ()
{
  if (! _period)
  {
    if (_weeks)
      _period = (_weeks * 7 * 86400);
    else
      _period = (_year  * 365 * 86400) +
                (_month  * 30 * 86400) +
                (_day         * 86400) +
                (_hours       *  3600) +
                (_minutes     *    60) +
                _seconds;
  }
}

////////////////////////////////////////////////////////////////////////////////
