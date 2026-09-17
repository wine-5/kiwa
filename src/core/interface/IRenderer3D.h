#pragma once
#include "core/utility/Color.h"
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 3D 描画のインターフェース
	 *
	 * 枡と液体はどちらも軸に沿った箱で表せるので、まずは箱と柱だけを用意する
	 */
	class IRenderer3D
	{
	  public:
		virtual ~IRenderer3D() = default;

		/**
		 * @brief 軸に沿った直方体を描く
		 * @param minCorner 各軸の小さい側の角の座標
		 * @param maxCorner 各軸の大きい側の角の座標
		 * @param color 色
		 * @param isFilled 塗りつぶすならtrue
		 */
		virtual void drawBox(const core::utility::Vector3& minCorner,
		                     const core::utility::Vector3& maxCorner, const core::utility::Color& color,
		                     bool isFilled = true) = 0;

		/**
		 * @brief 両端が丸い柱を描く（注がれる筋などに使う）
		 * @param from 始点
		 * @param to 終点
		 * @param radius 半径
		 * @param color 色
		 */
		virtual void drawCapsule(const core::utility::Vector3& from, const core::utility::Vector3& to,
		                         float radius, const core::utility::Color& color) = 0;

		/**
		 * @brief 球を描く
		 * @param center 中心の座標
		 * @param radius 半径
		 * @param color 色
		 */
		virtual void drawSphere(const core::utility::Vector3& center, float radius,
		                        const core::utility::Color& color) = 0;

		/**
		 * @brief 溜まっている 3D の描画を吐き出す
		 *
		 * 3D は内部でまとめてから描かれるため、呼ばないまま 2D を描くと
		 * 後から吐き出された 3D が 2D の上に乗ってしまう。
		 * 文字を 3D の手前に出したいときは、その前にこれを呼ぶ
		 */
		virtual void flush() = 0;
	};
} // namespace core::iface
