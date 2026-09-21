#pragma once

namespace core::iface
{
	class IRenderer;        // 前方宣言
	class IResourceManager; // 前方宣言
	class IScreen;          // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 場面の切り替わりを襖で見せる
	 *
	 * 左右から襖が閉じ、閉じ切ったところで場面を入れ替え、また開く。
	 * 一瞬で入れ替わると、どこへ移ったのか分からないまま次の画面が始まってしまう。
	 *
	 * 閉じ切ったかどうかは外から尋ねられる。場面の入れ替えはそこに合わせる
	 */
	class SceneTransition
	{
	  public:
		SceneTransition() = default;

		/**
		 * @brief 襖の紙を読み込む
		 * @param resource 画像の読み込み
		 */
		void load(core::iface::IResourceManager& resource);

		/**
		 * @brief 閉じ始める
		 */
		void begin();

		/**
		 * @brief 動きを進める
		 * @param deltaTime 進める時間（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief 襖を描く
		 * @param renderer 2D 描画
		 * @param screen 画面の大きさ
		 */
		void draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen) const;

		/**
		 * @brief 閉じ切ったかどうかを、一度だけ返す
		 * @details ここで場面を入れ替える。返したあとは開きに移る
		 * @return 閉じ切った瞬間なら true
		 */
		[[nodiscard]] bool consumeClosedMoment();

		/**
		 * @brief いま動いているかを返す
		 * @return 閉じているか開いている途中なら true
		 */
		[[nodiscard]] bool isPlaying() const noexcept;

	  private:
		/**
		 * @brief どこまで進んだか
		 */
		enum class Step
		{
			Idle,    ///< 何もしていない
			Closing, ///< 閉じている
			Holding, ///< 閉じ切って止まっている
			Opening, ///< 開いている
		};

		/// @brief 襖の紙
		int m_paperTexture{ -1 };

		/// @brief いまの段階
		Step m_step{ Step::Idle };

		/// @brief その段階が始まってから経った時間（秒）
		float m_time{ 0.0f };

		/// @brief 閉じ切ったことをまだ知らせていないか
		bool m_hasClosedMoment{ false };
	};
} // namespace game::view
