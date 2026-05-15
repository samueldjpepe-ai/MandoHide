#include <windows.h>
#include <setupapi.h>
#include <iostream>
#include <vector>
#include <conio.h>

// Definiciones manuales para evitar dependencias de archivos de cabecera pesados
extern "C" {
    typedef struct _HIDD_ATTRIBUTES {
        ULONG  Size;
        USHORT VendorID;
        USHORT ProductID;
        USHORT VersionNumber;
    } HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

    void __stdcall HidD_GetHidGuid(LPGUID HidGuid);
    BOOL __stdcall HidD_GetProductString(HANDLE HidDeviceObject, PVOID Buffer, ULONG BufferLength);
}

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "hid.lib")

std::vector<HANDLE> mandosBloqueados;

void ocultarMandos() {
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, i, &deviceInterfaceData); i++) {
        DWORD requiredSize = 0;
        SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

        PSP_DEVICE_INTERFACE_DETAIL_DATA_A detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)malloc(requiredSize);
        detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

        if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, detailData, requiredSize, NULL, NULL)) {
            // ABRIR EN MODO EXCLUSIVO (ShareMode = 0)
            HANDLE hDevice = CreateFileA(detailData->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

            if (hDevice != INVALID_HANDLE_VALUE) {
                std::cout << "Mando bloqueado correctamente." << std::endl;
                mandosBloqueados.push_back(hDevice);
            }
        }
        free(detailData);
    }
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
}

int main() {
    std::cout << "=== OCULTADOR INDEPENDIENTE (GITHUB BUILD) ===\n";
    ocultarMandos();
    if(mandosBloqueados.empty()) {
        std::cout << "No se detectaron mandos para bloquear.\n";
    } else {
        std::cout << "Mandos ocultos. Presiona una tecla para liberar y salir.\n";
    }
    _getch();
    for (HANDLE h : mandosBloqueados) CloseHandle(h);
    return 0;
}
