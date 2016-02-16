////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Göteborg Bit Factory.
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
  int in_day,
  int in_hours,
  int in_minutes,
  int in_seconds,
  time_t in_period,
  const std::string& output,
  const std::string& vague)
{
  std::string label = std::string ("parse (\"") + input + "\") --> ";

  Duration dur;
  std::string::size_type start = 0;

  t.ok (dur.parse (input, start),                 label + "true");
  t.is ((int) start,          in_start,           label + "[]");
  t.is (dur._year,            in_year,            label + "_year");
  t.is (dur._month,           in_month,           label + "_month");
  t.is (dur._day,             in_day,             label + "_day");
  t.is (dur._hours,           in_hours,           label + "_hours");
  t.is (dur._minutes,         in_minutes,         label + "_minutes");
  t.is (dur._seconds,         in_seconds,         label + "_seconds");
  t.is ((size_t) dur._period, (size_t) in_period, label + "_period");
  t.is (dur.format (),        output,             label + " format");
  t.is (dur.formatVague (),   vague,              label + " formatVague");
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (221);

  Duration dur;
  std::string::size_type start = 0;
  t.notok (dur.parse ("foo", start), "foo --> false");
  t.is ((int)start, 0,               "foo[0]");

  t.notok (dur.parse ("P", start),   "P --> false");
  t.is ((int)start, 0,               "P[0]");

  t.notok (dur.parse ("PT", start),  "PT --> false");
  t.is ((int)start, 0,               "PT[0]");

  t.notok (dur.parse ("P1", start),  "P1 --> false");
  t.is ((int)start, 0,               "P1[0]");

  t.notok (dur.parse ("P1T", start), "P1T --> false");
  t.is ((int)start, 0,               "P1T[0]");

  t.notok (dur.parse ("PT1", start), "PT1 --> false");
  t.is ((int)start, 0,               "PT1[0]");

  int year  = 365 * 86400;
  int month =  30 * 86400;
  int day   =       86400;
  int h     =        3600;
  int m     =          60;

  // Designated.
  //            input              i  Year  Mo   Da  Ho  Mi         Se                           time_t           format     vague
  testParse (t, "P1Y",             3,    1,  0,   0,  0,  0,         0,                            year,          "P365D",   "1.0y");
  testParse (t, "P1M",             3,    0,  1,   0,  0,  0,         0,                           month,           "P30D",     "4w");
  testParse (t, "P1D",             3,    0,  0,   1,  0,  0,         0,                             day,            "P1D",     "1d");
  testParse (t, "P1Y1M",           5,    1,  1,   0,  0,  0,         0,                    year + month,          "P395D",   "1.1y");
  testParse (t, "P1Y1D",           5,    1,  0,   1,  0,  0,         0,                      year + day,          "P366D",   "1.0y");
  testParse (t, "P1M1D",           5,    0,  1,   1,  0,  0,         0,                     month + day,           "P31D",     "4w");
  testParse (t, "P1Y1M1D",         7,    1,  1,   1,  0,  0,         0,              year + month + day,          "P396D",   "1.1y");
  testParse (t, "PT1H",            4,    0,  0,   0,  1,  0,         0,                               h,           "PT1H",     "1h");
  testParse (t, "PT1M",            4,    0,  0,   0,  0,  1,         0,                               m,           "PT1M",   "1min");
  testParse (t, "PT1S",            4,    0,  0,   0,  0,  0,         1,                               1,           "PT1S",     "1s");
  testParse (t, "PT1H1M",          6,    0,  0,   0,  1,  1,         0,                           h + m,         "PT1H1M",     "1h");
  testParse (t, "PT1H1S",          6,    0,  0,   0,  1,  0,         1,                           h + 1,         "PT1H1S",     "1h");
  testParse (t, "PT1M1S",          6,    0,  0,   0,  0,  1,         1,                           m + 1,         "PT1M1S",   "1min");
  testParse (t, "PT1H1M1S",        8,    0,  0,   0,  1,  1,         1,                       h + m + 1,       "PT1H1M1S",     "1h");
  testParse (t, "P1Y1M1DT1H1M1S", 14,    1,  1,   1,  1,  1,         1,  year + month + day + h + m + 1,   "P396DT1H1M1S",   "1.1y");
  testParse (t, "PT24H",           5,    0,  0,   0, 24,  0,         0,                             day,            "P1D",     "1d");
  testParse (t, "PT40000000S",    11,    0,  0,   0,  0,  0,  40000000,                        40000000, "P462DT23H6M40S",   "1.3y");
  testParse (t, "PT3600S",         7,    0,  0,   0,  0,  0,      3600,                               h,           "PT1H",     "1h");
  testParse (t, "PT60M",           5,    0,  0,   0,  0, 60,         0,                               h,           "PT1H",     "1h");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
