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

#include <algorithm>
#include <osgModeling/PolyMesh>

using namespace osgModeling;

bool PolyMesh::Vertex::hasEdge( PolyMesh::Edge* e, bool addIfNotFound )
{
    if ( !e ) return false;

    PolyMesh::EdgeList::iterator rtn;
    rtn = std::find( _edges.begin(), _edges.end(), e );
    if ( rtn!=_edges.end() ) return false;

    if ( addIfNotFound ) _edges.push_back( e );
    return true;
}

bool PolyMesh::Vertex::hasFace( PolyMesh::Face* f, bool addIfNotFound )
{
    if ( !f ) return false;

    PolyMesh::FaceList::iterator rtn;
    rtn = std::find( _faces.begin(), _faces.end(), f );
    if ( rtn!=_faces.end() ) return false;

    if ( addIfNotFound ) _faces.push_back( f );
    return true;
}

PolyMesh::EdgeType PolyMesh::Edge::getType()
{
    unsigned int size = _faces.size();
    if ( _v[0]<0 || _v[1]<0 || _v[0]==_v[1] ) return PolyMesh::INVALID_EDGE;
    switch ( size )
    {
    case 0: return PolyMesh::INVALID_EDGE;
    case 1: return PolyMesh::BORDER_EDGE;
    case 2: return PolyMesh::MANIFOLD_EDGE;
    default:return PolyMesh::JUNCTION_EDGE;
    }
}

bool PolyMesh::Edge::hasFace( PolyMesh::Face* f, bool addIfNotFound )
{
    if ( !f ) return false;

    PolyMesh::FaceList::iterator rtn;
    rtn = std::find( _faces.begin(), _faces.end(), f );
    if ( rtn!=_faces.end() ) return false;

    if ( addIfNotFound ) _faces.push_back( f );
    return true;
}

bool PolyMesh::Face::hasVertex( Vertex* v, bool addIfNotFound )
{
    if ( !v ) return false;

    PolyMesh::VertexList::iterator rtn;
    rtn = std::find( _vertices.begin(), _vertices.end(), v );
    if ( rtn!=_vertices.end() ) return false;

    if ( addIfNotFound ) _vertices.push_back( v );
    return true;
}

PolyMesh::PolyMesh():
    osg::Geometry()
{
}

PolyMesh::PolyMesh( const osg::Geometry& copy, const osg::CopyOp& copyop ):
    osg::Geometry(copy,copyop)
{
}

PolyMesh::PolyMesh( const PolyMesh& copy, const osg::CopyOp& copyop ):
    osg::Geometry(copy,copyop)
{
}

PolyMesh::~PolyMesh() 
{
    for ( VertexList::iterator itr=_vertices.begin();
        itr!=_vertices.end(); )
    {
        if ( !(*itr) ) ++itr;
        else
        {
            delete *itr; *itr = 0;
            itr = _vertices.erase( itr );
        }
    }

    for ( EdgeList::iterator itr=_edges.begin();
        itr!=_edges.end(); )
    {
        if ( !(*itr) ) ++itr;
        else
        {
            delete *itr; *itr = 0;
            itr = _edges.erase( itr );
        }
    }

    for ( FaceList::iterator itr=_faces.begin();
        itr!=_faces.end(); )
    {
        if ( !(*itr) ) ++itr;
        else
        {
            delete *itr; *itr = 0;
            itr = _faces.erase( itr );
        }
    }
}
