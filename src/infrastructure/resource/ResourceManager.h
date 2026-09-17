#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IResourceManager.h"
#include <string>
#include <unordered_map>

namespace infrastructure::resource
{
	/**
	 * @brief DxLib を使った IResourceManager の実装
	 *
	 * 読み込んだハンドルをパスをキーに覚えておき、同じ画像・音を二重に抱えないようにする
	 */
	class ResourceManager final : public core::iface::IResourceManager, private core::base::NonCopyable
	{
	  public:
		ResourceManager() = default;

		/**
		 * @brief デストラクタ（残っているリソースを解放する）
		 */
		~ResourceManager() override;

		int loadTexture(const std::string& path) override;

		int loadSound(const std::string& path) override;

		void playSe(int handle) override;

		void playBgm(int handle) override;

		void stopBgm(int handle) override;

		void unloadAll() override;

	  private:
		/// @brief パス → 画像ハンドル
		std::unordered_map<std::string, int> m_textures{};

		/// @brief パス → 音ハンドル
		std::unordered_map<std::string, int> m_sounds{};
	};
} // namespace infrastructure::resource
