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
			Vector3 d = ray.direction;

			float A = Vector3::Dot(ray.direction, ray.direction);
			float B = 2 * Vector3::Dot( d, (ray.origin - sphere.origin));
			float C = Vector3::Dot((ray.origin - sphere.origin), (ray.origin - sphere.origin)) - powf(sphere.radius, 2.f);

			float discriminat = powf(B, 2.f) - 4 * A * C;
			if (discriminat <= 0)
			{
				return false;
			}
			if (discriminat < ray.min || discriminat > ray.max)
			{
				return false;
			}


			float t = (-B - sqrtf(discriminat)) / (2 * A);
			if (t < ray.min)
			{
				t = (-B + sqrtf(discriminat)) / (2 * A);
			}

			////Geometric
			//Vector3 L = sphere.origin - ray.origin;
			//float tca = Vector3::Dot(L, ray.direction);
			//float od = sqrt(Vector3::Dot(L, L) - tca * tca);
			//float thc = sqrt(powf(sphere.radius, 2.f) - powf(od, 2.f));

			//float t0 = tca - thc;
			//float t1 = tca + thc;

			Vector3 P = ray.origin + t * ray.direction;
			Vector3 normal = (P - sphere.origin).Normalized();

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = P;
				hitRecord.normal = normal;
			}
			return true;
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
			Vector3 b = triangle.v2 - triangle.v1;
			Vector3 c = triangle.v0 - triangle.v2;


			//Normal of triangle
			Vector3 triangleNormal = Vector3::Cross(a, b);

			//Check for cullMode
			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (Vector3::Dot(triangleNormal, ray.direction) < 0)
				{
					return false;
				}
				break;
			case TriangleCullMode::BackFaceCulling:
				if (Vector3::Dot(triangleNormal, ray.direction) > 0)
				{
					return false;
				}
				break;
			case TriangleCullMode::NoCulling:
			default:
				if (Vector3::Dot(triangleNormal, ray.direction) == 0)
				{
					return false;
				}
				break;
			}			

			Vector3 center = ((triangle.v0 + triangle.v1 + triangle.v2) / 3);

			Vector3 L = center - ray.origin;
			float t = Vector3::Dot(L, triangleNormal) / Vector3::Dot(ray.direction, triangleNormal);

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			Vector3 p = ray.origin + t * ray.direction;

			Vector3 pointToSide = p - triangle.v0;
			if (Vector3::Dot(triangleNormal, Vector3::Cross(a, pointToSide)) < 0)
			{
				return false;
			}
			pointToSide = p - triangle.v1;
			if (Vector3::Dot(triangleNormal, Vector3::Cross(b, pointToSide)) < 0)
			{
				return false;
			}
			pointToSide = p - triangle.v2;
			if (Vector3::Dot(triangleNormal, Vector3::Cross(c, pointToSide)) < 0)
			{
				return false;
			}

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.t = t;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = p;
				hitRecord.normal = triangleNormal;
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
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}



			HitRecord closestRecord{};
			int normalIndex{};
			Triangle tempTriangle{};

			for (size_t index = 0; index < mesh.indices.size(); index += 3)
			{
				if (index + 2 < mesh.indices.size())
				{
					uint32_t p0 = mesh.indices[index];
					uint32_t p1 = mesh.indices[index + 1];
					uint32_t p2 = mesh.indices[index + 2];


					tempTriangle =
					{
						mesh.transformedPositions[p0],
						mesh.transformedPositions[p1],
						mesh.transformedPositions[p2],
						mesh.transformedNormals[normalIndex]
					};
					normalIndex++;

					tempTriangle.cullMode = mesh.cullMode;
					tempTriangle.materialIndex = mesh.materialIndex;

					if (GeometryUtils::HitTest_Triangle(tempTriangle, ray, closestRecord, ignoreHitRecord))
					{
						if (ignoreHitRecord)
						{
							return true;
						}
						else
						{
							if (closestRecord.t < hitRecord.t)
							{
								hitRecord = closestRecord;
							}
						}
					}
				}
			}

			return hitRecord.didHit;
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
			return light.origin - origin;
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
				Vector3 lightPosition{ light.origin };
				Vector3 PointToShade{ target };

				return light.color * (light.intensity / (lightPosition - PointToShade).SqrMagnitude() );
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