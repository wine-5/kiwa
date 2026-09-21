// 自前ヘッダを先に include する（DxLib のマクロと定数名が衝突するのを防ぐ）
#include "Application.h"
#include "core/constant/GameConfig.h"
#include "core/constant/ScreenConstants.h"
#include "infrastructure/debug/Scenario.h"
#include "game/constant/Palette.h"
#include "DxLib.h"

namespace
{
	/// @brief 1フレームで進める時間の上限（秒）
	///
	/// ウィンドウを掴んで止めた後などに巨大な経過時間が来ると、取り戻そうとして
	/// 更新が何十回も連続し、さらに重くなって止まってしまう。ここで頭を打たせる
	constexpr float MAX_DELTA_TIME{ 0.25f };

	/// @brief 高精度カウンタの1秒あたりの値（DxLib のカウンタはマイクロ秒単位）
	constexpr float MICROSECONDS_PER_SECOND{ 1000000.0f };
} // namespace

Application::Application(int screenWidth, int screenHeight)
    : m_screen{ screenWidth, screenHeight }, m_postEffect{ screenWidth, screenHeight },
      m_sceneManager{ m_renderer,      m_renderer3D, m_camera,  m_modelRenderer,
	                  m_scriptedInput, m_resource,   m_screen,  m_matchSetup,
	                  [this] { m_isRunning = false; } }
{
	// 画面を暗く落としておくと、枡と液体だけが浮かび上がる
	m_screen.setBackgroundColor(game::constant::palette::BACKGROUND);
	// 動作確認の段取りは Scenario に書く（ここには配線だけを置く）。
	// 製品版では丸ごと消えるよう if constexpr で分ける
	if constexpr (core::constant::GameConfig::USES_SCENARIO)
	{
		infrastructure::debug::Scenario::install(m_scriptedInput, m_frameCapture);

		if (infrastructure::debug::Scenario::startsInGame())
		{
			m_sceneManager.start(game::scene::SceneType::InGame);
			return;
		}
	}

	m_sceneManager.start(game::scene::SceneType::Title);
}

void Application::toggleFullscreen()
{
	// DxLib は「窓かどうか」しか返さないので、いまの状態を反転させて渡す
	m_isFullscreen = !m_isFullscreen;
	ChangeWindowMode(m_isFullscreen ? FALSE : TRUE);
	SetDrawScreen(DX_SCREEN_BACK);
}

void Application::run()
{
	LONGLONG previousCount{ GetNowHiPerformanceCount() };
	float accumulatedTime{ 0.0f }; // まだ更新に使っていない経過時間（秒）

	while (ProcessMessage() == 0 && m_isRunning)
	{
		const LONGLONG currentCount{ GetNowHiPerformanceCount() };
		float deltaTime{ static_cast<float>(currentCount - previousCount) / MICROSECONDS_PER_SECOND };
		previousCount = currentCount;

		if (deltaTime > MAX_DELTA_TIME)
			deltaTime = MAX_DELTA_TIME;

		// このフレームで使う入力をここで確定させる（以降は同じ状態を見続ける）
		m_scriptedInput.captureFrameInput();

		if (m_scriptedInput.isKeyPressed(core::input::KeyCode::Escape))
			m_isRunning = false;

		if (m_scriptedInput.consumeKeyPress(core::input::KeyCode::F1))
			toggleFullscreen();

		accumulatedTime += deltaTime;
		while (accumulatedTime >= core::constant::FIXED_TIME_STEP)
		{
			m_sceneManager.update(core::constant::FIXED_TIME_STEP);
			accumulatedTime -= core::constant::FIXED_TIME_STEP;
		}

		// 3D は一度別の面へ描き、光を滲ませてから画面へ戻す。
		// 文字や画面全体に被せるものは、滲ませたあとに重ねる
		m_postEffect.begin();
		m_sceneManager.draw();
		m_postEffect.end();
		m_sceneManager.drawOverlay();

		if constexpr (core::constant::GameConfig::ALLOWS_FRAME_CAPTURE)
		{
			m_frameCapture.endFrame(m_screen.getWidth(), m_screen.getHeight());
			if (m_frameCapture.isFinished())
				m_isRunning = false;
		}

		ScreenFlip();

		m_scriptedInput.updatePreviousState();
	}
}
