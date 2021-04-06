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
#include <iostream>
#include <PEG.h>
#include <Packrat.h>
#include <test.h>
#include <Pig.h>
#include <Tree.h>

////////////////////////////////////////////////////////////////////////////////
bool externalParser (Pig& pig, std::shared_ptr <Tree> branch)
{
  if (pig.skipLiteral ("foo"))
  {
    branch->tag ("TEST");
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (7);

  // External parsing.
  PEG peg1;
  peg1.loadFromString ("thing: <external:foo> <digit>");
  t.is (peg1.firstRule (), "thing", "external: firstRule found");

  auto rules = peg1.syntax ();
  t.is (rules["thing"][0][0]._token,  "<external:foo>",                  "external: thing: <external:foo>");
  t.ok (rules["thing"][0][0]._quantifier == PEG::Token::Quantifier::one, "external: thing: <external:foo> quantifier one");
  t.ok (rules["thing"][0][0]._lookahead == PEG::Token::Lookahead::none,  "external: thing: <external:foo> lookahead none");
  t.ok (rules["thing"][0][0].hasTag ("external"),                        "external: thing: <external:foo> tag 'external'");
  t.ok (rules["thing"][0][0].hasTag ("intrinsic"),                       "external: thing: <external:foo> tag 'intrinsic'");

  try
  {
    Packrat rat;
    rat.external ("foo", externalParser);
    rat.parse (peg1, "foo3");
    t.diag (rat.dump ());
    t.pass ("external: 'foo3' valid");
  }
  catch (const std::string& e) { t.fail ("external: 'foo3' " + e); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
