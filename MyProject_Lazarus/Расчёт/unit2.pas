unit Unit2;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, StdCtrls,
  ExtCtrls;

type

  { TForm2 }

  TForm2 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Edit1: TEdit;
    Edit2: TEdit;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    RadioGroup1: TRadioGroup;
    RadioGroup2: TRadioGroup;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Edit1KeyPress(Sender: TObject; var Key: char);
    procedure Edit2KeyPress(Sender: TObject; var Key: char);
  private

  public

  end;

var
  Form2: TForm2;

implementation

{$R *.lfm}

{ TForm2 }


procedure TForm2.Button3Click(Sender: TObject);
begin
  Close;
end;

procedure TForm2.Edit1KeyPress(Sender: TObject; var Key: char);
begin
  Case key of
  #13: Edit2.SetFocus;
  '0'..'9', #8:;
  '.', ',':key:=decimalseparator;
  else key:=chr(0);
  end;
end;

procedure TForm2.Edit2KeyPress(Sender: TObject; var Key: char);
begin
  Case key of
  #13: Button1.SetFocus;
  '0'..'9', #8:;
  '.', ',':key:=decimalseparator;
  else key:=chr(0);
  end;
end;

procedure TForm2.Button1Click(Sender: TObject);
var
  R, R1, R2: Real;
begin
    if (Edit1.Text='') OR (Edit2.Text='') then
    begin
      ShowMessage('Введите данные о сопротивлениях');
      Edit1.SetFocus;
    end;
    R1:=StrToFloat(edit1.Text);
    R2:=StrToFloat(Edit2.Text);
    if RadioGroup1.ItemIndex=0 then R:=R1+R2;
    if RadioGroup1.ItemIndex=1 then R:=R1*R2/(R1+R2);
    if RadioGroup2.ItemIndex=0 then Label3.Caption:= FloatToStrF(R, ffFixed,0,3) + ' Om';
    if RadioGroup2.ItemIndex=1 then Label3.Caption:= FloatToStrF(R, ffFixed,0,3) + ' kOm';
    if RadioGroup2.ItemIndex=2 then Label3.Caption:= FloatToStrF(R, ffFixed,0,3) + ' MOm';

end;

procedure TForm2.Button2Click(Sender: TObject);
begin
  edit1.Text:='';
  edit2.Text:='';
  Label3.Caption:='';
end;

end.

