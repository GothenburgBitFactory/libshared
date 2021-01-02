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

#ifndef INCLUDED_DATETIME
#define INCLUDED_DATETIME

#include <string>
#include <ctime>
#include <Pig.h>

class Datetime
{
public:
  static int weekstart;
  static int minimumMatchLength;
  static bool isoEnabled;
  static bool standaloneDateEnabled;
  static bool standaloneTimeEnabled;
  static bool timeRelative;

  Datetime ();
  Datetime (const std::string&, const std::string& format = "");
  Datetime (time_t);
  Datetime (const int, const int, const int);
  Datetime (const int, const int, const int, const int, const int, const int);
  bool parse (const std::string&, std::string::size_type&, const std::string& format = "");
  time_t toEpoch () const;
  std::string toEpochString () const;
  std::string toISO () const;
  std::string toISOLocalExtended () const;
  double toJulian () const;
  void toYMD (int&, int&, int&) const;
  const std::string toString (const std::string& format = "Y-M-D") const;

  Datetime startOfDay () const;
  Datetime startOfWeek () const;
  Datetime startOfMonth () const;
  Datetime startOfYear () const;

  static bool valid (const std::string&, const std::string& format = "");
  static bool valid (const int, const int, const int, const int, const int, const int);
  static bool valid (const int, const int, const int);
  static bool valid (const int, const int);
  static bool leapYear (int);
  static int daysInMonth (int, int);
  static int daysInYear (int);
  static std::string monthName (int);
  static std::string monthNameShort (int);
  static std::string dayName (int);
  static std::string dayNameShort (int);
  static int dayOfWeek (const std::string&);
  static int dayOfWeek (int, int, int);
  static int monthOfYear (const std::string&);
  static int length (const std::string&);

  int month () const;
  int week () const;
  int day () const;
  int year () const;
  int dayOfWeek () const;
  int dayOfYear () const;
  int hour () const;
  int minute () const;
  int second () const;

  bool operator==  (const Datetime&) const;
  bool operator!=  (const Datetime&) const;
  bool operator<   (const Datetime&) const;
  bool operator>   (const Datetime&) const;
  bool operator<=  (const Datetime&) const;
  bool operator>=  (const Datetime&) const;
  bool sameHour    (const Datetime&) const;
  bool sameDay     (const Datetime&) const;
  bool sameWeek    (const Datetime&) const;
  bool sameMonth   (const Datetime&) const;
  bool sameQuarter (const Datetime&) const;
  bool sameYear    (const Datetime&) const;
  Datetime operator+  (const int);
  Datetime operator-  (const int);
  Datetime& operator+= (const int);
  Datetime& operator-= (const int);
  time_t operator- (const Datetime&);
  void operator--  ();    // Prefix
  void operator--  (int); // Postfix
  void operator++  ();    // Prefix
  void operator++  (int); // Postfix

private:
  void clear ();
  bool parse_formatted     (Pig&, const std::string&);
  bool parse_named         (Pig&);
  bool parse_epoch         (Pig&);
  bool parse_date_time_ext (Pig&);
  bool parse_date_ext      (Pig&);
  bool parse_off_ext       (Pig&);
  bool parse_time_ext      (Pig&, bool terminated = true);
  bool parse_time_utc_ext  (Pig&);
  bool parse_time_off_ext  (Pig&);
  bool parse_date_time     (Pig&);
  bool parse_date          (Pig&);
  bool parse_time_utc      (Pig&);
  bool parse_time_off      (Pig&);
  bool parse_time          (Pig&, bool terminated = true);
  bool parse_off           (Pig&);

  bool parse_year          (Pig&, int&);
  bool parse_month         (Pig&, int&);
  bool parse_week          (Pig&, int&);
  bool parse_julian        (Pig&, int&);
  bool parse_day           (Pig&, int&);
  bool parse_weekday       (Pig&, int&);
  bool parse_hour          (Pig&, int&);
  bool parse_minute        (Pig&, int&);
  bool parse_second        (Pig&, int&);
  bool parse_off_hour      (Pig&, int&);
  bool parse_off_minute    (Pig&, int&);

  bool initializeNow            (Pig&);
  bool initializeYesterday      (Pig&);
  bool initializeToday          (Pig&);
  bool initializeTomorrow       (Pig&);
  bool initializeOrdinal        (Pig&);
  bool initializeDayName        (Pig&);
  bool initializeMonthName      (Pig&);
  bool initializeLater          (Pig&);
  bool initializeSopd           (Pig&);
  bool initializeSod            (Pig&);
  bool initializeSond           (Pig&);
  bool initializeEopd           (Pig&);
  bool initializeEod            (Pig&);
  bool initializeEond           (Pig&);
  bool initializeSopw           (Pig&);
  bool initializeSow            (Pig&);
  bool initializeSonw           (Pig&);
  bool initializeEopw           (Pig&);
  bool initializeEow            (Pig&);
  bool initializeEonw           (Pig&);
  bool initializeSopww          (Pig&);
  bool initializeSonww          (Pig&);
  bool initializeSoww           (Pig&);
  bool initializeEopww          (Pig&);
  bool initializeEonww          (Pig&);
  bool initializeEoww           (Pig&);
  bool initializeSopm           (Pig&);
  bool initializeSom            (Pig&);
  bool initializeSonm           (Pig&);
  bool initializeEopm           (Pig&);
  bool initializeEom            (Pig&);
  bool initializeEonm           (Pig&);
  bool initializeSopq           (Pig&);
  bool initializeSoq            (Pig&);
  bool initializeSonq           (Pig&);
  bool initializeEopq           (Pig&);
  bool initializeEoq            (Pig&);
  bool initializeEonq           (Pig&);
  bool initializeSopy           (Pig&);
  bool initializeSoy            (Pig&);
  bool initializeSony           (Pig&);
  bool initializeEopy           (Pig&);
  bool initializeEoy            (Pig&);
  bool initializeEony           (Pig&);
  bool initializeEaster         (Pig&);
  bool initializeMidsommar      (Pig&);
  bool initializeMidsommarafton (Pig&);
  bool initializeInformalTime   (Pig&);
  void easter (struct tm*) const;
  void midsommar (struct tm*) const;
  void midsommarafton (struct tm*) const;

  bool initializeNthDayInMonth  (const std::vector <std::string>&);

  bool isOrdinal (const std::string&, int&);

  bool validate ();
  void resolve ();

public:
  int _year    {0};
  int _month   {0};
  int _week    {0};
  int _weekday {0};
  int _julian  {0};
  int _day     {0};
  int _seconds {0};
  int _offset  {0};
  bool _utc    {false};
  time_t _date {0};
};

#endif

////////////////////////////////////////////////////////////////////////////////
