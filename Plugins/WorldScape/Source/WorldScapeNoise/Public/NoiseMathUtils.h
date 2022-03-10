
class NoiseMathUtils {
public :
	static float Clamp(const float X, const float Min, const float Max);
	static float Clamp01(const float X);
	static float Lerp(const float& A, const float& B, const float& Alpha);
	static float Max(const float& A, const float& B);
	static float Min(const float& A, const float& B);
	static float SmoothMin(const float& a, const float& b, float k);
	static float SmoothMax(const float& a, const float& b, float k);
	static float SmoothStep(float x);
	static double DClamp(double a, double min = 0, double max = 1);

};