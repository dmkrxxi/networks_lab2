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

bool recvAll(SOCKET s, char* data, int size) {
    int total = 0;
    while (total < size) {
        int received = recv(s, data + total, size - total, 0);
        if (received <= 0) return false;
        total += received;
    }
    return true;
}

bool sendAll(SOCKET s, const char* data, int size) {
    int total = 0;
    while (total < size) {
        int sent = send(s, data + total, size - total, 0);
        if (sent <= 0) return false;
        total += sent;
    }
    return true;
}

int main() {
    //setlocale(LC_ALL, "rus");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    WSADATA wsData;
    int erStat = WSAStartup(MAKEWORD(2, 2), &wsData);
    if (erStat != 0) {
        cout << "Ошибка инициализации winsock: " << WSAGetLastError() << "\n";
        system("pause");
        return 1;
    }

    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "Ошибка инициализации сокета: " << WSAGetLastError() << "\n";
        system("pause");
        //closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // 10 с - ограничиваем время для операций приема и отправки
    int timeout = 10000;
    setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_port = htons(12345); // 1234
    if (inet_pton(AF_INET, "127.0.0.1", &servInfo.sin_addr) <= 0) {
        cout << "Ошибка преобразования адреса" << "\n";
        system("pause");
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    erStat = connect(ClientSocket, (sockaddr*)&servInfo, sizeof(servInfo)); // установка TCP-соединения с сервером
    if (erStat != 0) {
        cout << "Ошибка подключения к серверу: " << WSAGetLastError() << "\n";
        system("pause");
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    /*string name;
    int grades[4];
    cout << "Фамилия: "; cin >> name;
    cout << "Введите 4 оценки: ";
    for (int i = 0; i < 4; i++)
        cin >> grades[i];*/

    Student s{};
    cout << "Фамилия: ";
    cin >> s.name;
    s.nameLen = strlen(s.name);
    cout << "Введите 4 оценки: ";
    for (int i = 0; i < 4; i++)
        cin >> s.grades[i];

    // сериализация

    /*int nameLen = (int)name.size();
    int dataSize = sizeof(int) + nameLen + 4 * sizeof(int);
    vector<char> packet;
    packet.reserve(dataSize);
    char* p = (char*)&nameLen;
    packet.insert(packet.end(), p, p + sizeof(int)); // скопировали размер сообщения в пакет
    packet.insert(packet.end(), name.begin(), name.end()); // скопировали фамилию в пакет
    for (int i = 0; i < 4; i++) { // копируем оценки в пакет
        p = (char*)&grades[i];
        packet.insert(packet.end(), p, p + sizeof(int));
    }*/

    char buffer[MAX_SIZE]; // хранится в стеке
    char* ptr = buffer; // &buffer[0]
    memcpy(ptr, &s.nameLen, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, s.name, s.nameLen);
    ptr += s.nameLen;
    for (int i = 0; i < 4; i++) {
        memcpy(ptr, &s.grades[i], sizeof(int));
        ptr += sizeof(int);
    }
    int dataSize = ptr - buffer; // &buffer[0]

    if (!sendAll(ClientSocket, (char*)&dataSize, sizeof(int))) {
        cout << "Ошибка отправки размера\n";
        system("pause");
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    if (!sendAll(ClientSocket, buffer, dataSize)) {
        cout << "Ошибка отправки данных: " << WSAGetLastError() << "\n";
        system("pause");
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // получение ответа
    int respSize = 0;
    if (recvAll(ClientSocket, (char*)&respSize, sizeof(int))) {
        //vector<char> respBuf(respSize);

        if (respSize <= 0 || respSize > MAX_SIZE) {
            cout << "Некорректный размер ответа\n";
            closesocket(ClientSocket);
            WSACleanup();
            system("pause");
            return 1;
        }
        char resp[MAX_SIZE];
        recvAll(ClientSocket, resp, respSize);
        cout << "Сервер ответил: " << resp << "\n";
    }
    else {
        cout << "Тайм-аут или ошибка получения ответа\n";
        system("pause");
    }

    closesocket(ClientSocket);
    WSACleanup();
    cout << "Завершение работы клиента.\n";
    system("pause");

    return 0;
}