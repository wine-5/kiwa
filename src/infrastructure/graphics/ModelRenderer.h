#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IModelRenderer.h"
#include <unordered_map>
#include <vector>

namespace infrastructure::graphics
{
	/**
	 * @brief DxLib のモデル機能を使った IModelRenderer の実装
	 *
	 * 同じファイルを二度読まないよう、読み込んだハンドルを覚えておく
	 */
	class ModelRenderer final : public core::iface::IModelRenderer, private core::base::NonCopyable
	{
	  public:
		ModelRenderer() = default;

		/**
		 * @brief デストラクタ（読み込んだモデルをすべて解放する）
		 */
		~ModelRenderer() override;

		int load(const std::string& path) override;

		void draw(int handle, const core::utility::Vector3& position,
		          const core::utility::Vector3& rotationRadian, float scale) override;

	  private:
		/// @brief パス → モデルハンドル
		std::unordered_map<std::string, int> m_models{};
	};
} // namespace infrastructure::graphics
