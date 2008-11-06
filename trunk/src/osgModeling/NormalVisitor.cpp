/* -*-c++-*- osgModeling - Copyright (C) 2008 Wang Rui <wangray84@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.

* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.

* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <iostream>
#include <algorithm>
#include <osg/TriangleFunctor>
#include <osgModeling/Utilities>
#include <osgModeling/Model>
#include <osgModeling/NormalVisitor>

using namespace osgModeling;

struct LessPtr
{
    inline bool operator() ( const osg::Vec3* lhs, const osg::Vec3* rhs ) const
    {
        return *lhs < *rhs;
    }
};

struct CalcNormalFunctor
{
    typedef std::multiset<const osg::Vec3*, LessPtr> CoordinateSet;
    unsigned int _coordSize;
    CoordinateSet _coordSet;
    osg::Vec3* _coordBase;

    // Normal calculating variables & functions.
    bool _flip;
    osg::Vec3* _normalBase;

    void setNormalPtr( osg::Vec3* nb, bool flip )
    {
        _normalBase = nb;
        _flip = flip;
    }

    inline void incNormal( const osg::Vec3& vec, const osg::Vec3& normal )
    {
        std::pair<CoordinateSet::iterator, CoordinateSet::iterator> p =
            _coordSet.equal_range( &vec );

        for ( CoordinateSet::iterator itr=p.first; itr!=p.second; ++itr )
        {
            osg::Vec3* nptr = _normalBase + (*itr - _coordBase);
            *nptr += normal;
        }
    }

    // General functions.
    CalcNormalFunctor():
        _coordSize(0), _coordBase(0)
    {}

    void setVerticsPtr( osg::Vec3* cb, unsigned int cs )
    {
        _coordSize = cs;
        _coordBase = cb;

        osg::Vec3* vptr = cb;
        for ( unsigned int i=0; i<cs; ++i )
            _coordSet.insert( vptr++ );
    }

    inline void operator() ( const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, bool treatVertexDataAsTemporary )
    {
        if ( treatVertexDataAsTemporary )
            return;

        osg::Vec3 normal = (v2-v1)^(v3-v1) * (_flip?-1.0:1.0);
        incNormal( v1, normal );
        incNormal( v2, normal );
        incNormal( v3, normal );
    }
};

NormalVisitor::NormalVisitor()
{
    setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
}

NormalVisitor::~NormalVisitor()
{
}

bool NormalVisitor::checkPrimitives( osg::Geometry& geom )
{
    osg::Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    osg::Geometry::PrimitiveSetList::iterator itr;
    unsigned int numSurfacePrimitives=0;
    for ( itr=primitives.begin(); itr!=primitives.end(); ++itr )
    {
        switch ( (*itr)->getMode() )
        {
        case (osg::PrimitiveSet::TRIANGLES):
        case (osg::PrimitiveSet::TRIANGLE_STRIP):
        case (osg::PrimitiveSet::TRIANGLE_FAN):
        case (osg::PrimitiveSet::QUADS):
        case (osg::PrimitiveSet::QUAD_STRIP):
        case (osg::PrimitiveSet::POLYGON):
            ++numSurfacePrimitives;
            break;
        default:
            break;
        }
    }

    if ( !numSurfacePrimitives ) return false;
    return true;
}

void NormalVisitor::buildNormal( osg::Geometry& geom, bool flip )
{
    if ( !checkPrimitives(geom) ) return;

    osg::Vec3Array *coords = dynamic_cast<osg::Vec3Array*>( geom.getVertexArray() );
    if ( !coords || !coords->size() ) return;

    osg::Vec3Array::iterator nitr;
    osg::Vec3Array *normals = new osg::Vec3Array( coords->size() );
    for ( nitr=normals->begin(); nitr!=normals->end(); ++nitr )
    {
        nitr->set( 0.0f, 0.0f, 0.0f );
    }

    osg::TriangleFunctor<CalcNormalFunctor> ctf;
    ctf.setVerticsPtr( &(coords->front()), coords->size() );
    ctf.setNormalPtr( &(normals->front()), flip );
    geom.accept( ctf );

    for ( nitr=normals->begin(); nitr!=normals->end(); ++nitr )
    {
        nitr->normalize();
    }

    geom.setNormalArray( normals );
    geom.setNormalIndices( geom.getVertexIndices() );
    geom.setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
}

void NormalVisitor::apply(osg::Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
      osg::Geometry* geom = dynamic_cast<osg::Geometry*>( geode.getDrawable(i) );
      if ( geom )
      	buildNormal( *geom, false );
    }
}
