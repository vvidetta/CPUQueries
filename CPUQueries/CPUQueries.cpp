// CPUQueries.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iomanip>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include "Windows.h"
#endif // _WIN32

auto getCacheDetails() -> std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>
{
    auto kernel32Handle = HMODULE{};
    if (!GetModuleHandleEx(NULL, TEXT("kernel32.dll"), &kernel32Handle))
    {
        return {};
    }

    using glpi_ptr = BOOL(WINAPI*)(LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD);
    auto const glpix = reinterpret_cast<glpi_ptr>(GetProcAddress(kernel32Handle, "GetLogicalProcessorInformationEx"));
    if (glpix == nullptr)
    {
        return {};
    }

    auto bufferLength = DWORD{ 0 };
    if (glpix(RelationCache, nullptr, &bufferLength) || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        // The first call to glpi just gets the size of the required buffer.
        // We expect it to fail with an insufficient buffer error, otherwise abort.
        return {};
    }

    auto buffer = std::vector<char>(bufferLength);
    if (!glpix(RelationCache, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &bufferLength))
    {
        return {};
    }

    auto constexpr STRUCT_SIZE_INDEX = 1;
    auto cacheInfo = std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>();
    auto const end_buffer = buffer.data() + bufferLength;
    for (auto pSrc = buffer.data(); pSrc < end_buffer;)
    {
        cacheInfo.emplace_back();
        auto const struct_size = reinterpret_cast<DWORD*>(pSrc)[STRUCT_SIZE_INDEX];
        std::memcpy(&cacheInfo.back(), pSrc, struct_size);
        pSrc += struct_size;
    }
    return cacheInfo;
}

std::ostream& operator<<(std::ostream& os, SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const& value)
{
    os << "======= PROCESSOR INFORMATION =======" << std::endl;
    switch (value.Relationship)
    {
    default:
        os << "Relationship: Unknown\n"
        "Size: " << value.Size << std::endl;
        break;

    case RelationCache:
        os << "Relationship: Cache\n"
            "Size: " << value.Size << "\n"
            "Level: " << static_cast<int>(value.Cache.Level) << "\n"
            "CacheSize: " << value.Cache.CacheSize << "\n"
            "LineSize: " << value.Cache.LineSize << "\n"
            "Associativity: " << static_cast<int>(value.Cache.Associativity) << "\n";
        switch (value.Cache.Type)
        {
        default:
            os << "CacheType: Unknown\n";
            break;

        case CacheUnified:
            os << "CacheType: Unified\n";
            break;

        case CacheData:
            os << "CacheType: Data\n";
            break;

        case CacheInstruction:
            os << "CacheType: Instruction\n";
            break;

        case CacheTrace:
            os << "CacheType: Trace\n";
            break;
        }

        os << "Group: " << value.Cache.GroupMask.Group << "\n"
            "Mask: 0x" << std::right << std::hex << std::setfill('0') << std::setw(8) << value.Cache.GroupMask.Mask << std::dec << "\n";

        os.flush();
        break;
    }

    return os;
}

int main()
{
    auto cacheDetails = getCacheDetails();
    for (auto const& cacheDetail : cacheDetails)
    {
        std::cout << cacheDetail;
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
