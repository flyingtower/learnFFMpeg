//
//  main.cpp
//  myLive
//
//  Created by yeguo on 2019/3/9.
//  Copyright © 2019年 yeguo. All rights reserved.
//

#include <iostream>
#include "av_public.h"

using namespace std;

int main(int argc, const char * argv[]) {
    int iRet = 0;
    char errBuf[1024];
    cout<<av_version_info()<<endl;
    av_register_all();
    avfilter_register_all();
    avformat_network_init();
    AVFormatContext * myReadCtx = avformat_alloc_context();
    string tmpStreamUrl = "rtmp://ks-rtmplive.app-remix.com/live/84222842108987983";
    iRet = avformat_open_input(&myReadCtx, tmpStreamUrl.c_str(), NULL, NULL);
    if (iRet!=0) {
        av_log(NULL, AV_LOG_ERROR, "open failed:%s",av_make_error_string(errBuf,sizeof(errBuf),iRet));
        return iRet;
    }
    iRet = avformat_find_stream_info(myReadCtx, NULL);
    if (iRet!=0) {
        av_log(NULL, AV_LOG_ERROR, "find failed:%s",av_make_error_string(errBuf,sizeof(errBuf),iRet));
        return iRet;
    }
    
    AVFormatContext * myWriteCtx = NULL;
    string outUrl = "/Users/yeguo/Downloads/tmpW.flv";
    iRet = avformat_alloc_output_context2(&myWriteCtx, NULL, NULL, outUrl.c_str());
    if (iRet!=0) {
        av_log(NULL, AV_LOG_ERROR, "open write failed:%s",av_make_error_string(errBuf,sizeof(errBuf),iRet));
        return iRet;
    }
    
    iRet = avio_open(&myWriteCtx->pb, outUrl.c_str(), AVIO_FLAG_READ_WRITE);
    if (iRet!=0) {
        av_log(NULL, AV_LOG_ERROR, "open write failed:%s",av_make_error_string(errBuf,sizeof(errBuf),iRet));
        return iRet;
    }
    
    cout<<"myWriteCtx->nb_streams : "<<myWriteCtx->nb_streams<<endl;
    for (int i=0; i<myReadCtx->nb_streams; i++) {
        AVStream * avStream = avformat_new_stream(myWriteCtx, myReadCtx->streams[i]->codec->codec);
        iRet = avcodec_copy_context(avStream->codec, myReadCtx->streams[i]->codec);
        if (iRet!=0) {
            av_log(NULL, AV_LOG_ERROR, "copy failed:%s",av_make_error_string(errBuf,sizeof(errBuf),iRet));
            return iRet;
        }
    }
    
    iRet = avformat_write_header(myWriteCtx, NULL);
    if (iRet<0) {
        av_log(NULL, AV_LOG_ERROR, "write header failed:%s",av_make_error_string(errBuf,sizeof(errBuf),iRet));
        return iRet;
    }
    
    int i=0;
    while (i++<100) {
        AVPacket pkt;
        av_new_packet(&pkt, 1024*100);
        iRet = av_read_frame(myReadCtx, &pkt);
        if (iRet!=0) {
            av_log(NULL, AV_LOG_ERROR, "av_read_frame failed:%s",av_make_error_string(errBuf,sizeof(errBuf),iRet));
            return iRet;
        }
        
        AVStream *inputStream = myReadCtx->streams[pkt.stream_index];
        AVStream *outputStream = myWriteCtx->streams[pkt.stream_index];
        av_packet_rescale_ts(&pkt, inputStream->time_base, outputStream->time_base);
        av_write_frame(myWriteCtx, &pkt);
    }
    
    
    return 0;
}
