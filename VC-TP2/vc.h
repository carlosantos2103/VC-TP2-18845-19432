//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITECNICO DO CAVADO E DO AVE
//                          2020/2021
//             ENGENHARIA DE SISTEMAS INFORMATICOS
//                 VISAO POR COMPUTADOR - TP2
//
//                [  JOAO AZEVEDO   - 18845  ]
//                [  CARLOS SANTOS  - 19432  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define VC_DEBUG
#define MAX(a,b) (a>b?a:b) 
#define MIN(a,b) (a<b?a:b)
#include <math.h> 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char* data;
	int width, height;
	int channels;			// Binario/Cinzentos=1; RGB=3
	int levels;				// Binario=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// �rea
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Per�metro
	int label;					// Etiqueta
} OVC;




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROT�TIPOS DE FUN��ES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNCOES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUNCOES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
int vc_rgb_to_hsv(IVC* src, IVC* dst);
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_hsv_red_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_binary_open(IVC* src, IVC* dst, int kernel, int kernel2);
int vc_convert_bgr_to_rgb(IVC* src, IVC* dst);

/*OVC*/
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);

// Desenha uma caixa delimitadora em cada um dos objetos
int vc_draw_bouding_box(IVC* src, IVC* dst, OVC* blobs, int labels);
// Desenha o centro de massa do objeto
int vc_draw_center_mass(IVC* src, IVC* dst, OVC* blobs, int labels);

