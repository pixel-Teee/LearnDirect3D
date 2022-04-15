#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "FrameResource.h"

namespace Shapes {
	using Microsoft::WRL::ComPtr;
	using namespace DirectX;
	using namespace DirectX::PackedVector;

	struct RenderItem {
		RenderItem() = default;

		XMFLOAT4X4 m_World = MathHelper::Identity4x4();

		int32_t m_NumFramesDirty = 3;

		//constant buffer index
		uint32_t ObjCBIndex = -1;

		MeshGeometry* Geo = nullptr;

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		//DrawIndexedInstanced Parameters
		uint32_t IndexCount = 0;
		uint32_t StartIndexLocation = 0;
		int32_t BaseVertexLocation = 0;
	};

	class ShapesApp : public D3DApp
	{
	public:
		ShapesApp(HINSTANCE hInstance);
		ShapesApp(const ShapesApp& rhs) = delete;
		ShapesApp& operator=(const ShapesApp& rhs) = delete;
		~ShapesApp();

		virtual bool Initialize() override;

	public:
		virtual void OnResize() override;
		virtual void Update(const GameTimer& gt) override;
		virtual void Draw(const GameTimer& gt) override;

		virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
		virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
		virtual void OnMouseMove(WPARAM btnState, int x, int y) override;

		void OnKeyboardInput(const GameTimer& gt);
		void UpdateCamera(const GameTimer& gt);
		void UpdateObjectCBs(const GameTimer& gt);
		void UpdateMainPassCB(const GameTimer& gt);

		//Initialize
		void BuildDescriptorHeaps();
		void BuildConstantBufferViews();
		void BuildRootSignature();
		void BuildShadersAndInputLayout();
		void BuildShapeGeometry();
		void BuildPSOs();
		void BuildFrameResources();
		void BuildRenderItems();
		//Draw Render Items
		void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
	private:
		//------FrameResources------
		std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
		FrameResource* m_CurrFrameResource = nullptr;
		int32_t m_CurrFrameResourceIndex = 0;
		//------FrameResources------

		ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_CbvHeap = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap = nullptr;

		std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> m_Geometries;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> m_Shaders;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_Psos;

		std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

		//list of all the render items
		std::vector<std::unique_ptr<RenderItem>> m_AllRitems;

		//render items divied by pso
		std::vector<RenderItem*> m_OpaqueRitems;

		PassConstants m_MainPassCB;

		uint32_t m_PassCbvOffset = 0;

		bool m_IsWireFrame = false;

		XMFLOAT3 m_EyePos = { 0.0f, 0.0f, 0.0f };
		XMFLOAT4X4 m_View = MathHelper::Identity4x4();
		XMFLOAT4X4 m_Proj = MathHelper::Identity4x4();

		float m_Theta = 1.5f * XM_PI;
		float m_Phi = 0.2f * XM_PI;
		float m_Radius = 15.0f;

		POINT m_LastMousePos;
	};
}

