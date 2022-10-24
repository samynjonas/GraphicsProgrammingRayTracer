#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

#include <iostream>

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//First triangle from ray origin to circle middle
			Vector3 tc{};
			tc = sphere.origin - ray.origin;

			Vector3 P{};
			P = Vector3::Project(tc, ray.direction);

			//From ray-origin to projected point 
			float dp = Vector3::Dot(tc, ray.direction);

			//From projected-point to sphere origin
			float od = sqrtf((tc.Magnitude() * tc.Magnitude()) - (dp * dp));

			//Second triangle from hitpoint to circle origin
			float tca = sqrtf((sphere.radius * sphere.radius) - (od * od));

			//Distance from ray origin to hitpoint
			float t0 = dp - tca;

			//Hitpoint
			Vector3 I = ray.origin + t0 * ray.direction;			

			Vector3 normal = (I - sphere.origin).Normalized();


			if (t0 > ray.min && t0 < ray.max)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = true;
					hitRecord.t = t0;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = I;
					hitRecord.normal = normal;
				}
				return true;
			}
			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			Vector3 n = plane.normal;

			float t = Vector3::Dot( (plane.origin - ray.origin), n ) / Vector3::Dot(ray.direction, plane.normal);

			Vector3 I = ray.origin + ((Vector3::Dot((plane.origin - ray.origin), n) / Vector3::Dot(ray.direction, n))) * ray.direction;
			if (t > ray.min && t < ray.max)
			{
				if (!ignoreHitRecord)
				{
					hitRecord.didHit = true;
					hitRecord.t = t;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.origin = I;
					hitRecord.normal = n;
				}
				return true;
			}
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//Creating edges
			Vector3 a = triangle.v1 - triangle.v0;
			Vector3 b = triangle.v2 - triangle.v0;

			//Normal of triangle
			Vector3 normal = Vector3::Cross(a, b);

			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (Vector3::Dot(normal, ray.direction) < 0)
				{
					return false;
				}
				break;
			case TriangleCullMode::BackFaceCulling:
				if (Vector3::Dot(normal, ray.direction) > 0)
				{
					return false;
				}
				break;
			case TriangleCullMode::NoCulling:
			default:
				if (Vector3::Dot(normal, ray.direction) == 0)
				{
					return false;
				}
				break;
			}			

			Vector3 center = ((triangle.v0 + triangle.v1 + triangle.v2) / 3);

			Vector3 L = center - ray.origin;
			float t = Vector3::Dot(L, normal) / Vector3::Dot(ray.direction, normal);

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			Vector3 p = ray.origin + t * ray.direction;


			Vector3 edgeA = triangle.v1 - triangle.v2;
			Vector3 pointToSide = p - triangle.v0;

			if (Vector3::Dot(normal, Vector3::Cross(edgeA, pointToSide)) < 0)
			{
				return false;
			}

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = p;
				hitRecord.normal = normal;
			}
			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Vector3 v0{};
			Vector3 v1{};
			Vector3 v2{};

			int triangleCounter{ 0 };
			int vectorCounter{ 0 };

			HitRecord closestRecord{};

			for (size_t index = 0; index < mesh.indices.size(); index++)
			{
				switch (vectorCounter)
				{
				case 0:
					v0 = mesh.transformedPositions[mesh.indices[index]];
					vectorCounter++;
					break;
				case 1:
					v1 = mesh.transformedPositions[mesh.indices[index]];
					vectorCounter++;
					break;
				case 2:
					v2 = mesh.transformedPositions[mesh.indices[index]];
					vectorCounter++;
					break;
				}

				if (vectorCounter == 3)
				{
					Triangle tempTriangle = { v0, v1, v2, mesh.transformedNormals[triangleCounter]};
					
					HitRecord tempHitRecord{};
					GeometryUtils::HitTest_Triangle(tempTriangle, ray, tempHitRecord);

					if (triangleCounter == 0)
					{
						closestRecord = tempHitRecord;
					}
					else if (tempHitRecord.t < closestRecord.t)
					{
						closestRecord = tempHitRecord;
					}

					++triangleCounter;
					vectorCounter = 0;
				}
			}

			if (!ignoreHitRecord)
			{
				hitRecord = closestRecord;
			}
			return true;

		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			return origin - light.origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			if (light.type == LightType::Directional)
			{				
				return light.color * light.intensity;
			}
			else if(light.type == LightType::Point)
			{
				return light.color * (light.intensity / ( powf((light.origin.x - target.x), 2.f) + powf((light.origin.y - target.y), 2.f) + powf((light.origin.z - target.z), 2.f)));
			}
			return {};
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}