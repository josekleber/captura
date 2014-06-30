#include "testes.h"

int main(int argc, char **argv)
{
    Testes* objTestes = new Testes();

    string radio1, radio2;

    radio1 = "/home/nelson/Projetos/Musicas/66.mp3";
    radio1 = "mmsh://radio.tokhost.com.br/germaniafm";  // ******* Erro
    radio1 = "http://wms5.fabricahost.com.br:8386/;stream.nsv";                          // mp3
    radio1 = "http://184-107-102-140.webnow.net.br:80/98fm.aac";                         // aac
//    radio1 = "http://livestream-f.akamaihd.net/3172111_1948081_f9882103_1_1756@103114";  // h24      ***** ERRO
//    radio1 = "rtmp://media.sgr.globo.com:80/VideoMusicais/beat98.sdp";                   // flv      ***** Erro
//    radio1 = "mmsh://divinal.dnip.com.br:1380/divinal?MSWMExt=.asf";                     // wmav2    ***** ERRO
//    radio1 = "mmsh://portalradios15.dnip.com.br:1380/radiocruzeiro?MSWMExt=.asf";        // wmapro   ***** ERRO
//    radio1 = "rtsp://servidor10.dnip.com.br/87fmagudos";                                 // *****ERRO

//    radio2 = "/home/nelson/Projetos/Musicas/66_out_mp3.aac";
    radio2 = "http://wms5.fabricahost.com.br:8386/;stream.nsv";                          // mp3

//    objTestes->ffmpeg_teste(radio1, radio2);
    objTestes->ffmpeg_teste2();

    return 0;
}
