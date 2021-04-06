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
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (47);

  // Grammar with no input.
  try
  {
    PEG p;
    p.loadFromString ("");
    t.fail ("PEG: No input");
  }
  catch (const std::string& e) { t.is (e, "There are no rules defined.", "PEG: No input"); }

  // Grammar with no rules.
  try
  {
    PEG p;
    p.loadFromString ("# No grammar\n\n");
    t.fail ("PEG: Missing grammar, only comment");
  }
  catch (const std::string& e) { t.is (e, "There are no rules defined.", "PEG: Missing grammar, only comment"); }

  // Grammar with undefined reference.
  try
  {
    PEG p;
    p.loadFromString ("this: that");
    t.fail ("PEG: Grammar with undefined reference");
  }
  catch (const std::string& e) { t.is (e, "Definition 'that' referenced, but not defined.", "PEG: Grammar with undefined reference"); }

  // Grammar with left recursion.
  try
  {
    PEG p;
    p.loadFromString ("this: this");
    t.fail ("PEG: Grammar with left recursion");
  }
  catch (const std::string& e) { t.is (e, "Definition 'this' is left recursive.", "PEG: Grammar with left recursion"); }

  // Grammar with unreferenced definition.
  try
  {
    PEG p;
    p.strict (true);
    p.loadFromString ("this: that\nthat: 'a'\nother: 'b'");
    t.fail ("PEG: Grammar with unreferenced definition");
  }
  catch (const std::string& e) { t.is (e, "Definition 'other' is defined, but not referenced.", "PEG: Grammar with unreferenced definition"); }

  // Grammar with unsupported intrinsic.
  try
  {
    PEG p;
    p.loadFromString ("this: <thing>");
    t.fail ("PEG: Grammar with unsupported intrinsic");
  }
  catch (const std::string& e) { t.is (e, "Specified intrinsic '<thing>' is not supported.", "PEG: Grammar with unsupported intrinsic"); }

  // Grammar that is valid.
  PEG p;
  p.loadFromString ("this: that+\nthat: other?\nother: a* a &a a !a\na: 'a'");
  //t.diag (p.dump ());
  t.is (p.firstRule (), "this", "PEG: firstRule found");

  auto rules = p.syntax ();
  t.is (rules["this"][0][0]._token,  "that",                                       "PEG: this: that");
  t.ok (rules["this"][0][0]._quantifier == PEG::Token::Quantifier::one_or_more,    "PEG: this: that quantifier one_or_more");
  t.ok (rules["this"][0][0]._lookahead == PEG::Token::Lookahead::none,             "PEG: this: that lookahead none");
  t.ok (rules["this"][0][0]._tags == std::set <std::string> {},                    "PEG: this: that tags {}");

  t.is (rules["that"][0][0]._token,  "other",                                      "PEG: that: other");
  t.ok (rules["that"][0][0]._quantifier == PEG::Token::Quantifier::zero_or_one,    "PEG: that: other quantifier zero_or_one");
  t.ok (rules["that"][0][0]._lookahead == PEG::Token::Lookahead::none,             "PEG: that: other lookahead none");
  t.ok (rules["that"][0][0]._tags == std::set <std::string> {},                    "PEG: that: other tags {}");

  t.is (rules["other"][0][0]._token, "a",                                          "PEG: other: a");
  t.ok (rules["other"][0][0]._quantifier == PEG::Token::Quantifier::zero_or_more,  "PEG: other: a quantifier zero_or_more");
  t.ok (rules["other"][0][0]._lookahead == PEG::Token::Lookahead::none,            "PEG: other: a lookahead none");
  t.ok (rules["other"][0][0]._tags == std::set <std::string> {},                   "PEG: other: a tags {}");
  t.is (rules["other"][0][1]._token, "a",                                          "PEG: other: a");
  t.ok (rules["other"][0][1]._quantifier == PEG::Token::Quantifier::one,           "PEG: other: a quantifier one");
  t.ok (rules["other"][0][1]._lookahead == PEG::Token::Lookahead::none,            "PEG: other: a lookahead none");
  t.ok (rules["other"][0][1]._tags == std::set <std::string> {},                   "PEG: other: a tags {}");
  t.is (rules["other"][0][2]._token, "a",                                          "PEG: other: a");
  t.ok (rules["other"][0][2]._quantifier == PEG::Token::Quantifier::one,           "PEG: other: a quantifier one");
  t.ok (rules["other"][0][2]._lookahead == PEG::Token::Lookahead::positive,        "PEG: other: a lookahead positive");
  t.ok (rules["other"][0][2]._tags == std::set <std::string> {},                   "PEG: other: a tags {}");
  t.is (rules["other"][0][3]._token, "a",                                          "PEG: other: a");
  t.ok (rules["other"][0][3]._quantifier == PEG::Token::Quantifier::one,           "PEG: other: a quantifier one");
  t.ok (rules["other"][0][3]._lookahead == PEG::Token::Lookahead::none,            "PEG: other: a lookahead none");
  t.ok (rules["other"][0][3]._tags == std::set <std::string> {},                   "PEG: other: a tags {}");
  t.is (rules["other"][0][4]._token, "a",                                          "PEG: other: a");
  t.ok (rules["other"][0][4]._quantifier == PEG::Token::Quantifier::one,           "PEG: other: a quantifier one");
  t.ok (rules["other"][0][4]._lookahead == PEG::Token::Lookahead::negative,        "PEG: other: a lookahead negative");
  t.ok (rules["other"][0][4]._tags == std::set <std::string> {},                   "PEG: other: a tags {}");

  t.is (rules["a"][0][0]._token,     "'a'",                                        "PEG: a: 'a'");
  t.ok (rules["a"][0][0]._quantifier == PEG::Token::Quantifier::one,               "PEG: a: 'a' quantifier one");
  t.ok (rules["a"][0][0]._lookahead == PEG::Token::Lookahead::none,                "PEG: a: 'a' lookahead none");
  t.ok (rules["a"][0][0]._tags == std::set <std::string> {"character", "literal"}, "PEG: a: 'a' tags {}");

  // PEG::removeComment (const std::string&) const;
  t.is (PEG::removeComment (""),                  "",          "PEG::removeComment '' --> ''");
  t.is (PEG::removeComment (" \t"),               " \t",       "PEG::removeComment ' \\t' --> ' \\t'");
  t.is (PEG::removeComment ("foo"),               "foo",       "PEG::removeComment 'foo' --> 'foo'");
  t.is (PEG::removeComment ("foo#bar"),           "foo",       "PEG::removeComment 'foo#bar' --> 'foo'");
  t.is (PEG::removeComment ("#foo"),              "",          "PEG::removeComment '#foo' --> ''");
  t.is (PEG::removeComment (" #foo#bar"),         " ",         "PEG::removeComment ' #foo#bar' --> ' '");
  t.is (PEG::removeComment ("foo\\#bar#comment"), "foo\\#bar", "PEG::removeComment 'foo\\#bar#comment' --> 'foo\\#bar'");
  t.is (PEG::removeComment ("foo'#'bar#comment"), "foo'#'bar", "PEG::removeComment \"foo'#'bar#comment\" --> \"foo'#'bar\"");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
