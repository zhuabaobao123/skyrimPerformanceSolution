#include "Localization.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <windows.h>

#include <nlohmann/json.hpp>

namespace Localization
{
	namespace
	{
		std::unordered_map<std::string, std::string> s_map;
		std::atomic<bool>                            s_ready{ false };

		// 翻译词条在 DLL 同目录的 SPS.json，随 DLL 取自身路径（中文件名文件夹安全）
		[[nodiscard]] std::filesystem::path JsonPath()
		{
			HMODULE mod{};
			GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				reinterpret_cast<LPCWSTR>(&JsonPath), &mod);
			if (!mod) {
				return {};
			}
			std::wstring self(32768, L'\0');
			const auto   len = GetModuleFileNameW(mod, self.data(), static_cast<DWORD>(self.size()));
			self.resize(len);
			return std::filesystem::path(self).parent_path() / L"SPS.json";
		}

		void LoadLocked(const std::filesystem::path& a_path)
		{
			std::ifstream in(a_path, std::ios::binary);
			if (!in.is_open()) {
				return;
			}
			std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
			// 剥离 UTF-8 BOM
			if (data.size() >= 3 && static_cast<unsigned char>(data[0]) == 0xEF &&
				static_cast<unsigned char>(data[1]) == 0xBB && static_cast<unsigned char>(data[2]) == 0xBF) {
				data.erase(0, 3);
			}
			const auto parsed = nlohmann::json::parse(data, nullptr, /*allow_exceptions=*/false);
			if (!parsed.is_object()) {
				return;
			}
			for (auto it = parsed.begin(); it != parsed.end(); ++it) {
				if (it.value().is_string()) {
					s_map[it.key()] = it.value().get<std::string>();
				}
			}
		}
	}

	void Load()
	{
		static std::mutex once;
		const std::lock_guard lock{ once };
		if (s_ready.load(std::memory_order_acquire)) {
			return;
		}
		const auto path = JsonPath();
		if (!path.empty()) {
			LoadLocked(path);
		}
		s_ready.store(true, std::memory_order_release);
	}

	bool Chinese() noexcept
	{
		return s_ready.load(std::memory_order_acquire) && !s_map.empty();
	}

	const char* Zh(const char* a_key) noexcept
	{
		if (!a_key || !s_ready.load(std::memory_order_acquire)) {
			return nullptr;
		}
		if (const auto it = s_map.find(a_key); it != s_map.end()) {
			return it->second.c_str();
		}
		return nullptr;
	}
}
