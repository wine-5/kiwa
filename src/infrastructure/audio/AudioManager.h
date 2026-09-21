#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IAudioPlayer.h"
#include <unordered_map>

namespace infrastructure::audio
{
	/**
	 * @brief DxLib を使って音を鳴らす
	 *
	 * 読み込みは ResourceManager の仕事なので、ここは渡された音ハンドルを
	 * 鳴らす・止める・高さと大きさを変える、だけを受け持つ
	 */
	class AudioManager final : public core::iface::IAudioPlayer, private core::base::NonCopyable
	{
	  public:
		AudioManager() = default;

		void playSe(int handle) override;

		void playLoop(int handle) override;

		void stopSound(int handle) override;

		void setPitch(int handle, float rate) override;

		void setVolume(int handle, float volume) override;

		void setMuted(bool isMuted) override;

		[[nodiscard]] bool isMuted() const noexcept override;

	  private:
		/// @brief 音ごとのもとの周波数（音程を変えるときの基準）
		std::unordered_map<int, int> m_baseFrequencies{};

		/// @brief 音ごとに頼まれた大きさ（消したあと戻すときに使う）
		std::unordered_map<int, float> m_volumes{};

		/// @brief いま音を消しているか
		bool m_isMuted{ false };
	};
} // namespace infrastructure::audio
