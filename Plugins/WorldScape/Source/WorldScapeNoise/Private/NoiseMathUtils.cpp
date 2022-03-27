#include "NoiseMathUtils.h"

float NoiseMathUtils::Clamp(const float X, const float Min, const float Max)
{
	return X < Min ? Min : X < Max ? X : Max;
}
float NoiseMathUtils::Clamp01(const float X)
{
	return X < 0 ? 0 : X < 1 ? X : 1;
}

float NoiseMathUtils::Lerp(const float& A, const float& B, const float& Alpha)
{
	return (A + Alpha * (B - A));
}
float NoiseMathUtils::Max(const float& A, const float& B) 
{

	return A > B ? A : B;
}
float NoiseMathUtils::Min(const float& A, const float& B) 
{

	return A < B ? A : B;
}

//SmoothMin and SmoothMax https://github.com/SebLague/Solar-System/blob/0c60882be69b8e96d6660c28405b9d19caee76d5/Assets/Scripts/Celestial/Shaders/Includes/Math.cginc#L27
/*
	Smooth minimum of two values, controlled by smoothing factor k
	When k = 0, this behaves identically to min(a, b)
 */
float NoiseMathUtils::SmoothMin(const float& a, const float& b, float k) 
{
	k = Max(0, k);
	float h = Max(0, Min(1, (b - a + k) / (2 * k)));
	return a * h + b * (1 - h) - k * h * (1 - h);
}

/*
	Smooth maximum of two values, controlled by smoothing factor k
	When k = 0, this behaves identically to max(a, b)
*/
float NoiseMathUtils::SmoothMax(const float& a, const float& b, float k) 
{
	k = Min(0, -k);
	float h = Max(0, Min(1, (b - a + k) / (2 * k)));
	return a * h + b * (1 - h) - k * h * (1 - h);
}

float NoiseMathUtils::SmoothStep(float x) 
{
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

double NoiseMathUtils::DClamp(double a, double min, double max) 
{
	if (a > max)
		return max;
	if (a < min)
		return min;
	return a;
}


float NoiseMathUtils::LerpStep(const float& a, const float& b, const float& k)
{
    return Clamp01((k - a)/(b - a));
}
