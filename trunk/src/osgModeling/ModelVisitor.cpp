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

struct CalcTriangleFunctor
{
    ModelVisitor::GeometryTask _task;
    unsigned int _coordSize;
    osg::Vec3* _coordBase;

    // Polymesh building variables & functions.
    PolyMesh::EdgeMap* _meshEdges;
    PolyMesh::FaceList* _meshFaces;

    void setMeshPtr( PolyMesh::EdgeMap* em, PolyMesh::FaceList* fl )
    {
        _meshEdges = em;
        _meshFaces = fl;
    }

    inline PolyMesh::Face* buildMeshFace( const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3 )
    {
        int p1=&v1-_coordBase, p2=&v2-_coordBase, p3=&v3-_coordBase;
        return new PolyMesh::Face( p1, p2, p3 );
    }

    inline void buildMeshEdges( PolyMesh::Face* face )
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

            if ( _meshEdges->find(p)==_meshEdges->end() )
                (*_meshEdges)[p] = new PolyMesh::Edge( p.first, p.second );
            (*_meshEdges)[p]->hasFace( face, true );
        }
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
        if ( treatVertexDataAsTemporary || v1==v2 || v1==v3 || v2==v3 )
            return;
        
        if ( _task==ModelVisitor::BUILD_MESH )
        {
            PolyMesh::Face* face = buildMeshFace( v1, v2, v3 );
            _meshFaces->push_back( face );
            buildMeshEdges( face );
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
    ctf.setMeshPtr( &(mesh._edges), &(mesh._faces) );
    mesh.accept( ctf );
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
