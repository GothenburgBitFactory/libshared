////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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

#ifndef INCLUDED_JSON2
#define INCLUDED_JSON2

#include <string>

class JSON2
{
public:
  class SAX
  {
  public:
    virtual void eventDocStart () {}
    virtual void eventDocEnd () {}
    virtual void eventObjectStart () {}
    virtual void eventObjectEnd (int) {}
    virtual void eventArrayStart () {}
    virtual void eventArrayEnd (int) {}
    virtual void eventName (const std::string&) {}
    virtual void eventValueNull () {}
    virtual void eventValueBool (bool) {}
    virtual void eventValueInt (int64_t) {}
    virtual void eventValueUint (uint64_t) {}
    virtual void eventValueDouble (double) {}
    virtual void eventValueString (const std::string&) {}
  };

  bool parse (const std::string&, JSON2::SAX&);

  static std::string encode (const std::string&);
  static std::string decode (const std::string&);

private:
  void ignoreWhitespace (const std::string&, std::string::size_type&);
  bool isObject         (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isArray          (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isPair           (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isValue          (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isKey            (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isString         (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isStringValue    (const std::string&, std::string::size_type&, std::string&);
  bool isNumber         (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isInt            (const std::string&, std::string::size_type&, std::string&);
  bool isFrac           (const std::string&, std::string::size_type&, std::string&);
  bool isDigits         (const std::string&, std::string::size_type&);
  bool isDecDigit       (int);
  bool isHexDigit       (int);
  int  hexToInt         (int);
  int  hexToInt         (int, int, int, int);
  bool isExp            (const std::string&, std::string::size_type&, std::string&);
  bool isE              (const std::string&, std::string::size_type&);
  bool isBool           (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isNull           (const std::string&, std::string::size_type&, JSON2::SAX&);
  bool isLiteral        (const std::string&, char, std::string::size_type&);
  void error            (const std::string&, std::string::size_type);
};

#endif

////////////////////////////////////////////////////////////////////////////////

