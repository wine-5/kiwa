#include "game/view/PourView3D.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IModelRenderer.h"
#include "core/utility/Easing.h"
#include "core/utility/MathConstants.h"
#include "core/interface/IRenderer3D.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "game/constant/Fonts.h"
#include "game/constant/UiTextures.h"
#include "game/constant/Palette.h"
#include "game/view/CupGeometry.h"
#include "game/view/SceneryMesh.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Vector3;
	namespace font = game::constant::font;
	namespace palette = game::constant::palette;
	namespace cup = game::view::cup;

	/// @brief こぼれが広がる範囲の半径（組むときの大きさ。描くときに器へ合わせる）
	constexpr float PUDDLE_RADIUS{ 0.68f };

	/// @brief 染みの広さ（器の口の半径に対する倍率）
	constexpr float PUDDLE_EXTENT{ 1.7f };

	/// @brief 手番を告げる文字の大きさ
	constexpr int CALL_FONT_SIZE{ 76 };

	/// @brief 染みの濃さ（畳の目が透けるくらいに抑える）
	constexpr float PUDDLE_ALPHA{ 0.6f };

	/// @brief 筋が台に届いたとみなす進み具合
	constexpr float PUDDLE_LANDED{ 0.92f };

	/// @brief 染みが滲み広がる速さ（1秒あたりの割合）
	constexpr float PUDDLE_SOAK_RATE{ 0.55f };

	/// @brief こぼれの厚み
	constexpr float PUDDLE_THICKNESS{ 0.012f };

	/// @brief こぼれの輪の分割数
	constexpr int PUDDLE_SEGMENTS{ 48 };

	/// @brief こぼれの中心から縁までの分割数
	constexpr int PUDDLE_RINGS{ 6 };

	/// @brief 床の畳表
	constexpr const char* FLOOR_TEXTURE_PATH{ "assets/textures/tatami.png" };

	/// @brief 周辺減光
	constexpr const char* VIGNETTE_TEXTURE_PATH{ "assets/textures/vignette.png" };

	/// @brief 粒状感
	constexpr const char* GRAIN_TEXTURE_PATH{ "assets/textures/grain.png" };

	/// @brief 札の裏
	constexpr const char* CARD_BACK_TEXTURE_PATH{ "assets/textures/card_back.png" };

	/// @brief 先攻の札
	constexpr const char* CARD_FIRST_TEXTURE_PATH{ "assets/textures/card_first.png" };

	/// @brief 後攻の札
	constexpr const char* CARD_SECOND_TEXTURE_PATH{ "assets/textures/card_second.png" };


	/// @brief 周辺減光の濃さ
	constexpr float VIGNETTE_STRENGTH{ 0.55f };

	/// @brief 粒状感の濃さ
	constexpr float GRAIN_STRENGTH{ 0.05f };

	// ---- 器と土瓶 ----

	/**
	 * @brief 器のモデルの在処を返す
	 * @param look 器の見た目
	 * @return モデルのファイルの場所
	 */
	constexpr const char* cupModelPath(game::view::VesselLook look) noexcept
	{
		switch (look)
		{
		case game::view::VesselLook::Guinomi:
			return "assets/model/guinomi.mqo";
		case game::view::VesselLook::Sobachoko:
			return "assets/model/sobachoko.mqo";
		case game::view::VesselLook::Chawan:
			return "assets/model/chawan.mqo";
		case game::view::VesselLook::Yunomi:
		default:
			return "assets/model/yunomi.mqo";
		}
	}

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


	// 器の中を覗き込む高さから、少し横にずらして構える。
	// 画角は狭めにして、写真のように歪みを抑える
	// 器の中が見える限界まで下げた、写真に近い高さ。
	// これ以上下げると手前の板に隠れて液面が見えなくなる
	constexpr Vector3 CAMERA_POSITION{ 0.4f, 1.62f, -1.9f };
	constexpr Vector3 CAMERA_TARGET{ -0.12f, 0.52f, 0.0f };
	constexpr float CAMERA_FOV{ 0.85f };

	/// @brief 寄り切ったときに周辺減光へ足す濃さ
	constexpr float CLOSE_VIGNETTE{ 0.22f };
	constexpr float CAMERA_NEAR{ 0.1f };
	constexpr float CAMERA_FAR{ 100.0f };
} // namespace

namespace game::view
{
	PourView3D::PourView3D(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
	                       core::iface::ICamera& camera, core::iface::IModelRenderer& modelRenderer,
	                       core::iface::IResourceManager& resource, core::iface::IScreen& screen)
	    : m_renderer3D{ renderer3D }, m_renderer{ renderer }, m_modelRenderer{ modelRenderer },
	      m_screen{ screen }, m_duelCamera{ camera }
	{
		// 枡も台も動かないので、形は最初に一度だけ組んで使い回す
		SceneryMesh::buildFloor(m_floorVertices, m_floorIndices);
		SceneryMesh::buildShadow(m_shadowVertices, m_shadowIndices);
		buildPuddle();

		m_floorTexture = resource.loadTexture(FLOOR_TEXTURE_PATH);
		m_vignetteTexture = resource.loadTexture(VIGNETTE_TEXTURE_PATH);
		m_grainTexture = resource.loadTexture(GRAIN_TEXTURE_PATH);
		m_cardBackTexture = resource.loadTexture(CARD_BACK_TEXTURE_PATH);
		m_cardFirstTexture = resource.loadTexture(CARD_FIRST_TEXTURE_PATH);
		m_cardSecondTexture = resource.loadTexture(CARD_SECOND_TEXTURE_PATH);

		// 画面に重ねる紙物。読み込みはここでまとめて済ませる
		namespace ui = game::constant::ui;
		m_turnPlateTexture = resource.loadTexture(ui::TURN_PLATE);
		m_scorePlateTexture = resource.loadTexture(ui::SCORE_PLATE);
		m_emblemOneTexture = resource.loadTexture(ui::EMBLEM_ONE);
		m_emblemTwoTexture = resource.loadTexture(ui::EMBLEM_TWO);
		m_keyCapSpaceTexture = resource.loadTexture(ui::KEY_CAP_SPACE);
		m_keyCapEnterTexture = resource.loadTexture(ui::KEY_CAP_ENTER);
		// 器はどれが出ても待たせないよう、はじめに全部読んでおく
		for (std::size_t i{ 0 }; i < m_cupModels.size(); ++i)
			m_cupModels[i] = resource.loadModel(cupModelPath(static_cast<VesselLook>(i)));

		m_potModel = resource.loadModel(POT_MODEL_PATH);

		m_headingFont = resource.loadFont(font::HEADING_FAMILY, font::HEADING_SIZE);
		m_callFont = resource.loadFont(font::HEADING_FAMILY, CALL_FONT_SIZE);
		m_bodyFont = resource.loadFont(font::BODY_FAMILY, font::BODY_SIZE);
	}

	void PourView3D::update(float deltaTime)
	{
		// 注ぐときは土瓶を前へ倒す。急に切り替わらないよう追いかけさせる
		const float target{ m_isPouring ? POT_POUR_TILT : POT_REST_TILT };
		m_potTilt = core::utility::Easing::approach(m_potTilt, target, POT_TILT_RATE, deltaTime);

		// 液体は傾いた注ぎ口の先から出る
		m_liquid.setPourOrigin(spoutTip(m_potTilt));

		m_duelCamera.update(deltaTime, DuelCamera::Focus{ m_amountRatio, m_isPouring,
		                                                  m_isOverflowed,
		                                                  cup::shapeOf(m_vesselLook).rimHeight });

		m_liquid.update(deltaTime, m_isPouring, m_amountRatio);
		m_spill.update(deltaTime, m_isOverflowed);

		// 筋が台に届いてから、染みがじわじわ広がる
		const bool hasLanded{ m_isOverflowed && m_spill.getReach() > PUDDLE_LANDED };
		m_puddleGrowth = hasLanded
		                     ? core::utility::Easing::approach(m_puddleGrowth, 1.0f, PUDDLE_SOAK_RATE,
		                                                       deltaTime)
		                     : 0.0f;
		m_hud.update(deltaTime, m_hudContent);
		m_turnCall.update(deltaTime, m_turnCallContent);
		m_cardDraw.update(deltaTime, m_cardContent);
		m_grainTime += deltaTime;
	}

	void PourView3D::draw()
	{
		m_duelCamera.apply();

		drawScenery();
		m_modelRenderer.draw(m_cupModels[static_cast<std::size_t>(m_vesselLook)],
		                     Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{}, 1.0f);
		m_modelRenderer.draw(m_potModel, potPosition(m_potTilt), Vector3{ 0.0f, 0.0f, m_potTilt },
		                     POT_SCALE);
		m_spill.draw(m_renderer3D);
		drawPuddle();
		m_liquid.draw(m_renderer3D, m_duelCamera.getEye());

		// 溜まっている 3D を吐き出しておく
		m_renderer3D.flush();
	}

	void PourView3D::drawOverlay()
	{
		drawFilmLook();

		m_hud.draw(m_renderer, m_screen,
		           GameHud::Resources{ m_turnPlateTexture, m_scorePlateTexture, m_emblemOneTexture,
			                           m_emblemTwoTexture, m_keyCapSpaceTexture,
			                           m_keyCapEnterTexture, m_headingFont, m_bodyFont,
			                           font::HEADING_SIZE, font::BODY_SIZE });

		// 手番の告知は、器の上の空いたところで一度だけ大きく見せる
		m_turnCall.draw(m_renderer, m_screen, m_callFont, CALL_FONT_SIZE);

		// 札は画面に重ねて大きく見せる。3D の中に置くと小さすぎて読み取れない
		m_cardDraw.draw(m_renderer, m_screen,
		                CardDraw::Resources{ m_cardBackTexture, m_cardFirstTexture,
			                                 m_cardSecondTexture, m_headingFont, m_bodyFont });
	}

	void PourView3D::drawScenery() const
	{
		m_renderer3D.setTexture(m_floorTexture);
		m_renderer3D.drawTriangles(m_floorVertices, m_floorIndices);

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

		// 寄っているあいだは四隅をさらに落とす。視野が狭まると息が詰まる
		m_renderer.drawTextureStretched(m_vignetteTexture, origin, size,
		                                VIGNETTE_STRENGTH + CLOSE_VIGNETTE * m_duelCamera.getCloseUp());
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

				// 真円だと水たまりに見えない。畳の目に沿って不均一に広がるので、
				// 向きによって伸び方を変えていびつな染みにする
				const float wobble{ 1.0f + 0.16f * std::sin(angle * 3.0f + 0.7f) +
					                0.09f * std::sin(angle * 5.0f + 2.1f) };

				core::utility::Vertex3D vertex{};
				vertex.position = Vector3{ std::cos(angle) * radius * wobble, PUDDLE_THICKNESS,
					                       std::sin(angle) * radius * wobble };
				vertex.normal = Vector3{ 0.0f, 1.0f, 0.0f };
				// 畳に吸われた茶は沈んだ色になる。外へ行くほど薄く、滲んで消える
				vertex.color = core::utility::mixed(palette::LIQUID_SPILLED, palette::LIQUID_SOAKED, t);
				vertex.alpha = 0.78f * (1.0f - t * t) * (1.0f - t * 0.35f);
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

		// 染みは筋が下りきってから、時間をかけて滲み広がる。
		// 一気に広げると、いきなり床が緑になったように見えてしまう
		const float spread{ m_puddleGrowth };
		if (spread <= 0.01f)
			return;

		// 染みの広さは器の口に合わせる。小さい器から湖ができては嘘になる
		const float extent{ cup::shapeOf(m_vesselLook).rimRadius * PUDDLE_EXTENT / PUDDLE_RADIUS };
		const float scale{ extent * spread };

		m_puddleFrame = m_puddleVertices;
		for (core::utility::Vertex3D& vertex : m_puddleFrame)
		{
			vertex.position.x *= scale;
			vertex.position.z *= scale;
			vertex.alpha *= spread * PUDDLE_ALPHA;
		}

		// こぼれたぶんは器の外へ広がる。
		// 自前で組んだメッシュは裏表を取り違えやすいので、面の省略は切っておく
		m_renderer3D.setBackCulling(false);
		m_renderer3D.setBlend(core::utility::BlendMode::Alpha, 1.0f);
		m_renderer3D.drawTriangles(m_puddleFrame, m_puddleIndices);
		m_renderer3D.setBlend(core::utility::BlendMode::None, 1.0f);
		m_renderer3D.setBackCulling(true);
	}

} // namespace game::view
