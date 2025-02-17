#ifndef PROCESSADORSH_H
#define PROCESSADORSH_H

#include "processador.h"

class ProcessadorSerieHistorica
{
        int janela;
        int janela_max;
        string atributo;
        bool attcriados;
		float *diferenca_i;
        bool criarNovosAtributos();
        vector<string> novosAtributos;

    public:
        ProcessadorSerieHistorica(int janela, string atributo);
        vector<string> processarCorpus(Corpus&, int janela_max);
        ~ProcessadorSerieHistorica();
		void atualizarAtributo(string att);
		void criarAtributosAuxiliares(Corpus&, int janela_ini, int janela_fim);
		bool removerRegistrosZerados(Corpus&, int janela_atual);
		bool normalizacao_min_max(Corpus &objCorpus, int iColNormalizar, float _min_norm, float _max_norm);
		bool normalizacao_padronizacao(Corpus &objCorpus, int iColNormalizar);

};

#endif // PROCESSADORSH_H

