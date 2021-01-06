/*
 * Debug to Memory

//***********EfiLib.c start**********************
#define dbBaseAddress 0x20000000

void Dbg1(CHAR16* str,UINTN Val)
{

 UINT8 i=0;
 UINTN pLast=0;
 CHAR16 tempBuf[100];
 EFI_STATUS Status;
 static UINTN Address=dbBaseAddress; 
 
 if(*(volatile UINT32*)Address!=0x99aa8877) 
	 {
   	   Status=pBS->AllocatePages(AllocateAddress,EfiBootServicesData,5,&Address);//20K buff
       if(EFI_ERROR(Status)) return;
	 }

 
 Swprintf(tempBuf,L"%s %X",str,Val);
 
 
 //4 bytes signature,2 bytes data len. 0x10~,data
 if(*(volatile UINT32*)Address!=0x99aa8877) 
  {//New
    *(volatile UINT32*)Address=0x99aa8877;
    pLast=0;
  }else
  {
    pLast=*(volatile UINT16*)(Address+4);	  
  }


 while(tempBuf[i]!=0)
 {
  *(volatile UINT16*)(Address+0x10+pLast) = tempBuf[i];
  i++;
  pLast=pLast+2;
 }
 
 *(volatile UINT16*)(Address+0x10+pLast)=0x00;
 pLast=pLast+2;
 
 *(volatile UINT16*)(Address+4)=(UINT16)pLast;
 
}
//***********EfiLib.c end**********************

//************AmiDxeLib.h Start******
void Dbg1(CHAR16* str,UINTN Val); //;wwp+
//************AmiDxeLib.h end********

 */


#include <Efi.h>
#include <Guid/GlobalVariable.h>
#include <token.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SmmAccess2.h>
#include <Library/DebugLib.h>
#include <AmiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimpleTextIn.h>
#include <Guid/FileInfo.h>
#include <AmiDxeLib.h>

/*
Function: Read BIOS to fileBuff from Disk in root director.

*/
BOOLEAN ReadBiosFromDisk(CHAR16* fileName,void** fileBuff,UINTN* Len)
{
  UINTN                           NumberFileSystemHandles;
  EFI_HANDLE                      *FileSystemHandles;
  UINTN                           Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileVolume;
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL*                 RootFileHandle;
  EFI_FILE_PROTOCOL*                 FileHandle;
  UINTN fileSize;
  EFI_FILE_INFO fileInfo;
  EFI_GUID fileInfoGuid=EFI_FILE_INFO_ID;

  pBS->LocateHandleBuffer (ByProtocol,&gEfiSimpleFileSystemProtocolGuid,NULL,&NumberFileSystemHandles,&FileSystemHandles);

  for (Index = 0; Index < NumberFileSystemHandles; Index++)
  {
    //
    Status = gBS->HandleProtocol (
                    FileSystemHandles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **) &FileVolume
                    );

    // Open Volume
    Status = FileVolume->OpenVolume (FileVolume, &RootFileHandle);

    // Open BIOS file
    Status = RootFileHandle->Open (RootFileHandle,&FileHandle,fileName,EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,0);
    if(EFI_ERROR (Status)) break;

    //Get fileSize
    Status=FileHandle->GetInfo(FileHandle,&fileInfoGuid,&fileSize,&fileInfo);
    Status=FileHandle->GetInfo(FileHandle,&fileInfoGuid,&fileSize,&fileInfo);
    fileSize=fileInfo.FileSize;
    *Len=fileSize;

    //Read BIOS
    pBS->AllocatePool(EfiBootServicesData,fileSize,fileBuff);
    Status=FileHandle->Read(FileHandle,&fileSize,*fileBuff);


    FileHandle->Close(FileHandle);


    return TRUE;
  }

return FALSE;
}

/*
*/
BOOLEAN WriteDisk(CHAR16* fileName,void* fileBuff)
{
  UINTN                           NumberFileSystemHandles;
  EFI_HANDLE                      *FileSystemHandles;
  UINTN                           Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileVolume;
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL*                 RootFileHandle;
  EFI_FILE_PROTOCOL*                 FileHandle;
  UINTN fileSize;
  EFI_GUID fileInfoGuid=EFI_FILE_INFO_ID;
  CHAR16 tempBuff[100];

  pBS->LocateHandleBuffer (ByProtocol,&gEfiSimpleFileSystemProtocolGuid,NULL,&NumberFileSystemHandles,&FileSystemHandles);

  fileSize=0;
  while( *((UINT8*)fileBuff+fileSize) !=0 ) fileSize=fileSize+1;

  
  for (Index = 0; Index < NumberFileSystemHandles; Index++)
  {
    //
    Status = gBS->HandleProtocol (
                    FileSystemHandles[Index],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **) &FileVolume
                    );

    //Open Volume
    Status = FileVolume->OpenVolume (FileVolume, &RootFileHandle);
    if(EFI_ERROR (Status)) 
    	{
    	Swprintf(tempBuff,L"OpenVolume()=%r \r\n",Status);    
        gST->ConOut->OutputString (gST->ConOut, tempBuff);    
    	break;
    	}

    //Open BIOS file
    Status = RootFileHandle->Open (RootFileHandle,&FileHandle,fileName, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,0);
    if(EFI_ERROR (Status)) 
    	{
    	Swprintf(tempBuff,L"Open for Delete=%r \r\n",Status);    
        gST->ConOut->OutputString (gST->ConOut, tempBuff);    
    	break;
    	}
    Status=FileHandle->Delete(FileHandle);
    if(EFI_ERROR (Status)) 
    	{
    	Swprintf(tempBuff,L"Delete()=%r \r\n",Status);    
        gST->ConOut->OutputString (gST->ConOut, tempBuff);    
    	break;
    	}    
    
    //Open BIOS file
    Status = RootFileHandle->Open (RootFileHandle,&FileHandle,fileName, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,0);
    if(EFI_ERROR (Status)) 
    	{
    	Swprintf(tempBuff,L"Open for Write=%r \r\n",Status);    
        gST->ConOut->OutputString (gST->ConOut, tempBuff);    
    	break;
    	}
    
    //Write BIOS
    Status=FileHandle->Write(FileHandle,&fileSize,fileBuff);


    FileHandle->Close(FileHandle);


    return TRUE;
  }

return FALSE;
}



//
// Module entry point
//
#define maxBufSize 1024*20

CHAR16 traceBuf[maxBufSize];
CHAR8  fileBuff[maxBufSize];
EFI_STATUS
EFIAPI
JwApp1EntryPoint (
    IN EFI_HANDLE                     ImageHandle,
    IN EFI_SYSTEM_TABLE               *SystemTable )
{
	
    UINTN Address=0; 
    UINT16 i=0,j=0;
    UINTN pLast=0;
    CHAR16 tempBuff[100];

	    
    InitAmiLib(ImageHandle, SystemTable);

 	     			

    Address=0x20000000;
   if(*(volatile UINT32*)Address!=0x99aa8877) 
	   {
		Swprintf(tempBuff,L"No DebugLog:  %X \r\n",Address);    
	    gST->ConOut->OutputString (gST->ConOut, tempBuff);    	   
	    return EFI_SUCCESS;
	   }
   
   
    
   
    //Show Debug Log to screen
    gST->ConOut->ClearScreen  (gST->ConOut);
	Swprintf(tempBuff,L"Debug Information: 0x%X  Length=%i \r\n",Address,(*(volatile UINT16*)(Address+4))/2);    
    gST->ConOut->OutputString (gST->ConOut, tempBuff);    
    
    for(i=0;i<*(volatile UINT16*)(Address+4);i=i+2)
    {

	    if(*(volatile UINT16*)(Address+0x10 +i)!=0)
	    {
		    tempBuff[j]=*(volatile UINT16*)(Address+i+0x10);
		    j++;
	    }else
	    {
		    tempBuff[j]=0;
		    gST->ConOut->OutputString (gST->ConOut, tempBuff);   
		    gST->ConOut->OutputString (gST->ConOut, L"\r\n");   
	        j=0;
	    }
    }

    //Save DebugLog to file
    j=0;
    for(i=0;i<*(volatile UINT16*)(Address+4);i=i+2)
    {

	    if(*(volatile UINT16*)(Address+0x10 +i)!=0)
	    {
	    	fileBuff[j]=*(volatile UINT8*)(Address+i+0x10);
		    j++;
	    }else
	    {
	    	fileBuff[j]=0x0d;
	    	fileBuff[j+1]=0x0a;
	        j=j+2;
	    }
    }
    fileBuff[j]= 0;
         

    gST->ConOut->OutputString (gST->ConOut, L"\r\nWrite to file...");       
    WriteDisk(L"Trace.txt",fileBuff);
    gST->ConOut->OutputString (gST->ConOut, L"Done\r\n");           
    return EFI_SUCCESS;
}

