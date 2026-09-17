#pragma once
#include <string>

namespace core::iface
{
	/**
	 * @brief 画像・音の読み込みを担うインターフェース
	 *
	 * 同じパスを何度読み込んでも実体は1つに保たれる（2回目以降はキャッシュを返す）
	 */
	class IResourceManager
	{
	  public:
		virtual ~IResourceManager() = default;

		/**
		 * @brief 画像を読み込む
		 * @param path 画像ファイルのパス（実行ファイルからの相対パス）
		 * @return 画像ハンドル（失敗した場合は -1）
		 */
		virtual int loadTexture(const std::string& path) = 0;

		/**
		 * @brief 音を読み込む
		 * @param path 音ファイルのパス（実行ファイルからの相対パス）
		 * @return 音ハンドル（失敗した場合は -1）
		 */
		virtual int loadSound(const std::string& path) = 0;

		/**
		 * @brief 効果音を鳴らす（重ねて鳴らせる）
		 * @param handle loadSound が返した音ハンドル
		 */
		virtual void playSe(int handle) = 0;

		/**
		 * @brief BGM を鳴らす（すでに鳴っていれば鳴らし直さない）
		 * @param handle loadSound が返した音ハンドル
		 */
		virtual void playBgm(int handle) = 0;

		/**
		 * @brief 鳴っている BGM を止める
		 * @param handle loadSound が返した音ハンドル
		 */
		virtual void stopBgm(int handle) = 0;

		/**
		 * @brief 読み込んだリソースをすべて解放する
		 */
		virtual void unloadAll() = 0;
	};
} // namespace core::iface
