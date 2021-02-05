typedef BOOL(_fastcall* QueryWorkingSetEx_t)(HANDLE hProcess, PVOID  pv, DWORD  cb);
QueryWorkingSetEx_t QueryWorkingSetEx_ptr;
 
HMODULE mod = GetModuleHandleA("KERNELBASE.dll");
void* queryAddr = (void*)GetProcAddress(mod, "QueryWorkingSetEx");
 
struct savedData {
    _MEMORY_BASIC_INFORMATION VIR_BLOCK;
    PSAPI_WORKING_SET_EX_BLOCK EX_BLOCK;
    SIZE_T MemoryInformationLength;
};
 
std::map<void*, savedData> savedResults;
 
BOOL QueryWorkingSetEx_detour(HANDLE hProcess, PVOID pv, DWORD cb)
{
    BOOL result = QueryWorkingSetEx_ptr(hProcess, pv, cb);
 
    for (int k = 0; k < cb / sizeof(PMEMORY_BASIC_INFORMATION); k++) {
        void* manipuledAddress = ((PMEMORY_BASIC_INFORMATION)pv)[k].BaseAddress;
        for (int i = 0; i < cb / sizeof(PSAPI_WORKING_SET_EX_INFORMATION); i++) {
            if (!savedResults.count(manipuledAddress)) {
                savedData saveToData;
                saveToData.VIR_BLOCK = ((PMEMORY_BASIC_INFORMATION)pv)[k];
                saveToData.EX_BLOCK = ((PPSAPI_WORKING_SET_EX_INFORMATION)pv)[i].VirtualAttributes;
                saveToData.MemoryInformationLength = cb;
                savedResults.insert(std::pair<void*, savedData>(manipuledAddress, saveToData));
            }
            else {
                ((PMEMORY_BASIC_INFORMATION)pv)[k] = savedResults.find(manipuledAddress)->second.VIR_BLOCK;
                ((PPSAPI_WORKING_SET_EX_INFORMATION)pv)[i].VirtualAttributes = savedResults.find(manipuledAddress)->second.EX_BLOCK;
                cb = savedResults.find(manipuledAddress)->second.MemoryInformationLength;
            }
        }
    }
 
    return result;
}

// MORE LOWER
void* NtQueryVirtualMemoryAddr = (void*)GetProcAddress(GetModuleHandleA("ntdll.dll"), "ZwQueryVirtualMemory");
typedef LONG(_stdcall* NtQueryVirtualMemory_t)(HANDLE  ProcessHandle, PVOID BaseAddress, __int64 MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength);
NtQueryVirtualMemory_t NtQueryVirtualMemory_ptr = nullptr;
 
LONG _stdcall NtQueryVirtualMemory_detour(HANDLE ProcessHandle, PVOID BaseAddress, __int64 MemoryInformationClass, PVOID MemoryInformation, SIZE_T MemoryInformationLength, PSIZE_T ReturnLength)
{
    auto result = NtQueryVirtualMemory_ptr(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);
    for (int k = 0; k < MemoryInformationLength / sizeof(PMEMORY_BASIC_INFORMATION); k++) {
        void* manipuledAddress = ((PMEMORY_BASIC_INFORMATION)MemoryInformation)[k].BaseAddress;
        for (int i = 0; i < MemoryInformationLength / sizeof(PSAPI_WORKING_SET_EX_INFORMATION); i++) {
            if (!savedResults.count(manipuledAddress)) {
                savedData saveToData;
                saveToData.VIR_BLOCK = ((PMEMORY_BASIC_INFORMATION)MemoryInformation)[k];
                saveToData.EX_BLOCK = ((PPSAPI_WORKING_SET_EX_INFORMATION)MemoryInformation)[i].VirtualAttributes;
                saveToData.MemoryInformationLength = MemoryInformationLength;
                savedResults.insert(std::pair<void*, savedData>(manipuledAddress, saveToData));
            }
            else {
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)[k] = savedResults.find(manipuledAddress)->second.VIR_BLOCK;
                ((PPSAPI_WORKING_SET_EX_INFORMATION)MemoryInformation)[i].VirtualAttributes = savedResults.find(manipuledAddress)->second.EX_BLOCK;
                MemoryInformationLength = savedResults.find(manipuledAddress)->second.MemoryInformationLength;
            }
        }
    }
    return result;
}
