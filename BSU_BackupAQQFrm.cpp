//---------------------------------------------------------------------------
// Copyright (C) 2009-2015 Krzysztof Grochocki
//
// This file is part of BackupAQQ
//
// BackupAQQ is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// BackupAQQ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Radio; see the file COPYING. If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street,
// Boston, MA 02110-1301, USA.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "BackupAQQFrm.h"
#include <ShlObj.h>
#include <Shobjidl.h>
#include <Registry.hpp>
#include <tlhelp32.h>
#include <utilcls.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "acPNG"
#pragma link "acProgressBar"
#pragma link "sBevel"
#pragma link "sButton"
#pragma link "sCustomComboEdit"
#pragma link "sDialogs"
#pragma link "sLabel"
#pragma link "sListBox"
#pragma link "sMaskEdit"
#pragma link "sRadioButton"
#pragma link "sSkinManager"
#pragma link "sTooledit"
#pragma link "ZipForge"
#pragma resource "*.dfm"
TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
typedef TComInterface<ITaskbarList3, &IID_ITaskbarList3> ITaskbarListPtr;
ITaskbarListPtr FTaskbarList;
UnicodeString ProfilesPath; //Sciezka profili
UnicodeString UserProfilePath; //Sciezka profili
UnicodeString CommandLine[3]; //Komendy przekazane wraz z uruchomieniem
//---------------------------------------------------------------------------

__fastcall TBackupAQQForm::TBackupAQQForm(TComponent* Owner)
	: TForm(Owner)
{
  //Pobranie przekazanych parametrow
  CommandLine[0] = ParamStr(1);
  CommandLine[1] = ParamStr(2);
  CommandLine[2] = ParamStr(3);
  //Zostaly przekazane jakies parametry
  if(CommandLine[0]=="-backup")
  {
	//Zminimalizowanie aplikacji na starcie
	WindowState = wsMinimized;
	//Zostala przekazana nazwa profilu
	if(!CommandLine[1].IsEmpty())
	 aCommandBackupProfile->Execute();
	//Brak przekazanej nazwy profilu
	else
	 Application->Terminate();
  }
}
//---------------------------------------------------------------------------

bool AQQProcessExists()
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
		if(((UnicodeString)proces.szExeFile).LowerCase()=="aqq.exe")
		{
		  CloseHandle(Snap);
		  return true;
		}
	  }
	}
	while(Process32Next(Snap , &proces));
  }
  CloseHandle(Snap);

  return false;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::FindDirectories(TListBox* ListBox, UnicodeString Path)
{
  //Zmienna szukania
  TSearchRec searchResult;
  //Wywolanie szukania we wskazanym katalogu
  if(!FindFirst(Path + "*.*", faAnyFile, searchResult))
  {
	//Jezeli cos znaleziono
	do
	{
	  //Znaleziony plik spelnia wymagania - jest folderem
	  if(((searchResult.Attr & faDirectory)>0)&&(searchResult.Name!=".")&&(searchResult.Name!= ".."))
	   //Dodanie elementu do wskazanego komponentu
	   ListBox->Items->Add(searchResult.Name);
	}
	//Szukanie kolejnego katalogu/pliku
	while(!FindNext(searchResult));
	//Zakonczenie szukania
	FindClose(searchResult);
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::DeleteFiles(UnicodeString Path)
{
  //Zmienna szukania
  TSearchRec searchResult;
  //Przeszukanie katalogu
  int Result = FindFirst(Path + "*.*", faAnyFile, searchResult);
  //Gdy w katalogu znajduja sie jakies pliki
  while(!Result)
  {
	//Znaleziony plik spelnia wymagania - nie jest folderem
	if((searchResult.Name!=".")&&(searchResult.Name!="..")&&(!(searchResult.Attr & faDirectory)>0))
	 DeleteFile(Path + searchResult.Name);
	else if((searchResult.Name!=".")&&(searchResult.Name!="..")&&((searchResult.Attr & faDirectory)>0))
	 DeleteFiles(Path + searchResult.Name + "\\");
	//Szukanie kolejnego katalogu/pliku
	Result = FindNext(searchResult);
  }
  //Zakonczenie szukania
  FindClose(searchResult);
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aExitExecute(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aGetProfilesExecute(TObject *Sender)
{
  //Usuwanie wczesniej wczytanej listy
  ProfilesListBox->Clear();
  //Pobieranie folderu zainstalowania AQQ z rejestru
  TRegistry *Reg = new TRegistry();
  Reg->RootKey = HKEY_CURRENT_USER;
  //W rejestrze znajduje sie klucz dodany przez AQQ
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
	//Odczyt klucza ze sciezka AQQ.exe
	Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
	ProfilesPath = Reg->ReadString("Path");
	ProfilesPath = ExtractFilePath(ProfilesPath) + "Profiles\\";
	//Profile znajduja sie w glownym folderze AQQ
	if(DirectoryExists(ProfilesPath))
	 FindDirectories(ProfilesListBox,ProfilesPath);
	//Profile trzymane w profilu uzytkownika Windows
	else
	{
	  //Pobieranie sciezki profilu uzytkownika w Windowsie
	  LPITEMIDLIST pidlItem;
	  LPMALLOC pShellMalloc;
	  wchar_t szPath[MAX_PATH] = L"";
	  if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
	  {
		if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidlItem)))
		{
		  if(SHGetPathFromIDList(pidlItem, szPath)) ProfilesPath = (wchar_t*)szPath;
		  pShellMalloc->Free(pidlItem);
		}
	  }
	  pShellMalloc->Release();
	  UserProfilePath = ProfilesPath;
	  ProfilesPath = ProfilesPath+"\\Wapster\\AQQ Folder\\Profiles\\";
	  //Wywolanie funkcji szukajacej profile
	  FindDirectories(ProfilesListBox,ProfilesPath);
	}
  }
  //Nie odnaleziono klucza w rejestrze
  else
  {
	//Pobieranie sciezki profilu uzytkownika w Windowsie
	  LPITEMIDLIST pidlItem;
	  LPMALLOC pShellMalloc;
	  wchar_t szPath[MAX_PATH] = L"";
	  if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
	  {
		if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidlItem)))
		{
		  if(SHGetPathFromIDList(pidlItem, szPath)) ProfilesPath = (wchar_t*)szPath;
		  pShellMalloc->Free(pidlItem);
		}
	  }
	  pShellMalloc->Release();
	  UserProfilePath = ProfilesPath;
	  ProfilesPath = ProfilesPath+"\\Wapster\\AQQ Folder\\Profiles\\";
	  //Wywolanie funkcji szukajacej profile
	  FindDirectories(ProfilesListBox,ProfilesPath);
  }
  Reg->CloseKey();
  delete Reg;
  //Zaznaczanie pierwszego profilu na liscie
  if(ProfilesListBox->Items->Count)
   ProfilesListBox->ItemIndex = 0;
  else
   RestoreProfileRadioButton->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aBackupProfileExecute(TObject *Sender)
{
  //Ustawienie wlasciwego caption labela
  InfoLabel->Caption = "Proszê czekaæ, trwa tworzenie kopii zapasowej profilu \"" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\". Mo¿e to potrwaæ nawet kilka minut.";
  //Wylaczenie przyciskow
  NextButton->Enabled = false;
  CloseButton->Enabled = false;
  //Zmiana aktywnej planszy
  PageControl->ActivePage = ProgressTabSheet;
  //Tworzenie folderu z plikami backup
  if(!DirectoryExists(sDirectoryEdit->Text)) CreateDir(sDirectoryEdit->Text);
  //Definiowanie nazwy pliku backup
  TDateTime Today = TDateTime::CurrentDate();
  UnicodeString BackupName = sDirectoryEdit->Text + "\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "_" + Today + ".aqqbackup";
  //Wskazany plik juz istnieje
  if(FileExists(BackupName))
  {
	//Ustawienie licznika
	int Count = 1;
	//Dodawanie numeracji do pliku az nie bedzie takiego samego
	while(FileExists(BackupName))
	{
	  Count++;
	  BackupName = sDirectoryEdit->Text + "\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "_" + Today + "_" + IntToStr(Count) + ".aqqbackup";
	}
  }
  //Resetowanie paska postepu
  ProgressBar->Position = 0;
  ProgressBar->Visible = true;
  ProgressLabel->Caption = "";
  ProgressLabel->Visible = true;
  //Ustawienie paska postepu na taskbarze
  if((!FTaskbarList)&&(SUCCEEDED(FTaskbarList.CreateInstance(CLSID_TaskbarList, 0))))
   FTaskbarList->HrInit();
  //Usuwanie tymczasowego folderu w profilu
  DeleteFiles(ProfilesPath + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\\Data\\Temp\\");
  RemoveDirectory((ProfilesPath + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\\Data\\Temp").w_str());
  //Inicjonowanie tworzenia backupu
  ZipForge->BaseDir = ProfilesPath;
  ZipForge->FileName = BackupName;
  ZipForge->OpenArchive(fmCreate);
  ZipForge->AddFiles("\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\\*.*");
  ZipForge->CloseArchive();
  //Resetowanie paska postepu
  ProgressBar->Visible = false;
  ProgressLabel->Visible = false;
  //Ustawienie paska postepu na taskbarze
  if(FTaskbarList) FTaskbarList->SetProgressState(Handle, TBPF_NOPROGRESS);
  //Informacja koncowa
  InfoLabel->Caption = "Proces tworzenia kopii zapasowej przebieg³ prawid³owo. Kopia profilu \"" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\" zosta³a zapisana do pliku \"" + ExtractFileName(BackupName) + "\".";
  //Wlaczenie przyciskow
  PreviousButton->Enabled = true;
  CloseButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aCommandBackupProfileExecute(TObject *Sender)
{
  //Pobieranie listy profili
  aGetProfiles->Execute();
  //Pobranie przekazanych parametrow
  UnicodeString ProfileName = CommandLine[1].Trim();
  UnicodeString BackupPath = CommandLine[2].Trim();
  //Wskazany profil istnieje
  if(DirectoryExists(ProfilesPath + "\\" + ProfileName))
  {
	//Pobieranie sciezki folderu kopii zapasowej z rejestru
	UnicodeString BackupDefPath;
	if(BackupPath.IsEmpty())
	{
	  TRegistry *Reg = new TRegistry();
	  Reg->RootKey = HKEY_CURRENT_USER;
	  //W rejestrze znajduje sie klucz dodany przez AQQ
	  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
	  {
		//Odczyt klucza ze sciezka folderu kopii zapasowej
		Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
		if(!Reg->ReadString("BackupAQQ").IsEmpty()) BackupDefPath = Reg->ReadString("BackupAQQ");
		//Ustawianie domyslnego folder z plikami backup
		else BackupDefPath = ExtractFilePath(Application->ExeName) + "\Backups";
	  }
	  //Ustawianie domyslnego folder z plikami backup
	  else BackupDefPath = ExtractFilePath(Application->ExeName) + "\Backups";
	  Reg->CloseKey();
	  delete Reg;
	  //Tworzenie zdefiniowanego folderu z plikami backup
	  if(!DirectoryExists(BackupDefPath)) CreateDir(BackupDefPath);
	}
	//Ustawianie poczatkowego hinta
	TrayIcon->Hint = "Tworzenie kopii zapasowej profilu \"" + CommandLine[1] + "\" - 0%";
	//Pokazanie ikonki w tray
	TrayIcon->Visible = true;
	//Pokazanie informacji o rozpoczeciu procesu tworzenia backupu
	TrayIcon->BalloonHint = "Tworzenie kopii zapasowej profilu \"" + CommandLine[1] + "\"";
	TrayIcon->BalloonTitle = "BackupAQQ";
	TrayIcon->ShowBalloonHint();
	//Definiowanie nazwy pliku backup
	TDateTime Today = TDateTime::CurrentDate();
	UnicodeString BackupName;
	if(BackupPath.IsEmpty())
	 BackupName = BackupDefPath + "\\" + ProfileName + "_" + Today + ".aqqbackup";
	else
	 BackupName = BackupPath + "\\" + ProfileName + "_" + Today + ".aqqbackup";
	//Jezeli plik juz istnieje
	if(FileExists(BackupName))
	{
	  //Ustawienie licznika
	  int Count = 1;
	  //Dodawanie numeracji do pliku az nie bedzie takiego samego
	  while(FileExists(BackupName))
	  {
		Count++;
		if(BackupPath.IsEmpty())
		 BackupName = BackupDefPath + "\\" + ProfileName + "_" + Today + "_" + IntToStr(Count) + ".aqqbackup";
		else
		 BackupName = BackupPath + "\\" + ProfileName + "_" + Today + "_" + IntToStr(Count) + ".aqqbackup";
	  }
	}
	//Usuwanie tymczasowego folderu w profilu
	DeleteFiles(ProfilesPath + ProfileName + "\\Data\\Temp\\");
	RemoveDirectory((ProfilesPath + ProfileName + "\\Data\\Temp").w_str());
	//Inicjonowanie tworzenia backupu
	ZipForge->BaseDir = ProfilesPath;
	ZipForge->FileName = BackupName;
	ZipForge->OpenArchive(fmCreate);
	ZipForge->AddFiles("\\" + ProfileName + "\\*.*");
	ZipForge->CloseArchive();
  }
  //Zamkniecie aplikacji
  Application->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aRestoreProfile(UnicodeString FileName)
{
  //Ustawienie wlasciwego caption labela
  InfoLabel->Caption = "Proszê czekaæ, trwa przywracanie Twojego profilu AQQ. Mo¿e to potrwaæ nawet kilka minut.";
  //Wylaczenie przyciskow
  PreviousButton->Enabled = false;
  NextButton->Enabled = false;
  CloseButton->Enabled = false;
  //Zmiana aktywnej planszy
  PageControl->ActivePage = ProgressTabSheet;
  //Tworzenie struktury katalogow w profilu uzytkownika Windows
  if(!UserProfilePath.IsEmpty())
  {
	if(!DirectoryExists(UserProfilePath + "\\Wapster"))
	 CreateDir(UserProfilePath + "\\Wapster");
	if(!DirectoryExists(UserProfilePath + "\\Wapster\\AQQ Folder"))
	 CreateDir(UserProfilePath + "\\Wapster\\AQQ Folder");
	if(!DirectoryExists(UserProfilePath + "\\Wapster\\AQQ Folder\\Profiles"))
	 CreateDir(UserProfilePath + "\\Wapster\\AQQ Folder\\Profiles");
  }
  //Resetowanie paska postepu
  ProgressBar->Position = 0;
  ProgressBar->Visible = true;
  ProgressLabel->Caption = "";
  ProgressLabel->Visible = true;
  //Ustawienie paska postepu na taskbarze
  if((!FTaskbarList)&&(SUCCEEDED(FTaskbarList.CreateInstance(CLSID_TaskbarList, 0))))
   FTaskbarList->HrInit();
  //Wypakowywanie kopii zapasowej profilu
  ZipForge->FileName = FileName;
  ZipForge->OpenArchive(fmOpenRead);
  ZipForge->BaseDir = ProfilesPath;
  ZipForge->ExtractFiles("*.*");
  ZipForge->CloseArchive();
  //Resetowanie paska postepu
  ProgressBar->Visible = false;
  ProgressLabel->Visible = false;
  //Ustawienie paska postepu na taskbarze
  if(FTaskbarList) FTaskbarList->SetProgressState(Handle, TBPF_NOPROGRESS);
  //Informacja koncowa
  InfoLabel->Caption = "Zakoñczono! BackupAQQ pomyœlnie przywróci³ kopiê profilu AQQ z pliku \"" + ExtractFileName(FileName) + "\".";
  //Wlaczenie przyciskow
  PreviousButton->Enabled = true;
  CloseButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::FormShow(TObject *Sender)
{
  //Ustawienie domyslnej planszy
  PageControl->ActivePage = WizzardTabSheet;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
  CanClose = CloseButton->Enabled;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::WizzardTabSheetShow(TObject *Sender)
{
  //Pobieranie sciezki folderu kopii zapasowej z rejestru
  TRegistry *Reg = new TRegistry();
  Reg->RootKey = HKEY_CURRENT_USER;
  //W rejestrze znajduje sie klucz dodany przez AQQ
  if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
  {
	//Odczyt klucza ze sciezka folderu kopii zapasowej
	Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
	if(!Reg->ReadString("BackupAQQ").IsEmpty())
	 sDirectoryEdit->Text = Reg->ReadString("BackupAQQ");
	//Ustawianie domyslnego folder z plikami backup
	else sDirectoryEdit->Text = ExtractFilePath(Application->ExeName) + "\Backups";
  }
  //Ustawianie domyslnego folder z plikami backup
  else sDirectoryEdit->Text = ExtractFilePath(Application->ExeName) + "\Backups";
  Reg->CloseKey();
  delete Reg;
  //Pobieranie listy profili
  aGetProfiles->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::NextButtonClick(TObject *Sender)
{
  //Widoczna plansza kreatora
  if(PageControl->ActivePage==WizzardTabSheet)
  {
	//Tworzenie kopii zapasowej profilu
	if((CreateBackupRadioButton->Checked)&&(ProfilesListBox->Items->Count))
	{
	  if(AQQProcessExists())
	  {
		if(Application->MessageBox(
		L"Zaleca siê wy³¹czenie komunikatora AQQ przed rozpoczêciem procesu tworzenia kopii zapasowej profilu!\n\n"
		L"Czy mimo to kontynuowaæ?",
		L"Niewy³¹czony komunikator AQQ!",
		MB_YESNO | MB_ICONEXCLAMATION)==IDYES)
		 aBackupProfile->Execute();
	  }
	  else
	   aBackupProfile->Execute();
	}
	//Przywracanie kopii zapasowej profilu
	else if(RestoreProfileRadioButton->Checked)
	{
	  //Pobranie listy stworzonych kopii zapasowych
	  if(DirectoryExists(sDirectoryEdit->Text))
	  {
		BackupsFileListBox->Directory = sDirectoryEdit->Text;
		BackupsFileListBox->Update();
	  }
	  //Zmiana aktywnej planszy
	  PageControl->ActivePage = BackupsListTabSheet;
	  //Stan przyciskow
	  PreviousButton->Enabled = true;
	  if(BackupsFileListBox->Count)
	  {
		BackupsFileListBox->ItemIndex = 0;
		NextButton->Enabled = true;
	  }
	  else NextButton->Enabled = false;
	}
  }
  //Widoczna plansza przywracania kopii zapasowej profilu
  else if(PageControl->ActivePage==BackupsListTabSheet)
  {
	if(AQQProcessExists())
	{
	  if(Application->MessageBox(
	  L"Zaleca siê wy³¹czenie komunikatora AQQ przed rozpoczêciem procesu przywracania kopii zapasowej profilu!\n\n"
	  L"Czy mimo to kontynuowaæ?",
	  L"Niewy³¹czony komunikator AQQ!",
	  MB_YESNO | MB_ICONEXCLAMATION)==IDYES)
	   aRestoreProfile(BackupsFileListBox->FileName);
	}
	else
	 aRestoreProfile(BackupsFileListBox->FileName);
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::PreviousButtonClick(TObject *Sender)
{
  //Zmiana aktywnej planszy
  PageControl->ActivePage = WizzardTabSheet;
  //Resetowanie stany przyciskow
  PreviousButton->Enabled = false;
  NextButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::BrowseButtonClick(TObject *Sender)
{
  sOpenDialog->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::sOpenDialogCanClose(TObject *Sender, bool &CanClose)
{
  //Wybrano jakis plik kopii zapasowej profilu
  if(!sOpenDialog->FileName.IsEmpty())
  {
	if(AQQProcessExists())
	{
	  if(Application->MessageBox(
	  L"Zaleca siê wy³¹czenie komunikatora AQQ przed rozpoczêciem procesu przywracania kopii zapasowej profilu!\n\n"
	  L"Czy mimo to kontynuowaæ?",
	  L"Niewy³¹czony komunikator AQQ!",
	  MB_YESNO | MB_ICONEXCLAMATION)==IDYES)
	   aRestoreProfile(sOpenDialog->FileName);
	}
	else
	 aRestoreProfile(sOpenDialog->FileName);
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ZipForgeFileProgress(TObject *Sender, UnicodeString FileName,
		  double Progress, TZFProcessOperation Operation, TZFProgressPhase ProgressPhase,
		  bool &Cancel)
{
  //Usuwanie zbednych sciezek
  if(ProfilesListBox->Count) FileName = StringReplace(FileName, ProfilesPath+ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex], "...", TReplaceFlags() << rfReplaceAll);
  else FileName = "...\\" + FileName;
  //Skracanie tekstu z podstepem
  if(FileName.Length()>56)
  {
	FileName.SetLength(53);
	FileName = FileName + "...";
  }
  //Ustawienie tekstu
  ProgressLabel->Caption = FileName;
  //Wywolanie odrysowania aplikacji
  Application->ProcessMessages();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ZipForgeOverallProgress(TObject *Sender, double Progress,
		  TZFProcessOperation Operation, TZFProgressPhase ProgressPhase, bool &Cancel)
{
  //Ustawienie paska postepu
  ProgressBar->Position = Progress;
  //Ustawienie postepu w dymku tray
  if(TrayIcon->Visible) TrayIcon->Hint = "Tworzenie kopii zapasowej profilu \"" + CommandLine[1] + "\" - " + IntToStr((int)Progress) + "%";
  //Ustawienie paska postepu na taskbarze
  if(FTaskbarList) FTaskbarList->SetProgressValue(Handle, Progress, 100);
  //Wywolanie odrysowania aplikacji
  Application->ProcessMessages();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::sDirectoryEditAfterDialog(TObject *Sender, UnicodeString &Name,
		  bool &Action)
{
  //Wybrana sciezka jest inna niz ta domyslna
  if(Name!=ExtractFilePath(Application->ExeName) + "\\Backups")
  {
	//Zapisanie sciezki folderu kopii zapasowej do rejestru
	TRegistry *Reg = new TRegistry();
	Reg->RootKey = HKEY_CURRENT_USER;
	//W rejestrze znajduje sie klucz dodany przez AQQ
	if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
	{
	  //Odczyt klucza gdzie ma byc zapisana sciezka folderu kopii zapasowej
	  Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
	  //Zapisywanie sciezki folderu kopii zapasowej
	  Reg->WriteString("BackupAQQ",Name);
	}
	Reg->CloseKey();
	delete Reg;
  }
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::ProfilesListBoxClick(TObject *Sender)
{
  if(ProfilesListBox->Count) CreateBackupRadioButton->Checked = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::sDirectoryEditChange(TObject *Sender)
{
  //Wybrana sciezka jest inna niz ta domyslna
  if(sDirectoryEdit->Text!=ExtractFilePath(Application->ExeName) + "\\Backups")
  {
	//Zapisanie sciezki folderu kopii zapasowej do rejestru
	TRegistry *Reg = new TRegistry();
	Reg->RootKey = HKEY_CURRENT_USER;
	//W rejestrze znajduje sie klucz dodany przez AQQ
	if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
	{
	  //Odczyt klucza gdzie ma byc zapisana sciezka folderu kopii zapasowej
	  Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
	  //Zapisywanie sciezki folderu kopii zapasowej
	  Reg->WriteString("BackupAQQ",sDirectoryEdit->Text);
	}
	Reg->CloseKey();
	delete Reg;
  }
}
//---------------------------------------------------------------------------
