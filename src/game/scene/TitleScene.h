#pragma once
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"

namespace game::scene
{
	/**
	 * @brief タイトル画面のシーン
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

	  private:
		SceneContext m_context;

		/// @brief 「PRESS ENTER」の点滅に使う経過時間（秒）
		float m_elapsedTime{ 0.0f };
	};
} // namespace game::scene
