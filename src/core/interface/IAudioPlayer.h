#pragma once

namespace core::iface
{
	/**
	 * @brief 音を鳴らす
	 *
	 * 読み込みと持ち回りは IResourceManager の仕事で、ここは鳴らすことだけを受け持つ。
	 * 渡すのは読み込み済みの音ハンドル
	 */
	class IAudioPlayer
	{
	  public:
		virtual ~IAudioPlayer() = default;

		/**
		 * @brief 効果音を鳴らす（重ねて鳴らせる）
		 * @param handle 音ハンドル
		 */
		virtual void playSe(int handle) = 0;

		/**
		 * @brief 音を繰り返し鳴らす（すでに鳴っていれば鳴らし直さない）
		 * @details 曲や環境音のほか、注いでいる間ずっと鳴る音にも使う
		 * @param handle 音ハンドル
		 */
		virtual void playLoop(int handle) = 0;

		/**
		 * @brief 鳴っている音を止める
		 * @param handle 音ハンドル
		 */
		virtual void stopSound(int handle) = 0;

		/**
		 * @brief 音の高さを変える
		 * @details 注ぐ音は、器に溜まるほど響きが高くなる。それを再生速度で作る
		 * @param handle 音ハンドル
		 * @param rate もとの高さに対する倍率（1.0でそのまま）
		 */
		virtual void setPitch(int handle, float rate) = 0;

		/**
		 * @brief 音の大きさを変える
		 * @param handle 音ハンドル
		 * @param volume 大きさ（0.0〜1.0）
		 */
		virtual void setVolume(int handle, float volume) = 0;

		/**
		 * @brief 音を消すか出すかを切り替える
		 * @details 消している間も曲は流れ続ける（戻したところから聞こえる）
		 * @param isMuted 消すなら true
		 */
		virtual void setMuted(bool isMuted) = 0;

		/**
		 * @brief いま音を消しているかを返す
		 * @return 消しているなら true
		 */
		[[nodiscard]] virtual bool isMuted() const noexcept = 0;
	};
} // namespace core::iface
