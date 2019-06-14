#include <stdio.h>								
#include <stdlib.h>								// For use atoi
#include <string.h>								// For use strstr, strncpy, strtok...etc
#include <Library/UefiBootServicesTableLib.h>	// For use gBS
#include <ctype.h>
#include <Uefi.h>                               // Chant match James enviroment
#include <Library/ShellCEntryLib.h>           
#include <Library/UefiLib.h>                    // For use Printf	


#include  <Library/cJSON/cJSON.h>               // for JSON data structure
#include <Protocol/SmbusHc.h>					// For smbus protocol
#include <IndustryStandard/SmBus.h>				// For smbus protocol // w/o this Purly locateprotocol fail
#include <Library/SmbusLib.h>					// For smbus protocol // w/o this Purly locateprotocol fail


#define   SCREENTOLERENCE   19
#define   CHARSIZE   70
#define   CHARBOX    20
#define   SMBCOUNT   500

typedef struct {
	CHAR8*	ArgStr;
	UINT8	ArgNumNeed;
	EFI_STATUS (*func)(int argc, char ** argv);
} ARG_INPUT_LIST;

typedef struct {
    char     CharBuff[CHARSIZE];     
} CharStruct;

struct _NodeLinkList{
    UINT8                 SmbNodeAddress;
    UINT8                 SmbNodeChannel;
    UINT32                SmbNodeLayer;
    UINT32                OffsetPtr;
    struct _NodeLinkList  *Inherited;
};
typedef struct _NodeLinkList NodeLinkList;

typedef struct {
    UINT8 	              AddressFolder[20];
    UINT8 	              ChannelFolder[20];
    UINT32                NumberOfChain;
// OemDefined	
	char                  OemDefined[100][100];
    UINT8                 Reference[100];	
} ScanRuleFolder;

typedef struct {
    UINT8                 YuAddress;
    UINT8                 TraceSw[20];
    UINT8                 TraceCh[20];
    UINT32                NumberOfDevice; // Pointer | Offset
} SmbDevAttribute; 

typedef struct {
	UINT8                 NodeAddress;
	UINT8                 Dummy1;
	UINT8                 NodeChannel;
	UINT8                 Dummy2;
} NodeStruct;

typedef struct {
	char                  ModuleName[100];    
	char                  DeviceType[100];
	char                  DeviceID[100];
	UINT32                 JxAddress;
    UINT32                 Dummy1; // prvent sscanf x overwrite
    UINT32                 JxCh[20];	
    UINT32                 Dummy2; // prvent sscanf x overwrite	
	UINT32                 JxSw[20];
    UINT32                 Dummy3; // prvent sscanf x overwrite	
// OemDefined	
	char                  OemDefined[100][100];
    UINT8                 Reference[100];
// ConfigRule	
	UINT8                 RuleOffset[100];
	UINT8                 RuleValue[100];
	
} ExtJsonFile;

EFI_STATUS
HTC( int argc , const char *argv []);		


EFI_STATUS
Test(int argc, char* argv[]);

EFI_STATUS
NodeCompare (
  UINT8        SmbAddr,
  NodeStruct   *Node,
  UINT8        *Channel  
  );
  
EFI_STATUS
SmbusPorting(
  EFI_SMBUS_HC_PROTOCOL  *Smbus , 
  UINT8                  SmbAddr ,
  EFI_SMBUS_OPERATION    Operator ,
  UINT32                 Offset,
  UINT8                  *Register
  );
  
void  GUI_ScreenClean( void );

void  GUI_GetPosition( IN  UINTN   *x, IN  UINTN   *y);  
  
void  GUI_SetPosition( IN  UINTN   x, IN  UINTN   y);  
  
void  GUI_SetColor( IN  UINTN   color);  

void  GUI_WaitKey( IN	EFI_INPUT_KEY	*KeyValue);
  
void  GUI_Delay( IN  UINTN	Microsecond);  
  
void  GUI_Write(
    IN	   EFI_INPUT_KEY	*KeyValue,
	IN     UINTN            NumberOfInput,
    IN OUT CHAR16           *UnicodeBuffer
); 

void
LightBarSnd (
  IN      UINTN             Columns , 
  IN OUT  UINTN             Rows , 
  IN      UINT16            LightBarFeedBack ,
  IN OUT  UINT32            LightBarSpace, 
  IN      CharStruct*       sSmbTreeBuff
  );  

  
EFI_STATUS
RuLikeInput(
  IN OUT   CHAR16   *mUnicodeCharBuff, 
  IN       UINTN    mColumns, 
  IN       UINTN    mRows,
  IN       UINT32    rMode,
  IN       UINT8    *SmbrConfigBuf,
  IN       UINT32   OfsetConfig
);  
  
EFI_STATUS
SmbConfigSpace (
  EFI_SMBUS_HC_PROTOCOL   *mSmbus,
  SmbDevAttribute         *DevsAttr,
  UINT32                  CharsBoxIndex
  );  
  
void
LightBarAsciTrans(
  IN      UINT16 LightBarFeedBack,
  IN      UINT8  *SmbrConfigBuf,
  IN      UINTN  ConfigOffset,
  IN      UINT32 ByteWord
  ); 
 
void
LightBarSmbConfig (
  IN      UINTN   Columns , 
  IN OUT  UINTN   Rows , 
  IN      UINT8   *SmbrConfigBuf,
  IN      UINT16  LightBarFeedBack,
  IN OUT  UINT32  ConfigOffset,
  IN      UINT32  ByteWord
); 
 
 
void
AsciTransfer(
  IN      UINT8  *SmbrConfigBuf
  ); 
 
EFI_STATUS
LogOutputJson( 
  EFI_SMBUS_HC_PROTOCOL *mSmbus,
  IN  SmbDevAttribute   *DevsAttr, // !!!! change
  IN  CharStruct*       InhSmbTreeBuff,
  IN  UINT32*           InhKeyPointer                
 ); 
 
EFI_STATUS
ExfileImport(
  ExtJsonFile    *SmbExtFormat,
  NodeStruct     *eMode,
  UINT32         *DeviceAttrCount
  //int argc, char *argv[]
  ); 
  
EFI_STATUS
DumpConfigBIN(
  UINT8                   ConfigSize,      // 1: byte  / 2: word / 4: block 
  SmbDevAttribute         *DevsmAttr,
  UINT32                  CharsBoxmIndex,
  UINT8                   *mlBUffer,
  UINT32                  TurnsDump
);  
 /*
EFI_STATUS
DumpConfigBIN(
  IN      UINT8  IndicateSmbAddress,
  IN      UINT8  ConfigSize,      // 1: byte  / 2: word / 4: block 
  IN      UINT8  ReadWriteFlag,    // 2: write / 1: read to/from gBuffer[]
  IN      UINT8* DumSmbPath ,   // NULL means no trace
  IN      UINT8* DumsenSmbIo,       // NULL means no trace
  IN      UINT8* mlBUffer      
);

EFI_STATUS
LoadBinFile2SmbusCfg(int argc, char** argv);
 */
void
PrintError( void );

void
PrintSuccess( void );
  
void
ShowHelpMsg( void );

EFI_STATUS
CheckSmbusParameter(int argc, char** argv);

EFI_STATUS
Arg_r_Function(int argc, char** argv);

EFI_STATUS
Arg_w_Function(int argc, char** argv);