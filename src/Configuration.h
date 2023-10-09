////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 - 2017, 2019 - 2021, 2023, Gothenburg Bit Factory.
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

#ifndef INCLUDED_CONFIGURATION
#define INCLUDED_CONFIGURATION

#include <FS.h>
#include <map>
#include <string>
#include <vector>

bool setVariableInFile   (const std::string&, const std::string&, const std::string&);
bool unsetVariableInFile (const std::string&, const std::string&);

class Configuration : public std::map <std::string, std::string>
{
public:
  void load (
      const std::string& file_path,
      int nest = 1,
      const std::vector <std::string>& search_paths = {});
  void save ();
  void parse (
    const std::string& content,
    int nest = 1,
    const std::vector <std::string>& search_paths = {},
    const std::string& file_path = {});

  bool        has            (const std::string&) const;
  std::string get            (const std::string&, bool getFromContext = true) const;
  int         getInteger     (const std::string&, bool getFromContext = true) const;
  double      getReal        (const std::string&, bool getFromContext = true) const;
  bool        getBoolean     (const std::string&, bool getFromContext = true) const;

  void set (const std::string&, const int);
  void set (const std::string&, const double);
  void set (const std::string&, const std::string&);
  void setIfBlank (const std::string&, const std::string&);
  std::vector <std::string> all () const;

  std::string file () const;

  bool dirty ();

private:
  File _original_file {};
  bool _dirty         {false};
};

#endif
