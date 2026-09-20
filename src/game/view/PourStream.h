#pragma once
#include "core/utility/Vector3.h"
#include "core/utility/Vertex3D.h"
#include <vector>

namespace game::view
{
	/**
	 * @brief 注ぎ口から落ちてくる液体の筋
	 *
	 * 一本の棒ではなく筒のメッシュとして組む。落ちるほど細くなり、太さのむらが
	 * 下へ流れていく。縁を明るくすることで、透けた液体らしい見え方にする
	 */
	class PourStream
	{
	  public:
		PourStream() = default;

		/**
		 * @brief 注ぎ口の位置を伝える
		 * @param origin 液体が出てくる位置
		 */
		void setOrigin(const core::utility::Vector3& origin) noexcept
		{
			m_origin = origin;
		}

		/**
		 * @brief 筋の状態を進める
		 * @param deltaTime 進める時間（秒）
		 * @param isPouring いま注がれているか
		 */
		void update(float deltaTime, bool isPouring);

		/**
		 * @brief 筋が見えているか
		 * @return 見えているならtrue
		 */
		[[nodiscard]] bool isVisible() const noexcept;

		/**
		 * @brief 筋の勢い（0.0〜1.0）
		 * @details 注ぎ始めは太くなっていき、やめると細くなって消える
		 * @return 勢い
		 */
		[[nodiscard]] float getFlow() const noexcept
		{
			return m_flow;
		}

		/**
		 * @brief 筋をメッシュに焼く
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 * @param surfaceHeight 液面の高さ（ここまで落ちる）
		 * @param cameraPosition 視点の座標（照りの計算に使う）
		 */
		void build(std::vector<core::utility::Vertex3D>& vertices, std::vector<unsigned short>& indices,
		           float surfaceHeight, const core::utility::Vector3& cameraPosition) const;

		/**
		 * @brief 筋が液面へ当たっている位置のX座標を返す
		 * @param surfaceHeight 液面の高さ
		 * @return X座標
		 */
		[[nodiscard]] float getImpactX(float surfaceHeight) const;

		/**
		 * @brief 筋が液面へ当たっている位置のZ座標を返す
		 * @param surfaceHeight 液面の高さ
		 * @return Z座標
		 */
		[[nodiscard]] float getImpactZ(float surfaceHeight) const;

	  private:
		/**
		 * @brief 筋の中心の横ぶれを返す
		 * @param t 上から下への位置（0.0〜1.0）
		 * @return 中心の位置
		 */
		[[nodiscard]] core::utility::Vector3 centerAt(float t, float surfaceHeight) const;

		/**
		 * @brief 筋の太さを返す
		 * @param t 上から下への位置（0.0〜1.0）
		 * @return 半径
		 */
		[[nodiscard]] float radiusAt(float t) const;

		/// @brief 経過時間（秒）
		float m_time{ 0.0f };

		/// @brief 注がれているか
		bool m_isPouring{ false };

		/// @brief 勢い（0.0〜1.0）。急に現れたり消えたりしないよう補間する
		float m_flow{ 0.0f };

		/// @brief 液体が出てくる位置
		core::utility::Vector3 m_origin{ 0.0f, 2.2f, 0.0f };
	};
} // namespace game::view
