////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
#include <JSON2.h>
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
class EventSink : public JSON2::SAX
{
public:
  void eventDocStart ()                            { combined << "<doc>";                            }
  void eventDocEnd ()                              { combined << "</doc>";                           }
  void eventObjectStart ()                         { combined << "<object>";                         }
  void eventObjectEnd (int)                        { combined << "</object>";                        }
  void eventArrayStart ()                          { combined << "<array>";                          }
  void eventArrayEnd (int)                         { combined << "</array>";                         }
  void eventName (const std::string& value)        { combined << "<name>"   << value << "</name>";   }
  void eventValueNull ()                           { combined << "<null />";                         }
  void eventValueBool (bool value)                 { combined << "<bool>"   << value << "</bool>";   }
  void eventValueInt (int64_t value)               { combined << "<int>"    << value << "</int>";    }
  void eventValueUint (uint64_t value)             { combined << "<uint>"   << value << "</uint>";   }
  void eventValueDouble (double value)             { combined << "<double>" << value << "</double>"; }
  void eventValueString (const std::string& value) { combined << "<string>" << value << "</string>"; }
};

////////////////////////////////////////////////////////////////////////////////
void saxTest (UnitTest& t, const std::string& input, const std::string& expected)
{
  // JSON2
  try
  {
    combined.str (std::string ());
    EventSink sink;
    JSON2 json;
    t.ok (json.parse (input, sink), std::string ("sax: '") + input + "' --> '" + expected + "'");
  }

  catch (const std::string& e) { t.pass (e); }
  catch (...)                  { t.fail ("Unknown error"); }
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (NUM_POSITIVE_TESTS + NUM_NEGATIVE_TESTS + 22 + 22 + 4);

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
    // Regular unit tests.
    t.is (json::encode ("1\b2"), "1\\b2",    "json::encode slashslashb -> slashslashslashslashb");
    t.is (json::decode ("1\\b2"), "1\b2",    "json::decode slashslashslashslashb -> slashslashb");

    t.is (json::encode ("1\n2"), "1\\n2",    "json::encode slashslashn -> slashslashslashslashn");
    t.is (json::decode ("1\\n2"), "1\n2",    "json::decode slashslashslashslashn -> slashslashn");

    t.is (json::encode ("1\r2"), "1\\r2",    "json::encode slashslashr -> slashslashslashslashr");
    t.is (json::decode ("1\\r2"), "1\r2",    "json::decode slashslashslashslashr -> slashslashr");

    t.is (json::encode ("1\t2"), "1\\t2",    "json::encode slashslasht -> slashslashslashslasht");
    t.is (json::decode ("1\\t2"), "1\t2",    "json::decode slashslashslashslasht -> slashslasht");

    t.is (json::encode ("1\\2"), "1\\\\2",   "json::encode slashslash -> slashslashslashslash");
    t.is (json::decode ("1\\\\2"), "1\\2",   "json::decode slashslashslashslash -> slashslash");

    t.is (json::encode ("1\x2"), "1\x2",     "json::encode slashslashx -> slashslashx(NOP)");
    t.is (json::decode ("1\x2"), "1\x2",     "json::decode slashslashx -> slashslashx(NOP)");

    t.is (json::encode ("1€2"), "1€2",       "json::encode € -> €");
    t.is (json::decode ("1\\u20ac2"), "1€2", "json::decode slashslashu20ac -> €");

    std::string encoded = json::encode ("one\\");
    t.is (encoded, "one\\\\",                "json::encode oneslashslashslashslash -> oneslashslashslashslashslashslashslashslash");
    t.is ((int)encoded.length (), 5,         "json::encode oneslashslashslashslash -> length 5");
    t.is (encoded[0], 'o',                   "json::encode oneslashslashslashslash[0] -> o");
    t.is (encoded[1], 'n',                   "json::encode oneslashslashslashslash[1] -> n");
    t.is (encoded[2], 'e',                   "json::encode oneslashslashslashslash[2] -> e");
    t.is (encoded[3], '\\',                  "json::encode oneslashslashslashslash[3] -> slashslash");
    t.is (encoded[4], '\\',                  "json::encode oneslashslashslashslash[4] -> slashslash");

    t.is (json::decode (encoded), "one\\",   "json::decode oneslashslashslashslashslashslashslashslash -> oneslashslashslashslash");

    // Regular unit tests.
    t.is (JSON2::encode ("1\b2"), "1\\b2",    "JSON2::encode slashslashb -> slashslashslashslashb");
    t.is (JSON2::decode ("1\\b2"), "1\b2",    "JSON2::decode slashslashslashslashb -> slashslashb");

    t.is (JSON2::encode ("1\n2"), "1\\n2",    "JSON2::encode slashslashn -> slashslashslashslashn");
    t.is (JSON2::decode ("1\\n2"), "1\n2",    "JSON2::decode slashslashslashslashn -> slashslashn");

    t.is (JSON2::encode ("1\r2"), "1\\r2",    "JSON2::encode slashslashr -> slashslashslashslashr");
    t.is (JSON2::decode ("1\\r2"), "1\r2",    "JSON2::decode slashslashslashslashr -> slashslashr");

    t.is (JSON2::encode ("1\t2"), "1\\t2",    "JSON2::encode slashslasht -> slashslashslashslasht");
    t.is (JSON2::decode ("1\\t2"), "1\t2",    "JSON2::decode slashslashslashslasht -> slashslasht");

    t.is (JSON2::encode ("1\\2"), "1\\\\2",   "JSON2::encode slashslash -> slashslashslashslash");
    t.is (JSON2::decode ("1\\\\2"), "1\\2",   "JSON2::decode slashslashslashslash -> slashslash");

    t.is (JSON2::encode ("1\x2"), "1\x2",     "JSON2::encode slashslashx -> slashslashx(NOP)");
    t.is (JSON2::decode ("1\x2"), "1\x2",     "JSON2::decode slashslashx -> slashslashx(NOP)");

    t.is (JSON2::encode ("1€2"), "1€2",       "JSON2::encode € -> €");
    t.is (JSON2::decode ("1\\u20ac2"), "1€2", "JSON2::decode slashslashu20ac -> €");

    encoded = JSON2::encode ("one\\");
    t.is (encoded, "one\\\\",                "JSON2::encode oneslashslashslashslash -> oneslashslashslashslashslashslashslashslash");
    t.is ((int)encoded.length (), 5,         "JSON2::encode oneslashslashslashslash -> length 5");
    t.is (encoded[0], 'o',                   "JSON2::encode oneslashslashslashslash[0] -> o");
    t.is (encoded[1], 'n',                   "JSON2::encode oneslashslashslashslash[1] -> n");
    t.is (encoded[2], 'e',                   "JSON2::encode oneslashslashslashslash[2] -> e");
    t.is (encoded[3], '\\',                  "JSON2::encode oneslashslashslashslash[3] -> slashslash");
    t.is (encoded[4], '\\',                  "JSON2::encode oneslashslashslashslash[4] -> slashslash");

    t.is (JSON2::decode (encoded), "one\\",   "JSON2::decode oneslashslashslashslashslashslashslashslash -> oneslashslashslashslash");
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
