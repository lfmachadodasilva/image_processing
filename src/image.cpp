/**
 *   @file image.c Image: opera��es imagens. Escreve e le formato TGA.
 *
 *
 *   @author
 *         - Marcelo Gattass
 *         - Fab�ola Maffra
 *
 *   @date
 *         Criado em:      1 de Dezembro de 2002
 *         �ltima Modifica��o:   1 de maio de 2005
 *
 *   @version 3.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <math.h>
#include <float.h>
#include <memory.h>
#include <algorithm>

using namespace std;
using std::vector;

#include "image.h"

#define ROUND(_) (int)floor( (_) + 0.5 )

const int kx[3][3] ={
  { -1, -2, -1},
  { 0, 0, 0},
  { 1, 2, 1}
};

const int ky[3][3] ={
  { 1, 0, -1},
  { 2, 0, -2},
  { 1, 0, -1}
};

struct Image_imp {
  int dcs; /* define a dim do espaco de cor (dimension of the color space): 3=RGB, 1=luminancia */
  int width; /* numero de pixels na direcao horizontal da imagem */
  int height; /* numero de pixels na direcao vertical da imagem   */
  float *buf; /* vetor de dimensao dcs*width*height que armazena consecutivamente as componentes de cor */
  /* de cada pixel a partir do canto inferior esquerdo da imagem.  */
  /* A posicao das componentes de cor do pixel (x,y) fica armazenada */
  /* a partir da posicao: (y*width*dcs) + (x*dcs)  */
};


/************************************************************************/
/* Definicao das Funcoes Privadas                                       */
/************************************************************************/

/*  getuint e putuint:
 * Funcoes auxiliares para ler e escrever inteiros na ordem (lo-hi)
 * Note  que  no Windows as variaveis tipo "unsigned short int" sao
 * armazenadas  no disco em dois bytes na ordem inversa. Ou seja, o
 * numero  400, por exemplo, que  pode ser escrito como 0x190, fica
 * armazenado  em dois bytes consecutivos 0x90 e 0x01. Nos sistemas
 * UNIX e Mac este mesmo inteiro seria armazenado  na  ordem 0x01 e
 * 0x90. O armazenamento do Windows e' chamado  de  "little endian"
 * (i.e., lowest-order byte stored first), e  no  sitemas  Unix sao
 * "big-endian" (i.e., highest-order byte stored first).
 */

/***************************************************************************
 * Reads an unsigned integer from input                                     *
 ***************************************************************************/
static int getuint(unsigned short *uint, FILE *input) {
  int got;
  unsigned char temp[2];
  unsigned short tempuint;

  got = (int) fread(&temp, 1, 2, input);
  if (got != 2) return 0;

  tempuint = ((unsigned short) (temp[1]) << 8) | ((unsigned short) (temp[0]));

  *uint = tempuint;

  return 1;
}

/***************************************************************************
 * Writes an unsigned integer in output                                     *
 ***************************************************************************/
static int putuint(unsigned short uint, FILE *output) {
  int put;
  unsigned char temp[2];

  temp[0] = uint & 0xff;
  temp[1] = (uint >> 8) & 0xff;

  put = (int) fwrite(&temp, 1, 2, output);
  if (put != 2) return 0;

  return 1;
}

/***************************************************************************
 * Reads a long integer from input                                          *
 ***************************************************************************/
static int getlong(FILE *input, long int *longint) {
  int got;
  unsigned char temp[4];
  long int templongint;

  got = (int) fread(&temp, 1, 4, input);
  if (got != 4) return 0;

  templongint = ((long int) (temp[3]) << 24) | ((long int) (temp[2]) << 16)
    | ((long int) (temp[1]) << 8) | ((long int) (temp[0]));

  *longint = templongint;

  return 1;
}

/***************************************************************************
 * Writes a long integer in output                                          *
 ***************************************************************************/
static int putlong(FILE *output, long int longint) {
  int put;
  unsigned char temp[4];

  temp[0] = (unsigned char) longint & 0xff;
  temp[1] = (unsigned char) (longint >> 8) & 0xff;
  temp[2] = (unsigned char) (longint >> 16) & 0xff;
  temp[3] = (unsigned char) (longint >> 24) & 0xff;

  put = (int) fwrite(&temp, 1, 4, output);

  if (put != 4) return 0;

  return 1;
}

/***************************************************************************
 * Reads a word from input                                                  *
 ***************************************************************************/
static int getword(FILE *input, unsigned short int *word) {
  int got;
  unsigned char temp[2];
  unsigned short int tempword;

  got = (int) fread(&temp, 1, 2, input);
  if (got != 2) return 0;

  tempword = ((unsigned short int) (temp[1]) << 8) | ((unsigned short int) (temp[0]));

  *word = tempword;

  return 1;
}

/***************************************************************************
 * Writes a word in output                                                  *
 ***************************************************************************/
static int putword(FILE *output, unsigned short int word) {
  int put;
  unsigned char temp[2];

  temp[0] = word & 0xff;
  temp[1] = (word >> 8) & 0xff;

  put = (int) fwrite(&temp, 1, 2, output);
  if (put != 2) return 0;

  return 1;
}

/***************************************************************************
 * Reads a double word from input                                           *
 ***************************************************************************/
static int getdword(FILE *input, unsigned long int *dword) {
  int got;
  unsigned char temp[4];
  unsigned long int tempdword;

  got = (int) fread(&temp, 1, 4, input);
  if (got != 4) return 0;

  tempdword = ((unsigned long int) (temp[3]) << 24) | ((unsigned long int) (temp[2]) << 16)
    | ((unsigned long int) (temp[1]) << 8) | ((unsigned long int) (temp[0]));

  *dword = tempdword;

  return 1;
}

/***************************************************************************
 * Writes a double word in output                                           *
 ***************************************************************************/
static int putdword(FILE *output, unsigned long int dword) {
  int put;
  unsigned char temp[4];

  temp[0] = (unsigned char) (dword & 0xff);
  temp[1] = (unsigned char) ((dword >> 8) & 0xff);
  temp[2] = (unsigned char) ((dword >> 16) & 0xff);
  temp[3] = (unsigned char) ((dword >> 24) & 0xff);

  put = (int) fwrite(&temp, 1, 4, output);

  if (put != 4) return 0;

  return 1;
}

static float luminance(float red, float green, float blue) {
  return 0.2126f * red + 0.7152f * green + 0.0722f * blue;
}


/************************************************************************/
/* Definicao das Funcoes Exportadas                                     */

/************************************************************************/

Image* imgCreate(int w, int h, int dcs) {
  Image* image = (Image*) malloc(sizeof (Image));
  assert(image);
  image->width = w;
  image->height = h;
  image->dcs = dcs;
  image->buf = (float *) malloc(w * h * dcs * sizeof (float));
  assert(image->buf);
  return image;
}

void imgDestroy(Image* image) {
  if (image) {
    if (image->buf) free(image->buf);
    free(image);
  }
}

Image* imgCopy(Image* image) {
  int w = imgGetWidth(image);
  int h = imgGetHeight(image);
  int dcs = imgGetDimColorSpace(image);
  Image* img1 = imgCreate(w, h, dcs);
  int x, y;
  float rgb[3];

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      imgGetPixel3fv(image, x, y, rgb);
      imgSetPixel3fv(img1, x, y, rgb);
    }
  }

  return img1;
}

Image* imgGrey(Image* image) {
  int w = imgGetWidth(image);
  int h = imgGetHeight(image);
  Image* img1 = imgCreate(w, h, 1);
  int x, y;
  float rgb[3], grey[3];

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      imgGetPixel3fv(image, x, y, rgb);
      grey[0] = luminance(rgb[0], rgb[1], rgb[2]);
      grey[1] = grey[0];
      grey[2] = grey[0];
      imgSetPixel3fv(img1, x, y, grey);
    }
  }

  return img1;
}

Image* imgResize(Image* img0, int w1, int h1) {
  Image* img1 = imgCreate(w1, h1, img0->dcs);
  float w0 = (float) img0->width; /* passa para float para fazer contas */
  float h0 = (float) img0->height;

  int x0, y0, x1, y1;
  float color[3];

  for (y1 = 0; y1 < h1; y1++)
    for (x1 = 0; x1 < w1; x1++) {
      x0 = ROUND(w0 * x1 / w1); /* pega a cor do pixel mais proxima */
      y0 = ROUND(h0 * y1 / h1);
      imgGetPixel3fv(img0, x0, y0, color);
      imgGetPixel3fv(img1, x1, y1, color);
    }
  return img1;
}

Image* imgAdjust2eN(Image*img0) {
  Image* img1;
  int i = 0, valid[14] = {1, 2, 4, 8, 16, 32, 64, 128, 512, 1024, 2048, 4096, 8192, 16384};
  int w0 = img0->width;
  int h0 = img0->height;
  int w1, h1;
  int x, y;
  float rgb[3], black[3] = {0.f, 0.f, 0.f};

  for (i = 0; i < 14 && valid[i] < w0; i++);
  w1 = valid[i];

  for (i = 0; i < 14 && valid[i] < h0; i++);
  h1 = valid[i];

  img1 = imgCreate(w1, h1, img0->dcs);

  for (y = 0; y < h1; y++) {
    for (x = 0; x < w1; x++) {
      if (x < w0 && y < h0) {
        imgGetPixel3fv(img0, x, y, rgb);
        imgSetPixel3fv(img1, x, y, rgb);
      } else
        imgSetPixel3fv(img1, x, y, black);
    }
  }

  return img1;
}

float imgDif(Image*img0, Image*img1, float gamma) {
  int w = imgGetWidth(img0);
  int h = imgGetHeight(img0);
  int x, y;
  float rgb0[3], rgb1[3], avg = 0.f;

  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      float luminance, new_luminance, ratio;

      imgGetPixel3fv(img0, x, y, rgb0);
      imgGetPixel3fv(img1, x, y, rgb1);

      /* calcula o modulo da diferenca */
      rgb0[0] = (rgb1[0] > rgb0[0]) ? rgb1[0] - rgb0[0] : rgb0[0] - rgb1[0];
      rgb0[1] = (rgb1[1] > rgb0[1]) ? rgb1[1] - rgb0[1] : rgb0[1] - rgb1[1];
      rgb0[2] = (rgb1[2] > rgb0[2]) ? rgb1[2] - rgb0[2] : rgb0[2] - rgb1[2];

      /* acumula a soma */
      avg += (rgb0[0] + rgb0[1] + rgb0[2]);

      /* corrige por gamma */
      luminance = 0.299f * rgb0[0] + 0.587f * rgb0[1] + 0.144f * rgb0[2];
      new_luminance = (float) pow(luminance, 1.0f / gamma);
      ratio = new_luminance / luminance;
      rgb0[0] *= ratio;
      rgb0[1] *= ratio;
      rgb0[2] *= ratio;

      imgSetPixel3fv(img0, x, y, rgb0);
    }
  }
  avg = avg / (3 * w * h);
  return avg;
}

int imgGetWidth(Image* image) {
  return image->width;
}

int imgGetHeight(Image* image) {
  return image->height;
}

int imgGetDimColorSpace(Image* image) {
  return image->dcs;
}

float* imgGetData(Image* image) {
  return image->buf;
}

void imgSetPixel3fv(Image* image, int x, int y, float* color) {
  int pos = (y * image->width * image->dcs) + (x * image->dcs);
  switch (image->dcs) {
    case 3:
      image->buf[pos ] = color[0];
      image->buf[pos + 1] = color[1];
      image->buf[pos + 2] = color[2];
      break;
    case 1:
      image->buf[pos ] = luminance(color[0], color[1], color[2]);
      break;
    default:
      break;
  }
}

void imgSetPixel3fcolor(Image* image, int x, int y, cColor<float> color) {
  int pos = (y * image->width * image->dcs) + (x * image->dcs);
  switch (image->dcs) {
    case 3:
      image->buf[pos ] = color.r;
      image->buf[pos + 1] = color.g;
      image->buf[pos + 2] = color.b;
      break;
    case 1:
      image->buf[pos ] = luminance(color.r, color.g, color.b);
      break;
    default:
      break;
  }
}

void imgGetPixel3fv(Image* image, int x, int y, float* color) {
  int pos = (y * image->width * image->dcs) + (x * image->dcs);
  switch (image->dcs) {
    case 3:
      color[0] = image->buf[pos ];
      color[1] = image->buf[pos + 1];
      color[2] = image->buf[pos + 2];
      break;
    case 1:
      color[0] = image->buf[pos ];
      color[1] = color[0];
      color[2] = color[0];
      break;
    default:
      break;
  }
}

cColor<float> imgGetPixel3fcolor(Image* image, int x, int y) {
  cColor<float> color;
  int pos = (y * image->width * image->dcs) + (x * image->dcs);
  switch (image->dcs) {
    case 3:
      color.r = image->buf[pos ];
      color.g = image->buf[pos + 1];
      color.b = image->buf[pos + 2];
      break;
    case 1:
      color.r = image->buf[pos ];
      color.g = color.r;
      color.b = color.r;
      break;
    default:
      break;
  }

  return color;
}

void imgSetPixel3ubv(Image* image, int x, int y, unsigned char * color) {
  int pos = (y * image->width * image->dcs) + (x * image->dcs);
  switch (image->dcs) {
    case 3:
      image->buf[pos ] = (float) (color[0] / 255.);
      image->buf[pos + 1] = (float) (color[1] / 255.);
      image->buf[pos + 2] = (float) (color[2] / 255.);
      break;
    case 1:
      image->buf[pos ] = luminance((float) (color[0] / 255.), (float) (color[1] / 255.), (float) (color[2] / 255.));
      break;
    default:
      break;
  }
}

void imgGetPixel3ubv(Image* image, int x, int y, unsigned char *color) {
  int pos = (y * image->width * image->dcs) + (x * image->dcs);
  int r, g, b;
  switch (image->dcs) {
    case 3:
      r = ROUND(255 * image->buf[pos]);
      g = ROUND(255 * image->buf[pos + 1]);
      b = ROUND(255 * image->buf[pos + 2]);
      color[0] = (unsigned char) (r < 256) ? r : 255;
      color[1] = (unsigned char) (g < 256) ? g : 255;
      color[2] = (unsigned char) (b < 256) ? b : 255;
      break;
    case 1:
      r = g = b = ROUND(255 * image->buf[pos]);
      color[0] = (unsigned char) (r < 256) ? r : 255;
      color[1] = (unsigned char) (g < 256) ? g : 255;
      color[2] = (unsigned char) (b < 256) ? b : 255;
      break;
    default:
      break;
  }
}

Image* imgReadTGA(char *filename) {
  FILE *filePtr;

  Image *image; /* imagem a ser criada */
  unsigned char *buffer; /* buffer para ler o vetor de rgb da imagem  */

  unsigned char imageType; /* 2 para imagens RGB */
  unsigned short int imageWidth; /* largura da imagem  */
  unsigned short int imageHeight; /* altura da imagem   */
  unsigned char bitCount; /* numero de bits por pixel */

  int x, y; /* posicao de um pixel  */

  unsigned char ucharSkip; /* dado lixo unsigned char */
  short int sintSkip; /* dado lixo short int */

  /* abre o arquivo com a imagem TGA */
  filePtr = fopen(filename, "rb");
  assert(filePtr);

  /* pula os primeiros dois bytes que devem ter valor zero */
  ucharSkip = getc(filePtr); /* tamanho do descritor da imagem (0) */
  if (ucharSkip != 0) printf("erro na leitura de %s: imagem com descritor\n", filename);

  ucharSkip = getc(filePtr);
  if (ucharSkip != 0) printf("erro na leitura de %s: imagem com tabela de cores\n", filename);

  /* le o tipo de imagem (que deve ser obrigatoriamente 2).
  nao estamos tratando dos outros tipos */
  imageType = getc(filePtr);
  assert(imageType == 2);

  /* pula 9 bytes relacionados com a tabela de cores
  (que nao existe quando a imagem e' RGB, imageType=2) */
  getuint((short unsigned int *) &sintSkip, filePtr);
  getuint((short unsigned int *) &sintSkip, filePtr);
  ucharSkip = getc(filePtr);

  /* especificacao da imagem */
  getuint((short unsigned int *) &sintSkip, filePtr); /* origem em x (por default = 0) */
  getuint((short unsigned int *) &sintSkip, filePtr); /* origem em y (por default = 0) */
  getuint(&imageWidth, filePtr); /* largura */
  getuint(&imageHeight, filePtr); /* altura  */

  /* read image bit depth */
  bitCount = getc(filePtr);
  assert(bitCount == 24); /* colorMode -> 3 = BGR (24 bits) */

  /* read 1 byte of garbage data */
  ucharSkip = getc(filePtr);

  /* cria uma instancia do tipo Imagem */
  image = imgCreate(imageWidth, imageHeight, 3);
  buffer = (unsigned char *) malloc(3 * imageWidth * imageHeight * sizeof (unsigned char));
  assert(image);
  assert(buffer);

  /* read in image data */
  fread(buffer, sizeof (unsigned char), 3 * imageWidth*imageHeight, filePtr);

  /* copia e troca as compontes de BGR para RGB */
  for (y = 0; y < imageHeight; y++) {
    for (x = 0; x < imageWidth; x++) {
      unsigned char color[3];
      int pos = (y * imageWidth * 3) + (x * 3);
      color[0] = buffer[pos + 2];
      color[1] = buffer[pos + 1];
      color[2] = buffer[pos ];
      imgSetPixel3ubv(image, x, y, color);
    }
  }
  free(buffer);
  fclose(filePtr);
  return image;
}

int imgWriteTGA(char *filename, Image* image) {
  unsigned char imageType = 2; /* RGB(A) sem compress�o */
  unsigned char bitDepth = 24; /* 24 bits por pixel     */

  FILE *filePtr; /* ponteiro do arquivo    */
  unsigned char * buffer; /* buffer de bytes        */
  int x, y;

  unsigned char byteZero = 0; /* usado para escrever um byte zero no arquivo      */
  short int shortZero = 0; /* usado para escrever um short int zero no arquivo */

  if (!image) return 0;

  /* cria um arquivo binario novo */
  filePtr = fopen(filename, "wb");
  assert(filePtr);

  /* cria o buffer */
  buffer = (unsigned char *) malloc(3 * image->width * image->height * sizeof (unsigned char));
  assert(buffer);

  /* copia e troca as compontes de BGR para RGB */
  for (y = 0; y < image->height; y++) {
    for (x = 0; x < image->width; x++) {
      unsigned char color[3];
      int pos = (y * image->width * 3) + (x * 3);
      imgGetPixel3ubv(image, x, y, color);
      buffer[pos + 2] = color[0];
      buffer[pos + 1] = color[1];
      buffer[pos ] = color[2];
    }
  }

  /* escreve o cabecalho */
  putc(byteZero, filePtr); /* 0, no. de caracteres no campo de id da imagem     */
  putc(byteZero, filePtr); /* = 0, imagem nao tem palheta de cores              */
  putc(imageType, filePtr); /* = 2 -> imagem "true color" (RGB)                  */
  putuint(shortZero, filePtr); /* info sobre a tabela de cores (inexistente)        */
  putuint(shortZero, filePtr); /* idem                                              */
  putc(byteZero, filePtr); /* idem                                              */
  putuint(shortZero, filePtr); /* =0 origem em x                                    */
  putuint(shortZero, filePtr); /* =0 origem em y                                    */
  putuint(image->width, filePtr); /* largura da imagem em pixels                       */
  putuint(image->height, filePtr); /* altura da imagem em pixels                        */
  putc(bitDepth, filePtr); /* numero de bits de um pixel                        */
  putc(byteZero, filePtr); /* =0 origem no canto inf esquedo sem entrelacamento */

  /* escreve o buf de cores da imagem */
  fwrite(buffer, sizeof (unsigned char), 3 * image->width * image->height, filePtr);

  free(buffer);
  fclose(filePtr);
  return 1;
}


/* Compiler dependent definitions */
typedef unsigned char BYTE;
typedef unsigned short int USHORT;
typedef unsigned short int WORD;
typedef long int LONG;
typedef unsigned long int DWORD;

Image* imgReadBMP(char *filename) {
  FILE *filePtr; /* ponteiro do arquivo */
  Image*image; /* imagem a ser criada */
  BYTE *linedata;

  USHORT bfType; /* "BM" = 19788           */
  LONG biWidth; /* image width in pixels  */
  LONG biHeight; /* image height in pixels */
  WORD biBitCount; /* bitmap color depth     */
  DWORD bfSize;

  USHORT ushortSkip; /* dado lixo USHORT */
  DWORD dwordSkip; /* dado lixo DWORD  */
  LONG longSkip; /* dado lixo LONG   */
  WORD wordSkip; /* dado lixo WORD   */

  LONG i, j, k, l, linesize, got;

  /* abre o arquivo com a imagem BMP */
  filePtr = fopen(filename, "rb");
  assert(filePtr);

  /* verifica se eh uma imagem bmp */
  getuint(&bfType, filePtr);
  assert(bfType == 19778);

  /* pula os 12 bytes correspondentes a bfSize, Reserved1 e Reserved2 */
  getdword(filePtr, &bfSize);
  getuint(&ushortSkip, filePtr); /* Reserved1, deve ter valor 0 */
  assert(ushortSkip == 0);
  getuint(&ushortSkip, filePtr); /* Reserved2, deve ter valor 0 */
  assert(ushortSkip == 0);

  /* pula os 4 bytes correspondentes a bfOffBits, que deve ter valor 54 */
  getdword(filePtr, &dwordSkip);
  assert(dwordSkip == 54);

  /* pula os 4 bytes correspondentes a biSize, que deve ter valor 40 */
  getdword(filePtr, &dwordSkip);
  assert(dwordSkip == 40);

  /* pega largura e altura da imagem */
  getlong(filePtr, &biWidth);
  getlong(filePtr, &biHeight);

  /* verifica que o numero de quadros eh igual a 1 */
  getword(filePtr, &wordSkip);
  assert(wordSkip == 1);

  /* Verifica se a imagem eh de 24 bits */
  getword(filePtr, &biBitCount);
  if (biBitCount != 24) {
    fprintf(stderr, "imgReadBMP: Not a bitmap 24 bits file.\n");
    fclose(filePtr);
    return (NULL);
  }

  /* pula os demais bytes do infoheader */
  getdword(filePtr, &dwordSkip);
  assert(dwordSkip == 0);
  getdword(filePtr, &dwordSkip);
  getlong(filePtr, &longSkip);
  getlong(filePtr, &longSkip);
  getdword(filePtr, &dwordSkip);
  getdword(filePtr, &dwordSkip);

  image = imgCreate(biWidth, biHeight, 3);

  /* a linha deve terminar em uma fronteira de dword */
  linesize = 3 * image->width;
  if (linesize & 3) {
    linesize |= 3;
    linesize++;
  }

  /* aloca espaco para a area de trabalho */
  linedata = (BYTE *) malloc(linesize);
  if (linedata == NULL) {
    fprintf(stderr, "get24bits: Not enough memory.\n");
    return 0;
  }

  /* pega as componentes de cada pixel */
  for (k = 0, i = 0; i < image->height; i++) {
    got = (unsigned long int) fread(linedata, linesize, 1, filePtr);
    if (got != 1) {
      free(linedata);
      fprintf(stderr, "get24bits: Unexpected end of file.\n");
    }
    for (l = 1, j = 0; j < image->width; j++, l = l + 3) {
      image->buf[k++] = (float) (linedata[l + 1] / 255.0f);
      image->buf[k++] = (float) (linedata[l ] / 255.0f);
      image->buf[k++] = (float) (linedata[l - 1] / 255.0f);

      //printf("%f\t%f\t%f\n", image->buf[k - 3], image->buf[k - 2], image->buf[k - 1]);
    }
  }

  free(linedata);
  fclose(filePtr);
  return image;
}

int imgWriteBMP(char *filename, Image* bmp) {
  FILE *filePtr; /* ponteiro do arquivo */
  unsigned char *filedata;
  DWORD bfSize;
  int i, k, l;

  int linesize, put;

  if (!bmp) return 0;

  /* cria um novo arquivo binario */
  filePtr = fopen(filename, "wb");
  assert(filePtr);

  /* a linha deve terminar em uma double word boundary */
  linesize = bmp->width * 3;
  if (linesize & 3) {
    linesize |= 3;
    linesize++;
  }

  /* calcula o tamanho do arquivo em bytes */
  bfSize = 14 + /* file header size */
    40 + /* info header size */
    bmp->height * linesize; /* image data  size */

  /* Preenche o cabe�alho -> FileHeader e InfoHeader */
  putuint(19778, filePtr); /* type = "BM" = 19788                         */
  putdword(filePtr, bfSize); /* bfSize -> file size in bytes                   */
  putuint(0, filePtr); /* bfReserved1, must be zero                   */
  putuint(0, filePtr); /* bfReserved2, must be zero                   */
  putdword(filePtr, 54); /* bfOffBits -> offset in bits to data             */

  putdword(filePtr, 40); /* biSize -> structure size in bytes             */
  putlong(filePtr, bmp->width); /* biWidth -> image width in pixels                */
  putlong(filePtr, bmp->height); /* biHeight -> image height in pixels             */
  putword(filePtr, 1); /* biPlanes, must be 1                         */
  putword(filePtr, 24); /* biBitCount, 24 para 24 bits -> bitmap color depth */
  putdword(filePtr, 0); /* biCompression, compression type -> no compression */
  putdword(filePtr, 0); /* biSizeImage, nao eh usado sem compressao          */
  putlong(filePtr, 0); /* biXPelsPerMeter                            */
  putlong(filePtr, 0); /* biYPelsPerMeter                            */
  putdword(filePtr, 0); /* biClrUsed, numero de cores na palheta          */
  putdword(filePtr, 0); /* biClrImportant, 0 pq todas sao importantes       */

  /* aloca espacco para a area de trabalho */
  filedata = (unsigned char *) malloc(linesize);
  assert(filedata);

  /* a linha deve ser zero padded */
  for (i = 0; i < (linesize - (3 * bmp->width)); i++)
    filedata[linesize - 1 - i] = 0;

  for (k = 0; k < bmp->height; k++) {
    l = 1;
    /* coloca as componentes BGR no buffer */
    for (i = 0; i < bmp->width; i++) {
      unsigned char color[3];
      int r, g, b;
      imgGetPixel3ubv(bmp, i, k, color);
      r = color[0];
      g = color[1];
      b = color[2];
      filedata[l - 1] = (unsigned char) (b < 256) ? b : 255;
      filedata[l ] = (unsigned char) (g < 256) ? g : 255;
      filedata[l + 1] = (unsigned char) (r < 256) ? r : 255;
      l += 3;
    }

    /* joga para o arquivo */
    put = (int) fwrite(filedata, linesize, 1, filePtr);
    if (put != 1) {
      fprintf(stderr, "put24bits: Disk full.");
      free(filedata);
      return 0;
    }
  }

  /* operacao executada com sucesso */
  fprintf(stdout, "imgWriteBMP: %s successfuly generated\n", filename);
  free(filedata);
  fclose(filePtr);
  return 1;
}

/*- PFM Interface Functions  ---------------------------------------*/

Image* imgReadPFM(char *filename) {
  FILE *fp;
  Image* img;
  float scale;
  int w, h;

  char line[256];

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    printf("%s nao pode ser aberto\n", filename);
    return NULL;
  }

  fgets(line, 256, fp);

  if (strcmp(line, "PF\n")) {
    return 0;
  }

  while (fscanf(fp, " %d ", &w) != 1)
    fgets(line, 256, fp);
  while (fscanf(fp, " %d ", &h) != 1)
    fgets(line, 256, fp);
  while (fscanf(fp, " %f", &scale) != 1)
    fgets(line, 256, fp);

  fgetc(fp);

  img = imgCreate(w, h, 3);
  fread(img->buf, 3 * w*h, sizeof (float), fp);

  fprintf(stdout, "imgReadPFM: %s successfuly loaded\n", filename);
  fclose(fp);
  return img;
}

int imgWritePFM(char * filename, Image* img) {
  FILE * fp;
  float scale = 1.f;

  if ((fp = fopen(filename, "wb")) == NULL) {
    printf("\nN�o foi possivel abrir o arquivo %s\n", filename);
    return 0;
  }

  /* the ppm file header */
  fprintf(fp, "PF\n%d %d\n%f\n", img->width, img->height, scale);

  fwrite(img->buf, 3 * img->width * img->height, sizeof (float), fp);

  fprintf(stdout, "imgWritePFM: %s successfuly created\n", filename);
  fclose(fp);
  return 1;
}

static int comparaCor3(const void * p1, const void * p2) {
  int *c1 = (int *) p1; /* aponta para o vermelho quantizado da cor 1 */
  int *c2 = (int *) p2; /* aponta para o vermelho quantizado da cor 2 */

  /* compara o canal vermelho */
  if (*c1 < *c2) return -1;
  if (*c1 > *c2) return 1;

  /* compara  o canal verde, uma vez que o vermelho e' igual */
  c1++;
  c2++;
  if (*c1 < *c2) return -1;
  if (*c1 > *c2) return 1;

  /* compara o canal azul, uma vez que o vermelho e o azul sao iguais */
  c1++;
  c2++;
  if (*c1 < *c2) return -1;
  if (*c1 > *c2) return 1;

  /* sao iguais */
  return 0;
}

static int comparaCor1(const void * p1, const void * p2) {
  int *c1 = (int *) p1; /* aponta para a luminosidade quantizada da cor 1 */
  int *c2 = (int *) p2; /* aponta para a luminosidade quantizada da cor 2 */

  /* compara o canal de luminosidade */
  if (*c1 < *c2) return -1;
  if (*c1 > *c2) return 1;

  /* sao iguais */
  return 0;
}

int imgCountColor(Image * img, float tol) {
  int numCor = 1;
  int w = imgGetWidth(img);
  int h = imgGetHeight(img);
  int dcs = imgGetDimColorSpace(img);
  float* buf = imgGetData(img);
  int *vet = (int*) malloc(3 * w * h * sizeof (int));
  int i;


  /* copia o buffer da imagem no vetor de floats fazendo
  uma quantizacao para (1/tol) tons de cada componente de cor */
  for (i = 0; i < dcs * w * h; i++)
    vet[i] = (int) (buf[i] / tol + 0.5);

  /* ordena o vetor */
  if (dcs == 3)
    qsort(vet, w * h, 3 * sizeof (int), comparaCor3);
  else
    qsort(vet, w*h, sizeof (int), comparaCor1);



  /* conta o numero de cores diferentes */
  if (dcs == 3) {
    int freq = 1;
    for (i = 3; i < 3 * w * h; i += 3) {
      freq++;
      if (comparaCor3(&vet[i - 3], &vet[i]) != 0) {
        freq = 1;
        numCor++;
      }
    }
  } else {
    for (i = 1; i < w * h; i += 1)
      if (comparaCor1(&vet[i - 1], &vet[i]) != 0) numCor++;
  }


  free(vet);
  return numCor;
}



/***************************************************************************
 * Funcoes do T1                                             *
 ***************************************************************************/
/* substitua este string com os nomes dos componentes do seu grupo */
char grupo[] = "0712196 Luiz Felipe Machado da Silva\n";

void imgGauss(Image* img_dst, Image* img_src) {
  cColor<float> color;
  int w = imgGetWidth(img_src);
  int h = imgGetHeight(img_src);

  for (int i = 1; i < w - 1; i++) {
    for (int j = 1; j < h - 1; j++) {
      color += imgGetPixel3fcolor(img_src, i, j) * 4;

      color += imgGetPixel3fcolor(img_src, i + 1, j) * 2;
      color += imgGetPixel3fcolor(img_src, i - 1, j) * 2;
      color += imgGetPixel3fcolor(img_src, i, j + 1) * 2;
      color += imgGetPixel3fcolor(img_src, i, j - 1) * 2;

      color += imgGetPixel3fcolor(img_src, i - 1, j - 1);
      color += imgGetPixel3fcolor(img_src, i + 1, j - 1);
      color += imgGetPixel3fcolor(img_src, i + 1, j + 1);
      color += imgGetPixel3fcolor(img_src, i - 1, j + 1);

      color /= 16;

      imgSetPixel3fcolor(img_dst, i, j, color);
    }
  }
}

void imgMedian(Image* image) {
  Image* img = imgCopy(image);

  int w = imgGetWidth(image);
  int h = imgGetHeight(image);

  vector<float> r, g, b;
  cColor<float> color;

  for (int x = 1; x < w - 1; x++) {
    for (int y = 1; y < h - 1; y++) {

      color = imgGetPixel3fcolor(img, x - 1, y - 1);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);
      color = imgGetPixel3fcolor(img, x, y - 1);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);
      color = imgGetPixel3fcolor(img, x + 1, y - 1);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);

      color = imgGetPixel3fcolor(img, x - 1, y);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);
      color = imgGetPixel3fcolor(img, x, y);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);
      color = imgGetPixel3fcolor(img, x + 1, y);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);

      color = imgGetPixel3fcolor(img, x - 1, y + 1);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);
      color = imgGetPixel3fcolor(img, x, y + 1);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);
      color = imgGetPixel3fcolor(img, x + 1, y + 1);
      r.push_back(color.r);
      g.push_back(color.g);
      b.push_back(color.b);

      sort(r.begin(), r.end());
      sort(g.begin(), g.end());
      sort(b.begin(), b.end());

      color.r = r[4];
      color.g = g[4];
      color.b = b[4];

      r.clear();
      g.clear();
      b.clear();

      imgSetPixel3fcolor(image, x, y, color);

    }
  }

  imgDestroy(img);
}

static float convolve(Image *img, int x, int y, const int k[3][3]) {
  float r = 0;
  cColor<float> color;
  for (int xx = -1; xx <= 1; xx++) {
    for (int yy = -1; yy <= 1; yy++) {
      color = imgGetPixel3fcolor(img, x + xx, y + yy);
      r += luminance(color.r, color.g, color.b) * k[xx + 1][yy + 1];
    }
  }

  return r;
}

Image* imgEdges(Image* imgIn) {
  int w = imgGetWidth(imgIn);
  int h = imgGetHeight(imgIn);

  Image* imgOut = imgCreate(w, h, 1);

  cColor<float> color;
  float max = 0.0f, val = 0.0f;

  for (int x = 0; x < w; x++) {
    for (int y = 0; y < h; y++) {
      if (x == 0 || y == 0 || x == w - 1 || y == h - 1) {
        float v[3] = {0.0f, 0.0f, 0.0f};
        imgSetPixel3fv(imgOut, x, y, v);
      } else {
        float dx = convolve(imgIn, x, y, kx);
        float dy = convolve(imgIn, x, y, ky);
        val = sqrt(dx * dx + dy * dy);
        max = (max > val) ? max : val;
        imgOut->buf[y * w + x] = val;
      }
    }
  }

  return imgOut;
}

// implementacao da reducao de cores por median cut

static int comparaR(const void *p1, const void *p2) {
  cColor<float> *color1 = (cColor<float>*) p1;
  cColor<float> *color2 = (cColor<float>*) p2;

  if (color1->r < color2->r) return -1;
  if (color1->r > color2->r) return 1;

  return 0;
}

static int comparaG(const void *p1, const void *p2) {
  cColor<float> *color1 = (cColor<float> *) p1;
  cColor<float> *color2 = (cColor<float> *) p2;

  if (color1->g < color2->g) return -1;
  if (color1->g > color2->g) return 1;

  return 0;
}

static int comparaB(const void *p1, const void *p2) {
  cColor<float> *color1 = (cColor<float> *) p1;
  cColor<float> *color2 = (cColor<float> *) p2;

  if (color1->b < color2->b) return -1;
  if (color1->b > color2->b) return 1;

  return 0;
}

static void caixaEnvolvente(colorCube *cube, cColor<float> *colorVec) {
  float r, g, b;

  // inicializa com o pior caso
  cube->min.r = cube->min.g = cube->min.b = 1.0;
  cube->max.r = cube->max.g = cube->max.b = 0.0;

  // percorre o cubo ajustando o dom�nio das cores
  for (int i = cube->ini; i <= cube->fim; i++) {
    r = colorVec[i].r;
    if (r > cube->max.r) cube->max.r = r;
    if (r < cube->min.r) cube->min.r = r;
    g = colorVec[i].g;
    if (g > cube->max.g) cube->max.g = g;
    if (g < cube->min.g) cube->min.g = g;
    b = colorVec[i].b;
    if (b > cube->max.b) cube->max.b = b;
    if (b < cube->min.b) cube->min.b = b;
  }
}

static Image* bestColor(Image *img0, cColor<float>* pal, int pal_size) {
  int w = imgGetWidth(img0);
  int h = imgGetHeight(img0);

  Image *img1 = imgCreate(w, h, 3);

  cColor<float> color0, color1, color_dif;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int i_menor;
      float m_menor;

      color0 = imgGetPixel3fcolor(img0, x, y);

      m_menor = FLT_MAX;
      i_menor = 0;

      for (int i = 0; i < pal_size; i++) {
        color_dif = color0 - pal[i];

        float m = (float) ((color_dif.r * color_dif.r) +
          (color_dif.g * color_dif.g) +
          (color_dif.b * color_dif.b));

        if (m < m_menor) {
          i_menor = i;
          m_menor = m;

          color1 = pal[i_menor];
        }
      }

      imgSetPixel3fcolor(img1, x, y, color1);
    }
  }

  return img1;
}

static void paleta(cColor<float> *pal, colorCube *cubeVec, cColor<float> *colorVec, int numCubos) {
  int count;
  cColor<float> color;

  for (int i = 0; i < numCubos; i++) {
    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;

    count = 0;

    for (int j = cubeVec[i].ini; j <= cubeVec[i].fim; j++) {
      color += colorVec[j];
      count++;
    }

    pal[i] = color / (float) count;
  }
}

static void ordenaMaiorDim(colorCube *cubeVec, cColor<float> *colorVec, int i) {
  size_t numElem;
  size_t tamElem;
  cColor<float> var;

  numElem = (size_t) (cubeVec[i].fim - cubeVec[i].ini + 1);
  tamElem = (size_t) (sizeof (cColor<float>));

  var = cubeVec[i].max - cubeVec[i].min;

  // procura a maior dimensao em um cubo
  if ((var.r >= var.g) && (var.r >= var.b)) cubeVec[i].maiorDim = 0;
  else
    if ((var.g >= var.r) && (var.g >= var.b)) cubeVec[i].maiorDim = 1;
  else
    if ((var.b > var.r) && (var.b > var.g)) cubeVec[i].maiorDim = 2;

  // ordena de acordo com a maior dimensao da caixa
  if (cubeVec[i].maiorDim == 0) {
    qsort(&colorVec[cubeVec[i].ini], numElem, tamElem, comparaR);
    cubeVec[i].var = var.r;
  }
  if (cubeVec[i].maiorDim == 1) {
    qsort(&colorVec[cubeVec[i].ini], numElem, tamElem, comparaG);
    cubeVec[i].var = var.g;
  }
  if (cubeVec[i].maiorDim == 2) {
    qsort(&colorVec[cubeVec[i].ini], numElem, tamElem, comparaB);
    cubeVec[i].var = var.b;
  }
}

static void cortaCubo(colorCube *cubeVec, int posCorte, int numCubos) {
  // divide o cubo
  int ini = cubeVec[posCorte].ini;
  int fim = cubeVec[posCorte].fim;

  if ((fim - ini) % 2 != 0) // numero par de elementos
  {
    cubeVec[numCubos].ini = ini + (fim - ini + 1) / 2;
    cubeVec[numCubos].fim = fim;
    cubeVec[posCorte].fim = cubeVec[numCubos].ini - 1;
  } else // numero impar de elementos
  {
    cubeVec[numCubos].ini = ini + (fim - ini) / 2;
    cubeVec[numCubos].fim = fim;
    cubeVec[posCorte].fim = cubeVec[numCubos].ini - 1;
  }
}

static int cuboCorte(colorCube *cubeVec, int numCubos) {
  // escolhe o cubo a ser cortado
  float maiorVar = -1;
  int posCorte = -1;
  for (int k = 0; k < numCubos; k++) {
    // cubo com uma unica cor
    if (cubeVec[k].max == cubeVec[k].min)
      continue;

    if (cubeVec[k].var > maiorVar) {
      maiorVar = cubeVec[k].var;
      posCorte = k;
    }
  }

  return posCorte;
}

Image* imgReduceColors(Image *img0, int maxCores) {
  int w = imgGetWidth(img0);
  int h = imgGetHeight(img0);

  int numCubos = 0;
  int posCorte = -1;

  colorCube* cubeVec = (colorCube*) malloc(maxCores * sizeof (colorCube));
  cColor<float>* colorVec = (cColor<float>*)malloc(w * h * sizeof (cColor<float>));
  cColor<float>* pal = (cColor<float>*)malloc(maxCores * sizeof (cColor<float>));

  // guarda as cores nos vetores (com repeticao)
  int i = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      colorVec[i] = imgGetPixel3fcolor(img0, x, y);
      i++;
    }
  }

  // cria o cubo inicial
  cubeVec[0].checked = 0;
  cubeVec[0].fim = w * h - 1;
  cubeVec[0].ini = 0;

  numCubos = 1;

  for (int j = 0; j < maxCores - 1; j++) {
    for (i = 0; i < numCubos; i++) {
      if (cubeVec[i].checked) continue;

      // calcula os maximos e os minimos
      caixaEnvolvente(&cubeVec[i], colorVec);

      // ordena de acordo com a maior dimensao
      ordenaMaiorDim(cubeVec, colorVec, i);

      cubeVec[i].checked = 1;
    }

    // escolhe o cubo a ser cortado
    posCorte = cuboCorte(cubeVec, numCubos);

    if (posCorte == -1) break;

    // divide o cubo
    cortaCubo(cubeVec, posCorte, numCubos);

    cubeVec[numCubos].checked = 0;
    cubeVec[posCorte].checked = 0;

    numCubos++;
  }

  // cria a paleta de cores
  paleta(pal, cubeVec, colorVec, numCubos);

  Image *img1 = imgCreate(w, h, 3);
  // preenche a imagem com as cores da paleta
  img1 = bestColor(img0, &pal[0], maxCores);

  free(colorVec);
  free(cubeVec);
  free(pal);

  return img1;
}
