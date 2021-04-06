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

#ifndef INCLUDED_DURATION
#define INCLUDED_DURATION

#include <Pig.h>
#include <string>
#include <time.h>

class Duration
{
public:
  static bool standaloneSecondsEnabled;

  Duration ();
  Duration (const std::string&);
  Duration (time_t);
  bool operator< (const Duration&);
  bool operator> (const Duration&);
  bool operator<= (const Duration&);
  bool operator>= (const Duration&);
  std::string toString () const;
  time_t toTime_t () const;
  bool parse (const std::string&, std::string::size_type&);
  bool parse_seconds (Pig&);
  bool parse_designated (Pig&);
  bool parse_weeks (Pig&);
  bool parse_units (Pig&);
  const std::string format () const;
  const std::string formatHours () const;
  const std::string formatISO () const;
  const std::string formatVague (bool padding = false) const;

  int days () const;
  int hours () const;
  int minutes () const;
  time_t seconds () const;

private:
  void clear ();
  void resolve ();
  std::string dump () const;

public:
  int _year       {0};
  int _month      {0};
  int _weeks      {0};
  int _day        {0};
  int _hours      {0};
  int _minutes    {0};
  time_t _seconds {0};
  time_t _period  {0};
};

#endif

////////////////////////////////////////////////////////////////////////////////
