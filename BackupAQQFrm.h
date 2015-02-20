//---------------------------------------------------------------------------

#ifndef BackupAQQFrmH
#define BackupAQQFrmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "XPMan.hpp"
#include <ComCtrls.hpp>
#include <TabNotBk.hpp>
#include <ExtCtrls.hpp>
#include <ActnList.hpp>
#include <FileCtrl.hpp>
#include "LMDBaseEdit.hpp"
#include "LMDBrowseEdit.hpp"
#include "LMDControl.hpp"
#include "LMDCustomBevelPanel.hpp"
#include "LMDCustomBrowseEdit.hpp"
#include "LMDCustomControl.hpp"
#include "LMDCustomEdit.hpp"
#include "LMDCustomPanel.hpp"
#include "LMDPNGImage.hpp"
#include <Dialogs.hpp>
#include "IdThreadComponent.hpp"
#include <IdBaseComponent.hpp>
//---------------------------------------------------------------------------
class TBackupAQQForm : public TForm
{
__published:	// IDE-managed Components
        TXPMan *XPMan;
        TTabControl *MainTabControl;
        TTabControl *BackupTabControl;
        TListBox *ProfilesListBox;
        TRadioButton *CreateBackupRadioButton;
        TRadioButton *RestoreProfileRadioButton;
        TButton *ExitButton;
        TButton *NextButton;
        TButton *PreviousButton;
        TActionList *ActionList;
        TAction *aGetProfilesList;
        TAction *aDoBackup;
        TLabel *ProfilesListLabel;
        TLabel *MainInfoLabel;
        TLabel *BackupLabel;
        TTabControl *BackupsListTabControl;
        TLabel *BackupsListLabel;
        TFileListBox *BackupsFileListBox;
        TAction *aRestoreBackup;
        TTabControl *RestoreTabControl;
        TLabel *RestoreLabel;
        TLabel *SaveToPathLabel;
        TLMDBrowseEdit *BrowseEdit;
        TButton *BrowseButton;
        TImage *Image1;
        TImage *Image2;
        TImage *Image4;
        TImage *Image3;
        TOpenDialog *OpenDialog;
        TIdThreadComponent *BackupIdThreadComponent;
        TIdThreadComponent *RestoreIdThreadComponent;
        TIdThreadComponent *ManualRestoreIdThreadComponent;
        TTimer *GetProgressTimer;
        TProgressBar *ProgressBar;
        TProgressBar *ProgressBar2;
        TCheckBox *PluginsCheckBox;
        TCheckBox *ThemesCheckBox;
        TCheckBox *SmileysCheckBox;
        TCheckBox *CustomEmotsCheckBox;
        TCheckBox *IncomingCheckBox;
        TCheckBox *TempCheckBox;
        TLabel *Label1;
        void __fastcall aGetProfilesListExecute(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall ExitButtonClick(TObject *Sender);
        void __fastcall aDoBackupExecute(TObject *Sender);
        void __fastcall NextButtonClick(TObject *Sender);
        void __fastcall PreviousButtonClick(TObject *Sender);
        void __fastcall aRestoreBackupExecute(TObject *Sender);
        void __fastcall BrowseButtonClick(TObject *Sender);
        void __fastcall OpenDialogCanClose(TObject *Sender,
          bool &CanClose);
        void __fastcall BackupIdThreadComponentRun(
          TIdCustomThreadComponent *Sender);
        void __fastcall RestoreIdThreadComponentRun(
          TIdCustomThreadComponent *Sender);
        void __fastcall ManualRestoreIdThreadComponentRun(
          TIdCustomThreadComponent *Sender);
        void __fastcall GetProgressTimerTimer(TObject *Sender);
private:	// User declarations
        void __fastcall FindDir(TListBox *lista, AnsiString Dir);
public:		// User declarations
        __fastcall TBackupAQQForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TBackupAQQForm *BackupAQQForm;
//---------------------------------------------------------------------------
#endif
