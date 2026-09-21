#pragma once
#include "core/utility/Vector3.h"

namespace core::iface
{
	class ICamera; // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 対局を写すカメラの寄り引き
	 *
	 * 際が近いほど器へ寄り、注ぎ終わりに縁が近ければ寄ったまま一拍止まる。
	 * 「越えたか」を見せる間を作るのがこの寄りの役目。
	 *
	 * 何を写すかは注ぐ View の仕事ではないので、ここへ切り出してある。
	 * 渡された嵩だけを見て動き、Model も入力も知らない
	 */
	class DuelCamera
	{
	  public:
		/**
		 * @brief カメラが見ている場のようす
		 */
		struct Focus
		{
			/// @brief いまの嵩（0.0〜1.0）
			float amountRatio{ 0.0f };

			/// @brief いま注がれているか
			bool isPouring{ false };

			/// @brief こぼれているか
			bool isOverflowed{ false };

			/// @brief いま出ている器の口の高さ（寄ったときに見る先）
			float rimHeight{ 0.6f };
		};

		/**
		 * @brief DuelCamera のコンストラクタ
		 * @param camera 操るカメラ
		 */
		explicit DuelCamera(core::iface::ICamera& camera);

		/**
		 * @brief 寄り引きを進める
		 * @param deltaTime 進める時間（秒）
		 * @param focus 場のようす
		 */
		void update(float deltaTime, const Focus& focus);

		/**
		 * @brief いまの寄り具合をカメラへ反映する
		 */
		void apply() const;

		/**
		 * @brief いまの視点の座標を返す
		 * @details 液体の照りは視点の向きで変わるので、描く側がこれを見る
		 * @return 視点の座標
		 */
		[[nodiscard]] core::utility::Vector3 getEye() const;

		/**
		 * @brief 器へ寄っている具合を返す
		 * @details 寄っているあいだは周辺減光も濃くしたいので、外から見えるようにする
		 * @return 寄り具合（0.0〜1.0）
		 */
		[[nodiscard]] float getCloseUp() const noexcept
		{
			return m_closeUp;
		}

	  private:
		/**
		 * @brief 寄り切ったときに見る先（器の縁）を返す
		 * @return 見る先
		 */
		[[nodiscard]] core::utility::Vector3 brinkTarget() const;

		core::iface::ICamera& m_camera;

		/// @brief いま見ている場のようす
		Focus m_focus{};

		/// @brief 器へ寄っている具合（0.0〜1.0）
		float m_closeUp{ 0.0f };

		/// @brief 寄ったまま止まっている残り時間（秒）
		float m_lingerTime{ 0.0f };

		/// @brief 1フレーム前に注いでいたか（注ぎ終わりを見つけるために持つ）
		bool m_wasPouring{ false };
	};
} // namespace game::view
