#include "processadorsh.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <math.h>

using namespace std;

ProcessadorSerieHistorica::ProcessadorSerieHistorica(int _janela_max, string atributo){
    this->janela = _janela_max;
    this->atributo = atributo;
    this->attcriados = false;
    diferenca_i = new float[_janela_max];
}

ProcessadorSerieHistorica::~ProcessadorSerieHistorica(){
    delete []diferenca_i; // When done, free memory pointed to by a.
    diferenca_i = NULL; // Clear a to prevent using invalid memory reference.
}

void ProcessadorSerieHistorica::atualizarAtributo(string att){
    this->atributo = att;
}

bool ProcessadorSerieHistorica::normalizacao_min_max(Corpus &objCorpus, int iColNormalizar, float _min_norm, float _max_norm){
    int qtdConjExemplos, c, totlinhas, linhai;
    qtdConjExemplos = objCorpus.pegarQtdConjExemplos();
    float _max =-999999999999, _min = 999999999999;


    for (c=0; c<qtdConjExemplos; c++){
        totlinhas = objCorpus.pegarQtdExemplos(c); //sempre = 1
        for (linhai=0; linhai < totlinhas; linhai++){

                int  vatual= objCorpus.pegarValor(c,linhai,iColNormalizar);
                float valor_atual;
               (std::istringstream)(objCorpus.pegarSimbolo(vatual)) >> valor_atual >> std::dec;//converte para float
                if (valor_atual > _max) {
                    _max = valor_atual;
                }
                if (valor_atual < _min){
                    _min = valor_atual;
                }

            }
    }


    for (c=0; c<qtdConjExemplos; c++){
        totlinhas = objCorpus.pegarQtdExemplos(c); //sempre = 1
        for (linhai=0; linhai < totlinhas; linhai++){

                //pega o valor do atributo
                int  vatual= objCorpus.pegarValor(c,linhai,iColNormalizar);

                //converte para float
                float valor_atual;
                (std::istringstream)(objCorpus.pegarSimbolo(vatual)) >> valor_atual >> std::dec;//converte para float

                //normaliza
                if (_max == _min){
                    valor_atual = 0.5;
                }else{
                    valor_atual = _min_norm + (((valor_atual - _min) / (_max - _min)) * (_max_norm - _min_norm));
                }
                //converte pra texto
                std::stringstream out2;
                //out2 << setprecision(2) << setiosflags(ios::fixed);
                out2 << valor_atual;

                //salva
                objCorpus.ajustarValor(c,linhai,iColNormalizar, objCorpus.pegarIndice(out2.str()));
            }
        }

}

bool ProcessadorSerieHistorica::normalizacao_padronizacao(Corpus &objCorpus, int iColNormalizar){
    int qtdConjExemplos, c, totlinhas, linhai;
    qtdConjExemplos = objCorpus.pegarQtdConjExemplos();
    float soma = 0;
    float media = 0;
    float variancia = 0;
    float desvio_padrao = 0;

    //calculo da media
    for (c=0; c<qtdConjExemplos; c++){
        totlinhas = objCorpus.pegarQtdExemplos(c); //sempre = 1
        for (linhai=0; linhai < totlinhas; linhai++){
            int  vatual= objCorpus.pegarValor(c,linhai,iColNormalizar);
            float valor_atual;
            (std::istringstream)(objCorpus.pegarSimbolo(vatual)) >> valor_atual >> std::dec;//converte para float
            soma+=valor_atual;
        }
    }
    media = soma / qtdConjExemplos;

    //calculo da variancia
    for (c=0; c<qtdConjExemplos; c++){
        totlinhas = objCorpus.pegarQtdExemplos(c); //sempre = 1
        for (linhai=0; linhai < totlinhas; linhai++){
            int  vatual= objCorpus.pegarValor(c,linhai,iColNormalizar);
            float valor_atual;
            (std::istringstream)(objCorpus.pegarSimbolo(vatual)) >> valor_atual >> std::dec;//converte para float
            variancia = variancia + ((pow((valor_atual - media),2)) / (qtdConjExemplos - 1));
        }
    }

    desvio_padrao = sqrt(variancia);

    //normalizacao
    for (c=0; c<qtdConjExemplos; c++){
        totlinhas = objCorpus.pegarQtdExemplos(c); //sempre = 1
        for (linhai=0; linhai < totlinhas; linhai++){

                //pega o valor do atributo
                int  vatual= objCorpus.pegarValor(c,linhai,iColNormalizar);

                //converte para float
                float valor_atual;
                (std::istringstream)(objCorpus.pegarSimbolo(vatual)) >> valor_atual >> std::dec;//converte para float

                valor_atual = (valor_atual - media) / desvio_padrao;

                //converte pra texto
                std::stringstream out2;
                //out2 << setprecision(2) << setiosflags(ios::fixed);
                out2 << valor_atual;

                //salva
                objCorpus.ajustarValor(c,linhai,iColNormalizar, objCorpus.pegarIndice(out2.str()));
            }
        }

}


void ProcessadorSerieHistorica::criarAtributosAuxiliares(Corpus &objCorpus, int janela_ini, int janela_fim){

    int d;

    this->novosAtributos.clear();
    for (d=janela_ini; d<=janela_fim; d++)
    {
        stringstream out;
        out << d;
        objCorpus.criarAtributo("d-" + out.str(), "0");
        this->novosAtributos.push_back("d-" + out.str());
    }
    //variavel alvo (y) deve ser a ultima por conta de implementacao da regressao logistica
    objCorpus.criarAtributo("y","0");
    objCorpus.criarAtributo("saida_bls","0");
    objCorpus.criarAtributo("saida_nb","0");
    objCorpus.criarAtributo("saida_reglog","0");
    objCorpus.criarAtributo("saida_svm","0");

    //Este atributo controla a sensibilidade pela qual vamos trabalhar.
    //Pequenas oscila��es (tanto no acr�scimo quanto no decrescimo) ser�o desconsideradas atrav�s de um fator de sensibilidade
    //objCorpus.criarAtributo("considerar", "0");

    //n�o adiciona y nos novos Atributos (n�o deve ser treinado com ele)
}

vector<string> ProcessadorSerieHistorica::processarCorpus(Corpus &objCorpus, int janela_max){

    this->janela_max = janela_max;

    int totlinhas, qtdConjExemplos, c, pos, neg;
    int  d, ipreco, idColDiferenca_i,  iY, linhai, coldif, iSaidaNB,iSaidaBLS,iSaidaSVM,iSaidaRegLog,iConsiderar;

    pos = objCorpus.pegarIndice("+1");
    neg = objCorpus.pegarIndice("-1");

    ipreco = objCorpus.pegarPosAtributo(this->atributo);
    iY = objCorpus.pegarPosAtributo("y");

//    int iAbertura = objCorpus.pegarPosAtributo("abertura");
//    int iMax = objCorpus.pegarPosAtributo("maximo");
//    int iMin = objCorpus.pegarPosAtributo("minimo");

    iSaidaNB = objCorpus.pegarPosAtributo("saida_nb");
    iSaidaBLS = objCorpus.pegarPosAtributo("saida_bls");
    iSaidaSVM = objCorpus.pegarPosAtributo("saida_svm");
    iSaidaRegLog = objCorpus.pegarPosAtributo("saida_reglog");
    /*
    iConsiderar = objCorpus.pegarPosAtributo("considerar");
    */

    for (d=0; d<janela; d++){
        diferenca_i[d] = 0;
    }

    qtdConjExemplos = objCorpus.pegarQtdConjExemplos();
    //tive que alterar aqui pq agora cada registro eh considerado um conjunto de exemplos!
    for (c=0; c<qtdConjExemplos; c++){
        totlinhas = objCorpus.pegarQtdExemplos(c); //sempre = 1

        //preenche os valores das diferen�as
        for (linhai=0; linhai < totlinhas; linhai++){

            //atualiza os valores do vetor de diferencas
            for (coldif=janela - 1;coldif > 0; coldif--){
                diferenca_i[coldif] = diferenca_i[coldif - 1];
            }

            //obtem o valor atual do ativo
            int  vatual= objCorpus.pegarValor(c,linhai,ipreco);
            float valor_atual;
            (std::istringstream)(objCorpus.pegarSimbolo(vatual)) >> valor_atual >> std::dec;//converte para float

            //obtem o valor de ontem do ativo
            int  v_di;
            float valor_di, valor_futuro;
            //if (linhai > 0){
            if (c > 0){ //alterei aqui por conta da separacao das linhas por conjunto de exemplos
                //v_di = objCorpus.pegarValor(c, linhai - 1, ipreco);
                v_di = objCorpus.pegarValor(c - 1, linhai , ipreco);
                (std::istringstream)(objCorpus.pegarSimbolo(v_di)) >> valor_di >> std::dec;//converte para float
            }
            else{
                valor_di = valor_atual;
            }

            //if (linhai != totlinhas - 1){
            if (c != qtdConjExemplos - 1){
                //v_di = objCorpus.pegarValor(c, linhai + 1, ipreco);
                v_di = objCorpus.pegarValor(c + 1, linhai , ipreco);
                (std::istringstream)(objCorpus.pegarSimbolo(v_di)) >> valor_futuro >> std::dec;//converte para float
            }
            else{
                valor_futuro = valor_atual;
            }

            //d-1
            diferenca_i[0] = valor_atual - valor_di;

            objCorpus.ajustarValor(c, linhai, iY, (valor_futuro > valor_atual)?pos:neg);
            //testando uma parada...
            //objCorpus.ajustarValor(c, linhai, iY, (valor_futuro > (valor_atual * 1.03))?pos:neg);

            /*
            float fator_sensibilidade = 0.05; //ver aqui porque possivelmente esse fator ter� que ser diferente para cada ativo

            int iUm, iZero;
            iUm = objCorpus.pegarIndice("1");
            iZero = objCorpus.pegarIndice("0");

            if ((diferenca_i[0] > 0.0) && (valor_atual * fator_sensibilidade) <= diferenca_i[0])  {
                objCorpus.ajustarValor(c, linhai, iConsiderar, iZero); //subiu mais que o fator, considera
            }else if ((diferenca_i[0] > 0.0) && (valor_atual * fator_sensibilidade) > diferenca_i[0]){
                objCorpus.ajustarValor(c, linhai, iConsiderar, iUm); //nao subiu mais que o fator, nao considera
            }else if ((diferenca_i[0] < 0.0) && ((valor_atual * fator_sensibilidade) * -1) > diferenca_i[0]) {
                objCorpus.ajustarValor(c, linhai, iConsiderar, iZero); //caiu mais que o fator, considera
            }else{
                objCorpus.ajustarValor(c, linhai, iConsiderar, iUm); //nao caiu mais que o fator, nao considera
            }
            */


            //preenche os valores das diferen�as
            for (coldif=1;coldif <= janela ; coldif++){
                std::string s;
                std::stringstream out;
                out << coldif;
                s = "d-" + out.str();

                idColDiferenca_i = objCorpus.pegarPosAtributo(s);

                std::stringstream out2;
                out2 << setprecision(2) << setiosflags(ios::fixed);
                out2 << diferenca_i[coldif-1];

                objCorpus.ajustarValor(c,linhai,idColDiferenca_i, objCorpus.pegarIndice(out2.str()));

            }

            //zerando novamente as colunas de saidas dos algoritmos
            objCorpus.ajustarValor(c,linhai,iSaidaNB, objCorpus.pegarIndice("0"));
            objCorpus.ajustarValor(c,linhai,iSaidaBLS, objCorpus.pegarIndice("0"));
            objCorpus.ajustarValor(c,linhai,iSaidaRegLog, objCorpus.pegarIndice("0"));
            objCorpus.ajustarValor(c,linhai,iSaidaSVM, objCorpus.pegarIndice("0"));

        }
    }

    //chamada para retirar valores indesejados do corpus (0�s), criados a partir da diferenca nos pre�os.
    //nesta implementa��o vou usar a maior janela para o dataset n�o ter tamanho diferente em fun��o da janela, n�o comprometendo assim a qualidade
    //da saida do modelo.

    //objCorpus.gravarArquivo("../outputs/#antes.txt");

    removerRegistrosZerados(objCorpus, janela_max);

    //objCorpus.gravarArquivo("../outputs/#depois.txt");

    return this->novosAtributos;
}

bool ProcessadorSerieHistorica::removerRegistrosZerados(Corpus &objCorpus, int janela_atual){

    int qtd_linhas =  objCorpus.pegarQtdSentencas();
    int j;
    vector< int > vetMascara(qtd_linhas);
    vector< Corpus* > vetCorpus;

    try{
        for( j = 0; j < qtd_linhas; ++j ){
            if (j < janela_atual){
                vetMascara[j] = 0;
            }
            else{
                vetMascara[j] = 1;
            }
        }

        vetCorpus = objCorpus.splitCorpus(vetMascara, 2);

        objCorpus = *vetCorpus[1];

        delete vetCorpus[1];

        return true;

    }catch(string err){
        return false;
    }



}
