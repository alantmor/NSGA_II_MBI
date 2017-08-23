#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <stdint.h>
#include <float.h>

#define iA 843314861
#define iB 453816693
#define iM 1073741824

#define QTDESOLUCAO 100 /*QUANTIDADE DE INIVIDUOS*/
#define NUMGERACOES 1000
#define NUMEXECUCOES 1
#define MEDIA 50

#define PMUTACAO 0.7/*taxa de mutacao*/

/*DECLARAÇÃO DE VARIÁVEIS E PARAMETROS*/
#define N 1000
#define MU 0.016
#define B 3.5
#define Y 0.33
#define DT 0.1
#define TF 100

#define PROPINF 0.01
#define PROPSUS 0.9

int isem=1564854685;

typedef struct
{
   int estado;
}individuo;

typedef struct {
    float *valores;/*Vetor de itens*/
    float tk;/*Peso total do individuo*/
    int Qtditens;
    float funcObjetivo[2];
    int rank;
    float CrwDist;
}solucao;

typedef struct {
  int *IndSolucoes;
  int QtdeFrontItens;
  int Limites;/*INDICES DOS LIMITES MAXIMO E MINIMO, [F1MIN F1MAX F2MIN F2MAX]*/
}front;


/*int sem_inicial,sem_anterior;*/

int Valor_Max;
int K,Alpha=0;/*Capacidade da mochila, e valor de itens na sala*/


//int Escolhido[QTDEESCOLHA];/*vertor que contem o indice das mochilas escolhidas para cruzamento*/
//int rank[QTDESOLUCAO];

front *fronteiras;
int QtdeFronteira;


float rand0();
void Cruzamento(solucao *pvalores,solucao *qvalores);
int IniciaPopulacao(solucao *pvalores);
void Mutacao(solucao *pvalores);
void copiaIndiv(solucao *destino,int indDest, solucao* origem,int indOri);
void printIndividuo (solucao *pvalores, int Indice);
void MBI(solucao *pvalores,int indice);
void AvaliacaoFronteiras(solucao *rvalores,int Q);
void GeraNovaPopulacao(solucao *rvalores, solucao *pvalores);
void UniaoPQ(solucao *rvalores, solucao *pvalores, solucao *qvalores);
int lerQtd(const individuo Pop[], int state);

FILE *resultadog;

int main ()
{
   int i,j,geracao,execucao;
   /*sem_inicial=isem;*/
   /*int auxPesoF1,auxPesoF2;*/

    FILE *resultado;
    resultado=fopen("Resultado_NSGA_II_MBI.txt","w");
    if (!resultado) printf("Erro na abertura do arquivo.");

    resultadog=fopen("Fronteira_MBI.txt","w");
    if (!resultadog)
      printf("Erro na abertura do arquivo.");

      for (execucao=1;execucao<=NUMEXECUCOES;execucao++)
      {
        solucao *pvalores;
        pvalores=(solucao*)malloc(sizeof(solucao)*QTDESOLUCAO);/*Para alocar a struct objetos*/
        IniciaPopulacao(pvalores);

        for (geracao=1;geracao<=NUMGERACOES;geracao++)
        {

          solucao *qvalores, *rvalores;
          qvalores=(solucao*)malloc(QTDESOLUCAO*sizeof(solucao));/*Para alocar a struct objetos*/
          rvalores=(solucao*)malloc(QTDESOLUCAO*2*sizeof(solucao));/*Para alocar a struct objetos*/

          //printf("%d !!\n",cout++);
          Cruzamento(pvalores,qvalores);
          //printf("%d !!\n",cout++);
          Mutacao(qvalores);
          //printf("%d !!\n",cout++);
          UniaoPQ(rvalores,pvalores,qvalores);
          //printf("%d !!\n",cout++);
          AvaliacaoFronteiras(rvalores,2);/**/
          //printf("%d !!\n",cout++);
          GeraNovaPopulacao(rvalores,pvalores);
          //printf("%d !!\n",cout++);

          for (i=0;i<QTDESOLUCAO*2;i++)
          {
            free(rvalores[i].valores);
          }
          free(rvalores);
          for (i=0;i<QTDESOLUCAO;i++)
          {
            free(qvalores[i].valores);
          }
          free(qvalores);

          fprintf(resultadog,"resultado(%d).valores=[",geracao);
          for (i=0;i<QTDESOLUCAO;i++)
          {
            if (pvalores[i].rank==0)
            {
              fprintf(resultadog,"%f, %f;",pvalores[i].funcObjetivo[0],pvalores[i].funcObjetivo[1]);
            }
          }
          fprintf(resultadog,"];\n");


          //printf("%d !!\n",cout++);
          //system("pause");
        }/*FIM FOR GERACAO*/

        for (i=0,j=0;i<QTDESOLUCAO;i++)
        {
          if (pvalores[i].rank==0)
          {
            fprintf(resultado,"solucao(%d).ponto=[%f %f];\n",j,pvalores[i].funcObjetivo[0],pvalores[i].funcObjetivo[1]);
            fprintf(resultado,"solucao(%d).valores=[",j);
            for (j=0;j<pvalores[i].Qtditens;j++)
            {
               fprintf(resultado,"%f ",pvalores[i].valores[j]);
            }
            fprintf(resultado,"];");
            //fprintf(resultado,"%d - (%f, %f) \n",pvalores[i].rank,pvalores[i].funcObjetivo[1],pvalores[i].funcObjetivo[0]);
            j++;
          }
        }
        



        for(j=0;j<QTDESOLUCAO;j++)
        {
          free(pvalores[j].valores);
        }
        free(pvalores);

        fprintf(resultado,"\n");
      }/*FIM FOR EXECUCAO*/
      fclose(resultado);
      fclose(resultadog);
      return 0;
}

float rand0()
{
  float aux, x;
  aux = 0.5 / iM;
  isem = isem*iA + iB;
  if (isem < 0)
    isem = (isem + iM) + iM;

  x=isem*aux;
  return x;
}

int IniciaPopulacao(solucao *pvalores)/*OK-  FALTA TESTAR*/
{
    int QtdeItens=0,j,i,min=2,max=TF;
    for (j=0;j<QTDESOLUCAO;j++)
    {
       pvalores[j].tk=0;
       pvalores[j].funcObjetivo[0]=0;
       pvalores[j].funcObjetivo[1]=0;
       pvalores[j].Qtditens=0;

       QtdeItens=min+((int)(max-min)*(rand0()));
       pvalores[j].valores=(float*)calloc(QtdeItens,sizeof(float));
       if (pvalores[j].valores == NULL)
          {
             printf ("\n** Erro: Memoria Insuficiente 4 - %d**\n",QtdeItens);
             return 0;
             system("pause");
          }

       for (i=0;i<QtdeItens;i++)
       {
            pvalores[j].valores[i]=rand0();
       }
       pvalores[j].Qtditens=QtdeItens;
       pvalores[j].tk=TF*1.0/QtdeItens*1.0;
       MBI(pvalores,j);
       /*printf("Peso=%d  Beneficio=%d Função Objetivo=%lld\n",mochila[j].PesoInd,mochila[j].SomaBen,mochila[j].funcPen);*/
    }
    AvaliacaoFronteiras(pvalores,1);
    printf("População Iniciada...\n");
    return 0;
}

void Cruzamento(solucao *pvalores,solucao *qvalores)/*OK-  FALTA TESTAR*/
{
    int i,j,k1=0,k2=0,QtdeCruzamentos=0,numAleat=0;
    int auxQtde1,auxQtde2;
    int Pares[4];
    int Escolhido[2];

    QtdeCruzamentos=QTDESOLUCAO/2;

    for (i=0;i<QtdeCruzamentos;i++)/*A quantidade de cruzamentos é total de individuos escolhidos divididos por 2 */
    {
        for (j=0;j<4;j++)
        {
           Pares[j]=(int)(rand0()*QTDESOLUCAO);
        }

        for (j=0;j<2;j++)
        {
            if (pvalores[Pares[j*2]].rank==pvalores[Pares[(j*2)+1]].rank)
            {
                if ((pvalores[Pares[j*2]].CrwDist==pvalores[Pares[(j*2)+1]].CrwDist)&&(pvalores[Pares[j*2]].CrwDist!=0))
                   {
                     if (rand0()<0.5)
                     {
                        Escolhido[j]=Pares[j*2];
                     }
                     else
                     {
                        Escolhido[j]=Pares[(j*2)+1];
                     }
                   }
               else if (pvalores[Pares[j*2]].CrwDist<pvalores[Pares[(j*2)+1]].CrwDist)
               {
                  Escolhido[j]=Pares[j*2];
               }
               else if (pvalores[Pares[j*2]].CrwDist>pvalores[Pares[(j*2)+1]].CrwDist)
               {
                  Escolhido[j]=Pares[(j*2)+1];
               }
               else
               {
                  Escolhido[j] = (int)(rand0()*QTDESOLUCAO);/*Gambiarra para tentar evitar individuo repetidos*/
               }
            }
            else if (pvalores[Pares[j*2]].rank<pvalores[Pares[(j*2)+1]].rank)
            {
               Escolhido[j]=Pares[j*2];
            }
            else
            {
               Escolhido[j]=Pares[(j*2)+1];
            }
        }

        //printf("Cruzamento 1!\n");

        auxQtde1=pvalores[Escolhido[0]].Qtditens;
        auxQtde2=pvalores[Escolhido[1]].Qtditens;

        //printf("Cruzamento 2!\n");

        qvalores[(i*2)].valores=(float*)calloc(1,sizeof(float));
        if (qvalores[(i*2)].valores == NULL)
          {
             printf ("** Erro: Memoria Insuficiente 6**");
          }
        qvalores[(i*2)+1].valores=(float*)calloc(1,sizeof(float));
        if (qvalores[(i*2)+1].valores == NULL)
          {
             printf ("** Erro: Memoria Insuficiente 7**");
          }

        //printf("Cruzamento 3!\n");
        qvalores[(i*2)].tk=0;
        qvalores[(i*2)+1].tk=0;
        qvalores[i*2].Qtditens=0;
        qvalores[(i*2)+1].Qtditens=0;

        numAleat=rand0();
        j=0;
        k1=0;
        k2=0;
        //printf("Cruzamento 4!\n");
        while (auxQtde1)/*HERDANDO ALELOS DO PAI1*/
        {
            if (pvalores[Escolhido[0]].valores[j]>numAleat)
            {
               qvalores[i*2].valores = (float *) realloc (qvalores[i*2].valores, (k1+1)*sizeof(float));
               if (qvalores[i*2].valores == NULL)
                       {
                           printf ("** Erro: Memoria Insuficiente 8**");
                       }
               qvalores[i*2].valores[k1]=pvalores[Escolhido[0]].valores[j];
               qvalores[i*2].Qtditens++;
               k1++;
            }
            else
            {
               qvalores[(i*2)+1].valores = (float *) realloc (qvalores[(i*2)+1].valores, (k2+1)*sizeof(float));
               if (qvalores[(i*2)+1].valores == NULL)
                       {
                           printf ("** Erro: Memoria Insuficiente 9**");
                       }
               qvalores[(i*2)+1].valores[k2]=pvalores[Escolhido[0]].valores[j];
               qvalores[(i*2)+1].Qtditens++;
               k2++;
            }
            auxQtde1--;
            j++;
        }
        j=0;
        while (auxQtde2)/*HERDANDO ALELOS DO PAI2*/
        {
            if (pvalores[Escolhido[1]].valores[j]>numAleat)
            {


               qvalores[(i*2)+1].valores = (float *) realloc (qvalores[(i*2)+1].valores, (k2+1)*sizeof(float));
               if (qvalores[(i*2)+1].valores == NULL)
                       {
                           printf ("** Erro: Memoria Insuficiente 10**");
                       }
               qvalores[(i*2)+1].valores[k2]=pvalores[Escolhido[1]].valores[j];
               qvalores[(i*2)+1].Qtditens++;
               k2++;
            }
            else
            {
               qvalores[i*2].valores = (float *) realloc (qvalores[i*2].valores, (k1+1)*sizeof(float));
               if (qvalores[(i*2)+1].valores == NULL)
                    {
                       printf ("** Erro: Memoria Insuficiente 11*");
                    }
               qvalores[i*2].valores[k1]=pvalores[Escolhido[1]].valores[j];
               qvalores[i*2].Qtditens++;
               k1++;
            }
            auxQtde2--;
            j++;
        }
       qvalores[i*2].tk=(TF*1.0)/(qvalores[i*2].Qtditens*1.0);
       qvalores[(i*2)+1].tk=(TF*1.0)/(qvalores[(i*2)+1].Qtditens*1.0);

       /*printf("FO1 %lld ",Filhos[i*2].funcObjetivo);
        printf("FO2 %lld\n",Filhos[(i*2)+1].funcObjetivo);*/
    }
}

void Mutacao(solucao *qvalores) /*OK - FALTA TESTAR*/
{
    int i,j,QtdeAlelosMutaveis,AleloMutavel=0;
    float NewValor;

    for (i=0;i<QTDESOLUCAO;i++)
    {
        if(qvalores[i].Qtditens==0)
        {
            qvalores[i].Qtditens=1;
            qvalores[i].valores[AleloMutavel]=rand0();
            qvalores[i].tk=TF/2;
        }
        else
        {
            QtdeAlelosMutaveis=(int)(qvalores[i].Qtditens*PMUTACAO);
            for (j=1;j<=QtdeAlelosMutaveis;j++)
            {
                AleloMutavel=(int)(rand0()*(1.0*qvalores[i].Qtditens-1));
                NewValor=rand0();
                qvalores[i].valores[AleloMutavel]=NewValor;            /*Mudou se o indice*/
            }
        }
        MBI(qvalores,i);
    }
}

void printIndividuo (solucao *pvalor, int Indice)/*OK - FALTA TESTAR*/
{
    int i;
    for (i=0;i<pvalor[Indice].Qtditens;i++)
    {
       printf("%f ",pvalor[Indice].valores[i]);
    }

    printf("\n");
}

void MBI(solucao *pvalores,int indice)/*OK - FALTA TESTAR*/
{

  pvalores[indice].funcObjetivo[0]=0.0;
  pvalores[indice].funcObjetivo[1]=0.0;


  int i,media,j;
  float t,tau;
  int n,ind,status;
  int somatorioI=0;
  int somatorioVk=0;

  ind=(int)(TF/DT);
  int S[ind],I[ind],R[ind];

  int propinf,posinf,posrec,proprec,indiceP;
  propinf=(int)N*PROPINF; /*quantidade de infectados*/
  proprec=(int)N*(1-PROPSUS-PROPINF);


   for (media=0,j=0;media<MEDIA;media++)
   {
      individuo *P;
      P=(individuo*)malloc(sizeof(individuo)*N);

      for (i=0;i<N;i++)
      {
        P[i].estado=0;
      }
      for (i=0;i<propinf;i++)
      {
        posinf=rand0()*N;
        P[posinf].estado=1;
      }
      for (i=0;i<proprec;i++)
      {
        posrec=rand0()*N;
        P[posrec].estado=2;
      }

      S[j]=lerQtd(P,0);
      I[j]=lerQtd(P,1);
      R[j]=lerQtd(P,2);
      /*fprintf(resultado,"%d %d %d\n",S[j],I[j],R[j]);*/
      tau=pvalores[indice].tk/2;
      /*fprintf(resultado,"S I R\n");*/
      for(t=DT,j=1,indiceP=-1;t<TF;t+=DT,j++)
      {
         status=1;
         if (t>=tau)
         {
           if (indiceP<pvalores[indice].Qtditens)
           {
              indiceP++;
              status=0;
              tau+=pvalores[indice].tk;
           }
           else status=1;
         }

        /*printf("%f \n",sorteio);*/
        for (n=0;n<N;n++)/*Varre os Individuos*/
        {
          if (rand0()<MU*DT)/*MORTE e NASCIMENTO*/
          {
             P[n].estado=0;
          }
          else if((rand0()<pvalores[indice].valores[indiceP])&&(P[n].estado==0)&&(!status))/*Vacinação da população*/
             {
               P[n].estado=2;
               somatorioVk++;
             }
          else if (P[n].estado==1)/*RECUPERAÇÃO*/
             {
                for (i=0;i<N;i++)/*PERCORRENDO A VIZINHANÇA DE N, REDE REGULAR*/
                {
                  if ((i!=n)&&(rand0()<((B*DT)/(N-1)))&&(P[i].estado==0))
                     P[i].estado=1;/*ESTÁ INFECTANDO O VIZINHO*/
                }/*FIM FOR VARREDURA*/
                if (rand0()<Y*DT) /*RECUPERAÇÃO DO INDIVIDUO  */
                {
                   P[n].estado=2;
                }
             }

         }
          /*printf("Estado:%d Indice:%d Iteracao no tempo: %d\n",Paux[n],n,j);*/
           S[j]=lerQtd(P,0);
           I[j]=lerQtd(P,1);

          somatorioI+=(I[j]);/*INTEGRAL DO INFECTADOS*/

          /*if (!status)
          {
              if (indiceP<pvalores[indice].Qtditens)
              {
                  somatorioSk+=((int)(S[j]*pvalores[indice].valores[indiceP]));
              }
         }*/
   }/*FIM DO FOR DE TEMPO*/
    free(P);
   }//FIM DO FOR MEDIA
  pvalores[indice].funcObjetivo[1]=somatorioI/MEDIA;
  pvalores[indice].funcObjetivo[0]=(somatorioVk)/MEDIA;

} /*FIM DO function*/

void AvaliacaoFronteiras(solucao *rvalores,int Q)
{
  //printf("Teste 0\n");
  int i,j,p,q,numVariaveis=2,parada,auxindnp=0,indP=0,nq;
  //printf("Teste 0a\n");
  int flag1,flag2;
  //printf("Teste 0b\n");
  int **Sp=(int**)malloc(QTDESOLUCAO*Q*sizeof(int*));
  for (i=0;i<QTDESOLUCAO*Q;i++)
  {
    Sp[i] = (int*)malloc(QTDESOLUCAO*Q*sizeof(int*));
    for (j = 0; j < QTDESOLUCAO*Q; j++)
    { //Percorre o Vetor de Inteiros atual.
            Sp[i][j] = 0; //Inicializa com 0.
     }
  }

  //printf("Teste 0c\n");
  int QtdeSp[QTDESOLUCAO*Q];
  //printf("Teste 0d\n");
  int np[QTDESOLUCAO*Q];
  //printf("Teste 0e\n");

  QtdeFronteira=0;
  for (i=0;i<(QTDESOLUCAO*Q);i++)
  {
    QtdeSp[i]=0;
  }

 //printf("Teste 1 \n");
  fronteiras=(front*)calloc(QTDESOLUCAO*Q,sizeof(front));
  fronteiras[0].QtdeFrontItens=0;
  fronteiras[0].IndSolucoes=(int*)calloc(QTDESOLUCAO*Q,sizeof(int));
  //printf("Teste 2 \n");
  for (p=0;p<QTDESOLUCAO*Q;p++)/*PARA CADA Q PERTENCE P*/
  {
    np[p]=0;
    nq=0;
    /*###################################################*/
    /*printf("\n----------------------------------\n");
    printf("Valores que o Elemento %d domina:\n",p);*/
    /*###################################################*/
        for(q=0;q<QTDESOLUCAO*Q;q++)
    {
        if (p!=q)
        {
            flag1 = 0;
            flag2 = 0;
            for (i=0; i<numVariaveis; i++)
            {
                if (rvalores[p].funcObjetivo[i] < rvalores[q].funcObjetivo[i])
                {
                    flag1 = 1;
                }
                else if (rvalores[p].funcObjetivo[i] > rvalores[q].funcObjetivo[i])
                     {
                       flag2 = 1;
                     }
            }/*FIM FOR I*/

            if (flag1==0 && flag2==1)/*P É DOMINADO POR Q*/
            {
              np[p]++;/*NUMERO DE SOLUÇÕES QUE DOMINAM P*/
            }
            else if (flag1==1 && flag2==0)/*P DOMINA Q*/
            {
               Sp[p][nq]=q;/*CONJUNTO DE SOLUÇÕES QUE P DOMINA*/
               nq++;
               /*printf("%d ",q);*/
            }

        }/*FIM if(j!=k)*/

    }/*FIM FOR q*/

    QtdeSp[p]=nq;


    if (np[p]==0)
    {
        fronteiras[0].QtdeFrontItens++;
        fronteiras[0].IndSolucoes[fronteiras[0].QtdeFrontItens-1]=p;
        rvalores[p].rank=0;

    }
  }/*FIM FOR p*/
  //printf("Teste 3 \n");
    /*###################################################*/
    /*fprintf(resultadog,"ELMENTOS DA FRONTEIRA 1:\n");
    for (i=0;i<fronteiras[0].QtdeFrontItens;i++)
    {
        fprintf(resultadog,"%d (%f, %f) \n",fronteiras[0].IndSolucoes[i],rvalores[fronteiras[0].IndSolucoes[i]].funcObjetivo[1],rvalores[fronteiras[0].IndSolucoes[i]].funcObjetivo[0]);
    }
    fprintf(resultadog,"\n");
    fprintf(resultadog,"----------------------------------\n");*/

 /*###################################################*/

  parada=(QTDESOLUCAO*Q)-(fronteiras[0].QtdeFrontItens);
  /*printf("%d \n",fronteiras[0].QtdeFrontItens);*/
 // printf("Teste 1 \n");
  while(parada)
  {
     QtdeFronteira++;
     fronteiras = (front *) realloc (fronteiras, (QtdeFronteira+1)*sizeof(front));
     fronteiras[QtdeFronteira].IndSolucoes=(int*)calloc(QTDESOLUCAO*Q,sizeof(int));
     fronteiras[QtdeFronteira].QtdeFrontItens=0;/*AUMENTA O NUMERO DAS FRONTEIRAS*/
     for (i=0;i<fronteiras[QtdeFronteira-1].QtdeFrontItens;i++)/* VARRENDO OS ITENS DA FRONTEIRA - QUANTIDADE DE ITENS DAS FRONTEIRAS*/
     {
            auxindnp=0;
            indP=fronteiras[QtdeFronteira-1].IndSolucoes[i];
            if (QtdeSp[indP])
            {
                for (q=0;q<(QtdeSp[indP]);q++)/*VARRE OS ITENS QUE DOMINAM O ITEM CORRENTE*/
                {
                    auxindnp = Sp[indP][q];
                    np[auxindnp]--;/*DECREMENTA O CONTADOR DE QUANTIDADE DE ITENS QUE DOMINAM O ITEM CORRENTE QUE ESTÃO NO VETOR SP*/
                    if (np[auxindnp]==0)
                    {
                        fronteiras[QtdeFronteira].QtdeFrontItens++;
                        //printf("passou avaliacao4\n");
                        //printf("%d %d %d\n",QtdeFronteira,(fronteiras[QtdeFronteira].QtdeFrontItens)-1,auxindnp);
                        fronteiras[QtdeFronteira].IndSolucoes = (int *) realloc (fronteiras[QtdeFronteira].IndSolucoes, (fronteiras[QtdeFronteira].QtdeFrontItens+1)*sizeof(int));
                        //printf("passou avaliacao5\n");
                        fronteiras[QtdeFronteira].IndSolucoes[(fronteiras[QtdeFronteira].QtdeFrontItens)-1]=auxindnp;
                        //printf("passou avaliacao6\n");
                        rvalores[auxindnp].rank=QtdeFronteira;
                    }
                }
            }
      }
      parada-=(fronteiras[QtdeFronteira].QtdeFrontItens);
      /*printf("%d ",parada);*/
      /*###################################################*/
      /*fprintf(resultadog,"ELEMENTOS DA FRONTEIRA %d:\n",QtdeFronteira+1);
      for (i=0;i<fronteiras[QtdeFronteira].QtdeFrontItens;i++)
      {
           fprintf(resultadog,"%d (%f, %f) \n",fronteiras[QtdeFronteira].IndSolucoes[i],rvalores[fronteiras[QtdeFronteira].IndSolucoes[i]].funcObjetivo[1],rvalores[fronteiras[QtdeFronteira].IndSolucoes[i]].funcObjetivo[0]);
      }
      fprintf(resultadog,"----------------------------------\n");*/
      /*###################################################*/
  }
  for (i=0;i<QTDESOLUCAO*Q;i++)
  {
    free(Sp[i]);
  }
  free(Sp);
  //printf("passou avaliacaoFim\n");
}

void GeraNovaPopulacao(solucao *rvalores, solucao *pvalores)
{

  int i,j,k=0,m,n;
  int auxContFront=0,auxTotalSolucao=0,auxStatus=1,diferenca;
  int troca=1;
  int memoria=0;

  /*CALCULO DA CROWDISTANCE*/

    for (i=0;i<QtdeFronteira;i++)
    {
       for (j=0;j<fronteiras[i].QtdeFrontItens;j++)
       {
           rvalores[fronteiras[i].IndSolucoes[j]].CrwDist=0;
       }

       for  (m=0;m<2;m++)/*m -  quantidade de funções objetivo*/
       {
          troca=1; /*A variável "troca" será a verificação da troca em cada passada*/
	      for(j=fronteiras[i].QtdeFrontItens-1; (j>=1) && (troca==1); j--)/*Troca serve para parar o for se nao houver  troca hora nenhuma*/
          {
                troca=0; /*Se o valor continuar 0 na próxima passada quer dizer que não houve troca e a função é encerrada.*/
                for(k=0; k<j; k++)
                {
                    if(rvalores[fronteiras[i].IndSolucoes[k]].funcObjetivo[m]>rvalores[fronteiras[i].IndSolucoes[k+1]].funcObjetivo[m])
                    {
                        memoria=fronteiras[i].IndSolucoes[k];
                        fronteiras[i].IndSolucoes[k]=fronteiras[i].IndSolucoes[k+1];
                        fronteiras[i].IndSolucoes[k+1]=memoria;
                        troca=1; /*Se houve troca, "troca" recebe 1 para continuar rodando.*/
                    }
                }
          }

          rvalores[fronteiras[i].IndSolucoes[0]].CrwDist=INFINITE;
          rvalores[fronteiras[i].IndSolucoes[fronteiras[i].QtdeFrontItens-1]].CrwDist=INFINITE;
          for (n=1;n<fronteiras[i].QtdeFrontItens-1;n++)
          {
            rvalores[fronteiras[i].IndSolucoes[n]].CrwDist=rvalores[fronteiras[i].IndSolucoes[n]].CrwDist+((rvalores[fronteiras[i].IndSolucoes[n+1]].funcObjetivo[m]-rvalores[fronteiras[i].IndSolucoes[n-1]].funcObjetivo[m])/(rvalores[fronteiras[i].IndSolucoes[fronteiras[i].QtdeFrontItens-1]].funcObjetivo[m]-rvalores[fronteiras[i].IndSolucoes[0]].funcObjetivo[m]));
          }
       }/*FIM FOR M*/

       if (auxStatus)/*faz isso apenas uma vez quando a quatidade de itens excede o tamanho da solução */
       {
          auxContFront+=fronteiras[i].QtdeFrontItens;
          if (auxContFront>QTDESOLUCAO)
          {
            auxTotalSolucao=i;/*FRONTEIRA*/
            auxContFront-=fronteiras[i].QtdeFrontItens;
            auxStatus=0;
            /*breqk;*/
          }
       }

    }/*FIR FOR(I=0..*/
    /*FIM DA CALCULO DA CROWDISTANCE*/
    for(i=0,k=0;i<auxTotalSolucao;i++)
    {
        for (j=0;j<fronteiras[i].QtdeFrontItens;j++,k++)
        {
           copiaIndiv(pvalores,k,rvalores,fronteiras[i].IndSolucoes[j]);
        }
    }
    diferenca=k;

    troca=1; /*A variável "troca" será a verificação da troca em cada passada*/
    /*FAZ UMA ORGANIZAÇÃO DA ULTIMA FRONTEIRA PARA SER INSERIDA NA POPULAÇÃO DE ACORDO COM O CRWO DISTANCE*/
	for(j=fronteiras[auxTotalSolucao].QtdeFrontItens-1; (j>=1) && (troca==1); j--)
    {
        troca=0; /*Se o valor continuar 0 na próxima passada quer dizer que não houve troca e a função é encerrada.*/
        for(k=0; k<j; k++)
        {
             if(rvalores[fronteiras[auxTotalSolucao].IndSolucoes[k]].CrwDist<rvalores[fronteiras[auxTotalSolucao].IndSolucoes[k+1]].CrwDist)
             {
                  memoria=fronteiras[auxTotalSolucao].IndSolucoes[k];
                  fronteiras[auxTotalSolucao].IndSolucoes[k]=fronteiras[auxTotalSolucao].IndSolucoes[k+1];
                  fronteiras[auxTotalSolucao].IndSolucoes[k+1]=memoria;
                  troca=1; /*Se houve troca, "troca" recebe 1 para continuar rodando.*/
             }
        }
    }
    /*printf("%d ",diferenca);*/
    for (j=0,k=diferenca;(k<QTDESOLUCAO);j++,k++)
    {
           copiaIndiv(pvalores,k,rvalores,fronteiras[auxTotalSolucao].IndSolucoes[j]);
    }


   /* for (j=0;j<fronteiras[auxTotalSolucao].QtdeFrontItens;j++)
    {
        printf("%f ",rvalores[fronteiras[auxTotalSolucao].IndSolucoes[j]].CrwDist);
    }
     printf("\n");*/

    for (i=0;i<QtdeFronteira;i++);
    {
      free(fronteiras[i].IndSolucoes);
    }
    free(fronteiras);

}

void UniaoPQ(solucao *rvalores, solucao *pvalores, solucao *qvalores)
{
    int i,p,q,aux=0;

    for (i=0;i<(QTDESOLUCAO*2);i++)
    {
       rvalores[i].valores=(float*)calloc(TF,sizeof(float));
       if (rvalores[i].valores == NULL)
       {
         printf ("\n**UniaoPQ : Memoria Insuficiente**\n");
       }
    }
    for (p=0;p<QTDESOLUCAO;p++)
    {
        copiaIndiv(rvalores,p,pvalores,p);
    }
    for (q=0,aux=QTDESOLUCAO;q<QTDESOLUCAO;q++,aux++)
    {
        copiaIndiv(rvalores,aux,qvalores,q);
    }
}

void copiaIndiv(solucao* destino,int indDest, solucao* origem,int indOri)/*OK - FALTA TESTAR*/
{
      int i;

      destino[indDest].tk=origem[indOri].tk;
      destino[indDest].rank=origem[indOri].rank;
      destino[indDest].CrwDist=origem[indOri].CrwDist;
      destino[indDest].funcObjetivo[0]=origem[indOri].funcObjetivo[0];
      destino[indDest].funcObjetivo[1]=origem[indOri].funcObjetivo[1];

      //printf("Destino:%d Origem:%d\n",indDest,indOri);

      destino[indDest].Qtditens=origem[indOri].Qtditens;
      destino[indDest].valores = ( float * ) realloc (destino[indDest].valores,(destino[indDest].Qtditens) * sizeof(float));
      if (destino[indDest].valores == NULL)
      {
          printf ("\n** copiaIndiv:1- Erro: Memoria Insuficiente Copia**\n");
      }
      for(i=0;i<destino[indDest].Qtditens;i++)
      {
          destino[indDest].valores[i]=origem[indOri].valores[i];
      }
      //printf("copiou\n");
}

int lerQtd(const individuo Pop[], int state)/*OK*/
{
   int k,contador=0;
   for (k=0;k<N;k++)
    {
      if (Pop[k].estado==state)
         contador++;
    }
   return contador;
}
