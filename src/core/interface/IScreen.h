#pragma once
#include "core/utility/Color.h"

namespace core::iface
{
	/**
	 * @brief 画面の情報を提供するインターフェース
	 *
	 * Game 層が DxLib のウィンドウ処理を直接触らずに画面サイズを知るための窓口
	 */
	class IScreen
	{
	  public:
		virtual ~IScreen() = default;

		/**
		 * @brief 画面幅を取得する
		 * @return 画面の幅（ピクセル）
		 */
		[[nodiscard]] virtual int getWidth() const noexcept = 0;

		/**
		 * @brief 画面高さを取得する
		 * @return 画面の高さ（ピクセル）
		 */
		[[nodiscard]] virtual int getHeight() const noexcept = 0;

		/**
		 * @brief 画面クリア時の背景色を設定する
		 * @param color 背景色
		 */
		virtual void setBackgroundColor(const core::utility::Color& color) noexcept = 0;
	};
} // namespace core::iface
