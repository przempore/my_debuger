#include "windows.h"
#include "map"

BOOL SpawnAndAttachProcess(LPTSTR lpApplicationName) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);

  if (!CreateProcess(lpApplicationName, NULL, NULL, NULL, FALSE,
                        DEBUG_ONLY_THIS_PROCESS, NULL, NULL,
                        &si, &pi)) {
    return FALSE;
  }

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return TRUE;
}

int main(int argc, char **argv)
{
  if (argc != 2) {
    printf("Wrong argument count.\nPlease give path to program to debug.");
    return 1;
  }

  if (!SpawnAndAttachProcess(argv[1])) {
    return 1;
  }

  std::map<DWORD, HANDLE> processes;
  std::map<DWORD, HANDLE> threads;

  do {
    DEBUG_EVENT debug_ev;
    DWORD cont_status = DBG_CONTINUE;


    if (!WaitForDebugEvent(&debug_ev, INFINITE)) {
      break;
    }

    switch (debug_ev.dwDebugEventCode) {
      case CREATE_PROCESS_DEBUG_EVENT: 
        printf("CREATE_PROCESS, id: %u, base address: %p, start address: %p\n",
               debug_ev.dwProcessId,
               debug_ev.u.CreateProcessInfo.lpBaseOfImage,
               debug_ev.u.CreateProcessInfo.lpStartAddress);

        if (debug_ev.u.CreateProcessInfo.hFile != NULL) {
          CloseHandle(debug_ev.u.CreateProcessInfo.hFile);
        }

        processes[debug_ev.dwProcessId] = debug_ev.u.CreateProcessInfo.hProcess;
        threads[debug_ev.dwThreadId] = debug_ev.u.CreateProcessInfo.hThread;
        break;
      case CREATE_THREAD_DEBUG_EVENT:
        printf("CREATE_THREAD, tid: %u, TLS: %p, start address: %p\n",
            debug_ev.u.CreateThread.lpThreadLocalBase,
            debug_ev.u.CreateThread.lpStartAddress);
        threads[debug_ev.dwThreadId] = debug_ev.u.CreateThread.hThread;
        break;
    }

    ContinueDebugEvent(debug_ev.dwProcessId,
                       debug_ev.dwThreadId,
                       cont_status);
  } while (!processes.empty());

  return 0;
}
