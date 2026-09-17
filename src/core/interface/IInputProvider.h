#pragma once
#include "core/input/KeyCode.h"
#include "core/utility/Vector2.h"

namespace core::iface
{
	/**
	 * @brief 入力取得のインターフェース
	 *
	 * Game 層が Infrastructure 層（DxLib）へ直接依存しないための抽象化
	 */
	class IInputProvider
	{
	  public:
		virtual ~IInputProvider() = default;

		/**
		 * @brief フレームの最初に呼び、このフレームで使う入力状態を確定させる
		 *
		 * 確定させておかないと、同じフレーム内の複数箇所で入力を見たときに
		 * 途中で状態が変わり、「押した瞬間」の判定が取りこぼされることがある
		 */
		virtual void captureFrameInput() = 0;

		/**
		 * @brief フレームの最後に呼び、前フレームの入力状態を更新する
		 */
		virtual void updatePreviousState() = 0;

		/**
		 * @brief キーが押されているか判定する
		 * @param keyCode キーコード
		 * @return 押されているならtrue
		 */
		[[nodiscard]] virtual bool isKeyDown(core::input::KeyCode keyCode) const = 0;

		/**
		 * @brief キーが押された瞬間か判定する（押しっぱなしは無視）
		 * @param keyCode キーコード
		 * @return 押された瞬間ならtrue
		 */
		[[nodiscard]] virtual bool isKeyPressed(core::input::KeyCode keyCode) const = 0;

		/**
		 * @brief キーが離された瞬間か判定する
		 * @param keyCode キーコード
		 * @return 離された瞬間ならtrue
		 */
		[[nodiscard]] virtual bool isKeyReleased(core::input::KeyCode keyCode) const = 0;

		/**
		 * @brief マウスカーソルの座標を取得する
		 * @return カーソルの座標
		 */
		[[nodiscard]] virtual core::utility::Vector2 getMousePosition() const = 0;

		/**
		 * @brief マウス左ボタンが押されているか判定する
		 * @return 押されているならtrue
		 */
		[[nodiscard]] virtual bool isMouseLeftDown() const = 0;

		/**
		 * @brief マウス左ボタンが押された瞬間か判定する
		 * @return 押された瞬間ならtrue
		 */
		[[nodiscard]] virtual bool isMouseLeftPressed() const = 0;
	};
} // namespace core::iface
