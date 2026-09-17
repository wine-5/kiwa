#pragma once
#include "game/scene/SceneType.h"
#include <memory>

namespace game::scene
{
	class IScene;        // 前方宣言
	struct SceneContext; // 前方宣言

	/**
	 * @brief シーンの種類から実体を作るファクトリ
	 *
	 * 「どの種類がどのクラスか」を知っているのはここだけにして、
	 * SceneManager が個々のシーンを include せずに済むようにしている
	 */
	class SceneFactory
	{
	  public:
		/**
		 * @brief シーンを生成する
		 * @param sceneType 生成するシーンの種類
		 * @param context シーンへ渡す外部機能の束
		 * @return 生成したシーン
		 */
		[[nodiscard]] static std::unique_ptr<IScene> create(SceneType sceneType, const SceneContext& context);
	};
} // namespace game::scene
