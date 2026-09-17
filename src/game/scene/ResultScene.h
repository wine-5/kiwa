#pragma once
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"

namespace game::scene
{
	/**
	 * @brief リザルト画面のシーン
	 *
	 * 表示する内容はゲームの中身が決まってから足す
	 */
	class ResultScene final : public IScene
	{
	  public:
		/**
		 * @brief ResultScene のコンストラクタ
		 * @param context シーンが使う外部機能の束
		 */
		explicit ResultScene(const SceneContext& context);

		void update(float deltaTime) override;

		void draw() override;

	  private:
		SceneContext m_context;
	};
} // namespace game::scene
