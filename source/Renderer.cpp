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

using namespace dae;

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
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float aspectRatio{ m_Width / float(m_Height) };
	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			//Calculate FOV
			float fovAngle = pScene->GetCamera().fovAngle * M_PI / 180;
			float fov = tanf(fovAngle / 2);

			float cx = ((2 * (px + 0.5f)) / m_Width - 1) * aspectRatio * fov;
			float cy = (1 - (2 * (py + 0.5f)) / m_Height) * fov;

			//Ray calculation
			Vector3 rayDirection{ cx, cy, 1 };
			rayDirection.Normalize();

			const Matrix cameraToWorld = camera.CalculateCameraToWorld();			
			rayDirection = cameraToWorld.TransformVector(rayDirection);

			Ray viewRay{ camera.origin,  rayDirection };
			
			ColorRGB finalColor{};
			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);
			if (closestHit.didHit)
			{
				//finalColor = materials[closestHit.materialIndex]->Shade();
				finalColor += LightUtils::GetRadiance(lights[0], rayDirection);

				if (m_ShadowsEnabled)
				{
					for (size_t index = 0; index < lights.size(); index++)
					{
						Vector3 startPos{ closestHit.origin };
						startPos.y += 0.1f;

						Vector3 lightVector{ LightUtils::GetDirectionToLight(lights[index], startPos) };
						Ray lightRay{ lights[index].origin, rayDirection };

						if (pScene->DoesHit(lightRay))
						{
							finalColor *= 0.5f;
						}
					}
				}
				
				//const float scaled_t = -closestHit.t / 250.f;
				//ColorRGB colorShading = materials[closestHit.materialIndex]->Shade();
				//finalColor = { (scaled_t) * colorShading.r, (scaled_t) * colorShading.g, (scaled_t) * colorShading .b};

				//finalColor = { 1 - scaled_t, 1 - scaled_t, 1 - scaled_t };
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::ModeSwitcher()
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	if (pKeyboardState[SDL_SCANCODE_F2])
	{
		m_ShadowsEnabled != m_ShadowsEnabled;
	}

	if (pKeyboardState[SDL_SCANCODE_F3])
	{
		int currentMode{ int(m_CurrentLightingMode) };
		
		++currentMode % int(LightingMode::Combined);

		static_cast<LightingMode>(currentMode);
	}
}