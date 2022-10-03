#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"


#include <iostream>


namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}

		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{ 0.266f,  -0.453f,  0.860f };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			Matrix cameraONB;
			Vector3 worldUp{ Vector3::UnitY };
			
			right	= Vector3::Cross(worldUp, forward).Normalized();
			up		= Vector3::Cross(forward, right).Normalized();

			cameraONB = { right, up, forward, origin };
			return cameraONB;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			float currentSpeed{ 10.f };

			if (pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin.x -= currentSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin.x += currentSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_UP])
			{
				origin.z += currentSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin.z -= currentSpeed * deltaTime;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);			
			

			//todo: W2
			//assert(false && "Not Implemented Yet");
		}
	};
}
