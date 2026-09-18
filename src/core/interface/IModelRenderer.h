#pragma once
#include "core/utility/Vector3.h"
#include <string>

namespace core::iface
{
	/**
	 * @brief 3D モデルの読み込みと描画のインターフェース
	 *
	 * 器や急須のように、形が決まっていて動かないものはモデルとして持つ。
	 * 自前で頂点を組むより形を作り込める
	 */
	class IModelRenderer
	{
	  public:
		virtual ~IModelRenderer() = default;

		/**
		 * @brief モデルを読み込む
		 * @param path モデルファイルのパス（実行ファイルからの相対パス）
		 * @return モデルハンドル（失敗した場合は -1）
		 */
		virtual int load(const std::string& path) = 0;

		/**
		 * @brief モデルを描く
		 * @param handle モデルハンドル
		 * @param position 置く位置
		 * @param rotationRadian 各軸まわりの回転（ラジアン）
		 * @param scale 拡大率
		 */
		virtual void draw(int handle, const core::utility::Vector3& position,
		                  const core::utility::Vector3& rotationRadian, float scale) = 0;
	};
} // namespace core::iface
