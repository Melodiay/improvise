unit Unit1;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, Types;

type

  { TForm1 }

  TForm1 = class(TForm)
    Timer1: TTimer;
    procedure FormClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormKeyPress(Sender: TObject; var Key: char);
    procedure Timer1Timer(Sender: TObject);
  private

  public

  end;

var
  Form1: TForm1;

implementation

{$R *.lfm}

{ TForm1 }

procedure TForm1.FormClick(Sender: TObject);
  Var
   r,g,b:integer;


begin
    r:=random(255);
    g:=random(255);
    b:=random(255);
    Form1.color:=rgbtoColor(r,g,b);
    Timer1.Enabled:=true;
end;


procedure TForm1.FormCreate(Sender: TObject);
begin

end;

procedure TForm1.FormKeyPress(Sender: TObject; var Key: char);
begin
    Timer1.Enabled:=true;
    if Key = #32 then Timer1.Tag := Timer1.Tag + 1;
    if Key = #27 then Close;
    if Key = #13 then Close;
end;



procedure TForm1.Timer1Timer(Sender: TObject);
begin
     if Timer1.Tag < 35
     then Timer1.Tag := Timer1.Tag + 1
     else Close;

end;

end.

