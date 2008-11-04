osgModeling is a open source modeling library for OpenSceneGraph(OSG).
Its purpose is to help generate kinds of parametric curves and surfaces,
and calculate vertices, normals and texture coordnates automatically.

For up-to-date information and latest versions of the project, please visit:

    http://code.google.com/p/osgmodeling/

For in-depth details of OpenSceneGraph, please visit its website:

    http://www.openscenegraph.org

===============================
Requirements:
===============================

- OpenSceneGraph: version 2.6.0 or higher

===============================
Features:
===============================

- Support for 4 kinds of curves: k-degree Bezier, k-degree NURBS, helix (3-dimensional
spiral curves resembling a spring.) and user customized curves.

- Support for 6 kinds of surfaces: m,n-degree Bezier, m,n-degree NURBS, extrusions
(constructed by a profile extruded along a path), revolutions (constructed by a profile
rotated specified angles), lofts (constructed by lofting a series of curves that define
the cross section on a specified path) and user customized models.

- Generate normal arrays and texture coordinate arrays for various models (except user
customizations). You may also define customized algorithms to created vertices arrays,
normal arrays and texture coordinate arrays for own models. All models can be added to
geodes and displayed in the OSG scene graph.

- Construct the binary space partitioning (BSP) trees for models created by classes above
or converted from osg::Geometry.

- Boolean operations (Intersection, Union and Difference) based on BSP trees of models.
Vertices data of generated boolean objects will be stored in osg::Geometry instances.

===============================
How to build:
===============================

The osgModeling uses the CMake build system to generate a platform-specific build environment.
If you don't have CMake version 2.4.6 or later installed, you may download it from:
    
    http://www.cmake.org

Under unices, type the following commands:

    ./configure
    make
    sudo make install

Under Windows, just use the CMake GUI tool to build Visual Studio solutions.

===============================
Libraries and examples:
===============================

There is only 1 dynamic library file after installation:

- {libosgModeling.so | osgModeling.dll}: The library.

There are 4 executable files as examples:

- osgmodelingbasic.exe: Demonstrate how to build extrusions, revolutions and lofts with osgModeling
classes (Extrude, Lathe, Loft, Helix and so on).

- osgmodelingboolean.exe: Demonstrate how to create a boolean object from two specified models.

- osgmodelingbsptree.exe: Demonstrate how to build BSP trees for models.

- osgmodelingnurbs.exe: Demonstrate how to create Bezier and NURBS surfaces with osgModeling classes.
