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

int main()
{
  std::map<DWORD, HANDLE> processes;
  std::map<DWORD, HANDLE> threads;

  do {
    DEBUG_EVENT debug_ev;
    DWORD cont_status = DBG_CONTINUE;

    if (!WaitForDebugEvent(&debug_ev, INFINITE)) {
      break;
    }

    switch (debug_ev.dwDebugEventCode) {
      // obsłuż poszczególne zdarzenia.
    }

    ContinueDebugEvent(debug_ev.dwProcessId,
                       debug_ev.dwThreadId,
                       cont_status);
  } while (!processes.empty());

  return 0;
}
