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
  UnitTest t (26);

  // Grammar that is valid.
  PEG peg1;
  peg1.loadFromString ("thing: <character> <digit>");
  t.is (peg1.firstRule (), "thing",                                         "intrinsic: firstRule found");

  auto rules = peg1.syntax ();
  t.is (rules["thing"][0][0]._token,  "<character>",                        "intrinsic: thing: <character>");
  t.ok (rules["thing"][0][0]._quantifier == PEG::Token::Quantifier::one,    "intrinsic: thing: <character> quantifier one");
  t.ok (rules["thing"][0][0]._lookahead == PEG::Token::Lookahead::none,     "intrinsic: thing: <character> lookahead none");
  t.ok (rules["thing"][0][0]._tags == std::set <std::string> {"intrinsic"}, "intrinsic: thing: <character> tags {'intrinsic'}");

  t.is (rules["thing"][0][1]._token,  "<digit>",                            "intrinsic: thing: <digit>");
  t.ok (rules["thing"][0][1]._quantifier == PEG::Token::Quantifier::one,    "intrinsic: thing: <digit> quantifier one");
  t.ok (rules["thing"][0][1]._lookahead == PEG::Token::Lookahead::none,     "intrinsic: thing: <digit> lookahead none");
  t.ok (rules["thing"][0][1]._tags == std::set <std::string> {"intrinsic"}, "intrinsic: thing: <digit> tags {'intrinsic'}");

  // '12' is valid.
  try
  {
    Packrat rat;
    rat.parse (peg1, "12");
    t.pass ("intrinsic: '12' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: '12' " + e); }

  // '1x' is not valid.
  try
  {
    Packrat rat;
    rat.parse (peg1, "1x");  // Expected to fail.
    t.fail ("intrinsic: '1x' not valid");
  }
  catch (const std::string& e) { t.pass ("intrinsic: '1x' " + e); }

  // thing: <punct>
  //        <sep>
  //        <eol>
  PEG peg2;
  peg2.loadFromString ("thing: <punct>\n"
                       "       <sep>\n"
                       "       <eol>\n");
  t.is (peg2.firstRule (), "thing", "intrinsic: firstRule found");

  try
  {
    Packrat rat;
    rat.parse (peg2, ",");
    t.pass ("intrinsic: ',' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: ',' " + e); }

  try
  {
    Packrat rat;
    rat.parse (peg2, " ");
    t.pass ("intrinsic: ' ' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: ' ' " + e); }

  try
  {
    Packrat rat;
    rat.parse (peg2, "\n");
    t.pass ("intrinsic: '\\n' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: '\\n' " + e); }

  try
  {
    Packrat rat;
    rat.parse (peg2, "3");
    t.pass ("intrinsic: '3' not valid");
  }
  catch (const std::string& e) { t.pass ("intrinsic: '3' " + e); }

  // thing: <ws>
  //        <alpha>
  PEG peg3;
  peg3.loadFromString ("thing: <ws>\n"
                       "       <alpha>\n");
  t.is (peg3.firstRule (), "thing", "intrinsic: firstRule found");

  try
  {
    Packrat rat;
    rat.parse (peg3, " ");
    t.pass ("intrinsic: ' ,' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: ' ' " + e); }

  try
  {
    Packrat rat;
    rat.parse (peg3, "\n");
    t.pass ("intrinsic: '\\n' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: '\\n' " + e); }

  try
  {
    Packrat rat;
    rat.parse (peg3, "A");
    t.pass ("intrinsic: 'A' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: 'A' " + e); }

  try
  {
    Packrat rat;
    rat.parse (peg3, ",");  // Expected to fail.
    t.fail ("intrinsic: ',' not valid");
  }
  catch (const std::string& e) { t.pass ("intrinsic: ',' " + e); }

  // thing: <word> <sep> <digit>
  PEG peg4;
  peg4.loadFromString ("thing: <word> <sep> <digit>\n");
  t.is (peg4.firstRule (), "thing", "intrinsic: firstRule found");

  try
  {
    Packrat rat;
    rat.parse (peg4, "foo 3");
    t.pass ("intrinsic: 'foo 3' valid");
  }
  catch (const std::string& e) { t.fail ("intrinsic: 'foo 3' " + e); }

  try
  {
    Packrat rat;
    rat.parse (peg4, "foo bar");  // Expected to fail.
    t.fail ("intrinsic: 'foo bar' not valid");
  }
  catch (const std::string& e) { t.pass ("intrinsic: 'foo bar' " + e); }

  // Entities
  PEG peg5;
  peg5.loadFromString ("thing: <entity:foo> <digit>");
  t.is (peg5.firstRule (), "thing", "entity: firstRule found");

  try
  {
    Packrat rat;
    rat.entity ("foo", "bar");
    rat.entity ("foo", "baz");
    rat.parse (peg5, "bar3");
    t.pass ("entity: 'bar3' valid");
  }
  catch (const std::string& e) { t.fail ("entity: 'bar3' " + e); }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
