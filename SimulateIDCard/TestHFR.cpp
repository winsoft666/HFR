// TestHFR.cpp : �������̨Ӧ�ó������ڵ㡣
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
        printf("������HumanFace.exe\n");
        getchar();
    }
    else {
        cout << "��q�˳�������������";
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

            cout << endl << "��q�˳�������������";
            cout << endl << ":>";
        }
    }

	return 0;
}

