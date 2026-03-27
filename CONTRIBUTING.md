# Contributing to libshared

Code contributions are only accepted as pull-requests.
In general your contributions have to be associated with an issue on our [GitHub issue tracker](https://github.com/GothenburgBitFactory/libshared/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen).

Any code contributions should have sufficient test coverage.

When writing bug-fixes, first write a test to confirm it, then add the commit to fix it.
When implementing a feature, make sure its behavior is defined by tests.

Tests are likely the most useful contributions of all, because they not only improve the quality of the code, but prevent future regressions, therefore maintaining quality of subsequent releases.

Nevertheless, all code has to adhere to the coding style guidelines and project architecture.
See the respective section below.

If you want to go the extra mile, update the ChangeLog and add your name to the AUTHORS file in the right place.

By contributing, you are declaring that you have the right to submit the code under the project licensing terms.
Therefore, commits should be signed off according to the [DCO](DCO) (use `-s` / `--signoff` flag when committing).

## Coding style

The general guideline is simply this:

> Make all changes and additions such that they blend in perfectly with the surrounding code, so it looks like only one person worked on the source, and that person is rigidly consistent.

To be a little more explicit, the common elements across the languages are:

* Indent code using two spaces, no tabs
* With Python, follow PEP8 as much as possible
* Surround operators and expression terms with a space
* No cuddled braces
* No need to stick slavishly to 80 characters per line, but do not make them overly long (rule of thumb: *One sentence/statement per line*)!
* Class names are capitalized, variable names are not

We target Python 3 so that our test suite runs on the broadest set of platforms.

We can safely target C++17 because all the default compilers on our supported platforms are ready.
Feel free to use C++20 provided that all build platforms support this.
