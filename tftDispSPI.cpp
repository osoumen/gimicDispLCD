#include "tftDispSPI.h"

#include "shnm8x16r.h"
#include "mplus_j10r_jisx0201.h"
#include "mplus_j10r.h"
#include "shnmk16.h"
#include "misaki_4x8_jisx0201.h"
#include "misaki_gothic.h"
#include "image.h"

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

tftDispSPI::ScreenGlyph   tftDispSPI::mScreenChars[MAX_LINES*MAX_COLUMNS];
#ifdef SINGLEBYTEGLYPH_TO_RAM
uint8_t tftDispSPI::mAsciiGlyphCatch[16*256];
#endif

#ifdef ENABLE_MULTI_CORE
semaphore_t xSemLcdPushWait;
semaphore_t xSemLcdPushMutex;
#ifdef TOUCH_CS
mutex_t xSPIMutex;
#endif
#endif

tftDispSPI::tftDispSPI()
: mBgSpr(&mTft)
, mTmpSpr{TFT_eSprite(&mTft), TFT_eSprite(&mTft)}
, mCursSpr(&mTft)
, mTmpSprPtr{nullptr, nullptr}
, mCursSprPtr(nullptr)
, mWriteTmpSpr(0)
, mReadTmpSpr(0)
, mTextPosX(0)
, mTextPosY(0)
, mTextColor(10)
, mFontStyle(0)
, mGlyphFirstByte(0)
, mReading2ByteCode(false)
, mPointerX(-POINTER_POSY_SIZE)
, mPointerY(-POINTER_POSY_SIZE)
{
  for (int i=0; i<MAX_LINES; ++i) {
    mUpdateStartCol[i] = MAX_COLUMNS;
    mUpdateEndCol[i] = 0;
  }
}

tftDispSPI::~tftDispSPI()
{
}

void tftDispSPI::init()
{
  mTft.init();
  mTft.invertDisplay(INVERT_DISPLAY);
  mTft.setRotation(SCREEN_ROTATION);
  mTft.fillScreen(TFT_BLACK);
  mBgSpr.setColorDepth(16);
  mBgSpr.fillSprite(TFT_BLACK);
  mBgSprPtr = (uint16_t*)mBgSpr.createSprite(VIEW_WIDTH, VIEW_HEIGHT);
  mCursSpr.setColorDepth(16);
  mCursSprPtr = (uint16_t*)mCursSpr.createSprite(imgcurs[0], imgcurs[1]);
  mCursSpr.pushImage(0, 0, imgcurs[0], imgcurs[1], &imgcurs[2]);
  set_charsize(kNormalFont);
  memset(mScreenChars, 0, sizeof(mScreenChars));
  mTft.initDMA();
#ifdef ENABLE_MULTI_CORE
  sem_init(&xSemLcdPushWait, 0, 2);
  sem_init(&xSemLcdPushMutex, 2, 2);
#ifdef TOUCH_CS
  mutex_init(&xSPIMutex);
#endif
#endif
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
#ifdef TOUCH_CS
  bool result;
#ifdef ENABLE_MULTI_CORE
  mutex_enter_blocking(&xSPIMutex);
#endif
  result = mTft.getTouch(x, y, threshold);
#ifdef ENABLE_MULTI_CORE
  mutex_exit(&xSPIMutex);
#endif
  return result;
#else
  return false;
#endif
}

int tftDispSPI::sjisToLiner(int sjis)
{
  int linear;
  int up = sjis >> 8;
  int lo = sjis & 0xff;
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
  const int textHeight = sTextHeight[mFontType];
  const int textWidth = sTextWidth[mFontType];
  const int updateEndRow = VIEW_HEIGHT / textHeight;  // TODO: 定数化
  for (int i=0; i<updateEndRow; ++i) {
    if (mUpdateStartCol[i] < mUpdateEndCol[i]) {
      int updateStartCol = mUpdateStartCol[i];
      int updateEndCol = mUpdateEndCol[i];
      // ２バイト文字の境界にある場合は前後に伸ばす
      const ScreenGlyph *lineText = &mScreenChars[sRowChars[mFontType] * i];
      if (lineText[updateStartCol].glyph < 0) updateStartCol--;
      if (lineText[updateEndCol].glyph < 0 && updateEndCol < sRowChars[mFontType]) updateEndCol++;
      updateStartCol = max(0, updateStartCol);
      updateEndCol = min(sRowChars[mFontType], updateEndCol);

      const int updateRectWidth = (updateEndCol - updateStartCol) * textWidth;
#ifdef ENABLE_MULTI_CORE
      sem_acquire_blocking(&xSemLcdPushMutex);
#endif
      if (mTmpSprPtr[mWriteTmpSpr] != nullptr) {
        mTmpSpr[mWriteTmpSpr].deleteSprite();
      }
      TFT_eSprite *tmpSpr = &mTmpSpr[mWriteTmpSpr];
      tmpSpr->setColorDepth(16);
      mTmpSprPtr[mWriteTmpSpr] = (uint16_t*)tmpSpr->createSprite(updateRectWidth, textHeight);
      // 背景のコピー
      tmpSpr->pushImage(-updateStartCol * textWidth, -i * textHeight, VIEW_WIDTH, VIEW_HEIGHT, mBgSprPtr);
      // 文字の描画
      update1LineText16bppBuffer(updateStartCol, updateEndCol, tmpSpr, mTmpSprPtr[mWriteTmpSpr], i);
#if defined(ENABLE_CURSOR_POINTER)
      redrawCursorPointerToSpr(updateStartCol * textWidth, updateEndCol * textWidth, tmpSpr, i);
#endif
#ifdef ENABLE_MULTI_CORE
      mTmpSprXPos[mWriteTmpSpr] = updateStartCol * textWidth;
      mTmpSprYPos[mWriteTmpSpr] = i*textHeight;
      sem_release(&xSemLcdPushWait);
#else
      mTft.startWrite();
      mTft.pushImageDMA(updateStartCol * textWidth, i*textHeight, updateRectWidth, textHeight, mTmpSprPtr[mWriteTmpSpr]);
      mTft.endWrite();
#endif

      mWriteTmpSpr ^= 1;
      mUpdateStartCol[i] = MAX_COLUMNS;
      mUpdateEndCol[i] = 0;
    }
  }
  return true;
}

void tftDispSPI::redrawCursorPointerToSpr(int updateX1, int updateX2, TFT_eSprite *spr, int line)
{
  const int textHeight = sTextHeight[mFontType];
  int tmpSprSt = line*textHeight;
  int tmpSprEd = (line+1)*textHeight;
  int pointerEdgeX1 = mPointerX - POINTER_NEGX_SIZE;
  int pointerEdgeX2 = mPointerX + POINTER_POSX_SIZE;
  int pointerEdgeY1 = mPointerY - POINTER_NEGY_SIZE;
  int pointerEdgeY2 = mPointerY + POINTER_POSY_SIZE;
  if (
    (tmpSprSt <= pointerEdgeY1 && pointerEdgeY1 < tmpSprEd) || (tmpSprSt <= pointerEdgeY2 && pointerEdgeY2 < tmpSprEd) ||
    (updateX1 <= pointerEdgeX1 && pointerEdgeX1 < updateX2) || (updateX1 <= pointerEdgeX2 && pointerEdgeX2 < updateX2)
    ) {
    mCursSpr.pushToSprite(spr, mPointerX - updateX1, mPointerY - tmpSprSt, TFT_WHITE);
  }
}

void tftDispSPI::update1LineText16bppBuffer(int startCol, int endCol, TFT_eSprite *spr, uint16_t *buffer, int line)
{
  const int fontHeight = sFontHeight[mFontType];
  const int textHeight = sTextHeight[mFontType];
  const int textWidth = sTextWidth[mFontType];
  const int rowChars = endCol - startCol;
  const ScreenGlyph *lineText = &mScreenChars[sRowChars[mFontType] * line + startCol]; 
  for (int drawPos=0; drawPos<rowChars; ++drawPos) {
    const uint8_t fontColor = lineText->b_f_col;
    const uint8_t fontStyle = lineText->attr;
    const int16_t glyph = lineText->glyph;
    ++lineText;
    if (glyph >= 256) {
      int twoByteGlyph = glyph - 256;
      uint16_t fg_color = edisp_4bit_palette[fontColor & 0x0f];
      uint16_t bg_color = edisp_4bit_palette[fontColor >> 4];
      if ((drawPos+1) < rowChars)
        drawGlyphTo16bppBuffer(m2ByteGlyphData+m2ByteGlyphBytes*twoByteGlyph, buffer, rowChars * textWidth, drawPos*textWidth, textWidth*2,fontHeight,fg_color,bg_color);
    }
    else {
      if (glyph > 0) {
        uint16_t fg_color = edisp_4bit_palette[fontColor & 0x0f];
        uint16_t bg_color = edisp_4bit_palette[fontColor >> 4];
        if (glyph == ' ') {
          if (bg_color != TFT_TRANSPARENT) {
            spr->fillRect(drawPos*textWidth,0, textWidth,fontHeight, bg_color);
          }
        }
        else {
          drawGlyphTo16bppBuffer(mAsciiGlyphData+mAsciiGlyphBytes*glyph, buffer, rowChars * textWidth, drawPos*textWidth, textWidth,fontHeight,fg_color,bg_color);
        }
      }
    }
    if (fontStyle & STYLE_UNDERLINED) {
      uint16_t fg_color = edisp_4bit_palette[fontColor & 0x0f];
      if (fg_color != TFT_TRANSPARENT) {
        spr->drawFastHLine(drawPos * textWidth, textHeight-1, textWidth, fg_color);
      }
      else {
        spr->pushImage(drawPos * textWidth, textHeight-1, textWidth, 1, mBgSprPtr+spr->width()*((line+1)*textHeight-1)+drawPos * textWidth);
      }
    }
    else if (fontHeight == 11) {
      // 中フォントのサイズは11pxなので12段目を背景色で描画する
      uint16_t bg_color = edisp_4bit_palette[fontColor >> 4];
      if (bg_color != TFT_TRANSPARENT) {
        spr->drawFastHLine(drawPos * textWidth, textHeight-1, textWidth, bg_color);
      }
    }
  }
}

void tftDispSPI::lcdPushProc()
{
#ifdef ENABLE_MULTI_CORE
  sem_acquire_blocking(&xSemLcdPushWait);
#ifdef TOUCH_CS
  mutex_enter_blocking(&xSPIMutex);
#endif
  const int textHeight = sTextHeight[mFontType];
  mTft.startWrite();
  mTft.pushImageDMA(mTmpSprXPos[mReadTmpSpr], mTmpSprYPos[mReadTmpSpr], mTmpSpr[mReadTmpSpr].width(), textHeight, mTmpSprPtr[mReadTmpSpr]);
  mTft.endWrite();
#ifdef TOUCH_CS
  mutex_exit(&xSPIMutex);
#endif
  sem_release(&xSemLcdPushMutex);
  mReadTmpSpr ^= 1;
#endif
}

void  tftDispSPI::drawGlyphTo16bppBuffer(const uint8_t *glyphSt, uint16_t *dst, uint16_t dstWidth, uint16_t xpos, uint16_t fontWidth, uint16_t fontHeight, uint16_t foreColor, uint16_t backColor)
{
  const uint8_t *readPtr = glyphSt;
  dst += xpos;
  for (int row=0; row<fontHeight; ++row) {
    uint16_t *out = dst;
    uint16_t *out_end = dst + fontWidth;
    int shift=0;
    uint8_t inByte;
    while (out < out_end) {
      if (shift == 0) {
        inByte = *(readPtr++);
      }
      uint16_t color = (inByte & 1) ? foreColor : backColor;
      if (color != TFT_TRANSPARENT) {
        color = (color << 8) | (color >> 8);
        *out = color;
      }
      ++out;
      inByte >>= 1;
      shift = (shift + 1) & 0x07;
    }
    dst += dstWidth;
  }
}

void	tftDispSPI::setUpdateArea(int startCol, int endCol, int row)
{
  if (row < MAX_LINES) {
    if (startCol < mUpdateStartCol[row]) {
      mUpdateStartCol[row] = startCol;
    }
    if (endCol > mUpdateEndCol[row]) {
      mUpdateEndCol[row] = endCol;
    }
  }
}

void	tftDispSPI::setUpdateArea(int startX, int endX, int startY, int endY)
{
    const int textHeight = sTextHeight[mFontType];
    const int textWidth = sTextWidth[mFontType];
    const int updateStartCol = startX / textWidth;
    const int updateEndCol = (endX + textWidth - 1) / textWidth;
    const int updateStartRow = max(0, startY / textHeight);
    const int updateEndRow = min(MAX_LINES, (endY + textHeight - 1) / textHeight);
    for (int i=updateStartRow; i<updateEndRow; ++i) {
      setUpdateArea(updateStartCol, updateEndCol, i);
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
  ScreenGlyph glyph;
  glyph.b_f_col = (mFontStyle & STYLE_INVERTED) ? ((mTextColor << 4) | (mTextColor >> 4)) : mTextColor;
  glyph.attr = mFontStyle;
  int startCol = 0;
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
  int rowChars = sRowChars[mFontType];
  int endLine = VIEW_HEIGHT / sTextHeight[mFontType];

  do {
    const char *lineText = &str[startCol];
    int lineLen = getLineLength(lineText);
    int lineStartCol = mTextPosX;
    int numUpdatesPerLine = 0;

    // すでにある文字と同じか画面外の場合は更新しない
    int drawPos = 0;
    int screenCharInd = rowChars * mTextPosY + mTextPosX;
    while (drawPos < lineLen) {
      if (mReading2ByteCode) {
        // 2バイト文字はバッファリングして句点コードの並びに変換する
        ScreenGlyph glyph2 = glyph;
        glyph2.glyph = -1;  // 負数はマルチバイト文字の開始位置へのオフセットを表す
        glyph.glyph = sjisToLiner((mGlyphFirstByte << 8) | lineText[drawPos]) + 256;
        if (((mTextPosX+1) < rowChars) && (mTextPosY < endLine) && ((mScreenChars[screenCharInd] != glyph) || (mScreenChars[screenCharInd+1] != glyph2))) {
          mScreenChars[screenCharInd] = glyph;
          mScreenChars[screenCharInd+1] = glyph2;
          ++numUpdatesPerLine;
        }
        mReading2ByteCode = false;
        mTextPosX += 2;
        screenCharInd += 2;
      }
      else {
        if ((0x81 <= lineText[drawPos] && lineText[drawPos] <= 0x9f) || (0xe0 <= lineText[drawPos] && lineText[drawPos] <= 0xef)) {
          mGlyphFirstByte = lineText[drawPos];
          mReading2ByteCode = true;
        }
        else {
          // 1バイト文字
          glyph.glyph = lineText[drawPos];
          if ((mTextPosX < rowChars) && (mTextPosY < endLine) && (mScreenChars[screenCharInd] != glyph)) {
            mScreenChars[screenCharInd] = glyph;
            ++numUpdatesPerLine;
          }
          mTextPosX += 1;
          screenCharInd += 1;
        }
      }
      ++drawPos;
    }
    if (numUpdatesPerLine > 0) {
      setUpdateArea(lineStartCol, mTextPosX, mTextPosY);
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
  mReading2ByteCode = false;
  memset(mScreenChars, 0, sizeof(mScreenChars));
  for (int i=0; i<MAX_LINES; ++i) {
    setUpdateArea(0, sRowChars[mFontType], i);
  }
}

void tftDispSPI::move(int r, int c)
{
	mTextPosY = r;
	mTextPosX = c;
  mReading2ByteCode = false;
}

void tftDispSPI::curmove(int r, int c)
{
	mTextPosY += r;
	mTextPosX += c;
  mReading2ByteCode = false;
}

void tftDispSPI::cur_rowtop(void)
{
	mTextPosX = 0;
  mReading2ByteCode = false;
}

void tftDispSPI::set_attribute( int n )
{
	switch (n) {
		case 0:
			mTextColor = 10 | (TFT_EDISP_TRANSPARENT << 4);//TFT_WHITE;
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
			mTextColor = TFT_EDISP_TRANSPARENT | (TFT_EDISP_TRANSPARENT << 4);
			break;
		case 30:
      mTextColor &= 0xf0;
			mTextColor |= 1;//TFT_BLACK;
			break;
		case 17:
		case 31:
      mTextColor &= 0xf0;
			mTextColor |= 3;//TFT_RED;
			break;
		case 18:
		case 32:
      mTextColor &= 0xf0;
			mTextColor |= 6;//TFT_GREEN;
			break;
		case 19:
		case 33:
      mTextColor &= 0xf0;
			mTextColor |= 5;//TFT_YELLOW;
			break;
		case 20:
		case 34:
      mTextColor &= 0xf0;
			mTextColor |= 7;//TFT_BLUE;
			break;
		case 21:
		case 35:
      mTextColor &= 0xf0;
			mTextColor |= 12;//TFT_MAGENTA;
			break;
		case 22:
		case 36:
      mTextColor &= 0xf0;
			mTextColor |= 11;//TFT_CYAN;
			break;
		case 23:
		case 37:
		case 39:
      mTextColor &= 0xf0;
			mTextColor |= 10;//TFT_WHITE;
			break;
			
		case 40:
      mTextColor &= 0x0f;
			mTextColor |= 1 << 4;//TFT_BLACK;
			break;
		case 41:
      mTextColor &= 0x0f;
			mTextColor |= 3 << 4;//TFT_RED;
			break;
		case 42:
      mTextColor &= 0x0f;
			mTextColor |= 6 << 4;//TFT_GREEN;
			break;
		case 43:
      mTextColor &= 0x0f;
			mTextColor |= 5 << 4;//TFT_YELLOW;
			break;
		case 44:
      mTextColor &= 0x0f;
			mTextColor |= 7 << 4;//TFT_BLUE;
			break;
		case 45:
      mTextColor &= 0x0f;
			mTextColor |= 12 << 4;//TFT_MAGENTA;
			break;
		case 46:
      mTextColor &= 0x0f;
			mTextColor |= 11 << 4;//TFT_CYAN;
			break;
		case 47:
      mTextColor &= 0x0f;
			mTextColor |= 10 << 4;//TFT_WHITE;
			break;
		case 49:
      mTextColor &= 0x0f;
			mTextColor |= TFT_EDISP_TRANSPARENT << 4;
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
      mTextColor &= 0xf0;
			mTextColor |= TFT_EDISP_TRANSPARENT;
			break;
		case 61:
      mTextColor &= 0x0f;
			mTextColor |= TFT_EDISP_TRANSPARENT << 4;
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
  del(sRowChars[mFontType] - mTextPosX);
}

void tftDispSPI::del_row()
{
  int currentX = mTextPosX;
  cur_rowtop();
  del(sRowChars[mFontType]);
  mTextPosX = currentX;
}

void tftDispSPI::del(int n)
{
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
  int delSt = sRowChars[mFontType] * mTextPosY + mTextPosX;
  int delEnd = delSt + n;
  if ((delSt < 0) || (delEnd >= (MAX_LINES*MAX_COLUMNS))) return;

  int unchanged = 0;
  int headUnchanged = 0;
  int tailUnchanged = 0;
  const ScreenGlyph *ptr = &mScreenChars[delSt];
  for (int i=mTextPosX; i<(mTextPosX+n); ++i) {
    if (ptr->glyph == 0) {
      unchanged++;
    }
    else {
      if (unchanged > 0 && unchanged == (i-mTextPosX)) headUnchanged = unchanged;
      unchanged = 0;
    }
    ++ptr;
  }
  tailUnchanged = unchanged;

  memset(&mScreenChars[delSt], 0, n * sizeof(ScreenGlyph));
  setUpdateArea(mTextPosX + headUnchanged, mTextPosX + n - tailUnchanged, mTextPosY);
  mReading2ByteCode = false;
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
    int textHeight = sTextHeight[x];
    int textLines = VIEW_HEIGHT / textHeight;
    mFontType = x;
    memset(mScreenChars, 0, sizeof(mScreenChars));

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
  mReading2ByteCode = false;
}

void tftDispSPI::set_bgbuff(int x)
{
	// mCurrentBg = x;
}

void tftDispSPI::clear_bgbuff(int x)
{
	mBgSpr.fillSprite(TFT_BLACK);
  for (int i=0; i<MAX_LINES; ++i) {
    setUpdateArea(0, sRowChars[mFontType], i);
  }
}

void tftDispSPI::draw_fillrect(int bg, int w, int h, int x, int y, int col )
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.fillRect(x, y, w, h, tft_col);
  setUpdateArea(x, x+w, y, y+h);
}

void tftDispSPI::draw_line(int bg, int x0, int y0, int x1, int y1, int col )
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.drawLine(x0, y0, x1, y1, tft_col);
  setUpdateArea(min(x0,x1), max(x0,x1)+1, min(y0,y1), max(y0,y1)+1);
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
  setUpdateArea(cx - xw, cx + xw + 1, cy - yh, cy + yh + 1);
}

void tftDispSPI::draw_rectangle(int bg, int w, int h, int x, int y, int col)
{
  uint16_t tft_col = conv555To565(col);
  mBgSpr.drawRect(x, y, w, h, tft_col);
  setUpdateArea(x, x+w, y, y+h);
}

void tftDispSPI::draw_image(int file, int bg, int x, int y)
{
  if (file > 117) return;
  const uint16_t  *imagePtr = imgArray[file];
  if (imagePtr != nullptr) {
    mBgSpr.pushImage(x, y, imagePtr[0], imagePtr[1], &imagePtr[2]);
    setUpdateArea(x, x+imagePtr[0], y, y+imagePtr[1]);
  }
}

void tftDispSPI::draw_image2(int file, int bg, int fol, int x, int y)
{
  // image2は未使用
}

void  tftDispSPI::setCursorPointer(int16_t x, int16_t y)
{
#if defined(ENABLE_CURSOR_POINTER)
  setUpdateArea(mPointerX - POINTER_NEGX_SIZE, mPointerX + POINTER_POSX_SIZE, mPointerY - POINTER_NEGY_SIZE, mPointerY + POINTER_POSY_SIZE);
  mPointerX = x;
  mPointerY = y;
  setUpdateArea(mPointerX - POINTER_NEGX_SIZE, mPointerX + POINTER_POSX_SIZE, mPointerY - POINTER_NEGY_SIZE, mPointerY + POINTER_POSY_SIZE);
#endif
}

void  tftDispSPI::hideCursorPointer()
{
#if defined(ENABLE_CURSOR_POINTER)
  setUpdateArea(mPointerX - POINTER_NEGX_SIZE, mPointerX + POINTER_POSX_SIZE, mPointerY - POINTER_NEGY_SIZE, mPointerY + POINTER_POSY_SIZE);
  mPointerX = -POINTER_POSX_SIZE;
  mPointerY = -POINTER_POSY_SIZE;
#endif
}
