libshared
=========
A git submodule used by multiple projects. Not to be released independently.


Branching
---------

Development for the common library ("libshared") is done on the master branch.
Once the first product is release using the common library, a '1.0.0' branch will
be made. When an incompatible change is introduced to the common library, a
'2.0.0' branch will be made. 

This allows client programs to choose when to upgrade from '1.0.0' to '2.0.0',
which by definition will require client code changes.
