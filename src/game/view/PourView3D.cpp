#include "game/view/PourView3D.h"
#include "core/interface/ICamera.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IRenderer3D.h"
#include "core/interface/IScreen.h"
#include "game/constant/Palette.h"

namespace
{
	using core::utility::Vector3;
	namespace palette = game::constant::palette;

	// ---- 枡の形（原点を枡の底の中心に置く） ----

	/// @brief 枡の外側の半分の幅
	constexpr float OUTER_HALF{ 1.0f };

	/// @brief 板の厚み
	constexpr float WALL_THICKNESS{ 0.12f };

	/// @brief 枡の高さ
	constexpr float MASU_HEIGHT{ 1.4f };

	/// @brief 内側の底の高さ
	constexpr float FLOOR_TOP{ 0.14f };

	/// @brief 内側の半分の幅
	constexpr float INNER_HALF{ OUTER_HALF - WALL_THICKNESS };

	/// @brief 内側に液体が入る高さ
	constexpr float INNER_HEIGHT{ MASU_HEIGHT - FLOOR_TOP };

	/// @brief 際を示す線の太さ
	constexpr float LINE_THICKNESS{ 0.02f };

	/// @brief 注がれる筋の半径
	constexpr float STREAM_RADIUS{ 0.05f };

	/// @brief 注ぎ口の高さ
	constexpr float STREAM_TOP{ 2.6f };

	/// @brief こぼれが広がる範囲の半分の幅
	constexpr float PUDDLE_HALF{ 1.45f };

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

	void PourView3D::draw()
	{
		m_camera.setPerspective(CAMERA_FOV, CAMERA_NEAR, CAMERA_FAR);
		m_camera.lookAt(CAMERA_POSITION, CAMERA_TARGET);

		// 台
		m_renderer3D.drawBox(Vector3{ -6.0f, -0.3f, -6.0f }, Vector3{ 6.0f, 0.0f, 6.0f }, palette::TABLE);

		drawMasu();
		drawLiquid();
		drawLimitLine();
		drawStream();

		// 文字は 3D の手前に出したいので、ここで 3D を吐き出してから描く
		m_renderer3D.flush();
		drawTexts();
	}

	void PourView3D::drawMasu() const
	{
		// 底板
		m_renderer3D.drawBox(Vector3{ -OUTER_HALF, 0.0f, -OUTER_HALF },
		                     Vector3{ OUTER_HALF, FLOOR_TOP, OUTER_HALF }, palette::MASU_FLOOR);

		// 側板は3枚だけ描く。手前の板を省くと中の液面がいつでも見えるため
		// （実物を切り開いて覗いているような見え方になる）
		m_renderer3D.drawBox(Vector3{ -OUTER_HALF, 0.0f, INNER_HALF },
		                     Vector3{ OUTER_HALF, MASU_HEIGHT, OUTER_HALF }, palette::MASU_BACK);
		m_renderer3D.drawBox(Vector3{ INNER_HALF, 0.0f, -OUTER_HALF },
		                     Vector3{ OUTER_HALF, MASU_HEIGHT, OUTER_HALF }, palette::MASU_RIGHT);
		m_renderer3D.drawBox(Vector3{ -OUTER_HALF, 0.0f, -OUTER_HALF },
		                     Vector3{ -INNER_HALF, MASU_HEIGHT, OUTER_HALF }, palette::MASU_LEFT);
	}

	void PourView3D::drawLiquid() const
	{
		if (m_amountRatio <= 0.0f)
			return;

		const float top{ FLOOR_TOP + INNER_HEIGHT * m_amountRatio };

		m_renderer3D.drawBox(Vector3{ -INNER_HALF, FLOOR_TOP, -INNER_HALF },
		                     Vector3{ INNER_HALF, top, INNER_HALF }, palette::LIQUID);

		if (!m_isOverflowed)
			return;

		// こぼれたぶんは枡の外へ広がる
		m_renderer3D.drawBox(Vector3{ -PUDDLE_HALF, 0.0f, -PUDDLE_HALF },
		                     Vector3{ PUDDLE_HALF, 0.03f, PUDDLE_HALF }, palette::LIQUID_SPILLED);
	}

	void PourView3D::drawLimitLine() const
	{
		if (!m_isLimitVisible)
			return;

		const float y{ FLOOR_TOP + INNER_HEIGHT * m_limitRatio };

		// 内側の3面に沿って細い線を回す
		m_renderer3D.drawBox(Vector3{ -INNER_HALF, y, INNER_HALF - LINE_THICKNESS },
		                     Vector3{ INNER_HALF, y + LINE_THICKNESS, INNER_HALF }, palette::LIMIT_LINE);
		m_renderer3D.drawBox(Vector3{ INNER_HALF - LINE_THICKNESS, y, -INNER_HALF },
		                     Vector3{ INNER_HALF, y + LINE_THICKNESS, INNER_HALF }, palette::LIMIT_LINE);
		m_renderer3D.drawBox(Vector3{ -INNER_HALF, y, -INNER_HALF },
		                     Vector3{ -INNER_HALF + LINE_THICKNESS, y + LINE_THICKNESS, INNER_HALF },
		                     palette::LIMIT_LINE);
	}

	void PourView3D::drawStream() const
	{
		if (!m_isPouring)
			return;

		const float surface{ FLOOR_TOP + INNER_HEIGHT * m_amountRatio };
		m_renderer3D.drawCapsule(Vector3{ 0.0f, STREAM_TOP, 0.0f }, Vector3{ 0.0f, surface, 0.0f },
		                         STREAM_RADIUS, palette::STREAM);
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
