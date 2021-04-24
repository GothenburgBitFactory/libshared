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
#include <shared.h>
#include <utf8.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <strings.h>
#include <unistd.h>
#include <sys/select.h>
#include <cerrno>
#include <csignal>
#include <cmath>
#include <cstring>
#include <sys/wait.h>
#include <format.h>

///////////////////////////////////////////////////////////////////////////////
void wrapText (
  std::vector <std::string>& lines,
  const std::string& text,
  const int width,
  bool hyphenate)
{
  std::string line;
  unsigned int offset = 0;
  while (extractLine (line, text, width, hyphenate, offset))
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

  if (input.length ())
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

////////////////////////////////////////////////////////////////////////////////
// Walk the input text looking for a break point.  A break point is one of:
//   - EOS
//   - \n
//   - last space before 'length' characters
//   - last punctuation (, ; . :) before 'length' characters, even if not
//     followed by a space
//   - first 'length' characters
//
// text       "one two three\n  four"
// bytes       0123456789012 3456789
// characters  1234567890a23 4567890
//
// leading_ws
// ws             ^   ^       ^^
// punct
// break                     ^
bool extractLine (
  std::string& line,
  const std::string& text,
  int width,
  bool hyphenate,
  unsigned int& offset,
  char surrogate)
{
  // Terminate processing.
  // Note: bytes vs bytes.
  if (offset >= text.length ())
    return false;

  std::string::size_type last_last_bytes = offset;
  std::string::size_type last_bytes = offset;
  std::string::size_type bytes = offset;
  unsigned int last_ws = 0;
  int character;
  int char_width = 0;
  int line_width = 0;
  while (1)
  {
    last_last_bytes = last_bytes;
    last_bytes = bytes;
    character = utf8_next_char (text, bytes);

    if (character == 0 ||
        character == '\n')
    {
      line = text.substr (offset, last_bytes - offset);
      offset = bytes;
      break;
    }
    else if (character == ' ')
      last_ws = last_bytes;

    char_width = mk_wcwidth (character);
    if (line_width + char_width > width)
    {
      int last_last_character = text[last_last_bytes];
      int last_character = text[last_bytes];

      // [case 1] one| two --> last_last != 32, last == 32, ws == 0
      if (last_last_character != ' ' &&
          last_character      == ' ')
      {
        line = text.substr (offset, last_bytes - offset);
        offset = last_bytes + 1;
        break;
      }

      // [case 2] one |two --> last_last == 32, last != 32, ws != 0
      else if (last_last_character == ' ' &&
               last_character      != ' ' &&
               last_ws             != 0)
      {
        line = text.substr (offset, last_bytes - offset - 1);
        offset = last_bytes;
        break;
      }

      else if (last_last_character != ' ' &&
               last_character      != ' ')
      {
        // [case 3] one t|wo --> last_last != 32, last != 32, ws != 0
        if (last_ws != 0)
        {
          line = text.substr (offset, last_ws - offset);
          offset = last_ws + 1;
          break;
        }
        // [case 4] on|e two --> last_last != 32, last != 32, ws == 0
        else
        {
          if (char_width > width && last_bytes == offset)
          {
            // the first character is already too wide,
            // no other way to split the line but to replace the character
            // with a surrogate
            line = surrogate;
            offset = bytes;
          }
          else
          {
            if (hyphenate)
            {
              if (line_width + 1 <= width)
              {
                // if the last good part + hyphen is short enough,
                // ie. the just read character is wider than one column
                line = text.substr (offset, last_bytes - offset) + '-';
                offset = last_bytes;
              }
              else if (last_last_bytes - offset > 0)
              {
                // sacrifice last character from the last good part,
                // but only if there is at least one character left
                line = text.substr (offset, last_last_bytes - offset) + '-';
                offset = last_last_bytes;
              }
              else
              {
                // no other way to split the line but to omit the hyphen
                line = text.substr (offset, last_bytes - offset);
                offset = last_bytes;
              }
            }
            else
            {
              // just use the last good part
              line = text.substr (offset, last_bytes - offset);
              offset = last_bytes;
            }
          }
        }

        break;
      }
    }

    line_width += char_width;
  }

  return true;
}
/*

TODO Resolve above against below, which is from Taskwarrior 2.6.0, and known to
     be wrong.
////////////////////////////////////////////////////////////////////////////////
// Break UTF8 text into chunks no more than width characters.
bool extractLine (
  std::string& line,
  const std::string& text,
  int width,
  bool hyphenate,
  unsigned int& offset)
{
  // Terminate processing.
  if (offset >= text.length ())
    return false;

  int line_length                     {0};
  int character                       {0};
  std::string::size_type lastWordEnd  {std::string::npos};
  bool something                      {false};
  std::string::size_type cursor       {offset};
  std::string::size_type prior_cursor {offset};
  while ((character = utf8_next_char (text, cursor)))
  {
    // Premature EOL.
    if (character == '\n')
    {
      line = text.substr (offset, line_length);
      offset = cursor;
      return true;
    }

    if (! Lexer::isWhitespace (character))
    {
      something = true;
      if (! text[cursor] || Lexer::isWhitespace (text[cursor]))
        lastWordEnd = prior_cursor;
    }

    line_length += mk_wcwidth (character);

    if (line_length >= width)
    {
      // Backtrack to previous word end.
      if (lastWordEnd != std::string::npos)
      {
        // Eat one WS after lastWordEnd.
        std::string::size_type lastBreak = lastWordEnd;
        utf8_next_char (text, lastBreak);

        // Position offset at following char.
        std::string::size_type nextStart = lastBreak;
        utf8_next_char (text, nextStart);

        line = text.substr (offset, lastBreak - offset);
        offset = nextStart;
        return true;
      }

      // No backtrack, possible hyphenation.
      else if (hyphenate)
      {
        line = text.substr (offset, prior_cursor - offset) + '-';
        offset = prior_cursor;
        return true;
      }

      // No hyphenation, just truncation.
      else
      {
        line = text.substr (offset, cursor - offset);
        offset = cursor;
        return true;
      }
    }

    // Hindsight.
    prior_cursor = cursor;
  }

  // Residual text.
  if (something)
  {
    line = text.substr (offset, cursor - offset);
     offset = cursor;
    return true;
  }

  return false;
}
*/

////////////////////////////////////////////////////////////////////////////////
bool compare (
  const std::string& left,
  const std::string& right,
  bool sensitive /*= true*/)
{
  // Use strcasecmp if required.
  if (! sensitive)
    return strcasecmp (left.c_str (), right.c_str ()) == 0 ? true : false;

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
  return matches.size () == 1 && matches[0] == "yes" ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
// Run a binary with args, capturing output.
int execute (
  const std::string& executable,
  const std::vector <std::string>& args,
  const std::string& input,
  std::string& output)
{
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

    argv[args.size () + 1] = NULL;
    int rc = execvp (executable.c_str (), argv);
    std::cerr << "Failed to execute '" << executable << "' Error: " << strerror (errno) << '\n';
    _exit (rc);
  }

  // This is only reached in the parent
  close (pin[0]);   // Close the read end of the input pipe.
  close (pout[1]);  // Close the write end of the output pipe.

  if (input.size () == 0)
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

    select_retval = select (std::max (pout[0], pin[1]) + 1, &rfds, &wfds, NULL, &tv);

    if (select_retval == -1)
      throw std::string (std::strerror (errno));

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
        throw std::string (std::strerror (errno));

      buf[read_retval] = '\0';
      output += buf;
    }
  }

  close (pout[0]);  // Close the read end of the output pipe.

  int status = -1;
  if (wait (&status) == -1)
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
