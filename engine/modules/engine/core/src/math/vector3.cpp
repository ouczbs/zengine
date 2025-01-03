#include <cassert>
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"
#include "math/matrix3.h"
#include "math/math.h"
#include "zlog.h"

namespace api
{
	const Vector3 Vector3::Zero    = Vector3( 0.0f,  0.0f,  0.0f);
	const Vector3 Vector3::One     = Vector3( 1.0f,  1.0f,  1.0f);
	const Vector3 Vector3::Up      = Vector3( 0.0f,  1.0f,  0.0f);
	const Vector3 Vector3::Down    = Vector3( 0.0f, -1.0f,  0.0f);
	const Vector3 Vector3::Left    = Vector3(-1.0f,  0.0f,  0.0f);
	const Vector3 Vector3::Right   = Vector3( 1.0f,  0.0f,  0.0f);
	const Vector3 Vector3::Forward = Vector3( 0.0f,  0.0f,  1.0f);
	const Vector3 Vector3::Back    = Vector3( 0.0f,  0.0f, -1.0f);

	Vector3::Vector3()
	{
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
	}

	Vector3::Vector3(float n)
	{
		this->x = n;
		this->y = n;
		this->z = n;
	}

	Vector3::Vector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3::Vector3(const Vector2& v2)
	{
		this->x = v2.x;
		this->y = v2.y;
		this->z = 0.0f;
	}

	Vector3::Vector3(const Vector2& v2, float z)
	{
		this->x = v2.x;
		this->y = v2.y;
		this->z = z;
	}

	Vector3::Vector3(float x, const Vector2& v2)
	{
		this->x = x;
		this->y = v2.x;
		this->z = v2.y;
	}

	Vector3::Vector3(const Vector3& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	Vector3::Vector3(const Vector4& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	void Vector3::Normalize()
	{
		float l = sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
		x /= l;
		y /= l;
		z /= l;
	}

	Vector3 Vector3::GetNormalized() const
	{
		float l = sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2));
		return Vector3(x / l, y / l, z / l);
	}

	void Vector3::Absolutize()
	{
		x = fabsf(x);
		y = fabsf(y);
		z = fabsf(z);
	}

	Vector3 Vector3::GetAbsolutized() const
	{
		return Vector3(fabsf(x), fabsf(y), fabsf(z));
	}

	void Vector3::ToArray(float* array) const
	{
		array[0] = x;
		array[1] = y;
		array[2] = z;
	}

	string Vector3::ToString() const
	{
		return "Vector3(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
	}

	float Vector3::GetMagnitude() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float Vector3::GetMagnitudeSquared() const
	{
		return x * x + y * y + z * z;
	}

	void Vector3::Clear()
	{
		x = y = z = 0.0f;
	}

	Vector4 Vector3::ToPosVec4() const
	{
		return Vector4(x, y, z, 1.0f);
	}

	Vector4 Vector3::ToDirVec4() const
	{
		return Vector4(x, y, z, 0.0f);
	}

	float& Vector3::operator[] (int i)
	{
		switch (i)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			zlog::error("Invalid index to Vector3 !");
			assert(false);
			return x;
			break;
		}
	}

	const float& Vector3::operator[] (int i) const
	{
		switch (i)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			zlog::error("Invalid index to Vector3 !");
			assert(false);
			return x;
			break;
		}
	}

	bool Vector3::operator== (const Vector3& v) const
	{
		return Math::Approximately(x, v.x) && Math::Approximately(y, v.y) && Math::Approximately(z, v.z);
	}

	bool Vector3::operator!= (const Vector3& v) const
	{
		return !Math::Approximately(x, v.x) || !Math::Approximately(y, v.y) || !Math::Approximately(z, v.z);
	}

	Vector3& Vector3::operator= (const Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Vector3 Vector3::operator- () const
	{
		return Vector3(-x, -y, -z);
	}

	Vector3 Vector3::operator+ (const Vector3& v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 Vector3::operator- (const Vector3& v) const
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 Vector3::operator* (const Vector3& v) const
	{
		return Vector3(x * v.x, y * v.y, z * v.z);
	}

	Vector3 Vector3::operator/ (const Vector3& v) const
	{
		return Vector3(x / v.x, y / v.y, z / v.z);
	}

	Vector3& Vector3::operator+= (const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3& Vector3::operator-= (const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3& Vector3::operator*= (const Vector3& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	Vector3& Vector3::operator/= (const Vector3& v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	Vector3 Vector3::operator+ (float n) const
	{
		return Vector3(x + n, y + n, z + n);
	}

	Vector3 Vector3::operator- (float n) const
	{
		return Vector3(x - n, y - n, z - n);
	}

	Vector3 Vector3::operator* (float n) const
	{
		return Vector3(x * n, y * n, z * n);
	}

	Vector3 Vector3::operator/ (float n) const
	{
		return Vector3(x / n, y / n, z / n);
	}

	Vector3& Vector3::operator+= (float n)
	{
		x += n;
		y += n;
		z += n;
		return *this;
	}

	Vector3& Vector3::operator-= (float n)
	{
		x -= n;
		y -= n;
		z -= n;
		return *this;
	}

	Vector3& Vector3::operator*= (float n)
	{
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}

	Vector3& Vector3::operator/= (float n)
	{
		x /= n;
		y /= n;
		z /= n;
		return *this;
	}

	Vector3 operator+ (float n, const Vector3& v)
	{
		return Vector3(n + v.x, n + v.y, n + v.z);
	}

	Vector3 operator- (float n, const Vector3& v)
	{
		return Vector3(n - v.x, n - v.y, n - v.z);
	}

	Vector3 operator* (float n, const Vector3& v)
	{
		return Vector3(n * v.x, n * v.y, n * v.z);
	}

	Vector3 operator/ (float n, const Vector3& v)
	{
		return Vector3(n / v.x, n / v.y, n / v.z);
	}

	Vector3 Vector3::operator* (const Matrix3& mat) const
	{
		float x = this->x * mat.m00 + this->y * mat.m10 + this->z * mat.m20;
		float y = this->x * mat.m01 + this->y * mat.m11 + this->z * mat.m21;
		float z = this->x * mat.m02 + this->y * mat.m12 + this->z * mat.m22;

		return Vector3(x, y, z);
	}

	Vector3& Vector3::operator*= (const Matrix3& mat)
	{
		float tmp_x = x * mat.m00 + y * mat.m10 + z * mat.m20;
		float tmp_y = x * mat.m01 + y * mat.m11 + z * mat.m21;
		float tmp_z = x * mat.m02 + y * mat.m12 + z * mat.m22;

		x = tmp_x;
		y = tmp_y;
		z = tmp_z;

		return *this;
	}
}