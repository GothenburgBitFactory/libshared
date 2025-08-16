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
// https://opensource.org/license/mit
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_COLOR
#define INCLUDED_COLOR

#include <string>

#define _COLOR_24BIT     0x0000000080000000  // 24-bit mode.
#define _COLOR_INVERSE   0x0000000040000000  // Inverse attribute.
#define _COLOR_256       0x0000000020000000  // 256-color mode.
#define _COLOR_HASBG     0x0000000010000000  // Has background color (all values taken).
#define _COLOR_HASFG     0x0000000008000000  // Has foreground color (all values taken).
#define _COLOR_UNDERLINE 0x0000000004000000  // General underline attribute.
#define _COLOR_BOLD      0x0000000002000000  // 16-color bold attribute.
#define _COLOR_BRIGHT    0x0000000001000000  // 16-color bright background attribute.
#define _COLOR_BG        0x000000000000FF00  // 8-bit background color index.
#define _COLOR_FG        0x00000000000000FF  // 8-bit foreground color index.
#define _COLOR_24BIT_BG  0x00FFFFFF00000000  // 24-bit background color index.
#define _COLOR_24BIT_FG  0x0000000000FFFFFF  // 24-bit foreground color index.

class Color
{
public:
  enum color_id {nocolor = 0, black, red, green, yellow, blue, magenta, cyan, white};

  Color ();
  Color (const Color&) = default;
  Color& operator= (const Color&) = default;
  Color (unsigned long int);                    // 256 | INVERSE | UNDERLINE | BOLD | BRIGHT | (BG << 8) | FG
  Color (const std::string&);                   // "red on bright black"
  Color (color_id);                             // fg.
  Color (color_id, color_id, bool, bool, bool); // fg, bg, underline, bold, bright
  operator std::string () const;
  operator int () const;

  void upgrade ();
  void upgrade24b ();
  unsigned int index2truecolor(unsigned int);
  void blend (const Color&);

  std::string colorize (const std::string&) const;
  static std::string colorize (const std::string&, const std::string&);
  void _colorize (std::string&, const std::string&) const;
  static std::string strip (const std::string&);

  std::string code () const;
  std::string end () const;

  bool nontrivial () const;
  bool operator== (const Color&) const;

private:
  int find (const std::string&);
  std::string fg () const;
  std::string bg () const;

private:
  unsigned long int _value;
};

#endif
