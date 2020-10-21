/*
 * Copyright (c) 2014, Wei Mingzhi <whistler_wmz@users.sf.net>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author and contributors may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Sprite.h"
#include <search.h>
#include <assert.h>
#include <string.h>

CSprite::CSprite(SDL_Surface *pRenderer, const char *szImageFileName, const char *szTxtFileName)
{
  int ret = hcreate(512);
  assert(ret);
  Load(pRenderer, szImageFileName, szTxtFileName);
}

CSprite::~CSprite()
{
  if (m_pTexture != NULL)
    {
      SDL_FreeSurface(m_pTexture);
    }
  hdestroy();
}

static void myBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(dst && src);

  int sx = (srcrect == NULL ? 0 : srcrect->x);
  int sy = (srcrect == NULL ? 0 : srcrect->y);
  int dx = (dstrect == NULL ? 0 : dstrect->x);
  int dy = (dstrect == NULL ? 0 : dstrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  if (sx < 0) { w += sx; dx -= sx; sx = 0; }
  if (sy < 0) { h += sy; dy -= sy; sy = 0; }
  if (dx < 0) { w += dx; sx -= dx; dx = 0; }
  if (dy < 0) { h += dy; sy -= dy; dy = 0; }
  if (sx >= src->w) return;
  if (sy >= src->h) return;
  if (dx >= dst->w) return;
  if (dy >= dst->h) return;
  if (src->w - sx < w) { w = src->w - sx; }
  if (src->h - sy < h) { h = src->h - sy; }
  if (dst->w - dx < w) { w = dst->w - dx; }
  if (dst->h - dy < h) { h = dst->h - dy; }
  if (dstrect != NULL) {
    dstrect->w = w;
    dstrect->h = h;
  }

  for (int j = 0; j < h; j ++) {
    uint32_t *pdst = (uint32_t *)dst->pixels + (dy + j) * dst->w + dx;
    uint32_t *psrc = (uint32_t *)src->pixels + (sy + j) * src->w + sx;
    for (int i = 0; i < w; i ++) {
      union {
        struct { uint8_t b, g, r, a; };
        uint32_t val;
      } pd, ps;
      pd.val = *pdst;
      ps.val = *psrc;
      if (ps.a == 0xff) {
        pd.r = ps.r;
        pd.g = ps.g;
        pd.b = ps.b;
      } else {
        pd.r += (ps.r - pd.r) * ps.a / 255;
        pd.g += (ps.g - pd.g) * ps.a / 255;
        pd.b += (ps.b - pd.b) * ps.a / 255;
      }
      *pdst = pd.val;
      pdst ++;
      psrc ++;
    }
  }
}

void CSprite::Draw(SDL_Surface *pRenderer, const char *szTag, int x, int y)
{
  ENTRY item;
  item.key = (char *)szTag;
  ENTRY *ret = hsearch(item, FIND);

  if (ret)
    {
      SDL_Rect srcrect, dstrect;
      SpritePart_t *it = (SpritePart_t *)ret->data;

      srcrect.x = it->X;
      srcrect.y = it->Y;
      srcrect.w = it->usWidth;
      srcrect.h = it->usHeight;

      dstrect.x = x;
      dstrect.y = y;
      dstrect.w = it->usWidth;
      dstrect.h = it->usHeight;

      myBlit(m_pTexture, &srcrect, pRenderer, &dstrect);
    }
}

void CSprite::DrawEx(SDL_Surface *pRenderer, const char *szTag, int x, int y, int angle)
{
  ENTRY item;
  item.key = (char *)szTag;
  ENTRY *ret = hsearch(item, FIND);

  if (ret)
    {
      SDL_Rect srcrect, dstrect;
      SpritePart_t *it = (SpritePart_t *)ret->data;

      srcrect.x = it->X;
      srcrect.y = it->Y;
      srcrect.w = it->usWidth;
      srcrect.h = it->usHeight;

      dstrect.x = x;
      dstrect.y = y;
      dstrect.w = it->usWidth;
      dstrect.h = it->usHeight;

      myBlit(m_pTexture, &srcrect, pRenderer, &dstrect);
    }
}

void CSprite::Load(SDL_Surface *pRenderer, const char *szImageFileName, const char *szTxtFileName)
{
  SDL_Surface *pSurface = IMG_Load(szImageFileName);

  if (pSurface == NULL)
    {
      fprintf(stderr, "CSprite::Load(): IMG_Load failed: %s\n", IMG_GetError());
      return;
    }

  m_iTextureWidth = pSurface->w;
  m_iTextureHeight = pSurface->h;

  SDL_PixelFormat *fmt = pSurface->format;
  SDL_PixelFormat to = *fmt;
  to.Rloss = fmt->Bloss; to.Rshift = fmt->Bshift; to.Rmask = fmt->Bmask;
  to.Bloss = fmt->Rloss; to.Bshift = fmt->Rshift; to.Bmask = fmt->Rmask;
  m_pTexture = SDL_ConvertSurface(pSurface, &to, 0);
  assert(m_pTexture);
  SDL_FreeSurface(pSurface);

  // Load txt file
  if (!LoadTxt(szTxtFileName))
    {
      SDL_FreeSurface(m_pTexture);
      m_pTexture = NULL;

      fprintf(stderr, "CSprite::Load(): LoadTxte failed\n");
      return;
    }
}

bool CSprite::LoadTxt(const char *szTxtFileName)
{
  FILE *fp = fopen(szTxtFileName, "r");

  if (fp == NULL)
    {
      return false;
    }

  while (!feof(fp))
    {
      char name[256];
      int w, h, x, y;

      if (fscanf(fp, "%s %d %d %d %d", name, &w, &h, &x, &y) != 5)
	{
	  continue;
	}

      SpritePart_t *spritePart = new SpritePart_t;

      spritePart->usWidth = w;
      spritePart->usHeight = h;
      spritePart->X = x;
      spritePart->Y = y;

      ENTRY item;
      item.key = strdup(name);
      item.data = spritePart;
      ENTRY *ret = hsearch(item, ENTER);
      assert(ret != NULL);
    }

  fclose(fp);
  return true;
}
