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
  std::string label = std::string ("parse (\"") + input + "\") --> ";

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
int main (int, char**)
{
  UnitTest t (1890);

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
  std::cout << "# local midnight today " << local << "\n";

  int year = 2013;
  int mo = 12;

  local_now->tm_year  = year - 1900;
  local_now->tm_mon   = mo - 1;
  local_now->tm_mday  = 6;
  local_now->tm_isdst = 0;
  time_t local6 = mktime (local_now);
  std::cout << "# local midnight 2013-12-06 " << local6 << "\n";

  local_now->tm_year  = year - 1900;
  local_now->tm_mon   = mo - 1;
  local_now->tm_mday  = 1;
  local_now->tm_isdst = 0;
  time_t local1 = mktime (local_now);
  std::cout << "# local midnight 2013-12-01 " << local1 << "\n";

  struct tm* utc_now = gmtime (&now);
  int utc_s = (utc_now->tm_hour * 3600) +
              (utc_now->tm_min  * 60)   +
              utc_now->tm_sec;
  utc_now->tm_hour  = 0;
  utc_now->tm_min   = 0;
  utc_now->tm_sec   = 0;
  utc_now->tm_isdst = -1;
  time_t utc = timegm (utc_now);
  std::cout << "# utc midnight today " << utc << "\n";

  utc_now->tm_year  = year - 1900;
  utc_now->tm_mon   = mo - 1;
  utc_now->tm_mday  = 6;
  utc_now->tm_isdst = 0;
  time_t utc6 = timegm (utc_now);
  std::cout << "# utc midnight 2013-12-06 " << utc6 << "\n";

  utc_now->tm_year  = year - 1900;
  utc_now->tm_mon   = mo - 1;
  utc_now->tm_mday  = 1;
  utc_now->tm_isdst = 0;
  time_t utc1 = timegm (utc_now);
  std::cout << "# utc midnight 2013-12-01 " << utc1 << "\n";

  int hms = (12 * 3600) + (34 * 60) + 56; // The time 12:34:56 in seconds.
  int hm  = (12 * 3600) + (34 * 60);      // The time 12:34:00 in seconds.
  int z   = 3600;                         // TZ offset.

  int ld = local_s > hms ? 86400 : 0;     // Local extra day if now > hms.
  int ud = utc_s   > hms ? 86400 : 0;     // UTC extra day if now > hms.
  std::cout << "# ld " << ld << "\n";
  std::cout << "# ud " << ud << "\n";

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

  // The only non-extended forms.
  testParse (t, "20131206T123456Z",          16, year, mo,  0, 0,   0,  6,   hms,     0,  true, utc6+hms  );
  testParse (t, "20131206T123456",           15, year, mo,  0, 0,   0,  6,   hms,     0, false, local6+hms);

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

  // Informal time.
  int t8a   = (8 * 3600);
  int t830a = (8 * 3600) + (30 * 60);
  int t8p   = (20 * 3600);
  int t830p = (20 * 3600) + (30 * 60);
  testParse (t, "8:30am",                     6,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830a);
  testParse (t, "8:30a",                      5,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830a);
  testParse (t, "8:30",                       4,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830a);
  testParse (t, "8am",                        3,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8a  );
  testParse (t, "8a",                         2,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8a  );
  testParse (t, "8:30pm",                     6,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830p);
  testParse (t, "8:30p",                      5,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t830p);
  testParse (t, "8pm",                        3,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8p  );
  testParse (t, "8p",                         2,    0,  0,  0, 0,   0,  0,     0,     0, false, local+t8p  );

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
    t.ok (left.sameDay   (comp1), "7/4/2008 is on the same day as 7/4/2008");
    t.ok (left.sameWeek  (comp1), "7/4/2008 is on the same week as 7/4/2008");
    t.ok (left.sameMonth (comp1), "7/4/2008 is in the same month as 7/4/2008");
    t.ok (left.sameYear  (comp1), "7/4/2008 is in the same year as 7/4/2008");

    Datetime comp2 ("7/5/2008", "m/d/Y");
    t.notok (left.sameDay   (comp2), "7/4/2008 is not on the same day as 7/5/2008");
    t.ok    (left.sameMonth (comp2), "7/4/2008 is in the same month as 7/5/2008");
    t.ok    (left.sameYear  (comp2), "7/4/2008 is in the same year as 7/5/2008");

    Datetime comp3 ("8/4/2008", "m/d/Y");
    t.notok (left.sameDay   (comp3), "7/4/2008 is not on the same day as 8/4/2008");
    t.notok (left.sameWeek  (comp3), "7/4/2008 is not on the same week as 8/4/2008");
    t.notok (left.sameMonth (comp3), "7/4/2008 is not in the same month as 8/4/2008");
    t.ok    (left.sameYear  (comp3), "7/4/2008 is in the same year as 8/4/2008");

    Datetime comp4 ("7/4/2009", "m/d/Y");
    t.notok (left.sameDay   (comp4), "7/4/2008 is not on the same day as 7/4/2009");
    t.notok (left.sameWeek  (comp3), "7/4/2008 is not on the same week as 7/4/2009");
    t.notok (left.sameMonth (comp4), "7/4/2008 is not in the same month as 7/4/2009");
    t.notok (left.sameYear  (comp4), "7/4/2008 is not in the same year as 7/4/2009");

    // Validity.
    t.ok    (Datetime::valid (2, 29, 2008), "valid: 2/29/2008");
    t.notok (Datetime::valid (2, 29, 2007), "invalid: 2/29/2007");

    t.ok    (Datetime::valid ("2/29/2008", "m/d/Y"), "valid: 2/29/2008");
    t.notok (Datetime::valid ("2/29/2007", "m/d/Y"), "invalid: 2/29/2007");

    t.ok    (Datetime::valid (366, 2008), "valid: 366 days in 2008");
    t.notok (Datetime::valid (366, 2007), "invalid: 366 days in 2007");

    // Time validity.
    t.ok    (Datetime::valid (2, 28, 2010,  0,  0,  0), "valid 2/28/2010 0:00:00");
    t.ok    (Datetime::valid (2, 28, 2010, 23, 59, 59), "valid 2/28/2010 23:59:59");
    t.notok (Datetime::valid (2, 28, 2010, 24, 59, 59), "valid 2/28/2010 24:59:59");
    t.notok (Datetime::valid (2, 28, 2010, -1,  0,  0), "valid 2/28/2010 -1:00:00");

    // Leap year.
    t.ok    (Datetime::leapYear (2008), "2008 is a leap year");
    t.notok (Datetime::leapYear (2007), "2007 is not a leap year");
    t.ok    (Datetime::leapYear (2000), "2000 is a leap year");
    t.notok (Datetime::leapYear (1900), "1900 is not a leap year");

    // Days in year.
    t.is (Datetime::daysInYear (2016), 366, "366 days in 2016");
    t.is (Datetime::daysInYear (2015), 365, "365 days in 2015");

    // Days in month.
    t.is (Datetime::daysInMonth (2, 2008), 29, "29 days in February 2008");
    t.is (Datetime::daysInMonth (2, 2007), 28, "28 days in February 2007");

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

    Datetime happyNewYear (1, 1, 2008);
    t.is (happyNewYear.dayOfWeek (),         2, "1/1/2008 == Tuesday");
    t.is (happyNewYear.month (),             1, "1/1/2008 == January");
    t.is (happyNewYear.day (),               1, "1/1/2008 == 1");
    t.is (happyNewYear.year (),           2008, "1/1/2008 == 2008");
    t.is (happyNewYear.toString (), "1/1/2008", "toString 1/1/2008");

    int m, d, y;
    happyNewYear.toMDY (m, d, y);
    t.is (m, 1, "1/1/2008 == January");
    t.is (d, 1, "1/1/2008 == 1");
    t.is (y, 2008, "1/1/2008 == 2008");

    Datetime epoch (9, 8, 2001);
    t.ok ((int)epoch.toEpoch () < 1000000000, "9/8/2001 < 1,000,000,000");
    epoch += 172800;
    t.ok ((int)epoch.toEpoch () > 1000000000, "9/10/2001 > 1,000,000,000");

    Datetime fromEpoch (epoch.toEpoch ());
    t.is (fromEpoch.toString (), epoch.toString (), "ctor (time_t)");

    Datetime iso (1000000000);
    t.is (iso.toISO (), "20010909T014640Z", "1,000,000,000 -> 20010909T014640Z");

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

    // Day of year
    t.is (Datetime ("1/1/2011",   "m/d/Y").dayOfYear (),   1, "dayOfYear (1/1/2011)   ->   1");
    t.is (Datetime ("5/1/2011",   "m/d/Y").dayOfYear (), 121, "dayOfYear (5/1/2011)   -> 121");
    t.is (Datetime ("12/31/2011", "m/d/Y").dayOfYear (), 365, "dayOfYear (12/31/2011) -> 365");

    // Relative dates.
    Datetime r1 ("today");
    t.ok (r1.sameDay (now), "today = now");

    Datetime r4 ("sunday");
    if (now.dayOfWeek () >= 0)
      t.ok (r4.sameDay (now + (0 - now.dayOfWeek () + 7) * 86400), "next sunday");
    else
      t.ok (r4.sameDay (now + (0 - now.dayOfWeek ()) * 86400), "next sunday");;

    Datetime r5 ("monday");
    if (now.dayOfWeek () >= 1)
      t.ok (r5.sameDay (now + (1 - now.dayOfWeek () + 7) * 86400), "next monday");
    else
      t.ok (r5.sameDay (now + (1 - now.dayOfWeek ()) * 86400), "next monday");;

    Datetime r6 ("tuesday");
    if (now.dayOfWeek () >= 2)
      t.ok (r6.sameDay (now + (2 - now.dayOfWeek () + 7) * 86400), "next tuesday");
    else
      t.ok (r6.sameDay (now + (2 - now.dayOfWeek ()) * 86400), "next tuesday");;

    Datetime r7 ("wednesday");
    if (now.dayOfWeek () >= 3)
      t.ok (r7.sameDay (now + (3 - now.dayOfWeek () + 7) * 86400), "next wednesday");
    else
      t.ok (r7.sameDay (now + (3 - now.dayOfWeek ()) * 86400), "next wednesday");;

    Datetime r8 ("thursday");
    if (now.dayOfWeek () >= 4)
      t.ok (r8.sameDay (now + (4 - now.dayOfWeek () + 7) * 86400), "next thursday");
    else
      t.ok (r8.sameDay (now + (4 - now.dayOfWeek ()) * 86400), "next thursday");;

    Datetime r9 ("friday");
    if (now.dayOfWeek () >= 5)
      t.ok (r9.sameDay (now + (5 - now.dayOfWeek () + 7) * 86400), "next friday");
    else
      t.ok (r9.sameDay (now + (5 - now.dayOfWeek ()) * 86400), "next friday");;

    Datetime r10 ("saturday");
    if (now.dayOfWeek () >= 6)
      t.ok (r10.sameDay (now + (6 - now.dayOfWeek () + 7) * 86400), "next saturday");
    else
      t.ok (r10.sameDay (now + (6 - now.dayOfWeek ()) * 86400), "next saturday");;

    Datetime r11 ("eow");
    t.ok (r11 < now + (8 * 86400), "eow < 7 days away");

    Datetime r12 ("eocw");
    t.ok (r12 > now - (8 * 86400), "eocw < 7 days in the past");

    Datetime r13 ("eom");
    t.ok (r13.sameMonth (now), "eom in same month as now");

    Datetime r14 ("eocm");
    t.ok (r14.sameMonth (now), "eocm in same month as now");

    Datetime r15 ("eoy");
    t.ok (r15.sameYear (now), "eoy in same year as now");

    Datetime r16 ("sow");
    t.ok (r16 < now + (8 * 86400), "sow < 7 days away");

    Datetime r23 ("socw");
    t.ok (r23 > now - (8 * 86400), "sow < 7 days in the past");

    Datetime r17 ("som");
    t.notok (r17.sameMonth (now), "som not in same month as now");

    Datetime r18 ("socm");
    t.ok (r18.sameMonth (now), "socm in same month as now");

    Datetime r19 ("soy");
    t.notok (r19.sameYear (now), "soy not in same year as now");

    Datetime r19a ("socy");
    t.ok (r19a.sameYear (now), "socy in same year as now");
    t.ok (r19a < now,          "socy < now");

    Datetime r19b ("eocy");
    t.ok (r19b.sameYear (now), "eocy in same year as now");
    t.ok (r19b > now,          "eocy > now");

    Datetime r19c ("socq");
    t.ok (r19c.sameYear (now), "socq in same year as now");
    t.ok (r19c < now,          "socq < now");

    Datetime r19d ("eocq");
    t.ok (r19d.sameYear (now), "eocq in same year as now");
    t.ok (r19d > now,          "eocq > now");

    Datetime first ("1st");
    t.notok (first.sameMonth (now), "1st not in same month as now");
    t.is (first.day (),   1, "1st day is 1");

    Datetime later ("later");
    t.is (later.month (),   1, "later -> m = 1");
    t.is (later.day (),    18, "later -> d = 18");
    t.is (later.year (), 2038, "later -> y = 2038");

    // Quarters
    Datetime soq ("soq");
    Datetime eoq ("eoq");
    t.is (soq.day (),  1,      "soq is the first day of a month");
    t.is (eoq.day () / 10,  3, "eoq is the 30th or 31th of a month");
    t.is (soq.month () % 3, 1, "soq month is 1, 4, 7 or 10");
    t.is (eoq.month () % 3, 0, "eoq month is 3, 6, 9 or 12");

    // Note: these fail during the night of daylight savings end.
    t.ok (soq.sameYear (now) ||
          (now.month () >= 10 &&
           soq.year () == now.year () + 1), "soq is in same year as now");
    t.ok (eoq.sameYear (now),  "eoq is in same year as now");

    // Datetime::sameHour
    Datetime r20 ("6/7/2010 01:00:00", "m/d/Y H:N:S");
    Datetime r21 ("6/7/2010 01:59:59", "m/d/Y H:N:S");
    t.ok (r20.sameHour (r21), "two dates within the same hour");

    Datetime r22 ("6/7/2010 00:59:59", "m/d/Y H:N:S");
    t.notok (r20.sameHour (r22), "two dates not within the same hour");

    // Datetime::operator-
    Datetime r25 (1234567890);
    t.is ((r25 - 1).toEpoch (), 1234567889, "1234567890 - 1 = 1234567889");

    // Datetime::operator--
    Datetime r26 (11, 7, 2010, 23, 59, 59);
    r26--;
    t.is (r26.toString ("YMDHNS"), "20101106235959", "decrement across fall DST boundary");

    Datetime r27 (3, 14, 2010, 23, 59, 59);
    r27--;
    t.is (r27.toString ("YMDHNS"), "20100313235959", "decrement across spring DST boundary");

    // Datetime::operator++
    Datetime r28 (11, 6, 2010, 23, 59, 59);
    r28++;
    t.is (r28.toString ("YMDHNS"), "20101107235959", "increment across fall DST boundary");

    Datetime r29 (3, 13, 2010, 23, 59, 59);
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
    t.ok (r37 >= now,                 "'19th' > 'now'");


    // Test Datetime::lookForwards = false
    Datetime::lookForwards = false;
    Datetime r38 ("1st");
    Datetime r39 ("29th");
    t.ok (r38.sameMonth (r39),        "'1st' and '29th' always in the same month when Datetime::lookForwards = false");

    Datetime::lookForwards = false;
    Datetime r40 ("sow");
    Datetime r41 ("socw");
    t.ok (r40 == r41,                 "'sow' is equal to 'socw' when Datetime::lookForwards = false");

    Datetime::lookForwards = true;
    Datetime r42 ("sow");
    Datetime r43 ("socw");
    t.ok (r42 != r43,                 "'sow' is not equal to 'socw' when Datetime::lookForwards = true");

    Datetime::lookForwards = true;
    t.diag ("Datetime::lookForwards == true");
    t.diag ("  sod            " + Datetime ("sod").toISOLocalExtended ());
    t.diag ("  monday         " + Datetime ("monday").toISOLocalExtended ());
    t.diag ("  january        " + Datetime ("january").toISOLocalExtended ());
    t.diag ("  socy           " + Datetime ("socy").toISOLocalExtended ());
    t.diag ("  soy            " + Datetime ("soy").toISOLocalExtended ());
    t.diag ("  socq           " + Datetime ("socq").toISOLocalExtended ());
    t.diag ("  soq            " + Datetime ("soq").toISOLocalExtended ());
    t.diag ("  socm           " + Datetime ("socm").toISOLocalExtended ());
    t.diag ("  som            " + Datetime ("som").toISOLocalExtended ());
    t.diag ("  socw           " + Datetime ("socw").toISOLocalExtended ());
    t.diag ("  sow            " + Datetime ("sow").toISOLocalExtended ());
    t.diag ("  soww           " + Datetime ("soww").toISOLocalExtended ());
    t.diag ("  1st            " + Datetime ("1st").toISOLocalExtended ());
    t.diag ("  easter         " + Datetime ("easter").toISOLocalExtended ());
    t.diag ("  midsommar      " + Datetime ("midsommar").toISOLocalExtended ());
    t.diag ("  midsommarafton " + Datetime ("midsommarafton").toISOLocalExtended ());
    t.diag ("  3am            " + Datetime ("3am").toISOLocalExtended ());

    Datetime::lookForwards = false;
    t.diag ("Datetime::lookForwards == false");
    t.diag ("  sod            " + Datetime ("sod").toISOLocalExtended ());
    t.diag ("  monday         " + Datetime ("monday").toISOLocalExtended ());
    t.diag ("  january        " + Datetime ("january").toISOLocalExtended ());
    t.diag ("  socy           " + Datetime ("socy").toISOLocalExtended ());
    t.diag ("  soy            " + Datetime ("soy").toISOLocalExtended ());
    t.diag ("  socq           " + Datetime ("socq").toISOLocalExtended ());
    t.diag ("  soq            " + Datetime ("soq").toISOLocalExtended ());
    t.diag ("  socm           " + Datetime ("socm").toISOLocalExtended ());
    t.diag ("  som            " + Datetime ("som").toISOLocalExtended ());
    t.diag ("  socw           " + Datetime ("socw").toISOLocalExtended ());
    t.diag ("  sow            " + Datetime ("sow").toISOLocalExtended ());
    t.diag ("  soww           " + Datetime ("soww").toISOLocalExtended ());
    t.diag ("  1st            " + Datetime ("1st").toISOLocalExtended ());
    t.diag ("  easter         " + Datetime ("easter").toISOLocalExtended ());
    t.diag ("  midsommar      " + Datetime ("midsommar").toISOLocalExtended ());
    t.diag ("  midsommarafton " + Datetime ("midsommarafton").toISOLocalExtended ());
    t.diag ("  3am            " + Datetime ("3am").toISOLocalExtended ());
  }

  catch (const std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
