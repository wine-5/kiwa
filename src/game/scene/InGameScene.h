#pragma once
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"

namespace game::scene
{
	/**
	 * @brief ゲーム本編のシーン
	 *
	 * 中身はこれから決めるため、いまは基盤の動作確認（固定ステップの更新と入力）だけを持つ
	 */
	class InGameScene final : public IScene
	{
	  public:
		/**
		 * @brief InGameScene のコンストラクタ
		 * @param context シーンが使う外部機能の束
		 */
		explicit InGameScene(const SceneContext& context);

		void update(float deltaTime) override;

		void draw() override;

	  private:
		SceneContext m_context;

		/// @brief 動作確認用に動かしている四角の位置
		core::utility::Vector2 m_position{ 0.0f, 0.0f };
	};
} // namespace game::scene
