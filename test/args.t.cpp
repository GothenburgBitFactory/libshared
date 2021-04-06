////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 - 2021, Paul Beckingham, Federico Hernandez.
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
#include <Args.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (16);

  // Success case.
  Args a1;
  a1.limitPositionals (2);
  a1.addOption ("option1");
  a1.addOption ("option2", false);
  a1.addOption ("option3", true);
  a1.addNamed ("named1");
  a1.addNamed ("named2");
  a1.addNamed ("named3", "default");
  a1.enableNegatives ();
  const char* argv1[] = {"binary", "-option1", "--nooption2", "--named1", "value1", "pos1", "--named2", "value2", "pos2"};
  a1.scan (9, argv1);
  t.diag (a1.dump ());

  t.is (a1.getPositionalCount (), 2,       "Args1 --> positionals 2");
  t.is (a1.getPositional (0), "pos1",      "Args1 --> positional[0] 'pos1'");
  t.is (a1.getPositional (1), "pos2",      "Args1 --> positional[1] 'pos2'");

  t.is (a1.getOption ("option1"), true,    "Args1 --> option1 'true'");
  t.is (a1.getOption ("option2"), false,   "Args1 --> option2 'false'");
  t.is (a1.getOption ("option3"), true,    "Args1 --> option3 'true'");

  t.is (a1.getNamed ("named1"), "value1",  "Args1 --> named1 'value1'");
  t.is (a1.getNamed ("named2"), "value2",  "Args1 --> named2 'value2'");
  t.is (a1.getNamed ("named3"), "default", "Args1 --> named3 'default'");

  // Long/short arg names.
  Args a2;
  a2.addOption ("present");
  const char* argv2[] = {"binary", "--present"};
  a2.scan (2, argv2);
  t.diag (a2.dump ());

  t.is (a2.getPositionalCount (), 0,       "Args2 --> positionals 0");
  t.is (a2.getOption ("present"), true,    "Args2 --> present 'true'");
  t.is (a2.getOption ("missing"), false,   "Args2 --> missing 'false'");

  // Negation
  Args a3;
  a3.addOption ("positive", true);
  a3.addOption ("negative", false);
  a3.enableNegatives ();
  const char* argv3[] = {"binary", "--nopositive", "--nonegative"};
  a3.scan (3, argv3);
  t.diag (a3.dump ());

  t.is (a3.getPositionalCount (), 0,       "Args3 --> positionals 0");
  t.is (a3.getOption ("positive"), false,  "Args3 --> nopositive 'false'");
  t.is (a3.getOption ("negative"), false,  "Args3 --> nonegative 'true'");

  // Option counting.
  Args a4;
  a4.addOption ("debug");
  const char* argv4[] = {"binary", "--debug", "-d"};
  a4.scan (3, argv4);
  t.is (a4.getOptionCount ("debug"), 2,    "Args4 --> optionCount 2");

  // TODO Test ambiguous abbreviations.

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

