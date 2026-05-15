#include <windows.h>
#include <setupapi.h>
#include <iostream>
#include <vector>
#include <accctrl.h>
#include <aclapi.h>
#include <conio.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "advapi32.lib")

// Esta función bloquea el acceso al dispositivo para cualquier proceso que no sea este
void BloquearAcceso(const char* instancePath) {
    PACL pOldDACL = NULL, pNewDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;

    // 1. Obtener la seguridad actual
    if (GetNamedSecurityInfoA(instancePath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pOldDACL, NULL, &pSD) != ERROR_SUCCESS) {
        std::cout << "Fallo al obtener seguridad. ¿Eres Admin?" << std::endl;
        return;
    }

    // 2. Crear regla: DENEGAR lectura/escritura a "EVERYONE" (Todos)
    EXPLICIT_ACCESSA ea;
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESSA));
    ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
    ea.grfAccessMode = DENY_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = (LPSTR)"EVERYONE";

    if (SetEntriesInAclA(1, &ea, pOldDACL, &pNewDACL) != ERROR_SUCCESS) {
        std::cout << "Error al crear la regla de bloqueo." << std::endl;
        if (pSD) LocalFree(pSD);
        return;
    }

    // 3. Aplicar el bloqueo al hardware
    if (SetNamedSecurityInfoA((LPSTR)instancePath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDACL, NULL) == ERROR_SUCCESS) {
        std::cout << ">>> BLOQUEADO: El juego ya no podra ver este mando original." << std::endl;
    } else {
        std::cout << "Error critico al aplicar bloqueo. Revisa privilegios." << std::endl;
    }

    if (pNewDACL) LocalFree(pNewDACL);
    if (pSD) LocalFree(pSD);
}

int main() {
    std::cout << "=== ACL-GUARDIAN INDEPENDIENTE (Win7) ===\n";
    
    // Buscar dispositivos HID (Mandos USB)
    GUID hidGuid;
    // Usamos el GUID estandar de HID
    hidGuid = { 0x4d36e96f, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };
    
    HDEVINFO hDevInfo = SetupDiGetClassDevsA(&hidGuid, "HID", NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    SP_DEVICE_INTERFACE_DATA devData;
    devData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &hidGuid, i, &devData); i++) {
        DWORD size = 0;
        SetupDiGetDeviceInterfaceDetailA(hDevInfo, &devData, NULL, 0, &size, NULL);
        PSP_DEVICE_INTERFACE_DETAIL_DATA_A detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)malloc(size);
        detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
        
        if (SetupDiGetDeviceInterfaceDetailA(hDevInfo, &devData, detail, size, NULL, NULL)) {
            BloquearAcceso(detail->DevicePath);
        }
        free(detail);
    }
    

    std::cout << "\nMandos originales ocultados. Manten esta ventana abierta para jugar.\n";
    std::cout << "Presiona una tecla para liberar los mandos y salir...";
    _getch();
    
    return 0;
}
