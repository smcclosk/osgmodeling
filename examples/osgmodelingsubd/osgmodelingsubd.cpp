/* -*-c++-*- osgModeling Example: Basic modeling tools
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

#include <osg/Geode>
#include <osg/TexGenNode>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osgModeling/Utilities>
#include <osgModeling/Extrude>
#include <osgModeling/Subdivision>

osg::ref_ptr<osg::Geometry> createGeometry()
{
    osg::ref_ptr<osg::Vec3Array> ptArray = new osg::Vec3Array;
    ptArray->push_back( osg::Vec3( 0.0f,-1.0f, 0.0f) );
    ptArray->push_back( osg::Vec3( 1.0f, 1.0f, 0.0f) );
    ptArray->push_back( osg::Vec3(-1.0f, 1.0f, 0.0f) );
    ptArray->push_back( osg::Vec3( 0.0f,-1.0f, 0.0f) );
    osg::ref_ptr<osgModeling::Curve> profile = new osgModeling::Curve;
    profile->setPath( ptArray.get() );
    osg::ref_ptr<osgModeling::Extrude> geom = new osgModeling::Extrude;
    geom->setGenerateParts( osgModeling::Model::ALL_PARTS );
    geom->setExtrudeDirection( osg::Vec3(0.0f, 0.0f, -1.0f) );
    geom->setExtrudeLength( 2.0f );
    geom->setProfile( profile.get() );
    geom->update();

    return geom;
}

osg::ref_ptr<osg::Geode> createSubd()
{
    osg::ref_ptr<osgModeling::PolyMesh> mesh = new osgModeling::PolyMesh( *createGeometry() );
    mesh->subdivide( new osgModeling::LoopSubdivision );
    for ( osgModeling::PolyMesh::FaceList::iterator fitr=mesh->_faces.begin();
        fitr!=mesh->_faces.end();
        ++fitr )
    {
        PRINT_VEC3( "A", (*(*fitr))[0] );
        PRINT_VEC3( "B", (*(*fitr))[1] );
        PRINT_VEC3( "C", (*(*fitr))[2] );
        std::cout << std::endl;
    }

    /*for ( osgModeling::PolyMesh::EdgeMap::iterator itr=mesh->_edges.begin();
        itr!=mesh->_edges.end();
        ++itr )
    {
        osgModeling::PolyMesh::Edge* e = itr->second;
    }*/

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( mesh.get() );
    return geode;
}

int main( int argc, char** argv )
{
    osgViewer::Viewer viewer;
    viewer.setSceneData( createSubd().get() );
    return viewer.run();
}
