#pragma once
#include "game/view/IPourView.h"
#include "core/utility/Vertex3D.h"
#include "game/view/CardDraw.h"
#include "game/view/CupGeometry.h"
#include "game/view/LiquidVisual.h"
#include "game/view/SpillStreaks.h"
#include <array>
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
		 * @param resource テクスチャ・モデル・書体の読み込み
		 * @param screen 画面サイズの取得
		 */
		PourView3D(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
		           core::iface::ICamera& camera, core::iface::IModelRenderer& modelRenderer,
		           core::iface::IResourceManager& resource, core::iface::IScreen& screen);

		void showAmount(float ratio) override
		{
			m_amountRatio = ratio;
		}

		void showVessel(VesselLook look) override
		{
			if (look == m_vesselLook)
				return;

			// 器が変われば内側の形も変わる。液体はこの形に沿って描かれる
			m_vesselLook = look;
			m_liquid.setShape(cup::shapeOf(look));
			m_spill.setShape(cup::shapeOf(look));
		}

		void showPouring(bool isPouring) override
		{
			m_isPouring = isPouring;
		}

		void showOverflowed(bool isOverflowed) override
		{
			m_isOverflowed = isOverflowed;
		}

		void showCardDraw(bool isActive, bool isRevealed, bool isFirstCard,
		                  const std::string& leftLabel, const std::string& rightLabel) override
		{
			m_cardContent.isActive = isActive;
			m_cardContent.isRevealed = isRevealed;
			m_cardContent.isFirstCard = isFirstCard;
			m_cardContent.leftLabel = leftLabel;
			m_cardContent.rightLabel = rightLabel;
		}

		void showTurn(const std::string& turnLabel) override
		{
			m_turnLabel = turnLabel;
		}

		void showScore(const std::string& scoreLabel) override
		{
			m_scoreLabel = scoreLabel;
		}

		void showMessage(const std::string& message) override
		{
			m_message = message;
		}

		void showPrompt(const std::string& prompt) override
		{
			m_prompt = prompt;
		}

		void update(float deltaTime) override;

		void draw() override;

		void drawOverlay() override;

	  private:
		/**
		 * @brief 台と影を描く（どちらも動かないので形は作り直さない）
		 */
		void drawScenery() const;

		/**
		 * @brief こぼれて台に広がったぶんの形を組む
		 */
		void buildPuddle();

		/**
		 * @brief こぼれて台に広がったぶんを描く
		 */
		void drawPuddle() const;




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

		/// @brief 床（畳）の形
		std::vector<core::utility::Vertex3D> m_floorVertices{};
		std::vector<unsigned short> m_floorIndices{};

		/// @brief こぼれて広がった液体の形
		std::vector<core::utility::Vertex3D> m_puddleVertices{};
		std::vector<unsigned short> m_puddleIndices{};

		/// @brief 器が落とす影の形
		std::vector<core::utility::Vertex3D> m_shadowVertices{};
		std::vector<unsigned short> m_shadowIndices{};

		/// @brief 畳表
		int m_floorTexture{ -1 };

		/// @brief 周辺減光
		int m_vignetteTexture{ -1 };

		/// @brief 粒状感
		int m_grainTexture{ -1 };

		/// @brief 札の裏
		int m_cardBackTexture{ -1 };

		/// @brief 先攻の札
		int m_cardFirstTexture{ -1 };

		/// @brief 後攻の札
		int m_cardSecondTexture{ -1 };

		/// @brief 見出しの書体（毛筆）
		int m_headingFont{ -1 };

		/// @brief 本文の書体
		int m_bodyFont{ -1 };

		/// @brief 器のモデル（VesselLook の順に並べる）
		std::array<int, 4> m_cupModels{ -1, -1, -1, -1 };

		/// @brief 土瓶のモデル
		int m_potModel{ -1 };

		/// @brief いまの土瓶の傾き（ラジアン）
		float m_potTilt{ 0.0f };

		/// @brief 粒状感をずらすための経過時間（毎フレーム同じ位置だと模様が固まって見える）
		float m_grainTime{ 0.0f };

		/// @brief 液体の見せ方（揺れ・波紋・しぶき・照り）
		LiquidVisual m_liquid{};

		/// @brief こぼれて外壁を伝う筋
		SpillStreaks m_spill{};

		/// @brief 広がる途中の染み（毎フレーム作り直す）
		mutable std::vector<core::utility::Vertex3D> m_puddleFrame{};

		/// @brief 染みの広がり具合（0.0〜1.0）
		float m_puddleGrowth{ 0.0f };

		/// @brief 先攻を決める札の見せ方
		CardDraw m_cardDraw{};

		float m_amountRatio{ 0.0f };

		/// @brief いま出ている器
		VesselLook m_vesselLook{ VesselLook::Yunomi };

		bool m_isPouring{ false };
		bool m_isOverflowed{ false };
		/// @brief 札の表示に必要な内容
		CardDraw::Content m_cardContent{};

		std::string m_turnLabel{};
		std::string m_scoreLabel{};
		std::string m_message{};
		std::string m_prompt{};
	};
} // namespace game::view
