#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

//#include <vector>
//#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int MAX_SIZE = 256;

struct Student {
    int nameLen;
    char name[100];
    int grades[4];
};

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    WSADATA wsData;
    WSAStartup(MAKEWORD(2, 2), &wsData);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0); // sock_dgram - udp

    int timeout = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    sockaddr_in servInfo{};
    servInfo.sin_family = AF_INET;
    servInfo.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &servInfo.sin_addr);

    Student s{};
    cout << "Фамилия: ";
    cin >> s.name;
    s.nameLen = strlen(s.name);
    cout << "Введите 4 оценки: ";
    for (int i = 0; i < 4; i++)
        cin >> s.grades[i];

    /*int nameLen = name.size();
    int dataSize = sizeof(int) + nameLen + 4 * sizeof(int);

    vector<char> packet;

    char* p = (char*)&nameLen;
    packet.insert(packet.end(), p, p + sizeof(int));
    packet.insert(packet.end(), name.begin(), name.end());

    for (int i = 0; i < 4; i++) {
        p = (char*)&grades[i];
        packet.insert(packet.end(), p, p + sizeof(int));
    }*/

    char buffer[MAX_SIZE];
    char* ptr = buffer;

    memcpy(ptr, &s.nameLen, sizeof(int));
    ptr += sizeof(int);

    memcpy(ptr, s.name, s.nameLen);
    ptr += s.nameLen;

    for (int i = 0; i < 4; i++) {
        memcpy(ptr, &s.grades[i], sizeof(int));
        ptr += sizeof(int);
    }

    int dataSize = ptr - buffer;

    if (dataSize > MAX_SIZE) {
        cout << "Слишком большой пакет\n";
        system("pause");
        return 1;
    }

    // отправка
    sendto(sock, buffer, dataSize, 0, (sockaddr*)&servInfo, sizeof(servInfo));

    // получение
    char recvBuf[MAX_SIZE];
    int servSize = sizeof(servInfo);

    int bytes = recvfrom(sock, recvBuf, MAX_SIZE, 0, (sockaddr*)&servInfo, &servSize);

    if (bytes > 0) {
        char* ptr = recvBuf;

        /*int nameLen;
        memcpy(&nameLen, ptr, sizeof(int));
        ptr += sizeof(int);

        string name(ptr, nameLen);
        ptr += nameLen;

        int grades[4];
        for (int i = 0; i < 4; i++) {
            memcpy(&grades[i], ptr, sizeof(int));
            ptr += sizeof(int);
        }*/

        Student r{};

        memcpy(&r.nameLen, ptr, sizeof(int));
        ptr += sizeof(int);

        if (r.nameLen <= 0 || r.nameLen >= 100) {
            cout << "Ошибка данных\n";
            system("pause");
            return 1;
        }

        memcpy(r.name, ptr, r.nameLen);
        r.name[r.nameLen] = '\0';
        ptr += r.nameLen;

        for (int i = 0; i < 4; i++) {
            memcpy(&r.grades[i], ptr, sizeof(int));
            ptr += sizeof(int);
        }

        cout << "Эхо от сервера:\n";
        cout << "Фамилия: " << r.name << endl;
        cout << "Оценки: ";
        for (int i = 0; i < 4; i++)
            cout << r.grades[i] << " ";
        cout << "\n";
    }
    else {
        cout << "Тайм-аут или ошибка\n";
    }

    closesocket(sock);
    WSACleanup();

    system("pause");
}