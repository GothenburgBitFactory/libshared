////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2010 - 2021, Göteborg Bit Factory.
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
#include <PEG.h>
#include <Packrat.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (27);

  // Grammar that is valid.
  PEG peg;
  peg.loadFromString ("thing: a !b c\na: 'a'\nb: 'b'\nc: 'c'");
  t.is (peg.firstRule (), "thing", "negative: firstRule found");

  auto rules = peg.syntax ();
  t.is (rules["thing"][0][0]._token,  "a",                                  "negative: thing: a");
  t.ok (rules["thing"][0][0]._quantifier == PEG::Token::Quantifier::one,    "negative: thing: a quantifier one");
  t.ok (rules["thing"][0][0]._lookahead == PEG::Token::Lookahead::none,     "negative: thing: a lookahead none");
  t.ok (rules["thing"][0][0]._tags == std::set <std::string> {},            "negative: thing: a tags {}");

  t.is (rules["thing"][0][1]._token,  "b",                                  "negative: thing: b");
  t.ok (rules["thing"][0][1]._quantifier == PEG::Token::Quantifier::one,    "negative: thing: b quantifier one");
  t.ok (rules["thing"][0][1]._lookahead == PEG::Token::Lookahead::negative, "negative: thing: b lookahead negative");
  t.ok (rules["thing"][0][1]._tags == std::set <std::string> {},            "negative: thing: b tags {}");

  t.is (rules["thing"][0][2]._token,  "c",                                  "negative: thing: c");
  t.ok (rules["thing"][0][2]._quantifier == PEG::Token::Quantifier::one,    "negative: thing: c quantifier one");
  t.ok (rules["thing"][0][2]._lookahead == PEG::Token::Lookahead::none,     "negative: thing: c lookahead none");
  t.ok (rules["thing"][0][2]._tags == std::set <std::string> {},            "negative: thing: c tags {}");

  t.is (rules["a"][0][0]._token,  "'a'",                                    "negative: a: char literal");
  t.ok (rules["a"][0][0]._quantifier == PEG::Token::Quantifier::one,        "negative: a: char literal quantifier one");
  t.ok (rules["a"][0][0]._lookahead == PEG::Token::Lookahead::none,         "negative: a: char literal lookahead none");
  t.ok (rules["a"][0][0]._tags == std::set <std::string> {"character", "literal"},
                                                                            "negative: a: char literal tags {'character', 'literal'}");

  t.is (rules["b"][0][0]._token,  "'b'",                                    "negative: b: char literal");
  t.ok (rules["b"][0][0]._quantifier == PEG::Token::Quantifier::one,        "negative: b: char literal quantifier one");
  t.ok (rules["b"][0][0]._lookahead == PEG::Token::Lookahead::none,         "negative: b: char literal lookahead none");
  t.ok (rules["b"][0][0]._tags == std::set <std::string> {"character", "literal"},
                                                                            "negative: b: char literal tags {'character', 'literal'}");

  t.is (rules["c"][0][0]._token,  "'c'",                                    "negative: c: char literal");
  t.ok (rules["c"][0][0]._quantifier == PEG::Token::Quantifier::one,        "negative: c: char literal quantifier one");
  t.ok (rules["c"][0][0]._lookahead == PEG::Token::Lookahead::none,         "negative: c: char literal lookahead none");
  t.ok (rules["c"][0][0]._tags == std::set <std::string> {"character", "literal"},
                                                                            "negative: c: char literal tags {'character', 'literal'}");

  // 'ac' is valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "ac");
    t.pass ("intrinsic: 'ac' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: 'ac' " + e); }

  // 'ab' is not valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "ab");  // Expected to fail.
    t.fail ("intrinsic: 'ab' not valid");
  }
  catch (const std::string& e) { t.pass ("intrinsic: 'ab' " + e); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
