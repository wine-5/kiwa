#include "game/view/RoomBackdrop.h"
#include "core/interface/ICamera.h"
#include "core/interface/IModelRenderer.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IRenderer3D.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/utility/MathConstants.h"
#include "game/constant/Palette.h"
#include "game/view/SceneryMesh.h"
#include <cmath>

namespace
{
	using core::utility::Vector3;
	namespace palette = game::constant::palette;

	/// @brief 畳表
	constexpr const char* FLOOR_TEXTURE_PATH{ "assets/textures/tatami.png" };

	/// @brief 周辺減光
	constexpr const char* VIGNETTE_TEXTURE_PATH{ "assets/textures/vignette.png" };

	/// @brief 粒状感
	constexpr const char* GRAIN_TEXTURE_PATH{ "assets/textures/grain.png" };

	/// @brief 湯呑のモデル
	constexpr const char* CUP_MODEL_PATH{ "assets/model/yunomi.mqo" };

	/// @brief 土瓶のモデル
	constexpr const char* POT_MODEL_PATH{ "assets/model/dobin.mqo" };

	/// @brief 土瓶の大きさ
	constexpr float POT_SCALE{ 0.85f };

	/// @brief 土瓶を据える場所
	constexpr Vector3 POT_POSITION{ -0.78f, 0.0f, 0.34f };

	/// @brief 器を据える場所
	constexpr Vector3 CUP_POSITION{ 0.36f, 0.0f, -0.12f };

	/// @brief 視点の基準の位置
	constexpr Vector3 CAMERA_BASE{ 0.62f, 1.98f, -3.45f };

	/// @brief 見る先
	constexpr Vector3 CAMERA_TARGET{ 0.30f, 0.02f, 0.18f };

	/// @brief 画角
	constexpr float CAMERA_FOV{ 0.82f };

	/// @brief 近くの見切り
	constexpr float CAMERA_NEAR{ 0.1f };

	/// @brief 遠くの見切り
	constexpr float CAMERA_FAR{ 100.0f };

	/// @brief 視点が漂う幅（横・縦）
	constexpr float DRIFT_X{ 0.26f };
	constexpr float DRIFT_Y{ 0.07f };

	/// @brief 視点が漂う速さ（横・縦）
	constexpr float DRIFT_X_SPEED{ 0.13f };
	constexpr float DRIFT_Y_SPEED{ 0.09f };

	/// @brief 周辺減光の強さ
	constexpr float VIGNETTE_STRENGTH{ 0.55f };

	/// @brief 粒状感の濃さ
	constexpr float GRAIN_STRENGTH{ 0.05f };

	/// @brief 湯気の筋の本数
	constexpr int STEAM_WISPS{ 3 };

	/// @brief 湯気を縦に刻む数
	constexpr int STEAM_STEPS{ 14 };

	/// @brief 湯気が立ち上る高さ
	constexpr float STEAM_HEIGHT{ 0.44f };

	/// @brief 湯気が立ち始める高さ（器の口のあたり）
	constexpr float STEAM_BASE{ 0.58f };

	/// @brief 湯気の太さ
	constexpr float STEAM_WIDTH{ 0.05f };

	/// @brief 湯気の揺れ幅
	constexpr float STEAM_SWAY{ 0.11f };

	/// @brief 湯気の濃さ
	constexpr float STEAM_ALPHA{ 0.09f };

	/// @brief 湯気が昇る速さ
	constexpr float STEAM_SPEED{ 0.22f };
} // namespace

namespace game::view
{
	RoomBackdrop::RoomBackdrop(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
	                           core::iface::ICamera& camera,
	                           core::iface::IModelRenderer& modelRenderer,
	                           core::iface::IResourceManager& resource, core::iface::IScreen& screen)
	    : m_renderer3D{ renderer3D }, m_renderer{ renderer }, m_camera{ camera },
	      m_modelRenderer{ modelRenderer }, m_screen{ screen }
	{
		// 床も影も動かないので、形は最初に一度だけ組んで使い回す
		SceneryMesh::buildFloor(m_floorVertices, m_floorIndices);
		SceneryMesh::buildShadow(m_shadowVertices, m_shadowIndices);

		m_floorTexture = resource.loadTexture(FLOOR_TEXTURE_PATH);
		m_vignetteTexture = resource.loadTexture(VIGNETTE_TEXTURE_PATH);
		m_grainTexture = resource.loadTexture(GRAIN_TEXTURE_PATH);
		m_cupModel = resource.loadModel(CUP_MODEL_PATH);
		m_potModel = resource.loadModel(POT_MODEL_PATH);
	}

	void RoomBackdrop::update(float deltaTime)
	{
		m_time += deltaTime;
	}

	void RoomBackdrop::drawSteam() const
	{
		m_steamVertices.clear();
		m_steamIndices.clear();

		for (int wisp{ 0 }; wisp < STEAM_WISPS; ++wisp)
		{
			// 筋ごとに出どころと揺れをずらす。揃っていると湯気に見えない
			const float phase{ core::utility::math::TWO_PI * wisp / STEAM_WISPS };
			const float offsetX{ std::cos(phase) * 0.05f };
			const float offsetZ{ std::sin(phase) * 0.05f };

			for (int step{ 0 }; step <= STEAM_STEPS; ++step)
			{
				const float t{ static_cast<float>(step) / STEAM_STEPS };

				// 上へ行くほど散って薄くなる
				const float rise{ std::fmod(m_time * STEAM_SPEED + phase, 1.0f) * 0.15f };
				const float height{ STEAM_BASE + STEAM_HEIGHT * (t + rise) };
				const float sway{ std::sin(m_time * 0.9f + phase + t * 3.4f) * STEAM_SWAY * t };
				const float width{ STEAM_WIDTH * (0.4f + t * 1.6f) };
				const float alpha{ STEAM_ALPHA * std::sin(t * core::utility::math::PI) };

				const float x{ CUP_POSITION.x + offsetX + sway };
				const float z{ CUP_POSITION.z + offsetZ + sway * 0.4f };

				const auto base{ static_cast<unsigned short>(m_steamVertices.size()) };

				for (int side{ 0 }; side < 2; ++side)
				{
					core::utility::Vertex3D vertex{};
					vertex.position =
					    Vector3{ x + (side == 0 ? -width : width) * 0.5f, height, z };
					vertex.normal = Vector3{ 0.0f, 0.0f, -1.0f };
					vertex.color = palette::TEXT_PRIMARY;
					vertex.alpha = alpha;
					m_steamVertices.push_back(vertex);
				}

				if (step == 0)
					continue;

				const auto previous{ static_cast<unsigned short>(base - 2) };
				m_steamIndices.push_back(previous);
				m_steamIndices.push_back(base);
				m_steamIndices.push_back(static_cast<unsigned short>(previous + 1));

				m_steamIndices.push_back(static_cast<unsigned short>(previous + 1));
				m_steamIndices.push_back(base);
				m_steamIndices.push_back(static_cast<unsigned short>(base + 1));
			}
		}

		if (m_steamIndices.empty())
			return;

		// 湯気は光を透かす。足し合わせて描くと、灯りの中で白く浮かぶ
		m_renderer3D.setTexture(-1);
		m_renderer3D.setBackCulling(false);
		m_renderer3D.setBlend(core::utility::BlendMode::Add, 1.0f);
		m_renderer3D.drawTriangles(m_steamVertices, m_steamIndices);
		m_renderer3D.setBlend(core::utility::BlendMode::None, 1.0f);
		m_renderer3D.setBackCulling(true);
	}

	void RoomBackdrop::draw() const
	{
		// 視点をゆっくり漂わせる。止まっていると絵葉書に見えてしまう
		const Vector3 position{ CAMERA_BASE.x + std::sin(m_time * DRIFT_X_SPEED) * DRIFT_X,
			                    CAMERA_BASE.y + std::sin(m_time * DRIFT_Y_SPEED) * DRIFT_Y,
			                    CAMERA_BASE.z };

		m_camera.setPerspective(CAMERA_FOV, CAMERA_NEAR, CAMERA_FAR);
		m_camera.lookAt(position, CAMERA_TARGET);

		m_renderer3D.setTexture(m_floorTexture);
		m_renderer3D.drawTriangles(m_floorVertices, m_floorIndices);

		m_renderer3D.setTexture(-1);
		m_renderer3D.setBlend(core::utility::BlendMode::Alpha, 1.0f);
		m_renderer3D.drawTriangles(m_shadowVertices, m_shadowIndices);
		m_renderer3D.setBlend(core::utility::BlendMode::None, 1.0f);

		m_modelRenderer.draw(m_cupModel, CUP_POSITION, Vector3{}, 1.0f);
		m_modelRenderer.draw(m_potModel, POT_POSITION, Vector3{}, POT_SCALE);

		drawSteam();

		// 溜まっている 3D を吐き出しておく
		m_renderer3D.flush();
	}

	void RoomBackdrop::drawOverlay() const
	{
		const core::utility::Vector2 origin{ 0.0f, 0.0f };
		const core::utility::Vector2 size{ static_cast<float>(m_screen.getWidth()),
			                               static_cast<float>(m_screen.getHeight()) };

		// 粒状感は毎フレーム位置をずらす。止まっていると汚れに見えてしまう
		const float shift{ std::fmod(m_time * 37.0f, 64.0f) };
		m_renderer.drawTextureStretched(m_grainTexture,
		                                core::utility::Vector2{ -shift, -shift },
		                                core::utility::Vector2{ size.x + 64.0f, size.y + 64.0f },
		                                GRAIN_STRENGTH);

		m_renderer.drawTextureStretched(m_vignetteTexture, origin, size, VIGNETTE_STRENGTH);
	}
} // namespace game::view
