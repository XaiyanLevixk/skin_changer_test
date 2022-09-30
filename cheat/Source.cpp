#include"memory.h"
#include<thread>
#include"hazedumper.hpp"
#include<array>

using namespace std;

constexpr const int GetWeaponPaint(const short& itemDefinition)
{
	switch (itemDefinition)
	{
	case 1: return 1090; // deagle
	case 4: return 963; // glock
	case 7: return 433; // ak
	case 9: return 662; //awp
	case 29: return 638; //sawed off
	case 42: return 568;
	default: return 0;
	}
}

int main()
{
	auto memory = Memory("csgo.exe");
	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");

	while (true)
	{
		this_thread::sleep_for(chrono::milliseconds(2));

		const auto localPlayer = memory.Read<uintptr_t>(client + hazedumper::signatures::dwLocalPlayer);
		const auto weapons = memory.Read<array<unsigned long, 8>>(localPlayer + hazedumper::netvars::m_hMyWeapons); // handle is an unsigned long

		for (const auto handle : weapons)
		{
			const auto weapon = memory.Read<uintptr_t>((client + hazedumper::signatures::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

			if (!weapon)
				continue;

			if (const auto paint = GetWeaponPaint(memory.Read<short>(weapon + hazedumper::netvars::m_iItemDefinitionIndex)))
			{
				const bool shouldUpdate = memory.Read<int32_t>(weapon + hazedumper::netvars::m_nFallbackPaintKit) != paint;

				memory.Write<int32_t>(weapon + hazedumper::netvars::m_iItemIDHigh, -1);

				memory.Write<int32_t>(weapon + hazedumper::netvars::m_nFallbackPaintKit, paint);
				memory.Write<float>(weapon + hazedumper::netvars::m_flFallbackWear, 0.1f);

				memory.Write<int32_t>(weapon + hazedumper::netvars::m_nFallbackStatTrak, 1370);
				memory.Write<int32_t>(weapon + hazedumper::netvars::m_iAccountID, memory.Read<int32_t>(weapon + hazedumper::netvars::m_OriginalOwnerXuidLow));


				if (shouldUpdate)
					memory.Write<int32_t>(memory.Read<uintptr_t>(engine + hazedumper::signatures::dwClientState) + 0x174, -1);
			}
		}
	}

	return 0;
}