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
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_FS
#define INCLUDED_FS

#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
typedef unsigned short mode_t;
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

class Path
{
public:
  Path ();
  explicit Path (const Path&);
  Path (const std::string&);

  Path& operator= (const Path&);
  bool operator== (const Path&);
  Path& operator+= (const std::string&);
  operator std::string () const;

  std::string name () const;
  std::string parent () const;
  std::string extension () const;
  std::string realpath () const;
  bool exists () const;
  bool is_directory () const;
  bool is_absolute () const;
  bool is_link () const;
  bool readable () const;
  bool writable () const;
  bool executable () const;
  bool rename (const std::string&);

  // Statics
  static std::string expand (const std::string&);
  static std::vector<std::string> glob (const std::string&);

public:
  std::string _original;
  std::string _data;
};

class File : public Path
{
public:
  File ();
  explicit File (const Path&);
  explicit File (const File&);
  File (const std::string&);
  virtual ~File ();

  File& operator= (const File&);

  virtual bool create (int mode = 0640);
  virtual bool remove () const;

  bool open ();
  void close ();

  bool lock ();
  void unlock ();

  void read (std::string&);
  void read (std::vector <std::string>&);

  void append (const std::string&);
  void append (const std::vector <std::string>&);
  void write_raw (const std::string&);

  void truncate ();

  virtual mode_t mode ();
  virtual size_t size () const;
  virtual time_t mtime () const;
  virtual time_t ctime () const;
  virtual time_t btime () const;

  static bool create (const std::string&, int mode = 0640);
  static bool read (const std::string&, std::string&);
  static bool read (const std::string&, std::vector <std::string>&);
  static bool write (const std::string&, const std::string&);
  static bool write (const std::string&, const std::vector <std::string>&, bool addNewlines = true);
  static bool remove (const std::string&);
  static bool copy (const std::string&, const std::string&);
  static bool move (const std::string&, const std::string&);
  static std::string removeBOM (const std::string&);

private:
  FILE* _fh;
  int   _h;
  bool  _locked;
};

class Directory : public File
{
public:
  Directory ();
  explicit Directory (const Directory&);
  explicit Directory (const File&);
  explicit Directory (const Path&);
  Directory (const std::string&);

  Directory& operator= (const Directory&);

  virtual bool create (int mode = 0755);
  virtual bool remove () const;

  std::vector <std::string> list ();
  std::vector <std::string> listRecursive ();

  static std::string cwd ();
  bool up ();
  bool cd () const;

private:
  void list (const std::string&, std::vector <std::string>&, bool);
  bool remove_directory (const std::string&) const;
};

#endif
