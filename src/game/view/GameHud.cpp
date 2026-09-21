#include "game/view/GameHud.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Easing.h"
#include "game/constant/Palette.h"
#include "game/view/Plate.h"
#include <algorithm>

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

	/// @brief キーの絵と案内のあいだ
	constexpr float KEY_CAP_GAP{ 14.0f };

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

			renderer.setFont(resources.headingFont);
			renderer.drawTextCentered(Vector2{ centerX, TURN_Y }, m_content.turnLabel,
			                          palette::TEXT_PRIMARY);
		}

		renderer.setFont(resources.bodyFont);

		if (!m_content.message.empty())
			renderer.drawTextCentered(Vector2{ centerX, height - MESSAGE_BOTTOM }, m_content.message,
			                          palette::TEXT_PRIMARY);

		if (m_content.prompt.empty())
			return;

		// 人が打つ番なら、押すキーの絵を案内の左へ添える
		const int keyCap{ m_content.isPlayerOneTurn ? resources.keyCapSpace
			                                        : resources.keyCapEnter };
		const Vector2 capSource{ renderer.getTextureSize(keyCap) };
		const bool showsKey{ !m_content.isNpcTurn && capSource.y > 0.0f };

		const float capWidth{ showsKey ? capSource.x * (KEY_CAP_HEIGHT / capSource.y) : 0.0f };
		const float gap{ showsKey ? KEY_CAP_GAP : 0.0f };
		const float promptWidth{ Plate::widthFor(m_content.prompt, resources.bodySize, 0.0f) };
		const float left{ centerX - (capWidth + gap + promptWidth) * 0.5f };
		const float promptY{ height - PROMPT_BOTTOM };

		if (showsKey)
			renderer.drawTextureStretched(keyCap, Vector2{ left, promptY - KEY_CAP_HEIGHT * 0.5f },
			                              Vector2{ capWidth, KEY_CAP_HEIGHT }, 1.0f);

		renderer.drawTextCentered(Vector2{ left + capWidth + gap + promptWidth * 0.5f, promptY },
		                          m_content.prompt, palette::TEXT_SUB);
	}
} // namespace game::view
