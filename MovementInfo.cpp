#include "MovementInfo.h"
#include <algorithm>
#include <sstream>
#include "utils/BenefitsUtil.h"
#include <unordered_map>

MovementInfo::MovementInfo(Paper *paper, Cluster *cluster, int removeValue)
{
    this->paper = paper;
    this->cluster = cluster;
    this->removeValue = removeValue;
}

int MovementInfo::getCurrentObjectiveValue()
{
    int sum = 0;
    for (auto cluster : *ClustersInfo::clusters)
        sum += cluster->getClusterValue();
    return sum;
}

void MovementInfo::calculateInsertMovements()
{
    ClustersInfo::deleteInsertMovement(paper->index, cluster->index);
    // ClustersInfo::insertMovements[paper->index][cluster->index] = NULL;
    vector<Cluster *> *clustersInfo = ClustersInfo::clusters;
    for (auto c : *clustersInfo)
    {
        if (c->index != cluster->index)
        {
            int benefit = c->calculateBenefitOfInsertPaper(paper, false);
            // ClustersInfo::insertMovements[paper->index][c->index] = benefit;
            ClustersInfo::setInsertMovement(paper->index, c->index, benefit);
        }
    }
    calculateBestPaperMovement();
}

void MovementInfo::recalculateInsertMovementsToClusters(vector<Cluster *> *modifiedClusters, vector<MovementInfo *> *movementsInfo)
{
    // atualiza o valores de inserção dos papers para os clusters envolvidos no move
    for (auto c : *modifiedClusters)
    {
        vector<int> paperIndexes;
        auto papers = c->getPapers();
        for (auto p : *papers)
            paperIndexes.push_back(p->index);

        for (auto m : *movementsInfo)
        {
            if (find(paperIndexes.begin(), paperIndexes.end(), m->paper->index) == paperIndexes.end())
            {
                // ClustersInfo::insertMovements[m->paper->index][c->index] =
                //     c->calculateBenefitOfInsertPaper(m->paper, false);
                int benefit = c->calculateBenefitOfInsertPaper(m->paper, false);
                ClustersInfo::setInsertMovement(m->paper->index, c->index, benefit);
            }
        }
    }
}

void MovementInfo::calculateBestPaperMovement()
{
    /* Procura nos cluters que ainda não estão cheios o melhor benefício
    * de remover do cluster atual e inserir em algum outro cluster. */    
    bestClusterIndexToMove = -1;
    if (cluster->canRemoveFromCluster()) {
        for (auto c : *ClustersInfo::clusters)
        {
            // Considera o primeiro cluster não cheio como o melhor
            if (bestClusterIndexToMove == -1)
            {
                if (cluster->index != c->index && c->isNotFull())
                {
                    bestClusterIndexToMove = c->index;                    
                }
                continue;
            }            
            /* Se o cluster não estiver cheio e apresentar um benefício maior, é escolhido
            O removeValue tem que está na comparação pois pode ocorrer de um movimento
            ter um aumento de 10 mas ao remover se perde 20 */
            if (cluster->index != c->index && c->isNotFull() 
                && ClustersInfo::getInsertMovement(paper->index, bestClusterIndexToMove) - removeValue < 
                    ClustersInfo::getInsertMovement(paper->index, c->index) - removeValue)
            {

                // cout << "bestClusterIndexToMove2 : " << bestClusterIndexToMove << endl;
                bestClusterIndexToMove = c->index;
            }            
        }
    }
        
    bestMovementValue = bestClusterIndexToMove >= 0
        ? ClustersInfo::getInsertMovement(paper->index, bestClusterIndexToMove) - removeValue
        : 0;
        // if (bestClusterIndexToMove >= 0)
        //     cout << ClustersInfo::getInsertMovement(paper->index, bestClusterIndexToMove) << " - " << removeValue << " = " << bestMovementValue << endl;
        // cout << cluster->index << ". bestMovementValue: " << bestMovementValue << endl;
    }

    void MovementInfo::makeMoveMovement(vector<MovementInfo*> *movementsInfo, vector<vector<pair<MovementInfo*, MovementInfo*>>> *movementsIn2, vector<vector<vector<MovementInfo*>>>* movementsIn3)
    {
        makeMoveMovement(movementsInfo, movementsIn2, movementsIn3, -1);
    }

    void MovementInfo::makeMoveMovement(vector<MovementInfo *> *movementsInfo, vector<vector<pair<MovementInfo*, MovementInfo*>>> *movementsIn2, vector<vector<vector<MovementInfo*>>>* movementsIn3, int clusterIndex)
    {
        // Troca os cluster corrente pelo cluster de destino
        auto oldClusterIndex = cluster->index;
        int resIndex = clusterIndex >= 0 ? clusterIndex : bestClusterIndexToMove;
        movePaperToCluster(resIndex);
        updateSwapMovementsInsideCluster();
        
        //cout << "antigo cluster: " << oldClusterIndex << " novo cluster: " << resIndex << endl;
        // Atualiza valores de remover o cluster (o index), o RemoveValue deste movementInfo
        // já foi calculado em MovePaperToCluster
        // vector<MovementInfo*> movementsWithClustersInTheMove;
        /* OBSERVAÇÃO: talvez poderia ser acessado em O(1) utilzando uma estrutura de dados diferente,
       um array 2D por exemplo */
        for (auto m : *movementsInfo)
        {
            if ((m->cluster->index == oldClusterIndex || m->cluster->index == cluster->index) && m->paper->index != paper->index)
            {
                m->updateRemoveValue();
            }
        }

        // Atualiza o movimento de inserção dos outros clusters para os clusteres
        // que foram atualizados (o que sofreu remoção e o que sofreu um inserção)
        vector<Cluster *> modifiedClusters;
        modifiedClusters.push_back(cluster);
        modifiedClusters.push_back(ClustersInfo::clusters->at(oldClusterIndex));
        recalculateInsertMovementsToClusters(&modifiedClusters, movementsInfo);

        // Atualiza a vizinhança de swap
        for (auto m : *movementsInfo)
        {
            if (m->cluster->index == oldClusterIndex || m->cluster->index == cluster->index)
            {
                m->recalculateSwapMovents(movementsInfo);
            }
        }
        
        updateSwap22Movements(paper->index, oldClusterIndex, resIndex, movementsInfo, movementsIn2);
        //updateSwap33Movements(paper->index, oldClusterIndex, resIndex, movementsInfo, movementsIn3);
    }

    /* Move um paper para um novo cluster */
    void MovementInfo::movePaperToCluster(int clusterIndex)
    {
        if (cluster->isDummyCluster())
        {
            removeValue = 0;
        }
        // O benefício desse paper voltar para este cluster é igual ao de remoção
        ClustersInfo::setInsertMovement(paper->index, cluster->index, removeValue);

        cluster->removePaper(paper);
        ClustersInfo::clusters->at(clusterIndex)->add(paper);
        // Faz o cluster de destino ser o novo cluster desta classe
        cluster = ClustersInfo::clusters->at(clusterIndex);

        // O valor do paper sair do novo cluster é igual ao dele entrar
        removeValue = ClustersInfo::getInsertMovement(paper->index, cluster->index);

        // Remove o benefíco já que o paper já se encontra no cluster corrente
        ClustersInfo::deleteInsertMovement(paper->index, cluster->index);

        //// Recalcula o melhor cluster para mover o paper (necessário na vizinhança mover)
        //CalculateBestPaperMovement(); Obs: por enquanto vou calcular todos juntos
    }

    // Atualiza o Movimento de Swap para não conter benefício entre os papers do mesmo cluster
    void MovementInfo::updateSwapMovementsInsideCluster()
    {
        auto clusterPapers = cluster->getPapers();
        for (auto p : *clusterPapers)
        {
            if (p->index == paper->index)
                continue;

            if (paper->index < p->index)
                ClustersInfo::swapMovements[paper->index][p->index] = NULL;
            else
                ClustersInfo::swapMovements[p->index][paper->index] = NULL;
        }
    }

    void MovementInfo::updateSwap22Movements(int paper_changed, int old_cluster, int new_cluster, vector<MovementInfo*>* moves_single, vector<vector<pair<MovementInfo*, MovementInfo*>>> *moves_pair)
    {
        
        for (int i = (*moves_pair)[old_cluster].size()-1; i >= 0; --i) {      //tirando os movimentos do cluster antigo

            if ((*moves_pair)[old_cluster][i].first->paper->index == paper_changed or (*moves_pair)[old_cluster][i].second->paper->index == paper_changed) { 
                
                std::swap((*moves_pair)[old_cluster][i], (*moves_pair)[old_cluster].back());
                (*moves_pair)[old_cluster].pop_back();
            }
        }
        

        int greater_i = -1;
        int minor_i = 99999999;
        if(old_cluster > new_cluster){
            greater_i = old_cluster;
            minor_i = new_cluster;
        }else{
            greater_i = new_cluster;
            minor_i = old_cluster;
        }
     
        for(int i = minor_i+1; i < ClustersInfo::swap22Movements.size(); i++){              

            if(ClustersInfo::swap22Movements[minor_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[minor_i][i].size()-1; j >= 0; j--)            
            {
               //auto mov = ClustersInfo::swap22Movements[minor_i][i][j];

               // std::swap(mov, ClustersInfo::swap22Movements[minor_i][i].back());
               // delete ClustersInfo::swap22Movements[minor_i][i][ClustersInfo::swap22Movements[minor_i][i].size()-1];
                delete ClustersInfo::swap22Movements[minor_i][i].back();
		ClustersInfo::swap22Movements[minor_i][i].pop_back();
            }
        }
        
        for(int i = 0; i < minor_i; i++){

            if(ClustersInfo::swap22Movements[i][minor_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[i][minor_i].size()-1; j >= 0; j--)           
            {
                //auto mov = ClustersInfo::swap22Movements[i][minor_i][j];

                //std::swap(mov, ClustersInfo::swap22Movements[i][minor_i].back());
                //delete ClustersInfo::swap22Movements[i][minor_i][ClustersInfo::swap22Movements[i][minor_i].size()-1];
                delete ClustersInfo::swap22Movements[i][minor_i].back();
		ClustersInfo::swap22Movements[i][minor_i].pop_back();
            }
        }
        
        for(int i = greater_i+1; i < ClustersInfo::swap22Movements.size(); i++){              

            if(ClustersInfo::swap22Movements[greater_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[greater_i][i].size()-1; j >= 0; j--)            
            {
                //auto mov = ClustersInfo::swap22Movements[greater_i][i][j];

                //std::swap(mov, ClustersInfo::swap22Movements[greater_i][i].back());
                //delete ClustersInfo::swap22Movements[greater_i][i][ClustersInfo::swap22Movements[greater_i][i].size()-1];
                delete ClustersInfo::swap22Movements[greater_i][i].back();
		ClustersInfo::swap22Movements[greater_i][i].pop_back();
            }
        }
    
        for(int i = 0; i < greater_i; i++){

            if(ClustersInfo::swap22Movements[i][greater_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[i][greater_i].size()-1; j >= 0; j--)           
            {
                //auto mov = ClustersInfo::swap22Movements[i][greater_i][j];

                //std::swap(mov, ClustersInfo::swap22Movements[i][greater_i].back());
                //delete ClustersInfo::swap22Movements[i][greater_i][ClustersInfo::swap22Movements[i][greater_i].size()-1];
                delete ClustersInfo::swap22Movements[i][greater_i].back();
		ClustersInfo::swap22Movements[i][greater_i].pop_back();
            }
        }

        vector<MovementInfo*> moves_newcluster;
        for(auto mov : *moves_single){               

            if(mov->cluster->index == new_cluster){
                moves_newcluster.push_back(mov);
            }
        }

        for(auto mov : moves_newcluster){               
            
            if(mov->paper->index == paper->index){
                continue;
            }else{
                pair<MovementInfo*, MovementInfo*> new_pair = {mov, this}; 
                (*moves_pair)[new_cluster].push_back(new_pair);              //adicionando o moves pair
            }
        }
        
        for (auto m : (*moves_pair)[old_cluster])
        {
            m.first->recalculateSwap22Movements(m.second, moves_pair);
        }

        for (auto m : (*moves_pair)[new_cluster])
        {
            m.first->recalculateSwap22Movements(m.second, moves_pair);
        }
    }

    void MovementInfo::updateSwap33Movements(int paper_changed, int old_cluster, int new_cluster, vector<MovementInfo*> *moves_single, vector<vector<vector<MovementInfo*>>> *moves_triple)
    {

        for (int i = (*moves_triple)[old_cluster].size()-1; i >= 0; --i) {      //tirando os movimentos do cluster antigo

            if ((*moves_triple)[old_cluster][i][0]->paper->index == paper_changed or (*moves_triple)[old_cluster][i][1]->paper->index == paper_changed or
                (*moves_triple)[old_cluster][i][2]->paper->index == paper_changed) { 
                
                std::swap((*moves_triple)[old_cluster][i], (*moves_triple)[old_cluster].back());
                (*moves_triple)[old_cluster].pop_back();
            }
        }
        
        int greater_i = -1;
        int minor_i = 99999999;
        if(old_cluster > new_cluster){
            greater_i = old_cluster;
            minor_i = new_cluster;
        }else{
            greater_i = new_cluster;
            minor_i = old_cluster;
        }
     
        for(int i = minor_i+1; i < ClustersInfo::swap33Movements.size(); i++){              

            if(ClustersInfo::swap33Movements[minor_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[minor_i][i].size()-1; j >= 0; j--)            
            {
                auto mov = ClustersInfo::swap33Movements[minor_i][i][j];

                std::swap(mov, ClustersInfo::swap33Movements[minor_i][i].back());
                delete ClustersInfo::swap33Movements[minor_i][i][ClustersInfo::swap33Movements[minor_i][i].size()-1];
                ClustersInfo::swap33Movements[minor_i][i].pop_back();
            }
        }
        
        for(int i = 0; i < minor_i; i++){

            if(ClustersInfo::swap33Movements[i][minor_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[i][minor_i].size()-1; j >= 0; j--)           
            {
                auto mov = ClustersInfo::swap33Movements[i][minor_i][j];

                std::swap(mov, ClustersInfo::swap33Movements[i][minor_i].back());
                delete ClustersInfo::swap33Movements[i][minor_i][ClustersInfo::swap33Movements[i][minor_i].size()-1];
                ClustersInfo::swap33Movements[i][minor_i].pop_back();
            }
        }
        
        for(int i = greater_i+1; i < ClustersInfo::swap33Movements.size(); i++){              

            if(ClustersInfo::swap33Movements[greater_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[greater_i][i].size()-1; j >= 0; j--)            
            {
                auto mov = ClustersInfo::swap33Movements[greater_i][i][j];

                std::swap(mov, ClustersInfo::swap33Movements[greater_i][i].back());
                delete ClustersInfo::swap33Movements[greater_i][i][ClustersInfo::swap33Movements[greater_i][i].size()-1];
                ClustersInfo::swap33Movements[greater_i][i].pop_back();
            }
        }
    
        for(int i = 0; i < greater_i; i++){

            if(ClustersInfo::swap33Movements[i][greater_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[i][greater_i].size()-1; j >= 0; j--)           
            {
                auto mov = ClustersInfo::swap33Movements[i][greater_i][j];

                std::swap(mov, ClustersInfo::swap33Movements[i][greater_i].back());
                delete ClustersInfo::swap33Movements[i][greater_i][ClustersInfo::swap33Movements[i][greater_i].size()-1];
                ClustersInfo::swap33Movements[i][greater_i].pop_back();
            }
        }

        vector<MovementInfo*> moves_newcluster;
        for(auto mov : *moves_single){               

            if(mov->cluster->index == new_cluster){
                moves_newcluster.push_back(mov);
            }
        }

        int i = 0;
        for(MovementInfo* m : moves_newcluster){                           //falta consertar isso
              
            int j = 0;
            for(MovementInfo* n : moves_newcluster){ 

                if(i == j){
                    continue;
                }
                int k = 0;
                for(MovementInfo* o : moves_newcluster){                //preenchendo o vetor com todos os possiveis pares de movimento

                    if(i == k or j == k){
                        continue;
                    }else{
                        vector<MovementInfo*> trio = {m, n, o};
                       // cout << m->getPaper()->index << " " << n->getPaper()->index << " " << o->getPaper()->index << endl; getchar();
                        (*moves_triple)[new_cluster].push_back(trio);  
                    }                
                    k++;
                }
                j++;
            }
            i++;
        }

       

        for (auto m : (*moves_triple)[old_cluster])
        {
            m[0]->recalculateSwap33Movements(m[1],m[2], moves_triple);
        }

        for (auto m : (*moves_triple)[new_cluster])
        {
            m[0]->recalculateSwap33Movements(m[1],m[2], moves_triple);
        }
    }



    void MovementInfo::updateRemoveValue()
    {
        removeValue = cluster->calculatePaperRemoveValue(paper);
    }

    void MovementInfo::calculateSwapMovement(MovementInfo * movement)
    {
        int index1, index2;

        index1 = min(paper->index, movement->paper->index);
        index2 = max(paper->index, movement->paper->index);

        vector<int> clusterPaperIndex;
        auto clusterPapers = cluster->getPapers();
        for (auto p : *clusterPapers)
            clusterPaperIndex.push_back(p->index);

        // if (Cluster.Papers.Contains(movement.Paper))
        if (find(clusterPaperIndex.begin(), clusterPaperIndex.end(), movement->paper->index) != clusterPaperIndex.end())
        {
            ClustersInfo::swapMovements[index1][index2] = NULL;
            return;
        }

        auto paper1 = paper;
        auto paper2 = movement->paper;
        auto cluster1 = cluster;
        auto cluster2 = movement->cluster;

        int benefitOfPaper1inCluster1 = removeValue;
        int benefitOfPaper2inCluster2 = movement->removeValue;

        int benefitOfPaper1inCluter2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index);
        int benefitOfPaper2inCluster1 = ClustersInfo::getInsertMovement(paper2->index, cluster1->index);

        /* Esta operação é realizada pois o benefício é calculado considerando que o cluster de destino possui todos os papers.
       Por exemplo, ao calcular o benefício do paper 1 ir para o cluster 2, no cálculo leva-se em consideração o valor
       do paper 2 já que esse entontra-se dentro do cluster 2. E portando esse valor deve ser subtraído
       ao se calcular o benefício de trocar o paper de cluster.
    */
        int benefitOfPaper1AndPaper2 = BenefitsUtil::getBenefit(paper1->index, paper2->index);

        int swapBenefitP1ToC2 = benefitOfPaper1inCluter2 - benefitOfPaper1inCluster1 - benefitOfPaper1AndPaper2;
        int swapBenefitP2ToC1 = benefitOfPaper2inCluster1 - benefitOfPaper2inCluster2 - benefitOfPaper1AndPaper2;

        int swapBenefit = swapBenefitP1ToC2 + swapBenefitP2ToC1;

        ClustersInfo::setSwapMovement(index1, index2, new SwapMovementInfo(this, movement, swapBenefit));
    }

    /* Calcula o melhor movimento de swap entre esta classe e os MovementInfo passados como parâmetro */
    void MovementInfo::calculateSwapMovements(vector<MovementInfo *> * movements)
    {
        vector<MovementInfo *> movementsToCompare;
        for (MovementInfo *m : *movements)
        {
            if (m->cluster->index > cluster->index)
            {
                movementsToCompare.push_back(m);
            }
        }

        int index1, index2;
        for (MovementInfo *m : movementsToCompare)
        {

            index1 = min(paper->index, m->paper->index);
            index2 = max(paper->index, m->paper->index);

            Paper *paper1 = paper;
            Paper *paper2 = m->paper;
            Cluster *cluster1 = cluster;
            Cluster *cluster2 = m->cluster;

            int benefitOfPaper1inCluster1 = removeValue;
            int benefitOfPaper2inCluster2 = m->getRemoveValue();

            int benefitOfPaper1inCluster2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index);
            int benefitOfPaper2inCluster1 = ClustersInfo::getInsertMovement(paper2->index, cluster1->index);

            int benefitOfPaper1AndPaper2 = BenefitsUtil::getBenefit(paper1->index, paper2->index);

            int swapBenefitP1ToC2 = benefitOfPaper1inCluster2 - benefitOfPaper1inCluster1 - benefitOfPaper1AndPaper2;
            int swapbenefitP2ToC1 = benefitOfPaper2inCluster1 - benefitOfPaper2inCluster2 - benefitOfPaper1AndPaper2;

            int swapBenefit = swapBenefitP1ToC2 + swapbenefitP2ToC1;
            ClustersInfo::setSwapMovement(index1, index2, new SwapMovementInfo(this, m, swapBenefit));
        }
    }


    // void MovementInfo::calculateSwap22Movements(vector<vector<pair<MovementInfo*,MovementInfo*>>> *movements, index)
    // {
    //     vector<pair<MovementInfo*, MovementInfo*>> movementsToCompare;
    //     vector<pair<MovementInfo*, MovementInfo*>> sameCluster;

    //     for (pair<MovementInfo*, MovementInfo*> m : *movements)
    //     {
    //         if (m.first->cluster->index == cluster->index)
    //         {
    //             sameCluster.push_back(m);
    //         }

    //         if (m.first->cluster->index > cluster->index)
    //         {
    //             movementsToCompare.push_back(m);
    //         }
    //     }
        
    //     auto cluster1 = cluster;                       //cluster da classe
    //     for(int i = 0; i < sameCluster.size(); i++){

    //         auto paper1 = sameCluster[i].first->paper;
    //         auto paper2 = sameCluster[i].second->paper;

    //         int benefitOfPaper12inCluster1 = sameCluster[i].first->getRemoveValue() + sameCluster[i].second->getRemoveValue();  //beneficio a ser retirado

    //         for(int j = 0; j < movementsToCompare.size(); j++){

    //             auto cluster2 = movementsToCompare[j].first->cluster;
    //             auto paper3 = movementsToCompare[j].first->paper;
    //             auto paper4 = movementsToCompare[j].second->paper;


    //             int benefitOfPaper34inCluster2 = movementsToCompare[j].first->getRemoveValue() + movementsToCompare[j].second->getRemoveValue();

    //             int benefitOfPaper12inCluster2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index) + ClustersInfo::getInsertMovement(paper2->index, cluster2->index)
    //                                             +(2*BenefitsUtil::getBenefit(paper1->index, paper2->index));

    //             int benefitOfPaper34inCluster1 = ClustersInfo::getInsertMovement(paper3->index, cluster1->index) + ClustersInfo::getInsertMovement(paper4->index, cluster1->index)
    //                                             +(2*BenefitsUtil::getBenefit(paper3->index, paper4->index));


    //             int swapBenefitP1P2ToC2 = benefitOfPaper12inCluster2 - benefitOfPaper12inCluster1 - BenefitsUtil::getBenefit(paper1->index, paper3->index) 
    //                                       - BenefitsUtil::getBenefit(paper1->index, paper4->index) - BenefitsUtil::getBenefit(paper2->index, paper3->index)
    //                                       - BenefitsUtil::getBenefit(paper2->index, paper4->index);
                
    //             int swapBenefitP3P4ToC1 = benefitOfPaper34inCluster1 - benefitOfPaper34inCluster2 - BenefitsUtil::getBenefit(paper1->index, paper3->index) 
    //                                       - BenefitsUtil::getBenefit(paper1->index, paper4->index) - BenefitsUtil::getBenefit(paper2->index, paper3->index)
    //                                       - BenefitsUtil::getBenefit(paper2->index, paper4->index);

    //             int swapBenefit = swapBenefitP1P2ToC2 + swapBenefitP3P4ToC1;

    //             Swap22MovementInfo *swap22 = new Swap22MovementInfo(sameCluster[i], movementsToCompare[j], swapBenefit);
    //             //ClustersInfo::setSwap22Movement(swap22);

    //         }
    //     }
    // }

    void MovementInfo::calculateSwap22Movements(vector<vector<pair<MovementInfo*,MovementInfo*>>> *movements, int indice)
    {
        int cont = 1;
        for(int i = 0; i < (*movements)[indice].size(); i++){

            pair<MovementInfo*, MovementInfo*> pair_cluster1 = (*movements)[indice][i];

            auto cluster1 = pair_cluster1.first->cluster;
            auto paper1 = pair_cluster1.first->paper;
            auto paper2 = pair_cluster1.second->paper;

            int benefitOfPaper12inCluster1 = pair_cluster1.first->getRemoveValue() + pair_cluster1.second->getRemoveValue();  //beneficio a ser retirado

            while(1){
                
                if(indice+cont == (*movements).size()){  //acabou os arrays a serem percorridos
                    break;
                }

                int greater_i = -1;
                int minor_i = 99999999;
                if(cluster1->index > (indice+cont) ){             //o indice+cont eh equivalente ao index do cluster2
                    greater_i = cluster1->index;
                    minor_i = indice+cont;
                }else{
                    greater_i = indice+cont;
                    minor_i = cluster1->index;
                }

                for(int j = 0; j < (*movements)[indice+cont].size(); j++){

                    pair<MovementInfo*, MovementInfo*> pair_cluster2 = (*movements)[indice+cont][j];

                    
                    auto cluster2 = pair_cluster2.first->cluster;
                    auto paper3 = pair_cluster2.first->paper;
                    auto paper4 = pair_cluster2.second->paper;

                    int benefitOfPaper34inCluster2 = pair_cluster2.first->getRemoveValue() + pair_cluster2.second->getRemoveValue();

                    int benefitOfPaper12inCluster2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index) + ClustersInfo::getInsertMovement(paper2->index, cluster2->index)
                                                    +(2*BenefitsUtil::getBenefit(paper1->index, paper2->index));

                    int benefitOfPaper34inCluster1 = ClustersInfo::getInsertMovement(paper3->index, cluster1->index) + ClustersInfo::getInsertMovement(paper4->index, cluster1->index)
                                                    +(2*BenefitsUtil::getBenefit(paper3->index, paper4->index));


                    int swapBenefitP1P2ToC2 = benefitOfPaper12inCluster2 - benefitOfPaper12inCluster1 - BenefitsUtil::getBenefit(paper1->index, paper3->index) 
                                            - BenefitsUtil::getBenefit(paper1->index, paper4->index) - BenefitsUtil::getBenefit(paper2->index, paper3->index)
                                            - BenefitsUtil::getBenefit(paper2->index, paper4->index);
                    
                    int swapBenefitP3P4ToC1 = benefitOfPaper34inCluster1 - benefitOfPaper34inCluster2 - BenefitsUtil::getBenefit(paper1->index, paper3->index) 
                                            - BenefitsUtil::getBenefit(paper1->index, paper4->index) - BenefitsUtil::getBenefit(paper2->index, paper3->index)
                                            - BenefitsUtil::getBenefit(paper2->index, paper4->index);

                    int swapBenefit = swapBenefitP1P2ToC2 + swapBenefitP3P4ToC1;

                    Swap22MovementInfo *swap22 = new Swap22MovementInfo(pair_cluster1, pair_cluster2, swapBenefit);
                    ClustersInfo::setSwap22Movement(minor_i, greater_i, swap22);
                }
                cont++;
            }
        }
    }



    void MovementInfo::calculateSwap33Movements(vector<vector<vector<MovementInfo*>>> *movements, int index)
    {
        int cont = 1;
        for(int i = 0; i < (*movements)[index].size(); i++){

            vector<MovementInfo*> trio_cluster1 = (*movements)[index][i];

            auto cluster1 = trio_cluster1[0]->cluster;  
            auto paper1 = trio_cluster1[0]->paper;
            auto paper2 = trio_cluster1[1]->paper;
            auto paper3 = trio_cluster1[2]->paper;

            int benefitOfPaper123inCluster1 = trio_cluster1[0]->getRemoveValue() + trio_cluster1[1]->getRemoveValue() + trio_cluster1[2]->getRemoveValue();  //beneficio a ser retirado 

            while(1){
                
                if(index+cont == (*movements).size()){  //acabou os arrays a serem percorridos
                    break;
                }

                int greater_i = -1;
                int minor_i = 99999999;
                if(cluster1->index > (index+cont) ){             //o indice+cont eh equivalente ao index do cluster2
                    greater_i = cluster1->index;
                    minor_i = index+cont;
                }else{
                    greater_i = index+cont;
                    minor_i = cluster1->index;
                }
        
        
                for(int j = 0; j < (*movements)[index+cont].size(); j++){

                    vector<MovementInfo*> trio_cluster2 = (*movements)[index+cont][j];


                    auto cluster2 = trio_cluster2[0]->cluster;
                    auto paper4 = trio_cluster2[0]->paper;
                    auto paper5 = trio_cluster2[1]->paper;
                    auto paper6 = trio_cluster2[2]->paper;


                    int benefitOfPaper456inCluster2 = trio_cluster2[0]->getRemoveValue() + trio_cluster2[1]->getRemoveValue() + trio_cluster2[2]->getRemoveValue();  //beneficio a ser retirado
                    
                    int benefitOfPaper123inCluster2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index) + ClustersInfo::getInsertMovement(paper2->index, cluster2->index)
                                                        +ClustersInfo::getInsertMovement(paper3->index, cluster2->index) + (2*BenefitsUtil::getBenefit(paper1->index, paper2->index))
                                                        +(2*BenefitsUtil::getBenefit(paper3->index, paper1->index)) + (2*BenefitsUtil::getBenefit(paper3->index, paper2->index)); 

                    int benefitOfPaper456inCluster1 = ClustersInfo::getInsertMovement(paper4->index, cluster1->index) + ClustersInfo::getInsertMovement(paper5->index, cluster1->index)  
                                                        +ClustersInfo::getInsertMovement(paper6->index, cluster1->index) + (2*BenefitsUtil::getBenefit(paper4->index, paper5->index))
                                                        +(2*BenefitsUtil::getBenefit(paper5->index, paper6->index)) + (2*BenefitsUtil::getBenefit(paper4->index, paper6->index));


                    int swapBenefitP1P2P3ToC2 = benefitOfPaper123inCluster2 - benefitOfPaper123inCluster1 - BenefitsUtil::getBenefit(paper1->index, paper4->index) - BenefitsUtil::getBenefit(paper1->index, paper5->index)
                                                -BenefitsUtil::getBenefit(paper1->index, paper6->index) - BenefitsUtil::getBenefit(paper2->index, paper4->index) - BenefitsUtil::getBenefit(paper2->index, paper5->index) 
                                                -BenefitsUtil::getBenefit(paper2->index, paper6->index) - BenefitsUtil::getBenefit(paper3->index, paper4->index) - BenefitsUtil::getBenefit(paper3->index, paper5->index)
                                                -BenefitsUtil::getBenefit(paper3->index, paper6->index);

                    int swapBenefitP4P5P6ToC1 = benefitOfPaper456inCluster1 - benefitOfPaper456inCluster2 - BenefitsUtil::getBenefit(paper4->index, paper1->index) -  BenefitsUtil::getBenefit(paper4->index, paper2->index)
                                                -BenefitsUtil::getBenefit(paper4->index, paper3->index) - BenefitsUtil::getBenefit(paper5->index, paper1->index) - BenefitsUtil::getBenefit(paper5->index, paper2->index) 
                                                -BenefitsUtil::getBenefit(paper5->index, paper3->index) - BenefitsUtil::getBenefit(paper6->index, paper1->index) - BenefitsUtil::getBenefit(paper6->index, paper2->index) 
                                                -BenefitsUtil::getBenefit(paper6->index, paper3->index);

                    int swapBenefit = swapBenefitP1P2P3ToC2 + swapBenefitP4P5P6ToC1;
                    Swap33MovementInfo *swap33 = new Swap33MovementInfo(trio_cluster1, trio_cluster2, swapBenefit);
                    ClustersInfo::setSwap33Movement(minor_i, greater_i, swap33);    
                }
                cont++;
            }
        }              
        
    }



    /* Calcula o melhor movimento de swap entre esta classe e os MovementInfo passados como parâmetro */
    void MovementInfo::recalculateSwapMovents(vector<MovementInfo *> * movements)
    {
        int index1, index2;
        for (auto m : *movements)
        {
            if (m->cluster->index == cluster->index)
                continue;

            index1 = min(paper->index, m->paper->index);
            index2 = max(paper->index, m->paper->index);

            auto paper1 = paper;
            auto paper2 = m->paper;
            auto cluster1 = cluster;
            auto cluster2 = m->cluster;

            int benefitOfPaper1inCluster1 = removeValue;
            int benefitOfPaper2inCluster2 = m->removeValue;

            int benefitOfPaper1inCluter2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index);
            int benefitOfPaper2inCluster1 = ClustersInfo::getInsertMovement(paper2->index, cluster1->index);

            int benefitOfPaper1AndPaper2 = BenefitsUtil::getBenefit(paper1->index, paper2->index); //paper1.CalculateBenefit(paper2);

            int swapBenefitP1ToC2 = benefitOfPaper1inCluter2 - benefitOfPaper1inCluster1 - benefitOfPaper1AndPaper2;
            int swapBenefitP2ToC1 = benefitOfPaper2inCluster1 - benefitOfPaper2inCluster2 - benefitOfPaper1AndPaper2;

            int swapBenefit = swapBenefitP1ToC2 + swapBenefitP2ToC1;

            ClustersInfo::setSwapMovement(index1, index2, new SwapMovementInfo(this, m, swapBenefit));
        }
    }

    // Procura em toda a tabela pelo melhor movimento de troca
    SwapMovementInfo *MovementInfo::findBestSwapMovement()
    {
        SwapMovementInfo *bestSwapMovement = NULL;
        for (int i = 0; i < ClustersInfo::swapMovementsSize; i++)
        {
            for (int j = i + 1; j < ClustersInfo::swapMovementsSize; j++)
            {
                SwapMovementInfo *swapMove = ClustersInfo::swapMovements[i][j];
                if (swapMove == NULL)
                {
                    continue;
                }

                // Escolhe um primeiro swap que encontrar como o melhor
                if (bestSwapMovement == NULL)
                {
                    bestSwapMovement = swapMove;
                    continue;
                }

                // Se encontrar um movimento de troca melhor atualiza o melhor movimento
                if (swapMove->getValue() > bestSwapMovement->getValue())
                {
                    bestSwapMovement = swapMove;
                }
            }
        }

        return bestSwapMovement;
    }

    Swap22MovementInfo* MovementInfo::findBestSwap22Movement()
    {
        Swap22MovementInfo *bestSwap22Movement = NULL;
    
        for(int i = 0; i < ClustersInfo::swap22Movements.size(); i++){
            for(int j = 0; j < ClustersInfo::swap22Movements[i].size(); j++){
                for(int k = 0; k < ClustersInfo::swap22Movements[i][j].size(); k++){
                    
                    Swap22MovementInfo* swap22Move = ClustersInfo::swap22Movements[i][j][k];

                    if (bestSwap22Movement == NULL){
                        bestSwap22Movement = swap22Move;
                        continue;
                    }

                    // Se encontrar um movimento de troca melhor atualiza o melhor movimento
                    if (swap22Move->getValue() > bestSwap22Movement->getValue())
                    {
                        bestSwap22Movement = swap22Move;
                    }
                }
            }
        }
        
        return bestSwap22Movement;
    }


    Swap33MovementInfo* MovementInfo::findBestSwap33Movement()
    {
        Swap33MovementInfo *bestSwap33Movement = NULL;


        for(int i = 0; i < ClustersInfo::swap33Movements.size(); i++){
            for(int j = 0; j < ClustersInfo::swap33Movements[i].size(); j++){
                for(int k = 0; k < ClustersInfo::swap33Movements[i][j].size(); k++){
                    
                    Swap33MovementInfo* swap33Move = ClustersInfo::swap33Movements[i][j][k];

                    if (bestSwap33Movement == NULL){
                        bestSwap33Movement = swap33Move;
                        continue;
                    }

                    // Se encontrar um movimento de troca melhor atualiza o melhor movimento
                    if (swap33Move->getValue() > bestSwap33Movement->getValue())
                    {
                        bestSwap33Movement = swap33Move;
                    }
                }
            }
        }

        return bestSwap33Movement;
    }


    void MovementInfo::makeSwap(SwapMovementInfo * swapMovement, vector<MovementInfo *> * movementsInfo)
    {
        vector<MovementInfo *> &movements_p = swapMovement->getMovements();
        movements_p[0]->clusterToSwap = movements_p[1]->cluster->index;
        movements_p[1]->clusterToSwap = movements_p[0]->cluster->index;

        // Move os papers para seus novos clusters
        vector<Paper *> papers;
        for (auto m : *movementsInfo)
        {
            vector<Paper *> *papersToInsert = m->cluster->getPapers();
            for (auto p : *papersToInsert)
            {
                papers.push_back(p);
            }
        }
        makeSwapMovement(&swapMovement->getMovements(), &papers);

        // Autualiza os valores de remoção de todos os papers nos dois clusters envolvidos no swap
        vector<MovementInfo *> aux = swapMovement->getMovements();
        for (auto movement : aux)
        {
            movement->updateSwapMovementsInsideCluster();
            for (auto m : *movementsInfo)
            {
                if (m->cluster->index == movement->cluster->index)
                {
                    m->updateRemoveValue();
                }
            }
        }
        // Recalcula os movimentos de swap entre os dois movimentos envolvidos e os demais clusters
        for (auto movement : aux)
        {
            for (auto m : *movementsInfo)
            {
                if (m->cluster->index == movement->cluster->index)   //se o cluster do mov analasidao == cluster q o mov de swap
                {
                    m->recalculateSwapMovents(movementsInfo);
                }
            }
        }

        // int cluster1_ind = movements_p[0]->cluster->index;
        // int cluster2_ind = movements_p[1]->cluster->index;
        // int paper1_ind = movements_p[0]->paper->index;
        // int paper2_ind = movements_p[1]->paper->index;

        //  for (int i = (*moves_pair)[cluster1_ind].size()-1; i >= 0; --i) {      //tirando os movimentos do cluster antigo

        //     if ((*moves_pair)[cluster1_ind][i].first->paper->index == paper1_ind or (*moves_pair)[cluster1_ind][i].second->paper->index == paper1_ind) { 
                
        //         std::swap((*moves_pair)[cluster1_ind][i], (*moves_pair)[cluster1_ind].back());
        //         (*moves_pair)[cluster1_ind].pop_back();
        //     }
        // }

        // for (int i = ClustersInfo::swap22Movements.size()-1; i >= 0; i--)            //deletando os movimentos que tão com par de cluster errado do vector final
        // { 
        //     auto mov = ClustersInfo::swap22Movements[i];

        //     if(mov->first_pair_cluster.first->cluster->index == cluster1_ind or mov->first_pair_cluster.first->cluster->index == cluster2_ind or 
        //        mov->second_pair_cluster.first->cluster->index == cluster1_ind or mov->second_pair_cluster.first->cluster->index == cluster2_ind){

        //         std::swap(ClustersInfo::swap22Movements[i], ClustersInfo::swap22Movements.back());
        //         delete ClustersInfo::swap22Movements[ClustersInfo::swap22Movements.size()-1];
        //         ClustersInfo::swap22Movements.pop_back();
        //     }
        // }

        // vector<MovementInfo*> moves_cluster1;
        // vector<MovementInfo*> moves_cluster2;
        // for(auto mov : *movementsInfo){               

        //     if(mov->cluster->index == swapMovement->movements[0]->paper->index){
        //         moves_cluster1.push_back(mov);
        //     }
        //     if(mov->cluster->index == swapMovement->movements[1]->paper->index){
        //         moves_cluster2.push_back(mov);
        //     }
        // }

        // for(auto mov : moves_cluster1){               
            
        //     if(mov->paper->index == paper2_ind){
        //         continue;
        //     }else{
        //         pair<MovementInfo*, MovementInfo*> new_pair = {mov, swapMovement->movements[0]}; 
        //         (*moves_pair)[swapMovement->movements[0]->cluster->index].push_back(new_pair);              //adicionando o moves pair
        //     }
        // }
        // for(auto mov : moves_cluster2){               
            
        //     if(mov->paper->index == paper1_ind){
        //         continue;
        //     }else{
        //         pair<MovementInfo*, MovementInfo*> new_pair = {mov, swapMovement->movements[1]}; 
        //         (*moves_pair)[swapMovement->movements[1]->cluster->index].push_back(new_pair);              //adicionando o moves pair
        //     }
        // }
        // for (auto m : (*moves_pair)[cluster1_ind])
        // {
        //     m.first->recalculateSwap22Movements(m.second, moves_pair);
        // }

        // for (auto m : (*moves_pair)[cluster2_ind])
        // {
        //     m.first->recalculateSwap22Movements(m.second, moves_pair);
        // }
    }

    void MovementInfo::makeSwap22(Swap22MovementInfo *swap22Movement, vector<vector<pair<MovementInfo*, MovementInfo*>>>* movementsInfo, vector<MovementInfo*> *moves_single)
    {
        
        vector<Paper *> papers;
        for (auto m : *moves_single)
        {
            vector<Paper *> *papersToInsert = m->cluster->getPapers();   
            for (auto p : *papersToInsert)
            {
                papers.push_back(p);
            }
        }
        
        //-------------------make swap22 movement----------------------
        auto paper1 = swap22Movement->first_pair_cluster.first->paper;
        auto paper2 = swap22Movement->first_pair_cluster.second->paper;      
        auto cluster1 = swap22Movement->first_pair_cluster.first->cluster; 

        auto paper3 = swap22Movement->second_pair_cluster.first->paper;
        auto paper4 = swap22Movement->second_pair_cluster.second->paper;      
        auto cluster2 = swap22Movement->second_pair_cluster.first->cluster;

        cluster1->removePaper(paper1);   //removendo os papers do seu cluster antes do mov
        cluster1->removePaper(paper2);  
        cluster2->removePaper(paper3);
        cluster2->removePaper(paper4);

        cluster1->add(paper3);  //adicionando os novos papers ao cluster
        cluster1->add(paper4);
        cluster2->add(paper1);
        cluster2->add(paper2);

        ClustersInfo::deleteInsertMovement(paper1->index, cluster2->index);   //deletando o movimento que foi implementando
        ClustersInfo::deleteInsertMovement(paper2->index, cluster2->index);
        ClustersInfo::deleteInsertMovement(paper3->index, cluster1->index);
        ClustersInfo::deleteInsertMovement(paper4->index, cluster1->index);

        swap22Movement->first_pair_cluster.first->cluster = cluster2;    //redefinindo os clusters      //ajustando os clusters de acordo com o movimento já feito
        swap22Movement->first_pair_cluster.second->cluster = cluster2;
        swap22Movement->second_pair_cluster.first->cluster = cluster1;
        swap22Movement->second_pair_cluster.second->cluster = cluster1;
        
       
       
	vector<MovementInfo*> mov_cluster1;       //armazena os MovementInfo solo do cluster1
	vector<MovementInfo*> mov_cluster2;       //armazena os MovementInfo solo do cluster2

        for (int i = (*movementsInfo)[cluster1->index].size() - 1; i >= 0; --i) {   
       
            if ((*movementsInfo)[cluster1->index][i].first->paper->index == paper1->index or (*movementsInfo)[cluster1->index][i].second->paper->index == paper1->index or
                (*movementsInfo)[cluster1->index][i].first->paper->index == paper2->index or (*movementsInfo)[cluster1->index][i].second->paper->index == paper2->index) { 
                
                std::swap((*movementsInfo)[cluster1->index][i], (*movementsInfo)[cluster1->index].back());
                (*movementsInfo)[cluster1->index].pop_back();
		continue;
            }
	    mov_cluster1.push_back((*movementsInfo)[cluster1->index][i].first);
	    mov_cluster1.push_back((*movementsInfo)[cluster1->index][i].second);
        }

        for (int i = (*movementsInfo)[cluster2->index].size() - 1; i >= 0; --i) {      //retirando os pares de MovimentInfo que ficaram errados (tem q v se isso faz sentido)


            if ((*movementsInfo)[cluster2->index][i].first->paper->index == paper3->index or (*movementsInfo)[cluster2->index][i].second->paper->index == paper3->index or
                (*movementsInfo)[cluster2->index][i].first->paper->index == paper4->index or (*movementsInfo)[cluster2->index][i].second->paper->index  == paper4->index) { 
                
                std::swap((*movementsInfo)[cluster2->index][i], (*movementsInfo)[cluster2->index].back());
                (*movementsInfo)[cluster2->index].pop_back();
            }
	    mov_cluster2.push_back((*movementsInfo)[cluster2->index][i].first);
	    mov_cluster2.push_back((*movementsInfo)[cluster2->index][i].second);
        }

        vector<int> papersC1;
        auto clusterPapers = cluster1->getPapers();  //papers pertencentes ao cluster 1  
        for (auto p : *clusterPapers)
            papersC1.push_back(p->index); 

        vector<int> papersC2;
        auto clusterPapers2 = cluster2->getPapers(); //papers pertencentes ao cluster 2
        for (auto p : *clusterPapers2)
            papersC2.push_back(p->index); 


        for (auto paper : papers){

            if (find(papersC1.begin(), papersC1.end(), paper->index) != papersC1.end()){    //se o paper for encontrado no c1, deleta o movimento pra inserir nele 

                ClustersInfo::deleteInsertMovement(paper->index, cluster1->index); 
            
            }else{                                                                         //se o paper n for encontrado no c1, recalcula o insert
                int benefit = cluster1->calculateBenefitOfInsertPaper(paper, false);
                ClustersInfo::setInsertMovement(paper->index, cluster1->index, benefit);
            }

            if (find(papersC2.begin(), papersC2.end(), paper->index) != papersC2.end()){  //se o paper for enconrado no c2...
                
                ClustersInfo::deleteInsertMovement(paper->index, cluster2->index);

            }else{
                int benefit = cluster2->calculateBenefitOfInsertPaper(paper, false);
                ClustersInfo::setInsertMovement(paper->index, cluster2->index, benefit);

            }
        }

        swap22Movement->first_pair_cluster.first->updateRemoveValue();
        swap22Movement->first_pair_cluster.second->updateRemoveValue();
        swap22Movement->second_pair_cluster.first->updateRemoveValue();
        swap22Movement->second_pair_cluster.second->updateRemoveValue();
        //------------------------------------------------------------------------------------------------
         //fazendo os novos pares que surgiram com os novos papers em cada cluster

	/*
	std::sort(mov_cluster1.begin(), mov_cluster1.end(), [](const MovementInfo* a, const MovementInfo* b){
		return a->paper->index < b->paper->index;
	});
	
	std::sort(mov_cluster2.begin(), mov_cluster2.end(), [](const MovementInfo* a, const MovementInfo* b){
		return a->paper->index < b->paper->index;
	});
	int ult_index_paper = -1;
	for(auto m : mov_cluster1){
		
		cout << "m = " << m->paper->index << endl;	

		if(ult_index_paper == m->paper->index){
			continue;
		}
		pair <MovementInfo*, MovementInfo*> new_pair = {m, swap22Movement->second_pair_cluster.first};		
		(*movementsInfo)[cluster1->index].push_back(new_pair);
		cout << "par com: " << swap22Movement->second_pair_cluster.first->paper->index << endl;
		new_pair = {m, swap22Movement->second_pair_cluster.second};
		(*movementsInfo)[cluster1->index].push_back(new_pair);
		cout << "par com: " << swap22Movement->second_pair_cluster.second->paper->index << endl;
		ult_index_paper = m->paper->index;
	}
	pair<MovementInfo*, MovementInfo*> new_pair = {swap22Movement->second_pair_cluster.first, swap22Movement->second_pair_cluster.second};
	(*movementsInfo)[cluster1->index].push_back(new_pair);
	getchar();

	ult_index_paper = -1;
	for(auto m : mov_cluster2){
		if(ult_index_paper == m->paper->index){
			continue;
		}	
		pair <MovementInfo*, MovementInfo*> new_pair = {m, swap22Movement->first_pair_cluster.first};		
		(*movementsInfo)[cluster2->index].push_back(new_pair);
		new_pair = {m, swap22Movement->first_pair_cluster.second};
		(*movementsInfo)[cluster2->index].push_back(new_pair);
		ult_index_paper = m->paper->index;
	}
	new_pair = {swap22Movement->first_pair_cluster.first, swap22Movement->first_pair_cluster.second};
	(*movementsInfo)[cluster2->index].push_back(new_pair);
        */


        //recalculando o valor de remover os papers dos clusters envolvidos na alteracao
            for (auto m : (*movementsInfo)[cluster1->index])
            {
            	m.first->updateRemoveValue();
                m.second->updateRemoveValue();    
            
            }

	    for (auto m : (*movementsInfo)[cluster2->index]){
		m.first->updateRemoveValue();
		m.second->updateRemoveValue();
   	    }
    
        int greater_i = -1;
        int minor_i = 99999999;
        if(cluster1->index > cluster2->index){
            greater_i = cluster1->index;
            minor_i = cluster2->index;
        }else{
            greater_i = cluster2->index;
            minor_i = cluster1->index;
        }
    
        //----------------------------------------------------------------------------------------------------- retirando os movimentos dos clusters envolvidos do ClustersInfo
        for(int i = minor_i+1; i < ClustersInfo::swap22Movements.size(); i++){              

            if(ClustersInfo::swap22Movements[minor_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[minor_i][i].size()-1; j >= 0; j--)            
            {
                delete ClustersInfo::swap22Movements[minor_i][i].back();
		ClustersInfo::swap22Movements[minor_i][i].pop_back();
            }
        }
        
        for(int i = 0; i < minor_i; i++){

            if(ClustersInfo::swap22Movements[i][minor_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[i][minor_i].size()-1; j >= 0; j--)           
            {
              
                delete ClustersInfo::swap22Movements[i][minor_i].back();
		ClustersInfo::swap22Movements[i][minor_i].pop_back();
            }
        }
        
        for(int i = greater_i+1; i < ClustersInfo::swap22Movements.size(); i++){              

            if(ClustersInfo::swap22Movements[greater_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[greater_i][i].size()-1; j >= 0; j--)            
            {
                delete ClustersInfo::swap22Movements[greater_i][i].back();
		ClustersInfo::swap22Movements[greater_i][i].pop_back();
            }
        }
    
        for(int i = 0; i < greater_i; i++){

            if(ClustersInfo::swap22Movements[i][greater_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap22Movements[i][greater_i].size()-1; j >= 0; j--)           
            {
                delete ClustersInfo::swap22Movements[i][greater_i].back();
		ClustersInfo::swap22Movements[i][greater_i].pop_back();
            }
        }
      
        for (auto m : (*movementsInfo)[cluster1->index]){
            
            m.first->recalculateSwap22Movements(m.second, movementsInfo);
        }

        for (auto m : (*movementsInfo)[cluster2->index]){

            m.first->recalculateSwap22Movements(m.second, movementsInfo);
        }

        
    }

    void MovementInfo::makeSwap33(Swap33MovementInfo *swap33Movement, vector<vector<vector<MovementInfo*>>>* movementsInfo, vector<MovementInfo*> *moves_single)
    {
        
        auto cluster1 = swap33Movement->first_trio_cluster[0]->cluster;
        auto cluster2 = swap33Movement->second_trio_cluster[0]->cluster;

        vector<Paper *> papers;
        for (auto m : *moves_single)           //so pra pegar os clusters
        {
            vector<Paper *> *papersToInsert = m->cluster->getPapers();   //pegando os papers do cluster do movimento analisado
            
            for (auto p : *papersToInsert)
            {
                papers.push_back(p);
            }
        }

        //-------------------make swap33 movement----------------------
        auto paper1 = swap33Movement->first_trio_cluster[0]->paper;
        auto paper2 = swap33Movement->first_trio_cluster[1]->paper;   
        auto paper3 = swap33Movement->first_trio_cluster[2]->paper; 
        

        auto paper4 = swap33Movement->second_trio_cluster[0]->paper;
        auto paper5 = swap33Movement->second_trio_cluster[1]->paper;
        auto paper6 = swap33Movement->second_trio_cluster[2]->paper;    
       

        cluster1->removePaper(paper1);   //removendo os papers do seu cluster antes do mov
        cluster1->removePaper(paper2);
        cluster1->removePaper(paper3);   
        cluster2->removePaper(paper4);
        cluster2->removePaper(paper5);
        cluster2->removePaper(paper6);
    

        cluster1->add(paper4);  //adicionando os novos papers ao cluster
        cluster1->add(paper5);
        cluster1->add(paper6);
        cluster2->add(paper1);
        cluster2->add(paper2);
        cluster2->add(paper3);

        ClustersInfo::deleteInsertMovement(paper1->index, cluster2->index);   //deletando o movimento que foi implementando
        ClustersInfo::deleteInsertMovement(paper2->index, cluster2->index);
        ClustersInfo::deleteInsertMovement(paper3->index, cluster2->index);
        ClustersInfo::deleteInsertMovement(paper4->index, cluster1->index);
        ClustersInfo::deleteInsertMovement(paper5->index, cluster1->index);
        ClustersInfo::deleteInsertMovement(paper6->index, cluster1->index);
	
        swap33Movement->first_trio_cluster[0]->cluster = cluster2;    //ajustando os clusters de acordo com o movimento já feito
        swap33Movement->first_trio_cluster[1]->cluster = cluster2;
        swap33Movement->first_trio_cluster[2]->cluster = cluster2;
        swap33Movement->second_trio_cluster[0]->cluster = cluster1;
        swap33Movement->second_trio_cluster[1]->cluster = cluster1;
        swap33Movement->second_trio_cluster[2]->cluster = cluster1;


        for (int i = (*movementsInfo)[cluster1->index].size() - 1; i >= 0; --i) {      //retirando os pares de MovimentInfo que ficaram errados (tem q v se isso faz sentido)

            if((*movementsInfo)[cluster1->index].size() == 0){
                continue;
            }
       
            if ((*movementsInfo)[cluster1->index][i][0]->paper->index == paper1->index or (*movementsInfo)[cluster1->index][i][1]->paper->index == paper1->index or 
                (*movementsInfo)[cluster1->index][i][2]->paper->index == paper1->index or (*movementsInfo)[cluster1->index][i][0]->paper->index == paper2->index or 
                (*movementsInfo)[cluster1->index][i][1]->paper->index == paper2->index or (*movementsInfo)[cluster1->index][i][2]->paper->index == paper2->index or
                (*movementsInfo)[cluster1->index][i][0]->paper->index == paper3->index or (*movementsInfo)[cluster1->index][i][1]->paper->index == paper3->index or 
                (*movementsInfo)[cluster1->index][i][2]->paper->index == paper3->index){ 

                std::swap((*movementsInfo)[cluster1->index][i], (*movementsInfo)[cluster1->index].back());
                (*movementsInfo)[cluster1->index].pop_back();
            }
            
        }

        for (int i = (*movementsInfo)[cluster2->index].size() - 1; i >= 0; --i) {      //retirando os pares de MovimentInfo que ficaram errados (tem q v se isso faz sentido)

            if((*movementsInfo)[cluster2->index].size() == 0){
                continue;
            }

            if ((*movementsInfo)[cluster2->index][i][0]->paper->index == paper4->index or (*movementsInfo)[cluster2->index][i][1]->paper->index == paper4->index or
                (*movementsInfo)[cluster2->index][i][2]->paper->index == paper4->index or (*movementsInfo)[cluster2->index][i][0]->paper->index == paper5->index or
                (*movementsInfo)[cluster2->index][i][1]->paper->index == paper5->index or (*movementsInfo)[cluster2->index][i][2]->paper->index == paper5->index or
                (*movementsInfo)[cluster2->index][i][0]->paper->index == paper6->index or (*movementsInfo)[cluster2->index][i][1]->paper->index == paper6->index or
                (*movementsInfo)[cluster2->index][i][2]->paper->index == paper6->index) { 
                
                std::swap((*movementsInfo)[cluster2->index][i], (*movementsInfo)[cluster2->index].back());
                (*movementsInfo)[cluster2->index].pop_back();
            }
        }
        

        vector<int> papersC1;
        auto clusterPapers = cluster1->getPapers();  //papers pertencentes ao cluster 1  
        for (auto p : *clusterPapers){
            papersC1.push_back(p->index); 
        }

        vector<int> papersC2;
        auto clusterPapers2 = cluster2->getPapers(); //papers pertencentes ao cluster 2
        for (auto p : *clusterPapers2){
            papersC2.push_back(p->index);
        } 


        for (auto paper : papers){            

            if (find(papersC1.begin(), papersC1.end(), paper->index) != papersC1.end()){    //se o paper for encontrado no c1, deleta o movimento pra inserir nele 

                ClustersInfo::deleteInsertMovement(paper->index, cluster1->index); 
            
            
            }else{                                                                         //se o paper n for encontrado no c1, recalcula o insert
                int benefit = cluster1->calculateBenefitOfInsertPaper(paper, false);
                ClustersInfo::setInsertMovement(paper->index, cluster1->index, benefit);

            }

            if (find(papersC2.begin(), papersC2.end(), paper->index) != papersC2.end()){  //se o paper for enconrado no c2...
                
                ClustersInfo::deleteInsertMovement(paper->index, cluster2->index);


            }else{
                int benefit = cluster2->calculateBenefitOfInsertPaper(paper, false);
                ClustersInfo::setInsertMovement(paper->index, cluster2->index, benefit);
            }
        }

        swap33Movement->first_trio_cluster[0]->updateRemoveValue();
        swap33Movement->first_trio_cluster[1]->updateRemoveValue();
        swap33Movement->first_trio_cluster[2]->updateRemoveValue();
        swap33Movement->second_trio_cluster[0]->updateRemoveValue();
        swap33Movement->second_trio_cluster[1]->updateRemoveValue();
        swap33Movement->second_trio_cluster[2]->updateRemoveValue();

        //------------------------------------------------------------------------------------------------
       
         //incluir os movimentos com os papers recém mudados no movementsInfo
        vector<MovementInfo*> moves_cluster1;
        vector<MovementInfo*> moves_cluster2;
        for(auto mov : *moves_single){                //juntando os MovementInfo de cada cluster envolvido na troca

            if(mov->cluster->index == swap33Movement->first_trio_cluster[0]->cluster->index){

                moves_cluster1.push_back(mov);
            }

            if(mov->cluster->index == swap33Movement->second_trio_cluster[1]->cluster->index){

                moves_cluster2.push_back(mov);
            }
        }


        vector<vector<MovementInfo*>> trios = {swap33Movement->first_trio_cluster, swap33Movement->second_trio_cluster};
        int it = 0;
        for (vector<MovementInfo*> trio : trios){                //reinserindo no vector movementsInfo os trios ajeitados
            
            vector<MovementInfo*>* moves = &moves_cluster1; 
            if(it == 1){
                moves = &moves_cluster2;
            }

            int index_new = trio[0]->cluster->index;
            for(int i = 0; i < 3; i++){

                MovementInfo* from_trio = trio[0];
                if(i == 1){
                    from_trio = trio[1];
                }else if(i == 2){
                    from_trio = trio[2];
                }

                for(auto mov : *moves){
                    if(mov->paper->index == from_trio->paper->index){
                        continue;
                    }
                
                    for(auto mov2 : *moves){
                        if(mov2->paper->index == from_trio->paper->index or mov2->paper->index == mov->paper->index){
                            continue;
                        }
                        
                        vector<MovementInfo*> new_trio = {from_trio, mov, mov2};
                        (*movementsInfo)[index_new].push_back(new_trio);
                    }
                }
            }
            it++;
        }

        vector<MovementInfo *> aux = {swap33Movement->first_trio_cluster[0], swap33Movement->second_trio_cluster[1]};   
        for (auto swap33 : aux){

            swap33->updateSwapMovementsInsideCluster();
            for (auto m : *moves_single)
            {
                if (m->cluster->index == swap33->cluster->index)
                {
                    m->updateRemoveValue();
                }
            }
        }

        //--------------------------------------------------------------------------------
        int greater_i = -1;
        int minor_i = 99999999;
        if(cluster1->index > cluster2->index){
            greater_i = cluster1->index;
            minor_i = cluster2->index;
        }else{
            greater_i = cluster2->index;
            minor_i = cluster1->index;
        }
     
        for(int i = minor_i+1; i < ClustersInfo::swap33Movements.size(); i++){              

            if(ClustersInfo::swap33Movements[minor_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[minor_i][i].size()-1; j >= 0; j--)            
            {
                auto mov = ClustersInfo::swap33Movements[minor_i][i][j];

                std::swap(mov, ClustersInfo::swap33Movements[minor_i][i].back());
                delete ClustersInfo::swap33Movements[minor_i][i][ClustersInfo::swap33Movements[minor_i][i].size()-1];
                ClustersInfo::swap33Movements[minor_i][i].pop_back();
            }
        }
        
        for(int i = 0; i < minor_i; i++){

            if(ClustersInfo::swap33Movements[i][minor_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[i][minor_i].size()-1; j >= 0; j--)           
            {
                auto mov = ClustersInfo::swap33Movements[i][minor_i][j];

                std::swap(mov, ClustersInfo::swap33Movements[i][minor_i].back());
                delete ClustersInfo::swap33Movements[i][minor_i][ClustersInfo::swap33Movements[i][minor_i].size()-1];
                ClustersInfo::swap33Movements[i][minor_i].pop_back();
            }
        }
        
        for(int i = greater_i+1; i < ClustersInfo::swap33Movements.size(); i++){              

            if(ClustersInfo::swap33Movements[greater_i][i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[greater_i][i].size()-1; j >= 0; j--)            
            {
                auto mov = ClustersInfo::swap33Movements[greater_i][i][j];

                std::swap(mov, ClustersInfo::swap33Movements[greater_i][i].back());
                delete ClustersInfo::swap33Movements[greater_i][i][ClustersInfo::swap33Movements[greater_i][i].size()-1];
                ClustersInfo::swap33Movements[greater_i][i].pop_back();
            }
        }
    
        for(int i = 0; i < greater_i; i++){

            if(ClustersInfo::swap33Movements[i][greater_i].size() == 0){
                continue;
            }

            for (int j = ClustersInfo::swap33Movements[i][greater_i].size()-1; j >= 0; j--)           
            {
                auto mov = ClustersInfo::swap33Movements[i][greater_i][j];

                std::swap(mov, ClustersInfo::swap33Movements[i][greater_i].back());
                delete ClustersInfo::swap33Movements[i][greater_i][ClustersInfo::swap33Movements[i][greater_i].size()-1];
                ClustersInfo::swap33Movements[i][greater_i].pop_back();
            }
        }

        //--------------------------------------------------------------------------------
        for (size_t j = 0; j < (*movementsInfo)[cluster1->index].size(); j++) {

            (*movementsInfo)[cluster1->index][j][0]->recalculateSwap33Movements((*movementsInfo)[cluster1->index][j][1], (*movementsInfo)[cluster1->index][j][2], movementsInfo);
            
        }
    
        for (auto m : (*movementsInfo)[cluster2->index]){

            m[0]->recalculateSwap33Movements(m[1], m[2], movementsInfo);
        }
    }


    void MovementInfo::recalculateSwap22Movements(MovementInfo* second, vector<vector<pair<MovementInfo*, MovementInfo*>>> *movements)
    {
        
        auto cluster1 = cluster;
        auto paper1 = paper;
        auto paper2 = second->paper;
        
        for (int i = 0; i < (*movements).size(); i++){

            vector<pair<MovementInfo*, MovementInfo*>> m = (*movements)[i];

            if (m.size() == 0){
                continue;
            }

            if(m[0].first->cluster->index == cluster1->index){
                continue;
            }

            int greater_i = -1;
            int minor_i = 99999999;
            if(cluster1->index > i){             //o i eh equivalente ao index do cluster2
                greater_i = cluster1->index;
                minor_i = i;
            }else{
                greater_i = i;
                minor_i = cluster1->index;
            }


            for(int j = 0; j < m.size(); j++){

                pair<MovementInfo*, MovementInfo*> n = (*movements)[i][j];
                
                int benefitOfPaper12inCluster1 = removeValue + second->getRemoveValue(); //- BenefitsUtil::getBenefit(paper1->index, paper2->index);

                auto cluster2 = n.first->cluster;
                auto paper3 = n.first->paper;
                auto paper4 = n.second->paper;

                int benefitOfPaper34inCluster2 = n.first->getRemoveValue() + n.second->getRemoveValue();

                int benefitOfPaper12inCluster2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index) + ClustersInfo::getInsertMovement(paper2->index, cluster2->index)
                                                +(2*BenefitsUtil::getBenefit(paper1->index, paper2->index));

                int benefitOfPaper34inCluster1 = ClustersInfo::getInsertMovement(paper3->index, cluster1->index) + ClustersInfo::getInsertMovement(paper4->index, cluster1->index)
                                                +(2*BenefitsUtil::getBenefit(paper3->index, paper4->index));


                int swapBenefitP1P2ToC2 = benefitOfPaper12inCluster2 - benefitOfPaper12inCluster1 - BenefitsUtil::getBenefit(paper1->index, paper3->index) 
                                            - BenefitsUtil::getBenefit(paper1->index, paper4->index)- BenefitsUtil::getBenefit(paper2->index, paper3->index)
                                            - BenefitsUtil::getBenefit(paper2->index, paper4->index);
                
                int swapBenefitP3P4ToC1 = benefitOfPaper34inCluster1 - benefitOfPaper34inCluster2 - BenefitsUtil::getBenefit(paper1->index, paper3->index) 
                                            - BenefitsUtil::getBenefit(paper1->index, paper4->index)- BenefitsUtil::getBenefit(paper2->index, paper3->index)
                                            - BenefitsUtil::getBenefit(paper2->index, paper4->index);

                int swapBenefit = swapBenefitP1P2ToC2 + swapBenefitP3P4ToC1;

                Swap22MovementInfo *swap22 = new Swap22MovementInfo(pair<MovementInfo*,MovementInfo*>(this, second), n, swapBenefit);
                ClustersInfo::setSwap22Movement(minor_i, greater_i, swap22);
            }
        }
    }

    void MovementInfo::recalculateSwap33Movements(MovementInfo* second, MovementInfo* third, vector<vector<vector<MovementInfo*>>>* movements)
    {
      
        auto cluster1 = cluster;
        auto paper1 = paper;
        auto paper2 = second->paper;
        auto paper3 = third->paper;

        for (int i = 0; i < (*movements).size(); i++){

            vector<vector<MovementInfo*>> m = (*movements)[i];

            if (m.size() == 0){
                continue;
            }

            if(m[0][0]->cluster->index == cluster1->index){
                continue;
            }

            int greater_i = -1;
            int minor_i = 99999999;
            if(cluster1->index > i){             //o i eh equivalente ao index do cluster2
                greater_i = cluster1->index;
                minor_i = i;
            }else{
                greater_i = i;
                minor_i = cluster1->index;
            }


            for(int j = 0; j < m.size(); j++){

                MovementInfo* first2 = (*movements)[i][j][0];
                MovementInfo* second2 = (*movements)[i][j][1];
                MovementInfo* third2 = (*movements)[i][j][2];

                int benefitOfPaper123inCluster1 =  removeValue + second->getRemoveValue() + third->getRemoveValue();  //beneficio a ser retirado 

                auto cluster2 = first2->cluster;
                auto paper4 = first2->paper;
                auto paper5 = second2->paper;
                auto paper6 = third2->paper;
            

                int benefitOfPaper456inCluster2 = first2->getRemoveValue() + second2->getRemoveValue() + third2->getRemoveValue();   //beneficio a ser retirado

                int benefitOfPaper123inCluster2 = ClustersInfo::getInsertMovement(paper1->index, cluster2->index) + ClustersInfo::getInsertMovement(paper2->index, cluster2->index)
                                                +ClustersInfo::getInsertMovement(paper3->index, cluster2->index) + (2*BenefitsUtil::getBenefit(paper1->index, paper2->index))
                                                +(2*BenefitsUtil::getBenefit(paper3->index, paper1->index)) + (2*BenefitsUtil::getBenefit(paper3->index, paper2->index));
                

                int benefitOfPaper456inCluster1 = ClustersInfo::getInsertMovement(paper4->index, cluster1->index) + ClustersInfo::getInsertMovement(paper5->index, cluster1->index)  
                                                +ClustersInfo::getInsertMovement(paper6->index, cluster1->index) + (2*BenefitsUtil::getBenefit(paper4->index, paper5->index))
                                                +(2*BenefitsUtil::getBenefit(paper5->index, paper6->index)) + (2*BenefitsUtil::getBenefit(paper4->index, paper6->index));


                int benefitOfP1andP4P5P6 = BenefitsUtil::getBenefit(paper1->index, paper4->index) + BenefitsUtil::getBenefit(paper1->index, paper5->index) + BenefitsUtil::getBenefit(paper1->index, paper6->index);
                int benefitOfP2andP4P5P6 = BenefitsUtil::getBenefit(paper2->index, paper4->index) + BenefitsUtil::getBenefit(paper2->index, paper5->index) + BenefitsUtil::getBenefit(paper2->index, paper6->index);
                int benefitOfP3andP4P5P6 = BenefitsUtil::getBenefit(paper3->index, paper4->index) + BenefitsUtil::getBenefit(paper3->index, paper5->index) + BenefitsUtil::getBenefit(paper3->index, paper6->index);
                int benefitOfP4andP1P2P3 = BenefitsUtil::getBenefit(paper4->index, paper1->index) + BenefitsUtil::getBenefit(paper4->index, paper2->index) + BenefitsUtil::getBenefit(paper4->index, paper3->index);
                int benefitOfP5andP1P2P3 = BenefitsUtil::getBenefit(paper5->index, paper1->index) + BenefitsUtil::getBenefit(paper5->index, paper2->index) + BenefitsUtil::getBenefit(paper5->index, paper3->index);
                int benefitOfP6andP1P2P3 = BenefitsUtil::getBenefit(paper6->index, paper1->index) + BenefitsUtil::getBenefit(paper6->index, paper2->index) + BenefitsUtil::getBenefit(paper6->index, paper3->index);

                int swapBenefitP1P2P3ToC2 = benefitOfPaper123inCluster2 - benefitOfPaper123inCluster1 - benefitOfP4andP1P2P3 - benefitOfP5andP1P2P3 - benefitOfP6andP1P2P3;
                int swapBenefitP4P5P6ToC1 = benefitOfPaper456inCluster1 - benefitOfPaper456inCluster2 - benefitOfP1andP4P5P6 - benefitOfP2andP4P5P6 - benefitOfP3andP4P5P6;

                int swapBenefit = swapBenefitP1P2P3ToC2 + swapBenefitP4P5P6ToC1;
                vector<MovementInfo*> trio = {this, second,third};
                vector<MovementInfo*> trio2 = {first2, second2, third2};
                Swap33MovementInfo *swap33 = new Swap33MovementInfo(trio, trio2, swapBenefit);
                ClustersInfo::setSwap33Movement(minor_i, greater_i, swap33);
            }
        }   
    }


    void MovementInfo::makeSwapMovement(vector<MovementInfo *> * movements, vector<Paper *> * papers)
    {
        vector<MovementInfo *> &move = *movements;
        auto paper1 = move[0]->paper;
        auto cluster1 = move[0]->cluster;

        auto paper2 = move[1]->paper;
        auto cluster2 = move[1]->cluster;

        cluster1->removePaper(paper1);
        cluster2->removePaper(paper2);

        cluster1->add(paper2);
        cluster2->add(paper1);

        ClustersInfo::deleteInsertMovement(paper1->index, cluster2->index);
        ClustersInfo::deleteInsertMovement(paper2->index, cluster1->index);

        move[0]->cluster = cluster2;
        move[1]->cluster = cluster1;

        // custo dos papers voltarem ao cluster original
        vector<int> papersC1;
        auto clusterPapers = cluster1->getPapers();
        for (auto p : *clusterPapers)
            papersC1.push_back(p->index);

        vector<int> papersC2;
        auto clusterPapers2 = cluster2->getPapers();
        for (auto p : *clusterPapers2)
            papersC2.push_back(p->index);

        for (auto paper : *papers)
        {
            if (find(papersC1.begin(), papersC1.end(), paper->index) != papersC1.end())
            {
                ClustersInfo::deleteInsertMovement(paper->index, cluster1->index);
            }
            else
            {
                int benefit = cluster1->calculateBenefitOfInsertPaper(paper, false);
                ClustersInfo::setInsertMovement(paper->index, cluster1->index, benefit);
            }

            if (find(papersC2.begin(), papersC2.end(), paper->index) != papersC2.end())
            {
                ClustersInfo::deleteInsertMovement(paper->index, cluster2->index);
            }
            else
            {
                int benefit = cluster2->calculateBenefitOfInsertPaper(paper, false);
                ClustersInfo::setInsertMovement(paper->index, cluster2->index, benefit);
            }
        }
        move[0]->updateRemoveValue();
        move[1]->updateRemoveValue();
    }
    // getters and setters
    Paper *MovementInfo::getPaper()
    {
        return paper;
    }

    void MovementInfo::setPaper(Paper * paper)
    {
        this->paper = paper;
    }

    Cluster *MovementInfo::getCluster()
    {
        return cluster;
    }

    void MovementInfo::setCluster(Cluster * cluster)
    {
        this->cluster = cluster;
    }

    int MovementInfo::getRemoveValue()
    {
        return removeValue;
    }

    void MovementInfo::setRemoveValue(int removeValue)
    {
        this->removeValue = removeValue;
    }

    int MovementInfo::getBestMovementValue()
    {
        return bestMovementValue;
    }

    int MovementInfo::getBestSwapMovement()
    {
        return bestSwapMovement;
    }

    void MovementInfo::setBestSwapMovement(int bestSwapMovement)
    {
        this->bestClusterIndexToMove = bestSwapMovement;
    }

    int MovementInfo::getBestClusterIndexToMove() {
        return bestClusterIndexToMove;
    }

    int MovementInfo::getClusterToSwap()
    {
        return clusterToSwap;
    }

    void MovementInfo::setClusterToSwap(int clusterToSwap)
    {
        this->clusterToSwap = clusterToSwap;
    }

    string MovementInfo::toString()
    {
        return "p" + to_string(paper->index) + " - rm " + to_string(removeValue);
    }

    string MovementInfo::removeValueToString(vector<MovementInfo *> * movements)
    {
        stringstream text;
        vector<int> rm(movements->size());
        for (MovementInfo *m : *movements)
        {
            rm[m->getPaper()->index] = m->getRemoveValue();
        }
        for (int i = 0; i < rm.size(); i++)
        {
            text << "(" << i << ", " << rm[i] << ")"
                 << " ";
        }
        text << "\n";
        return text.str();
    }

    string MovementInfo::swapTableToString()
    {
        stringstream text;
        auto swapMovements = ClustersInfo::swapMovements;
        for (int i = 0; i < swapMovements.size(); i++)
        {
            for (int j = 0; j < swapMovements[i].size(); j++)
            {
                if (i > j)
                {
                    text << "x ";
                    continue;
                }
                if (swapMovements[i][j] != NULL)
                    text << swapMovements[i][j]->getValue();
                else
                    text << "n";
                text << " ";
            }
            text << endl;
        }
        return text.str();
    }

    string MovementInfo::insertTableToString()
    {
        // stringstream text;
        // auto insertMovements = ClustersInfo::insertMovements;
        // for (int i = 0; i < insertMovements.size(); i++) {
        //     for (int j = 0; j < insertMovements[i].size(); j++) {
        //         text << insertMovements[i][j];
        //         text << " ";
        //     }
        //     text << endl;
        // }
        // return text.str();
        return "";
    }
