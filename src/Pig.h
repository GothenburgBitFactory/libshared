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

#ifndef INCLUDED_PIG
#define INCLUDED_PIG

#include <string>
#include <vector>

class Pig
{
public:
  explicit Pig (const std::string&);

  bool skip (int);
  bool skipN (const int quantity = 1);
  bool skipWS ();
  bool skipLiteral (const std::string&);

  bool getUntilWS (std::string&);
  bool getDigit (int&);
  bool getDigit2 (int&);
  bool getDigit3 (int&);
  bool getDigit4 (int&);
  bool getDigits (int&);
  bool getHexDigit (int&);
  bool getNumber (std::string&);
  bool getNumber (double&);
  bool getQuoted (int, std::string&);
  bool getOneOf (const std::vector <std::string>&, std::string&);
  bool getRemainder (std::string&);

  bool eos () const;
  int peek () const;
  std::string peek (const int) const;
  std::string::size_type cursor () const;
  std::string::size_type save ();
  std::string::size_type restore ();

  std::string dump () const;

private:
  const std::string&     _text;
  std::string::size_type _cursor {0};
  std::string::size_type _saved  {0};
};

#endif

////////////////////////////////////////////////////////////////////////////////
