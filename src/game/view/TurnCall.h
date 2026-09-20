#pragma once
#include <string>

namespace core::iface
{
	class IRenderer; // 前方宣言
	class IScreen;   // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 手番が移ったことを画面の中ほどで告げる
	 *
	 * 隅の小さな文字が変わるだけでは、手番が渡ったことに気付けない。
	 * 誰の番になったのかを中央で一度だけ大きく見せ、すぐ引く。
	 *
	 * 見た目だけの動きなので Model も Presenter も、ここが何秒で動くかを知らない
	 */
	class TurnCall
	{
	  public:
		/**
		 * @brief 表示に必要な内容
		 */
		struct Content
		{
			/// @brief 手番の通し番号（変わったときだけ告げる）
			int serial{ 0 };

			/// @brief 手番の側の呼び名
			std::string name{};

			/// @brief 左から出すか（右の側なら false）
			bool isFromLeft{ true };
		};

		TurnCall() = default;

		/**
		 * @brief 告知の動きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param content 表示する内容
		 */
		void update(float deltaTime, const Content& content);

		/**
		 * @brief 告知を描く
		 * @param renderer 2D 描画
		 * @param screen 画面の大きさ
		 * @param fontHandle 名前に使う書体
		 * @param fontSize 書体の大きさ（線を名前から逃がす幅の見当に使う）
		 */
		void draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen, int fontHandle,
		          int fontSize) const;

	  private:
		/// @brief いま出している内容
		Content m_content{};

		/// @brief 告り始めてから経った時間（秒）。出していない間は負にしておく
		float m_time{ -1.0f };
	};
} // namespace game::view
