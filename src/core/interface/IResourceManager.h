#pragma once
#include <string>

namespace core::iface
{
	/**
	 * @brief 資源（画像・音・モデル・フォント）の読み込みを担うインターフェース
	 *
	 * 資源をハンドルに変える窓口はここ一つに集める。描く側は「描く」ことだけを受け持つ。
	 * 同じものを何度頼んでも実体は1つに保たれる（2回目以降はキャッシュを返す）ので、
	 * 呼ぶ側が読み込み済みかどうかを気にする必要はない
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
		 * @brief 3D モデルを読み込む
		 * @param path モデルファイルのパス（実行ファイルからの相対パス）
		 * @return モデルハンドル（失敗した場合は -1）
		 */
		virtual int loadModel(const std::string& path) = 0;

		/**
		 * @brief フォントを用意する
		 * @details 同じ組み合わせを何度頼んでも作り直さない
		 * @param family フォント名（あらかじめ使える状態になっていること）
		 * @param size 文字の大きさ
		 * @param thickness 太さ（-1で既定）
		 * @return フォントハンドル（失敗した場合は -1）
		 */
		virtual int loadFont(const std::string& family, int size, int thickness = -1) = 0;

		/**
		 * @brief 効果音を鳴らす（重ねて鳴らせる）
		 * @param handle loadSound が返した音ハンドル
		 */
		virtual void playSe(int handle) = 0;

		/**
		 * @brief 音を繰り返し鳴らす（すでに鳴っていれば鳴らし直さない）
		 * @details 曲や環境音のほか、注いでいる間ずっと鳴る音にも使う
		 * @param handle loadSound が返した音ハンドル
		 */
		virtual void playLoop(int handle) = 0;

		/**
		 * @brief 鳴っている音を止める
		 * @param handle loadSound が返した音ハンドル
		 */
		virtual void stopSound(int handle) = 0;

		/**
		 * @brief 音の高さを変える
		 * @details 注ぐ音は、器に溜まるほど響きが高くなる。それを再生速度で作る
		 * @param handle loadSound が返した音ハンドル
		 * @param rate もとの高さに対する倍率（1.0でそのまま）
		 */
		virtual void setPitch(int handle, float rate) = 0;

		/**
		 * @brief 音の大きさを変える
		 * @param handle loadSound が返した音ハンドル
		 * @param volume 大きさ（0.0〜1.0）
		 */
		virtual void setVolume(int handle, float volume) = 0;

		/**
		 * @brief 読み込んだリソースをすべて解放する
		 */
		virtual void unloadAll() = 0;
	};
} // namespace core::iface
