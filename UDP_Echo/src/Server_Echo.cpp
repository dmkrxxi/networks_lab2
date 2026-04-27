#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

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

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0); //SOCK_DGRAM = UDP

    int timeout = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    sockaddr_in servInfo{};
    servInfo.sin_family = AF_INET;
    servInfo.sin_port = htons(12345);
    servInfo.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (sockaddr*)&servInfo, sizeof(servInfo));

    cout << "UDP эхо-сервер запущен...\n";

    char buffer[MAX_SIZE];

    while (true) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);

        int bytes = recvfrom(sock, buffer, MAX_SIZE, 0, (sockaddr*)&clientAddr, &clientSize);

        if (bytes > 0) {
            cout << "Получены данные (" << bytes << " байт)\n";
            if (bytes > MAX_SIZE) {
                cout << "Слишком большие данные\n";
                continue;
            }
            cout << "Отправляем обратно\n";
            sendto(sock, buffer, bytes, 0, (sockaddr*)&clientAddr, clientSize); //
        }
    }

    closesocket(sock);
    WSACleanup();
}