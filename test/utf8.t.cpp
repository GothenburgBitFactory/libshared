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
#include <utf8.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (33);

  std::string ascii_text            = "This is a test";
  std::string utf8_text             = "más sábado miércoles";
  std::string utf8_wide_text        = "改变各种颜色";

  std::string ascii_text_color      = "This [1mis[0m a test";
  std::string utf8_text_color       = "más [1msábado[0m miércoles";
  std::string utf8_wide_text_color  = "改[1m变各种[0m颜色";

  // unsigned int utf8_codepoint (const std::string&);
  t.is ((int) utf8_codepoint ("\\u0020"),              32, "\\u0020 --> ' '");
  t.is ((int) utf8_codepoint ("U+0020"),               32, "U+0020 --> ' '");

  // TODO unsigned int utf8_next_char (const std::string&, std::string::size_type&);
  // TODO std::string utf8_character (unsigned int);
  // TODO int utf8_sequence (unsigned int);

  // unsigned int utf8_length (const std::string&);
  t.is ((int) utf8_length (ascii_text),                14, "ASCII utf8_length");
  t.is ((int) utf8_length (utf8_text),                 20, "UTF8 utf8_length");
  t.is ((int) utf8_length (utf8_wide_text),             6, "UTF8 wide utf8_length");

  // unsigned int utf8_width (const std::string&);
  t.is ((int) utf8_width (ascii_text),                 14, "ASCII utf8_width");
  t.is ((int) utf8_width (utf8_text),                  20, "UTF8 utf8_width");
  t.is ((int) utf8_width (utf8_wide_text),             12, "UTF8 wide utf8_width");

  // unsigned int utf8_text_length (const std::string&);
  t.is ((int) utf8_text_length (ascii_text_color),     14, "ASCII utf8_text_length");
  t.is ((int) utf8_text_length (utf8_text_color),      20, "UTF8 utf8_text_length");
  t.is ((int) utf8_text_length (utf8_wide_text_color),  6, "UTF8 wide utf8_text_length");

  // unsigned int utf8_text_width (const std::string&);
  t.is ((int) utf8_text_width (ascii_text_color),      14, "ASCII utf8_text_width");
  t.is ((int) utf8_text_width (utf8_text_color),       20, "UTF8 utf8_text_width");
  t.is ((int) utf8_text_width (utf8_wide_text_color),  12, "UTF8 wide utf8_text_width");

  // const std::string utf8_substr (const std::string&, unsigned int, unsigned int length = 0);
  t.is (utf8_substr (ascii_text, 0, 2),                    "Th", "ASCII utf8_substr");
  t.is (utf8_substr (utf8_text, 0, 2),                     "má", "UTF8 utf8_substr");
  t.is (utf8_substr (utf8_wide_text, 0, 2),                "改变", "UTF8 wide utf8_substr");

  // int mk_wcwidth (wchar_t);

  // Control characters
  t.is (mk_wcwidth ('\0'),                              0, "mk_wcwidth U+0000 --> 0");
  t.is (mk_wcwidth ('\r'),                              0, "mk_wcwidth U+000D --> 0");

  // Basic latin characters
  t.is (mk_wcwidth ('!'),                               1, "mk_wcwidth U+0021 '!' --> 1");
  t.is (mk_wcwidth ('$'),                               1, "mk_wcwidth U+0024 '$' --> 1");
  t.is (mk_wcwidth ('%'),                               1, "mk_wcwidth U+0025 '%' --> 1");
  t.is (mk_wcwidth ('+'),                               1, "mk_wcwidth U+002B '+' --> 1");
  t.is (mk_wcwidth ('0'),                               1, "mk_wcwidth U+0030 '0' --> 1");
  t.is (mk_wcwidth ('A'),                               1, "mk_wcwidth U+0041 'A' --> 1");
  t.is (mk_wcwidth ('Z'),                               1, "mk_wcwidth U+005A 'Z' --> 1");
  t.is (mk_wcwidth ('a'),                               1, "mk_wcwidth U+0061 'a' --> 1");
  t.is (mk_wcwidth ('z'),                               1, "mk_wcwidth U+007A 'z' --> 1");
  t.is (mk_wcwidth ('~'),                               1, "mk_wcwidth U+007E '~' --> 1");

  // Latin-1 Supplement
  t.is (mk_wcwidth (0x00A3),                            1, "mk_wcwidth U+00A3 '£' --> 1");
  t.is (mk_wcwidth (0x00AE),                            1, "mk_wcwidth U+00AE '®' --> 1");
  t.is (mk_wcwidth (0x00B5),                            1, "mk_wcwidth U+00B5 'µ' --> 1");
  t.is (mk_wcwidth (0x00C0),                            1, "mk_wcwidth U+00C0 'À' --> 1");
  t.is (mk_wcwidth (0x00C1),                            1, "mk_wcwidth U+00C1 'Á' --> 1");
  t.is (mk_wcwidth (0x00C4),                            1, "mk_wcwidth U+00C4 'Ä' --> 1");
  t.is (mk_wcwidth (0x00C7),                            1, "mk_wcwidth U+00C7 'Ç' --> 1");
  t.is (mk_wcwidth (0x00D6),                            1, "mk_wcwidth U+00D6 'Ö' --> 1");
  t.is (mk_wcwidth (0x00D8),                            1, "mk_wcwidth U+00D8 'Ø' --> 1");
  t.is (mk_wcwidth (0x00DD),                            1, "mk_wcwidth U+00DD 'Ý' --> 1");
  t.is (mk_wcwidth (0x00DF),                            1, "mk_wcwidth U+00DF 'ß' --> 1");
  t.is (mk_wcwidth (0x00E0),                            1, "mk_wcwidth U+00E0 'à' --> 1");
  t.is (mk_wcwidth (0x00E1),                            1, "mk_wcwidth U+00E1 'á' --> 1");
  t.is (mk_wcwidth (0x00E4),                            1, "mk_wcwidth U+00E4 'ä' --> 1");
  t.is (mk_wcwidth (0x00F4),                            1, "mk_wcwidth U+00F4 'ô' --> 1");
  t.is (mk_wcwidth (0x00FC),                            1, "mk_wcwidth U+00FC 'ü' --> 1");

  // Latin-1 Extended
  t.is (mk_wcwidth (0x010C),                            1, "mk_wcwidth U+010C 'Č' --> 1");
  t.is (mk_wcwidth (0x010D),                            1, "mk_wcwidth U+010D 'č' --> 1");
  t.is (mk_wcwidth (0x0110),                            1, "mk_wcwidth U+0110 'Đ' --> 1");
  t.is (mk_wcwidth (0x0118),                            1, "mk_wcwidth U+0118 'Ę' --> 1");
  t.is (mk_wcwidth (0x0122),                            1, "mk_wcwidth U+0122 'Ģ' --> 1");
  t.is (mk_wcwidth (0x0139),                            1, "mk_wcwidth U+0139 'Ĺ' --> 1");
  t.is (mk_wcwidth (0x013A),                            1, "mk_wcwidth U+013A 'ĺ' --> 1");
  t.is (mk_wcwidth (0x013D),                            1, "mk_wcwidth U+013D 'Ľ' --> 1");
  t.is (mk_wcwidth (0x0151),                            1, "mk_wcwidth U+0151 'ő' --> 1");
  t.is (mk_wcwidth (0x0161),                            1, "mk_wcwidth U+0161 'š' --> 1");
  t.is (mk_wcwidth (0x0164),                            1, "mk_wcwidth U+0164 'Ť' --> 1");
  t.is (mk_wcwidth (0x017E),                            1, "mk_wcwidth U+017E 'ž' --> 1");

  // Latin-1 Extended
  t.is (mk_wcwidth (0x018C),                            1, "mk_wcwidth U+018C 'ƌ' --> 1");
  t.is (mk_wcwidth (0x0190),                            1, "mk_wcwidth U+0190 'Ɛ' --> 1");
  t.is (mk_wcwidth (0x019E),                            1, "mk_wcwidth U+019E 'ƞ' --> 1");
  t.is (mk_wcwidth (0x01FA),                            1, "mk_wcwidth U+01FA 'Ǻ' --> 1");

  // Slovenian, Croatian
  t.is (mk_wcwidth (0x020F),                            1, "mk_wcwidth U+020F 'ȏ' --> 1");
  t.is (mk_wcwidth (0x0213),                            1, "mk_wcwidth U+0213 'ȓ' --> 1");

  // Romanian
  t.is (mk_wcwidth (0x0219),                            1, "mk_wcwidth U+0219 'ș' --> 1");

  // Livonian
  t.is (mk_wcwidth (0x022F),                            1, "mk_wcwidth U+022F 'ȯ' --> 1");

  // Misc
  t.is (mk_wcwidth (0x0247),                            1, "mk_wcwidth U+0247 'ɇ' --> 1");

  // Emojis
  t.is (mk_wcwidth (0x26C4),                            2, "mk_wcwidth U+26C4 '⛄' --> 2");
  t.is (mk_wcwidth (0x2744),                            1, "mk_wcwidth U+2744 '❄' --> 1");
  t.is (mk_wcwidth (0x1F381),                           2, "mk_wcwidth U+1F381 '🎁' --> 2");
  t.is (mk_wcwidth (0x1F384),                           2, "mk_wcwidth U+1F384 '🎄' --> 2");
  t.is (mk_wcwidth (0x1F4BB),                           2, "mk_wcwidth U+1F4BB '💻' --> 2");
  t.is (mk_wcwidth (0x1F44D),                           2, "mk_wcwidth U+1F44D '👍' --> 2");
  t.is (mk_wcwidth (0x1F602),                           2, "mk_wcwidth U+1F602 '😂' --> 2");
  t.is (mk_wcwidth (0x1F64F),                           2, "mk_wcwidth U+1F64F '🙏' --> 2");

  t.is (mk_wcwidth (0x5149),                            2, "mk_wcwidth U+5149 --> 2");
  t.is (mk_wcwidth (0x9a8c),                            2, "mk_wcwidth U+9a8c --> 2");
  t.is (mk_wcwidth (0x4e70),                            2, "mk_wcwidth U+4e70 --> 2");
  t.is (mk_wcwidth (0x94b1),                            2, "mk_wcwidth U+94b1 --> 2");
  t.is (mk_wcwidth (0x5305),                            2, "mk_wcwidth U+5305 --> 2");
  t.is (mk_wcwidth (0x91cd),                            2, "mk_wcwidth U+91cd --> 2");
  t.is (mk_wcwidth (0x65b0),                            2, "mk_wcwidth U+65b0 --> 2");
  t.is (mk_wcwidth (0x8bbe),                            2, "mk_wcwidth U+8bbe --> 2");
  t.is (mk_wcwidth (0x8ba1),                            2, "mk_wcwidth U+8ba1 --> 2");
  t.is (mk_wcwidth (0x5411),                            2, "mk_wcwidth U+5411 --> 2");
  t.is (mk_wcwidth (0x4e0a),                            2, "mk_wcwidth U+4e0a --> 2");
  t.is (mk_wcwidth (0x4e0b),                            2, "mk_wcwidth U+4e0b --> 2");
  t.is (mk_wcwidth (0x7bad),                            2, "mk_wcwidth U+7bad --> 2");
  t.is (mk_wcwidth (0x5934),                            2, "mk_wcwidth U+5934 --> 2");
  t.is (mk_wcwidth (0xff0c),                            2, "mk_wcwidth U+ff0c --> 2"); // comma

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
