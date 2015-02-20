//---------------------------------------------------------------------------
#ifndef BackupAQQFrmH
#define BackupAQQFrmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ActnList.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <FileCtrl.hpp>
#include "IdBaseComponent.hpp"
#include "IdThreadComponent.hpp"
#include "LMDBaseEdit.hpp"
#include "LMDBrowseEdit.hpp"
#include "LMDControl.hpp"
#include "LMDCustomBevelPanel.hpp"
#include "LMDCustomBrowseEdit.hpp"
#include "LMDCustomControl.hpp"
#include "LMDCustomEdit.hpp"
#include "LMDCustomPanel.hpp"
#include "LMDPNGImage.hpp"
//---------------------------------------------------------------------------
class TBackupAQQForm : public TForm
{
__published:	// IDE-managed Components
	TTabControl *BackupsListTabControl;
	TLabel *BackupsListLabel;
	TImage *Image3;
	TFileListBox *BackupsFileListBox;
	TButton *BrowseButton;
	TTabControl *RestoreTabControl;
	TLabel *RestoreLabel;
	TImage *Image4;
	TProgressBar *ProgressBar2;
	TTabControl *BackupTabControl;
	TLabel *BackupLabel;
	TImage *Image2;
	TProgressBar *ProgressBar;
	TTabControl *MainTabControl;
	TLabel *ProfilesListLabel;
	TLabel *SaveToPathLabel;
	TLabel *ExecuteLabel;
	TImage *Image1;
	TListBox *ProfilesListBox;
	TRadioButton *CreateBackupRadioButton;
	TRadioButton *RestoreProfileRadioButton;
	TLMDBrowseEdit *BrowseEdit;
	TCheckBox *PluginsCheckBox;
	TCheckBox *ThemesCheckBox;
	TCheckBox *SmileysCheckBox;
	TCheckBox *CustomEmotsCheckBox;
	TCheckBox *IncomingCheckBox;
	TCheckBox *TempCheckBox;
	TButton *ExitButton;
	TButton *NextButton;
	TButton *PreviousButton;
	TActionList *ActionList;
	TAction *aGetProfilesList;
	TAction *aDoBackup;
	TAction *aRestoreBackup;
	TOpenDialog *OpenDialog;
	TIdThreadComponent *BackupIdThreadComponent;
	TIdThreadComponent *RestoreIdThreadComponent;
	TIdThreadComponent *ManualRestoreIdThreadComponent;
	TIdThreadComponent *GetProgressIdThreadComponent;
	void __fastcall aGetProfilesListExecute(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ExitButtonClick(TObject *Sender);
	void __fastcall aDoBackupExecute(TObject *Sender);
	void __fastcall NextButtonClick(TObject *Sender);
	void __fastcall PreviousButtonClick(TObject *Sender);
	void __fastcall aRestoreBackupExecute(TObject *Sender);
	void __fastcall BrowseButtonClick(TObject *Sender);
	void __fastcall OpenDialogCanClose(TObject *Sender, bool &CanClose);
	void __fastcall BackupIdThreadComponentRun(TIdThreadComponent *Sender);
	void __fastcall RestoreIdThreadComponentRun(TIdThreadComponent *Sender);
	void __fastcall ManualRestoreIdThreadComponentRun(TIdThreadComponent *Sender);
	void __fastcall GetProgressIdThreadComponentRun(TIdThreadComponent *Sender);
private:	// User declarations
	void __fastcall FindDir(TListBox *lista, AnsiString Dir);
public:		// User declarations
	__fastcall TBackupAQQForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
#endif
