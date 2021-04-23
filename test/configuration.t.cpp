///////////////////////////////////////////////////////////////////////////////
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
#include <Configuration.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (21);

  // bool has (const std::string&);
  Configuration c1;
  t.notok (c1.has ("missing"),    "Configuration 'missing' --> false");

  // void set (const std::string&, const std::string&);
  // std::string get (const std::string&);
  c1.set ("name", "value");
  t.ok (c1.has ("name"),          "Configuration 'name' --> true");
  t.is (c1.get ("name"), "value", "Configuration 'name' --> true");

  // void set (const std::string&, const int);
  // int getInteger (const std::string&);
  c1.set ("integer", 1);
  t.ok (c1.getInteger ("integer") == 1, "Configuration 'integer' --> 1");

  // void set (const std::string&, const double);
  // double getReal (const std::string&);
  c1.set ("real", 3.14);
  t.ok (c1.getReal ("real") == 3.14, "Configuration 'real' --> 3.14");

  // bool getBoolean (const std::string&);
  c1.set ("boolean1", "true");
  c1.set ("boolean2", "1");
  c1.set ("boolean3", "y");
  c1.set ("boolean4", "yes");
  c1.set ("boolean5", "on");
  t.ok (c1.getBoolean ("boolean1") == true,    "Configuration 'boolean1' --> true");
  t.ok (c1.getBoolean ("boolean2") == true,    "Configuration 'boolean2' --> true");
  t.ok (c1.getBoolean ("boolean3") == true,    "Configuration 'boolean3' --> true");
  t.ok (c1.getBoolean ("boolean4") == true,    "Configuration 'boolean4' --> true");
  t.ok (c1.getBoolean ("boolean5") == true,    "Configuration 'boolean5' --> true");

  // void parse (const std::string&, int nest = 1, const std::vector <std::string>& search_paths = {});
  Configuration c2;
  c2.parse ("\n"
            "# Comment\n"
            "integer=1\n"
            "boolean=true\n"
            "real=3.14\n"
            "string=hello\n"
            "\n"
            " \t foo\t=     bar   # Comment\n");
  t.ok (c2.getInteger ("integer") == 1,       "Configuration 'integer' --> 1");
  t.ok (c2.getBoolean ("boolean") == true,    "Configuration 'boolean' --> true");
  t.ok (c2.getReal    ("real")    == 3.14,    "Configuration 'real' --> 3.14");
  t.ok (c2.get        ("string")  == "hello", "Configuration 'string' --> 'hello'");
  t.is (c2.get        ("foo"),       "bar",   "Configuration 'foo' --> 'bar'");

  // std::vector <std::string> all () const;
  t.ok (c2.all ().size () == 5, "Configuration::all --> 'integer', 'boolean', 'real', 'string', 'foo'");
  t.ok (c2.size () == 5,        "Configuration::size --> 'integer', 'boolean', 'real', 'string', 'foo'");

  // void setIfBlank (const std::string& key, const std::string& value)
  Configuration c3;
  c3.set ("foo", 1);
  c3.set ("bar", "");
  c3.setIfBlank ("foo", "1plus");
  c3.setIfBlank ("bar", "2plus");
  c3.setIfBlank ("baz", "3plus");
  t.is (c3.get ("foo"), "1",     "Configuration::setIfBlank doesn't change non-blank values.");
  t.is (c3.get ("bar"), "2plus", "Configuration::setIfBlank changes blank values.");
  t.is (c3.get ("baz"), "3plus", "Configuration::setIfBlank sets missing values.");

  // void load (const std::string&, int nest = 1, const std::vector <std::string>& search_paths = {});
  setenv("FOO", "2.conf", 1);
  Configuration c4;
  c4.load ("config/1.conf", 1, { "config/nest" });
  t.is (c4.get ("hello"), "world", "Configuration::load with search paths");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
