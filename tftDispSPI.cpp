#include "tftDispSPI.h"

#include "shnm8x16r.h"
#include "mplus_j10r_jisx0201.h"
#include "mplus_j10r.h"
#include "shnmk16.h"
#include "misaki_4x8_jisx0201.h"
#include "misaki_gothic.h"
#include "image.h"

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
, mTextSpr{TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)
, TFT_eSprite(&mTft)}
, mTmpSpr(&mTft)
, mTextSprPtr{nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr
, nullptr}
, mTmpSprPtr(nullptr)
, mUpdateStartY(VIEW_HEIGHT)
, mUpdateEndY(0)
, mTextPosX(0)
, mTextPosY(0)
, mTextColor(9)
, mTextBgColor(TFT_EDISP_TRANSPARENT)
, mFontStyle(0)
, mGlyphFirstByte(0)
, mReading2ByteCode(0)
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
  set_charsize(kNormalFont);
  memset(mScreenChars, 0, (3 * VIEW_WIDTH * VIEW_HEIGHT) / (sTextHeight[kNormalFont] * sTextWidth[kNormalFont]));
  mTft.initDMA();

  // TODO: キャリブレーション処理で得た値を使用する
  uint16_t calData[5] = { 420, 3487, 287, 3524, 3 };
  mTft.setTouch(calData);
}

bool  tftDispSPI::getTouch(uint16_t *x, uint16_t *y)
{
  return mTft.getTouch(x, y);
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
    // ちらつきあり
/*
		mTft.startWrite();
    mTft.pushImageDMA(0, mUpdateStartY, VIEW_WIDTH, mUpdateEndY - mUpdateStartY, mBgSprPtr+VIEW_WIDTH*mUpdateStartY);
    int textHeight = sTextHeight[mFontType];
    int updateStartRow = mUpdateStartY / textHeight;
    int updateEndRow = (mUpdateEndY + textHeight - 1) / textHeight;
    for (int i=updateStartRow; i<updateEndRow; ++i) {
      mTextSpr[i].pushSprite(0, textHeight * i, TFT_EDISP_TRANSPARENT);
    }
    mTft.endWrite();
    */
    
    int textHeight = sTextHeight[mFontType];
    int updateStartRow = mUpdateStartY / textHeight;
    int updateEndRow = (mUpdateEndY + textHeight - 1) / textHeight;
    for (int i=updateStartRow; i<updateEndRow; ++i) {
      mTmpSpr.pushImage(0, 0, VIEW_WIDTH, textHeight, mBgSprPtr+VIEW_WIDTH*i*textHeight);
      blend4bppImageToBRG555(mTextSprPtr[i], mTmpSprPtr, VIEW_WIDTH, textHeight, TFT_EDISP_TRANSPARENT, default_4bit_palette);// mTextSpr[i].pushToSprite(&mTmpSpr, 0, 0, TFT_EDISP_TRANSPARENT);
      mTft.startWrite();
      mTft.pushImageDMA(0, i*textHeight, VIEW_WIDTH, textHeight, mTmpSprPtr);
      mTft.endWrite();
    }

    mUpdateStartY = VIEW_HEIGHT;
    mUpdateEndY = 0;
	}
  return true;
}

void  tftDispSPI::blend4bppImageToBRG555(const uint16_t *src, uint16_t *dst, uint16_t width, uint16_t height, uint16_t transpColor, const uint16_t *cmap)
{
  const uint8_t *in = (const uint8_t*)src;
  int     nibble = 4;
  int pixels = width * height;
  for (int i=0; i<pixels; ++i) {
    uint16_t  index = (*in >> nibble) & 0x0f;
    if (index != transpColor) {
      uint16_t color = cmap[index];
      *dst = ((color << 10)&0x7c00) | ((color >> 6)&0x03ff);
      // *dst = color;
    }
    ++dst;
    nibble = (nibble + 4) & 4;
    if (nibble == 4) {
      ++in;
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

  int fontHeight = sFontHeight[mFontType];
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
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
      if (mReading2ByteCode) {
        // 2バイト文字はバッファリングして句点コードの並びに変換する
        if ((mTextPosX < rowChars) && (mTextPosY < endLine) &&
          ((mScreenChars[screenCharInd] != back_foreColor) ||
          (mScreenChars[screenCharInd+1] != mFontStyle) ||
          (mScreenChars[screenCharInd+2] != mGlyphFirstByte) ||
          /*(back_foreColor != mScreenChars[screenCharInd+2]) ||*/
          (mScreenChars[screenCharInd+3] != lineText[drawPos]))) {
          mScreenChars[screenCharInd] = back_foreColor;
          mScreenChars[screenCharInd+1] = mFontStyle;
          mScreenChars[screenCharInd+2] = mGlyphFirstByte;
          mScreenChars[screenCharInd+3] = back_foreColor;
          mScreenChars[screenCharInd+4] = mFontStyle;
          mScreenChars[screenCharInd+5] = lineText[drawPos];
          int glyph = sjisToLiner((mGlyphFirstByte << 8) | lineText[drawPos]);
          // mTextSpr[mTextPosY].drawXBitmap(mTextPosX*textWidth,0,m2ByteGlyphData+m2ByteGlyphBytes*glyph,textWidth*2,fontHeight,foreColor,backColor);
          drawGlyphTo4bppBuffer(m2ByteGlyphData+m2ByteGlyphBytes*glyph, (uint8_t *)mTextSprPtr[mTextPosY], mTextPosX*textWidth, textWidth*2,fontHeight,foreColor,backColor);
          ++numUpdatesPerLine;
        }
        mReading2ByteCode = false;
        mTextPosX += 2;
        screenCharInd += 4;
      }
      else {
        if ((0x81 <= lineText[drawPos] && lineText[drawPos] <= 0x9f) || (0xe0 <= lineText[drawPos] && lineText[drawPos] <= 0xef)) {
          mGlyphFirstByte = lineText[drawPos];
          mReading2ByteCode = true;
        }
        else {
          if ((mTextPosX < rowChars) && (mTextPosY < endLine) && 
              ((mScreenChars[screenCharInd] != back_foreColor) ||
              (mScreenChars[screenCharInd+1] != mFontStyle) ||
              (mScreenChars[screenCharInd+2] != lineText[drawPos]))) {
            mScreenChars[screenCharInd] = back_foreColor;
            mScreenChars[screenCharInd+1] = mFontStyle;
            mScreenChars[screenCharInd+2] = lineText[drawPos];
            if (lineText[drawPos] == ' ') {
              mTextSpr[mTextPosY].fillRect(mTextPosX*textWidth,0, textWidth,fontHeight, backColor);
            }
            else {
              // mTextSpr[mTextPosY].drawXBitmap(mTextPosX*textWidth,0,mAsciiGlyphData+mAsciiGlyphBytes*lineText[drawPos],textWidth,fontHeight,foreColor,backColor);
              drawGlyphTo4bppBuffer(mAsciiGlyphData+mAsciiGlyphBytes*lineText[drawPos], (uint8_t *)mTextSprPtr[mTextPosY], mTextPosX*textWidth, textWidth,fontHeight,foreColor,backColor);
            }
            ++numUpdatesPerLine;
          }
          mTextPosX += 1;
          screenCharInd += 2;
        }
      }
      ++drawPos;
    }
    if (numUpdatesPerLine > 0) {
      // 下線が設定されていれば描画する
      if (mFontStyle & STYLE_UNDERLINED) {
        mTextSpr[mTextPosY].drawFastHLine(lineStartPosX * textWidth, textHeight-1, (mTextPosX - lineStartPosX) * textWidth, foreColor);
      }
      else if (fontHeight == 11) {
        // 中フォントのサイズは11pxなので12段目を背景色で描画する
        mTextSpr[mTextPosY].drawFastHLine(lineStartPosX * textWidth, textHeight-1, (mTextPosX - lineStartPosX) * textWidth, backColor);
      }
      // アップデート領域を設定
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

void  tftDispSPI::drawGlyphTo4bppBuffer(const uint8_t *glyphSt, uint8_t *dst, uint16_t xpos, uint16_t fontWidth, uint16_t fontHeight, uint8_t foreColor, uint8_t backColor)
{
  // uint8_t buf[32];
  // memcpy(buf, glyphSt, ((fontWidth + 7)>>3) * fontHeight);
  const uint8_t *readPtr = glyphSt;
  for (int row=0; row<fontHeight; ++row) {
    uint8_t *out = dst + ((xpos + VIEW_WIDTH * row) >> 1);
    int nibble = (xpos & 1) << 2;
    int x=0;
    int shift=0;
    uint8_t inByte;
    while (x<fontWidth) {
      if (shift == 0) {
        inByte = *(readPtr++);
      }
      uint8_t color = (inByte & 1) ? foreColor : backColor;
      *out &= 0x0f << nibble;
      *out |= color << (4 - nibble);
      nibble = (nibble + 4) & 4;
      if (nibble == 0) {
        ++out;
      }
      inByte >>= 1;
      shift = (shift + 1) & 0x07;
      ++x;
    }
  }
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
  for (int i=0; i<30; ++i) {
    if (mTextSprPtr[i] != nullptr) {
      mTextSpr[i].fillSprite(TFT_EDISP_TRANSPARENT);
    }
  }
  mReading2ByteCode = false;
  memset(mScreenChars, 0, (3 * VIEW_WIDTH * VIEW_HEIGHT) / (sTextHeight[mFontType] * sTextWidth[mFontType]));
  setUpdateArea(0, VIEW_HEIGHT);
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
			mTextColor = 9;//TFT_WHITE;
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
			mTextColor = 0;//TFT_BLACK;
			break;
		case 17:
		case 31:
			mTextColor = 2;//TFT_RED;
			break;
		case 18:
		case 32:
			mTextColor = 5;//TFT_GREEN;
			break;
		case 19:
		case 33:
			mTextColor = 4;//TFT_YELLOW;
			break;
		case 20:
		case 34:
			mTextColor = 6;//TFT_BLUE;
			break;
		case 21:
		case 35:
			mTextColor = 11;//TFT_MAGENTA;
			break;
		case 22:
		case 36:
			mTextColor = 10;//TFT_CYAN;
			break;
		case 23:
		case 37:
		case 39:
			mTextColor = 9;//TFT_WHITE;
			break;
			
		case 40:
			mTextBgColor = 0;//TFT_BLACK;
			break;
		case 41:
			mTextBgColor = 2;//TFT_RED;
			break;
		case 42:
			mTextBgColor = 5;//TFT_GREEN;
			break;
		case 43:
			mTextBgColor = 4;//TFT_YELLOW;
			break;
		case 44:
			mTextBgColor = 6;//TFT_BLUE;
			break;
		case 45:
			mTextBgColor = 11;//TFT_MAGENTA;
			break;
		case 46:
			mTextBgColor = 10;//TFT_CYAN;
			break;
		case 47:
			mTextBgColor = 9;//TFT_WHITE;
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
  mTextSpr[mTextPosY].fillRect(textWidth * mTextPosX, 0, VIEW_WIDTH - textWidth * mTextPosX, textHeight, TFT_EDISP_TRANSPARENT);
  int delSt = (sRowChars[mFontType] * mTextPosY + mTextPosX) * 3;
  int delEnd = delSt + (sRowChars[mFontType] - mTextPosX) * 3;
  if ((delSt < 0) || (delEnd >= sizeof(mScreenChars)) || (mTextPosX >= sRowChars[mFontType])) return;
  memset(&mScreenChars[delSt], 0, (sRowChars[mFontType] - mTextPosX) * 3);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
  mReading2ByteCode = false;
}

void tftDispSPI::del_row()
{
  int textHeight = sTextHeight[mFontType];
  mTextSpr[mTextPosY].fillRect(0, 0, VIEW_WIDTH, textHeight, TFT_EDISP_TRANSPARENT);
  int delSt = (sRowChars[mFontType] * mTextPosY) * 3;
  int delEnd = delSt + sRowChars[mFontType] * 3;
  if ((delSt < 0) || (delEnd >= sizeof(mScreenChars))) return;
  memset(&mScreenChars[delSt], 0, sRowChars[mFontType] * 3);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
  mReading2ByteCode = false;
}

void tftDispSPI::del(int n)
{
  int textHeight = sTextHeight[mFontType];
  int textWidth = sTextWidth[mFontType];
  mTextSpr[mTextPosY].fillRect(mTextPosX * textWidth, 0, textWidth * n, textHeight, TFT_EDISP_TRANSPARENT);
  int delSt = (sRowChars[mFontType] * mTextPosY + mTextPosX) * 3;
  int delEnd = delSt + n * 3;
  if ((delSt < 0) || (delEnd >= sizeof(mScreenChars))) return;
  memset(&mScreenChars[delSt], 0, n * 3);
  setUpdateArea(textHeight * mTextPosY, textHeight * (mTextPosY + 1));
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
    mFontType = x;
    for (int i=0; i<30; ++i) {
      if (mTextSprPtr[i] != nullptr) {
        mTextSpr[i].deleteSprite();
        mTextSprPtr[i] = nullptr;
      }
    }
    if (mTmpSprPtr != nullptr) {
      mTmpSpr.deleteSprite();
      mTmpSprPtr = nullptr;
    }
    int textHeight = sTextHeight[x];
    int textLines = VIEW_HEIGHT / textHeight;
    for (int i=0; i<textLines; ++i) {
      mTextSpr[i].setColorDepth(4);
      mTextSprPtr[i] = (uint16_t*)mTextSpr[i].createSprite(VIEW_WIDTH, textHeight);
      mTextSpr[i].fillSprite(TFT_EDISP_TRANSPARENT);
    }
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
  mReading2ByteCode = false;
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
