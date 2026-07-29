$dllPath = "C:\Users\ak\.gemini\antigravity\scratch\client\output\Client.dll"
if (-not (Test-Path $dllPath)) {
    Write-Host "DLL file does not exist: $dllPath" -ForegroundColor Red
    exit 1
}

$proc = Get-Process javaw -ErrorAction SilentlyContinue
if (-not $proc) {
    Write-Host "javaw.exe process not found!" -ForegroundColor Red
    exit 1
}

$pidNum = $proc.Id
Write-Host "Target javaw.exe PID: $pidNum" -ForegroundColor Cyan

$signature = @"
using System;
using System.Runtime.InteropServices;
using System.Text;

public class NativeInjector {
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern IntPtr OpenProcess(uint processAccess, bool bInheritHandle, int processId);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out IntPtr lpNumberOfBytesWritten);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern IntPtr GetModuleHandle(string lpModuleName);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern uint WaitForSingleObject(IntPtr hHandle, uint dwMilliseconds);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool CloseHandle(IntPtr hObject);

    public const uint PROCESS_ALL_ACCESS = 0x001F0FFF;
    public const uint MEM_COMMIT = 0x1000;
    public const uint MEM_RESERVE = 0x2000;
    public const uint PAGE_READWRITE = 0x04;

    public static bool InjectDll(int processId, string dllPath) {
        IntPtr hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
        if (hProcess == IntPtr.Zero) {
            Console.WriteLine("OpenProcess failed: " + Marshal.GetLastWin32Error());
            return false;
        }

        byte[] pathBytes = Encoding.Unicode.GetBytes(dllPath + "\0");
        IntPtr remoteMem = VirtualAllocEx(hProcess, IntPtr.Zero, (uint)pathBytes.Length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (remoteMem == IntPtr.Zero) {
            Console.WriteLine("VirtualAllocEx failed: " + Marshal.GetLastWin32Error());
            CloseHandle(hProcess);
            return false;
        }

        IntPtr bytesWritten;
        if (!WriteProcessMemory(hProcess, remoteMem, pathBytes, (uint)pathBytes.Length, out bytesWritten)) {
            Console.WriteLine("WriteProcessMemory failed: " + Marshal.GetLastWin32Error());
            CloseHandle(hProcess);
            return false;
        }

        IntPtr hKernel32 = GetModuleHandle("kernel32.dll");
        IntPtr pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");

        IntPtr hThread = CreateRemoteThread(hProcess, IntPtr.Zero, 0, pLoadLibraryW, remoteMem, 0, IntPtr.Zero);
        if (hThread == IntPtr.Zero) {
            Console.WriteLine("CreateRemoteThread failed: " + Marshal.GetLastWin32Error());
            CloseHandle(hProcess);
            return false;
        }

        WaitForSingleObject(hThread, 5000);
        CloseHandle(hThread);
        CloseHandle(hProcess);
        Console.WriteLine("DLL Injection successful into PID: " + processId);
        return true;
    }
}
"@

Add-Type -TypeDefinition $signature -Language CSharp
[NativeInjector]::InjectDll($pidNum, $dllPath)
