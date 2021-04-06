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

#include <cmake.h>
#include <Timer.h>
#include <format.h>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (7);

  // Start this timer early to allow for non-trivial elapsed time.
  Timer t0;
  t0.start ();

  Timer t1;
  t1.stop ();
  t.ok (t1.total_s ()  >= 0.0, "Timer: possibliy non-zero s if not started");
  t.ok (t1.total_ms () >= 0.0, "Timer: possibliy non-zero ms if not started");
  t.ok (t1.total_us () >= 0.0, "Timer: possibliy non-zero us if not started");
  t.ok (t1.total_ns () >= 0.0, "Timer: possibliy non-zero ns if not started");

  t0.stop ();
  t.ok (t0.total_ms () >= t0.total_s (),  "Timer: more ms than s");
  t.ok (t0.total_us () >= t0.total_ms (), "Timer: more us than ms");
  t.ok (t0.total_ns () >= t0.total_us (), "Timer: more ns than us");

  t.diag (format ("Total {1} s",  t0.total_s ()));
  t.diag (format ("Total {1} ms", t0.total_ms ()));
  t.diag (format ("Total {1} μs", t0.total_us ()));
  t.diag (format ("Total {1} ns", t0.total_ns ()));

  Timer t2;
  t2.start ();
  t.diag (format ("Running total {1} s",  t2.total_s ()));
  t.diag (format ("Running total {1} ms", t2.total_ms ()));
  t.diag (format ("Running total {1} us", t2.total_us ()));
  t.diag (format ("Running total {1} ns", t2.total_ns ()));

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
