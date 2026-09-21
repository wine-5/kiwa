#pragma once
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"
#include "game/view/RoomBackdrop.h"
#include <string>

namespace game::scene
{
	/**
	 * @brief リザルト画面のシーン
	 *
	 * 掛軸が上から下り、勝者の名が浮かび、最後に勝の落款が押される。
	 * 誰が勝ったかは対局から `MatchSetup` に書き戻されたものを読む
	 */
	class ResultScene final : public IScene
	{
	  public:
		/**
		 * @brief ResultScene のコンストラクタ
		 * @param context シーンが使う外部機能の束
		 */
		explicit ResultScene(const SceneContext& context);

		/**
		 * @brief ResultScene のデストラクタ（曲を止める）
		 */
		~ResultScene() override;

		void update(float deltaTime) override;

		void draw() override;

		void drawOverlay() override;

	  private:
		/**
		 * @brief 勝った側の呼び名を作る
		 * @return 呼び名
		 */
		[[nodiscard]] std::string buildWinnerName() const;

		SceneContext m_context;

		/// @brief 後ろに敷く茶室
		view::RoomBackdrop m_backdrop;

		/// @brief 掛軸
		int m_scrollTexture{ -1 };

		/// @brief 勝の落款
		int m_sealTexture{ -1 };

		/// @brief 勝者の名に使う書体（毛筆・大きめ）
		int m_winnerFont{ -1 };

		/// @brief 本文の書体
		int m_bodyFont{ -1 };

		/// @brief 勝ちを告げる音
		int m_winSound{ -1 };

		/// @brief 掛軸が下りる音
		int m_scrollSound{ -1 };

		/// @brief 落款が押される音
		int m_sealSound{ -1 };

		/// @brief 対局の決着を締める音
		int m_matchWinSound{ -1 };

		/// @brief リザルトの曲
		int m_bgm{ -1 };

		/// @brief 場面が始まってから経った時間（秒）
		float m_elapsedTime{ 0.0f };
	};
} // namespace game::scene
