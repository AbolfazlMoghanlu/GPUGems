#pragma once

#include "Vector.h"
#include "Rotator.h"
#include "Plane.h"
#include "MathLibrary.h"

template<typename T>
class Matrix
{
public:
	T M[4][4] = { 0 };

	Matrix(const Plane<T>& InX, const Plane<T>& InY, const Plane<T>& InZ, const Plane<T>& InW);
	Matrix(const Vector3<T>& InX, const Vector3<T>& InY, const Vector3<T>& InZ, const Vector3<T>& InW);

	Matrix();

	static const Matrix Identity;


private:
};

template<typename T>
Matrix<T>::Matrix()
{

}

template<typename T>
Matrix<T>::Matrix(const Plane<T>& InX, const Plane<T>& InY, const Plane<T>& InZ, const Plane<T>& InW)
{
	M[0][0] = InX.X; M[0][1] = InX.Y; M[0][2] = InX.Z; M[0][3] = InX.W;
	M[1][0] = InY.X; M[1][1] = InY.Y; M[1][2] = InY.Z; M[1][3] = InY.W;
	M[2][0] = InZ.X; M[2][1] = InZ.Y; M[2][2] = InZ.Z; M[2][3] = InZ.W;
	M[3][0] = InW.X; M[3][1] = InW.Y; M[3][2] = InW.Z; M[3][3] = InW.W;
}

template<typename T>
Matrix<T>::Matrix(const Vector3<T>& InX, const Vector3<T>& InY, const Vector3<T>& InZ, const Vector3<T>& InW)
{
	M[0][0] = InX.X; M[0][1] = InX.Y; M[0][2] = InX.Z; M[0][3] = 0;
	M[1][0] = InY.X; M[1][1] = InY.Y; M[1][2] = InY.Z; M[1][3] = 0;
	M[2][0] = InZ.X; M[2][1] = InZ.Y; M[2][2] = InZ.Z; M[2][3] = 0;
	M[3][0] = InW.X; M[3][1] = InW.Y; M[3][2] = InW.Z; M[3][3] = 1;
}

template<typename T>
const Matrix<T> Matrix<T>::Identity = Matrix<T>(
	Vector3<int32>(1, 0, 0), Vector3<int32>(0, 1, 0), Vector3<int32>(0, 0, 1));

// -------------------------- view matrix ---------------------------------------------

template<typename T>
class ViewMatrix : public Matrix<T>
{
public:
	ViewMatrix(const Vector3f& Location, const Vector3f& ForwardVector,
		const Vector3f& RightVector, const Vector3f& UpVector);
};

template<typename T>
ViewMatrix<T>::ViewMatrix(const Vector3f& Location, const Vector3f& ForwardVector,
	const Vector3f& RightVector, const Vector3f& UpVector)
{
	for (int RowIndex = 0; RowIndex < 3; RowIndex++)
	{
		M[RowIndex][0] = RowIndex == 0 ? RightVector.X : RowIndex == 1 ? RightVector.Y : RightVector.Z;
		M[RowIndex][1] = RowIndex == 0 ? UpVector.X : RowIndex == 1 ? UpVector.Y : UpVector.Z;
		M[RowIndex][2] = RowIndex == 0 ? ForwardVector.X : RowIndex == 1 ? ForwardVector.Y : ForwardVector.Z;
		M[RowIndex][3] = 0.0f;
	}

	M[3][0] = Vector3f::DotProduct(Location * -1, RightVector);
	M[3][1] = Vector3f::DotProduct(Location * -1, UpVector);
	M[3][2] = Vector3f::DotProduct(Location * -1, ForwardVector);
	M[3][3] = 1.0f;
}

// ------------------------------------------------------------------------

template<typename T>
class ScaleTranslationMatrix : public Matrix<T>
{
public:
	ScaleTranslationMatrix(const Vector3f& Location, const Vector3f& Scale);
};

template<typename T>
ScaleTranslationMatrix<T>::ScaleTranslationMatrix(const Vector3f& Location, const Vector3f& Scale)
{
	M[0][0] = Scale.X;		M[0][1] = 0.0f;			M[0][2] = 0.0f;			M[0][3] = Location.X;
	M[1][0] = 0.0f;			M[1][1] = Scale.Y;		M[1][2] = 0.0f;			M[1][3] = Location.Y;
	M[2][0] = 0.0f;			M[2][1] = 0.0f;			M[2][2] = Scale.Z;		M[2][3] = Location.Z;
	M[3][0] = 0.0f;			M[3][1] = 0.0f;			M[3][2] = 0.0f;			M[3][3] = 1.0f;
}

// -------------------------------------------------------------------



template<typename T>
class ScaleRotationTranslationMatrix : public Matrix<T>
{
public:
	ScaleRotationTranslationMatrix(const Vector3f& Scale, const Rotatorf& Rotation, const Vector3f& Origin);
};

template<typename T>
ScaleRotationTranslationMatrix<T>::ScaleRotationTranslationMatrix(const Vector3f& Scale, const Rotatorf& Rot, const Vector3f& Origin)
{
	auto GetSinCos = [](float& S, float& C, float Degrees)
	{
		if (Degrees == 0.f)
		{
			S = 0.f;
			C = 1.f;
		}
		else if (Degrees == 90.f)
		{
			S = 1.f;
			C = 0.f;
		}
		else if (Degrees == 180.f)
		{
			S = 0.f;
			C = -1.f;
		}
		else if (Degrees == 270.f)
		{
			S = -1.f;
			C = 0.f;
		}
		else
		{
			Math::SinCos(&S, &C, Math::DegreesToRadians(Degrees));
		}
	};

	float SP, SY, SR;
	float CP, CY, CR;
	GetSinCos(SP, CP, -Rot.Yaw);
	GetSinCos(SY, CY, Rot.Roll);
	GetSinCos(SR, CR, Rot.Pitch);

	M[0][0] = (CP * CY) * Scale.X;
	M[0][1] = (CP * SY) * Scale.X;
	M[0][2] = (SP)*Scale.X;
	M[0][3] = 0.f;

	M[1][0] = (SR * SP * CY - CR * SY) * Scale.Y;
	M[1][1] = (SR * SP * SY + CR * CY) * Scale.Y;
	M[1][2] = (-SR * CP) * Scale.Y;
	M[1][3] = 0.f;

	M[2][0] = (-(CR * SP * CY + SR * SY)) * Scale.Z;
	M[2][1] = (CY * SR - CR * SP * SY) * Scale.Z;
	M[2][2] = (CR * CP) * Scale.Z;
	M[2][3] = 0.f;

	M[3][0] = Origin.X;
	M[3][1] = Origin.Y;
	M[3][2] = Origin.Z;
	M[3][3] = 1.f;
}

// -----------------------------------------------------------

template<typename T>
class PerspectiveMatrix : public Matrix<T>
{
public:
	PerspectiveMatrix(float FOV, float AspectRaio, float MinZ, float MaxZ);
};

template<typename T>
PerspectiveMatrix<T>::PerspectiveMatrix(float FOV, float AspectRaio, float MinZ, float MaxZ)
{
	float FOVFactor = Math::Tan(Math::DegreesToRadians(FOV/2));

	float Q = MaxZ / (MaxZ - MinZ);

	M[0][0] = 1 / FOVFactor;	M[0][1] = 0.0f;						M[0][2] = 0.0f;		M[0][3] = 0.0f;
	M[1][0] = 0.0f;				M[1][1] = AspectRaio / FOVFactor;	M[1][2] = 0.0f;		M[1][3] = 0.0f;
	M[2][0] = 0.0f;				M[2][1] = 0.0f;						M[2][2] = Q;		M[2][3] = 1.0f;
	M[3][0] = 0.0f;				M[3][1] = 0.0f;						M[3][2] = -Q*MinZ;	M[3][3] = 0.0f;
}
