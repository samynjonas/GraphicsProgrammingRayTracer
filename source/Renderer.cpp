//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#include <thread>
#include <future> //ASYNC stuff
#include <ppl.h>

using namespace dae;

//#define ASYNC
#define PARALLEL_FOR

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	//Camera
	Camera& camera = pScene->GetCamera();
	camera.CalculateCameraToWorld();

	//Aspect Ratio
	float aspectRatio{ m_Width / float(m_Height) };

	//Calculate FOV
	float fovAngle	= pScene->GetCamera().fovAngle * M_PI / 180;
	float fov		= tanf(fovAngle / 2);

	//Ask materials and lights
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	//Go through pixels
	const uint32_t numPixels = m_Width * m_Height;

#if defined(ASYNC)
	//ASYNC
	const uint32_t numCores = std::thread::hardware_concurrency();
	std::vector<std::future<void>> async_futures{};

	const uint32_t numPixelsPerTask = numPixels / numCores; //Int division can skip pixels
	uint32_t numUnassignedPixels = numPixels % numCores; //Rest of division
	uint32_t currentPixelIndex = 0;

	//Create task
	for (uint32_t index{ 0 }; index < numCores; ++index)
	{
		uint32_t taskSize = numPixelsPerTask;
		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(
			std::async(std::launch::async, [=, this] 
			{
				const uint32_t pixelIndexEnd = currentPixelIndex + taskSize;
				for (uint32_t pixelIndex = currentPixelIndex; pixelIndex < pixelIndexEnd; ++pixelIndex)
				{
					RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
				}
			})
		);

		currentPixelIndex += taskSize;
	}

	//Wait for all task
	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	//PARALLEL
	Concurrency::parallel_for(0u, numPixels, [=, this](int i) 
		{
			RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
		});

#else
	//SYNCHRONOUS
	for (uint32_t index = 0; index < numPixels; index++)
	{
		RenderPixel(pScene, index, fov, aspectRatio, camera, lights, materials);
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;

	float gradient	 = px / static_cast<float>(m_Width);
	gradient		+= py / static_cast<float>(m_Width);
	gradient /= 2.0f;	

	float cx = ((2 * (px + 0.5f)) / m_Width - 1) * aspectRatio * fov;
	float cy = (1 - (2 * (py + 0.5f)) / m_Height) * fov;

	//Ray calculation
	Vector3 rayDirection{ cx, cy, 1 };
	rayDirection.Normalize();

	const Matrix cameraToWorld = camera.cameraToWorld;
	rayDirection = cameraToWorld.TransformVector(rayDirection);

	Ray viewRay{ camera.origin,  rayDirection };

	ColorRGB finalColor{};
	HitRecord closestHit{};

	pScene->GetClosestHit(viewRay, closestHit);
	if (closestHit.didHit)
	{
		//finalColor = materials[closestHit.materialIndex]->Shade();

		for (size_t index = 0; index < lights.size(); index++)
		{
			Vector3 lightDirection = LightUtils::GetDirectionToLight(lights[index], closestHit.origin + closestHit.normal * 0.01f);
			lightDirection.Normalize();

			float dotProduct = Vector3::Dot(closestHit.normal, lightDirection);
			//Dot products always gives 0
			
			if (dotProduct > 0)
			{				
				ColorRGB IncidentRadiance{ LightUtils::GetRadiance(lights[index], closestHit.origin) };
				ColorRGB BRDF = materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, viewRay.direction);

				ColorRGB addColor = IncidentRadiance * BRDF * dotProduct;

				finalColor += addColor;
			}

			if (m_ShadowsEnabled)
			{
				Ray lightRay{};
				lightRay.origin = closestHit.origin;
				lightRay.direction = LightUtils::GetDirectionToLight(lights[index], lightRay.origin + closestHit.normal * 0.01f);
				lightRay.min = 0.1f;
				lightRay.max = lightRay.direction.Magnitude();
				lightRay.direction.Normalize();

				if (pScene->DoesHit(lightRay))
				{
					finalColor *= 0.5f;
				}
			}

		}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::ModeSwitcher()
{
	int currentMode{ int(m_CurrentLightingMode) };

	++currentMode% int(LightingMode::Combined);

	static_cast<LightingMode>(currentMode);
}

void Renderer::SwitchShadows()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}