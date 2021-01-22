#include <efi.h>
#include <efilib.h>

#include "../../../kernel.h"

GRUBMultiboot multi;
 
EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
  InitializeLib(ImageHandle, SystemTable);

  //
  // Set video mode to 2 so it uses uefi code to link
  setVideoMode(2);

  //
  // Setup things for GRUBMultiboot stuff
  char *commandline = "";
  multi.cmdline = (unsigned long)commandline;

  //
  // Check if all goes well....

  printf("[UEFI] This is the SanderOSUSB Operating System.\n");
  printf("[UEFI] This system is booted with UEFI\n");
  printf("[UEFI] The system is build at %s %s \n",__DATE__,__TIME__);
  printf("[UEFI] If this text is showing an hi, then this is good: %s \n","hi");
  printf("[UEFI] If this text is showing an 0x10, then this is good: 0x%x \n",0x10);

  //
  // lets go
  kernel_main((GRUBMultiboot*)&multi,0x2BADB002);
  
  for(;;);
}

void uefi_print(char e){
  CHAR16 *tele = L" ";
  tele[0] = e;
  Print(tele);
}