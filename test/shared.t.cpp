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
#include <shared.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (126);

  // void wrapText (std::vector <std::string>& lines, const std::string& text, const int width, bool hyphenate)
  std::string text = "This is a test of the line wrapping code.";
  std::vector <std::string> lines;
  wrapText (lines, text, 10, true);
  t.is (lines.size (), (size_t) 5, "wrapText 'This is a test of the line wrapping code.' -> total 5 lines");
  t.is (lines[0], "This is a",     "wrapText line 0 -> 'This is a'");
  t.is (lines[1], "test of",       "wrapText line 1 -> 'test of'");
  t.is (lines[2], "the line",      "wrapText line 2 -> 'the line'");
  t.is (lines[3], "wrapping",      "wrapText line 3 -> 'wrapping'");
  t.is (lines[4], "code.",         "wrapText line 4 -> 'code.'");

  text = "This ☺ is a test of utf8 line extraction.";
  lines.clear ();
  wrapText (lines, text, 7, true);
  t.is (lines.size (), (size_t) 7, "wrapText 'This ☺ is a test of utf8 line extraction.' -> total 7 lines");
  t.is (lines[0], "This ☺",        "wrapText line 0 -> 'This ☺'");
  t.is (lines[1], "is a",          "wrapText line 1 -> 'is a'");
  t.is (lines[2], "test of",       "wrapText line 2 -> 'test of'");
  t.is (lines[3], "utf8",          "wrapText line 3 -> 'utf8'");
  t.is (lines[4], "line",          "wrapText line 4 -> 'line'");
  t.is (lines[5], "extrac-",       "wrapText line 5 -> 'extrac-'");
  t.is (lines[6], "tion.",         "wrapText line 6 -> 'tion.'");

  text = "one two three\n  four";
  lines.clear ();
  wrapText (lines, text, 13, true);
  t.is (lines.size (), (size_t) 2, "wrapText 'one two three\\n  four' -> 2 lines");
  t.is (lines[0], "one two three", "wrapText line 0 -> 'one two three'");
  t.is (lines[1], "  four",        "wrapText line 1 -> '  four'");

  // void extractLine (std::string& text, std::string& line, int length, bool hyphenate, unsigned int& offset)
  text = "This ☺ is a test of utf8 line extraction.";
  unsigned int offset = 0;
  std::string line;
  extractLine (line, text, 7, true, offset);
  t.is (line, "This ☺", "extractLine 7 'This ☺ is a test of utf8 line extraction.' -> 'This ☺'");

  // void extractLine (std::string& text, std::string& line, int length, bool hyphenate, unsigned int& offset)
  text = "line 1\nlengthy second line that exceeds width";
  offset = 0;
  extractLine (line, text, 10, true, offset);
  t.is (line, "line 1", "extractLine 10 'line 1\\nlengthy second line that exceeds width' -> 'line 1'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "lengthy", "extractLine 10 'lengthy second line that exceeds width' -> 'lengthy'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "second", "extractLine 10 'second line that exceeds width' -> 'second'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "line that", "extractLine 10 'line that exceeds width' -> 'line that'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "exceeds", "extractLine 10 'exceeds width' -> 'exceeds'");

  extractLine (line, text, 10, true, offset);
  t.is (line, "width", "extractLine 10 'width' -> 'width'");

  t.notok (extractLine (line, text, 10, true, offset), "extractLine 10 '' -> ''");

  text = "AAAAAAAAAABBBBBBBBBB";
  offset = 0;
  extractLine (line, text, 10, true, offset);
  t.is (line, "AAAAAAAAA-", "extractLine hyphenated unbreakable line");
  t.diag (line);

  // std::vector <std::string> split (const std::string& input, const char delimiter)
  std::string unsplit = "";
  std::vector <std::string> items = split (unsplit, '-');
  t.is (items.size (), (size_t) 0, "split '' '-' -> 0 items");

  unsplit = "a";
  items = split (unsplit, '-');
  t.is (items.size (), (size_t) 1, "split 'a' '-' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  items = split (unsplit, '-');
  t.is (items.size (), (size_t) 1, "split 'a' '-' -> 1 item");
  t.is (items[0], "a",             "split 'a' '-' -> 'a'");

  unsplit = "-";
  items = split (unsplit, '-');
  t.is (items.size (), (size_t) 2, "split '-' '-' -> '' ''");
  t.is (items[0], "",              "split '-' '-' -> [0] ''");
  t.is (items[1], "",              "split '-' '-' -> [1] ''");

  unsplit = "-a-bc--def";
  items = split (unsplit, '-');
  t.is (items.size (), (size_t) 5, "split '-a-bc--def' '-' -> '' 'a' 'bc' '' 'def'");
  t.is (items[0], "",              "split '-a-bc--def' '-' -> [0] ''");
  t.is (items[1], "a",             "split '-a-bc--def' '-' -> [1] 'a'");
  t.is (items[2], "bc",            "split '-a-bc--def' '-' -> [2] 'bc'");
  t.is (items[3], "",              "split '-a-bc--def' '-' -> [3] ''");
  t.is (items[4], "def",           "split '-a-bc--def' '-' -> [4] 'def'");

  // std::vector <std::string> split (const std::string& input);
  unsplit = "";
  items = split (unsplit);
  t.is (items.size (), (size_t) 0, "split '' -> 0 items");

  unsplit = "abc";
  items = split (unsplit);
  t.is (items.size (), (size_t) 1, "split 'abc' -> 1 item");
  t.is (items[0], "abc",           "split 'abc' -> [0] 'abc'");

  unsplit = "a b c";
  items = split (unsplit);
  t.is (items.size (), (size_t) 3, "split 'a b c' -> 3 items");
  t.is (items[0], "a",             "split 'a b c' -> [0] 'a'");
  t.is (items[1], "b",             "split 'a b c' -> [1] 'b'");
  t.is (items[2], "c",             "split 'a b c' -> [2] 'c'");

  unsplit = "  a   b   c  ";
  items = split (unsplit);
  t.is (items.size (), (size_t) 3, "split '  a  b  c  ' -> 3 items");
  t.is (items[0], "a",             "split '  a  b  c  ' -> [0] 'a'");
  t.is (items[1], "b",             "split '  a  b  c  ' -> [1] 'b'");
  t.is (items[2], "c",             "split '  a  b  c  ' -> [2] 'c'");

  // std::string join (const std::string&r, const std::vector<std::string>&)
  std::vector <std::string> unjoined;
  std::string joined;

  joined = join ("", unjoined);
  t.is (joined.length (), (size_t) 0,  "join -> length 0");
  t.is (joined,           "",          "join -> ''");

  unjoined = {"", "a", "bc", "def"};
  joined = join ("", unjoined);
  t.is (joined.length (), (size_t) 6, "join '' 'a' 'bc' 'def' -> length 6");
  t.is (joined,           "abcdef",   "join '' 'a' 'bc' 'def' -> 'abcdef'");

  joined = join ("-", unjoined);
  t.is (joined.length (), (size_t) 9,  "join '' - 'a' - 'bc' - 'def' -> length 9");
  t.is (joined,           "-a-bc-def", "join '' - 'a' - 'bc' - 'def' -> '-a-bc-def'");

  // std::string trim (const std::string&);
  t.is (trim ("one"),     "one",   "trim 'one' --> 'one'");
  t.is (trim ("  one"),   "one",   "trim '  one' --> 'one'");
  t.is (trim ("one  "),   "one",   "trim 'one  ' --> 'one'");
  t.is (trim ("  one  "), "one",   "trim '  one  ' --> 'one'");
  t.is (trim (""),        "",      "trim '' --> ''");
  t.is (trim (" \t\r\f\nfoo\n\f\r\t "), "foo",
                                   "trim ' \t\r\f\nfoo\n\f\r\t ' --> 'foo'");

  t.is (trim ("abcdedcba", "abc"), "ded", "trim 'abcdedcba', 'abc' --> 'ded'");

  // std::string ltrim (const std::string&);
  t.is (ltrim ("one"),     "one",   "ltrim 'one' --> 'one'");
  t.is (ltrim ("  one"),   "one",   "ltrim '  one' --> 'one'");
  t.is (ltrim ("one  "),   "one  ", "ltrim 'one  ' --> 'one  '");
  t.is (ltrim ("  one  "), "one  ", "ltrim '  one  ' --> 'one  '");
  t.is (ltrim (""),        "",      "ltrim '' --> ''");
  t.is (ltrim ("  "),      "",      "ltrim '  ' --> ''");
  t.is (ltrim (" \t\r\f\nfoo\n\f\r\t "), "foo\n\f\r\t ",
                                    "ltrim ' \t\r\f\nfoo\n\f\r\t ' --> 'foo\\n\\f\\r\\t '");

  // std::string rtrim (const std::string&);
  t.is (rtrim ("one"),     "one",   "rtrim 'one' --> 'one'");
  t.is (rtrim ("  one"),   "  one", "rtrim '  one' --> '  one'");
  t.is (rtrim ("one  "),   "one",   "rtrim 'one  ' --> 'one'");
  t.is (rtrim ("  one  "), "  one", "rtrim '  one' --> '  one'");
  t.is (rtrim (""),        "",      "rtrim '' --> ''");
  t.is (rtrim ("  "),      "",      "rtrim '  ' --> ''");
  t.is (rtrim (" \t\r\f\nfoo\n\f\r\t "), " \t\r\f\nfoo",
                                    "rtrim ' \t\r\f\nfoo\n\f\r\t ' --> ' \\t\\r\\f\\nfoo'");

  // int longestWord (const std::string&)
  t.is (longestWord ("    "),                   0, "longestWord (    ) --> 0");
  t.is (longestWord ("this is a test"),         4, "longestWord (this is a test) --> 4");
  t.is (longestWord ("this is a better test"),  6, "longestWord (this is a better test) --> 6");
  t.is (longestWord ("house Çirçös clown"),     6, "longestWord (Çirçös) --> 6");

  // int longestLine (const std::string&)
  t.is (longestLine ("one two three four"),    18, "longestLine (one two three four) --> 18");
  t.is (longestLine ("one\ntwo three four"),   14, "longestLine (one\\ntwo three four) --> 14");
  t.is (longestLine ("one\ntwo\nthree\nfour"),  5, "longestLine (one\\ntwo\\nthree\\nfour) --> 5");

  // bool compare (const std::string&, const std::string&, bool sensitive = true);
  t.notok (compare ("a", "b"), "compare 'a' : 'b' --> false");
  t.notok (compare ("a", ""),  "compare 'a' : ''  --> false");
  t.notok (compare ("", "b"),  "compare ''  : 'b' --> false");
  t.ok    (compare ("", ""),   "compare ''  : ''  --> true");
  t.notok (compare ("A", "a"),        "compare            'A' : 'a' --> false");
  t.notok (compare ("A", "a", true),  "compare sensitive  'A' : 'a' --> false");
  t.ok    (compare ("A", "a", false), "compare !sensitive 'A' : 'a' --> true");

  // bool closeEnough (const std::string&, const std::string&, unsigned int minLength = 0);
  t.ok (closeEnough ("foobar", "foobar"),      "closeEnough foobar == foobar");
  t.ok (closeEnough ("foobar", "foobar", 0),   "closeEnough foobar == foobar,0");
  t.ok (closeEnough ("foobar", "foobar", 1),   "closeEnough foobar == foobar,1");
  t.ok (closeEnough ("foobar", "foobar", 2),   "closeEnough foobar == foobar,2");
  t.ok (closeEnough ("foobar", "foobar", 3),   "closeEnough foobar == foobar,3");
  t.ok (closeEnough ("foobar", "foobar", 4),   "closeEnough foobar == foobar,4");
  t.ok (closeEnough ("foobar", "foobar", 5),   "closeEnough foobar == foobar,5");
  t.ok (closeEnough ("foobar", "foobar", 6),   "closeEnough foobar == foobar,6");
  t.ok (closeEnough ("foobar", "foo",    3),   "closeEnough foobar == foo,3");

  // std::string lowerCase (const std::string&);
  t.is (lowerCase (""),   "",   "lowerCase '' --> ''");
  t.is (lowerCase ("a"),  "a",  "lowerCase 'a' --> 'a'");
  t.is (lowerCase ("aA"), "aa", "lowerCase 'aA' --> 'aa'");
  t.is (lowerCase ("A"),  "a",  "lowerCase 'A' --> 'a'");
  t.is (lowerCase ("$"),  "$",  "lowerCase '$' --> '$'");

  // std::string upperCase (const std::string&);
  t.is (upperCase (""),   "",   "upperCase '' --> ''");
  t.is (upperCase ("a"),  "A",  "upperCase 'a' --> 'A'");
  t.is (upperCase ("aA"), "AA", "upperCase 'aA' --> 'AA'");
  t.is (upperCase ("A"),  "A",  "upperCase 'A' --> 'A'");
  t.is (upperCase ("$"),  "$",  "upperCase '$' --> '$'");

  // std::string upperCaseFirst (const std::string&);
  t.is (upperCaseFirst (""),   "",   "upperCaseFirst '' --> ''");
  t.is (upperCaseFirst ("a"),  "A",  "upperCaseFirst 'a' --> 'A'");
  t.is (upperCaseFirst ("A"),  "A",  "upperCaseFirst 'A' --> 'A'");
  t.is (upperCaseFirst ("aa"), "Aa", "upperCaseFirst 'aa' --> 'Aa'");
  t.is (upperCaseFirst ("$"),  "$",  "upperCaseFirst '$' --> '$'");

  // std::string str_replace (const std::string&, const std::string&, const std::string&);
  std::string input = "Lorem ipsum dolor sit amet, est aliquip scaevola dignissim in, vim nominavi electram te.";
  t.is (str_replace (input, "x", "X"),
        "Lorem ipsum dolor sit amet, est aliquip scaevola dignissim in, vim nominavi electram te.",
        "str_replace 'x' -- 'X' (NOP)");

  t.is (str_replace (input, "e", "E"),
        "LorEm ipsum dolor sit amEt, Est aliquip scaEvola dignissim in, vim nominavi ElEctram tE.",
        "str_replace 'e' -- 'E'");

  // std::string::size_type find (const std::string&, const std::string&, bool sensitive = true);
  t.ok (find ("   AAAaaa", "A", true)  == 3, "find 'A' (sensitive)   in '   AAAaaa' --> 3");
  t.ok (find ("   AAAaaa", "A", false) == 3, "find 'A' (insensitive) in '   AAAaaa' --> 3");
  t.ok (find ("   AAAaaa", "a", true)  == 6, "find 'a' (sensitive)   in '   AAAaaa' --> 6");
  t.ok (find ("   AAAaaa", "a", false) == 3, "find 'a' (insensitive) in '   AAAaaa' --> 3");

  // std::string::size_type find (const std::string&, const std::string&, std::string::size_type, bool sensitive = true);
  t.ok (find ("aA AAAaaa", "A", 2, true)  == 3, "find 'A',2 (sensitive)   in 'aA AAAaaa' --> 3");
  t.ok (find ("aA AAAaaa", "A", 2, false) == 3, "find 'A',2 (insensitive) in 'aA AAAaaa' --> 3");
  t.ok (find ("aA AAAaaa", "a", 2, true)  == 6, "find 'a',2 (sensitive)   in 'aA AAAaaa' --> 6");
  t.ok (find ("aA AAAaaa", "a", 2, false) == 3, "find 'a',2 (insensitive) in 'aA AAAaaa' --> 3");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
