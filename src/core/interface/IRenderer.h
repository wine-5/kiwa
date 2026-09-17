#pragma once
#include "core/utility/Color.h"
#include "core/utility/Vector2.h"
#include <string>

namespace core::iface
{
	/**
	 * @brief 2D 描画のインターフェース
	 *
	 * Game 層はここだけを見て描く。DxLib の関数・ハンドル・色の作り方は
	 * Infrastructure 層の実装に閉じ込める
	 */
	class IRenderer
	{
	  public:
		virtual ~IRenderer() = default;

		/**
		 * @brief 矩形を描く
		 * @param position 左上の座標
		 * @param size 幅と高さ
		 * @param color 色
		 * @param isFilled 塗りつぶすならtrue
		 */
		virtual void drawRect(const core::utility::Vector2& position, const core::utility::Vector2& size,
		                      const core::utility::Color& color, bool isFilled = true) = 0;

		/**
		 * @brief 円を描く
		 * @param center 中心の座標
		 * @param radius 半径
		 * @param color 色
		 * @param isFilled 塗りつぶすならtrue
		 */
		virtual void drawCircle(const core::utility::Vector2& center, float radius,
		                        const core::utility::Color& color, bool isFilled = true) = 0;

		/**
		 * @brief 線を描く
		 * @param from 始点
		 * @param to 終点
		 * @param color 色
		 */
		virtual void drawLine(const core::utility::Vector2& from, const core::utility::Vector2& to,
		                      const core::utility::Color& color) = 0;

		/**
		 * @brief 文字列を描く
		 * @param position 左上の座標
		 * @param text 描く文字列（UTF-8）
		 * @param color 色
		 */
		virtual void drawText(const core::utility::Vector2& position, const std::string& text,
		                      const core::utility::Color& color) = 0;

		/**
		 * @brief 文字列を指定座標を中心にして描く
		 * @param center 中心の座標
		 * @param text 描く文字列（UTF-8）
		 * @param color 色
		 */
		virtual void drawTextCentered(const core::utility::Vector2& center, const std::string& text,
		                              const core::utility::Color& color) = 0;

		/**
		 * @brief 画像を描く
		 * @param handle ResourceManager が返した画像ハンドル
		 * @param position 左上の座標
		 */
		virtual void drawTexture(int handle, const core::utility::Vector2& position) = 0;

		/**
		 * @brief 画像を中心・拡大率・回転を指定して描く
		 * @param handle ResourceManager が返した画像ハンドル
		 * @param center 中心の座標
		 * @param scale 拡大率（1.0fで等倍）
		 * @param angleRadian 回転角（ラジアン、時計回り）
		 */
		virtual void drawTextureRotated(int handle, const core::utility::Vector2& center, float scale,
		                                float angleRadian) = 0;
	};
} // namespace core::iface
