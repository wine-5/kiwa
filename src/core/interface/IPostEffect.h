#pragma once

namespace core::iface
{
	/**
	 * @brief 描き上がった絵に後から掛ける処理のインターフェース
	 *
	 * 3D を直接画面へ描かず一度別の面へ描き、明るいところを滲ませてから
	 * 画面へ戻す。写真で光が滲むのと同じ効果で、絵が急に本物らしくなる
	 */
	class IPostEffect
	{
	  public:
		virtual ~IPostEffect() = default;

		/**
		 * @brief 3D の描画を受け取る面へ切り替える
		 */
		virtual void begin() = 0;

		/**
		 * @brief 受け取った絵を仕上げて画面へ戻す
		 */
		virtual void end() = 0;
	};
} // namespace core::iface
