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
      case EXCEPTION_DEBUG_EVENT:
        cont_status = DBG_EXCEPTION_NOT_HANDLED;

        printf("EXCEPTION, first chance: %u, code: %x, addres: %p\n",
               debug_ev.u.Exception.dwFirstChance,
               debug_ev.u.Exception.ExceptionRecord.ExceptionCode,
               debug_ev.u.Exception.ExceptionRecord.ExceptionAddress);

        if (!debug_ev.u.Exception.dwFirstChance) {
          TerminateProcess(processes[debug_ev.dwProcessId], 0);
        }
        break;
      case EXIT_PROCESS_DEBUG_EVENT:
        printf("EXIT_PROCESS, pid: %u, tid: %u, exit code: %u\n",
               debug_ev.dwProcessId, debug_ev.dwThreadId,
               debug_ev.u.ExitProcess.dwExitCode);

        CloseHandle(processes[debug_ev.dwProcessId]);
        CloseHandle(threads[debug_ev.dwThreadId]);

        processes.erase(debug_ev.dwProcessId);
        threads.erase(debug_ev.dwThreadId);
        break;
      case EXIT_THREAD_DEBUG_EVENT:
        printf("EXIT THREAD, tid: %u, exit code: %u\n",
               debug_ev.dwThreadId, debug_ev.u.ExitThread.dwExitCode);

        CloseHandle(threads[debug_ev.dwThreadId]);
        threads.erase(debug_ev.dwThreadId);
        break;
      case LOAD_DLL_DEBUG_EVENT:
        printf("LOAD_DLL, base address: %p\n", debug_ev.u.LoadDll.lpBaseOfDll);

        if (debug_ev.u.LoadDll.hFile != NULL) {
          CloseHandle(debug_ev.u.LoadDll.hFile);
        }
        break;
      case UNLOAD_DLL_DEBUG_EVENT:
        printf("UNLOAD_DLL, base address: %p\n", debug_ev.u.LoadDll.lpBaseOfDll);

        break;
      case OUTPUT_DEBUG_STRING_EVENT:
        printf("OUTPUT_DEBUG_STRING\n");
        break;
      case RIP_EVENT:
        printf("RIP_EVENT\n");
        break;
    }

    ContinueDebugEvent(debug_ev.dwProcessId,
                       debug_ev.dwThreadId,
                       cont_status);
  } while (!processes.empty());

  return 0;
}
