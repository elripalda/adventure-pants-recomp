// Adventure Pants Recomp - Windows-only helpers

#include "platform.h"

#include <cstdio>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shobjidl.h>
#endif

namespace adventure_pants {

#if defined(_WIN32)
namespace {
std::string WideToUtf8(const wchar_t* w) {
  int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
  std::string s(n > 0 ? n - 1 : 0, '\0');
  if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), n, nullptr, nullptr);
  return s;
}
}  // namespace

std::string BrowseForCopy(bool folder) {
  std::string result;
  HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  IFileOpenDialog* dialog = nullptr;
  if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(&dialog)))) {
    DWORD options = 0;
    dialog->GetOptions(&options);
    options |= FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST;
    if (folder) options |= FOS_PICKFOLDERS;
    dialog->SetOptions(options);
    dialog->SetTitle(folder ? L"Choose the folder with your copy"
                            : L"Choose your Fancy Pants Adventures package");
    if (SUCCEEDED(dialog->Show(GetActiveWindow()))) {
      IShellItem* item = nullptr;
      if (SUCCEEDED(dialog->GetResult(&item))) {
        PWSTR path = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
          result = WideToUtf8(path);
          CoTaskMemFree(path);
        }
        item->Release();
      }
    }
    dialog->Release();
  }
  if (SUCCEEDED(init)) CoUninitialize();
  return result;
}

void WriteStdout(const std::string& text) {
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  if (out && out != INVALID_HANDLE_VALUE) {
    DWORD written = 0;
    WriteFile(out, text.data(), DWORD(text.size()), &written, nullptr);
    return;
  }
  std::fputs(text.c_str(), stdout);
  std::fflush(stdout);
}
#else
std::string BrowseForCopy(bool) { return {}; }
void WriteStdout(const std::string& text) {
  std::fputs(text.c_str(), stdout);
  std::fflush(stdout);
}
#endif

}  // namespace adventure_pants
