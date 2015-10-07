/**
 *   @file CGT1Img.c Trabalho de processamento de imagem com a IUP e OpenGL.
 *
 *   @author
 *         - Marcelo Gattass
 *
 *   @date
 *         Criado em:           12 de Agosto de 2004
 *         �ltima Modifica��o:   26 de Agosto de 2010
 *
 */
/*- Bibliotecas padrao usadas: --------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*- Inclusao das bibliotecas IUP e CD: ------------------------------------*/
#include <iup.h>                    /* interface das funcoes IUP */
#include <iupgl.h>                  /* interface das funcoes do canvas do OpenGL no IUP */
#include <iupcontrols.h>            /* interface para as funcoes de botoes de controle */

#ifdef WIN32
#include <windows.h>                /* inclui as definicoes do windows para o OpenGL */
#include <gl/gl.h>                  /* prototypes do OpenGL */
#include <gl/glu.h>                 /* prototypes do OpenGL */
#else
#include <GL/gl.h>                  /* prototypes do OpenGL */
#include <GL/glu.h>                 /* prototypes do OpenGL */
#endif


#include "image.h"                  /* TAD para imagem */

/*- Contexto do Programa: -------------------------------------------------*/
Image *image1, *image2, *undo; /* imagens dos dois canvas */
Ihandle *left_canvas, *right_canvas; /* hadle dos dois canvas */
Ihandle *label; /* handle do label para colocar mensagens para usuario */
Ihandle *dialog; /* handle para o dialogo principal */

FILE *fpLog; /* arquivo de log que registra as acoes do usuario */
extern char grupo[];

/*- Funcoes auxiliares ------------*/

/* Dialogo de selecao de arquivo  */
static char * get_file_name(void) {
  Ihandle* getfile = IupFileDlg();
  char* filename = NULL;

  IupSetAttribute(getfile, IUP_TITLE, "Abertura de arquivo");
  IupSetAttribute(getfile, IUP_DIALOGTYPE, IUP_OPEN);
  //IupSetAttribute(getfile, IUP_FILTER, "*.bmp;*.pfm");
  IupSetAttribute(getfile, IUP_FILTERINFO, "Arquivo de imagem (*.bmp ou *.hdr)");
  IupPopup(getfile, IUP_CENTER, IUP_CENTER); /* o posicionamento nao esta sendo respeitado no Windows */

  filename = IupGetAttribute(getfile, IUP_VALUE);
  return filename;
}

static char * get_new_file_name(void) {
  Ihandle* getfile = IupFileDlg();
  char* filename = NULL;

  IupSetAttribute(getfile, IUP_TITLE, "Salva arquivo");
  IupSetAttribute(getfile, IUP_DIALOGTYPE, IUP_SAVE);
  IupSetAttribute(getfile, IUP_FILTER, "*.bmp;*.pfm");
  IupSetAttribute(getfile, IUP_FILTERINFO, "Arquivo de imagem (*.bmp ou *.pfm)");
  IupPopup(getfile, IUP_CENTER, IUP_CENTER); /* o posicionamento nao esta sendo respeitado no Windows */

  filename = IupGetAttribute(getfile, IUP_VALUE);
  return filename;
}

/* Ajusta o tamanho da janela do dialogo de acordo com o tamanho da imagem exibida */
static void update_dialog_size(void) {
  if (image1) {
    int w, h;
    char buffer[512];
    w = imgGetWidth(image1);
    h = imgGetHeight(image1);
    sprintf(buffer, "%dx%d", w, h);
    IupSetAttribute(left_canvas, IUP_RASTERSIZE, buffer);
    IupSetAttribute(right_canvas, IUP_RASTERSIZE, buffer);
    IupSetAttribute(dialog, IUP_RASTERSIZE, NULL);
    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
  }
}
/*------------------------------------------*/
/* Callbacks do IUP.                        */
/*------------------------------------------*/

/* - Callback de mudanca de tamanho no canvas (mesma para ambos os canvas) */
int resize_cb(Ihandle *self, int width, int height) {
  IupGLMakeCurrent(self); /* torna o foco do OpenGL para este canvas */

  /* define a area do canvas que deve receber as primitivas do OpenGL */
  glViewport(0, 0, width, height);

  /* transformacao de instanciacao dos objetos no sistema de coordenadas da camera */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity(); /* identidade,  ou seja nada */

  /* transformacao de projecao */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLsizei) (width), 0.0, (GLsizei) (height)); /* ortografica no plano xy de [0,w]x[0,h] */

  return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
}

/* - Callback de repaint do canvas 1 */
int repaint_cb1(Ihandle *self) {
  int w = imgGetWidth(image1);
  int h = imgGetHeight(image1);
  //int type = imgGetDimColorSpace(image1);
  //float* rgbData = imgGetData(image1);

  IupGLMakeCurrent(self);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  //glRasterPos2d(0.0, 0.0); /* define a origem da imagem */

  glBegin(GL_POINTS);
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      float rgb[3];
      imgGetPixel3fv(image1, x, y, rgb);
            glColor3fv(rgb);
            glVertex2i(x, y);
        }
    }
    glEnd();

    glFlush(); /* forca atualizacao do OpenGL  (que pode ser bufferizado) */

    IupGLSwapBuffers(self); /* change the back buffer with the front buffer */

    //  if (type == 3)
    //    glDrawPixels(w, h, GL_RGB, GL_FLOAT, rgbData);
    //  else if (type == 1)
    //    glDrawPixels(w, h, GL_LUMINANCE, GL_FLOAT, rgbData);
    //  else
    //    printf("Imagem de tipo desconhecida. Nao e' RGB nem Luminancia\n");

  
  return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
}

/* - Callback de repaint do canvas 2 */

/* - poderia ser a mesma do 1, feita diferente para exemplificar OpenGL */
int repaint_cb2(Ihandle *self) {
  int w = imgGetWidth(image2);
  int h = imgGetHeight(image2);
  int x, y;

  IupGLMakeCurrent(self);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBegin(GL_POINTS);
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
            float rgb[3];
            imgGetPixel3fv(image2, x, y, rgb);
            glColor3fv(rgb);
            glVertex2i(x, y);
        }
    }
    glEnd();
    
    glFlush(); /* forca atualizacao do OpenGL  (que pode ser bufferizado) */
    
    IupGLSwapBuffers(self); /* change the back buffer with the front buffer */

    return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
}

int transf_cb(void) {
  if (image2) imgDestroy(image2);
  image2 = imgCopy(image1);
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  IupSetfAttribute(label, "TITLE", " ");
  fprintf(fpLog, "TRANSF\n");
  return IUP_DEFAULT;
}

int undo_cb(void) {
  Image* tmp = image2;
  image2 = undo;
  undo = tmp;
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  IupSetfAttribute(label, "TITLE", "Undo");
  fprintf(fpLog, "UNDO\n");
  return IUP_DEFAULT;
}

/* - Callback do botao que conta cor */
int conta_cb(Ihandle* self, int but, int pressed, int x, int y, char* status) {
  int finish_time;
  double duration;
  int start_time;
  int ncolors1, ncolors2;

  IupSetfAttribute(label, "TITLE", "Contando numero de cores distintas....por favor aguarde...");

  start_time = clock();
  ncolors1 = imgCountColor(image1, 1.f / 256);
  ncolors2 = imgCountColor(image2, 1.f / 256);
  finish_time = clock();
  duration = (double) (finish_time - start_time) / CLOCKS_PER_SEC;
  IupSetfAttribute(label, "TITLE", "Numero de cores. Esq: %d  Dir:%d - Tempo: %2.1f",
    ncolors1, ncolors2, duration);
  fprintf(fpLog, "Numero de cores. Esq: %d  Dir:%d - Tempo: %2.1f\n", ncolors1, ncolors2, duration);
  repaint_cb1(left_canvas); /* redesenha o canvas 2 */
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
}

int gauss_cb(void) {
  int ncolors2;
  int start_time;
  int finish_time;
  double duration;
  IupSetfAttribute(label, "TITLE", "Aplicando o filtro de Gauss...por favor aguarde...");

  /* salva a imagem corrente para acao de undo */
  if (undo) imgDestroy(undo);
  undo = imgCopy(image2);

  start_time = clock();
  imgGauss(image2, undo);
  finish_time = clock();

  duration = (double) (finish_time - start_time) / CLOCKS_PER_SEC;
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  ncolors2 = imgCountColor(image2, 1.f / 256);
  IupSetfAttribute(label, "TITLE", "Numero de cores: %d - tempo de processamento: %2.1f segundos",
    ncolors2, duration);
  fprintf(fpLog, "GAUSS Numero de cores: %d - tempo de processamento: %2.1f segundos\n", ncolors2, duration);
  return IUP_DEFAULT;
}

int reduz_cb(void) {
  int ncolors2;
  int start_time;
  int finish_time;
  double duration;
  IupSetfAttribute(label, "TITLE", "Reduzindo o numero de cores...por favor aguarde...");

  /* salva a imagem corrente para acao de undo */
  if (undo) imgDestroy(undo);
  undo = imgCopy(image2);

  start_time = clock();
  image2 = imgReduceColors(undo, 256);
  finish_time = clock();

  duration = (double) (finish_time - start_time) / CLOCKS_PER_SEC;
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  ncolors2 = imgCountColor(image2, 1.f / 256);
  IupSetfAttribute(label, "TITLE", "Numero de cores: %d - tempo de processamento: %2.1f segundos",
    ncolors2, duration);
  fprintf(fpLog, "ReduzCores256 Numero de cores: %d - tempo de processamento: %2.1f segundos\n", ncolors2, duration);
  return IUP_DEFAULT;
}

int mediana_cb(void) {
  int ncolors2;
  int start_time;
  int finish_time;
  double duration;
  IupSetfAttribute(label, "TITLE", "Aplicando o filtro de Gauss...por favor aguarde...");

  /* salva a imagem corrente para acao de undo */
  if (undo) imgDestroy(undo);
  undo = imgCopy(image2);

  start_time = clock();
  imgMedian(image2);
  finish_time = clock();

  duration = (double) (finish_time - start_time) / CLOCKS_PER_SEC;
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  ncolors2 = imgCountColor(image2, 1.f / 256);
  IupSetfAttribute(label, "TITLE", "Numero de cores: %d - tempo de processamento: %2.1f segundos",
    ncolors2, duration);
  fprintf(fpLog, "MEDIANA Numero de cores: %d - tempo de processamento: %2.1f segundos\n", ncolors2, duration);
  return IUP_DEFAULT;
}

int arestas_cb(void) {
  int start_time;
  int finish_time;
  double duration;
  IupSetfAttribute(label, "TITLE", "Calculando as arestas da imagem...por favor aguarde...");

  /* salva a imagem corrente para acao de undo */
  if (undo) imgDestroy(undo);
  undo = image2;

  start_time = clock();
  image2 = imgEdges(image2);
  finish_time = clock();

  duration = (double) (finish_time - start_time) / CLOCKS_PER_SEC;
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  IupSetfAttribute(label, "TITLE", "ARESTAS: tempo de processamento: %2.1f segundos", duration);
  fprintf(fpLog, "ARESTAS tempo de processamento: %2.1f segundos\n", duration);
  return IUP_DEFAULT;
}

int dif_cb(void) {
  float avg;
  IupSetfAttribute(label, "TITLE", "Comparando imagens....por favor aguarde...");
  avg = imgDif(image2, image1, 2.f);

  IupSetfAttribute(label, "TITLE", "Diferenca corrigida com gama de 0.5. Valor medio dos pixels = %.3f", avg);
  fprintf(fpLog, "DIF Valor medio dos pixels = %.3f\n", avg);
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */
  return IUP_DEFAULT;
}

int save_cb(Ihandle *self) {
  char* filename = get_new_file_name(); /* chama o dialogo de abertura de arquivo */
  if (filename == NULL) return 0;
  if (strstr(filename, ".bmp") || strstr(filename, ".BMP"))
    imgWriteBMP(filename, image2);
  else
    imgWritePFM(filename, image2);
  fprintf(fpLog, "SAVE %s\n", filename);
  return IUP_DEFAULT;
}

int load_cb(void) {
  char* filename = get_file_name(); /* chama o dialogo de abertura de arquivo */

  if (filename == NULL) return 0;

  if (strstr(filename, ".bmp") || strstr(filename, ".BMP")) {
    /* le nova imagem */
    if (image1) imgDestroy(image1);
    image1 = imgReadBMP(filename);
    if (image2) imgDestroy(image2);
    image2 = imgCopy(image1);
  } else if (strstr(filename, ".pfm") || strstr(filename, ".PFM")) {
    /* le nova imagem */
    if (image1) imgDestroy(image1);
    image1 = imgReadPFM(filename);
    if (image2) imgDestroy(image2);
    image2 = imgCopy(image1);
  } else
    IupMessage("Aviso", "Formato do arquivo de imagem n�o reconhecido\n");


  IupSetFunction("repaint_cb1", (Icallback) repaint_cb1);
  IupSetFunction("repaint_cb2", (Icallback) repaint_cb2);

  update_dialog_size();
  repaint_cb1(left_canvas); /* redesenha o canvas 2 */
  repaint_cb2(right_canvas); /* redesenha o canvas 2 */

  IupSetfAttribute(label, "TITLE", "%3dx%3d", imgGetWidth(image1), imgGetHeight(image1));

  IupSetFunction("transf_cb", (Icallback) transf_cb);
  IupSetFunction("undo_cb", (Icallback) undo_cb);
  IupSetFunction("conta_cb", (Icallback) conta_cb);
  IupSetFunction("gauss_cb", (Icallback) gauss_cb);
  IupSetFunction("reduz_cb", (Icallback) reduz_cb);
  IupSetFunction("mediana_cb", (Icallback) mediana_cb);
  IupSetFunction("arestas_cb", (Icallback) arestas_cb);
  IupSetFunction("dif_cb", (Icallback) dif_cb);
  IupSetFunction("save_cb", (Icallback) save_cb);
  fprintf(fpLog, "OPEN %s\n", filename);

  return IUP_DEFAULT;

}

/*-------------------------------------------------------------------------*/
/* Incializa o programa.                                                   */

/*-------------------------------------------------------------------------*/

int init(void) {
  Ihandle *toolbar, *statusbar, *box;
  Ihandle *load, * transf, *conta, *reduz, *gauss, *mediana, *arestas, *dif, *undo, *save;
  time_t now = time(NULL); /* obtem a data corrente para registro no arquivo de log */

  /* creates the toolbar and its buttons */
  load = IupButton("", "load_cb");
  IupSetAttribute(load, "TIP", "Carrega uma imagem.");
  IupSetAttribute(load, "IMAGE", "IUP_FileOpen");
  IupSetFunction("load_cb", (Icallback) load_cb);


  transf = IupButton("", "transf_cb");
  IupSetAttribute(transf, "IMAGE", "IUP_ArrowRight");
  IupSetAttribute(transf, "TIP", "Transfere para o outro canvas.");

  undo = IupButton("", "undo_cb"),
    IupSetAttribute(undo, "TIP", "Desfaz processamento (qdo possivel).");
  IupSetAttribute(undo, "IMAGE", "IUP_EditUndo");

  save = IupButton("", "save_cb");
  IupSetAttribute(save, "TIP", "Salva imagem processada.");
  IupSetAttribute(save, "IMAGE", "IUP_FileSave");

  conta = IupButton("nCores", "conta_cb");
  reduz = IupButton("Reduz", "reduz_cb");
  gauss = IupButton("Gauss", "gauss_cb");
  mediana = IupButton("Mediana", "mediana_cb");
  arestas = IupButton("Arestas", "arestas_cb");
  dif = IupButton("Dif.", "dif_cb");

  toolbar = IupHbox(
    load,
    transf,
    IupFill(),
    conta, reduz, gauss, mediana, arestas, dif,
    IupFill(),
    undo,
    save,
    NULL);

  IupSetAttribute(toolbar, "FONT", "HELVETICA_NORMAL_8");
  IupSetAttribute(toolbar, "ALIGNMENT", "ACENTER");


  /* cria dois canvas */
  left_canvas = IupGLCanvas("repaint_cb1");
  IupSetAttribute(left_canvas, IUP_RASTERSIZE, "320x240");
  IupSetAttribute(left_canvas, "RESIZE_CB", "resize_cb");

  right_canvas = IupGLCanvas("repaint_cb2");
  IupSetAttribute(right_canvas, IUP_RASTERSIZE, "320x240");
  IupSetAttribute(right_canvas, "RESIZE_CB", "resize_cb");

  /* associa o evento de repaint a funccao repaint_cb */
  IupSetFunction("repaint_cb1", NULL);
  IupSetFunction("repaint_cb2", NULL);
  IupSetFunction("resize_cb", (Icallback) resize_cb);

  /* the status bar is just a label to put some usefull information in run time */
  label = IupLabel("status");
  IupSetAttribute(label, "EXPAND", "HORIZONTAL");
  IupSetAttribute(label, "FONT", "HELVETICA_NORMAL_8");
  statusbar = IupSetAttributes(IupHbox(
    IupFrame(IupHbox(label, NULL)),
    NULL), "MARGIN=5x5");

  /* this is the most external box that puts together
  the toolbar, the two canvas and the status bar */
  box = IupVbox(
    toolbar,
    IupSetAttributes(IupHbox(
    left_canvas,
    right_canvas,
    NULL), "GAP=5"),
    statusbar,
    NULL);

  /* create the dialog and set its attributes */

  dialog = IupDialog(box);
  IupSetAttribute(dialog, "CLOSE_CB", "app_exit_cb");
  IupSetAttribute(dialog, "TITLE", "CG:Trabalho 1");

  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

  fpLog = fopen("inf1761T1.txt", "at");
  fprintf(fpLog, "****************************************************************\n");
  fprintf(fpLog, "%s\n", grupo);
  fprintf(fpLog, "Data: %s\n", ctime(&now));

  return 1;
}

/*-------------------------------------------------------------------------*/
/* Rotina principal.                                                       */

/*-------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
  IupOpen(&argc, &argv);
  IupGLCanvasOpen();
  IupImageLibOpen();
  if (init())
    IupMainLoop();
  IupClose();

  return 0;
}