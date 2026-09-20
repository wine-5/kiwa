#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IModelRenderer.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLib のモデル機能を使った IModelRenderer の実装
	 */
	class ModelRenderer final : public core::iface::IModelRenderer, private core::base::NonCopyable
	{
	  public:
		ModelRenderer() = default;

		void draw(int handle, const core::utility::Vector3& position,
		          const core::utility::Vector3& rotationRadian, float scale) override;
	};
} // namespace infrastructure::graphics
