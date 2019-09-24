#include "Mesh.hpp"

void Mesh::ExtractMatPoints(std::vector<MatPoint>& aPoints)
{
	aPoints.assign(verts.size(), MatPoint());
	for (size_t i = 0; i < verts.size(); ++i)
	{
		aPoints[i].point = verts[i];
	}
	for (size_t i = 0; i < faces.size(); ++i)
	{
		Face& f = faces[i];
		for (size_t j = 0; j < 3; ++j)
		{
			int vi = f.vertex[j];
			for (size_t m = 0; m < 2; ++m)
			{
				int mat = f.materials[m];
				tiny_vector<int, 4>& mats = aPoints[vi].materials;
				if (std::find(mats.begin(), mats.end(), mat) == mats.end())
				{
					mats.push_back(mat);
				}
			}
		}
	}
	for (size_t i = 0; i < aPoints.size(); ++i)
	{
		MatPoint& pt = aPoints[i];
		std::sort(pt.materials.begin(), pt.materials.end());
//		if (pt.materials.size() < 4 ){
//			for (int j=0; j<4-pt.materials.size() +1; j++) {
//				pt.materials.push_back(-3);
//			}
//		}
	}
}
