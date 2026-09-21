#include "game/view/SceneTransition.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/utility/Easing.h"
#include "game/constant/Palette.h"
#include <algorithm>

namespace
{
	using core::utility::Easing;
	using core::utility::Vector2;
	namespace palette = game::constant::palette;

	/// @brief 襖の紙
	constexpr const char* PAPER_TEXTURE_PATH{ "assets/textures/fusuma.png" };

	/// @brief 閉じるのにかかる時間（秒）
	constexpr float CLOSE_TIME{ 0.34f };

	/// @brief 閉じ切って止まっている時間（秒）
	constexpr float HOLD_TIME{ 0.14f };

	/// @brief 開くのにかかる時間（秒）
	constexpr float OPEN_TIME{ 0.42f };

	/// @brief 襖が合わさるところに落ちる影の幅
	constexpr float SEAM_WIDTH{ 26.0f };

	/// @brief 合わせ目に立てる框（かまち）の太さ
	constexpr float STILE_WIDTH{ 7.0f };

	/**
	 * @brief 背景へ溶かして濃さを変える
	 * @param color もとの色
	 * @param strength 濃さ（0.0で見えない、1.0でそのまま）
	 * @return 溶かした色
	 */
	core::utility::Color faded(const core::utility::Color& color, float strength)
	{
		return core::utility::mixed(palette::BACKGROUND, color, std::clamp(strength, 0.0f, 1.0f));
	}
} // namespace

namespace game::view
{
	void SceneTransition::load(core::iface::IResourceManager& resource)
	{
		m_paperTexture = resource.loadTexture(PAPER_TEXTURE_PATH);
	}

	void SceneTransition::begin()
	{
		m_step = Step::Closing;
		m_time = 0.0f;
		m_hasClosedMoment = false;
	}

	bool SceneTransition::isPlaying() const noexcept
	{
		return m_step != Step::Idle;
	}

	void SceneTransition::update(float deltaTime)
	{
		if (m_step == Step::Idle)
			return;

		m_time += deltaTime;

		switch (m_step)
		{
		case Step::Closing:
			if (m_time < CLOSE_TIME)
				break;

			// 閉じ切ったら一拍止める。間を置かないと、閉じた気がしないまま開いてしまう
			m_step = Step::Holding;
			m_time = 0.0f;
			m_hasClosedMoment = true;
			break;

		case Step::Holding:
			if (m_time < HOLD_TIME)
				break;

			m_step = Step::Opening;
			m_time = 0.0f;
			break;

		case Step::Opening:
			if (m_time < OPEN_TIME)
				break;

			m_step = Step::Idle;
			m_time = 0.0f;
			break;

		case Step::Idle:
		default:
			break;
		}
	}

	bool SceneTransition::consumeClosedMoment()
	{
		if (!m_hasClosedMoment)
			return false;

		m_hasClosedMoment = false;
		return true;
	}

	void SceneTransition::draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen) const
	{
		if (m_step == Step::Idle)
			return;

		const float width{ static_cast<float>(screen.getWidth()) };
		const float height{ static_cast<float>(screen.getHeight()) };
		const float half{ width * 0.5f };

		// 閉じるのは勢いよく、開くのはゆっくり。襖は引くより閉めるほうが速い
		float covered{ 1.0f };
		if (m_step == Step::Closing)
			covered = Easing::easeIn(m_time / CLOSE_TIME);
		else if (m_step == Step::Opening)
			covered = 1.0f - Easing::easeOut(m_time / OPEN_TIME);

		const float panel{ half * std::clamp(covered, 0.0f, 1.0f) };
		if (panel <= 0.0f)
			return;

		// 左右の端から一枚ずつ、中央へ向かって引いてくる。
		// 紙は横へ伸ばすので、縦の漉き目はそのまま残る
		renderer.drawTextureStretched(m_paperTexture, Vector2{ 0.0f, 0.0f },
		                              Vector2{ panel, height }, 1.0f);
		renderer.drawTextureStretched(m_paperTexture, Vector2{ width - panel, 0.0f },
		                              Vector2{ panel, height }, 1.0f);

		// 引いてくる側の端（框）と、その前に落ちる影
		renderer.drawRect(Vector2{ panel - STILE_WIDTH, 0.0f }, Vector2{ STILE_WIDTH, height },
		                  faded(palette::TEXT_SUB, 0.3f));
		renderer.drawRect(Vector2{ width - panel, 0.0f }, Vector2{ STILE_WIDTH, height },
		                  faded(palette::TEXT_SUB, 0.3f));

		if (panel >= half)
			return;

		// まだ開いているあいだ、框の前へ細く影を落とす。紙が手前にあると分かる
		renderer.drawRect(Vector2{ panel, 0.0f }, Vector2{ SEAM_WIDTH * 0.35f, height },
		                  faded(palette::BACKGROUND, 0.55f));
		renderer.drawRect(Vector2{ width - panel - SEAM_WIDTH * 0.35f, 0.0f },
		                  Vector2{ SEAM_WIDTH * 0.35f, height },
		                  faded(palette::BACKGROUND, 0.55f));
	}
} // namespace game::view
