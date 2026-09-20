#pragma once
#include <array>

namespace game::model
{
	/**
	 * @brief 注ぐ器の種類
	 *
	 * 器ごとに入る量が違う。毎局ちがう器が出ることで、
	 * 「あと何回注げるか」の見当が毎回つけ直しになる
	 */
	enum class VesselType
	{
		Guinomi,   ///< ぐい呑。小さく浅い
		Yunomi,    ///< 湯呑。基準になる大きさ
		Sobachoko, ///< そば猪口。細く深い
		Chawan,    ///< 茶碗。広く大きい
	};

	/**
	 * @brief 器の諸元
	 */
	struct Vessel
	{
		/// @brief 器の種類
		VesselType type{ VesselType::Yunomi };

		/// @brief 器の名前（画面に出す）
		const char* name{ "湯呑" };

		/// @brief 口まで満たすのに要る量（湯呑を 1.0 とした倍率）
		///
		/// tools/make_models.py が断面から求めた体積の比をそのまま写している。
		/// 注ぐ速さは器によらず一定なので、この値がそのまま
		/// 「口まで満たすのにかかる時間」の倍率になる
		float capacity{ 1.0f };
	};

	/// @brief 出てくる器の一覧
	inline constexpr std::array<Vessel, 4> VESSELS{ {
		{ VesselType::Guinomi, "ぐい呑", 0.25f },
		{ VesselType::Yunomi, "湯呑", 1.00f },
		{ VesselType::Sobachoko, "そば猪口", 0.75f },
		{ VesselType::Chawan, "茶碗", 1.83f },
	} };

	/**
	 * @brief 種類から諸元を引く
	 * @param type 器の種類
	 * @return その器の諸元
	 */
	[[nodiscard]] inline constexpr const Vessel& vesselOf(VesselType type) noexcept
	{
		for (const Vessel& vessel : VESSELS)
		{
			if (vessel.type == type)
				return vessel;
		}
		return VESSELS[1];
	}
} // namespace game::model
