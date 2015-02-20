//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "BackupAQQFrm.h"
#include <ShlObj.h>
#include <Shobjidl.h>
#include <Registry.hpp>
#include <tlhelp32.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
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
#pragma link "LMDCustomComponent"
#pragma link "LMDWndProcComponent"
#pragma link "LMDTrayIcon"
#pragma link "ZipForge"
#pragma resource "*.dfm"
TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
//Sciezka profili
UnicodeString ProfilesPath;
//Komendy przekazane wraz z uruchomieniem
UnicodeString CommandLine[3];
//Progress na pasku w Win7+
ITaskbarList3* m_pTaskBarlist;
bool Win7Support;

__fastcall TBackupAQQForm::TBackupAQQForm(TComponent* Owner)
	: TForm(Owner)
{
  CommandLine[0] = ParamStr(1);
  CommandLine[1] = ParamStr(2);
  CommandLine[2] = ParamStr(3);
}
//---------------------------------------------------------------------------

bool CheckWin7Support()
{
  OSVERSIONINFO osvi;
  BOOL bIsWindowsXPorLater;

  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&osvi);

  if((osvi.dwMajorVersion>=6)&&(osvi.dwMinorVersion>=1))
   return true;
  else return false;
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

//Funkcja szukaj¹ca profile
void __fastcall TBackupAQQForm::FindDir(TListBox *lista, UnicodeString Dir)
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
  bool ProfilesInAQQExePath = false;

  TRegistry *Reg = new TRegistry();
  Reg->RootKey = HKEY_CURRENT_USER;
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
    Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
	ProfilesPath = Reg->ReadString("Path");
    ProfilesPath.SetLength(ProfilesPath.Length()-7);
	ProfilesPath = ProfilesPath + "Profiles\\";
	//Profile w g³ównym folderze AQQ
    if(DirectoryExists(ProfilesPath))
	{
	  FindDir(ProfilesListBox,ProfilesPath);
	  ProfilesInAQQExePath = true;
    }
  }
  Reg->CloseKey();
  delete Reg;

  if(!ProfilesInAQQExePath)
  {
    //Funkcja dostepu do profilu uzytkownika w Windows'ie
    LPITEMIDLIST pidl;
	LPMALLOC pShellMalloc;
	char ProfileDir[MAX_PATH];

	if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
	 if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidl)))
	 {
	   if(SHGetPathFromIDList(pidl, ProfileDir))
		ProfilesPath=ProfileDir;
	   pShellMalloc->Free(pidl);
	 }
	pShellMalloc->Release();

	ProfilesPath=ProfilesPath+"\\Wapster\\AQQ Folder\\Profiles\\";
	//Wywolanie funkcji szukajacej profile
	FindDir(ProfilesListBox,ProfilesPath);
  }
  //Zaznaczanie pierwszego profilu na liscie
  if(ProfilesListBox->Items->Count>0)
  {
	ProfilesListBox->ItemIndex = 0;
	NextButton->Enabled = true;
  }
  else
   NextButton->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::FormShow(TObject *Sender)
{
  //Ustalanie wersji Windows
  Win7Support = CheckWin7Support();
  //Pobieranie listy profili
  aGetProfilesList->Execute();
  //Ustawianie domyslnego folder z plikami backup
  BrowseEdit->Path=ExtractFilePath(Application->ExeName) + "\Backups";
  //Jezeli uruchomiono program z parametrami
  if(CommandLine[0]=="-backup")
  {
	if(!CommandLine[1].IsEmpty())
	{
	  aCommandDoBackup->Execute();
	}
	else
	 Close();
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ExitButtonClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aDoBackupExecute(TObject *Sender)
{
  BackupLabel->Caption="Proszê czekaæ, trwa tworzenie kopii zapasowej Twojego profilu AQQ. Mo¿e to potrwaæ nawet kilka minut.";

  MainTabControl->Visible = false;
  BackupTabControl->Visible = true;

  PreviousButton->Enabled = false;
  NextButton->Enabled = false;
  ExitButton->Enabled = false;

  BackupIdThreadComponent->Start();
  if(Win7Support) Win7ProgressTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::NextButtonClick(TObject *Sender)
{
  if(MainTabControl->Visible)
  {
	if(CreateBackupRadioButton->Checked)
	{
	  if(IsThereAQQ())
	  {
		int Response = MessageBox(Application->Handle,
		 "Zaleca siê wy³¹czenie komunikatora AQQ przed rozpoczêciem procesu tworzenia kopii zapasowej profilu!\n"
		 "Czy mimo to kontynuowaæ?",
		 "Niewy³¹czony komunikator AQQ!",
		 MB_YESNO | MB_ICONEXCLAMATION);
		if(Response==6)
		 aDoBackup->Execute();
	  }
	  else
	   aDoBackup->Execute();
	}
	if(RestoreProfileRadioButton->Checked)
	{
	  MainTabControl->Visible = false;
	  BackupsListTabControl->Visible=  true;
	  PreviousButton->Enabled = true;

	  try
	  {
		BackupsFileListBox->Directory=ExtractFileDir(Application->ExeName)+"\\Backups";
		BackupsFileListBox->Update();
	  }
	  catch(...) {}

      if(BackupsFileListBox->Count!=0)
	  {
		BackupsFileListBox->ItemIndex = 0;
		NextButton->Enabled = true;
      }
      else
	   NextButton->Enabled = false;
	}
  }
  else if(BackupsListTabControl->Visible)
  {
	if(IsThereAQQ())
	{
	  int Response = MessageBox(Application->Handle,
	   "Zaleca siê wy³¹czenie komunikatora AQQ przed rozpoczêciem procesu przywracania kopii zapasowej profilu!\n"
	   "Czy mimo to kontynuowaæ?",
	   "Niewy³¹czony komunikator AQQ!",
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
  if(BackupsListTabControl->Visible)
  {
	BackupsListTabControl->Visible = false;
	MainTabControl->Visible = true;
	PreviousButton->Enabled = false;
	NextButton->Enabled = true;

	aGetProfilesList->Execute();
  }
  else if(RestoreTabControl->Visible)
  {
	RestoreTabControl->Visible = false;
	MainTabControl->Visible = true;
	PreviousButton->Enabled = false;
	NextButton->Enabled = true;

    aGetProfilesList->Execute();
  }
  else if(BackupTabControl->Visible)
  {
	BackupTabControl->Visible = false;
	MainTabControl->Visible = true;
	PreviousButton->Enabled = false;
    NextButton->Enabled = true;

	aGetProfilesList->Execute();
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aRestoreBackupExecute(TObject *Sender)
{
  RestoreLabel->Caption = "Proszê czekaæ, trwa przywracanie Twojego profilu AQQ. Mo¿e to potrwaæ nawet kilka minut.";

  BackupsListTabControl->Visible = false;
  RestoreTabControl->Visible = true;

  PreviousButton->Enabled = false;
  NextButton->Enabled = false;
  ExitButton->Enabled = false;

  RestoreIdThreadComponent->Start();
  if(Win7Support) Win7ProgressTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::BrowseButtonClick(TObject *Sender)
{
  OpenDialog->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::OpenDialogCanClose(TObject *Sender, bool &CanClose)
{
  if(!OpenDialog->FileName.IsEmpty())
  {
	if(IsThereAQQ())
	{
	  int Response = MessageBox(Application->Handle,
	   "Zaleca siê wy³¹czenie komunikatora AQQ przed rozpoczêciem procesu przywracania kopii zapasowej profilu!\n"
	   "Czy mimo to kontynuowaæ?",
	   "Niewy³¹czony komunikator AQQ!",
	   MB_YESNO | MB_ICONEXCLAMATION);
	  if(Response==6)
	  {
		RestoreLabel->Caption="Proszê czekaæ, trwa przywracanie Twojego profilu AQQ. Mo¿e to potrwaæ nawet kilka minut.";

		RestoreTabControl->Visible = true;
		BackupsListTabControl->Visible = false;

		PreviousButton->Enabled = false;
		NextButton->Enabled = false;
		ExitButton->Enabled = false;

		ManualRestoreIdThreadComponent->Start();
	  }
	}
	else
	{
	  RestoreLabel->Caption="Proszê czekaæ, trwa przywracanie Twojego profilu AQQ. Mo¿e to potrwaæ nawet kilka minut.";

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
  //Tworzenie domyslnego folderu z plikami backup
  if(BrowseEdit->Path==ExtractFilePath(Application->ExeName) + "\Backups")
   if(!DirectoryExists(ExtractFilePath(Application->ExeName) + "\\Backups"))
	CreateDir(ExtractFilePath(Application->ExeName) + "\\\\Backups");
  //Definiowanie nazwy pliku backup
  TDateTime Todey = TDateTime::CurrentDate();
  UnicodeString BackupName = BrowseEdit->Path + "\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "_" + Todey + ".aqqbackup";
  //Jezeli plik juz istnieje
  if(FileExists(BackupName))
  {
	int Count = 1;
	while(FileExists(BackupName))
	{
	  Count++;
	  BackupName = BrowseEdit->Path + "\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "_" + Todey + "_" + IntToStr(Count) + ".aqqbackup";
	}
  }
  //Resetowanie progress
  ProgressBar->Position = 0;
  ProgressBar->Visible = true;
  ProgressLabel->Caption = "";
  //Inicjonowanie tworzenia backupu
  ZipForge->TempDir = ExtractFilePath(Application->ExeName);
  ZipForge->BaseDir = ProfilesPath;
  ZipForge->FileName = BackupName;
  ZipForge->OpenArchive(fmCreate);
  ZipForge->AddFiles("\\" +ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\\*.*");
  ZipForge->CloseArchive();
  //Resetowanie progress
  ProgressBar->Visible = false;
  ProgressLabel->Caption = "";
  //Informacje koncowe i wlaczenie przyciskow
  BackupLabel->Caption = "Proces tworzenia kopii zapasowej przebieg³ prawid³owo. Kopia profilu " + AnsiQuotedStr(ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex], '"') + " zosta³a zapisana do pliku " + AnsiQuotedStr(ExtractFileName(BackupName), '"') + ".";
  PreviousButton->Enabled = true;
  ExitButton->Enabled = true;
  //Wylaczenie watku
  BackupIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::RestoreIdThreadComponentRun(TIdThreadComponent *Sender)
{
  bool ProfilesInAQQExePath = false;

  TRegistry *Reg = new TRegistry();
  Reg->RootKey=HKEY_CURRENT_USER;
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
	Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
	ProfilesPath = Reg->ReadString("Path");
	ProfilesPath.SetLength(ProfilesPath.Length()-7);
	ProfilesPath = ProfilesPath + "Profiles\\";
	//Profile w g³ównym folderze AQQ
	if(DirectoryExists(ProfilesPath))
	 ProfilesInAQQExePath = true;
  }
  Reg->CloseKey();
  delete Reg;

  //Resetowanie progress
  ProgressBar2->Position = 0;
  ProgressBar2->Visible = true;
  ProgressLabel2->Caption = "";
  //Przywracanie profilu wedlug umiejscowienia
  if(ProfilesInAQQExePath)
  {
    //Jezeli zadnych profili nie ma jeszcze
	if(ProfilesListBox->Items->Count==0)
	{
	  ProfilesListBox->Enabled = true;
	  ProfilesListBox->Items->Add("Temp");
	  ProfilesListBox->ItemIndex = 0;
	}
	ZipForge->FileName = BackupsFileListBox->FileName;
	ZipForge->OpenArchive(fmOpenRead);
	ZipForge->BaseDir = ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles\\";
	ZipForge->ExtractFiles("*.*");
	ZipForge->CloseArchive();
  }
  else
  {
	//Funkcja dostepu do profilu uzytkownika w Windows'ie
	LPITEMIDLIST pidl;
	LPMALLOC pShellMalloc;
	char ProfileDir[MAX_PATH];

	if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
	 if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidl)))
	 {
	   if(SHGetPathFromIDList(pidl, ProfileDir))
		ProfilesPath=ProfileDir;
	   pShellMalloc->Free(pidl);
	 }
	pShellMalloc->Release();

	//Tworzenie struktury katalogow
	if(!DirectoryExists(ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles"))
	{
	  CreateDir(ProfilesPath + "\\Wapster");
	  CreateDir(ProfilesPath + "\\Wapster\\AQQ Folder");
	  CreateDir(ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles");
	}
	//Jezeli zadnych profili nie ma jeszcze
	if(ProfilesListBox->Items->Count==0)
	{
	  ProfilesListBox->Enabled = true;
	  ProfilesListBox->Items->Add("Temp");
	  ProfilesListBox->ItemIndex = 0;
	}
	ZipForge->FileName = BackupsFileListBox->FileName;
	ZipForge->OpenArchive(fmOpenRead);
	ZipForge->BaseDir = ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles\\";
	ZipForge->TempDir = ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles\\";
	ZipForge->ExtractFiles("*.*");
	ZipForge->CloseArchive();
	if(ProfilesListBox->Items->Count==0)
	{
	  RemoveDir(ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles\\Temp");
	}
  }
  //Resetowanie progress
  ProgressBar2->Visible = false;
  ProgressLabel2->Caption = "";
  //Informacje koncowe i wlaczenie przyciskow
  RestoreLabel->Caption = "Zakoñczono! BackupAQQ pomyœlnie przywróci³ kopiê profilu AQQ z pliku " + AnsiQuotedStr(ExtractFileName(BackupsFileListBox->FileName),'"') + ".";
  PreviousButton->Enabled = true;
  ExitButton->Enabled = true;
  //Wylaczenie watku
  RestoreIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ManualRestoreIdThreadComponentRun(TIdThreadComponent *Sender)

{
  bool ProfilesInAQQExePath = false;

  TRegistry *Reg = new TRegistry();
  Reg->RootKey=HKEY_CURRENT_USER;
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
	Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
	ProfilesPath = Reg->ReadString("Path");
	ProfilesPath.SetLength(ProfilesPath.Length()-7);
	ProfilesPath = ProfilesPath + "Profiles\\";
	//Profile w g³ównym folderze AQQ
	if(DirectoryExists(ProfilesPath))
	 ProfilesInAQQExePath = true;
  }
  Reg->CloseKey();
  delete Reg;

  //Resetowanie progress
  ProgressBar2->Position = 0;
  ProgressBar2->Visible = true;
  ProgressLabel2->Caption = "";
  //Przywracanie profilu wedlug umiejscowienia
  if(ProfilesInAQQExePath)
  {
    //Jezeli zadnych profili nie ma jeszcze
	if(ProfilesListBox->Items->Count==0)
	{
	  ProfilesListBox->Enabled = true;
	  ProfilesListBox->Items->Add("Temp");
	  ProfilesListBox->ItemIndex = 0;
	}
	ZipForge->FileName = OpenDialog->FileName;
	ZipForge->OpenArchive(fmOpenRead);
	ZipForge->BaseDir = ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles\\";
	ZipForge->ExtractFiles("*.*");
	ZipForge->CloseArchive();
  }
  else
  {
	//Funkcja dostepu do profilu uzytkownika w Windows'ie
	LPITEMIDLIST pidl;
	LPMALLOC pShellMalloc;
	char ProfileDir[MAX_PATH];

	if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
	 if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidl)))
	 {
	   if(SHGetPathFromIDList(pidl, ProfileDir))
		ProfilesPath=ProfileDir;
	   pShellMalloc->Free(pidl);
	 }
	pShellMalloc->Release();

	//Tworzenie struktury katalogow
	if(!DirectoryExists(ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles"))
	{
	  CreateDir(ProfilesPath + "\\Wapster");
	  CreateDir(ProfilesPath + "\\Wapster\\AQQ Folder");
	  CreateDir(ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles");
	}
	//Jezeli zadnych profili nie ma jeszcze
	if(ProfilesListBox->Items->Count==0)
	{
	  ProfilesListBox->Enabled = true;
	  ProfilesListBox->Items->Add("Temp");
	  ProfilesListBox->ItemIndex = 0;
	}

	ZipForge->FileName = OpenDialog->FileName;
	ZipForge->OpenArchive(fmOpenRead);
	ZipForge->BaseDir = ProfilesPath + "\\Wapster\\AQQ Folder\\Profiles\\";
	ZipForge->ExtractFiles("*.*");
	ZipForge->CloseArchive();
  }
  //Resetowanie progress
  ProgressBar2->Visible = false;
  ProgressLabel2->Caption = "";
  //Informacje koncowe i wlaczenie przyciskow
  RestoreLabel->Caption = "Zakoñczono! BackupAQQ pomyœlnie przywróci³ kopiê profilu AQQ z pliku " + AnsiQuotedStr(ExtractFileName(BackupsFileListBox->FileName),'"') + ".";
  PreviousButton->Enabled = true;
  ExitButton->Enabled = true;
  //Wylaczenie watku
  ManualRestoreIdThreadComponent->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aCommandDoBackupExecute(TObject *Sender)
{
  UnicodeString ProfileName = CommandLine[1];
  ProfileName = ProfileName.Trim();
  UnicodeString BackupPath = CommandLine[2];

  if(DirectoryExists(ProfilesPath+"\\"+ProfileName))
  {
	TrayIcon->Active = true;
	//Definiowanie nazwy pliku backup
	TDateTime Todey = TDateTime::CurrentDate();
	UnicodeString BackupName;
	if(BackupPath.IsEmpty())
	 BackupName = BrowseEdit->Path + "\\" + ProfileName + "_" + Todey + ".aqqbackup";
	else
	 BackupName = BackupPath + "\\" + ProfileName + "_" + Todey + ".aqqbackup";
	//Jezeli plik juz istnieje
	if(FileExists(BackupName))
	{
	  int Count = 1;
	  while(FileExists(BackupName))
	  {
		Count++;
		BackupName = BrowseEdit->Path + "\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "_" + Todey + "_" + IntToStr(Count) + ".aqqbackup";
	  }
	}
	//Inicjonowanie tworzenia backupu
	ZipForgeCommand->TempDir = BackupPath;
	ZipForgeCommand->BaseDir = ProfilesPath;
	ZipForgeCommand->FileName = BackupName;
	ZipForgeCommand->OpenArchive(fmCreate);
	ZipForgeCommand->AddFiles("\\" + ProfileName + "\\*.*");
	ZipForgeCommand->CloseArchive();
  }
  //Zamykanie programu
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::FormPaint(TObject *Sender)
{
  if(CommandLine[0]=="-backup")
   ShowWindow(Handle, SW_HIDE);
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ZipForgeFileProgress(TObject *Sender, UnicodeString FileName,
		  double Progress, TZFProcessOperation Operation, TZFProgressPhase ProgressPhase,
          bool &Cancel)
{
  if(ProfilesListBox->Items>0)
   FileName = StringReplace(FileName, ProfilesPath+ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex], "...", TReplaceFlags() << rfReplaceAll);
  ProgressLabel->Caption = FileName;
  ProgressLabel2->Caption = "...\\" + FileName;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ZipForgeOverallProgress(TObject *Sender, double Progress,
          TZFProcessOperation Operation, TZFProgressPhase ProgressPhase, bool &Cancel)

{
  ProgressBar->Position = Progress;
  ProgressBar2->Position = Progress;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::CreateBackupRadioButtonClick(TObject *Sender)
{
  ProfilesListLabel->Enabled = true;
  ProfilesListBox->Enabled = true;
  SaveToPathLabel->Enabled = true;
  BrowseEdit->Enabled = true;
  if(ProfilesListBox->Items->Count>0)
   NextButton->Enabled = true;
  else
   NextButton->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::RestoreProfileRadioButtonClick(TObject *Sender)
{
  ProfilesListLabel->Enabled = false;
  ProfilesListBox->Enabled = false;
  SaveToPathLabel->Enabled = false;
  BrowseEdit->Enabled = false;
  NextButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::Win7ProgressTimerTimer(TObject *Sender)
{
  if(Win7Support)
  {
	if(!m_pTaskBarlist)
	{
	  CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL,IID_ITaskbarList3, (void**)&m_pTaskBarlist);
	  m_pTaskBarlist->SetProgressState(Handle, TBPF_NORMAL);
	}
	m_pTaskBarlist->SetProgressValue(Handle, ProgressBar->Position, 100);
	if(ProgressBar->Position==100)
	{
	  m_pTaskBarlist->SetProgressState(Handle, TBPF_NOPROGRESS);
	  Win7ProgressTimer->Enabled = false;
	}
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ZipForgeCommandOverallProgress(TObject *Sender, double Progress,
          TZFProcessOperation Operation, TZFProgressPhase ProgressPhase, bool &Cancel)

{
  int PProgress = Progress;
  UnicodeString ProfileName = CommandLine[1];
  TrayIcon->Hint = "Tworzenie kopii zapasowej profilu " + ProfileName + " - " + IntToStr(PProgress) + "%";
}
//---------------------------------------------------------------------------


