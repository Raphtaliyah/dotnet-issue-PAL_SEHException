#include <cstddef>

#include <cassert>
#include <coreclr/coreclr_delegates.h>
#include <coreclr/hostfxr.h>
#include <coreclr/nethost.h>
#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
#include <libloaderapi.h>
#else // _WIN32
#include <dlfcn.h>
#endif

using LibraryHandle = void*;

namespace Library {
static LibraryHandle Load(const char_t* path);
static void* GetExport(LibraryHandle handle, const char* name);

#ifdef _WIN32
static LibraryHandle Load(const char_t* path) { return (LibraryHandle)::LoadLibraryW(path); }
static void* GetExport(LibraryHandle handle, const char* name) { return ::GetProcAddress((HMODULE)handle, name); }
#else // _WIN32
static LibraryHandle Load(const char_t* path) { return dlopen(path, RTLD_LAZY | RTLD_LOCAL); }
static void* GetExport(LibraryHandle handle, const char* name) { return dlsym(handle, name); }
#endif // _WIN32

template <typename TDelegate>
static TDelegate GetFunction(LibraryHandle handle, const char* name) { return (TDelegate)GetExport(handle, name); }
}

struct Hostfxr {
    static inline hostfxr_initialize_for_runtime_config_fn InitRuntimeWithConfig = nullptr;
    static inline hostfxr_get_runtime_delegate_fn GetRuntimeDelegate = nullptr;
    static inline hostfxr_close_fn CloseHostfxr = nullptr;
};

struct CoreCLR {
    static inline load_assembly_fn LoadAssembly = nullptr;
    static inline get_function_pointer_fn GetFunctionPointer = nullptr;
};

static void AsciiToPlatformString(std::string_view asciiStr, char_t* destination)
{
    for (size_t i = 0; i < asciiStr.length(); i++)
        destination[i] = asciiStr[i];

    destination[asciiStr.length()] = '\0';
}

template <typename TDelegate>
static bool LoadFunction(LibraryHandle handle, const char* name, TDelegate* delegate)
{
    *delegate = Library::GetFunction<TDelegate>(handle, name);
    assert(delegate != nullptr);
    return *delegate != nullptr;
}

static LibraryHandle LoadHostfxr(const char_t* runtimePath)
{
    size_t bufferSize;
    get_hostfxr_parameters params = {
        .size = sizeof(get_hostfxr_parameters),
        .assembly_path = nullptr,
        .dotnet_root = runtimePath
    };
    auto res = get_hostfxr_path(nullptr, &bufferSize, &params);
    // The required result is the buffer too small error.
    // Anything else means something is actually wrong. (no runtime?)
    assert(res == (decltype(res))0x80008098);

    auto buffer = (char_t*)alloca(bufferSize * sizeof(char_t));
    res = get_hostfxr_path(buffer, &bufferSize, &params);
    assert(res == 0);

    auto libHandle = Library::Load(buffer);
    assert(libHandle != nullptr);

    return libHandle;
}

bool LoadHostfxrFunctions(LibraryHandle hostfxrLib)
{
    return LoadFunction(hostfxrLib, "hostfxr_initialize_for_runtime_config", &Hostfxr::InitRuntimeWithConfig)
        && LoadFunction(hostfxrLib, "hostfxr_get_runtime_delegate", &Hostfxr::GetRuntimeDelegate)
        && LoadFunction(hostfxrLib, "hostfxr_close", &Hostfxr::CloseHostfxr);
}

bool LoadCoreCLRFunctions(hostfxr_handle hostfxr)
{
    return Hostfxr::GetRuntimeDelegate(hostfxr, hdt_load_assembly, (void**)&CoreCLR::LoadAssembly) == 0
        && CoreCLR::LoadAssembly != nullptr
        && Hostfxr::GetRuntimeDelegate(hostfxr, hdt_get_function_pointer, (void**)&CoreCLR::GetFunctionPointer) == 0
        && CoreCLR::GetFunctionPointer != nullptr;
}

bool Load(const std::filesystem::path& path, const std::filesystem::path& runtimeConfigPath)
{
    auto hostfxr = LoadHostfxr(path.empty() ? nullptr : path.c_str());
    if (hostfxr == nullptr)
        return false;

    auto res = LoadHostfxrFunctions(hostfxr);
    assert(res);

    hostfxr_handle hostfxrHandle;
    res = Hostfxr::InitRuntimeWithConfig(runtimeConfigPath.c_str(), nullptr, &hostfxrHandle);
    assert(res == 0 && hostfxrHandle != nullptr);

    res = LoadCoreCLRFunctions(hostfxrHandle);
    assert(res);

    Hostfxr::CloseHostfxr(hostfxrHandle);
    return true;
}

bool LoadAssembly(const std::filesystem::path& path)
{
    auto res = CoreCLR::LoadAssembly(path.c_str(), nullptr, nullptr);
    return res == 0;
}

template <typename TFunc>
    requires std::is_function_v<TFunc>
static TFunc* GetFunctionPointer(std::string_view typeName, std::string_view methodName)
{
    // This doesn't allow having non ASCII identifiers but that's fine.
    auto typeNameBuffer = (char_t*)alloca((typeName.length() + 1) * sizeof(char_t));
    AsciiToPlatformString(typeName, typeNameBuffer);
    auto methodNameBuffer = (char_t*)alloca((methodName.length() + 1) * sizeof(char_t));
    AsciiToPlatformString(methodName, methodNameBuffer);

    void* fptr;
    auto res = CoreCLR::GetFunctionPointer(
        typeNameBuffer,
        methodNameBuffer,
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        nullptr,
        &fptr);

    if (res != 0)
        return nullptr;

    return (TFunc*)fptr;
}

static constexpr auto RUNTIME_CFG_PATH = "Lib/bin/Release/Lib.runtimeconfig.json";
static constexpr auto ASSEMBLY_PATH = "Lib/bin/Release/Lib.dll";
static constexpr auto RUNTIME_PATH = "runtime"; // can be empty to use system installed runtime.

int main()
{
    Load(RUNTIME_PATH, RUNTIME_CFG_PATH);
    LoadAssembly(ASSEMBLY_PATH);

    auto fptr = GetFunctionPointer<void()>("Namespace.Entry, Lib", "Initialize");
    assert(fptr != nullptr);
    fptr();
}