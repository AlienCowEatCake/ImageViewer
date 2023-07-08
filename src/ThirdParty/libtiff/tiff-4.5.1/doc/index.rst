####################################
LibTIFF - TIFF Library and Utilities
####################################

.. table:: References
    :widths: auto

    =====================  =====================================
    Name                   Reference
    =====================  =====================================
    Home Page #1           http://www.simplesystems.org/libtiff/
    Home Page #2           https://libtiff.gitlab.io/libtiff/
    Home Page #3           http://libtiff.maptools.org/
    Latest Stable Release  v4.5.1
    Master Download Site   `download.osgeo.org <https://download.osgeo.org/libtiff/>`_
    Mailing List           `tiff@lists.osgeo.org <tiff@lists.osgeo.org>`_
    List subscription      http://lists.osgeo.org/mailman/listinfo/tiff/
    List archive           http://www.awaresystems.be/imaging/tiff/tml.html
    Source repository      https://gitlab.com/libtiff/libtiff
    =====================  =====================================

The LibTIFF software provides support for the *Tag Image File Format* (TIFF),
a widely used format for storing image data.  The latest version of
the TIFF specification is :doc:`specification/index`.

Included in this software distribution are:

* a library, :file:`libtiff`, for reading and writing TIFF images
* a small collection of tools for performing simple manipulations of TIFF images
* documentation for the library and tools.

The :file:`libtiff` library, along with associated tool programs, should handle most of
your needs for reading and writing TIFF images.

LibTIFF is portable software, supported on various operating systems including
UNIX (Linux, BSD, MacOS X) and Windows.

.. warning::

    libtiff v4.5.1 will be the last one supporting most TIFF tools (except tiffinfo,
    tiffdump, tiffcp and tiffset), whose maintenance will be discontinued, due
    to the lack of contributors able to address reported security issues.
    Starting with libtiff v4.6.0, their source code, at this time ,will still be
    available in the source distribution, but they will no longer be built by
    default, and issues related to them will no longer be accepted in the
    libtiff bug tracker.

The following sections are included in this documentation:

.. toctree::
    :maxdepth: 2
    :titlesonly:

    specification/index
    images
    build
    terms
    libtiff
    multi_page
    functions
    internals
    addingtags
    tools
    contrib
    project/index
    releases/index
