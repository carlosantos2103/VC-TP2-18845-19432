//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITECNICO DO CAVADO E DO AVE
//                          2020/2021
//             ENGENHARIA DE SISTEMAS INFORMATICOS
//                 VISAO POR COMPUTADOR - TP2
//
//                [  JOAO AZEVEDO   - 18845  ]
//                [  CARLOS SANTOS  - 19432  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar mem�ria para uma imagem
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar mem�ria de uma imagem
IVC* vc_image_free(IVC* image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

//Converter RGB para HSV
int vc_rgb_to_hsv(IVC* src, IVC* dst) {
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	int width = src->width;
	int height = src->height;
	long int pos;
	float rf, gf, bf;
	float value;
	float max;
	float min;
	float sat;
	float hue;
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			pos = y * bytesperline_src + x * channels_src;

			rf = (float)src->data[pos];
			gf = (float)src->data[pos + 1];
			bf = (float)src->data[pos + 2];

			//maximo
			if ((rf >= gf) && (rf >= bf))
				max = rf;
			else if ((gf >= rf) && (gf >= bf))
				max = gf;
			else max = bf;
			value = max;

			//minimo
			if ((rf <= gf) && (rf <= bf))
				min = rf;
			else if ((gf <= rf) && (gf <= bf))
				min = gf;
			else min = bf;

			if (max == 0) {
				sat = 0;
				hue = 0;
			}
			else {
				sat = (max - min) / value;

				if ((max == rf) && gf > bf)
					hue = 60.0f * (gf - bf) / (max - min);
				else if ((max == rf) && (bf >= gf))
					hue = 360.0f + 60.0f * (gf - bf) / (max - min);
				else if ((max == gf))
					hue = 120.0f + 60.0f * (bf - rf) / (max - min);
				else if ((max == bf))
					hue = 240.0f + 60.0f * (rf - gf) / (max - min);
				else if (max == min)
					hue = 0;
			}
			dst->data[pos] = (unsigned char)((hue / 360.0f) * 255.0f);
			dst->data[pos + 1] = (unsigned char)(sat * 255.0f);
			dst->data[pos + 2] = (unsigned char)value;
		}
	}
	return 1;
}

//Converter RGB para HSV
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	float h, s, v;
	long int pos_src, pos_dst;
	int x, y;

	// Verifica��o de erros+
	if ((width <= 0) || (height <= 0) || (datasrc == NULL)) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels_src != 3 || dst->channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			h = ((float)datasrc[pos_src]) / 255.0f * 360.0f;
			s = ((float)datasrc[pos_src + 1]) / 255.0f * 100.0f;
			v = ((float)datasrc[pos_src + 2]) / 255.0f * 100.0f;


			if (h >= hmin && h <= hmax
				&& s >= smin && s <= smax
				&& v >= vmin && v <= vmax) {
				datadst[pos_dst] = (unsigned char)255;
			}
			else {
				datadst[pos_dst] = (unsigned char)0;
			}
		}
	}
	return 1;
}

//Dilata��o de imagem bin�ria
int vc_binary_dilate(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int offset = (kernel - 1) / 2;
	long int pos;
	int x, y, x2, y2, level;


	// Verifica��o de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL)) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels_src != 1 || channels_dst != 1) return 0;

	//Calculo media da vizinhanca de cada pixel
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			level = 0;
			for (y2 = y - offset; y2 <= y + offset; y2++)
			{
				//Verifica se o pixel existe
				if (y2 > -1 && y2 < height)
					for (x2 = x - offset; x2 <= x + offset; x2++)
					{
						if (x2 > -1 && x2 < width) {
							pos = y2 * bytesperline + x2 * channels_src;
							if (datasrc[pos] != 0) {
								level = 1;
								break;
							}
						}
					}
			}
			pos = y * bytesperline + x * channels_src;

			if (level == 1)
				datadst[pos] = 255;
			else
				datadst[pos] = 0;
		}
	}
	return 1;
}

//Eros�o de imagem bin�ria
int vc_binary_erode(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int offset = (kernel - 1) / 2;
	long int pos;
	int x, y, x2, y2, level;

	// Verifica��o de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL)) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels_src != 1 || channels_dst != 1) return 0;

	//Calculo media da vizinhanca de cada pixel
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			level = 0;
			for (y2 = y - offset; y2 <= y + offset; y2++)
			{
				//Verifica se o pixel existe
				if (y2 > -1 && y2 < height)
					for (x2 = x - offset; x2 <= x + offset; x2++)
					{
						if (x2 > -1 && x2 < width) {
							pos = y2 * bytesperline + x2 * channels_src;
							if (datasrc[pos] == 0) {
								level = 1;
								break;
							}
						}
					}
			}
			pos = y * bytesperline + x * channels_src;

			if (level == 1)
				datadst[pos] = 0;
			else
				datadst[pos] = 255;
		}
	}
	return 1;
}

//OPEN de imagem bin�ria
int vc_binary_open(IVC* src, IVC* dst, int kernel, int kernel2)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;

	// Verifica��o de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL)) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels_src != 1 || channels_dst != 1) return 0;

	IVC* aux = vc_image_new(width, height, channels_src, src->levels);
	if (aux == NULL)
	{
		printf("ERROR -> vc_image_new():\n\tFail to create file!\n");
		return 0;
	}

	vc_binary_erode(src, aux, kernel);
	vc_binary_dilate(aux, dst, kernel2);

	vc_image_free(aux);

	return 1;
}

// Etiquetagem de blobs
// src		: Imagem bin�ria de entrada
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel, onde ser� armazenado o n�mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

// Desenha uma caixa delimitadora em cada um dos objetos
int vc_draw_bouding_box(IVC* src, IVC* dst, OVC* blobs, int labels)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;
	int channels_dst = dst->channels;
	int x, y, i;
	long int pos, pos_dst;
	int npixels = width * height;

	//Verificacao de erros
	if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
	if (channels != 1 || channels_dst != 3) return 0;
	//Verifica se existe blobs
	if (blobs != NULL)
	{
		// Percorre todos os objetos existentes
		for (i = 0; i < labels; i++)
		{
			// Percorre todos os pixeis da imagem
			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					// Calculo da posicao da imagem cinzenta, com 1 channel
					pos = y * bytesperline + x * channels;
					// Calculo da posicao da imagem RGB, com 3 channel
					pos_dst = y * dst->bytesperline + x * dst->channels;
					// Caso o y e x estejam compreendidos entre os parametros dos blobs desenha-se a caixa delimitadora
					if (y >= blobs[i].y && y <= blobs[i].y + blobs[i].height && x >= blobs[i].x && x <= blobs[i].x + blobs[i].width)
						if (x == blobs[i].x || x == blobs[i].x + blobs[i].width) {
							datadst[pos_dst] = 0;
							datadst[pos_dst + 1] = 0;
							datadst[pos_dst + 2] = 0;
							datadst[pos_dst + 3] = 255;
							datadst[pos_dst + 4] = 255;
							datadst[pos_dst + 5] = 255;
							datadst[pos_dst - 1] = 255;
							datadst[pos_dst - 2] = 255;
							datadst[pos_dst - 3] = 255;
						}
						else if (y == blobs[i].y || y == blobs[i].y + blobs[i].height) {
							datadst[pos_dst] = 0;
							datadst[pos_dst + 1] = 0;
							datadst[pos_dst + 2] = 0;
							datadst[pos_dst + width * 3] = 255;
							datadst[pos_dst + width * 3 + 1] = 255;
							datadst[pos_dst + width * 3 + 2] = 255;
							datadst[pos_dst - width * 3] = 255;
							datadst[pos_dst - width * 3 + 1] = 255;
							datadst[pos_dst - width * 3 + 2] = 255;
						}
				}
			}
		}
	}

	return 1;
}

// Desenha o centro de massa do objeto
int vc_draw_center_mass(IVC* src, IVC* dst, OVC* blobs, int labels)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;
	int channels_dst = dst->channels;
	int x, y, i, j;
	long int pos, pos_dst;

	//Verificacao de erros
	if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
	if (channels != 1 || channels_dst != 3) return 0;
	//Verifica se existe blobs
	if (blobs != NULL)
	{
		// Percorre todos os objetos existentes
		for (i = 0; i < labels; i++)
		{
			// Percorre todos os pixeis da imagem
			for (y = 0; y < height; y++)
			{
				// Caso o y corresponda ao valor do blobg calculado em y
				if (y == blobs[i].yc) {
					for (x = 0; x < width; x++)
					{
						// Caso o x corresponda ao valor do blobg calculado em x
						if (x == blobs[i].xc) {
							// Calculo da posicao
							pos_dst = y * dst->bytesperline + x * dst->channels;

							// Alteracao dos pixeis para a cor branca de forma a salientar o centro de massa

							// Pixel central
							datadst[pos_dst] = 0;
							datadst[pos_dst + 1] = 255;
							datadst[pos_dst + 2] = 0;

							// Pixel seguinte
							datadst[pos_dst + 3] = 0;
							datadst[pos_dst + 4] = 255;
							datadst[pos_dst + 5] = 0;

							// Pixel Anterior
							datadst[pos_dst - 1] = 0;
							datadst[pos_dst - 2] = 255;
							datadst[pos_dst - 3] = 0;

							// Pixel superior em relacao ao pixel central
							datadst[pos_dst - width * 3] = 0;
							datadst[pos_dst - width * 3 + 1] = 255;
							datadst[pos_dst - width * 3 + 2] = 0;

							// Pixel inferior em relacao ao pixel central
							datadst[pos_dst + width * 3] = 0;
							datadst[pos_dst + width * 3 + 1] = 255;
							datadst[pos_dst + width * 3 + 2] = 0;

							break;
						}
					}
					break;
				}
			}
		}
	}

	return 1;
}

int vc_convert_bgr_to_rgb(IVC* src, IVC* dst)
{
	unsigned char* data = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;
	int channels_dst = dst->channels;
	int x, y, i, j;
	long int pos;

	//Verificacao de erros
	if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
	if (channels != 3 || channels_dst != 3) return 0;
	//Verifica se existe blobs
	
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * dst->bytesperline + x * dst->channels;
			datadst[pos] = data[pos + 2];
			datadst[pos+1] = data[pos + 1];
			datadst[pos+2] = data[pos];
		}
	}

	return 1;
}

//Converter RGB para HSV
int vc_hsv_red_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	float h, s, v;
	long int pos_src, pos_dst;
	int x, y;

	// Verifica��o de erros
	if ((width <= 0) || (height <= 0) || (datasrc == NULL)) return 0;
	if (width != dst->width || height != dst->height) return 0;
	if (channels_src != 3 || dst->channels != 1) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			h = ((float)datasrc[pos_src]) / 255.0f * 360.0f;
			s = ((float)datasrc[pos_src + 1]) / 255.0f * 100.0f;
			v = ((float)datasrc[pos_src + 2]) / 255.0f * 100.0f;

			if (h >= hmin && h < 360 || h <= hmax && h > 0
				&& s >= smin && s <= smax
				&& v >= vmin && v <= vmax) {
				datadst[pos_dst] = (unsigned char)255;
			}
			else {
				datadst[pos_dst] = (unsigned char)0;
			}
		}
	}
	return 1;
}
