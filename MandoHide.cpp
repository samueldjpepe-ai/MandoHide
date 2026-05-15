#include <windows.h>
#include <setupapi.h>
#include <iostream>
#include <vector>
#include <conio.h>

extern "C" {
    void __stdcall HidD_GetHidGuid(LPGUID HidGuid);
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
            // CAMBIO CLAVE: Permitimos compartir lectura/escritura (FILE_SHARE_READ | FILE_SHARE_WRITE)
            // Pero abrimos con GENERIC_READ para "enganchar" el dispositivo.
            HANDLE hDevice = CreateFileA(
                detailData->DevicePath, 
                GENERIC_READ, 
                FILE_SHARE_READ | FILE_SHARE_WRITE, // Permite que tu emulador tambien lo abra
                NULL, 
                OPEN_EXISTING, 
                0, 
                NULL
            );

            if (hDevice != INVALID_HANDLE_VALUE) {
                std::cout << "Mando 'Anclado'. Tu emulador puede usarlo, el juego tendra conflictos para verlo como principal." << std::endl;
                mandosBloqueados.push_back(hDevice);
            }
        }
        free(detailData);
    }
    SetupDiDestroyDeviceInfoList(deviceInfoSet);
}

int main() {
    std::cout << "=== OCULTADOR COMPARTIDO PARA WIN7 ===\n";
    ocultarMandos();
    
    if(mandosBloqueados.empty()) {
        std::cout << "No se pudo anclar ningun mando fisico.\n";
    } else {
        std::cout << "Mandos anclados exitosamente.\n";
        std::cout << "1. Mantén esta ventana abierta.\n";
        std::cout << "2. Abre tu emulador ahora.\n";
        std::cout << "3. Abre el juego.\n";
    }
    
    _getch();
    for (HANDLE h : mandosBloqueados) CloseHandle(h);
    return 0;
}
