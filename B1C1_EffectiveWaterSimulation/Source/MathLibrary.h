#pragma once

#define SMALL_NUMBER		(1.e-8f)
#define PI_ON_180_DEGREES 0.0174532778 // PI / 180

#include "Matrix.h"
#include "Vector.h"

class Math
{
public:
	template<typename t>
	static t Clamp(t Value, t Min, t Max)
	{
		return Value < Min ? Min : (Value > Max ? Max : Value);
	}

	template<typename T>
	static T DegreesToRadians(T const& Degrees)
	{
		return Degrees * (T)PI_ON_180_DEGREES;
	}

	static float Sin(float Value)
	{
		return sinf(Value);
	}


	static float Cos(float Value)
	{
		return cosf(Value);
	}

	static float Tan(float Value)
	{
		return tan(Value);
	}

	static void SinCos(float* SinRes, float* CosRes, float Value)
	{
		*SinRes = Sin(Value);
		*CosRes = Cos(Value);
	}

	static float Abs(float A)
	{
		return fabsf(A);
	}

	static double TruncToDouble(double A)
	{
		return trunc(A);
	}

	static float Mod(float A, float B)
	{
		const float AbsB = Abs(B);

		if (B < SMALL_NUMBER)
		{
			return 0.0;
		}

		const double DA = double(A);
		const double DB = double(B);

		const double Div = DA / DB;
		const double IntPortion = TruncToDouble(Div) * DB;
		const double Result = DA - IntPortion;

		return float(Result);
	}

	static Matrix<float> LookAt(const Vector3f Eye, const Vector3f& CameraForwardVector, const Vector3f& WorldUpVector)
	{
		Vector3f ZAxis = CameraForwardVector;
		ZAxis = ZAxis.Normalize();

		Vector3f XAxis = Vector3f::CrossProduct(WorldUpVector, ZAxis);
		XAxis = XAxis.Normalize();

		Vector3f YAxis = Vector3f::CrossProduct(ZAxis, XAxis);

		float XDis = -Vector3f::DotProduct(XAxis, Eye);
		float YDis = -Vector3f::DotProduct(YAxis, Eye);
		float ZDis = -Vector3f::DotProduct(ZAxis, Eye);

		Matrix<float> Mat;

		Mat.M[0][0] = XAxis.X;	Mat.M[0][1] = YAxis.X;	Mat.M[0][2] = ZAxis.X;	Mat.M[0][3] = 0.0f;
		Mat.M[1][0] = XAxis.Y;	Mat.M[1][1] = YAxis.Y;	Mat.M[1][2] = ZAxis.Y;	Mat.M[1][3] = 0.0f;
		Mat.M[2][0] = XAxis.Z;	Mat.M[2][1] = YAxis.Z;	Mat.M[2][2] = ZAxis.Z;	Mat.M[2][3] = 0.0f;
		Mat.M[3][0] = XDis;		Mat.M[3][1] = YDis;		Mat.M[3][2] = ZDis;		Mat.M[3][3] = 1.0f;

		return Mat;
	}
};
