#include "infrastructure/resource/ResourceManager.h"
#include "DxLib.h"
#include <algorithm>

namespace infrastructure::resource
{
	ResourceManager::~ResourceManager()
	{
		unloadAll();
	}

	int ResourceManager::loadTexture(const std::string& path)
	{
		if (const auto found{ m_textures.find(path) }; found != m_textures.end())
			return found->second;

		const int handle{ LoadGraph(path.c_str()) };
		if (handle < 0)
			return -1; // 読めなかったものは覚えない（あとでファイルを置けば読み直せる）

		m_textures.emplace(path, handle);
		return handle;
	}

	int ResourceManager::loadSound(const std::string& path)
	{
		if (const auto found{ m_sounds.find(path) }; found != m_sounds.end())
			return found->second;

		const int handle{ LoadSoundMem(path.c_str()) };
		if (handle < 0)
			return -1;

		m_sounds.emplace(path, handle);
		return handle;
	}

	int ResourceManager::loadModel(const std::string& path)
	{
		if (const auto found{ m_models.find(path) }; found != m_models.end())
			return found->second;

		const int handle{ MV1LoadModel(path.c_str()) };
		if (handle < 0)
			return -1;

		m_models.emplace(path, handle);
		return handle;
	}

	int ResourceManager::loadFont(const std::string& family, int size, int thickness)
	{
		// 同じ書体でも大きさが違えば別のハンドルが要るので、鍵に混ぜる
		const std::string key{ family + "|" + std::to_string(size) + "|" +
			                   std::to_string(thickness) };

		if (const auto found{ m_fonts.find(key) }; found != m_fonts.end())
			return found->second;

		const int handle{ CreateFontToHandle(family.c_str(), size, thickness) };
		if (handle < 0)
			return -1;

		m_fonts.emplace(key, handle);
		return handle;
	}

	void ResourceManager::playSe(int handle)
	{
		if (handle < 0)
			return;

		PlaySoundMem(handle, DX_PLAYTYPE_BACK, TRUE);
	}

	void ResourceManager::playLoop(int handle)
	{
		if (handle < 0 || CheckSoundMem(handle) == 1)
			return;

		PlaySoundMem(handle, DX_PLAYTYPE_LOOP, TRUE);
	}

	void ResourceManager::stopSound(int handle)
	{
		if (handle < 0)
			return;

		StopSoundMem(handle);
	}

	void ResourceManager::setPitch(int handle, float rate)
	{
		if (handle < 0)
			return;

		// DxLib は周波数で指定する。いまの周波数へ掛け続けると倍率が積もっていくので、
		// もとの周波数を最初に控えておき、毎回そこから計算する
		const auto found{ m_baseFrequencies.find(handle) };
		if (found == m_baseFrequencies.end())
		{
			const int base{ GetFrequencySoundMem(handle) };
			if (base <= 0)
				return;

			m_baseFrequencies.emplace(handle, base);
		}

		SetFrequencySoundMem(static_cast<int>(m_baseFrequencies[handle] * rate), handle);
	}

	void ResourceManager::setVolume(int handle, float volume)
	{
		if (handle < 0)
			return;

		ChangeVolumeSoundMem(static_cast<int>(std::clamp(volume, 0.0f, 1.0f) * 255.0f), handle);
	}

	void ResourceManager::unloadAll()
	{
		for (const auto& [path, handle] : m_textures)
			DeleteGraph(handle);
		m_textures.clear();

		for (const auto& [path, handle] : m_sounds)
			DeleteSoundMem(handle);
		m_sounds.clear();

		for (const auto& [path, handle] : m_models)
			MV1DeleteModel(handle);
		m_models.clear();

		for (const auto& [key, handle] : m_fonts)
			DeleteFontToHandle(handle);
		m_fonts.clear();
	}
} // namespace infrastructure::resource
