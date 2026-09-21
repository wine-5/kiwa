#pragma once
#include "core/base/NonCopyable.h"
#include <memory>
#include "core/constant/GameConfig.h"
#include "game/scene/SceneManager.h"
#include "infrastructure/debug/FrameCapture.h"
#include "infrastructure/debug/ScriptedInput.h"
#include "infrastructure/graphics/Camera.h"
#include "infrastructure/graphics/Renderer.h"
#include "infrastructure/graphics/ModelRenderer.h"
#include "infrastructure/graphics/PostEffect.h"
#include "game/constant/Fonts.h"
#include "platform/PlatformFactory.h"
#include "infrastructure/graphics/Renderer3D.h"
#include "infrastructure/graphics/Screen.h"
#include "infrastructure/input/InputProvider.h"
#include "infrastructure/resource/ResourceManager.h"

/**
 * @brief アプリケーション全体を統括する最上位クラス（コンポジションルート）
 *
 * サービスの実体をすべてここが所有し、必要とする相手へ参照で手渡す。
 * どこからでも取り出せる置き場（Singleton など）を作らないことで、
 * 誰が何に依存しているかをコンストラクタの引数だけで追えるようにしている
 */
class Application final : private core::base::NonCopyable
{
  public:
	/**
	 * @brief Application のコンストラクタ（サービスを生成して配線する）
	 * @param screenWidth 画面の幅（ピクセル）
	 * @param screenHeight 画面の高さ（ピクセル）
	 */
	Application(int screenWidth, int screenHeight);

	/**
	 * @brief メインループを実行する（ウィンドウが閉じられるか終了操作まで戻らない）
	 *
	 * 更新は固定タイムステップ（1/60秒）、描画はフレームに1回行う。
	 * ScreenFlip は垂直同期を待つため、描画の回数はモニタのリフレッシュレートに
	 * 引きずられる（120Hz なら毎秒120回）。更新まで同じ回数呼ぶとゲームの進む速さが
	 * モニタ依存で変わってしまうので、実経過時間を貯めておき1/60秒ぶん溜まるごとに
	 * 更新を回す。こうすればモニタが何Hzでもゲーム内時間の進み方が一定になる
	 */
	void run();

  private:
	/**
	 * @brief 全画面と窓を切り替える
	 */
	void toggleFullscreen();

	// 宣言順にそのまま生成されるため、依存される側を先に置く。
	// 同梱のフォントは、どの資源より先に使える状態にしておく必要があり、
	// 外すのは最後（＝いちばん上に置く）でなければならない。
	// どの OS の実体になるかは PlatformFactory が決める
	std::unique_ptr<core::iface::IFontInstaller> m_fontInstaller{
		platform::PlatformFactory::createFontInstaller()
	};

	infrastructure::graphics::Screen m_screen;
	infrastructure::graphics::Renderer m_renderer{};
	infrastructure::graphics::Renderer3D m_renderer3D{};
	infrastructure::graphics::Camera m_camera{};
	infrastructure::graphics::ModelRenderer m_modelRenderer{};
	infrastructure::graphics::PostEffect m_postEffect;
	infrastructure::input::InputProvider m_input{};

	// 動作確認のための仕掛け。普段は何も仕掛かっていないので素通しになる
	infrastructure::debug::ScriptedInput m_scriptedInput{ m_input };
	infrastructure::debug::FrameCapture m_frameCapture{};
	infrastructure::resource::ResourceManager m_resource{};

	/// @brief タイトルで選んだ対局の設定（シーンをまたいで残る）
	game::model::MatchSetup m_matchSetup{};
	game::scene::SceneManager m_sceneManager;

	/// @brief メインループを回し続けるか（Escape か ウィンドウを閉じる操作で false になる）
	bool m_isRunning{ true };

	/// @brief いま全画面かどうか
	bool m_isFullscreen{ core::constant::GameConfig::STARTS_FULLSCREEN };
};
