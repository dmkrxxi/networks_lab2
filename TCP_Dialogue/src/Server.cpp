#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

//#include <vector>
//#include <string>

#include <iomanip>
#include <sstream>

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

DWORD WINAPI ClientHandler(LPVOID lpParam) {
    SOCKET ClientSocket = (SOCKET)lpParam;
    cout << "Клиент подключился\n";

    //
    int timeout = 10000;
    setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    int totalSize = 0;
    if (!recvAll(ClientSocket, (char*)&totalSize, sizeof(int))) {
        closesocket(ClientSocket);
        return 0;
    }
    cout << "Получен размер: " << totalSize << "\n";

    if (totalSize <= 0 || totalSize > MAX_SIZE) {
        closesocket(ClientSocket);
        return 0;
    }
    cout << "Размер получен верно\n";

    //vector<char> buffer(totalSize); 
    // получение данных
    char buffer[MAX_SIZE];
    if (!recvAll(ClientSocket, buffer, totalSize)) {
        closesocket(ClientSocket);
        return 0;
    }
    cout << "Данные получены\n";

    // десериализация
    /*char* ptr = buffer.data();
    int nameLen;
    memcpy(&nameLen, ptr, sizeof(int)); // прочитали, какой объём данных передан
    ptr += sizeof(int);

    if (nameLen <= 0 || nameLen > 100) {
        closesocket(ClientSocket);
        return 0;
    }

    string name(ptr, nameLen); // прочли строку из памяти
    ptr += nameLen;

    int grades[4];
    for (int i = 0; i < 4; i++) {
        memcpy(&grades[i], ptr, sizeof(int)); // поочередно читаем оценки из памяти
        ptr += sizeof(int);
    }

    double avg = (grades[0] + grades[1] + grades[2] + grades[3]) / 4.0;
    int minGrade = grades[0];
    for (int i = 1; i < 4; i++) {
        if (grades[i] < minGrade)
            minGrade = grades[i];
    }*/

    char* ptr = buffer;
    Student s{};
    memcpy(&s.nameLen, ptr, sizeof(int));
    ptr += sizeof(int);
    if (s.nameLen <= 0 || s.nameLen >= 100) {
        cout << "Слишком длинное имя\n";
        closesocket(ClientSocket);
        return 0;
    }
    memcpy(s.name, ptr, s.nameLen);
    s.name[s.nameLen] = '\0';
    ptr += s.nameLen;

    for (int i = 0; i < 4; i++) {
        memcpy(&s.grades[i], ptr, sizeof(int));
        ptr += sizeof(int);
    }

    double avg = (s.grades[0] + s.grades[1] + s.grades[2] + s.grades[3]) / 4.0;

    int minGrade = s.grades[0];
    for (int i = 1; i < 4; i++)
        if (s.grades[i] < minGrade)
            minGrade = s.grades[i];

    // Вывод
    stringstream ss;
    if (minGrade < 3) {
        ss << "Есть задолженности. Средний балл: " << fixed << setprecision(2) << avg << ". Стипендии нет.";
    }
    else if (minGrade == 3) {
        ss << "Задолженностей нет. Средний балл: " << fixed << setprecision(2) << avg << ". Стипендии нет.";
    }
    else if (minGrade == 4) {
        ss << "Задолженностей нет. Средний балл: " << fixed << setprecision(2) << avg << ". Стипендия: 2200р.";
    }
    else {
        ss << "Задолженностей нет. Средний балл: " << fixed << setprecision(2) << avg << ". Стипендия: 3300р.";
    }
    string msg = ss.str();

    cout << "[Сервер] " << s.name << " -> " << msg << "\n";

    int msgSize = (int)msg.size() + 1;

    if (!sendAll(ClientSocket, (char*)&msgSize, sizeof(int)) ||
        !sendAll(ClientSocket, msg.c_str(), msgSize)) {
        // отправили размер данных, а затем сами данные
        cout << "Ошибка отправки данных\n";
        closesocket(ClientSocket);
        return 0;
    }

    closesocket(ClientSocket);
    return 0;
}

int main() {
    //setlocale(LC_ALL, "rus");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    WSADATA wsData; // включает работу с сетью
    int erStat = WSAStartup(MAKEWORD(2, 2), &wsData);
    if (erStat != 0) {
        cout << "Ошибка инициализации winsock: " << WSAGetLastError() << "\n";
        system("pause");
        return 1;
    }

    SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // af_inet - семейство адресов ipv4
    // sock_stream - tcp; либо sock_dgram - udp, либо sock_raw - собственный
    if (ServerSocket == INVALID_SOCKET) {
        cout << "Ошибка инициализации сокета: " << WSAGetLastError() << "\n";
        system("pause");
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr.s_addr = INADDR_ANY;
    servInfo.sin_port = htons(12345); // 1234
    erStat = bind(ServerSocket, (sockaddr*)&servInfo, sizeof(servInfo));
    if (erStat != 0) {
        cout << "Ошибка привязки сервера к порту (bind): " << WSAGetLastError() << "\n";
        system("pause");
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    erStat = listen(ServerSocket, SOMAXCONN); // somaxconn - максимальное число подключений
    // либо, если надо N подключений, SOMAXCONN_HINT(N)
    if (erStat != 0) {
        cout << "Ошибка прослушивания подключений: " << WSAGetLastError() << "\n";
        system("pause");
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }
    
    cout << "TCP-сервер запущен (порт 12345). Ожидание подключений..." << endl;

    while (true) {
        SOCKET ClientSocket = accept(ServerSocket, NULL, NULL); // принятие клиента
        if (ClientSocket == INVALID_SOCKET) {
            cout << "Обнаружен клиент. Ошибка соединения: " << WSAGetLastError() << "\n";
            continue;
        }
        HANDLE hThread = CreateThread(NULL, 0, ClientHandler, (LPVOID)ClientSocket, 0, NULL); // для каждого клиента свой поток
        CloseHandle(hThread);
    }

    closesocket(ServerSocket);
    WSACleanup();
    return 0;
}