#include "infrastructure/audio/AudioManager.h"
#include "DxLib.h"
#include <algorithm>

namespace infrastructure::audio
{
	void AudioManager::playSe(int handle)
	{
		if (handle < 0 || m_isMuted)
			return;

		PlaySoundMem(handle, DX_PLAYTYPE_BACK, TRUE);
	}

	void AudioManager::playLoop(int handle)
	{
		if (handle < 0 || CheckSoundMem(handle) == 1)
			return;

		PlaySoundMem(handle, DX_PLAYTYPE_LOOP, TRUE);
	}

	void AudioManager::stopSound(int handle)
	{
		if (handle < 0)
			return;

		StopSoundMem(handle);
	}

	void AudioManager::setPitch(int handle, float rate)
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

	void AudioManager::setVolume(int handle, float volume)
	{
		if (handle < 0)
			return;

		// 頼まれた大きさは覚えておく。音を戻すときにここへ戻す
		m_volumes[handle] = std::clamp(volume, 0.0f, 1.0f);
		ChangeVolumeSoundMem(static_cast<int>((m_isMuted ? 0.0f : m_volumes[handle]) * 255.0f),
		                     handle);
	}

	void AudioManager::setMuted(bool isMuted)
	{
		m_isMuted = isMuted;

		// 読み込んである音をすべて黙らせる。止めずに大きさだけ落とすので、
		// 戻したときに曲が頭から鳴り直したりしない
		// 大きさを頼まれたことのある音をすべて黙らせる。止めずに大きさだけ落とすので、
		// 戻したときに曲が頭から鳴り直したりしない
		for (const auto& [handle, volume] : m_volumes)
			ChangeVolumeSoundMem(static_cast<int>((m_isMuted ? 0.0f : volume) * 255.0f), handle);
	}

	bool AudioManager::isMuted() const noexcept
	{
		return m_isMuted;
	}
} // namespace infrastructure::audio
