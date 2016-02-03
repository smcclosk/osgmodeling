# New release #

osgModeling 0.1.0 Released on Nov. 4, 2008. This is the first developer release of this library.

# Features #

osgModeling 0.1.0 includes these main features:

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