Boost.Actor
=======

Boost.Actor is an LGPL C++11 actor model implementation featuring lightweight & fast
actor implementations, pattern matching for messages,
network transparent messaging, and more.


Get the Sources
---------------

* git clone https://github.com/Neverlord/boost.actor.git
* cd boost.actor


First Steps
-----------

* ./configure
* make
* make install [as root, optional]

It is recommended to run the unit tests as well.

* make test

Please submit a bug report that includes (a) your compiler version, (b) your OS,
and (c) the content of the file build/Testing/Temporary/LastTest.log
if an error occurs.


Dependencies
------------

* CMake
* Boost
* Pthread (until C++11 compilers support the new `thread_local` keyword)


Supported Compilers
-------------------

* GCC >= 4.7
* Clang >= 3.2


Supported Operating Systems
---------------------------

* Linux
* Mac OS X
* Windows (currently only via MinGW)
