#define USE_DLLIMPORT

using System.Runtime.InteropServices;

namespace Namespace;

internal static unsafe class Entry
{
    [SuppressGCTransition]
    [DllImport("DoesntHaveToBeValid", EntryPoint = "DoesntHaveToExist")]
    private static extern void Test();

    [UnmanagedCallersOnly]
    internal static void Initialize(delegate* unmanaged[SuppressGCTransition]<void> test)
    {
#if USE_DLLIMPORT
        Test();
#else
        test();
#endif
    }
}
