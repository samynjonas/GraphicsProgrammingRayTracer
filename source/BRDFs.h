#pragma once
#include <cassert>
#include "Math.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			ColorRGB rho = cd * kd;
			ColorRGB lambert = rho / PI;

			return { lambert };
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			ColorRGB rho = cd * kd;
			ColorRGB lambert = rho / PI;

			return { lambert };
		}

		/**
		 * \brief todo
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			//Vector3 reflect = l - 2 * Vector3::Dot(n,l) * n;
			const Vector3 reflect{ Vector3::Reflect(l, n)};
			const float angle = std::max(Vector3::Dot(reflect, v), 0.f);

			const float Phong = ks * powf((angle), exp);

			return { Phong, Phong, Phong }; //Dont know what to return here
		}

		/**
		 * \brief BRDF Fresnel Function >> Schlick
		 * \param h Normalized Halfvector between View and Light directions
		 * \param v Normalized View direction
		 * \param f0 Base reflectivity of a surface based on IOR (Indices Of Refrection), this is different for Dielectrics (Non-Metal) and Conductors (Metal)
		 * \return
		 */
		static ColorRGB FresnelFunction_Schlick(const Vector3& h, const Vector3& v, const ColorRGB& f0)
		{
			const ColorRGB shlick = ( f0 + (ColorRGB{ 1.f, 1.f, 1.f } - f0) * powf((1 - (std::max(Vector3::Dot(h, v), 0.f))), 5));
			return shlick;
		}

		/**
		 * \brief BRDF NormalDistribution >> Trowbridge-Reitz GGX (UE4 implemetation - squared(roughness))
		 * \param n Surface normal
		 * \param h Normalized half vector
		 * \param roughness Roughness of the material
		 * \return BRDF Normal Distribution Term using Trowbridge-Reitz GGX
		 */
		static float NormalDistribution_GGX(const Vector3& n, const Vector3& h, float roughness)
		{
			const float a = powf(roughness, 2.f);
			const float a2 = powf(a, 2.f);

			const float dotnh{ std::max(Vector3::Dot(n, h), 0.f) };
			const float dotSqrt = dotnh * dotnh;

			const float normal = a2 / (PI * powf(dotSqrt * (a2 - 1) + 1, 2.f));

			return normal;
		}


		/**
		 * \brief BRDF Geometry Function >> Schlick GGX (Direct Lighting + UE4 implementation - squared(roughness))
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using SchlickGGX
		 */
		static float GeometryFunction_SchlickGGX(const Vector3& n, const Vector3& v, float roughness)
		{
			const float a = powf(roughness, 2.f);
			const float k = (powf((a + 1.f), 2.f)) / 8.f;
			const float dotnv{ std::max(Vector3::Dot(n, v), 0.f) };

			const float shlickGGX = dotnv / (dotnv * (1 - k) + k);

			return shlickGGX;
		}

		/**
		 * \brief BRDF Geometry Function >> Smith (Direct Lighting)
		 * \param n Normal of the surface
		 * \param v Normalized view direction
		 * \param l Normalized light direction
		 * \param roughness Roughness of the material
		 * \return BRDF Geometry Term using Smith (> SchlickGGX(n,v,roughness) * SchlickGGX(n,l,roughness))
		 */
		static float GeometryFunction_Smith(const Vector3& n, const Vector3& v, const Vector3& l, float roughness)
		{
			const float smith = GeometryFunction_SchlickGGX(n, v, roughness * roughness) * GeometryFunction_SchlickGGX(n, l, roughness * roughness);			
			
			return smith;
		}

	}
}