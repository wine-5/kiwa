#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IInputProvider.h"
#include <array>
#include <vector>

namespace infrastructure::debug
{
	/**
	 * @brief 実際の入力に、決められたキー操作を重ねて流す仕掛け
	 *
	 * 動作確認のために「何フレーム目に何を押す」を台本として与える。
	 * 本物の入力提供者を包む形なので、台本が空なら何も変わらない。
	 * 確認用の処理を Application や InputProvider へ書き足さずに済ませるためにある
	 */
	class ScriptedInput final : public core::iface::IInputProvider, private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief ScriptedInput のコンストラクタ
		 * @param inner 本物の入力提供者
		 */
		explicit ScriptedInput(core::iface::IInputProvider& inner) : m_inner{ inner }
		{
		}

		/**
		 * @brief 台本を一行足す（この区間、そのキーを押しっぱなしにする）
		 * @param keyCode 押すキー
		 * @param fromFrame 押し始めるフレーム
		 * @param toFrame 離すフレーム（この値は含まない）
		 */
		void addHold(core::input::KeyCode keyCode, int fromFrame, int toFrame);

		/**
		 * @brief 台本があるかを返す
		 * @return あるならtrue
		 */
		[[nodiscard]] bool hasScript() const noexcept
		{
			return !m_steps.empty();
		}

		// ---- IInputProvider ----

		void captureFrameInput() override;

		void updatePreviousState() override;

		[[nodiscard]] bool isKeyDown(core::input::KeyCode keyCode) const override;

		[[nodiscard]] bool isKeyPressed(core::input::KeyCode keyCode) const override;

		bool consumeKeyPress(core::input::KeyCode keyCode) override;

		void clearPendingPresses() override;

		[[nodiscard]] bool isKeyReleased(core::input::KeyCode keyCode) const override;

		[[nodiscard]] core::utility::Vector2 getMousePosition() const override;

		[[nodiscard]] bool isMouseMoved() const override;

		[[nodiscard]] bool isMouseLeftDown() const override;

		[[nodiscard]] bool isMouseLeftPressed() const override;

	  private:
		/**
		 * @brief 台本の一行
		 */
		struct Step
		{
			core::input::KeyCode keyCode{};
			int fromFrame{ 0 };
			int toFrame{ 0 };
		};

		/// @brief 扱うキーの数
		static constexpr std::size_t KEY_COUNT{ static_cast<std::size_t>(core::input::KeyCode::Count) };

		core::iface::IInputProvider& m_inner;

		std::vector<Step> m_steps{};

		/// @brief いま何フレーム目か
		int m_frame{ 0 };

		/// @brief 台本で押されていることになっているキー
		std::array<bool, KEY_COUNT> m_scriptedKeys{};

		/// @brief 前フレームに台本で押されていたキー
		std::array<bool, KEY_COUNT> m_previousScriptedKeys{};

		/// @brief 台本ぶんの、まだ消費されていない「押された瞬間」
		std::array<bool, KEY_COUNT> m_pendingPresses{};
	};
} // namespace infrastructure::debug
