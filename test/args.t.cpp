////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 - 2016, Paul Beckingham, Federico Hernandez.
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
  UnitTest t (9);

  // Success case.
  Args a1;
  a1.limitPositionals (2);
  a1.addOption ("option1");
  a1.addOption ("option2", true);
  a1.addOption ("option3", true);
  a1.addNamed ("named1");
  a1.addNamed ("named2");
  a1.addNamed ("named3", "default");
  const char* argv1[] = {"binary", "-option1", "--nooption2", "--named1", "value1", "pos1", "--named2", "value2", "pos2"};
  a1.scan (9, argv1);
  t.diag (a1.dump ());

  t.is (a1.getPositionalCount (), 2,       "Args --> positionals 2");
  t.is (a1.getPositional (0), "pos1",      "Args --> positional[0] 'pos1'");
  t.is (a1.getPositional (1), "pos2",      "Args --> positional[1] 'pos2'");

  t.is (a1.getOption ("option1"), true,    "Args --> option1 'true'");
  t.is (a1.getOption ("option2"), false,   "Args --> option2 'false'");
  t.is (a1.getOption ("option3"), true,    "Args --> option3 'true'");

  t.is (a1.getNamed ("named1"), "value1",  "Args --> named1 'value1'");
  t.is (a1.getNamed ("named2"), "value2",  "Args --> named2 'value2'");
  t.is (a1.getNamed ("named3"), "default", "Args --> named3 'default'");

  // TODO Test errors
/*
  try
  {
  }

  catch (std::string& e)
  {
    t.diag (e);
  }
*/

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

