#pragma once
#include "core/utility/Vector3.h"
#include "core/utility/Vertex3D.h"
#include <vector>

namespace game::view
{
	/**
	 * @brief 枡の中の液体の面
	 *
	 * 平らな板ではなく高さの場として持つ。着水のたびに波を起こし、その重ね合わせで
	 * 面が上下する。毎フレーム格子状に高さを焼き、そこから法線・色・水中の光の模様を作る。
	 * 波の傾きで映り込む先が変わるため、うねりに沿って明暗が走る
	 */
	class WaterSurface
	{
	  public:
		WaterSurface() = default;

		/**
		 * @brief 溜まっている嵩を伝える
		 * @param ratio 嵩（0.0〜1.0）
		 */
		void setLevel(float ratio) noexcept
		{
			m_levelRatio = ratio;
		}

		/**
		 * @brief 注がれている位置を伝える
		 * @param x 注ぎ口の真下のX座標
		 * @param z 注ぎ口の真下のZ座標
		 * @param isPouring いま注がれているか
		 */
		void setPourPoint(float x, float z, bool isPouring) noexcept
		{
			m_pourX = x;
			m_pourZ = z;
			m_isPouring = isPouring;
		}

		/**
		 * @brief その場所に波を起こす
		 * @param x 波の中心のX座標
		 * @param z 波の中心のZ座標
		 * @param strength 強さ（1.0を標準とする）
		 */
		void disturb(float x, float z, float strength);

		/**
		 * @brief 波を進め、格子の高さを焼き直す
		 * @param deltaTime 進める時間（秒）
		 */
		void advance(float deltaTime);

		/**
		 * @brief その場所の液面の高さを返す
		 * @param x X座標
		 * @param z Z座標
		 * @return 液面の高さ
		 */
		[[nodiscard]] float heightAt(float x, float z) const;

		/**
		 * @brief 波を除いた、平らだとしたときの液面の高さを返す
		 * @return 液面の高さ
		 */
		[[nodiscard]] float getLevelHeight() const noexcept;

		/**
		 * @brief 液体の上面をメッシュに焼く
		 * @details 透かして重ねる前提の頂点を吐く（底が薄く透けて見える）
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 * @param cameraPosition 視点の座標（映り込みの計算に使う）
		 */
		void build(std::vector<core::utility::Vertex3D>& vertices, std::vector<unsigned short>& indices,
		           const core::utility::Vector3& cameraPosition) const;

		/**
		 * @brief 液体の側面をメッシュに焼く
		 *
		 * 深いところは濁って向こうが見えないので、こちらは透かさずに描く。
		 * 上面と分けておかないと、奥の面が手前の面の上に重なって白く濁る
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 */
		void buildSides(std::vector<core::utility::Vertex3D>& vertices,
		                std::vector<unsigned short>& indices) const;

		/**
		 * @brief 枡の底に落ちる光の模様をメッシュに焼く
		 *
		 * 波打つ水面はレンズの役目をして、光を集めたり散らしたりする。
		 * 底に揺れる網目が出るのはそのため。光を足すように重ねて使う
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 */
		void buildCaustics(std::vector<core::utility::Vertex3D>& vertices,
		                   std::vector<unsigned short>& indices) const;

	  private:
		/**
		 * @brief 広がっていく波ひとつ
		 */
		struct Wave
		{
			float originX{ 0.0f };
			float originZ{ 0.0f };
			float age{ 0.0f };
			float strength{ 1.0f };
		};

		/**
		 * @brief 格子の各点の高さを焼き直す
		 */
		void refreshHeights();

		/**
		 * @brief 波と細かなうねりによる高さの変化を返す
		 * @param x X座標
		 * @param z Z座標
		 * @return 平らな液面からの差
		 */
		[[nodiscard]] float displacementAt(float x, float z) const;

		/**
		 * @brief その場所の泡立ちの強さを返す
		 * @param x X座標
		 * @param z Z座標
		 * @return 泡立ち（0.0〜1.0）
		 */
		[[nodiscard]] float foamAt(float x, float z) const;

		/// @brief 経過時間（秒）
		float m_time{ 0.0f };

		/// @brief 溜まっている嵩（0.0〜1.0）
		float m_levelRatio{ 0.0f };

		/// @brief 注ぎが当たっている位置
		float m_pourX{ 0.0f };
		float m_pourZ{ 0.0f };

		/// @brief いま注がれているか
		bool m_isPouring{ false };

		/// @brief 広がっている波
		std::vector<Wave> m_waves{};

		/// @brief 格子の各点の高さ（毎フレーム焼き直す）
		std::vector<float> m_heights{};
	};
} // namespace game::view
