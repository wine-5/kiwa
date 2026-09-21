#pragma once
#include "game/presenter/DuelPresenter.h"
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"
#include "game/view/PourView3D.h"

namespace game::scene
{
	/**
	 * @brief 二人で注ぎ合う勝負のシーン
	 *
	 * 自分ではルールも描画も持たず、View と Presenter を組み立てて繋ぐだけにしている
	 */
	class InGameScene final : public IScene
	{
	  public:
		/**
		 * @brief InGameScene のコンストラクタ
		 * @param context シーンが使う外部機能の束
		 */
		explicit InGameScene(const SceneContext& context);

		/**
		 * @brief InGameScene のデストラクタ（曲を止める）
		 */
		~InGameScene() override;

		void update(float deltaTime) override;

		void draw() override;

		void drawOverlay() override;

	  private:
		SceneContext m_context;

		/// @brief 茶室の間（環境音）
		int m_ambience{ -1 };

		/// @brief 対局中の曲
		int m_bgm{ -1 };

		// View を先に作ってから Presenter へ渡すため、この順で宣言する
		view::PourView3D m_view;
		presenter::DuelPresenter m_presenter;
	};
} // namespace game::scene
