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

#ifndef INCLUDED_TABLE
#define INCLUDED_TABLE

#include <string>
#include <vector>
#include <Color.h>

class Table
{
public:
  // View specifications.
  void add (const std::string& col, bool alignLeft = true, bool wrap = true);
  void width (int width)               { _width = width;             }
  void leftMargin (int margin)         { _left_margin = margin;      }
  void colorHeader (const Color& c)    { _header = c;                }
  void colorOdd (const Color& c)       { _odd = c;                   }
  void colorEven (const Color& c)      { _even = c;                  }
  void intraPadding (int padding)      { _intra_padding = padding;   }
  void intraColorOdd (const Color& c)  { _intra_odd = c;             }
  void intraColorEven (const Color& c) { _intra_even = c;            }
  void extraPadding (int padding)      { _extra_padding = padding;   }
  void extraColorOdd (const Color& c)  { _extra_odd = c;             }
  void extraColorEven (const Color& c) { _extra_even = c;            }
  void truncateLines (int n)           { _truncate_lines = n;        }
  void truncateRows (int n)            { _truncate_rows = n;         }
  void forceColor ()                   { _forceColor = true;         }
  void obfuscate ()                    { _obfuscate = true;          }
  void underlineHeaders ()             { _underline_headers = true;  }
  int lines ()                         { return _lines;              }
  int rows ()                          { return (int) _data.size (); }

  // Data provision.
  int addRow ();
  int addRowOdd ();
  int addRowEven ();
  void set (int, int, const std::string&, const Color color = Color::nocolor);
  void set (int, int, int, const Color color = Color::nocolor);
  void set (int, int, const Color);

  // View rendering.
  std::string render ();

private:
  void measureCell (const std::string&, unsigned int&, unsigned int&) const;
  void renderCell (std::vector <std::string>&, const std::string&, int, bool, bool, const Color&) const;

private:
  std::vector <std::vector <std::string>> _data;
  std::vector <std::vector <Color>>       _color;
  std::vector <std::string>               _columns;
  std::vector <bool>                      _align;
  std::vector <bool>                      _wrap;
  std::vector <bool>                      _oddness;
  int                                     _width             {0};
  int                                     _left_margin       {0};
  Color                                   _header            {0};
  Color                                   _odd               {0};
  Color                                   _even              {0};
  int                                     _intra_padding     {1};
  Color                                   _intra_odd         {0};
  Color                                   _intra_even        {0};
  int                                     _extra_padding     {0};
  Color                                   _extra_odd         {0};
  Color                                   _extra_even        {0};
  int                                     _truncate_lines    {0};
  int                                     _truncate_rows     {0};
  bool                                    _forceColor        {false};
  bool                                    _obfuscate         {false};
  bool                                    _underline_headers {false};
  int                                     _lines             {0};
  int                                     _rows              {0};
};

#endif
