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

using namespace osgModeling;

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

void LoopSubdivision::operator()( PolyMesh* mesh )
{
    /*if ( !mesh ) return;

    PolyMesh::MeshType type = mesh->getType();
    if ( type==PolyMesh::INVALID_MESH || type==PolyMesh::NONMANIFOLD_MESH )
        return;

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>( mesh->getVertexArray() );
    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>( mesh->getNormalArray() );

    PolyMesh::VertexList subdividedVectices;
    for ( PolyMesh::VertexList::iterator itr=mesh->_vertices.begin();
        itr!=mesh->_vertices.end();
        ++itr )
    {
        PolyMesh::Vertex* vec = *itr;
        osg::Vec3 currValue = (*vertices)[vec->get()];

        osg::Vec3 allNeighborValue = osg::Vec3( 0.0f, 0.0f, 0.0f );
        unsigned int neighborCount = vec->_edges.size();
        double beta = 0.1875f;
        if ( neighborCount>3 ) beta = 0.375f / (double)vec->_edges.size();

        // Calculate new vertex and new edge points
        PolyMesh::VertexList neighbors;
        for ( PolyMesh::EdgeList::iterator itr=vec->_edges.begin(); itr!=vec->_edges.end(); ++itr )
        {
            PolyMesh::Edge* edge = *itr;
            if ( (*edge)[0]==vec->get() ) neighbors.push_back( edge->_v[1] );
            else neighbors.push_back( edge->_v[0] );
            osg::Vec3 currEdgePointValue = (*vertices)[ neighbors.back()->get() ];
            allNeighborValue += currEdgePointValue;
        }

        // Push new vertex into vertex array
        (*vertices)[vec->get()] = currValue * (1-neighborCount*beta) + allNeighborValue * beta;

        subdividedVectices.push_back( vec );
    }*/
}

void LoopSubdivision::subdivideFace( PolyMesh::Face* f, osg::Vec3Array* pts )
{
/*    for ( PolyMesh::EdgeList::iterator itr=f->_edges.begin(); itr!=f->_edges.end(); ++itr )
    {
        PolyMesh::Edge* edge = *itr;
        PolyMesh::Vertex* newPt=NULL;
        if ( _edgePointMap.find(edge)!=_edgePointMap.end() )
            newPt = _edgePointMap[edge];
        else
        {
            // Calculate a new edge point and add it to the vertex array
            PolyMesh::VertexList vlist;
            for ( PolyMesh::FaceList::iterator fitr=edge->_faces.begin(); fitr!=edge->_faces.end(); ++fitr )
            {
                PolyMesh::Vertex* v = *(*fitr) - edge;
                vlist.push_back( v );
            }
            if ( vlist.size()>1 )
            {
                pts->push_back(
                    (pts->at((*edge)[0]) + pts->at((*edge)[1])) * 0.375
                    + (pts->at(vlist[0]->get()) + pts->at(vlist[1]->get())) * 0.125 );
            }
            else
            {
                pts->push_back( (pts->at((*edge)[0]) + pts->at((*edge)[1])) * 0.5 );
            }
            newPt = new PolyMesh::Vertex( pts->size()-1 );
            _edgePointMap[edge] = newPt;
        }
    }*/
}
