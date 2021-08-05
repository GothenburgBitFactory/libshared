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
#include <Configuration.h>
#include <inttypes.h>
#include <stdlib.h>
#include <FS.h>
#include <JSON.h>
#include <shared.h>
#include <format.h>

////////////////////////////////////////////////////////////////////////////////
bool setVariableInFile (
  const std::string& file,
  const std::string& name,
  const std::string& value)
{
  // Read the file contents.
  std::vector <std::string> contents;
  File::read (file, contents);

  bool found = false;
  bool change = false;

  for (auto& line : contents)
  {
    // If there is a comment on the line, it must follow the pattern.
    auto comment = line.find ('#');
    auto pos     = line.find (name + '=');

    if (pos != std::string::npos &&
        (comment == std::string::npos ||
         comment > pos))
    {
      found = true;
      if (comment != std::string::npos)
        line = name + '=' + value + ' ' + line.substr (comment);
      else
        line = name + '=' + value;

      change = true;
    }
  }

  // Not found, so append instead.
  if (! found)
  {
    contents.push_back (name + '=' + value);
    change = true;
  }

  if (change)
    File::write (file, contents);

  return change;
}

////////////////////////////////////////////////////////////////////////////////
bool unsetVariableInFile (
  const std::string& file,
  const std::string& name)
{
  // Read configuration file.
  std::vector <std::string> contents;
  File::read (file, contents);

  bool change = false;

  for (auto line = contents.begin (); line != contents.end (); )
  {
    bool lineDeleted = false;

    // If there is a comment on the line, it must follow the pattern.
    auto comment = line->find ('#');
    auto pos     = line->find (name + '=');

    if (pos != std::string::npos &&
        (comment == std::string::npos ||
         comment > pos))
    {
      // vector::erase method returns a valid iterator to the next object
      line = contents.erase (line);
      lineDeleted = true;
      change = true;
    }

    if (! lineDeleted)
      line++;
  }

  if (change)
    File::write (file, contents);

  return change;
}

////////////////////////////////////////////////////////////////////////////////
// Read the Configuration file and populate the *this map.  The file format is
// simply lines with name=value pairs.  Whitespace between name, = and value is
// not tolerated, but blank lines and comments starting with # are allowed.
//
// Nested files are now supported, with the following construct:
//   include /absolute/path/to/file
//
void Configuration::load (
  const std::string& file,
  int nest /* = 1 */,
  const std::vector <std::string>& search_paths /* = {} */)
{
  if (nest > 10)
    throw std::string ("Configuration files may only be nested to 10 levels.");

  // Read the file, then parse the contents.
  File config (file);

  if (nest == 1)
    _original_file = config;

  if (config.exists () &&
      config.readable ())
  {
    std::string contents;
    if (File::read (file, contents) && contents.length ())
      parse (contents, nest, search_paths, file);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Write the Configuration file.
void Configuration::save ()
{
  std::string contents;
  for (const auto& i : *this)
    contents += i.first + "=" + i.second + '\n';

  File::write (_original_file, contents);
  _dirty = false;
}

////////////////////////////////////////////////////////////////////////////////
void Configuration::parse (
  const std::string& input,
  int nest /* = 1 */,
  const std::vector <std::string>& search_paths /* = {} */,
  const std::string& file_path /* = {} */)
{
  // Shortcut case for default constructor.
  if (input.length () == 0)
    return;

  // Parse each line.
  for (auto& line : split (input, '\n'))
  {
    // Remove comments.
    auto pound = line.find ('#');
    if (pound != std::string::npos)
      line = line.substr (0, pound);

    // Skip empty lines.
    line = trim (line);
    if (line.length () > 0)
    {
      auto equal = line.find ('=');
      if (equal != std::string::npos)
      {
        std::string key   = trim (line.substr (0, equal));
        std::string value = trim (line.substr (equal+1, line.length () - equal));
        value = Path::expand(value);

        (*this)[key] = json::decode (value);
      }
      else
      {
        auto include = line.find ("include");
        if (include != std::string::npos)
        {
          Path included (trim (line.substr (include + 7)));

          do
          {
            // 0. Absolute path is not searched.
            if (included.is_absolute ())
              break;

            // 1. Try relative to CWD first, break if exists. This is the legacy behavior.
            if (included.exists ())
              break;

            // 2. Try path relative to the config file itself.
            if (!file_path.empty ())
            {
              std::string file_dir = Path(file_path).realpath();
              auto slash = file_dir.rfind('/');
              file_dir.resize (slash != std::string::npos ? slash + 1 : 0); // `/` is kept.
              Path file_relative (file_dir + included._data);
              if (file_relative.exists ())
              {
                included = file_relative;
                break;
              }
            }

            // 3. Try search paths.
            for (auto &search: search_paths)
            {
              Path concated (search + "/" + included._data);
              if (concated.exists ()) {
                included = concated;
                break;
              }
            }
            if (!included.exists ())
              throw format (
                  "Could not find file in CWD, directory of config file or search paths '{1}'.",
                  included._data);
          }
          while (0);

          if (!included.readable ())
            throw format ("Could not read include file '{1}'.", included._data);

          load (included, nest + 1, search_paths);
        }
        else
          throw format ("Malformed entry '{1}' in config file.", line);
      }
    }
  }

  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
bool Configuration::has (const std::string& key) const
{
  auto ckey = key;
  auto itContext = find ("context");
  if(itContext != end() && key.rfind("context.", 0) != 0) {
    ckey = "context." + itContext->second + ".rc." + key;
  }
  if (find (ckey) != end ()) 
    return true;
  return find (key) != end ();
}

////////////////////////////////////////////////////////////////////////////////
// Return the configuration value given the specified key.
std::string Configuration::get (const std::string& key, bool getFromContext) const
{
  auto ckey = key;
  auto itContext = find ("context");
  if(itContext != end() && getFromContext && key.rfind("context.", 0) != 0) {
    ckey = "context." + itContext->second + ".rc." + key;
  }
  auto found = find (ckey);
  if (found != end ())
    return found->second;
  // Fallback - use global config value
  found = find (key);
  if (found != end ())
    return found->second;

  return "";
}

////////////////////////////////////////////////////////////////////////////////
int Configuration::getInteger (const std::string& key, bool getFromContext) const
{
  auto val = get(key, getFromContext);
  if (val.length() > 0)
    return strtoimax (val.c_str (), nullptr, 10);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
double Configuration::getReal (const std::string& key, bool getFromContext) const
{
  auto val = get(key, getFromContext);
  if (val.length() > 0)
    return strtod (val.c_str (), nullptr);

  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////
bool Configuration::getBoolean (const std::string& key, bool getFromContext) const
{
  auto val = get(key, getFromContext);
  if (val.length() > 0)
  {
    auto value = lowerCase (val);
    if (value == "true"   ||
        value == "1"      ||
        value == "y"      ||
        value == "yes"    ||
        value == "on")
      return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
void Configuration::set (const std::string& key, const int value)
{
  (*this)[key] = format (value);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void Configuration::set (const std::string& key, const double value)
{
  (*this)[key] = format (value, 1, 8);
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
void Configuration::set (const std::string& key, const std::string& value)
{
  (*this)[key] = value;
  _dirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Autovivification is ok here.
void Configuration::setIfBlank (const std::string& key, const std::string& value)
{
  if ((*this)[key] == "")
  {
    (*this)[key] = value;
    _dirty = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Provide a vector of all configuration keys.
std::vector <std::string> Configuration::all () const
{
  std::vector <std::string> items;
  for (const auto& it : *this)
    items.push_back (it.first);

  return items;
}

////////////////////////////////////////////////////////////////////////////////
std::string Configuration::file () const
{
  return _original_file._data;
}

////////////////////////////////////////////////////////////////////////////////
bool Configuration::dirty ()
{
  return _dirty;
}

////////////////////////////////////////////////////////////////////////////////
