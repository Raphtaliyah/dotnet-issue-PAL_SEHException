#define NO_INLINE_METHOD

using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Namespace;

internal static unsafe class Entry
{
    private const string INTERNAL_CALL = "__internal";

    [SuppressGCTransition]
    [DllImport(INTERNAL_CALL, EntryPoint = "HelloCpp")]
    private static extern void Test();

    [UnmanagedCallersOnly]
    internal static void Initialize()
    {
        SetDllResolver();
        OneFrame();
    }

#if NO_INLINE_METHOD
    [MethodImpl(MethodImplOptions.NoInlining)]
#else
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
#endif
    private static void OneFrame()
    {
        Test();
    }

    private static void SetDllResolver()
    {
        NativeLibrary.SetDllImportResolver(
            Assembly.GetExecutingAssembly(),
            static (library, _, _) =>
                library == INTERNAL_CALL ? NativeLibrary.GetMainProgramHandle() : IntPtr.Zero
        );
    }
}
