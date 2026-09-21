#pragma once
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"
#include "game/view/ChoiceList.h"

namespace game::scene
{
	/**
	 * @brief タイトル画面のシーン
	 *
	 * 誰と打つかをここで決める。二人で打つか、一人で打って二の手を NPC に任せるか。
	 * NPC に任せるときは、続けてその強さ（誰に打たせるか）を選ぶ
	 */
	class TitleScene final : public IScene
	{
	  public:
		/**
		 * @brief TitleScene のコンストラクタ
		 * @param context シーンが使う外部機能の束
		 */
		explicit TitleScene(const SceneContext& context);

		void update(float deltaTime) override;

		void draw() override;

		void drawOverlay() override;

	  private:
		/**
		 * @brief いま何を選ばせているか
		 */
		enum class Step
		{
			Mode,     ///< 二人で打つか、一人で打つか
			Strength, ///< 二の手を誰に打たせるか
		};

		/**
		 * @brief いまの段で並べるものを作る
		 * @return 並べる内容
		 */
		[[nodiscard]] view::ChoiceList::Content buildContent() const;

		/**
		 * @brief 選ばれたものを決める
		 */
		void decide();

		SceneContext m_context;

		/// @brief いま何を選ばせているか
		Step m_step{ Step::Mode };

		/// @brief 指している番号
		int m_index{ 0 };

		/// @brief 選ばせる並び
		view::ChoiceList m_choices{};

		/// @brief 題字の書体（毛筆・大きめ）
		int m_titleFont{ -1 };

		/// @brief 見出しの書体（毛筆）
		int m_headingFont{ -1 };

		/// @brief 本文の書体
		int m_bodyFont{ -1 };

		/// @brief 経過時間（案内の点滅に使う）
		float m_elapsedTime{ 0.0f };
	};
} // namespace game::scene
