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
#include <PEG.h>
#include <Lexer.h>
#include <utf8.h>
#include <shared.h>
#include <format.h>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////
std::string PEG::Token::dump () const
{
  std::stringstream out;
  out << (_lookahead == Token::Lookahead::positive ? "[34m&[0m" :
          _lookahead == Token::Lookahead::negative ? "[34m![0m" : "")
      << _token
      << (_quantifier == Token::Quantifier::zero_or_one  ? "[34m?[0m" :
          _quantifier == Token::Quantifier::one_or_more  ? "[34m+[0m" :
          _quantifier == Token::Quantifier::zero_or_more ? "[34m*[0m" : "");

  for (const auto& tag : _tags)
    out << " [34m" << tag << "[0m";

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
void PEG::loadFromFile (File& file)
{
  if (! file.exists ())
    throw format ("PEG file '{1}' not found.", file._data);

  std::string contents;
  file.read (contents);

  // TODO Instead of simply reading a file, read a file and allow lines that
  //      match /^include <path>$/ to represent nested files.

  loadFromString (contents);
}

////////////////////////////////////////////////////////////////////////////////
// Load and parse PEG.
//
// Syntax:
//   rule-name:  alternate1-token1 alternate1-token2
//               alternate2-token1
//
// - Rules are aligned at left margin only, followed by colon.
// - Productions are indented and never at left margin.
// - Blank lines delineate rules.
//
// Details:
// - String literals are always double-quoted.
// - Character literals are alをays single-quoted.
// - "*", "+" and "?" suffixes have POSIX wildcard semantics.
//
void PEG::loadFromString (const std::string& input)
{
  // This is a state machine.  Read each line.
  std::string rule_name;
  for (auto& line : loadImports (split (input, '\n')))
  {
    line = trim (removeComment (line));

    // Skip blank lines with no semantics.
    if (line == "" and rule_name == "")
      continue;

    if (line != "")
    {
      int token_count = 0;

      // Instantiate and configure the Lexer.
      Lexer l (line);
      l.noDate ();
      l.noDuration ();
      l.noUUID ();
      l.noHexNumber ();
      l.noURL ();
      l.noPath ();
      l.noPattern ();
      l.noOperator ();

      Lexer::Type type;
      std::string token;
      while (l.token (token, type))
      {
        ++token_count;

        // Rule definitions end in a colon.
        if (token.back () == ':')
        {
          // Capture the Rule_name.
          rule_name = token.substr (0, token.size () - 1);

          // If this is the first Rule, capture it as a starting point.
          if (_start == "")
            _start = rule_name;

          _rules[rule_name] = PEG::Rule ();
          token_count = 0;
        }
        // Production definition.
        else
        {
          // If no Production was added yet, add one.
          if (token_count <= 1)
            _rules[rule_name].push_back (PEG::Production ());

          // Decorate the token, if necessary.
          std::string::size_type start = 0;
          std::string::size_type end = token.length ();

          auto q = Token::Quantifier::one;
          auto l = Token::Lookahead::none;

          if (token.back () == '?')
          {
            q = Token::Quantifier::zero_or_one;
            --end;
          }
          else if (token.back () == '+')
          {
            q = Token::Quantifier::one_or_more;
            --end;
          }
          else if (token.back () == '*')
          {
            q = Token::Quantifier::zero_or_more;
            --end;
          }

          if (token.front () == '&')
          {
            l = Token::Lookahead::positive;
            ++start;
          }
          else if (token.front () == '!')
          {
            l = Token::Lookahead::negative;
            ++start;
          }

          PEG::Token t (token.substr (start, end - start));
          t._quantifier = q;
          t._lookahead = l;

          if (type == Lexer::Type::string)
          {
            t.tag ("literal");
            t.tag (token[0] == '\'' ? "character" : "string");
          }
          else if (t._token.front () == '<' and
                   t._token.back ()  == '>')
          {
            t.tag ("intrinsic");

            if (t._token.substr (0, 8) == "<entity:")
              t.tag ("entity");

            if (t._token.substr (0, 10) == "<external:")
              t.tag ("external");
          }

          // Add the new Token to the most recent Production, of the current
          // Rule.
          _rules[rule_name].back ().push_back (t);
        }
      }
    }

    // A blank line in the input ends the current rule definition.
    else
      rule_name = "";
  }

  if (_debug)
    std::cout << dump ();

  // Validate the parsed grammar.
  validate ();
}

////////////////////////////////////////////////////////////////////////////////
std::map <std::string, PEG::Rule> PEG::syntax () const
{
  return _rules;
}

////////////////////////////////////////////////////////////////////////////////
std::string PEG::firstRule () const
{
  return _start;
}

////////////////////////////////////////////////////////////////////////////////
void PEG::debug ()
{
  ++_debug;
}

////////////////////////////////////////////////////////////////////////////////
void PEG::strict (bool value)
{
  _strict = value;
}

////////////////////////////////////////////////////////////////////////////////
std::string PEG::dump () const
{
  std::stringstream out;
  out << "PEG\n";

  // Show the import files, if any.
  if (_imports.size ())
  {
    for (const auto& import : _imports)
      out << "  import " << import << '\n';
    out << '\n';
  }

  // Determine longest rule name, for display alignment.
  size_t longest = 0;
  for (const auto& rule : _rules)
    if (rule.first.length () > longest)
      longest = rule.first.length ();

  for (const auto& rule : _rules)
  {
    // Indicate the start Rule.
    out << "  "
        << (rule.first == _start ? "▶" : " ")
        << ' '
        << rule.first
        << ':'
        << std::string (1 + longest - rule.first.length (), ' ');

    int count = 0;
    for (const auto& production : rule.second)
    {
      if (count)
        out << std::string (6 + longest, ' ');

      for (const auto& token : production)
        out << token.dump () << ' ';

      out << "\n";
      ++count;
    }

    out << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
std::vector <std::string> PEG::loadImports (const std::vector <std::string>& lines)
{
  std::vector <std::string> resolved;

  for (auto& line : lines)
  {
    auto copy = trim (removeComment (line));

    if (copy.find ("import ") == 0)
    {
      File file (trim (copy.substr (7)));
      if (file.exists () &&
          file.readable ())
      {
        // Only import files that are not already imported.
        if (std::find (_imports.begin (), _imports.end (), file._data) == _imports.end ())
        {
          _imports.push_back (file._data);

          std::vector <std::string> imported;
          file.read (imported);
          imported = loadImports (imported);

          resolved.insert(std::end(resolved), std::begin(imported), std::end(imported));
        }
      }
      else
        throw format ("Cannot import '{1}'", file._data);
    }
    else
    {
      // Store original line.
      resolved.push_back (line);
    }
  }

  return resolved;
}

////////////////////////////////////////////////////////////////////////////////
// Remove a comment from the line, which is means truncate after the first '#'
// character that is not quoted.
std::string PEG::removeComment (const std::string& line)
{
  // Scan line, keeping track of whether quotes are active.  Truncate at first
  // unquoted #.
  bool insideQuote = false;
  int quote = 0;

  std::string::size_type i = 0;
  int previous = 0;
  int character = 0;

  while ((character = utf8_next_char (line, i)))
  {
    if (insideQuote)
    {
       if (character == quote)
       {
         quote = 0;
         insideQuote = false;
       }
    }
    else if (character == '\'' && previous != '\\')
    {
      quote = '\'';
      insideQuote = true;
    }
    else if (character == '"' && previous != '\\')
    {
      quote = '"';
      insideQuote = true;
    }

    if (character == '#' && previous != '\\'  && ! insideQuote)
      return line.substr (0, i - 1);

    previous = character;
  }

  return line;
}

////////////////////////////////////////////////////////////////////////////////
void PEG::validate () const
{
  if (_start == "")
    throw std::string ("There are no rules defined.");

  std::vector <std::string> allRules;
  std::vector <std::string> allTokens;
  std::vector <std::string> allLeftRecursive;
  std::vector <std::string> intrinsics;
  std::vector <std::string> externals;

  for (const auto& rule : _rules)
  {
    allRules.push_back (rule.first);

    for (const auto& production : rule.second)
    {
      for (const auto& token : production)
      {
        if (token.hasTag ("intrinsic"))
          intrinsics.push_back (token._token);

        else if (token.hasTag ("external"))
          externals.push_back (token._token);

        else if (! token.hasTag ("literal"))
          allTokens.push_back (token._token);

        if (token._token == production[0]._token and
            rule.first == production[0]._token   and
            production.size () == 1)
          allLeftRecursive.push_back (token._token);
      }
    }
  }

  std::vector <std::string> notUsed;
  std::vector <std::string> notDefined;
  listDiff (allRules, allTokens, notUsed, notDefined);

  // Undefined value - these are definitions that appear in token, but are
  // not in _rules.
  for (const auto& nd : notDefined)
    if (std::find (externals.begin (), externals.end (), nd) == externals.end ())
      throw format ("Definition '{1}' referenced, but not defined.", nd);

  // Circular definitions - these are names in _rules that also appear as
  // the only token in any of the alternates for that definition.
  for (const auto& lr : allLeftRecursive)
    throw format ("Definition '{1}' is left recursive.", lr);

  for (const auto& r : allRules)
    if (r[0] == '<')
      throw format ("Definition '{1}' may not redefine an intrinsic.");

  for (const auto& r : allRules)
    if (r[0] == '"' or
        r[0] == '\'')
      throw format ("Definition '{1}' may not be a literal.");

  // Unused definitions - these are names in _rules that are never
  // referenced as token.
  for (const auto& nu : notUsed)
  {
    if (std::find (externals.begin (), externals.end (), nu) == externals.end () &&
        nu != _start)
    {
      if (_strict)
        throw format ("Definition '{1}' is defined, but not referenced.", nu);
      else
        std::cout << "Warning: Definition '" << nu << "' is defined, but not referenced.\n";
    }
  }

  // Intrinsics must be recognized.
  for (auto& intrinsic : intrinsics)
    if (intrinsic != "<digit>"                 &&
        intrinsic != "<hex>"                   &&
        intrinsic != "<punct>"                 &&
        intrinsic != "<eol>"                   &&
        intrinsic != "<sep>"                   &&
        intrinsic != "<ws>"                    &&
        intrinsic != "<alpha>"                 &&
        intrinsic != "<character>"             &&
        intrinsic != "<word>"                  &&
        intrinsic != "<token>"                 &&
        intrinsic.substr (0, 8)  != "<entity:" &&
        intrinsic.substr (0, 10) != "<external:")
      throw format ("Specified intrinsic '{1}' is not supported.", intrinsic);
}

////////////////////////////////////////////////////////////////////////////////
