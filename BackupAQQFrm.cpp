//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "BackupAQQFrm.h"
#include <ShlObj.h>
#include <Registry.hpp>
#include <tlhelp32.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
/*#pragma link "IdBaseComponent"
#pragma link "IdThreadComponent"
#pragma link "LMDBaseEdit"
#pragma link "LMDBrowseEdit"
#pragma link "LMDControl"
#pragma link "LMDCustomBevelPanel"
#pragma link "LMDCustomBrowseEdit"
#pragma link "LMDCustomControl"
#pragma link "LMDCustomEdit"
#pragma link "LMDCustomPanel"
#pragma link "LMDPNGImage"*/
#pragma link "IdBaseComponent"
#pragma link "IdThreadComponent"
#pragma link "LMDBaseEdit"
#pragma link "LMDBrowseEdit"
#pragma link "LMDControl"
#pragma link "LMDCustomBevelPanel"
#pragma link "LMDCustomBrowseEdit"
#pragma link "LMDCustomControl"
#pragma link "LMDCustomEdit"
#pragma link "LMDCustomPanel"
#pragma link "LMDPNGImage"
#pragma resource "*.dfm"
TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
UnicodeString ProfilesPath; //do sciezki profilów

wchar_t this_title[2048];  //do nazwy/tekstu okna
UnicodeString Progress; //j.w.
HWND Hwnd; //do uchwytu okna 7-Zip
UnicodeString ExeName;
int Response;
//---------------------------------------------------------------------------

__fastcall TBackupAQQForm::TBackupAQQForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

bool IsThereAQQ()
{
  void *Snap;
  PROCESSENTRY32 proces;
  bool IsThereExe = false;

  Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS , 0);
  proces.dwSize = sizeof(PROCESSENTRY32);

  if(Process32First(Snap , &proces))
  {
	do
	{
	  if(proces.szExeFile[ 0 ] != '[')
	  {
		//ExeName=proces.szExeFile;
		if(((UnicodeString)proces.szExeFile).LowerCase()=="aqq.exe")
		 IsThereExe=true;
	  }
	}
	while(Process32Next(Snap , &proces));
  }
  CloseHandle(Snap);

  return IsThereExe;
}
//---------------------------------------------------------------------------

//Pobieranie uchwytu okna przez PID
HWND HwndPID(DWORD dwPID)
{
  HWND Hwnd = GetTopWindow(0);
  HWND hWnd = 0;;
  DWORD pid;

  while(Hwnd)
  {
    GetWindowThreadProcessId(Hwnd, &pid);
    if(pid == dwPID) hWnd = Hwnd;
    Hwnd = GetNextWindow(Hwnd, GW_HWNDNEXT);
  }

  return hWnd;
}
//---------------------------------------------------------------------------

void __fastcall ExecuteApplication(UnicodeString FileName, UnicodeString Param)
{
  FileName = StringReplace(FileName, "\\", "\\\\", TReplaceFlags() << rfReplaceAll);

  SHELLEXECUTEINFO sei;
  memset(&sei, 0, sizeof (sei));
  sei.cbSize = sizeof(sei);
  sei.fMask = SEE_MASK_NOCLOSEPROCESS;
  sei.hwnd = Application->Handle;
  sei.lpVerb = "open";
  sei.lpFile = FileName.t_str();
  sei.lpParameters = Param.t_str();
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
	do
	{
	  if(((sr.Attr & faDirectory) > 0) & (sr.Name != ".") & (sr.Name != ".."))
	   lista->Items->Add(sr.Name);
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
      if(IsThereAQQ()==true)
	  {
		Response = MessageBox(Application->Handle,
		 "Zaleca siê wy³¹czenie AQQ przed rozpoczêciem procesu tworzenia kopii zapasowej profilu!\n"
		 "Czy mimo to kontynuowaæ?",
		 "Niewy³¹czone AQQ!",
		 MB_YESNO | MB_ICONEXCLAMATION);
		if(Response==6)
		 aDoBackup->Execute();
	  }
	  else
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
	if(IsThereAQQ()==true)
	{
	  Response = MessageBox(Application->Handle,
	   "Zaleca siê wy³¹czenie AQQ przed rozpoczêciem procesu przywracania kopii zapasowej profilu!\n"
	   "Czy mimo to kontynuowaæ?",
	   "Niewy³¹czone AQQ!",
	   MB_YESNO | MB_ICONEXCLAMATION);
	  if(Response==6)
	   aRestoreBackup->Execute();
	}
	else
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

void __fastcall TBackupAQQForm::OpenDialogCanClose(TObject *Sender, bool &CanClose)
{
  if(OpenDialog->FileName!=NULL)
  {
    if(IsThereAQQ()==true)
	{
	  Response = MessageBox(Application->Handle,
	   "Zaleca siê wy³¹czenie AQQ przed rozpoczêciem procesu przywracania kopii zapasowej profilu!\n"
	   "Czy mimo to kontynuowaæ?",
	   "Niewy³¹czone AQQ!",
	   MB_YESNO | MB_ICONEXCLAMATION);
	  if(Response==6)
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
	else
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
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::BackupIdThreadComponentRun(TIdThreadComponent *Sender)
{
  TDateTime Todey = TDateTime::CurrentDate();
  AnsiString BackupName = BrowseEdit->Path + "\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "_" + Todey +".aqqbackup";

  ProgressBar->Position=0;
  ProgressBar->Visible=true;
  Hwnd=NULL;
  GetProgressIdThreadComponent->Start();

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

  //Usuwanie starej kopii z tego samego dnia
  if(FileExists(BackupName))
   DeleteFile(BackupName);

  //Oczekiwanie na zakonczenie procesu
  if(Exclude.Length()>0)
   ExecuteApplication((ExtractFilePath(Application->ExeName)+"\\7-Zip\\7z.exe"),("a " + AnsiQuotedStr(BackupName,'"') + " " + AnsiQuotedStr((ProfilesPath+ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex]),'"')) + Exclude);
  else
   ExecuteApplication((ExtractFilePath(Application->ExeName)+"\\7-Zip\\7z.exe"),("a " + AnsiQuotedStr(BackupName,'"') + " " + AnsiQuotedStr((ProfilesPath+ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex]),'"')));

  ProgressBar->Visible=false;
  GetProgressIdThreadComponent->Stop();

  BackupLabel->Caption="Zakoñczono! BackupAQQ zrobi³ kopiê profilu " + AnsiQuotedStr(ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex], '"') + " do pliku " + AnsiQuotedStr(ExtractFileName(BackupName), '"') + ".";

  PreviousButton->Enabled=true;
  ExitButton->Enabled=true;

  BackupIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::RestoreIdThreadComponentRun(TIdThreadComponent *Sender)
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
	  GetProgressIdThreadComponent->Start();

	  //Oczekiwanie na zakonczenie procesu
	  ExecuteApplication((ExtractFilePath(Application->ExeName)+"\\7-Zip\\7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(BackupsFileListBox->FileName,'"')));
	  ProfilesInAQQExePath=1;

	  ProgressBar2->Visible=false;
	  GetProgressIdThreadComponent->Stop();
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
	GetProgressIdThreadComponent->Start();

	//Oczekiwanie na zakonczenie procesu
	ExecuteApplication((ExtractFilePath(Application->ExeName)+"\\7-Zip\\7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(BackupsFileListBox->FileName,'"')));

	ProgressBar2->Visible=false;
	GetProgressIdThreadComponent->Stop();
  }

  RestoreLabel->Caption="Zakoñczono! BackupAQQ pomyœlnie przywróci³ kopiê profilu AQQ z pliku " + AnsiQuotedStr(ExtractFileName(BackupsFileListBox->FileName),'"') + ".";

  PreviousButton->Enabled=true;
  ExitButton->Enabled=true;

  RestoreIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ManualRestoreIdThreadComponentRun(TIdThreadComponent *Sender)

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
	  GetProgressIdThreadComponent->Start();

	  //Oczekiwanie na zakonczenie procesu
	  ExecuteApplication((ExtractFilePath(Application->ExeName)+"\\7-Zip\\7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(OpenDialog->FileName,'"')));
	  ProfilesInAQQExePath=1;

	  ProgressBar2->Visible=false;
	  GetProgressIdThreadComponent->Stop();
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
	GetProgressIdThreadComponent->Start();

	//Oczekiwanie na zakonczenie procesu
	ExecuteApplication((ExtractFilePath(Application->ExeName)+"\\7-Zip\\7z.exe"),("x -y -o" + AnsiQuotedStr(ProfilesPath,'"') + " " + AnsiQuotedStr(OpenDialog->FileName,'"')));

	ProgressBar2->Visible=false;
	GetProgressIdThreadComponent->Stop();
  }
  RestoreLabel->Caption="Zakoñczono! BackupAQQ pomyœlnie przywróci³ kopiê profilu AQQ z pliku " + AnsiQuotedStr(ExtractFileName(BackupsFileListBox->FileName),'"') + ".";

  PreviousButton->Enabled=true;
  ExitButton->Enabled=true;

  ManualRestoreIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::GetProgressIdThreadComponentRun(TIdThreadComponent *Sender)

{
  if(Hwnd==NULL)
  {
	void *Snap;
    PROCESSENTRY32 proces;

	Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS , 0);
	proces.dwSize = sizeof(PROCESSENTRY32);

	if(Process32First(Snap , &proces))
	{
	  do
	  {
		if(proces.szExeFile[ 0 ] != '[')
		{
		  ExeName=proces.szExeFile;
		  if(ExeName=="7z.exe")
		   Hwnd = HwndPID(proces.th32ProcessID);
		}
	  }
	  while(Process32Next(Snap , &proces));
	}
	CloseHandle(Snap);
  }

  if(Hwnd!=NULL)
  {
	GetWindowTextW(Hwnd,this_title,sizeof(this_title));

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

