#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	/**
	 * @brief 3D モデルの描画のインターフェース
	 *
	 * 器や急須のように、形が決まっていて動かないものはモデルとして持つ。
	 * 読み込みは IResourceManager の受け持ちで、ここは描くことだけを行う
	 */
	class IModelRenderer
	{
	  public:
		virtual ~IModelRenderer() = default;

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
