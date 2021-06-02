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
	int labels, i;
	OVC* blobs;
	int aux;

	cv::Mat frame;
	while (key != 'q') {
		/* Leitura de uma frame do vídeo */
		capture.read(frame);
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
		vc_binary_open(image[2], image[3], 5, 7);
		blobs = vc_binary_blob_labelling(image[3], image[2], &labels);

		if (labels == 0) {
			free(blobs);
			vc_hsv_segmentation(image[1], image[2], 335, 360, 30, 100, 50, 100); // H S V VERMELHO H200-230 S79-93 V48-60 
			vc_binary_open(image[2], image[3], 5, 7);
			blobs = vc_binary_blob_labelling(image[3], image[2], &labels);
		}
		vc_binary_blob_info(image[2], blobs, labels);
		printf("\n\nNumero de objs: %d\n", labels);


		/*for (i = 0; i < labels; i++)
		{
			printf("\nCentro de massa de objs: x %d - y %d\n", blobs[i].xc, blobs[i].yc);
			printf("-> Area Nucleo %d: %d pixeis\n", i + 1, blobs[i].area);
		}*/

		//vc_draw_bouding_box(image[2], image[0], blobs, labels);
		//vc_draw_center_mass(image[2], image[0], blobs, labels);
		//free(blobs);

		vc_gray_to_rgb(image[3], image[0]);
		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, image[0]->data, video.width * video.height * 3);
		// +++++++++++++++++++++++++
		/* Exibe a frame */
		cv::imshow("VC - Video", frame);

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