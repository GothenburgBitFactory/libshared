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
  UnitTest t (7);

  // Grammar that is valid.
  PEG peg;
  peg.loadFromString ("thing: 'a'");
  t.is (peg.firstRule (), "thing",                                          "charliteral: firstRule found");

  auto rules = peg.syntax ();
  t.is (rules["thing"][0][0]._token,  "'a'",                                "charliteral: thing: 'a'");
  t.ok (rules["thing"][0][0]._quantifier == PEG::Token::Quantifier::one,    "charliteral: thing: 'a' quantifier one");
  t.ok (rules["thing"][0][0]._lookahead == PEG::Token::Lookahead::none,     "charliteral: thing: 'a' lookahead none");
  t.ok (rules["thing"][0][0]._tags == std::set <std::string> {"character", "literal"},
                                                                            "charliteral: thing: 'a' tags {'character', 'literal'}");

  // 'a' is valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "a");
    t.pass ("charliteral: 'a' valid");
  }
  catch (const std::string& e) { t.fail ("charliteral: 'a' " + e); }

  // 'b' is not valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "b");  // Expected to fail.
    t.fail ("charliteral: 'b' not valid");
  }
  catch (const std::string& e) { t.pass ("charliteral: 'b' " + e); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
