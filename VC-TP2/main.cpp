#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
using namespace std;

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

	capture.open(0, cv::CAP_DSHOW);

	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o vídeo!\n";
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

	/* Declaração de variáveis*/
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
	string sinal;

	cv::Mat frame;
	cv::Mat frame2;
	while (key != 'q') {
		/* Leitura de uma frame do vídeo */
		capture.read(frame);
		capture.read(frame2);
		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;


		/* Número da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES) / 2;

		//// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image[0]->data, frame.data, video.width * video.height * 3);
		memcpy(image[1]->data, frame.data, video.width * video.height * 3);

		vc_convert_rgb(image[1], image[0]);
		vc_rgb_to_hsv(image[0], image[1]);

		vc_hsv_segmentation(image[1], image[2], 180, 260, 50, 100, 50, 100); // H S V AZUL H200-230 S79-93 V48-60
		vc_binary_open(image[2], image[3], 5, 11);
		blobs = vc_binary_blob_labelling(image[3], image[2], &labels);
		vc_binary_blob_info(image[2], blobs, labels);

		mainBlob.area = 0;

		if (blobs != NULL) {

			// Ordenação dos blobs por ordem decrescente tendo em conta a sua área
			for (i = 0; i < labels - 1; i++)
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

			if (blobs[0].area > 7000) { //TODO: VERIFICAR SE E UM QUADRADO (OU PERTO)
				mainBlob = blobs[0];
				percent = mainBlob.perimeter * 100 / (float)mainBlob.area;
				printf("mainBlob percent: %f\n", percent);

				if (percent >= 3.7 && percent <= 5.0) {	//setas
					xCenter = mainBlob.x + mainBlob.width / 2;
					//printf("%d - %d\n", xCenter, mainBlob.xc);
					if (mainBlob.xc > xCenter)
						sinal = "Obrigatorio virar a esquerda";
					else if (mainBlob.xc < xCenter)
						sinal = "Obrigatorio virar a direita";
				}
				else if (percent > 5.1) { // AUTO ESTRADA OU CARRO
					if (labels > 3) {
						percent2 = blobs[1].area * 100 / (float)mainBlob.area;
						percent3 = blobs[2].area * 100 / (float)mainBlob.area;
						percent4 = blobs[3].area * 100 / (float)mainBlob.area;
						if (percent2 >= 13.0 && percent2 < 17.0 && percent3 >= 0.8 && percent3 < 2.7 && percent4 >= 0.8 && percent4 < 2.7) {
							sinal = "Via de automoveis e motociclos";
						}
						else sinal = "Auto-Estrada";
						printf("%f\t%f\t%f\t\n", percent2, percent3, percent4);
					}
					else
						sinal = "Auto-Estrada";

				}
			}
		}
		printf("\n\t\t%d", mainBlob.area);
		if (mainBlob.area == 0) {
			free(blobs);
			printf("REd");
			vc_hsv_red_segmentation(image[1], image[2], 345, 7, 38, 70, 85, 100); // H S V VERMELHO H200-230 S79-93 V48-60 
			vc_binary_open(image[2], image[3], 3, 5);
			/*blobs = vc_binary_blob_labelling(image[3], image[2], &labels);*/

			//if (blobs != NULL) {

			//	// Ordenação dos blobs por ordem decrescente tendo em conta a sua área
			//	for (i = 0; i < labels - 1; i++)
			//	{
			//		for (j = i + 1; j < labels; j++)
			//		{
			//			if (blobs[i].area < blobs[j].area)
			//			{
			//				auxBlob = blobs[i];
			//				blobs[i] = blobs[j];
			//				blobs[j] = auxBlob;
			//			}
			//		}
			//	}

			//	if (blobs[0].area > 7000) { //TODO: VERIFICAR SE E UM QUADRADO (OU PERTO)
			//		mainBlob = blobs[0];
			//		percent = mainBlob.perimeter * 100 / (float)mainBlob.area;
			//		printf("RED - mainBlob percent: %f\n", percent);

			//		//if (percent >= 3.7 && percent <= 5.0) {	//setas
			//		//	xCenter = mainBlob.x + mainBlob.width / 2;
			//		//	//printf("%d - %d\n", xCenter, mainBlob.xc);
			//		//	if (mainBlob.xc > xCenter)
			//		//		sinal = "Obrigatorio virar a esquerda";
			//		//	else if (mainBlob.xc < xCenter)
			//		//		sinal = "Obrigatorio virar a direita";
			//		//}
			//		//else if (percent > 5.1) { // AUTO ESTRADA OU CARRO
			//		//	if (labels > 3) {
			//		//		percent2 = blobs[1].area * 100 / (float)mainBlob.area;
			//		//		percent3 = blobs[2].area * 100 / (float)mainBlob.area;
			//		//		percent4 = blobs[3].area * 100 / (float)mainBlob.area;
			//		//		if (percent2 >= 13.0 && percent2 < 17.0 && percent3 >= 0.8 && percent3 < 2.7 && percent4 >= 0.8 && percent4 < 2.7) {
			//		//			sinal = "Via de automoveis e motociclos";
			//		//		}
			//		//		else sinal = "Auto-Estrada";
			//		//		printf("%f\t%f\t%f\t\n", percent2, percent3, percent4);
			//		//	}
			//		//	else
			//		//		sinal = "Auto-Estrada";

			//		//}
			//	}
			//}
		}


		//free(blobs);
		/*if (mainBlob.area != 0) {
			blobs = (OVC*)calloc((1), sizeof(OVC));
			blobs[0] = mainBlob;
			vc_draw_bouding_box(image[2], image[0], blobs, 1);
			vc_draw_center_mass(image[2], image[0], blobs, 1);
			free(blobs);
		}*/


		vc_gray_to_rgb(image[3], image[1]);


		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, image[0]->data, video.width* video.height * 3);
		if (mainBlob.area != 0) {
			str = std::string("SINAL: ").append(sinal);
			cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
			cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		}
		cv::imshow("VC - Video", frame);
		// +++++++++++++++++++++++++
		memcpy(frame2.data, image[1]->data, video.width * video.height * 3);
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