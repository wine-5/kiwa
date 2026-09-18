#pragma once
#include "core/utility/Vector3.h"
#include "core/utility/Vertex3D.h"
#include "game/view/PourStream.h"
#include "game/view/WaterSurface.h"
#include <random>
#include <vector>

namespace core::iface
{
	class IRenderer3D; // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 枡の中の液体と、注がれている筋をまとめて見せる
	 *
	 * 液面（WaterSurface）と筋（PourStream）を持ち、着水のたびに波としぶきを起こす。
	 * どれも見た目だけの動きなので、Model も Presenter もここを知らない
	 */
	class LiquidVisual
	{
	  public:
		LiquidVisual() = default;

		/**
		 * @brief 見た目の動きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param isPouring いま注がれているか
		 * @param amountRatio 溜まっている嵩（0.0〜1.0）
		 */
		void advance(float deltaTime, bool isPouring, float amountRatio);

		/**
		 * @brief 注ぎ口の位置を伝える
		 * @param origin 液体が出てくる位置
		 */
		void setPourOrigin(const core::utility::Vector3& origin) noexcept
		{
			m_stream.setOrigin(origin);
		}

		/**
		 * @brief 液体を描く
		 * @param renderer 3D 描画
		 * @param cameraPosition 視点の座標（陰影と照りの計算に使う）
		 */
		void draw(core::iface::IRenderer3D& renderer,
		          const core::utility::Vector3& cameraPosition) const;

		/**
		 * @brief 枡の中心あたりの液面の高さを返す
		 * @return 液面の高さ
		 */
		[[nodiscard]] float getSurfaceHeight() const;

	  private:
		/**
		 * @brief 跳ねたしずく
		 */
		struct Droplet
		{
			core::utility::Vector3 position{};
			core::utility::Vector3 velocity{};
			float radius{ 0.0f };
			float life{ 0.0f };
		};

		/**
		 * @brief 着水のしぶきを飛ばす
		 * @param x 着水点のX座標
		 * @param z 着水点のZ座標
		 * @param surfaceHeight そこの液面の高さ
		 */
		void spawnSplash(float x, float z, float surfaceHeight);

		/**
		 * @brief 範囲内の乱数を返す
		 * @param min 下限
		 * @param max 上限
		 * @return 乱数
		 */
		[[nodiscard]] float randomRange(float min, float max);

		/** @brief しぶきを描く */
		void drawDroplets(core::iface::IRenderer3D& renderer) const;

		WaterSurface m_surface{};
		PourStream m_stream{};

		/// @brief 次に波としぶきを起こすまでの時間（秒）
		float m_splashTimer{ 0.0f };

		/// @brief 前のフレームで注いでいたか（注ぎ終わりに一度だけ波を起こすために見る）
		bool m_wasPouring{ false };

		std::vector<Droplet> m_droplets{};

		/// @brief しぶきのばらつきに使う乱数（見た目だけなので種は固定でよい）
		std::mt19937 m_random{ 20260917 };

		// メッシュは毎フレーム組み直すが、領域は使い回して確保を繰り返さない
		mutable std::vector<core::utility::Vertex3D> m_vertices{};
		mutable std::vector<unsigned short> m_indices{};

		// 底に落ちる光の模様も重ね方が違うので、さらに別のメッシュとして持つ
		mutable std::vector<core::utility::Vertex3D> m_causticVertices{};
		mutable std::vector<unsigned short> m_causticIndices{};
	};
} // namespace game::view
