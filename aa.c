/*===============================================================
*   Copyright (C) 2023 All rights reserved.
*   
*   文件名称：business.cpp
*   创 建 者：chenbo
*   创建日期：2023年04月11日
*   描   述：
================================================================*/
#include "business.h"
#include "base64.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define BUFSIZE 4096
#define TEST_DEBUG 1
#define REV_DEBUG 0

int ret = 0;

SwsContext *vsc = NULL;
AVFrame *yuv = NULL;
AVCodecContext *vc = NULL;
AVFormatContext *ic = NULL;
AVStream *vs = NULL;
AVPacket pack;
int vpts = 0;

char *outUrl = "rtmp://127.0.0.1:1935/hls/";
string out_rtmp;

int get_time_ms(char *buff, int len)
{
    struct timeval tv;
    struct tm* ptm;
    char time_string[40];
    long milliseconds;
 
    if(buff == NULL)
    {
        printf("%s buff is NULL.\n", __func__);
        return -1;
    }
 
    gettimeofday(&tv, NULL);
 
    ptm = localtime (&(tv.tv_sec));
 
//    strftime (time_string, sizeof(time_string), "-%Y-%m-%d_%H:%M:%S", ptm);  //输出格式为: 2022-03-30 20:38:37
    strftime (time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", ptm);
    snprintf (buff, len, "%s", time_string); 
//    milliseconds = tv.tv_usec / 1000;
//    snprintf (buff, len, "%s.%03ld", time_string, milliseconds);            //输出格式为: 2022-03-30 20:38:37.182
    return 0;
}

void init_rtsp(int fFps, cv::Size sSize, const string &module_name, string video_id)
{
    //注册所有的编解码器
	avcodec_register_all();

	//注册所有的封装器
	av_register_all();

	//注册所有网络协议
	avformat_network_init();

	//像素格式转换上下文
//	SwsContext *vsc = NULL;

	//输出的数据结构
//	AVFrame *yuv = NULL;

	//编码器上下文
//	AVCodecContext *vc = NULL;

	//rtmp flv 封装器                                                           
//	AVFormatContext *ic = NULL;

    int inWidth = sSize.width;
    int inHeight = sSize.height;
    int fps = fFps;

    ///2 初始化格式转换上下文
    vsc = sws_getCachedContext(vsc,
        inWidth, inHeight, AV_PIX_FMT_BGR24,     //源宽、高、像素格式
        inWidth, inHeight, AV_PIX_FMT_YUV420P,//目标宽、高、像素格式
        SWS_BICUBIC,  // 尺寸变化使用算法
        0, 0, 0
        );
    if (!vsc)
    {
        printf("sws_getCachedContext failed!");
    }
    ///3 初始化输出的数据结构
    yuv = av_frame_alloc();
    yuv->format = AV_PIX_FMT_YUV420P;
    yuv->width = inWidth;
    yuv->height = inHeight;
    yuv->pts = 0;
    //分配yuv空间
    int ret = av_frame_get_buffer(yuv, 32);//字节与当前cpu对齐
    if (ret != 0)
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf)-1);
        //throw exception(buf);
        printf("buf:%s\n", buf);
    }

    ///4 初始化编码上下文
    //a 找到编码器
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec)
    {
        printf("Can`t find h264 encoder!");
    }
    //b 创建编码器上下文
    vc = avcodec_alloc_context3(codec);
    if (!vc)
    {
        printf("avcodec_alloc_context3 failed!");
    }
    //c 配置编码器参数
    vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER | AV_CODEC_FLAG_LOW_DELAY; //全局参数
    vc->codec_id = codec->id;
    vc->thread_count = 8;

    vc->bit_rate = 400 * 1024 * 8;//50 * 1024 * 8;//压缩后每秒视频的bit位大小 50kB
    vc->width = inWidth;
    vc->height = inHeight;
    vc->time_base = { 1, fps };
    vc->framerate = { fps, 1 };
    /*1 begin*/
    vc->lowres = codec->max_lowres;
    vc->flags2 |= AV_CODEC_FLAG2_FAST;
    /*1 end*/
    //实时推流，零延迟
    av_opt_set(vc->priv_data, "preset", "superfast", 0);
    av_opt_set(vc->priv_data, "tune", "zerolatency", 0);
    //画面组的大小，多少帧一个关键帧,一组图片的数量
    vc->gop_size = 50;
    //去掉B帧只留下 I帧和P帧
    vc->max_b_frames = 0;
    vc->pix_fmt = AV_PIX_FMT_YUV420P;

    //d 打开编码器上下文
    ret = avcodec_open2(vc, 0, 0);
    if (ret != 0)
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf)-1);
        printf("buf:%s\n", buf);
    }
    printf("avcodec_open2 success!");

    ///5 输出封装器和视频流配置
    //a 创建输出封装器上下文
    out_rtmp.clear();
    out_rtmp.append(outUrl);
    out_rtmp += module_name + "_" + video_id;// + "_" + video_name;
printf("out_rtmp:%s\n", out_rtmp.c_str());
    ret = avformat_alloc_output_context2(&ic, 0, "flv", out_rtmp.c_str());
    if (ret != 0)
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf)-1);
        printf("buf:%s\n", buf);
    }
    
    //b 添加视频流 
    vs = avformat_new_stream(ic, NULL);
    if (!vs)
    {
        printf("avformat_new_stream failed\n");
    }

    vs->codecpar->codec_tag = 0;
    //从编码器复制参数
    avcodec_parameters_from_context(vs->codecpar, vc);
    av_dump_format(ic, 0, out_rtmp.c_str(), 1);

    ///打开rtmp 的网络输出IO
    ret = avio_open(&ic->pb, out_rtmp.c_str(), AVIO_FLAG_WRITE);
    if (ret != 0)
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf)-1);
        printf("buf:%s\n", buf);
    }

    //写入封装头
    ret = avformat_write_header(ic, NULL);
    if (ret != 0)
    {
        char buf[1024] = { 0 };
        av_strerror(ret, buf, sizeof(buf)-1);
        printf("buf:%s\n", buf);
    }
}

int convert_rtsp(cv::Mat &src_img)
{
    memset(&pack, 0, sizeof(pack));

    //输入的数据结构
    uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };

    indata[0] = src_img.data;
    int insize[AV_NUM_DATA_POINTERS] = { 0 };
    //一行（宽）数据的字节数
    insize[0] = src_img.cols * src_img.elemSize();
    int h = sws_scale(vsc, indata, insize, 0, src_img.rows, //源数据
        yuv->data, yuv->linesize);
    if (h <= 0)
    {
        //printf("h:%d\n", h);
        return 0;//continue;
    }

    ///h264编码
    yuv->pts = vpts;
    vpts++;
    ret = avcodec_send_frame(vc, yuv);
    if (ret != 0)
        return 0;//continue;

    ret = avcodec_receive_packet(vc, &pack);
    if (ret != 0 || pack.size > 0)
    {
        //printf("%d\n",pack.size);
    }
    else
    {
        return 0;//continue;
    }
    //推流
    pack.pts = av_rescale_q(pack.pts, vc->time_base, vs->time_base);
    pack.dts = av_rescale_q(pack.dts, vc->time_base, vs->time_base);
    pack.duration = av_rescale_q(pack.duration, vc->time_base, vs->time_base);
    ret = av_interleaved_write_frame(ic, &pack);
    if (ret == 0)
    {
        //cout << "# pid:"<< getpid() << "\n" << flush;
    }
    return 0;
}

int release_rtsp(void)
{
    if (vsc)
    {
        sws_freeContext(vsc);
        vsc = NULL;
    }

    if (vc)
    {
        avio_closep(&ic->pb);
        avcodec_free_context(&vc);
    }

    if(yuv)
    {
        av_frame_free(&yuv);
    }

    if(ic)
    {
        avformat_free_context(ic);
    }
    return 0;
}


//imgType 包括png bmp jpg jpeg等opencv能够进行编码解码的文件
std::string Mat2Base64(const cv::Mat &image, std::string imgType) {
     //Mat转base64
     std::vector<uchar> buf;
     cv::imencode(imgType, image, buf);
     //uchar *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
     std::string img_data = base64_encode(buf.data(), buf.size(), false);
     return img_data;
}

cv::Mat Base2Mat(std::string &base64_data) {
     cv::Mat img;
     std::string s_mat;
     s_mat = base64_decode(base64_data.data(), false);
     std::vector<char> base64_img(s_mat.begin(), s_mat.end());
     img = cv::imdecode(base64_img, cv::IMREAD_COLOR); //CV::IMREAD_UNCHANGED
     return img;
}

void *thread_http(void *arg)
{
    pthread_detach(pthread_self());
    msg_info t_msg = *(msg_info *)arg;
    post_msg msg = ((msg_info *)arg)->msg;

    int length = t_msg.length;
    string ipaddr = t_msg.ipaddr;
    int port = t_msg.port;
    char *body = NULL;
	char *head = NULL;

printf("[%s][%s][%s][%s][%s][%d][%s][%d]\n", msg.CameraName.c_str(), msg.AlarmTime.c_str(), msg.AlgCode.c_str(), \
        msg.DeviceId.c_str(), msg.AlarmExtension.c_str(), /*msg.AlarmBase.c_str(),*/t_msg.length, t_msg.ipaddr.c_str(), t_msg.port );

    int sockfd, ret;
    struct sockaddr_in servaddr;
    char *str = NULL;

    socklen_t len;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            printf("1.socket 网络连接失败,本线程即将终止[socket error]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            delete (msg_info *)arg;
            printf("%ld exit!\n", pthread_self());
            pthread_exit((void *)"socket error!");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ipaddr.c_str(), &servaddr.sin_addr) <= 0 ){
            printf("2.inet 网络连接失败,本线程即将终止[inet_pton error]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            close(sockfd);
            delete (msg_info *)arg;
            printf("%ld exit!\n", pthread_self());
            pthread_exit((void *)"inet_pton error!");
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
            printf("3.connect 服务器失败[connect error]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            close(sockfd);
            delete (msg_info *)arg;
            printf("%ld exit!\n", pthread_self());
            pthread_exit((void *)"connect error!");
    }
    else
        printf("与远端建立了连接\n");

    body = (char *)malloc(length+500);
    head = (char *)malloc(length+1000);

    memset(body, 0, length+500);
    memset(head, 0, length+1000);

    sprintf(body, "{\"CameraName\":\"%s\",\"SiteData\":{\"Latitude\":\"16.24463,44.179439\",\"Longitude\":\"001\",\"Name\":\"001\"},\"ChannelName\":\"\",\"AlarmTime\":\"%s\",\"AlgCode\":\"%s\",\"DeviceId\":\"%s\",\"AlarmBoxs\":[{\"X\":1236,\"Y\":545,\"Height\":529,\"Width\":234},{\"X\":1419,\"Y\":509,\"Height\":337,\"Width\":126},{\"X\":1203,\"Y\":545,\"Height\":388,\"Width\":123}],\"AlarmExtension\":\"%s\",\"ChannelId\":\"eb5d32\",\"AlarmBase\":\"%s\"}",\
                    msg.CameraName.c_str(), msg.AlarmTime.c_str(), msg.AlgCode.c_str(), msg.DeviceId.c_str(), \
                    msg.AlarmExtension.c_str(), msg.AlarmBase.c_str());

    str=(char *)malloc(128);
    len = strlen(body);
    sprintf(str, "%d", len);

    strcat(head, "POST /api/smartbox/AlarmPost HTTP/1.1\r\n");
    strcat(head, "Cache-Control:no-cache\r\n");
    strcat(head, "Connection:Keep-Alive\r\n");
    strcat(head, "Accept-Encoding:gzip,deflate,br\r\n");
    strcat(head, "Accept:*/*\r\n");
    strcat(head, "Content-Type:application/json\r\n");
    strcat(head, "User-Agent:Mozilla/5.0\r\n");
    strcat(head, "host:127.0.0.1\r\n");
    strcat(head, "Content-Length:");
    strcat(head, str);
    strcat(head, "\r\n\r\n");
    //body的值为post的数据
    strcat(head, body);
    strcat(head, "\r\n\r\n");
//    printf("%s\n",head);

    ret = write(sockfd,head,strlen(head));
    if (ret < 0) {
            printf("4.send 发送失败！[write]错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            close(sockfd);
            free(body);
            free(head);
            free(str);
            body = NULL;
            head = NULL;
            str = NULL;
            delete (msg_info *)arg;
            printf("%ld exit!\n", pthread_self());
            pthread_exit((void *)"connect error!");
    }else{
            printf("消息发送成功，共发送了%d个字节! \n\n", ret);
    }

    close(sockfd);

    if(body != NULL)
        free(body);
    if(head != NULL)
        free(head);
    if(str != NULL)
        free(str);

    body = NULL;
    head = NULL;
    str = NULL;

    delete (msg_info *)arg;

printf("%ld exit!\n", pthread_self());
    pthread_exit((void *)"succeed end!");
    return (void *)0;
}

#if 0
int res_http(post_msg msg, int length, char *ipaddr, int port)
{
    int sockfd, ret, i, h;
    struct sockaddr_in servaddr;
    char *head, *body, buf[BUFSIZE], *str;

    socklen_t len;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            printf("创建网络连接失败,本线程即将终止---socket error!\n");
            return -1;
            //exit(0);
    };

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ipaddr, &servaddr.sin_addr) <= 0 ){
            printf("创建网络连接失败,本线程即将终止--inet_pton error!\n");
            //exit(0);
            return -1;
    };

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
            printf("连接到服务器失败,connect error!\n");
            //exit(0);
            return -1;
    }
    printf("与远端建立了连接\n");

    body = (char *)malloc(length+500);
    memset(body, 0, length+500);
/* 
	sprintf(body, "{\"CameraName\":\"%s\",\"SiteData\":{\"Latitude\":\"16.24463,44.179439\",\"Longitude\":\"001\",\"Name\":\"001\"},\"ChannelName\":\"\",\"AlarmTime\":\"%s\",\"AlgCode\":\"%s\",\"DeviceId\":\"%s\",\"AlarmBoxs\":[{\"X\":1236,\"Y\":545,\"Height\":529,\"Width\":234},{\"X\":1419,\"Y\":509,\"Height\":337,\"Width\":126},{\"X\":1203,\"Y\":545,\"Height\":388,\"Width\":123}],\"AlarmExtension\":\"%s\",\"ChannelId\":\"eb5d32\",\"AlarmBase\":\"%s\"}",\
		            msg.CameraName.c_str(), msg.AlarmTime.c_str(), msg.AlgCode.c_str(), msg.DeviceId.c_str(), \
                    msg.AlarmExtension.c_str(), msg.AlarmBase.c_str());
 */
    sprintf(body, "{\"CameraName\":\"%s\",\"AlarmTime\":\"%s\",\"AlgCode\":\"%s\",\"DeviceId\":\"%s\",\"AlarmExtension\":\"%s\",\"AlarmBase\":\"%s\"}",\
		            msg.CameraName.c_str(), msg.AlarmTime.c_str(), msg.AlgCode.c_str(), msg.DeviceId.c_str(), \
                    msg.AlarmExtension.c_str(), msg.AlarmBase.c_str());

    str=(char *)malloc(128);
    len = strlen(body);
    sprintf(str, "%d", len);

    head = (char *)malloc(length+1000);
    memset(head, 0, length+1000);
#if TEST_DEBUG
    strcat(head, "POST /api/smartbox/AlarmPost HTTP/1.1\r\n");
    strcat(head, "Cache-Control:no-cache\r\n");
    strcat(head, "Connection:Keep-Alive\r\n");
    strcat(head, "Accept-Encoding:gzip,deflate,br\r\n");
    strcat(head, "Accept:*/*\r\n");
    strcat(head, "Content-Type:application/json\r\n");
    strcat(head, "User-Agent:Mozilla/5.0\r\n");
    strcat(head, "host:127.0.0.1\r\n");
    strcat(head, "Content-Length:");
    strcat(head, str);
    strcat(head, "\r\n\r\n");
#else
    strcat(head, "POST /api/smartbox/AlarmPost HTTP/1.1\r\n");
    strcat(head, "Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(head, "Content-Length: ");
    strcat(head, str);
    strcat(head, "\r\n");
#endif
    //body的值为post的数据
    strcat(head, body);
    strcat(head, "\r\n\r\n");
//    printf("%s\n",head);

    ret = write(sockfd,head,strlen(head));
    if (ret < 0) {
            printf("发送失败！错误代码是%d, 错误信息是'%s'\n",errno, strerror(errno));
            //exit(0);
            close(sockfd);
            return -1;
    }else{
            printf("消息发送成功，共发送了%d个字节! \n\n", ret);
    }

#if REV_DEBUG
    while(1)
    {
        memset(buf, 0, 4096);
        i= read(sockfd, buf, 4095);
        if (i==0){
                close(sockfd);
                printf("读取数据报文时发现远端关闭，该线程终止！\n");
                return -1;
        }
        printf("buf:%s\n", buf);
        break;
    }
#endif

    close(sockfd);

    if(body != NULL)
        free(body);
    if(head != NULL)
        free(head);

    body = NULL;
    head = NULL;

    return 0;
}
#endif 
