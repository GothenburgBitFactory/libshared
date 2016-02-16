////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2016, Göteborg Bit Factory.
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
#include <Duration.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (12);

  Duration dur;
  std::string::size_type start = 0;
  t.notok (dur.parse ("foo", start), "foo --> false");
  t.is ((int)start, 0,               "foo[0]");

  t.notok (dur.parse ("P", start),   "P --> false");
  t.is ((int)start, 0,               "P[0]");

  t.notok (dur.parse ("PT", start),  "PT --> false");
  t.is ((int)start, 0,               "PT[0]");

  t.notok (dur.parse ("P1", start),  "P1 --> false");
  t.is ((int)start, 0,               "P1[0]");

  t.notok (dur.parse ("P1T", start), "P1T --> false");
  t.is ((int)start, 0,               "P1T[0]");

  t.notok (dur.parse ("PT1", start), "PT1 --> false");
  t.is ((int)start, 0,               "PT1[0]");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
