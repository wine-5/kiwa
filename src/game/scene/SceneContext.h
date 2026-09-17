#pragma once
#include "game/scene/SceneType.h"
#include <functional>

namespace core::iface
{
	class IInputProvider; // 前方宣言
	class IRenderer;
	class IResourceManager;
	class IScreen;
} // namespace core::iface

namespace game::scene
{
	/**
	 * @brief シーンが必要とする外部機能をまとめて渡すための束
	 *
	 * 各シーンはここに入っているものだけを使って動く。グローバルな取得口を用意せず
	 * 生成時に手渡すことで、そのシーンが何に依存しているかが型の上で分かるようにしている
	 */
	struct SceneContext
	{
		/// @brief 2D 描画
		core::iface::IRenderer& renderer;

		/// @brief 入力の取得
		core::iface::IInputProvider& input;

		/// @brief 画像・音の読み込み
		core::iface::IResourceManager& resource;

		/// @brief 画面サイズの取得
		core::iface::IScreen& screen;

		/// @brief シーンの切り替えを申し込む（実際の入れ替えはフレームの終わりに行われる）
		std::function<void(SceneType)> changeScene;
	};
} // namespace game::scene
