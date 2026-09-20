#include "infrastructure/graphics/ModelRenderer.h"
#include "DxLib.h"

namespace infrastructure::graphics
{
	void ModelRenderer::draw(int handle, const core::utility::Vector3& position,
	                         const core::utility::Vector3& rotationRadian, float scale)
	{
		if (handle < 0)
			return;

		// モデルの描画は既定で奥行きを見ない。器のように内と外の面が重なる形は、
		// 有効にしておかないと後から描いた面が手前の面を上書きしてしまう
		MV1SetUseZBuffer(handle, TRUE);
		MV1SetWriteZBuffer(handle, TRUE);

		MV1SetScale(handle, VGet(scale, scale, scale));
		MV1SetRotationXYZ(handle, VGet(rotationRadian.x, rotationRadian.y, rotationRadian.z));
		MV1SetPosition(handle, VGet(position.x, position.y, position.z));

		// モデルは自前で陰影を焼いていないので、描く間だけ光を当てる
		SetUseLighting(TRUE);
		MV1DrawModel(handle);
		SetUseLighting(FALSE);
	}
} // namespace infrastructure::graphics
