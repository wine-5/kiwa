#pragma once
#include "core/utility/BlendMode.h"
#include "core/utility/Color.h"
#include "core/utility/Vector3.h"
#include "core/utility/Vertex3D.h"
#include <span>

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
		 * @brief 三角形の集まりを描く
		 *
		 * 頂点ごとに色を持てるので、陰影や照りを呼ぶ側で計算して流し込める。
		 * 波打つ水面や流れる筋のように、出来合いの形では表せないものに使う
		 * @param vertices 頂点の並び
		 * @param indices 三角形を組む順番（3つで1枚）
		 */
		virtual void drawTriangles(std::span<const core::utility::Vertex3D> vertices,
		                           std::span<const unsigned short> indices) = 0;

		/**
		 * @brief 以降の drawTriangles に貼るテクスチャを指定する
		 * @param textureHandle テクスチャハンドル（-1 で貼らない）
		 */
		virtual void setTexture(int textureHandle) = 0;

		/**
		 * @brief 裏側を向いた面を描かないかどうかを切り替える
		 *
		 * 自前で組んだメッシュは裏表を取り違えやすい。切っておけば
		 * どちら向きでも必ず描かれる
		 * @param isEnabled 裏面を省くならtrue
		 */
		virtual void setBackCulling(bool isEnabled) = 0;

		/**
		 * @brief 以降の描画の重ね方を変える
		 *
		 * 透かしたり光を足したりしている間は奥行きの書き込みを止めるため、
		 * 使い終えたら必ず None へ戻すこと（戻さないと後続の描画の前後関係が壊れる）
		 * @param mode 重ね方
		 * @param strength 濃さ（0.0〜1.0）。None のときは無視される
		 */
		virtual void setBlend(core::utility::BlendMode mode, float strength) = 0;

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
