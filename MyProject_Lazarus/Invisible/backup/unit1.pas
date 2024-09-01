unit Unit1;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ComCtrls, ExtCtrls,
  StdCtrls;

type

  { TForm1 }

  TForm1 = class(TForm)
    Button1: TButton;
    Edit1: TEdit;
    Edit2: TEdit;
    Edit3: TEdit;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    ProgressBar1: TProgressBar;
    Timer1: TTimer;
    TrackBar1: TTrackBar;
    TrackBar2: TTrackBar;
    TrackBar3: TTrackBar;
    TrackBar4: TTrackBar;
    procedure Button1Click(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure Label5Click(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
    procedure TrackBar1Change(Sender: TObject);
    procedure TrackBar2Change(Sender: TObject);
    procedure TrackBar3Change(Sender: TObject);
    procedure TrackBar4Change(Sender: TObject);
  private

  public

  end;

var
  Form1: TForm1;
  R, G, B: Word;

implementation

{$R *.lfm}

{ TForm1 }

procedure TForm1.FormCreate(Sender: TObject);
begin

       AlphaBlend:=True;
       AlphaBlendValue:=255;

end;

procedure TForm1.Label5Click(Sender: TObject);
begin

end;

procedure TForm1.Button1Click(Sender: TObject);
begin
  Timer1.Enabled := true;
end;

procedure TForm1.Timer1Timer(Sender: TObject);
begin
  AlphaBlendValue:= AlphaBlendValue-5;
  if AlphaBlendValue <= 0 then close;
  Label1.Caption:=FloattoStr(AlphaBlendValue);
end;

procedure TForm1.TrackBar1Change(Sender: TObject);
begin
  AlphaBlendValue:=(TrackBar1.Position);
  ProgressBar1.Position:=(TrackBar1.Position);
  Label1.Caption:=FloattoStr(TrackBar1.Position);
  if AlphaBlendValue <= 0 then close;
end;

procedure TForm1.TrackBar2Change(Sender: TObject);
begin
  R:=TrackBar2.Position;
  Edit1.text:=inttostr(R);
  Form1.Color:=RGBToColor(R,G,B);
end;

procedure TForm1.TrackBar3Change(Sender: TObject);
begin
  G:=TrackBar3.Position;
  Edit2.text:=inttostr(G);
  Form1.Color:=RGBToColor(R,G,B);
  Label6.Color:=RGBToColor( 0, G,0 );
end;

procedure TForm1.TrackBar4Change(Sender: TObject);
begin
  B:=TrackBar4.Position;
  Edit3.text:=inttostr(B);
  Form1.Color:=RGBToColor(R,G,B);
end;

end.

