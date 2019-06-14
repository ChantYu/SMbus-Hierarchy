#include <SmbHierarchy.h>



EFI_SMBUS_HC_PROTOCOL   *gSmbus = NULL;
NodeStruct              *gNode = NULL;
ExtJsonFile             *gSmbExtFormat = NULL;
UINT32                  TurnsFileDump=0;
// console 
UINT8	                gSmbusOperation = 0;
UINT8	                gSmbusAddress = 0;
UINT8	                gSmbusOffset = 0;
UINT8	                *gBuffer = NULL;


ARG_INPUT_LIST ArgInputList[] = {
	{"-d",1 ,Test}, // DO NOT CHANGE THIS PRIORITY
	{"-g",1 ,Test}, // 
	{"-r", 4, Arg_r_Function},
	{"-w", 5, Arg_w_Function},	
	{"-h",1 ,HTC} // 
	//{"-l",2 ,LoadBinFile2SmbusCfg} // 
	//{"-t",1 ,ExfileImport }
};

EFI_STATUS
HTC( int argc , const char *argv[] ){
  const char *chyy="|E2-3|E6-0|E4-5";	
  UINT8 OutSw;
  UINT8 OutCh;
  UINT8 i;
  UINT8 len;
///////
  
  len = strlen(chyy)/(5*sizeof(char));
  
  for (i=0 ; i<len ;i++){
   //sscanf ( chyy ,"%[^|]%x%[^-]%x%[^|]",OutSw,OutCh );  	  
   //sscanf ( chyy ,"%*[^|]%[^-]-%[^|]",&OutSw,&OutCh );  	
   sscanf ( chyy ,"|%x-%x",&OutSw,&OutCh );  
   chyy += 5*sizeof(char); 
   printf("Result: %x %x \n",OutSw,OutCh);	  
  }
  return EFI_SUCCESS;	
	
}


void  GUI_ScreenClean( void )
{
	gST -> ConOut -> SetAttribute(gST -> ConOut, 0x0F);
	gST -> ConOut -> Reset(gST -> ConOut, 1);
  return;
}

void  GUI_GetPosition(
    IN  UINTN   *x,
    IN  UINTN   *y
)
{
    *x = gST->ConOut->Mode->CursorColumn;
    *y = gST->ConOut->Mode->CursorRow;
    return;
}

//Shift Position (ColumnsMAX:80 x RowsMAX:25)
void  GUI_SetPosition(
    IN  UINTN   x,
    IN  UINTN   y
)
{
  gST -> ConOut -> SetCursorPosition(gST -> ConOut, x, y);
  return;
}

//SetAttribute (High 4 bits = background, low 4 bits = font)
void  GUI_SetColor(
    IN  UINTN   color
)
{
	gST -> ConOut -> SetAttribute(gST -> ConOut, color);
	return;
}

//WaitKey
void  GUI_WaitKey(
    IN	EFI_INPUT_KEY	*KeyValue
)
{
	EFI_STATUS						Status;
	UINTN							Index;
	
	Status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
	if (EFI_ERROR(Status)) {
        Print(L"WaitKey: WaitForEvent Error!\n");
	}
	Status = gST->ConIn->ReadKeyStroke (gST->ConIn, KeyValue);
	if (EFI_ERROR(Status)) {
        Print(L"WaitKey: ReadKeyStroke Error!\n");
    }
	return;
}

void  GUI_Delay(
	IN  UINTN	Microsecond
)
{
	gBS->Stall(Microsecond);
	return;
}

//Input require
void  GUI_Write(
    IN	   EFI_INPUT_KEY	*KeyValue,
	IN     UINTN            NumberOfInput,
    IN OUT CHAR16           *UnicodeBuffer
)
{
	EFI_STATUS						Status;
	UINTN							Index;
	UINTN                           i;
//////////////////////////////////////////
	for (i=0 ; i<NumberOfInput ; i++)
	{
	  Status = gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &Index);
	  if (EFI_ERROR(Status)) {
          Print(L"WaitKey: WaitForEvent Error!\n");
	  }
	  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, KeyValue);
	  if (EFI_ERROR(Status)) {
          Print(L"WaitKey: ReadKeyStroke Error!\n");
      }
	  UnicodeBuffer[i] = KeyValue->UnicodeChar;
	}
	UnicodeBuffer[i] = '\0'; // set the boundary for UnicodeBuffer
}	


void
PrintError( void )  
{
	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTRED );
	Print(L"ERROR\n");
	gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE );
}

void
PrintSuccess( void )
{
	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGREEN );
	Print(L"SUCCESS\n");
	gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE );
}


void
ShowHelpMsg( void)
{
	gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
	////printf(CommonTitle, Title, Ver, DATE, TIME);
	printf("Commands:\n");
	printf("-g   \n");
	printf(" Create GUI interface and show Smbus Devices after scanning \n");
    printf("\n");	
	printf("-d   \n");
	printf(" Dump Smbus Hierarchy structure and all Smbus Device into SmbusInfo.json \n");	
	printf("\n");
	printf("-r <Operation> <Smbus address> <Offset>  \n");
    printf("  //Read smbus register with given offset \n");
    printf("   <Operation> Write 'Byte' or 'Word' register \n");
    printf("   <Smbus address> Target the slave address in 8 bits ex: 0xE2 \n");
    printf("   <Offset> The register location where user decide to write ex: 0x00   \n");
    printf("\n");
	printf("-w <Operation> <Smbus address> <Offset> <Register> \n");
    printf("  //Write smbus register with given offset and value \n");
    printf("   <Operation> Write 'Byte' or 'Word' register \n");
    printf("   <Smbus address> Target the slave address in 8 bits. ex: 0xE2 \n");
    printf("   <Offset> The register location where user decide to write. ex: 0x00  \n");
    printf("   <Register> The data that user decide to write. ex: 0x08 \n");
    printf("\n");
	//printf("  //Help  \n");
    //printf("\n");
}


void
AsciTransfer(
  IN      UINT8  *SmbrConfigBuf
  )
{
	UINT32                      i = 0;
	UINT8                       AsciPrint = 0;
    UINTN				        Columns=0,Rows=0;		        		
///////////
   GUI_SetPosition( 55 , 2 );
   GUI_GetPosition( &Columns , &Rows );
   for (i=0 ; i<0x100 ; i++){
	 AsciPrint  =  SmbrConfigBuf[i];
	 if ( (SmbrConfigBuf[i] < 33) || (SmbrConfigBuf[i] > 126) )
	   AsciPrint  =  46; // 0 => '.'	 
     GUI_SetPosition( Columns+(i%16) , Rows+(i/16) );
	 Print(L"%c",AsciPrint);
   }	 
}

void
LightBarAsciTrans(
  IN      UINT16 LightBarFeedBack,
  IN      UINT8  *SmbrConfigBuf,
  IN      UINTN  ConfigOffset,
  IN      UINT32 ByteWord) 
{
    UINTN				        Columns=0,Rows=0;
	UINT32                      mTempCfg = 0;
	UINT32                      mTempCfgW = 0;
///////////////////////////////
    GUI_SetColor(0x0F);
	GUI_SetPosition( 55 , 2 );
    GUI_GetPosition( &Columns , &Rows );
	Columns += (ConfigOffset%16);
	Rows    += (ConfigOffset/16);     
    GUI_SetPosition( Columns , Rows ); 	
	if (ByteWord){  // word
     // temp fail!! 
	  mTempCfg 	=  (UINT8) SmbrConfigBuf[ConfigOffset];
	  if ( (SmbrConfigBuf[ConfigOffset] < 33) || (SmbrConfigBuf[ConfigOffset] > 126) )
	    mTempCfg  =  46; // 0 => '.'		  
	  Print(L"%c",(UINT8)mTempCfg);		  
	  mTempCfgW 	=  (UINT8) SmbrConfigBuf[ConfigOffset+1];
	  if ( (SmbrConfigBuf[ConfigOffset+1] < 33) || (SmbrConfigBuf[ConfigOffset+1] > 126) )
	    mTempCfgW  =  46; // 0 => '.'		  
	  Print(L"%c",(UINT8)mTempCfgW);	
	}
    else{  // byte
	  mTempCfg 	=  (UINT8) SmbrConfigBuf[ConfigOffset];
	  if ( (SmbrConfigBuf[ConfigOffset] < 33) || (SmbrConfigBuf[ConfigOffset] > 126) )
	    mTempCfg  =  46; // 0 => '.'		  
	  Print(L"%c",(UINT8)mTempCfg);	
    }	
	
	switch (LightBarFeedBack)
	{
		case 0:
		{
			GUI_SetColor(0x70);  
			GUI_SetPosition(Columns,Rows);
		    if (ByteWord)	{
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset];
	           Print(L"%c",(UINT8)mTempCfg);	
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset+1];
	           Print(L"%c",(UINT8)mTempCfgW);					
			}
             // Print(L"%04x",mTempCfg); // temp fail!! 
	        else
		      Print(L"%c",(UINT8)mTempCfg);
		    GUI_SetColor(0x0F);			
			break;
		}
		case 1:
		{
		    if (ByteWord){
		      Rows--;
		      ConfigOffset-=16;					
		    }
		    else{
		      Rows--;
		      ConfigOffset-=16;	
		    }					
			GUI_SetColor(0x70);
			GUI_SetPosition(Columns,Rows);
		    if (ByteWord)	{
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset];
	           Print(L"%c",(UINT8)mTempCfg);	
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset+1];
	           Print(L"%c",(UINT8)mTempCfgW);					
			}
	        else
		      Print(L"%c",(UINT8)mTempCfg);
            GUI_SetColor(0x0F);			  
			break;
		}
		case 2:
		{
		    if (ByteWord){
		      Rows++;
		      ConfigOffset+=16;					
		    }
		    else{
		      Rows++;
		      ConfigOffset+=16;	
		    }					
			GUI_SetColor(0x70);
			GUI_SetPosition(Columns,Rows);
		    if (ByteWord)	{
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset];
	           Print(L"%c",(UINT8)mTempCfg);	
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset+1];
	           Print(L"%c",(UINT8)mTempCfgW);					
			}
	        else
		      Print(L"%c",(UINT8)mTempCfg);
            GUI_SetColor(0x0F);			  
			break;			
		}		
		case 3:
		{
		    if (ByteWord){
		      Columns+=2;
		      ConfigOffset++;					
		      ConfigOffset++;					
		    }
		    else{
		      Columns++;
		      ConfigOffset++;			
		    }
	        GUI_SetColor(0x70);
	        GUI_SetPosition(Columns,Rows);
		    if (ByteWord)	{
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset];
	           Print(L"%c",(UINT8)mTempCfg);	
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset+1];
	           Print(L"%c",(UINT8)mTempCfgW);					
			}
	        else
		      Print(L"%c",(UINT8)mTempCfg);
            GUI_SetColor(0x0F);				
			break;
		}	
		case 4:
		{
		    if (ByteWord){
		      Columns-=2;
		      ConfigOffset--;					
		      ConfigOffset--;					
		    }
		    else{
		      Columns--;
		      ConfigOffset--;			
		    }
	        GUI_SetColor(0x70);
	        GUI_SetPosition(Columns,Rows);
		    if (ByteWord)	{
	         //  mTempCfg 	=  (UINT8) gBuffer[ConfigOffset];
	           Print(L"%c",(UINT8)mTempCfg);	
	          // mTempCfg 	=  (UINT8) gBuffer[ConfigOffset+1];
	           Print(L"%c",(UINT8)mTempCfgW);					
			}
	        else
		      Print(L"%c",(UINT8)mTempCfg);
            GUI_SetColor(0x0F);				
			break;
		}		
        default:
        {
			break;
		}		
	}
}

void
LightBarSmbConfig (
  IN      UINTN   Columns , 
  IN OUT  UINTN   Rows , 
  IN      UINT8   *SmbrConfigBuf,
  IN      UINT16  LightBarFeedBack,
  IN OUT  UINT32  ConfigOffset,
  IN      UINT32  ByteWord
  ) 
{
  UINT32       mTempCfg = 0;
///////////////////////////////////////////////////
	
	
	
	GUI_SetColor(0x0F);
	GUI_SetPosition(Columns,Rows);
	
	if (ByteWord){	
	  mTempCfg 	=  (UINT16)(SmbrConfigBuf[ConfigOffset+1]<<8) | (SmbrConfigBuf[ConfigOffset]) ;
	  Print(L"%04x",(UINT16)mTempCfg);	
	}
    else{
	  mTempCfg 	=  (UINT8) SmbrConfigBuf[ConfigOffset];
	  Print(L"%02x",(UINT8)mTempCfg);	
    }
	switch (LightBarFeedBack)
	{
	  case 0:
	  {
		GUI_SetColor(0x70);  
		GUI_SetPosition(Columns,Rows);
		if (ByteWord)	
          Print(L"%04x",mTempCfg);
	    else
		  Print(L"%02x",mTempCfg);
		GUI_SetColor(0x0F);
		break;
	  }
      case 1:
	  {
		if (ByteWord){
		  Rows--;
		  ConfigOffset-=16;					
		}
		else{
		  Rows--;
		  ConfigOffset-=16;	
		}		 
	    GUI_SetColor(0x70);
	    GUI_SetPosition(Columns,Rows);
		if (ByteWord)	
          Print(L"%04x",mTempCfg);
	    else
		  Print(L"%02x",mTempCfg);
        GUI_SetColor(0x0F);		
		break;
	  } 	  
      case 2:
	  {		  
		if (ByteWord){
		  Rows++;
		  ConfigOffset+=16;					
		}
		else{
		  Rows++;
		  ConfigOffset+=16;	
		}		  
	    GUI_SetColor(0x70);
	    GUI_SetPosition(Columns,Rows);
		if (ByteWord)	
          Print(L"%04x",mTempCfg);
	    else
		  Print(L"%02x",mTempCfg);
        GUI_SetColor(0x0F);		
		break;
	  } 		  
      case 4:
	  {
		if (ByteWord){
		  Columns-=5;
		  ConfigOffset--;					
		  ConfigOffset--;					
		}
		else{
		  Columns-=3;
		  ConfigOffset--;			
		}
	    GUI_SetColor(0x70);
	    GUI_SetPosition(Columns,Rows);
		if (ByteWord)	
          Print(L"%04x",mTempCfg);
	    else
		  Print(L"%02x",mTempCfg);
        GUI_SetColor(0x0F);		
		break;
	  } 
	  case 3:
	  {
		if (ByteWord){
		  Columns+=5;
		  ConfigOffset++;					
		  ConfigOffset++;					
		}
		else{
		  Columns+=3;
		  ConfigOffset++;			
		}
	    GUI_SetColor(0x70);
	    GUI_SetPosition(Columns,Rows);
		if (ByteWord)	
          Print(L"%04x",mTempCfg);
	    else
		  Print(L"%02x",mTempCfg);
        GUI_SetColor(0x0F);		
		break;
	  } 
	  default:
	  {
	    break;
	  }
	} // switch (LightBarFeedBack)
	
}


EFI_STATUS
RuLikeInput(
  IN OUT   CHAR16   *mUnicodeCharBuff, 
  IN       UINTN    mColumns, 
  IN       UINTN    mRows,
  IN       UINT32    rMode,
  IN       UINT8    *SmbrConfigBuf,
  IN       UINT32    OfsetConfig  
)  
{
  EFI_INPUT_KEY		        *Key; 
  CHAR16                    *mTempUnicodeCharBuff;  // without malloc, memory get wrong Unicode data!!!	
  CHAR16                    mTempChar;
  UINT8                     i = 0 , j, k;
  UINT8                     KeyInNumber=0;
  UINT32                    maTempCfg=0;  
///////////////////////////////	

  switch ( rMode ){
	 case 0:    //byte
	  KeyInNumber =2;   
	  maTempCfg  =  SmbrConfigBuf[OfsetConfig];
	  GUI_SetPosition(mColumns,mRows);	
	  GUI_SetColor(0x40);
	  Print(L"%02x",(UINT8)maTempCfg);	   
	   
	   
	   break;
	 case 1:    //word
	   KeyInNumber =4; 
	   maTempCfg 	=  ((UINT32)(SmbrConfigBuf[OfsetConfig+1]<<8)) | (UINT32)(SmbrConfigBuf[OfsetConfig]) ;
	   GUI_SetPosition(mColumns,mRows);
	   GUI_SetColor(0x40);
	   Print(L"%04x",(UINT16)maTempCfg);	   
	   break;	
     default:
	   break;		
  }


  mTempUnicodeCharBuff = malloc( (KeyInNumber+1) * sizeof(CHAR16));  // not sure 4+1 || 2+1
  Key                  = malloc(sizeof(EFI_INPUT_KEY));
  
  
//   blank buffer
  GUI_SetColor(0x40); // back:red4 & char:green0
  for ( j=0 ; j<KeyInNumber ; j++){
	mTempUnicodeCharBuff[j] = '0' ;	
  }	
  mTempUnicodeCharBuff[KeyInNumber] = '\0';
  k = KeyInNumber-1;

  
while (1){	
    GUI_WaitKey(Key);
  if ( (Key->ScanCode == 23)){  //ScanCode:ESC  -> leave the loop

	//GUI_SetPosition( mColumns , mRows );
	//Print(L"%02x",mConfig);
    free(mTempUnicodeCharBuff); // 20180510 debug!!
    free(Key);		
    return EFI_OUT_OF_RESOURCES;		
  }	
  if ( i < KeyInNumber ){		
	if ( (Key->ScanCode == 0) && (Key->UnicodeChar != 0) )  // ScanCode: Enter
	{
		mTempChar = Key->UnicodeChar;
		if ( (mTempChar<58) && (mTempChar>47) ) {
		  GUI_SetPosition( mColumns , mRows );
		  //mTempUnicodeCharBuff[k]= mTempChar;

		  for ( j=0 ; j<k ; j++ ){ // ?	
		    mTempUnicodeCharBuff[j] = mTempUnicodeCharBuff[j+1]; 			  
		  }
		  mTempUnicodeCharBuff[k]= mTempChar; 	  
		  
		  Print(L"%s",mTempUnicodeCharBuff);
		  
		  i++;
		}	
		if ( (mTempChar<103) && (mTempChar>96) ) { // alpha S
		   GUI_SetPosition( mColumns , mRows );		
		   //mTempChar -= 32; // alpha S->L

		  for ( j=0 ; j<k ; j++ ){
		    mTempUnicodeCharBuff[j] = mTempUnicodeCharBuff[j+1]; 			  
		  }
		  mTempUnicodeCharBuff[k]= mTempChar;
		  
		  Print(L"%s",mTempUnicodeCharBuff);
		  
		  i++;
		}
		if ( (mTempChar<71) && (mTempChar>64) ) { // alpha L
		  GUI_SetPosition( mColumns , mRows );		
		 // mTempUnicodeCharBuff[k]= mTempChar;
		  
		  for ( j=0 ; j<k ; j++ ){
		    mTempUnicodeCharBuff[j] = mTempUnicodeCharBuff[j+1]; 			  
		  }
		  mTempUnicodeCharBuff[k]= mTempChar;			  
		  Print(L"%s",mTempUnicodeCharBuff);  
		  i++;
	    }	
	}	
  }
  if (( Key->ScanCode == 0 ) && (Key->UnicodeChar == 13)){	  
    //  save(copy) & output
	memcpy( mUnicodeCharBuff , mTempUnicodeCharBuff , (KeyInNumber+1)*sizeof(CHAR16) );
    free(mTempUnicodeCharBuff);
    free(Key);	
	return EFI_SUCCESS;
  } 

}
  //GUI_SetColor(0x0F);  
  //GUI_SetPosition( mColumns , mRows );	
  //Print(L"%s",mUnicodeCharBuff); 
  
  
  free(mTempUnicodeCharBuff);
  free(Key);
  return EFI_SUCCESS;	
}

EFI_STATUS
SmbConfigSpace (
  EFI_SMBUS_HC_PROTOCOL   *mSmbus,
  SmbDevAttribute         *DevsAttr,
  UINT32                  CharsBoxIndex
  )
{
  UINT8               *SmbConfigBuf=NULL;
  //UINT32              Layers=0;	
  UINT32              i=0;	
  UINT32              LenthsSwitch=0;	
  UINT32              ByteWordSw=0;	
  UINT32              BitsBuffer=0;	
  UINT32              DummysBuffer=0;	
  EFI_STATUS	      Status;	
  EFI_INPUT_KEY	      *Key=NULL;
// GUI   
  UINTN				  Columns=0;
  UINTN				  Rows=0;	  
  UINT32			  ConfigOffset=0;	
  CHAR16              *UnicodeCharBuff=NULL;
  char                *ASCIICharBuff=NULL;
 
///////////////////////////	  
//////////////////////////////////////	
  SmbConfigBuf    = malloc(1000*sizeof(UINT8)); 
  Key             = malloc(sizeof(EFI_INPUT_KEY));
  ASCIICharBuff   = malloc(CHARSIZE*sizeof(char));
  UnicodeCharBuff = malloc(CHARSIZE*sizeof(CHAR16));
  
  while ( DevsAttr[CharsBoxIndex].TraceSw[LenthsSwitch] != 0xFF ){
	BitsBuffer = 0;  
	BitsBuffer = 1 << DevsAttr[CharsBoxIndex].TraceCh[LenthsSwitch]; 
    Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].TraceSw[LenthsSwitch] , EfiSmbusWriteByte, 0 , (UINT8*) &BitsBuffer );    	  
	LenthsSwitch++;  
  }
// purpose
	  if ( DevsAttr[CharsBoxIndex].YuAddress == 0x60 || DevsAttr[CharsBoxIndex].YuAddress == 0x64 || DevsAttr[CharsBoxIndex].YuAddress == 0x68 ){
        UINT8    *zeorz;
		*zeorz = 0;
		Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusWriteByte, 0xFF , zeorz);
		  
	  }
  
  
  Columns = 6;
  Rows    = 2;
  for (i=0 ; i<256 ; i++){
    Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusReadByte, i , (UINT8*) &SmbConfigBuf[i] );
  }	  
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
  Print(L"      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
  Print(L"      ===============================================\n");	
  for(i = 0; i < 0x10; i++){
    Print(L"%02X || %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
		  i*0x10, SmbConfigBuf[i*0x10+0], SmbConfigBuf[i*0x10+1], SmbConfigBuf[i*0x10+2], SmbConfigBuf[i*0x10+3], SmbConfigBuf[i*0x10+4], SmbConfigBuf[i*0x10+5], SmbConfigBuf[i*0x10+6], SmbConfigBuf[i*0x10+7],
		  SmbConfigBuf[i*0x10+8], SmbConfigBuf[i*0x10+9], SmbConfigBuf[i*0x10+10], SmbConfigBuf[i*0x10+11], SmbConfigBuf[i*0x10+12], SmbConfigBuf[i*0x10+13], SmbConfigBuf[i*0x10+14], SmbConfigBuf[i*0x10+15]);
  }    
  
  
  
  
  
  
  for ( i=LenthsSwitch; i>0 ;i-- ){
    DummysBuffer=0;
    Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].TraceSw[i-1] , EfiSmbusWriteByte, 0 , (UINT8*) &DummysBuffer );    	      	
  }    
  
  
  
  while(1){
	GUI_SetColor( 0xB2 );
	GUI_SetPosition (0,20);
	Print(L"============== Smbus Configuration =============================================");
	GUI_SetPosition (0,21);
	Print(L"                                                                                ");		
	GUI_SetPosition (0,22);
	Print(L"                                                                                ");		
	GUI_SetPosition (0,23);
	Print(L"                                                                                ");
    GUI_SetPosition (0,21);
    Print(L" Smbus Address : 0x%02x \n",DevsAttr[CharsBoxIndex].YuAddress);
	Print(L" F7: Word/Byte convert  Enter: Set Configuration         \n");		
	Print(L" F2: Pour F3: Dump BIN");	  
	  
	// ASCII map
	GUI_SetColor( 0x0F );
	AsciTransfer( SmbConfigBuf);
	
	// reload the smb config and give the light bar
	LightBarAsciTrans( 0 , SmbConfigBuf , ConfigOffset , ByteWordSw );	
    LightBarSmbConfig(Columns,Rows,SmbConfigBuf,0,ConfigOffset ,ByteWordSw);
	  
	GUI_WaitKey(Key);  
	if(Key->ScanCode == 23){  //ScanCode:ESC -> leave the loop
	  break;
	}	 
    if(Key->ScanCode == 1){  // up
	  if ( Rows == 2 ){               //boundary
        continue;
	  }	
      LightBarSmbConfig(Columns,Rows,SmbConfigBuf,Key->ScanCode,ConfigOffset,ByteWordSw);
	  LightBarAsciTrans( Key->ScanCode , SmbConfigBuf , ConfigOffset , ByteWordSw );
	  if (ByteWordSw){
		Rows--;
		ConfigOffset-=16;					
	  }
	  else{
		Rows--;
		ConfigOffset-=16;	
	  }			  
	}	
    if(Key->ScanCode == 2){  // down
	  if ( Rows == 17 ){               //boundary
        continue;
	  }	
 	  LightBarSmbConfig(Columns,Rows,SmbConfigBuf,Key->ScanCode,ConfigOffset,ByteWordSw);
	  LightBarAsciTrans( Key->ScanCode , SmbConfigBuf , ConfigOffset , ByteWordSw );
	  if (ByteWordSw){
		Rows++;
		ConfigOffset+=16;					
	  }
	  else{
		Rows++;
		ConfigOffset+=16;	
      }	
    }		
    if(Key->ScanCode == 3){  // right
      if ( (ConfigOffset%16 == 15) || ( (ConfigOffset%16 == 14) && ByteWordSw == 1 ) ){               //boundary coherent with ip/down!! Need find a another method
        continue;
	  }	  
	  LightBarSmbConfig(Columns,Rows, SmbConfigBuf ,Key->ScanCode,ConfigOffset,ByteWordSw);
	  LightBarAsciTrans( Key->ScanCode , SmbConfigBuf , ConfigOffset , ByteWordSw );
	  if (ByteWordSw){
	    Columns += 5;
	    ConfigOffset+=2;				
	  }
	  else{
		Columns += 3;
		ConfigOffset++;	
	  }			  
	}		
    if(Key->ScanCode == 4){  // left
      if ( ConfigOffset%16 == 0 ){               //boundary NOT coherent with ip/down!! Need find a another method
          continue;
	  }	  
	  LightBarSmbConfig(Columns,Rows, SmbConfigBuf ,Key->ScanCode,ConfigOffset,ByteWordSw);
	  LightBarAsciTrans( Key->ScanCode , SmbConfigBuf , ConfigOffset , ByteWordSw );
	  if (ByteWordSw){
		Columns -= 5;
		ConfigOffset -= 2;				
	  }
	  else{
		Columns -= 3;
		ConfigOffset--;	
	  }			  	  
	}				
	if(( Key->ScanCode == 0 ) && (Key->UnicodeChar == 13)){ // Enter  
	  //maTempCfg  =  SmbConfigBuf[ConfigOffset];
	  GUI_SetPosition(Columns,Rows);	
	  GUI_SetColor(0x40);
	  //Print(L"%02x",(UINT8)maTempCfg);
	  Status = RuLikeInput( UnicodeCharBuff , Columns , Rows , ByteWordSw , SmbConfigBuf , ConfigOffset );		      
	  if(EFI_ERROR(Status)){
		continue;
	  }	 	  
	  UnicodeStrToAsciiStr (UnicodeCharBuff , ASCIICharBuff);
	  if ( (strcmp( ASCIICharBuff , "00"  ) == 0) || (strcmp( ASCIICharBuff , "0000" ) == 0) ){
		DummysBuffer =  0; 
	  }
	  else{
		DummysBuffer = (UINT32) strtoul( ASCIICharBuff ,  NULL , 16);  
	  }
	  
	  for ( i=0; i<LenthsSwitch ;i++ ){
	    BitsBuffer = 0;  
	    BitsBuffer = 1 << DevsAttr[CharsBoxIndex].TraceCh[i]; 
        Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].TraceSw[i] , EfiSmbusWriteByte, 0 , (UINT8*) &BitsBuffer );    	  
      }  	  
	  
	  if (ByteWordSw){	// word
        Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusWriteWord, ConfigOffset , (UINT8*) &DummysBuffer ); 
	  } 
	  else{ // byte
		Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusWriteByte, ConfigOffset , (UINT8*) &DummysBuffer ); 
	  }	
	  if(EFI_ERROR(Status)){
		 Print(L"Access Failure\n");
		 gBS->Stall(1000000); // delay 1 m second	
	  }	   
	  GUI_ScreenClean();
	  GUI_SetPosition(0, 0);	
      
	  
	  switch( ByteWordSw ){
		case 0: // byte  
          for (i=0 ; i<256 ; i++){
            Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusReadByte, i , (UINT8*)&SmbConfigBuf[i] );
          }	  
	      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);		  
          Print(L"      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
          Print(L"      ===============================================\n");	
          for(i = 0; i < 0x10; i++){
            Print(L"%02X || %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
		          i*0x10, SmbConfigBuf[i*0x10+0], SmbConfigBuf[i*0x10+1], SmbConfigBuf[i*0x10+2], SmbConfigBuf[i*0x10+3], SmbConfigBuf[i*0x10+4], SmbConfigBuf[i*0x10+5], SmbConfigBuf[i*0x10+6], SmbConfigBuf[i*0x10+7],
		          SmbConfigBuf[i*0x10+8], SmbConfigBuf[i*0x10+9], SmbConfigBuf[i*0x10+10], SmbConfigBuf[i*0x10+11], SmbConfigBuf[i*0x10+12], SmbConfigBuf[i*0x10+13], SmbConfigBuf[i*0x10+14], SmbConfigBuf[i*0x10+15]);
          }   	  
		  break;
        case 1: // word
          for (i=0 ; i<256 ; i++){
            Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusReadWord, i , (UINT8*)&SmbConfigBuf[i] );
          }			    
		  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
	      Print(L"        00   02   04   06   08   0A   0C   0E\n");
		  Print(L"      =======================================\n");
	      for(i = 0; i < 0x10; i++){
			Print(L"%02X || %04X %04X %04X %04X %04X %04X %04X %04X\n",
				  i*0x10,
				  (UINT16)SmbConfigBuf[i*0x10+1] << 8 | SmbConfigBuf[i*0x10+0], (UINT16)SmbConfigBuf[i*0x10+3] << 8 | SmbConfigBuf[i*0x10+2], (UINT16)SmbConfigBuf[i*0x10+5] << 8 | SmbConfigBuf[i*0x10+4], (UINT16)SmbConfigBuf[i*0x10+7] << 8 | SmbConfigBuf[i*0x10+6],
				  (UINT16)SmbConfigBuf[i*0x10+9] << 8 | SmbConfigBuf[i*0x10+8], (UINT16)SmbConfigBuf[i*0x10+11] << 8 | SmbConfigBuf[i*0x10+10], (UINT16)SmbConfigBuf[i*0x10+13] << 8 | SmbConfigBuf[i*0x10+12], (UINT16)SmbConfigBuf[i*0x10+15] << 8 | SmbConfigBuf[i*0x10+14]);
		  }		
          break;
        default:
		  break;   
	  }	  
	  
	  for ( i=LenthsSwitch; i>0 ;i-- ){
        DummysBuffer=0;
        Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].TraceSw[i-1] , EfiSmbusWriteByte, 0 , (UINT8*) &DummysBuffer );    	      	
      }    
	  
	}
    if(Key->ScanCode == 13){  // F3  
		   // prevent crash...20180423
		   //if (TempMaskforPrevCrash)
			//  continue; 
		   // from gbuffer to txt
		   //continue; // mask due to bug...
	  GUI_SetPosition (8,19);		   
	  Print(L"Loading the SMB 0x%02x Configuration Table",DevsAttr[CharsBoxIndex].YuAddress);			   
	  gBS->Stall(1000000); // delay 1 m second		   
	  
	  // Purpose !!! 

	  
	  
	  Status = DumpConfigBIN(1, DevsAttr, CharsBoxIndex , SmbConfigBuf , TurnsFileDump );
      TurnsFileDump++;
	  GUI_SetPosition (8,19);
	  Print(L"                                                 ");
	}
	if(Key->ScanCode == 17 ){  // F7
	  GUI_ScreenClean();
	  GUI_SetPosition(0, 0);  	
      ByteWordSw++;
	  ByteWordSw = ByteWordSw%2;
	  
      switch( ByteWordSw ){
        case 0: // byte  
          for(i = 0; i < 0x100; i++){  
		    Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusReadByte, i , (UINT8*)&SmbConfigBuf[i] );
		  }  
	      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);		  
          Print(L"      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
          Print(L"      ===============================================\n");	
          for(i = 0; i < 0x10; i++){
            Print(L"%02X || %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
		          i*0x10, SmbConfigBuf[i*0x10+0], SmbConfigBuf[i*0x10+1], SmbConfigBuf[i*0x10+2], SmbConfigBuf[i*0x10+3], SmbConfigBuf[i*0x10+4], SmbConfigBuf[i*0x10+5], SmbConfigBuf[i*0x10+6], SmbConfigBuf[i*0x10+7],
		          SmbConfigBuf[i*0x10+8], SmbConfigBuf[i*0x10+9], SmbConfigBuf[i*0x10+10], SmbConfigBuf[i*0x10+11], SmbConfigBuf[i*0x10+12], SmbConfigBuf[i*0x10+13], SmbConfigBuf[i*0x10+14], SmbConfigBuf[i*0x10+15]);
          }   	  
		  break;
		case 1: // word 
          for(i = 0; i < 0x100; i++){  
		    Status = SmbusPorting( mSmbus, DevsAttr[CharsBoxIndex].YuAddress , EfiSmbusReadWord, i , (UINT8*)&SmbConfigBuf[i] );		  
		  } 
		  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
	      Print(L"        00   02   04   06   08   0A   0C   0E\n");
		  Print(L"      =======================================\n");
	      for(i = 0; i < 0x10; i++){
		    Print(L"%02X || %04X %04X %04X %04X %04X %04X %04X %04X\n",
				  i*0x10,
				  (UINT16)SmbConfigBuf[i*0x10+1] << 8 | SmbConfigBuf[i*0x10+0], (UINT16)SmbConfigBuf[i*0x10+3] << 8 | SmbConfigBuf[i*0x10+2], (UINT16)SmbConfigBuf[i*0x10+5] << 8 | SmbConfigBuf[i*0x10+4], (UINT16)SmbConfigBuf[i*0x10+7] << 8 | SmbConfigBuf[i*0x10+6],
				  (UINT16)SmbConfigBuf[i*0x10+9] << 8 | SmbConfigBuf[i*0x10+8], (UINT16)SmbConfigBuf[i*0x10+11] << 8 | SmbConfigBuf[i*0x10+10], (UINT16)SmbConfigBuf[i*0x10+13] << 8 | SmbConfigBuf[i*0x10+12], (UINT16)SmbConfigBuf[i*0x10+15] << 8 | SmbConfigBuf[i*0x10+14]);
		  }				
		  break;		
        default:
		  break; 	  
	  }
	  Columns  = 6;	
	  Rows     = 2;	
      ConfigOffset = 0;  // reset!! for safty
	  LightBarAsciTrans( Key->ScanCode , SmbConfigBuf , ConfigOffset , ByteWordSw );	
      LightBarSmbConfig(Columns,Rows,SmbConfigBuf,Key->ScanCode,ConfigOffset ,ByteWordSw);	  
	}
  }
  
  
  if( SmbConfigBuf != NULL )
    free(SmbConfigBuf);
  if( Key != NULL )
    free(Key);
  if( UnicodeCharBuff != NULL )
    free(UnicodeCharBuff);
  if( ASCIICharBuff != NULL )
    free(ASCIICharBuff);

  return EFI_SUCCESS;  
}

void
LightBarSnd (
  IN      UINTN             Columns , 
  IN OUT  UINTN             Rows , 
  IN      UINT16            LightBarFeedBack ,
  IN OUT  UINT32             LightBarSpace, 
  IN      CharStruct*       sSmbTreeBuff
  )
{
//	UINTN   TempColumns;  no use for temp
	//CHAR16               trynerror[CHARSIZE];   // CHARSIZE follow type struct number
	CHAR16               *trynerror;   // CHARSIZE follow type struct number
///////////////////////////////////////////////////

    trynerror = malloc( CHARSIZE * sizeof(CHAR16) );


	GUI_SetColor(0x0F);
	GUI_SetPosition(Columns,Rows);
	AsciiStrToUnicodeStr( sSmbTreeBuff[LightBarSpace].CharBuff , trynerror);
	Print(L"%s ",trynerror);	

	switch (LightBarFeedBack)
	{
	  case 0:
	  {
		GUI_SetColor(0x70);  
		GUI_SetPosition(Columns,Rows);
		AsciiStrToUnicodeStr( sSmbTreeBuff[LightBarSpace].CharBuff , trynerror);
        Print(L"%s ",trynerror);
		GUI_SetColor(0x0F);
		break;
	  }
      case 1:
	  {
		Rows--;
		LightBarSpace--;
	    GUI_SetColor(0x70);
	    GUI_SetPosition(Columns,Rows);
	    AsciiStrToUnicodeStr( sSmbTreeBuff[LightBarSpace].CharBuff , trynerror);
	    Print(L"%s ",trynerror);
        GUI_SetColor(0x0F);		
		break;
	  } 
	  case 2:
	  {
		Rows++;
		LightBarSpace++;
	    GUI_SetColor(0x70);
	    GUI_SetPosition(Columns,Rows);
	    AsciiStrToUnicodeStr( sSmbTreeBuff[LightBarSpace].CharBuff , trynerror);
	    Print(L"%s ",trynerror);
        GUI_SetColor(0x0F);		
		break;
	  } 
	  default:
	  {
	    break;
	  }
	} // switch (LightBarFeedBack)
	
	
	free(trynerror);
}

EFI_STATUS
SmbusPorting(
  EFI_SMBUS_HC_PROTOCOL  *Smbus , 
  UINT8                  SmbAddr ,
  EFI_SMBUS_OPERATION    Operator ,
  UINT32                 Offset,
  UINT8                 *Register
  )
{
  EFI_STATUS	                Status;	
  EFI_SMBUS_DEVICE_ADDRESS   	SlaveAddress;
  UINTN	                        Length = 1;	
////////////////////////  
// Step 1. transfer Addr 8bits into 7 bits
  SlaveAddress.SmbusDeviceAddress = SmbAddr >> 1;
// Step 2. Prepare Smbus Function Variable 
  if ( (Operator == EfiSmbusReadByte) || (Operator == EfiSmbusWriteByte) ){
	 Length = 1; 
  }
  else if ( (Operator == EfiSmbusReadWord) || (Operator == EfiSmbusWriteWord) ){
	 Length = 2;  
  }  
  
// Step 3. Use Smbus Execute Function  
  Status = Smbus->Execute (
    Smbus,
    SlaveAddress,
    Offset,
    Operator,
    FALSE,
    &Length,
    Register
  );
  if(EFI_ERROR(Status)){
	//printf("Smnbus Function Fail!\n");
    return Status; 
  }
  return EFI_SUCCESS;  
}

EFI_STATUS
NodeCompare (
  UINT8        SmbAddr,
  NodeStruct   *Node,
  UINT8        *Channel
  )
{
  UINT32       i=0;
/////////////////////////	
  //for( i=0 ; i<sizeof(Node)/sizeof(Node[0]) ; i++ ){
	    
  for( i=0 ; i<10 ; i++ ){    // danger!!
	  	  //printf("Smb:%02x Node:%02x ",SmbAddr ,Node[i].NodeAddress);	
    if ( SmbAddr == Node[i].NodeAddress ){

      memcpy( Channel , &Node[i].NodeChannel , sizeof(UINT32));
      return EFI_SUCCESS;
	}
  }	
  return EFI_INVALID_PARAMETER;	
}


EFI_STATUS
main(int argc, char* argv[])
{
   //UINT32       ArgInputNumber = 0;
   UINT8        i = 0;
   EFI_STATUS   Status;	  
/////////////////////////////////   
   for(i = 0; i < sizeof(ArgInputList)/sizeof(ArgInputList[0]); i++){
	 if(strcasecmp(argv[1], ArgInputList[i].ArgStr) == 0){
	   if((argc-1) >= ArgInputList[i].ArgNumNeed){
	     Status = ArgInputList[i].func(argc, argv);
	     break;
       }	
       else{
	     gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTRED);
         return EFI_INVALID_PARAMETER;
       }       
     } // if(strcasecmp(argv[1], ArgInputList[i].ArgStr) == 0)
   }
   if(Status != EFI_SUCCESS){
     ShowHelpMsg();
	 return EFI_INVALID_PARAMETER;
   }   
   return EFI_SUCCESS;
}

EFI_STATUS
Test(int argc, char* argv[])
{
   UINT32            DummyBuffer=0; 
   UINT32            i=0; 
   UINT32            iDWORD=0; 
   UINT32            j=0; 
   UINT32            k=0; 
   UINT32            m=0; 
   UINT32            s=0; 
   UINT32            z=0; 
   UINT32            Count=0; 
   EFI_STATUS	    Status;	
   UINT8            *CloseLoopScnRes=NULL;
   NodeLinkList     *hierarchyTableLink=NULL;
   NodeLinkList     *TempNodeStruct=NULL;
   UINT32            ListNumber=0; 
   UINT32            ListShiftNumber=0; 
   UINT8            SpeificChannel=0;   
   UINT32            BitBuffer=0;
   UINT32            LinkTablePtr=0;
   //UINT8            InheritedFlag=0;
   UINT8            *TraceSwAddress=NULL;
   UINT8            *TraceSwChannel=NULL;
   UINT32            BaseSwitchCount=0;
   UINT32            TracePtr=0; 
   UINT32            AfterCou=0; 
   UINT32            NodeLyaerTerms=0; 
   UINT32            ColumnFlag=0;
   UINT32            RowFlag=0;
   ScanRuleFolder   *CascadeScanTable=NULL;
   SmbDevAttribute  *DevAttr=NULL;
   UINT32           SmbusDeviceCount=0;
   UINT32           SimpleScanLoop=0;
   UINT8            *SimpleScanBuffer=NULL;
   UINT8            *ColMatrix=NULL;
// Step 5
   UINT32           GuiJump = 0;
   UINT32     ScreenListOffset=0;
   UINTN		    Columns=0;
   UINTN			Rows=0;	   
   UINT32           CharBoxIndex = 0;
   UINT32           ExfileDeviceCount = 0;
   EFI_INPUT_KEY	*Key=NULL;
   CharStruct       *GuiCharBox=NULL;
   //UINT8            ChantDebug=0; 
   char             *TempASCIIChar=NULL;
   CHAR16           *TempUnicodeChar=NULL;
   UINT32           TraceGroup=0;
   UINT32           GroupBoundryFlag=0;
   UINT32           moduleDiffFlag=0;
/////////////////////////////////
  
  TempASCIIChar      = malloc( CHARSIZE * sizeof(char));
  TempUnicodeChar    = malloc( CHARSIZE * sizeof(CHAR16));
  
  CloseLoopScnRes    = malloc(200*sizeof(UINT8));
  hierarchyTableLink = malloc(200*sizeof(NodeLinkList));
  TempNodeStruct     = malloc(1*sizeof(NodeLinkList));
  gNode              = malloc(10*sizeof(NodeStruct)); // 
  gSmbExtFormat      = malloc(500*sizeof(ExtJsonFile)); // 
  TraceSwAddress     = malloc(50*sizeof(UINT8));
  TraceSwChannel     = malloc(50*sizeof(UINT8));
  SimpleScanBuffer   = malloc(50*sizeof(UINT8));
  //ColMatrix          = malloc(10*sizeof(UINT8));
  ColMatrix          = malloc(10*sizeof(UINT8));
  Key                = malloc(sizeof(EFI_INPUT_KEY));
  GuiCharBox         = malloc(SMBCOUNT*sizeof(CharStruct));
  
  for (i=0 ; i<SMBCOUNT ; i++ ){
	GuiCharBox[i].CharBuff[0] = '\0';    
  }
  
  for (i=0 ; i<200 ; i++){
    hierarchyTableLink[i].Inherited = NULL; 	  
  } 	
	
  // reset/clear the buffer	
  for (i=0 ; i<500 ;i++){
	gSmbExtFormat[i].ModuleName[0] = '\0';  
	gSmbExtFormat[i].DeviceType[0] = '\0';  
	gSmbExtFormat[i].DeviceID[0] = '\0';  
	for (j=0 ; j<100 ; j++){
	  gSmbExtFormat[i].OemDefined[j][0] = '\0'; 		
	} 
	gSmbExtFormat[i].JxAddress = 0xAF;  
	//gSmbExtFormat[i].Reference[0] = 0xAF;  
	memset( gSmbExtFormat[i].Reference , 0xAF , 100 );  
	//gSmbExtFormat[i].RuleOffset[0] = 0xAF;  
	//gSmbExtFormat[i].RuleValue[0] = 0xAF;  
	memset( gSmbExtFormat[i].RuleOffset , 0xAF , 100 );  
	memset( gSmbExtFormat[i].RuleValue , 0xAF , 100 );   
	//gSmbExtFormat[i].JxSw[0] = 0xAF;  
	//gSmbExtFormat[i].JxCh[0] = 0xAF;  
	//memset( (UINT8*) gSmbExtFormat[i].JxSw , 0xAF , 20 );  // BUG 0xAFAFAFAF
	//memset( (UINT8*) gSmbExtFormat[i].JxCh , 0xAF , 20 );  // BUG 0xAFAFAFAF  
	for (j=0 ; j<20 ; j++){
	  gSmbExtFormat[i].JxSw[j] = 0xAF;
	  gSmbExtFormat[i].JxCh[j] = 0xAF;
	  
	}   
  }	

  for (i=0 ; i<10 ; i++){
	gNode[i].NodeAddress = 0xFF;  
	gNode[i].NodeChannel = 0xFF;  
  }  
	
  //printf("gSmbExtFormat[0].YuAddress %x -- %x\n",gSmbExtFormat[0].JxAddress ,gSmbExtFormat[0].JxSw[0]);
	
  Status = ExfileImport( gSmbExtFormat , gNode ,&ExfileDeviceCount );	
  //printf("ExfileDeviceCount: %d\n",ExfileDeviceCount);
/*  
  for (i=0 ;i<ExfileDeviceCount ; i++){
	  printf("gSmbExtFormat[i].JxAddress:%x \n",gSmbExtFormat[i].JxAddress);
	      for (j=0 ;j<100; j++)
	  printf(" gSmbExtFormat[i].Reference[%d]:%x \n",j,gSmbExtFormat[i].Reference[j]);
      	  for (j=0 ;j<100; j++)
	  printf(" gSmbExtFormat[i].RuleOffset[%d]:%x \n",j,gSmbExtFormat[i].RuleOffset[j]);
	  	  for (j=0 ;j<100; j++)
	  printf(" gSmbExtFormat[i].RuleValue[%d]:%x \n",j,gSmbExtFormat[i].RuleValue[j]);
	  	  for (j=0 ;j<20; j++)
	  printf(" gSmbExtFormat[i].JxSw[%d]:%x \n",j,gSmbExtFormat[i].JxSw[j]);
	  	  for (j=0 ;j<20; j++)
	  printf(" gSmbExtFormat[i].JxCh[%d]:%x \n",j,gSmbExtFormat[i].JxCh[j]);
  printf("\n");
  }
	for ( i=0 ; i<10 ; i++){
      printf("%x %d\n" ,gNode[i].NodeAddress ,gNode[i].NodeChannel  );
	}
*/	
// Step 1. Locate Smbus Protocol  
  printf("Step 1. Locate the Smbus protocol\n");
  Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID**)&gSmbus);
  if(EFI_ERROR(Status)){
  	return EFI_INVALID_PARAMETER; 
  } 
  
   
// Step 2. Scheratch the Smbus hierarchy Tree 
  printf("Step 2. Create the Smbus Hierarchy Tree\n");
do{
// Step 2-1-1. Scan Current state, and note the found smbus device
  //ChantDebug = 0;
  AfterCou   = 0;
  ColumnFlag = 0;
  while (1){
    UINT32             CloseLine=0;   
    UINT32             ScanLoop=0;  
/*/
    if ( ChantDebug == 10 ){
  if ( CloseLoopScnRes != NULL )
	free(CloseLoopScnRes);  
  if ( hierarchyTableLink != NULL )
	free(hierarchyTableLink);  
  if ( TempNodeStruct != NULL )
	free(TempNodeStruct);  
  if ( gNode != NULL )
	free(gNode);  
  if ( TraceSwAddress != NULL )
	free(TraceSwAddress);  
  if ( TraceSwChannel != NULL )
	free(TraceSwChannel);  	  
	  return EFI_SUCCESS;		
	}
    ChantDebug++;
*/
    //printf("CDYU Step 2-1-1 \n");
	Count = 0;
    for ( ScanLoop=0x02 ; ScanLoop<0x100 ; ScanLoop=ScanLoop+2 ){
	  UINT8              ChainRepeatFlag=0;
////----////    		
	  //printf("%02x ",ScanLoop);  
	  // filter previous .. some reason for 9545
	  for ( i=0 ; i<TracePtr ; i++ ){
		NodeLyaerTerms = RowFlag+1;  
		if ( TraceSwAddress[i] == (UINT8)ScanLoop ){
		  ChainRepeatFlag = 1;
		  break;
		}	 
	  }
	  if ( ChainRepeatFlag != 0 ){
		//printf("Repeate Jump \n");
		continue; 
	  }
	  Status = SmbusPorting( gSmbus, (UINT8)ScanLoop , EfiSmbusReadByte ,0 , (UINT8*)&DummyBuffer );
      //printf("2-1-1 SmbPorting %x Count:%d AfterCou:%d\n",Status,Count,AfterCou);
	  if( Status == EFI_SUCCESS ){
		 Status = NodeCompare( (UINT8)ScanLoop , gNode , &SpeificChannel );
		 //printf("NodeCompare %x \n",Status);
		 if ( Status == EFI_SUCCESS ){
		   memcpy( &CloseLoopScnRes[Count],&ScanLoop,sizeof(UINT8)); 
		   //printf("CloseLoop %x count %d \n",CloseLoopScnRes[Count],Count);
	       Count++;	
		 }  
	  }    
    }
// Step 2-1-2. Check if CloseLoopScnRes meet the B.C. and jump. Otherwise keep looping...    
    //printf("CDYU Step 2-1-2 \n");  
	//printf("CDYU Count:%d AfterCou: %d \n",Count,AfterCou);
	if ( Count==0 || Count==AfterCou ){
	   //printf("Jump! \n");	
	   break; // the only way to jump out form "true-while-loop"  
    }   
// Step 2-1-3. Close Channel to find "Real-Layer" switch
    //printf("CDYU Step 2-1-3 \n");      
    DummyBuffer = 0;
    AfterCou = 0;
    for ( CloseLine=0 ; CloseLine<Count ; CloseLine++ ){
      Status = SmbusPorting( gSmbus, CloseLoopScnRes[CloseLine] , EfiSmbusWriteByte, 0 , (UINT8*)&DummyBuffer );
      //printf("2-1-3 SmbPorting %x Count:%d\n",Status,Count);
	  if( Status == EFI_SUCCESS ){
	    //printf("%02x ",CloseLoopScnRes[CloseLine]);
		AfterCou++; 
      }
    }
	//printf("\n");
  } // While (1) Until now, output Count & *CloseLoopScnRes
// Step 2-2. Fill in hierarchyTableLink and link successed...
  //for ( i=0 ; i<Count ; i++ ) 
    //printf(" %02x || \n",CloseLoopScnRes[i]);
  for ( i=0 ; i<Count ; i++ ){
	//printf("CDYU Step 2-2+ \n");  
    Status = NodeCompare( CloseLoopScnRes[i] , gNode , &SpeificChannel );
	for ( j=0 ; j<SpeificChannel ; j++ ){ // temp solution
	  memcpy( &hierarchyTableLink[ListNumber].SmbNodeAddress , &CloseLoopScnRes[i] ,sizeof(UINT8) );  
	  memcpy( &hierarchyTableLink[ListNumber].SmbNodeChannel , (UINT8*)&j ,sizeof(UINT8) );  
	  memcpy( &hierarchyTableLink[ListNumber].SmbNodeLayer , &NodeLyaerTerms ,sizeof(UINT32) );  
	  memcpy( &hierarchyTableLink[ListNumber].OffsetPtr , &ListNumber ,sizeof(UINT32) );  
	  
	  if ( LinkTablePtr > 0 ){ // if(LinkTablePtr) // != 0
	    //memcpy( hierarchyTableLink[ListNumber].Inherited  , &hierarchyTableLink[LinkTablePtr] , sizeof(NodeLinkList) );   
	    hierarchyTableLink[ListNumber].Inherited  =   &hierarchyTableLink[LinkTablePtr-1];  // Very Danger!!!
	  }
	  else {
		BaseSwitchCount = ListNumber;  
	  }
	  ListNumber++;  	
	}
	//printf("CDYU Step 2-2- \n");
  }	 // for ( i=0 ; i<Count ; i++ ) after these, output matrix/structure *hierarchyTableLink & ListNumber & BaseSwitchCount 
  
// Step 2-3. explore next layer (Behind the specificd Node) and mark the relative trace information  
// switch register ,open channel may output *TraceSwAddress & *TraceSwChannel & TracePtr
  //printf("CDYU Step 2-3 List:%d Base:%d NodeLayer:%d \n",ListNumber,BaseSwitchCount,NodeLyaerTerms); 
  if ( LinkTablePtr == 0 && BaseSwitchCount == 0 ){ 
	printf("Can't Find Any Switch on Smb!! \n");	    
  }  
  else{
    if ( LinkTablePtr > 0 ){
      for ( i=0 ; i<RowFlag+1 ; i++ ){
	    DummyBuffer = 0;
        Status = SmbusPorting( gSmbus, TraceSwAddress[ColMatrix[i]] , EfiSmbusWriteByte , 0,  (UINT8*)&DummyBuffer );  
        //printf("\n 2-3 SmbPorting %x NodeAdd:%x CloseCh:%x\n",Status,TraceSwAddress[ColMatrix[i]],DummyBuffer);
        //printf("ColMatrix %d\n",ColMatrix[i]);
	  }	 		
	}
	RowFlag=0;
    TracePtr = 0; 
	TempNodeStruct = &hierarchyTableLink[LinkTablePtr];
	while( 1 ){
	  ColMatrix[RowFlag] = (UINT8)ColumnFlag;	  
	  TraceSwAddress[TracePtr] = (UINT8) TempNodeStruct->SmbNodeAddress;
	  TraceSwChannel[TracePtr] = (UINT8) TempNodeStruct->SmbNodeChannel;	
	  TracePtr++;
	  //ColumnFlag = 0;	
	  for( i=0 ; i<ListNumber ; i++ ){
	    if( (TempNodeStruct->SmbNodeLayer == hierarchyTableLink[i].SmbNodeLayer) ){
	      TraceSwAddress[TracePtr] = (UINT8) hierarchyTableLink[i].SmbNodeAddress;
	      TraceSwChannel[TracePtr] = (UINT8) hierarchyTableLink[i].SmbNodeChannel;	
		  TracePtr++;
		  ColumnFlag++;
		}
      }	  
	  if ( TempNodeStruct->Inherited == NULL ){
        //printf("Node endof Inherited \n");
		break;
	  }
	  TempNodeStruct = TempNodeStruct->Inherited;	  
	  RowFlag++;
	  ColumnFlag++; // for duplicate address , keep the offset 
	  }

	for ( i=0 ; i<TracePtr ; i++){
	  //printf("%x Y ",TraceSwAddress[i]);
	}
	// switch channel base on past
    for ( i=RowFlag+1 ; i>0 ; i-- ){
	  BitBuffer = 0;	
	  BitBuffer = 1 <<  TraceSwChannel[ColMatrix[i-1]] ;
      Status = SmbusPorting( gSmbus, TraceSwAddress[ColMatrix[i-1]] , EfiSmbusWriteByte, 0 , (UINT8*)&BitBuffer );  
      //printf("\n 2-3 SmbPorting %x NodeAdd:%x NodeChannel:%x\n",Status,TraceSwAddress[ColMatrix[i-1]],TraceSwChannel[ColMatrix[i-1]]);
      //printf("ColMatrix %d\n",ColMatrix[i-1]);
	}		
    LinkTablePtr++;	
	
    if ( LinkTablePtr == ListNumber ){
      for ( i=0 ; i<RowFlag+1 ; i++ ){
	    DummyBuffer = 0;
        Status = SmbusPorting( gSmbus, TraceSwAddress[ColMatrix[i]] , EfiSmbusWriteByte, 0 , (UINT8*)&DummyBuffer );  
        //printf("\n 2-3 SmbPorting %x NodeAdd:%x CloseCh:%x\n",Status,TraceSwAddress[ColMatrix[i]],DummyBuffer);
        //printf("ColMatrix %d\n",ColMatrix[i]);
	  }	 
    }	
  }
  //printf("ListNum: %d LinkTabPtr:%d \n",ListNumber,LinkTablePtr);	
} while ( LinkTablePtr < ListNumber ); // after these, output matrix/structure *hierarchyTableLink & ListNumber
  
// Step 2-4. print the bone
/*
  for ( i=0 ; i<ListNumber ; i++ ){
    printf("No.%03d NodeMMIO:%08x NodeAddr:%02x NodeChannel:%02d Succed-MMIO:%08x \n",i,(UINT32)&hierarchyTableLink[i],hierarchyTableLink[i].SmbNodeAddress,hierarchyTableLink[i].SmbNodeChannel,(UINT32)hierarchyTableLink[i].Inherited);
  }
*/
  
// Step 3. Transfer *hierarchyTableLink into *CascadeScanTable
  printf("Step 3. List Scan topology and priority\n");
// Step 3-1. Prepare Buffer
  CascadeScanTable     = malloc(ListNumber*sizeof(ScanRuleFolder));
  for ( i=0 ; i<ListNumber ;i++ ){
	memset( CascadeScanTable[i].AddressFolder , 0xFF , 20 );  
	memset( CascadeScanTable[i].ChannelFolder , 0xFF , 20 );  
  }

// Step 3-2. Connection - Layer
  for ( i=0 ; i<ListNumber ; i++ ){
	UINT32      TempNumberChain=1;
////----//// 	
    //printf("No.%03d \n",i);  
	TempNodeStruct = &hierarchyTableLink[i];
	if ( TempNodeStruct->Inherited != NULL){
	  do{
	    TempNodeStruct = TempNodeStruct->Inherited;
		TempNumberChain++;
		//printf("Have Succed! \n");
	  }while( TempNodeStruct->Inherited != NULL );
	}
	memcpy( &CascadeScanTable[i].NumberOfChain , &TempNumberChain , sizeof(UINT32) );
	//CascadeScanTable[i].NumberOfChain = TempNumberChain;
  }
// Step 3-3. Connection - Address , Channel
  for ( i=0 ; i<ListNumber ; i++ ){
	TempNodeStruct = &hierarchyTableLink[i];  
	for ( j=CascadeScanTable[i].NumberOfChain ; j>0 ; j-- ){
		//!! A.B fail  but  &(A->B) ok !!  
	  memcpy( &CascadeScanTable[i].AddressFolder[j-1] , &(TempNodeStruct->SmbNodeAddress) , sizeof(UINT8) );
	  //CascadeScanTable[i].AddressFolder[j-1] = TempNodeStruct->SmbNodeAddress;
	  memcpy( &CascadeScanTable[i].ChannelFolder[j-1] , &(TempNodeStruct->SmbNodeChannel) , sizeof(UINT8) );
	  //CascadeScanTable[i].ChannelFolder[j-1] = TempNodeStruct->SmbNodeChannel;
	  TempNodeStruct = TempNodeStruct->Inherited;
	} 
  }  	

  
  // Prevent system crash...temp solution 
   for ( i=0 ; i<ListNumber ; i++){  
	 if (CascadeScanTable[i].NumberOfChain > 10){
		 printf("Smbus Crash...");
		return EFI_INVALID_PARAMETER; 
	 }
   }
// Step 3-4 Print List
/*
  for ( i=0 ; i<ListNumber ;i++ ){
	printf("N0:%d \nAddr:",i);  
    for ( j=0 ; j<4 ; j++){
	  printf("%x |",CascadeScanTable[i].AddressFolder[j]);	 
	}
	printf("\n");
	printf("Chan:"); 
    for ( j=0 ; j<4 ; j++){
	  printf("%x |",CascadeScanTable[i].ChannelFolder[j]);	 
	}	
	printf("\n");
	printf("Layer: %d\n",CascadeScanTable[i].NumberOfChain);     	
  }  
*/
  printf("Step 4. Scanning Smbus Device...\n");	
// Step 4 Find The Leaf(Device) in Trunk(Switch Arch) output *DevAttr & SmbusDeviceCount
// Step 4-1 Prepare  buffer 
  DevAttr  = malloc(SMBCOUNT*sizeof(SmbDevAttribute)); 
  //printf("CDYU Step 4-1\n");	
  for ( iDWORD=0 ; iDWORD<SMBCOUNT ; iDWORD++ ){
	//printf("%d |",iDWORD);  
	memset( DevAttr[iDWORD].TraceSw , 0xFF , 20 );  
	memset( DevAttr[iDWORD].TraceCh , 0xFF , 20 );  
  }    
// Step 4-2 Consider No Switch Situation
  //printf("CDYU Step 4-2\n");
  SpeificChannel = 0;  
  DummyBuffer = 0;
  memset( SimpleScanBuffer , 0xFF , 50 );
  for ( SimpleScanLoop=0x02 ; SimpleScanLoop<0x100 ; SimpleScanLoop=SimpleScanLoop+2 ){	  
    Status = NodeCompare( (UINT8)SimpleScanLoop , gNode , &SpeificChannel );	
    if(EFI_ERROR(Status)){
      Status = SmbusPorting( gSmbus, (UINT8)SimpleScanLoop , EfiSmbusReadByte, 0 ,(UINT8*) &DummyBuffer );
	  if ( Status == EFI_SUCCESS ){
		memcpy( &DevAttr[SmbusDeviceCount].YuAddress , (UINT8*) &SimpleScanLoop , sizeof(UINT8) );	
		memcpy( &DevAttr[SmbusDeviceCount].NumberOfDevice ,  &SmbusDeviceCount , sizeof(UINT32) );	
		memcpy( &SimpleScanBuffer[SmbusDeviceCount] , (UINT8*) &SimpleScanLoop , sizeof(UINT8) );	
		//printf("-4.2-  %02x ",DevAttr[SmbusDeviceCount].YuAddress);
		//printf("\n");
		SmbusDeviceCount++;
	  }	
	}	  
  } 
// Step 4-3 Scan with *CascadeScanTable 
  for (i=0 ; i<ListNumber ; i++){
	UINT32        ScanLoop=0;
////----////   
// Step 4-3-1 Open the road by given ScanRule-Map  
    for ( j=0 ; j< CascadeScanTable[i].NumberOfChain ; j++ ){		
	  BitBuffer = 0;
	  BitBuffer = 1 << CascadeScanTable[i].ChannelFolder[j] ;
      Status = SmbusPorting( gSmbus, CascadeScanTable[i].AddressFolder[j] , EfiSmbusWriteByte, 0 ,(UINT8*) &BitBuffer );  
	  
	  //printf("i:%03d j:%02x\n",i,j);
	  //printf("SWs:  %02x\n",CascadeScanTable[i].AddressFolder[j]);
	  //printf("Chan: %02x\n",BitBuffer);
	  //printf("EFI: %x\n",Status);	
	  
	  //printf("4-1 SmbPorting %x NodeAdd:%x NodeChannel:%x\n",Status ,CascadeScanTable[i].AddressFolder[j], CascadeScanTable[i].ChannelFolder[j] );
	}	   
// Step 4-3-2 Grouping and Filter	Duplicate Node Address	
	Status = NodeCompare( hierarchyTableLink[i].SmbNodeAddress , gNode , &SpeificChannel );	
    if ( i == ListShiftNumber ){
	  ListShiftNumber += SpeificChannel;	
	  memcpy( &DevAttr[SmbusDeviceCount].YuAddress  , &hierarchyTableLink[i].SmbNodeAddress , sizeof(UINT8) );	
      memcpy( &DevAttr[SmbusDeviceCount].NumberOfDevice ,  &SmbusDeviceCount , sizeof(UINT32) );	
	  TempNodeStruct = &hierarchyTableLink[i];
	  if ( TempNodeStruct->Inherited != NULL ){
	    TempNodeStruct = TempNodeStruct->Inherited;
		memcpy( DevAttr[SmbusDeviceCount].TraceSw  , CascadeScanTable[TempNodeStruct->OffsetPtr].AddressFolder ,  20*sizeof(UINT8) );	
	    memcpy( DevAttr[SmbusDeviceCount].TraceCh  , CascadeScanTable[TempNodeStruct->OffsetPtr].ChannelFolder ,  20*sizeof(UINT8) );		
	  }
	  SmbusDeviceCount++;
	}	  				
// Step 4-3-3 Scanning   	
    for ( ScanLoop=0x02 ; ScanLoop<0x100 ; ScanLoop=ScanLoop+2 ){
      //UINT8        CmpDiffFlag=0;
      UINT8        CmpSameFlag=0;
      //UINT8        s=0;
      //UINT8        DuplicAddr[10] = ;
///---	  
      // filter the Base Smb Address
	  for ( j=0 ; j<50 ; j++){
	    if ( ScanLoop == SimpleScanBuffer[j] ){
          CmpSameFlag = 1;
		  break;
	    }	  
	  }
	  if (CmpSameFlag){
		continue;  
	  }
	  //printf("Pose here \n")	  ;
      SpeificChannel = 0;  
	  Status = NodeCompare( (UINT8)ScanLoop , gNode , &SpeificChannel );	
      if(EFI_ERROR(Status)){
		DummyBuffer = 0;  		
        Status = SmbusPorting( gSmbus, (UINT8)ScanLoop , EfiSmbusReadByte, 0 , (UINT8*)&DummyBuffer );  
			  //printf("Addr: %02x \n",ScanLoop );
			  //printf("SWs:  %02x %02x %02x \n",CascadeScanTable[i].AddressFolder[0],CascadeScanTable[i].AddressFolder[1],CascadeScanTable[i].AddressFolder[2] );
			  //printf("Chan: %02x %02x %02x \n",CascadeScanTable[i].ChannelFolder[0],CascadeScanTable[i].ChannelFolder[1],CascadeScanTable[i].ChannelFolder[2] );
			  //printf("EFI: %x\n",Status);			
			  //printf("\n");			
		if ( Status == EFI_SUCCESS ){  		
		// nothing
		  TempNodeStruct = &hierarchyTableLink[i];
		  if ( TempNodeStruct->Inherited != NULL ){
			TempNodeStruct = TempNodeStruct->Inherited;
			for ( m=0 ; m<SmbusDeviceCount ; m++ ){	
			      //printf("Jug 2 DevNo: %d\n",m);
				  //printf("Dev: %02x \n",DevAttr[m].TraceSw[CascadeScanTable[TempNodeStruct->OffsetPtr].NumberOfChain] );
				  //printf("adr: %02x \n",DevAttr[m].YuAddress );			
			  if ( DevAttr[m].TraceSw[CascadeScanTable[TempNodeStruct->OffsetPtr].NumberOfChain] == 0xFF && ( DevAttr[m].YuAddress == (UINT8)ScanLoop ) ){
			    for ( k=0 ; k < CascadeScanTable[TempNodeStruct->OffsetPtr].NumberOfChain ; k++ ){
  				  //printf("Jug 1 \n");
				  //printf("SW1: %02x \n",DevAttr[m].TraceSw[k] );
				  //printf("SW2: %02x \n", CascadeScanTable[TempNodeStruct->OffsetPtr].AddressFolder[k] );
				  //printf("Tr1: %02x \n",DevAttr[m].TraceCh[k] );	                 
				  //printf("Tr2: %02x \n",CascadeScanTable[TempNodeStruct->OffsetPtr].ChannelFolder[k] );	                                  
				  if ( (DevAttr[m].TraceSw[k] == CascadeScanTable[TempNodeStruct->OffsetPtr].AddressFolder[k]) && (DevAttr[m].TraceCh[k] == CascadeScanTable[TempNodeStruct->OffsetPtr].ChannelFolder[k]) ){		      
				    CmpSameFlag = 1; 
					break;
				  }
				}   
			  }
		    } //for ( m=0 ; m<SmbusDeviceCount ; m++ ) 
            if ( CmpSameFlag ){
			  //printf("Here for Repeat Config! \n");	
			  continue;  
		    }  				
		  } // if ( TempNodeStruct->Inherited != NULL )
		  
		  memcpy( &DevAttr[SmbusDeviceCount].YuAddress , (UINT8*) &ScanLoop , sizeof(UINT8) );	
		  memcpy( &DevAttr[SmbusDeviceCount].NumberOfDevice ,  &SmbusDeviceCount , sizeof(UINT32) );	
		  memcpy( DevAttr[SmbusDeviceCount].TraceSw , CascadeScanTable[i].AddressFolder , 20*sizeof(UINT8) );	
		  memcpy( DevAttr[SmbusDeviceCount].TraceCh , CascadeScanTable[i].ChannelFolder , 20*sizeof(UINT8) );	
		//printf("Step 4-3 ");
		//printf("%02x Trace Sw",DevAttr[SmbusDeviceCount].YuAddress);
		//printf("%02x %02x %02x ",DevAttr[SmbusDeviceCount].TraceSw[0],DevAttr[SmbusDeviceCount].TraceSw[1],DevAttr[SmbusDeviceCount].TraceSw[2]);
		//printf("%| %02x %02x %02x ",DevAttr[SmbusDeviceCount].TraceCh[0],DevAttr[SmbusDeviceCount].TraceCh[1],DevAttr[SmbusDeviceCount].TraceCh[2]);
		//printf("\n");		  
		  SmbusDeviceCount++;
		  //printf("SmbusDeviceCount: %d \n",SmbusDeviceCount);
		}	//if ( Status == EFI_SUCCESS )
	  }  // if(EFI_ERROR(Status))
  
	}//for ( ScanLoop=0x02 ; ScanLoop<0x100 ; ScanLoop=ScanLoop+2 )	
// Step 4-3-4  Close the road by given ScanRule-Map   
    for ( j=CascadeScanTable[i].NumberOfChain ; j>0 ; j-- ){
	  //BitBuffer = 0;	
	  DummyBuffer = 0;	
	  //BitBuffer = 1 <<  CascadeScanTable[i].ChannelFolder[j-1] ;
      Status = SmbusPorting( gSmbus, CascadeScanTable[i].AddressFolder[j-1] , EfiSmbusWriteByte, 0 , (UINT8*)&DummyBuffer );  
      //printf("4-3-4 SmbPorting %x NodeAdd:%x close:%x\n",Status ,CascadeScanTable[i].AddressFolder[j-1], DummyBuffer );
	}	 
  }	// for (i=0 ; i<ListNumber ; i++)
	  
// Step 4-4 Print All Found Smbus device
/*
  for (i=0 ; i<SmbusDeviceCount ; i++ ){
	printf ("No: %03d \n",i);
	printf("Addr: %02x\n",DevAttr[i].YuAddress );
    printf("Sw: ");
    for (j=0; j<10 ; j++){	
	  printf("%02x|",DevAttr[i].TraceSw[j] );		
	}
	printf("\n");
    printf("Ch: ");	
    for (j=0; j<10 ; j++){
	  printf("%02x|",DevAttr[i].TraceCh[j] );		
	}
    printf("\n");	
    printf("\n");	
  }  
*/

// Step 5  GUI Output & Prepare Text post	
// 5-1 Transfer *DevAttr into *GuiCharBox	
// 5-1-1 create a correct count bix for SMB device and fill dummy char
  for ( i=0 ; i<SmbusDeviceCount ; i++){
    for ( j=0 ; j<CHARSIZE ; j++){
	  GuiCharBox[i].CharBuff[j] = ' ';
    }
    GuiCharBox[i].CharBuff[CHARSIZE-1] = '\0'; 	  
  }
// 5-1-2 Smbus Address
  for ( i=0 ; i<SmbusDeviceCount ; i++){
	sprintf( TempASCIIChar , "%02x" , DevAttr[i].YuAddress);
	GuiCharBox[i].CharBuff[0] = '0';  
	GuiCharBox[i].CharBuff[1] = 'x';  
	GuiCharBox[i].CharBuff[2] = TempASCIIChar[0];  
	GuiCharBox[i].CharBuff[3] = TempASCIIChar[1];  
  }  
// 5-1-2 Smbus Trace	
  for ( i=0 ; i<SmbusDeviceCount ; i++){
	GuiCharBox[i].CharBuff[20] = '.';
	GuiCharBox[i].CharBuff[21] = '.';
	j=0;  	
	while( DevAttr[i].TraceSw[j] != 0xFF ){
	  sprintf( TempASCIIChar , "%02x" , DevAttr[i].TraceSw[j]);
	  GuiCharBox[i].CharBuff[20+6*j] = TempASCIIChar[0];
	  GuiCharBox[i].CharBuff[21+6*j] = TempASCIIChar[1];
	  GuiCharBox[i].CharBuff[22+6*j] = '-';
	  sprintf( TempASCIIChar , "%02x" , DevAttr[i].TraceCh[j]);
	  GuiCharBox[i].CharBuff[23+6*j] = TempASCIIChar[0];
	  GuiCharBox[i].CharBuff[24+6*j] = TempASCIIChar[1];	  
      GuiCharBox[i].CharBuff[25+6*j] = '|';				
	  j++;
	}  
  }  

// 5-1-3  Module Name 
// 5-1-4  Smbus Device ID
  for ( i=0 ; i<SmbusDeviceCount ; i++){
    s =0;
    while( DevAttr[i].TraceSw[s] != 0xFF ){
	  s++;	
	}
	if (s == 0){
	  s=1;	// prevention
	}
	// for smbsw location & did
    for (j=0 ; j<ExfileDeviceCount ; j++){
	  if ( (DevAttr[i].YuAddress == (UINT8)gSmbExtFormat[j].JxAddress) \
           && (strcmp( gSmbExtFormat[j].DeviceType , "SmbSW" )==0 ) \
         ){
			//printf("DevAttr[i].YuAddress: %x\n",DevAttr[i].YuAddress); 
			//printf("DevAttr[i].TraceSw[s-1]: %x\n",DevAttr[i].TraceSw[s-1]); 
			//printf("gSmbExtFormat[j].JxSw[0]: %x\n",gSmbExtFormat[j].JxSw[0]); 
	     if ( (DevAttr[i].TraceSw[0] == 0xFF) && (gSmbExtFormat[j].JxSw[0] == 0xAF) ){
			for ( m=0 ; m<strlen(gSmbExtFormat[j].ModuleName) ; m++  ){
		      GuiCharBox[i].CharBuff[10+m] = gSmbExtFormat[j].ModuleName[m]; // 1-to-M pointer 
		    }
			for ( m=0 ; m<strlen(gSmbExtFormat[j].DeviceID) ; m++  ){
		      GuiCharBox[i].CharBuff[50+m] = gSmbExtFormat[j].DeviceID[m]; // 1-to-M pointer 
		    } 				
		 }
         else if ( (DevAttr[i].TraceSw[0] != 0xFF) && (gSmbExtFormat[j].JxSw[0] != 0xAF)  ){
		   
		   
		   moduleDiffFlag = 0;
		   for ( k=0 ; k<s ; k++){
			   //printf("2- DevAttr[i].TraceSw[s-1]: %x\n",DevAttr[i].TraceSw[s-1]); 			   
			   //printf("2- DevAttr[i].TraceCh[s-1]: %x\n",DevAttr[i].TraceCh[s-1]); 			   
			   //printf("2- gSmbExtFormat[j].JxSw[k]: %x\n",gSmbExtFormat[j].JxSw[k]); 			   
			   //printf("2- gSmbExtFormat[j].JxCh[k]: %x\n",gSmbExtFormat[j].JxCh[k]); 			   
			   
			   
			  if ( (DevAttr[i].TraceSw[k] != (UINT8)gSmbExtFormat[j].JxSw[k]  ) \
                 ||(DevAttr[i].TraceCh[k] != (UINT8)gSmbExtFormat[j].JxCh[k]  )
			  ){
				moduleDiffFlag = 1;		
			    break;
			  }
		   }	  
		   if ( !moduleDiffFlag ){
			 for ( m=0 ; m<strlen(gSmbExtFormat[j].ModuleName) ; m++  ){
		       GuiCharBox[i].CharBuff[10+m] = gSmbExtFormat[j].ModuleName[m]; // 1-to-M pointer 
		     }
		     for ( m=0 ; m<strlen(gSmbExtFormat[j].DeviceID) ; m++  ){
		       GuiCharBox[i].CharBuff[50+m] = gSmbExtFormat[j].DeviceID[m]; // 1-to-M pointer 
		     } 					
		   }	  
		    
		 }
		 else{	 
		   //printf("Error Logic\n");	 
		  // printf("DevAttr[i].TraceSw[s-1]: %x\n",DevAttr[i].TraceSw[s-1]);
		  // printf("gSmbExtFormat[j].JxSw[0]: %x\n",gSmbExtFormat[j].JxSw[0]);
		   //printf("DevAttr[i].TraceSw[s-1]: %x",DevAttr[i].TraceSw[s-1]);
		 }
	  }	
	}// for (j=0 ; j<ExfileDeviceCount ; j++)

	// for smb device location
	for (j=0 ; j<ExfileDeviceCount ; j++){ 
	 if ( (DevAttr[i].YuAddress == (UINT8)gSmbExtFormat[j].JxAddress) \
        && 	(strcmp( gSmbExtFormat[j].DeviceType , "SmbSW" ) != 0 )\
		){
		  memcpy(&TraceGroup , &i , sizeof(UINT32) ); // fix j
		  GroupBoundryFlag = 0;
		  while(TraceGroup) {
            if (GroupBoundryFlag){
			  break;	
			} 	
            
			//printf("DevAttr[i].YuAddress:%x \n ",DevAttr[i].YuAddress);			
 			//printf("TraceGroup: %d DevAttr[TraceGroup].YuAddress:%x \n ",TraceGroup,DevAttr[TraceGroup].YuAddress);			
          
		    for ( k=0 ; k<ExfileDeviceCount ; k++ ){
			  if ( (DevAttr[TraceGroup].YuAddress == (UINT8)gSmbExtFormat[k].JxAddress) \
                 && (strcmp( gSmbExtFormat[k].DeviceType , "SmbSW" ) == 0) \
			  ){
 			   //printf("In!! k=%d\n",k);	
				  
		       moduleDiffFlag = 0;
		         for ( z=0 ; z<s-1 ; z++){
			       //printf("DevAttr[TraceGroup].TraceSw[z]:%x\n",DevAttr[TraceGroup].TraceSw[z]);
			       //printf("DevAttr[TraceGroup].TraceCh[z]:%x\n",DevAttr[TraceGroup].TraceCh[z]);
			       //printf("gSmbExtFormat[k].JxSw[z]:%x\n",gSmbExtFormat[k].JxSw[z]);
			       //printf("gSmbExtFormat[k].JxCh[z]:%x\n",gSmbExtFormat[k].JxCh[z]);
			       if ( (DevAttr[TraceGroup].TraceSw[z] != (UINT8)gSmbExtFormat[k].JxSw[z]  ) \
                      ||(DevAttr[TraceGroup].TraceCh[z] != (UINT8)gSmbExtFormat[k].JxCh[z]  )
			       ){
				      moduleDiffFlag = 1;		
			          break;
			       }
			     }

			   
               if ( moduleDiffFlag ){
			     continue;  // filter &  fix k
		       }			   
			   //printf("\n ");			
			   //printf("gSmbExtFormat[k].JxAddress:%x \n ",gSmbExtFormat[k].JxAddress);			
			   //printf("gSmbExtFormat[k].ModuleName:%s \n ",gSmbExtFormat[k].ModuleName);			
			   //printf("gSmbExtFormat[j].JxAddress:%x \n ",gSmbExtFormat[j].JxAddress);			
			   //printf("gSmbExtFormat[j].ModuleName:%s \n ", gSmbExtFormat[j].ModuleName);			
			   //printf("\n ");	
			   
			   if( strcmp( gSmbExtFormat[k].ModuleName ,  gSmbExtFormat[j].ModuleName)  ==  0 ){
				 GroupBoundryFlag = 1;  
				 for ( m=0 ; m<strlen(gSmbExtFormat[j].ModuleName) ; m++  ){
		           GuiCharBox[i].CharBuff[10+m] = gSmbExtFormat[j].ModuleName[m]; // 1-to-M pointer 
		         } 
				 for ( m=0 ; m<strlen(gSmbExtFormat[j].DeviceID) ; m++  ){
		           GuiCharBox[i].CharBuff[50+m] = gSmbExtFormat[j].DeviceID[m]; // 1-to-M pointer 
		         } 
			   }    	  
			 }
		   } // for ( k=0 ; k<ExfileDeviceCount ; k++ )		
		   TraceGroup--;
		 }	// while(TraceGroup)		  
	   }  		   
	} // for (j=0 ; j<ExfileDeviceCount ; j++)
	//printf(" YOUT DevAttr[i].YuAddress: %x \n",DevAttr[i].YuAddress);
  } //for ( i=0 ; i<SmbusDeviceCount ; i++)
  
// 5-1-5 OEM Slot
  for ( i=0 ; i<SmbusDeviceCount ; i++){
    s =0;
    while( DevAttr[i].TraceSw[s] != 0xFF ){
	  s++;	
	}
	if (s == 0){
	  s=1;	// prevention
	}
	//printf("%d SmbusDeviceCount: %x\n",i,DevAttr[i].YuAddress); 
    for (j=0 ; j<ExfileDeviceCount ; j++){
	  if ( (DevAttr[i].YuAddress == (UINT8)gSmbExtFormat[j].JxAddress) \
           && (strcmp( gSmbExtFormat[j].DeviceType , "SmbSW" )==0 ) \
         ){
			//printf("DevAttr[i].YuAddress: %x\n",DevAttr[i].YuAddress); 
			//printf("DevAttr[i].TraceSw[s-1]: %x\n",DevAttr[i].TraceSw[s-1]); 
			//printf("gSmbExtFormat[j].JxSw[0]: %x\n",gSmbExtFormat[j].JxSw[0]); 

			
		 if ( (DevAttr[i].TraceSw[0] == 0xFF) && (gSmbExtFormat[j].JxSw[0] == 0xAF) ){
           memcpy(&TraceGroup , &i , sizeof(UINT32) );
		   GroupBoundryFlag = 0;
		   while ( TraceGroup ){   
             if (GroupBoundryFlag){
			   break;
               //printf(" GroupBoundryFlag set break! \n");			   
			 }
			 TraceGroup++; 
             //printf("TraceGroup: %d GroupBoundryFlag: %d \n",TraceGroup,GroupBoundryFlag);
             //printf("DevAttr[TraceGroup].YuAddress: %x DevAttr[TraceGroup].TraceSw[0]: %x \n",DevAttr[TraceGroup].YuAddress,DevAttr[TraceGroup].TraceSw[0]);			   
						 
			 
			 if ( TraceGroup > SmbusDeviceCount ){ 
			   break;	 
			 }
			 
			 for ( m=0 ; m<ExfileDeviceCount ; m++ ){
			   if ( ( DevAttr[TraceGroup].YuAddress == (UINT8)gSmbExtFormat[m].JxAddress )   && (strcmp( gSmbExtFormat[m].DeviceType , "SmbSW" )==0 ) ){
				 GroupBoundryFlag=1;
                 break;				 
			   }	 
			 } // for ( m=0 ; m<ExfileDeviceCount ; m++ )	
			 
			 if ( (DevAttr[TraceGroup].TraceSw[0] == (UINT8)gSmbExtFormat[j].JxAddress) && (!GroupBoundryFlag) ){ // under the SmbSw ALWAYs trace +1 
               k=0;
			   //printf("Init Reference: %d DevAttr[TraceGroup].TraceCh[0]: %d \n",gSmbExtFormat[j].Reference[k],DevAttr[TraceGroup].TraceCh[0]);
			   while ( gSmbExtFormat[j].Reference[k] != 0xAF ){
			     //printf("Deeper Reference: %d DevAttr[TraceGroup].TraceCh[0]: %d \n",gSmbExtFormat[j].Reference[k],DevAttr[TraceGroup].TraceCh[0]);  
				 if ( gSmbExtFormat[j].Reference[k] == DevAttr[TraceGroup].TraceCh[0] ){
				   for ( m=0 ; m<strlen(gSmbExtFormat[j].OemDefined[k]) ; m++  ){
		             GuiCharBox[TraceGroup].CharBuff[40+m] = gSmbExtFormat[j].OemDefined[k][m]; // 1-to-M pointer 
				   } 				   	 
				   //printf("Done! \n");
				   break;
				 }
                 k++;				 
			   } // while ( gSmbExtFormat[j].Reference[k] != 0xAF )	 
			 } //  if ( DevAttr[TraceGroup].TraceSw[0] == gSmbExtFormat[j].JxSw[0] ){ // under the SmbSw ALWAYs trace +1 
			 //TraceGroup++; 	
		   
		   } // while ( TraceGroup )

		 
		 } //  if ( (DevAttr[i].TraceSw[0] == 0xFF) && (gSmbExtFormat[j].JxSw[0] == 0xAF) ){

         else if ( (DevAttr[i].TraceSw[0] != 0xFF) && (gSmbExtFormat[j].JxSw[0] != 0xAF)  ){	   	     
		   moduleDiffFlag = 0;
		   for ( z=0 ; z<s ; z++){  
			  //printf("actSw:%x extSw:%x actCh:%x extCh:%x \n",DevAttr[i].TraceSw[z],gSmbExtFormat[j].JxSw[z],DevAttr[i].TraceCh[z],gSmbExtFormat[j].JxCh[z]);
			  if ( (DevAttr[i].TraceSw[z] != (UINT8)gSmbExtFormat[j].JxSw[z]  ) ||(DevAttr[i].TraceCh[z] != (UINT8)gSmbExtFormat[j].JxCh[z]  )  ){
				moduleDiffFlag = 1;		
			    break;
			  }
		   } // for ( z=0 ; z<s ; z++)			 
		   if (moduleDiffFlag){
		     continue;	 
		   }
		   //printf("2nd In!\n");
		   memcpy(&TraceGroup , &i , sizeof(UINT32) );	
		   GroupBoundryFlag = 0;
		   while ( TraceGroup ){
             if (GroupBoundryFlag){
			   break;
			   //printf(" GroupBoundryFlag set break! \n"); 
			 }

             TraceGroup++; 
             //printf("TraceGroup: %d GroupBoundryFlag: %d \n",TraceGroup,GroupBoundryFlag);
             //printf("DevAttr[TraceGroup].YuAddress: %x DevAttr[TraceGroup].TraceSw[s]: %x \n",DevAttr[TraceGroup].YuAddress,DevAttr[TraceGroup].TraceSw[s]);			   

			 if ( TraceGroup > SmbusDeviceCount ){ 
			   break;	 
			 }		
			 
			 for ( m=0 ; m<ExfileDeviceCount ; m++ ){
			   if ( ( DevAttr[TraceGroup].YuAddress == (UINT8)gSmbExtFormat[m].JxAddress )   && (strcmp( gSmbExtFormat[m].DeviceType , "SmbSW" )==0 ) ){
				 GroupBoundryFlag=1;
                 break;				 
			   }	 
			 } // for ( m=0 ; m<ExfileDeviceCount ; m++ )	
				 
			 if ( (DevAttr[TraceGroup].TraceSw[s] == (UINT8)gSmbExtFormat[j].JxAddress) && (!GroupBoundryFlag) ){ // under the SmbSw ALWAYs trace +1 
               k=0;
			   //printf("Init Reference: %d DevAttr[TraceGroup].TraceCh[s]: %d \n",gSmbExtFormat[j].Reference[k],DevAttr[TraceGroup].TraceCh[s]);
			   while ( gSmbExtFormat[j].Reference[k] != 0xAF ){
			     //printf("Deeper Reference: %d DevAttr[TraceGroup].TraceCh[s]: %d \n",gSmbExtFormat[j].Reference[k],DevAttr[TraceGroup].TraceCh[s]);  				   
				 if ( gSmbExtFormat[j].Reference[k] == DevAttr[TraceGroup].TraceCh[s] ){
				   for ( m=0 ; m<strlen(gSmbExtFormat[j].OemDefined[k]) ; m++  ){
		             GuiCharBox[TraceGroup].CharBuff[40+m] = gSmbExtFormat[j].OemDefined[k][m]; // 1-to-M pointer 
		           } 		
				   //printf("Done! \n"); 
				   break;
				 }  
                 k++;				 
			   } // while ( gSmbExtFormat[j].Reference[k] != 0xAF )	 
			 } //  if ( DevAttr[TraceGroup].TraceSw[s] == gSmbExtFormat[j].JxSw[s] ){ // under the SmbSw ALWAYs trace +1 
		   } // while ( TraceGroup ) 	  
		 } //          else if ( (DevAttr[i].TraceSw[0] != 0xFF) && (gSmbExtFormat[j].JxSw[0] != 0xAF)  ){
		 
		 else{	 
		   //printf("Error Logic\n");	 
		   //printf("DevAttr[i].TraceSw[s-1]: %x\n",DevAttr[i].TraceSw[s-1]);
		   //printf("gSmbExtFormat[j].JxSw[0]: %x\n",gSmbExtFormat[j].JxSw[0]);
		   //printf("DevAttr[i].TraceSw[s-1]: %x",DevAttr[i].TraceSw[s-1]);
		 }
	  }	
	}// for (j=0 ; j<ExfileDeviceCount ; j++)	 
  } //for ( i=0 ; i<SmbusDeviceCount ; i++)
  
	

// 5-2 dump log for all smb information on paltform
  if ( strcmp(argv[1] , ArgInputList[0].ArgStr ) == 0 ){
	GuiJump = 1;
    Status = LogOutputJson( gSmbus , DevAttr ,GuiCharBox  ,&SmbusDeviceCount  );
    if (EFI_ERROR(Status)){
      printf(" InheritanceLog Log fail\n");
      return Status;
    }
    else{
	  printf(" InheritanceLog Log Success\n");  
    }	  
  }	
	
	
	
if ( !GuiJump ){	
  GUI_ScreenClean();	  
  GUI_SetPosition(0, 1); 
  GUI_GetPosition(&Columns,&Rows); 	
  CharBoxIndex = 0;   // set to default	  
  while(1){ 
    UINT32     GUI_i=0;
///---///
// Step 5-3 create background & show the TextInfo
	GUI_SetPosition (0, 0);
	GUI_SetColor (0xA6);
	Print(L"Addr      Location  Trace               Slot      DeviceID            ");		
	GUI_SetColor( 0xE2 );
	GUI_SetPosition (0,20);
	Print(L"============== Smbus Tree ======================================================");
	GUI_SetPosition (0,21);
	Print(L"                                                                                ");		
	GUI_SetPosition (0,22);
	Print(L"                                                                                ");		
    GUI_SetPosition (0,23);
	Print(L"                                                                                ");		
	GUI_SetPosition (0,21);
	Print(L" PgUp/PgDn: scroll next/previoous page\n");
	GUI_SetPosition (0,22);
	Print(L" UP/DOWN  : select SMB Device\n");
	Print(L" Enter:   Read SMB Configuration Table    Esc: Exit!!");
    GUI_SetColor( 0x0F );
	GUI_SetColor (0x30);   
	for (GUI_i=0 ; GUI_i<=SCREENTOLERENCE ; GUI_i++){
	  GUI_SetPosition (70, 0+GUI_i);	
	  Print(L"          ");			
	}
	GUI_SetPosition (70, 1);
    Print(L" Function ");
	GUI_SetPosition (70, 3);
    Print(L"F1:SaveSMB");	
	GUI_SetColor( 0x0F );
	for (GUI_i=0 ; GUI_i< SCREENTOLERENCE; GUI_i++ ){
      if ( (GUI_i+ScreenListOffset) == SmbusDeviceCount ){
	    break;		  
	  }
	  AsciiStrToUnicodeStr( GuiCharBox[GUI_i+ScreenListOffset].CharBuff , TempUnicodeChar);
	  GUI_SetPosition(0, GUI_i+1); 
      Print(L"%s ",TempUnicodeChar);	   
	}	
	
// Step 5-4 Wait Kry for HII
    LightBarSnd(Columns,Rows,0,CharBoxIndex,GuiCharBox);
    GUI_WaitKey(Key);
	if(Key->ScanCode == 23){  //ScanCode:ESC -> leave the loop
	  break;
	}	
	if(Key->ScanCode == 1 ){  //ScanCode:Up 
	  if ( Rows == 1 ){               //boundary
		continue;
	  }	
	  LightBarSnd(Columns,Rows,Key->ScanCode,CharBoxIndex,GuiCharBox);
	  Rows--;   // because in out no use
	  CharBoxIndex--;
	}
	if(Key->ScanCode == 2 ){  //ScanCode:Down  
	  if ( CharBoxIndex == (SmbusDeviceCount-1) || Rows == SCREENTOLERENCE ){ //(RedriverCount-1) ) //boundary 
		continue;
	  }	
      LightBarSnd(Columns,Rows,Key->ScanCode,CharBoxIndex,GuiCharBox);
	  Rows++;
	  CharBoxIndex++;
	}	
	if(Key->ScanCode == 9 ){  //ScanCode: PgUp 
      if ( ScreenListOffset == 0 ){
		continue;
	  }	
	  ScreenListOffset -= SCREENTOLERENCE;
	  CharBoxIndex = ( (CharBoxIndex/SCREENTOLERENCE) -1 )*SCREENTOLERENCE; //reset to default @ ttha screen 
	  Rows = 1; //reset to default  
	  GUI_ScreenClean();		 
	  }	 	
	if(Key->ScanCode == 10 ){  //ScanCode: PgDn 
	  ScreenListOffset += SCREENTOLERENCE;
	  if ( ScreenListOffset >= SmbusDeviceCount ){
		ScreenListOffset -= SCREENTOLERENCE;
		continue;
	  }
      CharBoxIndex = ( (CharBoxIndex/SCREENTOLERENCE) + 1 )*SCREENTOLERENCE; //reset to default @ ttha screen 
	  Rows = 1; //reset to default  
	  GUI_ScreenClean();
	}		
    if((Key->ScanCode == 0) && (Key->UnicodeChar == 13) ){
	  GUI_ScreenClean();	  
	  SmbConfigSpace( gSmbus , DevAttr , CharBoxIndex ); // ,  mSmbDev[CharBoxIndex],mSmbTracePath[CharBoxIndex] , mSmbTraceIo[CharBoxIndex] ,gJsonObjectSize ,HadSpecReferencePointer );
	  GUI_ScreenClean();
	}	
	
  }
// Step 5-5 Reset to default after exit the GUI interface
  GUI_SetColor(0x0F);  
  GUI_ScreenClean();	
}  
	
// Final Step. Free the pool  
  if ( CloseLoopScnRes != NULL )
	free(CloseLoopScnRes);  
  if ( hierarchyTableLink != NULL )
	free(hierarchyTableLink);  
  if ( TempNodeStruct != NULL )
	free(TempNodeStruct);  
  if ( gNode != NULL )
	free(gNode);  
  if ( TraceSwAddress != NULL )
	free(TraceSwAddress);  
  if ( TraceSwChannel != NULL )
	free(TraceSwChannel);  
   if ( CascadeScanTable != NULL )
	free(CascadeScanTable);   
   if ( DevAttr != NULL )
	free(DevAttr);  
   if ( SimpleScanBuffer != NULL )
	free(SimpleScanBuffer);  
   if (ColMatrix != NULL)
    free(ColMatrix);
   if (Key != NULL)
	free(Key);
   if (GuiCharBox != NULL)
	free(GuiCharBox);
   if (TempASCIIChar != NULL)
    free(TempASCIIChar);	
   if (TempUnicodeChar != NULL)
	free(TempUnicodeChar);	
   if (gSmbExtFormat != NULL)
	free(gSmbExtFormat);	

   return EFI_SUCCESS;
}

EFI_STATUS
LogOutputJson( 
  IN  EFI_SMBUS_HC_PROTOCOL *mSmbus,
  IN  SmbDevAttribute   *DevsAttr, //!!!! change
  IN  CharStruct*       InhSmbTreeBuff,
  IN  UINT32*           InhKeyPointer                
 )
 {
  EFI_STATUS                  Status;	
  //EFI_SMBUS_DEVICE_ADDRESS	  SlaveAddress; 
  
  cJSON        *RootJson=NULL; 
  cJSON        *ArrayJson=NULL; 
  cJSON        *SmbAttrJson=NULL; 
  cJSON        *ArrayTraceJson=NULL; 
  cJSON        *ObjectTraceJson=NULL; 
  //cJSON        *ArrayConfigJson=NULL;   // 20180705
  //cJSON        *EmptyJsonDummy=NULL; 

  UINT32        i=0;
  UINT32        j=0;
  //UINT32        k=0;
  UINT32        m=0;
  UINT32        NumOfDevice=0;
  UINT32        LenthSwitch=0;
  UINT32        BitsBuffer=0;
  UINT32        DummysBuffer=0;
  
  //UINT8         TempWriteBuffer=0;
  //UINT8         *TempReadBuffer=NULL;
  UINT8         TempReadBuffer=0;
  //UINTN	        Length=0;  
  UINTN	        NonTraceLocation=0;  
  
  //char         *TempProject = "MH11-OC0";
  char         *TempAddress=NULL;
  char         *TempType=NULL;
  char         *TempDeviceID=NULL;
  char         *TempIO=NULL;
  char         *TempChannel=NULL;
  char         *TempConfiguration=NULL;
  char         *TempbyteConfig=NULL;
  char         *gProjectName = "Test";
  //char         *NulWord=NULL;
  char         *OutChar=NULL;  // wo allocate
  
  FILE         *fp=NULL;
  
////////////////////////////	

  TempAddress        = malloc(3*sizeof(char));	 // without this createobject string fail..
  TempType           = malloc(11*sizeof(char));	 // without this createobject string fail..
  TempDeviceID       = malloc(11*sizeof(char));	 // without this createobject string fail..
  TempIO             = malloc(3*sizeof(char));	 // without this createobject string fail..
  TempChannel        = malloc(3*sizeof(char));	 // without this createobject string fail..
  TempConfiguration  = malloc((512+1)*sizeof(char));	 // without this createobject string fail..
  //TempReadBuffer     = malloc(sizeof(UINT8));	 // without this createobject string fail..
  TempbyteConfig     = malloc(3*sizeof(char));
 
  
  NumOfDevice = *InhKeyPointer;		
   printf("Start create JSON log num:%d \n",NumOfDevice);
  RootJson = cJSON_CreateObject();	
  cJSON_AddItemToObject( RootJson, "Project", cJSON_CreateString(gProjectName) );  
  ArrayJson = cJSON_CreateArray ();	
	
  for ( j=0 ; j<NumOfDevice ; j++ ){
	strcpy(TempConfiguration,""); // normalized/reset offset
////////////////////////////	  
	//printf("CDYU 1\n");  
	if ( DevsAttr[j].YuAddress == 0xda  ) // CLK buffer config problem...
      continue;		
	
    SmbAttrJson = cJSON_CreateObject();	// ?? Need free after for loop ??
 	// Address JsonObject
	sprintf(TempAddress,"%02x",DevsAttr[j].YuAddress);
	cJSON_AddItemToObject( SmbAttrJson , "Address" , cJSON_CreateString(TempAddress) );
	// Type JsonObject
	for ( m=0 ; m<10 ; m++ ){
	  //memcpy ( TempType[m] , InhSmbTreeBuff[j].CharBuff[m+40] , sizeof(char) );
	  TempType[m] = InhSmbTreeBuff[j].CharBuff[m+40];
	}
	TempType[10] = '\0' ; 
	if (TempType[0]  == ' '){
       //cJSON_AddItemToObject( SmbAttrJson , "Type" , cJSON_CreateNull() );
       cJSON_AddStringToObject( SmbAttrJson , "Type" , "" );
	}
    else{
	   cJSON_AddItemToObject( SmbAttrJson , "Type" , cJSON_CreateString(TempType) );		
	}	

	// DeviceID JsonObject	
	for ( m=0 ; m<10 ; m++ ){
	  //memcpy ( TempDeviceID[m]  , InhSmbTreeBuff[j].CharBuff[m+50] , sizeof(char) );
	  TempDeviceID[m] = InhSmbTreeBuff[j].CharBuff[m+50];
	}
	TempDeviceID[10] = '\0' ; 
	if (TempDeviceID[0]  == ' '){
       //cJSON_AddItemToObject( SmbAttrJson , "DeviceID" , cJSON_CreateNull() );
       cJSON_AddStringToObject( SmbAttrJson , "DeviceID" , "" );
	}
    else{
	   cJSON_AddItemToObject( SmbAttrJson , "DeviceID" , cJSON_CreateString(TempDeviceID) );		
	}		
	
	// Trace Array
	ArrayTraceJson = cJSON_CreateArray ();	// ?? Need free after for loop ??
	
	
	
	for ( i=0 ; DevsAttr[j].TraceSw[i] != 0xFF ; i++ ){  
	    //printf("DevsAttr[j].TraceSw[i]:%x Trace on j: %d i: %d\n",DevsAttr[j].TraceSw[i],j,i);	
    	ObjectTraceJson = cJSON_CreateObject();  
	// Trace IO JsonObject	
	    sprintf(TempIO,"%02x",DevsAttr[j].TraceSw[i]);
		TempIO[2]='\0';
		cJSON_AddItemToObject(ObjectTraceJson,"IO",cJSON_CreateString(TempIO));	
        
	// Trace Channel JsonObject	
        sprintf(TempChannel,"%02x",DevsAttr[j].TraceCh[i]);
		TempChannel[2]='\0';
		cJSON_AddItemToObject(ObjectTraceJson,"Channel",cJSON_CreateString(TempChannel));			
	// add objectIO & Channel to "Trace" Array	
        cJSON_AddItemToArray( ArrayTraceJson , ObjectTraceJson );		
	}
	
	if ( (DevsAttr[j].TraceSw[i] == 0xFF) && (i == 0) ){
		 //printf("No trace: DevsAttr[j].TraceSw[i]:%x  No Trace on j:%d i:%d\n",DevsAttr[j].TraceSw[i],j,i);	
		 ObjectTraceJson = cJSON_CreateObject(); 
		 //cJSON_AddItemToObject(ObjectTraceJson,"IO",cJSON_CreateNull());
		 cJSON_AddStringToObject(ObjectTraceJson,"IO","");
		 //cJSON_AddItemToObject(ObjectTraceJson,"Channel",cJSON_CreateNull());
		 cJSON_AddStringToObject(ObjectTraceJson,"Channel","");
	// add objectIO & Channel to "Trace" Array	
         cJSON_AddItemToArray( ArrayTraceJson , ObjectTraceJson );		 
		 NonTraceLocation = 1;
	}
	//cJSON_AddItemToArray( ArrayTraceJson , ObjectTraceJson );	
	
	//printf("CDYU 2\n");
	// hook Array "Trace" to Object "SmbAttrJson"  
	cJSON_AddItemToObject(SmbAttrJson , "Trace" , ArrayTraceJson );
    // Configuration Array -->20180705 Object contain config string beam
	//ArrayConfigJson = cJSON_CreateArray();
	
	
	
	// Read smbus device configuration   ================= 
	  // 1. open the trace   
    LenthSwitch = 0;
	if( NonTraceLocation ){ 
	  NonTraceLocation = 0;
      //printf("Doesnt Need to Seitch j:%d i:%d\n",j,i);	
	}
	else{	  
      while ( DevsAttr[j].TraceSw[LenthSwitch] != 0xFF ){
	    BitsBuffer = 0;  
	    BitsBuffer = 1 << DevsAttr[j].TraceCh[LenthSwitch]; 
        Status = SmbusPorting( mSmbus, DevsAttr[j].TraceSw[LenthSwitch] , EfiSmbusWriteByte, 0 , (UINT8*) &BitsBuffer );    	  
	    LenthSwitch++;  
      }
	} // else{  
	  // 2. read the value
         for (i=0 ; i<256 ; i++){
		   TempReadBuffer = 0;	 
           Status = SmbusPorting( mSmbus, DevsAttr[j].YuAddress , EfiSmbusReadByte, i , &TempReadBuffer );
	       sprintf(TempbyteConfig,"%02x",TempReadBuffer);
		   TempbyteConfig[2]='\0';
		   strcat(TempConfiguration,TempbyteConfig);         
		 }	    

       for ( i=LenthSwitch; i>0 ;i-- ){
         DummysBuffer=0;
         Status = SmbusPorting( mSmbus, DevsAttr[j].TraceSw[i-1] , EfiSmbusWriteByte, 0 , (UINT8*) &DummysBuffer );    	      	
       }    
		 //printf("offset : %d %s \n",k,TempbyteConfig);
	     //cJSON_AddItemToArray( ArrayConfigJson , cJSON_CreateString(TempConfiguration) );	
	// Smbus Finish smbus device configuration   =================  
		//sprintf (TempConfiguration , "%0512x", TempReadBuffer); 
	    TempConfiguration[512] = '\0';
        cJSON_AddItemToObject( SmbAttrJson , "Configuration" , cJSON_CreateString(TempConfiguration) );	
        //cJSON_AddItemToObject( SmbAttrJson , "Configuration" , cJSON_CreateString(TempbyteConfig) );	
	
	
	// hook Array "Configuration" to Object "SmbAttrJson"
    //cJSON_AddItemToObject( SmbAttrJson , "Configuration" , ArrayConfigJson );
	// hook Object "SmbAttrJson" to Array "ArrayJson"
    cJSON_AddItemToArray( ArrayJson , SmbAttrJson );	  

  }  	
  
  cJSON_AddItemToObject( RootJson, "SmbHierachy", ArrayJson );
   	
  printf("Waiting for create JSON file...\n");
  OutChar = cJSON_Print(RootJson);  // wait malloc/free
  cJSON_Delete(RootJson);
  fp = fopen("SmbusInfo.json","w");
  if(fp != NULL){
    fwrite(OutChar,strlen(OutChar),1,fp);
    fclose(fp);	
  }    
  
  if (OutChar!=NULL)
    free(OutChar);
  if (TempAddress!=NULL)
	free(TempAddress); 
  if (TempType!=NULL)
	free(TempType); 
  if (TempDeviceID!=NULL)
	free(TempDeviceID); 
  if (TempIO!=NULL)
	free(TempIO); 
  if (TempChannel!=NULL)
	free(TempChannel); 
  if (TempConfiguration!=NULL)
	free(TempConfiguration); 
  //if (TempReadBuffer!=NULL)
	//free(TempReadBuffer); 
  if (TempbyteConfig!=NULL)
	free(TempbyteConfig); 

  return EFI_SUCCESS;	
}



EFI_STATUS
ExfileImport(
  ExtJsonFile    *SmbExtFormat,
  NodeStruct     *eMode,
  UINT32         *DeviceAttrCount
  //int argc, char* argv[]
  )
{
  FILE    *fp=NULL;
  long    len = 0;
  char    *data=NULL;   
  //char    *TempStr=NULL;   
  cJSON   *RootJson=NULL;
  cJSON   *ClassJson=NULL;
  cJSON   *SubClasJson=NULL;
  cJSON   *SubOemJson=NULL;
  cJSON   *AddressJson=NULL;
  cJSON   *DeviceIdJson=NULL;
  cJSON   *DeviceTypeJson=NULL;
  cJSON   *InitCondJson=NULL;
  cJSON   *OddJson=NULL;
  cJSON   *OemDefined=NULL;
  cJSON   *EvenJson=NULL;
  //cJSON   *=NULL;
  UINT32   ModuleArraSize = 0;
  UINT32   SlotArraSize = 0;
  UINT32   i = 0;
  UINT32   j = 0;
  UINT32   eModOffset = 0;
  UINT32   ExtFormatOffset = 0;
  UINT32   GetAddressSuccessFlag = 0;
  UINT32   leStr = 0;
 // UINT8    *TempAddrss=NULL;
  
/////////////////////////
  //TempStr = malloc(100*sizeof(char));

  
// step 1. Open the file and read the data into buffer
  fp = fopen("config/GbtSmbTree.json", "rb");
  if(fp == NULL){	
    printf("File 'GbtSmbTree.json' Can't Open\n");
    return EFI_OUT_OF_RESOURCES;
  }
  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  data = malloc((len + 1) * sizeof(char));  // debug pointer or not?
  fseek(fp, 0, SEEK_SET);	
  fread(data, 1, len, fp);
  fclose(fp);  
// step 2  Parse Json Format data
  RootJson = cJSON_Parse(data);  
  if (data != NULL){
	free(data);  // sometime it will crash  // free after json locate	  	
  }
  if ( RootJson == NULL )  {  
    //printf("Error before: [%s]\n",cJSON_GetErrorPtr());
    return EFI_OUT_OF_RESOURCES;
  }   

 // TempAddrss = malloc(sizeof(UINT8));  // danger
  
  ClassJson = RootJson->child;
  while( ClassJson != NULL ){
	//memcpy( ClassJson->string , TempStr , ( strlen( ClassJson->string ) ) );
	SubClasJson = cJSON_GetObjectItem( RootJson , ClassJson->string );
	if ( SubClasJson == NULL ){
	  //printf("%s Module Fail \n",ClassJson->string);	
	  ClassJson = ClassJson->next;
	  continue;  //crash module, garbage module!! loop next one and parse!!
	}
	
	ModuleArraSize = (UINT32) cJSON_GetArraySize( SubClasJson );
	//printf("ModuleArraSize: %d\n",ModuleArraSize);
	
	for ( i=0 ;i<ModuleArraSize ; i++){
	  //printf("ModuleArray No:%d get array item \n",i);
	  OddJson = cJSON_GetArrayItem( SubClasJson , i );	
	  if ( OddJson == NULL ){	
		//printf("Array Fail, break;");
		break; // crash array, garbage array!! discard!!
 	  }		
		
	  AddressJson    = cJSON_GetObjectItem( OddJson , "Address" );
	  GetAddressSuccessFlag=0;
	  if ( AddressJson == NULL ){
	    //printf("No:%d get address fail! \n",i);	
		continue; // danger
 	  }
	  else{
		if ( strlen(AddressJson->valuestring) != 0){
          //gSmbExtFormat[0].YuAddress e2 -- 0  // will overwrite bwlow register!!!!!
		  
	      sscanf( AddressJson->valuestring , "%x" ,   &(SmbExtFormat[ExtFormatOffset].JxAddress) );
	      (*DeviceAttrCount)++;		  
	      GetAddressSuccessFlag=1;
		}  
	  }
	  
	  
	  DeviceIdJson   = cJSON_GetObjectItem( OddJson , "DeviceID" );
	  if ( DeviceIdJson == NULL ){
	    //printf("No:%d get DID fail! \n",i);	
	  }	
	  else{
	    if ( strlen(DeviceIdJson->valuestring) != 0){	
		  memcpy( SmbExtFormat[ExtFormatOffset].DeviceID , DeviceIdJson->valuestring , (strlen(DeviceIdJson->valuestring)+1)  );  
	    }
	  }
	  
	  DeviceTypeJson = cJSON_GetObjectItem( OddJson , "DeviceType" );
	  if ( DeviceTypeJson == NULL ){
	    //printf("No:%d get Type fail! \n",i);	
	  }	  
	  else{
		if ( strlen(DeviceTypeJson->valuestring) != 0){
		  memcpy( SmbExtFormat[ExtFormatOffset].DeviceType , DeviceTypeJson->valuestring , (strlen(DeviceTypeJson->valuestring)+1)  );  
	      if ( (strcmp ( DeviceTypeJson->valuestring , "SmbSW" )==0) && (GetAddressSuccessFlag) ){
	        sscanf( AddressJson->valuestring , "%x" , & (eMode[eModOffset].NodeAddress) );
	        eMode[eModOffset].NodeChannel = 8; // whatever... I dont wont to figure do divide anymore...
            eModOffset++;
	      }	  
		}  
	  }
	  
	  InitCondJson = cJSON_GetObjectItem( OddJson , "InitCond" );
	  if ( InitCondJson == NULL ){
	    //printf("No:%d get I.C. fail! \n",i);	
	  }	   
	  else{
		if (strlen(InitCondJson->valuestring) != 0){	
          //printf("SmbExtFormat[ExtFormatOffset].JxAddress %x  FF\n",SmbExtFormat[ExtFormatOffset].JxAddress );			  
          //printf("A%s  FF\n",InitCondJson->valuestring );			  
          leStr = strlen(InitCondJson->valuestring)/(5*sizeof(char));
          for (j=0 ; j<leStr ;j++){
           sscanf ( InitCondJson->valuestring ,"|%x-%x",&(SmbExtFormat[ExtFormatOffset].JxSw[j]) , &(SmbExtFormat[ExtFormatOffset].JxCh[j]) );     
		   InitCondJson->valuestring += 5*sizeof(char); 
          }
		  /*
		  SmbExtFormat[ExtFormatOffset].JxSw[leStr] = 0xAF;
		  SmbExtFormat[ExtFormatOffset].JxSw[leStr+1] = 0xAF;
		  SmbExtFormat[ExtFormatOffset].JxSw[leStr+2] = 0xAF;
		  SmbExtFormat[ExtFormatOffset].JxCh[leStr] = 0xAF;
		  SmbExtFormat[ExtFormatOffset].JxCh[leStr+1] = 0xAF;
		  SmbExtFormat[ExtFormatOffset].JxCh[leStr+2] = 0xAF;	
          */		  
	  //sscanf(InitCondJson->valuestring  , "%02x-%x"  , &(SmbExtFormat[ExtFormatOffset].JxSw[0]) , &(SmbExtFormat[ExtFormatOffset].JxCh[0]) );
	    }
	  }
      // surgrey  
	  memcpy( SmbExtFormat[ExtFormatOffset].ModuleName , ClassJson->string , (strlen(ClassJson->string)+1)  );  
	  

	  //printf("\n");
	  //printf("ExtFormatOffset: %d\n",ExtFormatOffset);
	  //printf("%s \n",SmbExtFormat[ExtFormatOffset].ModuleName );
	  //printf(" %s \n",SmbExtFormat[ExtFormatOffset].DeviceType);
	  //printf(" %s \n",SmbExtFormat[ExtFormatOffset].DeviceID);
	  //printf(" %x \n",SmbExtFormat[ExtFormatOffset].JxAddress);
	  //printf(" %s\n",InitCondJson->valuestring);
	  //printf("\n");	
	  
      OemDefined = cJSON_GetObjectItem ( OddJson , "OemDefined" );
	  if ( OemDefined == NULL ){
	    //printf("No:%d Without OemDefined \n",i);	
	  }	    
	  else{
		SlotArraSize = (UINT32) cJSON_GetArraySize( OemDefined );  
		//printf("SlotArraSize: %d\n",SlotArraSize);
		for ( j=0 ; j<SlotArraSize ;j++ ){
		  EvenJson = cJSON_GetArrayItem( OemDefined , j );	
 	      if ( EvenJson == NULL ){	
	    	//printf("Array Fail, break;");
			break; // crash array, garbage array!! discard!!
 	      }	         	
          SubOemJson = EvenJson->child;
		  //printf("%s : %d ",SubOemJson->string,SubOemJson->valueint); 
    	  if ( strlen(SubOemJson->string) != 0 ){
		    memcpy( SmbExtFormat[ExtFormatOffset].OemDefined[j] , SubOemJson->string , (strlen(SubOemJson->string)+1) );	  
    	  //}
		  //if ( SubOemJson->valueint != NULL ){
		    memcpy( &SmbExtFormat[ExtFormatOffset].Reference[j] , (UINT8*)&(SubOemJson->valueint) , sizeof(UINT8) );	  
		  }
		  EvenJson = EvenJson->next;
		}	// for ( j=0 ; j<SlotArraSize ;j++ )
		//printf("\n");	
	  } // if ( OemDefined == NULL )
	  ExtFormatOffset++;
	} // for ( i=0 ;i<ModuleArraSize ; i++)
	
	ClassJson = ClassJson->next;
  }	// while( ClassJson != NULL )
	
  //it (TempAddrss != NULL)
   // free(TempAddrss);
	
  cJSON_Delete(RootJson);		
  return EFI_SUCCESS;
}


EFI_STATUS
DumpConfigBIN(
  UINT8                   ConfigSize,      // 1: byte  / 2: word / 4: block 
  SmbDevAttribute         *DevsmAttr,
  UINT32                  CharsBoxmIndex,
  UINT8                   *mlBUffer,
  UINT32                  TurnsDump
)
{
   FILE    *fcv = NULL;	
   UINT8   ByPassBuf = 0xAF;
   UINT8   *TempBuf=NULL;
   UINT32   LoopCount=0;
   UINT32   i;

///////////////////////	

  fcv = fopen( "SmbusConfiguration.bin" ,"ab");
  if ( fcv == NULL ){
	return EFI_LOAD_ERROR;
  }
  
   
  TempBuf = malloc(sizeof(UINT8)); 
  
  switch (ConfigSize){
	case 1: // byte 
         //for (i=0 ; i<0x100 ; i++){
		   //fwrite( &ByPassBuf ,sizeof(UINT8),sizeof(UINT8) ,	fcv);
	     //}
		 //fputs(&ByPassBuf,fcv);	  	
	    
          //fseek(fcv, 0, SEEK_SET);   // reset the location of pointer	 
		  
		  //Print(L"HHH %x ",DevsmAttr[CharsBoxmIndex].YuAddress);
 		  //TempBuf = DevsmAttr[CharsBoxmIndex].YuAddress;	  
		  memcpy( TempBuf, &DevsmAttr[CharsBoxmIndex].YuAddress ,sizeof(UINT8) );
		  fwrite( TempBuf ,sizeof(UINT8),sizeof(UINT8) ,	fcv);
		  for (i=0 ; i<15 ; i++){
		    fwrite(&ByPassBuf ,sizeof(UINT8),sizeof(UINT8), fcv  );			  
		    //replace fseek(fcv, 0, SEEK_SET); 
		  }

		  
		  //fputs(&TempBuf,fcv);  
		  
          //fseek(fcv, 16, SEEK_SET);   // reset the location of pointer
		  while (  DevsmAttr[CharsBoxmIndex].TraceSw[LoopCount] != 0xFF)  
		    LoopCount++;
		  for ( i=0 ; i<LoopCount ; i++){
			//TempBuf = DevsmAttr[CharsBoxmIndex].TraceSw[i];
			memcpy( TempBuf, &DevsmAttr[CharsBoxmIndex].TraceSw[i] ,sizeof(UINT8) );
			fwrite( TempBuf ,sizeof(UINT8),sizeof(UINT8) ,	fcv);
			//fputs(&TempBuf,fcv);   
		  } 
		  for (i=0 ; i< (16-LoopCount) ; i++){
		    fwrite(&ByPassBuf ,sizeof(UINT8),sizeof(UINT8), fcv  );			  
		    //replacefseek(fcv, 16, SEEK_SET); 			  
		  }

          //fseek(fcv, 32, SEEK_SET);   // reset the location of pointer	 
		  for ( i=0 ; i<LoopCount ; i++){
			//TempBuf = DevsmAttr[CharsBoxmIndex].TraceCh[i];
			memcpy( TempBuf, &DevsmAttr[CharsBoxmIndex].TraceCh[i] ,sizeof(UINT8) );			
			fwrite( TempBuf ,sizeof(UINT8),sizeof(UINT8) ,	fcv);
			//fputs(&TempBuf,fcv);   
		  } 
		  for (i=0 ; i< (16-LoopCount) ; i++){
		    fwrite(&ByPassBuf ,sizeof(UINT8),sizeof(UINT8), fcv  );			  
		    //replace fseek(fcv, 32, SEEK_SET); 		  
		  }		
		  for (i=0 ; i< 208 ; i++){
		    fwrite(&ByPassBuf ,sizeof(UINT8),sizeof(UINT8), fcv  );			  	  
		  }				  

        fseek(fcv, 256, SEEK_SET);   // reset the location of pointer	 
        fwrite(mlBUffer,sizeof(UINT8),256*sizeof(UINT8),fcv);	   
		break;
	case 2: // word 
        fwrite(mlBUffer,sizeof(UINT16),256*sizeof(UINT8),fcv); 
		break;
	case 4: // block 
	  break;	
	default:
	  break;
  }
  fclose(fcv);
  if (TempBuf != NULL)
	free(TempBuf);  
 return EFI_SUCCESS;
}


EFI_STATUS
CheckSmbusParameter(int argc, char** argv)
{
	if(strcasecmp(argv[2], "byte") == 0) gSmbusOperation = EfiSmbusReadByte;
	else if(strcasecmp(argv[2], "word") == 0) gSmbusOperation = EfiSmbusReadWord;
	else if(strcasecmp(argv[2], "block") == 0) gSmbusOperation = EfiSmbusReadBlock;
	else{
		// Type input Arg string wrong
		gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTRED);
		printf("Input wrong Smbus Type\n");
		return EFI_INVALID_PARAMETER;
	}

	return EFI_SUCCESS;
}

EFI_STATUS
Arg_r_Function(int argc, char** argv)
{
	EFI_STATUS	                Status;
	EFI_SMBUS_DEVICE_ADDRESS	SlaveAddress;
	UINTN	                    Length;
	UINT8	                    i;
////////////////////////////////////////
    gBuffer = malloc( 512* sizeof(UINT8));
    //printf("Step 1. Locate the Smbus protocol\n");
    Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID**)&gSmbus);
    if(EFI_ERROR(Status)){
  	  return EFI_INVALID_PARAMETER; 
    } 	

	Status = CheckSmbusParameter(argc, argv);
	if(EFI_ERROR(Status)) return Status;

	gSmbusAddress = (UINT8)strtol(argv[3], NULL, 16);
	gSmbusOffset = (UINT8)strtol(argv[4], NULL, 16);

	switch(gSmbusOperation){
		case EfiSmbusReadByte:
			Length = 1;
			break;
		case EfiSmbusReadWord:
			Length = 2;
			break;
		case EfiSmbusReadBlock:
			Length = 0x20;
	}

	SlaveAddress.SmbusDeviceAddress = gSmbusAddress >> 1;
	Status = gSmbus->Execute (
		gSmbus,
		SlaveAddress,
		gSmbusOffset,
		gSmbusOperation,
		FALSE,
		&Length,
		gBuffer);
	if(EFI_ERROR(Status)){
		gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTRED);
		printf("Access Failure\n");
		return Status;
	}

	// Print result
	gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
	printf("Address: 0x%02X, Offset: 0x%02X \n", gSmbusAddress, gSmbusOffset);
	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGREEN);
	switch(gSmbusOperation){
		case EfiSmbusReadByte:
			printf("Read-Byte Data: 0x%02X\n", gBuffer[0]);
			break;
		case EfiSmbusReadWord:
			printf("Read-Word Data: 0x%04X\n", (UINT16)gBuffer[1] << 8 | gBuffer[0]);Length = 2;
			break;
		case EfiSmbusReadBlock:
			printf("Read-Block:");
			for(i = 0; i < Length; i++){
				if(i % 4 == 0) printf("\n");
				printf("Data[0x%02X]:0x%02X ", i + gSmbusOffset, gBuffer[i]);
			}
			printf("\n");
	}
	
	if(gBuffer != NULL)
	  free(gBuffer);	 
  
	return EFI_SUCCESS;
}

EFI_STATUS
Arg_w_Function(int argc, char** argv)
{
	EFI_STATUS	                Status;
	EFI_SMBUS_DEVICE_ADDRESS	SlaveAddress;
	UINTN	                    Length;
	UINT8	                    i;
/////////////////////////////////////////////	
    gBuffer = malloc( 512* sizeof(UINT8));
// Step 1. Locate Smbus Protocol  
    //printf("Step 1. Locate the Smbus protocol\n");
    Status = gBS->LocateProtocol(&gEfiSmbusHcProtocolGuid, NULL, (VOID**)&gSmbus);
    if(EFI_ERROR(Status)){
  	  return EFI_INVALID_PARAMETER; 
    } 



	Status = CheckSmbusParameter(argc, argv);
	if(EFI_ERROR(Status)) return Status;

	gSmbusAddress = (UINT8)strtol(argv[3], NULL, 16);
	gSmbusOffset = (UINT8)strtol(argv[4], NULL, 16);

	switch(gSmbusOperation){
		case EfiSmbusReadByte:
			Length = 1;
			gBuffer[0] = (UINT8)strtol(argv[5], NULL, 16);
			break;
		case EfiSmbusReadWord:
			Length = 2;
			gBuffer[0] = (UINT8)strtol(argv[5], NULL, 16);
			gBuffer[1] = ((UINT16)strtol(argv[5], NULL, 16) >> 8) & 0xFF;
			break;
		case EfiSmbusReadBlock:
			Length = argc - 5;
			for(i = 0; i < Length; i++){
				gBuffer[i] = (UINT8)strtol(argv[5 + i], NULL, 16);
			}
	}

	SlaveAddress.SmbusDeviceAddress = gSmbusAddress >> 1;
	Status = gSmbus->Execute (
		gSmbus,
		SlaveAddress,
		gSmbusOffset,
		gSmbusOperation + 1,
		FALSE,
		&Length,
		gBuffer);
	if(EFI_ERROR(Status)){
		gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTRED);
		printf("Access Failure\n");
	    if(gBuffer != NULL)
	      free(gBuffer);	 		
		return Status;
	}

	// Print result
	gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE);
	printf("Address: 0x%02X, Offset: 0x%02X \n", gSmbusAddress, gSmbusOffset);
	gST->ConOut->SetAttribute(gST->ConOut, EFI_LIGHTGREEN);
	switch(gSmbusOperation){
		case EfiSmbusReadByte:
			printf("Write-Byte Data: 0x%02X\n", gBuffer[0]);
			break;
		case EfiSmbusReadWord:
			printf("Write-Word Data: 0x%04X\n", (UINT16)gBuffer[1] << 8 | gBuffer[0]);
			break;
		case EfiSmbusReadBlock:
			printf("Write-Block");
			for(i = 0; i < Length; i++){
				if(i % 4 == 0) printf("\n");
				printf("Data[0x%02X]:0x%02X ", i + gSmbusOffset, gBuffer[i]);
			}
			printf("\n");
	}
	
	if(gBuffer != NULL)
	  free(gBuffer);	 
	
	return EFI_SUCCESS;
}