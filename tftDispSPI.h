#ifndef tftDispSPI_h
#define tftDispSPI_h

#include <TFT_eSPI.h>

//#define SINGLEBYTEGLYPH_TO_RAM 1

class tftDispSPI {
public:
	tftDispSPI();
	~tftDispSPI();
	
  void init();

	// bool isAvailable() { return mIsAvailable; }
	
	void printw(const char* fmt, ...);
	void vprintw(const char* fmt, va_list arg);
	
	//画面全体の文字を削除し、カーソル位置を最上行の左端に移動します。
	void clear(void);
	
	//カーソルを(y)行(x)列に移動します（初期値x=0;y=0）。設定値は半角基準です。
	//(x)及び(y)を省略した場合は、初期値の位置にカーソルを移動します。
	//移動先が、全角の2バイト目の場合も、列補正を行いません。
	//画面範囲外の設定値の場合は、設定値を0として判断します。
	void move(int r, int c);
	
	void curmove(int r, int c);
	// カーソルを(n)行上に移動します（初期値n=1）。
	// 列の位置は保ちますが、移動先が全角文字の場合は、列位置の補正を行います。
	// 最上行から上には移動できません。
	void curup(void){ curmove(-1,0); };
	void curup(int n){ curmove(-n,0); };
	// カーソルを(n)行下に移動します（初期値n=1）。
	// 列の位置は保ちますが、移動先が全角文字の場合は、列位置の補正を行います。
	// 最下行からは下には移動できません。
	void curdown(void){ curmove(1,0); };
	void curdown(int n){ curmove(n,0); };
	// カーソルを(n)列左に移動します（初期値n=1）。
	// カーソルの移動量は、該当位置の文字サイズに依存します。
	// 文字が設定されていない場合は、半角文字１つ分移動します。
	void curleft(void){ curmove(0,-1); };
	void curleft(int n){ curmove(0,-n); };
	// カーソルを(n)列右に移動します（初期値n=1）。
	// カーソルの移動量は、該当位置の文字サイズに依存します。
	// 文字が設定されていない場合は、半角文字１つ分移動します。
	void curright(void){ curmove(0,1); };
	void curright(int n){ curmove(0,n); };

	//文字属性を変更します。(n)には以下の設定値を入力して下さい。（初期値n=0）
	//ESC[(n);(n);…m
	//ESC[m
	// 設定値 内容
	// 0              初期値が設定されます。初期値は以下の通りです。
	//                文字色：白、文字背景色：透過、下線：無し、太字：無し、
	//                点滅表示：無し、反転：無し
	// 1              太字表示に設定
	// 4              下線付きに設定
	// 5              点滅表示に設定
	// 7              文字色と文字背景色を反転
	// 8 or 16        不可視（文字色と文字背景色が透過となります。）
	// 30             文字色を黒に設定
	// 17 or 31       文字色を赤に設定
	// 18 or 32       文字色を緑に設定
	// 19 or 33       文字色を黄色に設定
	// 20 or 34       文字色を青に設定
	// 21 or 35       文字色を紫に設定
	// 22 or 36       文字色を水色に設定
	// 23 or 37 or 39 文字色を白に設定
	// 40             文字背景色を黒に設定
	// 41             文字背景色を赤に設定
	// 42             文字背景色を緑に設定
	// 43             文字背景色を黄色に設定
	// 44             文字背景色を青に設定
	// 45             文字背景色を紫に設定
	// 46             文字背景色を水色に設定
	// 47             文字背景色を白に設定
	// 49             文字背景色を初期値（透過）に設定
	// 24             下線無しに設定
	// 25             点滅表示無しに設定
	// 27             反転無しに設定
	// 29             太字表示無しに設定
	// 60             文字色を透過に設定
	// 61             文字背景色を透過に設定
	void set_attribute( int n );
	void set_attribute( int n1, int n2 );
	void set_attribute( int n1, int n2, int n3 );
	void set_attribute( int n1, int n2, int n3, int n4 );
	void set_attribute( int n1, int n2, int n3, int n4, int n5 );
	void set_attribute( int n1, int n2, int n3, int n4, int n5, int n6 );
	
	// カーソル位置から最下行の右端までの文字を削除します。
	void del_to_screen_end();
	// 画面先頭からカーソル位置までの文字を削除します。
	void del_from_screen_top();
	//カーソル位置からカーソル位置の行の右端までの文字を削除します。
	void del_to_end();
	//カーソル位置の行の先頭列から、カーソル位置までの文字を削除します。
	void del_from_top();
	// カーソル位置の行の文字を削除し、カーソル位置をカーソル位置の行の先頭列に移動します。
	void del_row();

	// カーソル位置から(n)文字削除します（初期値n=1）。カーソルは移動しません。
	void del(int n);
	
	// カーソル位置及び文字属性を保存します。
	void save_attribute(int n);
	//カーソル位置及び文字属性をロードします。
	//（未設定の場合は初期値がロードされます。）
	void load_attribute(int n);
	
	// カーソル表示をONにします。
	void curon(void);
	// カーソル表示をOFFにします。
	void curoff(void);
	
	// カーソル位置をカーソル位置の行の先頭に移動します。
	void cur_rowtop(void);
	
	// 受信データを削除し、背景を含め画面をクリアします。
	// （文字サイズ、文字コード、ボーレートは変更しません。）
	void init_disp(void);

	// 文字サイズ設定
	// 背景を含め画面をクリアします。受信データも削除します。
	// 0 小（4x8）
	// 1 中（5x10）
	// 2 大（8x16） [初期値]
	void set_charsize(int x);

	// 自動改行設定
	void set_autocr(int x){
		if(x){
			// 画面右端にカーソルがある場合の、自動的な改行動作を有効とします。[初期値]
			//printw( "¥033@20Z" );
		}else{
			// 画面右端にカーソルがある場合の、自動的な改行動作を無効とします。
			// CR、LF、FFによって、改行動作が有効となります。
			// 改行無効中は画面右端にカーソルがある場合に文字は追加されませんが、
			// エスケープシーケンスや制御コードによるカーソルの移動または文字の削除を伴う動作があった後は、
			// カーソル位置への文字の追加が有効となります。画面外にはみ出た文字については、保存されません。
			//printw( "¥033@21Z" );
		}
	};
	
	// 背景表示用バッファ設定
	void set_bgbuff(int x);
	
	// 背景表示用バッファクリア（黒で塗りつぶされます）。
	// x:背景バッファ番号
	void clear_bgbuff(int x);
	
	// 四角形の単色データを背景に表示します。
	// 320×240の表示エリアからはみ出るデータに関しては、無視されます。
	// また、設定値が範囲外の場合は無効となります。
	// X座標は0-319、Y座標は0-239の値を設定して下さい。
	//  bg: 設定バッファ(0-3)
	//   w: 四角形幅
	//   h: 四角形高さ
	//   x: 表示位置X座標
	//   y: 表示位置Y座標
	// r,g,b: カラーデータ(RGB555形式10進)
	void draw_fillrect(int bg, int w, int h, int x, int y, int col );
	
	// 画像データを背景に表示します。
	// 320×240の表示エリアからはみ出るデータに関しては、無視されます。
	// また、設定値が範囲外の場合は無効となります。
	// X座標は0-319、Y座標は0-239の値を設定して下さい。
	//  bg: 設定バッファ(0-3)
	//   w: 画像幅
	//   h :画像高さ
	//   x: 表示位置X座標
	//   y: 表示位置Y座標
	void draw_background(int bg, int w, int h, int x, int y);
	
	// (x0,y0)−(x1,y1)間にラインを表示します。
	// X座標は0-319、Y座標は0-239の値を設定して下さい。
	//  bg: 設定バッファ(0-3)
	//  x0: x0座標
	//  y0: y0座標
	//  x1: x1座標
	//  y1: y1座標
	// r,g,b: カラーデータ(RGB555形式:10進)
	void draw_line(int bg, int x0, int y0, int x1, int y1, int col );
	
	// 楕円を描画します。
	// 320×240の表示エリアからはみ出るデータに関しては、無視されます。
	// また、設定値が範囲外の場合は無効となります。
	// X座標は0-319、Y座標は0-239の値を設定して下さい。
	//   bg: 設定バッファ(0-3)
	// fill: 塗りつぶしフラグ(0:塗りつぶしなし,1:あり)
	//   cx: 中心X座標
	//   cy: 中心Y座標
	//   xw: X軸直径
	//   yh: Y軸直径
	// r,g,b:カラーデータ(RGB555形式:10進)
	void draw_ellipse(int bg, int fill, int cx, int cy, int xw, int yh, int col );
	
	// 四角形を背景に表示します。塗りつぶしは行いません。
	// 320×240の表示エリアからはみ出るデータに関しては、無視されます。
	// また、設定値が範囲外の場合は無効となります。
	// X座標は0-319、Y座標は0-239の値を設定して下さい。
	//  bg: 設定バッファ(0-3)
	//   w: 四角形幅
	//   h: 四角形高さ
	//   x: 表示位置X座標
	//   y: 表示位置Y座標
	// r,g,b:カラーデータ(RGB555形式10進)
	void draw_rectangle(int bg, int w, int h, int x, int y, int col);
	
	// ドットを描画します。
	// bg: 設定バッファ(0-3)
	//  x: 表示位置X座標
	//  y: 表示位置Y座標
	//  r,g,b: カラーデータ(RGB555形式10進)
	// X座標は0-319、Y座標は0-239の値を設定して下さい。
	void draw_dot(int bg, int x, int y, int col);
	
	// microSD カードにある JPEG ファイルを背景バッファに部分的に展開します。
	// Image フォルダ直下の JPEG ファイルが対象となります。
	// 320×240 の表示エリアからはみ出るデータに関しては、無視されます。
	// file: JPEG ファイルのファイル名(拡張子除く)となります。1~255 が有効な数値です。
	//   bg: 展開先バッファとなります。0~3 が有効な数値です。
	//    x: 展開先の x 座標(0-319)となります。
	//    y: 展開先の y 座標(0-239)となります。
	void draw_image(int file, int bg, int x, int y);
	
	// microSD カードにある JPEG ファイルを背景バッファに部分的に展開します。
	// Image フォルダ内の特定フォルダ内の JPEG ファイルが対象となります。
	// 320×240 の表示エリアからはみ出るデータに関しては、無視されます。
	// file: JPEG ファイルのファイル名(拡張子除く)となります。1~255 が有効な数値です。
	//   bg: 展開先バッファとなります。0~3 が有効な数値です。
	//  fol: Image フォルダ直下のフォルダ名となります。
	//		設定した数値のフォルダ名以下の(n1).jpg ファイルを展開します。
	//		1~255 が有効な数値です。
	//    x: 展開先の x 座標(0-319)となります。
	//    y: 展開先の y 座標(0-239)となります。
	void draw_image2(int file, int bg, int fol, int x, int y);
	
  bool  updateContent();

  bool  getTouch(uint16_t *x, uint16_t *y, uint16_t threshold=600);

// protected:
	void puts_(const char* str, uint32_t max_len=0);
	
  void  setCursorPointer(int16_t x, int16_t y);
  void  hideCursorPointer();
  void  touch_calibrate(uint16_t *calData);
  void  set_calibrate(const uint16_t *calData);

private:
	enum FontType {
		kTinyFont = 0,
		kSmallFont,
		kNormalFont
	};
  static const int	VIEW_WIDTH = 320;
  static const int	VIEW_HEIGHT = 240;
	static const int	BG_BUFF_NUM = 4;
	static const int	MAX_LINES = 30;
	static const int	MAX_COLUMNS = 80;
	static const int	STYLE_BOLD = 0x01;
	static const int	STYLE_UNDERLINED = 0x02;
	static const int	STYLE_BLINKED = 0x04;
	static const int	STYLE_INVERTED = 0x08;
  static const int  TFT_EDISP_TRANSPARENT = 15;
  static const int  POINTER_POSY_SIZE = 12;
  static const int  POINTER_NEGY_SIZE = 0;

  TFT_eSPI      mTft;
  TFT_eSprite   mBgSpr;
  TFT_eSprite   mTextSpr[MAX_LINES];
  TFT_eSprite   mTmpSpr;
  TFT_eSprite   mCursSpr;
  uint16_t*     mBgSprPtr;
  uint16_t*     mTextSprPtr[MAX_LINES];
  uint16_t*     mTmpSprPtr;
  uint16_t*     mCursSprPtr;
  int           mFontType;
  int           mUpdateStartY;
  int           mUpdateEndY;
  int     		  mTextPosX;
  int     		  mTextPosY;
	uint16_t		  mTextColor;
	uint16_t		  mTextBgColor;
	uint8_t 		  mFontStyle;
  char          mGlyphFirstByte;
  bool          mReading2ByteCode;
  int           mAsciiGlyphBytes;
  int           m2ByteGlyphBytes;
  const uint8_t *mAsciiGlyphData;
  const uint8_t *m2ByteGlyphData;
  static char   mScreenChars[MAX_LINES*MAX_COLUMNS*3];
#ifdef SINGLEBYTEGLYPH_TO_RAM
  static uint8_t mAsciiGlyphCatch[16*256];
#endif
  int16_t       mPointerX;
  int16_t       mPointerY;

  static uint16_t    sjisToLiner(uint16_t sjis);
	static int	getLineLength(const char *str);
	void			  setUpdateArea(int startY, int endY);
  uint16_t    conv555To565(int col) { return ((col << 1) & 0xffc0) | (col & 0x1f); }
  void        drawGlyphTo4bppBuffer(const uint8_t *glyphSt, uint8_t *dst, uint16_t xpos, uint16_t fontWidth, uint16_t fontHeight, uint8_t foreColor, uint8_t backColor);
  void        blend4bppImageToBRG555(const uint16_t *src, uint16_t *dst, uint16_t width, uint16_t height, uint16_t transpColor, const uint16_t *cmap);
};

#endif // tftDispSPI_h