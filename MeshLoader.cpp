#include "MeshLoader.hpp"

#include "boost/algorithm/string.hpp"
#include "GtsFile.hpp"
#include "MeshView.hpp"
#include "ObjFile.hpp"
#include "OffFile.hpp"
#include "PolFile.hpp"
#include "SmeshFile.hpp"
#include "SufFile.hpp"
#include "TetgenFile.hpp"
#include "DebugHelper.hpp"
#include "DMesh.hpp"
#include "SubFile.hpp"
#include "PatchMesh.hpp"
#include "FeatureFile.hpp"
namespace MeshLoader
{

bool Load(MeshView&   aView,
          std::string aFileName)
{
   DEBUG_LOG("MeshLoader::Load\n")
   bool loaded=true;
   bool polymesh=false;
   std::vector<std::string> parts;
   aView.GetMesh().clear();
   aView.mName = boost::split(parts, aFileName, boost::is_any_of("."))[0];
   aView.mName = boost::split(parts, aView.mName, boost::is_any_of("/\\")).back();
   if (boost::ends_with(aFileName, ".suf"))
   {
      SufFile::Read(aFileName, aView.GetMesh());
   }
   else if (boost::ends_with(aFileName, ".obj"))
   {
      ObjFile::Read(aFileName, aView.GetMesh());
   }
   else if (boost::ends_with(aFileName, ".pol"))
   {
      polymesh=true;
      PolFile::Read(aFileName, aView.GetMesh());
   }
   else
   {
      loaded=false;
   }
   if (loaded)
   {
      aView.UpdateMesh();
   }
   DEBUG_LOG(".\n")
   return loaded;

}

bool LoadFeatures(MeshView * aView,
          std::string aFileName)
{
   DEBUG_LOG("MeshLoader::LoadFeatures\n")
   bool loaded=true;
   bool polymesh=false;
   DMesh aMesh;
   if (boost::ends_with(aFileName, ".obj"))
   {
           polymesh=true;
      ObjFile::Read(aFileName, aMesh);
   }
   else if (boost::ends_with(aFileName, ".pol"))
   {
      polymesh=true;
      PolFile::Read(aFileName, aMesh);
   }
   else if (boost::ends_with(aFileName, ".fea"))
   {
          vector<FeaturePoint> features;
      FeatureFile::Read(aFileName, features);
          aView->mFeatures = features;
   }
   else
   {
      loaded=false;
   }
   if (loaded)
   {
           std::vector<FeaturePoint> aPoints;
           if (polymesh) {
                   Mesh m;
                   aMesh.create_mesh(m);
                        PolyMesh pm;
                        pm.crease_faces.ref(m.faces);
                        pm.verts.ref(m.verts);
                        pm.polys.ref(m.polys);
                        pm.crease_edges.ref(m.edges);
                   aPoints.assign(pm.verts.size(), FeaturePoint());
                   for (size_t i = 0; i < pm.verts.size(); ++i)
                   {
                           std::stringstream ss;
                           std::string str;
                           ss << i;
                           ss >> str;
                           aPoints[i].point = pm.verts[i];
                           aPoints[i].label = "Feature " + str;
                   }
                   aView->mFeatures = aPoints;
           }
      aView->UpdateMesh();
   }
   DEBUG_LOG(".\n")
   return loaded;

}

}