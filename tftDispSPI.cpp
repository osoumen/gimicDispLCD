#include "tftDispSPI.h"

#include "shnm8x16r.h"
#include "mplus_j10r_jisx0201.h"
#include "mplus_j10r.h"
#include "shnmk16.h"
#include "misaki_4x8_jisx0201.h"
#include "misaki_gothic.h"
#include "image.h"

#define ENABLE_CURSOR_POINTER 1

const uint16_t imgcurs[] PROGMEM = {
    8, 12,
    0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0x0000,0x3CE7,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0x0000,0x3CE7,0x3CE7,0x0000,0xFFFF,0xFFFF,0xFFFF,0xFFFF,
    0x0000,0x3CE7,0x3CE7,0x3CE7,0x0000,0xFFFF,0xFFFF,0xFFFF,
    0x0000,0x3CE7,0x3CE7,0x3CE7,0x3CE7,0x0000,0xFFFF,0xFFFF,
    0x0000,0x3CE7,0x3CE7,0x3CE7,0x3CE7,0x3CE7,0x0000,0xFFFF,
    0x0000,0x3CE7,0x3CE7,0x3CE7,0x0000,0x0000,0x0000,0xFFFF,
    0x0000,0x3CE7,0x0000,0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,
    0x0000,0x0000,0xFFFF,0x0000,0x0000,0xFFFF,0xFFFF,0xFFFF,
    0xFFFF,0xFFFF,0xFFFF,0x0000,0x0000,0x0000,0xFFFF,0xFFFF,
    0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x0000,0x0000,0xFFFF,0xFFFF,
};

static const uint16_t edisp_4bit_palette[] PROGMEM = {
  TFT_TRANSPARENT,
  TFT_BLACK,
  TFT_BROWN,
  TFT_RED,
  TFT_ORANGE,
  TFT_YELLOW,
  TFT_GREEN,
  TFT_BLUE,
  TFT_PURPLE,
  TFT_DARKGREY,
  TFT_WHITE,
  TFT_CYAN,
  TFT_MAGENTA,
  TFT_MAROON,
  TFT_DARKGREEN,
  TFT_NAVY,
};

const int sFontHeight[3] PROGMEM = {8, 11, 16};
const int sTextHeight[3] PROGMEM = {8, 12, 16};
const int sTextWidth[3] PROGMEM = {4, 5, 8};
const int sRowChars[3] PROGMEM = {80, 64, 40};

char   tftDispSPI::mScreenChars[MAX_LINES*MAX_COLUMNS*3];
#ifdef SINGLEBYTEGLYPH_TO_RAM
uint8_t tftDispSPI::mAsciiGlyphCatch[16*256];
#endif

tftDispSPI::tftDispSPI()
: mBgSpr(&mTft)
, mTmpSpr(&mTft)
, mCursSpr(&mTft)
, mTmpSprPtr(nullptr)
, mCursSprPtr(nullptr)
, mUpdateStartY(VIEW_HEIGHT)
, mUpdateEndY(0)
, mTextPosX(0)
, mTextPosY(0)
, mTextColor(10)
, mTextBgColor(TFT_EDISP_TRANSPARENT)
, mFontStyle(0)
, mPointerX(-POINTER_POSY_SIZE)
, mPointerY(-POINTER_POSY_SIZE)
{
}

tftDispSPI::~tftDispSPI()
{
}

void tftDispSPI::init()
{
  mTft.init();
  mTft.invertDisplay(true);
  mTft.setRotation(1);
  mTft.fillScreen(TFT_BLACK);
  mBgSpr.setColorDepth(16);
  mBgSpr.fillSprite(TFT_BLACK);
  mBgSprPtr = (uint16_t*)mBgSpr.createSprite(VIEW_WIDTH, VIEW_HEIGHT);
  mCursSpr.setColorDepth(16);
  mCursSprPtr = (uint16_t*)mCursSpr.createSprite(imgcurs[0], imgcurs[1]);
  mCursSpr.pushImage(0, 0, imgcurs[0], imgcurs[1], &imgcurs[2]);
  set_charsize(kNormalFont);
  memset(mScreenChars, 0, (3 * VIEW_WIDTH * VIEW_HEIGHT) / (sTextHeight[kNormalFont] * sTextWidth[kNormalFont]));
  mTft.initDMA();
}

void  tftDispSPI::touch_calibrate(uint16_t *calData)
{
  uint8_t calDataOK = 0;

  mTft.fillScreen(TFT_BLACK);
  mTft.setCursor(20, 0);
  mTft.setTextFont(2);
  mTft.setTextSize(1);
  mTft.setTextColor(TFT_WHITE, TFT_BLACK);

  mTft.println("Touch corners as indicated");

  mTft.setTextFont(1);
  mTft.println();

  mTft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  mTft.fillScreen(TFT_BLACK);
  
  mTft.setTextColor(TFT_GREEN, TFT_BLACK);
  mTft.println("Calibration complete!");

  delay(3000);
}

void  tftDispSPI::set_calibrate(const uint16_t *calData)
{
  mTft.setTouch((uint16_t *)calData);
}

bool  tftDispSPI::getTouch(uint16_t *x, uint16_t *y, uint16_t threshold)
{
  return mTft.getTouch(x, y, threshold);
}

uint16_t tftDispSPI::sjisToLiner(uint16_t sjis)
{
  uint16_t linear;
  uint8_t up = sjis >> 8;
  uint8_t lo = sjis & 0xff;
  if (0x81 <= up && up <= 0x9f) {
    linear = up - 0x81;
  }
  else if (0xe0 <= up && up <= 0xef) {
    linear = up - 0xc1;
  }
  else {
    return 0;
  }
  if(0x40 <= lo && lo <= 0x7e) {
    linear *= 188;
    linear += lo - 0x40;
  }
  else if (0x80 <= lo && lo <= 0xfc) {
    linear *= 188;
    linear += 63;
    linear += lo - 0x80;
  }
  else {
    return 0;
  }
  return linear;
}

bool tftDispSPI::updateContent()
{
	if (mUpdateStartY < mUpdateEndY) {
    if (mTft.dmaBusy()) return false;
    const int fontHeight = sFontHeight[mFontType];
    const int textHeight = sTextHeight[mFontType];
    const int textWidth = sTextWidth[mFontType];
    const int rowChars = sRowChars[mFontType];
    const int updateStartRow = mUpdateStartY / textHeight;
    const int updateEndRow = (mUpdateEndY + textHeight - 1) / textHeight;
    for (int i=updateStartRow; i<updateEndRow; ++i) {
      mTmpSpr.pushImage(0, 0, VIEW_WIDTH, textHeight, mBgSprPtr+VIEW_WIDTH*i*textHeight);

      char glyphFirstByte;
      bool reading2ByteCode = false;
      const char *lineText = &mScreenChars[rowChars * i * 3]; 
      for (int drawPos=0; drawPos<rowChars; ++drawPos) {
        const uint8_t fontColor = *(lineText++);
        const uint8_t fontStyle = *(lineText++);
        const char  glyph = *(lineText++);
        if (reading2ByteCode) {
          int twoByteGlyph = sjisToLiner((glyphFirstByte << 8) | glyph);
          uint16_t fg_color = edisp_4bit_palette[fontColor & 0x0f];
          uint16_t bg_color = edisp_4bit_palette[fontColor >> 4];
          // mTmpSpr.drawXBitmap((drawPos-1)*textWidth,0,m2ByteGlyphData+m2ByteGlyphBytes*twoByteGlyph,textWidth*2,fontHeight,fg_color);
          drawGlyphToBRG555Buffer(m2ByteGlyphData+m2ByteGlyphBytes*twoByteGlyph, mTmpSprPtr, (drawPos-1)*textWidth, textWidth*2,fontHeight,fg_color,bg_color);
          reading2ByteCode = false;
        }
        else {
          if (glyph != 0) {
            if ((0x81 <= glyph && glyph <= 0x9f) || (0xe0 <= glyph && glyph <= 0xef)) {
              glyphFirstByte = glyph;
              reading2ByteCode = true;
            }
            else {
              uint16_t fg_color = edisp_4bit_palette[fontColor & 0x0f];
              uint16_t bg_color = edisp_4bit_palette[fontColor >> 4];
              if (glyph == ' ') {
                if (bg_color != TFT_TRANSPARENT) {
                  mTmpSpr.fillRect(drawPos*textWidth,0, textWidth,fontHeight, bg_color);
                }
              }
              else {
                // mTmpSpr.drawXBitmap(drawPos*textWidth,0,mAsciiGlyphData+mAsciiGlyphBytes*glyph,textWidth,fontHeight,fg_color,bg_color);
                drawGlyphToBRG555Buffer(mAsciiGlyphData+mAsciiGlyphBytes*glyph, mTmpSprPtr, drawPos*textWidth, textWidth,fontHeight,fg_color,bg_color);
              }
            }
          }
        }
        if (fontStyle & STYLE_UNDERLINED) {
          uint16_t color = edisp_4bit_palette[fontColor & 0x0f];
          if (color != TFT_TRANSPARENT) {
            mTmpSpr.drawFastHLine(drawPos * textWidth, textHeight-1, textWidth, color);
          }
        }
        else if (fontHeight == 11) {
          // 中フォントのサイズは11pxなので12段目を背景色で描画する
          uint16_t color = edisp_4bit_palette[fontColor >> 4];
          if (color != TFT_TRANSPARENT) {
            mTmpSpr.drawFastHLine(drawPos * textWidth, textHeight-1, textWidth, color);
          }
        }
      }

#if defined(ENABLE_CURSOR_POINTER)
      int tmpSprSt = i*textHeight;
      int tmpSprEd = (i+1)*textHeight;
      int pointerEdge1 = mPointerY - POINTER_NEGY_SIZE;
      int pointerEdge2 = mPointerY + POINTER_POSY_SIZE;
      if ((tmpSprSt <= pointerEdge1 && pointerEdge1 < pointerEdge1) || (tmpSprSt <= pointerEdge2 && pointerEdge1 < pointerEdge2)) {
        mCursSpr.pushToSprite(&mTmpSpr, mPointerX, mPointerY - tmpSprSt, TFT_WHITE);
      }
#endif
      mTft.startWrite();
      mTft.pushImageDMA(0, i*textHeight, VIEW_WIDTH, textHeight, mTmpSprPtr);
      mTft.endWrite();
    }

    mUpdateStartY = VIEW_HEIGHT;
    mUpdateEndY = 0;
	}
  return true;
}

void  tftDispSPI::drawGlyphToBRG555Buffer(const uint8_t *glyphSt, uint16_t *dst, uint16_t xpos, uint16_t fontWidth, uint16_t fontHeight, uint16_t foreColor, uint16_t backColor)
{
  // uint8_t buf[32];
  // memcpy(buf, glyphSt, ((fontWidth + 7)>>3) * fontHeight);
  const uint8_t *readPtr = glyphSt;
  for (int row=0; row<fontHeight; ++row) {
    uint16_t *out = dst + (xpos + VIEW_WIDTH * row);
    int x=0;
    int shift=0;
    uint8_t inByte;
    while (x<fontWidth) {
      if (shift == 0) {
        inByte = *(readPtr++);
      }
      uint16_t color = (inByte & 1) ? foreColor : backColor;
      if (color != TFT_TRANSPARENT) {
        color = ((color << 10)&0x7c00) | ((color >> 6)&0x03ff);
        *out = color;
      }
      ++out;
      inByte >>= 1;
      shift = (shift + 1) & 0x07;
      ++x;
    }
  }
}

void	tftDispSPI::setUpdateArea(int startY, int endY)
{
  if (startY < mUpdateStartY) {
    mUpdateStartY = startY;
  }
  if (endY > mUpdateEndY) {
    mUpdateEndY = endY;
  }
}

int tftDispSPI::getLineLength(const char *str)
{
	int len = 0;
	while (str[len] != '\n' && str[len] != '\r' && str[len] != 0) {
		++len;
	}
	return len;
}

void tftDispSPI::puts_(const char* str, uint32_t max_len)
{
  uint16_t	foreColor;
  uint16_t	backColor;
  if (mFontStyle & STYLE_INVERTED) {
    foreColor = mTextBgColor;
    backColor = mTextColor;
  }
  else {
    foreColor = mTextColor;
    backColor = mTextBgColor;
  }
  char back_foreColor = (backColor << 4) | foreColor;
  int startCol = 0;

  int textHeight = sTextHeight[mFontType];
  int rowChars = sRowChars[mFontType];
  int endLine = VIEW_HEIGHT / sTextHeight[mFontType];

  do {
    const char *lineText = &str[startCol];
    int lineLen = getLineLength(lineText);
    int lineStartPosX = mTextPosX;
    int lineStartY = mTextPosY * textHeight;
    int lineEndY = (mTextPosY + 1) * textHeight;
    int numUpdatesPerLine = 0;

    // テキストを1文字ずつ描画
    int drawPos = 0;
    int screenCharInd = (rowChars * mTextPosY + mTextPosX) * 3;
    while (drawPos < lineLen) {
      if ((mTextPosX < rowChars) && (mTextPosY < endLine) && 
          ((mScreenChars[screenCharInd] != back_foreColor) ||
          (mScreenChars[screenCharInd+1] != mFontStyle) ||
          (mScreenChars[screenCharInd+2] != lineText[drawPos]))) {
        mScreenChars[screenCharInd] = back_foreColor;
        mScreenChars[screenCharInd+1] = mFontStyle;
        mScreenChars[screenCharInd+2] = lineText[drawPos];
        ++numUpdatesPerLine;
      }
      mTextPosX += 1;
      screenCharInd += 3;
      ++drawPos;
    }
    if (numUpdatesPerLine > 0) {
      setUpdateArea(lineStartY, lineEndY);
    }
    // 改行コードがあれば次の行に進む処理
    startCol += lineLen;
    while (str[startCol] == '\r' || str[startCol] == '\n') {
      if (str[startCol] == '\r') {
        mTextPosX = 0;
      }
      if (str[startCol] == '\n') {
        ++mTextPosY;
      }
      ++startCol;
    }
  } while (str[startCol] != 0);
}

void tftDispSPI::printw(const char* fmt, ...)
{
	va_list arg;
	static char buf[100];
	va_start(arg, fmt);
	vsnprintf(buf, 100, fmt, arg);
	va_end(arg);
	puts_(buf);
}

void tftDispSPI::vprintw(const char* fmt, va_list arg)
{
	static char buf[100];
	vsnprintf(buf, 100, fmt, arg);
	puts_(buf);
}

void tftDispSPI::clear(void)
{
	mTextPosX = mTextPosY = 0;
  mBgSpr.fillSprite(TFT_BLACK);
  memset(mScreenChars, 0, (3 * VIEW_WIDTH * VIEW_HEIGHT) / (sTextHeight[mFontType] * sTextWidth[mFontType]));
  setUpdateArea(0, VIEW_HEIGHT);
}

void tftDispSPI::move(int r, int c)
{
	mTextPosY = r;
	mTextPosX = c;
}

void tftDispSPI::curmove(int r, int c)
{
	mTextPosY += r;
	mTextPosX += c;
}

void tftDispSPI::cur_rowtop(void)
{
	mTextPosX = 0;
}

void tftDispSPI::set_attribute( int n )
{
	switch (n) {
		case 0:
			mTextColor = 10;//TFT_WHITE;
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			mFontStyle &= ~STYLE_BOLD;
			mFontStyle &= ~STYLE_UNDERLINED;
			mFontStyle &= ~STYLE_BLINKED;
			mFontStyle &= ~STYLE_INVERTED;
			break;
		case 1:
			mFontStyle |= STYLE_BOLD;
			break;
		case 4:
			mFontStyle |= STYLE_UNDERLINED;
			break;
		case 5:
			mFontStyle |= STYLE_BLINKED;
			break;
		case 7:
			mFontStyle |= STYLE_INVERTED;
			break;
		case 8:
		case 16:
			mTextColor = TFT_EDISP_TRANSPARENT;
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			break;
		case 30:
			mTextColor = 1;//TFT_BLACK;
			break;
		case 17:
		case 31:
			mTextColor = 3;//TFT_RED;
			break;
		case 18:
		case 32:
			mTextColor = 6;//TFT_GREEN;
			break;
		case 19:
		case 33:
			mTextColor = 5;//TFT_YELLOW;
			break;
		case 20:
		case 34:
			mTextColor = 7;//TFT_BLUE;
			break;
		case 21:
		case 35:
			mTextColor = 12;//TFT_MAGENTA;
			break;
		case 22:
		case 36:
			mTextColor = 11;//TFT_CYAN;
			break;
		case 23:
		case 37:
		case 39:
			mTextColor = 10;//TFT_WHITE;
			break;
			
		case 40:
			mTextBgColor = 1;//TFT_BLACK;
			break;
		case 41:
			mTextBgColor = 3;//TFT_RED;
			break;
		case 42:
			mTextBgColor = 6;//TFT_GREEN;
			break;
		case 43:
			mTextBgColor = 5;//TFT_YELLOW;
			break;
		case 44:
			mTextBgColor = 7;//TFT_BLUE;
			break;
		case 45:
			mTextBgColor = 12;//TFT_MAGENTA;
			break;
		case 46:
			mTextBgColor = 11;//TFT_CYAN;
			break;
		case 47:
			mTextBgColor = 10;//TFT_WHITE;
			break;
		case 49:
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			break;
		case 24:
			mFontStyle &= ~STYLE_UNDERLINED;
			break;
		case 25:
			mFontStyle &= ~STYLE_BLINKED;
			break;
		case 27:
			mFontStyle &= ~STYLE_INVERTED;
			break;
		case 29:
			mFontStyle &= ~STYLE_BOLD;
			break;
		case 60:
			mTextColor = TFT_EDISP_TRANSPARENT;
			break;
		case 61:
			mTextBgColor = TFT_EDISP_TRANSPARENT;
			break;
	}
}
void tftDispSPI::set_attribute( int n1, int n2 )
{
	set_attribute(n1);
	set_attribute(n2);
}

void tftDispSPI::set_attribute( int n1, int n2, int n3 )
{
	set_attribute(n1);
	set_attribute(n2);
	set_attribute(n3);
}

void tftDispSPI::set_attribute( int n1, int n2, int n3, int n4 )
{
	set_attribute(n1);
	set_attribute(n2);
	set_attribute(n3);
	set_attribute(n4);
}

void tftDispSPI::del_to_end()
{
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
  int delSt = (sRowChars[mFontType] * mTextPosY + mTextPosX) * 3;
  int delEnd = delSt + (sRowChars[mFontType] - mTextPosX) * 3;
  if ((delSt < 0) || (delEnd >= sizeof(mScreenChars)) || (mTextPosX >= sRowChars[mFontType])) return;
  memset(&mScreenChars[delSt], 0, (sRowChars[mFontType] - mTextPosX) * 3);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
}

void tftDispSPI::del_row()
{
  int textHeight = sTextHeight[mFontType];
  int delSt = (sRowChars[mFontType] * mTextPosY) * 3;
  int delEnd = delSt + sRowChars[mFontType] * 3;
  if ((delSt < 0) || (delEnd >= sizeof(mScreenChars))) return;
  memset(&mScreenChars[delSt], 0, sRowChars[mFontType] * 3);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
}

void tftDispSPI::del(int n)
{
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
  int delSt = (sRowChars[mFontType] * mTextPosY + mTextPosX) * 3;
  int delEnd = delSt + n * 3;
  if ((delSt < 0) || (delEnd >= sizeof(mScreenChars))) return;
  memset(&mScreenChars[delSt], 0, n * 3);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
}

void tftDispSPI::save_attribute(int n)
{
	// 非対応
}

void tftDispSPI::load_attribute(int n)
{
	// 非対応
}

void tftDispSPI::curon(void)
{
	// 非対応
}

void tftDispSPI::curoff(void)
{
	// 非対応
}

void tftDispSPI::init_disp(void)
{
  mBgSpr.fillSprite(TFT_BLACK);
	clear();
	set_attribute(0);
}

void tftDispSPI::set_charsize(int x)
{
  if (mFontType != x) {
    mFontType = x;
    if (mTmpSprPtr != nullptr) {
      mTmpSpr.deleteSprite();
      mTmpSprPtr = nullptr;
    }
    int textHeight = sTextHeight[x];
    int textLines = VIEW_HEIGHT / textHeight;
    memset(mScreenChars, 0, (3 * VIEW_WIDTH*VIEW_HEIGHT) / (sTextHeight[mFontType] * sTextWidth[mFontType]));
    mTmpSpr.setColorDepth(16);
    mTmpSprPtr = (uint16_t*)mTmpSpr.createSprite(VIEW_WIDTH, textHeight);

    switch(x) {
      case kTinyFont:
        mAsciiGlyphBytes = 8;
        m2ByteGlyphBytes = 8;
#ifdef SINGLEBYTEGLYPH_TO_RAM
        memcpy(mAsciiGlyphCatch, misaki_4x8_jisx0201, sizeof(misaki_4x8_jisx0201));
        mAsciiGlyphData = mAsciiGlyphCatch;
#else
        mAsciiGlyphData = misaki_4x8_jisx0201;
#endif
        m2ByteGlyphData = misaki_gothic;
        break;
      case kSmallFont:
        mAsciiGlyphBytes = 11;
        m2ByteGlyphBytes = 22;
#ifdef SINGLEBYTEGLYPH_TO_RAM
        memcpy(mAsciiGlyphCatch, mplus_j10r_jisx0201, sizeof(mplus_j10r_jisx0201));
        mAsciiGlyphData = mAsciiGlyphCatch;
#else
        mAsciiGlyphData = mplus_j10r_jisx0201;
#endif
        m2ByteGlyphData = mplus_j10r;
        break;
      case kNormalFont:
        mAsciiGlyphBytes = 16;
        m2ByteGlyphBytes = 32;
#ifdef SINGLEBYTEGLYPH_TO_RAM
        memcpy(mAsciiGlyphCatch, shnm8x16r, sizeof(shnm8x16r));
        mAsciiGlyphData = mAsciiGlyphCatch;
#else
        mAsciiGlyphData = shnm8x16r;
#endif
        m2ByteGlyphData = shnmk16;
        break;
    }
  }
}

void tftDispSPI::set_bgbuff(int x)
{
	// mCurrentBg = x;
}

void tftDispSPI::clear_bgbuff(int x)
{
	mBgSpr.fillSprite(TFT_BLACK);
  mUpdateStartY = 0;
  mUpdateEndY = VIEW_HEIGHT;
  setUpdateArea(0, VIEW_HEIGHT);
}

void tftDispSPI::draw_fillrect(int bg, int w, int h, int x, int y, int col )
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.fillRect(x, y, w, h, tft_col);
  setUpdateArea(y, y+h);
}

void tftDispSPI::draw_line(int bg, int x0, int y0, int x1, int y1, int col )
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.drawLine(x0, y0, x1, y1, tft_col);
  setUpdateArea(y0, y1);
}

void tftDispSPI::draw_ellipse(int bg, int fill, int cx, int cy, int xw, int yh, int col )
{
  uint16_t tft_col = conv555To565(col);
  if (fill) {
    mBgSpr.fillEllipse(cx, cy, xw, yh, tft_col);
  }
  else {
    mBgSpr.drawEllipse(cx, cy, xw, yh, tft_col);
  }
  setUpdateArea(max(0, cy - yh), min(VIEW_HEIGHT, cy + yh + 1));
}

void tftDispSPI::draw_rectangle(int bg, int w, int h, int x, int y, int col)
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.drawRect(x, y, w, h, tft_col);
  setUpdateArea(y, y+h);
}

void tftDispSPI::draw_image(int file, int bg, int x, int y)
{
  if (file > 117) return;
  const uint16_t  *imagePtr = imgArray[file];
  if (imagePtr != nullptr) {
    mBgSpr.pushImage(x, y, imagePtr[0], imagePtr[1], &imagePtr[2]);
    setUpdateArea(y, y+imagePtr[1]);
  }
}

void tftDispSPI::draw_image2(int file, int bg, int fol, int x, int y)
{
  // image2は未使用
}

void  tftDispSPI::setCursorPointer(int16_t x, int16_t y)
{
#if defined(ENABLE_CURSOR_POINTER)
  setUpdateArea(mPointerY - POINTER_NEGY_SIZE, mPointerY + POINTER_POSY_SIZE);
  mPointerX = x;
  mPointerY = y;
  setUpdateArea(mPointerY - POINTER_NEGY_SIZE, mPointerY + POINTER_POSY_SIZE);
#endif
}

void  tftDispSPI::hideCursorPointer()
{
#if defined(ENABLE_CURSOR_POINTER)
  setUpdateArea(mPointerY - POINTER_NEGY_SIZE, mPointerY + POINTER_POSY_SIZE);
  mPointerX = -POINTER_POSY_SIZE;
  mPointerY = -POINTER_POSY_SIZE;
#endif
}
