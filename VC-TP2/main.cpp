#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

extern "C" {
	#include "vc.h"
}


int main(void) {
	// Vídeo
	char videofile[20] = "video.avi";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de vídeo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi deverá estar localizado no mesmo directório que o ficheiro de código fonte.
	*/
	//capture.open(videofile);

	/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
	capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
		return 1;
	}

	/* Número total de frames no vídeo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do vídeo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolução do vídeo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o vídeo */
	cv::namedWindow("VC - Video", cv::WINDOW_AUTOSIZE);

	IVC* image[4];
	image[0] = vc_image_new(video.width, video.height, 3, 255);
	image[1] = vc_image_new(video.width, video.height, 3, 255);
	image[2] = vc_image_new(video.width, video.height, 1, 255);
	image[3] = vc_image_new(video.width, video.height, 1, 255);
	int labels, i, j;
	OVC* blobs;
	int aux, xCenter;
	float percent, percent2, percent3, percent4;
	OVC mainBlob, auxBlob;

	cv::Mat frame;
	cv::Mat frame2;
	while (key != 'q') {
		/* Leitura de uma frame do vídeo */
		capture.read(frame);
		capture.read(frame2);
		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* Número da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES)/2;

		// Faça o seu código aqui...
		//// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image[0]->data, frame.data, video.width * video.height * 3);
		memcpy(image[1]->data, frame.data, video.width* video.height * 3);

		// Executa uma função da nossa biblioteca vc
		vc_convert_rgb(image[1], image[0]);
		vc_rgb_to_hsv(image[0], image[1]);

		vc_hsv_segmentation(image[1], image[2], 180, 260, 50, 100, 50, 100); // H S V AZUL H200-230 S79-93 V48-60 
		vc_binary_open(image[2], image[3], 3, 5);
		blobs = vc_binary_blob_labelling(image[3], image[2], &labels);
		vc_binary_blob_info(image[2], blobs, labels);

		mainBlob.area = 0;

		for (i = 0; i < labels-1; i++)
		{
			for (j = i + 1; j < labels; j++)
			{
				if (blobs[i].area < blobs[j].area)
				{
					auxBlob = blobs[i];
					blobs[i] = blobs[j];
					blobs[j] = auxBlob;
				}
			}
		}
		if (blobs != NULL) {
			if (blobs[0].area > 6000) {
				mainBlob = blobs[0];
				percent = mainBlob.perimeter * 100 / (float)mainBlob.area;
				//printf("mainBlob percent: %f\n", percent);

				if (percent >= 3.7 && percent <= 5.0) {	//setas
					xCenter = mainBlob.x + mainBlob.width / 2;
					//printf("%d - %d\n", xCenter, mainBlob.xc);
					if (mainBlob.xc > xCenter)
						printf("Virar a esquerda\n");
					else if (mainBlob.xc < xCenter)
						printf("Virar a direita\n");
				}
				else if (percent > 5.1) { // AUTO ESTRADA OU CARRO
					if (labels > 3) {
						percent2 = blobs[1].area * 100 / (float)mainBlob.area;
						percent3 = blobs[2].area * 100 / (float)mainBlob.area;
						percent4 = blobs[3].area * 100 / (float)mainBlob.area;
						if (percent2 >= 14.0 && percent2 < 16.0 && percent3 >= 1.0 && percent3 < 2.2 && percent4 >= 1.0 && percent4 < 2.2) {
							printf("CARRO\n");
						}
						else printf("AUTO ESTRADA\n");
						printf("%f\t%f\t%f\t\n", percent2, percent3, percent4);
					}
					else printf("AUTO ESTRADA\n");

				}
			}
		}

		if (labels == 4)
		{
			// Carro
		}

		if (labels == 0) {
			free(blobs);
			vc_hsv_red_segmentation(image[1], image[2], 345, 5, 63, 85, 60, 90); // H S V VERMELHO H200-230 S79-93 V48-60 
			vc_binary_open(image[2], image[3], 5, 7);
			blobs = vc_binary_blob_labelling(image[3], image[2], &labels);
		}
		//printf("\n\nNumero de objs: %d\n", labels);


		/*for (i = 0; i < labels; i++)
		{
			printf("\nCentro de massa de objs: x %d - y %d\n", blobs[i].xc, blobs[i].yc);
			printf("-> Area Nucleo %d: %d pixeis\n", i + 1, blobs[i].area);
		}*/
		free(blobs);
		blobs = (OVC*)calloc((1), sizeof(OVC));
		blobs[0] = mainBlob;
		vc_draw_bouding_box(image[2], image[0], blobs, 1);
		vc_draw_center_mass(image[2], image[0], blobs, 1);
		free(blobs);


		vc_gray_to_rgb(image[3], image[1]);


		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, image[0]->data, video.width * video.height * 3);
		cv::imshow("VC - Video", frame); 
		// +++++++++++++++++++++++++
		memcpy(frame2.data, image[1]->data, video.width* video.height * 3);
		cv::imshow("VC - Video2", frame2);

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	vc_image_free(image[0]);
	vc_image_free(image[1]);
	vc_image_free(image[2]);
	vc_image_free(image[3]);
	/* Fecha a janela */
	cv::destroyWindow("VC - Video");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	return 0;
}