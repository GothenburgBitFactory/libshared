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

#ifndef INCLUDED_PEG
#define INCLUDED_PEG

#include <FS.h>
#include <string>
#include <vector>
#include <map>
#include <set>

class PEG
{
public:
  class Token
  {
  public:
    Token (const std::string& value)           { _token = value; }
    void tag (const std::string& tag)          { _tags.insert (tag); }
    bool hasTag (const std::string& tag) const { return _tags.find (tag) != _tags.end (); };
    std::string dump () const;

    enum class Quantifier                      { one, zero_or_one, one_or_more, zero_or_more };
    enum class Lookahead                       { none, positive, negative };

    std::string _token                         {};
    std::set <std::string> _tags               {};
    Quantifier  _quantifier                    {Quantifier::one};
    Lookahead   _lookahead                     {Lookahead::none};
    // TODO Added Lexer::Type support, which allows the PEG to specify
    //      "<Lexer::Type>" as a built-in type.
  };

  class Production : public std::vector <Token>
  {
  };

  class Rule : public std::vector <Production>
  {
  };

public:
  static std::string removeComment (const std::string&);

public:
  void loadFromFile (File&);
  void loadFromString (const std::string&);
  std::map <std::string, PEG::Rule> syntax () const;
  std::string firstRule () const;
  void debug ();
  void strict (bool);
  std::string dump () const;

private:
  std::vector <std::string> loadImports (const std::vector <std::string>&);
  void validate () const;

private:
  //        rule name    rule
  //        |            |
  std::map <std::string, PEG::Rule> _rules     {};
  std::string                       _start     {};
  int                               _debug     {0};
  bool                              _strict    {false};
  std::vector <std::string>         _imports   {};
};

#endif
