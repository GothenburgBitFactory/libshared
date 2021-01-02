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

#include <cmake.h>
#include <format.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (79);

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

  t.is (format ("pre {1} post",    "mid"),        "pre mid post",    "format 1a");
  t.is (format ("pre {1}{1} post", "mid"),        "pre midmid post", "format 1b");
  t.is (format ("pre {1} post",    0),            "pre 0 post",      "format 1c");
  t.is (format ("pre {1}{1} post", 0),            "pre 00 post",     "format 1d");

  t.is (format ("pre {1}{2} post", "one", "two"), "pre onetwo post", "format 2a");
  t.is (format ("pre {2}{1} post", "one", "two"), "pre twoone post", "format 2b");
  t.is (format ("pre {1}{2} post", "one", 2),     "pre one2 post",   "format 2c");
  t.is (format ("pre {1}{2} post", 1, "two"),     "pre 1two post",   "format 2d");
  t.is (format ("pre {1}{2} post", 1, 2),         "pre 12 post",     "format 2e");

  t.is (format ("pre {1}{2}{3} post", "one", "two", "three"), "pre onetwothree post", "format 3a");
  t.is (format ("pre {3}{1}{2} post", "one", "two", "three"), "pre threeonetwo post", "format 3b");

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

  // std::string commify (const std::string& data)
  t.is (commify (""),           "",              "commify '' -> ''");
  t.is (commify ("1"),          "1",             "commify '1' -> '1'");
  t.is (commify ("12"),         "12",            "commify '12' -> '12'");
  t.is (commify ("123"),        "123",           "commify '123' -> '123'");
  t.is (commify ("1234"),       "1,234",         "commify '1234' -> '1,234'");
  t.is (commify ("12345"),      "12,345",        "commify '12345' -> '12,345'");
  t.is (commify ("123456"),     "123,456",       "commify '123456' -> '123,456'");
  t.is (commify ("1234567"),    "1,234,567",     "commify '1234567' -> '1,234,567'");
  t.is (commify ("12345678"),   "12,345,678",    "commify '12345678' -> '12,345,678'");
  t.is (commify ("123456789"),  "123,456,789",   "commify '123456789' -> '123,456,789'");
  t.is (commify ("1234567890"), "1,234,567,890", "commify '1234567890' -> '1,234,567,890'");

  t.is (commify ("pre"),         "pre",          "commify 'pre' -> 'pre'");
  t.is (commify ("pre1234"),     "pre1,234",     "commify 'pre1234' -> 'pre1,234'");
  t.is (commify ("1234post"),    "1,234post",    "commify '1234post' -> '1,234post'");
  t.is (commify ("pre1234post"), "pre1,234post", "commify 'pre1234post' -> 'pre1,234post'");

  // std::string formatBytes (size_t);
  t.is (formatBytes (0), "0 B", "0 -> 0 B");

  t.is (formatBytes (994),  "994 B", "994 -> 994 B");
  t.is (formatBytes (995),  "1.0 KiB", "995 -> 1.0 KiB");
  t.is (formatBytes (999),  "1.0 KiB", "999 -> 1.0 KiB");
  t.is (formatBytes (1000), "1.0 KiB", "1000 -> 1.0 KiB");
  t.is (formatBytes (1001), "1.0 KiB", "1001 -> 1.0 KiB");

  t.is (formatBytes (999999),  "1.0 MiB", "999999 -> 1.0 MiB");
  t.is (formatBytes (1000000), "1.0 MiB", "1000000 -> 1.0 MiB");
  t.is (formatBytes (1000001), "1.0 MiB", "1000001 -> 1.0 MiB");

  t.is (formatBytes (999999999),  "1.0 GiB", "999999999 -> 1.0 GiB");
  t.is (formatBytes (1000000000), "1.0 GiB", "1000000000 -> 1.0 GiB");
  t.is (formatBytes (1000000001), "1.0 GiB", "1000000001 -> 1.0 GiB");

  // std::string printable (const std::string&);
  t.is (printable ("f\ro\no\tb\va\vr"), "f\\ro\\no\\tb\\va\\vr", "printable f\\ro\\no\\tb\\va\\vr --> f\\\\ro\\\\no\\\\tb\\\\va\\\\vr");
  t.is (printable ("foobar"), "foobar",                          "printable foobar --> foobar");

  // std::string printable (char);
  t.is (printable ('\r'), "\\r", "printable \\r --> \\\\r");
  t.is (printable ('\n'), "\\n", "printable \\n --> \\\\n");
  t.is (printable ('\f'), "\\f", "printable \\f --> \\\\f");
  t.is (printable ('\t'), "\\t", "printable \\t --> \\\\t");
  t.is (printable ('\v'), "\\v", "printable \\v --> \\\\v");
  t.is (printable ('x'), "x",    "printable x --> x");

  // std::string obfuscateText (const std::string&);
  t.is (obfuscateText ("foo"),                    "xxx",                    "obfuscateText 'foo' --> 'xxx'");
  t.is (obfuscateText ("[33mfoo[0m"),         "[33mxxx[0m",         "obfuscateText '<red>foo</red>' --> '<red>xxx</red>'");
  t.is (obfuscateText ("one[33mtwo[0mthree"), "xxx[33mxxx[0mxxxxx", "obfuscateText 'one<red>two</red>three' --> 'xxx<red>xxx</red>xxxxx'");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
