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
		float fovAngle{ 90.f };

		Vector3 forward{ 0.266f,  -0.453f,  0.860f };

		Vector3 up{		Vector3::UnitY };
		Vector3 right{	Vector3::UnitX };

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};
		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			Matrix cameraONB;
			Vector3 worldUp{ 0, 1, 0 };
			
			right	= Vector3::Cross(worldUp, forward).Normalized();
			up		= Vector3::Cross(forward, right).Normalized();

			cameraONB = { right, up, forward, origin };
			return cameraONB;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();
			const float currentSpeed{ 5.f };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin.x = -currentSpeed * deltaTime;
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
			int prevMouseX{},	prevMouseY{};
			int mouseX{},		mouseY{};
			const int offset{ 5 };
			const float angleChange{ 1.f * deltaTime };

			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);			
			
			if ((mouseState & SDL_BUTTON_RMASK) != 0)
			{
				//Right mousebutton pressed
				//Left mousebutton pressed
				if (mouseX > prevMouseX + offset)
				{
					//moving right
					forward = Matrix::CreateRotationY(angleChange).TransformVector(forward);
				}
				else if (mouseX < prevMouseX - offset)
				{
					//moving left
					forward = Matrix::CreateRotationY(-angleChange).TransformVector(forward);
				}

				if (mouseY > prevMouseY + offset)
				{
					//moving up
					forward = Matrix::CreateRotationX(-angleChange).TransformVector(forward);
				}
				else if (mouseY < prevMouseY - offset)
				{
					//moving down
					forward = Matrix::CreateRotationX(angleChange).TransformVector(forward);
				}

			}
			forward.Normalize();

			//todo: W2
		}
	};
}
