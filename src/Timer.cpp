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
#include <Timer.h>

////////////////////////////////////////////////////////////////////////////////
Timer::Timer ()
{
  start ();
}

////////////////////////////////////////////////////////////////////////////////
void Timer::start ()
{
  _start = std::chrono::high_resolution_clock::now ();
  _end = {};
  _running = true;
}

////////////////////////////////////////////////////////////////////////////////
void Timer::stop ()
{
  if (_running)
  {
    _end = std::chrono::high_resolution_clock::now ();
    _running = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
double Timer::total_s () const
{
  auto endpoint = _end;
  if (_running)
    endpoint = std::chrono::high_resolution_clock::now ();

  return std::chrono::duration_cast<std::chrono::seconds>(endpoint - _start).count();
}

////////////////////////////////////////////////////////////////////////////////
double Timer::total_ms () const
{
  auto endpoint = _end;
  if (_running)
    endpoint = std::chrono::high_resolution_clock::now ();

  return std::chrono::duration_cast<std::chrono::milliseconds>(endpoint - _start).count();
}

////////////////////////////////////////////////////////////////////////////////
double Timer::total_us () const
{
  auto endpoint = _end;
  if (_running)
    endpoint = std::chrono::high_resolution_clock::now ();

  return std::chrono::duration_cast<std::chrono::microseconds>(endpoint - _start).count();
}

////////////////////////////////////////////////////////////////////////////////
double Timer::total_ns () const
{
  auto endpoint = _end;
  if (_running)
    endpoint = std::chrono::high_resolution_clock::now ();

  return std::chrono::duration_cast<std::chrono::nanoseconds>(endpoint - _start).count();
}

////////////////////////////////////////////////////////////////////////////////
