#include "game/view/GameHud.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Easing.h"
#include "game/constant/Palette.h"
#include "game/view/Plate.h"
#include "core/utility/MathConstants.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Easing;
	using core::utility::Vector2;
	namespace palette = game::constant::palette;

	/// @brief 手番の短冊を置く高さ
	constexpr float TURN_Y{ 76.0f };

	/// @brief 手番の短冊で文字の左右に取る余白
	constexpr float TURN_PADDING{ 54.0f };

	/// @brief 短冊の幅が変わるときの追いつく速さ
	constexpr float WIDTH_RATE{ 9.0f };

	/// @brief 勝ち星を置く高さ
	constexpr float SCORE_Y{ 74.0f };

	/// @brief 勝ち星の短冊を画面の端から離す距離
	constexpr float SCORE_MARGIN{ 138.0f };

	/// @brief 勝ち星の短冊の幅
	constexpr float SCORE_WIDTH{ 200.0f };

	/// @brief 紋の大きさ
	constexpr float EMBLEM_SIZE{ 42.0f };

	/// @brief 勝ち星の丸の半径
	constexpr float PIP_RADIUS{ 7.0f };

	/// @brief 勝ち星の丸の間隔
	constexpr float PIP_STEP{ 26.0f };

	/// @brief 紋と丸のあいだ
	constexpr float PIP_OFFSET{ 30.0f };

	/// @brief 文言を置く高さ（画面の下からの距離）
	constexpr float MESSAGE_BOTTOM{ 140.0f };

	/// @brief 案内を置く高さ（画面の下からの距離）
	constexpr float PROMPT_BOTTOM{ 96.0f };

	/// @brief キーの絵の高さ
	///
	/// 絵は枠が太く、中の字は小さい。小さく置くと字が読めないので大きめに取る
	constexpr float KEY_CAP_HEIGHT{ 48.0f };

	/// @brief キーが押し込まれる深さ
	constexpr float KEY_PRESS_DEPTH{ 7.0f };

	/// @brief キーが押し込まれる速さ
	constexpr float KEY_PRESS_RATE{ 18.0f };

	/// @brief 押していないときに上下する周期（秒）
	constexpr float KEY_BOB_CYCLE{ 1.5f };

	/// @brief 押していないときに上下する幅
	constexpr float KEY_BOB{ 4.0f };

	/// @brief 下向きの山形の大きさ
	constexpr float CHEVRON_WIDTH{ 17.0f };
	constexpr float CHEVRON_HEIGHT{ 11.0f };

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
	void GameHud::update(float deltaTime, const Content& content)
	{
		m_content = content;
		m_time += deltaTime;

		// 押している間はキーが沈んだままになる
		m_press = Easing::approach(m_press, content.isPouring ? 1.0f : 0.0f, KEY_PRESS_RATE,
		                           deltaTime);

		// 文言が変わると短冊の幅も変わる。急に伸び縮みすると目が行ってしまうので追わせる
		const float target{ Plate::widthFor(content.turnLabel, 48, TURN_PADDING) };
		m_turnWidth = m_turnWidth <= 0.0f
		                  ? target
		                  : Easing::approach(m_turnWidth, target, WIDTH_RATE, deltaTime);
	}

	void GameHud::drawScore(core::iface::IRenderer& renderer, const Resources& resources,
	                        float centerX, int emblem, int wins, bool isActive) const
	{
		Plate::draw(renderer, resources.scorePlate, Vector2{ centerX, SCORE_Y }, SCORE_WIDTH,
		            isActive ? 0.95f : 0.6f);

		// 手番の側の紋だけをはっきり出す。隅を見てもどちらの番か分かる
		const float left{ centerX - SCORE_WIDTH * 0.5f + 24.0f };

		renderer.drawTextureStretched(emblem, Vector2{ left, SCORE_Y - EMBLEM_SIZE * 0.5f },
		                              Vector2{ EMBLEM_SIZE, EMBLEM_SIZE }, isActive ? 1.0f : 0.45f);

		// 取った数は丸で出す。数字より、あと何本で終わるかが目で分かる
		for (int i{ 0 }; i < m_content.targetWins; ++i)
		{
			const Vector2 center{ left + EMBLEM_SIZE + PIP_OFFSET + PIP_STEP * i, SCORE_Y };
			const bool isWon{ i < wins };

			if (isWon)
			{
				renderer.drawCircle(center, PIP_RADIUS, faded(palette::TEXT_PRIMARY, 1.0f), true);
				continue;
			}

			// まだ取っていないぶんは輪郭だけ。残りが何本あるかを見せる
			renderer.drawCircle(center, PIP_RADIUS, faded(palette::TEXT_SUB, 0.5f), false);
		}
	}

	void GameHud::draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
	                   const Resources& resources) const
	{
		const float width{ static_cast<float>(screen.getWidth()) };
		const float height{ static_cast<float>(screen.getHeight()) };
		const float centerX{ width * 0.5f };

		// 一の手は左、二の手は右。手番の告知と同じ並びにして迷わせない
		drawScore(renderer, resources, SCORE_MARGIN, resources.emblemOne, m_content.oneWins,
		          m_content.isPlayerOneTurn);
		drawScore(renderer, resources, width - SCORE_MARGIN, resources.emblemTwo, m_content.twoWins,
		          !m_content.isPlayerOneTurn);

		if (!m_content.turnLabel.empty())
		{
			Plate::draw(renderer, resources.turnPlate, Vector2{ centerX, TURN_Y }, m_turnWidth);

			// 紙の上に白い字を置くと沈む。短冊の上は墨で書く
			renderer.setFont(resources.headingFont);
			renderer.drawTextCentered(Vector2{ centerX, TURN_Y }, m_content.turnLabel,
			                          palette::INK);
		}

		renderer.setFont(resources.bodyFont);

		if (!m_content.message.empty())
			renderer.drawTextCentered(Vector2{ centerX, height - MESSAGE_BOTTOM }, m_content.message,
			                          palette::TEXT_PRIMARY);

		if (!m_content.prompt.empty())
			renderer.drawTextCentered(Vector2{ centerX, height - PROMPT_BOTTOM }, m_content.prompt,
			                          palette::TEXT_SUB);

		drawKeyHint(renderer, screen, resources);
	}

	void GameHud::drawKeyHint(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
	                          const Resources& resources) const
	{
		if (!m_content.showsKeyHint)
			return;

		const float centerX{ screen.getWidth() * 0.5f };
		const float centerY{ screen.getHeight() - PROMPT_BOTTOM };

		// 注ぐのは両者ともスペース。どちらの番でも同じ絵を出す
		const Vector2 source{ renderer.getTextureSize(resources.keyCapSpace) };
		if (source.y <= 0.0f)
			return;

		// 押していないあいだは軽く上下し、押すと沈んだままになる
		const float bob{ m_press > 0.5f
			                 ? 0.0f
			                 : std::sin(m_time * core::utility::math::TWO_PI / KEY_BOB_CYCLE) *
			                       KEY_BOB };
		const float sink{ KEY_PRESS_DEPTH * m_press + bob };
		const float capWidth{ source.x * (KEY_CAP_HEIGHT / source.y) };
		const float capTop{ centerY - KEY_CAP_HEIGHT * 0.5f + sink };

		// 押し込む向きを、キーの上から下向きの山形で示す。押している間は引っ込める
		const float chevronStrength{ (1.0f - m_press) * (0.55f + 0.45f * (bob / KEY_BOB + 1.0f) * 0.5f) };
		if (chevronStrength > 0.02f)
		{
			const float chevronY{ capTop - 18.0f + bob };
			const core::utility::Color color{ faded(palette::TEXT_SUB, chevronStrength) };

			// 線は1本だと細いので、少しずらして重ね、筆の太さを出す
			for (int offset{ 0 }; offset < 3; ++offset)
			{
				const float y{ chevronY + offset };

				renderer.drawLine(Vector2{ centerX - CHEVRON_WIDTH, y },
				                  Vector2{ centerX, y + CHEVRON_HEIGHT }, color);
				renderer.drawLine(Vector2{ centerX, y + CHEVRON_HEIGHT },
				                  Vector2{ centerX + CHEVRON_WIDTH, y }, color);
			}
		}

		renderer.drawTextureStretched(resources.keyCapSpace,
		                              Vector2{ centerX - capWidth * 0.5f, capTop },
		                              Vector2{ capWidth, KEY_CAP_HEIGHT }, 1.0f);

		// 沈んだぶんだけ台に近づくので、下の影を縮める
		const float shadowWidth{ capWidth * (0.82f - 0.22f * m_press) };
		const float shadowY{ centerY + KEY_CAP_HEIGHT * 0.5f + 8.0f };

		renderer.drawLine(Vector2{ centerX - shadowWidth * 0.5f, shadowY },
		                  Vector2{ centerX + shadowWidth * 0.5f, shadowY },
		                  faded(palette::TEXT_SUB, 0.25f + 0.25f * m_press));
	}
} // namespace game::view
