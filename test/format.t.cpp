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

#include <cmake.h>
#include <format.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (30);

  // std::string format (char);
  t.is (format ('A'), "A", "format ('A') -> A");

  // std::string format (int);
  t.is (format (0),  "0",  "format (0) -> 0");
  t.is (format (-1), "-1", "format (-1) -> -1");

  // std::string formatHex (int);
  t.is (formatHex (0),   "0",  "formatHex (0) -> 0");
  t.is (formatHex (10),  "a",  "formatHex (10) -> a");
  t.is (formatHex (123), "7b", "formatHex (123) -> 7b");

  // std::string format (float, int, int);
  t.is (format (0.12345678, 8, 4),      "  0.1235",     "format (0.12345678,    8,   4) -> __0.1235");

  t.is (format (1.23456789, 8, 1),      "       1",     "format (1.23456789,    8,   1) -> _______1");
  t.is (format (1.23456789, 8, 2),      "     1.2",     "format (1.23456789,    8,   2) -> _____1.2");
  t.is (format (1.23456789, 8, 3),      "    1.23",     "format (1.23456789,    8,   3) -> ____1.23");
  t.is (format (1.23456789, 8, 4),      "   1.235",     "format (1.23456789,    8,   4) -> ___1.235");
  t.is (format (1.23456789, 8, 5),      "  1.2346",     "format (1.23456789,    8,   5) -> __1.2346");
  t.is (format (1.23456789, 8, 6),      " 1.23457",     "format (1.23456789,    8,   6) ->  1.23457");
  t.is (format (1.23456789, 8, 7),      "1.234568",     "format (1.23456789,    8,   7) -> 1.234568");
  t.is (format (1.23456789, 8, 8),      "1.2345679",    "format (1.23456789,    8,   8) -> 1.2345679");
  t.is (format (2444238.56789, 12, 11), "2444238.5679", "format (2444238.56789, 12, 11) -> 2444238.5679");

  // std::string leftJustify (const std::string&, const int);
  t.is (leftJustify (123, 3), "123",   "leftJustify 123,3 -> '123'");
  t.is (leftJustify (123, 4), "123 ",  "leftJustify 123,4 -> '123 '");
  t.is (leftJustify (123, 5), "123  ", "leftJustify 123,5 -> '123  '");

  // std::string leftJustify (const std::string&, const int);
  t.is (leftJustify ("foo", 3), "foo",   "leftJustify foo,3 -> 'foo'");
  t.is (leftJustify ("foo", 4), "foo ",  "leftJustify foo,4 -> 'foo '");
  t.is (leftJustify ("foo", 5), "foo  ", "leftJustify foo,5 -> 'foo  '");
  t.is (leftJustify ("föo", 5), "föo  ", "leftJustify föo,5 -> 'föo  '");

  // std::string rightJustify (const std::string&, const int);
  t.is (rightJustify (123, 3), "123",   "rightJustify 123,3 -> '123'");
  t.is (rightJustify (123, 4), " 123",  "rightJustify 123,4 -> ' 123'");
  t.is (rightJustify (123, 5), "  123", "rightJustify 123,5 -> '  123'");

  // std::string rightJustify (const std::string&, const int);
  t.is (rightJustify ("foo", 3), "foo",   "rightJustify foo,3 -> 'foo'");
  t.is (rightJustify ("foo", 4), " foo",  "rightJustify foo,4 -> ' foo'");
  t.is (rightJustify ("foo", 5), "  foo", "rightJustify foo,5 -> '  foo'");
  t.is (rightJustify ("föo", 5), "  föo", "rightJustify föo,5 -> '  föo'");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
