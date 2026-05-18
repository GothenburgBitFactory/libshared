////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 - 2017, 2019 - 2021, 2023, Gothenburg Bit Factory.
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
// https://opensource.org/license/mit
//
////////////////////////////////////////////////////////////////////////////////

#include <Composite.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (4);

  Composite c1;
  c1.add ("left",  2, Color ());
  c1.add ("right", 4, Color ());
  t.is (c1.str (), "  leright", "Composite left/2 + right/4 --> '  leright'");

  Composite c2;
  c2.add ("left",  2, Color ("white on red"));
  c2.add ("right", 4, Color ("white on blue"));
  t.diag (c2.str ());

  Composite c3;
  c3.add ("aaaaaaaaaa",  2, Color ());
  c3.add ("bbbbb",       5, Color ());
  c3.add ("c",          15, Color ());
  t.is (c3.str (), "  aaabbbbbaa   c", "Composite aaaaaaaaaa/2 + bbbbb/5 + c/15 --> '  aaabbbbbaa   c'");

  Composite c4;
  c4.add ("aaaaaaaaaa",  2, Color ("white on red"));
  c4.add ("bbbbb",       5, Color ("white on blue"));
  c4.add ("c",          15, Color ("white on green"));
  t.diag (c4.str ());

  Composite c5;
  c5.add ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0, Color ());
  c5.add ("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",   1, Color ());
  c5.add ("ccccccccccccccccccccccccccccccccccccccccccccccc",     2, Color ());
  c5.add ("ddddddddddddddddddddddddddddddddddddddddddddd",       3, Color ());
  c5.add ("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",         4, Color ());
  c5.add ("fffffffffffffffffffffffffffffffffffffffff",           5, Color ());
  c5.add ("ggggggggggggggggggggggggggggggggggggggg",             6, Color ());
  c5.add ("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh",               7, Color ());
  c5.add ("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",                 8, Color ());
  c5.add ("jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj",                   9, Color ());
  c5.add ("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",                    10, Color ());
  c5.add ("lllllllllllllllllllllllllllll",                      11, Color ());
  c5.add ("mmmmmmmmmmmmmmmmmmmmmmmmmmm",                        12, Color ());
  c5.add ("nnnnnnnnnnnnnnnnnnnnnnnnn",                          13, Color ());
  c5.add ("ooooooooooooooooooooooo",                            14, Color ());
  c5.add ("ppppppppppppppppppppp",                              15, Color ());
  c5.add ("qqqqqqqqqqqqqqqqqqq",                                16, Color ());
  c5.add ("rrrrrrrrrrrrrrrrr",                                  17, Color ());
  c5.add ("sssssssssssssss",                                    18, Color ());
  c5.add ("ttttttttttttt",                                      19, Color ());
  c5.add ("uuuuuuuuuuu",                                        20, Color ());
  c5.add ("vvvvvvvvv",                                          21, Color ());
  c5.add ("wwwwwww",                                            22, Color ());
  c5.add ("xxxxx",                                              23, Color ());
  c5.add ("yyy",                                                24, Color ());
  c5.add ("z",                                                  25, Color ());
  t.is (c5.str (), "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba", "Composite ... --> 'abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba'");

  Composite c6;
  c6.add ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0, Color ("blue on gray0"));
  c6.add ("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",   1, Color ("blue on gray1"));
  c6.add ("ccccccccccccccccccccccccccccccccccccccccccccccc",     2, Color ("blue on gray2"));
  c6.add ("ddddddddddddddddddddddddddddddddddddddddddddd",       3, Color ("blue on gray3"));
  c6.add ("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",         4, Color ("blue on gray4"));
  c6.add ("fffffffffffffffffffffffffffffffffffffffff",           5, Color ("blue on gray5"));
  c6.add ("ggggggggggggggggggggggggggggggggggggggg",             6, Color ("blue on gray6"));
  c6.add ("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh",               7, Color ("blue on gray7"));
  c6.add ("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",                 8, Color ("blue on gray8"));
  c6.add ("jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj",                   9, Color ("blue on gray9"));
  c6.add ("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",                    10, Color ("blue on gray10"));
  c6.add ("lllllllllllllllllllllllllllll",                      11, Color ("blue on gray11"));
  c6.add ("mmmmmmmmmmmmmmmmmmmmmmmmmmm",                        12, Color ("blue on gray12"));
  c6.add ("nnnnnnnnnnnnnnnnnnnnnnnnn",                          13, Color ("blue on gray13"));
  c6.add ("ooooooooooooooooooooooo",                            14, Color ("blue on gray14"));
  c6.add ("ppppppppppppppppppppp",                              15, Color ("blue on gray15"));
  c6.add ("qqqqqqqqqqqqqqqqqqq",                                16, Color ("blue on gray16"));
  c6.add ("rrrrrrrrrrrrrrrrr",                                  17, Color ("blue on gray17"));
  c6.add ("sssssssssssssss",                                    18, Color ("blue on gray18"));
  c6.add ("ttttttttttttt",                                      19, Color ("blue on gray19"));
  c6.add ("uuuuuuuuuuu",                                        20, Color ("blue on gray20"));
  c6.add ("vvvvvvvvv",                                          21, Color ("blue on gray21"));
  c6.add ("wwwwwww",                                            22, Color ("blue on gray22"));
  c6.add ("xxxxx",                                              23, Color ("blue on gray23"));
  c6.add ("yyy",                                                24, Color ("blue on bright white"));
  c6.add ("z",                                                  25, Color ("blue on bright white"));
  t.diag (c6.str ());

  // Bug:   The background red of "one" continues for "two" and should not.
  // Cause: This was because the composited output was like this:
  //          <fg><bg>one<fg>two<end>
  //        and should have been like this:
  //          <fg><bg>one<end><fg>two<end>
  Composite c7;
  c7.add ("one", 0, Color ("white on red"));
  c7.add ("two", 3, Color ("green"));
  t.diag (c7.str ());

  // Bug:   Strings that end at the same location bleed color.
  // Cause: Caused by layer 0 being used to terminate color, and if layer 0 had
  //        no color, there was no termination.
  Composite c8;
  c8.add ("..........", 0, Color ());
  c8.add (       "foo", 7, Color ("white on red"));
  t.diag (c8.str ());

  // Add layers containing characters with non-standard Unicode width.
  // Verify that they are composited correctly.
  //   * Each zero-width character should be included in the column of the
  //     preceding non-zero-width character on the same layer. (If there is
  //     no such character, the zero-width character should be skipped.)
  //   * Each wide character should be treated as occupying two columns of the
  //     layer, the one corresponding to the array index at which the character
  //     code is stored, and the next one.
  //   * If exactly one of the columns occupied by a wide characher is also
  //     occupied by a character in a higher layer (obscuring half of the wide
  //     character), then the wide character should not be displayed at all.
  //     The unobscured column should be treated as containing blank space
  //     (but still be covered by the current layer).
  Composite c9;
  c9.add ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0, Color ());  // BG
  c9.add ("a", 50, Color ());  // more BG
  c9.add ("😃😃😃", 1, Color ());  // some wide chars
  c9.add ("bb", 1, Color ());  // obscure the first of the two wide chars
  c9.add ("😖😖😖", 8, Color ());  // a few more wide chars
  c9.add ("cc", 9, Color ());  // obscure half of each of the first two
  c9.add ("😬😬😬", 15, Color ());  // even more
  c9.add ("会会会", 18, Color ());  // obscure the last one-and-half
  c9.add ("[èé][ñn̄][öô]", 25, Color ());  // layer with zero-width chars (combining diacritics)
  c9.add ("}{", 32, Color ());  // obscure two of the non-zero-width chars
  c9.add ("è🐋é🐋", 38, Color ());  // 1-col, 0-col and 2-col chars on same layer
  c9.add ("\a\aff", 45, Color ());  // zero-width characters at beginning of layer
  t.is (c9.str (), "abb😃😃a cc 😖a😬 会会会a[èé][ñn̄}{öô]aè🐋é🐋affa  a", "Composite ... --> 'abb😃😃a cc 😖a😬 会会会a[èé][ñn̄}{öô]aè🐋é🐋affa  a'");

  // Add colored layers containing characters with non-standard Unicode width.
  // Display the result.
  Composite c10;
  c10.add ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 0, Color ("black on bright blue"));  // BG
  c10.add ("a", 50, Color ("black on bright blue"));  // more BG
  c10.add ("😃😃😃", 1, Color ("yellow on grey10"));  // some wide chars
  c10.add ("bb", 1, Color ("red on black"));  // obscure the first of the two wide chars
  c10.add ("😖😖😖", 8, Color ("green on blue"));  // a few more wide chars
  c10.add ("cc", 9, Color ("grey18 on green"));  // obscure half of each of the first two
  c10.add ("😬😬😬", 15, Color ("white on red"));  // even more
  c10.add ("会会会", 18, Color ("magenta on grey6"));  // obscure the last one-and-half
  c10.add ("[èé][ñn̄][öô]", 25, Color ("blue on white"));  // layer with zero-width chars (combining diacritics)
  c10.add ("}{", 32, Color ("red on white"));  // obscure two of the non-zero-width chars
  c10.add ("è🐋é🐋", 38, Color ("yellow on cyan"));  // 1-col, 0-col and 2-col chars on same layer
  c10.add ("\a\aff", 45, Color ("black on bright yellow"));  // zero-width characters at beginning of layer
  t.diag (c10.str ());

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
