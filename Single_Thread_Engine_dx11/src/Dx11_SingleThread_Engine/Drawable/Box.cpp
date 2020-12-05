#include "Box.h"
#include "../Bindable/BindVertexBuffer.h"
#include "../Bindable/IndexBuffer.h"
#include "../objInfo/Vertex.h"
#include "../Bindable/VertexShader.h"
#include "../Bindable/PixelShader.h"
#include "../Bindable/BindConstantBuffer.h"
#include "../Bindable/InputLayout.h"
#include "../Bindable/Topology.h"
#include "../Bindable/TransformCbuf.h"

Box::Box(Graphics& _gfx, std::mt19937& _rng,
	std::uniform_real_distribution<float>& _adist,
	std::uniform_real_distribution<float>& _ddist,
	std::uniform_real_distribution<float>& _odist,
	std::uniform_real_distribution<float>& _rdist)
	:
	r_(_rdist(_rng)),
	roll_(0.0f),
	pitch_(0.0f),
	yaw_(0.0f),
	theta_(_adist(_rng)),
	phi_(_adist(_rng)),
	chi_(_adist(_rng)),
	droll_(_ddist(_rng)),
	dpitch_(_ddist(_rng)),
	dyaw_(_ddist(_rng)),
	dtheta_(_odist(_rng)),
	dphi_(_odist(_rng)),
	dchi_(_odist(_rng))
{
	if (!IsStaticInitialized()) {
		
		struct Vertex
		{
			struct
			{
				float x;
				float y;
				float z;
			} pos;
		};

		const std::vector<Vertex> vertices =
		{
			{ -1.0f, -1.0f, -1.0f },
			{  1.0f, -1.0f, -1.0f },
			{ -1.0f,  1.0f, -1.0f },
			{  1.0f,  1.0f, -1.0f },
			{ -1.0f, -1.0f,  1.0f },
			{  1.0f, -1.0f,  1.0f },
			{ -1.0f,  1.0f,  1.0f },
			{  1.0f,  1.0f,  1.0f }
		};
		AddBind(std::make_unique<BindVertexBuffer>(_gfx, vertices));

		auto pvs = std::make_unique<VertexShader>(_gfx, "VertexShader.cso");
		auto pvsbc = pvs->GetBytecode();
		AddBind(std::move(pvs));

		AddBind(std::make_unique<PixelShader>(_gfx, "PixelShader.cso"));

		// VertexBuffer vb;
		// 
		// AddStaticBind(std::make_unique<BindVertexBuffer>(_gfx, vb));
		// 
		// auto pvs = std::make_unique<VertexShader>(_gfx, L"VertexShader.cso");
		// auto pvsbc = pvs->GetBytecode();
		// AddStaticBind(std::move(pvs));
		// 
		// AddStaticBind(std::make_unique<PixelShader>(_gfx, L"PixelShader.cso"));

		const std::vector<unsigned short> indices =
		{
			0, 2, 1, 2, 3, 1,
			1, 3, 5, 3, 7, 5,
			2, 6, 3, 3, 6, 7,
			4, 5, 7, 4, 7, 6,
			0, 4, 2, 2, 4, 6,
			0, 1, 4, 1, 5, 4
		};
		AddIndexBuffer(std::make_unique<IndexBuffer>(_gfx, indices));

		struct ConstantBuffer2
		{
			struct
			{
				float r_;
				float g_;
				float b_;
				float a_;
			} face_colors[6];
		};

		const ConstantBuffer2 cb2 =
		{
			{
				{ 1.0f, 0.0f, 1.0f},
				{ 1.0f, 0.0f, 0.0f},
				{ 0.0f, 1.0f, 0.0f},
				{ 0.0f, 0.0f, 1.0f},
				{ 1.0f, 1.0f, 0.0f},
				{ 0.0f, 1.0f, 1.0f},
			}
		};
		//AddBind(std::make_unique<BindPixelConstantBuffer<ConstantBuffer2>>(_gfx, cb2));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		AddBind(std::make_unique<InputLayout>(_gfx, ied, pvsbc));
		AddBind(std::make_unique<Topology>(_gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	//	AddBind(std::make_unique<TransformCbuf>(_gfx, *this));
	}
}

void Box::Update(float _dt) noexcept
{
	roll_ += droll_ * _dt;
	pitch_ += dpitch_ * _dt;
	yaw_ += dyaw_ * _dt;
	theta_ += dtheta_ * _dt;
	phi_ += dphi_ * _dt;
	chi_ += dchi_ * _dt;
}

DirectX::XMMATRIX Box::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(pitch_, yaw_, roll_) *
		DirectX::XMMatrixTranslation(r_, 0.0f, 0.0f) * 
		DirectX::XMMatrixRotationRollPitchYaw(theta_, phi_, chi_) *
		DirectX::XMMatrixTranslation(0.0f, 0.0f, 20.0f);
}