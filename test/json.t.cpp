////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2021, Paul Beckingham, Federico Hernandez.
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
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <JSON.h>
#include <test.h>

const char *positive_tests[] =
{
  "{}",

  "    {    }    ",

  "[]",

  "{\"one\":1}",

  "{\n\"one\"\n:\n1\n}\n",

  "  {  \"one\"  :  1  }  ",

  "{\"name\":123, \"array\":[1,2,3.4], \"object\":{\"m1\":\"v1\", \"m2\":\"v2\"}}",

  "{\"name\":\"value\",\"array\":[\"one\",\"two\"],\"object\":{\"name2\":123,\"literal\":false}}",

  "{\n"
    "\"ticket\": { \"type\":\"add\", \"client\":\"taskwarrior 2.x\"},\n"
    "\"auth\":   { \"user\":\"paul\", \"org\":\"gbf\", \"key\":\".........\",\n"
    "            \"locale\":\"en-US\" },\n"
    "\n"
    "\"add\":    { \"description\":\"Wash the dog\",\n"
    "            \"project\":\"home\",\n"
    "            \"due\":\"20101101T000000Z\" }\n"
  "}",

  "{"
    "\"ticket\":{"
      "\"type\":\"synch\","
      "\"client\":\"taskd-test-suite 1.0\""
    "},"
    "\"synch\":{"
      "\"user\":{"
        "\"data\":["
          "{"
            "\"uuid\":\"11111111-1111-1111-1111-111111111111\","
            "\"status\":\"pending\","
            "\"description\":\"This is a test\","
            "\"entry\":\"20110111T124000Z\""
          "}"
        "],"
        "\"synch\":\"key\""
      "}"
    "},"
    "\"auth\":{"
      "\"org\":\"gbf\","
      "\"user\":\"Paul Beckingham\","
      "\"key\":\"K\","
      "\"locale\":\"en-US\""
    "}"
  "}"
};

#define NUM_POSITIVE_TESTS (sizeof (positive_tests) / sizeof (positive_tests[0]))

const char *negative_tests[] =
{
  "",
  "{",
  "}",
  "[",
  "]",
  "foo",
  "[?]"
};

#define NUM_NEGATIVE_TESTS (sizeof (negative_tests) / sizeof (negative_tests[0]))

std::stringstream combined;
class EventSink : public json::SAX::Sink
{
public:
  void eventDocStart () override                            { combined << "<doc>";                            }
  void eventDocEnd () override                              { combined << "</doc>";                           }
  void eventObjectStart () override                         { combined << "<object>";                         }
  void eventObjectEnd (int) override                        { combined << "</object>";                        }
  void eventArrayStart () override                          { combined << "<array>";                          }
  void eventArrayEnd (int) override                         { combined << "</array>";                         }
  void eventName (const std::string& value) override        { combined << "<name>"   << value << "</name>";   }
  void eventValueNull () override                           { combined << "<null />";                         }
  void eventValueBool (bool value) override                 { combined << "<bool>"   << value << "</bool>";   }
  void eventValueInt (int64_t value) override               { combined << "<int>"    << value << "</int>";    }
  void eventValueUint (uint64_t value) override             { combined << "<uint>"   << value << "</uint>";   }
  void eventValueDouble (double value) override             { combined << "<double>" << value << "</double>"; }
  void eventValueString (const std::string& value) override { combined << "<string>" << value << "</string>"; }
};

////////////////////////////////////////////////////////////////////////////////
void saxTest (UnitTest& t, const std::string& input, const std::string& expected)
{
  try
  {
    combined.str (std::string ());
    EventSink sink;
    json::SAX sax;
    t.ok (sax.parse (input, sink), std::string ("sax: '") + input + "' --> '" + expected + "'");
  }

  catch (const std::string& e) { t.pass (e); }
  catch (...)                  { t.fail ("Unknown error"); }
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (NUM_POSITIVE_TESTS + NUM_NEGATIVE_TESTS + 28 + 4);

  // Ensure environment has no influence.
  unsetenv ("TASKDATA");
  unsetenv ("TASKRC");

  // Positive tests.
  for (unsigned int i = 0; i < NUM_POSITIVE_TESTS; ++i)
  {
    // JSON
    try
    {
      json::value* root = json::parse (positive_tests[i]);
      t.ok (root, std::string ("positive: ") + positive_tests[i]);
      if (root)
      {
        t.diag (root->dump ());
        delete root;
      }
    }

    catch (const std::string& e) { t.fail (e); }
    catch (...)                  { t.diag ("Unknown error"); }
  }

  // Negative tests.
  for (unsigned int i = 0; i < NUM_NEGATIVE_TESTS; ++i)
  {
    // JSON
    try
    {
      json::value* root = json::parse (negative_tests[i]);
      t.is ((const char*) root, (const char*) NULL,
            std::string ("negative: ") + negative_tests[i]);
    }

    catch (const std::string& e) { t.pass (e); }
    catch (...)                  { t.fail ("Unknown error"); }
  }

  // Other tests.
  try
  {
    // JSON: Regular unit tests.
    t.is (json::encode ("1\"2"), "1\\\"2",   "json::encode <quote> -> <backslash><quote>");
    t.is (json::decode ("1\\\"2"), "1\"2",   "json::decode <backslash><quote> -> <quote>");

    t.is (json::encode ("1/2"), "1\\/2",     "json::encode <slash> -> <backslash><slash>");
    t.is (json::decode ("1\\/2"), "1/2",     "json::decode <backslash><slash> -> <slash>");

    t.is (json::encode ("1\b2"), "1\\b2",    "json::encode <bell> -> <backslash><backslash><b>");
    t.is (json::decode ("1\\b2"), "1\b2",    "json::decode <backslash><backslash><b> -> <bell>");

    t.is (json::encode ("1\f2"), "1\\f2",    "json::encode <formfeed> -> <backslash><backslash><f>");
    t.is (json::decode ("1\\f2"), "1\f2",    "json::decode <backslash><backslash><f> -> <formfeed>");

    t.is (json::encode ("1\n2"), "1\\n2",    "json::encode <newline> -> <backslash><newline>");
    t.is (json::decode ("1\\n2"), "1\n2",    "json::decode <backslash><newline> -> <newline>");

    t.is (json::encode ("1\r2"), "1\\r2",    "json::encode <cr> -> <backslash><r>");
    t.is (json::decode ("1\\r2"), "1\r2",    "json::decode <backslash><r> -> <cr>");

    t.is (json::encode ("1\t2"), "1\\t2",    "json::encode <tab> -> <backslash><t>");
    t.is (json::decode ("1\\t2"), "1\t2",    "json::decode <backslash><t> -> <tab>");

    t.is (json::encode ("1\\2"), "1\\\\2",   "json::encode <backslash> -> <backslash><backslash>");
    t.is (json::decode ("1\\\\2"), "1\\2",   "json::decode <backslash><backslash> -> <backslash>");

    t.is (json::encode ("1\x2"), "1\x2",     "json::encode <backslash><x> -> <backslash><x>(NOP)");
    t.is (json::decode ("1\x2"), "1\x2",     "json::decode <backslash><x> -> <backslash><x>(NOP)");

    t.is (json::encode ("1€2"), "1€2",       "json::encode € -> €");
    t.is (json::decode ("1\\u20ac2"), "1€2", "json::decode <backslash>u20ac -> €");

    std::string encoded = json::encode ("one\\");
    t.is (encoded, "one\\\\",                "json::encode one<backslash> -> one<backslash><backslash>");
    t.is ((int)encoded.length (), 5,         "json::encode one<backslash> -> length 5");
    t.is (encoded[0], 'o',                   "json::encode one<backslash>[0] -> o");
    t.is (encoded[1], 'n',                   "json::encode one<backslash>[1] -> n");
    t.is (encoded[2], 'e',                   "json::encode one<backslash>[2] -> e");
    t.is (encoded[3], '\\',                  "json::encode one<backslash>[3] -> <backslash>");
    t.is (encoded[4], '\\',                  "json::encode one<backslash>[4] -> <backslash>");

    t.is (json::decode (encoded), "one\\",   "json::decode one<backslash><backslash> -> one<backslash>");
  }

  catch (const std::string& e) {t.diag (e);}

  // SAX tests.
  saxTest (t,
           "{}",
           "<doc><object></object></doc>");
  saxTest (t,
           "{\"name\":\"value\"}",
           "<doc><object><name>name</name><string>value</string></object></doc>");
  saxTest (t,
           "\n\t {\n\t \"name\"\n\t :\n\t \"value\"\n\t }\n\t ",
           "<doc><object><name>name</name><string>value</string></object></doc>");
  saxTest (t,
           "[1,\"2\",true,null]",
           "<doc><array><int>1</int><string>2</string><bool>true</bool><null /></array></doc>");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
