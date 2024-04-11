/**
  Copyright (c) 2011-2015, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/ArmMmuLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/DeviceMemoryMapLib.h>

VOID
BuildMemoryTypeInformationHob ();

STATIC
EFI_STATUS
InitMmu (IN ARM_MEMORY_REGION_DESCRIPTOR *MemoryTable)
{
  VOID          *TranslationTableBase;
  UINTN          TranslationTableSize;
  RETURN_STATUS  Status;

  // Note: Because we Called PeiServicesInstallPeiMemory() before to call InitMmu() the MMU Page Table resides in
  //       DRAM (even at the top of DRAM as it is the first Permanent Memory Allocation)
  Status = ArmConfigureMmu (MemoryTable, &TranslationTableBase, &TranslationTableSize);

  return Status;
}

STATIC
VOID
AddHob (PARM_MEMORY_REGION_DESCRIPTOR_EX Desc)
{
  if (Desc->HobOption != AllocOnly) {
    BuildResourceDescriptorHob (Desc->ResourceType, Desc->ResourceAttribute, Desc->Address, Desc->Length);
  }

  if (Desc->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY || Desc->MemoryType == EfiRuntimeServicesData) {
    BuildMemoryAllocationHob (Desc->Address, Desc->Length, Desc->MemoryType);
  }
}

EFI_STATUS
EFIAPI
MemoryPeim (
  IN EFI_PHYSICAL_ADDRESS UefiMemoryBase,
  IN UINT64               UefiMemorySize)
{
  EFI_STATUS                       Status;
  ARM_MEMORY_REGION_DESCRIPTOR     MemoryTable[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];
  PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx = GetDeviceMemoryMap();
  UINTN                            Index              = 0;

  // Ensure PcdSystemMemorySize has been Set
  ASSERT (PcdGet64 (PcdSystemMemorySize) != 0);

  // Run through each Memory Descriptor
  while (MemoryDescriptorEx->Length != 0) {
    switch (MemoryDescriptorEx->HobOption) {
      case AddMem:
      case AddDev:
      case HobOnlyNoCacheSetting:
      case AllocOnly:
        AddHob(MemoryDescriptorEx);
        break;

      case NoHob:
      default:
        goto update;
    }

    if (MemoryDescriptorEx->HobOption == HobOnlyNoCacheSetting) {
      MemoryDescriptorEx++;
      continue;
    }

  update:
    ASSERT (Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);

    MemoryTable[Index].PhysicalBase = MemoryDescriptorEx->Address;
    MemoryTable[Index].VirtualBase  = MemoryDescriptorEx->Address;
    MemoryTable[Index].Length       = MemoryDescriptorEx->Length;
    MemoryTable[Index].Attributes   = MemoryDescriptorEx->ArmAttributes;

    Index++;
    MemoryDescriptorEx++;
  }

  // Last one (terminator)
  ASSERT (Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);
  MemoryTable[Index].PhysicalBase = 0;
  MemoryTable[Index].VirtualBase  = 0;
  MemoryTable[Index].Length       = 0;
  MemoryTable[Index].Attributes   = 0;

  // Build Memory Allocation Hob
  Status = InitMmu (MemoryTable);
  if (EFI_ERROR (Status)) { goto exit; }

  if (FeaturePcdGet (PcdPrePiProduceMemoryTypeInformationHob)) {
    // Optional Feature that Helps prevent EFI Memory Map fragmentation
    BuildMemoryTypeInformationHob ();
  }

exit:
  return Status;
}
