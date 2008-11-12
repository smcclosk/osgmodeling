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
#include <osgModeling/Utilities>
#include <osgModeling/Subdivision>
#include <osgModeling/PolyMesh>
#include <osgModeling/ModelVisitor>
#include <osgModeling/NormalVisitor>

using namespace osgModeling;

PolyMesh::Edge::Edge( osg::Vec3 v1, osg::Vec3 v2, int f ):
    _flag(f)
{
    _v[0] = v1;
    _v[1] = v2;
}

PolyMesh::EdgeType PolyMesh::Edge::getType()
{
    if ( _v[0]==_v[1] ) return PolyMesh::INVALID_EDGE;

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

PolyMesh::Face::Face( osg::Vec3Array* array, int p1, int p2, int p3, int f ):
    _array(array), _flag(f)
{
    _pts.push_back( p1 );
    _pts.push_back( p2 );
    _pts.push_back( p3 );
}

PolyMesh::Face::Face( osg::Vec3Array* array, PolyMesh::VertexIndexList pts, int f ):
    _array(array), _flag(f)
{
    _pts.insert( _pts.end(), pts.begin(), pts.end() );
}

osg::Vec3 PolyMesh::Face::operator-( PolyMesh::Edge e )
{
    for ( PolyMesh::VertexIndexList::iterator itr=_pts.begin(); itr!=_pts.end(); ++itr )
    {
        osg::Vec3 v = (*_array)[*itr];
        if ( v!=e[0] && v!=e[1] ) return v;
    }
    return osg::Vec3(0.0f,0.0f,0.0f);
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

PolyMesh::Edge* PolyMesh::getEdge( osg::Vec3 p1, osg::Vec3 p2 )
{
    Segment p( p1, p2 );
    if ( p2<p1 )
    {
        p.first = p2;
        p.second = p1;
    }

    if ( _edges.find(p)==_edges.end() ) return NULL;
    else return _edges[p];
}

void PolyMesh::findEdgeList( osg::Vec3 p, EdgeList& elist )
{
    for ( EdgeMap::iterator itr=_edges.begin(); itr!=_edges.end(); ++itr )
    {
        if ( itr->first.first==p || itr->first.second==p )
            elist.push_back( itr->second );
    }
}

void PolyMesh::findEdgeList( Face* f, EdgeList& elist )
{
    unsigned int size = f->_pts.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        Edge* edge = getEdge( (*f)[i%size], (*f)[(i+1)%size] );
        if ( edge ) elist.push_back( edge );
    }
}

void PolyMesh::findNeighbors( osg::Vec3 p, VertexList& vlist )
{
    for ( EdgeMap::iterator itr=_edges.begin(); itr!=_edges.end(); ++itr )
    {
        if ( equivalent(itr->first.first,p) )
            vlist.push_back( itr->first.second );
        else if ( equivalent(itr->first.second,p) )
            vlist.push_back( itr->first.first );
    }
}

void PolyMesh::findNeighbors( Face* f, FaceList& flist )
{
    unsigned int size = f->_pts.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        Edge* edge = getEdge( (*f)[i%size], (*f)[(i+1)%size] );
        if ( !edge ) continue;
        
        for ( FaceList::iterator itr=edge->_faces.begin();
            itr!=edge->_faces.end();
            ++itr )
        {
            if ( *itr!=f )
                flist.push_back( *itr );
        }
    }
}

bool PolyMesh::convertFacesToGeometry( FaceList faces, osg::Geometry* geom )
{
    if ( !faces.size() || !geom ) return false;

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES, 0 );

    unsigned int i, index=0, firstIndex=0, lastIndex=0;
    for ( FaceList::iterator itr=faces.begin();
        itr!=faces.end();
        ++itr )
    {
        Face f = *(*itr);
        unsigned int size=f._pts.size();
        for ( i=0; i<size; ++i )
        {
            if ( i>2 )
            {
                indices->push_back( firstIndex );
                indices->push_back( lastIndex );
            }

            indices->push_back( f(i) );

            if ( !i ) firstIndex = indices->back();
            lastIndex = indices->back();
        }
    }

    geom->removePrimitiveSet( 0, geom->getPrimitiveSetList().size() );
    geom->addPrimitiveSet( indices.get() );
    NormalVisitor::buildNormal( *geom );
    geom->dirtyDisplayList();
    return true;
}

void PolyMesh::buildEdges( Face* f, EdgeMap& emap )
{
    unsigned int size = f->_pts.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Vec3 p1=(*f)[i%size], p2=(*f)[(i+1)%size];
        PolyMesh::Segment p( p1, p2 );
        if ( p2<p1 )
        {
            p.first = p2;
            p.second = p1;
        }

        if ( emap.find(p)==emap.end() )
            emap[p] = new PolyMesh::Edge( p.first, p.second );
        emap[p]->hasFace( f, true );
    }
}
