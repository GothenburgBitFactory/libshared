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

#ifndef INCLUDED_PACKRAT
#define INCLUDED_PACKRAT

#include <PEG.h>
#include <Tree.h>
#include <Pig.h>
#include <string>

class Packrat
{
public:
  void parse (const PEG&, const std::string&);
  void entity (const std::string&, const std::string&);
  void external (const std::string&, bool (*)(Pig&, std::shared_ptr <Tree>));

  void debug ();
  std::string dump () const;

private:
  bool matchRule           (const std::string&,     Pig&, const std::shared_ptr <Tree>&, int);
  bool matchProduction     (const PEG::Production&, Pig&, const std::shared_ptr <Tree>&, int);
  bool matchTokenQuant     (const PEG::Token&,      Pig&, const std::shared_ptr <Tree>&, int);
  bool matchTokenLookahead (const PEG::Token&,      Pig&, const std::shared_ptr <Tree>&, int);
  bool matchToken          (const PEG::Token&,      Pig&, const std::shared_ptr <Tree>&, int);
  bool matchIntrinsic      (const PEG::Token&,      Pig&, const std::shared_ptr <Tree>&, int);
  bool matchCharLiteral    (const PEG::Token&,      Pig&, const std::shared_ptr <Tree>&, int);
  bool matchStringLiteral  (const PEG::Token&,      Pig&, const std::shared_ptr <Tree>&, int);

  bool canonicalize (std::string&, const std::string&, const std::string&) const;

public:
  static int minimumMatchLength;

private:
  int                                                            _debug    {0};
  std::map <std::string, PEG::Rule>                              _syntax   {};
  std::shared_ptr <Tree>                                         _tree     {std::make_shared <Tree> ()};
  std::multimap <std::string, std::string>                       _entities {};
  std::map <std::string, bool (*)(Pig&, std::shared_ptr <Tree>)> _externals {};
};

#endif

////////////////////////////////////////////////////////////////////////////////
