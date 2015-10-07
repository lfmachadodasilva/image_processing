/**
 *	@file image.c Image: operações imagens. Escreve e le formato TGA e BMP.
 *
 *
 *	@author 
 *			- Marcelo Gattass
 *			- Maira Noronha
 *			- Thiago Bastos
 *			- Fabíola Maffra
 *
 *	@date
 *			Criado em:		 1 de Dezembro de 2002
 *			Última Modificação:	24 de Agosto de 2010
 *
 *	@version 3.0
 */

#ifndef IMAGE_H
#define IMAGE_H

template<class T> class cColor
{
 public:
  T r, g, b;

  cColor(T R = 0, T G = 0, T B = 0) : r(R), g(G), b(B) {}
  cColor(const cColor<T>& color) {*this = color;}

  void operator=(const cColor<T>& color)
  {r = color.r; g = color.g; b = color.b;}
  void operator+=(const cColor<T>& color)
  {r += color.r; g += color.g; b += color.b;}
  void operator-=(const cColor<T>& color)
  {r -= color.r; g -= color.g; b -= color.b;}
  void operator*=(float v)
  {r *= v; g *= v; b *= v;}
  void operator/=(float v)
  {r /= v; g /= v; b /= v;}

  void to_vec(T *v)
  {
   v[0] = r;
   v[1] = g;
   v[2] = b;
  }

  friend cColor<T> operator+(const cColor<T>& a, const cColor<T>& b)
  {cColor<T> c; c.r = a.r + b.r; c.g = a.g + b.g; c.b = a.b + b.b; return c;}
  friend cColor<T> operator-(const cColor<T>& a, const cColor<T>& b)
  {cColor<T> c; c.r = a.r - b.r; c.g = a.g - b.g; c.b = a.b - b.b; return c;}
  friend cColor<T> operator*(const cColor<T>& v, T c)
  {cColor<T> w; w.r = v.r * c; w.g = v.g * c; w.b = v.b * c; return w;}
	friend cColor<T> operator/(const cColor<T>& v, T c)
  {cColor<T> w; w.r = v.r / c; w.g = v.g / c; w.b = v.b / c; return w;}
  friend bool operator==(const cColor<T>& a, const cColor<T>& b)
  {return a.r == b.r && a.g == b.g && a.b == b.b;}
};

struct colorCube
{
	cColor<float> max, min;      
	float var;	 
	int ini, fim, maiorDim, checked;			
};


/************************************************************************/
/* Tipos Exportados                                                     */
/************************************************************************/
/**
 *   Imagem com um buffer rgb.
 */

typedef struct Image_imp Image;


/************************************************************************/
/* Funcoes Exportadas                                                   */
/************************************************************************/

/**
 *	Cria uma nova imagem com as dimensoes especificadas.
 *
 *	@param w Largura da imagem.
 *	@param h Altura da imagem.
 *	@param dcs Dimensao do espaco de cor de cada pixel (1=luminancia ou 3=RGB).
 *
 *	@return Handle da imagem criada.
 */
Image  * imgCreate (int w, int h, int dcs);

/**
 *	Destroi a imagem.
 *
 *	@param image imagem a ser destruida.
 */
void    imgDestroy (Image*image);

/**
 *	Cria uma nova nova copia imagem dada.
 *
 *	@param image imagem a ser copiada.
 *
 *	@return Handle da imagem criada.
 */
Image* imgCopy(Image* image);

/**
 *	Cria uma nova nova copia imagem dada em tons de cinza.
 *
 *	@param image imagem a ser copiada em tons de cinza.
 *
 *	@return Handle da imagem criada.
 */
Image* imgGrey(Image* image);

/**
 *	Obtem a largura (width) de uma imagem.
 *
 *	@param image Handle para uma imagem.
 *	@return  a largura em pixels (width) da imagem.
 */
int imgGetWidth(Image* image);

/**
 *	Obtem a altura (heigth) de uma imagem.
 *
 *	@param image Handle para uma imagem.
 *	@return  a altura em pixels (height) da imagem.
 */
int imgGetHeight(Image* image);

/**
 *	Obtem a dimensao do espaco de cor de cada pixel (1=lminancia ou 3=RGB).
 *
 *	@param image Handle para uma imagem.
 *	@return  dimensao do espaco de cor de cada pixel (1=lminancia ou 3=RGB) da imagem.
 */

int imgGetDimColorSpace(Image* image);
/**
 *	Obtem as dimensoes de uma imagem.
 *
 *	@param image Handle para uma imagem.
 *	@param w [out]Retorna a largura da imagem.
 *	@param h [out]Retorna a altura da imagem.
 */
float*  imgGetData(Image* image);

/**
 *	Ajusta o pixel de uma imagem com a cor especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color Cor do pixel(valor em float [0,1]).
 */
void imgSetPixel3fv(Image* image, int x, int y, float*  color);

void imgSetPixel3fcolor(Image* image, int x, int y, cColor<float> color);

/**
 *	Ajusta o pixel de uma imagem com a cor especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color Cor do pixel (valor em unsigend char[0,255]).
 */
void imgSetPixel3ubv(Image* image, int x, int y, unsigned char * color);

/**
 *	Obtem o pixel de uma imagem na posicao especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color [out] Pixel da posicao especificada(valor em float [0,1]).
 */
void imgGetPixel3fv(Image* image, int x, int y, float* color);

cColor<float> imgGetPixel3fcolor(Image* image, int x, int y);

/**
 *	Obtem o pixel de uma imagem na posicao especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color [out] Pixel da posicao especificada (valor em unsigend char[0,255]).
 */
void imgGetPixel3ubv(Image* image, int x, int y, unsigned char *color);

/**
*	Calcula a diferenca entre duas imagens.
*
*	@param img0 img0 =  |img0-img1|^1/gamma
*	@param img1.
*
*	@return retorna o valor medio dos canais rgb de todos pixels (antes da correcao gamma).
*
*/
float imgDif(Image*img0, Image*img1, float gamma);


/**
 *	Le a imagem a partir do arquivo especificado.
 *
 *	@param filename Nome do arquivo de imagem.
 *
 *	@return imagem criada.
 */
Image* imgReadTGA(char *filename);

/**
 *	Salva a imagem no arquivo especificado em formato TGA.
 *
 *	@param filename Nome do arquivo de imagem.
 *	@param image Handle para uma imagem.
 *
 *	@return retorna 1 caso nao haja erros.
 */
int imgWriteTGA(char *filename, Image* image);

/**
 *	Salva a imagem no arquivo especificado em formato BMP.
 *
 *	@param filename Nome do arquivo de imagem.
 *	@param bmp Handle para uma imagem.
 *
 *	@return retorna 1 caso nao haja erros.
 */
int imgWriteBMP(char *filename, Image* bmp);

/**
 *	Le a imagem a partir do arquivo especificado.
 *
 *	@param filename Nome do arquivo de imagem.
 *
 *	@return imagem criada.
 */
Image* imgReadBMP (char *filename);


/**
 *	Le a imagem a partir do arquivo especificado.
 *  A imagem e' armazenada como um arquivo binario
 *  onde os tres campos da .
 *
 *	@param filename Nome do arquivo de imagem.
 *
 *	@return imagem criada.
 */
Image* imgReadPFM(char *filename);

/**
 *	Salva a imagem no arquivo especificado .
 *
 *	@param filename Nome do arquivo de imagem.
 *	@param image Handle para uma imagem.
 *
 *	@return retorna 1 caso nao haja erros.
 */
int imgWritePFM(char *filename, Image* image);


/**
 *	Conta o numero de cores diferentes na imagem
 *
 *	@param image Handle para uma imagem.
 *	@param w Nova largura da imagem.
 *	@param h Nova altura da imagem.
 */
int imgCountColor(Image* image, float);




/*********************************************************/
/*** FUNCOES QUE DEVEM SER IMPLEMENTADAS NO TRABALHO 1 ***/
/*********************************************************/

/**
 *	 Aplica o filtro de Gauss para eliminar o ruido branco da imagem.
 *
 *	@param image Handle para uma imagem que vai receber a imagem filtrada.
 *	@param image Handle para uma imagem que vai ser filtrada.
 *
 */
void imgGauss(Image* img_dst, Image* img_src) ;

/**
 *	 Aplica o filtro de Mediana para eliminar o ruido sal e pimenta da imagem.
 *
 *	@param image Handle para uma imagem a ser filtrada. A resposta esta na própria imagem.
 *
 */
void imgMedian(Image* image);

/**
 *	 Calcula uma imagem com pixels nas arestas 
 *  da imagem dada.
 *
 *	@param image Handle para uma imagem.
 *
 * @return Handle para a image de luminosidade onde o branco destaca as arestas.
 */
Image* imgEdges(Image* image);

/*
 * Reduz o numero de cores distintas de uma imagem para ncolors.
 *
 * @param image Handle para a imagem original.
 *
 * @return Handle para a imagem que tem apenas ncolors distintaas.
 */
Image* imgReduceColors(Image * img, int ncolors);
#endif

