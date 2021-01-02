////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2012 - 2021, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_ARGS
#define INCLUDED_ARGS

#include <string>
#include <vector>
#include <map>

class Args
{
public:
  Args () = default;

  void addOption (const std::string&, bool defaultValue = true);
  void addNamed  (const std::string&, const std::string& defaultValue = "");
  void limitPositionals (int);
  void enableNegatives ();

  void scan (int, const char**);

  bool getOption (const std::string&) const;
  int getOptionCount (const std::string&) const;
  std::string getNamed (const std::string&) const;
  int getPositionalCount () const;
  std::string getPositional (int) const;

  std::string dump () const;

private:
  bool canonicalizeOption (const std::string&, std::string&) const;
  bool canonicalizeNamed (const std::string&, std::string&) const;

private:
  std::map <std::string, bool>        _options     {};
  std::map <std::string, int>         _optionCount {};
  std::map <std::string, std::string> _named       {};
  std::vector <std::string>           _positionals {};
  int                                 _limit       {-1};
  bool                                _negatives   {false};
};

#endif

