#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propsys.h> 
#include <iostream>
#include <atlbase.h>
#include <atlcom.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "propsys.lib")

// TODO: break this method up into multiple methods
void ListFileExplorerDetailsViewVisibleColumns(LPCWSTR folderPath) {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::wcout << L"Failed to initialize COM. Error: " << hr << std::endl;
        return;
    }

    //LPITEMIDLIST pidl;
    CComHeapPtr<ITEMIDLIST> pidl;
    hr = SHParseDisplayName(folderPath, NULL, &pidl, 0, NULL);
    if (FAILED(hr)) {
        std::wcout << L"Failed to parse display name. Error: " << hr << std::endl;
        CoUninitialize();
        return;
    }

    CComPtr<IShellFolder> pShellFolder;
    hr = SHBindToObject(NULL, pidl, NULL, IID_IShellFolder, (void**)&pShellFolder);
    if (FAILED(hr)) {
        std::wcout << L"Failed to bind to shell folder. Error: " << hr << std::endl;
        //ILFree(pidl);  // Clean up PIDL
        CoUninitialize();
        return;
    }

    CSFV csfv = { 0 };
    csfv.cbSize = sizeof(CSFV);
    csfv.pshf = pShellFolder;
    csfv.psvOuter = NULL;
    csfv.pidl = NULL;
    csfv.lEvents = 0;
    csfv.pfnCallback = NULL;

    CComPtr<IShellView> pShellView;
    hr = SHCreateShellFolderViewEx(&csfv, &pShellView);
    if (FAILED(hr)) {
        std::wcout << L"Failed to create shell folder view. Error: " << hr << std::endl;
        //ILFree(pidl);  // Clean up PIDL
        CoUninitialize();
        return;
    }

    CComPtr<IColumnManager> pColumnManager;
    hr = pShellView->QueryInterface(IID_PPV_ARGS(&pColumnManager));
    if (SUCCEEDED(hr)) {
        UINT columnCount = 0;
        HRESULT hr = pColumnManager->GetColumnCount(CM_ENUM_VISIBLE, &columnCount);
        if (FAILED(hr)) {
            std::wcout << L"Failed to get column count. Error: " << hr << std::endl;
            return;
        }

        //PROPERTYKEY* pColumns = new PROPERTYKEY[columnCount];
        CComHeapPtr<PROPERTYKEY> pColumns;

        // Allocate the memory for the columns
        if (!pColumns.Allocate(columnCount)) {
            std::wcout << L"Failed to allocate memory for columns." << std::endl;
            return;
        }

        std::wcout << L"Windows File Explorer Detail View Visible Columns for " << folderPath << std::endl;
        hr = pColumnManager->GetColumns(CM_ENUM_VISIBLE, pColumns, columnCount);
        if (SUCCEEDED(hr)) {
            for (UINT i = 0; i < columnCount; ++i) {
                //LPWSTR pszCanonicalName = nullptr;
                CComHeapPtr<WCHAR> pszCanonicalName;

                hr = PSGetNameFromPropertyKey(pColumns[i], &pszCanonicalName);
                if (SUCCEEDED(hr)) {
                    //std::wcout << L"Column " << i + 1 << L": " << pszCanonicalName.m_pData << std::endl;
                    std::wcout << L"Column " << i + 1 << L": " << static_cast<WCHAR*>(pszCanonicalName) << std::endl;
                    //CoTaskMemFree(pszCanonicalName);
                }
                else {
                    std::wcout << L"Failed to get column name for index " << i << L". Error: " << hr << std::endl;
                }
            }
        }
        else {
            std::wcout << L"Failed to get columns. Error: " << hr << std::endl;
        }

        //delete[] pColumns;
    }
    else {
        std::wcout << L"Failed to query IColumnManager interface. Error: " << hr << std::endl;
    }

    //ILFree(pidl);
    CoUninitialize();
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 2)
    {
        std::wcout << L"Usage: GetColumnsCppApATL.exe <directory_path>" << std::endl;
        return 1;
    }

    ListFileExplorerDetailsViewVisibleColumns(argv[1]);

    return 0;
}
