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

#ifndef INCLUDED_DURATION
#define INCLUDED_DURATION

#include <Pig.h>
#include <string>
#include <time.h>

class Duration
{
public:
  Duration ();
  Duration (const std::string&);
  Duration (time_t);
  bool parse (const std::string&, std::string::size_type&);
  bool parse_seconds (Pig&);

private:
  void clear ();
  void resolve ();
  std::string dump () const;

public:
  int _year;
  int _month;
  int _day;
  int _hours;
  int _minutes;
  int _seconds;
  time_t _period;
};

#endif

////////////////////////////////////////////////////////////////////////////////
