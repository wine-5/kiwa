#pragma once

namespace game::scene
{
	/**
	 * @brief シーンの基底となるインターフェース
	 */
	class IScene
	{
	  public:
		virtual ~IScene() = default;

		/**
		 * @brief シーンへ入ったときに一度だけ呼ばれる
		 *
		 * 重い読み込みはコンストラクタではなくここで行う。何もしなくてよいシーンのために
		 * 既定実装を置いている
		 */
		virtual void onEnter()
		{
		}

		/**
		 * @brief シーンから出るときに一度だけ呼ばれる
		 */
		virtual void onExit()
		{
		}

		/**
		 * @brief シーンを更新する
		 * @details 固定タイムステップ（1/60秒）で呼ばれる。1フレームに0回や2回のこともある
		 * @param deltaTime 進める時間（秒）
		 */
		virtual void update(float deltaTime) = 0;

		/**
		 * @brief シーンを描画する（フレームに必ず1回呼ばれる）
		 */
		virtual void draw() = 0;
	};
} // namespace game::scene
