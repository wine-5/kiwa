#include "game/view/LiquidVisual.h"
#include "core/interface/IRenderer3D.h"
#include "game/constant/Palette.h"
#include "game/view/MasuGeometry.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::BlendMode;
	using core::utility::Vector3;
	namespace palette = game::constant::palette;

	// ---- 液面の揺れ ----

	/// @brief 揺れの速さ
	constexpr float WOBBLE_SPEED{ 9.0f };

	/// @brief 注いでいる間ずっと保たれる揺れの大きさ
	constexpr float WOBBLE_WHILE_POURING{ 0.008f };

	/// @brief 揺れが収まる速さ（大きいほど早く静まる）
	constexpr float WOBBLE_DECAY{ 2.4f };

	/// @brief 水面のひとかわの厚み
	constexpr float SURFACE_FILM{ 0.016f };

	// ---- 照り ----

	/// @brief 照りの帯の幅（奥行き方向）
	constexpr float SHINE_WIDTH{ 0.07f };

	/// @brief 照りの濃さ
	constexpr float SHINE_STRENGTH{ 0.55f };

	// ---- 波紋 ----

	/// @brief 波紋を起こす間隔（秒）
	constexpr float RIPPLE_INTERVAL{ 0.16f };

	/// @brief 波紋が広がる速さ
	constexpr float RIPPLE_SPEED{ 0.85f };

	/// @brief 波紋が消えるまでの時間（秒）
	constexpr float RIPPLE_LIFE{ 0.9f };

	/// @brief 波紋の線の太さ
	constexpr float RIPPLE_THICKNESS{ 0.009f };

	/// @brief 波紋の円を何本の線で近似するか（少ないと円が角張って見える）
	constexpr int RIPPLE_SEGMENTS{ 28 };

	// ---- しぶき ----

	/// @brief 一度の着水で飛ぶしずくの数
	constexpr int SPLASH_COUNT{ 4 };

	/// @brief しずくにかかる重力
	constexpr float DROPLET_GRAVITY{ 6.5f };

	// ---- 注がれる筋 ----

	/// @brief 注ぎ口の高さ
	constexpr float STREAM_TOP{ 2.2f };

	/// @brief 筋を何段に分けて描くか（段ごとに太さと横ぶれを変える）
	constexpr int STREAM_SEGMENTS{ 5 };

	/// @brief 注ぎ口のところの太さ
	constexpr float STREAM_RADIUS_TOP{ 0.055f };

	/// @brief 落ちきったところの太さ（落ちるほど速くなるので細くなる）
	constexpr float STREAM_RADIUS_BOTTOM{ 0.028f };

	/// @brief 筋の横ぶれの大きさ
	constexpr float STREAM_SWAY{ 0.016f };

	/// @brief 段どうしを重ねる量（継ぎ目が粒に見えないように少し食い込ませる）
	constexpr float STREAM_OVERLAP{ 0.12f };

	/// @brief 円周
	constexpr float TWO_PI{ 6.2831853f };

	/**
	 * @brief 二つの値の間を補間する
	 * @param from 始まりの値
	 * @param to 終わりの値
	 * @param t 位置（0.0〜1.0）
	 * @return 補間した値
	 */
	constexpr float lerp(float from, float to, float t) noexcept
	{
		return from + (to - from) * t;
	}
} // namespace

namespace game::view
{
	void LiquidVisual::advance(float deltaTime, bool isPouring, float amountRatio)
	{
		m_time += deltaTime;
		m_amountRatio = amountRatio;
		m_isPouring = isPouring;

		// 注いでいる間は揺れ続け、やめると収まっていく
		m_wobble *= std::exp(-WOBBLE_DECAY * deltaTime);
		if (isPouring)
			m_wobble = std::max(m_wobble, WOBBLE_WHILE_POURING);

		if (isPouring)
		{
			m_rippleTimer -= deltaTime;
			if (m_rippleTimer <= 0.0f)
			{
				spawnRipple();
				spawnSplash();
				m_rippleTimer = RIPPLE_INTERVAL;
			}
		}

		for (Ripple& ripple : m_ripples)
		{
			ripple.radius += RIPPLE_SPEED * deltaTime;
			ripple.life -= deltaTime / RIPPLE_LIFE;
		}
		// 消えたものと、枡の内側からはみ出したものを取り除く
		std::erase_if(m_ripples, [](const Ripple& ripple) {
			return ripple.life <= 0.0f || ripple.radius >= masu::INNER_HALF;
		});

		const float surface{ getSurfaceHeight() };
		for (Droplet& droplet : m_droplets)
		{
			droplet.velocity.y -= DROPLET_GRAVITY * deltaTime;
			droplet.position += droplet.velocity * deltaTime;
			droplet.life -= deltaTime;
		}
		std::erase_if(m_droplets, [surface](const Droplet& droplet) {
			return droplet.life <= 0.0f || droplet.position.y <= surface;
		});
	}

	float LiquidVisual::getSurfaceHeight() const noexcept
	{
		return masu::surfaceHeight(m_amountRatio) + std::sin(m_time * WOBBLE_SPEED) * m_wobble;
	}

	void LiquidVisual::draw(core::iface::IRenderer3D& renderer) const
	{
		drawBody(renderer);
		drawSurface(renderer);
		drawStream(renderer);
		drawDroplets(renderer);
		drawRipples(renderer);
		renderer.setBlend(BlendMode::None, 1.0f);
	}

	void LiquidVisual::drawBody(core::iface::IRenderer3D& renderer) const
	{
		if (m_amountRatio <= 0.0f)
			return;

		renderer.drawBox(Vector3{ -masu::INNER_HALF, masu::FLOOR_TOP, -masu::INNER_HALF },
		                 Vector3{ masu::INNER_HALF, getSurfaceHeight(), masu::INNER_HALF },
		                 palette::LIQUID);
	}

	void LiquidVisual::drawSurface(core::iface::IRenderer3D& renderer) const
	{
		if (m_amountRatio <= 0.0f)
			return;

		const float surface{ getSurfaceHeight() };

		// 面をひとかわ明るくして、液体の「上面」をはっきり見せる
		renderer.drawBox(Vector3{ -masu::INNER_HALF, surface - SURFACE_FILM, -masu::INNER_HALF },
		                 Vector3{ masu::INNER_HALF, surface, masu::INNER_HALF }, palette::LIQUID_SURFACE);

		// 照りは光を足すように重ねる。ゆっくり位置をずらして水面が動いて見えるようにする
		renderer.setBlend(BlendMode::Add, SHINE_STRENGTH);

		const float shineZ{ std::sin(m_time * 0.7f) * masu::INNER_HALF * 0.35f };
		renderer.drawBox(Vector3{ -masu::INNER_HALF * 0.75f, surface, shineZ - SHINE_WIDTH },
		                 Vector3{ masu::INNER_HALF * 0.15f, surface + 0.002f, shineZ + SHINE_WIDTH },
		                 palette::LIQUID_SHINE);

		const float subZ{ std::sin(m_time * 0.5f + 1.7f) * masu::INNER_HALF * 0.45f };
		renderer.drawBox(Vector3{ masu::INNER_HALF * 0.25f, surface, subZ - SHINE_WIDTH * 0.5f },
		                 Vector3{ masu::INNER_HALF * 0.7f, surface + 0.002f, subZ + SHINE_WIDTH * 0.5f },
		                 palette::LIQUID_SHINE);

		renderer.setBlend(BlendMode::None, 1.0f);
	}

	void LiquidVisual::drawRipples(core::iface::IRenderer3D& renderer) const
	{
		if (m_ripples.empty())
			return;

		const float surface{ getSurfaceHeight() };

		for (const Ripple& ripple : m_ripples)
		{
			renderer.setBlend(BlendMode::Alpha, ripple.life * 0.8f);

			// 円は細い線をつないで近似する
			for (int i{ 0 }; i < RIPPLE_SEGMENTS; ++i)
			{
				const float angle{ TWO_PI * i / RIPPLE_SEGMENTS };
				const float nextAngle{ TWO_PI * (i + 1) / RIPPLE_SEGMENTS };
				const Vector3 from{ std::cos(angle) * ripple.radius, surface + 0.006f,
					                std::sin(angle) * ripple.radius };
				const Vector3 to{ std::cos(nextAngle) * ripple.radius, surface + 0.006f,
					              std::sin(nextAngle) * ripple.radius };
				renderer.drawCapsule(from, to, RIPPLE_THICKNESS, palette::RIPPLE);
			}
		}
	}

	void LiquidVisual::drawStream(core::iface::IRenderer3D& renderer) const
	{
		if (!m_isPouring)
			return;

		const float surface{ getSurfaceHeight() };

		// 一本の棒ではなく段に分け、下へ行くほど細く、上ほど揺らして「流れ」に見せる。
		// 透かして重ねると継ぎ目の丸みが粒に見えてしまうため、ここは不透明で描く
		for (int i{ 0 }; i < STREAM_SEGMENTS; ++i)
		{
			const float t0{ static_cast<float>(i) / STREAM_SEGMENTS };
			const float t1{ std::min(1.0f, (i + 1 + STREAM_OVERLAP) / STREAM_SEGMENTS) };

			const float y0{ lerp(STREAM_TOP, surface, t0) };
			const float y1{ lerp(STREAM_TOP, surface, t1) };

			const float sway0{ std::sin(m_time * 7.0f + t0 * 5.0f) * STREAM_SWAY * (1.0f - t0) };
			const float sway1{ std::sin(m_time * 7.0f + t1 * 5.0f) * STREAM_SWAY * (1.0f - t1) };

			const float radius{ lerp(STREAM_RADIUS_TOP, STREAM_RADIUS_BOTTOM, t0) };

			renderer.drawCapsule(Vector3{ sway0, y0, sway0 * 0.6f }, Vector3{ sway1, y1, sway1 * 0.6f },
			                     radius, palette::STREAM);
		}
	}

	void LiquidVisual::drawDroplets(core::iface::IRenderer3D& renderer) const
	{
		for (const Droplet& droplet : m_droplets)
			renderer.drawSphere(droplet.position, droplet.radius, palette::DROPLET);
	}

	void LiquidVisual::spawnRipple()
	{
		m_ripples.push_back(Ripple{ 0.04f, 1.0f });
	}

	void LiquidVisual::spawnSplash()
	{
		const float surface{ getSurfaceHeight() };

		for (int i{ 0 }; i < SPLASH_COUNT; ++i)
		{
			Droplet droplet{};
			droplet.position = Vector3{ randomRange(-0.05f, 0.05f), surface + 0.03f,
				                        randomRange(-0.05f, 0.05f) };
			droplet.velocity = Vector3{ randomRange(-0.7f, 0.7f), randomRange(1.3f, 2.2f),
				                        randomRange(-0.7f, 0.7f) };
			droplet.radius = randomRange(0.018f, 0.032f);
			droplet.life = randomRange(0.35f, 0.6f);
			m_droplets.push_back(droplet);
		}
	}

	float LiquidVisual::randomRange(float min, float max)
	{
		std::uniform_real_distribution<float> distribution{ min, max };
		return distribution(m_random);
	}
} // namespace game::view
