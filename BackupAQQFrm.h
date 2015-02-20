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
#include "sRadioButton.hpp"
#include "sSkinManager.hpp"
#include "sTooledit.hpp"
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
//---------------------------------------------------------------------------
class TBackupAQQForm : public TForm
{
__published:	// IDE-managed Components
	TsBevel *Bevel;
	TsSkinManager *sSkinManager;
	TPageControl *PageControl;
	TTabSheet *WizzardTabSheet;
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
	TTabSheet *ProgressTabSheet;
	TsLabel *InfoLabel;
	TsProgressBar *ProgressBar;
	TsLabel *ProgressLabel;
	TImage *Image2;
	TTabSheet *BackupsListTabSheet;
	TImage *Image3;
	TsLabel *BackupsListLabel;
	TsButton *BrowseButton;
	TFileListBox *BackupsFileListBox;
	TsOpenDialog *sOpenDialog;
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
	void __fastcall sDirectoryEditAfterDialog(TObject *Sender, UnicodeString &Name,
          bool &Action);
	void __fastcall ProfilesListBoxClick(TObject *Sender);
	void __fastcall sDirectoryEditChange(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
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
