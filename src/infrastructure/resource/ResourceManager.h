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
	 * 読み込んだハンドルを鍵（パスやフォントの組み合わせ）で覚えておき、
	 * 同じものを二重に抱えないようにする。解放もここが一括で面倒を見る
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

		int loadModel(const std::string& path) override;

		int loadFont(const std::string& family, int size, int thickness = -1) override;

		void playSe(int handle) override;

		void playBgm(int handle) override;

		void stopBgm(int handle) override;

		void unloadAll() override;

	  private:
		/// @brief パス → 画像ハンドル
		std::unordered_map<std::string, int> m_textures{};

		/// @brief パス → 音ハンドル
		std::unordered_map<std::string, int> m_sounds{};

		/// @brief パス → モデルハンドル
		std::unordered_map<std::string, int> m_models{};

		/// @brief フォントの組み合わせ → フォントハンドル
		std::unordered_map<std::string, int> m_fonts{};
	};
} // namespace infrastructure::resource
