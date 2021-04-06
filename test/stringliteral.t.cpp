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
  peg.loadFromString ("thing: \"abc\"");
  t.is (peg.firstRule (), "thing",                                          "stringliteral: firstRule found");

  auto rules = peg.syntax ();
  t.is (rules["thing"][0][0]._token,  "\"abc\"",                            "stringliteral: thing: 'abc'");
  t.ok (rules["thing"][0][0]._quantifier == PEG::Token::Quantifier::one,    "stringliteral: thing: 'abc' quantifier one");
  t.ok (rules["thing"][0][0]._lookahead == PEG::Token::Lookahead::none,     "stringliteral: thing: 'abc' lookahead none");
  t.ok (rules["thing"][0][0]._tags == std::set <std::string> {"string", "literal"},
                                                                            "stringliteral: thing: 'abc' tags {'string', 'literal'}");

  // 'abc' is valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "abc");
    t.pass ("stringliteral: 'abc' valid");
  }
  catch (const std::string& e) { t.fail ("stringliteral: 'abc' " + e); }

  // 'def' is not valid.
  try
  {
    Packrat rat;
    rat.parse (peg, "def");  // Expected to fail.
    t.fail ("stringliteral: 'def' not valid");
  }
  catch (const std::string& e) { t.pass ("stringliteral: 'def' " + e); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
