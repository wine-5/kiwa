#include "game/view/WaterSurface.h"
#include "core/utility/MathConstants.h"
#include "game/constant/Palette.h"
#include "game/view/CupGeometry.h"
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
	namespace cup = game::view::cup;

	// ---- 面の細かさ ----

	/// @brief 中心から縁までの分割数
	constexpr int RING_COUNT{ 18 };

	/// @brief 円周の分割数
	constexpr int SEGMENT_COUNT{ 56 };

	// ---- 着水の波 ----

	/// @brief 波が広がる速さ
	constexpr float WAVE_SPEED{ 0.62f };

	/// @brief 波の高さ
	constexpr float WAVE_AMPLITUDE{ 0.020f };

	/// @brief 波が収まる速さ
	constexpr float WAVE_DECAY{ 1.5f };

	/// @brief 波の峰の幅（小さいほど細い輪になる）
	constexpr float WAVE_WIDTH{ 0.16f };

	/// @brief 波の細かさ
	///
	/// 細かくすると、間隔を詰めて起こした波どうしが打ち消し合って面が平らになる
	constexpr float WAVE_FREQUENCY{ 26.0f };

	/// @brief 波を保っておく時間（秒）
	constexpr float WAVE_LIFE{ 2.2f };

	/// @brief 同時に持つ波の上限
	constexpr std::size_t WAVE_LIMIT{ 12 };

	// ---- 常にある細かなうねり ----

	/// @brief うねりの高さ
	constexpr float SWELL_AMPLITUDE{ 0.0016f };

	// ---- 壁際の盛り上がり ----

	/// @brief 縁に沿って持ち上がる高さ（表面張力）
	constexpr float MENISCUS_HEIGHT{ 0.006f };

	/// @brief 盛り上がりが効く範囲
	constexpr float MENISCUS_WIDTH{ 0.02f };

	// ---- 注がれている場所 ----

	/// @brief くぼみの深さ
	constexpr float POUR_DIP{ 0.016f };

	/// @brief くぼみの広さ
	constexpr float POUR_DIP_RADIUS{ 0.075f };

	/// @brief 泡立つ範囲の広さ
	constexpr float FOAM_RADIUS{ 0.07f };

	// ---- 陰影 ----

	/// @brief 光が来る向き（器の陰影と揃える）
	constexpr Vector3 LIGHT_DIRECTION{ 0.42f, 0.83f, -0.36f };

	/// @brief 鋭い照りの絞り
	constexpr float SPECULAR_POWER{ 90.0f };

	/// @brief 鋭い照りの強さ（1を超えさせて白飛びさせ、滲みの処理に拾わせる）
	constexpr float SPECULAR_STRENGTH{ 3.2f };

	/// @brief 広い照りの絞り（面のうねりはこれで見える）
	constexpr float SHEEN_POWER{ 10.0f };

	/// @brief 広い照りの強さ
	constexpr float SHEEN_STRENGTH{ 0.85f };

	/// @brief 映り込みの効き方（浅い角度ほど強く映る）
	constexpr float REFLECT_MIN{ 0.10f };
	constexpr float REFLECT_MAX{ 0.72f };

	/// @brief 上を向いた面に映るもの
	constexpr Color REFLECT_SKY{ 212, 200, 178 };

	/// @brief 横～下を向いた面に映るもの（器の内側）
	constexpr Color REFLECT_ROOM{ 74, 82, 76 };

	/// @brief 泡の色
	constexpr Color FOAM_COLOR{ 250, 244, 228 };

	/// @brief 深さで濁っていく速さ
	constexpr float ABSORPTION{ 5.6f };

	/// @brief 浅い角度から見たときに増す濃さ
	constexpr float GRAZING_OPACITY{ 0.45f };

	// ---- 水中の光の模様 ----

	/// @brief 模様の強さ
	constexpr float CAUSTIC_GAIN{ 55.0f };

	/// @brief 模様の色
	constexpr Color CAUSTIC_COLOR{ 255, 243, 205 };

	/// @brief 底から少し浮かせる高さ
	constexpr float CAUSTIC_LIFT{ 0.003f };

	/**
	 * @brief 面の向きと視点から、水面の色を決める
	 * @param position 座標
	 * @param normal 法線
	 * @param cameraPosition 視点の座標
	 * @param foam 泡立ちの強さ（0.0〜1.0）
	 * @param depth その場所の水の厚み
	 * @param outAlpha 透明度の出力先
	 * @return 頂点の色
	 */
	Color shadeSurface(const Vector3& position, const Vector3& normal, const Vector3& cameraPosition,
	                   float foam, float depth, float& outAlpha)
	{
		const Vector3 view{ (cameraPosition - position).normalized() };
		const Vector3 halfway{ (LIGHT_DIRECTION + view).normalized() };
		const float facing{ std::max(0.0f, normal.dot(view)) };

		// 水が水に見えるのは、波の傾きによって映り込む先が変わるから
		const Vector3 reflected{ view.reflected(normal).normalized() };
		const float skyAmount{ std::clamp(0.5f + 0.5f * reflected.y, 0.0f, 1.0f) };
		const Color reflection{ mixed(REFLECT_ROOM, REFLECT_SKY, skyAmount * skyAmount) };

		const float grazing{ std::pow(1.0f - facing, 3.0f) };
		const float reflectRate{ REFLECT_MIN + (REFLECT_MAX - REFLECT_MIN) * grazing };

		// 液体は厚いほど光を吸う。浅いところは底の色が透ける
		const float absorbed{ 1.0f - std::exp(-depth * ABSORPTION) };

		const float diffuse{ 0.52f + 0.48f * std::max(0.0f, normal.dot(LIGHT_DIRECTION)) };
		const Color base{ mixed(palette::LIQUID_SURFACE, palette::LIQUID, absorbed) };
		Color result{ mixed(core::utility::scaled(base, diffuse), reflection, reflectRate) };

		// 広い照りで面のうねりを見せ、鋭い照りで峰にきらめきを載せる
		const float alignment{ std::max(0.0f, normal.dot(halfway)) };
		const float shine{ std::pow(alignment, SHEEN_POWER) * SHEEN_STRENGTH +
			               std::pow(alignment, SPECULAR_POWER) * SPECULAR_STRENGTH };

		const Color shineColor{ palette::LIQUID_SHINE };
		result = Color{ toChannel(result.r + shineColor.r * shine),
			            toChannel(result.g + shineColor.g * shine),
			            toChannel(result.b + shineColor.b * shine) };

		outAlpha = std::clamp(absorbed + grazing * GRAZING_OPACITY, 0.0f, 0.98f);

		if (foam > 0.0f)
		{
			const float amount{ std::min(1.0f, foam * 0.5f) };
			result = mixed(result, FOAM_COLOR, amount);
			outAlpha = std::min(1.0f, outAlpha + amount * 0.5f);
		}

		return result;
	}
} // namespace

namespace game::view
{
	void WaterSurface::disturb(float x, float z, float strength)
	{
		if (m_waves.size() >= WAVE_LIMIT)
			m_waves.erase(m_waves.begin());

		m_waves.push_back(Wave{ x, z, 0.0f, strength });
	}

	void WaterSurface::advance(float deltaTime)
	{
		m_time += deltaTime;

		for (Wave& wave : m_waves)
			wave.age += deltaTime;

		std::erase_if(m_waves, [](const Wave& wave) { return wave.age > WAVE_LIFE; });
	}

	float WaterSurface::getLevelHeight() const noexcept
	{
		return cup::surfaceHeight(m_levelRatio);
	}

	float WaterSurface::heightAt(float x, float z) const
	{
		return getLevelHeight() + displacementAt(x, z);
	}

	float WaterSurface::displacementAt(float x, float z) const
	{
		// 常に少しだけうねらせておく。完全に止まった面は水に見えない
		float height{ SWELL_AMPLITUDE * std::sin(x * 22.0f + m_time * 2.1f) *
			              std::cos(z * 19.0f - m_time * 1.7f) };

		// 着水の波は、輪が広がりながら細かく振動して減衰していく
		for (const Wave& wave : m_waves)
		{
			const float dx{ x - wave.originX };
			const float dz{ z - wave.originZ };
			const float distance{ std::sqrt(dx * dx + dz * dz) };
			const float front{ WAVE_SPEED * wave.age };
			const float offset{ (distance - front) / WAVE_WIDTH };

			const float envelope{ std::exp(-WAVE_DECAY * wave.age) * std::exp(-offset * offset) };
			height += wave.strength * WAVE_AMPLITUDE * envelope *
			          std::sin((distance - front) * WAVE_FREQUENCY);
		}

		// 縁に接したところは表面張力で持ち上がる
		const float radius{ std::sqrt(x * x + z * z) };
		const float edgeDistance{ std::max(0.0f, cup::radiusAt(getLevelHeight()) - radius) };
		height += MENISCUS_HEIGHT * std::exp(-edgeDistance / MENISCUS_WIDTH);

		// 注がれているところは押されてへこむ
		if (m_isPouring)
		{
			const float dx{ x - m_pourX };
			const float dz{ z - m_pourZ };
			const float distance{ std::sqrt(dx * dx + dz * dz) / POUR_DIP_RADIUS };
			height -= POUR_DIP * std::exp(-distance * distance);
		}

		return height;
	}

	core::utility::Vector3 WaterSurface::normalAt(float x, float z) const
	{
		// 前後左右の高さの差から傾きを求める
		constexpr float STEP{ 0.008f };
		const float dx{ displacementAt(x + STEP, z) - displacementAt(x - STEP, z) };
		const float dz{ displacementAt(x, z + STEP) - displacementAt(x, z - STEP) };

		return Vector3{ -dx, 2.0f * STEP, -dz }.normalized();
	}

	float WaterSurface::foamAt(float x, float z) const
	{
		float foam{ 0.0f };

		if (m_isPouring)
		{
			const float dx{ x - m_pourX };
			const float dz{ z - m_pourZ };
			const float distance{ std::sqrt(dx * dx + dz * dz) / FOAM_RADIUS };
			foam += std::exp(-distance * distance);
		}

		// 波の峰にも泡が乗って一緒に広がっていく
		for (const Wave& wave : m_waves)
		{
			const float dx{ x - wave.originX };
			const float dz{ z - wave.originZ };
			const float distance{ std::sqrt(dx * dx + dz * dz) };
			const float offset{ (distance - WAVE_SPEED * wave.age) / (WAVE_WIDTH * 0.6f) };
			foam += 0.22f * wave.strength * std::exp(-WAVE_DECAY * 1.6f * wave.age) *
			        std::exp(-offset * offset);
		}

		return std::min(1.0f, foam);
	}

	void WaterSurface::build(std::vector<core::utility::Vertex3D>& vertices,
	                         std::vector<unsigned short>& indices,
	                         const core::utility::Vector3& cameraPosition) const
	{
		if (m_levelRatio <= 0.0f)
			return;

		const float level{ getLevelHeight() };
		const float outerRadius{ cup::radiusAt(level) };
		const auto baseIndex{ static_cast<unsigned short>(vertices.size()) };

		// 円い器なので、中心から輪を広げて面を張る
		for (int ring{ 0 }; ring <= RING_COUNT; ++ring)
		{
			const float radius{ outerRadius * ring / RING_COUNT };

			for (int segment{ 0 }; segment < SEGMENT_COUNT; ++segment)
			{
				const float angle{ core::utility::math::TWO_PI * segment / SEGMENT_COUNT };
				const float x{ std::cos(angle) * radius };
				const float z{ std::sin(angle) * radius };

				Vertex3D vertex{};
				vertex.position = Vector3{ x, heightAt(x, z), z };
				vertex.normal = normalAt(x, z);

				const float depth{ std::max(0.0f, vertex.position.y - cup::FLOOR_TOP) };
				vertex.color = shadeSurface(vertex.position, vertex.normal, cameraPosition,
				                            foamAt(x, z), depth, vertex.alpha);
				vertices.push_back(vertex);
			}
		}

		for (int ring{ 0 }; ring < RING_COUNT; ++ring)
		{
			for (int segment{ 0 }; segment < SEGMENT_COUNT; ++segment)
			{
				const int next{ (segment + 1) % SEGMENT_COUNT };
				const auto inner{ static_cast<unsigned short>(baseIndex + ring * SEGMENT_COUNT + segment) };
				const auto innerNext{ static_cast<unsigned short>(baseIndex + ring * SEGMENT_COUNT + next) };
				const auto outer{ static_cast<unsigned short>(inner + SEGMENT_COUNT) };
				const auto outerNext{ static_cast<unsigned short>(innerNext + SEGMENT_COUNT) };

				indices.push_back(inner);
				indices.push_back(outer);
				indices.push_back(innerNext);

				indices.push_back(innerNext);
				indices.push_back(outer);
				indices.push_back(outerNext);
			}
		}
	}

	void WaterSurface::buildWall(std::vector<core::utility::Vertex3D>& vertices,
	                             std::vector<unsigned short>& indices) const
	{
		if (m_levelRatio <= 0.0f)
			return;

		constexpr int WALL_RINGS{ 8 };

		// 器の内側にぴたりと沿わせる。少し内側へ寄せて、壁を突き抜けないようにする
		constexpr float HUG{ 0.965f };

		const float level{ getLevelHeight() };
		const auto baseIndex{ static_cast<unsigned short>(vertices.size()) };

		for (int ring{ 0 }; ring <= WALL_RINGS; ++ring)
		{
			const float t{ static_cast<float>(ring) / WALL_RINGS };
			const float y{ cup::FLOOR_TOP + (level - cup::FLOOR_TOP) * t };
			const float radius{ cup::radiusAt(y) * HUG };

			// 深いところほど暗い。上へ行くほど液体そのものの色に近づく
			const Color color{ core::utility::scaled(palette::LIQUID, 0.42f + 0.38f * t) };

			for (int segment{ 0 }; segment < SEGMENT_COUNT; ++segment)
			{
				const float angle{ core::utility::math::TWO_PI * segment / SEGMENT_COUNT };

				Vertex3D vertex{};
				vertex.position = Vector3{ std::cos(angle) * radius, y, std::sin(angle) * radius };
				// 中心を向かせる。覗き込んだときに見えるのは向こう側の内壁だから
				vertex.normal = Vector3{ -std::cos(angle), 0.0f, -std::sin(angle) };
				vertex.color = color;
				vertices.push_back(vertex);
			}
		}

		for (int ring{ 0 }; ring < WALL_RINGS; ++ring)
		{
			for (int segment{ 0 }; segment < SEGMENT_COUNT; ++segment)
			{
				const int next{ (segment + 1) % SEGMENT_COUNT };
				const auto lower{ static_cast<unsigned short>(baseIndex + ring * SEGMENT_COUNT + segment) };
				const auto lowerNext{ static_cast<unsigned short>(baseIndex + ring * SEGMENT_COUNT + next) };
				const auto upper{ static_cast<unsigned short>(lower + SEGMENT_COUNT) };
				const auto upperNext{ static_cast<unsigned short>(lowerNext + SEGMENT_COUNT) };

				indices.push_back(lower);
				indices.push_back(upper);
				indices.push_back(lowerNext);

				indices.push_back(lowerNext);
				indices.push_back(upper);
				indices.push_back(upperNext);
			}
		}
	}

	void WaterSurface::buildCaustics(std::vector<core::utility::Vertex3D>& vertices,
	                                 std::vector<unsigned short>& indices) const
	{
		if (m_levelRatio <= 0.0f)
			return;

		const float floorY{ cup::FLOOR_TOP + CAUSTIC_LIFT };
		const float outerRadius{ cup::FLOOR_RADIUS };
		const auto baseIndex{ static_cast<unsigned short>(vertices.size()) };

		for (int ring{ 0 }; ring <= RING_COUNT; ++ring)
		{
			const float radius{ outerRadius * ring / RING_COUNT };

			for (int segment{ 0 }; segment < SEGMENT_COUNT; ++segment)
			{
				const float angle{ core::utility::math::TWO_PI * segment / SEGMENT_COUNT };
				const float x{ std::cos(angle) * radius };
				const float z{ std::sin(angle) * radius };

				// 水面のへこみ具合が、光をどれだけ集めるかを決める。
				// 谷はレンズのように光を絞るので、底が明るくなる
				constexpr float STEP{ 0.01f };
				const float curvature{ displacementAt(x + STEP, z) + displacementAt(x - STEP, z) +
					                   displacementAt(x, z + STEP) + displacementAt(x, z - STEP) -
					                   4.0f * displacementAt(x, z) };
				const float intensity{ std::clamp(curvature * CAUSTIC_GAIN, 0.0f, 1.0f) };

				Vertex3D vertex{};
				vertex.position = Vector3{ x, floorY, z };
				vertex.normal = Vector3{ 0.0f, 1.0f, 0.0f };
				vertex.color = core::utility::scaled(CAUSTIC_COLOR, intensity);
				vertices.push_back(vertex);
			}
		}

		for (int ring{ 0 }; ring < RING_COUNT; ++ring)
		{
			for (int segment{ 0 }; segment < SEGMENT_COUNT; ++segment)
			{
				const int next{ (segment + 1) % SEGMENT_COUNT };
				const auto inner{ static_cast<unsigned short>(baseIndex + ring * SEGMENT_COUNT + segment) };
				const auto innerNext{ static_cast<unsigned short>(baseIndex + ring * SEGMENT_COUNT + next) };
				const auto outer{ static_cast<unsigned short>(inner + SEGMENT_COUNT) };
				const auto outerNext{ static_cast<unsigned short>(innerNext + SEGMENT_COUNT) };

				indices.push_back(inner);
				indices.push_back(outer);
				indices.push_back(innerNext);

				indices.push_back(innerNext);
				indices.push_back(outer);
				indices.push_back(outerNext);
			}
		}
	}
} // namespace game::view
