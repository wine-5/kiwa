#pragma once
#include <string>

namespace core::iface
{
	class IRenderer; // 前方宣言
	class IScreen;   // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 対局中に画面へ重ねる表示をまとめたもの
	 *
	 * 手番・勝ち星・案内を、素の文字ではなく和紙の短冊と紋で見せる。
	 * 勝ち星は数字ではなく丸の数で出す（数えるより、あと何本で終わるかが目で分かる）
	 */
	class GameHud
	{
	  public:
		/**
		 * @brief 表示に必要な内容
		 */
		struct Content
		{
			/// @brief 手番の文言（「一の手 の番」など）
			std::string turnLabel{};

			/// @brief 画面下に出す文言
			std::string message{};

			/// @brief 操作の案内
			std::string prompt{};

			/// @brief 一の手が取った数
			int oneWins{ 0 };

			/// @brief 二の手が取った数
			int twoWins{ 0 };

			/// @brief 先取する数
			int targetWins{ 3 };

			/// @brief いまの手番が一の手か
			bool isPlayerOneTurn{ true };

			/// @brief いまの手番を NPC が打っているか
			bool isNpcTurn{ false };
		};

		/**
		 * @brief 描くときに要る画像と書体
		 */
		struct Resources
		{
			/// @brief 手番を載せる短冊
			int turnPlate{ -1 };

			/// @brief 勝ち星を載せる短冊
			int scorePlate{ -1 };

			/// @brief 一の手の紋
			int emblemOne{ -1 };

			/// @brief 二の手の紋
			int emblemTwo{ -1 };

			/// @brief スペースキーの絵
			int keyCapSpace{ -1 };

			/// @brief Enter キーの絵
			int keyCapEnter{ -1 };

			/// @brief 見出しの書体
			int headingFont{ -1 };

			/// @brief 本文の書体
			int bodyFont{ -1 };

			/// @brief 見出しの字の大きさ
			int headingSize{ 48 };

			/// @brief 本文の字の大きさ
			int bodySize{ 26 };
		};

		GameHud() = default;

		/**
		 * @brief 見た目の動きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param content 表示する内容
		 */
		void update(float deltaTime, const Content& content);

		/**
		 * @brief 重ねて描く
		 * @param renderer 2D 描画
		 * @param screen 画面の大きさ
		 * @param resources 使う画像と書体
		 */
		void draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
		          const Resources& resources) const;

	  private:
		/**
		 * @brief 片側の勝ち星を描く
		 * @param renderer 2D 描画
		 * @param resources 使う画像と書体
		 * @param centerX 置く場所（横）
		 * @param emblem 紋の画像
		 * @param wins 取った数
		 * @param isActive いまの手番か
		 */
		void drawScore(core::iface::IRenderer& renderer, const Resources& resources, float centerX,
		               int emblem, int wins, bool isActive) const;

		/// @brief いま出している内容
		Content m_content{};

		/// @brief 手番の短冊の幅（文言が変わると滑らかに伸び縮みする）
		float m_turnWidth{ 0.0f };
	};
} // namespace game::view
