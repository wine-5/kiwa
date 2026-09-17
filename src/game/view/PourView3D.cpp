#include "game/view/PourView3D.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IRenderer3D.h"
#include "core/interface/IScreen.h"
#include "game/constant/Palette.h"
#include "game/view/MasuGeometry.h"

namespace
{
	using core::utility::Vector3;
	namespace palette = game::constant::palette;
	namespace masu = game::view::masu;

	/// @brief 際を示す線の太さ
	constexpr float LINE_THICKNESS{ 0.02f };

	/// @brief こぼれが広がる範囲の半分の幅
	constexpr float PUDDLE_HALF{ 1.45f };

	/// @brief こぼれの厚み
	constexpr float PUDDLE_THICKNESS{ 0.03f };

	// ---- 台 ----

	/// @brief 台の半分の幅
	constexpr float TABLE_HALF{ 6.0f };

	/// @brief 台の厚み
	constexpr float TABLE_THICKNESS{ 0.3f };

	// ---- カメラ ----

	constexpr Vector3 CAMERA_POSITION{ 0.0f, 2.6f, -3.4f };
	constexpr Vector3 CAMERA_TARGET{ 0.0f, 0.7f, 0.0f };
	constexpr float CAMERA_FOV{ 0.8f };
	constexpr float CAMERA_NEAR{ 0.1f };
	constexpr float CAMERA_FAR{ 100.0f };
} // namespace

namespace game::view
{
	PourView3D::PourView3D(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
	                       core::iface::ICamera& camera, core::iface::IScreen& screen)
	    : m_renderer3D{ renderer3D }, m_renderer{ renderer }, m_camera{ camera }, m_screen{ screen }
	{
	}

	void PourView3D::advance(float deltaTime)
	{
		m_liquid.advance(deltaTime, m_isPouring, m_amountRatio);
	}

	void PourView3D::draw()
	{
		m_camera.setPerspective(CAMERA_FOV, CAMERA_NEAR, CAMERA_FAR);
		m_camera.lookAt(CAMERA_POSITION, CAMERA_TARGET);

		// 台
		m_renderer3D.drawBox(Vector3{ -TABLE_HALF, -TABLE_THICKNESS, -TABLE_HALF },
		                     Vector3{ TABLE_HALF, 0.0f, TABLE_HALF }, palette::TABLE);

		drawMasu();
		drawPuddle();
		drawLimitLine();
		m_liquid.draw(m_renderer3D);

		// 文字は 3D の手前に出したいので、ここで 3D を吐き出してから描く
		m_renderer3D.flush();
		drawTexts();
	}

	void PourView3D::drawMasu() const
	{
		// 底板
		m_renderer3D.drawBox(Vector3{ -masu::OUTER_HALF, 0.0f, -masu::OUTER_HALF },
		                     Vector3{ masu::OUTER_HALF, masu::FLOOR_TOP, masu::OUTER_HALF },
		                     palette::MASU_FLOOR);

		// 側板は3枚だけ描く。手前の板を省くと中の液面がいつでも見えるため
		// （実物を切り開いて覗いているような見え方になる）
		m_renderer3D.drawBox(Vector3{ -masu::OUTER_HALF, 0.0f, masu::INNER_HALF },
		                     Vector3{ masu::OUTER_HALF, masu::HEIGHT, masu::OUTER_HALF },
		                     palette::MASU_BACK);
		m_renderer3D.drawBox(Vector3{ masu::INNER_HALF, 0.0f, -masu::OUTER_HALF },
		                     Vector3{ masu::OUTER_HALF, masu::HEIGHT, masu::OUTER_HALF },
		                     palette::MASU_RIGHT);
		m_renderer3D.drawBox(Vector3{ -masu::OUTER_HALF, 0.0f, -masu::OUTER_HALF },
		                     Vector3{ -masu::INNER_HALF, masu::HEIGHT, masu::OUTER_HALF },
		                     palette::MASU_LEFT);
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
