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
#include <osgModeling/ModelVisitor>
#include <osgModeling/PolyMesh>
#include <osgModeling/BspTree>

using namespace osgModeling;

struct FindEdgePtr 
{
    PolyMesh::Vertex* _v[2];

    FindEdgePtr( PolyMesh::Vertex* v1, PolyMesh::Vertex* v2 )
    { _v[0]=v1; _v[1]=v2; }

    inline bool operator() ( const PolyMesh::Edge* edge )
    {
        if ( (_v[0]==edge->_v[0] && _v[1]==edge->_v[1])
            || (_v[0]==edge->_v[1] && _v[1]==edge->_v[0]) )
            return true;
        return false;
    }
};

struct CalcTriangleFunctor
{
    ModelVisitor::GeometryTask _task;
    unsigned int _coordSize;
    osg::Vec3* _coordBase;

    // Polymesh building variables & functions.
    PolyMesh::VertexList* _meshVertices;
    PolyMesh::EdgeList* _meshEdges;
    PolyMesh::FaceList* _meshFaces;
    PolyMesh::VertexList _meshVecRecorder;

    void setMeshPtr( PolyMesh::VertexList* vl, PolyMesh::EdgeList* el, PolyMesh::FaceList* fl )
    {
        _meshVertices = vl;
        _meshEdges = el;
        _meshFaces = fl;
        _meshVecRecorder.resize( _coordSize, NULL );
    }

    inline PolyMesh::Vertex* buildMeshVertices( const osg::Vec3& vec, PolyMesh::Face* face )
    {
        unsigned int pos = &vec - _coordBase;
        PolyMesh::Vertex* vertex = _meshVecRecorder[pos];
        if ( !vertex )
        {
            vertex = new PolyMesh::Vertex( pos, &vec );
            _meshVertices->push_back( vertex );
            _meshVecRecorder[pos] = vertex;
        }

        vertex->hasFace( face, true );
        face->hasVertex( vertex, true );
        return vertex;
    }

    inline PolyMesh::Edge* buildMeshEdges( PolyMesh::Vertex* v1, PolyMesh::Vertex* v2, PolyMesh::Face* face )
    {
        PolyMesh::EdgeList::iterator itr =
            std::find_if( _meshEdges->begin(), _meshEdges->end(), FindEdgePtr(v1,v2) );

        PolyMesh::Edge* edge;
        if ( itr==_meshEdges->end() )
        {
            edge = new PolyMesh::Edge( v1, v2 );
            _meshEdges->push_back( edge );
        }
        else edge = *itr;

        v1->hasEdge( edge, true );
        v2->hasEdge( edge, true );
        edge->hasFace( face, true );
        return edge;
    }

    // BSP faces building variables & functions.
    BspTree* _bspTree;

    void setBspPtr( BspTree* bsp )
    {
        _bspTree = bsp;
    }

    // General functions.
    CalcTriangleFunctor():
        _coordSize(0), _coordBase(0)
    {}

    void setTask( ModelVisitor::GeometryTask t ) { _task=t; }

    void setVerticsPtr( osg::Vec3* cb, unsigned int cs )
    {
        _coordSize = cs;
        _coordBase = cb;
    }

    inline void operator() ( const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, bool treatVertexDataAsTemporary )
    {
        if ( treatVertexDataAsTemporary )
            return;
        
        if ( _task==ModelVisitor::BUILD_MESH )
        {
            PolyMesh::Face* face = new PolyMesh::Face;
            _meshFaces->push_back( face );
            PolyMesh::Vertex* pv1 = buildMeshVertices( v1, face );
            PolyMesh::Vertex* pv2 = buildMeshVertices( v2, face );
            PolyMesh::Vertex* pv3 = buildMeshVertices( v3, face );
            buildMeshEdges( pv1, pv2, face );
            buildMeshEdges( pv2, pv3, face );
            buildMeshEdges( pv1, pv3, face );
        }
        else if ( _task==ModelVisitor::BUILD_BSP )
        {
            BspTree::BspFace face;
            face.addPoint( v1 );
            face.addPoint( v2 );
            face.addPoint( v3 );

            bool hasNormal;
            calcNormal( v1, v2, v3, &hasNormal );

            if ( face.valid() && hasNormal )
                _bspTree->addFace( face );
        }
    }
};

ModelVisitor::ModelVisitor()
{
    setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
}

ModelVisitor::~ModelVisitor()
{
}

bool ModelVisitor::checkPrimitives( osg::Geometry& geom )
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

void ModelVisitor::buildBSP( Model& model )
{
    if ( !checkPrimitives(model) ) return;

    BspTree* bsp = model.getBspTree();
    osg::Vec3Array *coords = dynamic_cast<osg::Vec3Array*>( model.getVertexArray() );
    if ( !bsp || !coords || !coords->size() ) return;

    osg::TriangleFunctor<CalcTriangleFunctor> ctf;
    ctf.setTask( BUILD_BSP );
    ctf.setVerticsPtr( &(coords->front()), coords->size() );
    ctf.setBspPtr( bsp );
    model.accept( ctf );

    bsp->buildBspTree();
}

void ModelVisitor::buildMesh( PolyMesh& mesh )
{
    if ( !checkPrimitives(mesh) ) return;

    osg::Vec3Array *coords = dynamic_cast<osg::Vec3Array*>( mesh.getVertexArray() );
    if ( !coords || !coords->size() ) return;

    osg::TriangleFunctor<CalcTriangleFunctor> ctf;
    ctf.setTask( BUILD_MESH );
    ctf.setVerticsPtr( &(coords->front()), coords->size() );
    ctf.setMeshPtr( &(mesh._vertices), &(mesh._edges), &(mesh._faces) );
    mesh.accept( ctf );

    // Test
    /*int i=0;
    for ( PolyMesh::EdgeList::iterator itr=mesh._edges.begin();
        itr!=mesh._edges.end();
        ++itr, ++i )
    {
        PolyMesh::Edge* edge = *itr;
        std::cout << "Edge " << i << ": ";
        std::cout << "[" << edge->_faces.size() << "](" << edge->_v[0]->_index
            << ", " << edge->_v[1]->_index << ")";

        for ( PolyMesh::VertexList::iterator vitr=face->_vertices.begin();
            vitr!=face->_vertices.end();
            ++vitr )
        {
            PolyMesh::Vertex* vertex = *vitr;
            osg::Vec3 vec = (*coords)[vertex->_index];
            std::cout << "[" << vertex->_index << "](" <<
                vec.x() << ", " << vec.y() << ", " << vec.z() << ")";
            std::cout << " & " << vertex->_faces.size() << "F; ";
        }
        std::cout << std::endl;
    }*/
}

void ModelVisitor::apply(osg::Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
        if ( _task==BUILD_BSP )
        {
            Model* model = dynamic_cast<Model*>( geode.getDrawable(i) );
            if ( model ) buildBSP( *model );
        }
        else if ( _task==BUILD_MESH )
        {
            PolyMesh* mesh = dynamic_cast<PolyMesh*>( geode.getDrawable(i) );
            if ( mesh ) buildMesh( *mesh );
        }
    }
}
