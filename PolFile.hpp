#ifndef POLFILE_HPP
#define POLFILE_HPP
class DMesh;
#include <vector>
#include <string>
#include "GeometryTypes.hpp"
class PolFile
{
public:
	static bool ReadLandmarks(const std::string& aFile,
							  std::vector<MatPoint>& mLandmarks);
	static bool ReadSurfaceLandmarks(const std::string& aFile,
							  std::vector<MatPoint>& mLandmarks);
   static bool Read(const std::string& aFile,
                    DMesh&          aMesh);
   static bool Write(const std::string& aFile,
                     DMesh&    aMesh);
};

#endif // POLFILE_HPP