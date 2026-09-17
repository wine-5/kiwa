#pragma once
#include <string>

namespace game::view
{
	/**
	 * @brief 注ぎ画面の View インターフェース（Passive View）
	 *
	 * View は自分では何も考えない。Presenter から渡されたものをそのまま映すだけで、
	 * Model も入力も知らない。差し替えれば画面を出さずに Presenter を動かせる
	 */
	class IPourView
	{
	  public:
		virtual ~IPourView() = default;

		/**
		 * @brief 枡に入っている嵩を伝える
		 * @param ratio 嵩（0.0〜1.0）
		 */
		virtual void showAmount(float ratio) = 0;

		/**
		 * @brief こぼれる際の位置を伝える
		 * @param ratio 際の嵩（0.0〜1.0）
		 * @param isVisible 見せるならtrue（隠す勝負では false にする）
		 */
		virtual void showLimit(float ratio, bool isVisible) = 0;

		/**
		 * @brief いま注いでいるかを伝える
		 * @param isPouring 注いでいるならtrue
		 */
		virtual void showPouring(bool isPouring) = 0;

		/**
		 * @brief こぼしたかを伝える
		 * @param isOverflowed こぼしているならtrue
		 */
		virtual void showOverflowed(bool isOverflowed) = 0;

		/**
		 * @brief 画面中央に出す文言を伝える
		 * @param message 文言（空なら何も出さない）
		 */
		virtual void showMessage(const std::string& message) = 0;

		/**
		 * @brief 操作の案内を伝える
		 * @param prompt 案内の文言
		 */
		virtual void showPrompt(const std::string& prompt) = 0;

		/**
		 * @brief 伝えられた内容を描く
		 */
		virtual void draw() = 0;
	};
} // namespace game::view
