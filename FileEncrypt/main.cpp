#include "FileEncrypt.h"
#include <QtWidgets/QApplication>

#include <QtCore/QtPlugin>
#include <QDebug>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)

#define __STDC_CONSTANT_MACROS
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

#define SDL_MAIN_HANDLED


#include <SDL.h>

int main(int argc, char *argv[])
{
	//  ffmpeg test ------------------------------
	av_register_all() ;

	AVFormatContext *pFormatctx = avformat_alloc_context();
	

	//char testFilePath[] = "D:\\work\\fileEncrypt\\fileCrypt\\FileEncrypt\\Debug\\3529.mp4";
	char testFilePath[] = "D:\\work\\git\\simplest_ffmpeg_player\\simplest_ffmpeg_decoder\\Titanic.mkv";
	int rt = avformat_open_input(&pFormatctx, testFilePath, NULL, NULL);
	if ( rt != 0 )
	{
		char errorString[100] = {0};
		av_strerror(rt, errorString, 100);
		qDebug() << " error  avformat_open_input !"<< errorString;
		return 0;
	}

	// 时长
	qDebug() << "time:" << pFormatctx->duration;
	// 封装格式
	qDebug() << "format:" << pFormatctx->iformat->name<<", long name:"<< pFormatctx->iformat->long_name;

	if ( avformat_find_stream_info(pFormatctx, NULL) < 0)
	{
		char errorString[100] = { 0 };
		av_strerror(rt, errorString, 100);
		qDebug() << " error  avformat_find_stream_info !"<<errorString;
		return 0;
	}


	int videoindex = -1;
	for (int i = 0; i < pFormatctx->nb_streams; i++)
	{
		if ( pFormatctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
		{
			videoindex = i;
			break;
		}
	}

	if (videoindex == -1)
	{
		char errorString[100] = { 0 };
		av_strerror(rt, errorString, 100);
		qDebug() << "error  no video codec !"<<errorString;
		return 0;
	}
	qDebug() << " videoindex:" << videoindex;

	AVCodecContext *pCodecCtx = pFormatctx->streams[videoindex]->codec;
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	qDebug() << " pCodec:" << pCodec->name << ", long name:" << pCodec->long_name;

	if ( avcodec_open2(pCodecCtx, pCodec, NULL) != 0 )
	{
		char errorString[100] = { 0 };
		av_strerror(rt, errorString, 100);
		qDebug() << "error avcodec_open2 !"<<errorString;
		return 0;
	}

	AVPacket *pPacket = av_packet_alloc(); 
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameYUV = av_frame_alloc();
	int got_picture; 
	int ret;

	//SwsContext *swsCtx = sws_alloc_context();

	unsigned char *out_buffer;
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));



	struct SwsContext *swsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


	// sdl2 test ---------------------
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		qDebug() << "Unable to init SDL: %" << SDL_GetError();
		return 1;
	}
	SDL_Window *sdlW = SDL_CreateWindow("good job", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pCodecCtx->width, pCodecCtx->height, ::SDL_WINDOW_OPENGL | ::SDL_WINDOW_RESIZABLE);
	if (!sdlW) {
		qDebug() << "Unable to create sdl window: %" << SDL_GetError();
		return 1;
	}
	SDL_Renderer *render = SDL_CreateRenderer(sdlW, -1, SDL_RENDERER_ACCELERATED);
	if (!render) {
		qDebug() << "Unable to create sdl renderer: %" << SDL_GetError();
		return 1;
	}
	SDL_Texture *texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_TARGET, 320, 180);
	if (!texture) {
		qDebug() << "Unable to create sdl texture: %" << SDL_GetError();
		return 1;
	}

	SDL_Rect sdlRect;

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = pCodecCtx->width;
	sdlRect.h = pCodecCtx->height;
	int count = 10000;
	while (av_read_frame(pFormatctx, pPacket) == 0 && count > 0 ) {
		qDebug() << 10000 - count;
		qDebug() << "\tsize: " << pPacket->size << ", stream_index:" << pPacket->stream_index;

		got_picture = 0;
		if (pPacket && pPacket->stream_index==videoindex){

			//pPacket->data, pPacket->size

			//pFrameYUV = av_frame_alloc();
			av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
				AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);


			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, pPacket);
			if ( ret < 0 ){
				char errorString[100] = { 0 };
				av_strerror(rt, errorString, 100);
				qDebug() << "decode error !" << errorString;
				break;
			}
			qDebug() << "\t" << got_picture;

			if (got_picture) {
				sws_scale(swsCtx, (const uint8_t* const *)pFrame->data, pFrame->linesize, 0, pFrame->height, (uint8_t* const *)pFrameYUV->data, pFrameYUV->linesize);


				//qDebug() << "\t  sws_scale over" ;
				//char buffer[1024] = { 0 };
				//qDebug() << "\t  sws_scale over 2";
				//strcpy_s(buffer, pFrameYUV->width, (char *)(pFrameYUV->data[0]));
				//qDebug() << "\t  sws_scale over 3";
				//strcpy_s(buffer + pFrameYUV->width, pFrameYUV->width / 2, (char *)pFrameYUV->data[1]);
				//strcpy_s(buffer + pFrameYUV->width + pFrameYUV->width / 2, pFrameYUV->width / 2, (char *)pFrameYUV->data[2]);
				//qDebug() << "\t  copy over";




				if (SDL_UpdateYUVTexture(texture, &sdlRect,
					pFrameYUV->data[0], pFrameYUV->linesize[0],
					pFrameYUV->data[1], pFrameYUV->linesize[1],
					pFrameYUV->data[2], pFrameYUV->linesize[2]) == 0)
				{
					qDebug() << "\t  SDL_UpdateTexture over";

					SDL_RenderClear(render);
					qDebug() << "\t  SDL_RenderClear over";
					SDL_RenderCopy(render, texture, NULL, NULL);
					qDebug() << "\t  SDL_RenderCopy over";
					SDL_RenderPresent(render);
					qDebug() << "\t  SDL_RenderPresent over";
					SDL_Delay(40);

				}
			}
		}

		qDebug() << 10000 - count << "over";

		count--;
	}

	av_frame_free(&pFrame);
	av_frame_free(&pFrameYUV);
	av_free_packet(pPacket);

	avformat_free_context(pFormatctx);

	//---------------------------------------







	/*
	char *buffer;
	while (1){

	forread buffer;

	}
	*/

	// -------------------------------

	return 0;
    /*QApplication a(argc, argv);
    FileEncrypt w;
    w.show();
    return a.exec();*/
}
