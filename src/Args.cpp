////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2012 - 2016, Paul Beckingham, Federico Hernandez.
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

////////////////////////////////////////////////////////////////////////////////
void Args::addOption (const std::string& name, bool defaultValue)
{
  _options[name] = defaultValue;
}

////////////////////////////////////////////////////////////////////////////////
void Args::addNamed  (const std::string& name, const std::string& defaultValue)
{
  _named[name] = defaultValue;
}

////////////////////////////////////////////////////////////////////////////////
void Args::limitPositionals (int limit)
{
  _limit = limit;
}

////////////////////////////////////////////////////////////////////////////////
void Args::scan (int argc, const char** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    // Is an option or named arg.
    if (argv[i][0] == '-')
    {
      auto name = ltrim (argv[i], "-");

      // Recognized option.
      if (_options.find (name) != _options.end ())
      {
        _options[name] = true;
      }

      // Recognized option, but reversed.
      else if (name.find ("no") == 0 &&
               _options.find (name.substr (2)) != _options.end ())
      {
        _options[name.substr (2)] = false;
      }

      // Recognized named arg.
      else if (_named.find (name) != _named.end ())
      {
        if (i >= argc)
          throw std::string ("Argument '" + name + "' has no value.");

        ++i;
        _named[name] = argv[i];
      }

      // None of the above.
      else
        throw std::string ("Unrecognized argument '" + name + "'.");
    }

    // Is a positional.
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
  return _options.at (name);
}

////////////////////////////////////////////////////////////////////////////////
std::string Args::getNamed (const std::string& name) const
{
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
std::string Args::dump () const
{
  std::stringstream out;
  out << "Args\n"
      << "  Options\n";
  for (const auto& arg : _options)
    out << "    " << arg.first << " = " << arg.second << '\n';

  out << "  Named\n";
  for (const auto& arg : _named)
    out << "    " << arg.first << " = " << arg.second << '\n';

  out << "  Positionals\n"
      << "    limit = " << _limit << '\n';
  for (const auto& arg : _positionals)
    out << "    " << arg << '\n';

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
