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
#include <osgModeling/Utilities>
#include <osgModeling/Subdivision>

using namespace osgModeling;

void Subdivision::operator()( PolyMesh* mesh )
{
    subdivide( mesh );
    PolyMesh::convertFacesToGeometry( mesh->_faces, mesh );
}

LoopSubdivision::LoopSubdivision():
    Subdivision()
{
}

LoopSubdivision::LoopSubdivision( const LoopSubdivision& copy, const osg::CopyOp& copyop/*=osg::CopyOp::SHALLOW_COPY*/ ):
    Subdivision(copy, copyop)
{
}

LoopSubdivision::~LoopSubdivision()
{
}

void LoopSubdivision::subdivide( PolyMesh* mesh )
{
    if ( !mesh || !mesh->_faces.size() ) return;

    PolyMesh::MeshType type = mesh->getType();
    if ( type==PolyMesh::INVALID_MESH || type==PolyMesh::NONMANIFOLD_MESH )
        return;

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>( mesh->getVertexArray() );
    if ( !vertices || !vertices->size() ) return;
    unsigned int ptNum = vertices->size();

    // Split edges to build subdivision points
    subdivideFace( mesh, mesh->_faces.front(), vertices );

    // Reset current vertices to build subdivision points
    subdivideVertices( mesh, vertices, ptNum );

    mesh->destroyMesh();
    mesh->_edges.swap( _tempEdges );
    mesh->_faces.swap( _tempFaces );
    _edgeVertices.clear();
    _tempEdges.clear();
    _tempFaces.clear();
}

void LoopSubdivision::subdivideVertices( PolyMesh* mesh, osg::Vec3Array* pts, unsigned int ptNum )
{
    for ( unsigned int i=0; i<ptNum; ++i )
    {
        PolyMesh::VertexList vlist;
        osg::Vec3 vec = (*pts)[i];
        mesh->findNeighbors( vec, vlist );

        unsigned int size = vlist.size();
        if ( size>2 )
        {
            osg::Vec3 summaryVec( 0.0f, 0.0f, 0.0f );
            for ( PolyMesh::VertexList::iterator itr=vlist.begin(); itr!=vlist.end(); ++itr )
                summaryVec += *itr;

            double beta = (size==3)?0.1875f:(0.375f/size);
            (*pts)[i] = vec*(1-size*beta) + summaryVec*beta;
        }
        else if ( size==2 )
        {
            (*pts)[i] = vec*0.75f + (vlist[0]+vlist[1])*0.125f;
        }
    }
}

void LoopSubdivision::subdivideFace( PolyMesh* mesh, PolyMesh::Face* f, osg::Vec3Array* pts )
{
    if ( !f || f->_flag ) return;

    int evIndex[3] = {0}; // Only for triangles
    osg::Vec3 edgeVertex[3];
    PolyMesh::EdgeList edges;
    PolyMesh::EdgeList::iterator itr;
    mesh->findEdgeList( f, edges );

    int i=0;
    for ( itr=edges.begin(); itr!=edges.end(), i<3; ++i, ++itr )
    {
        PolyMesh::Edge* edge = *itr;
        if ( _edgeVertices.find(edge)!=_edgeVertices.end() )
        {
            evIndex[i] = _edgeVertices[edge];
            edgeVertex[i] = pts->at( evIndex[i] );
        }
        else
        {
            PolyMesh::FaceList fl = edge->_faces;
            if ( fl.size()<2 )
            {
                // Boundary edges
                edgeVertex[i] = (*edge)[0]*0.5f + (*edge)[1]*0.5f;
            }
            else
            {
                // Interior edges
                osg::Vec3 v1=*(fl[0])-(*edge), v2=*(fl[1])-(*edge);
                edgeVertex[i] = ((*edge)[0]+(*edge)[1])*0.375f + (v1+v2)*0.125f;
            }

            pts->push_back( edgeVertex[i] );
            evIndex[i] = pts->size()-1;
            _edgeVertices[edge] = evIndex[i];
        }
    }

    // Construct new edges and triangles
    PolyMesh::Face* newFace[4];
    newFace[0] = new PolyMesh::Face( f->_array, (*f)(0), evIndex[0], evIndex[2] );
    newFace[1] = new PolyMesh::Face( f->_array, (*f)(1), evIndex[1], evIndex[0] );
    newFace[2] = new PolyMesh::Face( f->_array, (*f)(2), evIndex[2], evIndex[1] );
    newFace[3] = new PolyMesh::Face( f->_array, evIndex[0], evIndex[1], evIndex[2] );
    for ( i=0; i<4; ++i )
    {
        PolyMesh::buildEdges( newFace[i], _tempEdges );
        _tempFaces.push_back( newFace[i] );
    }

    // Traverse to neighbor faces
    f->_flag = 1;   // Sign this face as 'visited'
    PolyMesh::FaceList flist;
    mesh->findNeighbors( f, flist );
    for ( PolyMesh::FaceList::iterator fitr=flist.begin(); fitr!=flist.end(); ++fitr )
        subdivideFace( mesh, *fitr, pts );
}
