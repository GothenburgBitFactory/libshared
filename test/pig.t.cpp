////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 - 2021, Paul Beckingham, Federico Hernandez.
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
#include <Pig.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (180);

  // Pig::skip
  // Pig::skipN
  Pig p0 ("12345");
  t.notok (p0.skip ('0'),   "skip='0', '12345' --> false");
  t.ok (p0.skip ('1'),      "skip='1', '12345' --> true");
  t.ok (p0.dump ().find (" 1/5") != std::string::npos, "dump: " + p0.dump ());

  t.ok (p0.skipN (3),       "skipN=3 '12345' --> true");
  t.ok (p0.dump ().find (" 4/5") != std::string::npos, "dump: " + p0.dump ());

  t.notok (p0.skipN (2),    "skipN=2 '5' --> false");
  t.ok (p0.dump ().find (" 4/5") != std::string::npos, "dump: " + p0.dump ());

  // Pig::skipWS
  Pig p1 ("  one");
  t.ok (p1.skipWS (),       "skipWS '  one' --> true");
  t.ok (p1.dump ().find (" 2/5") != std::string::npos, "dump: " + p1.dump ());

  t.notok (p1.skipWS (),    "skipWS 'one' --> false");
  t.ok (p1.dump ().find (" 2/5") != std::string::npos, "dump: " + p1.dump ());

  // Pig::skipLiteral
  Pig p2 ("onetwo");
  t.notok (p2.skipLiteral ("two"), "skipLiteral=two 'onetwo' --> false");
  t.ok (p2.skipLiteral ("one"),    "skipLiteral=one 'onetwo' --> true");
  t.ok (p2.dump ().find (" 3/6") != std::string::npos, "dump: " + p2.dump ());

  // Pig::skipPartial
  Pig p2a ("wonderfully");
  std::string value;
  t.notok (p2a.skipPartial ("foo",    value), "skipPartial=wonderfully 'foo' --> false");
  t.ok    (p2a.skipPartial ("wonder", value), "skipPartial=wonderfully 'wonder' --> true");
  t.is    (value, "wonder",                   "skipPartial=wonderfully 'wonder' --> 'wonder'");

  value = "";
  t.ok    (p2a.skipPartial ("fun", value), "skipPartial=fully 'fun' --> true");
  t.is    (value, "fu",                    "skipPartial=fully 'fun' --> 'fu'");

  // Pig::getUntilWS
  Pig p3 ("one two three  ");
  t.ok (p3.getUntilWS (value), "getUntilWS 'one two three  ' --> true");
  t.is (value, "one",          "getUntilWS 'one two three  ' --> 'one'");
  t.ok (p3.dump ().find (" 3/15") != std::string::npos, "dump: " + p3.dump ());

  t.ok (p3.skipWS (),          "skipWS ' two three  ' --> true");

  t.ok (p3.getUntilWS (value), "getUntilWS 'two three  ' --> true");
  t.is (value, "two",          "getUntilWS 'two three  ' --> 'two'");
  t.ok (p3.dump ().find (" 7/15") != std::string::npos, "dump: " + p3.dump ());

  t.ok (p3.skipWS (),          "skipWS ' three  ' --> true");

  t.ok (p3.getUntilWS (value), "getUntilWS 'three  ' --> true");
  t.is (value, "three",        "getUntilWS 'three  ' --> 'three'");
  t.ok (p3.dump ().find (" 13/15") != std::string::npos, "dump: " + p3.dump ());

  Pig p3a ("9th ");
  t.ok (p3a.getUntilWS (value), "getUntilWS '9th ' --> true");
  t.is (value, "9th",           "getUntilWS '9th ' --> '9th'");
  t.ok (p3a.dump ().find (" 3/4") != std::string::npos, "dump: " + p3a.dump ());

  // Pig::getDigit
  Pig p4 (" 123");
  int n;
  t.notok (p4.getDigit (n), "getDigit ' 123' --> false");
  t.ok (p4.skipWS (),       "skipWS ' 123' --> true");
  t.ok (p4.dump ().find (" 1/4") != std::string::npos, "dump: " + p4.dump ());
  t.ok (p4.getDigit (n),    "getDigit '123' --> true");
  t.is (n, 1,               "getDigit '123' --> '1'");
  t.ok (p4.dump ().find (" 2/4") != std::string::npos, "dump: " + p4.dump ());

  // Pig::getDigits
  Pig p5 ("123 ");
  t.ok (p5.getDigits (n),   "getDigits '123 ' --> true");
  t.is (n, 123,             "getDigits '123 ' --> 123");
  t.ok (p5.dump ().find (" 3/4") != std::string::npos, "dump: " + p5.dump ());

  Pig p6 ("1");
  t.notok (p6.eos (),       "eos '1' --> false");
  t.ok (p6.getDigit (n),    "getDigit '1' --> true");
  t.notok (p6.getDigit (n), "getDigit '' --> false");
  t.ok (p6.eos (),          "eos '' --> true");
  t.ok (p6.dump ().find (" 1/1") != std::string::npos, "dump: " + p6.dump ());

  // Pig::getNumber
  Pig p7 ("1 ");
  t.ok (p7.getNumber (value), "getNumber '1 ' --> true");
  t.is (value, "1",           "getNumber '1 ' --> '1'");
  t.ok (p7.dump ().find (" 1/2") != std::string::npos, "dump: " + p7.dump ());

  Pig p8 ("3.14");
  t.ok (p8.getNumber (value), "getNumber '3.14' --> true");
  t.is (value, "3.14",        "getNumber '3.14' --> '3.14'");
  t.ok (p8.dump ().find (" 4/4") != std::string::npos, "dump: " + p8.dump ());

  Pig p9 ("1.23e-4 ");
  t.ok (p9.getNumber (value), "getNumber '1.23e-4 ' --> true");
  t.is (value, "1.23e-4",     "getNumber '1.23e-4 ' --> '1.23e-4'");
  t.ok (p9.dump ().find (" 7/8") != std::string::npos, "dump: " + p9.dump ());

  Pig p10 ("2.34e-5");
  double dvalue;
  t.ok (p10.getNumber (dvalue), "getNumber '2.34e-5' --> true");
  t.is (dvalue, 2.34e-5, 1e-6,  "getNumber '2.34e-5' --> 2.34e-5 +/- 1e-6");
  t.ok (p10.dump ().find (" 7/7") != std::string::npos, "dump: " + p10.dump ());

  // Pig::getRemainder
  Pig p11 ("123");
  t.ok (p11.skipN (1),       "skipN=1 '123' --> true");
  t.ok (p11.dump ().find (" 1/3") != std::string::npos, "dump: " + p11.dump ());

  t.ok (p11.getRemainder (value), "getRemainder '23' --> true");
  t.is (value, "23",        "getRemainder '23' --> '23'");
  t.ok (p11.dump ().find (" 3/3") != std::string::npos, "dump: " + p11.dump ());

  t.notok (p11.getRemainder (value), "getRemainder '' --> false");
  t.ok (p11.dump ().find (" 3/3") != std::string::npos, "dump: " + p11.dump ());

  // Pig::peek
  Pig p12 ("123");
  t.is (p12.peek (), '1',     "peek '123' --> '1'");
  t.ok (p12.dump ().find (" 0/3") != std::string::npos, "dump: " + p12.dump ());
  t.is (p12.peek (2), "12",   "peek=2 '123' --> '12'");
  t.is (p12.peek (3), "123",  "peek=3 '123' --> '123'");
  t.is (p12.peek (4), "123",  "peek=4 '123' --> '123'");
  t.is (p12.peek (20), "123", "peek=20 '123' --> '123'");
  t.ok (p12.dump ().find (" 0/3") != std::string::npos, "dump: " + p12.dump ());

  // Pig::save, Pig::restore
  Pig p13 ("123");
  t.is ((int)p13.save (), 0,    "save '123' --> 0");
  t.ok (p13.skipN (2),          "skipN=2 '123' --> true");
  t.ok (p13.dump ().find (" 2/3") != std::string::npos, "dump: " + p13.dump ());
  t.is ((int)p13.cursor (), 2,  "cursor '123' --> 2");
  t.is ((int)p13.restore (), 0, "restore '123' --> 0");
  t.ok (p13.dump ().find (" 0/3") != std::string::npos, "dump: " + p13.dump ());

  // Pig::getOneOf
  Pig p14 ("fourteenfour five");
  t.ok (p14.getOneOf ({"fourteen", "four"}, value), "getOneOf={fourteen,four} 'fourteenfour five' --> true");
  t.is (value, "fourteen",                          "getOneOf={fourteen,four} 'fourteenfour five' --> 'fourteen'");
  t.ok (p14.getOneOf ({"fourteen", "four"}, value), "getOneOf={fourteen,four} 'fourteenfour five' --> true");
  t.is (value, "four",                              "getOneOf={fourteen,four} 'fourteenfour five' --> 'four'");
  t.ok (p14.dump ().find (" 12/17") != std::string::npos, "dump: " + p14.dump ());
  t.notok (p14.getOneOf ({"five"}, value),          "getOneOf={five} 'fourteenfour five' --> false");
  t.ok (p14.dump ().find (" 12/17") != std::string::npos, "dump: " + p14.dump ());

  // Pig::getHexDigit
  Pig p15 (" 9aF");
  t.notok (p15.getHexDigit (n), "getHexDigit ' 9aF' --> false");
  t.ok (p15.skipWS (),          "skipWS ' 9aF' --> true");
  t.ok (p15.dump ().find (" 1/4") != std::string::npos, "dump: " + p15.dump ());
  t.ok (p15.getHexDigit (n),    "getHexDigit '9aF' --> true");
  t.is (n, 9,                   "getHexDigit '9aF' --> '9'");
  t.ok (p15.dump ().find (" 2/4") != std::string::npos, "dump: " + p15.dump ());

  t.ok (p15.getHexDigit (n),    "getHexDigit '9aF' --> true");
  t.is (n, 10,                  "getHexDigit '9aF' --> '10'");
  t.ok (p15.dump ().find (" 3/4") != std::string::npos, "dump: " + p15.dump ());

  t.ok (p15.getHexDigit (n),    "getHexDigit '9aF' --> true");
  t.is (n, 15,                  "getHexDigit '9aF' --> '15'");
  t.ok (p15.dump ().find (" 4/4") != std::string::npos, "dump: " + p15.dump ());

  // Pig::getQuoted
  Pig p16 ("");
  t.notok (p16.getQuoted ('"', value),  "      \"\"   :      getQuoted ('\"')     --> false");

  Pig p17 ("''");
  t.ok (p17.getQuoted ('\'', value),    "      \"''\" :      getQuoted ('\\'')   --> true");
  t.is (value, "",                      "      \"''\" :      getQuoted ('\\'')   --> ''");

  Pig p18 ("'\"'");
  t.ok (p18.getQuoted ('\'', value),    "     \"'\"'\" :      getQuoted ('\\'')   --> true");
  t.is (value, "\"",                    "     \"'\"'\" :      getQuoted ('\\'')   --> '\"'");

  Pig p19 ("'x'");
  t.ok (p19.getQuoted ('\'', value),    "     \"'x'\" :      getQuoted ('\\'')   --> true");
  t.is (value, "x",                     "     \"'x'\" :      getQuoted ('\\'')   --> \"x\"");

  Pig p20 ("'x");
  t.notok (p20.getQuoted ('\'', value), "      \"'x\" :      getQuoted ('\\'')   --> false");

  Pig p21 ("x");
  t.notok (p21.getQuoted ('\'', value), "       'x' :      getQuoted ('\\'')   --> false");

  Pig p22 ("\"one\\\"two\"");
  t.notok (p22.getQuoted ('\'', value), " \"one\\\"two\" :     getQuoted ('\\'')   --> false");
  t.ok (p22.getQuoted ('"', value),     " \"one\\\"two\" :     getQuoted ('\"')    --> true");
  t.is (value, "one\\\"two",            " \"one\\\"two\" :     getQuoted ('\"')    --> \"one\\\"two\"");

  Pig p23 ("\"one\\\\\"");
  t.ok (p23.getQuoted ('\"', value),    "  \"one\\\\\"\" :     getQuoted ('\"')    --> true");
  t.is (value, "one\\\\",               "  \"one\\\\\"\" :     getQuoted ('\"')    --> \"one\\\\\"");

  // Pig::getDigit1
  // Pig::getDigit2
  // Pig::getDigit3
  // Pig::getDigit4
  // Pig::getDigits
  Pig p24 ("122333444455555555");
  t.ok (p24.getDigit (n),   "getDigit   '122333444455555555' --> true");
  t.is (n, 1,               "getDigit   '122333444455555555' --> 1");
  t.ok (p24.dump ().find (" 1/18") != std::string::npos, "dump: " + p24.dump ());

  t.ok (p24.getDigit2 (n),  "getDigit2   '22333444455555555' --> true");
  t.is (n, 22,              "getDigit2   '22333444455555555' --> 22");
  t.ok (p24.dump ().find (" 3/18") != std::string::npos, "dump: " + p24.dump ());

  t.ok (p24.getDigit3 (n),  "getDigit3   '22333444455555555' --> true");
  t.is (n, 333,             "getDigit3   '22333444455555555' --> 333");
  t.ok (p24.dump ().find (" 6/18") != std::string::npos, "dump: " + p24.dump ());

  t.ok (p24.getDigit4 (n),  "getDigit4   '22333444455555555' --> true");
  t.is (n, 4444,            "getDigit4   '22333444455555555' --> 4444");
  t.ok (p24.dump ().find (" 10/18") != std::string::npos, "dump: " + p24.dump ());

  t.ok (p24.getDigits (n),  "getDigits   '22333444455555555' --> true");
  t.is (n, 55555555,        "getDigits   '22333444455555555' --> 55555555");
  t.ok (p24.dump ().find (" 18/18") != std::string::npos, "dump: " + p24.dump ());

  t.ok (p24.eos (),         "eos --> true");

  // Pig::getUntil
  Pig p25 ("one two three");
  t.ok (p25.getUntil (' ', value), "getUntil ' ', 'one two three' --> true");
  t.is (value, "one",              "getUntil ' ', 'one two three' --> 'one'");
  t.ok (p25.dump ().find (" 3/13") != std::string::npos, "dump: " + p25.dump ());

  t.ok (p25.skipWS (),             "skipWS ' two three' --> true");

  t.ok (p25.getUntil (' ', value), "getUntil ' ', 'two three' --> true");
  t.is (value, "two",              "getUntil ' ', 'two three' --> 'two'");
  t.ok (p25.dump ().find (" 7/13") != std::string::npos, "dump: " + p25.dump ());

  t.ok (p25.skipWS (),             "skipWS ' three' --> true");

  t.ok (p25.getUntil (' ', value), "getUntil ' ', 'three' --> true");
  t.is (value, "three",            "getUntil ' ', 'three' --> 'three'");
  t.ok (p25.dump ().find (" 13/13") != std::string::npos, "dump: " + p25.dump ());

  Pig p25a ("[foo]");
  t.ok (p25a.skip ('['),            "Pig::skip [, [foo] --> foo] --> true");
  t.ok (p25a.getUntil (']', value), "Pig::getUntil ], foo] --> true");
  t.is (value, "foo",               "Pig::getUntil ], foo] --> foo");
  t.ok (p25a.skip (']'),            "Pig::skip ], ] --> true");
  t.ok (p25a.eos (),                "Pig::eos --> true");

  // Pig::str
  Pig p26 ("abc");
  t.is (p26.str (), "abc", "str 'abc'+0 --> 'abc'");
  t.ok (p26.skip ('a'),    "skip 'a' in 'abc'");
  t.is (p26.str (), "bc",  "str 'abc'+1 --> 'bc'");

  // Pig::getDecimal
  Pig p27 ("1 ");
  t.ok (p27.getDecimal (value), "getDecimal '1 ' --> true");
  t.is (value, "1",             "getDecimal '1 ' --> '1'");
  t.ok (p27.dump ().find (" 1/2") != std::string::npos, "dump: " + p27.dump ());

  Pig p28 ("3.14");
  t.ok (p28.getDecimal (value), "getDecimal '3.14' --> true");
  t.is (value, "3.14",          "getDecimal '3.14' --> '3.14'");
  t.ok (p28.dump ().find (" 4/4") != std::string::npos, "dump: " + p28.dump ());

  Pig p29 ("1:23:45");
  int h, m, s;
  t.ok (p29.getHMS (h, m, s),   "getHMS '1:23:45' --> true");
  t.is (h,  1,                  "getHMS '1:23:45' h --> 1");
  t.is (m, 23,                  "getHMS '1:23:45' m --> 23");
  t.is (s, 45,                  "getHMS '1:23:45' s --> 45");

  Pig p30 ("12:34:56");
  t.ok (p30.getHMS (h, m, s),   "getHMS '12:34:56' --> true");
  t.is (h, 12,                  "getHMS '12:34:56' h --> 12");
  t.is (m, 34,                  "getHMS '12:34:56' m --> 34");
  t.is (s, 56,                  "getHMS '12:34:56' s --> 56");

  // Pig::substr
  Pig p31 ("abc");
  t.is (p31.substr (0, 0), "",    "substr 0,0 --> ''");
  t.is (p31.substr (0, 1), "a",   "substr 0,1 --> 'a'");
  t.is (p31.substr (0, 2), "ab" , "substr 0,2 --> 'ab'");
  t.is (p31.substr (0, 3), "abc", "substr 0,3 --> 'abc'");
  t.is (p31.substr (1, 1), "",    "substr 1,1 --> ''");
  t.is (p31.substr (1, 2), "b",   "substr 1,2 --> 'b'");
  t.is (p31.substr (1, 3), "bc",  "substr 1,3 --> 'bc'");
  t.is (p31.substr (2, 2), "",    "substr 2,2 --> ''");
  t.is (p31.substr (2, 3), "c",   "substr 2,3 --> 'c'");

  // Pig.getCharacter
  Pig p32 ("abc");
  t.ok (p32.getCharacter (n), "getcharacter 'abc' --> true");
  t.is (n, 97,                "getcharacter 'abc ' --> 'a'");
  t.ok (p32.dump ().find (" 1/3") != std::string::npos, "dump: " + p32.dump ());
  t.ok (p32.getCharacter (n), "getcharacter 'abc' --> true");
  t.is (n, 98,                "getcharacter 'abc ' --> 'b'");
  t.ok (p32.dump ().find (" 2/3") != std::string::npos, "dump: " + p32.dump ());
  t.ok (p32.getCharacter (n), "getcharacter 'abc' --> true");
  t.is (n, 99,                "getcharacter 'abc ' --> 'c'");
  t.ok (p32.dump ().find (" 3/3") != std::string::npos, "dump: " + p32.dump ());
  t.ok (p32.eos (),           "eos --> true");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
