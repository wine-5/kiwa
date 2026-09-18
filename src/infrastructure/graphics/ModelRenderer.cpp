#include "infrastructure/graphics/ModelRenderer.h"
#include "DxLib.h"

namespace infrastructure::graphics
{
	ModelRenderer::~ModelRenderer()
	{
		for (const auto& [path, handle] : m_models)
			MV1DeleteModel(handle);
	}

	int ModelRenderer::load(const std::string& path)
	{
		if (const auto found{ m_models.find(path) }; found != m_models.end())
			return found->second;

		const int handle{ MV1LoadModel(path.c_str()) };
		if (handle < 0)
			return -1;

		// モデルの描画は既定で奥行きを見ない。器のように内と外の面が重なる形は、
		// 有効にしておかないと後から描いた面が手前の面を上書きしてしまう
		MV1SetUseZBuffer(handle, TRUE);
		MV1SetWriteZBuffer(handle, TRUE);

		m_models.emplace(path, handle);
		return handle;
	}

	void ModelRenderer::draw(int handle, const core::utility::Vector3& position,
	                         const core::utility::Vector3& rotationRadian, float scale)
	{
		if (handle < 0)
			return;

		MV1SetScale(handle, VGet(scale, scale, scale));
		MV1SetRotationXYZ(handle, VGet(rotationRadian.x, rotationRadian.y, rotationRadian.z));
		MV1SetPosition(handle, VGet(position.x, position.y, position.z));

		// モデルは自前で陰影を焼いていないので、描く間だけ光を当てる。
		// 奥行きの判定も既定では切れているため、ここで入れる（切れていると
		// 後から描いたものが器の手前に出てしまう）
		SetUseLighting(TRUE);
		SetUseZBufferFlag(TRUE);
		SetWriteZBufferFlag(TRUE);

		MV1DrawModel(handle);

		SetWriteZBufferFlag(FALSE);
		SetUseZBufferFlag(FALSE);
		SetUseLighting(FALSE);
	}
} // namespace infrastructure::graphics
