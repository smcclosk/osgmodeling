# New release #

osgModeling 0.1.1 Released on Nov. 17, 2008.

# Features #

osgModeling 0.1.1 includes these main features:

- Support for 4 kinds of curves:
  * k-degree Bezier curves.
  * k-degree NURBS curves.
  * Helix (3-dimensional spiral curves resembling a spring).
  * And user customized curves.

- Support for 6 kinds of surfaces:
  * m,n-degree Bezier surfaces.
  * m,n-degree NURBS surfaces.
  * Extrusions (constructed by a profile extruded along a path).
  * Revolutions (constructed by a profile rotated specified angles).
  * Lofts (constructed by lofting a series of curves that define the cross section on a specified path).
  * And user customized models.

- Generate normal arrays and texture coordinate arrays for various models (except user customizations).
  * Support 6 methods to generate normals with different weights.
  * Support the normal-flip operation.

- Free to define customized algorithms to created vertices arrays, normal arrays and texture coordinate arrays for own models.

- Construct the polygon mesh structures (Vertices-Edges-Faces) for geometries.

- Subdivide polygon meshes into higher level using different methods.
  * Loop method: Split each face into 4 parts at every level to build subdivisions.
  * Sqrt(3) method: Split each face into 3 parts at every level to build subdivisions.

- Construct the binary space partitioning (BSP) trees for models in built or converted from osg::Geometry.

- Geometric boolean operations (Intersection, Union and Difference) based on BSP trees of models.