#include "game/view/PourStream.h"
#include "core/utility/MathConstants.h"
#include "game/constant/Palette.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Color;
	using core::utility::mixed;
	using core::utility::toChannel;
	using core::utility::Vector3;
	using core::utility::Vertex3D;
	namespace palette = game::constant::palette;

	/// @brief 筋を縦に何段に分けるか
	constexpr int RING_COUNT{ 22 };

	/// @brief 筒の周りを何角形で作るか
	constexpr int SIDE_COUNT{ 10 };

	/// @brief 注ぎ口のところの太さ
	constexpr float RADIUS_TOP{ 0.052f };

	/// @brief 落ちきったところの太さ（速くなるぶん細くなる）
	constexpr float RADIUS_BOTTOM{ 0.022f };

	/// @brief 太さのむらの大きさ（大きくすると縞に見えてしまう）
	constexpr float BULGE{ 0.05f };

	/// @brief 中心の横ぶれの大きさ
	constexpr float SWAY{ 0.012f };

	/// @brief 勢いが立ち上がる速さ
	constexpr float FLOW_RISE{ 9.0f };

	/// @brief 勢いが引く速さ
	constexpr float FLOW_FALL{ 6.0f };

	/// @brief 見えているとみなす勢いの下限
	constexpr float FLOW_EPSILON{ 0.02f };

	/// @brief 光が来る向き（正規化済み）
	constexpr Vector3 LIGHT_DIRECTION{ 0.36f, 0.88f, -0.31f };

	/// @brief 縁の明るさ
	constexpr float RIM_STRENGTH{ 0.85f };

	/// @brief 筋の濃さ（少し透けるほうが液体らしい）
	constexpr float STREAM_ALPHA{ 0.88f };

	/**
	 * @brief 筒の表面の色を決める
	 * @param position 座標
	 * @param normal 法線
	 * @param cameraPosition 視点の座標
	 * @return 頂点の色
	 */
	Color shadeStream(const Vector3& position, const Vector3& normal, const Vector3& cameraPosition)
	{
		const Vector3 view{ (cameraPosition - position).normalized() };
		const Vector3 halfway{ (LIGHT_DIRECTION + view).normalized() };

		const float diffuse{ 0.58f + 0.42f * std::max(0.0f, normal.dot(LIGHT_DIRECTION)) };
		const float specular{ std::pow(std::max(0.0f, normal.dot(halfway)), 40.0f) * 1.1f };

		// 正面を向いた面より、視線に対して横を向いた縁のほうが明るい。
		// 透けた液体を通した光がまわり込むので、輪郭が光って見える
		const float rim{ std::pow(1.0f - std::abs(normal.dot(view)), 1.8f) * RIM_STRENGTH };

		const Color base{ palette::STREAM };
		const Color shine{ palette::LIQUID_SHINE };
		const float bright{ specular + rim };

		return Color{ toChannel(base.r * diffuse + shine.r * bright),
			          toChannel(base.g * diffuse + shine.g * bright),
			          toChannel(base.b * diffuse + shine.b * bright) };
	}
} // namespace

namespace game::view
{
	void PourStream::advance(float deltaTime, bool isPouring)
	{
		m_time += deltaTime;
		m_isPouring = isPouring;

		// 勢いは瞬時に切り替わらない。止めたあとも少しのあいだ細く落ち続ける
		const float target{ isPouring ? 1.0f : 0.0f };
		const float rate{ isPouring ? FLOW_RISE : FLOW_FALL };
		m_flow += (target - m_flow) * std::min(1.0f, rate * deltaTime);
	}

	bool PourStream::isVisible() const noexcept
	{
		return m_flow > FLOW_EPSILON;
	}

	core::utility::Vector3 PourStream::centerAt(float t, float surfaceHeight) const
	{
		// 上ほど大きく揺れ、落ちるにつれて真っ直ぐになる
		const float sway{ std::sin(m_time * 3.1f + t * 2.4f) * SWAY * (1.0f - t * 0.7f) };
		const float swayZ{ std::cos(m_time * 2.6f + t * 1.9f) * SWAY * 0.6f * (1.0f - t * 0.7f) };
		const float y{ m_origin.y + (surfaceHeight - m_origin.y) * t };
		return Vector3{ m_origin.x + sway, y, m_origin.z + swayZ };
	}

	float PourStream::radiusAt(float t) const
	{
		// 太さのむらが下へ流れていくと、落ちている速さが伝わる
		const float bulge{ 1.0f + BULGE * std::sin(t * 5.0f - m_time * 7.0f) +
			               BULGE * 0.5f * std::sin(t * 9.0f - m_time * 11.0f) };
		const float base{ RADIUS_TOP + (RADIUS_BOTTOM - RADIUS_TOP) * t };
		return base * bulge * m_flow;
	}

	float PourStream::getImpactX(float surfaceHeight) const
	{
		return centerAt(1.0f, surfaceHeight).x;
	}

	float PourStream::getImpactZ(float surfaceHeight) const
	{
		return centerAt(1.0f, surfaceHeight).z;
	}

	void PourStream::build(std::vector<core::utility::Vertex3D>& vertices,
	                       std::vector<unsigned short>& indices, float surfaceHeight,
	                       const core::utility::Vector3& cameraPosition) const
	{
		if (!isVisible())
			return;

		const auto baseIndex{ static_cast<unsigned short>(vertices.size()) };

		for (int ring{ 0 }; ring <= RING_COUNT; ++ring)
		{
			const float t{ static_cast<float>(ring) / RING_COUNT };
			const Vector3 center{ centerAt(t, surfaceHeight) };
			const float radius{ radiusAt(t) };

			for (int side{ 0 }; side <= SIDE_COUNT; ++side)
			{
				const float angle{ core::utility::math::TWO_PI * side / SIDE_COUNT };
				const Vector3 normal{ std::cos(angle), 0.0f, std::sin(angle) };

				Vertex3D vertex{};
				vertex.position = center + normal * radius;
				vertex.normal = normal;
				vertex.color = shadeStream(vertex.position, normal, cameraPosition);
				vertex.alpha = STREAM_ALPHA;
				vertex.u = static_cast<float>(side) / SIDE_COUNT;
				vertex.v = t;
				vertices.push_back(vertex);
			}
		}

		constexpr int STRIDE{ SIDE_COUNT + 1 };
		for (int ring{ 0 }; ring < RING_COUNT; ++ring)
		{
			for (int side{ 0 }; side < SIDE_COUNT; ++side)
			{
				const auto topLeft{ static_cast<unsigned short>(baseIndex + ring * STRIDE + side) };
				const auto topRight{ static_cast<unsigned short>(topLeft + 1) };
				const auto bottomLeft{ static_cast<unsigned short>(topLeft + STRIDE) };
				const auto bottomRight{ static_cast<unsigned short>(bottomLeft + 1) };

				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
				indices.push_back(topRight);

				indices.push_back(topRight);
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
			}
		}
	}
} // namespace game::view
