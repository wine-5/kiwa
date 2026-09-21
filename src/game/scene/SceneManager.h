#pragma once
#include "core/base/NonCopyable.h"
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"
#include "game/scene/SceneType.h"
#include "game/view/SceneTransition.h"
#include <memory>
#include <optional>

namespace game::scene
{
	/**
	 * @brief 現在のシーンを保持し、切り替えを受け持つクラス
	 *
	 * 切り替えの申し込みはその場では実行せず、次の update の頭でまとめて行う。
	 * update や draw の途中で自分自身を破棄してしまうのを防ぐため
	 */
	class SceneManager final : private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief SceneManager のコンストラクタ
		 * @param renderer 2D 描画
		 * @param renderer3D 3D 描画
		 * @param camera カメラ
		 * @param modelRenderer モデルの描画
		 * @param input 入力の取得
		 * @param resource 画像・音の読み込み
		 * @param screen 画面サイズの取得
		 */
		SceneManager(core::iface::IRenderer& renderer, core::iface::IRenderer3D& renderer3D,
		             core::iface::ICamera& camera, core::iface::IModelRenderer& modelRenderer,
		             core::iface::IInputProvider& input, core::iface::IResourceManager& resource,
		             core::iface::IScreen& screen, model::MatchSetup& setup);

		/**
		 * @brief 最初のシーンを開始する
		 * @param sceneType 開始するシーンの種類
		 */
		void start(SceneType sceneType);

		/**
		 * @brief シーンの切り替えを申し込む
		 * @param sceneType 切り替え先のシーンの種類
		 */
		void changeScene(SceneType sceneType);

		/**
		 * @brief 現在のシーンを更新する（申し込まれていた切り替えもここで反映する）
		 * @param deltaTime 進める時間（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief 現在のシーンを描画する
		 */
		void draw();

		/**
		 * @brief 現在のシーンの、仕上げのあとに重ねるものを描く
		 */
		void drawOverlay();

		/**
		 * @brief 現在のシーンの種類を返す
		 * @return 現在のシーンの種類
		 */
		[[nodiscard]] SceneType getCurrentSceneType() const noexcept
		{
			return m_currentSceneType;
		}

	  private:
		/**
		 * @brief 申し込まれていた切り替えを実行する
		 */
		void applyPendingChange();

		SceneContext m_context;

		/// @brief 場面の切り替わりに閉じる襖
		view::SceneTransition m_transition{};
		std::unique_ptr<IScene> m_currentScene{};
		SceneType m_currentSceneType{ SceneType::Title };

		/// @brief 次の update で切り替える先（申し込みが無ければ空）
		std::optional<SceneType> m_pendingSceneType{};
	};
} // namespace game::scene
