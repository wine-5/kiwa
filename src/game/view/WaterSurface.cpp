#include "game/view/WaterSurface.h"
#include "game/constant/Palette.h"
#include "game/view/MasuGeometry.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Color;
	using core::utility::Vector3;
	using core::utility::Vertex3D;
	namespace palette = game::constant::palette;
	namespace masu = game::view::masu;

	// ---- 面の細かさ ----

	/// @brief 面を何分割して描くか（細かいほど波と照りが滑らかになる）
	constexpr int GRID_DIVISIONS{ 40 };

	/// @brief 格子の一辺の点の数
	constexpr int GRID_POINTS{ GRID_DIVISIONS + 1 };

	/// @brief 格子が覆う幅
	constexpr float SPAN{ masu::INNER_HALF * 2.0f };

	/// @brief 格子の目の細かさ
	constexpr float CELL_SIZE{ SPAN / GRID_DIVISIONS };

	// ---- 着水の波 ----

	/// @brief 波が広がる速さ
	constexpr float WAVE_SPEED{ 1.15f };

	/// @brief 波の高さ
	constexpr float WAVE_AMPLITUDE{ 0.042f };

	/// @brief 波が収まる速さ
	constexpr float WAVE_DECAY{ 1.1f };

	/// @brief 波の峰の幅（小さいほど細い輪になる）
	constexpr float WAVE_WIDTH{ 0.30f };

	/// @brief 波の細かさ
	constexpr float WAVE_FREQUENCY{ 18.0f };

	/// @brief 波を保っておく時間（秒）
	constexpr float WAVE_LIFE{ 2.6f };

	/// @brief 同時に持つ波の上限
	constexpr std::size_t WAVE_LIMIT{ 14 };

	// ---- 常にある細かなうねり ----

	/// @brief うねりの高さ
	constexpr float SWELL_AMPLITUDE{ 0.0045f };

	// ---- 壁際の盛り上がり ----

	/// @brief 壁に沿って持ち上がる高さ（表面張力で縁が上がる）
	constexpr float MENISCUS_HEIGHT{ 0.009f };

	/// @brief 盛り上がりが効く範囲
	constexpr float MENISCUS_WIDTH{ 0.03f };

	// ---- 注がれている場所 ----

	/// @brief くぼみの深さ
	constexpr float POUR_DIP{ 0.035f };

	/// @brief くぼみの広さ
	constexpr float POUR_DIP_RADIUS{ 0.17f };

	/// @brief 泡立つ範囲の広さ
	constexpr float FOAM_RADIUS{ 0.15f };

	// ---- 陰影 ----

	/// @brief 光が来る向き（正規化済み）
	constexpr Vector3 LIGHT_DIRECTION{ 0.36f, 0.88f, -0.31f };

	/// @brief 照りの鋭さ（大きいほど小さく強く光る）
	constexpr float SPECULAR_POWER{ 48.0f };

	/// @brief 照りの強さ
	constexpr float SPECULAR_STRENGTH{ 1.5f };

	/// @brief 映り込みの効き方（浅い角度ほど強く映る）
	constexpr float REFLECT_MIN{ 0.10f };
	constexpr float REFLECT_MAX{ 0.72f };

	/// @brief 上を向いた面に映るもの（行灯のような暖かい明かり）
	constexpr Color REFLECT_SKY{ 212, 186, 138 };

	/// @brief 横～下を向いた面に映るもの（枡の内側や台）
	constexpr Color REFLECT_ROOM{ 92, 72, 52 };

	/// @brief 泡の色
	constexpr Color FOAM_COLOR{ 250, 244, 228 };

	/// @brief 底のほうの暗さ（側面の下端に掛ける倍率）
	constexpr float DEPTH_DARKEN{ 0.58f };

	// ---- 透け方 ----

	/// @brief 水面の透け具合（真上から覗いたときは底がうっすら見える）
	constexpr float SURFACE_ALPHA_MIN{ 0.74f };

	/// @brief 浅い角度から見たときの水面の濃さ（映り込みで底は見えなくなる）
	constexpr float SURFACE_ALPHA_MAX{ 0.97f };

	// ---- 水中の光の模様 ----

	/// @brief 模様の強さ
	constexpr float CAUSTIC_GAIN{ 22.0f };

	/// @brief 模様の色
	constexpr Color CAUSTIC_COLOR{ 255, 243, 205 };

	/// @brief 底から少し浮かせる高さ（同じ面に描いてちらつくのを防ぐ）
	constexpr float CAUSTIC_LIFT{ 0.004f };

	/**
	 * @brief 内積を返す
	 * @param a 一方のベクトル
	 * @param b もう一方のベクトル
	 * @return 内積
	 */
	float dot(const Vector3& a, const Vector3& b) noexcept
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	/**
	 * @brief 0〜255 に収めて整数にする
	 * @param value 元の値
	 * @return 収めた値
	 */
	int toChannel(float value) noexcept
	{
		return static_cast<int>(std::clamp(value, 0.0f, 255.0f));
	}

	/**
	 * @brief 二つの色を混ぜる
	 * @param from 混ぜる前の色
	 * @param to 混ぜ込む色
	 * @param t 混ぜる割合（0.0〜1.0）
	 * @return 混ぜた色
	 */
	Color mix(const Color& from, const Color& to, float t) noexcept
	{
		return Color{ toChannel(from.r + (to.r - from.r) * t), toChannel(from.g + (to.g - from.g) * t),
			          toChannel(from.b + (to.b - from.b) * t) };
	}

	/**
	 * @brief 格子の点の座標を返す
	 * @param index 格子の番号（0〜GRID_DIVISIONS）
	 * @return 座標
	 */
	float gridPosition(int index) noexcept
	{
		return -masu::INNER_HALF + CELL_SIZE * index;
	}

	/**
	 * @brief 面の向きと視点から、水面の色を決める
	 * @param position 座標
	 * @param normal 法線
	 * @param cameraPosition 視点の座標
	 * @param foam 泡立ちの強さ（0.0〜1.0）
	 * @param outAlpha 透明度の出力先
	 * @return 頂点の色
	 */
	Color shadeSurface(const Vector3& position, const Vector3& normal, const Vector3& cameraPosition,
	                   float foam, float& outAlpha)
	{
		const Vector3 view{ (cameraPosition - position).normalized() };
		const Vector3 halfway{ (LIGHT_DIRECTION + view).normalized() };
		const float facing{ std::max(0.0f, dot(normal, view)) };

		// 水が水に見えるのは、波の傾きによって「映り込む先」が変わるから。
		// 視線を水面で跳ね返し、その向きが上を向いていれば明るい天井、
		// 横や下を向いていれば枡の内側が映っていると見なす
		const Vector3 reflected{ (normal * (2.0f * dot(normal, view)) - view).normalized() };
		const float skyAmount{ std::clamp(0.5f + 0.5f * reflected.y, 0.0f, 1.0f) };
		const Color reflection{ mix(REFLECT_ROOM, REFLECT_SKY, skyAmount * skyAmount) };

		// 浅い角度で見るほど映り込みが強くなる（フレネル）
		const float grazing{ std::pow(1.0f - facing, 3.0f) };
		const float reflectRate{ REFLECT_MIN + (REFLECT_MAX - REFLECT_MIN) * grazing };

		const float diffuse{ 0.52f + 0.48f * std::max(0.0f, dot(normal, LIGHT_DIRECTION)) };
		const Color base{ palette::LIQUID_SURFACE };
		Color result{ mix(Color{ toChannel(base.r * diffuse), toChannel(base.g * diffuse),
			                     toChannel(base.b * diffuse) },
			              reflection, reflectRate) };

		// 光源そのものの映り込み（きらめき）
		const float specular{ std::pow(std::max(0.0f, dot(normal, halfway)), SPECULAR_POWER) *
			                  SPECULAR_STRENGTH };
		const Color shineColor{ palette::LIQUID_SHINE };
		result = Color{ toChannel(result.r + shineColor.r * specular),
			            toChannel(result.g + shineColor.g * specular),
			            toChannel(result.b + shineColor.b * specular) };

		// 真上から覗けば底が透けて見え、浅い角度では映り込みで濁る
		outAlpha = SURFACE_ALPHA_MIN + (SURFACE_ALPHA_MAX - SURFACE_ALPHA_MIN) * grazing;

		if (foam > 0.0f)
		{
			const float amount{ std::min(1.0f, foam * 0.45f) };
			result = mix(result, FOAM_COLOR, amount);
			outAlpha = std::min(1.0f, outAlpha + amount * 0.5f); // 泡立っているところは透けない
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

		refreshHeights();
	}

	void WaterSurface::refreshHeights()
	{
		// 高さは一度だけ求めて格子に置く。法線も水中の光の模様もここから作るので、
		// 同じ点を何度も計算し直さずに済む（波の数だけ計算が増えるため効く）
		m_heights.resize(static_cast<std::size_t>(GRID_POINTS) * GRID_POINTS);

		for (int iz{ 0 }; iz < GRID_POINTS; ++iz)
		{
			for (int ix{ 0 }; ix < GRID_POINTS; ++ix)
				m_heights[static_cast<std::size_t>(iz) * GRID_POINTS + ix] =
				    heightAt(gridPosition(ix), gridPosition(iz));
		}
	}

	float WaterSurface::getLevelHeight() const noexcept
	{
		return masu::surfaceHeight(m_levelRatio);
	}

	float WaterSurface::heightAt(float x, float z) const
	{
		return getLevelHeight() + displacementAt(x, z);
	}

	float WaterSurface::displacementAt(float x, float z) const
	{
		// 常に少しだけうねらせておく。完全に止まった面は水に見えない
		float height{ SWELL_AMPLITUDE * std::sin(x * 7.3f + m_time * 2.1f) *
			              std::cos(z * 6.1f - m_time * 1.7f) +
			          SWELL_AMPLITUDE * 0.7f * std::sin((x + z) * 9.7f - m_time * 2.9f) };

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

		// 壁に接したところは表面張力で持ち上がる。この縁の丸まりがあると、
		// 液体が「板」ではなく「中身」に見える
		const float edgeDistance{ std::min(masu::INNER_HALF - std::abs(x),
			                               masu::INNER_HALF - std::abs(z)) };
		height += MENISCUS_HEIGHT * std::exp(-std::max(0.0f, edgeDistance) / MENISCUS_WIDTH);

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

	float WaterSurface::foamAt(float x, float z) const
	{
		float foam{ 0.0f };

		// 着水したところ
		if (m_isPouring)
		{
			const float dx{ x - m_pourX };
			const float dz{ z - m_pourZ };
			const float distance{ std::sqrt(dx * dx + dz * dz) / FOAM_RADIUS };
			foam += std::exp(-distance * distance);
		}

		// 波の峰にも泡が少し乗って一緒に広がっていく（強くすると水面が白く濁る）
		for (const Wave& wave : m_waves)
		{
			const float dx{ x - wave.originX };
			const float dz{ z - wave.originZ };
			const float distance{ std::sqrt(dx * dx + dz * dz) };
			const float offset{ (distance - WAVE_SPEED * wave.age) / (WAVE_WIDTH * 0.5f) };
			foam += 0.10f * wave.strength * std::exp(-WAVE_DECAY * 2.4f * wave.age) *
			        std::exp(-offset * offset);
		}

		return std::min(1.0f, foam);
	}

	void WaterSurface::build(std::vector<core::utility::Vertex3D>& vertices,
	                         std::vector<unsigned short>& indices,
	                         const core::utility::Vector3& cameraPosition) const
	{
		if (m_levelRatio <= 0.0f || m_heights.empty())
			return;

		const auto baseIndex{ static_cast<unsigned short>(vertices.size()) };
		const auto sample{ [this](int ix, int iz) {
			const int clampedX{ std::clamp(ix, 0, GRID_POINTS - 1) };
			const int clampedZ{ std::clamp(iz, 0, GRID_POINTS - 1) };
			return m_heights[static_cast<std::size_t>(clampedZ) * GRID_POINTS + clampedX];
		} };

		// ---- 上面 ----
		for (int iz{ 0 }; iz < GRID_POINTS; ++iz)
		{
			for (int ix{ 0 }; ix < GRID_POINTS; ++ix)
			{
				const float x{ gridPosition(ix) };
				const float z{ gridPosition(iz) };

				// 法線は隣との高さの差から出す
				const float dx{ sample(ix + 1, iz) - sample(ix - 1, iz) };
				const float dz{ sample(ix, iz + 1) - sample(ix, iz - 1) };

				Vertex3D vertex{};
				vertex.position = Vector3{ x, sample(ix, iz), z };
				vertex.normal = Vector3{ -dx, 2.0f * CELL_SIZE, -dz }.normalized();
				vertex.color = shadeSurface(vertex.position, vertex.normal, cameraPosition,
				                            foamAt(x, z), vertex.alpha);
				vertex.u = static_cast<float>(ix) / GRID_DIVISIONS;
				vertex.v = static_cast<float>(iz) / GRID_DIVISIONS;
				vertices.push_back(vertex);
			}
		}

		for (int iz{ 0 }; iz < GRID_DIVISIONS; ++iz)
		{
			for (int ix{ 0 }; ix < GRID_DIVISIONS; ++ix)
			{
				const auto topLeft{ static_cast<unsigned short>(baseIndex + iz * GRID_POINTS + ix) };
				const auto topRight{ static_cast<unsigned short>(topLeft + 1) };
				const auto bottomLeft{ static_cast<unsigned short>(topLeft + GRID_POINTS) };
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

	void WaterSurface::buildSides(std::vector<core::utility::Vertex3D>& vertices,
	                              std::vector<unsigned short>& indices) const
	{
		if (m_levelRatio <= 0.0f || m_heights.empty())
			return;

		const auto sample{ [this](int ix, int iz) {
			const int clampedX{ std::clamp(ix, 0, GRID_POINTS - 1) };
			const int clampedZ{ std::clamp(iz, 0, GRID_POINTS - 1) };
			return m_heights[static_cast<std::size_t>(clampedZ) * GRID_POINTS + clampedX];
		} };

		// 縁の高さも波に合わせる。底へ向かって暗くすることで深さを出す
		const Color deepColor{ toChannel(palette::LIQUID.r * DEPTH_DARKEN),
			                   toChannel(palette::LIQUID.g * DEPTH_DARKEN),
			                   toChannel(palette::LIQUID.b * DEPTH_DARKEN) };

		struct Edge
		{
			int fromX;
			int fromZ;
			int toX;
			int toZ;
			Vector3 normal;
		};

		const Edge edges[]{
			{ 0, 0, GRID_DIVISIONS, 0, Vector3{ 0.0f, 0.0f, -1.0f } },
			{ GRID_DIVISIONS, 0, GRID_DIVISIONS, GRID_DIVISIONS, Vector3{ 1.0f, 0.0f, 0.0f } },
			{ GRID_DIVISIONS, GRID_DIVISIONS, 0, GRID_DIVISIONS, Vector3{ 0.0f, 0.0f, 1.0f } },
			{ 0, GRID_DIVISIONS, 0, 0, Vector3{ -1.0f, 0.0f, 0.0f } },
		};

		for (const Edge& edge : edges)
		{
			const auto edgeBase{ static_cast<unsigned short>(vertices.size()) };

			for (int i{ 0 }; i <= GRID_DIVISIONS; ++i)
			{
				const float t{ static_cast<float>(i) / GRID_DIVISIONS };
				const int ix{ static_cast<int>(edge.fromX + (edge.toX - edge.fromX) * t) };
				const int iz{ static_cast<int>(edge.fromZ + (edge.toZ - edge.fromZ) * t) };

				Vertex3D top{};
				top.position = Vector3{ gridPosition(ix), sample(ix, iz), gridPosition(iz) };
				top.normal = edge.normal;
				top.color = palette::LIQUID;
				vertices.push_back(top);

				Vertex3D bottom{};
				bottom.position = Vector3{ gridPosition(ix), masu::FLOOR_TOP, gridPosition(iz) };
				bottom.normal = edge.normal;
				bottom.color = deepColor;
				vertices.push_back(bottom);
			}

			for (int i{ 0 }; i < GRID_DIVISIONS; ++i)
			{
				const auto topLeft{ static_cast<unsigned short>(edgeBase + i * 2) };
				const auto bottomLeft{ static_cast<unsigned short>(topLeft + 1) };
				const auto topRight{ static_cast<unsigned short>(topLeft + 2) };
				const auto bottomRight{ static_cast<unsigned short>(topLeft + 3) };

				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
				indices.push_back(topRight);

				indices.push_back(topRight);
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
			}
		}
	}

	void WaterSurface::buildCaustics(std::vector<core::utility::Vertex3D>& vertices,
	                                 std::vector<unsigned short>& indices) const
	{
		if (m_levelRatio <= 0.0f || m_heights.empty())
			return;

		const auto baseIndex{ static_cast<unsigned short>(vertices.size()) };
		const auto sample{ [this](int ix, int iz) {
			const int clampedX{ std::clamp(ix, 0, GRID_POINTS - 1) };
			const int clampedZ{ std::clamp(iz, 0, GRID_POINTS - 1) };
			return m_heights[static_cast<std::size_t>(clampedZ) * GRID_POINTS + clampedX];
		} };

		const float floorY{ masu::FLOOR_TOP + CAUSTIC_LIFT };

		for (int iz{ 0 }; iz < GRID_POINTS; ++iz)
		{
			for (int ix{ 0 }; ix < GRID_POINTS; ++ix)
			{
				// 水面のへこみ具合（ラプラシアン）が、光をどれだけ集めるかを決める。
				// 谷になっているところはレンズのように光を絞るので、底が明るくなる
				const float curvature{ sample(ix + 1, iz) + sample(ix - 1, iz) + sample(ix, iz + 1) +
					                   sample(ix, iz - 1) - 4.0f * sample(ix, iz) };
				const float intensity{ std::clamp(curvature * CAUSTIC_GAIN, 0.0f, 1.0f) };

				Vertex3D vertex{};
				vertex.position = Vector3{ gridPosition(ix), floorY, gridPosition(iz) };
				vertex.normal = Vector3{ 0.0f, 1.0f, 0.0f };
				vertex.color = Color{ toChannel(CAUSTIC_COLOR.r * intensity),
					                  toChannel(CAUSTIC_COLOR.g * intensity),
					                  toChannel(CAUSTIC_COLOR.b * intensity) };
				vertices.push_back(vertex);
			}
		}

		for (int iz{ 0 }; iz < GRID_DIVISIONS; ++iz)
		{
			for (int ix{ 0 }; ix < GRID_DIVISIONS; ++ix)
			{
				const auto topLeft{ static_cast<unsigned short>(baseIndex + iz * GRID_POINTS + ix) };
				const auto topRight{ static_cast<unsigned short>(topLeft + 1) };
				const auto bottomLeft{ static_cast<unsigned short>(topLeft + GRID_POINTS) };
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
