#pragma once
#include "math/vector4.h"
#include "math/vector3.h"
#include "math/vector2.h"
#include "refl/pch.h"
// 顶点最多关联4个骨骼
#define MAX_NUM_BONES_PER_VERTEX 4
namespace api {
	struct Vertex {};
	template<typename T>
	concept Vertexs = requires {std::is_base_of_v<Vertex, T>; };
	struct PosVertex : public Vertex {
		UPROPERTY_gl({}, glVertexMeta{ 3, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk({}, uint32_t{ VK_FORMAT_R32G32B32_SFLOAT })
		Vector3 Position = {};
	};
	struct TexVertex : public Vertex {
		UPROPERTY_gl({}, glVertexMeta{ 3, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk({}, uint32_t{ VK_FORMAT_R32G32B32_SFLOAT })
		Vector3 Position = {};
		UPROPERTY_gl({}, glVertexMeta{ 2, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk()
		Vector2 TexCoords = {};
		UPROPERTY_gl({}, glVertexMeta{ 3, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk()
		Vector3 Normal = {};
	};
	struct BoneVertex : public Vertex
	{
		UPROPERTY_gl({}, glVertexMeta{ 3, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk({}, uint32_t{ VK_FORMAT_R32G32B32_SFLOAT })
		Vector3 Position = {};
		UPROPERTY_gl({}, glVertexMeta{ 2, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk()
		Vector2 TexCoords = {};
		UPROPERTY_gl({}, glVertexMeta{ 3, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk()
		Vector3 Normal = {};
		UPROPERTY_gl({}, glVertexMeta{ 3, GL_FLOAT, GL_FALSE })
		UPROPERTY_vk()
		Vector3 Tangent = {};
		// 骨骼蒙皮数据
		UPROPERTY_vk()
		float    Weights[MAX_NUM_BONES_PER_VERTEX] = {};
		UPROPERTY_vk()
		uint32_t BoneIDs[MAX_NUM_BONES_PER_VERTEX] = {};
	};
};
#include ".render/vertex_gen.inl"