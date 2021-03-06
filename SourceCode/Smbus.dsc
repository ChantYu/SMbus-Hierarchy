## @file
#   Intel(r) UEFI Application Development Kit for EDK II.
#   This package contains applications which depend upon Standard Libraries
#   from the StdLib package.
#
#   See the comments in the [LibraryClasses.IA32] and [BuildOptions] sections
#   for important information about configuring this package for your
#   environment.
#
#   Copyright (c) 2010 - 2014, Intel Corporation. All rights reserved.<BR>
#   This program and the accompanying materials
#   are licensed and made available under the terms and conditions of the BSD License
#   which accompanies this distribution. The full text of the license may be found at
#   http://opensource.org/licenses/bsd-license.
#
#   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
##

[Defines]
  PLATFORM_NAME                  = Smbus
  PLATFORM_GUID                  = ff0c6881-0ca3-4ecd-8655-0af627ba1450
##    0458dade-8b6e-4e45-b773-1b27cbda3e06
  PLATFORM_VERSION               = 0.01
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/Smbus
  SUPPORTED_ARCHITECTURES        = X64
  BUILD_TARGETS                  = RELEASE|DEBUG
#
#  Debug output control
#
  DEFINE DEBUG_ENABLE_OUTPUT      = FALSE       # Set to TRUE to enable debug output
  DEFINE DEBUG_PRINT_ERROR_LEVEL  = 0x80000040  # Flags to control amount of debug output
  DEFINE DEBUG_PROPERTY_MASK      = 0

  

[PcdsFixedAtBuild.IPF]

[LibraryClasses]
# Entry Point Libraries
   UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
   ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
   UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf

# Common Libraries  
   BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
   MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
   UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
   BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  !if $(DEBUG_ENABLE_OUTPUT)
    DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
    DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  !else   ## DEBUG_ENABLE_OUTPUT
    DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  !endif  ## DEBUG_ENABLE_OUTPUT
   UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
   UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
   DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf   
   PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
   HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
   UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
   PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
   UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf   
   DxeSmbLib|MdePkg/Library/DxeSmbusLib/DxeSmbusLib.inf
   
   
# CDYU 20170616 for GPIO
   IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
# CDYU 20170620 for SVID  
   PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
# try & error , compare to AppPkg.dsc  --> must exist
   PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
   
# CDYU 20170622 for SVID -> no used  / 20170627 for SMBIOS 
   UefiShellDebug1Lib|ShellPkg\Library\UefiShellDebug1CommandsLib\UefiShellDebug1CommandsLib.inf   
# CDYU 20170627 after "UefiShellDebug1Lib", comiler required to add-in
   ShellCommandLib|ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
   BcfgCommandLib|ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf   
   UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
   HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf 
   TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
   PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
   PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
   DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
   DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
   ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
   HandleParsingLib|ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
# above are needed if want to use SMBIOS.c ->  SMBios64View()
  
# CDYU 20170630 for purley test
#    GbtGPIOLib|Chant\Chant\PurleyGPIO\GbtGPIOLib.inf   
   
   
   
#  LibC|StdLib/LibC/LibC.inf
#  LibStdio|StdLib/LibC/Stdio/Stdio.inf
#  DevShell|StdLib/LibC/Uefi/Devices/daShell.inf
   
# For Remote Setup follow Gigabyte Format -> JSON type  
  cjsonlib|MdePkg\Library\cJSON\cjson.inf
  #LzmaCompressLib|MdePkg\Library\LzmaCompress\LzmaCompress.inf
  
# 7z test
  LzmaDecompressLib|IntelFrameworkModulePkg\Library\LzmaCustomDecompressLib\LzmaCustomDecompressLib.inf 

# Zlib test
  Zlib|MdePkg\Library\Zlib\Zlib.inf 
  
# Tar test  
#  Tar|MdePkg\Library\Tar\Tar.inf 
 
# LibArchive test  
#  LibArchive|MdePkg\Library\LibArchive\LibArchive.inf  
  
# Lite Tar test  
  TarLite|MdePkg\Library\TarLite\TarLite.inf   
  
###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################

[Components]
#### Sample Applications.
   Smbus/SmbusHierarchy.inf


#Error C2220: warning is considered an error - no "object" file is generated
# for cJSON function 
[BuildOptions]  
#MSFT:*_*_*_CC_FLAGS = /w /Od /Gs32768
MSFT:*_*_*_CC_FLAGS = /nologo /c /WX /W4 /Gs1048576
#MSFT:DEBUG_*_IA32_CC_FLAGS = /Od

##############################################################################
#
#  Include Boilerplate text required for building with the Standard Libraries.
#
##############################################################################
!include StdLib/StdLib.inc

