#include "game/view/LiquidVisual.h"
#include "core/interface/IRenderer3D.h"
#include "game/constant/Palette.h"
#include "game/view/MasuGeometry.h"
#include <algorithm>

namespace
{
	using core::utility::Vector3;
	namespace palette = game::constant::palette;

	/// @brief 波としぶきを起こす間隔（秒）
	constexpr float SPLASH_INTERVAL{ 0.085f };

	/// @brief 一度に飛ぶしずくの数
	constexpr int SPLASH_COUNT{ 3 };

	/// @brief しずくにかかる重力
	constexpr float DROPLET_GRAVITY{ 6.5f };

	/// @brief 注ぎ終わりに起こす波の強さ（最後のひと落ちぶん）
	constexpr float FINAL_WAVE_STRENGTH{ 1.4f };

	/// @brief 底の光の模様の濃さ
	constexpr float CAUSTIC_BLEND{ 0.55f };
} // namespace

namespace game::view
{
	void LiquidVisual::advance(float deltaTime, bool isPouring, float amountRatio)
	{
		m_surface.setLevel(amountRatio);
		m_stream.advance(deltaTime, isPouring);

		// 筋が当たる場所。液面は波打っているので、まず平らだとしたときの高さで当てる
		const float levelHeight{ m_surface.getLevelHeight() };
		const float impactX{ m_stream.getImpactX(levelHeight) };
		const float impactZ{ m_stream.getImpactZ(levelHeight) };
		const bool isHitting{ m_stream.isVisible() && amountRatio > 0.0f };

		m_surface.setPourPoint(impactX, impactZ, isHitting);

		if (isHitting)
		{
			m_splashTimer -= deltaTime;
			if (m_splashTimer <= 0.0f)
			{
				m_surface.disturb(impactX, impactZ, m_stream.getFlow());
				spawnSplash(impactX, impactZ, m_surface.heightAt(impactX, impactZ));
				m_splashTimer = SPLASH_INTERVAL;
			}
		}

		// 注ぎ終えた瞬間は、最後のひと落ちが一番大きな波を作る
		if (m_wasPouring && !isPouring)
			m_surface.disturb(impactX, impactZ, FINAL_WAVE_STRENGTH);
		m_wasPouring = isPouring;

		m_surface.advance(deltaTime);

		const float surfaceHeight{ m_surface.heightAt(impactX, impactZ) };
		for (Droplet& droplet : m_droplets)
		{
			droplet.velocity.y -= DROPLET_GRAVITY * deltaTime;
			droplet.position += droplet.velocity * deltaTime;
			droplet.life -= deltaTime;
		}
		std::erase_if(m_droplets, [surfaceHeight](const Droplet& droplet) {
			return droplet.life <= 0.0f || droplet.position.y <= surfaceHeight;
		});
	}

	float LiquidVisual::getSurfaceHeight() const
	{
		return m_surface.heightAt(0.0f, 0.0f);
	}

	void LiquidVisual::draw(core::iface::IRenderer3D& renderer,
	                        const core::utility::Vector3& cameraPosition) const
	{
		m_vertices.clear();
		m_indices.clear();
		m_causticVertices.clear();
		m_causticIndices.clear();

		m_surface.buildCaustics(m_causticVertices, m_causticIndices);
		m_surface.build(m_vertices, m_indices, cameraPosition);

		const float levelHeight{ m_surface.getLevelHeight() };
		const float impactX{ m_stream.getImpactX(levelHeight) };
		const float impactZ{ m_stream.getImpactZ(levelHeight) };
		m_stream.build(m_vertices, m_indices, m_surface.heightAt(impactX, impactZ), cameraPosition);

		// 自前で組んだメッシュは裏表を取り違えやすいので、面の省略は切っておく
		renderer.setBackCulling(false);

		// 底の光の模様がいちばん奥。水はこの上に重なる
		if (!m_causticIndices.empty())
		{
			renderer.setBlend(core::utility::BlendMode::Add, CAUSTIC_BLEND);
			renderer.drawTriangles(m_causticVertices, m_causticIndices);
		}

		// 上面と筋は透かして重ねる。真上から覗くと底がうっすら見える
		if (!m_indices.empty())
		{
			renderer.setBlend(core::utility::BlendMode::Alpha, 1.0f);
			renderer.drawTriangles(m_vertices, m_indices);
		}

		renderer.setBlend(core::utility::BlendMode::None, 1.0f);
		renderer.setBackCulling(true);

		drawDroplets(renderer);
	}

	void LiquidVisual::drawDroplets(core::iface::IRenderer3D& renderer) const
	{
		for (const Droplet& droplet : m_droplets)
			renderer.drawSphere(droplet.position, droplet.radius, palette::DROPLET);
	}

	void LiquidVisual::spawnSplash(float x, float z, float surfaceHeight)
	{
		for (int i{ 0 }; i < SPLASH_COUNT; ++i)
		{
			Droplet droplet{};
			droplet.position = Vector3{ x + randomRange(-0.04f, 0.04f), surfaceHeight + 0.02f,
				                        z + randomRange(-0.04f, 0.04f) };
			droplet.velocity = Vector3{ randomRange(-0.75f, 0.75f), randomRange(1.2f, 2.1f),
				                        randomRange(-0.75f, 0.75f) };
			droplet.radius = randomRange(0.012f, 0.026f);
			droplet.life = randomRange(0.3f, 0.55f);
			m_droplets.push_back(droplet);
		}
	}

	float LiquidVisual::randomRange(float min, float max)
	{
		std::uniform_real_distribution<float> distribution{ min, max };
		return distribution(m_random);
	}
} // namespace game::view
