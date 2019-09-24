#include <string>
class MeshView;
// Load a mesh from a file
namespace MeshLoader
{
   bool Load(MeshView&   aView,
             std::string aFileName);

   bool LoadFeatures(MeshView * aView,
          std::string aFileName);
}

