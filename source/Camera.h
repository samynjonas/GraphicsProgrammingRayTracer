#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <algorithm>

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
		float fovAngle{ 90.f };
		float fovMultiplier{ 1.0f };

		Vector3 forward{ Vector3::UnitZ };

		Vector3 up{		Vector3::UnitY };
		Vector3 right{	Vector3::UnitX };

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};

		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			Matrix cameraONB;
			Vector3 worldUp{ Vector3::UnitY };
			
			right	= Vector3::Cross(worldUp, forward).Normalized();
			up		= Vector3::Cross(forward, right);

			cameraONB = { right, up, forward, origin };

			cameraToWorld = cameraONB;

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			
			const float movementSpeed{ 5.f };
			const float minFov{ 30.f };
			const float maxFov{ 170.f };
			const float mouseSpeed{ 2.0f };
			const float rotateSpeed{ 10.f * TO_RADIANS };

			Vector3 directionVector{};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_Z] || pKeyboardState[SDL_SCANCODE_W])
			{
				directionVector += forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				directionVector -= forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_A])
			{
				directionVector -= right * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				directionVector += right * movementSpeed * deltaTime;
			}

			//Mouse Input
			int mouseX{},		mouseY{};

			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if ((mouseState & SDL_BUTTON_LMASK) != 0)
			{
				directionVector -= forward * (mouseY * mouseSpeed * deltaTime);
				totalYaw	+= mouseX * rotateSpeed * deltaTime;
			}
			else if ((mouseState & SDL_BUTTON_RMASK) != 0)
			{
				totalYaw	+= mouseX * rotateSpeed * deltaTime;
				totalPitch	-= mouseY * rotateSpeed * deltaTime;
			}
			totalPitch = std::clamp(totalPitch, -89.f * TO_RADIANS, 89.0f * TO_RADIANS);

			const float shiftSpeed{ 4.0f };
			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				directionVector *= shiftSpeed;
			}

			origin += directionVector;

			Matrix rotationMatrix = Matrix::CreateRotationX(totalPitch) * Matrix::CreateRotationY(totalYaw);

			forward = rotationMatrix.TransformVector(Vector3::UnitZ);
		}
	};
}
