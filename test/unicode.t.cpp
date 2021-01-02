////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2021, Göteborg Bit Factory.
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
#include <unicode.h>
#include <clocale>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (128);

  // White space detection.
  t.notok (unicodeWhitespace (0x0041), "U+0041 (A) ! unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x0020), "U+0020 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x0009), "U+0009 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x000A), "U+000A unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x000B), "U+000B unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x000C), "U+000C unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x000D), "U+000D unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x0085), "U+0085 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x00A0), "U+00A0 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x1680), "U+1680 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x180E), "U+180E unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2000), "U+2000 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2001), "U+2001 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2002), "U+2002 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2003), "U+2003 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2004), "U+2004 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2005), "U+2005 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2006), "U+2006 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2007), "U+2007 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2008), "U+2008 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2009), "U+2009 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x200A), "U+200A unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2028), "U+2028 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x2029), "U+2029 unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x202F), "U+202F unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x205F), "U+205F unicodeWhitespace");
  t.ok    (unicodeWhitespace (0x3000), "U+3000 unicodeWhitespace");

  // Horizontal white space detection.
  t.notok (unicodeHorizontalWhitespace (0x0041), "U+0041 (A) ! unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x0020), "U+0020 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x0009), "U+0009 unicodeHorizontalWhitespace");
  t.notok (unicodeHorizontalWhitespace (0x000A), "U+000A ! unicodeHorizontalWhitespace");
  t.notok (unicodeHorizontalWhitespace (0x000B), "U+000B ! unicodeHorizontalWhitespace");
  t.notok (unicodeHorizontalWhitespace (0x000C), "U+000C ! unicodeHorizontalWhitespace");
  t.notok (unicodeHorizontalWhitespace (0x000D), "U+000D ! unicodeHorizontalWhitespace");
  t.notok (unicodeHorizontalWhitespace (0x0085), "U+0085 ! unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x00A0), "U+00A0 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x1680), "U+1680 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x180E), "U+180E unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2000), "U+2000 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2001), "U+2001 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2002), "U+2002 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2003), "U+2003 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2004), "U+2004 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2005), "U+2005 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2006), "U+2006 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2007), "U+2007 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2008), "U+2008 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x2009), "U+2009 unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x200A), "U+200A unicodeHorizontalWhitespace");
  t.notok (unicodeHorizontalWhitespace (0x2028), "U+2028 ! unicodeHorizontalWhitespace");
  t.notok (unicodeHorizontalWhitespace (0x2029), "U+2029 ! unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x202F), "U+202F unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x205F), "U+205F unicodeHorizontalWhitespace");
  t.ok    (unicodeHorizontalWhitespace (0x3000), "U+3000 unicodeHorizontalWhitespace");

  // Vertical white space detection.
  t.notok (unicodeVerticalWhitespace (0x0041), "U+0041 (A) ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x0020), "U+0020 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x0009), "U+0009 ! unicodeVerticalWhitespace");
  t.ok    (unicodeVerticalWhitespace (0x000A), "U+000A unicodeVerticalWhitespace");
  t.ok    (unicodeVerticalWhitespace (0x000B), "U+000B unicodeVerticalWhitespace");
  t.ok    (unicodeVerticalWhitespace (0x000C), "U+000C unicodeVerticalWhitespace");
  t.ok    (unicodeVerticalWhitespace (0x000D), "U+000D unicodeVerticalWhitespace");
  t.ok    (unicodeVerticalWhitespace (0x0085), "U+0085 unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x00A0), "U+00A0 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x1680), "U+1680 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x180E), "U+180E ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2000), "U+2000 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2001), "U+2001 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2002), "U+2002 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2003), "U+2003 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2004), "U+2004 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2005), "U+2005 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2006), "U+2006 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2007), "U+2007 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2008), "U+2008 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x2009), "U+2009 ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x200A), "U+200A ! unicodeVerticalWhitespace");
  t.ok    (unicodeVerticalWhitespace (0x2028), "U+2028 unicodeVerticalWhitespace");
  t.ok    (unicodeVerticalWhitespace (0x2029), "U+2029 unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x202F), "U+202F ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x205F), "U+205F ! unicodeVerticalWhitespace");
  t.notok (unicodeVerticalWhitespace (0x3000), "U+3000 ! unicodeVerticalWhitespace");

  // Latin Alpha
  t.notok (unicodeLatinAlpha (0x0033), "U+0033 (3) ! unicodeLatinAlpha");
  t.ok    (unicodeLatinAlpha (0x0041), "U+0041 (A) unicodeLatinAlpha");
  t.ok    (unicodeLatinAlpha (0x005A), "U+005A (Z) unicodeLatinAlpha");
  t.ok    (unicodeLatinAlpha (0x0061), "U+0061 (a) unicodeLatinAlpha");
  t.ok    (unicodeLatinAlpha (0x007A), "U+007A (z) unicodeLatinAlpha");

  // Latin Digit
  t.notok (unicodeLatinDigit (0x0041), "U+0041 (A) ! unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0030), "U+0030 (0) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0031), "U+0031 (1) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0032), "U+0032 (2) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0033), "U+0033 (3) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0034), "U+0034 (4) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0035), "U+0035 (5) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0036), "U+0036 (6) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0037), "U+0037 (7) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0038), "U+0038 (8) unicodeLatinDigit");
  t.ok    (unicodeLatinDigit (0x0039), "U+0039 (9) unicodeLatinDigit");

  // Hex Digit
  t.notok (unicodeHexDigit (0x0047), "U+0047 (G) ! unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0030), "U+0030 (0) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0031), "U+0031 (1) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0032), "U+0032 (2) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0033), "U+0033 (3) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0034), "U+0034 (4) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0035), "U+0035 (5) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0036), "U+0036 (6) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0037), "U+0037 (7) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0038), "U+0038 (8) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0039), "U+0039 (9) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0041), "U+0041 (A) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0042), "U+0042 (B) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0043), "U+0043 (C) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0044), "U+0044 (D) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0045), "U+0045 (E) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0046), "U+0046 (F) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0061), "U+0061 (a) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0062), "U+0062 (b) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0063), "U+0063 (c) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0064), "U+0064 (d) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0065), "U+0065 (e) unicodeHexDigit");
  t.ok    (unicodeHexDigit (0x0066), "U+0066 (f) unicodeHexDigit");

  // Punctuation
  t.notok (unicodePunctuation (0x0033), "U+0033 (3) ! unicodePunctuation");
  t.ok    (unicodePunctuation (0x002C), "U+002C (,) unicodePunctuation");
  t.ok    (unicodePunctuation (0x002E), "U+002E (.) unicodePunctuation");
  t.ok    (unicodePunctuation (0x003A), "U+003A (:) unicodePunctuation");
  t.ok    (unicodePunctuation (0x0021), "U+0021 (!) unicodePunctuation");
  t.ok    (unicodePunctuation (0x003F), "U+003F (?) unicodePunctuation");

  // Alpha
  std::setlocale (LC_ALL, "en_US.utf8");
  t.ok    (unicodeAlpha (0x0041), "U+0041 (A) unicodeAlpha");
//  t.ok    (unicodeAlpha (0x00DF), "U+00DF (ß) unicodeAlpha");
//  t.ok    (unicodeAlpha (0x00E9), "U+00E9 (é) unicodeAlpha");
//  t.ok    (unicodeAlpha (0x00F6), "U+00F6 (ö) unicodeAlpha");
  t.notok (unicodeAlpha (0x3004), "U+3004 (〄) ! unicodeAlpha");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
