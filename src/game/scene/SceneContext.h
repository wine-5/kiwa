#pragma once
#include "game/model/MatchSetup.h"
#include "game/scene/SceneType.h"
#include <functional>

namespace core::iface
{
	class ICamera;        // 前方宣言
	class IAudioPlayer;    // 前方宣言
	class IInputProvider;  // 前方宣言
	class IModelRenderer;  // 前方宣言
	class IRenderer;      // 前方宣言
	class IRenderer3D;    // 前方宣言
	class IResourceManager; // 前方宣言
	class IScreen;        // 前方宣言
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

		/// @brief 3D 描画
		core::iface::IRenderer3D& renderer3D;

		/// @brief カメラ
		core::iface::ICamera& camera;

		/// @brief モデルの描画
		core::iface::IModelRenderer& modelRenderer;

		/// @brief 入力の取得
		core::iface::IInputProvider& input;

		/// @brief 画像・音の読み込み
		core::iface::IResourceManager& resource;

		/// @brief 音を鳴らす
		core::iface::IAudioPlayer& audio;

		/// @brief 画面サイズの取得
		core::iface::IScreen& screen;

		/// @brief タイトルで選んだ対局の設定（シーンをまたいで残る）
		model::MatchSetup& setup;

		/// @brief シーンの切り替えを申し込む（実際の入れ替えはフレームの終わりに行われる）
		std::function<void(SceneType)> changeScene;

		/// @brief アプリを終わらせる
		std::function<void()> quitGame;
	};
} // namespace game::scene
