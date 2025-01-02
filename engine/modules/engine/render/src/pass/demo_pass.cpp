#include "render/pass/demo_pass.h"
#include "render/asset/shader.h"
#include "render/asset/material.h"
#include "render/render_api.h"
#include "asset/resource_system.h"
#include "render/asset/vertex.h"
#include "render/asset/mesh.h"
namespace api {
	static RscHandle<Material> material;
	static RscHandle<Shader> shader;
	static RscHandle<Mesh> mesh;
	void DemoPass::Setup(FrameGraph& graph, RenderPassBuilder& builder)
	{
		builder.Name("MiniPass")
			.Type(RenderPassNodeType::Scene, RenderPassNodeFlag::Output)
			.Write(graph.GetSurface(), ResourceState::COLOR_ATTACHMENT)
				.Attachment(AttachmentFlag::Clear);
	}
	void DemoPass::Execute(FrameGraph& graph, RenderPassContext& ctx)
	{
		if (!shader) {
			shader = ResourceSystem::Ptr()->LoadEmplaceResource<Shader>(Guid("3585c167-9cff-4327-88db-d07601382640"));
			shader->Name("api::PosVertex");
			shader->ubName("api::ubSimple");
			auto vert = ResourceSystem::Ptr()->Load<ShaderProgram>("/engine/assets/shader/simple.vert");
			auto frag = ResourceSystem::Ptr()->Load<ShaderProgram>("/engine/assets/shader/simple.frag");
			shader->SetVertHandle(vert);
			shader->SetFragHandle(frag);
			RenderAPI::Ptr()->LoadShader(*shader, ctx.PassKey());
			vector<PosVertex> vertices = {
			   {.Position = { 0.0f, -0.5f, 0.f}}, // 底部顶点，红色
			   {.Position = { 0.5f,  0.5f, 0.f}}, // 右上顶点，绿色
			   {.Position = {-0.5f,  0.5f, 0.f}}  // 左上顶点，蓝色
			};
			vector<uint32_t> indices = { 0, 1, 2 };
			mesh = ResourceSystem::Ptr()->LoadEmplaceResource<Mesh>(vertices, indices);
			material = ResourceSystem::Ptr()->LoadEmplaceResource<Material>(shader);
			mesh->SetMaterial(material);
			RenderAPI::Ptr()->SetStaticMesh(*mesh);
			RenderAPI::Ptr()->SetUpMaterialInstance(mesh->GetMaterialInstance());
			auto materialInstance = mesh->GetMaterialInstance();
			materialInstance.SetClassResource(Name("uColor"), Vector4{1,0,0,1});
		}
		auto& surface = graph.GetSurface();
		ctx->SetViewport(0.0f, 0.0f,(float)surface.width,(float)surface.height, 0.f, 1.f);
		ctx->SetScissor(0, 0, surface.width, surface.height);
		RenderAPI::Ptr()->DrawStaticMesh(*mesh);
	}
}
