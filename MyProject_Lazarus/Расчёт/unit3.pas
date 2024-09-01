unit Unit3;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, FileUtil, Forms, Controls, Graphics, Dialogs, StdCtrls,
  ExtCtrls;

type

  { TForm3 }

  TForm3 = class(TForm)
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
  Form3: TForm3;

implementation

{$R *.lfm}

{ TForm3 }

procedure TForm3.Button1Click(Sender: TObject);
begin
  Close;
end;

procedure TForm3.Button2Click(Sender: TObject);
var
  C, C1, C2: Real;
begin
    if (Edit1.Text='') OR (Edit2.Text='') then
    begin
      ShowMessage('Введите данные о сопротивлениях');
      Edit1.SetFocus;
    end;
    C1:=StrToFloat(edit1.Text);
    C2:=StrToFloat(Edit2.Text);
    if RadioGroup1.ItemIndex=0 then C:=C1*C2/(C1+C2);
    if RadioGroup1.ItemIndex=1 then C:=C1+C2;
    if RadioGroup2.ItemIndex=0 then Label3.Caption:= FloatToStrF(C, ffFixed,0,5) + ' F';
    if RadioGroup2.ItemIndex=1 then Label3.Caption:= FloatToStrF(C, ffFixed,0,5) + ' pF';
    if RadioGroup2.ItemIndex=2 then Label3.Caption:= FloatToStrF(C, ffFixed,0,5) + ' nF';
    if RadioGroup2.ItemIndex=3 then Label3.Caption:= FloatToStrF(C, ffFixed,0,5) + ' uF';
    if RadioGroup2.ItemIndex=4 then Label3.Caption:= FloatToStrF(C, ffFixed,0,5) + ' mF';
end;

procedure TForm3.Button3Click(Sender: TObject);
begin
  Edit1.Text:='';
  Edit2.Text:='';
  Label3.Caption:='';
end;

procedure TForm3.Edit1KeyPress(Sender: TObject; var Key: char);
begin
  Case key of
  #13: Edit2.SetFocus;
  '0'..'9', #8:;
  '.', ',':key:=decimalseparator;
  else key:=chr(0);
  end;
end;

procedure TForm3.Edit2KeyPress(Sender: TObject; var Key: char);
begin
  Case key of
  #13: Button2.SetFocus;
  '0'..'9', #8:;
  '.', ',':key:=decimalseparator;
  else key:=chr(0);
  end;
end;


end.

