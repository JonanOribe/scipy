.. _distributing-a-release:

Distributing
============

Distributing Python packages is nontrivial - especially for a package with
complex build requirements like SciPy - and subject to change.  For an up-to-date
overview of recommended tools and techniques, see the `Python Packaging User
Guide`_.  This document discusses some of the main issues and considerations for
SciPy.

Dependencies
------------
Dependencies are things that a user has to install in order to use (or
build/test) a package.  They usually cause trouble, especially if they're not
optional.  SciPy tries to keep its dependencies to a minimum; currently they
are:

*Unconditional run-time dependencies:*

- Numpy_

*Conditional run-time dependencies:*

- pytest (to run the test suite)
- asv_ (to run the benchmarks)
- matplotlib_ (for some functions that can produce plots)
- Pillow_ (for image loading/saving)
- scikits.umfpack_ (optionally used in ``sparse.linalg``)
- mpmath_ (for more extended tests in ``special``)
- pydata/sparse  (compatibility support in ``scipy.sparse``)

*Unconditional build-time dependencies:*

- Numpy_
- A BLAS and LAPACK implementation (reference BLAS/LAPACK, ATLAS, OpenBLAS,
  MKL are all known to work)
- Cython_
- setuptools_
- pybind11_

*Conditional build-time dependencies:*

- wheel_ (``python setup.py bdist_wheel``)
- Sphinx_ (docs)
- `PyData Sphinx theme`_ (docs)
- `Sphinx-Panels`_  (docs)
- matplotlib_ (docs)
- LaTeX (pdf docs)
- Pillow_ (docs)

Furthermore of course one needs C, C++ and Fortran compilers to build SciPy,
but those we don't consider to be dependencies and are therefore not discussed
here.  For details, see https://scipy.github.io/devdocs/building/.

When a package provides useful functionality and it's proposed as a new
dependency, consider also if it makes sense to vendor (i.e. ship a copy of it with
scipy) the package instead.  For example, decorator_ is vendored in
``scipy._lib``.

The only dependency that is reported to pip_  is Numpy_, see
``install_requires`` in SciPy's main ``setup.py``.  The other dependencies
aren't needed for SciPy to function correctly

Issues with dependency handling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
There are some issues with how Python packaging tools handle
dependencies reported by projects.  Because SciPy gets regular bug reports
about this, we go in a bit of detail here.

SciPy only reports its dependency on NumPy via ``install_requires`` if NumPy
isn't installed at all on a system, or when building wheels with
``bdist_wheel``. SciPy no longer uses ``setup_requires`` (which in the past
invoked ``easy_install``); build dependencies are now handled only via
``pyproject.toml``. ``pyproject.toml`` relies on PEP 517; ``pip`` has
``--no-use-pep517`` and ``--no-build-isolation`` flags that may ignore
``pyproject.toml`` or treat it differently - if users use those flags, they
are responsible for installing the correct build dependencies themselves.


.. _numpy-version-ranges:

Version ranges for NumPy and other dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
For dependencies it's important to set lower and upper bounds on their
versions. For *build-time* dependencies, they are specified in
``pyproject.toml`` and the versions will _only_ apply to the SciPy build
itself. It's fine to specify either a range or a specific version for a
dependency like ``wheel`` or ``setuptools``. For NumPy we have to worry
about ABI compatibility too, hence we specify the version with ``==``
to the lowest supported version (because NumPy's ABI is backward but not
forward compatible).

For *run-time dependencies* (currently only ``numpy``), we specify the range
of versions in ``install_requires`` in ``setup.py``. Getting the upper bound
right is slightly tricky.  If we don't set any bound, a too-new version
will be pulled in a few years down the line, and NumPy may have deprecated and
removed some API that SciPy depended on by then. On the other hand if we set
the upper bound to the newest already-released version, then as soon as a new
NumPy version is released there will be no matching SciPy version that works
with it. Given that NumPy and SciPy both release in a 6-monthly cadence and
that features that get deprecated in NumPy should stay around for another two
releases, we specify the upper bound as ``<1.xx+3.0`` (where ``xx`` is the
minor version of the latest already-released NumPy.


.. _supported-py-numpy-versions:

Supported Python and NumPy versions
-----------------------------------
The Python_ versions that SciPy supports are listed in the list of PyPI
classifiers in ``setup.py``, and mentioned in the release notes for each
release.  All newly released Python versions will be supported as soon as
possible.  For the general policy on dropping support for a Python or NumPy
version, see :ref:`NEP 29 <NEP29>`.  The final decision on dropping support is
always taken on the scipy-dev mailing list.

The lowest supported Numpy_ version for a SciPy version is mentioned in the
release notes and is encoded in ``pyproject.toml``, ``scipy/__init__.py`` and the
``install_requires`` field of ``setup.py``.  Typically the latest SciPy release
supports ~5-7 minor versions of NumPy: up to 2.5 years' old NumPy versions,
(given that the frequency of NumPy releases is about 2x/year at the time of
writing) plus two versions into the future.

Supported versions of optional dependencies and compilers is documented in
:ref:`toolchain-roadmap`. Note that not all versions of optional dependencies
that are supported are tested well or at all by SciPy's Continuous
Integration setup.  Issues regarding this are dealt with as they come up in the
issue tracker or mailing list.


Building binary installers
--------------------------
.. note::

   This section is only about building SciPy binary installers to *distribute*.
   For info on building SciPy on the same machine as where it will be used, see
   `this scipy.org page <https://scipy.github.io/devdocs/building/>`_.

There are a number of things to take into consideration when building binaries
and distributing them on PyPI or elsewhere.

**General**

- A binary is specific for a single Python version (because different Python
  versions aren't ABI-compatible, at least up to Python 3.4).
- Build against the lowest NumPy version that you need to support, then it will
  work for all NumPy versions with the same major version number (NumPy does
  maintain backwards ABI compatibility).

**Windows**

- The currently most easily available toolchain for building
  Python.org compatible binaries for SciPy is installing MSVC (see
  https://wiki.python.org/moin/WindowsCompilers) and mingw64-gfortran.
  Support for this configuration requires numpy.distutils from
  NumPy >= 1.14.dev and a gcc/gfortran-compiled static ``openblas.a``.
  This configuration is currently used in the Appveyor configuration for
  https://github.com/MacPython/scipy-wheels
- For 64-bit Windows installers built with a free toolchain, use the method
  documented at https://github.com/numpy/numpy/wiki/Mingw-static-toolchain.
  That method will likely be used for SciPy itself once it's clear that the
  maintenance of that toolchain is sustainable long-term.  See the MingwPy_
  project and `this thread
  <https://mail.scipy.org/pipermail/numpy-discussion/2015-October/074056.html>`_ for
  details.
- The other way to produce 64-bit Windows installers is with ``icc``, ``ifort``
  plus ``MKL`` (or ``MSVC`` instead of ``icc``).  For Intel toolchain
  instructions see
  `this article <https://software.intel.com/en-us/articles/numpyscipy-with-intel-mkl>`_
  and for (partial) MSVC instructions see
  `this wiki page <https://github.com/numpy/numpy/wiki/Building-with-MSVC>`_.
- Older SciPy releases contained a .exe "superpack" installer.  Those contain
  3 complete builds (no SSE, SSE2, SSE3), and were built with
  https://github.com/numpy/numpy-vendor.  That build setup is known to not work
  well anymore and is no longer supported.  It used g77 instead of gfortran,
  due to complex DLL distribution issues (see `gh-2829
  <https://github.com/scipy/scipy/issues/2829>`_).  Because the toolchain is no
  longer supported, g77 support isn't needed anymore and SciPy can now include
  Fortran 90/95 code.

**OS X**

- To produce OS X wheels that work with various Python versions (from
  python.org, Homebrew, MacPython), use the build method provided by
  https://github.com/MacPython/scipy-wheels.
- DMG installers for the Python from python.org on OS X can still be produced
  by ``tools/scipy-macosx-installer/``.  SciPy doesn't distribute those
  installers anymore though, now that there are binary wheels on PyPi.

**Linux**

- PyPi-compatible Linux wheels can be produced via the manylinux_ project.
  The corresponding build setup for TravisCI for SciPy is set up in
  https://github.com/MacPython/scipy-wheels.

Other Linux build-setups result to PyPi incompatible wheels, which
would need to be distributed via custom channels, e.g. in a
Wheelhouse_, see at the wheel_ and Wheelhouse_ docs.


.. _Numpy: https://numpy.org
.. _Python: https://www.python.org
.. _nose: https://nose.readthedocs.io
.. _asv: https://asv.readthedocs.org
.. _matplotlib: https://matplotlib.org
.. _Pillow: https://pillow.readthedocs.org
.. _scikits.umfpack: https://pypi.org/project/scikit-umfpack
.. _mpmath: http://mpmath.org
.. _Cython: https://cython.org
.. _pybind11: https://github.com/pybind/pybind11
.. _setuptools: https://bitbucket.org/pypa/setuptools
.. _wheel: https://wheel.readthedocs.io/
.. _pip: https://pip.pypa.io/en/stable/
.. _Python Packaging User Guide: https://packaging.python.org
.. _Wheelhouse: https://pypi.org/project/Wheelhouse
.. _MingwPy: https://mingwpy.github.io
.. _Sphinx: http://www.sphinx-doc.org/
.. _PyData Sphinx theme: https://pydata-sphinx-theme.readthedocs.io/en/latest/
.. _Sphinx-Panels: https://sphinx-panels.readthedocs.io/en/latest/
.. _six: https://pypi.org/project/six
.. _decorator: https://github.com/micheles/decorator
.. _manylinux: https://github.com/pypa/manylinux/
