//---------------------------------------------------------------------------
#include <vcl.h>
#include <ShlObj.h>
#include <shellapi.h>
#include <Registry.hpp>
#include <tlhelp32.h>
#pragma hdrstop
#include "BackupAQQFrm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "XPMan"
#pragma link "LMDBaseEdit"
#pragma link "LMDBrowseEdit"
#pragma link "LMDControl"
#pragma link "LMDCustomBevelPanel"
#pragma link "LMDCustomBrowseEdit"
#pragma link "LMDCustomControl"
#pragma link "LMDCustomEdit"
#pragma link "LMDCustomPanel"
#pragma link "LMDPNGImage"
#pragma link "IdThreadComponent"
#pragma resource "*.dfm"
TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
AnsiString ProfilesPath; //do sciezki profilów

char this_title[2048];  //do nazwy/tekstu okna
AnsiString Progress; //j.w.
HWND Hwnd; //do uchwytu okna 7-Zip
DWORD procesID;
AnsiString ExeName;
//---------------------------------------------------------------------------
__fastcall TBackupAQQForm::TBackupAQQForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall ExecuteApplication(AnsiString FileName, AnsiString param, HWND h)
{
  FileName = StringReplace(FileName, "\\", "\\\\", TReplaceFlags() << rfReplaceAll);

  SHELLEXECUTEINFO sei;
  memset(&sei, 0, sizeof (sei));
  sei.cbSize = sizeof(sei);
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;
  sei.hwnd = h;
  sei.lpVerb = "open";
  sei.lpFile = FileName.c_str();
  sei.lpParameters = param.c_str();
  sei.nShow = SW_HIDE;

  if(ShellExecuteEx(&sei))
  try
  {
    WaitForSingleObject(sei.hProcess, INFINITE);
  }
  __finally
  {
    CloseHandle(sei.hProcess);
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::FindDir(TListBox *lista, AnsiString Dir) //Funkcja szukaj¹ca profile
{
  TSearchRec sr;

  if(FindFirst(Dir + "*.*", faAnyFile, sr) == 0)
     {
      do{
         if(((sr.Attr & faDirectory) > 0) & (sr.Name != ".") & (sr.Name != ".."))
         {
            lista->Items->Add(sr.Name);
         }
        }
        while(FindNext(sr) == 0);
        FindClose(sr);
     }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aGetProfilesListExecute(TObject *Sender)
{
  ProfilesListBox->Clear();

  bool ProfilesInAQQExePath=0;

  TRegistry *Reg = new TRegistry();
  Reg->RootKey=HKEY_CURRENT_USER;
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
    Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
    ProfilesPath=Reg->ReadString("Path");
    ProfilesPath.SetLength(ProfilesPath.Length()-7);
    ProfilesPath=ProfilesPath+"Profiles\\";
    //profile w g³ównym folderze AQQ
    if(DirectoryExists(ProfilesPath))
    {
      FindDir(ProfilesListBox,ProfilesPath);
      ProfilesInAQQExePath=1;
    }
  }
  Reg->CloseKey();
  delete Reg;

  if(ProfilesInAQQExePath==0)
  {
    //funkcja dostepu do profilu uzytkownika w Windows'ie
    LPITEMIDLIST pidl;
    LPMALLOC     pShellMalloc;
    char         ProfileDir[MAX_PATH];

    if (SUCCEEDED(SHGetMalloc(&pShellMalloc)))
      if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidl)))
      {
        if (SHGetPathFromIDList(pidl, ProfileDir))
        ProfilesPath=ProfileDir;
        pShellMalloc->Free(pidl);
      }

    pShellMalloc->Release();

    ProfilesPath=ProfilesPath+"\\Wapster\\AQQ Folder\\Profiles\\";

    //wywolanie funkcji szukajacej profile
    FindDir(ProfilesListBox,ProfilesPath);
  }

  if(ProfilesListBox->ItemIndex!=0)
   ProfilesListBox->ItemIndex=0;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::FormShow(TObject *Sender)
{
  aGetProfilesList->Execute();

  if(!DirectoryExists(ExtractFilePath(Application->ExeName) + "\\Backups"))
   CreateDir(ExtractFilePath(Application->ExeName) + "\\\\Backups");

  BrowseEdit->Path=ExtractFilePath(Application->ExeName) + "\Backups";
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ExitButtonClick(TObject *Sender)
{
  Close();        
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aDoBackupExecute(TObject *Sender)
{
  BackupLabel->Caption="Proszê czekaæ! BackupAQQ zapisuje kopiê Twojego profilu. Mo¿e to potrwaæ nawet kilka minut.";

  MainTabControl->Visible=false;
  BackupTabControl->Visible=true;

  PreviousButton->Enabled=false;
  NextButton->Enabled=false;
  ExitButton->Enabled=false;

  BackupIdThreadComponent->Start();
}
//---------------------------------------------------------------------------
void __fastcall TBackupAQQForm::NextButtonClick(TObject *Sender)
{
  if(MainTabControl->Visible==true)
  {
    if(CreateBackupRadioButton->Checked==true)
    {
      aDoBackup->Execute();
    }
    if(RestoreProfileRadioButton->Checked==true)
    {
      MainTabControl->Visible=false;
      BackupsListTabControl->Visible=true;
      PreviousButton->Enabled=true;

      BackupsFileListBox->Clear();
      BackupsFileListBox->Directory=ExtractFileDir(Application->ExeName)+"\\Backups\\\\";

      if(BackupsFileListBox->Count!=0)
      {
        BackupsFileListBox->ItemIndex=0;
        NextButton->Enabled=true;
      }
      else
       NextButton->Enabled=false;
    }
  }
  else if(BackupsListTabControl->Visible==true)
  {
    aRestoreBackup->Execute();
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::PreviousButtonClick(TObject *Sender)
{
  if(BackupsListTabControl->Visible==true)
  {
    BackupsListTabControl->Visible=false;
    MainTabControl->Visible=true;
    PreviousButton->Enabled=false;
    NextButton->Enabled=true;

    aGetProfilesList->Execute();
  }
  else if(RestoreTabControl->Visible==true)
  {
    RestoreTabControl->Visible=false;
    MainTabControl->Visible=true;
    PreviousButton->Enabled=false;
    NextButton->Enabled=true;

    aGetProfilesList->Execute();
  }
  else if(BackupTabControl->Visible==true)
  {
    BackupTabControl->Visible=false;
    MainTabControl->Visible=true;
    PreviousButton->Enabled=false;
    NextButton->Enabled=true;

    aGetProfilesList->Execute();
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aRestoreBackupExecute(TObject *Sender)
{
  RestoreLabel->Caption="Proszê czekaæ! BackupAQQ przywraca Twój profil AQQ. Proces ten mo¿e potrwaæ nawet kilka minut.";

  BackupsListTabControl->Visible=false;
  RestoreTabControl->Visible=true;

  PreviousButton->Enabled=false;
  NextButton->Enabled=false;
  ExitButton->Enabled=false;

  RestoreIdThreadComponent->Start();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::BrowseButtonClick(TObject *Sender)
{
  OpenDialog->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::OpenDialogCanClose(TObject *Sender,
      bool &CanClose)
{
  if(OpenDialog->FileName!=NULL)
  {
    RestoreLabel->Caption="Proszê czekaæ! BackupAQQ przywraca Twój profil AQQ. Proces ten mo¿e potrwaæ nawet kilka minut.";

    RestoreTabControl->Visible=true;
    BackupsListTabControl->Visible=false;

    PreviousButton->Enabled=false;
    NextButton->Enabled=false;
    ExitButton->Enabled=false;

    ManualRestoreIdThreadComponent->Start();
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::BackupIdThreadComponentRun(
      TIdCustomThreadComponent *Sender)
{
  TDateTime Todey = TDateTime::CurrentDate();
  AnsiString BackupName = BrowseEdit->Path + "\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "_" + Todey +".aqqbackup";

  ProgressBar->Position=0;
  ProgressBar->Visible=true;
  Hwnd=NULL;
  GetProgressTimer->Enabled=true;

  AnsiString Exclude;

  if(PluginsCheckBox->Checked==true)
   Exclude = Exclude + " -x!" + AnsiQuotedStr("*\\Plugins",'"');
  if(ThemesCheckBox->Checked==true)
   Exclude = Exclude + " -x!" + AnsiQuotedStr("*\\Themes",'"');
  if(SmileysCheckBox->Checked==true)
   Exclude = Exclude + " -x!" + AnsiQuotedStr("*\\Smileys",'"');
  if(CustomEmotsCheckBox->Checked==true)
   Exclude = Exclude + " -x!" + AnsiQuotedStr("*\\CustomEmots",'"');
  if(TempCheckBox->Checked==true)
   Exclude = Exclude + " -x!" + AnsiQuotedStr("*\\Data\\Temp",'"');
  if(IncomingCheckBox->Checked==true)
   Exclude = Exclude + " -x!" + AnsiQuotedStr("*\\Incoming",'"');

  //Oczekiwanie na zakonczenie procesu
  if(Exclude.Length()>0)
   ExecuteApplication((ExtractFilePath(Application->ExeName)+"7z.exe"),("a " + AnsiQuotedStr(BackupName,'"') + " " + AnsiQuotedStr((ProfilesPath+ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex]),'"')) + Exclude, this);
  else
   ExecuteApplication((ExtractFilePath(Application->ExeName)+"7z.exe"),("a " + AnsiQuotedStr(BackupName,'"') + " " + AnsiQuotedStr((ProfilesPath+ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex]),'"')), this);

  ProgressBar->Visible=false;
  GetProgressTimer->Enabled=false;

  BackupLabel->Caption="Zakoñczono! BackupAQQ zrobi³ kopiê profilu " + AnsiQuotedStr(ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex], '"') + " do pliku " + AnsiQuotedStr(ExtractFileName(BackupName), '"') + ".";

  PreviousButton->Enabled=true;
  ExitButton->Enabled=true;

  BackupIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::RestoreIdThreadComponentRun(
      TIdCustomThreadComponent *Sender)
{
  bool ProfilesInAQQExePath=0;

  TRegistry *Reg = new TRegistry();
  Reg->RootKey=HKEY_CURRENT_USER;
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
    Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
    ProfilesPath=Reg->ReadString("Path");
    ProfilesPath.SetLength(ProfilesPath.Length()-7);
    ProfilesPath=ProfilesPath+"Profiles\\";
    //profile w g³ównym folderze AQQ
    if(DirectoryExists(ProfilesPath))
    {
      ProgressBar2->Position=0;
      ProgressBar2->Visible=true;
      Hwnd=NULL;
      GetProgressTimer->Enabled=true;

      //Oczekiwanie na zakonczenie procesu
      ExecuteApplication((ExtractFilePath(Application->ExeName)+"7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(BackupsFileListBox->FileName,'"')), this);
      ProfilesInAQQExePath=1;

      ProgressBar2->Visible=false;
      GetProgressTimer->Enabled=false;
    }
  }
  Reg->CloseKey();
  delete Reg;

  if(ProfilesInAQQExePath==0)
  {
    //funkcja dostepu do profilu uzytkownika w Windows'ie
    LPITEMIDLIST pidl;
    LPMALLOC     pShellMalloc;
    char         ProfileDir[MAX_PATH];

    if (SUCCEEDED(SHGetMalloc(&pShellMalloc)))
      if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidl)))
      {
        if (SHGetPathFromIDList(pidl, ProfileDir))
        ProfilesPath=ProfileDir;
        pShellMalloc->Free(pidl);
      }

    pShellMalloc->Release();

    ProfilesPath=ProfilesPath+"\\Wapster\\AQQ Folder\\Profiles\\";

    ProgressBar2->Position=0;
    ProgressBar2->Visible=true;
    Hwnd=NULL;
    GetProgressTimer->Enabled=true;

    //Oczekiwanie na zakonczenie procesu
    ExecuteApplication((ExtractFilePath(Application->ExeName)+"7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(BackupsFileListBox->FileName,'"')), this);

    ProgressBar2->Visible=false;
    GetProgressTimer->Enabled=false;
  }

  RestoreLabel->Caption="Zakoñczono! BackupAQQ pomyœlnie przywróci³ kopiê profilu AQQ z pliku " + AnsiQuotedStr(ExtractFileName(BackupsFileListBox->FileName),'"') + ".";

  PreviousButton->Enabled=true;
  ExitButton->Enabled=true;

  RestoreIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ManualRestoreIdThreadComponentRun(
      TIdCustomThreadComponent *Sender)
{
  bool ProfilesInAQQExePath=0;

  TRegistry *Reg = new TRegistry();
  Reg->RootKey=HKEY_CURRENT_USER;
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
    Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
    ProfilesPath=Reg->ReadString("Path");
    ProfilesPath.SetLength(ProfilesPath.Length()-7);
    ProfilesPath=ProfilesPath+"Profiles\\";
    //profile w g³ównym folderze AQQ
    if(DirectoryExists(ProfilesPath))
    {
      ProgressBar2->Position=0;
      ProgressBar2->Visible=true;
      Hwnd=NULL;
      GetProgressTimer->Enabled=true;

      //Oczekiwanie na zakonczenie procesu
      ExecuteApplication((ExtractFilePath(Application->ExeName)+"7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(OpenDialog->FileName,'"')), this);
      ProfilesInAQQExePath=1;

      ProgressBar2->Visible=false;
      GetProgressTimer->Enabled=false;
    }
  }
  Reg->CloseKey();
  delete Reg;

  if(ProfilesInAQQExePath==0)
  {
    //funkcja dostepu do profilu uzytkownika w Windows'ie
    LPITEMIDLIST pidl;
    LPMALLOC     pShellMalloc;
    char         ProfileDir[MAX_PATH];

    if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
     if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidl)))
     {
       if(SHGetPathFromIDList(pidl, ProfileDir))
         ProfilesPath=ProfileDir;
         pShellMalloc->Free(pidl);
     }

    pShellMalloc->Release();

    ProfilesPath=ProfilesPath+"\\Wapster\\AQQ Folder\\Profiles\\";

    ProgressBar2->Position=0;
    ProgressBar2->Visible=true;
    Hwnd=NULL;
    GetProgressTimer->Enabled=true;

    //Oczekiwanie na zakonczenie procesu
    ExecuteApplication((ExtractFilePath(Application->ExeName)+"7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(OpenDialog->FileName,'"')), this);

    ProgressBar2->Visible=false;
    GetProgressTimer->Enabled=false;
  }  
  RestoreLabel->Caption="Zakoñczono! BackupAQQ pomyœlnie przywróci³ kopiê profilu AQQ z pliku " + AnsiQuotedStr(ExtractFileName(BackupsFileListBox->FileName),'"') + ".";

  PreviousButton->Enabled=true;
  ExitButton->Enabled=true;

  ManualRestoreIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

//Pobieranie sciezki procesu po PID
AnsiString GetPathOfProces(DWORD PID)
{
  HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,PID);
  MODULEENTRY32 me32;
  Module32First(hSnapshot,&me32);

  CloseHandle(hSnapshot);

  return me32.szExePath;
}
//---------------------------------------------------------------------------

//Szukanie okna 7-Zip
bool CALLBACK Find7Zip(HWND hWnd)
{
  GetClassName(hWnd, this_title, sizeof(this_title));
  GetWindowThreadProcessId(hWnd, &procesID);

  Progress = this_title;

  ExeName = GetPathOfProces(procesID);

  if(ExtractFileName(ExeName)=="7z.exe")
  {
    if(Progress=="#32770")
     Hwnd = hWnd;
  }
  
  return true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::GetProgressTimerTimer(TObject *Sender)
{
  if(Hwnd==NULL)
   EnumWindows((WNDENUMPROC)Find7Zip,0);

  if(Hwnd!=NULL)
  {
    GetWindowText(Hwnd,this_title,sizeof(this_title));

    Progress = this_title;

    if(AnsiPos("% ",Progress)>0)
    {
      Progress.Delete(AnsiPos("% ",Progress),Progress.Length());
      ProgressBar->Position=StrToInt(Progress);
      ProgressBar2->Position=StrToInt(Progress);
    }
  }
}
//---------------------------------------------------------------------------
