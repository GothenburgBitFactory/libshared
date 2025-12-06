////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 - 2017, 2019 - 2021, 2023, Gothenburg Bit Factory.
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

#include <Table.h>
#include <iostream>
#include <test.h>

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (3);

  try
  {
    // Create colors.
    Color header_color (Color (Color::yellow, Color::nocolor, false, false, false));
    Color odd_color ("on gray1");
    Color even_color ("on gray0");

    // Now render a string-only grid.
    Color single_cell ("bold white on red");

    Table t1;
    t1.width (80);
    t1.leftMargin (4);
    t1.extraPadding (0);
    t1.intraPadding (1);
    t1.colorHeader (header_color);
    t1.colorOdd (odd_color);
    t1.colorEven (even_color);
    t1.intraColorOdd (odd_color);
    t1.intraColorEven (even_color);

    t1.add ("Header1", true);
    t1.add ("Header2", true);
    t1.add ("Header3", false);

    int row = t1.addRow ();
    t1.set (row, 0, "top left");
    t1.set (row, 1, "top center");
    t1.set (row, 2, "top right");

    row = t1.addRow ();
    t1.set (row, 0, "bottom left", single_cell);
    t1.set (row, 1, "bottom center, containing sufficient text that "
                             "wrapping will occur because it exceeds all "
                             "reasonable values for default width.  Even in a "
                             "very wide terminal window.  Just look at the "
                             "lengths we must go to, to get passing unit tests "
                             "and not flaky tests.");
    t1.set (row, 2, "bottom right");

    std::cout << t1.render ();
    t.ok (t1.lines () > 4, "Table::lines > 4");
    
    // Test color behavior - with color enabled (default)
    Table t3;
    t3.width (80);
    t3.add ("Header", true);
    row = t3.addRow ();
    t3.set (row, 0, "cell with color", single_cell);
    std::string colored_output = t3.render ();
    t.ok (colored_output.find("\033[") != std::string::npos, "Colored output contains ANSI escape codes");
    
    // Test color behavior - with color disabled
    Table t4;
    t4.width (80);
    t4.withColor (false);  // Disable color
    t4.add ("Header", true);
    row = t4.addRow ();
    t4.set (row, 0, "cell without color", single_cell);
    std::string uncolored_output = t4.render ();
    t.ok (uncolored_output.find("\033[") == std::string::npos, "Uncolored output does not contain ANSI escape codes");

    // Chessboard example
    Table t2;
    t2.width (32);
    t2.leftMargin (4);
    t2.extraPadding (0);
    t2.intraPadding (0);

    t2.add ("", true);
    t2.add ("", true);
    t2.add ("", true);
    t2.add ("", true);
    t2.add ("", true);
    t2.add ("", true);
    t2.add ("", true);
    t2.add ("", true);

    Color blue ("on bright blue");
    Color white ("on bright white");

    for (row = 0; row < 8; ++row)
    {
      t2.addRow ();

      for (int col = 0; col < 8; ++col)
      {
        if ((row + col) % 2)
          t2.set (row, col, "  ", blue);
        else
          t2.set (row, col, "  ", white);
      }
    }

    std::cout << t2.render ();
  }

  catch (const std::string& e)
  {
    t.fail ("Exception thrown.");
    t.diag (e);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
