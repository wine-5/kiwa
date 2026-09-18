#include "game/view/PourView3D.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IModelRenderer.h"
#include "core/interface/IRenderer3D.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "game/constant/Palette.h"
#include "game/view/MasuGeometry.h"
#include "game/view/SceneryMesh.h"
#include <cmath>

namespace
{
	using core::utility::Vector3;
	namespace palette = game::constant::palette;
	namespace masu = game::view::masu;

	/// @brief 際を示す線の太さ
	constexpr float LINE_THICKNESS{ 0.012f };

	/// @brief こぼれが広がる範囲の半分の幅
	constexpr float PUDDLE_HALF{ 1.45f };

	/// @brief こぼれの厚み
	constexpr float PUDDLE_THICKNESS{ 0.03f };

	/// @brief 枡の木目
	constexpr const char* MASU_TEXTURE_PATH{ "assets/textures/wood_masu.png" };

	/// @brief 台の木目（ピントが外れている想定で、あらかじめぼかしてある）
	constexpr const char* TABLE_TEXTURE_PATH{ "assets/textures/wood_table.png" };

	/// @brief 周辺減光
	constexpr const char* VIGNETTE_TEXTURE_PATH{ "assets/textures/vignette.png" };

	/// @brief 粒状感
	constexpr const char* GRAIN_TEXTURE_PATH{ "assets/textures/grain.png" };

	/// @brief 周辺減光の濃さ
	constexpr float VIGNETTE_STRENGTH{ 0.55f };

	/// @brief 粒状感の濃さ
	constexpr float GRAIN_STRENGTH{ 0.05f };

	// ---- 急須 ----

	/// @brief 急須のモデル
	constexpr const char* KYUSU_MODEL_PATH{ "assets/model/kyusu.mqo" };

	/// @brief 急須の大きさ
	constexpr float KYUSU_SCALE{ 1.05f };

	/// @brief 急須の傾き（注ぎ口が下を向くように前へ倒す）
	constexpr float KYUSU_TILT{ -0.62f };

	/// @brief モデルの中での注ぎ口の先の位置
	constexpr Vector3 KYUSU_SPOUT_LOCAL{ 0.95f, 0.82f, 0.0f };

	/// @brief 注ぎ口の先を置きたい位置（枡の中心の真上）
	constexpr Vector3 SPOUT_TARGET{ 0.0f, 1.85f, 0.05f };

	/**
	 * @brief Z軸まわりに回した位置を返す
	 * @param point 回す点
	 * @param radian 回す角（ラジアン）
	 * @return 回したあとの位置
	 */
	Vector3 rotateZ(const Vector3& point, float radian)
	{
		const float cosine{ std::cos(radian) };
		const float sine{ std::sin(radian) };
		return Vector3{ point.x * cosine - point.y * sine, point.x * sine + point.y * cosine, point.z };
	}

	/**
	 * @brief 注ぎ口の先が狙った位置に来るよう、急須を置く位置を求める
	 * @return 急須を置く位置
	 */
	Vector3 kyusuPosition()
	{
		return SPOUT_TARGET - rotateZ(KYUSU_SPOUT_LOCAL * KYUSU_SCALE, KYUSU_TILT);
	}

	// ---- カメラ ----

	// 器の中を覗き込む高さから、少し横にずらして構える。
	// 画角は狭めにして、写真のように歪みを抑える
	// 器の中が見える限界まで下げた、写真に近い高さ。
	// これ以上下げると手前の板に隠れて液面が見えなくなる
	constexpr Vector3 CAMERA_POSITION{ 0.6f, 3.9f, -3.5f };
	constexpr Vector3 CAMERA_TARGET{ -0.15f, 1.1f, 0.0f };
	constexpr float CAMERA_FOV{ 0.8f };
	constexpr float CAMERA_NEAR{ 0.1f };
	constexpr float CAMERA_FAR{ 100.0f };
} // namespace

namespace game::view
{
	PourView3D::PourView3D(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
	                       core::iface::ICamera& camera, core::iface::IModelRenderer& modelRenderer,
	                       core::iface::IResourceManager& resource, core::iface::IScreen& screen)
	    : m_renderer3D{ renderer3D }, m_renderer{ renderer }, m_camera{ camera },
	      m_modelRenderer{ modelRenderer }, m_screen{ screen }
	{
		// 枡も台も動かないので、形は最初に一度だけ組んで使い回す
		SceneryMesh::buildMasu(m_masuVertices, m_masuIndices);
		SceneryMesh::buildTable(m_tableVertices, m_tableIndices);
		SceneryMesh::buildShadow(m_shadowVertices, m_shadowIndices);

		m_masuTexture = resource.loadTexture(MASU_TEXTURE_PATH);
		m_tableTexture = resource.loadTexture(TABLE_TEXTURE_PATH);
		m_vignetteTexture = resource.loadTexture(VIGNETTE_TEXTURE_PATH);
		m_grainTexture = resource.loadTexture(GRAIN_TEXTURE_PATH);
		m_kyusuModel = m_modelRenderer.load(KYUSU_MODEL_PATH);

		// 液体は急須の注ぎ口から出る
		m_liquid.setPourOrigin(SPOUT_TARGET);
	}

	void PourView3D::advance(float deltaTime)
	{
		m_liquid.advance(deltaTime, m_isPouring, m_amountRatio);
		m_grainTime += deltaTime;
	}

	void PourView3D::draw()
	{
		m_camera.setPerspective(CAMERA_FOV, CAMERA_NEAR, CAMERA_FAR);
		m_camera.lookAt(CAMERA_POSITION, CAMERA_TARGET);

		drawScenery();
		m_modelRenderer.draw(m_kyusuModel, kyusuPosition(), Vector3{ 0.0f, 0.0f, KYUSU_TILT },
		                     KYUSU_SCALE);
		drawPuddle();
		drawLimitLine();
		m_liquid.draw(m_renderer3D, CAMERA_POSITION);

		// 溜まっている 3D を吐き出しておく
		m_renderer3D.flush();
	}

	void PourView3D::drawOverlay()
	{
		drawFilmLook();
		drawTexts();
	}

	void PourView3D::drawScenery() const
	{
		m_renderer3D.setTexture(m_tableTexture);
		m_renderer3D.drawTriangles(m_tableVertices, m_tableIndices);

		// 影は台の上、枡より先に置く
		m_renderer3D.setTexture(-1);
		m_renderer3D.setBlend(core::utility::BlendMode::Alpha, 1.0f);
		m_renderer3D.drawTriangles(m_shadowVertices, m_shadowIndices);
		m_renderer3D.setBlend(core::utility::BlendMode::None, 1.0f);

		m_renderer3D.setTexture(m_masuTexture);
		m_renderer3D.drawTriangles(m_masuVertices, m_masuIndices);

		m_renderer3D.setTexture(-1);
	}

	void PourView3D::drawFilmLook() const
	{
		const core::utility::Vector2 origin{ 0.0f, 0.0f };
		const core::utility::Vector2 size{ static_cast<float>(m_screen.getWidth()),
			                               static_cast<float>(m_screen.getHeight()) };

		// 粒状感は毎フレーム位置をずらす。止まっていると汚れに見えてしまう
		const float shift{ std::fmod(m_grainTime * 37.0f, 64.0f) };
		m_renderer.drawTextureStretched(m_grainTexture,
		                                core::utility::Vector2{ -shift, -shift },
		                                core::utility::Vector2{ size.x + 64.0f, size.y + 64.0f },
		                                GRAIN_STRENGTH);

		m_renderer.drawTextureStretched(m_vignetteTexture, origin, size, VIGNETTE_STRENGTH);
	}

	void PourView3D::drawPuddle() const
	{
		if (!m_isOverflowed)
			return;

		m_renderer3D.drawBox(Vector3{ -PUDDLE_HALF, 0.0f, -PUDDLE_HALF },
		                     Vector3{ PUDDLE_HALF, PUDDLE_THICKNESS, PUDDLE_HALF },
		                     palette::LIQUID_SPILLED);
	}

	void PourView3D::drawLimitLine() const
	{
		if (!m_isLimitVisible)
			return;

		const float y{ masu::surfaceHeight(m_limitRatio) };

		// 内側の3面に沿って細い線を回す
		m_renderer3D.drawBox(Vector3{ -masu::INNER_HALF, y, masu::INNER_HALF - LINE_THICKNESS },
		                     Vector3{ masu::INNER_HALF, y + LINE_THICKNESS, masu::INNER_HALF },
		                     palette::LIMIT_LINE);
		m_renderer3D.drawBox(Vector3{ masu::INNER_HALF - LINE_THICKNESS, y, -masu::INNER_HALF },
		                     Vector3{ masu::INNER_HALF, y + LINE_THICKNESS, masu::INNER_HALF },
		                     palette::LIMIT_LINE);
		m_renderer3D.drawBox(Vector3{ -masu::INNER_HALF, y, -masu::INNER_HALF },
		                     Vector3{ -masu::INNER_HALF + LINE_THICKNESS, y + LINE_THICKNESS,
		                              masu::INNER_HALF },
		                     palette::LIMIT_LINE);
	}

	void PourView3D::drawTexts() const
	{
		const float centerX{ m_screen.getWidth() * 0.5f };
		const float height{ static_cast<float>(m_screen.getHeight()) };

		if (!m_message.empty())
			m_renderer.drawTextCentered(core::utility::Vector2{ centerX, height - 140.0f }, m_message,
			                            palette::TEXT_PRIMARY);

		if (!m_prompt.empty())
			m_renderer.drawTextCentered(core::utility::Vector2{ centerX, height - 100.0f }, m_prompt,
			                            palette::TEXT_SUB);
	}
} // namespace game::view
