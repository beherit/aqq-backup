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
#pragma link "sPageControl"
#pragma link "sRadioButton"
#pragma link "sSkinManager"
#pragma link "sToolEdit"
#pragma link "ZipForge"
#pragma resource "*.dfm"
TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
UnicodeString ProfilesPath; //Sciezka profili AQQ
UnicodeString WindowsLocalAppDataPath; //Sciezka danych lokalnych aplikacji w profilu Windows
UnicodeString CommandLine[3]; //Komendy przekazane wraz z uruchomieniem aplikacji
//---------------------------------------------------------------------------

__fastcall TBackupAQQForm::TBackupAQQForm(TComponent* Owner)
	: TForm(Owner)
{
	//Pobranie przekazanych parametrow
	CommandLine[0] = ParamStr(1);
	CommandLine[1] = ParamStr(2);
	CommandLine[2] = ParamStr(3);
	//Zostal przekazy parametr wykonania kopii zapasowej
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

//Szukanie uruchomionego procesu AQQ.exe
bool AQQProcessExists()
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS , 0);
	PROCESSENTRY32 proces;
	proces.dwSize = sizeof(PROCESSENTRY32);
	if(Process32First(hSnapShot , &proces))
	{
		do
		{
			if(proces.szExeFile[0]!='[')
			{
				if(((UnicodeString)proces.szExeFile).LowerCase()=="aqq.exe")
				{
					CloseHandle(hSnapShot);
					return true;
				}
			}
		}
		while(Process32Next(hSnapShot , &proces));
	}
	CloseHandle(hSnapShot);
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
		//Szukanie katalogow/plikow
		do
		{
			//Znaleziony plik spelnia wymagania - jest folderem
			if(((searchResult.Attr & faDirectory)>0)&&(searchResult.Name!=".")&&(searchResult.Name!= ".."))
				//Dodanie elementu do wskazanego komponentu
				ListBox->Items->Add(searchResult.Name);
		}
		while(!FindNext(searchResult));

	}
	//Zakonczenie szukania
	FindClose(searchResult);
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::DeleteFiles(UnicodeString Path)
{
	//Zmienna szukania
	TSearchRec searchResult;
	//Przeszukanie katalogu
	int Result = FindFirst(Path + "*.*", faAnyFile, searchResult);
	//W katalogu znajduja sie jakies pliki
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
	//Pobieranie z rejestru folderu zainstalowania AQQ
	TRegistry *Reg = new TRegistry();
	Reg->RootKey = HKEY_CURRENT_USER;
	//W rejestrze znajduje sie klucz instalacji AQQ
	if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
	{
		//Odczyt klucza ze sciezka AQQ.exe
		Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
		ProfilesPath = Reg->ReadString("Path");
		ProfilesPath = ExtractFilePath(ProfilesPath) + "Profiles\\";
		//Profile znajduja sie w folderze instalacyjnym AQQ
		if(DirectoryExists(ProfilesPath))
			FindDirectories(ProfilesListBox,ProfilesPath);
		//Profile trzymane w profilu uzytkownika Windows
		else goto LOCAL_PROFILES;
	}
	else if(Reg->KeyExists("\\Software\\MyPortal\\AQQ"))
	{
		//Odczyt klucza ze sciezka AQQ.exe
		Reg->OpenKey("\\Software\\MyPortal\\AQQ",true);
		ProfilesPath = Reg->ReadString("Path");
		ProfilesPath = ExtractFilePath(ProfilesPath) + "Profiles\\";
		//Profile znajduja sie w folderze instalacyjnym AQQ
		if(DirectoryExists(ProfilesPath))
			FindDirectories(ProfilesListBox,ProfilesPath);
		//Profile trzymane w profilu uzytkownika Windows
		else goto LOCAL_PROFILES;
	}
	//Nie odnaleziono klucza w rejestrze
	else
	{
		//Skok do szukania lokalnych profili AQQ w profilu uzytkownika Windows
		LOCAL_PROFILES:
		//Zmienne specjalnych sciezek uzytkownika w Windowsie
		UnicodeString LocalAppData, LocalProfile;
		//Pobieranie specjalnych sciezek uzytkownika w Windowsie
		LPITEMIDLIST pidlItem;
		LPMALLOC pShellMalloc;
		wchar_t szPath[MAX_PATH] = L"";
		if(SUCCEEDED(SHGetMalloc(&pShellMalloc)))
		{
			if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidlItem)))
				if(SHGetPathFromIDList(pidlItem, szPath)) LocalAppData = (wchar_t*)szPath;
			if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PROFILE, &pidlItem)))
				if(SHGetPathFromIDList(pidlItem, szPath)) LocalProfile = (wchar_t*)szPath;
		}
		pShellMalloc->Free(pidlItem);
		pShellMalloc->Release();
		//Ustalanie sciezki profilow AQQ
		if(DirectoryExists(LocalProfile+"\\Wapster\\AQQ Folder\\Profiles\\"))
			ProfilesPath = LocalProfile+"\\Wapster\\AQQ Folder\\Profiles\\";
		else
		{
			WindowsLocalAppDataPath = LocalAppData;
			ProfilesPath = LocalAppData+"\\MyPortal\\AQQ Folder\\Profiles\\";
		}
		//Szukanie profilow AQQ
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
	//Zmienna pomocnicza w przypadku bledow
	bool TerminateOperation = false;
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
	//Wlaczenie paska postepu
	ProgressBar->Position = 0;
	ProgressBar->Visible = true;
	ProgressLabel->Caption = "";
	ProgressLabel->Visible = true;
	//Wlaczenie paska postepu na taskbarze
	Taskbar->ProgressValue = 0;
	Taskbar->ProgressState = TTaskBarProgressState::Normal;
	//Usuwanie tymczasowego folderu w profilu
	DeleteFiles(ProfilesPath + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\\Data\\Temp\\");
	RemoveDirectory((ProfilesPath + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\\Data\\Temp").w_str());
	//Inicjonowanie tworzenia backupu
	ZipForge->BaseDir = ProfilesPath;
	ZipForge->FileName = BackupName;
	try { ZipForge->OpenArchive(fmCreate); } 
	catch (...)
	{
		//Odznaczenie wystapienie wyjatku
		TerminateOperation = true;
		//Wylaczenie tworzenia kopii zapasowej
		ZipForge->CloseArchive();
		//Informacja o bledzie
		InfoLabel->Caption = "Wyst¹pi³ b³¹d podczas tworzenia kopii zapasowej profilu \"" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\". Spróbuj wykonaæ kopiê zapasow¹ ponownie lub zmieñ lokalizacjê folderu kopii zapasowych.";
	} 
	if(!TerminateOperation)
	{ 
		ZipForge->AddFiles("\\" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\\*.*");
		ZipForge->CloseArchive();
		//Informacja koncowa
		InfoLabel->Caption = "Proces tworzenia kopii zapasowej przebieg³ prawid³owo. Kopia profilu \"" + ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex] + "\" zosta³a zapisana do pliku \"" + ExtractFileName(BackupName) + "\".";
	}
	//Wylaczenie paska postepu
	ProgressBar->Visible = false;
	ProgressLabel->Visible = false;
	//Wylaczenie paska postepu na taskbarze
	Taskbar->ProgressState = TTaskBarProgressState::None;
	//Wlaczenie przyciskow
	PreviousButton->Enabled = true;
	CloseButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::aCommandBackupProfileExecute(TObject *Sender)
{
	//Zmienna pomocnicza w przypadku bledow
	bool TerminateOperation = false;
	//Pobieranie listy profili
	aGetProfiles->Execute();
	//Pobranie przekazanych parametrow
	UnicodeString ProfileName = CommandLine[1].Trim();
	UnicodeString BackupPath = CommandLine[2].Trim();
	//Wskazany profil istnieje
	if(DirectoryExists(ProfilesPath + "\\" + ProfileName))
	{
		//Pobieranie sciezki folderu kopii zapasowej z rejestru
		if(BackupPath.IsEmpty())
		{
			TRegistry *Reg = new TRegistry();
			Reg->RootKey = HKEY_CURRENT_USER;
			//W rejestrze znajduje sie klucz instalacji AQQ
			if(Reg->KeyExists("\\Software\\MyPortal\\AQQ"))
			{
				//Odczyt klucza ze sciezka folderu kopii zapasowej
				Reg->OpenKey("\\Software\\MyPortal\\AQQ",true);
				if(!Reg->ReadString("BackupAQQ").IsEmpty()) BackupPath = Reg->ReadString("BackupAQQ");
				//Ustawianie domyslnego folder z plikami backup
				if(BackupPath.IsEmpty()) BackupPath = ExtractFilePath(Application->ExeName) + "\Backups";
			}
			else if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
			{
				//Odczyt klucza ze sciezka folderu kopii zapasowej
				Reg->OpenKey("\\Software\\Wapster\\AQQ",true);
				BackupPath = Reg->ReadString("BackupAQQ");
				//Ustawianie domyslnego folder z plikami backup
				if(BackupPath.IsEmpty()) BackupPath = ExtractFilePath(Application->ExeName) + "\Backups";
			}
			//Ustawianie domyslnego folder z plikami backup
			else BackupPath = ExtractFilePath(Application->ExeName) + "\Backups";
			Reg->CloseKey();
			delete Reg;
			//Tworzenie zdefiniowanego folderu z plikami backup
			if(!DirectoryExists(BackupPath)) CreateDir(BackupPath);
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
		UnicodeString BackupName = BackupPath + "\\" + ProfileName + "_" + Today + ".aqqbackup";
		//Jezeli plik juz istnieje
		if(FileExists(BackupName))
		{
			//Ustawienie licznika
			int Count = 1;
			//Dodawanie numeracji do pliku az nie bedzie takiego samego
			while(FileExists(BackupName))
			{
				Count++;
				BackupName = BackupPath + "\\" + ProfileName + "_" + Today + "_" + IntToStr(Count) + ".aqqbackup";
			}
		}
		//Usuwanie tymczasowego folderu w profilu
		DeleteFiles(ProfilesPath + ProfileName + "\\Data\\Temp\\");
		RemoveDirectory((ProfilesPath + ProfileName + "\\Data\\Temp").w_str());
		//Inicjonowanie tworzenia backupu
		ZipForge->BaseDir = ProfilesPath;
		ZipForge->FileName = BackupName;
		try { ZipForge->OpenArchive(fmCreate); } 
		catch (...)
		{
			//Odznaczenie wystapienie wyjatku
			TerminateOperation = true;
			//Wylaczenie tworzenia kopii zapasowej
			ZipForge->CloseArchive();
		}
		if(!TerminateOperation)
		{
			ZipForge->AddFiles("\\" + ProfileName + "\\*.*");
			ZipForge->CloseArchive();
		}
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
	//Tworzenie struktury katalogow profilow AQQ
	if(!DirectoryExists(ProfilesPath))
	{
		if(!DirectoryExists(WindowsLocalAppDataPath + "\\MyPortal"))
			CreateDir(WindowsLocalAppDataPath + "\\MyPortal");
		if(!DirectoryExists(WindowsLocalAppDataPath + "\\MyPortal\\AQQ Folder"))
			CreateDir(WindowsLocalAppDataPath + "\\MyPortal\\AQQ Folder");
		if(!DirectoryExists(WindowsLocalAppDataPath + "\\MyPortal\\AQQ Folder\\Profiles"))
			CreateDir(WindowsLocalAppDataPath + "\\MyPortal\\AQQ Folder\\Profiles");
	}
	//Resetowanie paska postepu
	ProgressBar->Position = 0;
	ProgressBar->Visible = true;
	ProgressLabel->Caption = "";
	ProgressLabel->Visible = true;
	//Wlaczenie paska postepu na taskbarze
	Taskbar->ProgressValue = 0;
	Taskbar->ProgressState = TTaskBarProgressState::Normal;
	//Wypakowywanie kopii zapasowej profilu
	ZipForge->FileName = FileName;
	ZipForge->OpenArchive(fmOpenRead);
	ZipForge->BaseDir = ProfilesPath;
	ZipForge->ExtractFiles("*.*");
	ZipForge->CloseArchive();
	//Wylaczenie paska postepu
	ProgressBar->Visible = false;
	ProgressLabel->Visible = false;
	//Wylaczenie paska postepu na taskbarze
	Taskbar->ProgressState = TTaskBarProgressState::None;
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
	//W rejestrze znajduje sie klucz instalacji AQQ
	if(Reg->KeyExists("\\Software\\MyPortal\\AQQ"))
	{
		//Odczyt klucza ze sciezka folderu kopii zapasowej
		Reg->OpenKey("\\Software\\MyPortal\\AQQ",true);
		if(!Reg->ReadString("BackupAQQ").IsEmpty())
			sDirectoryEdit->Text = Reg->ReadString("BackupAQQ");
		//Ustawianie domyslnego folder z plikami backup
		else sDirectoryEdit->Text = ExtractFilePath(Application->ExeName) + "\Backups";
	}
	else if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
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

void __fastcall TBackupAQQForm::ZipForgeFileProgress(TObject *Sender, UnicodeString FileName, double Progress,
				TZFProcessOperation Operation, TZFProgressPhase ProgressPhase, bool &Cancel)
{
	//Usuwanie zbednych sciezek
	if(ProfilesListBox->Count) FileName = StringReplace(FileName, ProfilesPath+ProfilesListBox->Items->Strings[ProfilesListBox->ItemIndex], "...", TReplaceFlags() << rfReplaceAll);
	else FileName = "...\\" + FileName;
	//Skracanie tekstu z postepem
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
	//Ustawienie paska postepu na taskbarze
	Taskbar->ProgressValue = Progress;
	//Ustawienie postepu w dymku tray
	if(TrayIcon->Visible) TrayIcon->Hint = "Tworzenie kopii zapasowej profilu \"" + CommandLine[1] + "\" - " + IntToStr((int)Progress) + "%";
	//Wywolanie odrysowania aplikacji
	Application->ProcessMessages();
}
//---------------------------------------------------------------------------

void __fastcall TBackupAQQForm::sDirectoryEditAfterDialog(TObject *Sender, UnicodeString &Name, bool &Action)
{
	//Wybrana sciezka jest inna niz ta domyslna
	if(Name!=ExtractFilePath(Application->ExeName) + "\\Backups")
	{
		//Zapisanie sciezki folderu kopii zapasowej do rejestru
		TRegistry *Reg = new TRegistry();
		Reg->RootKey = HKEY_CURRENT_USER;
		//W rejestrze znajduje sie klucz instalacji AQQ
		if(Reg->KeyExists("\\Software\\MyPortal\\AQQ"))
		{
			//Odczyt klucza gdzie ma byc zapisana sciezka folderu kopii zapasowej
			Reg->OpenKey("\\Software\\MyPortal\\AQQ",true);
			//Zapisywanie sciezki folderu kopii zapasowej
			Reg->WriteString("BackupAQQ",Name);
		}
		else if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
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
		//W rejestrze znajduje sie klucz instalacji AQQ
		if(Reg->KeyExists("\\Software\\MyPortal\\AQQ"))
		{
			//Odczyt klucza gdzie ma byc zapisana sciezka folderu kopii zapasowej
			Reg->OpenKey("\\Software\\MyPortal\\AQQ",true);
			//Zapisywanie sciezki folderu kopii zapasowej
			Reg->WriteString("BackupAQQ",sDirectoryEdit->Text);
		}
		else if(Reg->KeyExists("\\Software\\Wapster\\AQQ"))
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