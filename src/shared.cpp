////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 - 2017, 2019 - 2021, 2023, 2026, Gothenburg Bit Factory.
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

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmake.h>
#include <cmath>
#include <csignal>
#include <cstring>
#include <format.h>
#include <iostream>
#include <shared.h>
#include <sstream>
#ifndef _WIN32
#include <strings.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#endif
#include <utf8.h>
#ifdef _WIN32
#include <windows.h>
#endif

static const int MAX_CHAR_DISPLAY_WIDTH = 2;
static const std::string DEFAULT_SURROGATE_STR = ".";
// NOTE: HYPHENATION_STR should be a UTF-8 string with total display width exactly 1.
static const std::string HYPHENATION_STR = "-";

// Replaces a Unicode character in UTF-8 encoding at byte index (i) in the byte string (input)
// with the byte string (subst). Returns the byte index of the character following the substituted
// one, after the substitution.
static unsigned int utf8ReplaceChar (
  std::string& input, std::string::size_type& i, const std::string& subst)
{
  std::string::size_type old_i = i;

  int ch = utf8_next_char (input, i);
  if (ch == 0)
    return 0;

  input.replace (old_i, i - old_i, subst);
  i = old_i + subst.size ();  // byte index of next code point, post-substitution

  return ch;
}

// Gets the first-byte indexes of all Unicode characters in the byte string (s), which is assumed
// to contain a valid UTF-8 string. Stores the byte indexes in the vector (out).
static void getUtf8CharacterByteIndexes (const std::string& s, std::vector<std::string::size_type>& out)
{
  std::string::size_type byte_i = 0;

  while (byte_i < s.size ())
  {
    out.push_back (byte_i);
    utf8_next_char (s, byte_i);
  }
}

// Gets the substring of the byte string (s) that starts at index (begin) and ends at index (end).
static std::string rangeSubstr (const std::string& s, std::string::size_type begin, std::string::size_type end)
{
  return s.substr (begin, end - begin);
}

// Returns true if and only if the byte string (text), which is assumed to contain a valid
// UTF-8 string, contains a Unicode character with display width greater than (width).
static bool containsTooWideChars (const std::string& text, int width)
{
  std::string::size_type byte_i = 0;

  while (byte_i < text.size ())
  {
    int ch = utf8_next_char (text, byte_i);

    if (mk_wcwidth (ch) > width)
      return true;
  }

  return false;
}

// Replaces all Unicode characters with display width greater than (width) in the byte string (text),
// which is assumed to contain a valid UTF-8 string, with the byte string (surrogate).
static void replaceTooWideChars (std::string& text, int width, const std::string& surrogate)
{
  std::string::size_type byte_i = 0;

  while (byte_i < text.size ())
  {
    std::string::size_type old_byte_i = byte_i;
    int ch = utf8_next_char (text, byte_i);

    if (mk_wcwidth (ch) > width)
    {
      byte_i = old_byte_i;  // Rewind (byte_i) by one code point.
      utf8ReplaceChar (text, byte_i, surrogate);
    }
  }
}

// Fetches a Unicode character (i.e. code point) at position (ch_index) in the code point
// sequence from the byte string (text), which is assumed to contain a valid UTF-8 string.
// Also determines and outputs the display width of the fetched character. The vector
// (ch_byte_indexes) is assumed to contain the first-byte indexes of all Unicode characters
// in (text).
static void getUtf8CharAt (
  const std::string& s, const std::vector<std::string::size_type>& ch_byte_indexes,
  unsigned int ch_index, unsigned int& ch, int& ch_width)
{
  std::string::size_type ch_byte_index = ch_byte_indexes[ch_index];
  ch = utf8_next_char (s, ch_byte_index);
  ch_width = mk_wcwidth (ch);
}

static bool extractLine (
  std::string& line,
  const std::string& text,
  const std::vector<std::string::size_type>& ch_byte_indexes,
  int width,
  bool hyphenate,
  unsigned int& ch_index)
{
  auto text_len_utf8 = ch_byte_indexes.size ();  // length of (text) in Unicode characters

  if (ch_index >= text_len_utf8)  // Already at end of string, return false.
    return false;

  // code point index (CPI) of first (non-whitespace) character on current line
  unsigned int line_start_ch_i = ch_index;
  unsigned int prev_word_end_ch_i = text_len_utf8;  // CPI of most recently encountered word ending
  unsigned int prev_ws_start_ch_i = text_len_utf8;  // CPI of start of ongoing run of whitespace
  // CPIs of the two positive-width characters most recently added to the output line,
  // relevant for hyphenation
  unsigned int prev_pos_w_ch_i = text_len_utf8;
  unsigned int prev_prev_pos_w_ch_i = text_len_utf8;
  unsigned int ch_i = line_start_ch_i;  // CPI of current character
  unsigned int ch = 0;  // Unicode code point of current character
  int line_width = 0;  // accumulated display width of current line

  while (ch_i < text_len_utf8)
  {
    // Step 0: Fetch the current Unicode character and determine its display width.
    unsigned int prev_ch = ch;
    int ch_width;
    getUtf8CharAt (text, ch_byte_indexes, ch_i, ch, ch_width);

    // Step 1: Inspect the current character and update cursor variables.
    if (ch == '\0' || ch == '\n')  // mandatory line break
    {
      // Strip any run of whitespace at end of line.
      unsigned int line_end_ch_i = (prev_ws_start_ch_i < text_len_utf8) ? prev_ws_start_ch_i : ch_i;
      line = rangeSubstr (text, ch_byte_indexes[line_start_ch_i], ch_byte_indexes[line_end_ch_i]);
      ch_index = ch_i + 1U;  // Do not include the line break character in any line.
      return true;
    }
    else if (ch == ' ')  // whitespace
    {
      if (ch_i > line_start_ch_i && prev_ch != ' ')  // Detect word endings.
        prev_word_end_ch_i = ch_i;  // word ending

      if (ch_i == line_start_ch_i || prev_ch != ' ')  // Detect runs of whitespace.
        prev_ws_start_ch_i = ch_i;  // start of run of whitespace
    }

    // Step 2: Consider whether the current character fits on the current line.
    // NOTE: Characters that are wider than the maximum line width should have been fixed
    // in preprocessing. The fallback behavior is to pretend that the character fits on
    // a line by itself, even though it doesn't.
    if (ch_width > width)  // Character doesn't fit on a line by itself.
      ch_width = width;

    if (line_width + ch_width <= width)  // Line not full, include current character in line.
    {
      if (ch_width > 0)
      {
        prev_prev_pos_w_ch_i = prev_pos_w_ch_i;
        prev_pos_w_ch_i = ch_i;  // positive-width character added to line
      }

      if (ch != ' ')  // non-whitespace character added to line
        prev_ws_start_ch_i = text_len_utf8;  // run of whitespace ended here

      line_width += ch_width;
      ch_i++;
      continue;  // Done with current character, continue with the next.
    }

    // Step 3: Insert an appropriate line break, perhaps with hyphenation.
    if (prev_word_end_ch_i < text_len_utf8)  // Line full, break at previous word ending.
    {
      line = rangeSubstr (text, ch_byte_indexes[line_start_ch_i], ch_byte_indexes[prev_word_end_ch_i]);
      ch_index = prev_word_end_ch_i + 1U;  // Start next line after previous word ending.
    }
    else if (prev_ws_start_ch_i < text_len_utf8) {  // Line full but all whitespace, strip that out.
      line = "";  // Output empty line.
      ch_index = ch_i;  // Start next line at current character.
    }
    else if (hyphenate)  // Line full, no word ending available, hyphenation enabled.
    {
      // NOTE: There might be enough space left for a hyphen even if there's not enough
      // for the next character.
      bool hyphenation_failure = false;
      unsigned int hyphen_ch_i = (line_width < width) ? ch_i : prev_pos_w_ch_i;
      unsigned int hyphen_ch;
      int hyphen_ch_width;
      getUtf8CharAt (text, ch_byte_indexes, hyphen_ch_i, hyphen_ch, hyphen_ch_width);

      if (hyphen_ch_i == ch_i || line_width - hyphen_ch_width > 0) {
        // ISSUE: Some cumbersome logic to detect initial whitespace followed by a single
        // non-whitespace positive-width character.
        if (hyphen_ch_i == prev_pos_w_ch_i)
        {
          getUtf8CharAt (text, ch_byte_indexes, prev_prev_pos_w_ch_i, hyphen_ch, hyphen_ch_width);
          if (hyphen_ch == ' ')
            hyphenation_failure = true;  // Do not hyphenate whitespace.
        }
      }
      else
        hyphenation_failure = true;  // No room for hyphen.

      if (hyphenation_failure)  // Could't hyphenate here.
      {
        line = rangeSubstr (text, ch_byte_indexes[line_start_ch_i], ch_byte_indexes[ch_i]);
        ch_index = ch_i;  // Start next line at current character.
      }
      else  // Hyphenated line has positive width (and is not all whitespace), go ahead and hyphenate.
      {
        line = rangeSubstr (text, ch_byte_indexes[line_start_ch_i], ch_byte_indexes[hyphen_ch_i]);
        line += HYPHENATION_STR;
        ch_index = hyphen_ch_i;  // Start next line at character that was dropped to fit the hyphen.
      }
    }
    else  // Line full, no word ending available, hyphenation disabled.
    {
      line = rangeSubstr (text, ch_byte_indexes[line_start_ch_i], ch_byte_indexes[ch_i]);
      ch_index = ch_i;  // Start next line at current character.
    }

    return true;  // NOTE: If we reach this point, we have filled a line and should return true.
  }

  ch_index = text_len_utf8;  // Reached end of input string.

  if (line_start_ch_i < text_len_utf8)  // Include the last line.
  {
    // Strip any run of whitespace at end of line.
    std::string::size_type line_end_byte_i =
      (prev_ws_start_ch_i < text_len_utf8) ? ch_byte_indexes[prev_ws_start_ch_i] : text.size ();
    line = rangeSubstr (text, ch_byte_indexes[line_start_ch_i], line_end_byte_i);
    return true;
  }

  return false;  // Last line empty, return false.
}

///////////////////////////////////////////////////////////////////////////////
void wrapText (
  std::vector <std::string>& lines,
  const std::string& text,
  const int width,
  bool hyphenate)
{
  // Pre-process the input string.
  const std::string* text_ptr = &text;
  std::string text_fixed;

  if (width < MAX_CHAR_DISPLAY_WIDTH && containsTooWideChars (text, width))
  {
    text_fixed = text;
    replaceTooWideChars (text_fixed, width, DEFAULT_SURROGATE_STR);
    text_ptr = &text_fixed;
  }

  std::vector<std::string::size_type> ch_byte_indexes;
  getUtf8CharacterByteIndexes (*text_ptr, ch_byte_indexes);

  // Extract lines of wrapped text.
  std::string line;
  unsigned int ch_index = 0;
  while (extractLine (line, *text_ptr, ch_byte_indexes, width, hyphenate, ch_index))
    lines.push_back (line);
}

////////////////////////////////////////////////////////////////////////////////
// Split in a separator. Two adjacent separators means empty token.
std::vector <std::string> split (const std::string& input, const char delimiter)
{
  std::vector <std::string> results;
  std::string::size_type start = 0;
  std::string::size_type i;
  while ((i = input.find (delimiter, start)) != std::string::npos)
  {
    results.push_back (input.substr (start, i - start));
    start = i + 1;
  }

  if (!input.empty ())
    results.push_back (input.substr (start));

  return results;
}

////////////////////////////////////////////////////////////////////////////////
// Split on words. Adjacent separators collapsed.
std::vector <std::string> split (const std::string& input)
{
  static std::string delims = " \t\n\f\r";
  std::vector <std::string> results;

  std::string::size_type start = 0;
  std::string::size_type end;
  while ((start = input.find_first_not_of (delims, start)) != std::string::npos)
  {
    if ((end = input.find_first_of (delims, start)) != std::string::npos)
    {
      results.push_back (input.substr (start, end - start));
      start = end;
    }
    else
    {
      results.push_back (input.substr (start));
      start = std::string::npos;
    }
  }

  return results;
}

////////////////////////////////////////////////////////////////////////////////
std::string join (
  const std::string& separator,
  const std::vector<int>& items)
{
  std::stringstream s;
  auto size = items.size ();
  for (unsigned int i = 0; i < size; ++i)
  {
    if (i)
      s << separator;

    s << items[i];
  }

  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string join (
  const std::string& separator,
  const std::vector<std::string>& items)
{
  std::stringstream s;
  auto size = items.size ();
  for (unsigned int i = 0; i < size; ++i)
  {
    if (i)
      s << separator;

    s << items[i];
  }

  return s.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::string str_replace (
  const std::string &str,
  const std::string& search,
  const std::string& replacement)
{
  std::string modified {str};
  std::string::size_type pos = 0;
  while ((pos = modified.find (search, pos)) != std::string::npos)
  {
    modified.replace (pos, search.length (), replacement);
    pos += replacement.length ();
  }

  return modified;
}

////////////////////////////////////////////////////////////////////////////////
std::string trim (const std::string& input, const std::string& edible)
{
  auto start = input.find_first_not_of (edible);
  auto end   = input.find_last_not_of  (edible);

  if (start == std::string::npos)
    return "";

  if (end == std::string::npos)
    return input.substr (start);

  return input.substr (start, end - start + 1);
}

////////////////////////////////////////////////////////////////////////////////
std::string ltrim (const std::string& input, const std::string& edible)
{
  auto start = input.find_first_not_of (edible);
  if (start == std::string::npos)
    return "";

  return input.substr (start);
}

////////////////////////////////////////////////////////////////////////////////
std::string rtrim (const std::string& input, const std::string& edible)
{
  if (input.find_first_not_of (edible) == std::string::npos)
    return "";

  auto end = input.find_last_not_of (edible);
  if (end == std::string::npos)
    return input;

  return input.substr (0, end + 1);
}

////////////////////////////////////////////////////////////////////////////////
int longestWord (const std::string& input)
{
  int longest = 0;
  int length = 0;
  std::string::size_type i = 0;
  int character;

  while ((character = utf8_next_char (input, i)))
  {
    if (character == ' ')
    {
      if (length > longest)
        longest = length;

      length = 0;
    }
    else
      length += mk_wcwidth (character);
  }

  if (length > longest)
    longest = length;

  return longest;
}

////////////////////////////////////////////////////////////////////////////////
int longestLine (const std::string& input)
{
  int longest = 0;
  int length = 0;
  std::string::size_type i = 0;
  int character;

  while ((character = utf8_next_char (input, i)))
  {
    if (character == '\n')
    {
      if (length > longest)
        longest = length;

      length = 0;
    }
    else
      length += mk_wcwidth (character);
  }

  if (length > longest)
    longest = length;

  return longest;
}

// ISSUE: The multiline comment above extractLine() used to mention line breaking
// at certain punctuation characters, even though the function didn't actually
// do anything like that. Is a rule that preferentially makes line breaks at
// punctuation characters (when no word break is available) a desirable feature?

////////////////////////////////////////////////////////////////////////////////
// Walk the input text looking for a break point.  A break point is one of:
//   - EOS
//   - \n
//   - last space (word break) before column 'width' on the line
//   - first character that would make the line wider than 'width' columns
bool extractLine (
  std::string& line,
  const std::string& text,
  int width,
  bool hyphenate,
  unsigned int& offset,
  char surrogate)
{
  // Pre-process the input string.
  const std::string* text_ptr = &text;
  std::string text_fixed;

  if (width < MAX_CHAR_DISPLAY_WIDTH && containsTooWideChars (text, width))
  {
    std::string surrogate_str (1, surrogate);
    text_fixed = text;
    replaceTooWideChars (text_fixed, width, surrogate_str);
    text_ptr = &text_fixed;
  }

  std::vector<std::string::size_type> ch_byte_indexes;
  getUtf8CharacterByteIndexes (*text_ptr, ch_byte_indexes);

  auto find_res = std::find (ch_byte_indexes.begin (), ch_byte_indexes.end (), offset);
  if (find_res == ch_byte_indexes.end ())  // (offset) is not a valid character byte index.
    return false;

  unsigned int ch_index = find_res - ch_byte_indexes.begin ();

  // Extract a line of wrapped text.
  bool res = extractLine (line, *text_ptr, ch_byte_indexes, width, hyphenate, ch_index);
  offset = (ch_index < ch_byte_indexes.size ()) ? ch_byte_indexes[ch_index] : text.size ();
  return res;
}

////////////////////////////////////////////////////////////////////////////////
bool compare (
  const std::string& left,
  const std::string& right,
  bool sensitive /*= true*/)
{
  // Use case-insensitive comparison if required.
  if (! sensitive)
  {
#ifndef _WIN32
    return strcasecmp (left.c_str (), right.c_str ()) == 0;
#else
    return _stricmp (left.c_str (), right.c_str ()) == 0;
#endif
  }

  // Otherwise, just use std::string::operator==.
  return left == right;
}

////////////////////////////////////////////////////////////////////////////////
bool closeEnough (
  const std::string& reference,
  const std::string& attempt,
  unsigned int minLength /* = 0 */)
{
  // An exact match is accepted first.
  if (compare (reference, attempt, false))
    return true;

  // A partial match will suffice.
  if (attempt.length () < reference.length () &&
      attempt.length () >= minLength)
    return compare (reference.substr (0, attempt.length ()), attempt, false);

  return false;
}

////////////////////////////////////////////////////////////////////////////////
int matchLength (
  const std::string& left,
  const std::string& right)
{
  int pos = 0;
  while (left[pos] &&
         right[pos] &&
         left[pos] == right[pos])
    ++pos;

  return pos;
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type find (
  const std::string& text,
  const std::string& pattern,
  bool sensitive)
{
  return find (text, pattern, 0, sensitive);
}

////////////////////////////////////////////////////////////////////////////////
std::string::size_type find (
  const std::string& text,
  const std::string& pattern,
  std::string::size_type begin,
  bool sensitive)
{
  // Implement a sensitive find, which is really just a loop withing a loop,
  // comparing lower-case versions of each character in turn.
  if (!sensitive)
  {
    // Handle empty pattern.
    const char* p = pattern.c_str ();
    size_t len = pattern.length ();
    if (len == 0)
      return 0;

    // Handle bad begin.
    if (begin >= text.length ())
      return std::string::npos;

    // Evaluate these once, for performance reasons.
    const char* start = text.c_str ();
    const char* t = start + begin;
    const char* end = start + text.size ();

    for (; t <= end - len; ++t)
    {
      int diff = 0;
      for (size_t i = 0; i < len; ++i)
        if ((diff = tolower (t[i]) - tolower (p[i])))
          break;

      // diff == 0 means there was no break from the loop, which only occurs
      // when a difference is detected.  Therefore, the loop terminated, and
      // diff is zero.
      if (diff == 0)
        return t - start;
    }

    return std::string::npos;
  }

  // Otherwise, just use std::string::find.
  return text.find (pattern, begin);
}

////////////////////////////////////////////////////////////////////////////////
std::string lowerCase (const std::string& input)
{
  std::string output {input};
  std::transform (output.begin (), output.end (), output.begin (), tolower);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string upperCase (const std::string& input)
{
  std::string output {input};
  std::transform (output.begin (), output.end (), output.begin (), toupper);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
std::string upperCaseFirst (const std::string& input)
{
  std::string output {input};
  output[0] = toupper (output[0]);
  return output;
}

////////////////////////////////////////////////////////////////////////////////
int autoComplete (
  const std::string& partial,
  const std::vector<std::string>& list,
  std::vector<std::string>& matches,
  int minimum/* = 1*/)
{
  matches.clear ();

  // Handle trivial case.
  unsigned int length = partial.length ();
  if (length)
  {
    for (auto& item : list)
    {
      // An exact match is a special case.  Assume there is only one exact match
      // and return immediately.
      if (partial == item)
      {
        matches.clear ();
        matches.push_back (item);
        return 1;
      }

      // Maintain a list of partial matches.
      else if (length >= (unsigned) minimum &&
               length <= item.length ()     &&
               partial == item.substr (0, length))
        matches.push_back (item);
    }
  }

  return matches.size ();
}

////////////////////////////////////////////////////////////////////////////////
// Uses std::getline, because std::cin eats leading whitespace, and that means
// that if a newline is entered, std::cin eats it and never returns from the
// "std::cin >> answer;" line, but it does display the newline.  This way, with
// std::getline, the newline can be detected, and the prompt re-written.
static void signal_handler (int s)
{
  if (s == SIGINT)
  {
    std::cout << "\n\nInterrupted: No changes made.\n";
    exit (1);
  }
}

bool confirm (const std::string& question)
{
  std::vector <std::string> options {"yes", "no"};
  std::vector <std::string> matches;

  signal (SIGINT, signal_handler);

  do
  {
    std::cout << question
              << " (yes/no) ";

    std::string answer;
    std::getline (std::cin, answer);
    answer = std::cin.eof () ? "no" : lowerCase (trim (answer));

    autoComplete (answer, options, matches, 1); // Hard-coded 1.
  }
  while (! std::cin.eof () && matches.size () != 1);

  signal (SIGINT, SIG_DFL);
  return matches.size () == 1 && matches[0] == "yes";
}

////////////////////////////////////////////////////////////////////////////////
// Run a binary with args, capturing output.
int execute (
  const std::string& executable,
  const std::vector <std::string>& args,
  const std::string& input,
  std::string& output)
{
#ifdef _WIN32
  // Windows implementation using CreateProcess and pipes
  HANDLE hChildStdOutRd, hChildStdOutWr;
  HANDLE hChildStdInRd, hChildStdInWr;
  SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  // Create pipes for the child process's STDOUT and STDIN
  if (!CreatePipe(&hChildStdOutRd, &hChildStdOutWr, &saAttr, 0)) {
    return -1 * static_cast<int>(GetLastError());
  }
  if (!SetHandleInformation(hChildStdOutRd, HANDLE_FLAG_INHERIT, 0)) {
    return -1 * static_cast<int>(GetLastError());
  }
  if (!CreatePipe(&hChildStdInRd, &hChildStdInWr, &saAttr, 0)) {
    return -1 * static_cast<int>(GetLastError());
  }
  if (!SetHandleInformation(hChildStdInWr, HANDLE_FLAG_INHERIT, 0)) {
    return -1 * static_cast<int>(GetLastError());
  }

  // Prepare the child process
  PROCESS_INFORMATION piProcInfo = {0};
  STARTUPINFO siStartInfo = {sizeof(STARTUPINFO)};
  siStartInfo.hStdError = hChildStdOutWr;
  siStartInfo.hStdOutput = hChildStdOutWr;
  siStartInfo.hStdInput = hChildStdInRd;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

  // Build the command line
  std::string command = executable;
  for (const auto& arg : args)
    command += " " + arg;

  // Create the child process
  if (!CreateProcess(NULL, &command[0], NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
    return -1;

  // Close unused handles
  CloseHandle(hChildStdOutWr);
  CloseHandle(hChildStdInRd);

  // Write input to child process if provided
  if (!input.empty()) {
    DWORD bytesWritten;
    WriteFile(hChildStdInWr, input.c_str(), static_cast<DWORD>(input.length()), &bytesWritten, NULL);
  }
  CloseHandle(hChildStdInWr);

  // Read the child process's output
  char buffer[4096];
  DWORD bytesRead;
  output = "";
  while (ReadFile(hChildStdOutRd, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
    buffer[bytesRead] = '\0';
    output += buffer;
  }

  // Wait for the child process to finish
  WaitForSingleObject(piProcInfo.hProcess, INFINITE);

  // Get the exit code
  DWORD exitCode;
  GetExitCodeProcess(piProcInfo.hProcess, &exitCode);

  // Clean up
  CloseHandle(piProcInfo.hProcess);
  CloseHandle(piProcInfo.hThread);
  CloseHandle(hChildStdOutRd);

  return static_cast<int>(exitCode);
#else
  pid_t pid;
  int pin[2], pout[2];
  fd_set rfds, wfds;
  struct timeval tv;
  int select_retval, read_retval, write_retval;
  char buf[16384];
  unsigned int written;
  const char* input_cstr = input.c_str ();

  if (signal (SIGPIPE, SIG_IGN) == SIG_ERR) // Handled locally with EPIPE.
    throw std::string (std::strerror (errno));

  if (pipe (pin) == -1)
    throw std::string (std::strerror (errno));

  if (pipe (pout) == -1)
    throw std::string (std::strerror (errno));

  if ((pid = fork ()) == -1)
    throw std::string (std::strerror (errno));

  if (pid == 0)
  {
    // This is only reached in the child
    close (pin[1]);   // Close the write end of the input pipe.
    close (pout[0]);  // Close the read end of the output pipe.

    // Parent writes to pin[1]. Set read end pin[0] as STDIN for child.
    if (dup2 (pin[0], STDIN_FILENO) == -1)
      throw std::string (std::strerror (errno));
    close (pin[0]);

    // Parent reads from pout[0]. Set write end pout[1] as STDOUT for child.
    if (dup2 (pout[1], STDOUT_FILENO) == -1)
      throw std::string (std::strerror (errno));
    close (pout[1]);

    // Add executable as argv[0] and NULL-terminate the array for execvp().
    char** argv = new char* [args.size () + 2];
    argv[0] = (char*) executable.c_str ();
    for (unsigned int i = 0; i < args.size (); ++i)
      argv[i+1] = (char*) args[i].c_str ();

    argv[args.size () + 1] = nullptr;
    int rc = execvp (executable.c_str (), argv);
    std::cerr << "Failed to execute '" << executable << "' Error: " << strerror (errno) << '\n';
    _exit (rc);
  }

  // This is only reached in the parent
  close (pin[0]);   // Close the read end of the input pipe.
  close (pout[1]);  // Close the write end of the output pipe.

  if (input.empty ())
  {
    // Nothing to send to the child, close the pipe early.
    close (pin[1]);
  }

  output = "";
  read_retval = -1;
  written = 0;
  while (read_retval != 0 || input.size () != written)
  {
    FD_ZERO (&rfds);
    if (read_retval != 0)
      FD_SET (pout[0], &rfds);

    FD_ZERO (&wfds);
    if (input.size () != written)
      FD_SET (pin[1], &wfds);

    // On Linux, tv may be overwritten by select().  Reset it each time.
    // NOTE: Timeout chosen arbitrarily - we don't time out execute() calls.
    // select() is run over and over again unless the child exits or closes
    // its pipes.
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    select_retval = select (std::max (pout[0], pin[1]) + 1, &rfds, &wfds, nullptr, &tv);

    if (select_retval == -1)
    {
      // Haiku (and probably some very old-school UNIXs) fails with EINTR or
      // EAGAIN when SIGCHLD arrives during a blocking call. Retrying fixes it.
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        continue;
      throw std::string (std::strerror (errno));
    }

    // Write data to child's STDIN
    if (FD_ISSET (pin[1], &wfds))
    {
      write_retval = write (pin[1], input_cstr + written, input.size () - written);
      if (write_retval == -1)
      {
        if (errno == EPIPE)
        {
          // Child died (or closed the pipe) before reading all input.
          // We don't really care; pretend we wrote it all.
          write_retval = input.size () - written;
        }
        else if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        {
          // Interrupted, try again.
          write_retval = 0;
        }
        else
        {
          throw std::string (std::strerror (errno));
        }
      }
      written += write_retval;

      if (written == input.size ())
      {
        // Let the child know that no more input is coming by closing the pipe.
        close (pin[1]);
      }
    }

    // Read data from child's STDOUT
    if (FD_ISSET (pout[0], &rfds))
    {
      read_retval = read (pout[0], &buf, sizeof (buf) - 1);
      if (read_retval == -1)
      {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
          continue;
        throw std::string (std::strerror (errno));
      }

      buf[read_retval] = '\0';
      output += buf;
    }
  }

  close (pout[0]);  // Close the read end of the output pipe.

  int status = -1;
  int wait_retval;
  do
    wait_retval = waitpid (pid, &status, 0);
  while (wait_retval == -1 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK));
  if (wait_retval == -1)
    throw std::string (std::strerror (errno));

  if (WIFEXITED (status))
  {
    status = WEXITSTATUS (status);
  }
  else
  {
    throw std::string ("Error: Could not get Hook exit status!");
  }

  if (signal (SIGPIPE, SIG_DFL) == SIG_ERR)  // We're done, return to default.
    throw std::string (std::strerror (errno));

  return status;
#endif
}

////////////////////////////////////////////////////////////////////////////////
std::string osName ()
{
#if defined (DARWIN)
  return "Darwin";
#elif defined (SOLARIS)
  return "Solaris";
#elif defined (CYGWIN)
  return "Cygwin";
#elif defined (HAIKU)
  return "Haiku";
#elif defined (OPENBSD)
  return "OpenBSD";
#elif defined (FREEBSD)
  return "FreeBSD";
#elif defined (NETBSD)
  return "NetBSD";
#elif defined (DRAGONFLY)
  return "Dragonfly";
#elif defined (LINUX)
  return "Linux";
#elif defined (KFREEBSD)
  return "GNU/kFreeBSD";
#elif defined (GNUHURD)
  return "GNU/Hurd";
#elif defined (_WIN32)
  return "Windows";
#else
  return "<unknown>";
#endif
}

////////////////////////////////////////////////////////////////////////////////
// 16.8 Predefined macro names [cpp.predefined]
//
// The following macro names shall be defined by the implementation:
//
// __cplusplus
//   The name __cplusplus is defined to the value 201402L when compiling a C++
//   translation unit.156
//
// ---
//   156) It is intended that future versions of this standard will replace the
//   value of this macro with a greater value. Non-conforming compilers should
//   use a value with at most five decimal digits.
std::string cppCompliance ()
{
#ifdef __cplusplus
  auto level = __cplusplus;

       if (level == 199711) return "C++98/03";
  else if (level == 201103) return "C++11";
  else if (level == 201402) return "C++14";

  // This is a hack.  Replace with correct value on standard publication.
  else if (level >  201700) return "C++17";

  // Unknown, just show the value.
  else if (level >   99999) return format (__cplusplus);
#endif

  // No C++.
  return "non-compliant";
}

////////////////////////////////////////////////////////////////////////////////
