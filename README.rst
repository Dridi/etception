etception
=========

*n.* **etc·ep·tion**

1. an act or instance of etcepting.
2. the pre-load of ``libetcept`` when running a program.
3. the ability not to be constrained by global configuration.

This is ``libetcept``, a simple library meant for test suites struggling with
global configuration. It is similar to the cwrap_ project, but has different
goals and a much reduced scope. Instead of wrapping specific sets of APIs,
``libetcept`` will try to intercept accesses to any files located in ``/etc``.

.. _cwrap: https://cwrap.org

It works on dynamically linked programs using the preload capability of the
linker, in order to intercept a set of core functions relying directly or
implicitly on files. For example, ``getaddrinfo(3)`` may read ``/etc/hosts``
for domain resolution, leaking outside of a test suite involving networking.
When a file is being accessed inside ``/etc`` the pre-loaded ``libetcept``
looks for one in the process' working directory. You could in the case of
``getaddrinfo(3)`` use your own hosts file in ``${PWD}/etc/hosts``.

For standard system files, cwrap_ is most likely a better option, and already
capable of isolating you from system-wide configuration. For tighter tests
isolation, other solutions might be appropriate such as virtual machines,
namespaces on Linux, jails on FreeBSD etc. ``libetcept`` is meant to be a
small, simple, privilege-free and self-contained solution for system-wide
configuration interception.

Tutorial
========

It should be straightforward to use::

    $ make
    [...skipping output...]
    $ mkdir -p etc
    $ echo '::42 localhost' >etc/hosts
    $ export LD_PRELOAD=${PWD}/libetcept.so
    $ getent hosts localhost
    ::42            localhost

If ``libetcept`` is installed where your linker will expect it, you should
only need this much::

    $ mkdir -p etc
    $ echo '::42 localhost' >etc/hosts
    $ export LD_PRELOAD=libetcept.so
    $ getent hosts localhost
    ::42            localhost

On multi-lib systems, installing both the 32-bit and 64-bit builds of the
library, the example above should work with both kinds of programs. On a
64-bit system you would otherwise not be able to run 32-bit programs with
an absolute path in ``LD_PRELOAD``. Some systems support ``LD_PRELOAD_32``
and ``LD_PRELOAD_64`` environment variables that could also help in that
regard.

Portability
===========

Portability is currently not a hard requirement, and contributions are welcome
in this area. It is currently tested with the following components:

* ``gmake`` (GNU make)
* ``bmake`` (BSD make)
* GCC
* Clang
* GNU/Linux (Linux kernel, ``glibc``, GNU linker)
* ELF binaries

It may be more portable, but that's how far testing went so far. In particular
etception of functions wrapping system calls can be hidden compiler *"magic"*
that can be hard to discover without testing on the actual platforms.

Known limitations
=================

*Some of those limitations are intentional.*

No thorough research has been done yet regarding missing functions that need
etception.

Only regular files are intercepted, not directories. Deciding how to handle
symbolic links will need further testing. In order to keep the library simple,
etception happens only inside the current working directory, which is not
ideal when different processes may run in different working directories.

Only absolute paths starting with ``/etc/`` are etcepted, and paths containing
``.`` or ``..`` components aren't. Programs or script changing or clearing
their environments before executing other programs may prevent ``libetcept``
to be pre-loaded further downstream. Nothing is currently done to prevent it.

An etception occurs only if a file exists in the working directory: creating
a file in ``/etc`` would likely be attempted in ``/etc``. This use case is
a non goal for ``libetcept``.

The initialization of the library is racy. The severity is low, but still...
racy.

And obviously, statically-linked binaries would be impervious to pre-loading.

Contributing
============

Contributions of any kind, including feedback in successful or failed usage
of etception, are most welcome.
