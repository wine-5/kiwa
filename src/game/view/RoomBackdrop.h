#pragma once
#include "core/utility/Vertex3D.h"
#include <vector>

namespace core::iface
{
	class ICamera;          // 前方宣言
	class IModelRenderer;   // 前方宣言
	class IRenderer;        // 前方宣言
	class IRenderer3D;      // 前方宣言
	class IResourceManager; // 前方宣言
	class IScreen;          // 前方宣言
} // namespace core::iface

namespace game::view
{
	/**
	 * @brief 対局の外の画面（タイトル・リザルト）へ敷く茶室
	 *
	 * 文字だけを黒い画面に置くと、そこで時間が止まって見える。灯りの下に
	 * 土瓶と器を据え、視点をゆっくり漂わせ、器から湯気を立てて場を生かす。
	 *
	 * 対局の画面とは画角も置き方も違うので、そちらとは別に持つ
	 */
	class RoomBackdrop
	{
	  public:
		/**
		 * @brief RoomBackdrop のコンストラクタ
		 * @param renderer3D 3D 描画
		 * @param renderer 2D 描画
		 * @param camera カメラ
		 * @param modelRenderer モデルの描画
		 * @param resource 画像とモデルの読み込み
		 * @param screen 画面の大きさ
		 */
		RoomBackdrop(core::iface::IRenderer3D& renderer3D, core::iface::IRenderer& renderer,
		             core::iface::ICamera& camera, core::iface::IModelRenderer& modelRenderer,
		             core::iface::IResourceManager& resource, core::iface::IScreen& screen);

		/**
		 * @brief 見た目の動きを進める
		 * @param deltaTime 進める時間（秒）
		 */
		void update(float deltaTime);

		/**
		 * @brief 茶室を描く（3D）
		 */
		void draw() const;

		/**
		 * @brief 画面に被せるもの（周辺減光と粒状感）を描く
		 */
		void drawOverlay() const;

	  private:
		/**
		 * @brief 器から立つ湯気を組んで描く
		 */
		void drawSteam() const;

		core::iface::IRenderer3D& m_renderer3D;
		core::iface::IRenderer& m_renderer;
		core::iface::ICamera& m_camera;
		core::iface::IModelRenderer& m_modelRenderer;
		core::iface::IScreen& m_screen;

		/// @brief 床（畳）
		std::vector<core::utility::Vertex3D> m_floorVertices{};
		std::vector<unsigned short> m_floorIndices{};

		/// @brief 器が落とす影
		std::vector<core::utility::Vertex3D> m_shadowVertices{};
		std::vector<unsigned short> m_shadowIndices{};

		/// @brief 湯気（毎フレーム組み直す）
		mutable std::vector<core::utility::Vertex3D> m_steamVertices{};
		mutable std::vector<unsigned short> m_steamIndices{};

		/// @brief 畳表
		int m_floorTexture{ -1 };

		/// @brief 周辺減光
		int m_vignetteTexture{ -1 };

		/// @brief 粒状感
		int m_grainTexture{ -1 };

		/// @brief 湯呑のモデル
		int m_cupModel{ -1 };

		/// @brief 土瓶のモデル
		int m_potModel{ -1 };

		/// @brief 経過時間（秒）
		float m_time{ 0.0f };
	};
} // namespace game::view
