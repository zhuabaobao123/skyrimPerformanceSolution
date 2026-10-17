#pragma once

namespace Localization
{
	void Load();

	[[nodiscard]] bool Chinese() noexcept;

	// 英文原文查简中词条；查不到返回 nullptr（由调用方回退原文）
	[[nodiscard]] const char* Zh(const char* a_key) noexcept;
}
