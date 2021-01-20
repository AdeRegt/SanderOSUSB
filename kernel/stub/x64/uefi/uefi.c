#include <efi.h>
#include <efilib.h>

#include "../../../kernel.h"

GRUBMultiboot multi;
 
EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
  InitializeLib(ImageHandle, SystemTable);
  Print(L"[UEFI] This is the Sanderslando Operating System.\n");
  Print(L"[UEFI] This system is booted with UEFI\n");
  Print(L"[UEFI] The system is build at %s %s \n",__DATE__,__TIME__);
  kernel_main((GRUBMultiboot*)&multi,0x2BADB002);
  for(;;);
}