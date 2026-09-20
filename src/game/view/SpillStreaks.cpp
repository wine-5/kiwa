#include "game/view/SpillStreaks.h"
#include "core/interface/IRenderer3D.h"
#include "core/utility/Easing.h"
#include "core/utility/MathConstants.h"
#include "game/constant/Palette.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Easing;
	using core::utility::Vector3;
	namespace palette = game::constant::palette;

	/// @brief 縁から垂れる筋の本数
	constexpr int STREAK_COUNT{ 7 };

	/// @brief 筋を縦に刻む数（多いほど器の曲面に沿う）
	constexpr int STREAK_STEPS{ 14 };

	/// @brief 壁からわずかに浮かせる量（器と同じ面に置くとちらつくため）
	constexpr float SURFACE_OFFSET{ 0.002f };

	/// @brief 縁を乗り越える高さ（口の上に少し盛り上がって見せる）
	constexpr float LIP_RISE{ 0.008f };

	/// @brief 筋の太さの下限と上限
	constexpr float WIDTH_MIN{ 0.012f };
	constexpr float WIDTH_MAX{ 0.026f };

	/// @brief 下りる速さの下限と上限（1秒あたりの進み具合）
	constexpr float SPEED_MIN{ 0.55f };
	constexpr float SPEED_MAX{ 1.05f };

	/// @brief 垂れ始めるまでの待ちの上限（秒）
	constexpr float DELAY_MAX{ 0.45f };

	/// @brief 先端の玉が太る割合
	constexpr float BEAD_SWELL{ 1.7f };

	/// @brief 下りながら横へ揺れる幅（ラジアン）
	constexpr float WANDER{ 0.05f };
} // namespace

namespace game::view
{
	void SpillStreaks::update(float deltaTime, bool isOverflowed)
	{
		if (!isOverflowed)
		{
			m_streaks.clear();
			m_wasOverflowed = false;
			return;
		}

		// こぼれた最初の1回だけ筋を引く（毎フレーム引き直すと垂れ方が定まらない）
		if (!m_wasOverflowed)
		{
			spawn();
			m_wasOverflowed = true;
		}

		for (Streak& streak : m_streaks)
		{
			if (streak.delay > 0.0f)
			{
				streak.delay -= deltaTime;
				continue;
			}

			streak.progress = std::min(1.0f, streak.progress + streak.speed * deltaTime);
		}
	}

	void SpillStreaks::spawn()
	{
		m_streaks.clear();

		std::uniform_real_distribution<float> spread{ -0.35f, 0.35f };
		std::uniform_real_distribution<float> delay{ 0.0f, DELAY_MAX };
		std::uniform_real_distribution<float> speed{ SPEED_MIN, SPEED_MAX };
		std::uniform_real_distribution<float> width{ WIDTH_MIN, WIDTH_MAX };
		std::uniform_real_distribution<float> phase{ 0.0f, core::utility::math::TWO_PI };

		for (int i{ 0 }; i < STREAK_COUNT; ++i)
		{
			// 一周に散らしてから少しずらす。等間隔のままだと並びが見えてしまう
			const float base{ core::utility::math::TWO_PI * i / STREAK_COUNT };

			Streak streak{};
			streak.angle = base + spread(m_random);
			streak.delay = delay(m_random);
			streak.speed = speed(m_random);
			streak.width = width(m_random);
			streak.wander = phase(m_random);
			m_streaks.push_back(streak);
		}
	}

	float SpillStreaks::getReach() const noexcept
	{
		if (m_streaks.empty())
			return 0.0f;

		float total{ 0.0f };
		for (const Streak& streak : m_streaks)
			total += streak.progress;

		return total / static_cast<float>(m_streaks.size());
	}

	void SpillStreaks::buildStreak(const Streak& streak) const
	{
		const float rim{ m_shape.rimHeight };

		// 落ちるほど速くなる。等速で下りると水には見えない
		const float headDrop{ Easing::easeIn(streak.progress) };

		for (int step{ 0 }; step <= STREAK_STEPS; ++step)
		{
			const float t{ static_cast<float>(step) / STREAK_STEPS };

			// 口の上から始めて、先端まで下りる
			const float height{ rim + LIP_RISE - (rim + LIP_RISE) * headDrop * t };
			const float radius{ m_shape.outerRadiusAt(height) + SURFACE_OFFSET };

			// まっすぐ下りると印刷したように見える。器の凹凸を伝うように少し蛇行させる
			const float center{ streak.angle + std::sin(t * 5.0f + streak.wander) * WANDER };

			// 先端は玉になって膨らみ、後ろへ行くほど細く尾を引く
			const float bead{ 1.0f + (BEAD_SWELL - 1.0f) * Easing::easeIn(t) };
			const float taper{ 0.55f + 0.45f * (1.0f - t) };
			const float halfAngle{ streak.width * bead * taper * 0.5f / std::max(radius, 0.05f) };

			// 濡れた面は光を返す。先端の玉はとくに明るく見える
			const core::utility::Color color{ core::utility::mixed(
				palette::LIQUID_SPILLED, palette::LIQUID_SURFACE, 0.25f + 0.45f * t) };
			const float alpha{ 0.94f * (0.72f + 0.28f * t) };

			const auto base{ static_cast<unsigned short>(m_vertices.size()) };

			for (int side{ 0 }; side < 2; ++side)
			{
				const float angle{ center + (side == 0 ? -halfAngle : halfAngle) };

				core::utility::Vertex3D vertex{};
				vertex.position = Vector3{ std::cos(angle) * radius, height, std::sin(angle) * radius };
				vertex.normal = Vector3{ std::cos(angle), 0.0f, std::sin(angle) };
				vertex.color = color;
				// 縁は少し薄く落として、筋の丸みを出す
				vertex.alpha = alpha * 0.86f;
				m_vertices.push_back(vertex);
			}

			// 真ん中に山を一つ置いて、平たい帯ではなく盛り上がった筋に見せる
			core::utility::Vertex3D crest{};
			crest.position = Vector3{ std::cos(center) * (radius + SURFACE_OFFSET), height,
				                      std::sin(center) * (radius + SURFACE_OFFSET) };
			crest.normal = Vector3{ std::cos(center), 0.0f, std::sin(center) };
			crest.color = color;
			crest.alpha = alpha;
			m_vertices.push_back(crest);

			if (step == 0)
				continue;

			// 前の段と結んで帯にする。左‐山・山‐右の2枚を並べると、
			// 真ん中が膨らんだ筋になる（頂点は 左・右・山 の順に積んである）
			const auto previous{ static_cast<unsigned short>(base - 3) };
			constexpr int LEFT{ 0 };
			constexpr int RIGHT{ 1 };
			constexpr int CREST{ 2 };
			const int columns[2][2]{ { LEFT, CREST }, { CREST, RIGHT } };

			for (const auto& column : columns)
			{
				const auto a{ static_cast<unsigned short>(previous + column[0]) };
				const auto b{ static_cast<unsigned short>(previous + column[1]) };
				const auto c{ static_cast<unsigned short>(base + column[0]) };
				const auto d{ static_cast<unsigned short>(base + column[1]) };

				m_indices.push_back(a);
				m_indices.push_back(c);
				m_indices.push_back(b);

				m_indices.push_back(b);
				m_indices.push_back(c);
				m_indices.push_back(d);
			}
		}
	}

	void SpillStreaks::draw(core::iface::IRenderer3D& renderer) const
	{
		m_vertices.clear();
		m_indices.clear();

		for (const Streak& streak : m_streaks)
		{
			if (streak.delay > 0.0f || streak.progress <= 0.0f)
				continue;

			buildStreak(streak);
		}

		if (m_indices.empty())
			return;

		// 器の面に張り付くので、裏表の省略は切って両面から見えるようにする
		renderer.setBackCulling(false);
		renderer.setBlend(core::utility::BlendMode::Alpha, 1.0f);
		renderer.drawTriangles(m_vertices, m_indices);
		renderer.setBlend(core::utility::BlendMode::None, 1.0f);
		renderer.setBackCulling(true);
	}
} // namespace game::view
