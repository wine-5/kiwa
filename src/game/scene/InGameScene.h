#pragma once
#include "game/presenter/DuelPresenter.h"
#include "game/scene/IScene.h"
#include "game/scene/SceneContext.h"
#include "game/view/ChoiceList.h"
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
		/**
		 * @brief 休みの場面を開く
		 */
		void openPause();

		/**
		 * @brief 休みの場面を閉じる
		 */
		void closePause();

		/**
		 * @brief 休みの場面の操作を読む
		 * @param deltaTime 進める時間（秒）
		 */
		void updatePause(float deltaTime);

		/**
		 * @brief 休みの場面で選ばれたものを決める
		 */
		void decidePause();

		/**
		 * @brief 休みの場面に並べるものを作る
		 * @return 並べる内容
		 */
		[[nodiscard]] view::ChoiceList::Content buildPauseContent() const;

		/**
		 * @brief 休みへ入るための札を描き、押されたかを返す
		 * @return 押されたなら true
		 */
		[[nodiscard]] bool updatePauseButton();

		SceneContext m_context;

		/// @brief 茶室の間（環境音）
		int m_ambience{ -1 };

		/// @brief 対局中の曲
		int m_bgm{ -1 };

		/// @brief 休みの札
		int m_plateTexture{ -1 };

		/// @brief 見出しの書体
		int m_headingFont{ -1 };

		/// @brief 本文の書体
		int m_bodyFont{ -1 };

		/// @brief 指しているものが変わる音
		int m_cursorSound{ -1 };

		/// @brief 決める音
		int m_decideSound{ -1 };

		/// @brief 戻る音
		int m_backSound{ -1 };

		/// @brief いま休んでいるか
		bool m_isPaused{ false };

		/// @brief 休みの場面で指している番号
		int m_pauseIndex{ 0 };

		/// @brief 休みの場面の並び
		view::ChoiceList m_pauseMenu{};

		// View を先に作ってから Presenter へ渡すため、この順で宣言する
		view::PourView3D m_view;
		presenter::DuelPresenter m_presenter;
	};
} // namespace game::scene
