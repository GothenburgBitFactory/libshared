////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 - 2021, 2023, 2026 Gothenburg Bit Factory.
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

#include <Composite.h>
#include <format.h>
#include <limits>
#include <sstream>
#include <utf8.h>


////////////////////////////////////////////////////////////////////////////////

namespace
{

  // Helper function that either replaces a pre-existing element at index (i) in
  // a std::vector with the value (x) (if (i) is less than the size of the vector)
  // or extends the vector in such a way that it ends up with (i+1) elements, with
  // the value (x) at index (i) and the padding value (pad) at each index between
  // that of the final pre-existing element of the vector and (i).
  template <typename T>
  void put_or_extend (
    std::vector<T>& v, typename std::vector<T>::size_type i, const T& x, const T& pad = T {})
  {
    if (i < v.size ())
      v[i] = x;
    else
    {
      v.resize (i, pad);
      v.push_back (x);
    }
  }

  // Helper class that is used to store information about columns in a Composite.
  struct ColumnData
  {
    // Number of topmost layer that overlaps with the column represented by this ColumnData.
    // NOTE: Layer numbers start at 1. "Layer 0" is background not covered by any layer.
    unsigned int layer_num;

    // Byte offset into the UTF-8 text string of the layer identified by (layer_num).
    // Points to the first byte of the first character to include in the content
    // of the column represented by this ColumnData.
    std::string::size_type text_begin_i;

    // Byte offset into the UTF-8 text string of the layer identified by (layer_num).
    // Points to the first byte after the last character to include in the content
    // of the column represented by this ColumnData.
    std::string::size_type text_end_i;

    // Unicode display width of the first character to include in the content
    // of the column represented by this ColumnData. Should always be 1 or 2,
    // unless this ColumnData represents a padding column.
    unsigned char char_0_width;

    ColumnData (
      unsigned int layer = 0, std::string::size_type begin_i = 1, std::string::size_type end_i = 0,
      unsigned char c_0_w = 0)
    :
      layer_num (layer), text_begin_i (begin_i), text_end_i (end_i), char_0_width (c_0_w)
    {}

    ColumnData (const ColumnData& orig) = default;

    ColumnData& operator= (const ColumnData& orig) = default;

    std::string::difference_type byte_count () const
    {
      return text_end_i - text_begin_i;
    }

    // Changes the state of this ColumnData to one that indicates that the ColumnData
    // represents a padding column (i.e. a state where byte_count is negative).
    void make_padding ()
    {
      text_begin_i = 1;
      text_end_i = 0;
      char_0_width = 0;
    }

    bool is_padding () const
    {
      return byte_count () < 0;
    }
  };

  const ColumnData LAYER_0_PAD;  // ColumnData representing a padding column on "layer 0".

  // Special column index value, distinct from any valid column index.
  const std::string::size_type INVALID_COLUMN_I = std::numeric_limits<std::string::size_type>::max ();

  // Helper function that turns the uncovered half of half-covered wide characters into padding.
  inline void do_halfcovered_wide_char_check (
    std::vector<ColumnData>& columns, std::vector<ColumnData>::size_type column_i)
  {
    // If there is a wide character (on a lower layer) in the preceding column, replace
    // that character (and any nonspacing characters associated with it) with padding.
    // (Because the second half of that character will be covered, and we couldn't display
    // half a character if we wanted to.)
    if (column_i >= 1 && column_i - 1 < columns.size ())
    {
      ColumnData& prev_col_data = columns[column_i - 1];
      if (prev_col_data.char_0_width == 2)
        prev_col_data.make_padding ();
    }
  }

};

////////////////////////////////////////////////////////////////////////////////
// Initially assume no text, but infinite virtual space.
//
// Allow overlay placement of arbitrary text at any offset, real or virtual, and
// using a specific color.
//
// For example:
//   Composite c;
//   c.add ("aaaaaaaaaa",  2, Color ("..."));    // Layer 1
//   c.add ("bbbbb",       5, Color ("..."));    // Layer 2
//   c.add ("c",          15, Color ("..."));    // Layer 3
//
//   _layers = { std::make_tuple ("aaaaaaaaaa",  2, Color ("...")),
//               std::make_tuple ("bbbbb",       5, Color ("...")),
//               std::make_tuple ("c",          15, Color ("..."))};
//
void Composite::add (
  const std::string& text,
  std::string::size_type offset,
  const Color& color)
{
  _layers.emplace_back (text, offset, color);
}

////////////////////////////////////////////////////////////////////////////////
// Merge the layers of text and color into one string.
//
// For example:
//   Composite c;
//   c.add ("aaaaaaaaaa",  2, Color ("..."));    // Layer 1
//   c.add ("bbbbb",       5, Color ("..."));    // Layer 2
//   c.add ("c",          15, Color ("..."));    // Layer 3
//
//   _layers = { std::make_tuple ("aaaaaaaaaa",  2, Color ("...")),
//               std::make_tuple ("bbbbb",       5, Color ("...")),
//               std::make_tuple ("c",          15, Color ("..."))};
//
// Arrange strings conceptually:
//              111111
//    0123456789012345     // Position
//
//      aaaaaaaaaa         // Layer 1
//         bbbbb           // Layer 2
//                   c     // Layer 3
//
// Walk all layers left to right, selecting the character and color from the
// highest numbered layer. Emit color codes only on edge detection.
//
std::string Composite::str () const
{
  std::vector <ColumnData> columns;

  for (unsigned int layer_i = 0; layer_i < _layers.size (); ++layer_i)
  {
    const auto& text = std::get <0> (_layers[layer_i]);
    auto offset = std::get <1> (_layers[layer_i]);
    auto len = utf8_text_length (text);

    // Make sure the capacity of the column vector is large enough to support push_back()
    // without reallocation.
    if (columns.capacity () < offset + len)
      columns.reserve (offset + len);

    // Inspect and decide how to handle each character (i.e. Unicode code point)
    // in the current layer's text string.
    std::string::size_type prev_cursor = 0;
    std::string::size_type cursor = 0;
    unsigned int column_count = 0;
    std::string::size_type prev_spacer_column_i = INVALID_COLUMN_I;
    unsigned int character;
    while ((character = utf8_next_char (text, cursor)))
    {
      std::string::size_type column_i = offset + column_count;
      int ch_width = mk_wcwidth ((wchar_t)character);

      switch (ch_width)
      {
      case 0:  // zero-width / nonspacing character
        if (prev_spacer_column_i == INVALID_COLUMN_I)  // No preceding spacing character on this layer.
          ;  // Skip this character.
        else  // There is a preceding spacing character on this layer.
        {
          // Append the nonspacing character to the column of the previous spacing character.
          columns[prev_spacer_column_i].text_end_i = cursor;
        }
        break;
      case 1:  // ordinary narrow spacing character
        if (prev_spacer_column_i == INVALID_COLUMN_I)
          do_halfcovered_wide_char_check (columns, column_i);

        // Put the character in the appropriate column. Pad out the column list as necessary.
        put_or_extend (columns, column_i, ColumnData (layer_i + 1, prev_cursor, cursor, 1), LAYER_0_PAD);

        prev_spacer_column_i = column_i;
        column_count += 1;
        break;
      case 2:  // graphically wide spacing character
        if (prev_spacer_column_i == INVALID_COLUMN_I)
          do_halfcovered_wide_char_check (columns, column_i);

        // Put the character in the appropriate column. Pad out the column list as necessary.
        // Make the column after the current one (which is also covered by the wide character)
        // a padding column on the current layer.
        put_or_extend (columns, column_i, ColumnData (layer_i + 1, prev_cursor, cursor, 2), LAYER_0_PAD);
        put_or_extend (columns, column_i + 1, ColumnData (layer_i + 1), LAYER_0_PAD);

        prev_spacer_column_i = column_i;
        column_count += 2;
        break;
      default:  // Should not happen.
        throw format ("Unexpected character width {1} of code point 0x{2}.", ch_width, formatHex (character));
      }

      // Remember byte offset of first UTF-8 byte of next character in the layer text.
      prev_cursor = cursor;
    }
  }

  // Now walk the column vector, emitting every character and every detected layer change.
  std::stringstream out;
  unsigned int prev_layer = 0;
  for (unsigned int column_i = 0; column_i < columns.size (); ++column_i)
  {
    auto column_data = columns[column_i];
    auto curr_layer = column_data.layer_num;
    const auto& text = std::get <0> (_layers[curr_layer - 1]);

    // A change in layer triggers an ANSI escape code emit.
    if (prev_layer != curr_layer)
    {
      if (prev_layer)  // Reset attributes (if any) of previous layer.
        out << std::get <2> (_layers[prev_layer - 1]).end ();

      if (curr_layer) // Set attributes (if any) of current layer.
        out << std::get <2> (_layers[curr_layer - 1]).code ();

      prev_layer = curr_layer;
    }

    // The layer text string is already UTF-8, so we can output its bytes verbatim,
    // provided that we're keeping track of character (i.e. code point) boundaries.
    if (column_data.is_padding ())
      out << ' ';  // Display padding columns as spaces.
    else  // Display a slice of the layer text (Spacer [Nonspacer ...]).
      out.write(text.data () + column_data.text_begin_i, column_data.byte_count ());

    if (column_data.char_0_width == 2)
      ++column_i;  // Wide characters cover two columns.
  }

  // Terminate the color codes, if necessary.
  if (prev_layer)
    out << std::get <2> (_layers[prev_layer - 1]).end ();

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
// So the same instance can be reused.
void Composite::clear ()
{
  _layers.clear ();
}

////////////////////////////////////////////////////////////////////////////////
