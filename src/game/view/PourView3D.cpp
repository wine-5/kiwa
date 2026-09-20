#include "game/view/PourView3D.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IModelRenderer.h"
#include "core/utility/MathConstants.h"
#include "core/interface/IRenderer3D.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "game/constant/Palette.h"
#include "game/view/CupGeometry.h"
#include "game/view/SceneryMesh.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Vector3;
	namespace palette = game::constant::palette;
	namespace cup = game::view::cup;

	/// @brief 際を示す線の太さ
	constexpr float LINE_THICKNESS{ 0.006f };

	/// @brief 際の線を描く輪の分割数
	constexpr int LINE_SEGMENTS{ 56 };

	/// @brief こぼれが広がる範囲の半径
	constexpr float PUDDLE_RADIUS{ 0.68f };

	/// @brief こぼれの厚み
	constexpr float PUDDLE_THICKNESS{ 0.012f };

	/// @brief こぼれの輪の分割数
	constexpr int PUDDLE_SEGMENTS{ 48 };

	/// @brief こぼれの中心から縁までの分割数
	constexpr int PUDDLE_RINGS{ 6 };

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

	// ---- 器と土瓶 ----

	/// @brief 湯呑のモデル
	constexpr const char* CUP_MODEL_PATH{ "assets/model/yunomi.mqo" };

	/// @brief 土瓶のモデル
	constexpr const char* POT_MODEL_PATH{ "assets/model/dobin.mqo" };

	/// @brief 土瓶の大きさ
	constexpr float POT_SCALE{ 0.85f };

	/// @brief 注いでいないときの傾き
	constexpr float POT_REST_TILT{ -0.10f };

	/// @brief 注いでいるときの傾き
	constexpr float POT_POUR_TILT{ -0.80f };

	/// @brief 傾きが変わる速さ
	constexpr float POT_TILT_RATE{ 4.5f };

	/// @brief モデルの中で、傾きの中心にする点（胴の中心）
	constexpr Vector3 POT_PIVOT_LOCAL{ 0.0f, 0.34f, 0.0f };

	/// @brief モデルの中での注ぎ口の先の位置
	constexpr Vector3 POT_SPOUT_LOCAL{ 0.92f, 0.56f, 0.0f };

	/// @brief 傾きの中心を置く位置（ここを軸に土瓶が回る）
	constexpr Vector3 POT_PIVOT_WORLD{ -0.68f, 1.35f, 0.03f };

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
	 * @brief 傾きに応じた土瓶の置き位置を求める
	 * @details 胴の中心を軸に回すため、回したぶんだけ置き位置をずらす
	 * @param tilt 傾き（ラジアン）
	 * @return 土瓶を置く位置
	 */
	Vector3 potPosition(float tilt)
	{
		return POT_PIVOT_WORLD - rotateZ(POT_PIVOT_LOCAL * POT_SCALE, tilt);
	}

	/**
	 * @brief 傾きに応じた注ぎ口の先の位置を求める
	 * @param tilt 傾き（ラジアン）
	 * @return 注ぎ口の先の位置
	 */
	Vector3 spoutTip(float tilt)
	{
		return POT_PIVOT_WORLD + rotateZ((POT_SPOUT_LOCAL - POT_PIVOT_LOCAL) * POT_SCALE, tilt);
	}

	// ---- カメラ ----

	// 器の中を覗き込む高さから、少し横にずらして構える。
	// 画角は狭めにして、写真のように歪みを抑える
	// 器の中が見える限界まで下げた、写真に近い高さ。
	// これ以上下げると手前の板に隠れて液面が見えなくなる
	constexpr Vector3 CAMERA_POSITION{ 0.4f, 1.62f, -1.9f };
	constexpr Vector3 CAMERA_TARGET{ -0.12f, 0.52f, 0.0f };
	constexpr float CAMERA_FOV{ 0.85f };
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
		SceneryMesh::buildTable(m_tableVertices, m_tableIndices);
		SceneryMesh::buildShadow(m_shadowVertices, m_shadowIndices);
		buildPuddle();

		m_tableTexture = resource.loadTexture(TABLE_TEXTURE_PATH);
		m_vignetteTexture = resource.loadTexture(VIGNETTE_TEXTURE_PATH);
		m_grainTexture = resource.loadTexture(GRAIN_TEXTURE_PATH);
		m_cupModel = m_modelRenderer.load(CUP_MODEL_PATH);
		m_potModel = m_modelRenderer.load(POT_MODEL_PATH);
	}

	void PourView3D::advance(float deltaTime)
	{
		// 注ぐときは土瓶を前へ倒す。急に切り替わらないよう追いかけさせる
		const float target{ m_isPouring ? POT_POUR_TILT : POT_REST_TILT };
		m_potTilt += (target - m_potTilt) * std::min(1.0f, POT_TILT_RATE * deltaTime);

		// 液体は傾いた注ぎ口の先から出る
		m_liquid.setPourOrigin(spoutTip(m_potTilt));

		m_liquid.advance(deltaTime, m_isPouring, m_amountRatio);
		m_grainTime += deltaTime;
	}

	void PourView3D::draw()
	{
		m_camera.setPerspective(CAMERA_FOV, CAMERA_NEAR, CAMERA_FAR);
		m_camera.lookAt(CAMERA_POSITION, CAMERA_TARGET);

		drawScenery();
		m_modelRenderer.draw(m_cupModel, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{}, 1.0f);
		m_modelRenderer.draw(m_potModel, potPosition(m_potTilt), Vector3{ 0.0f, 0.0f, m_potTilt },
		                     POT_SCALE);
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

		// 影は台の上、器より先に置く
		m_renderer3D.setTexture(-1);
		m_renderer3D.setBlend(core::utility::BlendMode::Alpha, 1.0f);
		m_renderer3D.drawTriangles(m_shadowVertices, m_shadowIndices);
		m_renderer3D.setBlend(core::utility::BlendMode::None, 1.0f);
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

	void PourView3D::buildPuddle()
	{
		// 中心が濃く、外へ向かって消えていく円い染み
		const auto base{ static_cast<unsigned short>(m_puddleVertices.size()) };

		for (int ring{ 0 }; ring <= PUDDLE_RINGS; ++ring)
		{
			const float t{ static_cast<float>(ring) / PUDDLE_RINGS };
			const float radius{ PUDDLE_RADIUS * t };

			for (int segment{ 0 }; segment < PUDDLE_SEGMENTS; ++segment)
			{
				const float angle{ core::utility::math::TWO_PI * segment / PUDDLE_SEGMENTS };

				core::utility::Vertex3D vertex{};
				vertex.position = Vector3{ std::cos(angle) * radius, PUDDLE_THICKNESS,
					                       std::sin(angle) * radius };
				vertex.normal = Vector3{ 0.0f, 1.0f, 0.0f };
				// 濡れた面は光を返すので、器の中の色より明るく置く
				vertex.color = core::utility::mixed(palette::LIQUID, palette::LIQUID_SPILLED, t);
				vertex.alpha = 0.92f * (1.0f - t * t * t);
				m_puddleVertices.push_back(vertex);
			}
		}

		for (int ring{ 0 }; ring < PUDDLE_RINGS; ++ring)
		{
			for (int segment{ 0 }; segment < PUDDLE_SEGMENTS; ++segment)
			{
				const int next{ (segment + 1) % PUDDLE_SEGMENTS };
				const auto inner{ static_cast<unsigned short>(base + ring * PUDDLE_SEGMENTS + segment) };
				const auto innerNext{ static_cast<unsigned short>(base + ring * PUDDLE_SEGMENTS + next) };
				const auto outer{ static_cast<unsigned short>(inner + PUDDLE_SEGMENTS) };
				const auto outerNext{ static_cast<unsigned short>(innerNext + PUDDLE_SEGMENTS) };

				m_puddleIndices.push_back(inner);
				m_puddleIndices.push_back(outer);
				m_puddleIndices.push_back(innerNext);

				m_puddleIndices.push_back(innerNext);
				m_puddleIndices.push_back(outer);
				m_puddleIndices.push_back(outerNext);
			}
		}
	}

	void PourView3D::drawPuddle() const
	{
		if (!m_isOverflowed || m_puddleIndices.empty())
			return;

		// こぼれたぶんは器の外へ広がる。
		// 自前で組んだメッシュは裏表を取り違えやすいので、面の省略は切っておく
		m_renderer3D.setBackCulling(false);
		m_renderer3D.setBlend(core::utility::BlendMode::Alpha, 1.0f);
		m_renderer3D.drawTriangles(m_puddleVertices, m_puddleIndices);
		m_renderer3D.setBlend(core::utility::BlendMode::None, 1.0f);
		m_renderer3D.setBackCulling(true);
	}

	void PourView3D::drawLimitLine() const
	{
		if (!m_isLimitVisible)
			return;

		const float y{ cup::surfaceHeight(m_limitRatio) };
		const float radius{ cup::radiusAt(y) };

		// 器の内側に沿って細い線を一周させる
		for (int i{ 0 }; i < LINE_SEGMENTS; ++i)
		{
			const float angle{ core::utility::math::TWO_PI * i / LINE_SEGMENTS };
			const float nextAngle{ core::utility::math::TWO_PI * (i + 1) / LINE_SEGMENTS };

			const Vector3 from{ std::cos(angle) * radius, y, std::sin(angle) * radius };
			const Vector3 to{ std::cos(nextAngle) * radius, y, std::sin(nextAngle) * radius };
			m_renderer3D.drawCapsule(from, to, LINE_THICKNESS, palette::LIMIT_LINE);
		}
	}

	void PourView3D::drawTexts() const
	{
		const float centerX{ m_screen.getWidth() * 0.5f };
		const float height{ static_cast<float>(m_screen.getHeight()) };

		if (!m_turnLabel.empty())
			m_renderer.drawTextCentered(core::utility::Vector2{ centerX, 58.0f }, m_turnLabel,
			                            palette::TEXT_PRIMARY);

		// 勝敗は隅に小さく置く。手番の表示と重ねると読みにくい
		if (!m_scoreLabel.empty())
			m_renderer.drawText(core::utility::Vector2{ 36.0f, 32.0f }, m_scoreLabel,
			                    palette::TEXT_SUB);

		if (!m_message.empty())
			m_renderer.drawTextCentered(core::utility::Vector2{ centerX, height - 140.0f }, m_message,
			                            palette::TEXT_PRIMARY);

		if (!m_prompt.empty())
			m_renderer.drawTextCentered(core::utility::Vector2{ centerX, height - 100.0f }, m_prompt,
			                            palette::TEXT_SUB);
	}
} // namespace game::view
