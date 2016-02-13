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

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (53);

  try
  {
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

    t.is (Datetime::dayOfWeek ("SUNDAY"),    0, "SUNDAY == 0");
    t.is (Datetime::dayOfWeek ("sunday"),    0, "sunday == 0");
    t.is (Datetime::dayOfWeek ("Sunday"),    0, "Sunday == 0");
    t.is (Datetime::dayOfWeek ("Monday"),    1, "Monday == 1");
    t.is (Datetime::dayOfWeek ("Tuesday"),   2, "Tuesday == 2");
    t.is (Datetime::dayOfWeek ("Wednesday"), 3, "Wednesday == 3");
    t.is (Datetime::dayOfWeek ("Thursday"),  4, "Thursday == 4");
    t.is (Datetime::dayOfWeek ("Friday"),    5, "Friday == 5");
    t.is (Datetime::dayOfWeek ("Saturday"),  6, "Saturday == 6");
  }

  catch (const std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
