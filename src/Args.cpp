////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2012 - 2021, Paul Beckingham, Federico Hernandez.
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
#include <Args.h>
#include <shared.h>
#include <sstream>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
void Args::addOption (const std::string& name, bool defaultValue)
{
  _options[name] = defaultValue;
  _optionCount[name] = 0;
}

////////////////////////////////////////////////////////////////////////////////
void Args::addNamed (const std::string& name, const std::string& defaultValue)
{
  _named[name] = defaultValue;
}

////////////////////////////////////////////////////////////////////////////////
void Args::limitPositionals (int limit)
{
  _limit = limit;
}

////////////////////////////////////////////////////////////////////////////////
void Args::enableNegatives ()
{
  _negatives = true;
}

////////////////////////////////////////////////////////////////////////////////
void Args::scan (int argc, const char** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    // Is an option or named arg.
    if (argv[i][0] == '-' && strlen (argv[i]) > 1)
    {
      auto name = ltrim (argv[i], "-");

      std::string canonical;
      if (canonicalizeOption (name, canonical))
      {
        bool negated = _negatives && name.find ("no") == 0;
        _options[canonical] = ! negated;
        _optionCount[canonical]++;
      }

      else if (canonicalizeNamed (name, canonical))
      {
        if (i >= argc)
          throw std::string ("Argument '" + canonical + "' has no value.");

        ++i;
        _named[canonical] = argv[i];
      }

      else
        throw std::string ("Unrecognized argument '" + name + "'.");
    }

    // Or a positional.
    else
    {
      _positionals.push_back (argv[i]);
      if (_limit != -1 &&
          static_cast <int> (_positionals.size ()) > _limit)
        throw std::string ("Too many positional arguments.");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
bool Args::getOption (const std::string& name) const
{
  if (_options.find (name) == _options.end ())
    return false;

  return _options.at (name);
}

////////////////////////////////////////////////////////////////////////////////
int Args::getOptionCount (const std::string& name) const
{
  if (_optionCount.find (name) == _optionCount.end ())
    return false;

  return _optionCount.at (name);
}

////////////////////////////////////////////////////////////////////////////////
std::string Args::getNamed (const std::string& name) const
{
  if (_named.find (name) == _named.end ())
    return "";

  return _named.at (name);
}

////////////////////////////////////////////////////////////////////////////////
int Args::getPositionalCount () const
{
  return static_cast <int> (_positionals.size ());
}

////////////////////////////////////////////////////////////////////////////////
std::string Args::getPositional (int n) const
{
  return _positionals.at (n);
}

////////////////////////////////////////////////////////////////////////////////
// Assuming "abc" is a declared option, support the following canonicalization:
//
//   abc --> abc (exact match always canonicalizes)
//    ab --> abc (if unique)
//     a --> abc (if unique)
// noabc --> abc (exact negation match always canonicalizes)
//  noab --> abc (if unique)
//   noa --> abc (if unique)
//
bool Args::canonicalizeOption (const std::string& partial, std::string& canonical) const
{
  bool negated = _negatives && partial.find ("no") == 0;

  // Look for exact positive or negative matches first, which should succeed
  // regardless of any longer partial matches.
  if (_options.find (partial) != _options.end ())
  {
    canonical = partial;
    return true;
  }

  if (negated &&
      _options.find (partial.substr (2)) != _options.end ())
  {
    canonical = partial.substr (2);
    return true;
  }

  // Iterate over all options, and look for partial matches.  If there is only
  // one, we have canonicalization.
  std::vector <std::string> candidates;
  for (const auto& option : _options)
  {
    if (option.first.find (partial) == 0 ||
        (negated && option.first.find (partial, 2) == 2))
    {
      candidates.push_back (option.first);
    }
  }

  if (candidates.size () == 1)
  {
    canonical = candidates[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Assuming "abc" is a declared name, support the following canonicalization:
//
//   abc --> abc (exact match always canonicalizes)
//    ab --> abc (if unique)
//     a --> abc (if unique)
//
bool Args::canonicalizeNamed (const std::string& partial, std::string& canonical) const
{
  // Look for exact positive or negative matches first, which should succeed
  // regardless of longer partial matches.
  if (_named.find (partial) != _named.end ())
  {
    canonical = partial;
    return true;
  }

  // Iterate over all options, and look for partial matches.  If there is only
  // one, we have canonicalization.
  std::vector <std::string> candidates;
  for (const auto& name : _named)
    if (name.first.find (partial) == 0)
      candidates.push_back (name.first);

  if (candidates.size () == 1)
  {
    canonical = candidates[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Args::dump () const
{
  std::stringstream out;
  out << "Args\n"
      << "  Options\n";
  for (const auto& arg : _options)
    out << "    " << arg.first << " = " << arg.second << " (" << _optionCount.at (arg.first) << ")\n";

  out << "  Named\n";
  for (const auto& arg : _named)
    out << "    " << arg.first << " = " << arg.second << '\n';

  out << "  Positionals\n"
      << "    limit = " << _limit << '\n';
  for (const auto& arg : _positionals)
    out << "    " << arg << '\n';

  out << "  Negatives\n"
      << "    enabled = " << _negatives << '\n';

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
