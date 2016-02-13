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

#ifndef INCLUDED_DATETIME
#define INCLUDED_DATETIME

#include <string>
#include <ctime>
#include <Pig.h>

class Datetime
{
public:
  Datetime ();
  Datetime (const std::string&, const std::string& format = "");
  Datetime (time_t);
  Datetime (const int, const int, const int);
  Datetime (const int, const int, const int, const int, const int, const int);
  bool parse (const std::string&, std::string::size_type&, const std::string& format = "");
  time_t toEpoch () const;

  Datetime startOfDay () const;
  static bool leapYear (int);
  static int daysInMonth (int, int);
  static int daysInYear (int);
  static int dayOfWeek (int, int, int);

  int month () const;
  int day () const;
  int year () const;
  int dayOfWeek () const;
  int dayOfYear () const;
  int hour () const;
  int minute () const;
  int second () const;

  bool operator== (const Datetime&) const;
  bool operator!= (const Datetime&) const;
  bool operator<  (const Datetime&) const;
  bool operator>  (const Datetime&) const;
  bool operator<= (const Datetime&) const;
  bool operator>= (const Datetime&) const;
  bool sameHour   (const Datetime&) const;
  bool sameDay    (const Datetime&) const;
  bool sameMonth  (const Datetime&) const;
  bool sameYear   (const Datetime&) const;
  Datetime operator+  (const int);
  Datetime operator-  (const int);
  Datetime& operator+= (const int);
  Datetime& operator-= (const int);
  time_t operator- (const Datetime&);

private:
  void clear ();
  bool parse_formatted     (Pig&, const std::string&);
  bool parse_epoch         (Pig&);
  bool parse_date_time     (Pig&);
  bool parse_date_time_ext (Pig&);
  bool validate ();
  void resolve ();
  std::string dump () const;

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
