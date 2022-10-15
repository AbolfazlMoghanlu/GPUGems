#pragma once

template<typename T>
struct Vector3
{
	static_assert(std::is_floating_point_v<T>, "T must be floating point");

	T X;
	T Y;
	T Z;

	/** (0, 0, 0) */
	static const Vector3<T> ZeroVector;

	/** (1, 1, 1) */
	static const Vector3<T> OneVector;

	/** (0, 0, 1) */
	static const Vector3<T> UpVector;
	
	/** (0, 0, -1) */
	static const Vector3<T> DownVector;

	/** (1, 0, 0) */
	static const Vector3<T> ForwardVector;

	/** (0, 1, 0) */
	static const Vector3<T> RightVector;

	Vector3() : X(0), Y(0), Z(0) {};
	Vector3(T InF) : X(InF), Y(InF), Z(InF) {};
	Vector3(T InX, T InY, T InZ) : X(InX), Y(InY), Z(InZ) {};

	const Vector3<T> operator+(const Vector3<T>& Vec3) const
	{
		return Vector3<T>(X + Vec3.X, Y + Vec3.Y, Z + Vec3.Z);
	}

	void operator+=(const Vector3<T>& Vec3)
	{
		X += Vec3.X;
		Y += Vec3.Y;
		Z += Vec3.Z;
	}

	const Vector3<T> operator-(const Vector3<T>& Vec3) const
	{
		return Vector3<T>(X - Vec3.X, Y - Vec3.Y, Z - Vec3.Z);
	}

	const Vector3<T> operator*(const Vector3<T>& Vec3) const
	{
		return Vector3<T>(X * Vec3.X, Y * Vec3.Y, Z * Vec3.Z);
	}

	const Vector3<T> operator/(const Vector3<T>& Vec3) const
	{
		return Vector3<T>(X / Vec3.X, Y / Vec3.Y, Z / Vec3.Z);
	}

	const Vector3 operator*(float f) const
	{
		return Vector3<T>(X * f, Y * f, Z * f);
	}

	void operator*=(float f)
	{
		X = X * f;
		Y = Y * f;
		Z = Z * f;
	}

	static Vector3<T> CrossProduct(const Vector3<T>& A, const Vector3<T>& B)
	{
		return Vector3<T>
			(
				A.Y * B.Z - A.Z * B.Y,
				A.Z * B.X - A.X * B.Z,
				A.X * B.Y - A.Y * B.X
				);
	}

	static float DotProduct(const Vector3<T>& A, const Vector3<T>& B)
	{
		return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
	}

	Vector3<T> Normalize()
	{
		// TODO: fast sqrt
		const T Scale = 1 / sqrt(X * X + Y * Y + Z * Z);
		return Vector3<T>(X * Scale, Y * Scale, Z * Scale);
	}

	std::string ToString() const
	{
		std::stringstream ss;
		ss << "X : ";
		ss << X;
		ss << ", Y : ";
		ss << Y;
		ss << ", Z : ";
		ss << Z;

		return ss.str();
	}
};


template<typename T>
const Vector3<T> Vector3<T>::ZeroVector = Vector3<T>(0);

template<typename T>
const Vector3<T> Vector3<T>::OneVector = Vector3<T>(1);

template<typename T>
const Vector3<T> Vector3<T>::UpVector = Vector3<T>(0, 1, 0);

template<typename T>
const Vector3<T> Vector3<T>::DownVector = Vector3<T>(0, -1, 0);

template<typename T>
const Vector3<T> Vector3<T>::ForwardVector = Vector3<T>(0, 0, 1);

template<typename T>
const Vector3<T> Vector3<T>::RightVector = Vector3<T>(1, 0, 0);


typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;
