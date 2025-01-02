#pragma once
#include <string>

namespace api
{
	using std::string;
	class Matrix3
	{
		friend class Math;
		friend class Vector3;
		friend class Matrix4;
	public:
		Matrix3();
		Matrix3(float n);
		Matrix3(const Matrix3& mat3);
		Matrix3(const Matrix4& mat4);
		Matrix3(
			float m00, float m01, float m02,
			float m10, float m11, float m12,
			float m20, float m21, float m22
		);
		// б�Գƾ����캯��
		Matrix3(const Vector3& v);
		// ��ת�����캯��(��axis����תangle��)
		Matrix3(const Vector3& axis, float angle);

		float GetDeterminant() const;
		Vector3 GetRow(uint32_t index) const;
		Vector3 GetColumn(uint32_t index) const;

		void ToRowMajorArray(float* array) const;
		void ToColumnMajorArray(float* array) const;

		string ToString() const;

		Matrix3& operator= (const Matrix3& mat);
		bool operator== (const Matrix3& mat) const;
		bool operator!= (const Matrix3& mat) const;
		Matrix3 operator- () const;
		Matrix3 operator* (float n) const;
		Matrix3 operator+ (const Matrix3& mat) const;
		Matrix3 operator- (const Matrix3& mat) const;
		Matrix3 operator* (const Matrix3& mat) const;
		Matrix3& operator*= (float n);
		Matrix3& operator+= (const Matrix3& mat);
		Matrix3& operator-= (const Matrix3& mat);
		Matrix3& operator*= (const Matrix3& mat);
		Vector3 operator* (const Vector3& v) const;
		friend Matrix3 operator* (float n, const Matrix3& mat);

	private:
		// ��һ��
		float m00; float m01; float m02;
		// �ڶ���
		float m10; float m11; float m12;
		// ������
		float m20; float m21; float m22;
	};
}