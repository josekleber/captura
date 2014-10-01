#include "main.h"

/** \brief Sistema de Captura de Audios
    Captura através de streams web.
*/

//#define TESTE

#ifdef TESTE
#include <sys/resource.h>
struct strPP
{
    int a;
    int b;
};
#endif // TESTE

int main()
{
#ifdef TESTE
/**/
signal(SIGSEGV, Captura::signal_callback_handler1);

//  try
//  {
int* p = new int;
delete p;
printf(">>>> oia eu aqui 1\n");
*p = 10;
printf(">>>> oia eu aqui 2\n");
//  }
//  catch (SignalException& e)
//  {
//    printf(">>>> oia eu aqui 3\n");
//  }








// teste de monitoramento de memoria
struct rlimit limit;
if (getrlimit(RLIMIT_AS, &limit) != 0) {
    printf("getrlimit() failed with errno=%d\n", errno);
    exit(1);
  }

  printf("Limite default para tamanho de processo:\n");
  printf("O soft limit eh %ld MB\n", limit.rlim_cur/(1024*1024));       // ta dando -1
  printf("O hard limit eh %ld MB\n", limit.rlim_max/(1024*1024));       // ta dando -1

  if (getrlimit(RLIMIT_STACK, &limit) != 0) {
    printf("getrlimit() failed with errno=%d\n", errno);
    exit(1);
  }

  printf("\nLimite default para tamanho da area de pilha de um processo:\n");
  printf("O soft limit eh %ld MB\n", limit.rlim_cur/(1024*1024));
  printf("O hard limit eh %ld MB\n\n", limit.rlim_max/(1024*1024));      // ta dando -1

// testes de vector
vector<int> v01;
vector<int> v02;
vector<int> v03;
vector<vector<int>> v04;
int v05[] = {2, 3, 4, 5, 6, 7};
int v06[6];
int* v07;
vector<int> v08;
vector<vector<int>> v09;

queue <vector <int>> q01;
queue <vector<vector <int>>> q02;


v01 = vector<int>(std::begin(v05), std::end(v05));

q01.push(v01);
v08 = q01.front();
q01.pop();

v03 = v01;

v04.push_back(v01);
v04.push_back(v03);
v03.clear();

q02.push(v04);
v09 = q02.front();
q02.pop();

v02 = v01;

for (vector<int>::iterator it = v01.begin(); it != v01.end(); ++it)
    *it *= 2;

v07 = v02.data();

memcpy(&v06, v02.data(), v02.size() * sizeof(int));

v02[0] = 99;
printf(" i   v01  v02  v06  v07  v04[0]  v04[1]  v08  v09[0]  v09[1]\n");
for (int i = 0; i < v01.size(); i++)
    printf(" %d   %3d  %3d  %3d  %3d   %3d     %3d    %3d   %3d     %3d\n", i,
           v01[i], v02[i], v06[i], v07[i], v04[0][i], v04[1][i], v08[i], v09[0][i], v09[1][i]);

/**
AVSampleFormat sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_S16;
int nbChannel = 2;
int nbSamples = 1;

int bufSize;
int SampleSize;
int nbBuffers;
int alloc;

AVFifoBuffer **buf;

av_samples_get_buffer_size(&bufSize, nbChannel, nbSamples, sampleFormat, 1);
SampleSize = bufSize / nbSamples;
nbBuffers  = av_sample_fmt_is_planar(sampleFormat) ? nbChannel : 1;
alloc = nbBuffers * sizeof(*buf);

printf("\n\nTipo Planar:\n");
printf("bufSize: %d\n", bufSize);
printf("SampleSize: %d\n", SampleSize);
printf("nbBuffers: %d\n", nbBuffers);
printf("sizeof *buf: %d\n", sizeof(*buf));
printf("alloc: %d\n", alloc);

sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_S16P;
av_samples_get_buffer_size(&bufSize, nbChannel, nbSamples, sampleFormat, 1);
SampleSize = bufSize / nbSamples;
nbBuffers  = av_sample_fmt_is_planar(sampleFormat) ? nbChannel : 1;
alloc = nbBuffers * sizeof(*buf);

printf("\n\nTipo Planar:\n");
printf("bufSize: %d\n", bufSize);
printf("SampleSize: %d\n", SampleSize);
printf("nbBuffers: %d\n", nbBuffers);
printf("sizeof *buf: %d\n", sizeof(*buf));
printf("alloc: %d\n", alloc);


/**
vector <strPP*> teste;
strPP* pp = new strPP;
pp->a = 3;
pp->b = 4;
teste.push_back(pp);
//delete pp;
cout << "Vertor[0] : " << teste[0]->a << ", " << teste[0]->b << endl;
teste[0]->a *= 5;
cout << "pp : " << pp->a << ", " << pp->b << endl;
for (int i = 0; i < teste.size(); i++)
    delete teste[i];

teste.clear();

/**

// teste de excessoes
try
{
    throw GeneralException() << errno_code(MIR_DES_STMRADIO_1) << errmsg_info("Teste");
}
catch(GeneralException& err)
{
    cout << *boost::get_error_info<errno_code>(err) << endl << *boost::get_error_info<errmsg_info>(err);
}

/**

// teste de log
Logger* log = new Logger;
log->Teste();


//while(true)
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));

/**/

return 0;
#endif

    Captura* objCapura = new Captura;
}
