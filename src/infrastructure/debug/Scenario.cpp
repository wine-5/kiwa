#include "infrastructure/debug/Scenario.h"
#include "infrastructure/debug/FrameCapture.h"
#include "infrastructure/debug/ScriptedInput.h"

namespace infrastructure::debug
{
	void Scenario::install([[maybe_unused]] ScriptedInput& input,
	                       [[maybe_unused]] FrameCapture& capture)
	{
		// 普段は何も仕掛けない。
		//
		// 動きを確かめたいときだけ、ここへ段取りを書く。例えば次のように書くと、
		// 60フレーム目に札を引き、300〜900フレームの間ずっと注ぎ、
		// 三か所で画面を書き出して終わる。
		//
		//   input.addHold(core::input::KeyCode::Space, 60, 66);
		//   input.addHold(core::input::KeyCode::Space, 300, 900);
		//   input.addHold(core::input::KeyCode::Enter, 300, 900);
		//   capture.addShot(500, "C:/temp/a.png");
		//   capture.addShot(920, "C:/temp/b.png");
		//   capture.setQuitFrame(1000);
		//
		// 確かめ終わったらこの中を空に戻すこと
	}

	bool Scenario::startsInGame() noexcept
	{
		// 確認のときだけ true にする
		return false;
	}
} // namespace infrastructure::debug
