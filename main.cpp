#define ARVORE_PREFIXO "leaves/leaf_"
#define ARQ_OBJETO "objects/objectFile"
#define ARQ_CONFIG "./arvorebplus.config"
#define ARQ_SESSAO "./.arvore.session"
#define LOCAL_DEFAULT -1

#include <chrono>
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>


using namespace std;

namespace ArvoreBMais{

    class DBObjeto {
        private:
            double chave;
            long arqIndex;
            string dataString;

        public:
            static long objetoCount;

        public:
            DBObjeto(double _chave, string _dataString) : chave(_chave), dataString(_dataString) {
                arqIndex = objetoCount++;

                ofstream oarq(ARQ_OBJETO, ios::app);
                oarq << dataString << endl;
                oarq.close();
            }

            DBObjeto(double _chave, long _arqIndex) : chave(_chave), arqIndex(_arqIndex) {
                ifstream iarq(ARQ_OBJETO);
                for (long i = 0; i < arqIndex + 1; ++i) {
                    getline(iarq, dataString);
                }
                iarq.close();
            }

            double getChave() { return chave; }

            string getDataString() { return dataString; }

            long getArqIndex() { return arqIndex; }
    };

    long DBObjeto::objetoCount = 0;

    class No {
        public:
            static long arqCount;
            static long lacoMenor;
            static long lacoMaior;
            static long tamPag;

        private:
            long arqIndex;
            bool folha;

        public:
            long proprioIndex;
            long proxFolhaIndex;
            long antFolhaIndex;
            double chaveType;
            vector<double> chaves;
            vector<long> filhoInd;
            vector<long> objPonteiro;
 public:

            No();


            No(long _arqIndex);


            bool isFolha() { return folha; }


            string getNomeArq() { return ARVORE_PREFIXO + to_string(arqIndex); }


            long getArqIndex() { return arqIndex; }


            void setParaNoInt() { folha = false; }


            long size() { return chaves.size(); }


            static void iniciar();


            long getChavePosicao(double chave);


            void envioDisco();


            void lerDisco();


            void printNo();


            void serialize();


            void inserirObjeto(DBObjeto object);


            void insertNo(double chave, long esqFilhoIndex, long dirFilhoIndex);


            void splitFolha();


            void splitInterno();
    };


    long No::lacoMenor = 0;
    long No::lacoMaior = 0;
    long No::tamPag = 0;
    long No::arqCount = 0;

    No *bRaiz = nullptr;

    No::No() {

        proprioIndex = LOCAL_DEFAULT;
        proxFolhaIndex = LOCAL_DEFAULT;
        antFolhaIndex = LOCAL_DEFAULT;


        folha = true;


        if (lacoMenor == 0) {
            cout << "Indefinido";
            exit(1);
        }


        arqIndex = ++arqCount;
    }

    No::No(long _arqIndex) {

        if (lacoMenor == 0) {
            cout << "Indefinido";
            exit(1);
        }


        arqIndex = _arqIndex;
        lerDisco();
    }

    void No::iniciar() {

        ifstream configArq;
        configArq.open(ARQ_CONFIG);
        configArq >> tamPag;
        long cabecalho = sizeof(arqIndex)
            + sizeof(folha)
            + sizeof(proprioIndex)
            + sizeof(proxFolhaIndex)
            + sizeof(antFolhaIndex);
        tamPag = tamPag - cabecalho;


        long noSize = sizeof(arqIndex);
        long chaveSize = sizeof(chaveType);
        lacoMenor = floor((tamPag - noSize) / (2 * (chaveSize + noSize)));
        lacoMaior = 2 * lacoMenor;
        tamPag = tamPag + cabecalho;
    }

    long No::getChavePosicao(double chave) {

        if (chaves.size() == 0 || chave <= chaves.front()) {
            return 0;
        }

        for (long i = 1; i < (long)chaves.size(); ++i) {
            if (chaves[i -1] < chave && chave <= chaves[i]) {
                return i;
            }
        }

        return chaves.size();
    }

    void No::envioDisco() {

        long localizacao = 0;
        char buffer[tamPag];


        memcpy(buffer + localizacao, &arqIndex, sizeof(arqIndex));
        localizacao += sizeof(arqIndex);


        memcpy(buffer + localizacao, &folha, sizeof(folha));
        localizacao += sizeof(folha);


        memcpy(buffer + localizacao, &proprioIndex, sizeof(proprioIndex));
        localizacao += sizeof(proprioIndex);


        memcpy(buffer + localizacao, &antFolhaIndex, sizeof(proxFolhaIndex));
        localizacao += sizeof(proxFolhaIndex);


        memcpy(buffer + localizacao, &proxFolhaIndex, sizeof(proxFolhaIndex));
        localizacao += sizeof(proxFolhaIndex);

        long numChaves = chaves.size();
        memcpy(buffer + localizacao, &numChaves, sizeof(numChaves));
        localizacao += sizeof(numChaves);

        for (auto chave : chaves) {
            memcpy(buffer + localizacao, &chave, sizeof(chave));
            localizacao += sizeof(chave);
        }

        if (!folha) {
            for (auto filhoIndex : filhoInd) {
                memcpy(buffer + localizacao, &filhoIndex, sizeof(filhoIndex));
                localizacao += sizeof(filhoIndex);
            }
        } else {
            for (auto objPonteiro : objPonteiro) {
                memcpy(buffer + localizacao, &objPonteiro, sizeof(objPonteiro));
                localizacao += sizeof(objPonteiro);
            }
        }

        ofstream noarq;
        noarq.open(getNomeArq(), ios::binary|ios::out);
        noarq.write(buffer, tamPag);
        noarq.close();
    }

    void No::lerDisco() {
        long localizacao = 0;
        char buffer[tamPag];


        ifstream noarq;
        noarq.open(getNomeArq(), ios::binary|ios::in);
        noarq.read(buffer, tamPag);
        noarq.close();


        memcpy((char *) &arqIndex, buffer + localizacao, sizeof(arqIndex));
        localizacao += sizeof(arqIndex);


        memcpy((char *) &folha, buffer + localizacao, sizeof(folha));
        localizacao += sizeof(folha);


        memcpy((char *) &proprioIndex, buffer + localizacao, sizeof(proprioIndex));
        localizacao += sizeof(proprioIndex);


        memcpy((char *) &antFolhaIndex, buffer + localizacao, sizeof(antFolhaIndex));
        localizacao += sizeof(antFolhaIndex);


        memcpy((char *) &proxFolhaIndex, buffer + localizacao, sizeof(proxFolhaIndex));
        localizacao += sizeof(proxFolhaIndex);

        long numChaves;
        memcpy((char *) &numChaves, buffer + localizacao, sizeof(numChaves));
        localizacao += sizeof(numChaves);

        chaves.clear();
        double chave;
        for (long i = 0; i < numChaves; ++i) {
            memcpy((char *) &chave, buffer + localizacao, sizeof(chave));
            localizacao += sizeof(chave);
            chaves.push_back(chave);
        }

        if (!folha) {
            filhoInd.clear();
            long ];
            for (long i = 0; i < numChaves + 1; ++i) {
                memcpy((char *) &filhoIndex, buffer + localizacao, sizeof(filhoIndex));
                localizacao += sizeof(filhoIndex);
                filhoInd.push_back(filhoIndex);
            }
        } else {
            objPonteiro.clear();
            long objPonteiro;
            for (long i = 0; i < numChaves; ++i) {
                memcpy((char *) &objPonteiro, buffer + localizacao, sizeof(objPonteiro));
                localizacao += sizeof(objPonteiro);
                objPonteiro.push_back(objPonteiro);
            }
        }
    }

    void No::printNo() {
        cout << endl << endl;

        cout << "Arquivo : " << arqIndex << endl;
        cout << "Folha? : " << folha << endl;
        cout << "Atual : " << proprioIndex << endl;
        cout << "Folha anterior : " << antFolhaIndex << endl;
        cout << "Folha sucessora : " << proxFolhaIndex << endl;

        cout << "Chaves : ";
        for (auto chave : chaves) {
            cout << chave << " ";
        }
        cout << endl;
cout << "Filhoren : ";
        for (auto filhoIndex : filhoInd) {
            cout << filhoIndex << " ";
        }
        cout << endl;
    }

    void No::inserirObjeto(DBObjeto object) {
        long posicao = getChavePosicao(object.getChave());

        chaves.insert(chaves.begin() + posicao, object.getChave());

        objPonteiro.insert(objPonteiro.begin() + posicao, object.getArqIndex());


        envioDisco();
    }

    void No::serialize() {

        if (chaves.size() == 0) {
            return;
        }

        cout << endl << endl;

        queue< pair<long, char> > antNivel;
        antNivel.push(make_pair(arqIndex, 'N'));

        long atualIndex;
        No *iterator;
        char type;
        while (!antNivel.empty()) {
            queue< pair<long, char> > proxNivel;

            while (!antNivel.empty()) {

                atualIndex = antNivel.front().primeiro;
                iterator = new No(atualIndex);
                type = antNivel.front().second;
                antNivel.pop();


                if (type == '|') {
                    cout << "|| ";
                    continue;
                }


                for (auto chave : iterator->chaves) {
                    cout << chave << " ";
                }


                for (auto filhoIndex : iterator->filhoInd) {
                    proxNivel.push(make_pair(filhoIndex, 'N'));


                    proxNivel.push(make_pair(LOCAL_DEFAULT, '|'));
                }


                delete iterator;
            }


            cout << endl << endl;
            antNivel = proxNivel;
        }
    }

    void No::insertNo(double chave, long esqFilhoIndex, long dirFilhoIndex) {

        long posicao = getChavePosicao(chave);
        chaves.insert(chaves.begin() + posicao, chave);


        filhoInd.insert(filhoInd.begin() + posicao + 1, dirFilhoIndex);


        envioDisco();



        if ((long)chaves.size() > lacoMaior) {
            splitInterno();
        }


        if (arqIndex == bRaiz->getArqIndex()) {
            bRaiz->lerDisco();
        }
    }

    void No::splitInterno() {



        No *noIntSubs = new No();
        noIntSubs->setParaNoInt();


        double pontoInicial = *(chaves.begin() + lacoMenor);
        for (auto chave = chaves.begin() + lacoMenor + 1; chave != chaves.end(); ++chave) {
            noIntSubs->chaves.push_back(*chave);
        }
        chaves.resize(lacoMenor);



        for (auto filhoIndex = filhoInd.begin() + lacoMenor + 1; filhoIndex != filhoInd.end(); ++filhoIndex) {
            noIntSubs->filhoInd.push_back(*filhoIndex);


            No *tempFilhoNo = new No(*filhoIndex);
            tempFilhoNo->proprioIndex = noIntSubs->arqIndex;
            tempFilhoNo->envioDisco();
            delete tempFilhoNo;
        }


        filhoInd.resize(lacoMenor + 1);


        if (proprioIndex != LOCAL_DEFAULT) {

            noIntSubs->proprioIndex = proprioIndex;
            noIntSubs->envioDisco();
            envioDisco();


            No *tempProprio = new No(proprioIndex);
            tempProprio->insertNo(pontoInicial, arqIndex, noIntSubs->arqIndex);
            delete tempProprio;
        } else {

            No *newProprio = new No();
            newProprio->setParaNoInt();


            noIntSubs->proprioIndex = newProprio->arqIndex;
            proprioIndex = newProprio->arqIndex;


            newProprio->chaves.push_back(pontoInicial);


            newProprio->filhoInd.push_back(arqIndex);
            newProprio->filhoInd.push_back(noIntSubs->arqIndex);


            newProprio->envioDisco();
            envioDisco();
            noIntSubs->envioDisco();


            delete bRaiz;


            bRaiz = newProprio;
        }


        delete noIntSubs;
    }

    void No::splitFolha() {



        No *folhaNoSubs = new No();
        for (long i = lacoMenor; i < (long) chaves.size(); ++i) {
            DBObjeto object = DBObjeto(chaves[i], objPonteiro[i]);
            folhaNoSubs->inserirObjeto(object);
        }


        chaves.resize(lacoMenor);
        objPonteiro.resize(lacoMenor);



        long tempFolhaIndex = proxFolhaIndex;
        proxFolhaIndex = folhaNoSubs->arqIndex;
        folhaNoSubs->proxFolhaIndex = tempFolhaIndex;


        if (tempFolhaIndex != LOCAL_DEFAULT) {
            No *tempFolhaIndex = new No(tempFolhaIndex);
            tempFolha->antFolhaIndex = folhaNoSubs->arqIndex;
            tempFolha->envioDisco();
            delete tempFolha;
        }
        folhaNoSubs->antFolhaIndex = arqIndex;


        if (proprioIndex != LOCAL_DEFAULT) {

            folhaNoSubs->proprioIndex = proprioIndex;
            folhaNoSubs->envioDisco();
            envioDisco();


            No *tempProprio = new No(proprioIndex);
            tempProprio->insertNo(folhaNoSubs->chaves.front(), arqIndex, folhaNoSubs->arqIndex);
            delete tempProprio;
        } else {

            No *newProprio = new No();
            newProprio->setParaNoInt();


            folhaNoSubs->proprioIndex = newProprio->arqIndex;
            proprioIndex = newProprio->arqIndex;


            newProprio->chaves.push_back(folhaNoSubs->chaves.front());


            newProprio->filhoInd.push_back(this->arqIndex);
            newProprio->filhoInd.push_back(folhaNoSubs->arqIndex);


            newProprio->envioDisco();
            folhaNoSubs->envioDisco();
            envioDisco();


            delete bRaiz;


            bRaiz = newProprio;
        }


        delete folhaNoSubs;
    }


    void insert(No *Raiz, DBObjeto object) {

        if (Raiz->isFolha()) {

            Raiz->inserirObjeto(object);


            if (Raiz->size() > Raiz->lacoMaior) {
                Raiz->splitFolha();
            }


        } else {

            long posicao = Raiz->getChavePosicao(object.getChave());


            No *proxRaiz = new No(Raiz->filhoInd[posicao]);


            insert(proxRaiz, object);


            delete proxRaiz;
        }
    }


    void pontoFila(No *Raiz, double searchChave) {

        if (Raiz->isFolha()) {

            for (long i = 0; i < (long) Raiz->chaves.size(); ++i) {
                if (Raiz->chaves[i] == searchChave) {
                }
            }


            if (Raiz->proxFolhaIndex != LOCAL_DEFAULT) {

                No *tempNo = new No(Raiz->proxFolhaIndex);


                if (tempNo->chaves.front() == searchChave) {
                    pontoFila(tempNo, searchChave);
                }

                delete tempNo;
            }
        } else {

            long posicao = Raiz->getChavePosicao(searchChave);


            No *proxRaiz = new No(Raiz->filhoInd[posicao]);


            pontoFila(proxRaiz, searchChave);
            delete proxRaiz;
        }
    }

    void janQuery(No *Raiz, double limiteInf, double limiteSup) {

        if (Raiz->isFolha()) {

            for (long i = 0; i < (long) Raiz->chaves.size(); ++i) {
                if (Raiz->chaves[i] >= limiteInf && Raiz->chaves[i] <= limiteSup) {
                }
            }


            if (Raiz->proxFolhaIndex != LOCAL_DEFAULT) {
                No *tempNo= new No(Raiz->proxFolhaIndex);

                if (tempNo->chaves.front() >= limiteInf && tempNo->chaves.front() <=limiteSup) {
                    janQuery(tempNo, limiteInf, limiteSup);
                }


                delete tempNo;
            }
        } else {

            long posicao = Raiz->getChavePosicao(limiteInf);


            No *proxRaiz = new No(Raiz->filhoInd[posicao]);


            janQuery(proxRaiz, limiteInf, limiteSup);


            delete proxRaiz;
        }
    }


    void alcQuery(No *Raiz, double centro, double alcance) {
        double lacoMaior = centro + alcance;
        double lacoMenor = (centro - alcance >= 0) ? centro - alcance : 0;


        janQuery(Raiz, lacoMenor, lacoMaior);
    }


    void orgQuery(No *Raiz, double centro, long k) {

        if (Raiz->isFolha()) {
            vector< pair<double, long> > resposta;


            long posicao = Raiz->getChavePosicao(centro);


            long count = 0;
            for (long i = posicao; i < (long)Raiz->chaves.size(); ++i, ++count) {
                resposta.push_back(make_pair(Raiz->chaves[i], Raiz->objPonteiro[i]));
            }


            long proxIndex = Raiz->proxFolhaIndex;
            while (count < k && proxIndex != LOCAL_DEFAULT) {
                No tempNo = No(proxIndex);

                for (long i = 0; i < (long) tempNo.chaves.size(); ++i, ++ count) {
                    resposta.push_back(make_pair(tempNo.chaves[i], tempNo.objPonteiro[i]));
                }


                proxIndex = tempNo.proxFolhaIndex;
            }


            count = 0;
            for (long i = 0; i < (long) posicao; ++i, ++count) {
                resposta.push_back(make_pair(Raiz->chaves[i], Raiz->objPonteiro[i]));
            }


            long antIndex = Raiz->antFolhaIndex;
            while (count < k && antIndex != LOCAL_DEFAULT) {
                No tempNo = No(antIndex);

                for (long i = 0; i < (long) tempNo.chaves.size(); ++i, ++ count) {
                    resposta.push_back(make_pair(tempNo.chaves[i], tempNo.objPonteiro[i]));
                }


                antIndex = tempNo.antFolhaIndex;
                }
            sort(resposta.begin(), resposta.end(),
                    [&](pair<double, long> T1, pair<double, long> T2) {
                    return (abs(T1.primeiro - centro) < abs(T2.primeiro - centro));
                    });


            pair <double, long> respostas;
            for (long i = 0; i < k && i < (long) resposta.size(); ++i) {
                respostas = resposta[i];
            }
        } else {

            long posicao = Raiz->getChavePosicao(centro);


            No *proxRaiz = new No(Raiz->filhoInd[posicao]);


            orgQuery(proxRaiz, centro, k);


            delete proxRaiz;
        }
    }

    void guardarSecao() {

        long localizacao = 0;
        char buffer[No::tamPag];


        long arqIndex = bRaiz->getArqIndex();
        memcpy(buffer + localizacao, &a, sizeof(arqIndex));
        localizacao += sizeof(arqIndex);


        memcpy(buffer + localizacao, &No::arqCount, sizeof(No::arqCount));
        localizacao += sizeof(No::arqCount);


        memcpy(buffer + localizacao, &DBObjeto::objetoCount, sizeof(DBObjeto::objetoCount));
        localizacao += sizeof(DBObjeto::objetoCount);


        ofstream arqSecao;
        arqSecao.open(ARQ_SESSAO, ios::binary|ios::out);
        arqSecao.write(buffer, No::tamPag);
        arqSecao.close();
    }

    void carregarSecao() {

        long localizacao = 0;
        char buffer[No::tamPag];


        ifstream arqSecao;
        arqSecao.open(ARQ_SESSAO, ios::binary|ios::in);
        arqSecao.read(buffer, No::tamPag);
        arqSecao.close();


        long arqIndex;
        memcpy((char *) &arqIndex, buffer + localizacao, sizeof(arqIndex));
        localizacao += sizeof(arqIndex);


        long arqCount;
        memcpy((char *) &arqCount, buffer + localizacao, sizeof(arqCount));
        localizacao += sizeof(arqCount);


        long objetoCount;
        memcpy((char *) &objetoCount, buffer + localizacao, sizeof(objetoCount));
        localizacao += sizeof(objetoCount);


        No::arqCount = arqCount;
        DBObjeto::objetoCount = objetoCount;

        delete bRaiz;
        bRaiz = new No(arqIndex);
        bRaiz->lerDisco();
    }
}

using namespace ArvoreBMais;

void criarArvore() {
    ifstream iarq;
    iarq.open("./data_bmais.txt", ios::in);

    double chave;
    string dataString;
    long count = 0;
    while (iarq >> chave >> dataString) {
        if (count % 5000 == 0) {
        }


        insert(bRaiz, DBObjeto(chave, dataString));


        count++;
    }


    iarq.close();
}

void analisarQuery() {
    ifstream iarq;
    iarq.open("./lista_bmais.txt", ios::in);

    long query;
    while (iarq >> query) {
        if (query == 0) {
            double chave;
            string dataString;
            iarq >> chave >> dataString;


            insert(bRaiz, DBObjeto(chave, dataString));
        } else if (query == 1) {
            double chave;
            iarq >> chave;


            pontoFila(bRaiz, chave);
        } else if (query == 2) {
            double chave, alcance;
            iarq >> chave >> alcance;



            alcQuery(bRaiz, chave, alcance * 0.1);

        } else if (query == 3) {
            double chave;
            long k;
            iarq >> chave >> k;


            orgQuery(bRaiz, chave, k);

        } else if (query == 4) {
            double limiteInf;
            double limiteSup;
            iarq >> limiteInf >> limiteSup;

            janQuery(bRaiz, limiteInf, limiteSup);

        }
    }


    iarq.close();
}

int main() {

    No::iniciar();


    bRaiz = new No();


    ifstream arqSecao(ARQ_SESSAO);
    if (arqSecao.good()) {
        carregarSecao();
    } else {
        criarArvore();
    }
    analisarQuery();
    guardarSecao();

    return 0;
}
