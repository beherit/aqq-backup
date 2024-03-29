//---------------------------------------------------------------------------
// Copyright (C) 2009-2015 Krzysztof Grochocki
//
// This file is part of BackupAQQ
//
// BackupAQQ is free software: you can redistribute it and/or modify
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
// along with GNU Radio. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifndef BackupAQQFrmH
#define BackupAQQFrmH
//---------------------------------------------------------------------------
#include "acPNG.hpp"
#include "acProgressBar.hpp"
#include "sBevel.hpp"
#include "sButton.hpp"
#include "sCustomComboEdit.hpp"
#include "sDialogs.hpp"
#include "sLabel.hpp"
#include "sListBox.hpp"
#include "sMaskEdit.hpp"
#include "sPageControl.hpp"
#include "sRadioButton.hpp"
#include "sSkinManager.hpp"
#include "sToolEdit.hpp"
#include "ZipForge.hpp"
#include <System.Actions.hpp>
#include <System.Classes.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.FileCtrl.hpp>
#include <Vcl.Mask.hpp>
#include <Vcl.StdCtrls.hpp>
#include <System.Win.TaskbarCore.hpp>
#include <Vcl.Taskbar.hpp>
//---------------------------------------------------------------------------
class TBackupAQQForm : public TForm
{
__published:	// IDE-managed Components
	TsSkinManager *sSkinManager;
	TsPageControl *PageControl;
	TsTabSheet *WizzardTabSheet;
	TZipForge *ZipForge;
	TsButton *CloseButton;
	TsButton *NextButton;
	TsButton *PreviousButton;
	TActionList *ActionList;
	TAction *aExit;
	TsListBox *ProfilesListBox;
	TAction *aGetProfiles;
	TsRadioButton *CreateBackupRadioButton;
	TsRadioButton *RestoreProfileRadioButton;
	TsDirectoryEdit *sDirectoryEdit;
	TImage *Image1;
	TTrayIcon *TrayIcon;
	TAction *aBackupProfile;
	TAction *aCommandBackupProfile;
	TsTabSheet *ProgressTabSheet;
	TsLabel *InfoLabel;
	TsProgressBar *ProgressBar;
	TsLabel *ProgressLabel;
	TImage *Image2;
	TsTabSheet *BackupsListTabSheet;
	TImage *Image3;
	TsLabel *BackupsListLabel;
	TsButton *BrowseButton;
	TFileListBox *BackupsFileListBox;
	TsOpenDialog *sOpenDialog;
	TTaskbar *Taskbar;
	TsBevel *Bevel;
	void __fastcall aExitExecute(TObject *Sender);
	void __fastcall aGetProfilesExecute(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall NextButtonClick(TObject *Sender);
	void __fastcall PreviousButtonClick(TObject *Sender);
	void __fastcall aBackupProfileExecute(TObject *Sender);
	void __fastcall aCommandBackupProfileExecute(TObject *Sender);
	void __fastcall ZipForgeFileProgress(TObject *Sender, UnicodeString FileName, double Progress,
					TZFProcessOperation Operation, TZFProgressPhase ProgressPhase, bool &Cancel);
	void __fastcall ZipForgeOverallProgress(TObject *Sender, double Progress, TZFProcessOperation Operation,
					TZFProgressPhase ProgressPhase, bool &Cancel);
	void __fastcall BrowseButtonClick(TObject *Sender);
	void __fastcall sOpenDialogCanClose(TObject *Sender, bool &CanClose);
	void __fastcall WizzardTabSheetShow(TObject *Sender);
	void __fastcall sDirectoryEditAfterDialog(TObject *Sender, UnicodeString &Name, bool &Action);
	void __fastcall ProfilesListBoxClick(TObject *Sender);
	void __fastcall sDirectoryEditChange(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning);

private:	// User declarations
	void __fastcall FindDirectories(TListBox* ListBox, UnicodeString Path);
	void __fastcall DeleteFiles(UnicodeString Path);
	void __fastcall aRestoreProfile(UnicodeString FileName);
public:		// User declarations
	__fastcall TBackupAQQForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
#endif
