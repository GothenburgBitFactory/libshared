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
#include <Datetime.h>
#include <test.h>
#include <iostream>
#include <time.h>

////////////////////////////////////////////////////////////////////////////////
void testParse (
  UnitTest& t,
  const std::string& input,
  int in_start,
  int in_year,
  int in_month,
  int in_week,
  int in_weekday,
  int in_julian,
  int in_day,
  int in_seconds,
  int in_offset,
  bool in_utc,
  time_t in_date)
{
  std::string label = std::string ("Datetime::parse (\"") + input + "\") --> ";

  Datetime iso;
  std::string::size_type start = 0;

  t.ok (iso.parse (input, start),             label + "true");
  t.is ((int) start,        in_start,         label + "[]");
  t.is (iso._year,          in_year,          label + "_year");
  t.is (iso._month,         in_month,         label + "_month");
  t.is (iso._week,          in_week,          label + "_week");
  t.is (iso._weekday,       in_weekday,       label + "_weekday");
  t.is (iso._julian,        in_julian,        label + "_julian");
  t.is (iso._day,           in_day,           label + "_day");
  t.is (iso._seconds,       in_seconds,       label + "_seconds");
  t.is (iso._offset,        in_offset,        label + "_offset");
  t.is (iso._utc,           in_utc,           label + "_utc");
  t.is ((size_t) iso._date, (size_t) in_date, label + "_date");
}

////////////////////////////////////////////////////////////////////////////////
void testParse (
  UnitTest& t,
  const std::string& input)
{
  std::string label = std::string ("Datetime::parse positive '") + input + "' --> success";

  Datetime positive;
  std::string::size_type pos {0};
  t.ok (positive.parse (input, pos) && pos, label);
}

////////////////////////////////////////////////////////////////////////////////
void testParseError (
  UnitTest& t,
  const std::string& input)
{
  std::string label = std::string ("Datetime::parse negative '") + input + "' --> fail";

  Datetime neg;
  std::string::size_type pos {0};
  t.notok (neg.parse (input, pos), label);
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (3458);

  Datetime iso;
  std::string::size_type start = 0;
  t.notok (iso.parse ("foo", start), "foo --> false");
  t.is ((int)start, 0,               "foo[0]");

  // Determine local and UTC time.
  time_t now = time (nullptr);
  struct tm* local_now = localtime (&now);
  int local_s = (local_now->tm_hour * 3600) +
                (local_now->tm_min  * 60)   +
                local_now->tm_sec;
  local_now->tm_hour  = 0;
  local_now->tm_min   = 0;
  local_now->tm_sec   = 0;
  local_now->tm_isdst = -1;
  time_t local = mktime (local_now);
  std::cout << "# local midnight today " << local << '\n';

  int year = 2013;
  int mo = 12;
  int f_yr = 9850;  // future year with same starting weekday as 2013

  local_now->tm_year  = year - 1900;
  local_now->tm_mon   = mo - 1;
  local_now->tm_mday  = 6;
  local_now->tm_isdst = 0;
  time_t local6 = mktime (local_now);
  std::cout << "# local midnight 2013-12-06 " << local6 << '\n';

  local_now->tm_year  = f_yr - 1900;
  time_t f_local6 = mktime (local_now);
  std::cout << "# future midnight 9850-12-06 " << f_local6 << '\n';

  local_now->tm_year  = year - 1900;
  local_now->tm_mon   = mo - 1;
  local_now->tm_mday  = 1;
  local_now->tm_isdst = 0;
  time_t local1 = mktime (local_now);
  std::cout << "# local midnight 2013-12-01 " << local1 << '\n';

  local_now->tm_year  = f_yr - 1900;
  time_t f_local1 = mktime (local_now);
  std::cout << "# future midnight 9850-12-01 " << f_local1 << '\n';

  struct tm* utc_now = gmtime (&now);
  int utc_s = (utc_now->tm_hour * 3600) +
              (utc_now->tm_min  * 60)   +
              utc_now->tm_sec;
  utc_now->tm_hour  = 0;
  utc_now->tm_min   = 0;
  utc_now->tm_sec   = 0;
  utc_now->tm_isdst = -1;
  time_t utc = timegm (utc_now);
  std::cout << "# utc midnight today " << utc << '\n';

  utc_now->tm_year  = year - 1900;
  utc_now->tm_mon   = mo - 1;
  utc_now->tm_mday  = 6;
  utc_now->tm_isdst = 0;
  time_t utc6 = timegm (utc_now);
  std::cout << "# utc midnight 2013-12-06 " << utc6 << '\n';

  utc_now->tm_year  = f_yr - 1900;
  time_t f_utc6 = timegm (utc_now);
  std::cout << "# future midnight 9850-12-06 " << f_utc6 << '\n';

  utc_now->tm_year  = year - 1900;
  utc_now->tm_mon   = mo - 1;
  utc_now->tm_mday  = 1;
  utc_now->tm_isdst = 0;
  time_t utc1 = timegm (utc_now);
  std::cout << "# utc midnight 2013-12-01 " << utc1 << '\n';

  utc_now->tm_year  = f_yr - 1900;
  time_t f_utc1 = timegm (utc_now);
  std::cout << "# future midnight 9850-12-01 " << f_utc1 << '\n';


  int hms = (12 * 3600) + (34 * 60) + 56; // The time 12:34:56 in seconds.
  int hm  = (12 * 3600) + (34 * 60);      // The time 12:34:00 in seconds.
  int z   = 3600;                         // TZ offset.

  int ld = local_s > hms ? 86400 : 0;     // Local extra day if now > hms.
  int ud = utc_s   > hms ? 86400 : 0;     // UTC extra day if now > hms.
  std::cout << "# ld " << ld << '\n';
  std::cout << "# ud " << ud << '\n';

  // Aggregated.
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "12:34:56  ",                 8,    0,  0,  0, 0,   0,  0,   hms,     0, false, local+hms+ld );

  // time-ext
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "12:34:56Z",                  9,    0,  0,  0, 0,   0,  0,   hms,     0,  true, utc+hms+ud   );
  testParse (t, "12:34Z",                     6,    0,  0,  0, 0,   0,  0,    hm,     0,  true, utc+hm+ud    );
  testParse (t, "12:34:56+01:00",            14,    0,  0,  0, 0,   0,  0,   hms,  3600, false, utc+hms-z+ud );
  testParse (t, "12:34:56+01",               11,    0,  0,  0, 0,   0,  0,   hms,  3600, false, utc+hms-z+ud );
  testParse (t, "12:34+01:00",               11,    0,  0,  0, 0,   0,  0,    hm,  3600, false, utc+hm-z+ud  );
  testParse (t, "12:34+01",                   8,    0,  0,  0, 0,   0,  0,    hm,  3600, false, utc+hm-z+ud  );
  testParse (t, "12:34:56",                   8,    0,  0,  0, 0,   0,  0,   hms,     0, false, local+hms+ld );
  testParse (t, "12:34",                      5,    0,  0,  0, 0,   0,  0,    hm,     0, false, local+hm+ld  );

  // datetime-ext
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "2013-12-06",                10, year, mo,  0, 0,   0,  6,     0,     0, false, local6    );
  testParse (t, "2013-340",                   8, year,  0,  0, 0, 340,  0,     0,     0, false, local6    );
  testParse (t, "2013-W49-5",                10, year,  0, 49, 5,   0,  0,     0,     0, false, local6    );
  testParse (t, "2013-W49",                   8, year,  0, 49, 0,   0,  0,     0,     0, false, local1    );
  testParse (t, "2013-12",                    7, year, mo,  0, 0,   0,  1,     0,     0, false, local1    );

  testParse (t, "2013-12-06T12:34:56",       19, year, mo,  0, 0,   0,  6,   hms,     0, false, local6+hms);
  testParse (t, "2013-12-06T12:34",          16, year, mo,  0, 0,   0,  6,    hm,     0, false, local6+hm );
  testParse (t, "2013-340T12:34:56",         17, year,  0,  0, 0, 340,  0,   hms,     0, false, local6+hms);
  testParse (t, "2013-340T12:34",            14, year,  0,  0, 0, 340,  0,    hm,     0, false, local6+hm );
  testParse (t, "2013-W49-5T12:34:56",       19, year,  0, 49, 5,   0,  0,   hms,     0, false, local6+hms);
  testParse (t, "2013-W49-5T12:34",          16, year,  0, 49, 5,   0,  0,    hm,     0, false, local6+hm );
  testParse (t, "2013-W49T12:34:56",         17, year,  0, 49, 0,   0,  0,   hms,     0, false, local1+hms);
  testParse (t, "2013-W49T12:34",            14, year,  0, 49, 0,   0,  0,    hm,     0, false, local1+hm );

  testParse (t, "2013-12-06T12:34:56Z",      20, year, mo,  0, 0,   0,  6,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013-12-06T12:34Z",         17, year, mo,  0, 0,   0,  6,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013-340T12:34:56Z",        18, year,  0,  0, 0, 340,  0,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013-340T12:34Z",           15, year,  0,  0, 0, 340,  0,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013-W49-5T12:34:56Z",      20, year,  0, 49, 5,   0,  0,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013-W49-5T12:34Z",         17, year,  0, 49, 5,   0,  0,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013-W49T12:34:56Z",        18, year,  0, 49, 0,   0,  0,   hms,     0,  true, utc1+hms  );
  testParse (t, "2013-W49T12:34Z",           15, year,  0, 49, 0,   0,  0,    hm,     0,  true, utc1+hm   );

  testParse (t, "2013-12-06T12:34:56+01:00", 25, year, mo,  0, 0,   0,  6,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-12-06T12:34:56+01",    22, year, mo,  0, 0,   0,  6,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-12-06T12:34:56-01:00", 25, year, mo,  0, 0,   0,  6,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-12-06T12:34:56-01",    22, year, mo,  0, 0,   0,  6,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-12-06T12:34+01:00",    22, year, mo,  0, 0,   0,  6,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-12-06T12:34+01",       19, year, mo,  0, 0,   0,  6,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-12-06T12:34-01:00",    22, year, mo,  0, 0,   0,  6,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-12-06T12:34-01",       19, year, mo,  0, 0,   0,  6,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-340T12:34:56+01:00",   23, year,  0,  0, 0, 340,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-340T12:34:56+01",      20, year,  0,  0, 0, 340,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-340T12:34:56-01:00",   23, year,  0,  0, 0, 340,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-340T12:34:56-01",      20, year,  0,  0, 0, 340,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-340T12:34+01:00",      20, year,  0,  0, 0, 340,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-340T12:34+01",         17, year,  0,  0, 0, 340,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-340T12:34-01:00",      20, year,  0,  0, 0, 340,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-340T12:34-01",         17, year,  0,  0, 0, 340,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-W49-5T12:34:56+01:00", 25, year,  0, 49, 5,   0,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-W49-5T12:34:56+01",    22, year,  0, 49, 5,   0,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013-W49-5T12:34:56-01:00", 25, year,  0, 49, 5,   0,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-W49-5T12:34:56-01",    22, year,  0, 49, 5,   0,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013-W49-5T12:34+01:00",    22, year,  0, 49, 5,   0,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-W49-5T12:34+01",       19, year,  0, 49, 5,   0,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013-W49-5T12:34-01:00",    22, year,  0, 49, 5,   0,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-W49-5T12:34-01",       19, year,  0, 49, 5,   0,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013-W49T12:34:56+01:00",   23, year,  0, 49, 0,   0,  0,   hms,  3600, false, utc1+hms-z);
  testParse (t, "2013-W49T12:34:56+01",      20, year,  0, 49, 0,   0,  0,   hms,  3600, false, utc1+hms-z);
  testParse (t, "2013-W49T12:34:56-01:00",   23, year,  0, 49, 0,   0,  0,   hms, -3600, false, utc1+hms+z);
  testParse (t, "2013-W49T12:34:56-01",      20, year,  0, 49, 0,   0,  0,   hms, -3600, false, utc1+hms+z);
  testParse (t, "2013-W49T12:34+01:00",      20, year,  0, 49, 0,   0,  0,    hm,  3600, false, utc1+hm-z );
  testParse (t, "2013-W49T12:34+01",         17, year,  0, 49, 0,   0,  0,    hm,  3600, false, utc1+hm-z );
  testParse (t, "2013-W49T12:34-01:00",      20, year,  0, 49, 0,   0,  0,    hm, -3600, false, utc1+hm+z );
  testParse (t, "2013-W49T12:34-01",         17, year,  0, 49, 0,   0,  0,    hm, -3600, false, utc1+hm+z );

  // datetime-ext in the future
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "9850-12-06",                10, f_yr, mo,  0, 0,   0,  6,     0,     0, false, f_local6    );
  testParse (t, "9850-340",                   8, f_yr,  0,  0, 0, 340,  0,     0,     0, false, f_local6    );
  testParse (t, "9850-W49-5",                10, f_yr,  0, 49, 5,   0,  0,     0,     0, false, f_local6    );
  testParse (t, "9850-W49",                   8, f_yr,  0, 49, 0,   0,  0,     0,     0, false, f_local1    );
  testParse (t, "9850-12",                    7, f_yr, mo,  0, 0,   0,  1,     0,     0, false, f_local1    );

  testParse (t, "9850-12-06T12:34:56",       19, f_yr, mo,  0, 0,   0,  6,   hms,     0, false, f_local6+hms);
  testParse (t, "9850-12-06T12:34",          16, f_yr, mo,  0, 0,   0,  6,    hm,     0, false, f_local6+hm );
  testParse (t, "9850-340T12:34:56",         17, f_yr,  0,  0, 0, 340,  0,   hms,     0, false, f_local6+hms);
  testParse (t, "9850-340T12:34",            14, f_yr,  0,  0, 0, 340,  0,    hm,     0, false, f_local6+hm );
  testParse (t, "9850-W49-5T12:34:56",       19, f_yr,  0, 49, 5,   0,  0,   hms,     0, false, f_local6+hms);
  testParse (t, "9850-W49-5T12:34",          16, f_yr,  0, 49, 5,   0,  0,    hm,     0, false, f_local6+hm );
  testParse (t, "9850-W49T12:34:56",         17, f_yr,  0, 49, 0,   0,  0,   hms,     0, false, f_local1+hms);
  testParse (t, "9850-W49T12:34",            14, f_yr,  0, 49, 0,   0,  0,    hm,     0, false, f_local1+hm );

  testParse (t, "9850-12-06T12:34:56Z",      20, f_yr, mo,  0, 0,   0,  6,   hms,     0,  true, f_utc6+hms  );
  testParse (t, "9850-12-06T12:34Z",         17, f_yr, mo,  0, 0,   0,  6,    hm,     0,  true, f_utc6+hm   );
  testParse (t, "9850-340T12:34:56Z",        18, f_yr,  0,  0, 0, 340,  0,   hms,     0,  true, f_utc6+hms  );
  testParse (t, "9850-340T12:34Z",           15, f_yr,  0,  0, 0, 340,  0,    hm,     0,  true, f_utc6+hm   );
  testParse (t, "9850-W49-5T12:34:56Z",      20, f_yr,  0, 49, 5,   0,  0,   hms,     0,  true, f_utc6+hms  );
  testParse (t, "9850-W49-5T12:34Z",         17, f_yr,  0, 49, 5,   0,  0,    hm,     0,  true, f_utc6+hm   );
  testParse (t, "9850-W49T12:34:56Z",        18, f_yr,  0, 49, 0,   0,  0,   hms,     0,  true, f_utc1+hms  );
  testParse (t, "9850-W49T12:34Z",           15, f_yr,  0, 49, 0,   0,  0,    hm,     0,  true, f_utc1+hm   );

  testParse (t, "9850-12-06T12:34:56+01:00", 25, f_yr, mo,  0, 0,   0,  6,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850-12-06T12:34:56+01",    22, f_yr, mo,  0, 0,   0,  6,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850-12-06T12:34:56-01:00", 25, f_yr, mo,  0, 0,   0,  6,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850-12-06T12:34:56-01",    22, f_yr, mo,  0, 0,   0,  6,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850-12-06T12:34+01:00",    22, f_yr, mo,  0, 0,   0,  6,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850-12-06T12:34+01",       19, f_yr, mo,  0, 0,   0,  6,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850-12-06T12:34-01:00",    22, f_yr, mo,  0, 0,   0,  6,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850-12-06T12:34-01",       19, f_yr, mo,  0, 0,   0,  6,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850-340T12:34:56+01:00",   23, f_yr,  0,  0, 0, 340,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850-340T12:34:56+01",      20, f_yr,  0,  0, 0, 340,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850-340T12:34:56-01:00",   23, f_yr,  0,  0, 0, 340,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850-340T12:34:56-01",      20, f_yr,  0,  0, 0, 340,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850-340T12:34+01:00",      20, f_yr,  0,  0, 0, 340,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850-340T12:34+01",         17, f_yr,  0,  0, 0, 340,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850-340T12:34-01:00",      20, f_yr,  0,  0, 0, 340,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850-340T12:34-01",         17, f_yr,  0,  0, 0, 340,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850-W49-5T12:34:56+01:00", 25, f_yr,  0, 49, 5,   0,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850-W49-5T12:34:56+01",    22, f_yr,  0, 49, 5,   0,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850-W49-5T12:34:56-01:00", 25, f_yr,  0, 49, 5,   0,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850-W49-5T12:34:56-01",    22, f_yr,  0, 49, 5,   0,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850-W49-5T12:34+01:00",    22, f_yr,  0, 49, 5,   0,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850-W49-5T12:34+01",       19, f_yr,  0, 49, 5,   0,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850-W49-5T12:34-01:00",    22, f_yr,  0, 49, 5,   0,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850-W49-5T12:34-01",       19, f_yr,  0, 49, 5,   0,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850-W49T12:34:56+01:00",   23, f_yr,  0, 49, 0,   0,  0,   hms,  3600, false, f_utc1+hms-z);
  testParse (t, "9850-W49T12:34:56+01",      20, f_yr,  0, 49, 0,   0,  0,   hms,  3600, false, f_utc1+hms-z);
  testParse (t, "9850-W49T12:34:56-01:00",   23, f_yr,  0, 49, 0,   0,  0,   hms, -3600, false, f_utc1+hms+z);
  testParse (t, "9850-W49T12:34:56-01",      20, f_yr,  0, 49, 0,   0,  0,   hms, -3600, false, f_utc1+hms+z);
  testParse (t, "9850-W49T12:34+01:00",      20, f_yr,  0, 49, 0,   0,  0,    hm,  3600, false, f_utc1+hm-z );
  testParse (t, "9850-W49T12:34+01",         17, f_yr,  0, 49, 0,   0,  0,    hm,  3600, false, f_utc1+hm-z );
  testParse (t, "9850-W49T12:34-01:00",      20, f_yr,  0, 49, 0,   0,  0,    hm, -3600, false, f_utc1+hm+z );
  testParse (t, "9850-W49T12:34-01",         17, f_yr,  0, 49, 0,   0,  0,    hm, -3600, false, f_utc1+hm+z );

  // The only non-extended forms.
  testParse (t, "20131206T123456Z",          16, year, mo,  0, 0,   0,  6,   hms,     0,  true, utc6+hms  );
  testParse (t, "20131206T123456",           15, year, mo,  0, 0,   0,  6,   hms,     0, false, local6+hms);

  // The only non-extended forms - future
  testParse (t, "98501206T123456Z",          16, f_yr, mo,  0, 0,   0,  6,   hms,     0,  true, f_utc6+hms  );
  testParse (t, "98501206T123456",           15, f_yr, mo,  0, 0,   0,  6,   hms,     0, false, f_local6+hms);

  // Non-extended forms.

  // time
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "123456Z",                    7,    0,  0,  0, 0,   0,  0,   hms,     0,  true, utc+hms+ud   );
  testParse (t, "1234Z",                      5,    0,  0,  0, 0,   0,  0,    hm,     0,  true, utc+hm+ud    );
  testParse (t, "123456+0100",               11,    0,  0,  0, 0,   0,  0,   hms,  3600, false, utc+hms-z+ud );
  testParse (t, "123456+01",                  9,    0,  0,  0, 0,   0,  0,   hms,  3600, false, utc+hms-z+ud );
  testParse (t, "1234+0100",                  9,    0,  0,  0, 0,   0,  0,    hm,  3600, false, utc+hm-z+ud  );
  testParse (t, "1234+01",                    7,    0,  0,  0, 0,   0,  0,    hm,  3600, false, utc+hm-z+ud  );
  testParse (t, "123456",                     6,    0,  0,  0, 0,   0,  0,   hms,     0, false, local+hms+ld );
  testParse (t, "1234",                       4,    0,  0,  0, 0,   0,  0,    hm,     0, false, local+hm+ld  );

  // datetime
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "20131206",                   8, year, mo,  0, 0,   0,  6,     0,     0, false, local6    );
  testParse (t, "2013340",                    7, year,  0,  0, 0, 340,  0,     0,     0, false, local6    );
  testParse (t, "2013W495",                   8, year,  0, 49, 5,   0,  0,     0,     0, false, local6    );
  testParse (t, "2013W49",                    7, year,  0, 49, 0,   0,  0,     0,     0, false, local1    );
  testParse (t, "201312",                     6, year, mo,  0, 0,   0,  1,     0,     0, false, local1    );

  testParse (t, "20131206T123456",           15, year, mo,  0, 0,   0,  6,   hms,     0, false, local6+hms);
  testParse (t, "20131206T1234",             13, year, mo,  0, 0,   0,  6,    hm,     0, false, local6+hm );
  testParse (t, "2013340T123456",            14, year,  0,  0, 0, 340,  0,   hms,     0, false, local6+hms);
  testParse (t, "2013340T1234",              12, year,  0,  0, 0, 340,  0,    hm,     0, false, local6+hm );
  testParse (t, "2013W495T123456",           15, year,  0, 49, 5,   0,  0,   hms,     0, false, local6+hms);
  testParse (t, "2013W495T1234",             13, year,  0, 49, 5,   0,  0,    hm,     0, false, local6+hm );
  testParse (t, "2013W49T123456",            14, year,  0, 49, 0,   0,  0,   hms,     0, false, local1+hms);
  testParse (t, "2013W49T1234",              12, year,  0, 49, 0,   0,  0,    hm,     0, false, local1+hm );

  testParse (t, "20131206T123456Z",          16, year, mo,  0, 0,   0,  6,   hms,     0,  true, utc6+hms  );
  testParse (t, "20131206T1234Z",            14, year, mo,  0, 0,   0,  6,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013340T123456Z",           15, year,  0,  0, 0, 340,  0,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013340T1234Z",             13, year,  0,  0, 0, 340,  0,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013W495T123456Z",          16, year,  0, 49, 5,   0,  0,   hms,     0,  true, utc6+hms  );
  testParse (t, "2013W495T1234Z",            14, year,  0, 49, 5,   0,  0,    hm,     0,  true, utc6+hm   );
  testParse (t, "2013W49T123456Z",           15, year,  0, 49, 0,   0,  0,   hms,     0,  true, utc1+hms  );
  testParse (t, "2013W49T1234Z",             13, year,  0, 49, 0,   0,  0,    hm,     0,  true, utc1+hm   );

  testParse (t, "20131206T123456+0100",      20, year, mo,  0, 0,   0,  6,   hms,  3600, false, utc6+hms-z);
  testParse (t, "20131206T123456+01",        18, year, mo,  0, 0,   0,  6,   hms,  3600, false, utc6+hms-z);
  testParse (t, "20131206T123456-0100",      20, year, mo,  0, 0,   0,  6,   hms, -3600, false, utc6+hms+z);
  testParse (t, "20131206T123456-01",        18, year, mo,  0, 0,   0,  6,   hms, -3600, false, utc6+hms+z);
  testParse (t, "20131206T1234+0100",        18, year, mo,  0, 0,   0,  6,    hm,  3600, false, utc6+hm-z );
  testParse (t, "20131206T1234+01",          16, year, mo,  0, 0,   0,  6,    hm,  3600, false, utc6+hm-z );
  testParse (t, "20131206T1234-0100",        18, year, mo,  0, 0,   0,  6,    hm, -3600, false, utc6+hm+z );
  testParse (t, "20131206T1234-01",          16, year, mo,  0, 0,   0,  6,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013340T123456+0100",       19, year,  0,  0, 0, 340,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013340T123456+01",         17, year,  0,  0, 0, 340,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013340T123456-0100",       19, year,  0,  0, 0, 340,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013340T123456-01",         17, year,  0,  0, 0, 340,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013340T1234+0100",         17, year,  0,  0, 0, 340,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013340T1234+01",           15, year,  0,  0, 0, 340,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013340T1234-0100",         17, year,  0,  0, 0, 340,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013340T1234-01",           15, year,  0,  0, 0, 340,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013W495T123456+0100",      20, year,  0, 49, 5,   0,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013W495T123456+01",        18, year,  0, 49, 5,   0,  0,   hms,  3600, false, utc6+hms-z);
  testParse (t, "2013W495T123456-0100",      20, year,  0, 49, 5,   0,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013W495T123456-01",        18, year,  0, 49, 5,   0,  0,   hms, -3600, false, utc6+hms+z);
  testParse (t, "2013W495T1234+0100",        18, year,  0, 49, 5,   0,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013W495T1234+01",          16, year,  0, 49, 5,   0,  0,    hm,  3600, false, utc6+hm-z );
  testParse (t, "2013W495T1234-0100",        18, year,  0, 49, 5,   0,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013W495T1234-01",          16, year,  0, 49, 5,   0,  0,    hm, -3600, false, utc6+hm+z );
  testParse (t, "2013W49T123456+0100",       19, year,  0, 49, 0,   0,  0,   hms,  3600, false, utc1+hms-z);
  testParse (t, "2013W49T123456+01",         17, year,  0, 49, 0,   0,  0,   hms,  3600, false, utc1+hms-z);
  testParse (t, "2013W49T123456-0100",       19, year,  0, 49, 0,   0,  0,   hms, -3600, false, utc1+hms+z);
  testParse (t, "2013W49T123456-01",         17, year,  0, 49, 0,   0,  0,   hms, -3600, false, utc1+hms+z);
  testParse (t, "2013W49T1234+0100",         17, year,  0, 49, 0,   0,  0,    hm,  3600, false, utc1+hm-z );
  testParse (t, "2013W49T1234+01",           15, year,  0, 49, 0,   0,  0,    hm,  3600, false, utc1+hm-z );
  testParse (t, "2013W49T1234-0100",         17, year,  0, 49, 0,   0,  0,    hm, -3600, false, utc1+hm+z );
  testParse (t, "2013W49T1234-01",           15, year,  0, 49, 0,   0,  0,    hm, -3600, false, utc1+hm+z );

  // datetime - future
  //            input                         i  Year  Mo  Wk WD  Jul  Da   Secs     TZ    UTC      time_t
  testParse (t, "98501206",                   8, f_yr, mo,  0, 0,   0,  6,     0,     0, false, f_local6    );
  testParse (t, "9850340",                    7, f_yr,  0,  0, 0, 340,  0,     0,     0, false, f_local6    );
  testParse (t, "9850W495",                   8, f_yr,  0, 49, 5,   0,  0,     0,     0, false, f_local6    );
  testParse (t, "9850W49",                    7, f_yr,  0, 49, 0,   0,  0,     0,     0, false, f_local1    );
  testParse (t, "985012",                     6, f_yr, mo,  0, 0,   0,  1,     0,     0, false, f_local1    );

  testParse (t, "98501206T123456",           15, f_yr, mo,  0, 0,   0,  6,   hms,     0, false, f_local6+hms);
  testParse (t, "98501206T1234",             13, f_yr, mo,  0, 0,   0,  6,    hm,     0, false, f_local6+hm );
  testParse (t, "9850340T123456",            14, f_yr,  0,  0, 0, 340,  0,   hms,     0, false, f_local6+hms);
  testParse (t, "9850340T1234",              12, f_yr,  0,  0, 0, 340,  0,    hm,     0, false, f_local6+hm );
  testParse (t, "9850W495T123456",           15, f_yr,  0, 49, 5,   0,  0,   hms,     0, false, f_local6+hms);
  testParse (t, "9850W495T1234",             13, f_yr,  0, 49, 5,   0,  0,    hm,     0, false, f_local6+hm );
  testParse (t, "9850W49T123456",            14, f_yr,  0, 49, 0,   0,  0,   hms,     0, false, f_local1+hms);
  testParse (t, "9850W49T1234",              12, f_yr,  0, 49, 0,   0,  0,    hm,     0, false, f_local1+hm );

  testParse (t, "98501206T123456Z",          16, f_yr, mo,  0, 0,   0,  6,   hms,     0,  true, f_utc6+hms  );
  testParse (t, "98501206T1234Z",            14, f_yr, mo,  0, 0,   0,  6,    hm,     0,  true, f_utc6+hm   );
  testParse (t, "9850340T123456Z",           15, f_yr,  0,  0, 0, 340,  0,   hms,     0,  true, f_utc6+hms  );
  testParse (t, "9850340T1234Z",             13, f_yr,  0,  0, 0, 340,  0,    hm,     0,  true, f_utc6+hm   );
  testParse (t, "9850W495T123456Z",          16, f_yr,  0, 49, 5,   0,  0,   hms,     0,  true, f_utc6+hms  );
  testParse (t, "9850W495T1234Z",            14, f_yr,  0, 49, 5,   0,  0,    hm,     0,  true, f_utc6+hm   );
  testParse (t, "9850W49T123456Z",           15, f_yr,  0, 49, 0,   0,  0,   hms,     0,  true, f_utc1+hms  );
  testParse (t, "9850W49T1234Z",             13, f_yr,  0, 49, 0,   0,  0,    hm,     0,  true, f_utc1+hm   );

  testParse (t, "98501206T123456+0100",      20, f_yr, mo,  0, 0,   0,  6,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "98501206T123456+01",        18, f_yr, mo,  0, 0,   0,  6,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "98501206T123456-0100",      20, f_yr, mo,  0, 0,   0,  6,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "98501206T123456-01",        18, f_yr, mo,  0, 0,   0,  6,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "98501206T1234+0100",        18, f_yr, mo,  0, 0,   0,  6,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "98501206T1234+01",          16, f_yr, mo,  0, 0,   0,  6,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "98501206T1234-0100",        18, f_yr, mo,  0, 0,   0,  6,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "98501206T1234-01",          16, f_yr, mo,  0, 0,   0,  6,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850340T123456+0100",       19, f_yr,  0,  0, 0, 340,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850340T123456+01",         17, f_yr,  0,  0, 0, 340,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850340T123456-0100",       19, f_yr,  0,  0, 0, 340,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850340T123456-01",         17, f_yr,  0,  0, 0, 340,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850340T1234+0100",         17, f_yr,  0,  0, 0, 340,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850340T1234+01",           15, f_yr,  0,  0, 0, 340,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850340T1234-0100",         17, f_yr,  0,  0, 0, 340,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850340T1234-01",           15, f_yr,  0,  0, 0, 340,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850W495T123456+0100",      20, f_yr,  0, 49, 5,   0,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850W495T123456+01",        18, f_yr,  0, 49, 5,   0,  0,   hms,  3600, false, f_utc6+hms-z);
  testParse (t, "9850W495T123456-0100",      20, f_yr,  0, 49, 5,   0,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850W495T123456-01",        18, f_yr,  0, 49, 5,   0,  0,   hms, -3600, false, f_utc6+hms+z);
  testParse (t, "9850W495T1234+0100",        18, f_yr,  0, 49, 5,   0,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850W495T1234+01",          16, f_yr,  0, 49, 5,   0,  0,    hm,  3600, false, f_utc6+hm-z );
  testParse (t, "9850W495T1234-0100",        18, f_yr,  0, 49, 5,   0,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850W495T1234-01",          16, f_yr,  0, 49, 5,   0,  0,    hm, -3600, false, f_utc6+hm+z );
  testParse (t, "9850W49T123456+0100",       19, f_yr,  0, 49, 0,   0,  0,   hms,  3600, false, f_utc1+hms-z);
  testParse (t, "9850W49T123456+01",         17, f_yr,  0, 49, 0,   0,  0,   hms,  3600, false, f_utc1+hms-z);
  testParse (t, "9850W49T123456-0100",       19, f_yr,  0, 49, 0,   0,  0,   hms, -3600, false, f_utc1+hms+z);
  testParse (t, "9850W49T123456-01",         17, f_yr,  0, 49, 0,   0,  0,   hms, -3600, false, f_utc1+hms+z);
  testParse (t, "9850W49T1234+0100",         17, f_yr,  0, 49, 0,   0,  0,    hm,  3600, false, f_utc1+hm-z );
  testParse (t, "9850W49T1234+01",           15, f_yr,  0, 49, 0,   0,  0,    hm,  3600, false, f_utc1+hm-z );
  testParse (t, "9850W49T1234-0100",         17, f_yr,  0, 49, 0,   0,  0,    hm, -3600, false, f_utc1+hm+z );
  testParse (t, "9850W49T1234-01",           15, f_yr,  0, 49, 0,   0,  0,    hm, -3600, false, f_utc1+hm+z );

  // Informal time.
  int t8a   = (8 * 3600);
  int t830a = (8 * 3600) + (30 * 60);
  int t8p   = (20 * 3600);
  int t830p = (20 * 3600) + (30 * 60);
  int t12p  = (12 * 3600);
  int t1p   = (13 * 3600);

  Datetime time_now;
  int adjust = (time_now.hour () > 10 || (time_now.hour () == 10 && time_now.minute () > 30)) ? 86400 : 0;
  testParse (t, "10:30am",                    7,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830a+adjust+(2*3600));

  adjust = (time_now.hour () > 8 || (time_now.hour () == 8 && time_now.minute () > 30)) ? 86400 : 0;
  testParse (t, "8:30am",                     6,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830a+adjust);
  testParse (t, "8:30a",                      5,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830a+adjust);
  testParse (t, "8:30",                       4,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830a+adjust);

  adjust = (time_now.hour () >= 8) ? 86400 : 0;
  testParse (t, "8am",                        3,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8a+adjust);
  testParse (t, "8a",                         2,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8a+adjust);

  adjust = (time_now.hour () > 20 || (time_now.hour () == 20 && time_now.minute () > 30)) ? 86400 : 0;
  testParse (t, "8:30pm",                     6,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830p+adjust);
  testParse (t, "8:30p",                      5,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830p+adjust);

  adjust = (time_now.hour () >= 20) ? 86400 : 0;
  testParse (t, "8pm",                        3,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8p+adjust);
  testParse (t, "8p",                         2,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8p+adjust);

  adjust = (time_now.hour () >= 12) ? 86400 : 0;
  testParse (t, "12pm",                       4,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t12p+adjust);

  adjust = (time_now.hour () >= 13) ? 86400 : 0;
  testParse (t, "1pm",                        3,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t1p+adjust);

  try
  {
    Datetime now;
    t.ok (now.toISO ().find ("1969") == std::string::npos, "'now' != 1969");

    Datetime yesterday;
    yesterday -= 86400;
    Datetime tomorrow;
    tomorrow += 86400;

    t.ok    (yesterday <= now,       "yesterday <= now");
    t.ok    (yesterday <  now,       "yesterday < now");
    t.notok (yesterday == now,       "!(yesterday == now)");
    t.ok    (yesterday != now,       "yesterday != now");
    t.ok    (now       >= yesterday, "now >= yesterday");
    t.ok    (now       >  yesterday, "now > yesterday");

    t.ok    (tomorrow >= now,        "tomorrow >= now");
    t.ok    (tomorrow >  now,        "tomorrow > now");
    t.notok (tomorrow == now,        "!(tomorrow == now)");
    t.ok    (tomorrow != now,        "tomorrow != now");
    t.ok    (now      <= tomorrow,   "now <= tomorrow");
    t.ok    (now      <  tomorrow,   "now < tomorrow");

    // ctor ("now")
    Datetime::weekstart = 1;
    Datetime relative_now;
    t.ok (relative_now.sameHour (now),  "Datetime ().sameHour (Datetime (now))");
    t.ok (relative_now.sameDay (now),   "Datetime ().sameDay (Datetime (now))");
    t.ok (relative_now.sameWeek (now),  "Datetime ().sameWeek (Datetime (now))");
    t.ok (relative_now.sameMonth (now), "Datetime ().sameMonth (Datetime (now))");
    t.ok (relative_now.sameYear (now),  "Datetime ().sameYear (Datetime (now))");

    // Loose comparisons.
    Datetime left ("7/4/2008", "m/d/Y");
    Datetime comp1 ("7/4/2008", "m/d/Y");
    t.ok (left.sameDay     (comp1), "7/4/2008 is on the same day as 7/4/2008");
    t.ok (left.sameWeek    (comp1), "7/4/2008 is on the same week as 7/4/2008");
    t.ok (left.sameMonth   (comp1), "7/4/2008 is in the same month as 7/4/2008");
    t.ok (left.sameQuarter (comp1), "7/4/2008 is in the same quarter as 7/4/2008");
    t.ok (left.sameYear    (comp1), "7/4/2008 is in the same year as 7/4/2008");

    Datetime comp2 ("7/5/2008", "m/d/Y");
    t.notok (left.sameDay     (comp2), "7/4/2008 is not on the same day as 7/5/2008");
    t.ok    (left.sameMonth   (comp2), "7/4/2008 is in the same month as 7/5/2008");
    t.ok    (left.sameQuarter (comp2), "7/4/2008 is in the same quarter as 7/5/2008");
    t.ok    (left.sameYear    (comp2), "7/4/2008 is in the same year as 7/5/2008");

    Datetime comp3 ("8/4/2008", "m/d/Y");
    t.notok (left.sameDay     (comp3), "7/4/2008 is not on the same day as 8/4/2008");
    t.notok (left.sameWeek    (comp3), "7/4/2008 is not on the same week as 8/4/2008");
    t.notok (left.sameMonth   (comp3), "7/4/2008 is not in the same month as 8/4/2008");
    t.ok    (left.sameQuarter (comp3), "7/4/2008 is in the same quarter as 8/4/2008");
    t.ok    (left.sameYear    (comp3), "7/4/2008 is in the same year as 8/4/2008");

    Datetime comp4 ("7/4/2009", "m/d/Y");
    t.notok (left.sameDay     (comp4), "7/4/2008 is not on the same day as 7/4/2009");
    t.notok (left.sameWeek    (comp4), "7/4/2008 is not on the same week as 7/4/2009");
    t.notok (left.sameMonth   (comp4), "7/4/2008 is not in the same month as 7/4/2009");
    t.notok (left.sameQuarter (comp4), "7/4/2008 is not in the same quarter as 7/4/2009");
    t.notok (left.sameYear    (comp4), "7/4/2008 is not in the same year as 7/4/2009");

    // Validity.
    t.ok    (Datetime::valid (2008, 2, 29), "valid: 2/29/2008");
    t.notok (Datetime::valid (2007, 2, 29), "invalid: 2/29/2007");

    t.ok    (Datetime::valid ("2/29/2008", "m/d/Y"), "valid: 2/29/2008");
    t.notok (Datetime::valid ("2/29/2007", "m/d/Y"), "invalid: 2/29/2007");

    t.ok    (Datetime::valid (2008, 366), "valid: 366 days in 2008");
    t.notok (Datetime::valid (2007, 366), "invalid: 366 days in 2007");

    // Time validity.
    t.ok    (Datetime::valid (2010, 2, 28,  0,  0,  0), "valid 2/28/2010 0:00:00");
    t.ok    (Datetime::valid (2010, 2, 28, 23, 59, 59), "valid 2/28/2010 23:59:59");
    t.notok (Datetime::valid (2010, 2, 28, 24, 59, 59), "valid 2/28/2010 24:59:59");
    t.notok (Datetime::valid (2010, 2, 28, -1,  0,  0), "valid 2/28/2010 -1:00:00");

    // Leap year.
    t.ok    (Datetime::leapYear (2008), "2008 is a leap year");
    t.notok (Datetime::leapYear (2007), "2007 is not a leap year");
    t.ok    (Datetime::leapYear (2000), "2000 is a leap year");
    t.notok (Datetime::leapYear (1900), "1900 is not a leap year");

    // Days in year.
    t.is (Datetime::daysInYear (2016), 366, "366 days in 2016");
    t.is (Datetime::daysInYear (2015), 365, "365 days in 2015");

    // Days in month.
    t.is (Datetime::daysInMonth (2008, 2), 29, "29 days in February 2008");
    t.is (Datetime::daysInMonth (2007, 2), 28, "28 days in February 2007");

    // Names.
    t.is (Datetime::monthName (1),  "January",   "1 = January");
    t.is (Datetime::monthName (2),  "February",  "2 = February");
    t.is (Datetime::monthName (3),  "March",     "3 = March");
    t.is (Datetime::monthName (4),  "April",     "4 = April");
    t.is (Datetime::monthName (5),  "May",       "5 = May");
    t.is (Datetime::monthName (6),  "June",      "6 = June");
    t.is (Datetime::monthName (7),  "July",      "7 = July");
    t.is (Datetime::monthName (8),  "August",    "8 = August");
    t.is (Datetime::monthName (9),  "September", "9 = September");
    t.is (Datetime::monthName (10), "October",   "10 = October");
    t.is (Datetime::monthName (11), "November",  "11 = November");
    t.is (Datetime::monthName (12), "December",  "12 = December");

    // Names.
    t.is (Datetime::monthNameShort (1),  "Jan", "1 = Jan");
    t.is (Datetime::monthNameShort (2),  "Feb", "2 = Feb");
    t.is (Datetime::monthNameShort (3),  "Mar", "3 = Mar");
    t.is (Datetime::monthNameShort (4),  "Apr", "4 = Apr");
    t.is (Datetime::monthNameShort (5),  "May", "5 = May");
    t.is (Datetime::monthNameShort (6),  "Jun", "6 = Jun");
    t.is (Datetime::monthNameShort (7),  "Jul", "7 = Jul");
    t.is (Datetime::monthNameShort (8),  "Aug", "8 = Aug");
    t.is (Datetime::monthNameShort (9),  "Sep", "9 = Sep");
    t.is (Datetime::monthNameShort (10), "Oct", "10 = Oct");
    t.is (Datetime::monthNameShort (11), "Nov", "11 = Nov");
    t.is (Datetime::monthNameShort (12), "Dec", "12 = Dec");

    // Names.
    t.is (Datetime::monthOfYear ("January"),   1,  "January   =  1");
    t.is (Datetime::monthOfYear ("February"),  2,  "February  =  2");
    t.is (Datetime::monthOfYear ("March"),     3,  "March     =  3");
    t.is (Datetime::monthOfYear ("April"),     4,  "April     =  4");
    t.is (Datetime::monthOfYear ("May"),       5,  "May       =  5");
    t.is (Datetime::monthOfYear ("June"),      6,  "June      =  6");
    t.is (Datetime::monthOfYear ("July"),      7,  "July      =  7");
    t.is (Datetime::monthOfYear ("August"),    8,  "August    =  8");
    t.is (Datetime::monthOfYear ("September"), 9,  "September =  9");
    t.is (Datetime::monthOfYear ("October"),   10, "October   = 10");
    t.is (Datetime::monthOfYear ("November"),  11, "November  = 11");
    t.is (Datetime::monthOfYear ("December"),  12, "December  = 12");

    t.is (Datetime::dayName (0), "Sunday",    "0 == Sunday");
    t.is (Datetime::dayName (1), "Monday",    "1 == Monday");
    t.is (Datetime::dayName (2), "Tuesday",   "2 == Tuesday");
    t.is (Datetime::dayName (3), "Wednesday", "3 == Wednesday");
    t.is (Datetime::dayName (4), "Thursday",  "4 == Thursday");
    t.is (Datetime::dayName (5), "Friday",    "5 == Friday");
    t.is (Datetime::dayName (6), "Saturday",  "6 == Saturday");

    t.is (Datetime::dayNameShort (0), "Sun",  "0 == Sun");
    t.is (Datetime::dayNameShort (1), "Mon",  "1 == Mon");
    t.is (Datetime::dayNameShort (2), "Tue",  "2 == Tue");
    t.is (Datetime::dayNameShort (3), "Wed",  "3 == Wed");
    t.is (Datetime::dayNameShort (4), "Thu",  "4 == Thu");
    t.is (Datetime::dayNameShort (5), "Fri",  "5 == Fri");
    t.is (Datetime::dayNameShort (6), "Sat",  "6 == Sat");

    t.is (Datetime::dayOfWeek ("SUNDAY"),    0, "SUNDAY == 0");
    t.is (Datetime::dayOfWeek ("sunday"),    0, "sunday == 0");
    t.is (Datetime::dayOfWeek ("Sunday"),    0, "Sunday == 0");
    t.is (Datetime::dayOfWeek ("Monday"),    1, "Monday == 1");
    t.is (Datetime::dayOfWeek ("Tuesday"),   2, "Tuesday == 2");
    t.is (Datetime::dayOfWeek ("Wednesday"), 3, "Wednesday == 3");
    t.is (Datetime::dayOfWeek ("Thursday"),  4, "Thursday == 4");
    t.is (Datetime::dayOfWeek ("Friday"),    5, "Friday == 5");
    t.is (Datetime::dayOfWeek ("Saturday"),  6, "Saturday == 6");

    Datetime happyNewYear (2008, 1, 1);
    t.is (happyNewYear.dayOfWeek (),           2, "1/1/2008 == Tuesday");
    t.is (happyNewYear.month (),               1, "1/1/2008 == January");
    t.is (happyNewYear.day (),                 1, "1/1/2008 == 1");
    t.is (happyNewYear.year (),             2008, "1/1/2008 == 2008");
    t.is (happyNewYear.toString (), "2008-01-01", "toString 2008-01-01");

    int m, d, y;
    happyNewYear.toYMD (y, m, d);
    t.is (m, 1, "1/1/2008 == January");
    t.is (d, 1, "1/1/2008 == 1");
    t.is (y, 2008, "1/1/2008 == 2008");

    Datetime epoch (2001, 9, 8);
    t.ok ((int)epoch.toEpoch () < 1000000000, "9/8/2001 < 1,000,000,000");
    epoch += 172800;
    t.ok ((int)epoch.toEpoch () > 1000000000, "9/10/2001 > 1,000,000,000");

    Datetime fromEpoch (epoch.toEpoch ());
    t.is (fromEpoch.toString (), epoch.toString (), "ctor (time_t)");

    Datetime iso (1000000000);
    t.is (iso.toISO (), "20010909T014640Z", "1,000,000,000 -> 20010909T014640Z");

    // Test for Y2038 problem
    Datetime f_epoch (2039, 1, 1);
    std::cout << "# 2039-1-1 is " << (long long)f_epoch.toEpoch () << std::endl;
    t.ok ((long long)f_epoch.toEpoch () > 2147483647, "9/01/2039 > 2,147,483,647");

    Datetime f_iso (2147483650);
    t.is (f_iso.toISO (), "20380119T031410Z", "2147483650 -> 20380119T031410Z");

    // Quantization.
    Datetime quant (1234526400);
    t.is (quant.startOfDay ().toString ("YMDHNS"),   "20090213000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 2/13/2009 0:00:00");
    t.is (quant.startOfWeek ().toString ("YMDHNS"),  "20090208000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 2/8/2009 0:00:00");
    t.is (quant.startOfMonth ().toString ("YMDHNS"), "20090201000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 2/1/2009 0:00:00");
    t.is (quant.startOfYear ().toString ("YMDHNS"),  "20090101000000", "1234526400 -> 2/13/2009 12:00:00 UTC -> 1/1/2009 0:00:00");

    // Format parsing.
    Datetime fromString1 ("1/1/2008", "m/d/Y");
    t.is (fromString1.month (),   1, "ctor (std::string) -> m");
    t.is (fromString1.day (),     1, "ctor (std::string) -> d");
    t.is (fromString1.year (), 2008, "ctor (std::string) -> y");

    Datetime fromString2 ("20080101", "YMD");
    t.is (fromString2.month (),   1, "ctor (std::string) -> m");
    t.is (fromString2.day (),     1, "ctor (std::string) -> d");
    t.is (fromString2.year (), 2008, "ctor (std::string) -> y");

    Datetime fromString3 ("12/31/2007", "m/d/Y");
    t.is (fromString3.month (),  12, "ctor (std::string) -> m");
    t.is (fromString3.day (),    31, "ctor (std::string) -> d");
    t.is (fromString3.year (), 2007, "ctor (std::string) -> y");

    Datetime fromString4 ("01/01/2008", "m/d/Y");
    t.is (fromString4.month (),   1, "ctor (std::string) -> m");
    t.is (fromString4.day (),     1, "ctor (std::string) -> d");
    t.is (fromString4.year (), 2008, "ctor (std::string) -> y");

    Datetime fromString5 ("Tue 05 Feb 2008 (06)", "a D b Y (V)");
    t.is (fromString5.month (),   2, "ctor (std::string) -> m");
    t.is (fromString5.day (),     5, "ctor (std::string) -> d");
    t.is (fromString5.year (), 2008, "ctor (std::string) -> y");

    Datetime fromString6 ("Tuesday, February 5, 2008", "A, B d, Y");
    t.is (fromString6.month (),   2, "ctor (std::string) -> m");
    t.is (fromString6.day (),     5, "ctor (std::string) -> d");
    t.is (fromString6.year (), 2008, "ctor (std::string) -> y");

    Datetime fromString7 ("w01 Tue 2008-01-01", "wV a Y-M-D");
    t.is (fromString7.month (),   1, "ctor (std::string) -> m");
    t.is (fromString7.day (),     1, "ctor (std::string) -> d");
    t.is (fromString7.year (), 2008, "ctor (std::string) -> y");

    Datetime fromString8 ("6/7/2010 1:23:45",  "m/d/Y h:N:S");
    t.is (fromString8.month (),     6, "ctor (std::string) -> m");
    t.is (fromString8.day (),       7, "ctor (std::string) -> d");
    t.is (fromString8.year (),   2010, "ctor (std::string) -> Y");
    t.is (fromString8.hour (),      1, "ctor (std::string) -> h");
    t.is (fromString8.minute (),   23, "ctor (std::string) -> N");
    t.is (fromString8.second (),   45, "ctor (std::string) -> S");

    Datetime fromString9 ("6/7/2010 01:23:45", "m/d/Y H:N:S");
    t.is (fromString9.month (),     6, "ctor (std::string) -> m");
    t.is (fromString9.day (),       7, "ctor (std::string) -> d");
    t.is (fromString9.year (),   2010, "ctor (std::string) -> Y");
    t.is (fromString9.hour (),      1, "ctor (std::string) -> h");
    t.is (fromString9.minute (),   23, "ctor (std::string) -> N");
    t.is (fromString9.second (),   45, "ctor (std::string) -> S");

    Datetime fromString10 ("6/7/2010 12:34:56", "m/d/Y H:N:S");
    t.is (fromString10.month (),     6, "ctor (std::string) -> m");
    t.is (fromString10.day (),       7, "ctor (std::string) -> d");
    t.is (fromString10.year (),   2010, "ctor (std::string) -> Y");
    t.is (fromString10.hour (),     12, "ctor (std::string) -> h");
    t.is (fromString10.minute (),   34, "ctor (std::string) -> N");
    t.is (fromString10.second (),   56, "ctor (std::string) -> S");

    Datetime fromString11 ("6/7/3010 12:34:56", "m/d/Y H:N:S");
    t.is (fromString11.month (),     6, "ctor (std::string) -> m");
    t.is (fromString11.day (),       7, "ctor (std::string) -> d");
    t.is (fromString11.year (),   3010, "ctor (std::string) -> Y");
    t.is (fromString11.hour (),     12, "ctor (std::string) -> h");
    t.is (fromString11.minute (),   34, "ctor (std::string) -> N");
    t.is (fromString11.second (),   56, "ctor (std::string) -> S");

    // Day of year
    t.is (Datetime ("1/1/2011",   "m/d/Y").dayOfYear (),   1, "dayOfYear (1/1/2011)   ->   1");
    t.is (Datetime ("5/1/2011",   "m/d/Y").dayOfYear (), 121, "dayOfYear (5/1/2011)   -> 121");
    t.is (Datetime ("12/31/2011", "m/d/Y").dayOfYear (), 365, "dayOfYear (12/31/2011) -> 365");

    // Relative dates - look ahead
    {
      Datetime::timeRelative = true;

      Datetime r1 ("today");
      t.ok (r1.sameDay (now), "today = now");

      Datetime r4 ("sunday");
      if (now.dayOfWeek () >= 0)
        t.ok (r4.sameDay (now + (0 - now.dayOfWeek () + 7) * 86400), "sunday -> next sunday");
      else
        t.ok (r4.sameDay (now + (0 - now.dayOfWeek ()) * 86400), "sunday -> next sunday");;

      Datetime r5 ("monday");
      if (now.dayOfWeek () >= 1)
        t.ok (r5.sameDay (now + (1 - now.dayOfWeek () + 7) * 86400), "monday -> next monday");
      else
        t.ok (r5.sameDay (now + (1 - now.dayOfWeek ()) * 86400), "monday -> next monday");;

      Datetime r6 ("tuesday");
      if (now.dayOfWeek () >= 2)
        t.ok (r6.sameDay (now + (2 - now.dayOfWeek () + 7) * 86400), "tuesday -> next tuesday");
      else
        t.ok (r6.sameDay (now + (2 - now.dayOfWeek ()) * 86400), "tuesday -> next tuesday");;

      Datetime r7 ("wednesday");
      if (now.dayOfWeek () >= 3)
        t.ok (r7.sameDay (now + (3 - now.dayOfWeek () + 7) * 86400), "wednesday -> next wednesday");
      else
        t.ok (r7.sameDay (now + (3 - now.dayOfWeek ()) * 86400), "wednesday -> next wednesday");;

      Datetime r8 ("thursday");
      if (now.dayOfWeek () >= 4)
        t.ok (r8.sameDay (now + (4 - now.dayOfWeek () + 7) * 86400), "thursday -> next thursday");
      else
        t.ok (r8.sameDay (now + (4 - now.dayOfWeek ()) * 86400), "thursday -> next thursday");;

      Datetime r9 ("friday");
      if (now.dayOfWeek () >= 5)
        t.ok (r9.sameDay (now + (5 - now.dayOfWeek () + 7) * 86400), "friday -> next friday");
      else
        t.ok (r9.sameDay (now + (5 - now.dayOfWeek ()) * 86400), "friday -> next friday");;

      Datetime r10 ("saturday");
      if (now.dayOfWeek () >= 6)
        t.ok (r10.sameDay (now + (6 - now.dayOfWeek () + 7) * 86400), "saturday -> next saturday");
      else
        t.ok (r10.sameDay (now + (6 - now.dayOfWeek ()) * 86400), "saturday -> next saturday");;
    }

    // Relative dates - look back
    {
      Datetime::timeRelative = false;

      Datetime r1 ("today");
      t.ok (r1.sameDay (now), "today = now");

      Datetime r4 ("sunday");
      if (now.dayOfWeek () >= 0)
        t.ok (r4.sameDay (now + (0 - now.dayOfWeek ()) * 86400), "sunday -> previous sunday" );
      else
        t.ok (r4.sameDay (now + (0 - now.dayOfWeek () - 7) * 86400), "sunday -> previous sunday");

      Datetime r5 ("monday");
      if (now.dayOfWeek () >= 1)
        t.ok (r5.sameDay (now + (1 - now.dayOfWeek ()) * 86400), "monday -> previous monday");
      else
        t.ok (r5.sameDay (now + (1 - now.dayOfWeek () - 7) * 86400), "monday -> previous monday");

      Datetime r6 ("tuesday");
      if (now.dayOfWeek () >= 2)
        t.ok (r6.sameDay (now + (2 - now.dayOfWeek ()) * 86400), "tuesday -> previous tuesday");
      else
        t.ok (r6.sameDay (now + (2 - now.dayOfWeek () - 7) * 86400), "tuesday -> previous tuesday");

      Datetime r7 ("wednesday");
      if (now.dayOfWeek () >= 3)
        t.ok (r7.sameDay (now + (3 - now.dayOfWeek ()) * 86400), "wednesday -> previous wednesday");
      else
        t.ok (r7.sameDay (now + (3 - now.dayOfWeek () - 7) * 86400), "wednesday -> previous wednesday");

      Datetime r8 ("thursday");
      if (now.dayOfWeek () >= 4)
        t.ok (r8.sameDay (now + (4 - now.dayOfWeek ()) * 86400), "thursday -> previous thursday");
      else
        t.ok (r8.sameDay (now + (4 - now.dayOfWeek () - 7) * 86400), "thursday -> previous thursday");

      Datetime r9 ("friday");
      if (now.dayOfWeek () >= 5)
        t.ok (r9.sameDay (now + (5 - now.dayOfWeek ()) * 86400), "friday -> previous friday");
      else
        t.ok (r9.sameDay (now + (5 - now.dayOfWeek () - 7) * 86400), "friday -> previous friday");

      Datetime r10 ("saturday");
      if (now.dayOfWeek () >= 6)
        t.ok (r10.sameDay (now + (6 - now.dayOfWeek ()) * 86400), "saturday -> previous saturday");
      else
        t.ok (r10.sameDay (now + (6 - now.dayOfWeek () - 7) * 86400), "saturday -> previous saturday");
    }

    Datetime r11 ("eow");
    t.ok (r11 < now + (8 * 86400), "eow < 7 days away");

    Datetime r12 ("eow");
    t.ok (r12 > now - (8 * 86400), "eow < 7 days in the past");

    Datetime r16 ("sonw");
    t.ok (r16 < now + (8 * 86400), "sonw < 7 days away");

    Datetime r23 ("sow");
    t.ok (r23 > now - (8 * 86400), "sow < 7 days in the past");

    Datetime r17 ("sonm");
    t.notok (r17.sameMonth (now), "sonm not in same month as now");

    Datetime r18 ("som");
    t.ok (r18.sameMonth (now), "som in same month as now");
    t.ok (r18.sameQuarter (now), "som in same quarter as now");

    Datetime r19 ("sony");
    t.notok (r19.sameYear (now), "sony not in same year as now");

    Datetime r19a ("soy");
    t.ok (r19a.sameYear (now), "soy in same year as now");
    t.ok (r19a < now,          "soy < now");

    Datetime r19b ("eoy");
    t.ok (r19b > now,          "eoy > now");

    Datetime r19c ("soq");
    t.ok (r19c.sameYear (now), "soq in same year as now");
    t.ok (r19c < now,          "soq < now");

    Datetime r19d ("eoq");
    t.ok (r19d > now,          "eoq > now");

    {
      Datetime::timeRelative = true;

      Datetime first ("1st");
      std::cout << "actual (first - relative:true): " << first.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 1)
        t.notok (first.sameMonth (now), "1st not in same month as now");
      else
        t.ok (first.sameMonth (now), "1st in same month as now");
      t.is (first.day(), 1, "1st day is 1");

      Datetime second ("2nd");
      std::cout << "actual (second - relative:true): " << second.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 2)
        t.notok (second.sameMonth (now), "2nd not in same month as now");
      else
        t.ok (second.sameMonth (now), "2nd in same month as now");
      t.is (second.day(), 2, "2nd day is 2");

      Datetime third ("3rd");
      std::cout << "actual (third - relative:true): " << third.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 3)
        t.notok (third.sameMonth (now), "3rd not in same month as now");
      else
        t.ok (third.sameMonth (now), "3rd in same month as now");
      t.is (third.day(), 3, "3rd day is 3");

      Datetime fourth ("4th");
      std::cout << "actual (fourth - relative:true): " << fourth.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 4)
        t.notok (fourth.sameMonth (now), "4th not in same month as now");
      else
        t.ok (fourth.sameMonth (now), "4th not in same month as now");
      t.is (fourth.day(), 4, "4th day is 4");
    }

    {
      Datetime::timeRelative = false;

      Datetime first ("1st");
      std::cout << "actual (first - relative:false): " << first.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 1)
        t.ok (first.month() == now.month(), "1st in same month as now");
      else
        t.ok ((first.month() - now.month() - 12) % 12 == -1, "1st in previous month");
      t.is (first.day (), 1, "1st day is 1");

      Datetime second ("2nd");
      std::cout << "actual (second - relative:false): " << second.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 2)
        t.ok (second.month() == now.month(), "2nd in same month as now");
      else
        t.ok ((second.month() - now.month() - 12) % 12 == -1, "2nd in previous month");
      t.is (second.day(), 2, "2nd day is 2");

      Datetime third ("3rd");
      std::cout << "actual (third - relative:false): " << third.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 3)
        t.ok (third.month() == now.month(), "3rd in same month as now");
      else
        t.ok ((third.month() - now.month() - 12) % 12 == -1, "3rd in previous month");
      t.is (third.day(), 3, "3rd day is 3");

      Datetime fourth ("4th");
      std::cout << "actual (fourth - relative:false): " << fourth.toISO () << " now: " << now.toISO () << std::endl;
      if (now.day () >= 4)
        t.ok (fourth.month() == now.month(), "4th in same month as now");
      else
        t.ok ((fourth.month() - now.month() - 12) % 12 == -1, "4th in previous month");
      t.is (fourth.day(), 4, "4th day is 4");
    }

    Datetime later ("later");
    t.is (later.month (),  12, "later -> m = 12");
    t.is (later.day (),    30, "later -> d = 30");
    t.is (later.year (), 9999, "later -> y = 9999");

    // Quarters
    Datetime soq ("soq");
    Datetime eoq ("eoq");
    t.is (soq.day (),  1,      "soq day is the first day of a quarter");
    t.is (eoq.day () / 10 , 3, "eoq day is the last day of a quarter");
    t.is (soq.month () % 3, 1, "soq month is 1, 4, 7 or 10");
    t.is (eoq.month () % 3, 0, "eoq month is 3, 6, 9 or 12");

    // Note: these fail during the night of daylight savings end.
    t.ok (soq.sameYear (now) ||
          (now.month () >= 10 &&
           soq.year () == now.year () + 1), "soq is in same year as now");

    // Datetime::sameHour
    Datetime r20 ("6/7/2010 01:00:00", "m/d/Y H:N:S");
    Datetime r21 ("6/7/2010 01:59:59", "m/d/Y H:N:S");
    t.ok (r20.sameHour (r21), "two dates within the same hour");

    Datetime r22 ("6/7/2010 00:59:59", "m/d/Y H:N:S");
    t.notok (r20.sameHour (r22), "two dates not within the same hour");

    // Datetime::operator-
    Datetime r25 (1234567890);
    t.is ((r25 - 1).toEpoch (), (time_t) 1234567889, "1234567890 - 1 = 1234567889");

    // Datetime::operator--
    Datetime r26 (2010, 11, 7, 23, 59, 59);
    r26--;
    t.is (r26.toString ("YMDHNS"), "20101106235959", "decrement across fall DST boundary");

    Datetime r27 (2010, 3, 14, 23, 59, 59);
    r27--;
    t.is (r27.toString ("YMDHNS"), "20100313235959", "decrement across spring DST boundary");

    // Datetime::operator++
    Datetime r28 (2010, 11, 6, 23, 59, 59);
    r28++;
    t.is (r28.toString ("YMDHNS"), "20101107235959", "increment across fall DST boundary");

    Datetime r29 (2010, 3, 13, 23, 59, 59);
    r29++;
    t.is (r29.toString ("YMDHNS"), "20100314235959", "increment across spring DST boundary");

    // int Datetime::length (const std::string&);
    t.is (Datetime::length ("m"), 2,  "length 'm' --> 2");
    t.is (Datetime::length ("M"), 2,  "length 'M' --> 2");
    t.is (Datetime::length ("d"), 2,  "length 'd' --> 2");
    t.is (Datetime::length ("D"), 2,  "length 'D' --> 2");
    t.is (Datetime::length ("y"), 2,  "length 'y' --> 2");
    t.is (Datetime::length ("Y"), 4,  "length 'Y' --> 4");
    t.is (Datetime::length ("a"), 3,  "length 'a' --> 3");
    t.is (Datetime::length ("A"), 10, "length 'A' --> 10");
    t.is (Datetime::length ("b"), 3,  "length 'b' --> 3");
    t.is (Datetime::length ("B"), 10, "length 'B' --> 10");
    t.is (Datetime::length ("v"), 2,  "length 'v' --> 2");
    t.is (Datetime::length ("V"), 2,  "length 'V' --> 2");
    t.is (Datetime::length ("h"), 2,  "length 'h' --> 2");
    t.is (Datetime::length ("H"), 2,  "length 'H' --> 2");
    t.is (Datetime::length ("n"), 2,  "length 'n' --> 2");
    t.is (Datetime::length ("N"), 2,  "length 'N' --> 2");
    t.is (Datetime::length ("s"), 2,  "length 's' --> 2");
    t.is (Datetime::length ("S"), 2,  "length 'S' --> 2");
    t.is (Datetime::length ("j"), 3,  "length 'j' --> 3");
    t.is (Datetime::length ("J"), 3,  "length 'J' --> 3");
    t.is (Datetime::length (" "), 1,  "length ' ' --> 1");

    // Depletion requirement.
    Datetime r30 ("Mon Jun 30 2014", "a b D Y");
    t.is (r30.toString ("YMDHNS"), "20140630000000", "Depletion required on complex format with spaces");

    Datetime r31 ("Mon Jun 30 2014 xxx", "a b D Y");
    t.is (r31.toString ("YMDHNS"), "20140630000000", "Depletion not required on complex format with spaces");

    // Test all format options.
    Datetime r32 ("2015-10-28T12:55:00");
    t.is (r32.toString ("Y"),      "2015", "2015-10-28T12:55:01 -> Y ->      2015");
    t.is (r32.toString ("y"),        "15", "2015-10-28T12:55:01 -> y ->        15");
    t.is (r32.toString ("M"),        "10", "2015-10-28T12:55:01 -> M ->        10");
    t.is (r32.toString ("m"),        "10", "2015-10-28T12:55:01 -> m ->        10");
    t.is (r32.toString ("D"),        "28", "2015-10-28T12:55:01 -> D ->        28");
    t.is (r32.toString ("d"),        "28", "2015-10-28T12:55:01 -> d ->        28");
    t.is (r32.toString ("H"),        "12", "2015-10-28T12:55:01 -> H ->        12");
    t.is (r32.toString ("h"),        "12", "2015-10-28T12:55:01 -> h ->        12");
    t.is (r32.toString ("N"),        "55", "2015-10-28T12:55:01 -> N ->        55");
    t.is (r32.toString ("n"),        "55", "2015-10-28T12:55:01 -> n ->        55");
    t.is (r32.toString ("S"),        "00", "2015-10-28T12:55:01 -> S ->        01");
    t.is (r32.toString ("s"),         "0", "2015-10-28T12:55:01 -> s ->         1");
    t.is (r32.toString ("A"), "Wednesday", "2015-10-28T12:55:01 -> A -> Wednesday");
    t.is (r32.toString ("a"),       "Wed", "2015-10-28T12:55:01 -> a ->       Wed");
    t.is (r32.toString ("B"),   "October", "2015-10-28T12:55:01 -> B ->   October");
    t.is (r32.toString ("b"),       "Oct", "2015-10-28T12:55:01 -> b ->       Oct");
    t.is (r32.toString ("V"),        "44", "2015-10-28T12:55:01 -> V ->        44");
    t.is (r32.toString ("v"),        "44", "2015-10-28T12:55:01 -> v ->        44");
    t.is (r32.toString ("J"),       "301", "2015-10-28T12:55:01 -> J ->       301");
    t.is (r32.toString ("j"),       "301", "2015-10-28T12:55:01 -> j ->       301");
    t.is (r32.toString ("w"),         "3", "2015-10-28T12:55:01 -> w ->         3");

    // Test all parse options.
    Datetime r33 ("2015 10 28 19 28 01", "Y M D H N S");
    t.is(r33.year (),   2015,         "Y works");
    t.is(r33.month (),    10,         "M works");
    t.is(r33.day (),      28,         "D works");
    t.is(r33.hour (),     19,         "H works");
    t.is(r33.minute (),   28,         "N works");
    t.is(r33.second (),    1,         "S works");

    Datetime r34 ("15 5 4 3 2 1", "y m d h n s");
    t.is(r34.year (),   2015,         "y works");
    t.is(r34.month (),     5,         "m works");
    t.is(r34.day (),       4,         "d works");
    t.is(r34.hour (),      3,         "h works");
    t.is(r34.minute (),    2,         "n works");
    t.is(r34.second (),    1,         "s works");

    Datetime r35 ("Wednesday October 28 2015", "A B D Y");
    t.is(r35.year (),    2015,        "Y works");
    t.is(r35.month (),     10,        "B works");
    t.is(r35.day (),       28,        "D works");
    t.is(r35.dayOfWeek (),  3,        "A works");

    Datetime r36 ("Wed Oct 28 15", "a b d y");
    t.is(r36.year (),    2015,        "y works");
    t.is(r36.month (),     10,        "b works");
    t.is(r36.day (),       28,        "d works");
    t.is(r36.dayOfWeek (),  3,        "a works");

    Datetime r37 ("19th");
    t.is (r37.day (),      19,        "'19th' --> 19");

    // Test all format options for a future date
    Datetime r38 ("9015-10-28T12:55:00");
    t.is (r38.toString ("Y"),      "9015", "9015-10-28T12:55:01 -> Y ->      9015");
    t.is (r38.toString ("y"),        "15", "9015-10-28T12:55:01 -> y ->        15");
    t.is (r38.toString ("M"),        "10", "9015-10-28T12:55:01 -> M ->        10");
    t.is (r38.toString ("m"),        "10", "9015-10-28T12:55:01 -> m ->        10");
    t.is (r38.toString ("D"),        "28", "9015-10-28T12:55:01 -> D ->        28");
    t.is (r38.toString ("d"),        "28", "9015-10-28T12:55:01 -> d ->        28");
    t.is (r38.toString ("H"),        "12", "9015-10-28T12:55:01 -> H ->        12");
    t.is (r38.toString ("h"),        "12", "9015-10-28T12:55:01 -> h ->        12");
    t.is (r38.toString ("N"),        "55", "9015-10-28T12:55:01 -> N ->        55");
    t.is (r38.toString ("n"),        "55", "9015-10-28T12:55:01 -> n ->        55");
    t.is (r38.toString ("S"),        "00", "9015-10-28T12:55:01 -> S ->        01");
    t.is (r38.toString ("s"),         "0", "9015-10-28T12:55:01 -> s ->         1");
    t.is (r38.toString ("A"),  "Saturday", "9015-10-28T12:55:01 -> A ->  Saturday");
    t.is (r38.toString ("a"),       "Sat", "9015-10-28T12:55:01 -> a ->       Sat");
    t.is (r38.toString ("B"),   "October", "9015-10-28T12:55:01 -> B ->   October");
    t.is (r38.toString ("b"),       "Oct", "9015-10-28T12:55:01 -> b ->       Oct");
    t.is (r38.toString ("V"),        "43", "9015-10-28T12:55:01 -> V ->        43");
    t.is (r38.toString ("v"),        "43", "9015-10-28T12:55:01 -> v ->        43");
    t.is (r38.toString ("J"),       "301", "9015-10-28T12:55:01 -> J ->       301");
    t.is (r38.toString ("j"),       "301", "9015-10-28T12:55:01 -> j ->       301");
    t.is (r38.toString ("w"),         "6", "9015-10-28T12:55:01 -> w ->         6");

/*
    // Phrases.
    Datetime r42 ("4th thursday in november");
*/

    // Embedded parsing.
    testParseError (t, "nowadays");
    testParse      (t, "now+1d");
    testParse      (t, "now-1d");
    testParse      (t, "now)");
    testParseError (t, "now7");
    testParseError (t, "tomorrov");

    testParseError (t, "yesteryear");
    testParse      (t, "yest+1d");
    testParse      (t, "yest-1d");
    testParse      (t, "yest)");
    testParseError (t, "yest7");
    testParse      (t, "yesterday");

    testParse      (t, "1234567890+0");
    testParse      (t, "1234567890-0");
    testParse      (t, "1234567890)");

    // Negative tests, all expected to fail.
    testParseError (t, "");
    testParseError (t, "foo");
    testParseError (t, "-2014-07-07");
    testParseError (t, "2014-07-");
    testParseError (t, "2014-0-12");
    testParseError (t, "abcd-ab-ab");
    testParseError (t, "2014-000");
    testParse      (t, "2014-001");
    testParse      (t, "2014-365");
    testParseError (t, "2014-366");
    testParseError (t, "2014-367");
    testParseError (t, "2014-999");
    testParseError (t, "2014-999999999");
    testParseError (t, "2014-W00");
    testParseError (t, "2014-W54");
    testParseError (t, "2014-W240");
    testParseError (t, "2014-W248");
    testParseError (t, "2014-W24200");
    //testParseError (t, "2014-00");        // Looks like Datetime::parse_time_off 'hhmm-hh'
    testParseError (t, "2014-13");
    testParseError (t, "2014-99");
    testParseError (t, "25:00");
    testParseError (t, "99:00");
    testParseError (t, "12:60");
    testParseError (t, "12:99");
    testParseError (t, "12:ab");
    testParseError (t, "ab:12");
    testParseError (t, "ab:cd");
    testParseError (t, "-12:12");
    testParseError (t, "12:-12");
    testParseError (t, "25:00Z");
    testParseError (t, "99:00Z");
    testParseError (t, "12:60Z");
    testParseError (t, "12:99Z");
    testParseError (t, "12:abZ");
    testParseError (t, "ab:12Z");
    testParseError (t, "ab:cdZ");
    testParseError (t, "-12:12Z");
    testParseError (t, "12:-12Z");
    testParseError (t, "25:00+01:00");
    testParseError (t, "99:00+01:00");
    testParseError (t, "12:60+01:00");
    testParseError (t, "12:99+01:00");
    testParseError (t, "12:ab+01:00");
    testParseError (t, "ab:12+01:00");
    testParseError (t, "ab:cd+01:00");
    testParseError (t, "-12:12+01:00");
    testParseError (t, "12:-12+01:00");
    testParseError (t, "25:00-01:00");
    testParseError (t, "99:00-01:00");
    testParseError (t, "12:60-01:00");
    testParseError (t, "12:99-01:00");
    testParseError (t, "12:ab-01:00");
    testParseError (t, "ab:12-01:00");
    testParseError (t, "ab:cd-01:00");
    testParseError (t, "-12:12-01:00");
    testParseError (t, "12:-12-01:00");
    testParseError (t, "25:00:00");
    testParseError (t, "99:00:00");
    testParseError (t, "12:60:00");
    testParseError (t, "12:99:00");
    testParseError (t, "12:12:60");
    testParseError (t, "12:12:99");
    testParseError (t, "12:ab:00");
    testParseError (t, "ab:12:00");
    testParseError (t, "12:12:ab");
    testParseError (t, "ab:cd:ef");
    testParseError (t, "-12:12:12");
    testParseError (t, "12:-12:12");
    testParseError (t, "12:12:-12");
    testParseError (t, "25:00:00Z");
    testParseError (t, "99:00:00Z");
    testParseError (t, "12:60:00Z");
    testParseError (t, "12:99:00Z");
    testParseError (t, "12:12:60Z");
    testParseError (t, "12:12:99Z");
    testParseError (t, "12:ab:00Z");
    testParseError (t, "ab:12:00Z");
    testParseError (t, "12:12:abZ");
    testParseError (t, "ab:cd:efZ");
    testParseError (t, "-12:12:12Z");
    testParseError (t, "12:-12:12Z");
    testParseError (t, "12:12:-12Z");
    testParseError (t, "25:00:00+01:00");
    testParseError (t, "95:00:00+01:00");
    testParseError (t, "12:60:00+01:00");
    testParseError (t, "12:99:00+01:00");
    testParseError (t, "12:12:60+01:00");
    testParseError (t, "12:12:99+01:00");
    testParseError (t, "12:ab:00+01:00");
    testParseError (t, "ab:12:00+01:00");
    testParseError (t, "12:12:ab+01:00");
    testParseError (t, "ab:cd:ef+01:00");
    testParseError (t, "-12:12:12+01:00");
    testParseError (t, "12:-12:12+01:00");
    testParseError (t, "12:12:-12+01:00");
    testParseError (t, "25:00:00-01:00");
    testParseError (t, "95:00:00-01:00");
    testParseError (t, "12:60:00-01:00");
    testParseError (t, "12:99:00-01:00");
    testParseError (t, "12:12:60-01:00");
    testParseError (t, "12:12:99-01:00");
    testParseError (t, "12:ab:00-01:00");
    testParseError (t, "ab:12:00-01:00");
    testParseError (t, "12:12:ab-01:00");
    testParseError (t, "ab:cd:ef-01:00");
    testParseError (t, "-12:12:12-01:00");
    testParseError (t, "12:-12:12-01:00");
    testParseError (t, "12:12:-12-01:00");
    testParseError (t, "12:12:12-13:00");
    testParseError (t, "12:12:12-24:00");
    testParseError (t, "12:12:12-99:00");
    testParseError (t, "12:12:12-03:60");
    testParseError (t, "12:12:12-03:99");
    testParseError (t, "12:12:12-3:20");
    testParseError (t, "12:12:12-03:2");
    testParseError (t, "12:12:12-3:2");
    testParseError (t, "12:12:12+13:00");
    testParseError (t, "12:12:12+24:00");
    testParseError (t, "12:12:12+99:00");
    testParseError (t, "12:12:12+03:60");
    testParseError (t, "12:12:12+03:99");
    testParseError (t, "12:12:12+3:20");
    testParseError (t, "12:12:12+03:2");
    testParseError (t, "12:12:12+3:2");
    testParseError (t, "12:12-13:00");
    testParseError (t, "12:12-24:00");
    testParseError (t, "12:12-99:00");
    testParseError (t, "12:12-03:60");
    testParseError (t, "12:12-03:99");
    testParseError (t, "12:12-3:20");
    testParseError (t, "12:12-03:2");
    testParseError (t, "12:12-3:2");
    testParseError (t, "12:12+13:00");
    testParseError (t, "12:12+24:00");
    testParseError (t, "12:12+99:00");
    testParseError (t, "12:12+03:60");
    testParseError (t, "12:12+03:99");
    testParseError (t, "12:12+3:20");
    testParseError (t, "12:12+03:2");
    testParseError (t, "12:12+3:2");

    // Test with standlalone date enable/disabled.
    Datetime::standaloneDateEnabled = true;
    testParse      (t, "20170319");
    Datetime::standaloneDateEnabled = false;
    testParseError (t, "20170319");
    Datetime::standaloneDateEnabled = true;

    Datetime::standaloneTimeEnabled = true;
    testParse      (t, "235959");
    Datetime::standaloneTimeEnabled = false;
    testParseError (t, "235959");
    Datetime::standaloneTimeEnabled = true;

    // Weekdays and month names can no longer be followed by ':' or '='.
    testParse      (t, "jan");
    testParseError (t, "jan:");
    testParse      (t, "mon");
    testParseError (t, "mon:");

    {
      // Verify Datetime::timeRelative is working as expected.
      Datetime::timeRelative = true;
      Datetime today;
      Datetime r38("0:00:01");
      t.notok(today.sameDay(r38), "Datetime::timeRelative=true 0:00:01 --> tomorrow");
      Datetime::timeRelative = false;
      Datetime r39("0:00:01");
      t.ok(today.sameDay(r39), "Datetime::timeRelative=false 0:00:01 --> today");
      Datetime::timeRelative = true;
    }

    {
      // Verify Datetime::timeRelative=true puts months before and including current month into next year
      Datetime::timeRelative = true;

      Datetime today;

      for (auto& month: {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"})
      {
        auto test = Datetime(month);
        bool is_after = (test.month() > today.month());
        std::string message = "Datetime::timeRelative=true --> " + std::string(month) + " in " + (is_after ? "same" : "next") + " year";
        t.ok(test.year() == today.year() + (is_after ? 0 : 1), message);
      }
    }

    {
      // Verify Datetime::timeRelative=false puts months into current year
      Datetime::timeRelative = false;

      Datetime today;

      for (auto& month: {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"})
      {
        auto test = Datetime(month);
        std::string message = "Datetime::timeRelative=false --> " + std::string(month) + " in same year";
        t.ok(test.year() == today.year(), message);
      }
    }

    // This is just a diagnostic dump of all named dates, and is used to verify
    // correctness manually.
    t.diag ("--------------------------------------------");
    t.diag ("  now            " + Datetime ("now").toISOLocalExtended ());
    t.diag ("  yesterday      " + Datetime ("yesterday").toISOLocalExtended ());
    t.diag ("  today          " + Datetime ("today").toISOLocalExtended ());
    t.diag ("  tomorrow       " + Datetime ("tomorrow").toISOLocalExtended ());
    t.diag ("  1st            " + Datetime ("1st").toISOLocalExtended ());
    t.diag ("  monday         " + Datetime ("monday").toISOLocalExtended ());
    t.diag ("  january        " + Datetime ("january").toISOLocalExtended ());
    t.diag ("  later          " + Datetime ("later").toISOLocalExtended ());
    t.diag ("  someday        " + Datetime ("someday").toISOLocalExtended ());
    t.diag ("  sopd           " + Datetime ("sopd").toISOLocalExtended ());
    t.diag ("  sod            " + Datetime ("sod").toISOLocalExtended ());
    t.diag ("  sond           " + Datetime ("sond").toISOLocalExtended ());
    t.diag ("  eopd           " + Datetime ("eopd").toISOLocalExtended ());
    t.diag ("  eod            " + Datetime ("eod").toISOLocalExtended ());
    t.diag ("  eond           " + Datetime ("eond").toISOLocalExtended ());
    t.diag ("  sopw           " + Datetime ("sopw").toISOLocalExtended ());
    t.diag ("  sow            " + Datetime ("sow").toISOLocalExtended ());
    t.diag ("  sonw           " + Datetime ("sonw").toISOLocalExtended ());
    t.diag ("  eopw           " + Datetime ("eopw").toISOLocalExtended ());
    t.diag ("  eow            " + Datetime ("eow").toISOLocalExtended ());
    t.diag ("  eonw           " + Datetime ("eonw").toISOLocalExtended ());
    t.diag ("  sopww          " + Datetime ("sopww").toISOLocalExtended ());
    t.diag ("  sonww          " + Datetime ("sonww").toISOLocalExtended ());
    t.diag ("  soww           " + Datetime ("soww").toISOLocalExtended ());
    t.diag ("  eopww          " + Datetime ("eopww").toISOLocalExtended ());
    t.diag ("  eonww          " + Datetime ("eonww").toISOLocalExtended ());
    t.diag ("  eoww           " + Datetime ("eoww").toISOLocalExtended ());
    t.diag ("  sopm           " + Datetime ("sopm").toISOLocalExtended ());
    t.diag ("  som            " + Datetime ("som").toISOLocalExtended ());
    t.diag ("  sonm           " + Datetime ("sonm").toISOLocalExtended ());
    t.diag ("  eopm           " + Datetime ("eopm").toISOLocalExtended ());
    t.diag ("  eom            " + Datetime ("eom").toISOLocalExtended ());
    t.diag ("  eonm           " + Datetime ("eonm").toISOLocalExtended ());
    t.diag ("  sopq           " + Datetime ("sopq").toISOLocalExtended ());
    t.diag ("  soq            " + Datetime ("soq").toISOLocalExtended ());
    t.diag ("  sonq           " + Datetime ("sonq").toISOLocalExtended ());
    t.diag ("  eopq           " + Datetime ("eopq").toISOLocalExtended ());
    t.diag ("  eoq            " + Datetime ("eoq").toISOLocalExtended ());
    t.diag ("  eonq           " + Datetime ("eonq").toISOLocalExtended ());
    t.diag ("  sopy           " + Datetime ("sopy").toISOLocalExtended ());
    t.diag ("  soy            " + Datetime ("soy").toISOLocalExtended ());
    t.diag ("  sony           " + Datetime ("sony").toISOLocalExtended ());
    t.diag ("  eopy           " + Datetime ("eopy").toISOLocalExtended ());
    t.diag ("  eoy            " + Datetime ("eoy").toISOLocalExtended ());
    t.diag ("  eony           " + Datetime ("eony").toISOLocalExtended ());
    t.diag ("  easter         " + Datetime ("easter").toISOLocalExtended ());
    t.diag ("  eastermonday   " + Datetime ("eastermonday").toISOLocalExtended ());
    t.diag ("  ascension      " + Datetime ("ascension").toISOLocalExtended ());
    t.diag ("  pentecost      " + Datetime ("pentecost").toISOLocalExtended ());
    t.diag ("  goodfriday     " + Datetime ("goodfriday").toISOLocalExtended ());
    t.diag ("  midsommar      " + Datetime ("midsommar").toISOLocalExtended ());
    t.diag ("  midsommarafton " + Datetime ("midsommarafton").toISOLocalExtended ());
    t.diag ("  juhannus       " + Datetime ("juhannus").toISOLocalExtended ());
    t.diag ("  3am            " + Datetime ("3am").toISOLocalExtended ());
    t.diag ("  12am           " + Datetime ("12am").toISOLocalExtended ());
    t.diag ("  12pm           " + Datetime ("12pm").toISOLocalExtended ());
    t.diag ("  1234567890     " + Datetime ("1234567890").toISOLocalExtended ());
    t.diag ("--------------------------------------------");
  }

  catch (const std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
