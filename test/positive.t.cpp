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
  UnitTest t (23);

  // Grammar that is valid.
  PEG peg;
  peg.loadFromString ("thing: a &b b\na: 'a'\nb: 'b'");
  t.is (peg.firstRule (), "thing", "positive: firstRule found");

  auto rules = peg.syntax ();
  t.is (rules["thing"][0][0]._token,  "a",                                  "positive: thing: a");
  t.ok (rules["thing"][0][0]._quantifier == PEG::Token::Quantifier::one,    "positive: thing: a quantifier one");
  t.ok (rules["thing"][0][0]._lookahead == PEG::Token::Lookahead::none,     "positive: thing: a lookahead none");
  t.ok (rules["thing"][0][0]._tags == std::set <std::string> {},            "positive: thing: a tags {}");

  t.is (rules["thing"][0][1]._token,  "b",                                  "positive: thing: b");
  t.ok (rules["thing"][0][1]._quantifier == PEG::Token::Quantifier::one,    "positive: thing: b quantifier one");
  t.ok (rules["thing"][0][1]._lookahead == PEG::Token::Lookahead::positive, "positive: thing: b lookahead positive");
  t.ok (rules["thing"][0][1]._tags == std::set <std::string> {},            "positive: thing: b tags {}");

  t.is (rules["thing"][0][2]._token,  "b",                                  "positive: thing: c");
  t.ok (rules["thing"][0][2]._quantifier == PEG::Token::Quantifier::one,    "positive: thing: c quantifier one");
  t.ok (rules["thing"][0][2]._lookahead == PEG::Token::Lookahead::none,     "positive: thing: c lookahead none");
  t.ok (rules["thing"][0][2]._tags == std::set <std::string> {},            "positive: thing: c tags {}");

  t.is (rules["a"][0][0]._token,  "'a'",                                    "positive: a: char literal");
  t.ok (rules["a"][0][0]._quantifier == PEG::Token::Quantifier::one,        "positive: a: char literal quantifier one");
  t.ok (rules["a"][0][0]._lookahead == PEG::Token::Lookahead::none,         "positive: a: char literal lookahead none");
  t.ok (rules["a"][0][0]._tags == std::set <std::string> {"character", "literal"},
                                                                            "positive: a: char literal tags {'character', 'literal'}");

  t.is (rules["b"][0][0]._token,  "'b'",                                    "positive: b: char literal");
  t.ok (rules["b"][0][0]._quantifier == PEG::Token::Quantifier::one,        "positive: b: char literal quantifier one");
  t.ok (rules["b"][0][0]._lookahead == PEG::Token::Lookahead::none,         "positive: b: char literal lookahead none");
  t.ok (rules["b"][0][0]._tags == std::set <std::string> {"character", "literal"},
                                                                            "positive: b: char literal tags {'character', 'literal'}");

  // 'ab' is valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "ab");
    t.pass ("intrinsic: 'ab' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: 'ab' " + e); }

  // 'ac' is not valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "ac");  // Expected to fail.
    t.fail ("intrinsic: 'ac' not valid");
  }
  catch (const std::string& e) { t.pass ("intrinsic: 'ac' " + e); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
