#pragma once
#include "core/utility/Vector2.h"
#include <string>

namespace core::iface
{
	class IRenderer; // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 和紙の短冊を、端の飾りを保ったまま好きな幅で描く
	 *
	 * 短冊の絵は端がちぎれた形をしている。全体を引き伸ばすと、その端まで一緒に
	 * 間延びして紙に見えなくなる。そこで左右の端はそのままの大きさで置き、
	 * 中ほどだけを伸ばす。
	 *
	 * 持つ状態がないので実体は作らず、すべて静的な関数にする
	 */
	class Plate
	{
	  public:
		Plate() = delete;

		/**
		 * @brief 短冊を描く
		 * @param renderer 2D 描画
		 * @param texture 短冊の画像
		 * @param center 置く場所（短冊の中心）
		 * @param width 描く幅（絵より狭いときは端だけを詰める）
		 * @param alpha 濃さ（0.0〜1.0）
		 * @param scale 高さの拡大率（1.0 で絵のままの高さ）
		 */
		static void draw(core::iface::IRenderer& renderer, int texture,
		                 const core::utility::Vector2& center, float width, float alpha = 1.0f,
		                 float scale = 1.0f);

		/**
		 * @brief 文字を収めるのに要る短冊の幅を返す
		 * @param text 載せる文字列（UTF-8）
		 * @param fontSize 文字の大きさ
		 * @param padding 文字の左右に取る余白
		 * @return 短冊の幅
		 */
		[[nodiscard]] static float widthFor(const std::string& text, int fontSize, float padding);

	  private:
		/// @brief 端の飾りとして扱う幅の割合（絵の幅に対して）
		static constexpr float CAP_RATIO{ 0.26f };
	};
} // namespace game::view
