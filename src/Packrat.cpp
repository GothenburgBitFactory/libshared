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
#include <Packrat.h>
#include <shared.h>
#include <format.h>
#include <unicode.h>
#include <utf8.h>
#include <iostream>

int Packrat::minimumMatchLength = 3;

////////////////////////////////////////////////////////////////////////////////
void Packrat::debug ()
{
  ++_debug;
}

////////////////////////////////////////////////////////////////////////////////
// Walk the grammar tree to parse the input text, resulting in a parse tree.
void Packrat::parse (const PEG& peg, const std::string& input)
{
  // Used to walk the grammar tree.
  // Note there is only one rule at the top of the syntax tree, which was the
  // first one defined.
  _syntax = peg.syntax ();
  _tree->_name = peg.firstRule ();

  // The pig that will be sent down the pipe.
  Pig pig (input);
  if (_debug)
    std::cout << "trace " << pig.dump () << "\n";

  // Match the first rule.  Recursion does the rest.
  if (! matchRule (_tree->_name, pig, _tree, 0))
    throw std::string ("Parse failed.");

  if (! pig.eos ())
    throw format ("Parse failed - extra character at position {1}.", pig.cursor ());
}

////////////////////////////////////////////////////////////////////////////////
void Packrat::entity (const std::string& category, const std::string& name)
{
  // Walk the list of entities for category.
  auto c = _entities.equal_range (category);
  for (auto e = c.first; e != c.second; ++e)
    if (e->second == name)
      return;

  // The category/name pair was not found, therefore add it.
  _entities.insert (std::pair <std::string, std::string> (category, name));
}

////////////////////////////////////////////////////////////////////////////////
void Packrat::external (
  const std::string& rule,
  bool (*fn)(Pig&, std::shared_ptr <Tree>))
{
  if (_externals.find (rule) != _externals.end ())
    throw format ("There is already an external parser defined for rule '{1}'.", rule);

  _externals[rule] = fn;
}

////////////////////////////////////////////////////////////////////////////////
// If there is a match, pig advances further down the pipe.
bool Packrat::matchRule (
  const std::string& rule,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchRule " << rule << "\n";
  auto checkpoint = pig.cursor ();

  for (const auto& production : _syntax.find (rule)->second)
    if (matchProduction (production, pig, parseTree, indent + 1))
      return true;

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchProduction (
  const PEG::Production& production,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchProduction\n";
  auto checkpoint = pig.cursor ();

  auto collector = std::make_shared <Tree> ();
  for (const auto& token : production)
  {
    auto b = std::make_shared <Tree> ();
    if (! matchTokenQuant (token, pig, b, indent + 1))
    {
      pig.restoreTo (checkpoint);
      return false;
    }

    // Accumulate branches.
    collector->addBranch (b);
  }

  // On success transfer all sub-branches.
  for (auto& b : collector->_branches)
    for (const auto& sub : b->_branches)
      parseTree->addBranch (sub);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Wraps calls to matchTokenLookahead, while properly handling the quantifier.
bool Packrat::matchTokenQuant (
  const PEG::Token& token,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchTokenQuant " << token.dump () << "\n";

  // Must match exactly once, so run once and return the result.
  if (token._quantifier == PEG::Token::Quantifier::one)
  {
    return matchTokenLookahead (token, pig, parseTree, indent + 1);
  }

  // May match zero or one time.  If it matches, the cursor will be advanced.
  // If it fails, the cursor will not be advanced, but this is still considered
  // successful.  Return true either way, but backtrack the cursor on failure.
  else if (token._quantifier == PEG::Token::Quantifier::zero_or_one)
  {
    // Check for a single match, succeed anyway.
    matchTokenLookahead (token, pig, parseTree, indent + 1);
    if (_debug > 1)
      std::cout << "trace " << std::string (indent, ' ') << "[32mmatch ?[0m " << token.dump () << "\n";
    if (_debug)
      std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
    return true;
  }

  // May match 1 or more times.  If it matches on the first attempt, continue
  // to greedily match until it fails.  If it fails on the first attempt, then
  // the rule fails.
  else if (token._quantifier == PEG::Token::Quantifier::one_or_more)
  {
    if (! matchTokenLookahead (token, pig, parseTree, indent + 1))
      return false;

    while (matchTokenLookahead (token, pig, parseTree, indent + 1))
    {
      // "Forget it, he's rolling."
    }

    if (_debug > 1)
      std::cout << "trace " << std::string (indent, ' ') << "[32mmatch +[0m " << token.dump () << "\n";
    if (_debug)
      std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
    return true;
  }

  // May match zero or more times.  Keep calling while there are matches, and
  // return true always.  Backtrack the cursor on failure.
  else if (token._quantifier == PEG::Token::Quantifier::zero_or_more)
  {
    while (matchTokenLookahead (token, pig, parseTree, indent + 1))
    {
      // Let it go.
    }

    if (_debug > 1)
      std::cout << "trace " << std::string (indent, ' ') << "[32mmatch *[0m " << token.dump () << "\n";
    if (_debug)
      std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
    return true;
  }

  throw std::string ("This should never happen.");
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Wraps calls to matchToken, while properly handling lookahead.
bool Packrat::matchTokenLookahead (
  const PEG::Token& token,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchTokenLookahead " << token.dump () << "\n";

  if (token._lookahead == PEG::Token::Lookahead::none)
  {
    return matchToken (token, pig, parseTree, indent + 1);
  }
  else if (token._lookahead == PEG::Token::Lookahead::positive)
  {
    auto checkpoint = pig.cursor ();
    auto b = std::make_shared <Tree> ();
    if (matchToken (token, pig, b, indent + 1))
    {
      pig.restoreTo (checkpoint);
      return true;
    }
  }
  else if (token._lookahead == PEG::Token::Lookahead::negative)
  {
    auto checkpoint = pig.cursor ();
    auto b = std::make_shared <Tree> ();
    if (! matchToken (token, pig, b, indent + 1))
    {
      return true;
    }

    pig.restoreTo (checkpoint);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchToken (
  const PEG::Token& token,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchToken " << token.dump () << "\n";

  auto checkpoint = pig.cursor ();
  auto b = std::make_shared <Tree> ();

  if (token.hasTag ("intrinsic") &&
      matchIntrinsic (token, pig, parseTree, indent + 1))
  {
    return true;
  }

  else if (_syntax.find (token._token) != _syntax.end () &&
           matchRule (token._token, pig, b, indent + 1))
  {
    // This is the only case that adds a sub-branch.
    b->_name = token._token;
    parseTree->addBranch (b);
    return true;
  }

  else if (token.hasTag ("literal") &&
           token.hasTag ("character") &&
           matchCharLiteral (token, pig, parseTree, indent + 1))
  {
   return true;
  }

  else if (token.hasTag ("literal") &&
           token.hasTag ("string") &&
           matchStringLiteral (token, pig, parseTree, indent + 1))
  {
    return true;
  }

  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Supports the following:
//   <digit>      --> unicodeLatinDigit
//   <hex>        --> unicodeHexDigit
//   <character>  --> anything
//   <alpha>      --> unicodeAlpha
//   <punct>      --> unicodePunctuation
//   <ws>         --> unicodeWhitespace
//   <sep>        --> unicodeHorizontalWhitespace
//   <eol>        --> unicodeVerticalWhitespace
//   <word>       --> <alpha>
//   <token>      --> consecutive non <ws>
//   <entity:e>   --> Any category 'e' token
//   <external:x> --> Delegate to external function
bool Packrat::matchIntrinsic (
  const PEG::Token& token,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchIntrinsic " << token.dump () << "\n";
  auto checkpoint = pig.cursor ();

  // There are only 10 digits.
  if (token._token == "<digit>")
  {
    int digit;
    if (pig.getDigit (digit))
    {
      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", digit));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << digit << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // Upper or lower case hex digit.
  if (token._token == "<hex>")
  {
    int digit;
    if (pig.getHexDigit (digit))
    {
      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", digit));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << digit << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // Character means anything.
  else if (token._token == "<character>")
  {
    int character;
    if (pig.getCharacter (character))
    {
      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << character << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <punct> ::ispunct
  else if (token._token == "<punct>")
  {
    int character = pig.peek ();
    if (unicodePunctuation (character))
    {
      pig.skip (character);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << character << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <alpha>
  else if (token._token == "<alpha>")
  {
    int character = pig.peek ();
    if (unicodeAlpha (character))
    {
      pig.skip (character);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << character << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <ws>
  else if (token._token == "<ws>")
  {
    int character = pig.peek ();
    if (unicodeWhitespace (character))
    {
      pig.skip (character);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug > 10)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << character << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <sep>
  else if (token._token == "<sep>")
  {
    int character = pig.peek ();
    if (unicodeHorizontalWhitespace (character))
    {
      pig.skip (character);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << character << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <eol>
  else if (token._token == "<eol>")
  {
    int character = pig.peek ();
    if (unicodeVerticalWhitespace (character))
    {
      pig.skip (character);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", format ("{1}", character));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << character << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <word> consecutive non-<ws>, non-<punct>.
  else if (token._token == "<word>")
  {
    while (auto character = pig.peek ())
    {
      if (! character ||
          unicodeWhitespace (character) ||
          unicodePunctuation (character))
        break;

      pig.skip (character);
    }

    if (pig.cursor () > checkpoint)
    {
      auto word = pig.substr (checkpoint, pig.cursor () - checkpoint + 1);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", word);
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << word << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <token> consecutive non-<ws>.
  else if (token._token == "<token>")
  {
    while (auto character = pig.peek ())
    {
      if (! character ||
          unicodeWhitespace (character))
        break;

      pig.skip (character);
    }

    if (pig.cursor () > checkpoint)
    {
      auto word = pig.substr (checkpoint, pig.cursor () - checkpoint);

      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "intrinsic";
      b->attribute ("expected", token._token);
      b->attribute ("value", word);
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << word << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  // <entity:category>.
  else if (token._token.find ("<entity:") == 0)
  {
    // Extract entity category
    auto category = token._token.substr (8, token._token.length () - 9);

    // Match against any one of the entity values in this category.
    auto values = _entities.equal_range (category);
    for (auto value = values.first; value != values.second; ++value)
    {
      if (pig.skipLiteral (value->second))
      {
        // Create a populated branch.
        auto b = std::make_shared <Tree> ();
        b->_name = "intrinsic";
        b->tag ("entity");
        b->attribute ("expected", token._token);
        b->attribute ("value", value->second);
        parseTree->addBranch (b);

        if (_debug > 1)
          std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << value->second << "\n";
        if (_debug)
          std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";

        return true;
      }
    }
  }

  // <external:rule>
  else if (token._token.find ("<external:") == 0)
  {
    // Extract entity category
    auto rule = token._token.substr (10, token._token.length () - 11);

    // Any rule can be overridden by an external parser.
    if (_externals.find (rule) != _externals.end ())
    {
      // Create a pre-populated branch, which is attached on success only.
      auto newBranch = std::make_shared <Tree> ();
      newBranch->_name = "intrinsic";
      newBranch->tag ("external");
      newBranch->attribute ("expected", token._token);

      if (_externals[rule] (pig, newBranch))
      {
        // Determine what was parsed.
        auto word = pig.substr (checkpoint, pig.cursor () - checkpoint);

        // Attach the new branch.
        newBranch->attribute ("value", word);
        parseTree->addBranch (newBranch);

        if (_debug > 1)
          std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << word << "\n";
        if (_debug)
          std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";

        return true;
      }

      // Note: Branch 'newBranch' goes out of scope here if parsing fails.
    }
  }

  if (_debug > 1)
     std::cout << "trace " << std::string (indent, ' ') << "[31mfail[0m " << token._token << "\n";
  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchCharLiteral (
  const PEG::Token& token,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchCharLiteral " << token.dump () << "\n";
  auto checkpoint = pig.cursor ();

  if (token._token.length () >= 3 &&
      token._token[0] == '\'' &&
      token._token[2] == '\'')
  {
    int literal = token._token[1];
    if (pig.skip (literal))
    {
      // Create a populated branch.
      auto b = std::make_shared <Tree> ();
      b->_name = "charLiteral";
      b->attribute ("expected", token._token);
      b->attribute ("value", utf8_character (literal));
      parseTree->addBranch (b);

      if (_debug > 1)
        std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << token._token << "\n";
      if (_debug)
        std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
      return true;
    }
  }

  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "[31mfail[0m " << token._token << "\n";
  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
bool Packrat::matchStringLiteral (
  const PEG::Token& token,
  Pig& pig,
  const std::shared_ptr <Tree>& parseTree,
  int indent)
{
  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "matchStringLiteral " << token.dump () << "\n";
  auto checkpoint = pig.cursor ();

  std::string literal = token._token.substr (1, token._token.length () - 2);
  if (pig.skipLiteral (literal))
  {
    // Create a populated branch.
    auto b = std::make_shared <Tree> ();
    b->_name = "stringLiteral";
    b->attribute ("expected", token._token);
    b->attribute ("value", literal);
    parseTree->addBranch (b);

    if (_debug > 1)
      std::cout << "trace " << std::string (indent, ' ') << "[32mmatch[0m " << literal << "\n";
    if (_debug)
      std::cout << "trace " << pig.dump () << ' ' << token.dump () << "\n";
    return true;
  }

  if (_debug > 1)
    std::cout << "trace " << std::string (indent, ' ') << "[31mfail[0m " << token._token << "\n";
  pig.restoreTo (checkpoint);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// Search for 'value' in _entities category, return canonicalized value.
bool Packrat::canonicalize (
  std::string& canonicalized,
  const std::string& category,
  const std::string& value) const
{
  // Extract a list of entities for category.
  std::vector <std::string> options;
  auto c = _entities.equal_range (category);
  for (auto e = c.first; e != c.second; ++e)
  {
    // Shortcut: if an exact match is found, success.
    if (value == e->second)
    {
      canonicalized = value;
      return true;
    }

    options.push_back (e->second);
  }

  // Match against the options, throw away results.
  std::vector <std::string> matches;
  if (autoComplete (value, options, matches, minimumMatchLength) == 1)
  {
    canonicalized = matches[0];
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
std::string Packrat::dump () const
{
  std::stringstream out;
  if (_debug)
    out << '\n';

  out << "Packrat Parse "
      << _tree->dump ();

  if (_entities.size ())
  {
    out << "  Entities\n";
    for (const auto& entity : _entities)
      out << "    " << entity.first << ':' << entity.second << '\n';
  }

  if (_externals.size ())
  {
    out << "  Externals\n";
    for (const auto& external : _externals)
      out << "    " << external.first << "\n";
  }

  return out.str ();
}

////////////////////////////////////////////////////////////////////////////////
