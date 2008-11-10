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
#include <osgModeling/Subdivision>
#include <osgModeling/PolyMesh>
#include <osgModeling/ModelVisitor>

using namespace osgModeling;

PolyMesh::Edge::Edge( int v1, int v2, int f ):
    _flag(f)
{
    _v[0] = v1;
    _v[1] = v2;
}

PolyMesh::EdgeType PolyMesh::Edge::getType()
{
    if ( _v[0]<0 || _v[1]<0 || _v[0]==_v[1] ) return PolyMesh::INVALID_EDGE;

    switch ( _faces.size() )
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

    if ( addIfNotFound )
        _faces.push_back( f );
    return true;
}

PolyMesh::Face::Face( int p1, int p2, int p3, int f ):
    _flag(f)
{
    _pts.push_back( p1 );
    _pts.push_back( p2 );
    _pts.push_back( p3 );
}

PolyMesh::Face::Face( VertexList pts, int f ):
    _flag(f)
{
    _pts.insert( _pts.end(), pts.begin(), pts.end() );
}

int PolyMesh::Face::operator-( PolyMesh::Edge e )
{
    for ( PolyMesh::VertexList::iterator itr=_pts.begin(); itr!=_pts.end(); ++itr )
    {
        if ( *itr!=e[0] && *itr!=e[1] ) return *itr;
    }
    return -1;
}

PolyMesh::PolyMesh():
    osg::Geometry()
{
}

PolyMesh::PolyMesh( const osg::Geometry& copy, const osg::CopyOp& copyop ):
    osg::Geometry(copy,copyop)
{
    ModelVisitor::buildMesh( *this );
}

PolyMesh::PolyMesh( const PolyMesh& copy, const osg::CopyOp& copyop ):
    osg::Geometry(copy,copyop),
    _edges(copy._edges), _faces(copy._faces)
{
}

PolyMesh::~PolyMesh()
{
    destroyMesh();
}

PolyMesh::MeshType PolyMesh::getType()
{
    bool closed=true;
    for ( EdgeMap::iterator itr=_edges.begin(); itr!=_edges.end(); ++itr )
    {
        EdgeType edgeType = itr->second->getType();
        if ( edgeType==INVALID_EDGE ) return INVALID_MESH;
        else if ( edgeType==JUNCTION_EDGE ) return NONMANIFOLD_MESH;
        else if ( edgeType==BORDER_EDGE ) closed = false;
    }
    return (closed?CLOSED_MESH:OPEN_MESH);
}

void PolyMesh::destroyMesh()
{
    for ( EdgeMap::iterator itr=_edges.begin(); itr!=_edges.end(); )
    {
        if ( !(itr->second) ) ++itr;
        else
        {
            delete itr->second; itr->second = 0;
            itr = _edges.erase( itr );
        }
    }

    for ( FaceList::iterator itr=_faces.begin(); itr!=_faces.end(); )
    {
        if ( !(*itr) ) ++itr;
        else
        {
            delete *itr; *itr = 0;
            itr = _faces.erase( itr );
        }
    }
}

void PolyMesh::subdivide( Subdivision* subd )
{
    if ( !subd ) return;
    (*subd)( this );
}

PolyMesh::Edge* PolyMesh::getEdge( int p1, int p2 )
{
    Segment p( p1, p2 );
    if ( p1>p2 )
    {
        p.first = p2;
        p.second = p1;
    }

    if ( _edges.find(p)==_edges.end() ) return NULL;
    else return _edges[p];
}

PolyMesh::EdgeList PolyMesh::findEdgeList( int p )
{
    EdgeList elist;
    if ( p<0 ) return elist;

    for ( EdgeMap::iterator itr=_edges.begin(); itr!=_edges.end(); ++itr )
    {
        if ( itr->first.first==p || itr->first.second==p )
            elist.push_back( itr->second );
    }
    return elist;
}

PolyMesh::EdgeList PolyMesh::findEdgeList( Face* f )
{
    EdgeList elist;
    if ( !f ) return elist;

    unsigned int size = f->_pts.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        int p1=(*f)[i%size], p2=(*f)[(i+1)%size];
        Edge* edge = getEdge( p1, p2 );
        if ( edge ) elist.push_back( edge );
    }
    return elist;
}

PolyMesh::VertexList PolyMesh::findNeighbors( int p )
{
    VertexList vlist;
    if ( p<0 ) return vlist;

    for ( EdgeMap::iterator itr=_edges.begin(); itr!=_edges.end(); ++itr )
    {
        if ( itr->first.first==p )
            vlist.push_back( itr->first.second );
        else if ( itr->first.second==p )
            vlist.push_back( itr->first.first );
    }
    return vlist;
}

PolyMesh::FaceList PolyMesh::findNeighbors( Face* f )
{
    FaceList flist;
    if ( !f ) return flist;

    unsigned int size = f->_pts.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        int p1=(*f)[i%size], p2=(*f)[(i+1)%size];
        Edge* edge = getEdge( p1, p2 );
        if ( !edge ) continue;
        
        for ( FaceList::iterator itr=edge->_faces.begin();
            itr!=edge->_faces.end();
            ++itr )
        {
            if ( *itr!=f )
                flist.push_back( *itr );
        }
    }
    return flist;
}

void PolyMesh::buildEdges( Face* face )
{
    unsigned int size = face->_pts.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        int p1=(*face)[i%size], p2=(*face)[(i+1)%size];
        PolyMesh::Segment p( p1, p2 );
        if ( p1>p2 )
        {
            p.first = p2;
            p.second = p1;
        }

        if ( _edges.find(p)==_edges.end() )
            _edges[p] = new PolyMesh::Edge( p.first, p.second );
        _edges[p]->hasFace( face, true );
    }
}
