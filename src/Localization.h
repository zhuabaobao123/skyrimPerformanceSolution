#pragma once

namespace Localization
{
	void Load();

	// 用英文原文查当前语言包词条；包没加载或查不到返回 nullptr（由调用方回退原文）
	[[nodiscard]] const char* Lookup(const char* a_key) noexcept;
}
