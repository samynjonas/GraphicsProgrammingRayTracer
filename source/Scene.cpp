#include "Scene.h"
#include "Utils.h"
#include "Material.h"

#include <iostream>

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		//todo W1
		for (size_t index = 0; index < m_SphereGeometries.size(); index++)
		{
			if (index != 0)
			{
				HitRecord tempHitRecord{};
				GeometryUtils::HitTest_Sphere(m_SphereGeometries[index], ray, tempHitRecord);
				if (tempHitRecord.t < closestHit.t)
				{
					closestHit = tempHitRecord;
				}
			}
			else
			{
				GeometryUtils::HitTest_Sphere(m_SphereGeometries[index], ray, closestHit);
			}
		}

		for (size_t index = 0; index < m_PlaneGeometries.size(); index++)
		{
			HitRecord tempHitRecord{};
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[index], ray, tempHitRecord);

			if (tempHitRecord.t < closestHit.t)
			{
				closestHit = tempHitRecord;
			}
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		//todo W3
		for (size_t index = 0; index < m_SphereGeometries.size(); index++)
		{
			HitRecord tempHitRecord{};
			if (GeometryUtils::HitTest_Sphere(m_SphereGeometries[index], ray, tempHitRecord))
			{
				return true;
			}
		}
		for (size_t index = 0; index < m_PlaneGeometries.size(); index++)
		{
			HitRecord tempHitRecord{};
			if (GeometryUtils::HitTest_Plane(m_PlaneGeometries[index], ray, tempHitRecord))
			{
				return true;
			}
		}
		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);


		/*
		float dotResult{};
		dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitX);
		std::cout << dotResult << '\n';
		dotResult = Vector3::Dot(Vector3::UnitX, -Vector3::UnitX);
		std::cout << dotResult << '\n';
		dotResult = Vector3::Dot(Vector3::UnitX, Vector3::UnitY);
		std::cout << dotResult << '\n';

		Vector3 crossResult{};
		crossResult = Vector3::Cross(Vector3::UnitZ, Vector3::UnitX);
		std::cout << crossResult.x << ',' << crossResult.y << ',' << crossResult.z << '\n';
		crossResult = Vector3::Cross(Vector3::UnitX, Vector3::UnitZ);
		std::cout << crossResult.x << ',' << crossResult.y << ',' << crossResult.z << '\n';
		*/

	}
#pragma endregion
#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;

		const unsigned char matId_Solid_Blue	= AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow	= AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green	= AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Plane
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f, 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f, -1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere({ -1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f, 1.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f, 1.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f, 3.f, 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);

		//Light
		AddPointLight({0.f, 5.f, -5.f}, 70.f, colors::White);
	}
#pragma endregion
#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal		= AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal	= AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .0f));
		const auto matCT_GraySmoothMetal	= AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic	= AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic	= AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic	= AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLambert_GrayBlue		= AddMaterial(new Material_Lambert({ .49f, .57f, .57f }, 1.f));

		//Plane
		AddPlane(Vector3{ 0.f, 0.f, 10.f },		Vector3{ 0.f, 0.f, -1.f },	matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f },		Vector3{ 0.f, 1.f, 0.f },	matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 10.f },	Vector3{ 0.f, -1.f, 0.f },	matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 10.f },		Vector3{ -1.f, 0.f, 0.f },	matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 10.f },	Vector3{ 1.f, 0.f, 0.f },	matLambert_GrayBlue); //LEFT

		//Sphere
		AddSphere(Vector3{ -1.75f, 1.f, 0.f },	.75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 1.f, 0.f },		.75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 1.f, 0.f },	.75f, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75f, 3.f, 0.f },	.75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f },		.75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f, 3.f, 0.f },	.75f, matCT_GraySmoothPlastic);

		//Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f },		50.f, ColorRGB{ 1.f, .61f, .45f });		//Backlight
		AddPointLight(Vector3{ -2.f, 5.f, -5.f },	70.f, ColorRGB{ 1.f, .8f, .45f });		//Front light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f },	50.f, ColorRGB{ .34f, .47f, .60f });

	}
#pragma endregion
#pragma region SCENE W3 TestScene
	void Scene_W3_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });

		//Spheres
		AddSphere({ -.75f, 1.f, .0f }, 1.f, matId_Solid_Red);
		AddSphere({ .75f, 1.f, .0f }, 1.f, matId_Solid_Blue);

		//Plane
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, matId_Solid_Yellow);

		//Light
		AddPointLight({ 0.f, 5.f, 5.f }, 25.f, colors::White);	
	}
#pragma endregion


}
