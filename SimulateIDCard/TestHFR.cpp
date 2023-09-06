// TestHFR.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE h = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("Global\\HumanFaceRecognitionNotify"));
    if (!h) {
        printf("先启动HumanFace.exe\n");
        getchar();
    }
    else {
        cout << "按q退出；其他键继续";
        cout << endl << ":>";

        while (true)
        {
            std::string strCmd;
            getline(cin, strCmd);

            if (strCmd == "q") {
                break;
            }
            else {
                SetEvent(h);
            }

            cout << endl << "按q退出；其他键继续";
            cout << endl << ":>";
        }
    }

	return 0;
}

