#pragma once
#include "game/view/IPourView.h"
#include "core/utility/Vertex3D.h"
#include "game/view/LiquidVisual.h"
#include <vector>

namespace core::iface
{
	class ICamera;          // 前方宣言
	class IRenderer;        // 前方宣言
	class IModelRenderer;   // 前方宣言
	class IRenderer3D;      // 前方宣言
	class IResourceManager; // 前方宣言
	class IScreen;          // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 枡と液体を 3D で描く IPourView の実装
	 *
	 * 枡・台・際の線・文字を受け持ち、液体の見せ方は LiquidVisual に任せる
	 */
	class PourView3D final : public IPourView
	{
	  public:
		/**
		 * @brief PourView3D のコンストラクタ
		 * @param renderer3D 3D 描画
		 * @param renderer 2D 描画（文字に使う）
		 * @param camera カメラ
		 * @param modelRenderer 急須のモデルの描画
		 * @param resource 木目のテクスチャの読み込み
		 * @param screen 画面サイズの取得
		 */
		PourView3D(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
		           core::iface::ICamera& camera, core::iface::IModelRenderer& modelRenderer,
		           core::iface::IResourceManager& resource, core::iface::IScreen& screen);

		void showAmount(float ratio) override
		{
			m_amountRatio = ratio;
		}

		void showLimit(float ratio, bool isVisible) override
		{
			m_limitRatio = ratio;
			m_isLimitVisible = isVisible;
		}

		void showPouring(bool isPouring) override
		{
			m_isPouring = isPouring;
		}

		void showOverflowed(bool isOverflowed) override
		{
			m_isOverflowed = isOverflowed;
		}

		void showMessage(const std::string& message) override
		{
			m_message = message;
		}

		void showPrompt(const std::string& prompt) override
		{
			m_prompt = prompt;
		}

		void advance(float deltaTime) override;

		void draw() override;

		void drawOverlay() override;

	  private:
		/**
		 * @brief 台と影を描く（どちらも動かないので形は作り直さない）
		 */
		void drawScenery() const;

		/**
		 * @brief こぼれて台に広がったぶんを描く
		 */
		void drawPuddle() const;

		/**
		 * @brief こぼれる際を示す線を描く
		 */
		void drawLimitLine() const;

		/**
		 * @brief 画面全体へ被せる仕上げ（周辺減光と粒状感）を描く
		 */
		void drawFilmLook() const;

		/**
		 * @brief 文字を描く
		 */
		void drawTexts() const;

		core::iface::IRenderer3D& m_renderer3D;
		core::iface::IRenderer& m_renderer;
		core::iface::ICamera& m_camera;
		core::iface::IModelRenderer& m_modelRenderer;
		core::iface::IScreen& m_screen;

		/// @brief 台の形
		std::vector<core::utility::Vertex3D> m_tableVertices{};
		std::vector<unsigned short> m_tableIndices{};

		/// @brief 枡が落とす影の形
		std::vector<core::utility::Vertex3D> m_shadowVertices{};
		std::vector<unsigned short> m_shadowIndices{};

		/// @brief 台の木目
		int m_tableTexture{ -1 };

		/// @brief 周辺減光
		int m_vignetteTexture{ -1 };

		/// @brief 粒状感
		int m_grainTexture{ -1 };

		/// @brief 湯呑のモデル
		int m_cupModel{ -1 };

		/// @brief 土瓶のモデル
		int m_potModel{ -1 };

		/// @brief いまの土瓶の傾き（ラジアン）
		float m_potTilt{ 0.0f };

		/// @brief 粒状感をずらすための経過時間（毎フレーム同じ位置だと模様が固まって見える）
		float m_grainTime{ 0.0f };

		/// @brief 液体の見せ方（揺れ・波紋・しぶき・照り）
		LiquidVisual m_liquid{};

		float m_amountRatio{ 0.0f };
		float m_limitRatio{ 1.0f };
		bool m_isLimitVisible{ true };
		bool m_isPouring{ false };
		bool m_isOverflowed{ false };
		std::string m_message{};
		std::string m_prompt{};
	};
} // namespace game::view
