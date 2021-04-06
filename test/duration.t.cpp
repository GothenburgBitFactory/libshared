////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2021, Göteborg Bit Factory.
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
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
void testParse (
  UnitTest& t,
  const std::string& input,
  int in_start,
  int in_year,
  int in_month,
  int in_weeks,
  int in_day,
  int in_hours,
  int in_minutes,
  time_t in_seconds,
  time_t in_period,
  const std::string& output,
  const std::string& hours,
  const std::string& iso,
  const std::string& vague)
{
  std::string label = std::string ("parse (\"") + input + "\") --> ";

  Duration dur;
  std::string::size_type start = 0;

  t.ok (dur.parse (input, start),                 label + "true");
  t.is ((int) start,          in_start,           label + "[]");
  t.is (dur._year,            in_year,            label + "_year");
  t.is (dur._month,           in_month,           label + "_month");
  t.is (dur._weeks,           in_weeks,           label + "_weeks");
  t.is (dur._day,             in_day,             label + "_day");
  t.is (dur._hours,           in_hours,           label + "_hours");
  t.is (dur._minutes,         in_minutes,         label + "_minutes");
  t.is (dur._seconds,         in_seconds,         label + "_seconds");
  t.is ((size_t) dur._period, (size_t) in_period, label + "_period");
  t.is (dur.format (),        output,             label + " format");
  t.is (dur.formatHours (),   hours,              label + " formatHours");
  t.is (dur.formatISO (),     iso,                label + " formatISO");
  t.is (dur.formatVague (),   vague,              label + " formatVague");
}

////////////////////////////////////////////////////////////////////////////////
void testParse (
  UnitTest& t,
  const std::string& input)
{
  std::string label = std::string ("Duration::parse positive '") + input + "' --> success";

  Duration positive;
  std::string::size_type pos {0};
  t.ok (positive.parse (input, pos) && pos, label);
}

////////////////////////////////////////////////////////////////////////////////
void testParseError (
  UnitTest& t,
  const std::string& input)
{
  std::string label = std::string ("Duration::parse negative '") + input + "' --> fail";

  Duration neg;
  std::string::size_type pos {0};
  t.notok (neg.parse (input, pos), label);
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (1921);

  // Simple negative tests.
  testParseError (t, "foo");
  testParseError (t, "P");
  testParseError (t, "PT");
  testParseError (t, "P1");
  testParseError (t, "P1T");
  testParseError (t, "PT1");

  int year  = 365 * 86400;
  int month =  30 * 86400;
  int day   =       86400;
  int h     =        3600;
  int m     =          60;

  // Designated.
  //            input              i  Year  Mo  We  Da  Ho  Mi         Se                           time_t           format          hours               iso     vague
  testParse (t, "P1Y",             3,    1,  0,  0,  0,  0,  0,         0,                            year,   "365d 0:00:00",  "8760:00:00",          "P365D",   "1.0y");
  testParse (t, "P1M",             3,    0,  1,  0,  0,  0,  0,         0,                           month,    "30d 0:00:00",   "720:00:00",           "P30D",     "4w");
  testParse (t, "P1D",             3,    0,  0,  0,  1,  0,  0,         0,                             day,     "1d 0:00:00",    "24:00:00",            "P1D",     "1d");
  testParse (t, "P1Y1M",           5,    1,  1,  0,  0,  0,  0,         0,                    year + month,   "395d 0:00:00",  "9480:00:00",          "P395D",   "1.1y");
  testParse (t, "P1Y1D",           5,    1,  0,  0,  1,  0,  0,         0,                      year + day,   "366d 0:00:00",  "8784:00:00",          "P366D",   "1.0y");
  testParse (t, "P1M1D",           5,    0,  1,  0,  1,  0,  0,         0,                     month + day,    "31d 0:00:00",   "744:00:00",           "P31D",     "4w");
  testParse (t, "P1Y1M1D",         7,    1,  1,  0,  1,  0,  0,         0,              year + month + day,   "396d 0:00:00",  "9504:00:00",          "P396D",   "1.1y");
  testParse (t, "PT1H",            4,    0,  0,  0,  0,  1,  0,         0,                               h,        "1:00:00",     "1:00:00",           "PT1H",     "1h");
  testParse (t, "PT1M",            4,    0,  0,  0,  0,  0,  1,         0,                               m,        "0:01:00",     "0:01:00",           "PT1M",   "1min");
  testParse (t, "PT1S",            4,    0,  0,  0,  0,  0,  0,         1,                               1,        "0:00:01",     "0:00:01",           "PT1S",     "1s");
  testParse (t, "PT1H1M",          6,    0,  0,  0,  0,  1,  1,         0,                           h + m,        "1:01:00",     "1:01:00",         "PT1H1M",     "1h");
  testParse (t, "PT1H1S",          6,    0,  0,  0,  0,  1,  0,         1,                           h + 1,        "1:00:01",     "1:00:01",         "PT1H1S",     "1h");
  testParse (t, "PT1M1S",          6,    0,  0,  0,  0,  0,  1,         1,                           m + 1,        "0:01:01",     "0:01:01",         "PT1M1S",   "1min");
  testParse (t, "PT1H1M1S",        8,    0,  0,  0,  0,  1,  1,         1,                       h + m + 1,        "1:01:01",     "1:01:01",       "PT1H1M1S",     "1h");
  testParse (t, "P1Y1M1DT1H1M1S", 14,    1,  1,  0,  1,  1,  1,         1,  year + month + day + h + m + 1,   "396d 1:01:01",  "9505:01:01",   "P396DT1H1M1S",   "1.1y");
  testParse (t, "PT24H",           5,    0,  0,  0,  0, 24,  0,         0,                             day,     "1d 0:00:00",    "24:00:00",            "P1D",     "1d");
  testParse (t, "PT40000000S",    11,    0,  0,  0,  0,  0,  0,  40000000,                        40000000,  "462d 23:06:40", "11111:06:40", "P462DT23H6M40S",   "1.3y");
  testParse (t, "PT3600S",         7,    0,  0,  0,  0,  0,  0,      3600,                               h,        "1:00:00",     "1:00:00",           "PT1H",     "1h");
  testParse (t, "PT60M",           5,    0,  0,  0,  0,  0, 60,         0,                               h,        "1:00:00",     "1:00:00",           "PT1H",     "1h");

  //            input              i  Year  Mo  We  Da  Ho  Mi         Se                           time_t           format          hours               iso     vague
  testParse (t, "0seconds",        8,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 seconds",       9,    0,  0,  0,  0,  0,  0,         0,                               2,        "0:00:02",     "0:00:02",           "PT2S",     "2s");
  testParse (t, "10seconds",       9,    0,  0,  0,  0,  0,  0,         0,                              10,        "0:00:10",     "0:00:10",          "PT10S",    "10s");
  testParse (t, "1.5seconds",     10,    0,  0,  0,  0,  0,  0,         0,                               1,        "0:00:01",     "0:00:01",           "PT1S",     "1s");

  testParse (t, "0second",         7,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 second",        8,    0,  0,  0,  0,  0,  0,         0,                               2,        "0:00:02",     "0:00:02",           "PT2S",     "2s");
  testParse (t, "10second",        8,    0,  0,  0,  0,  0,  0,         0,                              10,        "0:00:10",     "0:00:10",          "PT10S",    "10s");
  testParse (t, "1.5second",       9,    0,  0,  0,  0,  0,  0,         0,                               1,        "0:00:01",     "0:00:01",           "PT1S",     "1s");

  testParse (t, "0s",              2,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 s",             3,    0,  0,  0,  0,  0,  0,         0,                               2,        "0:00:02",     "0:00:02",           "PT2S",     "2s");
  testParse (t, "10s",             3,    0,  0,  0,  0,  0,  0,         0,                              10,        "0:00:10",     "0:00:10",          "PT10S",    "10s");
  testParse (t, "1.5s",            4,    0,  0,  0,  0,  0,  0,         0,                               1,        "0:00:01",     "0:00:01",           "PT1S",     "1s");

  testParse (t, "0minutes",        8,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 minutes",       9,    0,  0,  0,  0,  0,  0,         0,                           2 * m,        "0:02:00",     "0:02:00",           "PT2M",   "2min");
  testParse (t, "10minutes",       9,    0,  0,  0,  0,  0,  0,         0,                          10 * m,        "0:10:00",     "0:10:00",          "PT10M",  "10min");
  testParse (t, "1.5minutes",     10,    0,  0,  0,  0,  0,  0,         0,                          m + 30,        "0:01:30",     "0:01:30",        "PT1M30S",   "1min");

  testParse (t, "0minute",         7,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 minute",        8,    0,  0,  0,  0,  0,  0,         0,                           2 * m,        "0:02:00",     "0:02:00",           "PT2M",   "2min");
  testParse (t, "10minute",        8,    0,  0,  0,  0,  0,  0,         0,                          10 * m,        "0:10:00",     "0:10:00",          "PT10M",  "10min");
  testParse (t, "1.5minute",       9,    0,  0,  0,  0,  0,  0,         0,                          m + 30,        "0:01:30",     "0:01:30",        "PT1M30S",   "1min");

  testParse (t, "0min",            4,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 min",           5,    0,  0,  0,  0,  0,  0,         0,                           2 * m,        "0:02:00",     "0:02:00",           "PT2M",   "2min");
  testParse (t, "10min",           5,    0,  0,  0,  0,  0,  0,         0,                          10 * m,        "0:10:00",     "0:10:00",          "PT10M",  "10min");
  testParse (t, "1.5min",          6,    0,  0,  0,  0,  0,  0,         0,                          m + 30,        "0:01:30",     "0:01:30",        "PT1M30S",   "1min");

  testParse (t, "0hours",          6,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 hours",         7,    0,  0,  0,  0,  0,  0,         0,                           2 * h,        "2:00:00",     "2:00:00",           "PT2H",     "2h");
  testParse (t, "10hours",         7,    0,  0,  0,  0,  0,  0,         0,                          10 * h,       "10:00:00",    "10:00:00",          "PT10H",    "10h");
  testParse (t, "1.5hours",        8,    0,  0,  0,  0,  0,  0,         0,                      h + 30 * m,        "1:30:00",     "1:30:00",        "PT1H30M",     "1h");

  testParse (t, "0hour",           5,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 hour",          6,    0,  0,  0,  0,  0,  0,         0,                           2 * h,        "2:00:00",     "2:00:00",           "PT2H",     "2h");
  testParse (t, "10hour",          6,    0,  0,  0,  0,  0,  0,         0,                          10 * h,       "10:00:00",    "10:00:00",          "PT10H",    "10h");
  testParse (t, "1.5hour",         7,    0,  0,  0,  0,  0,  0,         0,                      h + 30 * m,        "1:30:00",     "1:30:00",        "PT1H30M",     "1h");

  testParse (t, "0h",              2,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 h",             3,    0,  0,  0,  0,  0,  0,         0,                           2 * h,        "2:00:00",     "2:00:00",           "PT2H",     "2h");
  testParse (t, "10h",             3,    0,  0,  0,  0,  0,  0,         0,                          10 * h,       "10:00:00",    "10:00:00",          "PT10H",    "10h");
  testParse (t, "1.5h",            4,    0,  0,  0,  0,  0,  0,         0,                      h + 30 * m,        "1:30:00",     "1:30:00",        "PT1H30M",     "1h");

  testParse (t, "weekdays",        8,    0,  0,  0,  0,  0,  0,         0,                             day,     "1d 0:00:00",    "24:00:00",            "P1D",     "1d");

  //            input              i  Year  Mo  We  Da  Ho  Mi         Se                           time_t           format          hours               iso     vague
  testParse (t, "daily",           5,    0,  0,  0,  0,  0,  0,         0,                             day,     "1d 0:00:00",    "24:00:00",            "P1D",     "1d");

  testParse (t, "0days",           5,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 days",          6,    0,  0,  0,  0,  0,  0,         0,                         2 * day,     "2d 0:00:00",    "48:00:00",            "P2D",     "2d");
  testParse (t, "10days",          6,    0,  0,  0,  0,  0,  0,         0,                        10 * day,    "10d 0:00:00",   "240:00:00",           "P10D",    "10d");
  testParse (t, "1.5days",         7,    0,  0,  0,  0,  0,  0,         0,                    day + 12 * h,    "1d 12:00:00",    "36:00:00",        "P1DT12H",     "1d");

  testParse (t, "0day",            4,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 day",           5,    0,  0,  0,  0,  0,  0,         0,                         2 * day,     "2d 0:00:00",    "48:00:00",            "P2D",     "2d");
  testParse (t, "10day",           5,    0,  0,  0,  0,  0,  0,         0,                        10 * day,    "10d 0:00:00",   "240:00:00",           "P10D",    "10d");
  testParse (t, "1.5day",          6,    0,  0,  0,  0,  0,  0,         0,                    day + 12 * h,    "1d 12:00:00",    "36:00:00",        "P1DT12H",     "1d");

  testParse (t, "0d",              2,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 d",             3,    0,  0,  0,  0,  0,  0,         0,                         2 * day,     "2d 0:00:00",    "48:00:00",            "P2D",     "2d");
  testParse (t, "10d",             3,    0,  0,  0,  0,  0,  0,         0,                        10 * day,    "10d 0:00:00",   "240:00:00",           "P10D",    "10d");
  testParse (t, "1.5d",            4,    0,  0,  0,  0,  0,  0,         0,                    day + 12 * h,    "1d 12:00:00",    "36:00:00",        "P1DT12H",     "1d");

  testParse (t, "weekly",          6,    0,  0,  0,  0,  0,  0,         0,                         7 * day,     "7d 0:00:00",   "168:00:00",            "P7D",     "7d");

  testParse (t, "0weeks",          6,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 weeks",         7,    0,  0,  0,  0,  0,  0,         0,                        14 * day,    "14d 0:00:00",   "336:00:00",           "P14D",     "2w");
  testParse (t, "10weeks",         7,    0,  0,  0,  0,  0,  0,         0,                        70 * day,    "70d 0:00:00",  "1680:00:00",           "P70D",    "10w");
  testParse (t, "1.5weeks",        8,    0,  0,  0,  0,  0,  0,         0,               10 * day + 12 * h,   "10d 12:00:00",   "252:00:00",       "P10DT12H",    "10d");

  testParse (t, "0week",           5,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 week",          6,    0,  0,  0,  0,  0,  0,         0,                        14 * day,    "14d 0:00:00",   "336:00:00",           "P14D",     "2w");
  testParse (t, "10week",          6,    0,  0,  0,  0,  0,  0,         0,                        70 * day,    "70d 0:00:00",  "1680:00:00",           "P70D",    "10w");
  testParse (t, "1.5week",         7,    0,  0,  0,  0,  0,  0,         0,               10 * day + 12 * h,   "10d 12:00:00",   "252:00:00",       "P10DT12H",    "10d");

  testParse (t, "0w",              2,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 w",             3,    0,  0,  0,  0,  0,  0,         0,                        14 * day,    "14d 0:00:00",   "336:00:00",           "P14D",     "2w");
  testParse (t, "10w",             3,    0,  0,  0,  0,  0,  0,         0,                        70 * day,    "70d 0:00:00",  "1680:00:00",           "P70D",    "10w");
  testParse (t, "1.5w",            4,    0,  0,  0,  0,  0,  0,         0,               10 * day + 12 * h,   "10d 12:00:00",   "252:00:00",       "P10DT12H",    "10d");

  //            input              i  Year  Mo  We  Da  Ho  Mi         Se                           time_t           format          hours               iso     vague
  testParse (t, "monthly",         7,    0,  0,  0,  0,  0,  0,         0,                        30 * day,    "30d 0:00:00",   "720:00:00",           "P30D",     "4w");

  testParse (t, "0months",         7,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 months",        8,    0,  0,  0,  0,  0,  0,         0,                        60 * day,    "60d 0:00:00",  "1440:00:00",           "P60D",     "8w");
  testParse (t, "10months",        8,    0,  0,  0,  0,  0,  0,         0,                       300 * day,   "300d 0:00:00",  "7200:00:00",          "P300D",   "10mo");
  testParse (t, "1.5months",       9,    0,  0,  0,  0,  0,  0,         0,                        45 * day,    "45d 0:00:00",  "1080:00:00",           "P45D",     "6w");

  testParse (t, "0month",          6,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 month",         7,    0,  0,  0,  0,  0,  0,         0,                        60 * day,    "60d 0:00:00",  "1440:00:00",           "P60D",     "8w");
  testParse (t, "10month",         7,    0,  0,  0,  0,  0,  0,         0,                       300 * day,   "300d 0:00:00",  "7200:00:00",          "P300D",   "10mo");
  testParse (t, "1.5month",        8,    0,  0,  0,  0,  0,  0,         0,                        45 * day,    "45d 0:00:00",  "1080:00:00",           "P45D",     "6w");

  testParse (t, "0mo",             3,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 mo",            4,    0,  0,  0,  0,  0,  0,         0,                        60 * day,    "60d 0:00:00",  "1440:00:00",           "P60D",     "8w");
  testParse (t, "10mo",            4,    0,  0,  0,  0,  0,  0,         0,                       300 * day,   "300d 0:00:00",  "7200:00:00",          "P300D",   "10mo");
  testParse (t, "1.5mo",           5,    0,  0,  0,  0,  0,  0,         0,                        45 * day,    "45d 0:00:00",  "1080:00:00",           "P45D",     "6w");

  testParse (t, "quarterly",       9,    0,  0,  0,  0,  0,  0,         0,                        91 * day,    "91d 0:00:00",  "2184:00:00",           "P91D",    "3mo");

  testParse (t, "0quarters",       9,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 quarters",     10,    0,  0,  0,  0,  0,  0,         0,                       182 * day,   "182d 0:00:00",  "4368:00:00",          "P182D",    "6mo");
  testParse (t, "10quarters",     10,    0,  0,  0,  0,  0,  0,         0,                       910 * day,   "910d 0:00:00", "21840:00:00",          "P910D",   "2.5y");
  testParse (t, "1.5quarters",    11,    0,  0,  0,  0,  0,  0,         0,              136 * day + 12 * h,  "136d 12:00:00",  "3276:00:00",      "P136DT12H",    "4mo");

  testParse (t, "0quarter",        8,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 quarter",       9,    0,  0,  0,  0,  0,  0,         0,                       182 * day,   "182d 0:00:00",  "4368:00:00",          "P182D",    "6mo");
  testParse (t, "10quarter",       9,    0,  0,  0,  0,  0,  0,         0,                       910 * day,   "910d 0:00:00", "21840:00:00",          "P910D",   "2.5y");
  testParse (t, "1.5quarter",     10,    0,  0,  0,  0,  0,  0,         0,              136 * day + 12 * h,  "136d 12:00:00",  "3276:00:00",      "P136DT12H",    "4mo");

  testParse (t, "0q",              2,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 q",             3,    0,  0,  0,  0,  0,  0,         0,                       182 * day,   "182d 0:00:00",  "4368:00:00",          "P182D",    "6mo");
  testParse (t, "10q",             3,    0,  0,  0,  0,  0,  0,         0,                       910 * day,   "910d 0:00:00", "21840:00:00",          "P910D",   "2.5y");
  testParse (t, "1.5q",            4,    0,  0,  0,  0,  0,  0,         0,              136 * day + 12 * h,  "136d 12:00:00",  "3276:00:00",      "P136DT12H",    "4mo");

  //            input              i  Year  Mo  We  Da  Ho  Mi         Se                           time_t           format          hours               iso     vague
  testParse (t, "yearly",          6,    0,  0,  0,  0,  0,  0,         0,                            year,   "365d 0:00:00",  "8760:00:00",          "P365D",   "1.0y");

  testParse (t, "0years",          6,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 years",         7,    0,  0,  0,  0,  0,  0,         0,                        2 * year,   "730d 0:00:00", "17520:00:00",          "P730D",   "2.0y");
  testParse (t, "10years",         7,    0,  0,  0,  0,  0,  0,         0,                       10 * year,  "3650d 0:00:00", "87600:00:00",         "P3650D",  "10.0y");
  testParse (t, "1.5years",        8,    0,  0,  0,  0,  0,  0,         0,              547 * day + 12 * h,  "547d 12:00:00", "13140:00:00",      "P547DT12H",   "1.5y");

  testParse (t, "0year",           5,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 year",          6,    0,  0,  0,  0,  0,  0,         0,                        2 * year,   "730d 0:00:00", "17520:00:00",          "P730D",   "2.0y");
  testParse (t, "10year",          6,    0,  0,  0,  0,  0,  0,         0,                       10 * year,  "3650d 0:00:00", "87600:00:00",         "P3650D",  "10.0y");
  testParse (t, "1.5year",         7,    0,  0,  0,  0,  0,  0,         0,              547 * day + 12 * h,  "547d 12:00:00", "13140:00:00",      "P547DT12H",   "1.5y");

  testParse (t, "0y",              2,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 y",             3,    0,  0,  0,  0,  0,  0,         0,                        2 * year,   "730d 0:00:00", "17520:00:00",          "P730D",   "2.0y");
  testParse (t, "10y",             3,    0,  0,  0,  0,  0,  0,         0,                       10 * year,  "3650d 0:00:00", "87600:00:00",         "P3650D",  "10.0y");
  testParse (t, "1.5y",            4,    0,  0,  0,  0,  0,  0,         0,              547 * day + 12 * h,  "547d 12:00:00", "13140:00:00",      "P547DT12H",   "1.5y");

  testParse (t, "annual",          6,    0,  0,  0,  0,  0,  0,         0,                            year,   "365d 0:00:00",  "8760:00:00",          "P365D",   "1.0y");
  testParse (t, "biannual",        8,    0,  0,  0,  0,  0,  0,         0,                        2 * year,   "730d 0:00:00", "17520:00:00",          "P730D",   "2.0y");
  testParse (t, "bimonthly",       9,    0,  0,  0,  0,  0,  0,         0,                        61 * day,    "61d 0:00:00",  "1464:00:00",           "P61D",     "8w");
  testParse (t, "biweekly",        8,    0,  0,  0,  0,  0,  0,         0,                        14 * day,    "14d 0:00:00",   "336:00:00",           "P14D",     "2w");
  testParse (t, "biyearly",        8,    0,  0,  0,  0,  0,  0,         0,                        2 * year,   "730d 0:00:00", "17520:00:00",          "P730D",   "2.0y");
  testParse (t, "fortnight",       9,    0,  0,  0,  0,  0,  0,         0,                        14 * day,    "14d 0:00:00",   "336:00:00",           "P14D",     "2w");
  testParse (t, "semiannual",     10,    0,  0,  0,  0,  0,  0,         0,                       183 * day,   "183d 0:00:00",  "4392:00:00",          "P183D",    "6mo");

  testParse (t, "0sennight",       9,    0,  0,  0,  0,  0,  0,         0,                               0,        "0:00:00",     "0:00:00",           "PT0S",       "");
  testParse (t, "2 sennight",     10,    0,  0,  0,  0,  0,  0,         0,                        28 * day,    "28d 0:00:00",   "672:00:00",           "P28D",     "4w");
  testParse (t, "10sennight",     10,    0,  0,  0,  0,  0,  0,         0,                       140 * day,   "140d 0:00:00",  "3360:00:00",          "P140D",    "4mo");
  testParse (t, "1.5sennight",    11,    0,  0,  0,  0,  0,  0,         0,                        21 * day,    "21d 0:00:00",   "504:00:00",           "P21D",     "3w");

  //            input              i  Year  Mo  We  Da  Ho  Mi         Se                           time_t           format          hours               iso     vague
  testParse (t, "P1W",             3,    0,  0,  1,  0,  0,  0,         0,                         7 * day,     "7d 0:00:00",   "168:00:00",            "P7D",     "7d");

  Duration left, right;

  // operator<
  left = Duration ("1s");     right = Duration ("2s");     t.ok (left < right,    "Duration 1s < 2s");
  left = Duration ("-2s");    right = Duration ("-1s");    t.ok (left < right,    "Duration -2s < -1s");
  left = Duration ("1s");     right = Duration ("1min");   t.ok (left < right,    "Duration 1s < 1min");
  left = Duration ("1min");   right = Duration ("1h");     t.ok (left < right,    "Duration 1min < 1h");
  left = Duration ("1h");     right = Duration ("1d");     t.ok (left < right,    "Duration 1h < 1d");
  left = Duration ("1d");     right = Duration ("1w");     t.ok (left < right,    "Duration 1d < 1w");
  left = Duration ("1w");     right = Duration ("1mo");    t.ok (left < right,    "Duration 1w < 1mo");
  left = Duration ("1mo");    right = Duration ("1q");     t.ok (left < right,    "Duration 1mo < 1q");
  left = Duration ("1q");     right = Duration ("1y");     t.ok (left < right,    "Duration 1q < 1y");
  left = Duration ("-3s");    right = Duration ("-6s");    t.ok (right < left,    "Duration -6s < -3s");

  // operator>
  left = Duration ("2s");     right = Duration ("1s");     t.ok (left > right,    "Duration 2s > 1s");
  left = Duration ("-1s");    right = Duration ("-2s");    t.ok (left > right,    "Duration -1s > -2s");
  left = Duration ("1min");   right = Duration ("1s");     t.ok (left > right,    "Duration 1min > 1s");
  left = Duration ("1h");     right = Duration ("1min");   t.ok (left > right,    "Duration 1h > 1min");
  left = Duration ("1d");     right = Duration ("1h");     t.ok (left > right,    "Duration 1d > 1h");
  left = Duration ("1w");     right = Duration ("1d");     t.ok (left > right,    "Duration 1w > 1d");
  left = Duration ("1mo");    right = Duration ("1w");     t.ok (left > right,    "Duration 1mo > 1w");
  left = Duration ("1q");     right = Duration ("1mo");    t.ok (left > right,    "Duration 1q > 1mo");
  left = Duration ("1y");     right = Duration ("1q");     t.ok (left > right,    "Duration 1y > 1q");
  left = Duration ("-3s");    right = Duration ("-6s");    t.ok (left > right,    "Duration -3s > -6s");

  // operator<=
  left = Duration ("1s");     right = Duration ("2s");     t.ok (left <= right,    "Duration 1s <= 2s");
  left = Duration ("2s");     right = Duration ("2s");     t.ok (left <= right,    "Duration 2s <= 2s");
  left = Duration ("2s");     right = Duration ("1s");     t.notok (left <= right, "Duration NOT 2s <= 1s");

  // operator<=
  left = Duration ("2s");     right = Duration ("1s");     t.ok (left >= right,    "Duration 2s >= 1s");
  left = Duration ("2s");     right = Duration ("2s");     t.ok (left >= right,    "Duration 2s >= 2s");
  left = Duration ("1s");     right = Duration ("2s");     t.notok (left >= right, "Duration NOT 1s >= 2s");

  // toString
  Duration d (1234567890);
  // 14288d 23:31:30
  t.ok (d.days ()    == 14288,      "Duration 1234567890 -> days = 14288");
  t.ok (d.hours ()   == 342935,     "Duration 1234567890 -> hours = 342935");
  t.ok (d.minutes () == 20576131,   "Duration 1234567890 -> miniutes = 20576131");
  t.ok (d.seconds () == 1234567890, "Duration 1234567890 -> seconds = 123456789030");

  // formatVague (true)
  Duration::standaloneSecondsEnabled = true;
  t.is (Duration ("123").formatVague (false),  "2min", "formatVague: 123 -> '2min'");
  t.is (Duration ("123").formatVague (true),   "2min", "formatVague (true): 123 -> '2min'");
  t.is (Duration ("3610").formatVague (false), "1h",   "formatVague: 3610 -> '1h'");
  t.is (Duration ("3610").formatVague (true),  "1h  ", "formatVague (true): 3610 -> '1h  '");

  // Seconds must be > 60 for a raw number.
  testParse      (t, "0");
  testParseError (t, "59");
  testParseError (t, "60");
  testParse      (t, "61");
  testParse      (t, "61+0");
  testParse      (t, "61-0");
  testParse      (t, "61)");

  // Repeat with seconds disabled.
  Duration::standaloneSecondsEnabled = false;
  testParseError (t, "0");
  testParseError (t, "59");
  testParseError (t, "60");
  testParseError (t, "61");
  testParseError (t, "61+0");
  testParseError (t, "61-0");
  testParseError (t, "61)");
  Duration::standaloneSecondsEnabled = true;

  // Embedded parsing.
  testParseError (t, "1weekend");
  testParse      (t, "1d+now");
  testParse      (t, "1d-now");
  testParse      (t, "1d)");
  testParseError (t, "1d6");

  // This is jus ta diagnostic dump of all named dates, and us used to verify
  // correctness manually.
  t.diag ("--------------------------------------------");
  t.diag ("  PT1S       " + Duration ("PT1S").formatISO ());
  t.diag ("  PT1M       " + Duration ("PT1M").formatISO ());
  t.diag ("  PT1H       " + Duration ("PT1H").formatISO ());
  t.diag ("  P1D        " + Duration ("P1D").formatISO ());
  t.diag ("  P1M        " + Duration ("P1M").formatISO ());
  t.diag ("  P1Y        " + Duration ("P1Y").formatISO ());

  // Standalone units, no numbers.
  t.diag ("  sec        " + Duration ("sec").formatISO ());
  t.diag ("  min        " + Duration ("min").formatISO ());
  t.diag ("  hr         " + Duration ("hr").formatISO ());
  t.diag ("  day        " + Duration ("day").formatISO ());
  t.diag ("  wk         " + Duration ("wk").formatISO ());
  t.diag ("  biweekly   " + Duration ("biweekly").formatISO ());
  t.diag ("  fortnight  " + Duration ("fortnight").formatISO ());
  t.diag ("  mo         " + Duration ("mo").formatISO ());
  t.diag ("  bimonthly  " + Duration ("bimonthly").formatISO ());
  t.diag ("  qtr        " + Duration ("qtr").formatISO ());
  t.diag ("  yr         " + Duration ("yr").formatISO ());
  t.diag ("  annual     " + Duration ("annual").formatISO ());
  t.diag ("  biannual   " + Duration ("biannual").formatISO ());
  t.diag ("  biyearly   " + Duration ("biyearly").formatISO ());

  // Units.
  t.diag ("  1s         " + Duration ("1s").formatISO ());
  t.diag ("  1min       " + Duration ("1min").formatISO ());
  t.diag ("  1h         " + Duration ("1h").formatISO ());
  t.diag ("  1d         " + Duration ("1d").formatISO ());
  t.diag ("  1w         " + Duration ("1w").formatISO ());
  t.diag ("  1m         " + Duration ("1m").formatISO ());
  t.diag ("  1q         " + Duration ("1q").formatISO ());
  t.diag ("  1y         " + Duration ("1y").formatISO ());

  t.diag ("--------------------------------------------");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
