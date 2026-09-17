#pragma once
#include "game/view/IPourView.h"

namespace core::iface
{
	class ICamera;     // 前方宣言
	class IRenderer;   // 前方宣言
	class IRenderer3D; // 前方宣言
	class IScreen;     // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 枡と液体を 3D で描く IPourView の実装
	 */
	class PourView3D final : public IPourView
	{
	  public:
		/**
		 * @brief PourView3D のコンストラクタ
		 * @param renderer3D 3D 描画
		 * @param renderer 2D 描画（文字に使う）
		 * @param camera カメラ
		 * @param screen 画面サイズの取得
		 */
		PourView3D(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
		           core::iface::ICamera& camera, core::iface::IScreen& screen);

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

		void draw() override;

	  private:
		/**
		 * @brief 枡そのものを描く
		 */
		void drawMasu() const;

		/**
		 * @brief 枡の中の液体を描く
		 */
		void drawLiquid() const;

		/**
		 * @brief こぼれる際を示す線を描く
		 */
		void drawLimitLine() const;

		/**
		 * @brief 注がれている筋を描く
		 */
		void drawStream() const;

		/**
		 * @brief 文字を描く
		 */
		void drawTexts() const;

		core::iface::IRenderer3D& m_renderer3D;
		core::iface::IRenderer& m_renderer;
		core::iface::ICamera& m_camera;
		core::iface::IScreen& m_screen;

		float m_amountRatio{ 0.0f };
		float m_limitRatio{ 1.0f };
		bool m_isLimitVisible{ true };
		bool m_isPouring{ false };
		bool m_isOverflowed{ false };
		std::string m_message{};
		std::string m_prompt{};
	};
} // namespace game::view
