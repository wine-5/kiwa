// 自前ヘッダを先に include する（DxLib のマクロと定数名が衝突するのを防ぐ）
#include "Application.h"
#include "core/constant/ScreenConstants.h"
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
    : m_screen{ screenWidth, screenHeight },
      m_sceneManager{ m_renderer, m_renderer3D, m_camera, m_input, m_resource, m_screen }
{
	// 画面を暗く落としておくと、枡と液体だけが浮かび上がる
	m_screen.setBackgroundColor(game::constant::palette::BACKGROUND);
	m_sceneManager.start(game::scene::SceneType::Title);
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
		m_input.captureFrameInput();

		if (m_input.isKeyPressed(core::input::KeyCode::Escape))
			m_isRunning = false;

		accumulatedTime += deltaTime;
		while (accumulatedTime >= core::constant::FIXED_TIME_STEP)
		{
			m_sceneManager.update(core::constant::FIXED_TIME_STEP);
			accumulatedTime -= core::constant::FIXED_TIME_STEP;
		}

		ClearDrawScreen();
		m_sceneManager.draw();
		ScreenFlip();

		m_input.updatePreviousState();
	}
}
