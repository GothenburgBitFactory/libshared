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
#include <iostream>
#include <fstream>
#include <sstream>
#include <JSON.h>

class EventSink : public json::SAX::Sink
{
public:
  void eventObjectStart () override                         { std::cout << "# object start\n";                                 }
  void eventObjectEnd (int count) override                  { std::cout << "# object end (" << count << " items)\n";           }
  void eventArrayStart () override                          { std::cout << "# array start\n";                                  }
  void eventArrayEnd (int count) override                   { std::cout << "# array end (" << count << " items)\n";            }
  void eventName (const std::string& value) override        { std::cout << "# name '" << value << "'\n";                       }
  void eventValueNull () override                           { std::cout << "# value 'null'\n";                                 }
  void eventValueBool (bool value) override                 { std::cout << "# value '" << (value ? "true" : "false") << "'\n"; }
  void eventValueInt (int64_t value) override               { std::cout << "# value '" << value << "'\n";                      }
  void eventValueUint (uint64_t value) override             { std::cout << "# value '" << value << "'\n";                      }
  void eventValueDouble (double value) override             { std::cout << "# value '" << value << "'\n";                      }
  void eventValueString (const std::string& value) override { std::cout << "# value '" << value << "'\n";                      }
};

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
  int status = 0;
  if (argc == 1)
  {
    std::cout << "\nUsage: json2_test <file> ...\n"
              << '\n'
              << "      <file>    file containing JSON\n"
              << '\n';
  }
  else
  {
    for (int i = 1; i < argc; ++i)
    {
      try
      {
        std::ifstream inputFile (argv[i]);
        std::stringstream input;
        input << inputFile.rdbuf ();

        EventSink sink;
        json::SAX sax;
        if (! sax.parse (input.str (), sink))
          status = 1;
      }

      catch (const std::string& e) { std::cout << e << '\n';         }
      catch (...)                  { std::cout << "Unknown error\n"; }
    }
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////
